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
#   System introspection.
# ---------------------------------------------------------------------------
find_package(Qt5 COMPONENTS Widgets REQUIRED)

# ---------------------------------------------------------------------------
#   Libraries.
# ---------------------------------------------------------------------------
#
#   Library: platform-deps
#
add_library(platform-deps INTERFACE)
target_link_libraries(platform-deps
        INTERFACE
        Qt5::Widgets
        curl
        )

# ---------------------------------------------------------------------------
#   Main target
# ---------------------------------------------------------------------------
add_executable(${VS_TARGET_NAME})

# ---------------------------------------------------------------------------
#   Deploy
# ---------------------------------------------------------------------------
find_program(DEPLOY_QT cqtdeployer)

add_custom_target(deploy
        COMMENT "Running QT deployer for Linux..."

        COMMAND "${DEPLOY_QT}" deploySystem
            -bin "$<TARGET_FILE:${VS_TARGET_NAME}>"
            -qmlDir "${PROJECT_SOURCE_DIR}/src/qml"
            -targetDir "${CMAKE_CURRENT_BINARY_DIR}/${VS_TARGET_NAME}.dist"
            -qmake "${QT_QMAKE_EXECUTABLE}" clear

        COMMAND "${CMAKE_COMMAND}" -E
            tar czf "${VS_TARGET_NAME}-${PROJECT_VERSION}.tgz" -- "${VS_TARGET_NAME}.dist"

        DEPENDS ${VS_TARGET_NAME}
        )
