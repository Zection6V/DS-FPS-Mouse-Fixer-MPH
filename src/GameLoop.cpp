#include "GameLoop.h"

#include "AppState.h"
#include "ConfigManager.h"
#include "InputUtils.h"
#include "MacroActions.h"
#include "RawInputHandler.h"

#include <sstream>

bool IsClipValid() {
    return g_Config.leftBound >= 0 && g_Config.topBound >= 0 &&
           g_Config.rightBound > g_Config.leftBound + 10 &&
           g_Config.bottomBound > g_Config.topBound + 10 &&
           g_Config.rightBound <= GetSystemMetrics(SM_CXSCREEN) &&
           g_Config.bottomBound <= GetSystemMetrics(SM_CYSCREEN);
}

void ComputeDerivedFromBounds() {
    SetRect(&g_State.playSpace, g_Config.leftBound, g_Config.topBound, g_Config.rightBound, g_Config.bottomBound);
    g_State.screenWidthRatio = static_cast<float>(g_Config.rightBound - g_Config.leftBound) / g_State.referenceWidth;
    g_State.screenHeightRatio = static_cast<float>(g_Config.bottomBound - g_Config.topBound) / g_State.referenceHeight;
    g_State.distanceFromBoarderSides = 50.0f * g_State.screenWidthRatio;
    g_State.distanceFromBoarderTop = 100.0f * g_State.screenHeightRatio;
    g_State.center = {
        ((g_Config.rightBound - g_Config.leftBound) / 2) + g_Config.leftBound,
        ((g_Config.bottomBound - g_Config.topBound) / 2) + g_Config.topBound,
    };
}

void UpdateSavedClipLabel() {
    if (!g_State.hLabelSaved) {
        return;
    }

    std::wstringstream wss;
    if (IsClipValid()) {
        wss << (g_State.isJapanese ? L"保存済み範囲: (" : L"Saved clip: (")
            << g_Config.leftBound << L"," << g_Config.topBound << L") - ("
            << g_Config.rightBound << L"," << g_Config.bottomBound << L")";
    } else {
        wss << (g_State.isJapanese ? L"保存済み範囲: なし" : L"Saved clip: none");
    }

    SetWindowTextW(g_State.hLabelSaved, wss.str().c_str());
    if (g_State.hCheckUseSaved) {
        SendMessageW(g_State.hCheckUseSaved, BM_SETCHECK, IsClipValid() ? BST_CHECKED : BST_UNCHECKED, 0);
    }
}

int InitializePoints(POINT* bl, POINT* tr) {
    int init = 0;
    bool click = false;
    while (init <= 1) {
        if (GetAsyncKeyState(VK_LBUTTON) & 0x8000) {
            if (!click) {
                click = true;
                if (init++ == 0) {
                    GetCursorPos(bl);
                } else {
                    GetCursorPos(tr);
                    Sleep(100);
                    g_Config.leftBound = bl->x + 1;
                    g_Config.rightBound = tr->x - 1;
                    g_Config.topBound = tr->y + 1;
                    g_Config.bottomBound = bl->y - 1;
                    ComputeDerivedFromBounds();
                    InputUtils::ResetPos();
                    ConfigManager::SaveConfig();
                    UpdateSavedClipLabel();
                    return 1;
                }
            }
        } else {
            click = false;
        }
        Sleep(1);
    }
    return 0;
}

void UpdateSystemKeys(InputState stateCheckers[2]) {
    for (int i = 0; i < 2; ++i) {
        auto& s = stateCheckers[i];
        bool physicalDown = g_PhysicalKeys[s.inpNum];
        bool physicalHit = g_PhysicalKeysHit[s.inpNum].exchange(false);
        bool down = physicalDown || physicalHit;

        if (down && !s.isDown) {
            if (s.gameFunction) {
                s.gameFunction();
            }
        }
        s.isDown = down;
    }
}

void ProcessTriggerMacros(InputState* inputList, std::chrono::steady_clock::time_point now, bool& anyTriggerActive, bool& anyTriggerWantM) {
    for (std::size_t i = 0; i < NUM_MAPPINGS; i++) {
        auto& in = inputList[i];
        if (in.inpNum == 0 || in.type != MacroType::TriggerLoop) {
            continue;
        }

        bool physicalDown = g_PhysicalKeys[in.inpNum];
        bool physicalHit = g_PhysicalKeysHit[in.inpNum].exchange(false);
        bool down = physicalDown || physicalHit;

        if (!physicalDown && in.physicalWasDown) {
            in.releaseTime = now;
        }
        in.physicalWasDown = physicalDown;

        if (!physicalDown && std::chrono::duration_cast<std::chrono::milliseconds>(now - in.releaseTime).count() < 150) {
            down = true;
        }

        if (down) {
            if (in.state == 0) {
                in.state = (g_Config.boostTriggerDelayMs > 0) ? 3 : 1;
                in.lastTime = now;
            } else if (in.state == 3) {
                if (std::chrono::duration_cast<std::chrono::milliseconds>(now - in.lastTime).count() >= g_Config.boostTriggerDelayMs) {
                    in.state = 1;
                    in.lastTime = now;
                }
            } else if (in.state == 1) {
                if (std::chrono::duration_cast<std::chrono::milliseconds>(now - in.lastTime).count() >= g_Config.boostTriggerHoldMs) {
                    in.state = 2;
                    in.lastTime = now;
                }
            } else if (in.state == 2) {
                if (std::chrono::duration_cast<std::chrono::milliseconds>(now - in.lastTime).count() >= g_Config.boostTriggerIntervalMs) {
                    in.state = 1;
                    in.lastTime = now;
                }
            }
        } else {
            in.state = 0;
        }

        if (in.state != 0) {
            anyTriggerActive = true;
        }
        if (in.state == 1) {
            anyTriggerWantM = true;
        }

        in.isDown = down;
    }
}

