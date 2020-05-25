#!/bin/bash
set -o errtrace
#
#   Global variables
#
SCRIPT_FOLDER="$(cd "$(dirname "$0")" && pwd)"
source ${SCRIPT_FOLDER}/ish/common.sh

ANDOID_APP_ID="com.virgilsecurity.qtmessenger"
PLATFORM=android-clang
ANDROID_MAKE="${ANDROID_NDK}/prebuilt/${HOST_PLATFORM}/bin/make"
ANDROID_PLATFORM="android-24"
if [ "$1" == "-release" ]; then
  ENABLE_RELEASE="true"
fi

#*************************************************************************************************************
# env variables passed to build anroid release
# $DISABLE_RELEASE                      : boolean
# $ANDROID_STORE_PASS                   : string
# $ANDROID_KEY_PASS                     : string
# {root_folder}/android.keystore        : file 
#*************************************************************************************************************
build_proc() {
    PLATFORM="$1"
    LIB_ARCH="$2"

    local ANDROID_QMAKE="${QT_SDK_DIR}/${PLATFORM}/bin/qmake"
    local BUILD_DIR="${PROJECT_DIR}/${BUILD_TYPE}/${TOOL_NAME}.${PLATFORM}"
    local ANDROID_DEPLOY_QT="${QT_SDK_DIR}/${PLATFORM}/bin/androiddeployqt"
    local ANDROID_DEPLOY_QT_ADD_ARGS=""

    export QT_BUILD_DIR_SUFFIX=android.${LIB_ARCH}

    print_title

    print_message "Make application bundle [${PLATFORM}]"
    new_dir ${BUILD_DIR}

    print_message "Build Messenger"
    pushd ${BUILD_DIR}
        ${ANDROID_QMAKE} ${PROJECT_FILE} -spec android-clang CONFIG+=qtquickcompiler VERSION="${VERSION}"

        ${ANDROID_MAKE} -j10

        ${ANDROID_MAKE} INSTALL_ROOT=${BUILD_DIR}/android-build install

        if [[ "x$ENABLE_RELEASE" != "x" ]]; then
            ANDROID_DEPLOY_QT_ADD_ARGS="--sign ${SCRIPT_FOLDER}/../android.keystore upload --storepass '${ANDROID_STORE_PASS}' --keypass '${ANDROID_KEY_PASS}'"
        fi

        ${ANDROID_DEPLOY_QT} --input ${BUILD_DIR}/android-lib${APPLICATION_NAME}.so-deployment-settings.json --output ${BUILD_DIR}/android-build --android-platform ${ANDROID_PLATFORM} ${ANDROID_DEPLOY_QT_ADD_ARGS} --gradle
    popd
}

#*************************************************************************************************************

prepare_libraries

build_proc android_arm64_v8a arm64-v8a
build_proc android_armv7 armeabi-v7a
build_proc android_x86 x86
