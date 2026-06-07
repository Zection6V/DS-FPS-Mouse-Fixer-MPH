#pragma once

#include "Platform.h"

#include <atomic>
#include <chrono>
#include <string>
#include <unordered_map>

struct AppConfig {
    int mouseResetWait = 33;
    int buttonWait = 33;
    int swapWait = 220;
    int keyWait = 30;
    int autoMouseDrag = 1;
    int boostInterval = 250;
    int boostSwipePercentVertical = 50;
    int boostSwipePercentHorizontal = 60;
    int boostSwipePercentDiagonal = 45;
    int boostOffsetXPercent = 0;
    int boostOffsetYPercent = 5;
    int boostTriggerDelayMs = 250;
    int boostTriggerHoldMs = 500;
    int boostTriggerIntervalMs = 250;

    int leftBound = 0;
    int rightBound = 0;
    int topBound = 0;
    int bottomBound = 0;
};

struct AppState {
    std::atomic<bool> isRunning{true};
    int isOn = 1;
    int isPaused = 0;
    int gameSelected = 7;

    POINT center{};
    RECT playSpace{};
    RECT windowRect{};

    float screenWidthRatio = 1.0f;
    float screenHeightRatio = 1.0f;
    float distanceFromBoarderSides = 50.0f;
    float distanceFromBoarderTop = 100.0f;
    float referenceWidth = 587.0f;
    float referenceHeight = 441.0f;

    std::atomic<bool> leftButtonDown{false};
    std::atomic<bool> isMKeyPressed{false};
    std::atomic<bool> isSwiping{false};

    std::chrono::steady_clock::time_point zoomPulseEndTime;
    bool zoomPulseActive = false;

    bool useSavedClipAtStart = false;
    bool isJapanese = false;

    HWND hwndMain = nullptr;
    HWND hCheckUseSaved = nullptr;
    HWND hLabelSaved = nullptr;
};

extern AppConfig g_Config;
extern AppState g_State;
extern std::atomic<bool> g_PhysicalKeys[256];
extern std::atomic<bool> g_PhysicalKeysHit[256];
extern std::unordered_map<std::string, int> g_Keybinds;
