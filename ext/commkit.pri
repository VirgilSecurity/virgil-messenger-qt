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

CONFIG += c++14

CONFIG(debug, debug|release) {
    BUILD_TYPE = debug
}
CONFIG(release, debug|release) {
    BUILD_TYPE = release
}

win32 {
    QMAKE_CFLAGS += -mno-ms-bitfields
    QMAKE_CXXFLAGS += -mno-ms-bitfields
}

DLL_EXT = "dylib"

unix:mac:                   OS_NAME = macos
linux:android:              OS_NAME = android.$$ANDROID_TARGET_ARCH
linux:!android:             OS_NAME = linux
win32:                      OS_NAME = windows

CONFIG(iphonesimulator, iphoneos | iphonesimulator) {
    OS_NAME = ios-sim
}

CONFIG(iphoneos, iphoneos | iphonesimulator) {
    OS_NAME = ios
}

PREBUILT_SYSROOT = $$PREBUILT_PATH/$${OS_NAME}/release/installed/usr/local
message("PREBUILT_SYSROOT : $${PREBUILT_SYSROOT}")


#
#   Copy shared libraries
#
unix:mac: {
    isEqual(OS_NAME, "ios")|isEqual(OS_NAME, "ios-sim"): {
        message("No need to copy vs-messenger-internal")
    } else {
        DST_DLL = $${OUT_PWD}/$${TARGET}.app/Contents/Frameworks
        MESSENGER_INTERNAL_DLL = $${PREBUILT_SYSROOT}/lib/libvs-messenger-internal.$${DLL_EXT}
        QMAKE_POST_LINK += $$quote(mkdir -p $${DST_DLL}/$$escape_expand(\n\t))
        QMAKE_POST_LINK += $$quote(cp $${MESSENGER_INTERNAL_DLL} $${DST_DLL}/$$escape_expand(\n\t))
    }
}

#
#   Libraries
#
LIBS += -L$${PREBUILT_SYSROOT}/lib
LIBS += -lvs-module-logger
LIBS += -lvs-module-messenger
LIBS += -lvscryptoc
!mac: {
  LIBS += -lcurl
}

win32: {
	LIBS += -lws2_32
        LIBS += $${PREBUILT_SYSROOT}/lib/libvs-messenger-internal.dll.a
} else: {
        LIBS += -lvs-messenger-internal
}

#
#   Include path
#
INCLUDEPATH +=  $${PREBUILT_SYSROOT}/include \
                $$PREBUILT_PATH/qt/config/pc

#
#   Compiler options
#

win32|linux:!android: QMAKE_CFLAGS+=-Wno-multichar
win32|linux:!android: QMAKE_CXXFLAGS+=-Wno-multichar
