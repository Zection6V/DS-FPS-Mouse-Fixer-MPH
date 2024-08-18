cmake_minimum_required(VERSION 3.28)
project(untitled5 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 26)

# 静的リンクのためのフラグを設定
set(CMAKE_EXE_LINKER_FLAGS "-static")

# 静的ランタイムを使用
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -static-libgcc -static-libstdc++")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -static-libgcc -static-libstdc++")

add_executable(untitled5 main.cpp)

# -mwindows オプションをつけなくてもコンパイルは可能だが、できあがったGUIアプリケーションをダブルクリックで実行するとウィンドウの背後にコマンドプロンプトが表示される状態になる。
set_target_properties(untitled5 PROPERTIES
        LINK_FLAGS "-mwindows"
        LINK_SEARCH_START_STATIC 1
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
