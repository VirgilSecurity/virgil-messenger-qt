#!/bin/bash
set -o errtrace
#
#   Global variables
#
SCRIPT_FOLDER="$(cd "$(dirname "$0")" && pwd)"
# Include common function
source ${SCRIPT_FOLDER}/ish/common.sh

PLATFORM=linux-g++
BUILD_DIR=${PROJECT_DIR}/${BUILD_TYPE}/${TOOL_NAME}.${PLATFORM}/
LINUX_QMAKE="${QT_SDK_DIR}/gcc_64/bin/qmake"
QMAKE_PARAMS="${QMAKE_PARAMS:-" "}"

#***************************************************************************************

############################################################################################
print_title

prepare_libraries

new_dir ${BUILD_DIR}

print_message "Build application bundle"

pushd "${BUILD_DIR}"
    ${LINUX_QMAKE} -config ${BUILD_TYPE} ${PROJECT_DIR} ${QMAKE_PARAMS} VERSION="${VERSION}" VS_CUSTOMER="${PARAM_CUSTOMER}"

    make -j10

    cqtdeployer -bin ${APPLICATION_NAME} -qmlDir ${PROJECT_DIR}/src/qml -qmake ${LINUX_QMAKE} clear
popd


pushd "${BUILD_DIR}/DistributionKit"
 sed -i 's/#!\/bin\/sh/#!\/bin\/bash/g'  virgil-messenger.sh
 if [ "${LINUX_NAME}" != "${APPLICATION_NAME}" ]; then
   echo "Rename virgil-messenger.sh => ${LINUX_NAME}.sh"
   mv -f virgil-messenger.sh "${LINUX_NAME}.sh"
 fi
popd


if [ "${PARAM_BUILD_PKG}" == "1" ]; then
 build_linux_deb
 build_linux_rpm
fi

print_message "All operation finished"
