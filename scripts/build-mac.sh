#!/bin/bash

#
#   Global variables
#
SCRIPT_FOLDER="$(cd "$(dirname "$0")" && pwd)"
source ${SCRIPT_FOLDER}/ish/common.sh

PLATFORM=mac
BUILD_DIR=${PROJECT_DIR}/${BUILD_TYPE}/${TOOL_NAME}.${PLATFORM}/
QMAKE_BIN=${QT_SDK_DIR}/clang_64/bin/qmake
MACDEPLOYQT_BIN=${QT_SDK_DIR}/clang_64/bin/macdeployqt

MESSENGER_BUNDLE_NAME="VirgilMessenger"

RESULT_FOLDER="${BUILD_DIR}/Release"
APP_BUNDLE="${APPLICATION_NAME}.app"
DMG_FILE=${BUILD_DIR}/${APPLICATION_NAME}.dmg

DMG_PREPARE_FOLDER="${BUILD_DIR}/DMG"

DMG_PACK_FOLDER="${MESSENGER_BUNDLE_NAME}"

IMAGES_FOLDER="${SCRIPT_FOLDER}/macos/pkg_resources"
ICON_FILE=""
BACKGROUND_FILE="Background.png"
PKG_IDENTIFIER="com.virgilsecurity.messenger"
DISTRIBUTION_XML="/tmp/distribution.xml"
DMG_ICON="Installer.icns"
APP_ICON="MyIcon.icns"

# not used
# ENTITLEMENTS="/tmp/MacSandbox-Entitlements.plist"
APPDMG_SPEC="/tmp/spec.json"

# Sparkle
SUFeedURL="${SUFeedURL:-""}"
SUPublicEDKey="${SUPublicEDKey:-""}"


#***************************************************************************************
function fill_plist() {
  echo
  echo "=== Fill Info.plist for sparkle"
  sed -e "s,@SUFeedURL@,${SUFeedURL},g" -e "s,@SUPublicEDKey@,${SUPublicEDKey},g" ${SCRIPT_FOLDER}/../src/virgil-messenger.plist.in > ${SCRIPT_FOLDER}/../src/virgil-messenger.plist
  check_error
}

#***************************************************************************************
function check_env() {
	if [ z"${CERT_ID}" == "z" ]; then
		echo "CERT_ID should contains certificate ID"
		exit 1
	fi

	if [ z"${USER_NAME}" == "z" ]; then
		echo "USER_NAME should contains Apple User ID"
		exit 1
	fi

	if [ z"${PASS}" == "z" ]; then
		echo "PASS should contains Apple User's password"
		exit 1
	fi
}

#***************************************************************************************
function sign_file() {
	echo "${1}"
	codesign --force --deep --timestamp --options runtime -s "${CERT_ID}" "${1}"
}
#***************************************************************************************
function sign_bundle() {
	local BUNDLE="${1}"

	pushd ${BUNDLE}

	print_message "Remove dSYM files"

	find . -name "*.dSYM"  -exec rm -rf {} \;
	popd

	print_message "Sign bundle"

	codesign --deep --force --verify --verbose --sign "${CERT_ID}" --options runtime "${BUNDLE}"
}

#***************************************************************************************
function build_project() {
	print_title

	prepare_libraries

	prepare_dir

	print_message "Build application bundle"

	pushd ${BUILD_DIR}

	${QMAKE_BIN} -config ${BUILD_TYPE} ${PROJECT_DIR} DEFINES+="VERSION=\"${VERSION}\""
	check_error
	
	make clean

	make -j10
	check_error

	print_message "Deploy MAC application"

	${MACDEPLOYQT_BIN} ${APPLICATION_NAME}.app -qmldir=${PROJECT_DIR}/src/qml

	print_message "Sign Autoupdate"

	AUTOUPDATE_APP="${BUILD_DIR}/${APPLICATION_NAME}.app/Contents/Frameworks/Sparkle.framework/Resources/Autoupdate.app"
	sign_file "${AUTOUPDATE_APP}/Contents/MacOS/Autoupdate"
	sign_file "${AUTOUPDATE_APP}/Contents/MacOS/fileop"

	print_message  "Sign Main application"

	MAIN_APP="${BUILD_DIR}/${APPLICATION_NAME}.app"
	sign_file "${MAIN_APP}/Contents/MacOS/${APPLICATION_NAME}"
	sign_bundle "${MAIN_APP}"

	# set on: Extension is hidden (allowed on folders)
	print_message "Set app file properties for ${APP_BUNDLE}"
	SetFile -a E  "${APP_BUNDLE}"

	setIcon "${IMAGES_FOLDER}" "${APP_ICON}" "${BUILD_DIR}/${APP_BUNDLE}"
}

