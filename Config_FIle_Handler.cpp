/*
目的: このファイルには、このスクリプト全体のためのコンフィグリーダーが含まれている。ゲーム機能が動作する待機時間を読み取る.
*/

#include <cstring>
#include <string>
#include <fstream>
#include <unordered_map>

// 二つの文字列を取り、全体の文字列からサブ文字列を削除し、サブ文字列が削除された文字列を返す
void removeSubstring(char *string, const char *substring)
{
    // サブ文字列の位置を検索
    char *match = strstr(string, substring);
    // サブ文字列の長さを取得
    size_t subLen = strlen(substring);

    if (match != nullptr) {
        // サブ文字列の後ろにある文字を前に移動
        memmove(match, match + subLen, strlen(match + subLen) + 1);
    }
}

// コンフィグファイルの作成・読み取り、およびその使用方法の指示
void ConfigReader() {
    const std::string filename = "DS_Config.txt";
    std::ifstream inFile(filename);

    if (!inFile) {
        std::ofstream outFile(filename);
        outFile <<
            "VALUES\n\n"
            "mouseResetWait = 33\n"
            "buttonWait = 33\n"
            "swapWait = 220\n"
            "keyWait = 30\n"
            "autoMouseDrag = 1\n";
        outFile.close();
        inFile.open(filename);
    }

    std::unordered_map<std::string, int*> configMap;
    configMap["mouseResetWait"] = &mouseResetWait;
    configMap["buttonWait"] = &buttonWait;
    configMap["swapWait"] = &swapWait;
    configMap["keyWait"] = &keyWait;

    std::string line, key, value;
    size_t equalsPos;
    while (std::getline(inFile, line)) {
        if ((equalsPos = line.find(" = ")) != std::string::npos) {
            key = line.substr(0, equalsPos);
            value = line.substr(equalsPos + 3);

            auto it = configMap.find(key);
            if (it != configMap.end()) {
                // 値を整数に変換し、絶対値にする
                *(it->second) = std::abs(std::stoi(value));
            } else if (key == "autoMouseDrag") {
                autoMouseDrag = std::stoi(value);
            }
        }
    }
}