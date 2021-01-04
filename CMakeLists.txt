#   Copyright (C) 2015-2020 Virgil Security Inc.
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


cmake_minimum_required(VERSION 3.17 FATAL_ERROR)

# ---------------------------------------------------------------------------
#   Project
# ---------------------------------------------------------------------------
project(virgil-messenger-qt VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---------------------------------------------------------------------------
#   Version
# ---------------------------------------------------------------------------
file(WRITE "${CMAKE_CURRENT_LIST_DIR}/VERSION_MESSENGER" "${virgil-messenger-qt_VERSION}")

# ---------------------------------------------------------------------------
#   Helper modules
# ---------------------------------------------------------------------------
include(GNUInstallDirs)
include(FetchContent)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake" ${CMAKE_MODULE_PATH})
include (clangformat)
include (helpers)
if (APPLE)
    include (macdeploy)
endif()


# ---------------------------------------------------------------------------
#   Options
# ---------------------------------------------------------------------------

#
# Test
#
option(ENABLE_TESTING "On/Off integration tests." ON)

#
#   macOS signing certificate
#
if (APPLE)
    option(DMG "On/Off DMG build." OFF)
    set(SIGN_CERT "" CACHE STRING "macOS Signing certificate")
    set(SIGN_CERT_ID "" CACHE STRING "macOS Signing certificate ID")

    if(SIGN_CERT STREQUAL "")
        message(FATAL_ERROR "SIGN_CERT must be set. Take a look 'security find-identity -p codesigning'. You have to add something like-DSIGN_CERT=\"Apple Development: kutashenko@gmail.com (C63Q8K74TF)\"")
    endif()

    if(SIGN_CERT_ID STREQUAL "")
        message(FATAL_ERROR "SIGN_CERT_ID must be set. Take a look 'security find-identity -p codesigning'. You have to add -DSIGN_CERT_ID=<hex value>")
    endif()
endif()

# ---------------------------------------------------------------------------
#   Project targets
# ---------------------------------------------------------------------------

#
#   Qt
#
find_package(Qt5 COMPONENTS Core Widgets Sql Network Concurrent Qml Xml Bluetooth WebSockets)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

#
#   Check submodules
#
if(NOT EXISTS "${CMAKE_CURRENT_LIST_DIR}/ext/quickfuture/.git")
    message(FATAL_ERROR
        "Git submodule 'ext/quickfuture' is not initialized.\n"
        " Run command 'git submodule update --init --recursive'")
endif()

if(NOT EXISTS "${CMAKE_CURRENT_LIST_DIR}/ext/quickpromise/.git")
    message(FATAL_ERROR
        "Git submodule 'ext/quickpromise' is not initialized.\n"
        "Run command 'git submodule update --init --recursive'")
endif()

#
#   Fetch prebuilt core libraries
#
file(STRINGS "${CMAKE_CURRENT_LIST_DIR}/VERSION_CORE" VERSION_CORE_CONTENT)
list(GET VERSION_CORE_CONTENT 0 VERSION_CORE)
set(PREBUILT_CORE_ARCHIVE "${CMAKE_CURRENT_LIST_DIR}/ext/downloads/prebuilt-${VERSION_CORE}.tgz")

if(NOT EXISTS "${PREBUILT_CORE_ARCHIVE}")
    file(DOWNLOAD
        "https://virgilsecurity.bintray.com/iotl-demo-cdn/prebuilt-${VERSION_CORE}.tgz"
        "${PREBUILT_CORE_ARCHIVE}"
        SHOW_PROGRESS
    )
endif()

#
#   Fetch prebuilt firebase library
#
option(PREBUILT_SKIP_FETCH "Skip fetching and installing prebilt binaries" OFF)
if(NOT PREBUILT_SKIP_FETCH)
    set(PREBUILT_FIREBASE_SDK_VERSION "6.11.0")
    set(PREBUILT_FIREBASE_SDK_ARCHIVE
        "${CMAKE_CURRENT_LIST_DIR}/ext/downloads/firebase_cpp_sdk_${PREBUILT_FIREBASE_SDK_VERSION}.zip"
    )

    if(NOT EXISTS "${PREBUILT_FIREBASE_SDK_ARCHIVE}")
        file(DOWNLOAD
            "https://dl.google.com/firebase/sdk/cpp/firebase_cpp_sdk_${PREBUILT_FIREBASE_SDK_VERSION}.zip"
            "${PREBUILT_FIREBASE_SDK_ARCHIVE}"
            SHOW_PROGRESS
        )
    endif()

    execute_process(
        COMMAND ${CMAKE_COMMAND} -E rm -fr -- *
        COMMAND ${CMAKE_COMMAND} -E tar xzf "${PREBUILT_CORE_ARCHIVE}"
        WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/ext"
    )

    execute_process(
        COMMAND ${CMAKE_COMMAND} -E tar xzf "${PREBUILT_FIREBASE_SDK_ARCHIVE}"
        WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/ext/prebuilt"
    )
endif()

#
#   Define prebuilt folders
#
if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(PREBUILT_PLATFORM_NAME "macos")

elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(PREBUILT_PLATFORM_NAME "linux")

elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(PREBUILT_PLATFORM_NAME "windows")

endif()

if(NOT PREBUILT_PLATFORM_NAME)
    message(FATAL_ERROR "Build for ${CMAKE_SYSTEM_NAME} currently is not supported via CMake.")
endif()

set(PREBUILT_COMMON_DIR "${CMAKE_CURRENT_LIST_DIR}/ext/prebuilt")
set(PREBUILT_QT_DIR "${PREBUILT_COMMON_DIR}/qt")

string(TOLOWER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE_LOWER)
if(CMAKE_BUILD_TYPE_LOWER STREQUAL "debug")
    set(PREBUILT_BASE_DIR "${PREBUILT_COMMON_DIR}/${PREBUILT_PLATFORM_NAME}/debug/installed/usr/local")
else()
    set(PREBUILT_BASE_DIR "${PREBUILT_COMMON_DIR}/${PREBUILT_PLATFORM_NAME}/release/installed/usr/local")
endif()

set(PREBUILT_INCLUDE_DIR "${PREBUILT_BASE_DIR}/include")
set(PREBUILT_LIB_DIR "${PREBUILT_BASE_DIR}/lib")

#
#   QuickFuture
#
set(QUICKFUTURE_DIR "${CMAKE_CURRENT_LIST_DIR}/ext/quickfuture/src")

#
#   QT HAL for IotKit
#
add_library(qt-iot-hal)
target_sources(qt-iot-hal
        PRIVATE

        # Headers
        "${PREBUILT_QT_DIR}/config/pc/stdlib-config.h"
        "${PREBUILT_QT_DIR}/config/pc/endian-config.h"
        "${PREBUILT_QT_DIR}/config/pc/global-hal.h"
        "${PREBUILT_QT_DIR}/config/pc/logger-config.h"
        "${PREBUILT_QT_DIR}/config/pc/trust_list-config.h"
        "${PREBUILT_QT_DIR}/config/pc/update-config.h"
        "${PREBUILT_QT_DIR}/default-impl/netif/websocket/include/virgil/iot/qt/netif/VSQNetifWebsocket.h"
        "${PREBUILT_QT_DIR}/default-impl/netif/ble/include/virgil/iot/qt/netif/VSQNetifBLEEnumerator.h"
        "${PREBUILT_QT_DIR}/default-impl/netif/ble/include/virgil/iot/qt/netif/VSQNetifBLE.h"
        "${PREBUILT_QT_DIR}/default-impl/netif/udp-broadcast/include/virgil/iot/qt/netif/VSQUdpBroadcast.h"
        "${PREBUILT_QT_DIR}/facade/include/virgil/iot/qt/VSQIoTKit.h"
        "${PREBUILT_QT_DIR}/facade/include/virgil/iot/qt/protocols/snap/VSQSnapCFGClient.h"
        "${PREBUILT_QT_DIR}/facade/include/virgil/iot/qt/protocols/snap/VSQSnapServiceBase.h"
        "${PREBUILT_QT_DIR}/facade/include/virgil/iot/qt/protocols/snap/VSQSnapINFOClientQml.h"
        "${PREBUILT_QT_DIR}/facade/include/virgil/iot/qt/protocols/snap/VSQSnapINFOClient.h"
        "${PREBUILT_QT_DIR}/facade/include/virgil/iot/qt/protocols/snap/VSQSnapSnifferQml.h"
        "${PREBUILT_QT_DIR}/facade/include/virgil/iot/qt/protocols/snap/VSQNetifBase.h"
        "${PREBUILT_QT_DIR}/facade/include/virgil/iot/qt/helpers/VSQSingleton.h"
        "${PREBUILT_QT_DIR}/facade/include/virgil/iot/qt/helpers/VSQImplementations.h"
        "${PREBUILT_QT_DIR}/facade/include/virgil/iot/qt/helpers/VSQManufactureId.h"
        "${PREBUILT_QT_DIR}/facade/include/virgil/iot/qt/helpers/VSQFeatures.h"
        "${PREBUILT_QT_DIR}/facade/include/virgil/iot/qt/helpers/VSQDeviceRoles.h"
        "${PREBUILT_QT_DIR}/facade/include/virgil/iot/qt/helpers/VSQDeviceSerial.h"
        "${PREBUILT_QT_DIR}/facade/include/virgil/iot/qt/helpers/VSQFileVersion.h"
        "${PREBUILT_QT_DIR}/facade/include/virgil/iot/qt/helpers/VSQDeviceType.h"
        "${PREBUILT_QT_DIR}/facade/include/virgil/iot/qt/helpers/VSQAppConfig.h"
        "${PREBUILT_QT_DIR}/facade/include/virgil/iot/qt/helpers/VSQMac.h"
        "${PREBUILT_QT_DIR}/facade/include/virgil/iot/qt/helpers/VSQIoTKitFacade.h"
        "${PREBUILT_QT_DIR}/facade/include/virgil/iot/qt/helpers/VSQHelpers.h"

        # Sources
        "${PREBUILT_QT_DIR}/default-impl/hal.cpp"
        "${PREBUILT_QT_DIR}/default-impl/netif/websocket/src/VSQNetifWebsocket.cpp"
        "${PREBUILT_QT_DIR}/default-impl/netif/ble/src/VSQNetifBLEEnumerator.cpp"
        "${PREBUILT_QT_DIR}/default-impl/netif/ble/src/VSQNetifBLE.cpp"
        "${PREBUILT_QT_DIR}/default-impl/netif/udp-broadcast/src/VSQUdpBroadcast.cpp"
        "${PREBUILT_QT_DIR}/facade/src/VSQSnapSnifferQml.cpp"
        "${PREBUILT_QT_DIR}/facade/src/VSQSnapINFOClient.cpp"
        "${PREBUILT_QT_DIR}/facade/src/VSQNetifBase.cpp"
        "${PREBUILT_QT_DIR}/facade/src/helpers/VSQDeviceType.cpp"
        "${PREBUILT_QT_DIR}/facade/src/helpers/VSQDeviceRoles.cpp"
        "${PREBUILT_QT_DIR}/facade/src/helpers/VSQMac.cpp"
        "${PREBUILT_QT_DIR}/facade/src/helpers/VSQManufactureId.cpp"
        "${PREBUILT_QT_DIR}/facade/src/helpers/VSQIoTKitFacade.cpp"
        "${PREBUILT_QT_DIR}/facade/src/helpers/VSQFileVersion.cpp"
        "${PREBUILT_QT_DIR}/facade/src/helpers/VSQDeviceSerial.cpp"
        "${PREBUILT_QT_DIR}/facade/src/VSQSnapCFGClient.cpp"
        "${PREBUILT_QT_DIR}/facade/src/VSQSnapINFOClientQml.cpp"
        )

target_include_directories(qt-iot-hal
        PUBLIC
        $<BUILD_INTERFACE:${PREBUILT_INCLUDE_DIR}>
        $<BUILD_INTERFACE:${PREBUILT_INCLUDE_DIR}/qxmpp>

        $<BUILD_INTERFACE:${PREBUILT_QT_DIR}>
        $<BUILD_INTERFACE:${PREBUILT_QT_DIR}/config/pc>
        $<BUILD_INTERFACE:${PREBUILT_QT_DIR}/facade/include>
        $<BUILD_INTERFACE:${PREBUILT_QT_DIR}/default-impl/netif/ble/include>
        $<BUILD_INTERFACE:${PREBUILT_QT_DIR}/default-impl/netif/udp-broadcast/include>
        $<BUILD_INTERFACE:${PREBUILT_QT_DIR}/default-impl/netif/websocket/include>
        )


target_compile_definitions(qt-iot-hal
        PUBLIC
        $<BUILD_INTERFACE:INFO_CLIENT=1>
        $<BUILD_INTERFACE:CFG_CLIENT=1>
        )

target_link_libraries(qt-iot-hal
        PUBLIC

        # Qt5
        Qt5::Core
        Qt5::Network
        Qt::Bluetooth
        Qt::WebSockets
        Qt::Qml

        # IotKit
        vs-module-logger
        vs-module-provision
        vs-module-snap-control
        vs-module-secbox
        vs-module-messenger

        # Compiler options
        enable_sanitizers
        )

#
#   Application
#
if (APPLE)
    add_executable(virgil-messenger-qt MACOSX_BUNDLE)
    set_target_properties(virgil-messenger-qt
        PROPERTIES MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_LIST_DIR}/platforms/macos/virgil-messenger.plist"
    )
    set(MACOSX_DEPLOYMENT_TARGET 10.14)
