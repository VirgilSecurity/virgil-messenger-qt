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

#prepare_libraries

prepare_dir

echo
echo "=== Build application bundle"

pushd ${BUILD_DIR}
${LINUX_QMAKE} -config ${BUILD_TYPE} ${PROJECT_DIR} -spec win64-x-g++
check_error

make -j10
check_error

echo
echo "== Deploying application"
echo ${LINUX_QMAKE}
echo 
echo
cqtdeployer -bin ${BUILD_DIR}/release/${APPLICATION_NAME}.exe -qmlDir ${PROJECT_DIR}/src/qml  -qmake ${LINUX_QMAKE} clear
check_error

exit 1

echo "=== Copy libvs-messenger-internal.dll "
cp ${PROJECT_DIR}/ext/prebuilt/windows/release/installed/usr/local/lib/libvs-messenger-internal.dll DistributionKit/lib
check_error

echo "=== Copy libvs-messenger-crypto.dll "
cp ${PROJECT_DIR}/ext/prebuilt/windows/release/installed/usr/local/lib/libvs-messenger-crypto.dll DistributionKit/lib
check_error

echo "=== Copy openssl libraries"
cp ${PROJECT_DIR}/ext/prebuilt/windows/release/installed/usr/local/lib/capi.dll           DistributionKit/lib
check_error
cp ${PROJECT_DIR}/ext/prebuilt/windows/release/installed/usr/local/lib/dasync.dll         DistributionKit/lib
check_error
cp ${PROJECT_DIR}/ext/prebuilt/windows/release/installed/usr/local/lib/libcrypto-1_1.dll  DistributionKit/lib
check_error
cp ${PROJECT_DIR}/ext/prebuilt/windows/release/installed/usr/local/lib/libssl-1_1.dll     DistributionKit/lib
check_error
cp ${PROJECT_DIR}/ext/prebuilt/windows/release/installed/usr/local/lib/libssl-10.dll      DistributionKit/lib
check_error

echo "=== Add custom env variables"
sed -i 's/start/SET VS_CURL_CA_BUNDLE=%BASE_DIR%\/ca\/curl-ca-bundle-win.crt\nstart/g' DistributionKit/virgil-messenger.bat
check_error

echo "=== Copy libcrypto-10.dll"
#cp /usr/i686-w64-mingw32/sys-root/mingw/bin/libcrypto-10.dll DistributionKit/lib
check_error

echo "=== Copy libcurl-4.dll"
#cp /usr/i686-w64-mingw32/sys-root/mingw/bin/libcurl-4.dll DistributionKit/lib
check_error

echo "=== Copy libgcc_s_sjlj-1.dll"
#cp /usr/i686-w64-mingw32/sys-root/mingw/bin/libgcc_s_sjlj-1.dll DistributionKit/lib
check_error

echo "=== Copy libcrypto-10.dll"
#cp /usr/i686-w64-mingw32/sys-root/mingw/bin/libcrypto-10.dll DistributionKit/lib
check_error

echo "=== Copy libssl-10.dll"
#cp /usr/i686-w64-mingw32/sys-root/mingw/bin/libssl-10.dll DistributionKit/lib
check_error

echo "=== Copy libssh2-1.dll"
#cp /usr/i686-w64-mingw32/sys-root/mingw/bin/libssh2-1.dll DistributionKit/lib
check_error

echo "=== Copy libidn2-0.dll"
#cp /usr/i686-w64-mingw32/sys-root/mingw/bin/libidn2-0.dll DistributionKit/lib
check_error

echo "=== Copy zlib1.dll"
#cp /usr/i686-w64-mingw32/sys-root/mingw/bin/zlib1.dll DistributionKit/lib
check_error

echo "=== Copy certs "
mkdir -p DistributionKit/ca
check_error
cp ${PROJECT_DIR}/src/qml/resources/ca/curl-ca-bundle-win.crt DistributionKit/ca
check_error
unix2dos DistributionKit/ca/curl-ca-bundle-win.crt
check_error

popd
