#include "MacroActions.h"

#include "AppState.h"
#include "InputUtils.h"

#include <algorithm>
#include <condition_variable>
#include <mutex>
#include <thread>

namespace MacroActions {
    namespace {
        std::mutex g_swipeMutex;
        std::condition_variable g_swipeCV;
        bool g_swipeRequested = false;
        bool g_workerRunning = false;
        std::thread g_swipeThread;

        void SwipeWorkerTask() {
            while (g_workerRunning) {
                std::unique_lock<std::mutex> lock(g_swipeMutex);
                g_swipeCV.wait(lock, [] { return g_swipeRequested || !g_workerRunning; });

                if (!g_workerRunning) {
                    break;
                }

                InputUtils::CursorUp();
                int dx = g_PhysicalKeys['D'] - g_PhysicalKeys['A'];
                int dy = g_PhysicalKeys['S'] - g_PhysicalKeys['W'];
                if (!dx && !dy) {
                    dy = -1;
                }

                int scrW = g_Config.rightBound - g_Config.leftBound;
                int scrH = g_Config.bottomBound - g_Config.topBound;
                int px = dx ? (dy ? g_Config.boostSwipePercentDiagonal : g_Config.boostSwipePercentHorizontal) : 0;
                int py = dy ? (dx ? g_Config.boostSwipePercentDiagonal : g_Config.boostSwipePercentVertical) : 0;

                int hx = (scrW * px / 200) * dx;
                int hy = (scrH * py / 200) * dy;
                int cx = g_State.center.x + (scrW * g_Config.boostOffsetXPercent / 100);
                int cy = g_State.center.y + (scrH * g_Config.boostOffsetYPercent / 100);

                int stX = std::clamp(cx - hx, g_Config.leftBound, g_Config.rightBound);
                int stY = std::clamp(cy - hy, g_Config.topBound, g_Config.bottomBound);
                int enX = std::clamp(cx + hx, g_Config.leftBound, g_Config.rightBound);
                int enY = std::clamp(cy + hy, g_Config.topBound, g_Config.bottomBound);

                SetCursorPos(stX, stY);
                Sleep(g_Config.mouseResetWait);
                InputUtils::RightDown();
                Sleep(g_Config.mouseResetWait);
                SetCursorPos(enX, enY);
                Sleep(g_Config.mouseResetWait);
                InputUtils::ResetPosAfterButton();

                g_swipeRequested = false;
                g_State.isSwiping = false;
            }
        }
    }

    void DoNothing() {}

    void Pause() {
        InputUtils::RightUp();
        SwapMouseButton(g_State.isPaused);
        InputUtils::SendShootKey(false);
        InputUtils::SafeSendMKey(false);
        g_State.zoomPulseActive = false;
        g_State.isPaused = !g_State.isPaused;
        if (g_State.isPaused) {
            ClipCursor(nullptr);
        } else {
            InputUtils::ResetPos();
        }
    }

    void Kill() {
        g_State.isOn = 0;
        g_State.isPaused = 1;
        InputUtils::RightUp();
        InputUtils::SendShootKey(false);
        InputUtils::SafeSendMKey(false);
        g_State.zoomPulseActive = false;
        SwapMouseButton(0);
        ClipCursor(nullptr);
    }

    void Tap(int x, int y, int delayMs) {
        InputUtils::CursorUp();
        SetCursorPos(x, y);
        InputUtils::RightDown();
        if (delayMs) {
            Sleep(delayMs);
        }
        InputUtils::ResetPosAfterButton();
    }

    void Drag(int sx, int sy, int ex, int ey) {
        InputUtils::CursorUp();
        SetCursorPos(sx, sy);
        InputUtils::RightDown();
        Sleep(g_Config.mouseResetWait);
        SetCursorPos(ex, ey);
        InputUtils::ResetPosAfterButton();
    }

    void HoldTap(int x, int y) {
        InputUtils::CursorUp();
        SetCursorPos(x, y);
        InputUtils::RightDown();
        Sleep(g_Config.mouseResetWait);
        InputUtils::ResetPosAfterButton();
    }

    void StartWorker() {
        g_workerRunning = true;
        g_swipeRequested = false;
        g_swipeThread = std::thread(SwipeWorkerTask);
    }

    void StopWorker() {
        {
            std::lock_guard<std::mutex> lock(g_swipeMutex);
            g_workerRunning = false;
            g_swipeRequested = true;
        }
        g_swipeCV.notify_all();
        if (g_swipeThread.joinable()) {
            g_swipeThread.join();
        }
    }

    void BoostBall_MPH() {
        if (g_State.isSwiping.exchange(true)) {
            return;
        }
        {
            std::lock_guard<std::mutex> lock(g_swipeMutex);
            g_swipeRequested = true;
        }
        g_swipeCV.notify_one();
    }
}
