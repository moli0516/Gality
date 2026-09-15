#!/bin/bash
set -e

echo "==================================================="
echo "  Gality Engine - macOS Build Script"
echo "==================================================="

# 1. 編譯 .gality DSL
echo "[1/4] Compiling .gality DSL script..."
python3 -m devtools.scripts.gality_compiler \
    assets/scripts/main_story_multi.gality \
    assets/scripts/demo_long.json

# 2. 打包 assets
echo "[2/4] Packing assets into data.pak..."
python3 -m devtools.scripts.gality_packer assets data.pak

# 3. 建置 C++ 專案
echo "[3/4] Building C++ Engine (Release Mode)..."
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release

# 4. 建立發布資料夾
echo "[4/4] Generating Release Package..."
mkdir -p dist
cp data.pak dist/
cp build/gality dist/

# macOS 不需要複製 .dylib，因為 Homebrew 會處理
# 但若要獨立發布，需要複製 .dylib：
# cp $(brew --prefix sfml)/lib/*.dylib dist/

echo "==================================================="
echo "SUCCESS: Release package generated in ./dist"
echo "Run: cd dist && ./gality"
echo "==================================================="