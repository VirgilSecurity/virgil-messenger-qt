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
        WinSparkle
        curl
        )

target_link_directories(platform-deps
        INTERFACE
        "$<BUILD_INTERFACE:${PREBUILT_DIR}/windows/winsparkle/x64/Release>"
        )

#
#   Library: platform-updates.
#
add_library(platform-updates STATIC)

target_sources(platform-updates
        PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/include/PlatformUpdatesWindows.h"
        "${CMAKE_CURRENT_LIST_DIR}/src/PlatformUpdatesWindows.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/src/winsparkle.qrc"
         )

target_include_directories(platform-updates
        PRIVATE
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>"
        "$<BUILD_INTERFACE:${PREBUILT_DIR}/${VS_PLATFORM}/winsparkle/include>"
        )

target_link_libraries(platform-updates
        PUBLIC
        platform-updates-api
        customer-config
        )

# ---------------------------------------------------------------------------
#   Main target
# ---------------------------------------------------------------------------
add_executable(${VS_TARGET_NAME})

# ---------------------------------------------------------------------------
#   Configure deployment files.
# ---------------------------------------------------------------------------
configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/messenger.nsi.in"
        "${CMAKE_CURRENT_BINARY_DIR}/messenger.nsi"
        @ONLY
        )

configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/appcast.xml.in"
        "${CMAKE_CURRENT_BINARY_DIR}/appcast.xml"
        @ONLY
        )

# ---------------------------------------------------------------------------
#   Deploy
# ---------------------------------------------------------------------------
find_program(DEPLOY_QT cqtdeployer)

set(DEPLOY_DIR "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.dist")
add_custom_target(deploy
        COMMENT "Running QT deployer for Windows..."
        COMMAND ${DEPLOY_QT}
            -bin "$<TARGET_FILE:${VS_TARGET_NAME}>"
            -qmlDir "${PROJECT_SOURCE_DIR}/src/qml"
            -targetDir "${DEPLOY_DIR}"
            -qmake "${QT_QMAKE_EXECUTABLE}" clear
        COMMAND cp -f "${VS_CUSTOMER_DIR}/platforms/windows/Logo.ico" "${DEPLOY_DIR}"
        COMMAND cp -f /usr/x86_64-w64-mingw32/sys-root/mingw/bin/libcrypto-1_1-x64.dll "${DEPLOY_DIR}/lib/"
        COMMAND cp -f /usr/x86_64-w64-mingw32/sys-root/mingw/bin/libcurl-4.dll         "${DEPLOY_DIR}/lib/"
        COMMAND cp -f /usr/x86_64-w64-mingw32/sys-root/mingw/bin/libidn2-0.dll         "${DEPLOY_DIR}/lib/"
        COMMAND cp -f /usr/x86_64-w64-mingw32/sys-root/mingw/bin/libssh2-1.dll         "${DEPLOY_DIR}/lib/"
        COMMAND cp -f /usr/x86_64-w64-mingw32/sys-root/mingw/bin/libssl-1_1-x64.dll    "${DEPLOY_DIR}/lib/"
        COMMAND cp -f /usr/x86_64-w64-mingw32/sys-root/mingw/bin/libssp-0.dll          "${DEPLOY_DIR}/lib/"
        COMMAND cp -f /usr/x86_64-w64-mingw32/sys-root/mingw/bin/zlib1.dll             "${DEPLOY_DIR}/lib/"
        COMMAND cp -f
            "${PROJECT_SOURCE_DIR}/ext/prebuilt/windows/winsparkle/x64/Release/WinSparkle.dll"
            "${DEPLOY_DIR}/lib/"
        COMMAND sed -i "s!Prefix= ./../!Prefix= ./!g" "${DEPLOY_DIR}/bin/qt.conf"
        COMMAND bash -c "cp -rf ${DEPLOY_DIR}/bin/* ${DEPLOY_DIR}/"
        COMMAND bash -c "cp -rf ${DEPLOY_DIR}/lib/* ${DEPLOY_DIR}/"
        COMMAND rm -f "${DEPLOY_DIR}/*.bat"
        COMMAND rm -rf "${DEPLOY_DIR}/bin"
        COMMAND rm -rf "${DEPLOY_DIR}/lib"
        COMMAND makensis messenger.nsi
        COMMAND "${CMAKE_CURRENT_LIST_DIR}/tools/signing_win.sh"
            "${PROJECT_BINARY_DIR}/${WIN_SU_FILE}"
            "${VS_WINSPARKE_KEY}"
        )
