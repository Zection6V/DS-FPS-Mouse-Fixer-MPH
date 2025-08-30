// main.cpp

/*
目的: DS用マウス入力補正ツールの単一翻訳単位化。UI作成、設定読込、入力監視、各種ゲーム操作マクロを1ファイルに集約する.
*/

// Unicode強制
#ifndef UNICODE
#  define UNICODE
#endif
#ifndef _UNICODE
#  define _UNICODE
#endif

#if !defined(WIN32_LEAN_AND_MEAN)
#  define WIN32_LEAN_AND_MEAN 1
#endif
#include <windows.h>
#include <winuser.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cwchar>
#include <clocale>
#include <string>
#include <fstream>
#include <unordered_map>
#include <thread>
#include <atomic>
#include <chrono>
#include <sstream>

#ifndef __forceinline
  #if defined(_MSC_VER)
    #define __forceinline __forceinline
  #else
    #define __forceinline inline __attribute__((always_inline))
  #endif
#endif

//======================== グローバル状態 ========================//

int on = 1, paused = 0;

RECT windowRect;

int gameSelected = 7;

constexpr int START_BUTTON_ID       = 1;
constexpr int DROPDOWN_ID           = 3;
constexpr int CHECK_USE_SAVED_ID    = 10;
constexpr int STATIC_SAVED_LABEL_ID = 14;

#define WINDOW_WIDTH  400
#define WINDOW_HEIGHT 320  // チェック＋ラベル分だけ拡張

int   leftBound, rightBound, topBound, bottomBound, weaponGrenadeToggle;
POINT center;
RECT  playSpace;

int mouseUp = MOUSEEVENTF_RIGHTUP, mouseDown = MOUSEEVENTF_RIGHTDOWN, activeMouse = 0x02;

float screenWidthRatio, screenHeightRatio, referenceWidth = 587.0f, referenceHeight = 441.0f;

float distanceFromBoarderSides = 50.0f, distanceFromBoarderTop = 100.0f, distanceNextButton = 100.0f;

int mouseResetWait = 33, buttonWait = 33, swapWait = 220, keyWait = 30;
int autoMouseDrag  = 1;

std::atomic<bool> running(true);
std::atomic<bool> leftButtonDown(false);

// クリップ保存関連
static const char* kClipFile = "DS_ClipRegion.txt";
bool gUseSavedClipAtStart = false;
bool gIsJapanese = false;

HWND gHwndMain = nullptr;
HWND gHCheckUseSaved = nullptr;
HWND gHLabelSaved = nullptr;

//======================== 前方宣言 ========================//

struct Input;

LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);
int      WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int);

__forceinline void GetActiveMouseDown(const int *waitTime);
__forceinline void GetActiveMouseUp  (const int *waitTime);
__forceinline void LockPlaySpace();
__forceinline void CursorUp();
__forceinline void ResetPos();
__forceinline int  ResetPosAfterButton();
void               removeSubstring(char *string, const char *substring);

void Zoom_MPH();
void AmmoTypeOne_MPH();
void AmmoTypeTwo_MPH();
void AmmoTypeThree_MPH();
void ScanVision_MPH();
void Ball_MPH();
void WeaponOne_MPH();
void WeaponTwo_MPH();
void WeaponThree_MPH();
void WeaponFour_MPH();
void WeaponFive_MPH();
void WeaponSix_MPH();
void ClickOK_MPH();
void ClickYes_MPH();
void ClickNo_MPH();
void Left_MPH();
void Right_MPH();

void DoNothing();
void Pause();
void Kill();
__forceinline void RunInput(struct Input *input);
__forceinline void InputCheck(struct Input* inputs, int length);
Input* InitializeStateCheckers();

void     ConfigReader();
Input*   MPH_Input_Reader();
Input*   FileHandle();

void ProcessRawInput(LPARAM lParam);

int  Run();
int  InitializePoints(POINT* bottomLeft, POINT* topRight);

// クリップ保存/適用ヘルパ
bool LoadSavedClip(RECT* outRect, int* outLeft, int* outTop, int* outRight, int* outBottom);
bool SaveCurrentClip();
void ComputeDerivedFromBounds();
bool TryApplySavedClip();
void UpdateSavedClipLabel();

