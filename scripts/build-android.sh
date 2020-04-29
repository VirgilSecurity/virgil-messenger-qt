#!/bin/bash

#
#   Global variables
#
SCRIPT_FOLDER="$(cd "$(dirname "$0")" && pwd)"
source ${SCRIPT_FOLDER}/ish/common.sh

ANDOID_APP_ID="com.virgilsecurity.qtmessenger"
PLATFORM=android-clang
ANDROID_MAKE="${ANDROID_NDK}/prebuilt/${HOST_PLATFORM}/bin/make"
ANDROID_PLATFORM="android-24"

#*************************************************************************************************************
build_proc() {
    PLATFORM="$1"
    LIB_ARCH="$2"

    local ANDROID_QMAKE="${QT_SDK_DIR}/${PLATFORM}/bin/qmake"
    local BUILD_DIR="${PROJECT_DIR}/${BUILD_TYPE}/${TOOL_NAME}.${PLATFORM}"
    local ANDROID_DEPLOY_QT="${QT_SDK_DIR}/${PLATFORM}/bin/androiddeployqt"

    export QT_BUILD_DIR_SUFFIX=android.${LIB_ARCH}

    print_title

    echo
    echo "=== Make application bundle [${PLATFORM}]"
    echo

    prepare_dir

    echo
    echo "=== Build Messenger"
    echo
    pushd ${BUILD_DIR}
    ${ANDROID_QMAKE} ${PROJECT_FILE} -spec android-clang CONFIG+=qtquickcompiler
    check_error

    ${ANDROID_MAKE} -j10
    check_error

    ${ANDROID_MAKE} INSTALL_ROOT=${BUILD_DIR}/android-build install
    check_error

    ${ANDROID_DEPLOY_QT} --input ${BUILD_DIR}/android-lib${APPLICATION_NAME}.so-deployment-settings.json --output ${BUILD_DIR}/android-build --android-platform ${ANDROID_PLATFORM} --gradle
    check_error
    popd
}

#*************************************************************************************************************

prepare_libraries

build_proc android_arm64_v8a arm64-v8a
build_proc android_armv7 armeabi-v7a
build_proc android_x86 x86
