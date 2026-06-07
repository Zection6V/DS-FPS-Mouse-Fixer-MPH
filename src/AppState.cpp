#include "AppState.h"

AppConfig g_Config;
AppState g_State;
std::atomic<bool> g_PhysicalKeys[256];
std::atomic<bool> g_PhysicalKeysHit[256];
std::unordered_map<std::string, int> g_Keybinds;
