// main.cpp

/*
目的: DS用マウス入力補正ツールの単一翻訳単位化・最適化版。
【真・完全決着版】
1. RawInputのメッセージポンプによる一瞬の入力（ファストクリック）消失を防ぐため、
   g_PhysicalKeysHit（トランジェント・キャプチャ）を導入。これによりZoom等の単発入力の「もたつき」を完全解消。
2. マクロソフトウェア等から送信される MakeCode == 0 の仮想Shift入力を VK_LSHIFT にフォールバックし、
   Boost (Trigger) が反応しないバグを解決。
3. 押しっぱなし（スタック）を数学的に排除するステート・ドリブン方式を継続。
*/

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

int on = 1, paused = 0, gameSelected = 7;
constexpr int START_BUTTON_ID = 1, DROPDOWN_ID = 3, CHECK_USE_SAVED_ID = 10, STATIC_SAVED_LABEL_ID = 14;
#define WINDOW_WIDTH  400
#define WINDOW_HEIGHT 320

int leftBound = 0, rightBound = 0, topBound = 0, bottomBound = 0;
POINT center; RECT playSpace, windowRect;
int mouseUp = MOUSEEVENTF_RIGHTUP, mouseDown = MOUSEEVENTF_RIGHTDOWN, activeMouse = 0x02;

float screenWidthRatio, screenHeightRatio, referenceWidth = 587.0f, referenceHeight = 441.0f;
float distanceFromBoarderSides = 50.0f, distanceFromBoarderTop = 100.0f, distanceNextButton = 100.0f;

int mouseResetWait = 33, buttonWait = 33, swapWait = 220, keyWait = 30; // Zoomパルス持続時間(30ms)
int autoMouseDrag = 1, boostInterval = 250; 
int boostSwipePercentVertical = 50, boostSwipePercentHorizontal = 60, boostSwipePercentDiagonal = 45;
int boostOffsetXPercent = 0, boostOffsetYPercent = 5;

// Triggerループの設定(ミリ秒)
int boostTriggerDelayMs = 250;    
int boostTriggerHoldMs = 500;     
int boostTriggerIntervalMs = 250; 

std::atomic<bool> running(true), leftButtonDown(false);

// ★ Zoomパルスの時間管理
std::chrono::steady_clock::time_point g_zoomPulseEndTime;
bool g_zoomPulseActive = false;

bool gUseSavedClipAtStart = false, gIsJapanese = false;
HWND gHwndMain = nullptr, gHCheckUseSaved = nullptr, gHLabelSaved = nullptr;

static const char* kConfigFile = "DS_Config.toml";
std::unordered_map<std::string, int> g_Keybinds;

// 物理キーボード/マウスの状態を完全にトラッキングする配列
std::atomic<bool> g_PhysicalKeys[256];
// ★ メッセージポンプでの一瞬の入力(ファストクリック)消失を防ぐキャプチャ配列
std::atomic<bool> g_PhysicalKeysHit[256];

//======================== ユーティリティ(送出・マウス状態) ========================//

// Mキー専用の安全な状態同期関数（スタック完全防止）
std::atomic<bool> isMKeyPressed(false);
static void SafeSendMKey(bool down) {
    if (isMKeyPressed.exchange(down) == down) return; 
    
    INPUT in{}; 
    in.type = INPUT_KEYBOARD;
    in.ki.wScan = (WORD)MapVirtualKeyW('M', MAPVK_VK_TO_VSC);
    in.ki.dwFlags = KEYEVENTF_SCANCODE | (down ? 0 : KEYEVENTF_KEYUP);
    SendInput(1, &in, sizeof(in));
}

// 汎用キー送信
static __forceinline void SendKey(WORD vk, bool down) {
    if (vk == 'M' || vk == 'm') { SafeSendMKey(down); return; }
    
    INPUT in{}; 
    in.type = INPUT_KEYBOARD;
    in.ki.wScan = (WORD)MapVirtualKeyW(vk, MAPVK_VK_TO_VSC);
    in.ki.dwFlags = KEYEVENTF_SCANCODE | (down ? 0 : KEYEVENTF_KEYUP);
    SendInput(1, &in, sizeof(in));
}

// 左クリック（射撃: Nキー）専用送信
static void SendShootKey(bool down) {
    SendKey('N', down);
}