void ProcessStandardMacros(InputState* inputList, std::chrono::steady_clock::time_point now, bool anyTriggerActive) {
    (void)anyTriggerActive;

    for (std::size_t i = 0; i < NUM_MAPPINGS; i++) {
        auto& in = inputList[i];
        if (in.inpNum == 0 || in.type == MacroType::TriggerLoop) {
            continue;
        }

        if (in.type == MacroType::Continuous) {
            bool physicalDown = g_PhysicalKeys[in.inpNum];
            bool physicalHit = g_PhysicalKeysHit[in.inpNum].exchange(false);
            bool down = physicalDown || physicalHit;
            if (down) {
                if (!in.isDown || std::chrono::duration_cast<std::chrono::milliseconds>(now - in.lastTime).count() >= g_Config.boostInterval) {
                    if (in.gameFunction) {
                        in.gameFunction();
                    }
                    in.lastTime = now;
                }
            }
            in.isDown = down;
        } else {
            SHORT keyState = GetAsyncKeyState(in.inpNum);
            bool isKeyDown = (keyState & 0x8000) != 0;

            if (in.inpNum == 0x02) {
                if (!isKeyDown && in.isDown) {
                    if (g_Config.autoMouseDrag) {
                        InputUtils::RightDown();
                    }

                    g_State.zoomPulseEndTime = now + std::chrono::milliseconds(g_Config.keyWait);
                    g_State.zoomPulseActive = true;
                    InputUtils::SafeSendMKey(true);
                }
            } else {
                if (isKeyDown && !in.isDown) {
                    if (in.gameFunction) {
                        in.gameFunction();
                    }
                }
            }
            in.isDown = isKeyDown;
        }
    }
}

int Run() {
    RAWINPUTDEVICE rid[2] = {
        {0x01, 0x06, RIDEV_INPUTSINK, g_State.hwndMain},
        {0x01, 0x02, RIDEV_INPUTSINK, g_State.hwndMain},
    };
    if (!RegisterRawInputDevices(rid, 2, sizeof(RAWINPUTDEVICE))) {
        return 1;
    }

    for (int i = 0; i < 256; i++) {
        g_PhysicalKeys[i] = false;
        g_PhysicalKeysHit[i] = false;
    }

    MacroActions::StartWorker();

    auto nowInit = std::chrono::steady_clock::now();
    auto pastInit = nowInit - std::chrono::milliseconds(1000);

    InputState stateCheckers[2] = {
        {0, MacroActions::Pause, VK_RSHIFT, MacroType::Single, nowInit, 0, pastInit, 0},
        {0, MacroActions::Kill, VK_BACK, MacroType::Single, nowInit, 0, pastInit, 0},
    };

    InputState inputList[NUM_MAPPINGS];
    for (std::size_t i = 0; i < NUM_MAPPINGS; i++) {
        const auto& mapping = g_Mappings[i];
        inputList[i] = {
            0,
            mapping.gameFunction ? mapping.gameFunction : MacroActions::DoNothing,
            g_Keybinds.count(mapping.keyName) ? g_Keybinds[mapping.keyName] : mapping.defaultVk,
            mapping.type,
            nowInit,
            0,
            pastInit,
            0,
        };
    }

    if (!(g_State.useSavedClipAtStart && IsClipValid())) {
        POINT bl{};
        POINT tr{};
        InitializePoints(&bl, &tr);
    } else {
        ComputeDerivedFromBounds();
        InputUtils::ResetPos();
    }

    SwapMouseButton(TRUE);
    g_State.zoomPulseActive = false;

    while (g_State.isOn) {
        MSG msg;
        while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_INPUT) {
                ProcessRawInput(msg.lParam);
            }
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }

        auto now = std::chrono::steady_clock::now();

        UpdateSystemKeys(stateCheckers);

        if (g_State.isPaused) {
            for (auto& in : inputList) {
                in.state = 0;
            }
            InputUtils::SafeSendMKey(false);
            g_State.zoomPulseActive = false;
            Sleep(10);
            continue;
        }

        POINT c;
        GetCursorPos(&c);
        if (g_Config.autoMouseDrag &&
            (c.x >= g_State.playSpace.right - 1 || c.x <= g_State.playSpace.left ||
             c.y >= g_State.playSpace.bottom - 1 || c.y <= g_State.playSpace.top)) {
            InputUtils::ResetPos();
        }

        bool anyTriggerActive = false;
        bool anyTriggerWantM = false;

        ProcessTriggerMacros(inputList, now, anyTriggerActive, anyTriggerWantM);
        ProcessStandardMacros(inputList, now, anyTriggerActive);

        if (g_State.zoomPulseActive && now >= g_State.zoomPulseEndTime) {
            g_State.zoomPulseActive = false;
        }

        bool holdCKey = g_PhysicalKeys['C'];
        InputUtils::SafeSendMKey(anyTriggerWantM || g_State.zoomPulseActive || holdCKey);

#ifdef YieldProcessor
        YieldProcessor();
#else
        Sleep(0);
#endif
    }

    SwapMouseButton(FALSE);
    InputUtils::SendMouseButton(MOUSEEVENTF_RIGHTUP | MOUSEEVENTF_LEFTUP);

    MacroActions::StopWorker();
    return 0;
}
