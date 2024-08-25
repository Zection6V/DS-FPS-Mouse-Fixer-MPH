/*
目的: このファイルには、メトロイドプライムハンターズに必要なゲーム関数が含まれている。各関数は、ゲーム内の入力に対応するための必要なキー押下やマウスイベントを含むマクロを形成する.
*/

extern int leftBound, rightBound, topBound, bottomBound, weaponGrenadeToggle;
extern float distanceFromBoarderSides, distanceFromBoarderTop, distanceNextButton;


#include <thread>
#include <chrono>
#include <windows.h>


// メイン関数やその他の場所から呼び出す
__forceinline void Zoom_MPH_impl()
{
    // keybd_event('M', 0, KEYEVENTF_KEYUP, 0);
    keybd_event('M', 0, 0, 0);
    Sleep(keyWait);
    keybd_event('M', 0, KEYEVENTF_KEYUP, 0);
}

// ズーム機能を使用する関数
__forceinline void Zoom_MPH()
{
    std::thread zoom_thread(Zoom_MPH_impl);
    zoom_thread.detach(); // スレッドをデタッチして、バックグラウンドで実行
}

// メイン武器に切り替える関数
void AmmoTypeOne_MPH()
{
    CursorUp();
    SetCursorPos(center.x - distanceFromBoarderSides * 2, topBound + distanceFromBoarderTop / 2);
    mouse_event(mouseDown, 0, 0, 0, 0);
    ResetPosAfterButton();
}

// ミサイルに切り替える関数
void AmmoTypeTwo_MPH()
{
    CursorUp();
    SetCursorPos(center.x, topBound + distanceFromBoarderTop / 2);
    mouse_event(mouseDown, 0, 0, 0, 0);
    ResetPosAfterButton();
}

// 第三の武器に切り替える関数
void AmmoTypeThree_MPH()
{
    CursorUp();
    SetCursorPos(center.x + distanceFromBoarderSides * 2, topBound + distanceFromBoarderTop / 2);
    mouse_event(mouseDown, 0, 0, 0, 0);
    ResetPosAfterButton();
}

// スキャンビジョンを使用する関数
void ScanVision_MPH()
{
    CursorUp();
    SetCursorPos(center.x, bottomBound - distanceFromBoarderTop / 2);
    mouse_event(mouseDown, 0, 0, 0, 0);
    Sleep(500);
    ResetPosAfterButton();
}

// ボールモードに切り替える関数
void Ball_MPH()
{
    CursorUp();
    SetCursorPos(rightBound - distanceFromBoarderSides * 1.5f, bottomBound - distanceFromBoarderTop / 2);
    mouse_event(mouseDown, 0, 0, 0, 0);
    ResetPosAfterButton();
}

// 特殊武器1を使用する関数
void WeaponOne_MPH()
{
    CursorUp();
    SetCursorPos(rightBound - distanceFromBoarderSides * 2, topBound + distanceFromBoarderTop);
    mouse_event(mouseDown, 0, 0, 0, 0);
    GetActiveMouseDown(&mouseResetWait);
    SetCursorPos(center.x - distanceFromBoarderSides * 1.5, topBound + distanceFromBoarderTop);
    ResetPosAfterButton();
}

// 特殊武器2を使用する関数
void WeaponTwo_MPH()
{
    CursorUp();
    SetCursorPos(rightBound - distanceFromBoarderSides * 2, topBound + distanceFromBoarderTop);
    mouse_event(mouseDown, 0, 0, 0, 0);
    GetActiveMouseDown(&mouseResetWait);
    SetCursorPos(center.x - distanceFromBoarderSides * 1.5, center.y - distanceFromBoarderTop / 4);
    ResetPosAfterButton();
}