#***************************************************************************************
function create_appdmg_spec() {
	cat <<EOT >"${APPDMG_SPEC}"
{
	"title": "Virgil Messenger ${VERSION}",
	"icon": "${IMAGES_FOLDER}/${DMG_ICON}",
	"background": "${IMAGES_FOLDER}/${BACKGROUND_FILE}",
	"icon-size": 90,
	"code-sign": {
		"signing-identity": "${CERT_ID}",
		"identifier": "${PKG_IDENTIFIER}"	
	},
	"contents": [
		{ "x": 180, "y": 200, "type": "file", "path": "${BUILD_DIR}/${APP_BUNDLE}" },
		{ "x": 460, "y": 200, "type": "link", "path": "/Applications" }
	]
}
EOT
}


#***************************************************************************************
function setIcon() {
	local ICON_FOLDER="${1}"
	local ICON_FILE="${2}"
	local FILE_TO_APPLY="${3}"

	pushd "${ICON_FOLDER}"
	./bin/seticon "${ICON_FILE}" "${FILE_TO_APPLY}"
	popd
}

#***************************************************************************************
function create_dmg() {
	create_appdmg_spec
	appdmg ${APPDMG_SPEC} ${DMG_FILE}

	setIcon "${IMAGES_FOLDER}" "${DMG_ICON}" "${DMG_FILE}"
}
#***************************************************************************************
function notarize_dmg() {

	print_message "Send Application for Apple's notarization"

	NOTARIZE_OUTPUT=$(xcrun altool -t osx -f "${DMG_FILE}" --primary-bundle-id "${PKG_IDENTIFIER}" --notarize-app --username ${USER_NAME} -p ${PASS} 2>&1)
	check_error

	echo "NOTARIZE_OUTPUT: $NOTARIZE_OUTPUT"
	
	NOTARIZE_ID=$(echo ${NOTARIZE_OUTPUT} | tr -d "\n" | grep -F 'No errors uploading' | awk -F 'RequestUUID' '{print $2}' | awk -F ' ' '{print $2}')
	check_error

	echo "NOTARIZE_ID = ${NOTARIZE_ID}"

	print_message "Get result of notarization"

	NOTARIZATION_DONE="false"

	count_max=20

	for count in $(seq 1 ${count_max} ); do
		echo "Wait .. ${count} of ${count_max}"
		sleep 10s
		INFO_OUTPUT=$(xcrun altool --notarization-info "${NOTARIZE_ID}" --username ${USER_NAME} -p ${PASS} 2>&1 | tr -d "\n")

		if echo ${INFO_OUTPUT} | grep -q -F 'Status Message: Package Approved'; then
			NOTARIZATION_DONE="true"
			check_error
			break
		fi

		if echo ${INFO_OUTPUT} | grep -q -F 'Status Message: Package Invalid'; then
			echo "${INFO_OUTPUT}"
			exit 1
		fi
	done

	if [ "${NOTARIZATION_DONE}" == "false" ]; then
		echo "${INFO_OUTPUT}"
		exit 1
	fi

	print_message "Staple result of the notarization"

	STAMPLE_OUTPUT=$(xcrun stapler staple -v "${DMG_FILE}" 2>&1 | tr -d "\n")

	if echo ${STAMPLE_OUTPUT} | grep -q -F 'The staple and validate action worked!'; then
		check_error
	else
		echo "${STAMPLE_OUTPUT}"
		exit 1
	fi
}

#***************************************************************************************

check_env
fill_plist
build_project
create_dmg
notarize_dmg
