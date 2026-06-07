#include "ConfigManager.h"

#include "AppConstants.h"
#include "AppState.h"
#include "InputMapping.h"

#include <cstdio>
#include <fstream>
#include <string>

namespace ConfigManager {
    namespace {
        std::string Trim(const std::string& str) {
            size_t f = str.find_first_not_of(" \t\r\n");
            size_t l = str.find_last_not_of(" \t\r\n");
            return (f == std::string::npos) ? "" : str.substr(f, l - f + 1);
        }

        std::string RemoveQuotes(const std::string& str) {
            return (str.length() >= 2 && str.front() == '"' && str.back() == '"')
                ? str.substr(1, str.length() - 2)
                : str;
        }

        int ParseInt(const std::string& str) {
            try {
                return (str.find("0x") == 0 || str.find("0X") == 0)
                    ? std::stoi(str, nullptr, 16)
                    : std::stoi(str);
            } catch (...) {
                return 0;
            }
        }
    }

    bool SaveConfig() {
        std::ofstream out(kConfigFile);
        if (!out) {
            return false;
        }

        out << "[ClipRegion]\nleft = " << g_Config.leftBound
            << "\ntop = " << g_Config.topBound
            << "\nright = " << g_Config.rightBound
            << "\nbottom = " << g_Config.bottomBound
            << "\n\n[Config]\nmouseResetWait = " << g_Config.mouseResetWait
            << "\nbuttonWait = " << g_Config.buttonWait
            << "\nswapWait = " << g_Config.swapWait
            << "\nkeyWait = " << g_Config.keyWait
            << "\nautoMouseDrag = " << g_Config.autoMouseDrag
            << "\nboostInterval = " << g_Config.boostInterval
            << "\nboostSwipePercentVertical = " << g_Config.boostSwipePercentVertical
            << "\nboostSwipePercentHorizontal = " << g_Config.boostSwipePercentHorizontal
            << "\nboostSwipePercentDiagonal = " << g_Config.boostSwipePercentDiagonal
            << "\nboostOffsetXPercent = " << g_Config.boostOffsetXPercent
            << "\nboostOffsetYPercent = " << g_Config.boostOffsetYPercent
            << "\nboostTriggerDelayMs = " << g_Config.boostTriggerDelayMs
            << "\nboostTriggerHoldMs = " << g_Config.boostTriggerHoldMs
            << "\nboostTriggerIntervalMs = " << g_Config.boostTriggerIntervalMs
            << "\n\n[Keybinds]\n";

        for (const auto& mapping : g_Mappings) {
            char hex[16];
            int vk = g_Keybinds.count(mapping.keyName)
                ? g_Keybinds[mapping.keyName]
                : mapping.defaultVk;
            std::snprintf(hex, sizeof(hex), "0x%02X", vk);
            out << "\"" << mapping.keyName << "\" = " << hex << "\n";
        }

        return true;
    }

    void LoadConfig() {
        std::ifstream in(kConfigFile);
        if (!in) {
            SaveConfig();
            return;
        }

        std::string line;
        std::string sec;
        while (std::getline(in, line)) {
            line = Trim(line);
            if (line.empty() || line[0] == '#' || line.find("//") == 0) {
                continue;
            }
            if (line.front() == '[' && line.back() == ']') {
                sec = line.substr(1, line.length() - 2);
                continue;
            }

            size_t eq = line.find('=');
            if (eq == std::string::npos) {
                continue;
            }

            std::string k = RemoveQuotes(Trim(line.substr(0, eq)));
            std::string v = Trim(line.substr(eq + 1));
            if (sec == "Keybinds") {
                g_Keybinds[k] = ParseInt(v);
            } else if (k == "left") {
                g_Config.leftBound = ParseInt(v);
            } else if (k == "top") {
                g_Config.topBound = ParseInt(v);
            } else if (k == "right") {
                g_Config.rightBound = ParseInt(v);
            } else if (k == "bottom") {
                g_Config.bottomBound = ParseInt(v);
            } else if (k == "mouseResetWait") {
                g_Config.mouseResetWait = ParseInt(v);
            } else if (k == "buttonWait") {
                g_Config.buttonWait = ParseInt(v);
            } else if (k == "swapWait") {
                g_Config.swapWait = ParseInt(v);
            } else if (k == "keyWait") {
                g_Config.keyWait = ParseInt(v);
            } else if (k == "autoMouseDrag") {
                g_Config.autoMouseDrag = ParseInt(v);
            } else if (k == "boostInterval") {
                g_Config.boostInterval = ParseInt(v);
            } else if (k == "boostSwipePercentVertical") {
                g_Config.boostSwipePercentVertical = ParseInt(v);
            } else if (k == "boostSwipePercentHorizontal") {
                g_Config.boostSwipePercentHorizontal = ParseInt(v);
            } else if (k == "boostSwipePercentDiagonal") {
                g_Config.boostSwipePercentDiagonal = ParseInt(v);
            } else if (k == "boostOffsetXPercent") {
                g_Config.boostOffsetXPercent = ParseInt(v);
            } else if (k == "boostOffsetYPercent") {
                g_Config.boostOffsetYPercent = ParseInt(v);
            } else if (k == "boostTriggerDelayMs") {
                g_Config.boostTriggerDelayMs = ParseInt(v);
            } else if (k == "boostTriggerHoldMs") {
                g_Config.boostTriggerHoldMs = ParseInt(v);
            } else if (k == "boostTriggerIntervalMs") {
                g_Config.boostTriggerIntervalMs = ParseInt(v);
            }
        }
    }
}
