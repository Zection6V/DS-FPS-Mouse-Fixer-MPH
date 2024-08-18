/*
目的: このファイルには、UIで選択されたゲームに応じて他のファイルハンドラ（入力リーダー）を呼び出すメインファイルハンドラが含まれている.
*/

#include "GameFunctions.cpp"

// 外部変数の宣言（他のファイルで定義されている）
int totalInputs;
extern int gameSelected;

#include "Config_File_Handler.cpp"
#include "MPH_File_Handler.cpp"

// ファイルハンドラ関数（ゲームに応じて入力リーダーを呼び出す）
struct Input* FileHandle()
{
  // コンフィグリーダーを実行（設定ファイルを読み取るため）
  ConfigReader();

  // 選択されたゲームに応じてMPH入力リーダーを実行
  return MPH_Input_Reader();
}
