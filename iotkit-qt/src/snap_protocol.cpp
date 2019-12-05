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

#include <virgil/iot-qt/snap_protocol.h>
#include <virgil/iot-qt/snap_service.h>

#include <virgil/iot/protocols/snap.h>

using namespace VirgilIoTKit;

VSMac& VSMac::set( const vs_mac_addr_t &mac ) {
    std::copy( mac.bytes, mac.bytes + sizeof( mac.bytes ), _mac.begin() );
    return *this;
}

VSMac& VSMac::set( const uint8_t *bytes ) {
    assert( bytes );
    std::copy( bytes, bytes + _mac.size(), _mac.begin() );
    return *this;
}

VSMac& VSMac::set( uint8_t b0, uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4, uint8_t b5 ) {
    _mac[0] = b0; _mac[1] = b1; _mac[2] = b2; _mac[3] = b3; _mac[4] = b4; _mac[5] = b5;
    return *this;
}

VSMac& VSMac::set( const VSMac &mac ) {
    _mac = mac._mac;
    return *this;
}

VSMac::operator vs_mac_addr_t() const {
    vs_mac_addr_t mac;
    std::copy( _mac.begin(), _mac.end(), mac.bytes );
    return mac;
}

std::string VSMac::describe( bool two_symbols, bool upper_case, char divider ) const {
    using namespace std;

    std::stringstream ss;

    ss << hex;

    if( upper_case )
        ss << uppercase;

    if( two_symbols )
        ss << setfill('0') << setw(2);

    for( size_t pos = 0; pos < _mac.size(); ++pos ) {
        if( pos )
            ss << divider;
        ss << static_cast<int>(_mac[pos]);
    }

    return ss.str();
}

VSManufactureId& VSManufactureId::set( const VSManufactureId& manufacture_id ) {
    _manufactureId = manufacture_id._manufactureId;
    return *this;
}

VSManufactureId& VSManufactureId::set( const vs_device_manufacture_id_t& buf ){
    std::copy( buf, buf + sizeof( vs_device_manufacture_id_t ), _manufactureId.begin() );
    return *this;
}

VSManufactureId::operator const uint8_t* () const {
    return _manufactureId.data();
}

std::string VSManufactureId::describe( bool stop_on_zero, char symbol_on_non_ascii ) const {
    std::string str;

    str.reserve( _manufactureId.size() );

    for( auto symbol : _manufactureId ) {
        if( symbol >= ' ' )
            str += symbol;
        else if ( symbol > 0 || !stop_on_zero)
            str += symbol_on_non_ascii;
        else
            break;
    }

    return str;
}

VSDeviceType& VSDeviceType::set( const VSDeviceType& device_type ) {
    _deviceType = device_type._deviceType;
    return *this;
}

VSDeviceType& VSDeviceType::set( const vs_device_type_t& buf ){
    std::copy( buf, buf + sizeof( vs_device_type_t ), _deviceType.begin() );
    return *this;
}

VSDeviceType::operator const uint8_t* () const{
    return _deviceType.data();
}

std::string VSDeviceType::describe( bool stop_on_zero, char symbol_on_non_ascii ) const {
    std::string str;

    str.reserve( _deviceType.size() );

    for( auto symbol : _deviceType ) {
        if( symbol >= ' ' )
            str += symbol;
        else if ( symbol > 0 || !stop_on_zero)
            str += symbol_on_non_ascii;
        else
            break;
    }

    return str;
}
VSDeviceSerial& VSDeviceSerial::set( const VSDeviceSerial& device_serial ) {
    _deviceSerial = device_serial._deviceSerial;
    return *this;
}

VSDeviceSerial& VSDeviceSerial::set( const vs_device_serial_t& buf ){
    std::copy( buf, buf + sizeof( vs_device_type_t ), _deviceSerial.begin() );
    return *this;
}

VSDeviceSerial::operator const uint8_t* () const{
    return _deviceSerial.data();
}

std::string VSDeviceSerial::describe( bool two_symbols, bool upper_case, char divider ) const {
    using namespace std;

    std::stringstream ss;

    ss << hex;

    if( upper_case )
        ss << uppercase;

    if( two_symbols )
        ss << setfill('0') << setw(2);

    for( size_t pos = 0; pos < _deviceSerial.size(); ++pos ) {
        if( pos )
            ss << divider;
        ss << static_cast<int>(_deviceSerial[pos]);
    }

    return ss.str();
}

/* static */ VSSnapProtocol * VSSnapProtocol::_instance = nullptr;

VSSnapProtocol::VSSnapProtocol(){
    assert( !_instance );
    _instance = this;

}

VSSnapProtocol::~VSSnapProtocol(){
    assert( _instance );
    _instance = nullptr;
}

bool VSSnapProtocol::init( const VSManufactureId &manufacture_id, const VSDeviceType &device_type, const VSDeviceSerial &device_serial,
           vs_snap_device_role_e device_roles, FChangeStateNotify change_state_notify ) {

    _changeStateNotify = change_state_notify;

    _networkInterface.user_data = this;
    _networkInterface.init = netIfInit;
    _networkInterface.deinit = netIfDeinit;
    _networkInterface.tx = netIfTx;
    _networkInterface.mac_addr = netIfMac;

    _state = EState::NotInitialized;

    if( vs_snap_init( &_networkInterface, manufacture_id, device_type, device_serial, device_roles ) != VS_CODE_OK ) {
        VS_LOG_CRITICAL( "Unable to initialize Virgil IoTKIT::SNAP protocol" );
        return false;
    }

    return true;
}

void VSSnapProtocol::processRxData( TData &&data ) {
    if( !_netIfRxCallback )
        return;

    const uint8_t *packet_data = nullptr;
    uint16_t packet_data_sz = 0;

    if( _netIfRxCallback( &_networkInterface, data.data(), data.size(), &packet_data, &packet_data_sz ) != VS_CODE_OK )
        return;

    if( _netIfRxProcessCallback( &_networkInterface, packet_data, packet_data_sz ) != VS_CODE_OK )
        VS_LOG_ERROR( "Unable to process received packet" );
}

void VSSnapProtocol::сhangeStateNotify( const std::string &description ) {
    _changeStateNotify( description );
}

/* static */ vs_status_e VSSnapProtocol::netIfInit( const vs_netif_rx_cb_t rx_cb, const vs_netif_process_cb_t process_cb ){
    assert( _instance );

    _instance->_netIfRxCallback = rx_cb;
    _instance->_netIfRxProcessCallback = process_cb;

    return _instance->initInterface();
}

/* static */ vs_status_e VSSnapProtocol::netIfDeinit(){
    assert( _instance );

    return _instance->destroy();

}

/* static */ vs_status_e VSSnapProtocol::netIfTx( const uint8_t* data, const uint16_t data_sz ){
    assert( _instance );

    return _instance->sendRawData( TData( data, data + data_sz ));

}

/* static */ vs_status_e VSSnapProtocol::netIfMac( vs_mac_addr_t* mac_addr ){
    assert( _instance );

    auto [ ret_code, mac ] = _instance->ownMac();

    *mac_addr = mac;

    return ret_code;

}

bool VSSnapProtocol::registerService( VSSnapService &snap_service ) {
    vs_status_e ret_code;

    if( vs_snap_register_service( snap_service.serviceInterface() ) != VS_CODE_OK ) {
        VS_LOG_CRITICAL( "Unable to register service \"", snap_service.serviceName().c_str(), "\"" );
        return false;
    }

    snap_service.setSnapProtocol( this );

    return true;
}
