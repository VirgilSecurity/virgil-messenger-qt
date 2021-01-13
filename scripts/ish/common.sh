#!/bin/bash


PROJECT_DIR="${SCRIPT_FOLDER}/.."
QT_SDK_DIR="${QT_SDK_ROOT:-/opt/Qt/5.15.0}"
export ANDROID_NDK_ROOT=${ANDROID_NDK_ROOT:-/opt/Android/Sdk/ndk/21.1.6352462}

BUILD_TYPE=release
#/# APPLICATION_NAME=virgil-messenger
#/# PROJECT_FILE=${PROJECT_DIR}/${APPLICATION_NAME}.pro
TOOL_NAME=qmake

export QT_INSTALL_DIR_BASE=${PROJECT_DIR}/ext/prebuilt

BUILD_NUMBER="${BUILD_NUMBER:-0}"
if [ -f "${PROJECT_DIR}/VERSION_MESSENGER" ]; then
    export ORIG_VERSION="$(cat ${PROJECT_DIR}/VERSION_MESSENGER | tr -d '\n')"
    export VERSION="$(cat ${PROJECT_DIR}/VERSION_MESSENGER | tr -d '\n').${BUILD_NUMBER}"
fi

trap 'err_trap  $@' ERR

############################################################################################
print_usage() {
  pushd "${PROJECT_DIR}/customers" 2>&1 > /dev/null
   for customer in */; do
     customer=${customer%?}
     [ "$customer" == "common" ] && continue
     if [ -z $CUSTOMER_LIST ]; then
          CUSTOMER_LIST="$customer"
     else
         CUSTOMER_LIST="${CUSTOMER_LIST}|$customer"
     fi
   done
  popd  2>&1 > /dev/null
  echo
  echo "$(basename ${0})"
  echo
  echo "  -c <${CUSTOMER_LIST}>   - Build for customer                            [default: Virgil]"
  echo "  -p                      - Build Linux packages (Required: docker)       [default: no]"  
  echo "  -h                      - Print help"
  exit 0
}

############################################################################################
#
#  Script parameters
#
############################################################################################

# Default customer
PARAM_CUSTOMER="${VS_CUSTOMER:-Virgil}"
PARAM_BUILD_PKG="0"
if [ "$IGNORE_PARAMS" != "1" ]; then 
 while [ -n "$1" ]
  do
    case "$1" in
      -h) print_usage
          exit 0
          ;;
      -p) PARAM_BUILD_PKG="1"
          ;;          
      -c) PARAM_CUSTOMER="$2"
          shift 
          ;;
      *) print_usage;;
    esac
    shift
  done
fi

# Include packaging 
if [ "${PARAM_BUILD_PKG}" == "1" ]; then 
  source ${PROJECT_DIR}/customers/${PARAM_CUSTOMER}/scripts/packaging.ish
fi

#***************************************************************************************
include_customer() {
   local CUSTOMER_DIRECTORY="${PROJECT_DIR}/customers/${PARAM_CUSTOMER}/scripts"
   pushd "${CUSTOMER_DIRECTORY}" 2>&1 > /dev/null
     for inc_file in *.ish; do
        source ${CUSTOMER_DIRECTORY}/${inc_file}
     done
   popd  2>&1 > /dev/null

}

#***************************************************************************************
remove_generated() {
   print_message "Remove generated...."
   rm -rf "${PROJECT_DIR}/generated"
}


#***************************************************************************************
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
    exit 127
}

#***************************************************************************************
function print_title() {
    echo
    echo "===================================="
    echo "=== ${PLATFORM} ${APPLICATION_NAME} build ${VERSION}"
    echo "=== Build type : ${BUILD_TYPE}"
    echo "=== Tool name : ${TOOL_NAME}"
    echo "=== Output directory : ${BUILD_DIR}"
    echo "=== Customer : ${PARAM_CUSTOMER}"    
    echo "===================================="
    echo
}

#***************************************************************************************
function print_message() {
    echo
    echo "===================================="
    echo "=== ${1}"
    echo "===================================="
}

#***************************************************************************************
function prepare_firebase() {
    FB_ARCH_NAME=firebase_cpp_sdk_6.11.0.zip
    FB_URL="https://dl.google.com/firebase/sdk/cpp/firebase_cpp_sdk_6.11.0.zip"

    pushd "${SCRIPT_FOLDER}/../ext/"
    wget -O "${FB_ARCH_NAME}" "${FB_URL}"
    unzip "${FB_ARCH_NAME}" -d "prebuilt"
    rm -rf "${FB_ARCH_NAME}"
    popd
}

#***************************************************************************************
function prepare_libraries() {

    CORE_VER=$(head -n 1 ${SCRIPT_FOLDER}/../VERSION_CORE)
    ARCH_NAME=prebuilt-${CORE_VER}.tgz
    PREBUILT_ARCHIVE="https://virgilsecurity.bintray.com/iotl-demo-cdn/${ARCH_NAME}"

    INSTALL_DIR="${SCRIPT_FOLDER}/../ext/"

    print_message "CORE VERSION = ${CORE_VER}"

    if [ -d ${INSTALL_DIR}/prebuilt ] && [ "${PREBUILT_SKIP}" == "true" ]; then
        print_message "Prebuild libraries found. Download skipped"
    else
        rm -rf ${INSTALL_DIR}/prebuilt

        pushd ${INSTALL_DIR}
           wget -O ${ARCH_NAME} ${PREBUILT_ARCHIVE}

           tar -xvf ${ARCH_NAME}

           rm ${ARCH_NAME}
        popd
    fi

    if [ -d ${INSTALL_DIR}/prebuilt/firebase_cpp_sdk ]; then
      print_message "Firebase SDK exist. skip..."
    else
      prepare_firebase
    fi

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

#***************************************************************************************
# Include customer settings
include_customer
remove_generated