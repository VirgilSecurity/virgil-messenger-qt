#!/bin/bash
set -o errtrace
#
#   Global variables
#
SCRIPT_FOLDER="$(cd "$(dirname "$0")" && pwd)"
source ${SCRIPT_FOLDER}/ish/common.sh

#export PREBUILT_SKIP="true"

PLATFORM=linux-g++
BUILD_DIR=${PROJECT_DIR}/${BUILD_TYPE}/${TOOL_NAME}.${PLATFORM}/
IOS_QMAKE="${QT_SDK_DIR}/ios/bin/qmake"
QMAKE_PARAMS="${QMAKE_PARAMS:-" "}"

#***************************************************************************************
ios_build () {
   local IOS_BUILD_CONFIG="${1}"
   PLATFORM="${2}"
   BUILD_DIR=${PROJECT_DIR}/${BUILD_TYPE}/${TOOL_NAME}.${PLATFORM}/
   new_dir ${BUILD_DIR}
   pushd "${BUILD_DIR}"
       ${IOS_QMAKE} -config macx-ios-clang ${PROJECT_DIR} ${QMAKE_PARAMS} VERSION="${VERSION}" CONFIG+="${IOS_BUILD_CONFIG}"
       make -j10
   popd

}


print_title

prepare_libraries

print_message "Build iphonesimulator"
ios_build "iphonesimulator simulator" "ios-sim"

print_message "Build iphoneos"
ios_build "release iphoneos device qtquickcompiler" "ios"
