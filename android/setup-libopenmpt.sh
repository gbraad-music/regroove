#!/bin/bash
# Setup script for libopenmpt on Android
# This script helps download or build libopenmpt for Android

set -e

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
LIBOPENMPT_DIR="$SCRIPT_DIR/app/src/main/cpp/libopenmpt"

echo "=== LibOpenMPT Setup for Android ==="
echo ""
echo "LibOpenMPT is required for Regroove to play module files."
echo ""
echo "Options:"
echo "  1. Download prebuilt binaries (if available)"
echo "  2. Build from source"
echo "  3. Instructions only"
echo ""
read -p "Choose option (1/2/3): " option

case $option in
    1)
        echo ""
        echo "Prebuilt binaries:"
        echo "  Check https://lib.openmpt.org/libopenmpt/download/"
        echo "  or https://github.com/OpenMPT/openmpt/releases"
        echo ""
        echo "Download and extract to:"
        echo "  $LIBOPENMPT_DIR/arm64-v8a/libopenmpt.so"
        echo "  $LIBOPENMPT_DIR/armeabi-v7a/libopenmpt.so"
        echo "  $LIBOPENMPT_DIR/include/libopenmpt/"
        echo ""
        ;;
    2)
        echo ""
        echo "Building from source:"
        echo ""
        echo "1. Clone openmpt:"
        echo "   git clone https://github.com/OpenMPT/openmpt.git"
        echo ""
        echo "2. Build for Android (requires NDK):"
        echo "   cd openmpt/build/android"
        echo "   ./build.sh"
        echo ""
        echo "3. Copy libraries to:"
        echo "   $LIBOPENMPT_DIR/arm64-v8a/libopenmpt.so"
        echo "   $LIBOPENMPT_DIR/armeabi-v7a/libopenmpt.so"
        echo "   $LIBOPENMPT_DIR/include/libopenmpt/"
        echo ""
        ;;
    3)
        echo ""
        echo "Manual setup instructions:"
        echo ""
        echo "Create this directory structure:"
        echo "  $LIBOPENMPT_DIR/"
        echo "    arm64-v8a/"
        echo "      libopenmpt.so"
        echo "    armeabi-v7a/"
        echo "      libopenmpt.so"
        echo "    include/"
        echo "      libopenmpt/"
        echo "        libopenmpt.h"
        echo "        libopenmpt.hpp"
        echo ""
        echo "Resources:"
        echo "  - Official site: https://lib.openmpt.org/libopenmpt/"
        echo "  - GitHub: https://github.com/OpenMPT/openmpt"
        echo "  - Docs: https://lib.openmpt.org/libopenmpt/md__home_manx__documents__web_site_2libopenmpt_2doc_2_readme.html"
        echo ""
        ;;
    *)
        echo "Invalid option"
        exit 1
        ;;
esac

echo ""
echo "After setting up libopenmpt, build with:"
echo "  cd $SCRIPT_DIR"
echo "  gradle assembleDebug"
echo ""
