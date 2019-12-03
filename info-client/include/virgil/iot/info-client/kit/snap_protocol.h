//
// Created by Oleksandr Nemchenko on 23.11.2019.
//

#ifndef VIRGIL_IOT_INFO_CLIENT_KIT_SNAP_PROTOCOL_H_
#define VIRGIL_IOT_INFO_CLIENT_KIT_SNAP_PROTOCOL_H_

#include <virgil/iot/status_code/status_code.h>
#include <virgil/iot/provision/provision-structs.h>
#include <virgil/iot/protocols/snap/snap-structs.h>

namespace VirgilIoTKit {

    using TData = std::vector<std::uint8_t>;

    class CSnapService;

    class CMac {
    public:
        CMac(): _mac{ 0 }   {}
        CMac( const CMac& ) = default;
        CMac( const vs_mac_addr_t &mac ) { set( mac ); }
        CMac( const uint8_t *bytes )     { set( bytes ); }
        CMac( uint8_t b0, uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4, uint8_t b5 )     { set( b0, b1, b2, b3, b4, b5 ); }

        CMac &operator= ( const vs_mac_addr_t &mac )    { return set( mac ); }
        CMac &operator= ( const CMac &mac )             { return set( mac ); }
        bool operator==( const CMac &mac ) const        { return equal( mac ); }

        CMac& set( const vs_mac_addr_t &mac );
        CMac& set( const uint8_t *bytes );
        CMac& set( uint8_t b0, uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4, uint8_t b5 );
        CMac& set( const CMac &mac );

        std::string describe( bool two_symbols = false, bool upper_case = false, char divider = ':' ) const;

        bool equal( const CMac &mac ) const { return _mac == mac._mac; }
        operator vs_mac_addr_t() const;
        operator const uint8_t*() const { return _mac.data(); }
        operator std::string() const    { return describe(); }

    private:
        std::array<uint8_t, 6> _mac;
    };

    class CManufactureId {
    public:
        CManufactureId(): _manufactureId{ 0 } {}
        CManufactureId( const CManufactureId& ) = default;
        CManufactureId( const vs_device_manufacture_id_t& buf ) { set( buf ); }

        CManufactureId& operator=( const CManufactureId& manufacture_id )   { return set( manufacture_id ); }
        CManufactureId& operator=( const vs_device_manufacture_id_t& buf )  { return set( buf ); }
        bool operator==( const CManufactureId &manufacture_id ) const       { return equal( manufacture_id ); }

        CManufactureId& set( const CManufactureId& manufacture_id );
        CManufactureId& set( const vs_device_manufacture_id_t& buf );

        std::string describe( bool stop_on_zero = true, char symbol_on_non_ascii = ' ' ) const;

        bool equal( const CManufactureId &manufacture_id ) const { return _manufactureId == manufacture_id._manufactureId; }
        operator const uint8_t* () const;
        operator std::string() const    { return describe(); }

    private:
        std::array<uint8_t, VS_DEVICE_MANUFACTURE_ID_SIZE> _manufactureId;
    };

    class CDeviceType {
    public:
        CDeviceType(): _deviceType{ 0 } {}
        CDeviceType( const CDeviceType& ) = default;
        CDeviceType( const vs_device_type_t& buf ) { set( buf ); }

        CDeviceType& operator=( const CDeviceType& device_type )    { return set( device_type ); }
        CDeviceType& operator=( const vs_device_type_t& buf )       { return set( buf ); }
        bool operator==( const CDeviceType &device_type ) const     { return equal( device_type ); }

        CDeviceType& set( const CDeviceType& device_type );
        CDeviceType& set( const vs_device_type_t& buf );

        std::string describe( bool stop_on_zero = true, char symbol_on_non_ascii = ' ' ) const;

        bool equal( const CDeviceType &device_type ) const { return _deviceType == device_type._deviceType; }
        operator const uint8_t* () const;
        operator std::string() const    { return describe(); }

    private:
        std::array<uint8_t, VS_DEVICE_TYPE_SIZE> _deviceType;
    };

    class CDeviceSerial {
    public:
        CDeviceSerial(): _deviceSerial{ 0 } {}
        CDeviceSerial( const CDeviceSerial& ) = default;
        CDeviceSerial( const vs_device_type_t& buf ) { set( buf ); }

        CDeviceSerial& operator=( const CDeviceSerial& device_serial )  { return set( device_serial ); }
        CDeviceSerial& operator=( const vs_device_serial_t& buf )       { return set( buf ); }
        bool operator==( const CDeviceSerial &device_serial ) const     { return equal( device_serial ); }

        CDeviceSerial& set( const CDeviceSerial& device_serial );
        CDeviceSerial& set( const vs_device_serial_t& buf );

        std::string describe( bool two_symbols = false, bool upper_case = false, char divider = ':' ) const;

        bool equal( const CDeviceSerial &device_serial ) const { return _deviceSerial == device_serial._deviceSerial; }
        operator const uint8_t* () const;
        operator std::string() const    { return describe(); }

    private:
        std::array<uint8_t, VS_DEVICE_SERIAL_SIZE> _deviceSerial;
    };

    class CSnapProtocol {
    public:
        using FChangeStateNotify = std::function<void( const std::string & )>;

        CSnapProtocol();
        virtual ~CSnapProtocol();

        bool init( const CManufactureId &manufacture_id, const CDeviceType &device_type, const CDeviceSerial &device_serial,
                vs_snap_device_role_e device_roles, FChangeStateNotify change_state_notify );
        bool registerService( CSnapService &snap_service );

        const vs_netif_t* netIf() const { return &_networkInterface; }
        virtual const CMac& ownMacAddress() const = 0;

    protected:
        enum class EState{ NotInitialized, Initialized, Destructed };
        using TMacResponse = std::pair<vs_status_e, CMac>;

        EState state() const { return _state; }

        virtual vs_status_e initInterface() = 0;
        virtual vs_status_e destroy() = 0;
        virtual vs_status_e sendRawData( TData &&data ) = 0;
        virtual TMacResponse ownMac() const = 0;

        void processRxData( TData &&data );
        void сhangeStateNotify( const std::string &description );

    private:
        static CSnapProtocol *_instance;
        vs_netif_t _networkInterface;
        EState _state;
        FChangeStateNotify _changeStateNotify;

        static vs_status_e netIfDeinit( struct vs_netif_t* netif );
        static vs_status_e netIfInit( struct vs_netif_t* netif, const vs_netif_rx_cb_t rx_cb, const vs_netif_process_cb_t process_cb );
        static vs_status_e netIfTx( struct vs_netif_t* netif, const uint8_t* data, const uint16_t data_sz );
        static vs_status_e netIfMac( const struct vs_netif_t* netif, vs_mac_addr_t* mac_addr );

        vs_netif_rx_cb_t _netIfRxCallback;
        vs_netif_process_cb_t _netIfRxProcessCallback;
    };

} // namespace VirgilIoTKit

extern inline VirgilIoTKit::CMac _broadcastMac( 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF );

#endif // VIRGIL_IOT_INFO_CLIENT_KIT_SNAP_PROTOCOL_H_
