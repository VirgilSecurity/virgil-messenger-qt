#!/bin/bash

#
#   Global variables
#
SCRIPT_FOLDER="$( cd "$( dirname "$0" )" && pwd )"
source ${SCRIPT_FOLDER}/ish/common.sh

PLATFORM=linux-g++
BUILD_DIR=${PROJECT_DIR}${BUILD_TYPE}/${TOOL_NAME}.${PLATFORM}/
LINUX_QMAKE="${QT_SDK_DIR}/gcc_64/bin/qmake"

#***************************************************************************************

print_title

prepare_libraries

prepare_dir

echo
echo "=== Build application bundle"

${LINUX_QMAKE} -config ${BUILD_TYPE} ${PROJECT_DIR} DEFINES+="VERSION=\"${VERSION}\""
check_error

make -j10
check_error

cqtdeployer -bin ${APPLICATION_NAME} -qmlDir ${PROJECT_DIR}src/qml -qmake ${LINUX_QMAKE} clear
check_error

echo
echo "=== Copy libvs-messenger-internal.so "
echo
cp ${PROJECT_DIR}/ext/prebuilt/linux/usr/local/lib/libvs-messenger-internal.so DistributionKit/lib
check_error

echo
echo "=== Copy libvs-messenger-crypto.so "
echo
cp ${PROJECT_DIR}/ext/prebuilt/linux/usr/local/lib/libvs-messenger-crypto.so DistributionKit/lib
check_error
