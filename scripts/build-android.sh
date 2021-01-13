#!/bin/bash
set -o errtrace
#
#   Global variables
#
SCRIPT_FOLDER="$(cd $(dirname "$0") && pwd)"

# Include common function
source ${SCRIPT_FOLDER}/ish/common.sh
PLATFORM=android-clang
ANDROID_MAKE="${ANDROID_NDK_ROOT}/prebuilt/${HOST_PLATFORM}/bin/make"
ANDROID_PLATFORM="android-29"

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
        ANDROID_BUILD_TYPE="release"
    else
        ANDROID_BUILD_TYPE="debug"    
    fi

    pushd ${BUILD_DIR}
        ${ANDROID_QMAKE} ANDROID_ABIS="${ANDROID_ABIS}" -spec android-clang CONFIG+=${ANDROID_BUILD_MODE} VERSION="1${VERSION}" VS_CUSTOMER="${PARAM_CUSTOMER}" ${PROJECT_FILE} 

        ${ANDROID_MAKE} -j10

        ${ANDROID_MAKE} INSTALL_ROOT=${BUILD_DIR}/android-build install

        ${ANDROID_DEPLOY_QT} --verbose --input ${BUILD_DIR}/android-${APPLICATION_NAME}-deployment-settings.json --output ${BUILD_DIR}/android-build --android-platform ${ANDROID_PLATFORM} ${ANDROID_DEPLOY_QT_ADD_ARGS} --aab
    popd

   print_message "Rename ${APPLICATION_NAME} => ${ANDROID_NAME}-${VERSION}"
   pushd "${BUILD_DIR}/android-build/build/outputs/apk/${ANDROID_BUILD_TYPE}"
    mv -f *.apk     "${ANDROID_NAME}-${VERSION}.apk"
   popd
   pushd "${BUILD_DIR}/android-build/build/outputs/bundle/${ANDROID_BUILD_TYPE}"
    mv -f *.aab  "${ANDROID_NAME}-${VERSION}.aab"
   popd
}

#*************************************************************************************************************

prepare_libraries
build_proc android  "armeabi-v7a arm64-v8a x86 x86_64"
