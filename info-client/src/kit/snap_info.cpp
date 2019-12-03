//
// Created by Oleksandr Nemchenko on 23.11.2019.
//

#include <app.h>

#include <kit/snap_info.h>

/* static */ VirgilIoTKit::CSnapInfoClient * VirgilIoTKit::CSnapInfoClient::_instance = nullptr;
/* static */ VirgilIoTKit::vs_snap_service_t * VirgilIoTKit::CSnapInfoClient::_snapService = nullptr;

VirgilIoTKit::CSnapInfoClient::CSnapInfoClient(){

    assert( !_instance );

    _instance = this;

    _snapInfoImpl.device_start = startNotify;
    _snapInfoImpl.general_info = generalInfo;
    _snapInfoImpl.statistics = statistics;

    _snapService = vs_snap_info_client( _snapInfoImpl );
}

VirgilIoTKit::CSnapInfoClient::~CSnapInfoClient(){
    assert( _instance );

    _instance = nullptr;

}

const std::string & VirgilIoTKit::CSnapInfoClient::serviceName() const {
    using namespace std;
    static const std::string service_name = "INFO"s;

    return service_name;
}

/* static */ VirgilIoTKit::vs_status_e VirgilIoTKit::CSnapInfoClient::startNotify( struct vs_snap_service_t* service, vs_snap_info_device_t *device ){
    assert( _instance );

    emit _instance->deviceStarted( *device );

    return VirgilIoTKit::VS_CODE_OK;
}

/* static */ VirgilIoTKit::vs_status_e VirgilIoTKit::CSnapInfoClient::generalInfo( struct vs_snap_service_t* service, vs_info_general_t *general_data ){
    assert( _instance );

    emit _instance->deviceGeneralInfo( *general_data );

    return VirgilIoTKit::VS_CODE_OK;
}

/* static */ VirgilIoTKit::vs_status_e VirgilIoTKit::CSnapInfoClient::statistics( struct vs_snap_service_t* service, vs_info_statistics_t *statistics ){
    assert( _instance );

    emit _instance->deviceStatistics( *statistics );

    return VirgilIoTKit::VS_CODE_OK;
}

VirgilIoTKit::CSnapInfoClient::TEnumDevicesArray VirgilIoTKit::CSnapInfoClient::enumDevices( size_t wait_msec, size_t max_devices_amount ) const {
    TEnumDevicesArray devices;
    size_t devices_amount;
    VirgilIoTKit::vs_status_e ret_code;

    devices.resize( max_devices_amount );
    ret_code = VirgilIoTKit::vs_snap_info_enum_devices( snapProtocol().netIf(), devices.data(), devices.size(), &devices_amount, wait_msec );

    devices.resize( devices_amount );

    return devices;
}

bool VirgilIoTKit::CSnapInfoClient::changePolling( size_t pooling_element, uint16_t period_seconds, bool enable, const CMac &device_mac) const {
    vs_mac_addr_t mac = device_mac;

    if( VirgilIoTKit::vs_snap_info_set_polling( snapProtocol().netIf(), &mac, pooling_element, enable, period_seconds ) != VS_CODE_OK ) {
        VS_LOG_ERROR( "Unable to setup info polling" );
        return false;
    }

    return true;
}