static __forceinline void SendMouseButton(WORD flags) {
    INPUT in{INPUT_MOUSE}; in.mi.dwFlags = flags; SendInput(1, &in, sizeof(in));
}
static __forceinline void RightDown() { SendMouseButton(MOUSEEVENTF_RIGHTDOWN); }
static __forceinline void RightUp()   { SendMouseButton(MOUSEEVENTF_RIGHTUP);   }

__forceinline void GetActiveMouseDown(int waitTime) { Sleep(waitTime); }
__forceinline void GetActiveMouseUp(int waitTime)   { Sleep(waitTime); }

__forceinline void ResetPosAfterButton() {
    GetActiveMouseDown(buttonWait); RightUp(); GetActiveMouseUp(buttonWait);
    SetCursorPos(center.x, center.y); if (autoMouseDrag) RightDown();
}

static void lockPlaySpaceAfterResetPos() {
    RECT t; GetClipCursor(&t);
    if (t.left != playSpace.left || t.right != playSpace.right || t.top != playSpace.top || t.bottom != playSpace.bottom)
        ClipCursor(&playSpace);
}

__forceinline void ResetPos() {
    RightUp(); GetActiveMouseUp(mouseResetWait); SetCursorPos(center.x, center.y);
    if (autoMouseDrag) RightDown();
    std::thread(lockPlaySpaceAfterResetPos).detach();
}

__forceinline void CursorUp() { RightUp(); GetActiveMouseUp(mouseResetWait); }

//======================== 共通マクロアクション ========================//

void DoNothing() {}

void Pause() { 
    RightUp(); SwapMouseButton(paused); SendShootKey(false); 
    SafeSendMKey(false); g_zoomPulseActive = false; 
    paused = !paused; 
    if (paused) ClipCursor(nullptr); else ResetPos(); 
}

void Kill() { 
    on = 0; paused = 1; RightUp(); SendShootKey(false); 
    SafeSendMKey(false); g_zoomPulseActive = false; 
    SwapMouseButton(0); ClipCursor(nullptr); 
}

void Tap(int x, int y, int delayMs = 0) {
    CursorUp(); SetCursorPos(x, y); RightDown(); if (delayMs) Sleep(delayMs); ResetPosAfterButton();
}
void Drag(int sx, int sy, int ex, int ey) {
    CursorUp(); SetCursorPos(sx, sy); RightDown(); GetActiveMouseDown(mouseResetWait); SetCursorPos(ex, ey); ResetPosAfterButton();
}
void HoldTap(int x, int y) {
    CursorUp(); SetCursorPos(x, y); RightDown(); GetActiveMouseDown(mouseResetWait); ResetPosAfterButton();
}

// Swipeの非同期スレッド処理
std::atomic<bool> isSwiping(false);
void BoostBall_MPH() {
    if (isSwiping.exchange(true)) return; 
    std::thread([]() {
        CursorUp();
        int dx = g_PhysicalKeys['D'] - g_PhysicalKeys['A'];
        int dy = g_PhysicalKeys['S'] - g_PhysicalKeys['W'];
        if (!dx && !dy) dy = -1;

        int scrW = rightBound - leftBound, scrH = bottomBound - topBound;
        int px = dx ? (dy ? boostSwipePercentDiagonal : boostSwipePercentHorizontal) : 0;
        int py = dy ? (dx ? boostSwipePercentDiagonal : boostSwipePercentVertical) : 0;

        int hx = (scrW * px / 200) * dx, hy = (scrH * py / 200) * dy;
        int cx = center.x + (scrW * boostOffsetXPercent / 100), cy = center.y + (scrH * boostOffsetYPercent / 100);

        int stX = std::clamp(cx - hx, leftBound, rightBound), stY = std::clamp(cy - hy, topBound, bottomBound);
        int enX = std::clamp(cx + hx, leftBound, rightBound), enY = std::clamp(cy + hy, topBound, bottomBound);

        SetCursorPos(stX, stY); Sleep(mouseResetWait); RightDown(); Sleep(mouseResetWait);
        SetCursorPos(enX, enY); Sleep(mouseResetWait); ResetPosAfterButton();
        
        isSwiping = false; 
    }).detach();
}

//======================== キーバインドマッピング ========================//

#define X center.x
#define Y center.y
#define DX distanceFromBoarderSides
#define DY distanceFromBoarderTop
#define L leftBound
#define R rightBound
#define T topBound
#define B bottomBound

enum class MacroType { Single, Continuous, TriggerLoop, ZoomPulse };

struct InputMappingDef { const char* keyName; void (*gameFunction)(); int defaultVk; MacroType type; };

