# 🔧 Build & Run Instructions (MinGW64)

## 1. Move to the project directory
cd /your/path/to/project/Source

## 2. Configure build environment (only once)
# Run this only once to set up the build environment
cmake -S . -B build -G "Ninja" -DCMAKE_BUILD_TYPE=Release

## 3. Build (for subsequent builds, just run this)
cmake --build build -j

# 👉 After success, the executable will be generated at  
# /your/path/to/project/Source/build/bin

## 4. Clean build (remove existing outputs and rebuild from scratch)
cd /your/path/to/project/Source
rm -rf build
cmake -S . -B build -G "Ninja" -DCMAKE_BUILD_TYPE=Release
cmake --build build -j

## 5. Run & Debug
cd /your/path/to/project/Source/build/bin
./DS_FPS_Mouse_Fixer.exe
