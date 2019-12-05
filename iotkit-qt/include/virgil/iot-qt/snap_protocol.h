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

#include <virgil/iot/status_code/status_code.h>
#include <virgil/iot/provision/provision-structs.h>
#include <virgil/iot/protocols/snap/snap-structs.h>

using TData = std::vector<std::uint8_t>;

class VSSnapService;

class VSMac {
public:
    VSMac(): _mac{ 0 }   {}
    VSMac( const VSMac& ) = default;
    VSMac( const VirgilIoTKit::vs_mac_addr_t &mac ) { set( mac ); }
    VSMac( const uint8_t *bytes )     { set( bytes ); }
    VSMac( uint8_t b0, uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4, uint8_t b5 )     { set( b0, b1, b2, b3, b4, b5 ); }

    VSMac &operator= ( const VirgilIoTKit::vs_mac_addr_t &mac )    { return set( mac ); }
    VSMac &operator= ( const VSMac &mac )             { return set( mac ); }
    bool operator==( const VSMac &mac ) const        { return equal( mac ); }

    VSMac& set( const VirgilIoTKit::vs_mac_addr_t &mac );
    VSMac& set( const uint8_t *bytes );
    VSMac& set( uint8_t b0, uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4, uint8_t b5 );
    VSMac& set( const VSMac &mac );

    std::string describe( bool two_symbols = false, bool upper_case = false, char divider = ':' ) const;

    bool equal( const VSMac &mac ) const { return _mac == mac._mac; }
    operator VirgilIoTKit::vs_mac_addr_t() const;
    operator const uint8_t*() const { return _mac.data(); }
    operator std::string() const    { return describe(); }

private:
    std::array<uint8_t, 6> _mac;
};

class VSManufactureId {
public:
    VSManufactureId(): _manufactureId{ 0 } {}
    VSManufactureId( const VSManufactureId& ) = default;
    VSManufactureId( const VirgilIoTKit::vs_device_manufacture_id_t& buf ) { set( buf ); }

    VSManufactureId& operator=( const VSManufactureId& manufacture_id )   { return set( manufacture_id ); }
    VSManufactureId& operator=( const VirgilIoTKit::vs_device_manufacture_id_t& buf )  { return set( buf ); }
    bool operator==( const VSManufactureId &manufacture_id ) const       { return equal( manufacture_id ); }

    VSManufactureId& set( const VSManufactureId& manufacture_id );
    VSManufactureId& set( const VirgilIoTKit::vs_device_manufacture_id_t& buf );

    std::string describe( bool stop_on_zero = true, char symbol_on_non_ascii = ' ' ) const;

    bool equal( const VSManufactureId &manufacture_id ) const { return _manufactureId == manufacture_id._manufactureId; }
    operator const uint8_t* () const;
    operator std::string() const    { return describe(); }

private:
    std::array<uint8_t, VS_DEVICE_MANUFACTURE_ID_SIZE> _manufactureId;
};

class VSDeviceType {
public:
    VSDeviceType(): _deviceType{ 0 } {}
    VSDeviceType( const VSDeviceType& ) = default;
    VSDeviceType( const VirgilIoTKit::vs_device_type_t& buf ) { set( buf ); }

    VSDeviceType& operator=( const VSDeviceType& device_type )    { return set( device_type ); }
    VSDeviceType& operator=( const VirgilIoTKit::vs_device_type_t& buf )       { return set( buf ); }
    bool operator==( const VSDeviceType &device_type ) const     { return equal( device_type ); }

    VSDeviceType& set( const VSDeviceType& device_type );
    VSDeviceType& set( const VirgilIoTKit::vs_device_type_t& buf );

    std::string describe( bool stop_on_zero = true, char symbol_on_non_ascii = ' ' ) const;

    bool equal( const VSDeviceType &device_type ) const { return _deviceType == device_type._deviceType; }
    operator const uint8_t* () const;
    operator std::string() const    { return describe(); }

private:
    std::array<uint8_t, VS_DEVICE_TYPE_SIZE> _deviceType;
};

class VSDeviceSerial {
public:
    VSDeviceSerial(): _deviceSerial{ 0 } {}
    VSDeviceSerial( const VSDeviceSerial& ) = default;
    VSDeviceSerial( const VirgilIoTKit::vs_device_type_t& buf ) { set( buf ); }

    VSDeviceSerial& operator=( const VSDeviceSerial& device_serial )  { return set( device_serial ); }
    VSDeviceSerial& operator=( const VirgilIoTKit::vs_device_serial_t& buf )       { return set( buf ); }
    bool operator==( const VSDeviceSerial &device_serial ) const     { return equal( device_serial ); }

    VSDeviceSerial& set( const VSDeviceSerial& device_serial );
    VSDeviceSerial& set( const VirgilIoTKit::vs_device_serial_t& buf );

    std::string describe( bool two_symbols = false, bool upper_case = false, char divider = ':' ) const;

    bool equal( const VSDeviceSerial &device_serial ) const { return _deviceSerial == device_serial._deviceSerial; }
    operator const uint8_t* () const;
    operator std::string() const    { return describe(); }

private:
    std::array<uint8_t, VS_DEVICE_SERIAL_SIZE> _deviceSerial;
};

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

inline VSMac _broadcastMac( 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF );

#endif // _VIRGIL_IOTKIT_QT_SNAP_PROTOCOL_H_
