#!/bin/bash

PROJECT_DIR="${SCRIPT_FOLDER}/.."
QT_SDK_DIR="${1:-/opt/Qt/5.12.6}"
ANDROID_NDK="${2:-/opt/android/ndk}"
export ANDROID_NDK_ROOT=${ANDROID_NDK}

BUILD_TYPE=release
APPLICATION_NAME=virgil-messenger
PROJECT_FILE=${PROJECT_DIR}/${APPLICATION_NAME}.pro
TOOL_NAME=qmake

export QT_INSTALL_DIR_BASE=${PROJECT_DIR}/ext/prebuilt

BUILD_NUMBER="${BUILD_NUMBER:-0}"
if [ -f "${PROJECT_DIR}/VERSION_MESSENGER" ]; then
    export VERSION="$(cat ${PROJECT_DIR}/VERSION_MESSENGER | tr -d '\n').${BUILD_NUMBER}"
fi

#***************************************************************************************
function check_error() {
    RETRES=$?
    if [ $RETRES != 0 ]; then
        echo "----------------------------------------------------------------------"
        echo "############# !!! PROCESS ERROR ERRORCODE=[$RETRES]  #################"
        echo "----------------------------------------------------------------------"
        [ "$1" == "0" ] || exit $RETRES
    else
        echo "-----# Process OK. ---------------------------------------------------"
    fi
    return $RETRES
}

# export function definitions to sub-shell
export -f check_error
#***************************************************************************************
function print_title() {
    echo
    echo "===================================="
    echo "=== ${PLATFORM} ${APPLICATION_NAME} build ${VERSION}"
    echo "=== Build type : ${BUILD_TYPE}"
    echo "=== Tool name : ${TOOL_NAME}"
    echo "=== Output directory : ${BUILD_DIR}"
    echo "===================================="
    echo
}


function print_message() {
	echo
	echo "=== $1"
	echo
}

export -f print_message

#***************************************************************************************
function prepare_dir() {
    echo "=== Prepare directory"
    echo
    rm -rf ${BUILD_DIR} || true
    mkdir -p ${BUILD_DIR}
    check_error
}

#***************************************************************************************
function prepare_libraries() {
    echo "=== Prepare libraries"
    echo
    ${PROJECT_DIR}/scripts/get-prebuilt-libs.sh
}

#***************************************************************************************

#
# Check platform
#
if [ $(uname) == "Darwin" ]; then
    HOST_PLATFORM="darwin-x86_64"
elif [ $(uname) == "Linux" ]; then
    HOST_PLATFORM="linux-x86_64"
else
    echo "Wrong platform $(uname). Supported only: [Linux, Darwin]"
    exit 1
fi

#
#    Check input parameters
#
if [ -z "$PROJECT_DIR" ] || [ ! -d ${PROJECT_DIR} ]; then
    echo "Wrong Project directory: ${PROJECT_DIR}"
    exit 1
fi
