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


import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Window 2.2
import QtQuick.Layouts 1.1

ListView {
    id: devicesList
    property alias visibility : devicesList.visible

    anchors.fill: parent
    model: SnapInfoClient
    delegate: Item
    {
        id: listDelegate

        property int marging: 5
        property var view: ListView.view

        width: view.width
        height: 80

        Rectangle {
            width: parent.width
            height: parent.height
            gradient: Gradient {
                orientation: Gradient.Horizontal
                GradientStop{ position: 0; color: index % 2 ? "#606060" : "#404040" }
                GradientStop{ position: 1; color: "#404040" }
            }

        }

        Rectangle {
            id: roleItem
            Layout.rowSpan: 2
            color: "#004000"
            x: marging
            y: marging
            width: parent.height - 2 * marging
            height: parent.height - 2 * marging

            Text {
                color: "white"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                width: parent.height
                height: parent.height
                text: deviceRoles
            }
        }

        Text {
            color: "yellow"
            horizontalAlignment: Text.AlignLeft
            verticalAlignment: Text.AlignVCenter
            x: parent.height + 2 * marging
            y: 0
            width: parent.height
            height: parent.height / 3
            text: isActive ? "active" : "not active"
        }

        Text {
            color: "white"
            horizontalAlignment: Text.AlignLeft
            verticalAlignment: Text.AlignVCenter
            x: parent.height + 2 * marging
            y: parent.height / 3
            width: parent.height
            height: parent.height / 3
            text: macAddress
        }

        Text {
            color: "white"
            horizontalAlignment: Text.AlignLeft
            verticalAlignment: Text.AlignVCenter
            x: parent.height + 2 * marging
            y: parent.height * 2 / 3
            width: parent.height
            height: parent.height / 3
            text: "fw " + fwVer + ", tl " + tlVer
        }
    }
}