// 特殊武器3を使用する関数
void WeaponThree_MPH()
{
    CursorUp();
    SetCursorPos(rightBound - distanceFromBoarderSides * 2, topBound + distanceFromBoarderTop);
    mouse_event(mouseDown, 0, 0, 0, 0);
    GetActiveMouseDown(&mouseResetWait);
    SetCursorPos(center.x - distanceFromBoarderSides, center.y + distanceFromBoarderTop / 1.5f);
    ResetPosAfterButton();
}

// 特殊武器4を使用する関数
void WeaponFour_MPH()
{
    CursorUp();
    SetCursorPos(rightBound - distanceFromBoarderSides * 2, topBound + distanceFromBoarderTop);
    mouse_event(mouseDown, 0, 0, 0, 0);
    GetActiveMouseDown(&mouseResetWait);
    SetCursorPos(center.x + distanceFromBoarderSides / 2, bottomBound - distanceFromBoarderTop / 1.2f);
    ResetPosAfterButton();
}

// 特殊武器5を使用する関数
void WeaponFive_MPH()
{
    CursorUp();
    SetCursorPos(rightBound - distanceFromBoarderSides * 2, topBound + distanceFromBoarderTop);
    mouse_event(mouseDown, 0, 0, 0, 0);
    GetActiveMouseDown(&mouseResetWait);
    SetCursorPos(center.x + distanceFromBoarderSides * 2, bottomBound - distanceFromBoarderTop / 1.5f);
    ResetPosAfterButton();
}

// 特殊武器6を使用する関数
void WeaponSix_MPH()
{
    CursorUp();
    SetCursorPos(rightBound - distanceFromBoarderSides * 2, topBound + distanceFromBoarderTop);
    mouse_event(mouseDown, 0, 0, 0, 0);
    GetActiveMouseDown(&mouseResetWait);
    SetCursorPos(rightBound - distanceFromBoarderSides * 1.8, bottomBound - distanceFromBoarderTop / 2);
    ResetPosAfterButton();
}

// メニューでOKをクリックする関数
void ClickOK_MPH()
{
    CursorUp();
    SetCursorPos(center.x, bottomBound - distanceFromBoarderTop * 1.2);
    mouse_event(mouseDown, 0, 0, 0, 0);
    ResetPosAfterButton();
}

// メニューでYesをクリックする関数
void ClickYes_MPH()
{
    CursorUp();
    SetCursorPos(center.x - distanceFromBoarderSides * 2, bottomBound - distanceFromBoarderTop * 1.2);
    mouse_event(mouseDown, 0, 0, 0, 0);
    ResetPosAfterButton();
}

// メニューでNoをクリックする関数
void ClickNo_MPH()
{
    CursorUp();
    SetCursorPos(center.x + distanceFromBoarderSides * 2, bottomBound - distanceFromBoarderTop * 1.2);
    mouse_event(mouseDown, 0, 0, 0, 0);
    ResetPosAfterButton();
}

// メニューで左をクリックする関数
void Left_MPH()
{
    CursorUp();
    SetCursorPos(center.x - distanceFromBoarderSides * 2.5, bottomBound - distanceFromBoarderTop * 1.2);
    mouse_event(mouseDown, 0, 0, 0, 0);
    ResetPosAfterButton();
}

// メニューで右をクリックする関数
void Right_MPH()
{
    CursorUp();
    SetCursorPos(center.x + distanceFromBoarderSides * 2.5, bottomBound - distanceFromBoarderTop * 1.2);
    mouse_event(mouseDown, 0, 0, 0, 0);
    ResetPosAfterButton();
}

/* 左にストレイフする関数
void LeftStrafe_MPH()
{
    ResetPos();
    GetActiveMouseDown(&mouseResetWait);
    SetCursorPos(leftBound, center.y);
    ResetPos();
}

// 右にストレイフする関数
void RightStrafe_MPH()
{
    ResetPos();
    GetActiveMouseDown(&mouseResetWait);
    SetCursorPos(rightBound, center.y);
    ResetPos();
}
*/
