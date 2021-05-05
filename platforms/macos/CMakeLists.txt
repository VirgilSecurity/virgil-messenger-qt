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

#
#   System introspection.
#
find_package(Qt5 COMPONENTS Widgets REQUIRED)

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
#   Deploy
# ---------------------------------------------------------------------------
find_program(MAC_DEPLOY_QT macdeployqt)
find_program(MAC_CODESIGN codesign)
find_program(MAC_APPDMG appdmg)

add_custom_target(dmg_release
        COMMAND echo "Deploy MacOS bundle data..."
        COMMAND ${MAC_DEPLOY_QT}
            "${CMAKE_BINARY_DIR}/${PROJECT_NAME}.app"
            -qmldir="${PROJECT_SOURCE_DIR}/src/qml"
            -verbose=1
        COMMAND echo "Signing sparkle [Autoupdate]"
        COMMAND ${MAC_CODESIGN}
             --display
             --verbose=4
            --force
            --deep
            --timestamp
            --options runtime
             -s "${VS_MACOS_IDENT}"
             "${CMAKE_BINARY_DIR}/${PROJECT_NAME}.app/Contents/Frameworks/Sparkle.framework/Resources/Autoupdate.app/Contents/MacOS/Autoupdate"
       COMMAND echo "Signing sparkle [fileop]"
       COMMAND ${MAC_CODESIGN}
           --display
           --verbose=4
           --force
           --deep
           --timestamp
           --options runtime
           -s "${VS_MACOS_IDENT}"
           "${CMAKE_BINARY_DIR}/${PROJECT_NAME}.app/Contents/Frameworks/Sparkle.framework/Resources/Autoupdate.app/Contents/MacOS/fileop"
        COMMAND echo "Signing bundle..."
        COMMAND ${MAC_CODESIGN}
            --display
            --verbose=4
            --force
            --deep
            --timestamp
            --options runtime
            -s "${VS_MACOS_IDENT}"
            "${CMAKE_BINARY_DIR}/${PROJECT_NAME}.app"
        COMMAND echo "Create DMG..."
        COMMAND ${MAC_APPDMG}
            "${CMAKE_BINARY_DIR}/dmg.json"
            "${CMAKE_BINARY_DIR}/${PROJECT_NAME}.dmg"
            -v
        COMMAND echo "Set DMG icon..."
        COMMAND ${PROJECT_SOURCE_DIR}/platforms/macos/tools/seticon
            "${VS_CUSTOMER_DIR}/platforms/macos/pkg_resources/${MACOSX_BUNDLE_ICON_FILE}"
            "${CMAKE_BINARY_DIR}/${PROJECT_NAME}.dmg"
        VERBATIM
        )

add_custom_target(dmg_notarization
        COMMAND "${PROJECT_SOURCE_DIR}/platforms/macos/tools/dmg-notarization.sh"
            -f "${CMAKE_BINARY_DIR}/${PROJECT_NAME}.dmg"
            -u ${NOTARIZATION_LOGIN}
            -p ${NOTARIZATION_PASSWORD}
            -i "${MACOSX_BUNDLE_GUI_IDENTIFIER}"
        VERBATIM
        )

add_custom_target(dmg_debug
        COMMAND echo "Deploy MacOS bundle data (without signing)..."
        COMMAND ${MAC_DEPLOY_QT}
            ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.app
            -qmldir=${PROJECT_SOURCE_DIR}/src/qml
            -verbose=1
            -dmg
        COMMAND ${PROJECT_SOURCE_DIR}/platforms/macos/tools/seticon
            "${VS_CUSTOMER_DIR}/platforms/macos/pkg_resources/${MACOSX_BUNDLE_ICON_FILE}"
            "${CMAKE_BINARY_DIR}/${PROJECT_NAME}.dmg"
        VERBATIM
        )

add_custom_target(dmg_update
        COMMAND echo "Creating sparkle update files..."
        COMMAND cp -f
            "${CMAKE_BINARY_DIR}/${PROJECT_NAME}.dmg"
            "${CMAKE_BINARY_DIR}/update/${PROJECT_NAME}-${PROJECT_VERSION}.dmg"
        COMMAND ${PROJECT_SOURCE_DIR}/ext/prebuilt/macos/sparkle/bin/generate_appcast "${CMAKE_BINARY_DIR}/update"
        VERBATIM
        )