#    macdeployqt(virgil-messenger-qt "${CMAKE_CURRENT_LIST_DIR}/src/qml")
else ()
    add_executable(virgil-messenger-qt)
endif ()

target_sources(virgil-messenger-qt
        PRIVATE

        # Headers
        ${CMAKE_CURRENT_LIST_DIR}/include/VSQApplication.h
        ${CMAKE_CURRENT_LIST_DIR}/include/VSQClipboardProxy.h
        ${CMAKE_CURRENT_LIST_DIR}/include/Messenger.h
        ${CMAKE_CURRENT_LIST_DIR}/include/VSQPushNotifications.h
        ${CMAKE_CURRENT_LIST_DIR}/include/VSQSqlChatModel.h
        ${CMAKE_CURRENT_LIST_DIR}/include/VSQSqlConversationModel.h
        ${CMAKE_CURRENT_LIST_DIR}/include/VSQLogging.h
        ${CMAKE_CURRENT_LIST_DIR}/include/ui/VSQUiHelper.h
        ${CMAKE_CURRENT_LIST_DIR}/include/macos/VSQMacos.h

        #   QuickFuture
        ${QUICKFUTURE_DIR}/qffuture.h
        ${QUICKFUTURE_DIR}/qfvariantwrapper.h
        ${QUICKFUTURE_DIR}/QuickFuture
        ${QUICKFUTURE_DIR}/quickfuture.h

        # Sources
        ${CMAKE_CURRENT_LIST_DIR}/src/main.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/VSQApplication.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/VSQClipboardProxy.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/Messenger.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/VSQPushNotifications.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/VSQSqlChatModel.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/VSQSqlConversationModel.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/VSQLogging.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/ui/VSQUiHelper.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/macos/VSQMacos.mm
        ${PREBUILT_QT_DIR}/default-impl/hal.cpp

        #   QuickFuture
        ${QUICKFUTURE_DIR}/qffuture.cpp

        # Qt Resources
        ${CMAKE_CURRENT_LIST_DIR}/src/resources.qrc
        )

