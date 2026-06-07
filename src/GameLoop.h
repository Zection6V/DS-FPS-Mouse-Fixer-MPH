#pragma once

#include "InputMapping.h"
#include "Platform.h"

#include <chrono>

bool IsClipValid();
void ComputeDerivedFromBounds();
void UpdateSavedClipLabel();
int InitializePoints(POINT* bl, POINT* tr);
void UpdateSystemKeys(InputState stateCheckers[2]);
void ProcessTriggerMacros(InputState* inputList, std::chrono::steady_clock::time_point now, bool& anyTriggerActive, bool& anyTriggerWantM);
void ProcessStandardMacros(InputState* inputList, std::chrono::steady_clock::time_point now, bool anyTriggerActive);
int Run();
