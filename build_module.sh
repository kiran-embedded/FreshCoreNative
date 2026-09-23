#!/bin/bash
set -e

NDK_PATH="$HOME/Android/Sdk/ndk/29.0.14206865"
if [ ! -d "$NDK_PATH" ]; then
    echo "NDK not found at $NDK_PATH"
    exit 1
fi

echo "Building FreshCore Native..."
rm -rf build && mkdir build && cd build
cmake .. \
    -DCMAKE_TOOLCHAIN_FILE="$NDK_PATH/build/cmake/android.toolchain.cmake" \
    -DANDROID_ABI=arm64-v8a \
    -DANDROID_PLATFORM=android-30 \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_TESTING=OFF

make -j$(nproc)
cd ..

echo "Packaging Module..."
mkdir -p module/system/bin
cp build/freshcore module/system/bin/
chmod +x module/system/bin/freshcore
chmod +x module/service.sh

# Download standard KernelSU/Magisk installer if not present
mkdir -p module/META-INF/com/google/android
if [ ! -f module/META-INF/com/google/android/update-binary ]; then
    wget -qO module/META-INF/com/google/android/update-binary https://raw.githubusercontent.com/topjohnwu/Magisk/master/scripts/module_installer.sh
    chmod +x module/META-INF/com/google/android/update-binary
fi

cat > module/META-INF/com/google/android/updater-script << 'EOF'
#MAGISK
EOF

cd module
zip -r9 ../FreshCore-KSU.zip .
cd ..

echo "Done! FreshCore-KSU.zip has been generated."