target_link_libraries(virgil-messenger-qt
        PUBLIC

        # Qt5
        Qt5::Core
        Qt5::Widgets
        Qt5::Sql
        Qt5::Network
        Qt5::Concurrent
        Qt5::Qml
        Qt5::Xml

        # Other libs
        qxmpp
        vs-module-logger
        vs-module-messenger
        vs-messenger-internal

        qt-iot-hal

        # Compiler options
        enable_pedantic_mode
        enable_sanitizers
        )

#
#   Common include directories
#
target_include_directories(virgil-messenger-qt
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
        $<BUILD_INTERFACE:${PREBUILT_INCLUDE_DIR}>
        $<BUILD_INTERFACE:${PREBUILT_INCLUDE_DIR}/qxmpp>
        $<BUILD_INTERFACE:${QUICKFUTURE_DIR}>
        )

target_compile_definitions(virgil-messenger-qt
    PRIVATE
    $<BUILD_INTERFACE:INFO_CLIENT=1>
    $<BUILD_INTERFACE:CFG_CLIENT=1>
)

#
#   Common link directories
#
target_link_directories(virgil-messenger-qt
        PRIVATE
        $<BUILD_INTERFACE:${PREBUILT_LIB_DIR}>
        )

install(TARGETS virgil-messenger-qt
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        BUNDLE DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
        )

# if (COMMAND add_clangformat)
#     add_clangformat(virgil-messenger-qt)
# endif ()

# ---------------------------------------------------------------------------
#   Done
# ---------------------------------------------------------------------------
