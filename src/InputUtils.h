#pragma once

#include "Platform.h"

namespace InputUtils {
    constexpr int ACTIVE_MOUSE = 0x02;

    void SafeSendMKey(bool down);
    void SendKey(WORD vk, bool down);
    void SendShootKey(bool down);
    void SendMouseButton(WORD flags);
    void RightDown();
    void RightUp();
    void GetActiveMouseUp(int waitTime);
    void GetActiveMouseDown(int waitTime);
    void LockPlaySpaceAfterResetPos();
    void ResetPos();
    void ResetPosAfterButton();
    void CursorUp();
}
