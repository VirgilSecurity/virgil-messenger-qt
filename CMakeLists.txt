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

set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")
cmake_minimum_required(VERSION 3.11 FATAL_ERROR)

include(GNUInstallDirs)

# ---------------------------------------------------------------------------
#   Project
# ---------------------------------------------------------------------------

project(virgil-iotkit-qt VERSION 0.1.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---------------------------------------------------------------------------
#   Qt
# ---------------------------------------------------------------------------
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt5 COMPONENTS Quick REQUIRED)

# ---------------------------------------------------------------------------
#   Helper modules
# ---------------------------------------------------------------------------
set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/ext/virgil-iotkit/cmake" ${CMAKE_MODULE_PATH})
include (clangformat)
include (helpers)

# ---------------------------------------------------------------------------
#   Version options
# ---------------------------------------------------------------------------
set(VIRGIL_IOTKIT_QT_VERSION_LABEL "dev" CACHE STRING "Version label, i.e. beta, rc1.")

file(WRITE "${CMAKE_CURRENT_LIST_DIR}/VERSION" "${virgil-iotkit-qt-VERSION}")

if(VIRGIL_IOT_SDK_VERSION_LABEL)
    file(APPEND "${CMAKE_CURRENT_LIST_DIR}/VERSION" "-${VIRGIL_IOTKIT_QT_VERSION_LABEL}")
endif()

file(APPEND "${CMAKE_CURRENT_LIST_DIR}/VERSION" "\n")

# ---------------------------------------------------------------------------
#   Build options
# ---------------------------------------------------------------------------

#
#   Use Qt Implementation
#
set(VIRGIL_IOT_QT ON CACHE BOOL "Use Qt implementation")

#
#   Config path
#
set(VIRGIL_IOT_CONFIG_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/ext/virgil-iotkit/config/pc CACHE STRING "Path to the PC configuration")

#
#   Features
#
option(VIRGIL_IOTKIT_QT_PROTOCOLS "Enable 'protocols'" ON)
option(VIRGIL_IOTKIT_QT_DEFAULT_IMPL "Enable Default implementations" ON)

#
#   Default netif implementations
#
option(VIRGIL_IOTKIT_QT_DEFAULT_NETIF_UDP_BCAST "Enable default netif UDP broadcast implementation" ON)

#
#   Default Global HAL implementations
#
option(VIRGIL_IOTKIT_QT_DEFAULT_GLOBAL_HAL "Enable default global HAL implementation" ON)


# ---------------------------------------------------------------------------
#   Dependencies / thirdparty libraries
# ---------------------------------------------------------------------------

#
#   IoTKit
#
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/ext/virgil-iotkit)


# ---------------------------------------------------------------------------
#   IoTKit Qt
# ---------------------------------------------------------------------------

#
#   Example
#
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/example)

