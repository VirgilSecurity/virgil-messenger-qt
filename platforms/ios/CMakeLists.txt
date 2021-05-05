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

#
#   Provide default (Qt) implementation for platform dependent operations.
#
cmake_minimum_required(VERSION 3.16 FATAL_ERROR)


#
#   System introspection.
#
if (NOT XCODE_VERSION)
        message(FATAL_ERROR "Xcode only")
endif ()

find_package(Qt5 COMPONENTS QmlImportScanner Gui REQUIRED)

#
#   Library: platform-deps.
#
add_library(platform-deps INTERFACE)
target_link_libraries(platform-deps
        INTERFACE
        Qt5::Gui
        Qt5::QIosOptionalPlugin_NSPhotoLibrary
        )

#
#   Library: platform.
#
add_library(platform STATIC)

target_sources(platform
        PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/include/PlatformIos.h"
        "${CMAKE_CURRENT_LIST_DIR}/src/PlatformIos.mm"

        # TODO: Move to the include/src.
        # TODO: Hide implementation under platform API.
        "${CMAKE_CURRENT_LIST_DIR}/AppDelegate.mm"
        "${CMAKE_CURRENT_LIST_DIR}/IosDocumentInteractionController.h"
        "${CMAKE_CURRENT_LIST_DIR}/IosDocumentInteractionController.mm"
        "${CMAKE_CURRENT_LIST_DIR}/IosViewController.h"
        "${CMAKE_CURRENT_LIST_DIR}/IosViewController.mm"
        )

target_include_directories(platform
        PUBLIC
        # TODO: When implementation will be hidden under platform API, then remove it.
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/../../include/controllers>"

        PRIVATE
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>"
        )

target_link_libraries(platform
        PUBLIC
        platform-base
        customer-config
        )

#
#   Library: platform-fs.
#
add_library(platform-fs STATIC)

target_sources(platform-fs
        PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/include/PlatformFsIos.h"
        "${CMAKE_CURRENT_LIST_DIR}/src/PlatformFsIos.cpp"
        )

target_include_directories(platform-fs
        PRIVATE
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>"
        )

target_link_libraries(platform-fs
        PUBLIC
        platform-fs-base
        )
