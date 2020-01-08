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
import QtQuick.Controls 2.12
import QtQuick.Window 2.2
import QtQuick.Layouts 1.1

ListView {
    id: sniffer
    property alias visibility : sniffer.visible
    property var listItemHeight
    property string backgroundColor: "#202020"

    anchors.fill: parent
    model: SnapSniffer

    delegate: Item
    {
        id: listDelegate
        height: listItemHeight
        width: parent.width

        Rectangle {
            anchors.fill: parent
            color: backgroundColor
        }

        ColumnLayout {
            Text {
                id: packetEthernetInfo
                wrapMode: Text.Wrap
                color: "yellow"
                Layout.alignment: Qt.AlignLeft | Qt.AlignBottom
                text: timestamp + " : " + macSrc + " ==> " + macDst
            }

            Text {
                id: packetSnapInfo
                wrapMode: Text.Wrap
                color: "lightBlue"
                Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                text: serviceId + " : " + elementId
            }

            Text {
                id: packetContent
                wrapMode: Text.Wrap
                color: "white"
                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                text: content
            }
        }
    }
}
