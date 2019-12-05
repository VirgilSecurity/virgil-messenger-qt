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

#ifndef _VIRGIL_IOTKIT_QT_INFO_CLIENT_MAIN_WINDOW_H_
#define _VIRGIL_IOTKIT_QT_INFO_CLIENT_MAIN_WINDOW_H_

#include "VSQApp.h"

#include <virgil/iot-qt/snap-protocol.h>
#include <virgil/iot/protocols/snap/info/info-structs.h>

class VSMainWindow : public QObject  {
    Q_OBJECT

public:
    VSMainWindow();
    ~VSMainWindow();

    void show();
    void changeConnectionState( const std::string &connection_state );

public slots:
    void deviceStarted( VirgilIoTKit::vs_snap_info_device_t &device );
    void deviceGeneralInfo( VirgilIoTKit::vs_info_general_t &general_data );
    void deviceStatistics( VirgilIoTKit::vs_info_statistics_t &statistics );
    void stateChanged( QAbstractSocket::SocketState connectionState, const std::string &description );

private:
    enum class EColumn{ MAC = 0, DevRoles, Manufacture, Type, FWVersion, TLVersion, ReceivedAmount, SentAmount, ColumnsAmount };
    struct SDeviceInfo {
        SDeviceInfo(): _hasGeneralInfo( false ), _hasStatistics( false )    {}

        VSMac _mac;
        VirgilIoTKit::vs_snap_device_role_e _roles;
        VSManufactureId _manufactureId;
        VSDeviceType _deviceType;
        VirgilIoTKit::vs_file_version_unpacked_t _fwVer;
        VirgilIoTKit::vs_file_version_unpacked_t _tlVer;
        uint32_t _sent;
        uint32_t _received;

        bool _hasGeneralInfo;
        bool _hasStatistics;
    };

    QMainWindow _wnd;
    QTableWidget _tbl;
    QStatusBar _status;
    std::vector<SDeviceInfo> _devices;
    QString _connectionState;

    void changeStatusBar();
    void updateTable();

    SDeviceInfo& device( const VSMac &mac );
};

#endif //_VIRGIL_IOTKIT_QT_INFO_CLIENT_MAIN_WINDOW_H_
