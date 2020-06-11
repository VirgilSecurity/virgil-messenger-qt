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


cmake_minimum_required(VERSION 3.11 FATAL_ERROR)

project(virgil-messenger-qt VERSION 0.1.0 LANGUAGES C CXX)

#
#   Qt
#
find_package(Qt5 COMPONENTS Core Widgets Sql Network Concurrent Qml Xml)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

#
#   QuickFuture
#
set(QUICKFUTURE_DIR "${CMAKE_CURRENT_LIST_DIR}/../ext/quickfuture/src")

#
#   Prebuilt folders
#
set(PREBUILT_COMMON_DIR "${CMAKE_CURRENT_LIST_DIR}/../ext/prebuilt")
set(PREBUILT_QT_DIR "${PREBUILT_COMMON_DIR}/qt")
set(PREBUILT_BASE_DIR "${PREBUILT_COMMON_DIR}/${VS_OS}/release/installed/usr/local")
set(PREBUILT_INCLUDE_DIR "${PREBUILT_BASE_DIR}/include")
set(PREBUILT_LIB_DIR "${PREBUILT_BASE_DIR}/lib")

#
#   Application
#
if (APPLE)
    add_executable(virgil-messenger-qt MACOSX_BUNDLE)
    set_target_properties(virgil-messenger-qt PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_LIST_DIR}/../platforms/macos/virgil-messenger.plist)
    set(MACOSX_DEPLOYMENT_TARGET 10.14)
    macdeployqt(virgil-messenger-qt "${CMAKE_CURRENT_LIST_DIR}/src/qml")
else ()
    add_executable(virgil-messenger-qt)
endif ()

target_sources(virgil-messenger-qt
        PRIVATE

        # Headers
        ${CMAKE_CURRENT_LIST_DIR}/../include/VSQApplication.h
        ${CMAKE_CURRENT_LIST_DIR}/../include/VSQClipboardProxy.h
        ${CMAKE_CURRENT_LIST_DIR}/../include/VSQMessenger.h
        ${CMAKE_CURRENT_LIST_DIR}/../include/VSQPushNotifications.h
        ${CMAKE_CURRENT_LIST_DIR}/../include/VSQSqlChatModel.h
        ${CMAKE_CURRENT_LIST_DIR}/../include/VSQSqlConversationModel.h
        ${CMAKE_CURRENT_LIST_DIR}/../include/ui/VSQUiHelper.h
        ${CMAKE_CURRENT_LIST_DIR}/../include/macos/VSQMacos.h

        #   QuickFuture
        ${QUICKFUTURE_DIR}/qffuture.h
        ${QUICKFUTURE_DIR}/qfvariantwrapper.h
        ${QUICKFUTURE_DIR}/QuickFuture
        ${QUICKFUTURE_DIR}/quickfuture.h

        # Sources
        ${CMAKE_CURRENT_LIST_DIR}/main.cpp
        ${CMAKE_CURRENT_LIST_DIR}/VSQApplication.cpp
        ${CMAKE_CURRENT_LIST_DIR}/VSQClipboardProxy.cpp
        ${CMAKE_CURRENT_LIST_DIR}/VSQMessenger.cpp
        ${CMAKE_CURRENT_LIST_DIR}/VSQPushNotifications.cpp
        ${CMAKE_CURRENT_LIST_DIR}/VSQSqlChatModel.cpp
        ${CMAKE_CURRENT_LIST_DIR}/VSQSqlConversationModel.cpp

        #   QuickFuture
        ${QUICKFUTURE_DIR}/qffuture.cpp

        # Qt Resources
        ${CMAKE_CURRENT_LIST_DIR}/resources.qrc
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

        # Compiler options
        enable_pedantic_mode
        )

#
#   Common include directories
#
target_include_directories(virgil-messenger-qt
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/../include>
        $<BUILD_INTERFACE:${PREBUILT_INCLUDE_DIR}>
        $<BUILD_INTERFACE:${PREBUILT_INCLUDE_DIR}/qxmpp>
        $<BUILD_INTERFACE:${PREBUILT_QT_DIR}/config/pc>
        $<BUILD_INTERFACE:${QUICKFUTURE_DIR}>
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

if (COMMAND add_clangformat)
    add_clangformat(virgil-messenger-qt)
endif ()
