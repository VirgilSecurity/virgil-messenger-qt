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

#include <virgil/iot-qt/logger.h>
#include <virgil/iot-qt/snap-info.h>

using namespace VirgilIoTKit;

/* static */ VSSnapInfoClient * VSSnapInfoClient::_instance = nullptr;
/* static */ const vs_snap_service_t * VSSnapInfoClient::_snapService = nullptr;

VSSnapInfoClient::VSSnapInfoClient(){

    Q_ASSERT( !_instance );

    _instance = this;

    _snapInfoImpl.device_start = startNotify;
    _snapInfoImpl.general_info = generalInfo;
    _snapInfoImpl.statistics = statistics;

    _snapService = vs_snap_info_client( _snapInfoImpl );
}

VSSnapInfoClient::~VSSnapInfoClient(){
    Q_ASSERT( _instance );

    _instance = nullptr;

}

const std::string & VSSnapInfoClient::serviceName() const {
    using namespace std;
    static const std::string service_name = "INFO"s;

    return service_name;
}

/* static */ vs_status_e VSSnapInfoClient::startNotify( vs_snap_info_device_t *device ){
    Q_ASSERT( _instance );

    emit _instance->deviceStarted( *device );

    return VS_CODE_OK;
}

/* static */ vs_status_e VSSnapInfoClient::generalInfo( vs_info_general_t *general_data ){
    Q_ASSERT( _instance );

    emit _instance->deviceGeneralInfo( *general_data );

    return VS_CODE_OK;
}

/* static */ vs_status_e VSSnapInfoClient::statistics( vs_info_statistics_t *statistics ){
    Q_ASSERT( _instance );

    emit _instance->deviceStatistics( *statistics );

    return VS_CODE_OK;
}

VSSnapInfoClient::TEnumDevicesArray VSSnapInfoClient::enumDevices( size_t wait_msec, size_t max_devices_amount ) const {
    TEnumDevicesArray devices;
    size_t devices_amount;
    vs_status_e ret_code;

    devices.resize( max_devices_amount );
    ret_code = vs_snap_info_enum_devices( snapProtocol().defaultNetif(), devices.data(), devices.size(), &devices_amount, wait_msec );

    devices.resize( devices_amount );

    return devices;
}

bool VSSnapInfoClient::changePolling( size_t pooling_element, uint16_t period_seconds, bool enable, const VSQMac &device_mac) const {
    vs_mac_addr_t mac = device_mac;

    if( vs_snap_info_set_polling( snapProtocol().defaultNetif(), &mac, pooling_element, enable, period_seconds ) != VS_CODE_OK ) {
        VSLogError( "Unable to setup info polling" );
        return false;
    }

    return true;
}
