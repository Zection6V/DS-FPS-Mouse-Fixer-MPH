/*
目的: このファイルにはスクリプトのメインループが含まれている。プレイスペースとコントロール、その他の必要な値を初期化し、その後入力リストのポーリングを開始する.
*/

#include "FileHandler.cpp"
#include <winuser.h>
#include <windows.h>
#include <atomic>

// アトミック変数の定義
std::atomic<bool> running(true);
std::atomic<bool> leftButtonDown(false);

// 構造体の前方宣言
struct Input;

// キーシミュレーション関数
__forceinline void SimulateKeyPress(UINT vKey) {
    // keybd_event(vKey, 0, KEYEVENTF_KEYUP, 0);
    keybd_event(vKey, 0, 0, 0);
}

__forceinline void SimulateKeyRelease(UINT vKey) {
    keybd_event(vKey, 0, KEYEVENTF_KEYUP, 0);
}

// キー入力処理関数
__forceinline void ProcessKeyInput(UINT vKey, bool isKeyDown) {

    if (vKey == 'C') {
        if (isKeyDown) {
            SimulateKeyPress(0x4D);  // 'M' key
            //std::cout << "C key down: M key pressed" << std::endl;
        } else {
            SimulateKeyRelease(0x4D);  // 'M' key
            //std::cout << "C key up: M key released" << std::endl;
        }
    }
}

// マウス入力処理関数
__forceinline void ProcessMouseInput(RAWINPUT* raw) {
    if (raw->data.mouse.usButtonFlags & RI_MOUSE_LEFT_BUTTON_DOWN) {
        if (!leftButtonDown.exchange(true)) {
            SimulateKeyPress(0x4E);  // 'N' key
            //std::cout << "Left click down: N key pressed" << std::endl;
        }
    }
    else if (raw->data.mouse.usButtonFlags & RI_MOUSE_LEFT_BUTTON_UP) {
        if (leftButtonDown.exchange(false)) {
            SimulateKeyRelease(0x4E);  // 'N' key
            //std::cout << "Left click up: N key released" << std::endl;
        }
    }
}

// RAW入力処理関数
__forceinline  void ProcessRawInput(LPARAM lParam) {
    if (paused) return;  // pausedの時は何もしない

    UINT dwSize = sizeof(RAWINPUT);
    static BYTE lpb[sizeof(RAWINPUT)];

    if (GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER)) != -1) {
        auto* raw = reinterpret_cast<RAWINPUT*>(lpb);
        if (raw->header.dwType == RIM_TYPEKEYBOARD) {
            ProcessKeyInput(raw->data.keyboard.VKey, !(raw->data.keyboard.Flags & RI_KEY_BREAK));
        } else if (raw->header.dwType == RIM_TYPEMOUSE) {
            ProcessMouseInput(raw);
        }
    }
}

// メイン実行関数
int Run() {

    RAWINPUTDEVICE rid[2] = {
        {0x01, 0x06, RIDEV_INPUTSINK, GetActiveWindow()}, // キーボード
        {0x01, 0x02, RIDEV_INPUTSINK, GetActiveWindow()}  // マウス
    };

    if (!RegisterRawInputDevices(rid, 2, sizeof(RAWINPUTDEVICE))) {
        return 1;
    }

    struct Input* stateCheckers = InitializeStateCheckers();
    struct Input* inputList = FileHandle();

    POINT bottomLeft, topRight;
    InitializePoints(&bottomLeft, &topRight);

    SwapMouseButton(TRUE);

    while (on) {
        MSG msg;
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_INPUT) {
                ProcessRawInput(msg.lParam);
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        InputCheck(stateCheckers, 2);

        if (!paused) {
            POINT cursorPos;
            GetCursorPos(&cursorPos);
            if (autoMouseDrag) {
                if (cursorPos.x >= rightBound || cursorPos.x <= leftBound ||
                    cursorPos.y >= bottomBound || cursorPos.y <= topBound) {
                    ResetPos();
                }
            }
            InputCheck(inputList, totalInputs);
        }
        // Sleep(10);
        YieldProcessor(); // CPU使用率を抑えつつ、低遅延を維持
    }

    SwapMouseButton(FALSE);
    mouse_event(MOUSEEVENTF_RIGHTUP | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
    return 0;
}