const InputMappingDef g_Mappings[] = {
    {"Shoot",                DoNothing, 0x01, MacroType::Single},
    {"Mouse Manual Reset",   ResetPos,  0x04, MacroType::Single},
    {"Swap To Main Weapon",  [](){ Tap(X - DX*2, T + DY/2); }, 0x06, MacroType::Single},
    {"Swap To Missiles",     [](){ Tap(X, T + DY/2); }, 0x05, MacroType::Single},
    {"Swap To Third Weapon", [](){ Tap(X + DX*2, T + DY/2); }, 0x52, MacroType::Single},
    // 右クリックとCキーを「ZoomPulse」に統合
    {"Zoom",                 DoNothing, 0x02, MacroType::ZoomPulse},
    {"Zoom (Keyboard)",      DoNothing, 0x43, MacroType::ZoomPulse},
    {"Special Weapon 1",     [](){ Drag(R - DX*2, T + DY, X - DX*1.5, T + DY); }, 0x31, MacroType::Single},
    {"Special Weapon 2",     [](){ Drag(R - DX*2, T + DY, X - DX*1.5, Y - DY/4); }, 0x32, MacroType::Single},
    {"Special Weapon 3",     [](){ Drag(R - DX*2, T + DY, X - DX, Y + DY/1.5); }, 0x33, MacroType::Single},
    {"Special Weapon 4",     [](){ Drag(R - DX*2, T + DY, X + DX/2, B - DY/1.2); }, 0x34, MacroType::Single},
    {"Special Weapon 5",     [](){ Drag(R - DX*2, T + DY, X + DX*2, B - DY/1.5); }, 0x35, MacroType::Single},
    {"Special Weapon 6",     [](){ HoldTap(R - DX*1.8, B - DY/2); }, 0x36, MacroType::Single},
    {"Scan Vision",          [](){ Tap(X, B - DY/2, 500); }, 0x56, MacroType::Single},
    {"Ball",                 [](){ Tap(R - DX*1.5, B - DY/2); }, 0xA2, MacroType::Single},
    {"Screen Tap Jump",      DoNothing, 0xA3, MacroType::Single},
    {"Ok (menu)",            [](){ Tap(X, B - DY*1.2); }, 0x46, MacroType::Single},
    {"Yes (menu)",           [](){ Tap(X - DX*2, B - DY*1.2); }, 0x05, MacroType::Single},
    {"No (menu)",            [](){ Tap(X + DX*2, B - DY*1.2); }, 0x06, MacroType::Single},
    {"Left (menu)",          [](){ Tap(X - DX*2.5, B - DY*1.2); }, 0x5A, MacroType::Single},
    {"Right (menu)",         [](){ Tap(X + DX*2.5, B - DY*1.2); }, 0x58, MacroType::Single},
    {"Boost Ball (Swipe)",   BoostBall_MPH, 0xA0, MacroType::Continuous},
    {"Boost Ball (Trigger)", DoNothing,     0xA0, MacroType::TriggerLoop}
};
constexpr int NUM_MAPPINGS = sizeof(g_Mappings) / sizeof(g_Mappings[0]);

struct Input { 
    int isDown; 
    void (*gameFunction)(); 
    int inpNum; 
    MacroType type; 
    std::chrono::steady_clock::time_point lastTime; 
    int state; 
    std::chrono::steady_clock::time_point releaseTime; 
    int physicalWasDown;
};

//======================== TOMLパーサー＆セーブ ========================//

std::string Trim(const std::string& str) { size_t f=str.find_first_not_of(" \t\r\n"), l=str.find_last_not_of(" \t\r\n"); return (f==std::string::npos)?"":str.substr(f,l-f+1); }
std::string RemoveQuotes(const std::string& str) { return (str.length()>=2 && str.front()=='"' && str.back()=='"') ? str.substr(1,str.length()-2) : str; }
int ParseInt(const std::string& str) { try { return (str.find("0x")==0 || str.find("0X")==0) ? std::stoi(str,nullptr,16) : std::stoi(str); } catch(...) { return 0; } }

