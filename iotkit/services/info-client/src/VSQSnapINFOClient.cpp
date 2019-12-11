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

#include <VSQIoTKit.h>
#include <VSQSnapINFOClient.h>

using namespace VirgilIoTKit;

VSQSnapInfoClient::VSQSnapInfoClient() {

    m_snapInfoImpl.device_start = startNotify;
    m_snapInfoImpl.general_info = generalInfo;
    m_snapInfoImpl.statistics = statistics;

    m_snapService = vs_snap_info_client(m_snapInfoImpl);
}

/* static */ vs_status_e
VSQSnapInfoClient::startNotify(vs_snap_info_device_t *device) {
    emit instance().deviceStarted(*device);

    return VS_CODE_OK;
}

/* static */ vs_status_e
VSQSnapInfoClient::generalInfo(vs_info_general_t *generalData) {
    emit instance().deviceGeneralInfo(*generalData);

    return VS_CODE_OK;
}

/* static */ vs_status_e
VSQSnapInfoClient::statistics(vs_info_statistics_t *statistics) {
    emit instance().deviceStatistics(*statistics);

    return VS_CODE_OK;
}

VSQSnapInfoClient::TEnumDevicesArray
VSQSnapInfoClient::enumDevices(size_t waitMsec, size_t maxDevicesAmount) const {
    TEnumDevicesArray devices;
    size_t devicesAmount;
    vs_status_e ret_code;

    devices.resize(maxDevicesAmount);
    ret_code = vs_snap_info_enum_devices(netif(), devices.data(), devices.size(), &devicesAmount, waitMsec);

    devices.resize(devicesAmount);

    return devices;
}

bool
VSQSnapInfoClient::changePolling(size_t poolingElement,
                                 uint16_t periodSeconds,
                                 bool enable,
                                 const VSQMac &deviceMac) const {
    vs_mac_addr_t mac = deviceMac;

    if (vs_snap_info_set_polling(netif(), &mac, poolingElement, enable, periodSeconds) != VS_CODE_OK) {
        VSLogError("Unable to setup info polling");
        return false;
    }

    return true;
}
