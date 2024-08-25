/*
目的: このファイルには、メトロイドプライムハンターズのファイルハンドラーと入力ジェネレーターが含まれている.
*/

extern int mouseResetWait, buttonWait, swapWait, keyWait;

extern int totalInputs;
extern int gameSelected;

void ScreenTapJump()
{
    // カーソルを上げる
    CursorUp();
    // 一時停止（マウスリセット待機時間）
    Sleep(mouseResetWait);
    // カーソルを中心に設定
    SetCursorPos(center.x, center.y);
    // マウスのダウンイベント
    mouse_event(mouseDown, 0, 0, 0, 0);
    // アクティブなマウスダウンを取得
    GetActiveMouseDown(&buttonWait);
    // カーソルを上げる
    CursorUp();
    // マウスのダウンイベント
    mouse_event(mouseDown, 0, 0, 0, 0);
}

// メインループ用の入力構造体リストを生成
void GenerateInputs_MPH(FILE *inputFile, struct Input* inputList)
{
    int i = 0;
    int hexVal;
    char line[100];
    // 各行を読み込む
    /*
    各行について:
    割り当てるコマンドを識別
    コマンド宣言のサブ文字列を削除（例: "Shoot = "）
    次に、16進値をスキャン
    指定されたコマンドと16進値を使用して、入力構造体リストに新しい入力構造体を作成
    */
    struct InputMapping {
        const char* key;
        void (*gameFunction)();
    };

    InputMapping inputMappings[] = {
        {"Shoot", Shoot_Button_Down},
        {"Mouse Manual Reset", ResetPos},
        {"Swap To Main Weapon", AmmoTypeOne_MPH},
        {"Swap To Missiles", AmmoTypeTwo_MPH},
        {"Swap To Third Weapon", AmmoTypeThree_MPH},
        {"Zoom", Zoom_MPH},
        {"Special Weapon 1", WeaponOne_MPH},
        {"Special Weapon 2", WeaponTwo_MPH},
        {"Special Weapon 3", WeaponThree_MPH},
        {"Special Weapon 4", WeaponFour_MPH},
        {"Special Weapon 5", WeaponFive_MPH},
        {"Special Weapon 6", WeaponSix_MPH},
        {"Scan Vision", ScanVision_MPH},
        {"Ball", Ball_MPH},
        {"Screen Tap Jump", ScreenTapJump},
        {"Ok (menu)", ClickOK_MPH},
        {"Yes (menu)", ClickYes_MPH},
        {"No (menu)", ClickNo_MPH},
        {"Left (menu)", Left_MPH},
        {"Right (menu)", Right_MPH},
        {nullptr, nullptr}  // 終端マーカー
    };

    while (fgets(line, sizeof(line), inputFile) != nullptr)
    {
        line[strcspn(line, "\n")] = '\0';  // 改行文字を削除

        for (int j = 0; inputMappings[j].key != nullptr; j++)
        {
            char searchStr[100];
            snprintf(searchStr, sizeof(searchStr), "%s = ", inputMappings[j].key);

            if (strstr(line, searchStr) != nullptr)
            {
                removeSubstring(line, searchStr);
                sscanf(line, "%x", &hexVal);

                inputList[i].isDown = 0;
                inputList[i].gameFunction = inputMappings[j].gameFunction;
                inputList[i].inpNum = hexVal;
                i++;
                break;
            }
        }
    }
}

// メトロイドプライムハンターズの入力ファイルを読み取る・作成する関数
struct Input* MPH_Input_Reader()
{
    totalInputs = 0;

