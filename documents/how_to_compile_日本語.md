# 🔧 コンパイル・実行手順まとめ (MinGW64)

## 1. 作業ディレクトリへ移動
cd /your/path/to/project/Source

## 2. 初回のみ（ビルド環境の構築）
# ※1回だけ実行すればOK
cmake -S . -B build -G "Ninja" -DCMAKE_BUILD_TYPE=Release

## 3. ビルド実行（2回目以降はこれだけで良い）
cmake --build build -j

# 👉 成功すると、実行ファイルが  
# /your/path/to/project/Source/build/bin  
# に生成される。

## 4. クリーンビルド（既存の成果物を削除して再構築したいとき）
cd /your/path/to/project/Source

rm -rf build

cmake -S . -B build -G "Ninja" -DCMAKE_BUILD_TYPE=Release

cmake --build build -j

## 5. 実行・デバッグ
cd /your/path/to/project/Source/build/bin

./DS_FPS_Mouse_Fixer.exe