bool SaveTomlConfig() {
    std::ofstream out(kConfigFile); if (!out) return false;
    out << "[ClipRegion]\nleft = "<<leftBound<<"\ntop = "<<topBound<<"\nright = "<<rightBound<<"\nbottom = "<<bottomBound<<"\n\n"
        << "[Config]\nmouseResetWait = "<<mouseResetWait<<"\nbuttonWait = "<<buttonWait<<"\nswapWait = "<<swapWait<<"\nkeyWait = "<<keyWait
        << "\nautoMouseDrag = "<<autoMouseDrag<<"\nboostInterval = "<<boostInterval<<"\nboostSwipePercentVertical = "<<boostSwipePercentVertical
        << "\nboostSwipePercentHorizontal = "<<boostSwipePercentHorizontal<<"\nboostSwipePercentDiagonal = "<<boostSwipePercentDiagonal
        << "\nboostOffsetXPercent = "<<boostOffsetXPercent<<"\nboostOffsetYPercent = "<<boostOffsetYPercent
        << "\nboostTriggerDelayMs = "<<boostTriggerDelayMs<<"\nboostTriggerHoldMs = "<<boostTriggerHoldMs<<"\nboostTriggerIntervalMs = "<<boostTriggerIntervalMs<<"\n\n[Keybinds]\n";
    for (int i = 0; i < NUM_MAPPINGS; i++) {
        char hex[16]; snprintf(hex, sizeof(hex), "0x%02X", g_Keybinds.count(g_Mappings[i].keyName) ? g_Keybinds[g_Mappings[i].keyName] : g_Mappings[i].defaultVk);
        out << "\"" << g_Mappings[i].keyName << "\" = " << hex << "\n";
    }
    return true;
}

void LoadTomlConfig() {
    std::ifstream in(kConfigFile); if (!in) { SaveTomlConfig(); return; }
    std::string line, sec;
    while (std::getline(in, line)) {
        line = Trim(line); if (line.empty() || line[0] == '#' || line.find("//") == 0) continue;
        if (line.front() == '[' && line.back() == ']') { sec = line.substr(1, line.length()-2); continue; }
        size_t eq = line.find('=');
        if (eq != std::string::npos) {
            std::string k = RemoveQuotes(Trim(line.substr(0, eq))), v = Trim(line.substr(eq + 1));
            if (sec == "Keybinds") g_Keybinds[k] = ParseInt(v);
            else if (k == "left") leftBound = ParseInt(v);          else if (k == "top") topBound = ParseInt(v);
            else if (k == "right") rightBound = ParseInt(v);        else if (k == "bottom") bottomBound = ParseInt(v);
            else if (k == "mouseResetWait") mouseResetWait = ParseInt(v); else if (k == "buttonWait") buttonWait = ParseInt(v);
            else if (k == "swapWait") swapWait = ParseInt(v);       else if (k == "keyWait") keyWait = ParseInt(v);
            else if (k == "autoMouseDrag") autoMouseDrag = ParseInt(v); else if (k == "boostInterval") boostInterval = ParseInt(v);
            else if (k == "boostSwipePercentVertical") boostSwipePercentVertical = ParseInt(v);
            else if (k == "boostSwipePercentHorizontal") boostSwipePercentHorizontal = ParseInt(v);
            else if (k == "boostSwipePercentDiagonal") boostSwipePercentDiagonal = ParseInt(v);
            else if (k == "boostOffsetXPercent") boostOffsetXPercent = ParseInt(v);
            else if (k == "boostOffsetYPercent") boostOffsetYPercent = ParseInt(v);
            else if (k == "boostTriggerDelayMs") boostTriggerDelayMs = ParseInt(v);
            else if (k == "boostTriggerHoldMs") boostTriggerHoldMs = ParseInt(v);
            else if (k == "boostTriggerIntervalMs") boostTriggerIntervalMs = ParseInt(v);
        }
    }
}

//======================== 入力処理・アプリロジック ========================//

