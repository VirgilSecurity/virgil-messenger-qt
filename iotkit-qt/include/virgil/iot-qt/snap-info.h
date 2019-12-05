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

#ifndef _VIRGIL_IOTKIT_QT_SNAP_INFO_SERVICE_H_
#define _VIRGIL_IOTKIT_QT_SNAP_INFO_SERVICE_H_

#include <vector>

#include <QtCore>

#include <virgil/iot-qt/snap-protocol.h>
#include <virgil/iot-qt/snap-service.h>

#include <virgil/iot/protocols/snap/info/info-structs.h>
#include <virgil/iot/protocols/snap/info/info-client.h>

class VSSnapInfoClient : public QObject, public VSSnapService {
    Q_OBJECT

public:
    VSSnapInfoClient();
    ~VSSnapInfoClient() override;

    using TEnumDevicesArray = std::vector<VirgilIoTKit::vs_snap_info_device_t>;

    const VirgilIoTKit::vs_snap_service_t *serviceInterface() override    { return _snapService; }
    const std::string &serviceName() const override;

    TEnumDevicesArray enumDevices( size_t wait_msec, size_t max_devices_amount = 1000 ) const;
    bool changePolling( size_t polling_element, uint16_t period_seconds, bool enable, const VSMac &device_mac = broadcastMac ) const;
    bool setPolling(    size_t polling_element, uint16_t period_seconds, const VSMac &device_mac = broadcastMac ) const { return changePolling( polling_element, period_seconds, true,  device_mac ); }
    bool resetPoling(   size_t polling_element, uint16_t period_seconds, const VSMac &device_mac = broadcastMac ) const { return changePolling( polling_element, period_seconds, false, device_mac ); }

signals:
    void deviceStarted( VirgilIoTKit::vs_snap_info_device_t &device );
    void deviceGeneralInfo( VirgilIoTKit::vs_info_general_t &general_data );
    void deviceStatistics( VirgilIoTKit::vs_info_statistics_t &statistics );

private:
    static VSSnapInfoClient *_instance;
    static const VirgilIoTKit::vs_snap_service_t * _snapService;
    mutable VirgilIoTKit::vs_snap_info_client_service_t _snapInfoImpl;

    static VirgilIoTKit::vs_status_e startNotify( VirgilIoTKit::vs_snap_info_device_t *device );
    static VirgilIoTKit::vs_status_e generalInfo( VirgilIoTKit::vs_info_general_t *general_data );
    static VirgilIoTKit::vs_status_e statistics( VirgilIoTKit::vs_info_statistics_t *statistics );

};

#endif // _VIRGIL_IOTKIT_QT_SNAP_INFO_SERVICE_H_
