#!/bin/bash

#
#   Global variables
#
SCRIPT_FOLDER="$( cd "$( dirname "$0" )" && pwd )"
. ${SCRIPT_FOLDER}/ish/error.ish

QXMPP_DIR="${SCRIPT_FOLDER}/../ext/qxmpp"
BUILD_DIR_BASE="${QXMPP_DIR}"

PLATFORM="${1}"
PARAM_PREFIX_PATH="${2}"
CMAKE_CUSTOM_PARAM="${3}"

#
#   Build
#
function build() {
    local BUILD_TYPE=$1
    local CMAKE_ARGUMENTS=$2
    local CORES=10

    local BUILD_DIR=${BUILD_DIR_BASE}/cmake-build-${PLATFORM}/${BUILD_TYPE}
    local INSTALL_DIR=${BUILD_DIR_BASE}/cmake-build-${PLATFORM}/${BUILD_TYPE}/installed
    local LIBS_DIR=${INSTALL_DIR}/usr/local/lib
    local LIBS_DIR64=${INSTALL_DIR}/usr/local/lib64

    echo
    echo "===================================="
    echo "=== ${PLATFORM} ${BUILD_TYPE} build"
    echo "=== Output directory: ${BUILD_DIR}"
    echo "===================================="
    echo

    rm -rf ${BUILD_DIR}
    mkdir -p ${BUILD_DIR}
    mkdir -p ${INSTALL_DIR}

    pushd ${BUILD_DIR}
      # prepare to build
      cmake ${BUILD_DIR_BASE} ${CMAKE_ARGUMENTS} -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -G "Unix Makefiles"
      check_error

      # build all targets
      make -j ${CORES}
      check_error

      # install all targets
      make DESTDIR=${INSTALL_DIR} install
      check_error

      if [ -d ${LIBS_DIR64} ]; then
          mv -f ${LIBS_DIR64} ${LIBS_DIR}
      fi

    popd
}

#
#   Prepare cmake parameters
#

#
#   MacOS
#
if [[ "${PLATFORM}" == "macos" ]]; then
    CMAKE_ARGUMENTS=" \
          -DBUILD_SHARED=OFF \
          -DBUILD_EXAMPLES=OFF \
          -DBUILD_TESTS=OFF \
          -DWITH_OPUS=OFF \
          -DWITH_VPX=OFF \
          -DCMAKE_PREFIX_PATH=${PARAM_PREFIX_PATH} \
           ${CMAKE_CUSTOM_PARAM} \    
    "

#
#   Windows (mingw) over Linux
#

elif [[ "${PLATFORM}" == "windows" && "$(uname)" == "Linux" ]]; then
    CMAKE_ARGUMENTS=" \
          -DBUILD_SHARED=OFF \
          -DBUILD_EXAMPLES=OFF \
          -DBUILD_TESTS=OFF \
          -DWITH_OPUS=OFF \
          -DWITH_VPX=OFF \
          -DCMAKE_PREFIX_PATH=${PARAM_PREFIX_PATH} \
          -DCMAKE_TOOLCHAIN_FILE=/usr/share/mingw/toolchain-mingw32.cmake \
          -DCYGWIN=1 \
           ${CMAKE_CUSTOM_PARAM} \
    "
#
#   Windows
#
elif [[ "${PLATFORM}" == "windows" ]]; then
    CMAKE_ARGUMENTS=" \
    "

#
#   Linux
#
elif [[ "${PLATFORM}" == "linux" ]]; then
    CMAKE_ARGUMENTS=" \
          -DBUILD_SHARED=OFF \
          -DBUILD_EXAMPLES=OFF \
          -DBUILD_TESTS=OFF \
          -DWITH_OPUS=OFF \
          -DWITH_VPX=OFF \
          -DCMAKE_PREFIX_PATH=${PARAM_PREFIX_PATH} \
           ${CMAKE_CUSTOM_PARAM} \
    "

#
#   iOS
#
elif [[ "${PLATFORM}" == "ios" ]]; then
    CMAKE_ARGUMENTS=" \
    "

#
#   iOS Simulator
#
elif [[ "${PLATFORM}" == "ios-sim" ]]; then
    build_messenger_deps
    CMAKE_ARGUMENTS=" \
    "

#
#   Android
#
elif [[ "${PLATFORM}" == "android" ]]; then
    CMAKE_ARGUMENTS=" \
    "
else
    echo "Virgil IoTKIT build script usage : "
    echo "$0 platform < platform-specific > < QT path >"
    echo "where : "
    echo "   platform - platform selector. Currently supported: android, ios, ios-sim, linux, macos, mingw32, windows"
    echo "   platform-specific for Android :"
    echo "     android_ABI [android_platform]"

    exit 1
fi

#
#   Build both Debug and Release
#
build "release" "${CMAKE_ARGUMENTS}"
