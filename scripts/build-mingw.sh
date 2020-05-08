#!/bin/bash

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

echo
echo "=== Build application bundle"

pushd ${BUILD_DIR}
${LINUX_QMAKE} -config ${BUILD_TYPE} ${PROJECT_DIR} -spec win64-x-g++ VERSION="${VERSION}"

make -j10

echo
echo "== Deploying application"
echo ${LINUX_QMAKE}
echo 
echo
cqtdeployer -bin ${BUILD_DIR}/release/${APPLICATION_NAME}.exe -qmlDir ${PROJECT_DIR}/src/qml  -qmake ${LINUX_QMAKE} clear
popd

pushd ${PROJECT_DIR}/ext/prebuilt/windows/release/installed/usr/local/lib

echo "=== Copy libvs-messenger-internal.dll "
cp libvs-messenger-internal.dll           ${BUILD_DIR}/DistributionKit/lib

echo "=== Copy libvs-messenger-crypto.dll "
cp libvs-messenger-crypto.dll             ${BUILD_DIR}/DistributionKit/lib

echo "=== Copy openssl libraries"
cp capi.dll                               ${BUILD_DIR}/DistributionKit/lib
cp padlock.dll                            ${BUILD_DIR}/DistributionKit/lib
cp libcrypto-1_1-x64.dll                  ${BUILD_DIR}/DistributionKit/lib
cp libssl-1_1-x64.dll                     ${BUILD_DIR}/DistributionKit/lib
cp libssl-10.dll                          ${BUILD_DIR}/DistributionKit/lib

echo "=== Copy depends libraries"
cp libcurl-4.dll                         ${BUILD_DIR}/DistributionKit/lib
cp libssh2-1.dll                         ${BUILD_DIR}/DistributionKit/lib
cp libidn2-0.dll                         ${BUILD_DIR}/DistributionKit/lib
cp zlib1.dll                             ${BUILD_DIR}/DistributionKit/lib
cp libgcc_s_seh-1.dll                    ${BUILD_DIR}/DistributionKit/lib
cp libcrypto-10.dll                      ${BUILD_DIR}/DistributionKit/lib
popd

echo "=== Add custom env variables"
sed -i 's/start/SET VS_CURL_CA_BUNDLE=%BASE_DIR%\/ca\/curl-ca-bundle-win.crt\nstart/g' ${BUILD_DIR}/DistributionKit/virgil-messenger.bat

echo "=== Copy certs "
mkdir -p ${BUILD_DIR}/DistributionKit/ca
cp ${PROJECT_DIR}/src/qml/resources/ca/curl-ca-bundle-win.crt ${BUILD_DIR}/DistributionKit/ca
unix2dos ${BUILD_DIR}/DistributionKit/ca/curl-ca-bundle-win.crt
