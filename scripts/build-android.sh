#!/bin/bash
set -o errtrace
#
#   Global variables
#
SCRIPT_FOLDER="$(cd "$(dirname "$0")" && pwd)"
source ${SCRIPT_FOLDER}/ish/common.sh

ANDOID_APP_ID="com.virgilsecurity.qtmessenger"
PLATFORM=android-clang
ANDROID_MAKE="${ANDROID_NDK_ROOT}/prebuilt/${HOST_PLATFORM}/bin/make"
ANDROID_PLATFORM="android-28"

#*************************************************************************************************************
# env variables passed to build anroid release
# $ENABLE_RELEASE                      : boolean
# $ANDROID_STORE_PASS                   : string
# $ANDROID_KEY_PASS                     : string
# {root_folder}/android.keystore        : file 
#*************************************************************************************************************
build_proc() {
    PLATFORM="$1"
    ANDROID_ABIS="$2"

    local ANDROID_QMAKE="${QT_SDK_DIR}/${PLATFORM}/bin/qmake"
    local BUILD_DIR="${PROJECT_DIR}/${BUILD_TYPE}/${TOOL_NAME}.${PLATFORM}"
    local ANDROID_DEPLOY_QT="${QT_SDK_DIR}/${PLATFORM}/bin/androiddeployqt"
    local ANDROID_DEPLOY_QT_ADD_ARGS=" "
    local ANDROID_BUILD_MODE="debug"    

    print_title

    print_message "Make application bundle [${PLATFORM}]"
    new_dir ${BUILD_DIR}

    print_message "Build Messenger"

    if [[ "x$ENABLE_RELEASE" != "x" ]]; then
        ANDROID_DEPLOY_QT_ADD_ARGS="--sign ${SCRIPT_FOLDER}/../android.keystore upload --storepass ${ANDROID_STORE_PASS} --keypass ${ANDROID_KEY_PASS}"
        ANDROID_BUILD_MODE="qtquickcompiler"            
    fi

    pushd ${BUILD_DIR}
        ${ANDROID_QMAKE} ANDROID_ABIS="${ANDROID_ABIS}" -spec android-clang CONFIG+=${ANDROID_BUILD_MODE} VERSION="${VERSION}" ${PROJECT_FILE}

        ${ANDROID_MAKE} -j10

        ${ANDROID_MAKE} INSTALL_ROOT=${BUILD_DIR}/android-build install

        ${ANDROID_DEPLOY_QT} --verbose --input ${BUILD_DIR}/android-${APPLICATION_NAME}-deployment-settings.json --output ${BUILD_DIR}/android-build --android-platform ${ANDROID_PLATFORM} ${ANDROID_DEPLOY_QT_ADD_ARGS} --aab
    popd
}

#*************************************************************************************************************

prepare_libraries
build_proc android  "armeabi-v7a arm64-v8a x86"
