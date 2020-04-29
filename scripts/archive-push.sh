#!/bin/bash

SCRIPT_FOLDER="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
. ${SCRIPT_FOLDER}/ish/common.ish

PROJECT_DIR="${1}"
ARTIFACT_DIR="${2}"

############################################################################################
echo_title() {
    echo "######"
    echo "# PROJECT: [${1}]"
    echo "######"
}

############################################################################################
archive_push_bintray() {
    curl -T ${FILE_NAME} -u${CDN_LOGIN}:${CDN_APIKEY} https://api.bintray.com/content/virgilsecurity/iotl-demo-cdn/iotl-demo/${BUILD_TYPE}/${FILE_NAME}
    check_error
}

############################################################################################
archive_publish_bintray() {
    curl -X POST -u${CDN_LOGIN}:${CDN_APIKEY} https://api.bintray.com/content/virgilsecurity/iotl-demo-cdn/iotl-demo/${BUILD_TYPE}/publish
    check_error
}

############################################################################################
archive_delete_bintray() {
    curl -X DELETE -u${CDN_LOGIN}:${CDN_APIKEY} https://api.bintray.com/content/virgilsecurity/iotl-demo-cdn/${FILE_NAME}
}

############################################################################################

# Get IOTKIT VERSION
BUILD_NUMBER="${BUILD_NUMBER:-0}"
echo "### Get iotkit version"
if [ -f "${PROJECT_DIR}/VERSION" ]; then
    IOTKIT_VERS="$(cat ${PROJECT_DIR}/VERSION | tr -d '\n').${BUILD_NUMBER}"
    echo "IOTKIT VERSION: [${IOTKIT_VERS}]"
else
    echo "IOTKIT Version file not found. Exit ..."
    exit 1
fi

echo "### Archiving artifacts directories"
pushd ${ARTIFACT_DIR}
check_error
for cur_dir in * ; do
    [ ! -d $cur_dir ] && continue
    echo "Archiving ${cur_dir} -> ${cur_dir}-${IOTKIT_VERS}.zip"
    zip -9 -r "${cur_dir}-${IOTKIT_VERS}.zip" "$cur_dir"
done
popd

echo "### Upload zip to bintray"
pushd ${ARTIFACT_DIR}
check_error
for cur_file in *.zip ; do
    [ ! -f $cur_file ] && continue
    echo "Upload ${cur_file}"
    FILE_NAME="${cur_file}"
    echo "Delete old [${FILE_NAME}]"
    archive_delete_bintray
    echo "Push new [${FILE_NAME}]"
    archive_push_bintray
    # Wait CDN
    sleep 5
    echo "Publishing data"
    archive_publish_bintray
done
popd

echo "### ALL OPERATION DONE"