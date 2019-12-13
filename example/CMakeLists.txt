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

cmake_minimum_required(VERSION 3.11)

project(iotkit-qt-example)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt5 COMPONENTS Qml REQUIRED)
find_package(Qt5 COMPONENTS Quick REQUIRED)

if(ANDROID)
    add_library(iotkit-qt-example SHARED)
else()
    add_executable(iotkit-qt-example)
endif()

target_sources(iotkit-qt-example
        PRIVATE

        # Headers
        ${CMAKE_CURRENT_LIST_DIR}/include/VSQApp.h
        ${CMAKE_CURRENT_LIST_DIR}/include/VSQController.h
        ${CMAKE_CURRENT_LIST_DIR}/include/VSQDeviceInfoController.h
        ${CMAKE_CURRENT_LIST_DIR}/include/VSQDeviceInfoModel.h

        # Sources
        ${CMAKE_CURRENT_LIST_DIR}/src/VSQApp.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/main.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/VSQController.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/VSQDeviceInfoController.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/VSQDeviceInfoModel.cpp

        # Resources
        ${CMAKE_CURRENT_LIST_DIR}/src/qml/resources.qrc
        )

target_include_directories(iotkit-qt-example
        PRIVATE

        ${CMAKE_CURRENT_LIST_DIR}/include
        )

target_link_libraries(iotkit-qt-example
        PRIVATE

        iotkit-qt-netif-udp-broadcast
        iotkit-qt-hal-global-impl
        iotkit-qt
        iotkit-qt-hal
        iotkit-qt-info-client

        Qt5::Widgets
        Qt5::Qml
        Qt5::Quick
        )

if(COMMAND add_clangformat)
    add_clangformat(iotkit-qt-example)
endif()

if(ANDROID)
    message("ANDROID configuration")

    #    set(ANDROID_SDK_ROOT $ANDROID_SDK)
    #    set(CMAKE_ANDROID_ARCH_ABI armeabi-v7a)
    #    set(CMAKE_ANDROID_STL_TYPE gnustl_shared)
    #    set(CMAKE_CXX_COMPILER $NDK_ROOT/toolchains/arm-linux-androideabi-4.9/prebuilt/linux-x86_64/bin/arm-linux-androideabi-g++)
    #    set(CMAKE_C_COMPILER:STRING=$NDK_ROOT/toolchains/arm-linux-androideabi-4.9/prebuilt/linux-x86_64/bin/arm-linux-androideabi-gcc)
    #    set(CMAKE_PREFIX_PATH $QT_ROOT)
    #    set(CMAKE_SYSTEM_NAME Android)
    #    set(QT_QMAKE_EXECUTABLE $QT_ROOT/bin/qmake)


    set(CMAKE_ANDROID_SDK "/Users/oleksandr/Android/SDK")
    set(CMAKE_ANDROID_STL_TYPE "c++_shared")
    set(ANDROID_SDK_ROOT ${CMAKE_ANDROID_SDK})
    set(CMAKE_ANDROID_NDK "/Users/oleksandr/Android/SDK/ndk/20.1.5948944")
    message("ANDROID_SDK_ROOT = ${ANDROID_SDK_ROOT}")
    include(../ext/virgil-iotkit/cmake/android/qt-android-mk-apk.cmake)
    qt_android_build_apk(
            TARGET iotkit-qt-example
            PACKAGE_NAME org.example.QmlCmakeDemo
            QML_ROOT_PATH ${CMAKE_CURRENT_SOURCE_DIR}
            ANDROID_EXTRA_FILES ${CMAKE_CURRENT_SOURCE_DIR}/android
    )
endif()





