#!/bin/bash
set -o errtrace
#
#   Global variables
#
SCRIPT_FOLDER="$(cd "$(dirname "$0")" && pwd)"
source ${SCRIPT_FOLDER}/ish/common.sh

#
#	General variables
#
PLATFORM="mac"
BUILD_DIR="${PROJECT_DIR}/${BUILD_TYPE}/${TOOL_NAME}.${PLATFORM}"
QMAKE_BIN="${QT_SDK_DIR}/clang_64/bin/qmake"
MACDEPLOYQT_BIN="${QT_SDK_DIR}/clang_64/bin/macdeployqt"
APPCAST_BIN="${PROJECT_DIR}/ext/prebuilt/macos/sparkle/bin/generate_appcast"
QMAKE_PARAMS="${QMAKE_PARAMS:-" "}"

# Sparkle
SUFeedURL="${SUFeedURL:-""}"
SUPublicEDKey="${SUPublicEDKey:-""}"

#
#	Resources
#
IMAGES_FOLDER="${SCRIPT_FOLDER}/macos/pkg_resources"
BACKGROUND_FILE="Background.png"
DMG_ICON="Installer.icns"
APP_ICON="MyIcon.icns"

#
#	DMG maker
#
APPDMG_SPEC="/tmp/spec.json"
PKG_IDENTIFIER="com.virgilsecurity.messenger"

#
#	Results
#
APP_BUNDLE="${APPLICATION_NAME}.app"
DMG_FILE="${BUILD_DIR}/${APPLICATION_NAME}.dmg"
UPDATE_DIR="${BUILD_DIR}/update"
RELEASE_NOTES="${PROJECT_DIR}/release-notes.html"

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
        echo "#--------------------------------------------"
	echo "# Signing [${1}]"
	codesign --display --verbose=4 --force --deep --timestamp --options runtime -s "${CERT_ID}" "${1}"
	if [ "${?}" != "0" ]; then
          echo "# ERROR SIGNING [${1}]"
          exit 127
	fi
        echo "# Signing OK"	
}
#***************************************************************************************
function sign_bundle() {
	local BUNDLE="${1}"

	print_message "Remove ${BUNDLE}/*.dSYM files" 
	find  ${BUNDLE} -type f -name "*.dSYM" -delete

	print_message "Sign bundle"
	codesign --display --verbose=4 --deep --force --verify --sign "${CERT_ID}" --options runtime "${BUNDLE}"
	if [ "${?}" != "0" ]; then
          echo "# ERROR SIGNING BUNDLE"
          exit 127
	fi
        echo "# Signing OK"		
}

#***************************************************************************************
function build_project() {
	print_title

	prepare_libraries

	new_dir ${BUILD_DIR}

	print_message "Build application bundle"

	pushd ${BUILD_DIR}

		${QMAKE_BIN} -config ${BUILD_TYPE} ${PROJECT_DIR} ${QMAKE_PARAMS} VERSION="${VERSION}"

		make clean

		make -j10

		print_message "Deploy MAC application"

		${MACDEPLOYQT_BIN} ${APPLICATION_NAME}.app -qmldir=${PROJECT_DIR}/src/qml

		print_message "Sign Autoupdate"
		AUTOUPDATE_APP="${BUILD_DIR}/${APPLICATION_NAME}.app/Contents/Frameworks/Sparkle.framework/Resources/Autoupdate.app"
		sign_file "${AUTOUPDATE_APP}/Contents/macos/Autoupdate"
		sign_file "${AUTOUPDATE_APP}/Contents/macos/fileop"

		print_message "Sign Main application"
		MAIN_APP="${BUILD_DIR}/${APPLICATION_NAME}.app"
		sign_file "${MAIN_APP}/Contents/macos/${APPLICATION_NAME}"
		sign_bundle "${MAIN_APP}"

		print_message "Set app file properties"
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
}
#***************************************************************************************
function notarize_dmg() {
	print_message "Send Application for Apple's notarization"

	NOTARIZE_OUTPUT=$(xcrun altool -t osx -f "${DMG_FILE}" --primary-bundle-id "${PKG_IDENTIFIER}" --notarize-app --username ${USER_NAME} -p ${PASS} 2>&1)
	NOTARIZE_ID=$(echo ${NOTARIZE_OUTPUT} | tr -d "\n" | grep -F 'No errors uploading' | awk -F 'RequestUUID' '{print $2}' | awk -F ' ' '{print $2}')

	echo "NOTARIZE_ID = ${NOTARIZE_ID}"

	print_message "Get result of notarization"

	NOTARIZATION_DONE="false"
	for count in $(seq 1 300); do
		echo "Wait .. ${count} of 300"
		sleep 10s
		INFO_OUTPUT=$(xcrun altool --notarization-info "${NOTARIZE_ID}" --username ${USER_NAME} -p ${PASS} 2>&1 | tr -d "\n")

		if echo ${INFO_OUTPUT} | grep -q -F 'Status Message: Package Approved'; then
			NOTARIZATION_DONE="true"
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
		echo ""
	else
		echo "${STAMPLE_OUTPUT}"
		exit 1
	fi
}

#***************************************************************************************
function prepare_update() {
	new_dir "${UPDATE_DIR}"

	cp "${RELEASE_NOTES}" "${UPDATE_DIR}/${APPLICATION_NAME}-${VERSION}.html"
	cp "${DMG_FILE}" "${UPDATE_DIR}/${APPLICATION_NAME}-${VERSION}.dmg"

	rm -rf "${HOME}/Library/Caches/Sparkle_generate_appcast"

	"${APPCAST_BIN}" "${UPDATE_DIR}"
}

#***************************************************************************************

check_env
"${SCRIPT_FOLDER}/generate-mac-plist.sh" "${SUFeedURL}" "${SUPublicEDKey}" "${VERSION}"
build_project
create_dmg
notarize_dmg
prepare_update
