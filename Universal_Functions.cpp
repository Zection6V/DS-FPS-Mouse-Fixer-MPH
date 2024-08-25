/*
目的: このファイルには、他の「XXX_Functions.c」ファイルで使用される関数が含まれている。これらの関数は、すべてのゲームを実行するために必要.
*/

#include <windows.h>

extern int mouseUp, mouseDown, activeMouse;
extern int mouseResetWait, buttonWait, swapWait, keyWait;
extern int autoMouseDrag;
extern RECT playSpace;
extern POINT center;

__forceinline void GetActiveMouseDown(const int *waitTime)
{
    while (GetAsyncKeyState(activeMouse) >= 0) { }
    Sleep(*waitTime);
}

__forceinline void GetActiveMouseUp(const int *waitTime)
{
    while (GetAsyncKeyState(activeMouse) < 0) { }
    Sleep(*waitTime);
}

__forceinline void LockPlaySpace()
{
    ClipCursor(&playSpace);
}

__forceinline void CursorUp()
{
    mouse_event(mouseUp, 0, 0, 0, 0);
    GetActiveMouseUp(&mouseResetWait);
}

void ResetPos()
{
    mouse_event(mouseUp, 0, 0, 0, 0);
    GetActiveMouseUp(&mouseResetWait);
    SetCursorPos(center.x, center.y);

    if (autoMouseDrag)
    {
        mouse_event(mouseDown, 0, 0, 0, 0);
    }

    RECT tempRect;
    GetClipCursor(&tempRect);
    if (tempRect.left != playSpace.left || tempRect.right != playSpace.right ||
        tempRect.top != playSpace.top || tempRect.bottom != playSpace.bottom)
    {
        LockPlaySpace();
    }
}

int ResetPosAfterButton()
{
    GetActiveMouseDown(&buttonWait);
    mouse_event(mouseUp, 0, 0, 0, 0);
    GetActiveMouseUp(&buttonWait);
    SetCursorPos(center.x, center.y);
    if (autoMouseDrag)
    {
        mouse_event(mouseDown, 0, 0, 0, 0);
    }
    return 0;
}