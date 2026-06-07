# 近年の最適化を安定利用するための最低バージョン指定
cmake_minimum_required(VERSION 3.24)

# プロジェクト名と使用言語
project(DS_FPS_Mouse_Fixer VERSION 1.4 LANGUAGES CXX)

# C++標準
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 実行ファイル生成
add_executable(DS_FPS_Mouse_Fixer
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AppState.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ConfigManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/GameLoop.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/InputMapping.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/InputUtils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MacroActions.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/RawInputHandler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Ui.cpp
)

target_include_directories(DS_FPS_Mouse_Fixer
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# 共通のコンパイル定義
target_compile_definitions(DS_FPS_Mouse_Fixer
    PRIVATE
        WIN32_LEAN_AND_MEAN
        NOMINMAX
        _CRT_SECURE_NO_WARNINGS
)

# コンパイル最適化フラグ
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")

# 静的リンクのためのフラグを設定
set(CMAKE_EXE_LINKER_FLAGS "-static")

# 静的ランタイムを使用
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -static-libgcc -static-libstdc++")
set(CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG}   -static-libgcc -static-libstdc++")

# Windows GUIアプリとしてビルド (-mwindows を付与)
set_target_properties(DS_FPS_Mouse_Fixer PROPERTIES
    LINK_FLAGS "-mwindows"
    LINK_SEARCH_START_STATIC 1
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
