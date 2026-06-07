#pragma once

#include <array>
#include <chrono>
#include <cstddef>

enum class MacroType {
    Single,
    Continuous,
    TriggerLoop,
};

struct InputMappingDef {
    const char* keyName;
    void (*gameFunction)();
    int defaultVk;
    MacroType type;
};

inline constexpr std::size_t NUM_MAPPINGS = 22;
extern const std::array<InputMappingDef, NUM_MAPPINGS> g_Mappings;

struct InputState {
    int isDown;
    void (*gameFunction)();
    int inpNum;
    MacroType type;
    std::chrono::steady_clock::time_point lastTime;
    int state;
    std::chrono::steady_clock::time_point releaseTime;
    int physicalWasDown;
};
