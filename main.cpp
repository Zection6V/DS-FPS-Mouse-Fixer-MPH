/*
目的：このファイルにはスクリプト制御のすべてのUI要素が含まれています。ドロップダウンメニュー、開始ボタン、機能を説明するテキストボックスを含む簡単なUIを作成します。

注意：再ビルド時にこのファイルをコンパイルしてください
*/

#include <windows.h>
#include <cstdio>
#include <cstring>

#include "Main_Loop.cpp"

int on = 1, paused = 0;

RECT windowRect;

int gameSelected = 7;  // Metroid Prime Huntersに設定（7）

//========================UI=========================//

constexpr int START_BUTTON_ID = 1;
constexpr int DROPDOWN_ID = 3;
#define WINDOW_WIDTH 400
#define WINDOW_HEIGHT 290

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // ウィンドウクラスを登録
    constexpr char CLASS_NAME[] = "Sample Window Class";

    WNDCLASS wc = {0};
    wc.hbrBackground = static_cast<HBRUSH>(GetStockObject(COLOR_WINDOW));
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    RegisterClass(&wc);

    // ウィンドウを作成
    HWND hwnd = CreateWindowEx(
        0,                          // オプションのウィンドウスタイル
        CLASS_NAME,                 // ウィンドウクラス
        "DS Mouse Input Fix",       // ウィンドウテキスト
        WS_OVERLAPPEDWINDOW,        // ウィンドウスタイル

        // 位置とサイズ
        CW_USEDEFAULT, CW_USEDEFAULT, WINDOW_WIDTH, WINDOW_HEIGHT,

        nullptr,       // 親ウィンドウ
        nullptr,       // メニュー
        hInstance,  // インスタンスハンドル
        nullptr        // 追加のアプリケーションデータ
    );

    HBRUSH brush = CreateSolidBrush(RGB(40, 49, 117));
    SetClassLongPtr(hwnd, GCLP_HBRBACKGROUND, reinterpret_cast<LONG_PTR>(brush));

    // ボタンを作成
    HWND button = CreateWindow(
        "BUTTON",                   // ボタンクラス
        "Start",                    // ボタンテキスト
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // ボタンスタイル
        10,                         // X位置
        10,                         // Y位置
        365,                        // 幅
        30,                         // 高さ
        hwnd,                       // 親ウィンドウハンドル
        reinterpret_cast<HMENU>(START_BUTTON_ID),     // ボタン識別子
        nullptr,                       // インスタンスハンドル
        nullptr                        // 追加のアプリケーションデータ
    );

    HWND hDropdown = CreateWindow(
        "combobox",
        "",
        CBS_DROPDOWNLIST | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE,
        10,
        50,
        365,
        500,
        hwnd,
        reinterpret_cast<HMENU>(DROPDOWN_ID),  // ドロップダウン識別子
        hInstance,
        nullptr
    );

    // ドロップダウンにMetroid Prime Huntersのみを追加
    SendMessage(hDropdown, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>("Metroid Prime Hunters"));

    // 唯一のオプションを自動的に選択
    SendMessage(hDropdown, CB_SETCURSEL, 0, 0);

    // 現在のロケールを取得
    wchar_t *locale = _wsetlocale(LC_ALL, L"");
    wchar_t message[1024];

    // 日本語ロケールかどうかをチェック
    if (wcsstr(locale, L"Japanese") != nullptr) {
        // 日本語メッセージ
        wcscpy_s(message, sizeof(message) / sizeof(wchar_t),
            L"「Start」で開始。2.5秒後に画面が赤くなる。\n"
            L"そのタイミングでDS下画面の左下をクリックして、次に右上をクリックする。これでカーソルの範囲を下画面に制限できる。\n"
            L"バックスペースキー（エンターキーの上）で制限を終了。\n右SHIFTキーで一時停止／再開できる。");
    } else {
        // 英語メッセージ
        wcscpy_s(message, sizeof(message) / sizeof(wchar_t),
            L"Press 'Start' to begin. \nAfter starting you will have 2 seconds before this window turns red, indicating the script is active. "
            L"Once active, click the bottom-left and top-right corners of your stylus play area, and then enjoy. "
            L"REMINDER: Pressing RIGHT_SHIFT at any time will pause this program, and BACKSPACE will kill it.");
    }

    // テキストボックスを作成
    HWND hStatic = CreateWindowW(
        L"STATIC",  // ウィンドウクラス名
        message,  // 表示するテキスト
        WS_CHILD | WS_VISIBLE,  // ウィンドウスタイル（子ウィンドウとして表示され、可視状態）
        10,  // X位置（親ウィンドウの左上隅からの水平オフセット）
        80,  // Y位置（親ウィンドウの左上隅からの垂直オフセット）
        365,  // 幅（テキストボックスの横幅）
        160,  // 高さ（テキストボックスの縦幅）
        hwnd,  // 親ウィンドウハンドル
        reinterpret_cast<HMENU>(3),  // コントロール識別子（メニューまたはコントロールID）
        hInstance,  // インスタンスハンドル
        nullptr  // 追加のアプリケーションデータ（使用しない）
    );

    // ウィンドウを表示
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);
    // コンソールを非表示
    HWND consoleWindow = GetConsoleWindow();
    ShowWindow(consoleWindow, SW_HIDE);

    // メッセージループ
    MSG msg = {nullptr};
    while (GetMessage(&msg, nullptr, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

// メッセージに反応
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_COMMAND:
            // ボタンが押されたら、2.5秒待ってから画面を赤に変え、スクリプトがアクティブであることを示す
            // その後、メインループを実行
            if (LOWORD(wParam) == START_BUTTON_ID)
            {
                Sleep(2500);
                HBRUSH brush = CreateSolidBrush(RGB(255, 0, 0));
                SetClassLongPtr(hwnd, GCLP_HBRBACKGROUND, reinterpret_cast<LONG_PTR>(brush));
                RedrawWindow(hwnd, nullptr, nullptr, RDW_ERASE | RDW_INVALIDATE);
                SendMessage(hwnd, WM_PAINT, wParam, lParam);
                on = 1;
                paused = 0;
                Run();
                FlashWindow(hwnd, 1);
                brush = CreateSolidBrush(RGB(40, 49, 117));
                SetClassLongPtr(hwnd, GCLP_HBRBACKGROUND, reinterpret_cast<LONG_PTR>(brush));
                SendMessage(hwnd, WM_PAINT, wParam, lParam);
                RedrawWindow(hwnd, nullptr, nullptr, RDW_ERASE | RDW_INVALIDATE);
            }
            break;
        case WM_INPUT:
            // このクラスの静的メソッドを呼び出す
            ProcessRawInput(lParam);
            return 0;
        // ユーザーがウィンドウのサイズを変更しようとしたら、許可しない
        case WM_SIZE:
            GetWindowRect(hwnd, &windowRect);
            SetWindowPos(hwnd, nullptr, windowRect.left, windowRect.top, WINDOW_WIDTH, WINDOW_HEIGHT, SWP_SHOWWINDOW);
            return 0;
        // ウィンドウを破棄
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

//========================UI=========================//