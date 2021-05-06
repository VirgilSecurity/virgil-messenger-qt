#   Copyright (C) 2015-2021 Virgil Security Inc.
#
#   All rights reserved.
#
#   Redistribution and use in source and binary forms, with or without
#   modification, are permitted provided that the following conditions are
#   met:
#
#       (1) Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#
#       (2) Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in
#       the documentation and/or other materials provided with the
#       distribution.
#
#       (3) Neither the name of the copyright holder nor the names of its
#       contributors may be used to endorse or promote products derived from
#       this software without specific prior written permission.
#
#   THIS SOFTWARE IS PROVIDED BY THE AUTHOR ''AS IS'' AND ANY EXPRESS OR
#   IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
#   WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
#   DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
#   INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
#   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
#   SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
#   HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
#   STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
#   IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
#   POSSIBILITY OF SUCH DAMAGE.
#
#   Lead Maintainer: Virgil Security Inc. <support@virgilsecurity.com>

cmake_minimum_required(VERSION 3.16 FATAL_ERROR)

# ---------------------------------------------------------------------------
#   System introspection
# ---------------------------------------------------------------------------
find_package(Qt5 COMPONENTS Widgets REQUIRED)

# ---------------------------------------------------------------------------
#   Libraries
# ---------------------------------------------------------------------------
#
#   Library: platform-deps
#
add_library(platform-deps INTERFACE)
target_link_libraries(platform-deps
        INTERFACE
        Qt5::Widgets
        curl
        )

#
#   Library: platform.
#
add_library(platform STATIC)

target_sources(platform
        PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/include/PlatformMacos.h"
        "${CMAKE_CURRENT_LIST_DIR}/src/PlatformMacos.mm"
        )

target_include_directories(platform
        PRIVATE
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>"
        )

target_link_libraries(platform
        PUBLIC
        platform-base
        customer-config
        )

#
#   Library: platform-updates.
#
add_library(platform-updates STATIC)

target_sources(platform-updates
        PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/include/PlatformUpdatesMacos.h"
        "${CMAKE_CURRENT_LIST_DIR}/src/PlatformUpdatesMacos.mm"
        )

target_include_directories(platform-updates
        PRIVATE
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>"
        "$<BUILD_INTERFACE:${PREBUILT_DIR}/${VS_PLATFORM}/sparkle/Sparkle.framework/Headers>"
        )

target_link_libraries(platform-updates
        PUBLIC
        platform-updates-api
        )

# ---------------------------------------------------------------------------
#   Main target
# ---------------------------------------------------------------------------
set(App_ICON "${VS_CUSTOMER_DIR}/platforms/macos/pkg_resources/${MACOSX_BUNDLE_ICON_FILE}")
set_source_files_properties(${App_ICON}
        PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")

add_executable(${VS_TARGET_NAME} MACOSX_BUNDLE ${App_ICON})

set_target_properties(${VS_TARGET_NAME}
        PROPERTIES MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_LIST_DIR}/Info.plist.in"
        )

add_custom_command(TARGET ${VS_TARGET_NAME}
        POST_BUILD COMMAND
        ${CMAKE_INSTALL_NAME_TOOL} -add_rpath "@executable_path/../Frameworks/"
        $<TARGET_FILE:${VS_TARGET_NAME}>)

set_target_properties(${VS_TARGET_NAME}
        PROPERTIES LINK_FLAGS "-F${PREBUILT_DIR}/${VS_PLATFORM}/sparkle -framework Sparkle -framework CoreFoundation -framework Foundation"
        )

add_custom_command(TARGET ${VS_TARGET_NAME} POST_BUILD
        COMMAND mkdir ARGS -p
            "$<TARGET_BUNDLE_DIR:${VS_TARGET_NAME}>/Contents/Frameworks"

        COMMAND cp ARGS -Rf
            "${PREBUILT_DIR}/${VS_PLATFORM}/sparkle/Sparkle.framework"
            "$<TARGET_BUNDLE_DIR:${VS_TARGET_NAME}>/Contents/Frameworks/Sparkle.framework"
        )

# ---------------------------------------------------------------------------
#   Configure template files.
# ---------------------------------------------------------------------------
configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/release-notes.html.in"
        "${CMAKE_CURRENT_BINARY_DIR}/update/release-notes.html"
        @ONLY
        )

configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/dmg.json.in"
        "${CMAKE_CURRENT_BINARY_DIR}/dmg.json"
        @ONLY
        )