// UIヘルパ
static void PaintBackground(HWND hwnd, COLORREF color) {
    HBRUSH brush = CreateSolidBrush(color);
    SetClassLongPtrW(hwnd, GCLP_HBRBACKGROUND, reinterpret_cast<LONG_PTR>(brush));
    RedrawWindow(hwnd, nullptr, nullptr, RDW_ERASE | RDW_INVALIDATE);
    SendMessageW(hwnd, WM_PAINT, 0, 0);
}
static HFONT CreateUIFont(bool isJapanese) {
    return CreateFontW(-12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                       DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                       DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
                       isJapanese ? L"Meiryo UI" : L"Segoe UI");
}
static void ApplyUIFont(HFONT hFont, std::initializer_list<HWND> ctrls) {
    for (HWND h : ctrls) {
        if (h) SendMessageW(h, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), TRUE);
    }
}

//======================== 送出ユーティリティ（SendInput） ========================//

static __forceinline void SendMouseButton(WORD flags) {
    INPUT in{};
    in.type = INPUT_MOUSE;
    in.mi.dwFlags = flags;
    SendInput(1, &in, sizeof(in));
}

static __forceinline void RightDown() { SendMouseButton(MOUSEEVENTF_RIGHTDOWN); }
static __forceinline void RightUp()   { SendMouseButton(MOUSEEVENTF_RIGHTUP);   }

static __forceinline void SendKey(WORD vk, bool down) {
    INPUT in{};
    in.type = INPUT_KEYBOARD;
    in.ki.wVk = vk;
    in.ki.dwFlags = down ? 0 : KEYEVENTF_KEYUP;
    SendInput(1, &in, sizeof(in));
}

//======================== 構造体定義 ========================//

struct Input {
  int   isDown;
  void (*gameFunction)();
  int   inpNum;
};

//======================== ユーティリティ(汎用) ========================//

__forceinline void GetActiveMouseDown(const int *waitTime) {
    while (GetAsyncKeyState(activeMouse) >= 0) { }
    Sleep(*waitTime);
}

__forceinline void GetActiveMouseUp(const int *waitTime) {
    while (GetAsyncKeyState(activeMouse) < 0) { }
    Sleep(*waitTime);
}

__forceinline void LockPlaySpace() {
    ClipCursor(&playSpace);
}

__forceinline void CursorUp() {
    RightUp();
    GetActiveMouseUp(&mouseResetWait);
}

static __forceinline void lockPlaySpaceAfterResetPos() {
    RECT tempRect;
    GetClipCursor(&tempRect);
    if (tempRect.left != playSpace.left || tempRect.right != playSpace.right ||
        tempRect.top  != playSpace.top  || tempRect.bottom != playSpace.bottom) {
        LockPlaySpace();
    }
}

__forceinline void ResetPos() {
    RightUp();
    GetActiveMouseUp(&mouseResetWait);
    SetCursorPos(center.x, center.y);

    if (autoMouseDrag) {
        RightDown();
    }

    std::thread(lockPlaySpaceAfterResetPos).detach();
}

__forceinline int ResetPosAfterButton() {
    GetActiveMouseDown(&buttonWait);
    RightUp();
    GetActiveMouseUp(&buttonWait);
    SetCursorPos(center.x, center.y);
    if (autoMouseDrag) {
        RightDown();
    }
    return 0;
}

//======================== MPH向けゲーム関数 ========================//

static __forceinline void Zoom_MPH_impl() {
    SendKey('M', true);
    Sleep(keyWait);
    SendKey('M', false);
}

void Zoom_MPH() {
    std::thread(Zoom_MPH_impl).detach();
}

void AmmoTypeOne_MPH() {
    CursorUp();
    SetCursorPos(center.x - LONG(distanceFromBoarderSides * 2.f), topBound + LONG(distanceFromBoarderTop / 2.f));
    RightDown();
    ResetPosAfterButton();
}

void AmmoTypeTwo_MPH() {
    CursorUp();
    SetCursorPos(center.x, topBound + LONG(distanceFromBoarderTop / 2.f));
    RightDown();
    ResetPosAfterButton();
}

void AmmoTypeThree_MPH() {
    CursorUp();
    SetCursorPos(center.x + LONG(distanceFromBoarderSides * 2.f), topBound + LONG(distanceFromBoarderTop / 2.f));
    RightDown();
    ResetPosAfterButton();
}

