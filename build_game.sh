#!/bin/bash
set -e

echo "==================================================="
echo "  Gality Engine - macOS/Linux Build Script"
echo "==================================================="

# 1. 編譯 .gality DSL
echo "[1/5] Compiling .gality DSL script..."
python3 -m devtools.scripts.gality_compiler \
    assets/scripts/main_story_multi.gality \
    assets/scripts/demo_long.json

# 2. 驗證編譯後的腳本
echo "[2/5] Validating compiled script..."
if ! python3 -m devtools.scripts.validate_script assets/scripts/demo_long.json; then
    echo ""
    echo "[ERROR] Script validation failed!"
    echo "        Fix the issues above before building."
    exit 1
fi

# 3. 打包 assets
echo "[3/5] Packing assets into data.pak..."
python3 -m devtools.scripts.gality_packer assets data.pak

# 4. 建置 C++ 專案
echo "[4/5] Building C++ Engine (Release Mode)..."
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release

# 5. 建立發布資料夾
echo "[5/5] Generating Release Package..."
mkdir -p dist
cp data.pak dist/
cp build/gality dist/

echo "==================================================="
echo "SUCCESS: Release package generated in ./dist"
echo "Run: cd dist && ./gality"
echo "==================================================="