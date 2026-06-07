#pragma once

namespace MacroActions {
    void DoNothing();
    void Pause();
    void Kill();
    void Tap(int x, int y, int delayMs = 0);
    void Drag(int sx, int sy, int ex, int ey);
    void HoldTap(int x, int y);
    void StartWorker();
    void StopWorker();
    void BoostBall_MPH();
}