void ScanVision_MPH() {
    CursorUp();
    SetCursorPos(center.x, bottomBound - LONG(distanceFromBoarderTop / 2.f));
    RightDown();
    Sleep(500);
    ResetPosAfterButton();
}

void Ball_MPH() {
    CursorUp();
    SetCursorPos(rightBound - LONG(distanceFromBoarderSides * 1.5f), bottomBound - LONG(distanceFromBoarderTop / 2.f));
    RightDown();
    ResetPosAfterButton();
}

void WeaponOne_MPH() {
    CursorUp();
    SetCursorPos(rightBound - LONG(distanceFromBoarderSides * 2.f), topBound + LONG(distanceFromBoarderTop));
    RightDown();
    GetActiveMouseDown(&mouseResetWait);
    SetCursorPos(center.x - LONG(distanceFromBoarderSides * 1.5f), topBound + LONG(distanceFromBoarderTop));
    ResetPosAfterButton();
}

void WeaponTwo_MPH() {
    CursorUp();
    SetCursorPos(rightBound - LONG(distanceFromBoarderSides * 2.f), topBound + LONG(distanceFromBoarderTop));
    RightDown();
    GetActiveMouseDown(&mouseResetWait);
    SetCursorPos(center.x - LONG(distanceFromBoarderSides * 1.5f), center.y - LONG(distanceFromBoarderTop / 4.f));
    ResetPosAfterButton();
}

void WeaponThree_MPH() {
    CursorUp();
    SetCursorPos(rightBound - LONG(distanceFromBoarderSides * 2.f), topBound + LONG(distanceFromBoarderTop));
    RightDown();
    GetActiveMouseDown(&mouseResetWait);
    SetCursorPos(center.x - LONG(distanceFromBoarderSides), center.y + LONG(distanceFromBoarderTop / 1.5f));
    ResetPosAfterButton();
}

void WeaponFour_MPH() {
    CursorUp();
    SetCursorPos(rightBound - LONG(distanceFromBoarderSides * 2.f), topBound + LONG(distanceFromBoarderTop));
    RightDown();
    GetActiveMouseDown(&mouseResetWait);
    SetCursorPos(center.x + LONG(distanceFromBoarderSides / 2.f), bottomBound - LONG(distanceFromBoarderTop / 1.2f));
    ResetPosAfterButton();
}

void WeaponFive_MPH() {
    CursorUp();
    SetCursorPos(rightBound - LONG(distanceFromBoarderSides * 2.f), topBound + LONG(distanceFromBoarderTop));
    RightDown();
    GetActiveMouseDown(&mouseResetWait);
    SetCursorPos(center.x + LONG(distanceFromBoarderSides * 2.f), bottomBound - LONG(distanceFromBoarderTop / 1.5f));
    ResetPosAfterButton();
}

void WeaponSix_MPH() {
    CursorUp();
    SetCursorPos(rightBound - LONG(distanceFromBoarderSides * 1.8f), bottomBound - LONG(distanceFromBoarderTop / 2.f));
    RightDown();
    GetActiveMouseDown(&mouseResetWait);
    ResetPosAfterButton();
}

void ClickOK_MPH() {
    CursorUp();
    SetCursorPos(center.x, bottomBound - LONG(distanceFromBoarderTop * 1.2f));
    RightDown();
    ResetPosAfterButton();
}

void ClickYes_MPH() {
    CursorUp();
    SetCursorPos(center.x - LONG(distanceFromBoarderSides * 2.f), bottomBound - LONG(distanceFromBoarderTop * 1.2f));
    RightDown();
    ResetPosAfterButton();
}

void ClickNo_MPH() {
    CursorUp();
    SetCursorPos(center.x + LONG(distanceFromBoarderSides * 2.f), bottomBound - LONG(distanceFromBoarderTop * 1.2f));
    RightDown();
    ResetPosAfterButton();
}

void Left_MPH() {
    CursorUp();
    SetCursorPos(center.x - LONG(distanceFromBoarderSides * 2.5f), bottomBound - LONG(distanceFromBoarderTop * 1.2f));
    RightDown();
    ResetPosAfterButton();
}

