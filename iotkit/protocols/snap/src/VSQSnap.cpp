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

#include <iomanip>
#include <sstream>

#include <virgil/iot-qt/logger.h>
#include <virgil/iot-qt/netif.h>
#include <virgil/iot-qt/snap-protocol.h>
#include <virgil/iot-qt/snap-service.h>

#include <virgil/iot/protocols/snap.h>

using namespace VirgilIoTKit;

/* static */ VSSnapProtocol * VSSnapProtocol::_instance = nullptr;

VSSnapProtocol::VSSnapProtocol(){
    assert( !_instance && "Constructor must be called once" );
    _instance = this;
}

VSSnapProtocol::~VSSnapProtocol(){
    _instance = nullptr;
}

bool VSSnapProtocol::init( VSNetif &network_interface, const VSManufactureId &manufacture_id, const VSDeviceType &device_type, const VSDeviceSerial &device_serial,
           vs_snap_device_role_e device_roles ) {

    _netif = network_interface.getImplementation();

    if( VirgilIoTKit::vs_snap_init( _netif, manufacture_id, device_type, device_serial, device_roles ) != VS_CODE_OK ) {
        VSLogCritical( "Unable to initialize Virgil IoT Kit SNAP protocol" );
        return false;
    }

    return true;
}

bool VSSnapProtocol::registerService( VSSnapService &snap_service ) {

    if( VirgilIoTKit::vs_snap_register_service( snap_service.serviceInterface() ) != VS_CODE_OK ) {
        VSLogCritical( "Unable to register service \"", snap_service.serviceName().c_str(), "\"" );
        return false;
    }

    snap_service.setSnapProtocol( this );

    return true;
}

/* static */ const VSManufactureId VSSnapProtocol::manufactureId() {
    return *VirgilIoTKit::vs_snap_device_manufacture();
}

/* static */ const VSDeviceSerial VSSnapProtocol::deviceSerial() {
    return *VirgilIoTKit::vs_snap_device_serial();
}

/* static */ const VSDeviceType VSSnapProtocol::deviceType() {
    return *VirgilIoTKit::vs_snap_device_type();
}

/* static */ uint32_t VSSnapProtocol::deviceRoles() {
    return VirgilIoTKit::vs_snap_device_roles();
}

/* static */ const VirgilIoTKit::vs_netif_t* defaultNetif() {
    return VirgilIoTKit::vs_snap_default_netif();
}

/* static */ const VirgilIoTKit::vs_netif_t* VSSnapProtocol::defaultNetif() {
    return VirgilIoTKit::vs_snap_default_netif();
}

/* static */ bool VSSnapProtocol::send( const TData &data, VirgilIoTKit::vs_netif_t* netif ) {
    return VirgilIoTKit::vs_snap_send( netif, data.data(), data.size() );
}

/* static */ VSMac VSSnapProtocol::macAddress( VirgilIoTKit::vs_netif_t* netif ) {
    VirgilIoTKit::vs_mac_addr_t ownMacAddr;

    VirgilIoTKit::vs_snap_mac_addr( netif, &ownMacAddr );

    return ownMacAddr;
}
