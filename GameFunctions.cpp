/*
目的: このファイルには、後の関数のために値を初期化するか、入力リストをポーリングするドライバ関数が含まれている。また、他のファイルに収まらなかった少数の例外も含まれている.
*/

#include "InputStruct.cpp"
#include "Universal_Functions.cpp"
#include "MPH_Functions.cpp"

// 外部変数の宣言（他のファイルで定義されている）
extern int on, paused;

int leftBound, rightBound, topBound, bottomBound, weaponGrenadeToggle;

int mouseUp = MOUSEEVENTF_RIGHTUP, mouseDown = MOUSEEVENTF_RIGHTDOWN, activeMouse = 0x02;

float screenWidthRatio, screenHeightRatio, referenceWidth = 587, referenceHeight = 441;

float distanceFromBoarderSides = 50, distanceFromBoarderTop = 100, distanceNextButton = 100;
int mouseResetWait = 33, buttonWait = 33, swapWait = 220, keyWait = 30;
int autoMouseDrag = 1;

RECT playSpace;

POINT center;

// ユーザーのスタイラススクリーンのプレイスペースを初期化する関数
// 最初のクリックを待ち、そのカーソル位置を左下のポイントに設定、次のクリックで右上のポイントを設定する
int InitializePoints(POINT* bottomLeft, POINT* topRight)
{
    // initializeはどのクリックが送信されたかをカウントする（0の時は左下を設定、0でない時は右上を設定）
    int initialize = 0;
    int clickDown = 0;
    while (initialize <= 1)
    {
        // マウスクリックを取得する場合
        if (GetAsyncKeyState(VK_LBUTTON) & 0x8000)
        {
            // 以前クリックされていなかった場合（スクリプトはマウスがダウン入力されたときにのみ呼ばれ、マウスが保持されているときには呼ばれない）
            if (!clickDown)
            {
                clickDown = 1;
                // 最初のクリックの場合、左下を初期化
                if (initialize == 0)
                {
                    GetCursorPos(bottomLeft);
                    initialize++;
                }
                // 二回目のクリックの場合、右上を初期化し、関数値をそれに応じてスケーリング
                else
                {
                    GetCursorPos(topRight);

                    // クリックとスクリプトのアクティベーションの間に少しの遅延を設けるためにスリープ
                    Sleep(100);

                    // 2つのポイントに基づいて境界を割り当てる
                    leftBound = bottomLeft->x;
                    rightBound = topRight->x;
                    topBound = topRight->y;
                    bottomBound = bottomLeft->y;

                    SetRect(&playSpace, leftBound, topBound, rightBound, bottomBound);

                    leftBound += 1;
                    rightBound -= 1;
                    topBound += 1;
                    bottomBound -= 1;

                    // プレイスペースの幅と高さを決定
                    float screenWidth = rightBound - leftBound;
                    float screenHeight = bottomBound - topBound;

                    // 現在のプレイスペースと参照プレイスペース（個人的な測定値）を比較して比率を取得
                    screenWidthRatio = screenWidth / referenceWidth;
                    screenHeightRatio = screenHeight / referenceHeight;

                    // (ユーザーのプレイスペース / 参照プレイスペース) の比率を使用して、デフォルトの距離値をその比率でスケーリング
                    distanceFromBoarderSides = 50 * screenWidthRatio;
                    distanceNextButton = 100 * screenWidthRatio;
                    distanceFromBoarderTop = 100 * screenHeightRatio;

                    // プレイスペースの中心ピクセルを初期化
                    center.x = (((rightBound - leftBound) / 2) + leftBound);
                    center.y = (((bottomBound - topBound) / 2) + topBound);
                    ResetPos();
                    return 1;
                }
            }
        }
        else
        {
            clickDown = 0;
        }
    }
}

// 攻撃に使用するボタンを押す関数
void Shoot_Button_Down()
{
    //keybd_event('N', 0, KEYEVENTF_KEYUP, 0);
    //keybd_event('N', 0, 0, 0);
}

// 攻撃に使用するボタンを離す関数
void Shoot_Button_Up()
{
    //keybd_event('N', 0, KEYEVENTF_KEYUP, 0);
}

// スクリプトを一時停止する関数（再開可能）
void Pause()
{
    mouse_event(mouseUp, 0, 0, 0, 0);
    SwapMouseButton(paused);
    Shoot_Button_Up();
    paused = !paused;
    if (paused)
    {
        ClipCursor(nullptr);
    }
    else
    {
        ResetPos();
        // LockPlaySpace();
    }
}

// スクリプトを終了する関数（再開可能）
void Kill()
{
    on = 0;
    paused = 1;
    mouse_event(mouseUp, 0, 0, 0, 0);
    Shoot_Button_Up();
    SwapMouseButton(0);
    ClipCursor(nullptr);
}

// いくつかの入力構造体でスペースを埋めるための何もしないスクリプト
void DoNothing()
{
    }
// 入力構造体を取得してポーリングする関数
void RunInput(struct Input *input)
{
    // キーの状態を1回だけ取得し、結果を保存
    SHORT keyState = GetAsyncKeyState(input->inpNum);
    BOOL isKeyDown = (keyState & 0x8000) != 0;  // キーが押されているかどうかを判定

    if (input->inpNum == 0x02)  // 右マウスボタンの場合
    {
        if (!isKeyDown && input->isDown)  // キーが離された瞬間を検出
        {
            if (autoMouseDrag)
            {
                mouse_event(mouseDown, 0, 0, 0, 0);
            }
            // TODO いらんかも
            input->gameFunction();  // ゲーム機能を実行

            //ResetPos();  // 位置をリセット

        }
    }
    else  // 右マウス以外の入力の場合
    {
        if (isKeyDown && !input->isDown)  // キーが押された瞬間を検出
        {
            input->gameFunction();  // ゲーム機能を実行
        }
        else if (!isKeyDown && input->isDown && input->gameFunction == Shoot_Button_Down)
        {
            // シュートボタンが離された場合の処理
            Shoot_Button_Up();
        }
    }

    // 現在のキーの状態を保存（次回の呼び出し時に使用）
    input->isDown = isKeyDown;
}

// 入力構造体のリストを取得して全てをチェックする関数
void InputCheck(struct Input* inputs, int length)
{
    // printf("%d\n", inputs[0].inpNum);
    for (int i = 0; i < length; i++)
    {
        RunInput(&inputs[i]);
    }
}

// ステートチェッカーの入力構造体リストを初期化する関数、これはスクリプトが一時停止または終了されているかどうかをチェックするために使用される
// 注意: PauseとKillボタンは右シフトキーとバックスペースキーに永久に設定されている
// InitializeStateCheckers関数の定義
Input* InitializeStateCheckers() {
    // メモリを動的に確保(2つのInput構造体分)
    auto* stateCheckers = new Input[2];

    // 最初のInput構造体の設定
    stateCheckers[0].isDown = false; // ボタンが押されていない
    stateCheckers[0].gameFunction = Pause; // Pause関数をセット
    stateCheckers[0].inpNum = 0xA1; // 入力番号0xA1

    // 二番目のInput構造体の設定
    stateCheckers[1].isDown = false; // ボタンが押されていない
    stateCheckers[1].gameFunction = Kill; // Kill関数をセット
    stateCheckers[1].inpNum = 0x08; // 入力番号0x08

    // 構造体の配列を返す
    return stateCheckers;
}
