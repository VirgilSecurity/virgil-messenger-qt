#!/usr/bin/env bash

USER_DIR=${HOME}

# Target name
TARGET=iotkit-qt-example

# Build type - Release, Debug
BUILD_TYPE=Debug

# Make directory
MAKE_DIR=${BUILD_TYPE}/shell.Android

# Path to the directory with Android SDK
ANDROID_SDK=${USER_DIR}/Android/SDK

# Path to the directory with Android NDK
ANDROID_NDK=${ANDROID_SDK}/ndk/20.1.5948944

# Toolchain C and C++ compilers
ANDROID_C_COMPILER=${ANDROID_NDK}/toolchains/llvm/prebuilt/darwin-x86_64/bin/clang
ANDROID_CXX_COMPILER=${ANDROID_C_COMPILER}++

# Android architecture
ANDROID_ABI=armeabi-v7a

# Qt Toolchain for Android
QT_TOOLCHAIN=${USER_DIR}/Qt/5.12.6/android_armv7

# Android package name / activity name. You can find both in the generated manifest file (AndroidManifest.xml in the
# ${target}-apk-build directory after build)
ANDROID_APP=org.qtproject.example.iotkit_qt_example/org.qtproject.qt5.android.bindings.QtActivity

# C++ library type
ANDROID_STL=c++_shared

# Android platform (API level)
ANDROID_PLATFORM=24

# CMake system name
CMAKE_SYSTEM_NAME=ANDROID

# Application description
APP_DESCRIPTION="Demo"

# Cores amount
CORES=4

CMAKE_ARGS=" -DANDROID_QT=ON"
CMAKE_ARGS+=" -DANDROID_ABI=${ANDROID_ABI}"
CMAKE_ARGS+=" -DANDROID_NDK=${ANDROID_NDK}"
CMAKE_ARGS+=" -DANDROID_SDK=${ANDROID_SDK}"
CMAKE_ARGS+=" -DANDROID_STL=${ANDROID_STL}"
CMAKE_ARGS+=" -DANDROID_PLATFORM=${ANDROID_PLATFORM}"
CMAKE_ARGS+=" -DCMAKE_ANDROID_ARCH_ABI=${ANDROID_ABI}"
CMAKE_ARGS+=" -DCMAKE_BUILD_TYPE=${BUILD_TYPE}"
CMAKE_ARGS+=" -DCMAKE_CXX_COMPILER=${ANDROID_CXX_COMPILER}"
CMAKE_ARGS+=" -DCMAKE_C_COMPILER=${ANDROID_C_COMPILER}"
CMAKE_ARGS+=" -DCMAKE_FIND_ROOT_PATH_MODE_INCLUDE=BOTH"
CMAKE_ARGS+=" -DCMAKE_FIND_ROOT_PATH_MODE_LIBRARY=BOTH"
CMAKE_ARGS+=" -DCMAKE_FIND_ROOT_PATH_MODE_PACKAGE=BOTH"
CMAKE_ARGS+=" -DCMAKE_FIND_ROOT_PATH_MODE_PROGRAM=BOTH"
CMAKE_ARGS+=" -DCMAKE_PREFIX_PATH=${QT_TOOLCHAIN}"
CMAKE_ARGS+=" -DCMAKE_SYSTEM_NAME=${CMAKE_SYSTEM_NAME}"
CMAKE_ARGS+=" -DCMAKE_SYSTEM_VERSION=${ANDROID_PLATFORM}"
CMAKE_ARGS+=" -DCMAKE_TOOLCHAIN_FILE=${ANDROID_NDK}/build/cmake/android.toolchain.cmake"
CMAKE_ARGS+=" -DANDROID_APP_DESCRIPTION=${APP_DESCRIPTION}"
CMAKE_ARGS+=" -DQT_TOOLCHAIN=${QT_TOOLCHAIN}"

echo
echo ==
echo ==  CMake arguments :
echo ==
echo

echo ${CMAKE_ARGS} | tr " " "\n"

echo
echo ==
echo ==  Prepare build system in directory ${MAKE_DIR}
echo ==
echo

rm -rf ${MAKE_DIR}
mkdir -p ${MAKE_DIR}
cmake -S . -B ${MAKE_DIR} ${CMAKE_ARGS}

echo
echo ==
echo ==  Build target ${TARGET} and its deployment ${TARGET}-apk-install
echo ==
echo

cmake --build ${MAKE_DIR} --target ${TARGET}-apk-install -- -j ${CORES}

echo
echo ==
echo ==  Run on Android device
echo ==
echo

${ANDROID_SDK}/platform-tools/adb shell am start -n ${ANDROID_APP}