// Raw Input (ハードウェア直結の監視フック)
void ProcessRawInput(LPARAM lParam) {
    UINT sz = sizeof(RAWINPUT); static BYTE lpb[sizeof(RAWINPUT)];
    if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, lpb, &sz, sizeof(RAWINPUTHEADER)) == (UINT)-1) return;
    auto* raw = (RAWINPUT*)lpb;
    
    // WindowsのSendInputによる「偽イベント（ノイズ）」を弾く
    if (raw->header.hDevice == nullptr) return; 

    if (raw->header.dwType == RIM_TYPEKEYBOARD) {
        const RAWKEYBOARD& kb = raw->data.keyboard;
        UINT vk = kb.VKey;
        if (vk == 0) vk = MapVirtualKeyW(kb.MakeCode, MAPVK_VSC_TO_VK_EX);
        
        if (vk > 0 && vk < 255) {
            // ★ マクロソフトの仮想Shift(MakeCode=0)なども確実に VK_LSHIFT に拾い上げる
            if (vk == VK_SHIFT) {
                vk = (kb.MakeCode == 0x36) ? VK_RSHIFT : VK_LSHIFT;
            } else if (vk == VK_CONTROL) {
                vk = (kb.Flags & RI_KEY_E0) ? VK_RCONTROL : VK_LCONTROL;
            } else if (vk == VK_MENU) {
                vk = (kb.Flags & RI_KEY_E0) ? VK_RMENU : VK_LMENU;
            }
            
            bool down = !(kb.Flags & RI_KEY_BREAK);
            g_PhysicalKeys[vk] = down;
            if (down) g_PhysicalKeysHit[vk] = true; // ★ トランジェント・キャプチャ（一瞬の押下も絶対逃さない）
        }
    } else if (raw->header.dwType == RIM_TYPEMOUSE) {
        const RAWMOUSE& m = raw->data.mouse;
        USHORT flags = m.usButtonFlags;

        if (flags & RI_MOUSE_LEFT_BUTTON_DOWN) { 
            g_PhysicalKeys[VK_LBUTTON] = true;
            g_PhysicalKeysHit[VK_LBUTTON] = true;
            if (!leftButtonDown.exchange(true)) {
                if (!paused) SendShootKey(true); 
            }
        }
        if (flags & RI_MOUSE_LEFT_BUTTON_UP) { 
            g_PhysicalKeys[VK_LBUTTON] = false;
            if (leftButtonDown.exchange(false)) {
                if (!paused) SendShootKey(false); 
            }
        }
        if (flags & RI_MOUSE_RIGHT_BUTTON_DOWN)  { g_PhysicalKeys[VK_RBUTTON] = true; g_PhysicalKeysHit[VK_RBUTTON] = true; }
        if (flags & RI_MOUSE_RIGHT_BUTTON_UP)    g_PhysicalKeys[VK_RBUTTON] = false;
        
        if (flags & RI_MOUSE_MIDDLE_BUTTON_DOWN) { g_PhysicalKeys[VK_MBUTTON] = true; g_PhysicalKeysHit[VK_MBUTTON] = true; }
        if (flags & RI_MOUSE_MIDDLE_BUTTON_UP)   g_PhysicalKeys[VK_MBUTTON] = false;
        
        if (flags & RI_MOUSE_BUTTON_4_DOWN)      { g_PhysicalKeys[VK_XBUTTON1] = true; g_PhysicalKeysHit[VK_XBUTTON1] = true; }
        if (flags & RI_MOUSE_BUTTON_4_UP)        g_PhysicalKeys[VK_XBUTTON1] = false;
        
        if (flags & RI_MOUSE_BUTTON_5_DOWN)      { g_PhysicalKeys[VK_XBUTTON2] = true; g_PhysicalKeysHit[VK_XBUTTON2] = true; }
        if (flags & RI_MOUSE_BUTTON_5_UP)        g_PhysicalKeys[VK_XBUTTON2] = false;
    }
}

bool IsClipValid() {
    return leftBound >= 0 && topBound >= 0 && rightBound > leftBound + 10 && bottomBound > topBound + 10 &&
           rightBound <= GetSystemMetrics(SM_CXSCREEN) && bottomBound <= GetSystemMetrics(SM_CYSCREEN);
}

void ComputeDerivedFromBounds() {
    SetRect(&playSpace, leftBound, topBound, rightBound, bottomBound);
    screenWidthRatio = float(rightBound - leftBound) / referenceWidth;
    screenHeightRatio = float(bottomBound - topBound) / referenceHeight;
    distanceFromBoarderSides = 50.0f * screenWidthRatio;
    distanceFromBoarderTop = 100.0f * screenHeightRatio;
    center = { ((rightBound - leftBound) / 2) + leftBound, ((bottomBound - topBound) / 2) + topBound };
}

void UpdateSavedClipLabel() {
    if (!gHLabelSaved) return;
    std::wstringstream wss;
    if (IsClipValid()) wss << (gIsJapanese ? L"保存済み範囲: (" : L"Saved clip: (") << leftBound << L"," << topBound << L") - (" << rightBound << L"," << bottomBound << L")";
    else wss << (gIsJapanese ? L"保存済み範囲: なし" : L"Saved clip: none");
    SetWindowTextW(gHLabelSaved, wss.str().c_str());
    if (gHCheckUseSaved) SendMessageW(gHCheckUseSaved, BM_SETCHECK, IsClipValid() ? BST_CHECKED : BST_UNCHECKED, 0);
}