    FILE* MPHInputs = fopen("DS_MPH_Controls.txt", "r");
    if (MPHInputs == nullptr)
    {
        // ファイルが見つからなかった場合、新しいMPHInputsを作成
        MPHInputs = fopen("DS_MPH_Controls.txt", "w");

        if (MPHInputs != nullptr)
        {
            // ファイルヘッダーの書き込み
            const char* header =
                "Edit Controls for Metroid Prime Hunters in this file.\n"
                "Inputs must be denoted by their corresponding hex values as denoted here: \n"
                "https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes \n\n";
            fprintf(MPHInputs, "%s", header);

            // 入力設定の定義
            const char* inputs[] = {
                "Shoot = 0x01",
                "Mouse Manual Reset = 0x04",
                "Swap To Main Weapon = 0x51",
                "Swap To Missiles = 0x45",
                "Swap To Third Weapon = 0x52",
                "Special Weapon 1 = 0x31",
                "Special Weapon 2 = 0x32",
                "Special Weapon 3 = 0x33",
                "Special Weapon 4 = 0x34",
                "Special Weapon 5 = 0x35",
                "Special Weapon 6 = 0x36",
                "Scan Vision = 0x56",
                "Ball = 0xA2",
                "Screen Tap Jump = 0xA3",
                "Zoom = 0x02",
                "Ok (menu) = 0x46",
                "Yes (menu) = 0x05",
                "No (menu) = 0x06",
                "Left (menu) = 0x5A",
                "Right (menu) = 0x58"
            };

            // 入力設定をファイルに書き込み
            for (const char* input : inputs) {
                fprintf(MPHInputs, "%s\n", input);
            }

            // ファイルを閉じる
            fclose(MPHInputs);

            // 総入力数を計算
            totalInputs = sizeof(inputs) / sizeof(inputs[0]);

            // 入力ファイルを読み込みモードで開く
            FILE* MPHInputs = fopen("DS_MPH_Controls.txt", "r");

            // 入力リストのメモリを確保
            Input* inputList = new Input[totalInputs];

            // 入力を生成
            GenerateInputs_MPH(MPHInputs, inputList);

            // ファイルを閉じる
            fclose(MPHInputs);

            // 入力リストを返す
            return inputList;
        }
        else
        {
            // ファイル作成エラーのメッセージ
            // printf("Error creating MPHInputs");
        }
    }
    else
    {
        // printf("File opened successfully\n");
        // 必要に応じてMPHInputsからデータを読み取る
        char line[100];
        // テキストファイルを行ごとに読み取り、入力構造体リストに必要な入力の数をカウント
        while (fgets(line, sizeof(line), MPHInputs) != nullptr)
        {
            // 検索する入力文字列の配列
            const char* inputStrings[] = {
                "Shoot = ",
                "Mouse Manual Reset = ",
                "Swap To Main Weapon = ",
                "Swap To Missiles = ",
                "Swap To Third Weapon = ",
                "Special Weapon 1 = ",
                "Special Weapon 2 = ",
                "Special Weapon 3 = ",
                "Special Weapon 4 = ",
                "Special Weapon 5 = ",
                "Special Weapon 6 = ",
                "Scan Vision = ",
                "Ball = ",
                "Screen Tap Jump = ",
                "Zoom = ",
                "Ok (menu) = ",
                "Yes (menu) = ",
                "No (menu) = ",
                "Left (menu) = ",
                "Right (menu) = "
            };

            // 行の終端文字を削除
            line[strcspn(line, "\n")] = '\0';

            // 入力文字列を検索
            bool inputFound = false;
            for (const char* inputStr : inputStrings) {
                if (strstr(line, inputStr) != nullptr) {
                    inputFound = true;
                    break;
                }
            }

            if (inputFound) {
                totalInputs++;
            }
        }
        // 入力をカウントした後、その数の要素を持つリストを作成
         Input* inputList = new Input[totalInputs];

        // ファイルの読み取りを再開
        fseek(MPHInputs, 0, SEEK_SET);

        // 入力を生成
        GenerateInputs_MPH(MPHInputs, inputList);

        // MPHInputsを閉じる
        fclose(MPHInputs);

        // 入力リストを返す
        return inputList;
    }
    return nullptr;
}
