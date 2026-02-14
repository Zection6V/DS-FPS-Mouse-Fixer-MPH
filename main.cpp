// main.cpp

/*
目的: DS用マウス入力補正ツールの単一翻訳単位化。
UI作成、設定読込、入力監視、各種ゲーム操作マクロを1ファイルに集約。
設定ファイルは DS_Config.toml 1つに統合。
BoostBall の連続発動、距離(%)個別設定、および中心位置オフセット(%)機能を追加。
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
#include <algorithm>

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
#define WINDOW_HEIGHT 320

int   leftBound = 0, rightBound = 0, topBound = 0, bottomBound = 0;
POINT center;
RECT  playSpace;

int mouseUp = MOUSEEVENTF_RIGHTUP, mouseDown = MOUSEEVENTF_RIGHTDOWN, activeMouse = 0x02;

float screenWidthRatio, screenHeightRatio, referenceWidth = 587.0f, referenceHeight = 441.0f;

float distanceFromBoarderSides = 50.0f, distanceFromBoarderTop = 100.0f, distanceNextButton = 100.0f;

int mouseResetWait = 33, buttonWait = 33, swapWait = 220, keyWait = 30;
int autoMouseDrag  = 1;
int boostInterval  = 250; // ブーストボールの連続発動インターバル(ミリ秒)

// ブーストスワイプの設定（すべてパーセンテージ）
int boostSwipePercentVertical   = 50; // 縦幅基準のスワイプ量
int boostSwipePercentHorizontal = 60; // 横幅基準のスワイプ量
int boostSwipePercentDiagonal   = 50; // 縦横それぞれに適用するスワイプ量
int boostOffsetXPercent         = 0;  // 中心からのXズレ調整(画面横幅に対する%)
int boostOffsetYPercent         = 5;  // 中心からのYズレ調整(画面縦幅に対する%)

std::atomic<bool> running(true);
std::atomic<bool> leftButtonDown(false);

bool gUseSavedClipAtStart = false;
bool gIsJapanese = false;

HWND gHwndMain = nullptr;
HWND gHCheckUseSaved = nullptr;
HWND gHLabelSaved = nullptr;

// 設定ファイルのパス
static const char* kConfigFile = "DS_Config.toml";

// ユーザーのキーバインドを保持
std::unordered_map<std::string, int> g_Keybinds;

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
void BoostBall_MPH();
void DoNothing();

void Pause();
void Kill();

//======================== 構造体定義 ========================//

// 連続入力状態を保持できるように構造体を拡張
struct Input {
    int   isDown;
    void (*gameFunction)();
    int   inpNum;
    bool  isContinuous;
    std::chrono::steady_clock::time_point lastTime;
};

//======================== キーバインドマッピング定義 ========================//

struct InputMappingDef {
    const char* keyName;
    void (*gameFunction)();
    int defaultVk;
    bool isContinuous;
};

// Boost Ball を左SHIFT(0xA0) に変更し、isContinuous フラグを true に設定
const InputMappingDef g_Mappings[] = {
    {"Shoot",                DoNothing,         0x01, false},
    {"Mouse Manual Reset",   ResetPos,          0x04, false},
    {"Swap To Main Weapon",  AmmoTypeOne_MPH,   0x06, false},
    {"Swap To Missiles",     AmmoTypeTwo_MPH,   0x05, false},
    {"Swap To Third Weapon", AmmoTypeThree_MPH, 0x52, false},
    {"Zoom",                 Zoom_MPH,          0x02, false},
    {"Special Weapon 1",     WeaponOne_MPH,     0x31, false},
    {"Special Weapon 2",     WeaponTwo_MPH,     0x32, false},
    {"Special Weapon 3",     WeaponThree_MPH,   0x33, false},
    {"Special Weapon 4",     WeaponFour_MPH,    0x34, false},
    {"Special Weapon 5",     WeaponFive_MPH,    0x35, false},
    {"Special Weapon 6",     WeaponSix_MPH,     0x36, false},
    {"Scan Vision",          ScanVision_MPH,    0x56, false},
    {"Ball",                 Ball_MPH,          0xA2, false},
    {"Screen Tap Jump",      DoNothing,         0xA3, false},
    {"Ok (menu)",            ClickOK_MPH,       0x46, false},
    {"Yes (menu)",           ClickYes_MPH,      0x05, false},
    {"No (menu)",            ClickNo_MPH,       0x06, false},
    {"Left (menu)",          Left_MPH,          0x5A, false},
    {"Right (menu)",         Right_MPH,         0x58, false},
    {"Boost Ball",           BoostBall_MPH,     0xA0, true} // 0xA0 = VK_LSHIFT
};
constexpr int NUM_MAPPINGS = sizeof(g_Mappings) / sizeof(g_Mappings[0]);
int totalInputs = NUM_MAPPINGS;

//======================== TOMLパーサー＆セーブ機能 ========================//

std::string Trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (std::string::npos == first) return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}

std::string RemoveQuotes(const std::string& str) {
    if (str.length() >= 2 && str.front() == '"' && str.back() == '"') {
        return str.substr(1, str.length() - 2);
    }
    return str;
}

int ParseInt(const std::string& str) {
    try {
        if (str.find("0x") == 0 || str.find("0X") == 0) {
            return std::stoi(str, nullptr, 16);
        }
        return std::stoi(str);
    } catch (...) {
        return 0;
    }
}

bool SaveTomlConfig() {
    std::ofstream out(kConfigFile);
    if (!out) return false;

    out << "[ClipRegion]\n"
        << "left = " << leftBound << "\n"
        << "top = " << topBound << "\n"
        << "right = " << rightBound << "\n"
        << "bottom = " << bottomBound << "\n\n";

    out << "[Config]\n"
        << "mouseResetWait = " << mouseResetWait << "\n"
        << "buttonWait = " << buttonWait << "\n"
        << "swapWait = " << swapWait << "\n"
        << "keyWait = " << keyWait << "\n"
        << "autoMouseDrag = " << autoMouseDrag << "\n"
        << "boostInterval = " << boostInterval << "\n"
        << "boostSwipePercentVertical = " << boostSwipePercentVertical << "\n"
        << "boostSwipePercentHorizontal = " << boostSwipePercentHorizontal << "\n"
        << "boostSwipePercentDiagonal = " << boostSwipePercentDiagonal << "\n"
        << "boostOffsetXPercent = " << boostOffsetXPercent << "\n"
        << "boostOffsetYPercent = " << boostOffsetYPercent << "\n\n";

    out << "[Keybinds]\n";
    for (int i = 0; i < NUM_MAPPINGS; i++) {
        const auto& m = g_Mappings[i];
        int vk = m.defaultVk;
        if (g_Keybinds.count(m.keyName)) {
            vk = g_Keybinds[m.keyName];
        }
        char hexStr[16];
        snprintf(hexStr, sizeof(hexStr), "0x%02X", vk);
        out << "\"" << m.keyName << "\" = " << hexStr << "\n";
    }

    return true;
}

void LoadTomlConfig() {
    std::ifstream in(kConfigFile);
    if (!in) {
        // ファイルが無ければデフォルト値を保存して作成
        SaveTomlConfig();
        return;
    }

    std::string line;
    std::string section;
    while (std::getline(in, line)) {
        line = Trim(line);
        if (line.empty() || line[0] == '#' || line.find("//") == 0) continue;

        if (line.front() == '[' && line.back() == ']') {
            section = line.substr(1, line.length() - 2);
            continue;
        }

        size_t eqPos = line.find('=');
        if (eqPos != std::string::npos) {
            std::string key = Trim(line.substr(0, eqPos));
            key = RemoveQuotes(key);
            std::string val = Trim(line.substr(eqPos + 1));

            if (section == "ClipRegion") {
                if (key == "left") leftBound = ParseInt(val);
                else if (key == "top") topBound = ParseInt(val);
                else if (key == "right") rightBound = ParseInt(val);
                else if (key == "bottom") bottomBound = ParseInt(val);
            } else if (section == "Config") {
                if (key == "mouseResetWait") mouseResetWait = ParseInt(val);
                else if (key == "buttonWait") buttonWait = ParseInt(val);
                else if (key == "swapWait") swapWait = ParseInt(val);
                else if (key == "keyWait") keyWait = ParseInt(val);
                else if (key == "autoMouseDrag") autoMouseDrag = ParseInt(val);
                else if (key == "boostInterval") boostInterval = ParseInt(val);
                else if (key == "boostSwipePercentVertical") boostSwipePercentVertical = ParseInt(val);
                else if (key == "boostSwipePercentHorizontal") boostSwipePercentHorizontal = ParseInt(val);
                else if (key == "boostSwipePercentDiagonal") boostSwipePercentDiagonal = ParseInt(val);
                else if (key == "boostOffsetXPercent") boostOffsetXPercent = ParseInt(val);
                else if (key == "boostOffsetYPercent") boostOffsetYPercent = ParseInt(val);
            } else if (section == "Keybinds") {
                g_Keybinds[key] = ParseInt(val);
            }
        }
    }
}

Input* CreateInputList() {
    Input* list = new Input[NUM_MAPPINGS];
    for (int i = 0; i < NUM_MAPPINGS; i++) {
        const auto& m = g_Mappings[i];
        int vk = m.defaultVk;
        if (g_Keybinds.count(m.keyName)) {
            vk = g_Keybinds[m.keyName];
        }
        list[i].inpNum = vk;
        list[i].gameFunction = m.gameFunction ? m.gameFunction : DoNothing;
        list[i].isDown = 0;
        list[i].isContinuous = m.isContinuous;
        list[i].lastTime = std::chrono::steady_clock::now();
    }
    return list;
}

//======================== 送出ユーティリティ ========================//

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

void Zoom_MPH() { std::thread(Zoom_MPH_impl).detach(); }

void AmmoTypeOne_MPH() {
    CursorUp();
    SetCursorPos(center.x - LONG(distanceFromBoarderSides * 2.f), topBound + LONG(distanceFromBoarderTop / 2.f));
    RightDown(); ResetPosAfterButton();
}

void AmmoTypeTwo_MPH() {
    CursorUp();
    SetCursorPos(center.x, topBound + LONG(distanceFromBoarderTop / 2.f));
    RightDown(); ResetPosAfterButton();
}

void AmmoTypeThree_MPH() {
    CursorUp();
    SetCursorPos(center.x + LONG(distanceFromBoarderSides * 2.f), topBound + LONG(distanceFromBoarderTop / 2.f));
    RightDown(); ResetPosAfterButton();
}

void ScanVision_MPH() {
    CursorUp();
    SetCursorPos(center.x, bottomBound - LONG(distanceFromBoarderTop / 2.f));
    RightDown(); Sleep(500); ResetPosAfterButton();
}

void Ball_MPH() {
    CursorUp();
    SetCursorPos(rightBound - LONG(distanceFromBoarderSides * 1.5f), bottomBound - LONG(distanceFromBoarderTop / 2.f));
    RightDown(); ResetPosAfterButton();
}

void BoostBall_MPH() {
    CursorUp();
    int dx = 0, dy = 0;
    if (GetAsyncKeyState('W') & 0x8000) dy -= 1;
    if (GetAsyncKeyState('S') & 0x8000) dy += 1;
    if (GetAsyncKeyState('A') & 0x8000) dx -= 1;
    if (GetAsyncKeyState('D') & 0x8000) dx += 1;

    // 入力がない場合はデフォルトで前(上)方向
    if (dx == 0 && dy == 0) dy = -1;

    int screenW = rightBound - leftBound;
    int screenH = bottomBound - topBound;
    
    int swipePixelsX = 0;
    int swipePixelsY = 0;

    // X軸(横)とY軸(縦)それぞれのスワイプピクセル幅を計算
    if (dx != 0 && dy != 0) {
        swipePixelsX = (screenW * boostSwipePercentDiagonal) / 100;
        swipePixelsY = (screenH * boostSwipePercentDiagonal) / 100;
    } else if (dx != 0) {
        swipePixelsX = (screenW * boostSwipePercentHorizontal) / 100;
    } else if (dy != 0) {
        swipePixelsY = (screenH * boostSwipePercentVertical) / 100;
    }

    int halfSwipeX = swipePixelsX / 2;
    int halfSwipeY = swipePixelsY / 2;

    // パーセンテージからピクセルに変換してオフセットを加味した中心座標の計算
    int offsetXPixels = (screenW * boostOffsetXPercent) / 100;
    int offsetYPixels = (screenH * boostOffsetYPercent) / 100;
    int cx = center.x + offsetXPixels;
    int cy = center.y + offsetYPixels;

    int startX = cx - halfSwipeX * dx;
    int startY = cy - halfSwipeY * dy;
    int endX   = cx + halfSwipeX * dx;
    int endY   = cy + halfSwipeY * dy;

    // クリップ領域をはみ出さないように補正
    startX = std::max(leftBound, std::min(startX, rightBound));
    startY = std::max(topBound, std::min(startY, bottomBound));
    endX   = std::max(leftBound, std::min(endX, rightBound));
    endY   = std::max(topBound, std::min(endY, bottomBound));

    SetCursorPos(startX, startY); Sleep(mouseResetWait); RightDown(); Sleep(mouseResetWait);
    SetCursorPos(endX, endY);     Sleep(mouseResetWait);
    ResetPosAfterButton();
}

void WeaponOne_MPH() {
    CursorUp();
    SetCursorPos(rightBound - LONG(distanceFromBoarderSides * 2.f), topBound + LONG(distanceFromBoarderTop));
    RightDown(); GetActiveMouseDown(&mouseResetWait);
    SetCursorPos(center.x - LONG(distanceFromBoarderSides * 1.5f), topBound + LONG(distanceFromBoarderTop));
    ResetPosAfterButton();
}

void WeaponTwo_MPH() {
    CursorUp();
    SetCursorPos(rightBound - LONG(distanceFromBoarderSides * 2.f), topBound + LONG(distanceFromBoarderTop));
    RightDown(); GetActiveMouseDown(&mouseResetWait);
    SetCursorPos(center.x - LONG(distanceFromBoarderSides * 1.5f), center.y - LONG(distanceFromBoarderTop / 4.f));
    ResetPosAfterButton();
}

void WeaponThree_MPH() {
    CursorUp();
    SetCursorPos(rightBound - LONG(distanceFromBoarderSides * 2.f), topBound + LONG(distanceFromBoarderTop));
    RightDown(); GetActiveMouseDown(&mouseResetWait);
    SetCursorPos(center.x - LONG(distanceFromBoarderSides), center.y + LONG(distanceFromBoarderTop / 1.5f));
    ResetPosAfterButton();
}

void WeaponFour_MPH() {
    CursorUp();
    SetCursorPos(rightBound - LONG(distanceFromBoarderSides * 2.f), topBound + LONG(distanceFromBoarderTop));
    RightDown(); GetActiveMouseDown(&mouseResetWait);
    SetCursorPos(center.x + LONG(distanceFromBoarderSides / 2.f), bottomBound - LONG(distanceFromBoarderTop / 1.2f));
    ResetPosAfterButton();
}

void WeaponFive_MPH() {
    CursorUp();
    SetCursorPos(rightBound - LONG(distanceFromBoarderSides * 2.f), topBound + LONG(distanceFromBoarderTop));
    RightDown(); GetActiveMouseDown(&mouseResetWait);
    SetCursorPos(center.x + LONG(distanceFromBoarderSides * 2.f), bottomBound - LONG(distanceFromBoarderTop / 1.5f));
    ResetPosAfterButton();
}

void WeaponSix_MPH() {
    CursorUp();
    SetCursorPos(rightBound - LONG(distanceFromBoarderSides * 1.8f), bottomBound - LONG(distanceFromBoarderTop / 2.f));
    RightDown(); GetActiveMouseDown(&mouseResetWait);
    ResetPosAfterButton();
}

void ClickOK_MPH() {
    CursorUp(); SetCursorPos(center.x, bottomBound - LONG(distanceFromBoarderTop * 1.2f)); RightDown(); ResetPosAfterButton();
}

void ClickYes_MPH() {
    CursorUp(); SetCursorPos(center.x - LONG(distanceFromBoarderSides * 2.f), bottomBound - LONG(distanceFromBoarderTop * 1.2f)); RightDown(); ResetPosAfterButton();
}

void ClickNo_MPH() {
    CursorUp(); SetCursorPos(center.x + LONG(distanceFromBoarderSides * 2.f), bottomBound - LONG(distanceFromBoarderTop * 1.2f)); RightDown(); ResetPosAfterButton();
}

void Left_MPH() {
    CursorUp(); SetCursorPos(center.x - LONG(distanceFromBoarderSides * 2.5f), bottomBound - LONG(distanceFromBoarderTop * 1.2f)); RightDown(); ResetPosAfterButton();
}

void Right_MPH() {
    CursorUp(); SetCursorPos(center.x + LONG(distanceFromBoarderSides * 2.5f), bottomBound - LONG(distanceFromBoarderTop * 1.2f)); RightDown(); ResetPosAfterButton();
}

void DoNothing() {}

void Pause() {
    RightUp();
    SwapMouseButton(paused);
    SendKey('N', false);
    paused = !paused;
    if (paused) ClipCursor(nullptr);
    else ResetPos();
}

void Kill() {
    on = 0; paused = 1;
    RightUp(); SendKey('N', false);
    SwapMouseButton(0); ClipCursor(nullptr);
}

//======================== 入力監視・状態操作 ========================//

__forceinline void RunInput(struct Input *input) {
    SHORT keyState = GetAsyncKeyState(input->inpNum);
    BOOL  isKeyDown = (keyState & 0x8000) != 0;

    if (input->inpNum == 0x02) { // Zoom用特別処理
        if (!isKeyDown && input->isDown) {
            if (autoMouseDrag) RightDown();
            if (input->gameFunction) input->gameFunction();
        }
    } else {
        if (input->isContinuous) {
            // 押しっぱなしの間、boostIntervalミリ秒ごとに発動
            if (isKeyDown) {
                auto now = std::chrono::steady_clock::now();
                if (!input->isDown || std::chrono::duration_cast<std::chrono::milliseconds>(now - input->lastTime).count() >= boostInterval) {
                    if (input->gameFunction) input->gameFunction();
                    input->lastTime = now;
                }
            }
        } else {
            // 通常の1回発動
            if (isKeyDown && !input->isDown) {
                if (input->gameFunction) input->gameFunction();
            }
        }
    }
    input->isDown = isKeyDown;
}

__forceinline void InputCheck(struct Input* inputs, int length) {
    for (int i = 0; i < length; i++) RunInput(&inputs[i]);
}

Input* InitializeStateCheckers() {
    auto* stateCheckers = new Input[2];
    stateCheckers[0].isDown = false; stateCheckers[0].gameFunction = Pause; stateCheckers[0].inpNum = VK_RSHIFT; stateCheckers[0].isContinuous = false;
    stateCheckers[1].isDown = false; stateCheckers[1].gameFunction = Kill;  stateCheckers[1].inpNum = VK_BACK;   stateCheckers[1].isContinuous = false;
    return stateCheckers;
}

static __forceinline void ProcessKeyInput(UINT vKey, bool isKeyDown) {
    if (vKey == 'C') {
        if (isKeyDown) SendKey('M', true);
        else SendKey('M', false);
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
    if (GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER)) == (UINT)-1) return;
    auto* raw = reinterpret_cast<RAWINPUT*>(lpb);
    if (raw->header.dwType == RIM_TYPEKEYBOARD) {
        ProcessKeyInput(raw->data.keyboard.VKey, !(raw->data.keyboard.Flags & RI_KEY_BREAK));
    } else if (raw->header.dwType == RIM_TYPEMOUSE) {
        ProcessMouseInput(raw);
    }
}

//======================== クリップ保存/適用ヘルパ ========================//

bool IsClipValid() {
    const int maxW = GetSystemMetrics(SM_CXSCREEN);
    const int maxH = GetSystemMetrics(SM_CYSCREEN);
    if (leftBound < 0 || topBound < 0 || rightBound > maxW || bottomBound > maxH) return false;
    if (leftBound >= rightBound || topBound >= bottomBound) return false;
    if (rightBound - leftBound < 10 || bottomBound - topBound < 10) return false;
    return true;
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

void UpdateSavedClipLabel() {
    if (!gHLabelSaved) return;
    std::wstringstream wss;
    if (IsClipValid()) {
        if (gIsJapanese) wss << L"保存済み範囲: (" << leftBound << L"," << topBound << L") - (" << rightBound << L"," << bottomBound << L")";
        else wss << L"Saved clip: (" << leftBound << L"," << topBound << L") - (" << rightBound << L"," << bottomBound << L")";
    } else {
        wss << (gIsJapanese ? L"保存済み範囲: なし" : L"Saved clip: none");
    }
    SetWindowTextW(gHLabelSaved, wss.str().c_str());
    if (gHCheckUseSaved) SendMessageW(gHCheckUseSaved, BM_SETCHECK, IsClipValid() ? BST_CHECKED : BST_UNCHECKED, 0);
}

int InitializePoints(POINT* bottomLeft, POINT* topRight) {
    int initialize = 0, clickDown = 0;
    while (initialize <= 1) {
        if (GetAsyncKeyState(VK_LBUTTON) & 0x8000) {
            if (!clickDown) {
                clickDown = 1;
                if (initialize == 0) {
                    GetCursorPos(bottomLeft); initialize++;
                } else {
                    GetCursorPos(topRight); Sleep(100);
                    leftBound   = bottomLeft->x + 1;
                    rightBound  = topRight->x  - 1;
                    topBound    = topRight->y  + 1;
                    bottomBound = bottomLeft->y - 1;

                    ComputeDerivedFromBounds();
                    ResetPos();
                    SaveTomlConfig(); // 確定時にTOMLへ上書き
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
    if (!RegisterRawInputDevices(rid, 2, sizeof(RAWINPUTDEVICE))) return 1;

    Input* stateCheckers = InitializeStateCheckers();
    Input* inputList     = CreateInputList();

    if (!(gUseSavedClipAtStart && IsClipValid())) {
        POINT bottomLeft{}, topRight{};
        InitializePoints(&bottomLeft, &topRight);
    } else {
        ComputeDerivedFromBounds();
        ResetPos();
    }

    SwapMouseButton(TRUE);

    while (on) {
        MSG msg;
        while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_INPUT) ProcessRawInput(msg.lParam);
            TranslateMessage(&msg); DispatchMessageW(&msg);
        }

        InputCheck(stateCheckers, 2);

        if (!paused) {
            POINT cursorPos; GetCursorPos(&cursorPos);
            if (autoMouseDrag) {
              const LONG rightEdge  = playSpace.right  - 1;
              const LONG bottomEdge = playSpace.bottom - 1;
              if (cursorPos.x >= rightEdge || cursorPos.x <= playSpace.left ||
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
    delete[] stateCheckers;
    delete[] inputList;
    return 0;
}

//======================== Win32 UI ========================//

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
    for (HWND h : ctrls) if (h) SendMessageW(h, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), TRUE);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_COMMAND: {
            if (LOWORD(wParam) == START_BUTTON_ID) {
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
            ProcessRawInput(lParam); return 0;
        case WM_SIZE:
            GetWindowRect(hwnd, &windowRect);
            SetWindowPos(hwnd, nullptr, windowRect.left, windowRect.top, WINDOW_WIDTH, WINDOW_HEIGHT, SWP_SHOWWINDOW);
            return 0;
        case WM_DESTROY:
            PostQuitMessage(0); return 0;
    }
    return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
    const wchar_t CLASS_NAME[] = L"Sample Window Class";
    SwapMouseButton(FALSE);
    std::atexit([](){ SwapMouseButton(FALSE); });

    WNDCLASSW wc{};
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    wc.lpfnWndProc   = WindowProc;
    wc.hInstance     = hInstance;
    wc.lpszClassName = CLASS_NAME;
    RegisterClassW(&wc);

    HWND hwnd = CreateWindowExW(
        0, CLASS_NAME, L"DS Mouse Input Fix", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, WINDOW_WIDTH, WINDOW_HEIGHT,
        nullptr, nullptr, hInstance, nullptr
    );
    gHwndMain = hwnd;

    wchar_t *locale = _wsetlocale(LC_ALL, L"");
    gIsJapanese = (locale && wcsstr(locale, L"Japanese") != nullptr);

    // ★アプリ起動時にTOMLをロード (無ければ作成)
    LoadTomlConfig();

    PaintBackground(hwnd, RGB(40, 49, 117));
    HFONT hUIFont = CreateUIFont(gIsJapanese);

    HWND hStart = CreateWindowW(L"BUTTON", L"Start", WS_TABSTOP|WS_VISIBLE|WS_CHILD|BS_DEFPUSHBUTTON,
                                10, 10, 365, 30, hwnd, reinterpret_cast<HMENU>(START_BUTTON_ID), nullptr, nullptr);
    HWND hDropdown = CreateWindowW(L"COMBOBOX", L"", CBS_DROPDOWNLIST|CBS_HASSTRINGS|WS_CHILD|WS_OVERLAPPED|WS_VISIBLE,
                                   10, 50, 365, 500, hwnd, reinterpret_cast<HMENU>(DROPDOWN_ID), hInstance, nullptr);
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
    HWND hStatic = CreateWindowW(L"STATIC", message, WS_CHILD|WS_VISIBLE,
                                 10, 80, 365, 140, hwnd, reinterpret_cast<HMENU>(3), hInstance, nullptr);

    gHCheckUseSaved = CreateWindowW(L"BUTTON", gIsJapanese ? L"保存済み範囲を使用（次回クリップ省略）" : L"Use saved clip (skip re-clip next time)",
                                    WS_TABSTOP|WS_VISIBLE|WS_CHILD|BS_AUTOCHECKBOX,
                                    10, 230, 365, 20, hwnd, reinterpret_cast<HMENU>(CHECK_USE_SAVED_ID), nullptr, nullptr);
    gHLabelSaved = CreateWindowW(L"STATIC", gIsJapanese ? L"保存済み範囲: なし" : L"Saved clip: none",
                                 WS_CHILD|WS_VISIBLE,
                                 10, 255, 365, 20, hwnd, reinterpret_cast<HMENU>(STATIC_SAVED_LABEL_ID), nullptr, nullptr);

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