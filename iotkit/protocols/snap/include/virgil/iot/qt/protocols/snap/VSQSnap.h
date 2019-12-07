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

//#include <array>
//#include <string>
//#include <vector>
//
//#include <virgil/iot-qt/helpers.h>
//#include <virgil/iot/status_code/status_code.h>
//#include <virgil/iot/provision/provision-structs.h>
//#include <virgil/iot/protocols/snap/snap-structs.h>

class VSSnapService;
class VSNetif;

class VSQSnap {
public:
    using FChangeStateNotify = std::function<void( const std::string & )>;

    VSSnapProtocol();
    virtual ~VSSnapProtocol();

    bool init( VSNetif &network_interface, const VSManufactureId &manufacture_id, const VSDeviceType &device_type, const VSDeviceSerial &device_serial,
               VirgilIoTKit::vs_snap_device_role_e device_roles );
    bool registerService( VSSnapService &snap_service );

    static const VSManufactureId manufactureId();
    static const VSDeviceSerial deviceSerial();
    static const VSDeviceType deviceType();
    static uint32_t deviceRoles();
    static const VirgilIoTKit::vs_netif_t* defaultNetif();
    static bool send( const TData &data, VirgilIoTKit::vs_netif_t* netif = nullptr );
    static VSMac macAddress( VirgilIoTKit::vs_netif_t* netif = nullptr );

private:
    static VSSnapProtocol* _instance;
    VirgilIoTKit::vs_netif_t* _netif = nullptr;
};

#endif // _VIRGIL_IOTKIT_QT_SNAP_PROTOCOL_H_
