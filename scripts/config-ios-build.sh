#!/bin/bash
set -o errtrace
#
#   Global variables
#
SCRIPT_FOLDER="$(cd "$(dirname "$0")" && pwd)"
source ${SCRIPT_FOLDER}/ish/common.sh

PLATFORM=linux-g++
BUILD_DIR=${PROJECT_DIR}/${BUILD_TYPE}/${TOOL_NAME}.${PLATFORM}/
IOS_QMAKE="${QT_SDK_DIR}/ios/bin/qmake"
QMAKE_PARAMS="${QMAKE_PARAMS:-" "}"

#***************************************************************************************
ios_config () {
   local IOS_BUILD_CONFIG="${1}"
   PLATFORM="${2}"
   BUILD_DIR=${PROJECT_DIR}/${BUILD_TYPE}/${TOOL_NAME}.${PLATFORM}/
   new_dir ${BUILD_DIR}
   pushd "${BUILD_DIR}"
       ${IOS_QMAKE} -config macx-ios-clang ${PROJECT_DIR} ${QMAKE_PARAMS} VERSION="${ORIG_VERSION}" CONFIG+="${IOS_BUILD_CONFIG}" VS_CUSTOMER="${PARAM_CUSTOMER}"
   popd
}

print_title

print_message "Config iphoneos"
ios_config "release iphoneos device qtquickcompiler" "ios"

print_message "Config iphonesimulator"
ios_config "iphonesimulator simulator" "ios-sim"

print_message "All operation finished"
