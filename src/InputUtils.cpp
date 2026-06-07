#include "InputUtils.h"

#include "AppState.h"

#include <thread>

namespace InputUtils {
    void SafeSendMKey(bool down) {
        if (g_State.isMKeyPressed.exchange(down) == down) {
            return;
        }

        INPUT in{};
        in.type = INPUT_KEYBOARD;
        in.ki.wScan = static_cast<WORD>(MapVirtualKeyW(0x4D, MAPVK_VK_TO_VSC));
        in.ki.wVk = 0x4D;
        in.ki.dwFlags = KEYEVENTF_SCANCODE | (down ? 0 : KEYEVENTF_KEYUP);
        SendInput(1, &in, sizeof(in));
    }

    void SendKey(WORD vk, bool down) {
        if (vk == 'M' || vk == 'm' || vk == 0x4D) {
            SafeSendMKey(down);
            return;
        }

        INPUT in{};
        in.type = INPUT_KEYBOARD;
        in.ki.wScan = static_cast<WORD>(MapVirtualKeyW(vk, MAPVK_VK_TO_VSC));
        in.ki.wVk = vk;
        in.ki.dwFlags = KEYEVENTF_SCANCODE | (down ? 0 : KEYEVENTF_KEYUP);
        SendInput(1, &in, sizeof(in));
    }

    void SendShootKey(bool down) {
        SendKey('N', down);
    }

    void SendMouseButton(WORD flags) {
        INPUT in{INPUT_MOUSE};
        in.mi.dwFlags = flags;
        SendInput(1, &in, sizeof(in));
    }

    void RightDown() {
        SendMouseButton(MOUSEEVENTF_RIGHTDOWN);
    }

    void RightUp() {
        SendMouseButton(MOUSEEVENTF_RIGHTUP);
    }

    void GetActiveMouseUp(int waitTime) {
        while (GetAsyncKeyState(ACTIVE_MOUSE) < 0) {
            Sleep(1);
        }
        Sleep(waitTime);
    }

    void GetActiveMouseDown(int waitTime) {
        while (GetAsyncKeyState(ACTIVE_MOUSE) >= 0) {
            Sleep(1);
        }
        Sleep(waitTime);
    }

    void LockPlaySpaceAfterResetPos() {
        RECT t;
        GetClipCursor(&t);
        if (t.left != g_State.playSpace.left || t.right != g_State.playSpace.right ||
            t.top != g_State.playSpace.top || t.bottom != g_State.playSpace.bottom) {
            ClipCursor(&g_State.playSpace);
        }
    }

    void ResetPos() {
        RightUp();
        GetActiveMouseUp(g_Config.mouseResetWait);
        SetCursorPos(g_State.center.x, g_State.center.y);
        if (g_Config.autoMouseDrag) {
            RightDown();
        }
        std::thread(LockPlaySpaceAfterResetPos).detach();
    }

    void ResetPosAfterButton() {
        GetActiveMouseDown(g_Config.buttonWait);
        RightUp();
        GetActiveMouseUp(g_Config.buttonWait);
        SetCursorPos(g_State.center.x, g_State.center.y);
        if (g_Config.autoMouseDrag) {
            RightDown();
        }
    }

    void CursorUp() {
        RightUp();
        GetActiveMouseUp(g_Config.mouseResetWait);
    }
}