# ---------------------------------------------------------------------------
#   Deploy
# ---------------------------------------------------------------------------
find_program(MAC_DEPLOY_QT macdeployqt)
find_program(MAC_CODESIGN codesign)
find_program(MAC_APPDMG appdmg)

add_custom_target(deploy
        COMMENT "Deploy MacOS bundle data..."
        COMMAND "${MAC_DEPLOY_QT}"
            "$<TARGET_BUNDLE_DIR:${VS_TARGET_NAME}>"
            -qmldir="${PROJECT_SOURCE_DIR}/src/qml"
            -verbose=1
        DEPENDS ${VS_TARGET_NAME}
        )

add_custom_target(sign_sparkle
        COMMENT "Signing sparkle..."
        COMMAND "${MAC_CODESIGN}"
            --display
            --verbose=4
            --force
            --deep
            --timestamp
            --options runtime
            -s "${VS_MACOS_IDENT}"
            "$<TARGET_BUNDLE_DIR:${VS_TARGET_NAME}>/Contents/Frameworks/Sparkle.framework/Resources/Autoupdate.app/Contents/MacOS/Autoupdate"

       COMMAND "${MAC_CODESIGN}"
           --display
           --verbose=4
           --force
           --deep
           --timestamp
           --options runtime
           -s "${VS_MACOS_IDENT}"
           "$<TARGET_BUNDLE_DIR:${VS_TARGET_NAME}>/Contents/Frameworks/Sparkle.framework/Resources/Autoupdate.app/Contents/MacOS/fileop"

        DEPENDS deploy
        )

add_custom_target(sign_bundle
        COMMENT "Signing bundle..."
        COMMAND "${MAC_CODESIGN}"
            --display
            --verbose=4
            --force
            --deep
            --timestamp
            --options runtime
            -s "${VS_MACOS_IDENT}"
            "$<TARGET_BUNDLE_DIR:${VS_TARGET_NAME}>"
        DEPENDS deploy sign_sparkle
        )

add_custom_target(create_dmg
        COMMENT "Creating DMG..."

        COMMAND "${CMAKE_COMMAND}" -E
            rm -f "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.dmg"

        COMMAND "${MAC_APPDMG}"
            "${CMAKE_CURRENT_BINARY_DIR}/dmg.json"
            "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.dmg"
            -v

        COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/tools/seticon"
            "${VS_CUSTOMER_DIR}/platforms/macos/pkg_resources/${MACOSX_BUNDLE_ICON_FILE}"
            "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.dmg"

        DEPENDS
            deploy
            sign_bundle
            "${CMAKE_CURRENT_BINARY_DIR}/dmg.json"
        )

add_custom_target(notarize_dmg
        COMMENT "Notarizing DMG..."

        COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/tools/dmg-notarization.sh"
            -f "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.dmg"
            -u ${NOTARIZATION_LOGIN}
            -p ${NOTARIZATION_PASSWORD}
            -i "${MACOSX_BUNDLE_GUI_IDENTIFIER}"

        DEPENDS create_dmg
        )


add_custom_target(dmg_update
        COMMENT "Creating Sparkle update files..."

        COMMAND "${CMAKE_COMMAND}" -E copy_if_different
            "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.dmg"
            "${CMAKE_CURRENT_BINARY_DIR}/update/${PROJECT_NAME}-${PROJECT_VERSION}.dmg"

        COMMAND "${PREBUILT_DIR}/macos/sparkle/bin/generate_appcast" "${CMAKE_CURRENT_BINARY_DIR}/update"

        DEPENDS notarize_dmg
        )

add_custom_target(dmg_release
        COMMENT "Create production ready MacOS DMG..."
        DEPENDS dmg_update
        )


add_custom_target(dmg_debug
        COMMENT "Create debug MacOS DMG (without signing)..."

        COMMAND "${CMAKE_COMMAND}" -E
            rm -f "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.dmg"

        COMMAND "${MAC_DEPLOY_QT}"
            "$<TARGET_BUNDLE_DIR:${VS_TARGET_NAME}>"
            -qmldir="${PROJECT_SOURCE_DIR}/src/qml"
            -verbose=1
            -dmg
        COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/tools/seticon"
            "${VS_CUSTOMER_DIR}/platforms/macos/pkg_resources/${MACOSX_BUNDLE_ICON_FILE}"
            "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.dmg"

        DEPENDS ${VS_TARGET_NAME}
        )