void Right_MPH() {
    CursorUp();
    SetCursorPos(center.x + LONG(distanceFromBoarderSides * 2.5f), bottomBound - LONG(distanceFromBoarderTop * 1.2f));
    RightDown();
    ResetPosAfterButton();
}

//======================== ユーティリティ(設定/文字列) ========================//

void removeSubstring(char *string, const char *substring) {
    char *match = strstr(string, substring);
    size_t subLen = strlen(substring);
    if (match != nullptr) {
        memmove(match, match + subLen, strlen(match + subLen) + 1);
    }
}

void ConfigReader() {
    const std::string filename = "DS_Config.txt";
    std::ifstream inFile(filename);

    if (!inFile) {
        std::ofstream outFile(filename);
        outFile <<
            "VALUES\n\n"
            "mouseResetWait = 33\n"
            "buttonWait = 33\n"
            "swapWait = 220\n"
            "keyWait = 30\n"
            "autoMouseDrag = 1\n";
        outFile.close();
        inFile.open(filename);
    }

    std::unordered_map<std::string, int*> configMap;
    configMap["mouseResetWait"] = &mouseResetWait;
    configMap["buttonWait"]     = &buttonWait;
    configMap["swapWait"]       = &swapWait;
    configMap["keyWait"]        = &keyWait;

    std::string line, key, value;
    size_t equalsPos;
    while (std::getline(inFile, line)) {
        if ((equalsPos = line.find(" = ")) != std::string::npos) {
            key   = line.substr(0, equalsPos);
            value = line.substr(equalsPos + 3);

            auto it = configMap.find(key);
            if (it != configMap.end()) {
                *(it->second) = std::abs(std::stoi(value));
            } else if (key == "autoMouseDrag") {
                autoMouseDrag = std::stoi(value);
            }
        }
    }
}

//======================== ファイルハンドラ ========================//

int totalInputs = 0;

static void GenerateInputs_MPH(FILE *inputFile, struct Input* inputList) {
    int i = 0;
    int hexVal;
    char line[100];

    struct InputMapping { const char* key; void (*gameFunction)(); };
    InputMapping inputMappings[] = {
        {"Shoot",                nullptr /* Shoot_Button_Down(未使用) */},
        {"Mouse Manual Reset",   ResetPos},
        {"Swap To Main Weapon",  AmmoTypeOne_MPH},
        {"Swap To Missiles",     AmmoTypeTwo_MPH},
        {"Swap To Third Weapon", AmmoTypeThree_MPH},
        {"Zoom",                 Zoom_MPH},
        {"Special Weapon 1",     WeaponOne_MPH},
        {"Special Weapon 2",     WeaponTwo_MPH},
        {"Special Weapon 3",     WeaponThree_MPH},
        {"Special Weapon 4",     WeaponFour_MPH},
        {"Special Weapon 5",     WeaponFive_MPH},
        {"Special Weapon 6",     WeaponSix_MPH},
        {"Scan Vision",          ScanVision_MPH},
        {"Ball",                 Ball_MPH},
        {"Screen Tap Jump",      nullptr /* ScreenTapJump(未使用) */},
        {"Ok (menu)",            ClickOK_MPH},
        {"Yes (menu)",           ClickYes_MPH},
        {"No (menu)",            ClickNo_MPH},
        {"Left (menu)",          Left_MPH},
        {"Right (menu)",         Right_MPH},
        {nullptr,                nullptr}
    };

    while (fgets(line, sizeof(line), inputFile) != nullptr) {
        line[strcspn(line, "\n")] = '\0';
        for (int j = 0; inputMappings[j].key != nullptr; j++) {
            char searchStr[100];
            snprintf(searchStr, sizeof(searchStr), "%s = ", inputMappings[j].key);
            if (strstr(line, searchStr) != nullptr) {
                removeSubstring(line, searchStr);
                sscanf(line, "%x", &hexVal);
                inputList[i].isDown       = 0;
                inputList[i].gameFunction = inputMappings[j].gameFunction ? inputMappings[j].gameFunction : DoNothing;
                inputList[i].inpNum       = hexVal;
                i++;
                break;
            }
        }
    }
}

