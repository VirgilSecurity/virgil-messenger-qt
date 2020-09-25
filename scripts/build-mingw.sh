#!/bin/bash -xe
set -o errtrace

#
#   Global variables
#
SCRIPT_FOLDER="$(cd "$(dirname "$0")" && pwd)"
source ${SCRIPT_FOLDER}/ish/common.sh

PLATFORM=linux-mingw
BUILD_DIR=${PROJECT_DIR}/${BUILD_TYPE}/${TOOL_NAME}.${PLATFORM}/
LINUX_QMAKE="${QT_SDK_DIR}/mingw64/bin/qmake"

#***************************************************************************************
print_title

prepare_libraries

new_dir ${BUILD_DIR}

print_message "Build application bundle"

pushd ${BUILD_DIR}
    ${LINUX_QMAKE} -config ${BUILD_TYPE} ${PROJECT_DIR} -spec win64-x-g++ VERSION="${VERSION}"

    make -j10

    print_message "Deploying application"
    echo ${LINUX_QMAKE}
    echo
    echo
    cqtdeployer -bin ${BUILD_DIR}/release/${APPLICATION_NAME}.exe -qmlDir ${PROJECT_DIR}/src/qml  -qmake ${LINUX_QMAKE} clear
popd

pushd ${PROJECT_DIR}/ext/prebuilt/windows/release/installed/usr/local/lib
    echo "=== Copy openssl libraries"

    flist="capi.dll padlock.dll libcrypto-1_1-x64.dll libssl-1_1-x64.dll libssl-10.dll"
    for ff in $flist; do
        cp ${ff} ${BUILD_DIR}/DistributionKit/lib
    done

    echo "=== Copy depends libraries"

    flist="libcurl-4.dll libssh2-1.dll libidn2-0.dll zlib1.dll libgcc_s_seh-1.dll libcrypto-10.dll"
    for ff in $flist; do
        cp ${ff} ${BUILD_DIR}/DistributionKit/lib
    done

popd

echo "=== Add custom env variables"
sed -i 's/start/SET VS_CURL_CA_BUNDLE=%BASE_DIR%\/ca\/curl-ca-bundle-win.crt\nstart/g' ${BUILD_DIR}/DistributionKit/virgil-messenger.bat

echo "=== Copy certs "
mkdir -p ${BUILD_DIR}/DistributionKit/ca
cp ${PROJECT_DIR}/src/qml/resources/ca/curl-ca-bundle-win.crt ${BUILD_DIR}/DistributionKit/ca
unix2dos ${BUILD_DIR}/DistributionKit/ca/curl-ca-bundle-win.crt
