//  Copyright (C) 2015-2019 Virgil Security, Inc.
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

#include <VSQMainWindow.h>
//#include <virgil/iot-qt/logger.h>

VSMainWindow::VSMainWindow()
{
}

VSMainWindow::~VSMainWindow()
{
}

void VSMainWindow::show()
{
    //    static const std::map<EColumn, QString> tbl_headers = { { EColumn::MAC, "MAC" }, { EColumn::DevRoles, "Device Roles" }, { EColumn::Manufacture, "Manufacture" }, { EColumn::Type, "Type" }, { EColumn::FWVersion, "Firmware Version" }, { EColumn::TLVersion, "Trust List Version" }, { EColumn::ReceivedAmount, "Receives" }, { EColumn::SentAmount, "Sends" } };
    //
    //    _tbl.setColumnCount(static_cast<int>(EColumn::ColumnsAmount));
    //    _tbl.setAlternatingRowColors(true);
    //
    //    for (const auto& [column_id, title] : tbl_headers)
    //        _tbl.setHorizontalHeaderItem(static_cast<int>(column_id), new QTableWidgetItem(title));
    //
    //    _wnd.setCentralWidget(&_tbl);
    _wnd.showNormal();

    //    _connectionState = "No connection";
    //    changeStatusBar();
}

//void VSMainWindow::changeConnectionState(const std::string& connection_state)
//{
//    _connectionState = QString::fromStdString(connection_state);
//    changeStatusBar();
//}
//
//void VSMainWindow::changeStatusBar()
//{
//    QString str;
//
//    if (_devices.empty())
//        str = "No devices";
//    else if (_devices.size() == 1)
//        str = "1 device";
//    else
//        str = QString("%1 devices").arg(_devices.size());
//
//    str += " *** ";
//    str += _connectionState;
//
//    _wnd.statusBar()->showMessage(str);
//}
//
//VSMainWindow::SDeviceInfo& VSMainWindow::device(const VSMac& mac)
//{
//    for (auto& device : _devices) {
//        if (device._mac == mac) {
//            return device;
//        }
//    }
//
//    _devices.emplace_back();
//    return _devices.back();
//}
//
//void VSMainWindow::deviceStarted(VirgilIoTKit::vs_snap_info_device_t& started_device)
//{
//
//    SDeviceInfo& dev = device(started_device.mac);
//
//    dev._mac = started_device.mac;
//    dev._roles = static_cast<VirgilIoTKit::vs_snap_device_role_e>(started_device.device_roles);
//
//    updateTable();
//}
//
//void VSMainWindow::deviceGeneralInfo(VirgilIoTKit::vs_info_general_t& general_data)
//{
//    SDeviceInfo& dev = device(general_data.default_netif_mac);
//
//    dev._manufactureId = general_data.manufacture_id;
//    dev._deviceType = general_data.device_type;
//    dev._roles = static_cast<VirgilIoTKit::vs_snap_device_role_e>(general_data.device_roles);
//    dev._fwVer = general_data.fw_ver;
//    dev._tlVer = general_data.tl_ver;
//    dev._hasGeneralInfo = true;
//
//    updateTable();
//}
//
//void VSMainWindow::deviceStatistics(VirgilIoTKit::vs_info_statistics_t& statistics)
//{
//    SDeviceInfo& dev = device(statistics.default_netif_mac);
//
//    dev._received = statistics.received;
//    dev._sent = statistics.sent;
//    dev._hasStatistics = true;
//
//    updateTable();
//}
//
//void VSMainWindow::stateChanged(QAbstractSocket::SocketState connectionState, const std::string& description)
//{
//    (void)connectionState;
//    changeConnectionState(description);
//}
//
//void VSMainWindow::updateTable()
//{
//    size_t row = 0;
//
//    _tbl.setRowCount(0);
//    _tbl.setRowCount(_devices.size());
//
//    for (const auto& device : _devices) {
//        QString text;
//
//        text = QString::fromStdString(device._mac);
//
//        _tbl.setItem(row, static_cast<int>(EColumn::MAC), new QTableWidgetItem(text));
//
//        text = "";
//        if (device._roles & VirgilIoTKit::VS_SNAP_DEV_GATEWAY)
//            text += ", Gateway";
//        if (device._roles & VirgilIoTKit::VS_SNAP_DEV_THING)
//            text += ", Thing";
//        if (device._roles & VirgilIoTKit::VS_SNAP_DEV_CONTROL)
//            text += ", Control";
//        if (device._roles & VirgilIoTKit::VS_SNAP_DEV_LOGGER)
//            text += ", Logger";
//        if (device._roles & VirgilIoTKit::VS_SNAP_DEV_SNIFFER)
//            text += ", Sniffer";
//        if (device._roles & VirgilIoTKit::VS_SNAP_DEV_DEBUGGER)
//            text += ", Debugger";
//        if (device._roles & VirgilIoTKit::VS_SNAP_DEV_INITIALIZER)
//            text += ", Initializer";
//        text.remove(0, 2);
//        _tbl.setItem(row, static_cast<int>(EColumn::DevRoles), new QTableWidgetItem(text));
//
//        text = "";
//        if (device._hasGeneralInfo) {
//            text = QString::fromStdString(device._manufactureId);
//        }
//        _tbl.setItem(row, static_cast<int>(EColumn::Manufacture), new QTableWidgetItem(text));
//
//        text = "";
//        if (device._hasGeneralInfo) {
//            text = QString::fromStdString(device._deviceType);
//        }
//        _tbl.setItem(row, static_cast<int>(EColumn::Type), new QTableWidgetItem(text));
//
//        text = "";
//        if (device._hasGeneralInfo) {
//            text = QString("%1.%2.%3.%4").arg(static_cast<int>(device._fwVer.major)).arg(static_cast<int>(device._fwVer.minor)).arg(static_cast<int>(device._fwVer.patch)).arg(static_cast<int>(device._fwVer.build));
//        }
//        _tbl.setItem(row, static_cast<int>(EColumn::FWVersion), new QTableWidgetItem(text));
//
//        text = "";
//        if (device._hasGeneralInfo) {
//            text = QString("%1.%2.%3.%4").arg(static_cast<int>(device._tlVer.major)).arg(static_cast<int>(device._tlVer.minor)).arg(static_cast<int>(device._tlVer.patch)).arg(static_cast<int>(device._tlVer.build));
//        }
//        _tbl.setItem(row, static_cast<int>(EColumn::TLVersion), new QTableWidgetItem(text));
//
//        text = "";
//        if (device._hasStatistics) {
//            text = QString("%1").arg(device._sent);
//        }
//        _tbl.setItem(row, static_cast<int>(EColumn::SentAmount), new QTableWidgetItem(text));
//
//        text = "";
//        if (device._hasStatistics) {
//            text = QString("%1").arg(device._received);
//        }
//        _tbl.setItem(row, static_cast<int>(EColumn::ReceivedAmount), new QTableWidgetItem(text));
//
//        ++row;
//    }
//
//    changeStatusBar();
//}