Input* MPH_Input_Reader() {
    totalInputs = 0;

    FILE* MPHInputs = fopen("DS_MPH_Controls.txt", "r");
    if (MPHInputs == nullptr) {
        MPHInputs = fopen("DS_MPH_Controls.txt", "w");
        if (MPHInputs != nullptr) {
            const char* header =
                "Edit Controls for Metroid Prime Hunters in this file.\n"
                "Inputs must be denoted by their corresponding hex values as denoted here: \n"
                "https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes \n\n";
            fprintf(MPHInputs, "%s", header);

            const char* inputs[] = {
                "Shoot = 0x01",
                "Mouse Manual Reset = 0x04",
                "Swap To Main Weapon = 0x06",
                "Swap To Missiles = 0x05",
                "Swap To Third Weapon = 0x52",
                "Special Weapon 1 = 0x31",
                "Special Weapon 2 = 0x32",
                "Special Weapon 3 = 0x33",
                "Special Weapon 4 = 0x34",
                "Special Weapon 5 = 0x35",
                "Special Weapon 6 = 0x36",
                "Scan Vision = 0x56",
                "Ball = 0xA2",
                "Screen Tap Jump = 0xA3",
                "Zoom = 0x02",
                "Ok (menu) = 0x46",
                "Yes (menu) = 0x05",
                "No (menu) = 0x06",
                "Left (menu) = 0x5A",
                "Right (menu) = 0x58"
            };
            for (const char* in : inputs) fprintf(MPHInputs, "%s\n", in);
            fclose(MPHInputs);

            totalInputs = int(sizeof(inputs) / sizeof(inputs[0]));
            FILE* f = fopen("DS_MPH_Controls.txt", "r");
            Input* inputList = new Input[totalInputs];
            GenerateInputs_MPH(f, inputList);
            fclose(f);
            return inputList;
        }
        return nullptr;
    } else {
        char line[100];
        const char* inputStrings[] = {
            "Shoot = ","Mouse Manual Reset = ","Swap To Main Weapon = ","Swap To Missiles = ",
            "Swap To Third Weapon = ","Special Weapon 1 = ","Special Weapon 2 = ",
            "Special Weapon 3 = ","Special Weapon 4 = ","Special Weapon 5 = ",
            "Special Weapon 6 = ","Scan Vision = ","Ball = ","Screen Tap Jump = ",
            "Zoom = ","Ok (menu) = ","Yes (menu) = ","No (menu) = ",
            "Left (menu) = ","Right (menu) = "
        };
        while (fgets(line, sizeof(line), MPHInputs) != nullptr) {
            line[strcspn(line, "\n")] = '\0';
            bool found = false;
            for (const char* s : inputStrings) {
                if (strstr(line, s) != nullptr) { found = true; break; }
            }
            if (found) totalInputs++;
        }
        Input* inputList = new Input[totalInputs];
        fseek(MPHInputs, 0, SEEK_SET);
        GenerateInputs_MPH(MPHInputs, inputList);
        fclose(MPHInputs);
        return inputList;
    }
}

Input* FileHandle() {
    ConfigReader();
    return MPH_Input_Reader();
}

//======================== 入力監視・状態操作 ========================//

void DoNothing() {}

void Pause() {
    RightUp();
    SwapMouseButton(paused);
    SendKey('N', false);
    paused = !paused;
    if (paused) {
        ClipCursor(nullptr);
    } else {
        ResetPos();
    }
}

void Kill() {
    on = 0;
    paused = 1;
    RightUp();
    SendKey('N', false);
    SwapMouseButton(0);
    ClipCursor(nullptr);
}

__forceinline void RunInput(struct Input *input) {
    SHORT keyState = GetAsyncKeyState(input->inpNum);
    BOOL  isKeyDown = (keyState & 0x8000) != 0;

    if (input->inpNum == 0x02) {
        if (!isKeyDown && input->isDown) {
            if (autoMouseDrag) RightDown();
            if (input->gameFunction) input->gameFunction();
        }
    } else {
        if (isKeyDown && !input->isDown) {
            if (input->gameFunction) input->gameFunction();
        } else if (!isKeyDown && input->isDown) {
        }
    }
    input->isDown = isKeyDown;
}

__forceinline void InputCheck(struct Input* inputs, int length) {
    for (int i = 0; i < length; i++) RunInput(&inputs[i]);
}

