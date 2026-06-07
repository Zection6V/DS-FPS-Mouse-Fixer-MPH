#include "RawInputHandler.h"

#include "AppState.h"
#include "InputUtils.h"

void ProcessRawInput(LPARAM lParam) {
    UINT sz = sizeof(RAWINPUT);
    static BYTE lpb[sizeof(RAWINPUT)];
    if (GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT, lpb, &sz, sizeof(RAWINPUTHEADER)) == static_cast<UINT>(-1)) {
        return;
    }

    auto* raw = reinterpret_cast<RAWINPUT*>(lpb);
    if (raw->header.hDevice == nullptr) {
        return;
    }

    if (raw->header.dwType == RIM_TYPEKEYBOARD) {
        const RAWKEYBOARD& kb = raw->data.keyboard;
        UINT vk = kb.VKey;
        if (vk == 0) {
            vk = MapVirtualKeyW(kb.MakeCode, MAPVK_VSC_TO_VK_EX);
        }

        if (vk > 0 && vk < 255) {
            if (vk == VK_SHIFT) {
                vk = (kb.MakeCode == 0x36) ? VK_RSHIFT : VK_LSHIFT;
            } else if (vk == VK_CONTROL) {
                vk = (kb.Flags & RI_KEY_E0) ? VK_RCONTROL : VK_LCONTROL;
            } else if (vk == VK_MENU) {
                vk = (kb.Flags & RI_KEY_E0) ? VK_RMENU : VK_LMENU;
            }

            bool down = !(kb.Flags & RI_KEY_BREAK);
            g_PhysicalKeys[vk] = down;
            if (down) {
                g_PhysicalKeysHit[vk] = true;
            }
        }
    } else if (raw->header.dwType == RIM_TYPEMOUSE) {
        const RAWMOUSE& m = raw->data.mouse;
        USHORT flags = m.usButtonFlags;

        if (flags & RI_MOUSE_LEFT_BUTTON_DOWN) {
            g_PhysicalKeys[VK_LBUTTON] = true;
            g_PhysicalKeysHit[VK_LBUTTON] = true;
            if (!g_State.leftButtonDown.exchange(true)) {
                if (!g_State.isPaused) {
                    InputUtils::SendShootKey(true);
                }
            }
        } else if (flags & RI_MOUSE_LEFT_BUTTON_UP) {
            g_PhysicalKeys[VK_LBUTTON] = false;
            if (g_State.leftButtonDown.exchange(false)) {
                if (!g_State.isPaused) {
                    InputUtils::SendShootKey(false);
                }
            }
        }

        if (flags & RI_MOUSE_RIGHT_BUTTON_DOWN) {
            g_PhysicalKeys[VK_RBUTTON] = true;
            g_PhysicalKeysHit[VK_RBUTTON] = true;
        }
        if (flags & RI_MOUSE_RIGHT_BUTTON_UP) {
            g_PhysicalKeys[VK_RBUTTON] = false;
        }

        if (flags & RI_MOUSE_MIDDLE_BUTTON_DOWN) {
            g_PhysicalKeys[VK_MBUTTON] = true;
            g_PhysicalKeysHit[VK_MBUTTON] = true;
        }
        if (flags & RI_MOUSE_MIDDLE_BUTTON_UP) {
            g_PhysicalKeys[VK_MBUTTON] = false;
        }
        if (flags & RI_MOUSE_BUTTON_4_DOWN) {
            g_PhysicalKeys[VK_XBUTTON1] = true;
            g_PhysicalKeysHit[VK_XBUTTON1] = true;
        }
        if (flags & RI_MOUSE_BUTTON_4_UP) {
            g_PhysicalKeys[VK_XBUTTON1] = false;
        }
        if (flags & RI_MOUSE_BUTTON_5_DOWN) {
            g_PhysicalKeys[VK_XBUTTON2] = true;
            g_PhysicalKeysHit[VK_XBUTTON2] = true;
        }
        if (flags & RI_MOUSE_BUTTON_5_UP) {
            g_PhysicalKeys[VK_XBUTTON2] = false;
        }
    }
}
