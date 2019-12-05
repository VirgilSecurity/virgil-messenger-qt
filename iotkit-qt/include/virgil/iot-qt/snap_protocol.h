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

#ifndef _VIRGIL_IOTKIT_QT_SNAP_PROTOCOL_H_
#define _VIRGIL_IOTKIT_QT_SNAP_PROTOCOL_H_

#include <array>
#include <string>
#include <vector>

#include <virgil/iot-qt/helpers.h>
#include <virgil/iot/status_code/status_code.h>
#include <virgil/iot/provision/provision-structs.h>
#include <virgil/iot/protocols/snap/snap-structs.h>

class VSSnapService;

class VSSnapProtocol {
public:
    using FChangeStateNotify = std::function<void( const std::string & )>;

    VSSnapProtocol();
    virtual ~VSSnapProtocol();

    bool init( const VSManufactureId &manufacture_id, const VSDeviceType &device_type, const VSDeviceSerial &device_serial,
               VirgilIoTKit::vs_snap_device_role_e device_roles, FChangeStateNotify change_state_notify );
    bool registerService( VSSnapService &snap_service );

    const VirgilIoTKit::vs_netif_t* netIf() const { return &_networkInterface; }
    virtual const VSMac& ownMacAddress() const = 0;

protected:
    enum class EState{ NotInitialized, Initialized, Destructed };
    using TMacResponse = std::pair<VirgilIoTKit::vs_status_e, VSMac>;

    EState state() const { return _state; }

    virtual VirgilIoTKit::vs_status_e initInterface() = 0;
    virtual VirgilIoTKit::vs_status_e destroy() = 0;
    virtual VirgilIoTKit::vs_status_e sendRawData( TData &&data ) = 0;
    virtual TMacResponse ownMac() const = 0;

    void processRxData( TData &&data );
    void сhangeStateNotify( const std::string &description );

private:
    static VSSnapProtocol *_instance;
    VirgilIoTKit::vs_netif_t _networkInterface;
    EState _state;
    FChangeStateNotify _changeStateNotify;

    static VirgilIoTKit::vs_status_e netIfDeinit();
    static VirgilIoTKit::vs_status_e netIfInit( const VirgilIoTKit::vs_netif_rx_cb_t rx_cb, const VirgilIoTKit::vs_netif_process_cb_t process_cb );
    static VirgilIoTKit::vs_status_e netIfTx( const uint8_t* data, const uint16_t data_sz );
    static VirgilIoTKit::vs_status_e netIfMac( VirgilIoTKit::vs_mac_addr_t* mac_addr );

    VirgilIoTKit::vs_netif_rx_cb_t _netIfRxCallback;
    VirgilIoTKit::vs_netif_process_cb_t _netIfRxProcessCallback;
};

#endif // _VIRGIL_IOTKIT_QT_SNAP_PROTOCOL_H_