Input* InitializeStateCheckers() {
    auto* stateCheckers = new Input[2];
    stateCheckers[0].isDown = false; stateCheckers[0].gameFunction = Pause; stateCheckers[0].inpNum = VK_RSHIFT; // 0xA1
    stateCheckers[1].isDown = false; stateCheckers[1].gameFunction = Kill;  stateCheckers[1].inpNum = VK_BACK;   // 0x08
    return stateCheckers;
}

//======================== Raw Input処理 ========================//

static __forceinline void ProcessKeyInput(UINT vKey, bool isKeyDown) {
    if (vKey == 'C') {
        if (isKeyDown) {
            SendKey('M', true);
        } else {
            SendKey('M', false);
        }
    }
}

static __forceinline void ProcessMouseInput(RAWINPUT* raw) {
    if (raw->data.mouse.usButtonFlags & RI_MOUSE_LEFT_BUTTON_DOWN) {
        if (!leftButtonDown.exchange(true)) SendKey('N', true);
    } else if (raw->data.mouse.usButtonFlags & RI_MOUSE_LEFT_BUTTON_UP) {
        if (leftButtonDown.exchange(false)) SendKey('N', false);
    }
}

void ProcessRawInput(LPARAM lParam) {
    if (paused) return;

    UINT dwSize = sizeof(RAWINPUT);
    static BYTE lpb[sizeof(RAWINPUT)];
    if (GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER)) == (UINT)-1)
        return;

    auto* raw = reinterpret_cast<RAWINPUT*>(lpb);
    if (raw->header.dwType == RIM_TYPEKEYBOARD) {
        ProcessKeyInput(raw->data.keyboard.VKey, !(raw->data.keyboard.Flags & RI_KEY_BREAK));
    } else if (raw->header.dwType == RIM_TYPEMOUSE) {
        ProcessMouseInput(raw);
    }
}

//======================== クリップ保存/適用ヘルパ ========================//

bool LoadSavedClip(RECT* outRect, int* outLeft, int* outTop, int* outRight, int* outBottom) {
    std::ifstream fin(kClipFile);
    if (!fin) return false;
    int l, t, r, b;
    if (!(fin >> l >> t >> r >> b)) return false;
    if (l >= r || t >= b) return false;

    const int maxW = GetSystemMetrics(SM_CXSCREEN);
    const int maxH = GetSystemMetrics(SM_CYSCREEN);
    if (l < 0 || t < 0 || r > maxW || b > maxH) return false;
    if (r - l < 10 || b - t < 10) return false;

    if (outRect) SetRect(outRect, l, t, r, b);
    if (outLeft)   *outLeft = l;
    if (outTop)    *outTop  = t;
    if (outRight)  *outRight= r;
    if (outBottom) *outBottom=b;
    return true;
}

bool SaveCurrentClip() {
    std::ofstream fout(kClipFile, std::ios::trunc);
    if (!fout) return false;
    fout << leftBound << ' ' << topBound << ' ' << rightBound << ' ' << bottomBound << '\n';
    return bool(fout);
}

void ComputeDerivedFromBounds() {
    SetRect(&playSpace, leftBound, topBound, rightBound, bottomBound);

    float screenWidth  = float(rightBound - leftBound);
    float screenHeight = float(bottomBound - topBound);

    screenWidthRatio  = screenWidth  / referenceWidth;
    screenHeightRatio = screenHeight / referenceHeight;

    distanceFromBoarderSides = 50.0f  * screenWidthRatio;
    distanceNextButton       = 100.0f * screenWidthRatio;
    distanceFromBoarderTop   = 100.0f * screenHeightRatio;

    center.x = ((rightBound - leftBound) / 2) + leftBound;
    center.y = ((bottomBound - topBound) / 2) + topBound;
}

bool TryApplySavedClip() {
    RECT r{};
    int l, t, rr, b;
    if (!LoadSavedClip(&r, &l, &t, &rr, &b)) return false;
    leftBound = l; topBound = t; rightBound = rr; bottomBound = b;
    ComputeDerivedFromBounds();
    ResetPos();
    return true;
}

