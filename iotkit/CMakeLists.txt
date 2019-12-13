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

#
#   Add services
#
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/services)

add_library(iotkit-qt)

target_compile_definitions(iotkit-qt
        PRIVATE
        USE_INFO_CLIENT=$<BOOL:${VIRGIL_IOTKIT_QT_INFO_CLIENT}>
        )

target_sources(iotkit-qt
        PRIVATE

        # Headers
        ${CMAKE_CURRENT_LIST_DIR}/include/VSQAppConfig.h
        ${CMAKE_CURRENT_LIST_DIR}/include/VSQDeviceRoles.h
        ${CMAKE_CURRENT_LIST_DIR}/include/VSQDeviceSerial.h
        ${CMAKE_CURRENT_LIST_DIR}/include/VSQDeviceType.h
        ${CMAKE_CURRENT_LIST_DIR}/include/VSQFeatures.h
        ${CMAKE_CURRENT_LIST_DIR}/include/VSQFileVersion.h
        ${CMAKE_CURRENT_LIST_DIR}/include/VSQImplementations.h
        ${CMAKE_CURRENT_LIST_DIR}/include/VSQIoTKit.h
        ${CMAKE_CURRENT_LIST_DIR}/include/VSQIoTKitFacade.h
        ${CMAKE_CURRENT_LIST_DIR}/include/VSQMac.h
        ${CMAKE_CURRENT_LIST_DIR}/include/VSQManufactureId.h
        ${CMAKE_CURRENT_LIST_DIR}/include/VSQSingleton.h

        # Sources
        ${CMAKE_CURRENT_LIST_DIR}/src/VSQDeviceRoles.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/VSQDeviceSerial.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/VSQDeviceType.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/VSQFileVersion.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/VSQIoTKitFacade.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/VSQMac.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/VSQManufactureId.cpp
        )

target_include_directories(iotkit-qt
        PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/include
        )

target_link_libraries(iotkit-qt
        PUBLIC

        vs-module-snap-control
        vs-module-provision
        iotkit-qt-hal
        iotkit-qt-info-client

        Qt5::Core
        Qt5::Network
        )

if(COMMAND add_clangformat)
    add_clangformat(iotkit-qt)
endif()

