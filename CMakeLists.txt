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

#
#   Version
#
set (PACKAGE_VERSION "0")
set (PACKAGE_VERSION_MAJOR "1")
set (PACKAGE_VERSION_MINOR "0")

include(GNUInstallDirs)

# ---------------------------------------------------------------------------
#   Project
# ---------------------------------------------------------------------------
project(virgil-messenger-qt VERSION ${PACKAGE_VERSION}.${PACKAGE_VERSION_MAJOR}.${PACKAGE_VERSION_MINOR} LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---------------------------------------------------------------------------
#   Helper modules
# ---------------------------------------------------------------------------
set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})
include (clangformat)
include (helpers)
if (APPLE)
    include (macdeploy)
endif()

# ---------------------------------------------------------------------------
#   Version
# ---------------------------------------------------------------------------
file(WRITE "${CMAKE_CURRENT_LIST_DIR}/VERSION_MESSENGER" "${PACKAGE_VERSION}.${PACKAGE_VERSION_MAJOR}.${PACKAGE_VERSION_MINOR}")

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
        message(FATAL_ERROR "SIGN_CERT must be set. Take a look 'security find-identity -p codesigning'. You have to add -DSIGN_CERT_ID=<hex value>")
    endif()
endif()

# ---------------------------------------------------------------------------
#   Project targets
# ---------------------------------------------------------------------------

#
#   UI App
#
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/src)

#
#   Tests
#
#add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/tests)

#
#   Documentation
#
#add_custom_target(ovpn-firewall-documentation
#        COMMAND rm -fR docs/doxygen
#        COMMAND mkdir -p docs/doxygen
#        COMMAND doxygen
#        WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})

# ---------------------------------------------------------------------------
#   Done
# ---------------------------------------------------------------------------
