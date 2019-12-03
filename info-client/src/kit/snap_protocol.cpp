//
// Created by Oleksandr Nemchenko on 23.11.2019.
//

#include <app.h>

#include <kit/snap_protocol.h>
#include <kit/snap_service.h>

#include <virgil/iot/protocols/snap.h>

VirgilIoTKit::CMac& VirgilIoTKit::CMac::set( const vs_mac_addr_t &mac ) {
    std::copy( mac.bytes, mac.bytes + sizeof( mac.bytes ), _mac.begin() );
    return *this;
}

VirgilIoTKit::CMac& VirgilIoTKit::CMac::set( const uint8_t *bytes ) {
    assert( bytes );
    std::copy( bytes, bytes + _mac.size(), _mac.begin() );
    return *this;
}

VirgilIoTKit::CMac& VirgilIoTKit::CMac::set( uint8_t b0, uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4, uint8_t b5 ) {
    _mac[0] = b0; _mac[1] = b1; _mac[2] = b2; _mac[3] = b3; _mac[4] = b4; _mac[5] = b5;
    return *this;
}

VirgilIoTKit::CMac& VirgilIoTKit::CMac::set( const CMac &mac ) {
    _mac = mac._mac;
    return *this;
}

VirgilIoTKit::CMac::operator VirgilIoTKit::vs_mac_addr_t() const {
    vs_mac_addr_t mac;
    std::copy( _mac.begin(), _mac.end(), mac.bytes );
    return mac;
}

std::string VirgilIoTKit::CMac::describe( bool two_symbols, bool upper_case, char divider ) const {
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

VirgilIoTKit::CManufactureId& VirgilIoTKit::CManufactureId::set( const CManufactureId& manufacture_id ) {
    _manufactureId = manufacture_id._manufactureId;
    return *this;
}

VirgilIoTKit::CManufactureId& VirgilIoTKit::CManufactureId::set( const vs_device_manufacture_id_t& buf ){
    std::copy( buf, buf + sizeof( vs_device_manufacture_id_t ), _manufactureId.begin() );
    return *this;
}

VirgilIoTKit::CManufactureId::operator const uint8_t* () const {
    return _manufactureId.data();
}

std::string VirgilIoTKit::CManufactureId::describe( bool stop_on_zero, char symbol_on_non_ascii ) const {
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

VirgilIoTKit::CDeviceType& VirgilIoTKit::CDeviceType::set( const CDeviceType& device_type ) {
    _deviceType = device_type._deviceType;
    return *this;
}

VirgilIoTKit::CDeviceType& VirgilIoTKit::CDeviceType::set( const vs_device_type_t& buf ){
    std::copy( buf, buf + sizeof( vs_device_type_t ), _deviceType.begin() );
    return *this;
}

VirgilIoTKit::CDeviceType::operator const uint8_t* () const{
    return _deviceType.data();
}

std::string VirgilIoTKit::CDeviceType::describe( bool stop_on_zero, char symbol_on_non_ascii ) const {
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
VirgilIoTKit::CDeviceSerial& VirgilIoTKit::CDeviceSerial::set( const CDeviceSerial& device_serial ) {
    _deviceSerial = device_serial._deviceSerial;
    return *this;
}

VirgilIoTKit::CDeviceSerial& VirgilIoTKit::CDeviceSerial::set( const vs_device_serial_t& buf ){
    std::copy( buf, buf + sizeof( vs_device_type_t ), _deviceSerial.begin() );
    return *this;
}

VirgilIoTKit::CDeviceSerial::operator const uint8_t* () const{
    return _deviceSerial.data();
}

std::string VirgilIoTKit::CDeviceSerial::describe( bool two_symbols, bool upper_case, char divider ) const {
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

/* static */ VirgilIoTKit::CSnapProtocol * VirgilIoTKit::CSnapProtocol::_instance = nullptr;

VirgilIoTKit::CSnapProtocol::CSnapProtocol(){
    assert( !_instance );
    _instance = this;

}

VirgilIoTKit::CSnapProtocol::~CSnapProtocol(){
    assert( _instance );
    _instance = nullptr;
}

bool VirgilIoTKit::CSnapProtocol::init( const CManufactureId &manufacture_id, const CDeviceType &device_type, const CDeviceSerial &device_serial,
           vs_snap_device_role_e device_roles, FChangeStateNotify change_state_notify ) {

    _changeStateNotify = change_state_notify;

    _networkInterface.user_data = this;
    _networkInterface.init = netIfInit;
    _networkInterface.deinit = netIfDeinit;
    _networkInterface.tx = netIfTx;
    _networkInterface.mac_addr = netIfMac;

    _state = EState::NotInitialized;

    if( VirgilIoTKit::vs_snap_init( &_networkInterface, manufacture_id, device_type, device_serial, device_roles ) != VS_CODE_OK ) {
        VS_LOG_CRITICAL( "Unable to initialize Virgil IoTKIT::SNAP protocol" );
        return false;
    }

    return true;
}

void VirgilIoTKit::CSnapProtocol::processRxData( VirgilIoTKit::TData &&data ) {
    if( !_netIfRxCallback )
        return;

    const uint8_t *packet_data = nullptr;
    uint16_t packet_data_sz = 0;

    if( _netIfRxCallback( &_networkInterface, data.data(), data.size(), &packet_data, &packet_data_sz ) != VirgilIoTKit::VS_CODE_OK )
        return;

    if( _netIfRxProcessCallback( &_networkInterface, packet_data, packet_data_sz ) != VirgilIoTKit::VS_CODE_OK )
        VS_LOG_ERROR( "Unable to process received packet" );
}

void VirgilIoTKit::CSnapProtocol::сhangeStateNotify( const std::string &description ) {
    _changeStateNotify( description );
}

/* static */ VirgilIoTKit::vs_status_e VirgilIoTKit::CSnapProtocol::netIfInit( struct vs_netif_t* netif, const vs_netif_rx_cb_t rx_cb, const vs_netif_process_cb_t process_cb ){
    (void) netif;

    assert( _instance );

    _instance->_netIfRxCallback = rx_cb;
    _instance->_netIfRxProcessCallback = process_cb;

    return _instance->initInterface();
}

/* static */ VirgilIoTKit::vs_status_e VirgilIoTKit::CSnapProtocol::netIfDeinit( struct vs_netif_t* netif ){
    (void) netif;

    assert( _instance );

    return _instance->destroy();

}

/* static */ VirgilIoTKit::vs_status_e VirgilIoTKit::CSnapProtocol::netIfTx( struct vs_netif_t* netif, const uint8_t* data, const uint16_t data_sz ){
    (void) netif;

    assert( _instance );

    return _instance->sendRawData( TData( data, data + data_sz ));

}

/* static */ VirgilIoTKit::vs_status_e VirgilIoTKit::CSnapProtocol::netIfMac( const struct vs_netif_t* netif, vs_mac_addr_t* mac_addr ){
    (void) netif;

    assert( _instance );

    auto [ ret_code, mac ] = _instance->ownMac();

    *mac_addr = mac;

    return ret_code;

}

bool VirgilIoTKit::CSnapProtocol::registerService( CSnapService &snap_service ) {
    vs_status_e ret_code;

    if( VirgilIoTKit::vs_snap_register_service( snap_service.serviceInterface() ) != VirgilIoTKit::VS_CODE_OK ) {
        VS_LOG_CRITICAL( "Unable to register service \"", snap_service.serviceName().c_str(), "\"" );
        return false;
    }

    snap_service.setSnapProtocol( this );

    return true;
}