int InitializePoints(POINT* bl, POINT* tr) {
    int init = 0; bool click = false;
    while (init <= 1) {
        if (GetAsyncKeyState(VK_LBUTTON) & 0x8000) {
            if (!click) {
                click = true;
                if (init++ == 0) GetCursorPos(bl);
                else {
                    GetCursorPos(tr); Sleep(100);
                    leftBound = bl->x + 1; rightBound = tr->x - 1; topBound = tr->y + 1; bottomBound = bl->y - 1;
                    ComputeDerivedFromBounds(); ResetPos(); SaveTomlConfig(); UpdateSavedClipLabel(); return 1;
                }
            }
        } else click = false;
        Sleep(1);
    }
    return 0;
}

int Run() {
    RAWINPUTDEVICE rid[2] = { {0x01, 0x06, RIDEV_INPUTSINK, gHwndMain}, {0x01, 0x02, RIDEV_INPUTSINK, gHwndMain} };
    if (!RegisterRawInputDevices(rid, 2, sizeof(RAWINPUTDEVICE))) return 1;

    for(int i = 0; i < 256; i++) {
        g_PhysicalKeys[i] = false;
        g_PhysicalKeysHit[i] = false;
    }

    auto nowInit = std::chrono::steady_clock::now();
    auto pastInit = nowInit - std::chrono::milliseconds(1000);

    Input stateCheckers[2] = { 
        {0, Pause, VK_RSHIFT, MacroType::Single, nowInit, 0, pastInit, 0}, 
        {0, Kill, VK_BACK, MacroType::Single, nowInit, 0, pastInit, 0} 
    };
    Input inputList[NUM_MAPPINGS];
    for (int i = 0; i < NUM_MAPPINGS; i++) {
        inputList[i] = {0, g_Mappings[i].gameFunction ? g_Mappings[i].gameFunction : DoNothing,
                        g_Keybinds.count(g_Mappings[i].keyName) ? g_Keybinds[g_Mappings[i].keyName] : g_Mappings[i].defaultVk,
                        g_Mappings[i].type, nowInit, 0, pastInit, 0};
    }

    if (!(gUseSavedClipAtStart && IsClipValid())) { POINT bl{}, tr{}; InitializePoints(&bl, &tr); }
    else { ComputeDerivedFromBounds(); ResetPos(); }

    SwapMouseButton(TRUE);
    g_zoomPulseActive = false;
    
    while (on) {
        MSG msg; while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_INPUT) ProcessRawInput(msg.lParam);
            TranslateMessage(&msg); DispatchMessageW(&msg);
        }
        
        auto now = std::chrono::steady_clock::now();

        // Pause/Kill チェック (Hitキャプチャを消費して確実な反応)
        for (auto& s : stateCheckers) {
            bool physicalDown = g_PhysicalKeys[s.inpNum];
            bool physicalHit = g_PhysicalKeysHit[s.inpNum].exchange(false);
            bool down = physicalDown || physicalHit;
            
            if (down && !s.isDown) if (s.gameFunction) s.gameFunction();
            s.isDown = down;
        }

        if (paused) {
            for (auto& in : inputList) in.state = 0;
            SafeSendMKey(false);
            g_zoomPulseActive = false;
            Sleep(10);
            continue;
        }

        POINT c; GetCursorPos(&c);
        if (autoMouseDrag && (c.x >= playSpace.right-1 || c.x <= playSpace.left || c.y >= playSpace.bottom-1 || c.y <= playSpace.top)) ResetPos();
        
        bool anyTriggerActive = false;
        bool anyTriggerWantM = false;

        // ★ PASS 1: Trigger(ブースト)の判定
        for (auto& in : inputList) {
            if (in.inpNum == 0 || in.type != MacroType::TriggerLoop) continue;
            
            bool physicalDown = g_PhysicalKeys[in.inpNum];
            bool physicalHit = g_PhysicalKeysHit[in.inpNum].exchange(false);
            bool down = physicalDown || physicalHit;

            // 150msのデバウンス（Windowsの偽KeyUpによるチャージ中断をブロック）
            if (!physicalDown && in.physicalWasDown) in.releaseTime = now;
            in.physicalWasDown = physicalDown;
            if (!physicalDown && std::chrono::duration_cast<std::chrono::milliseconds>(now - in.releaseTime).count() < 150) {
                down = true;
            }

            if (down) {
                if (in.state == 0) {
                    in.state = (boostTriggerDelayMs > 0) ? 3 : 1;
                    in.lastTime = now;
                } else if (in.state == 3) {
                    if (std::chrono::duration_cast<std::chrono::milliseconds>(now - in.lastTime).count() >= boostTriggerDelayMs) {
                        in.state = 1; in.lastTime = now;
                    }
                } else if (in.state == 1) {
                    if (std::chrono::duration_cast<std::chrono::milliseconds>(now - in.lastTime).count() >= boostTriggerHoldMs) {
                        in.state = 2; in.lastTime = now;
                    }
                } else if (in.state == 2) {
                    if (std::chrono::duration_cast<std::chrono::milliseconds>(now - in.lastTime).count() >= boostTriggerIntervalMs) {
                        in.state = 1; in.lastTime = now;
                    }
                }
            } else {
                in.state = 0;
            }

            if (in.state != 0) anyTriggerActive = true;
            if (in.state == 1) anyTriggerWantM = true;
            
            in.isDown = down;
        }

        // ★ PASS 2: その他の機能（Zoomやスワイプ）の判定
        for (auto& in : inputList) {
            if (in.inpNum == 0 || in.type == MacroType::TriggerLoop) continue; 
            
            bool physicalDown = g_PhysicalKeys[in.inpNum];
            bool physicalHit = g_PhysicalKeysHit[in.inpNum].exchange(false);
            bool down = physicalDown || physicalHit;

            // Zoom (右クリック / Cキー 共通処理)
            if (in.type == MacroType::ZoomPulse) {
                if (down && !in.isDown) {
                    // ★ ブーストが起動していない時だけ、ズームパルスのタイマーをセット (0ms遅延)
                    if (!anyTriggerActive) {
                        g_zoomPulseEndTime = now + std::chrono::milliseconds(keyWait);
                        g_zoomPulseActive = true;
                    }
                }
                if (!down && in.isDown) {
                    // 右クリックを離した時のみ、スタイラス(ドラッグ)を復帰
                    if (autoMouseDrag && in.inpNum == 0x02) RightDown(); 
                }
                in.isDown = down;
            }
            // Continuous (スワイプ連打)
            else if (in.type == MacroType::Continuous) {
                if (down) {
                    if (!in.isDown || std::chrono::duration_cast<std::chrono::milliseconds>(now - in.lastTime).count() >= boostInterval) {
                        if (in.gameFunction) in.gameFunction();
                        in.lastTime = now;
                    }
                }
                in.isDown = down;
            } 
            // Single (単発マクロ)
            else {
                if (down && !in.isDown) {
                    if (in.gameFunction) in.gameFunction();
                }
                in.isDown = down;
            }
        }
        
        // ★ 最終的なMキーの絶対同期
        if (g_zoomPulseActive && now >= g_zoomPulseEndTime) {
            g_zoomPulseActive = false;
        }

        // ブースト要求があるか、Zoomパルス中なら確実にON。それ以外は確実にOFF。
        SafeSendMKey(anyTriggerWantM || g_zoomPulseActive);

        Sleep(1); // CPU使用率を抑え、エミュレータを窒息させない
    }
    SwapMouseButton(FALSE); SendMouseButton(MOUSEEVENTF_RIGHTUP | MOUSEEVENTF_LEFTUP);
    return 0;
}

