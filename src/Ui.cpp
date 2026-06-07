#include "Ui.h"

#include "AppConstants.h"
#include "AppState.h"
#include "ConfigManager.h"
#include "GameLoop.h"
#include "RawInputHandler.h"

#include <clocale>
#include <cstdlib>
#include <cwchar>

namespace {
    void PaintBackground(HWND hwnd, COLORREF color) {
        SetClassLongPtrW(hwnd, GCLP_HBRBACKGROUND, reinterpret_cast<LONG_PTR>(CreateSolidBrush(color)));
        RedrawWindow(hwnd, nullptr, nullptr, RDW_ERASE | RDW_INVALIDATE);
        SendMessageW(hwnd, WM_PAINT, 0, 0);
    }

    LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
        if (uMsg == WM_COMMAND && LOWORD(wParam) == START_BUTTON_ID) {
            g_State.useSavedClipAtStart = (SendMessageW(g_State.hCheckUseSaved, BM_GETCHECK, 0, 0) == BST_CHECKED);
            Sleep(2500);
            PaintBackground(hwnd, RGB(255, 0, 0));
            g_State.isOn = 1;
            g_State.isPaused = 0;
            Run();
            PaintBackground(hwnd, RGB(40, 49, 117));
            UpdateSavedClipLabel();
            return 0;
        }

        if (uMsg == WM_INPUT) {
            ProcessRawInput(lParam);
            return 0;
        }

        if (uMsg == WM_SIZE) {
            GetWindowRect(hwnd, &g_State.windowRect);
            SetWindowPos(hwnd, nullptr, g_State.windowRect.left, g_State.windowRect.top, WINDOW_WIDTH, WINDOW_HEIGHT, SWP_SHOWWINDOW);
            return 0;
        }

        if (uMsg == WM_DESTROY) {
            g_State.isRunning = false;
            PostQuitMessage(0);
            return 0;
        }

        return DefWindowProcW(hwnd, uMsg, wParam, lParam);
    }
}

int RunApplication(HINSTANCE hInst, int nCmdShow) {
    SwapMouseButton(FALSE);
    std::atexit([] { SwapMouseButton(FALSE); });

    WNDCLASSW wc{};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInst;
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    wc.lpszClassName = L"AppClass";
    RegisterClassW(&wc);

    HWND hwnd = CreateWindowExW(
        0,
        L"AppClass",
        L"DS Mouse Input Fix",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        nullptr,
        nullptr,
        hInst,
        nullptr);
    g_State.hwndMain = hwnd;

    g_State.isJapanese = _wsetlocale(LC_ALL, L"") && std::wcsstr(_wsetlocale(LC_ALL, nullptr), L"Japanese");

    ConfigManager::LoadConfig();
    PaintBackground(hwnd, RGB(40, 49, 117));

    HFONT hFont = CreateFontW(
        -12, 0, 0, 0,
        FW_NORMAL,
        0, 0, 0,
        DEFAULT_CHARSET,
        0, 0,
        DEFAULT_QUALITY,
        0,
        g_State.isJapanese ? L"Meiryo UI" : L"Segoe UI");

    HWND hStart = CreateWindowW(
        L"BUTTON",
        L"Start",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        10, 10, 365, 30,
        hwnd,
        reinterpret_cast<HMENU>(START_BUTTON_ID),
        nullptr,
        nullptr);

    HWND hDrop = CreateWindowW(
        L"COMBOBOX",
        L"",
        CBS_DROPDOWNLIST | WS_CHILD | WS_VISIBLE,
        10, 50, 365, 500,
        hwnd,
        reinterpret_cast<HMENU>(DROPDOWN_ID),
        hInst,
        nullptr);
    SendMessageW(hDrop, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Metroid Prime Hunters"));
    SendMessageW(hDrop, CB_SETCURSEL, 0, 0);

    const wchar_t* msg = g_State.isJapanese
        ? L"「Start」で開始。2.5秒後に画面が赤くなる。\nそのタイミングでDS下画面の左下をクリックして、次に右上をクリックする。これでカーソルの範囲を下画面に制限できる。\nバックスペースキーで終了。右SHIFTキーで一時停止／再開できる。"
        : L"Press 'Start' to begin.\nAfter 2.5 seconds this window turns red.\nClick bottom-left, then top-right to clip cursor.\nBACKSPACE: kill, RIGHT SHIFT: pause/resume.";
    HWND hStatic = CreateWindowW(
        L"STATIC",
        msg,
        WS_CHILD | WS_VISIBLE,
        10, 80, 365, 140,
        hwnd,
        reinterpret_cast<HMENU>(3),
        hInst,
        nullptr);

    g_State.hCheckUseSaved = CreateWindowW(
        L"BUTTON",
        g_State.isJapanese ? L"保存済み範囲を使用（次回クリップ省略）" : L"Use saved clip",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX,
        10, 230, 365, 20,
        hwnd,
        reinterpret_cast<HMENU>(CHECK_USE_SAVED_ID),
        nullptr,
        nullptr);

    g_State.hLabelSaved = CreateWindowW(
        L"STATIC",
        L"",
        WS_CHILD | WS_VISIBLE,
        10, 255, 365, 20,
        hwnd,
        reinterpret_cast<HMENU>(STATIC_SAVED_LABEL_ID),
        nullptr,
        nullptr);

    for (HWND h : {hwnd, hStart, hDrop, hStatic, g_State.hCheckUseSaved, g_State.hLabelSaved}) {
        SendMessageW(h, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), TRUE);
    }

    UpdateSavedClipLabel();
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);
    if (HWND cw = GetConsoleWindow()) {
        ShowWindow(cw, SW_HIDE);
    }

    MSG m{};
    while (GetMessageW(&m, nullptr, 0, 0) > 0) {
        TranslateMessage(&m);
        DispatchMessageW(&m);
    }

    if (hFont) {
        DeleteObject(hFont);
    }
    return 0;
}
