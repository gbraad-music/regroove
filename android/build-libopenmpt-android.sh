#!/bin/bash
# Script to build libopenmpt for Android
# Based on build-libopenmpt-mingw.sh approach

set -e

# Configuration
LIBOPENMPT_VERSION="0.7.11"
LIBOPENMPT_URL="https://lib.openmpt.org/files/libopenmpt/src/libopenmpt-${LIBOPENMPT_VERSION}+release.autotools.tar.gz"
BUILD_DIR="build-libopenmpt-android"
ANDROID_NDK="${ANDROID_NDK_HOME:-$HOME/Android/Sdk/ndk-bundle}"
ANDROID_API=29
INSTALL_DIR="$(pwd)/app/src/main/cpp/libopenmpt"

# Check if Android NDK is installed
if [ ! -d "$ANDROID_NDK" ]; then
    echo "Error: Android NDK not found at $ANDROID_NDK"
    echo "Set ANDROID_NDK_HOME environment variable or install Android NDK"
    echo ""
    echo "Install with:"
    echo "  Android Studio -> SDK Manager -> SDK Tools -> NDK"
    echo "Or download from: https://developer.android.com/ndk/downloads"
    exit 1
fi

echo "=== Building libopenmpt for Android ==="
echo "Version: $LIBOPENMPT_VERSION"
echo "NDK: $ANDROID_NDK"
echo "Install to: $INSTALL_DIR"
echo ""

# Create build directory
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Download libopenmpt if not already downloaded
if [ ! -f "libopenmpt-${LIBOPENMPT_VERSION}+release.autotools.tar.gz" ]; then
    echo "Downloading libopenmpt ${LIBOPENMPT_VERSION}..."
    wget "$LIBOPENMPT_URL"
fi

# Function to build for specific architecture
build_for_arch() {
    local ARCH=$1
    local ABI=$2
    local TOOLCHAIN_PREFIX=$3

    echo ""
    echo "=== Building for $ABI ($ARCH) ==="
    echo ""

    # Extract fresh copy
    rm -rf "libopenmpt-${LIBOPENMPT_VERSION}+release.autotools-${ABI}"
    tar xzf "libopenmpt-${LIBOPENMPT_VERSION}+release.autotools.tar.gz"
    mv "libopenmpt-${LIBOPENMPT_VERSION}+release.autotools" "libopenmpt-${LIBOPENMPT_VERSION}+release.autotools-${ABI}"
    cd "libopenmpt-${LIBOPENMPT_VERSION}+release.autotools-${ABI}"

    # Set up toolchain paths
    TOOLCHAIN="$ANDROID_NDK/toolchains/llvm/prebuilt/linux-x86_64"
    export AR="$TOOLCHAIN/bin/llvm-ar"
    export CC="$TOOLCHAIN/bin/${TOOLCHAIN_PREFIX}${ANDROID_API}-clang"
    export CXX="$TOOLCHAIN/bin/${TOOLCHAIN_PREFIX}${ANDROID_API}-clang++"
    export LD="$TOOLCHAIN/bin/ld"
    export RANLIB="$TOOLCHAIN/bin/llvm-ranlib"
    export STRIP="$TOOLCHAIN/bin/llvm-strip"

    # Configure for Android
    ./configure \
        --host=${TOOLCHAIN_PREFIX%-} \
        --prefix="${INSTALL_DIR}/${ABI}" \
        --enable-shared \
        --disable-static \
        --disable-openmpt123 \
        --disable-examples \
        --disable-tests \
        --without-mpg123 \
        --without-vorbis \
        --without-vorbisfile \
        --without-portaudio \
        --without-portaudiocpp \
        --without-pulseaudio \
        --without-sndfile \
        --without-flac \
        CFLAGS="-O2 -fPIC" \
        CXXFLAGS="-O2 -fPIC -std=c++17"

    # Build
    make -j$(nproc)

    # Install
    make install

    # Copy just the .so to the right location
    mkdir -p "${INSTALL_DIR}/${ABI}"
    cp -v "bin/libopenmpt.so" "${INSTALL_DIR}/${ABI}/"

    cd ..

    echo ""
    echo "✓ Built for $ABI"
}

# Build for ARM64
build_for_arch "arm64" "arm64-v8a" "aarch64-linux-android"

# Build for ARM32
build_for_arch "arm" "armeabi-v7a" "armv7a-linux-androideabi"

# Copy headers (only need one copy)
echo ""
echo "=== Copying headers ==="
mkdir -p "${INSTALL_DIR}/include"
cp -rv "libopenmpt-${LIBOPENMPT_VERSION}+release.autotools-arm64-v8a/libopenmpt" "${INSTALL_DIR}/include/" || true
cp -v "libopenmpt-${LIBOPENMPT_VERSION}+release.autotools-arm64-v8a/bin/libopenmpt_config.h" "${INSTALL_DIR}/include/libopenmpt/" || true

echo ""
echo "==================================================================="
echo "✓ libopenmpt built successfully for Android!"
echo "==================================================================="
echo ""
echo "Libraries installed to:"
echo "  ${INSTALL_DIR}/arm64-v8a/libopenmpt.so"
echo "  ${INSTALL_DIR}/armeabi-v7a/libopenmpt.so"
echo "  ${INSTALL_DIR}/include/libopenmpt/"
echo ""
echo "You can now build regroove with:"
echo "  cd android"
echo "  gradle assembleDebug"
echo ""
