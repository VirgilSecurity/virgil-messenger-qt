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

ENTITLEMENTS="/tmp/MacSandbox-Entitlements.plist"
APPDMG_SPEC="/tmp/spec.json"

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
function build_project() {
	print_title

	prepare_libraries

	prepare_dir

	echo
	echo "=== Build application bundle"

	pushd ${BUILD_DIR}

	${QMAKE_BIN} -config ${BUILD_TYPE} ${PROJECT_DIR} DEFINES+="VERSION=\"${VERSION}\""
	check_error

	make clean

	make -j10
	check_error

	echo
	echo "=== Deploy MAC application"
	echo

	${MACDEPLOYQT_BIN} ${APPLICATION_NAME}.app \
		-qmldir=${PROJECT_DIR}/src/qml

	pushd ${APPLICATION_NAME}.app
	LIBS=$(find . -name "*.dylib")

	echo
	echo "=== Remove dSYM files"
	echo
	for i in $(find . -name "*.dSYM"); do
		rm -rf "${i}"
	done

	echo
	echo "=== Sign dylib files"
	echo
	for i in $(find . -name "*.dylib"); do
		codesign -s "${CERT_ID}" "${i}"
	done

	echo
	echo "=== Sign frameworks"
	echo
	for i in $(find . -name "*.framework"); do
		codesign -s "${CERT_ID}" "${i}"
	done

	popd

	echo
	echo "=== Sign executable"
	echo
	codesign -s "${CERT_ID}" "${BUILD_DIR}/${APPLICATION_NAME}.app/Contents/MacOS/${APPLICATION_NAME}"
	check_error

	echo
	echo "=== Sign Autoupdate.app"
	echo
	codesign --deep --force --verify --verbose --sign "${CERT_ID}" --options runtime "${BUILD_DIR}/${APPLICATION_NAME}.app/Contents/Frameworks/Sparkle.framework/Versions/A/Resources/Autoupdate.app"

	echo
	echo "=== Sign Main App"
	echo
	codesign --deep --force --verify --verbose --sign "${CERT_ID}" --options runtime "${BUILD_DIR}/${APPLICATION_NAME}.app"

	echo
	echo "=== Set app file properties"
	echo
	echo "hideExtention ${APP_BUNDLE}"
	hideExtention "${APP_BUNDLE}"

	setIcon "${IMAGES_FOLDER}" "${APP_ICON}" "${BUILD_DIR}/${APP_BUNDLE}"

	popd

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
function hideExtention() {
	SetFile -a E "${1}"
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

	# cp "${DMG_RESULT}.dmg" "$DMG_PREPARE_FOLDER/${MESSENGER_BUNDLE_NAME}.dmg"
}

#***************************************************************************************
function notarize_dmg() {
	echo
	echo "=== Send Application for Apple's notarization"
	echo
	NOTARIZE_OUTPUT=$(xcrun altool -t osx -f "${DMG_FILE}" --primary-bundle-id "${PKG_IDENTIFIER}" --notarize-app --username ${USER_NAME} -p ${PASS} 2>&1)
	check_error
	NOTARIZE_ID=$(echo ${NOTARIZE_OUTPUT} | tr -d "\n" | grep -F 'No errors uploading' | awk -F 'RequestUUID' '{print $2}' | awk -F ' ' '{print $2}')
	check_error

	echo "NOTARIZE_ID = ${NOTARIZE_ID}"

	echo
	echo "=== Get result of notarization"
	echo
	NOTARIZATION_DONE="false"
	for ((count = 1; count < 20; count++)); do
		echo "Wait .. ${count} of 20"
		sleep 10s
		INFO_OUTPUT=$(xcrun altool --notarization-info "${NOTARIZE_ID}" --username ${USER_NAME} -p ${PASS})

		echo ${INFO_OUTPUT} >${HOME}/2.txt

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

	echo
	echo "=== Staple result of the notarization"
	echo
	STAMPLE_OUTPUT=$(xcrun stapler staple -v "${DMG_FILE}")

	if echo ${STAMPLE_OUTPUT} | grep -q -F 'The staple and validate action worked!'; then
		check_error
	else
		echo "${STAMPLE_OUTPUT}"
		exit 1
	fi
}

#***************************************************************************************

check_env
build_project
create_dmg
notarize_dmg
