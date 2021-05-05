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
        WinSparkle
        curl
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
        )

# ---------------------------------------------------------------------------
#   Deploy
# ---------------------------------------------------------------------------
find_program(DEPLOY_QT cqtdeployer)

add_custom_target(deploy
        COMMAND ${DEPLOY_QT}
        -bin ${PROJECT_NAME}.exe
        -qmlDir ${PROJECT_SOURCE_DIR}/src/qml
        -targetDir ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.dist
        -qmake ${QT_QMAKE_EXECUTABLE} clear
        COMMAND cp -f "${VS_CUSTOMER_DIR}/platforms/windows/Logo.ico" "${CMAKE_BINARY_DIR}/${PROJECT_NAME}.dist"
        COMMAND cp -f /usr/x86_64-w64-mingw32/sys-root/mingw/bin/libcrypto-1_1-x64.dll                    "${CMAKE_BINARY_DIR}/${PROJECT_NAME}.dist/lib/"
        COMMAND cp -f /usr/x86_64-w64-mingw32/sys-root/mingw/bin/libcurl-4.dll                            "${CMAKE_BINARY_DIR}/${PROJECT_NAME}.dist/lib/"
        COMMAND cp -f /usr/x86_64-w64-mingw32/sys-root/mingw/bin/libidn2-0.dll                            "${CMAKE_BINARY_DIR}/${PROJECT_NAME}.dist/lib/"
        COMMAND cp -f /usr/x86_64-w64-mingw32/sys-root/mingw/bin/libssh2-1.dll                            "${CMAKE_BINARY_DIR}/${PROJECT_NAME}.dist/lib/"
        COMMAND cp -f /usr/x86_64-w64-mingw32/sys-root/mingw/bin/libssl-1_1-x64.dll                       "${CMAKE_BINARY_DIR}/${PROJECT_NAME}.dist/lib/"
        COMMAND cp -f /usr/x86_64-w64-mingw32/sys-root/mingw/bin/libssp-0.dll                             "${CMAKE_BINARY_DIR}/${PROJECT_NAME}.dist/lib/"
        COMMAND cp -f /usr/x86_64-w64-mingw32/sys-root/mingw/bin/zlib1.dll                                "${CMAKE_BINARY_DIR}/${PROJECT_NAME}.dist/lib/"
        COMMAND cp -f "${PROJECT_SOURCE_DIR}/ext/prebuilt/windows/winsparkle/x64/Release/WinSparkle.dll"  "${CMAKE_BINARY_DIR}/${PROJECT_NAME}.dist/lib/"
        COMMAND sed -i "s!Prefix= ./../!Prefix= ./!g" "${CMAKE_BINARY_DIR}/${PROJECT_NAME}.dist/bin/qt.conf"
        COMMAND bash -c "cp -rf ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.dist/bin/* ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.dist/"
        COMMAND bash -c "cp -rf ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.dist/lib/* ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.dist/"
        COMMAND rm -f "${CMAKE_BINARY_DIR}/${PROJECT_NAME}.dist/*.bat"
        COMMAND rm -rf "${CMAKE_BINARY_DIR}/${PROJECT_NAME}.dist/bin"
        COMMAND rm -rf "${CMAKE_BINARY_DIR}/${PROJECT_NAME}.dist/lib"
        COMMAND makensis messenger.nsi
        COMMAND ${PROJECT_SOURCE_DIR}/platforms/windows/tools/signing_win.sh "${CMAKE_BINARY_DIR}/${WIN_SU_FILE}" "${VS_WINSPARKE_KEY}"
        VERBATIM)
