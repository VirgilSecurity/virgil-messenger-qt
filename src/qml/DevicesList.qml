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
    property string backgroundColor: "#303030"
    property int curX
    property int curWidth
    property int margin: 5
    property int listItemHeight: 60
    property string evenGradientColor: "#20FFFFFF"

    anchors.fill: parent
    model: SnapInfoClient

    delegate: Item
    {
        id: listDelegate
        height: listItemHeight

        Rectangle {
            y: 0
            x: curX
            width: curWidth
            height: parent.height
            color: backgroundColor
        }

        Rectangle {
            x: curX
            width: curWidth
            height: parent.height
            gradient: Gradient {
                orientation: Gradient.Horizontal
                GradientStop{ position: 0; color: index % 2 ? evenGradientColor : "#00000000" }
                GradientStop{ position: 0.5; color: index % 2 ? evenGradientColor : "#00000000" }
                GradientStop{ position: 1; color: "#00000000" }
            }

        }

        Rectangle {
            id: roleItem
            Layout.rowSpan: 2
            color: "#004000"
            x: curX + margin
            y: margin
            width: parent.height - 2 * margin
            height: parent.height - 2 * margin

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
            x: curX + parent.height + 2 * margin
            y: 0
            width: curWidth
            height: parent.height / 3
            text: isActive ? "active" : "not active"
        }

        Text {
            color: "white"
            horizontalAlignment: Text.AlignLeft
            verticalAlignment: Text.AlignVCenter
            x: curX + parent.height + 2 * margin
            y: parent.height / 3
            width: curWidth
            height: parent.height / 3
            text: macAddress
        }

        Text {
            color: "white"
            horizontalAlignment: Text.AlignLeft
            verticalAlignment: Text.AlignVCenter
            x: curX + parent.height + 2 * margin
            y: parent.height * 2 / 3
            width: curWidth
            height: parent.height / 3
            text: "fw " + fwVer + ", tl " + tlVer
        }
    }
}