void UpdateSavedClipLabel() {
    if (!gHLabelSaved) return;
    RECT r{}; int l,t,rr,b;
    bool ok = LoadSavedClip(&r, &l, &t, &rr, &b);
    std::wstringstream wss;
    if (ok) {
        if (gIsJapanese) {
            wss << L"保存済み範囲: (" << l << L"," << t << L") - (" << rr << L"," << b << L")";
        } else {
            wss << L"Saved clip: (" << l << L"," << t << L") - (" << rr << L"," << b << L")";
        }
    } else {
        wss << (gIsJapanese ? L"保存済み範囲: なし" : L"Saved clip: none");
    }
    SetWindowTextW(gHLabelSaved, wss.str().c_str());

    if (gHCheckUseSaved) {
        SendMessageW(gHCheckUseSaved, BM_SETCHECK, ok ? BST_CHECKED : BST_UNCHECKED, 0);
    }
}

//======================== 初期クリックによる範囲決定（確定時に自動保存） ========================//

int InitializePoints(POINT* bottomLeft, POINT* topRight) {
    int initialize = 0;
    int clickDown  = 0;

    while (initialize <= 1) {
        if (GetAsyncKeyState(VK_LBUTTON) & 0x8000) {
            if (!clickDown) {
                clickDown = 1;
                if (initialize == 0) {
                    GetCursorPos(bottomLeft);
                    initialize++;
                } else {
                    GetCursorPos(topRight);
                    Sleep(100);

                    leftBound   = bottomLeft->x + 1;
                    rightBound  = topRight->x  - 1;
                    topBound    = topRight->y  + 1;
                    bottomBound = bottomLeft->y - 1;

                    ComputeDerivedFromBounds();
                    ResetPos();

                    // ★ここで自動保存
                    SaveCurrentClip();
                    UpdateSavedClipLabel();

                    return 1;
                }
            }
        } else {
            clickDown = 0;
        }
        Sleep(1);
    }
    return 0;
}

//======================== メインループ ========================//

int Run() {
    RAWINPUTDEVICE rid[2] = {
        {0x01, 0x06, RIDEV_INPUTSINK, GetActiveWindow()},
        {0x01, 0x02, RIDEV_INPUTSINK, GetActiveWindow()}
    };
    if (!RegisterRawInputDevices(rid, 2, sizeof(RAWINPUTDEVICE))) {
        return 1;
    }

    Input* stateCheckers = InitializeStateCheckers();
    Input* inputList     = FileHandle();

    if (!(gUseSavedClipAtStart && TryApplySavedClip())) {
        POINT bottomLeft{}, topRight{};
        InitializePoints(&bottomLeft, &topRight);
    }

    SwapMouseButton(TRUE);

    while (on) {
        MSG msg;
        while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_INPUT) {
                ProcessRawInput(msg.lParam);
            }
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }

        InputCheck(stateCheckers, 2);

        if (!paused) {
            POINT cursorPos;
            GetCursorPos(&cursorPos);
            if (autoMouseDrag) {
              const LONG rightEdge  = playSpace.right  - 1;
              const LONG bottomEdge = playSpace.bottom - 1;

              if (cursorPos.x >= rightEdge  || cursorPos.x <= playSpace.left ||
                  cursorPos.y >= bottomEdge || cursorPos.y <= playSpace.top) {
                  ResetPos();
              }
            }
            InputCheck(inputList, totalInputs);
        }

        #ifdef YieldProcessor
          YieldProcessor();
        #else
          Sleep(0);
        #endif
    }

    SwapMouseButton(FALSE);
    SendMouseButton(MOUSEEVENTF_RIGHTUP | MOUSEEVENTF_LEFTUP);
    return 0;
}

