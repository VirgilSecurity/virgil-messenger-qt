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
import QtQuick.Layouts 1.5

ApplicationWindow {
    property int footerHeight: 60
    property int listItemHeight: 80
    property bool snifferOnLeft : true
    property real snifferWidthRatio : 0.5
    property real widthHeightToShowBoth : 1.5
    property int margin: 5

    property bool bothChildren: true
    property bool snifferSelected: true
    property int devicesListX
    property int devicesListWidth
    property int snifferX
    property int snifferWidth


    function recalculateChildren() {
        devicesListButton.x = ( width / 2 - devicesListButton.side ) / 2;
        snifferButton.x = ( width * 3 / 2 - snifferButton.side ) / 2;

        bothChildren = width > height * widthHeightToShowBoth ? true : false;

        if(bothChildren) {

            snifferX = snifferOnLeft ? 0 : width * snifferWidthRatio;
            snifferWidth = width * snifferWidthRatio;

            devicesListX = snifferOnLeft ? width * snifferWidthRatio : 0;
            devicesListWidth = width * (1 - snifferWidthRatio );

        } else {

            snifferX = 0;
            snifferWidth = width;

            devicesListX = 0;
            devicesListWidth = width;
        }
    }

    id: applicationWindow
    visible: true
    title: "IoTKit Qt Demo"
    background: Rectangle {
        color: "#303030"
    }

    DevicesList {
        id: devicesList
        margin: margin
        listItemHeight: listItemHeight
        visibility: bothChildren || !snifferSelected
        curX: devicesListX
        curWidth: devicesListWidth
    }

    Sniffer {
        id: sniffer
        visibility: bothChildren || snifferSelected
        curX: snifferX
        curWidth: snifferWidth
    }

    footer: Rectangle {
        width: parent.width
        height: footerHeight
        color: "black"
        visible: !bothChildren

        DevicesListButton {
            id: devicesListButton
            side: footerHeight - 2 * margin
            y: margin
            onClicked: {
                snifferSelected = false;
                recalculateChildren();
            }
        }

        SnifferButton {
            id: snifferButton
            side: footerHeight - 2 * margin
            y: margin
            onClicked: {
                snifferSelected = true;
                recalculateChildren();
            }
        }
    }

    onWidthChanged: {
        recalculateChildren();
    }

    onHeightChanged: {
        recalculateChildren();
    }
}
