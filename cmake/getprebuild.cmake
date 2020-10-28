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

cmake_minimum_required(VERSION 3.16 FATAL_ERROR)

#
#   Fetch prebuilt core libraries
#
function(get_prebuild)
    set(PREBUILT_CORE_ARCHIVE "${CMAKE_CURRENT_LIST_DIR}/ext/downloads/prebuilt-${VS_CORE_VERSION}.tgz")
    message(STATUS "Get prebuild libraries [https://virgilsecurity.bintray.com/iotl-demo-cdn/prebuilt-${VS_CORE_VERSION}.tgz]...")
    if(NOT EXISTS "${PREBUILT_CORE_ARCHIVE}")
	file(DOWNLOAD
            "https://virgilsecurity.bintray.com/iotl-demo-cdn/prebuilt-${VS_CORE_VERSION}.tgz"
            "${PREBUILT_CORE_ARCHIVE}"
            SHOW_PROGRESS
        )
	message(STATUS "Unpacking ...")
	execute_process(
            COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_CURRENT_LIST_DIR}/ext/prebuild 
            COMMAND ${CMAKE_COMMAND} -E tar xzf "${PREBUILT_CORE_ARCHIVE}"
            WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/ext"
        )        
    endif()
endfunction()

#
#   Fetch prebuilt firebase library
#
function(get_firebase)
    set(PREBUILT_FIREBASE_SDK_VERSION "6.11.0")
    set(PREBUILT_FIREBASE_SDK_ARCHIVE
        "${CMAKE_CURRENT_LIST_DIR}/ext/downloads/firebase_cpp_sdk_${PREBUILT_FIREBASE_SDK_VERSION}.zip"
    )
    message(STATUS "Get prebuild firebase [https://dl.google.com/firebase/sdk/cpp/firebase_cpp_sdk_${PREBUILT_FIREBASE_SDK_VERSION}.zip]...")
    if(NOT EXISTS "${PREBUILT_FIREBASE_SDK_ARCHIVE}")
        file(DOWNLOAD
            "https://dl.google.com/firebase/sdk/cpp/firebase_cpp_sdk_${PREBUILT_FIREBASE_SDK_VERSION}.zip"
            "${PREBUILT_FIREBASE_SDK_ARCHIVE}"
            SHOW_PROGRESS
        )
    execute_process(
        COMMAND ${CMAKE_COMMAND} -E tar xzf "${PREBUILT_FIREBASE_SDK_ARCHIVE}"
        WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/ext/prebuilt"
    )
    endif()
endfunction()