//======================== Win32 UI ========================//

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_COMMAND: {
            const WORD id = LOWORD(wParam);
            if (id == START_BUTTON_ID) {
                gUseSavedClipAtStart = (SendMessageW(gHCheckUseSaved, BM_GETCHECK, 0, 0) == BST_CHECKED);

                Sleep(2500);
                PaintBackground(hwnd, RGB(255, 0, 0));
                on = 1; paused = 0;
                Run();
                PaintBackground(hwnd, RGB(40, 49, 117));
                UpdateSavedClipLabel();
                return 0;
            }
            break;
        }

        case WM_INPUT:
            ProcessRawInput(lParam);
            return 0;

        case WM_SIZE:
            GetWindowRect(hwnd, &windowRect);
            SetWindowPos(hwnd, nullptr, windowRect.left, windowRect.top, WINDOW_WIDTH, WINDOW_HEIGHT, SWP_SHOWWINDOW);
            return 0;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
    const wchar_t CLASS_NAME[] = L"Sample Window Class";

    WNDCLASSW wc{};
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    wc.lpfnWndProc   = WindowProc;
    wc.hInstance     = hInstance;
    wc.lpszClassName = CLASS_NAME;
    RegisterClassW(&wc);

    HWND hwnd = CreateWindowExW(
        0,
        CLASS_NAME,
        L"DS Mouse Input Fix",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, WINDOW_WIDTH, WINDOW_HEIGHT,
        nullptr, nullptr, hInstance, nullptr
    );
    gHwndMain = hwnd;

    // ロケール判定（日本語/英語）
    wchar_t *locale = _wsetlocale(LC_ALL, L"");
    gIsJapanese = (locale && wcsstr(locale, L"Japanese") != nullptr);

    PaintBackground(hwnd, RGB(40, 49, 117));

    HFONT hUIFont = CreateUIFont(gIsJapanese);

    HWND hStart = CreateWindowW(
        L"BUTTON", L"Start",
        WS_TABSTOP|WS_VISIBLE|WS_CHILD|BS_DEFPUSHBUTTON,
        10, 10, 365, 30,
        hwnd, reinterpret_cast<HMENU>(START_BUTTON_ID), nullptr, nullptr
    );

    HWND hDropdown = CreateWindowW(
        L"COMBOBOX", L"",
        CBS_DROPDOWNLIST|CBS_HASSTRINGS|WS_CHILD|WS_OVERLAPPED|WS_VISIBLE,
        10, 50, 365, 500, hwnd, reinterpret_cast<HMENU>(DROPDOWN_ID), hInstance, nullptr
    );

    SendMessageW(hDropdown, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Metroid Prime Hunters"));
    SendMessageW(hDropdown, CB_SETCURSEL, 0, 0);

    wchar_t message[1024];
    if (gIsJapanese) {
        wcscpy_s(message, L"「Start」で開始。2.5秒後に画面が赤くなる。\n"
                          L"そのタイミングでDS下画面の左下をクリックして、次に右上をクリックする。これでカーソルの範囲を下画面に制限できる。\n"
                          L"バックスペースキー（エンターキーの上）で制限を終了。\n"
                          L"右SHIFTキーで一時停止／再開できる。");
    } else {
        wcscpy_s(message, L"Press 'Start' to begin.\n"
                          L"After 2.5 seconds this window turns red (script active).\n"
                          L"Click bottom-left, then top-right of your stylus area to clip the cursor.\n"
                          L"BACKSPACE: kill, RIGHT SHIFT: pause/resume.");
    }

    HWND hStatic = CreateWindowW(
        L"STATIC", message, WS_CHILD|WS_VISIBLE,
        10, 80, 365, 140, hwnd, reinterpret_cast<HMENU>(3), hInstance, nullptr
    );

    // 残すUIはチェック＋保存済みラベルのみ
    gHCheckUseSaved = CreateWindowW(
        L"BUTTON",
        gIsJapanese ? L"保存済み範囲を使用（次回クリップ省略）" : L"Use saved clip (skip re-clip next time)",
        WS_TABSTOP|WS_VISIBLE|WS_CHILD|BS_AUTOCHECKBOX,
        10, 230, 365, 20, hwnd, reinterpret_cast<HMENU>(CHECK_USE_SAVED_ID), nullptr, nullptr
    );
    gHLabelSaved = CreateWindowW(
        L"STATIC", gIsJapanese ? L"保存済み範囲: なし" : L"Saved clip: none",
        WS_CHILD|WS_VISIBLE,
        10, 255, 365, 20, hwnd, reinterpret_cast<HMENU>(STATIC_SAVED_LABEL_ID), nullptr, nullptr
    );

    ApplyUIFont(hUIFont, { hwnd, hStart, hDropdown, hStatic, gHCheckUseSaved, gHLabelSaved });

    UpdateSavedClipLabel();

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    if (HWND cw = GetConsoleWindow()) ShowWindow(cw, SW_HIDE);

    MSG msg{};
    while (GetMessageW(&msg, nullptr, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    if (hUIFont) DeleteObject(hUIFont);
    return 0;
}
