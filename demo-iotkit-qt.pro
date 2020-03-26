#  Copyright (C) 2015-2020 Virgil Security, Inc.
#
#  All rights reserved.
#
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions are
#  met:
#
#      (1) Redistributions of source code must retain the above copyright
#      notice, this list of conditions and the following disclaimer.
#
#      (2) Redistributions in binary form must reproduce the above copyright
#      notice, this list of conditions and the following disclaimer in
#      the documentation and/or other materials provided with the
#      distribution.
#
#      (3) Neither the name of the copyright holder nor the names of its
#      contributors may be used to endorse or promote products derived from
#      this software without specific prior written permission.
#
#  THIS SOFTWARE IS PROVIDED BY THE AUTHOR ''AS IS'' AND ANY EXPRESS OR
#  IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
#  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
#  DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
#  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
#  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
#  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
#  HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
#  STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
#  IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
#  POSSIBILITY OF SUCH DAMAGE.
#
#  Lead Maintainer: Virgil Security Inc. <support@virgilsecurity.com>

QT += core network qml quick bluetooth sql xml concurrent

CONFIG += c++14

TARGET = demo-iotkit-qt

#
#   Include IoTKit Qt wrapper
#

include(ext/virgil-iotkit/integration/qt/iotkit.pri)

#
#   QXMPP
#
QXMPP_BUILD_PATH = $$PWD/ext/qxmpp/cmake-build-host/release/installed/usr/local
message("QXMPP location : $${QXMPP_BUILD_PATH}")

#
#   Defines
#

DEFINES += QT_DEPRECATED_WARNINGS \
        INFO_CLIENT=1 \
        CFG_CLIENT=1

CONFIG(iphoneos, iphoneos | iphonesimulator) {
    DEFINES += VS_IOS=1
}

#
#   Headers
#

HEADERS += \
        include/VSQApplication.h \
        include/VSQMessenger.h \
        include/VSQSqlContactModel.h \
        include/VSQSqlConversationModel.h

#
#   Sources
#

SOURCES += \
        src/VSQMessenger.cpp \
        src/VSQSqlContactModel.cpp \
        src/VSQSqlConversationModel.cpp \
        src/main.cpp \
        src/VSQApplication.cpp

#
#   Resources
#

RESOURCES += src/resources.qrc

#
#   Include path
#

INCLUDEPATH +=  include \
        $${QXMPP_BUILD_PATH}/include


#
#   Libraries
#

LIBS += -L$${QXMPP_BUILD_PATH}/lib -lqxmpp -L/usr/local/lib -lvpx -lopus

#
#   Default rules for deployment
#
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

#
#   Depencies
#

DEPENDPATH += $${INCLUDEPATH}
