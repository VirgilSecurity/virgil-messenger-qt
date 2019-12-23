//  Copyright (C) 2015-2020 Virgil Security, Inc.
//
//  All rights reserved.
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions are
//  met:
//
//      (1) Redistributions of source code must retain the above copyright
//      notice, this list of conditions and the following disclaimer.
//
//      (2) Redistributions in binary form must reproduce the above copyright
//      notice, this list of conditions and the following disclaimer in
//      the documentation and/or other materials provided with the
//      distribution.
//
//      (3) Neither the name of the copyright holder nor the names of its
//      contributors may be used to endorse or promote products derived from
//      this software without specific prior written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE AUTHOR ''AS IS'' AND ANY EXPRESS OR
//  IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
//  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
//  DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
//  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
//  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
//  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
//  HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
//  STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
//  IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
//  POSSIBILITY OF SUCH DAMAGE.
//
//  Lead Maintainer: Virgil Security Inc. <support@virgilsecurity.com>

import QtQuick 2.5
import QtQuick.Window 2.2
import QtQuick.Controls 1.4
import QtQuick.Dialogs 1.0

ApplicationWindow {
    id: applicationWindow
    visible: true
    visibility: VSQDeviceInfoController.visibilityMode
    title: "IoTKit Qt Demo"

    minimumWidth: 640
    minimumHeight: 480

    Row {
        id: labels
        Label { text: "MAC address / Roles"; width: applicationWindow.width / 6 }
        Label { text: "Device state"; width: applicationWindow.width / 6 }
        Label { text: "Manufacture ID / Device Type"; width: applicationWindow.width / 6 }
        Label { text: "Firmware / Trustlist version"; width: applicationWindow.width / 6 }
        Label { text: "Sent / Received"; width: applicationWindow.width / 6 }
        Label { text: "Last timestamp"; width: applicationWindow.width / 6 }
    }

    ListView {
        anchors.top: labels.bottom
        height: applicationWindow.height -labels.height

        flickableDirection: Flickable.AutoFlickDirection

        model: VSQDeviceInfoController.deviceInfoList
        delegate: Row {
            Label { text: macDeviceRoles; width: applicationWindow.width / 6 }
            Label { text: deviceState; width: applicationWindow.width / 6 }
            Label { text: manufactureIdDeviceType; width: applicationWindow.width / 6 }
            Label { text: fwTlVer; width: applicationWindow.width / 6 }
            Label { text: statistics; width: applicationWindow.width / 6 }
            Label { text: lastTimestamp; width: applicationWindow.width / 6 }
        }
    }
}