//======================== Win32 UI ========================//

static void PaintBackground(HWND hwnd, COLORREF color) {
    SetClassLongPtrW(hwnd, GCLP_HBRBACKGROUND, (LONG_PTR)CreateSolidBrush(color));
    RedrawWindow(hwnd, nullptr, nullptr, RDW_ERASE | RDW_INVALIDATE); SendMessageW(hwnd, WM_PAINT, 0, 0);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (uMsg == WM_COMMAND && LOWORD(wParam) == START_BUTTON_ID) {
        gUseSavedClipAtStart = (SendMessageW(gHCheckUseSaved, BM_GETCHECK, 0, 0) == BST_CHECKED);
        Sleep(2500); PaintBackground(hwnd, RGB(255, 0, 0)); on = 1; paused = 0; Run();
        PaintBackground(hwnd, RGB(40, 49, 117)); UpdateSavedClipLabel(); return 0;
    }
    if (uMsg == WM_INPUT) { ProcessRawInput(lParam); return 0; }
    if (uMsg == WM_SIZE) { GetWindowRect(hwnd, &windowRect); SetWindowPos(hwnd, nullptr, windowRect.left, windowRect.top, WINDOW_WIDTH, WINDOW_HEIGHT, SWP_SHOWWINDOW); return 0; }
    if (uMsg == WM_DESTROY) { running = false; PostQuitMessage(0); return 0; }
    return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int nCmdShow) {
    SwapMouseButton(FALSE); std::atexit([]{ SwapMouseButton(FALSE); });
    WNDCLASSW wc{0, WindowProc, 0, 0, hInst, nullptr, nullptr, (HBRUSH)(COLOR_WINDOW + 1), nullptr, L"AppClass"};
    RegisterClassW(&wc);
    HWND hwnd = CreateWindowExW(0, L"AppClass", L"DS Mouse Input Fix", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, WINDOW_WIDTH, WINDOW_HEIGHT, nullptr, nullptr, hInst, nullptr);
    gHwndMain = hwnd; gIsJapanese = _wsetlocale(LC_ALL, L"") && wcsstr(_wsetlocale(LC_ALL, nullptr), L"Japanese");

    LoadTomlConfig(); PaintBackground(hwnd, RGB(40, 49, 117));
    HFONT hFont = CreateFontW(-12, 0,0,0, FW_NORMAL, 0,0,0, DEFAULT_CHARSET, 0,0, DEFAULT_QUALITY, 0, gIsJapanese ? L"Meiryo UI" : L"Segoe UI");

    HWND hStart = CreateWindowW(L"BUTTON", L"Start", WS_TABSTOP|WS_VISIBLE|WS_CHILD|BS_DEFPUSHBUTTON, 10, 10, 365, 30, hwnd, (HMENU)START_BUTTON_ID, nullptr, nullptr);
    HWND hDrop = CreateWindowW(L"COMBOBOX", L"", CBS_DROPDOWNLIST|WS_CHILD|WS_VISIBLE, 10, 50, 365, 500, hwnd, (HMENU)DROPDOWN_ID, hInst, nullptr);
    SendMessageW(hDrop, CB_ADDSTRING, 0, (LPARAM)L"Metroid Prime Hunters"); SendMessageW(hDrop, CB_SETCURSEL, 0, 0);

    const wchar_t* msg = gIsJapanese ? L"「Start」で開始。2.5秒後に画面が赤くなる。\nそのタイミングでDS下画面の左下をクリックして、次に右上をクリックする。これでカーソルの範囲を下画面に制限できる。\nバックスペースキーで終了。右SHIFTキーで一時停止／再開できる。"
                                     : L"Press 'Start' to begin.\nAfter 2.5 seconds this window turns red.\nClick bottom-left, then top-right to clip cursor.\nBACKSPACE: kill, RIGHT SHIFT: pause/resume.";
    HWND hStatic = CreateWindowW(L"STATIC", msg, WS_CHILD|WS_VISIBLE, 10, 80, 365, 140, hwnd, (HMENU)3, hInst, nullptr);

    gHCheckUseSaved = CreateWindowW(L"BUTTON", gIsJapanese ? L"保存済み範囲を使用（次回クリップ省略）" : L"Use saved clip", WS_TABSTOP|WS_VISIBLE|WS_CHILD|BS_AUTOCHECKBOX, 10, 230, 365, 20, hwnd, (HMENU)CHECK_USE_SAVED_ID, nullptr, nullptr);
    gHLabelSaved = CreateWindowW(L"STATIC", L"", WS_CHILD|WS_VISIBLE, 10, 255, 365, 20, hwnd, (HMENU)STATIC_SAVED_LABEL_ID, nullptr, nullptr);

    for (HWND h : {hwnd, hStart, hDrop, hStatic, gHCheckUseSaved, gHLabelSaved}) SendMessageW(h, WM_SETFONT, (WPARAM)hFont, TRUE);
    UpdateSavedClipLabel(); ShowWindow(hwnd, nCmdShow); UpdateWindow(hwnd);
    if (HWND cw = GetConsoleWindow()) ShowWindow(cw, SW_HIDE);

    MSG m{}; while (GetMessageW(&m, nullptr, 0, 0) > 0) { TranslateMessage(&m); DispatchMessageW(&m); }
    if (hFont) DeleteObject(hFont); return 0;
}