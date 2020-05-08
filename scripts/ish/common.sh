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


trap 'err_trap  $@' ERR

err_trap(){
    err_code=$?
    # ${FUNCNAME[*]} : err_trap print_message main  
    # call from subfunction: lenght = 3
    # ${FUNCNAME[*]} : err_trap  main
    # call from main: lenght = 2
    arr_shift=$((${#FUNCNAME[@]} - 2))
    # if arr_shift eq 0. we need to ommit FUNCTION and ARGUMENTS
    echo "##############################################################################"
    echo "### SCRIPT ERROR AT $0, IN LINE ${BASH_LINENO[$arr_shift]}"
    [  $arr_shift -ne 0 ] && echo "### BASH FUNCTION NAME: ${FUNCNAME[$arr_shift]}"
    echo "### BASH COMMAND: ${BASH_COMMAND[*]}"
    [  $arr_shift -ne 0 ] && echo "### COMMAND ARGUMENTS: $@"
    echo "### ERRORCODE: $err_code"
    echo "##############################################################################"
}


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
    echo "===================================="
    echo "=== ${1}"
    echo "===================================="
}

#***************************************************************************************
function prepare_libraries() {

    CORE_VER=$(head -n 1 ${SCRIPT_FOLDER}/../VERSION_CORE)
    ARCH_NAME=prebuilt-${CORE_VER}.tgz
    PREBUILT_ARCHIVE="https://bintray.com/virgilsecurity/iotl-demo-cdn/download_file?file_path=${ARCH_NAME}"

    INSTALL_DIR="${SCRIPT_FOLDER}/../ext/"


    print_message "CORE VERSION = ${CORE_VER}"


    if [ -d ${INSTALL_DIR}/prebuilt ] && [ "${PREBUILT_SKIP}" == "true" ]; then
        print_message "Prebuild libraries found. Download skipped"
        exit 0
    fi

    new_dir ${INSTALL_DIR} 

    pushd ${INSTALL_DIR}
        wget -O ${ARCH_NAME} ${PREBUILT_ARCHIVE}

        tar -xvf ${ARCH_NAME}

        rm ${ARCH_NAME}
    popd

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
#***************************************************************************************
function new_dir() {
    echo "=== Prepare directory: ${1}"
    rm -rf "${1}"

    mkdir -p "${1}"
}

export -f new_dir
#***************************************************************************************
