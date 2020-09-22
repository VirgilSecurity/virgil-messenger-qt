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
ios_build () {
   local IOS_BUILD_CONFIG="${1}"
   PLATFORM="${2}"
   BUILD_DIR=${PROJECT_DIR}/${BUILD_TYPE}/${TOOL_NAME}.${PLATFORM}/
   new_dir ${BUILD_DIR}
   pushd "${BUILD_DIR}"
       ${IOS_QMAKE} -config macx-ios-clang ${PROJECT_DIR} ${QMAKE_PARAMS} VERSION="${VERSION}" CONFIG+="${IOS_BUILD_CONFIG}" VS_CUSTOMER="${PARAM_CUSTOMER}"
       make -j10
   popd
}

print_title

prepare_libraries

print_message "Build iphonesimulator"
ios_build "iphonesimulator simulator" "ios-sim"

print_message "Build iphoneos"
ios_build "release iphoneos device qtquickcompiler" "ios"

if [ "${IOS_NAME}" != "${APPLICATION_NAME}" ]; then
   echo "Rename VirgilMessenger.app => ${IOS_NAME}.app"

   pushd "${PROJECT_DIR}/${BUILD_TYPE}/${TOOL_NAME}.ios/Release-iphoneos"
     mv -f *.app "${IOS_NAME}.app"
   popd

   pushd "${PROJECT_DIR}/${BUILD_TYPE}/${TOOL_NAME}.ios-sim/Debug-iphonesimulator"
     mv -f *.app "${IOS_NAME}.app"
   popd
fi

if [ "${PARAM_BUILD_PKG}" == "1" ]; then
 build_ios_pkg
 build_iossim_pkg
fi

print_message "All operation finished"
