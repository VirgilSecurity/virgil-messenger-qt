//
// Created by Oleksandr Nemchenko on 23.11.2019.
//

#include <virgil/iot-qt/snap_info.h>

using namespace VirgilIoTKit;

/* static */ VSSnapInfoClient * VSSnapInfoClient::_instance = nullptr;
/* static */ const vs_snap_service_t * VSSnapInfoClient::_snapService = nullptr;

VSSnapInfoClient::VSSnapInfoClient(){

    assert( !_instance );

    _instance = this;

    _snapInfoImpl.device_start = startNotify;
    _snapInfoImpl.general_info = generalInfo;
    _snapInfoImpl.statistics = statistics;

    _snapService = vs_snap_info_client( _snapInfoImpl );
}

VSSnapInfoClient::~VSSnapInfoClient(){
    assert( _instance );

    _instance = nullptr;

}

const std::string & VSSnapInfoClient::serviceName() const {
    using namespace std;
    static const std::string service_name = "INFO"s;

    return service_name;
}

/* static */ vs_status_e VSSnapInfoClient::startNotify( vs_snap_info_device_t *device ){
    assert( _instance );

    emit _instance->deviceStarted( *device );

    return VS_CODE_OK;
}

/* static */ vs_status_e VSSnapInfoClient::generalInfo( vs_info_general_t *general_data ){
    assert( _instance );

    emit _instance->deviceGeneralInfo( *general_data );

    return VS_CODE_OK;
}

/* static */ vs_status_e VSSnapInfoClient::statistics( vs_info_statistics_t *statistics ){
    assert( _instance );

    emit _instance->deviceStatistics( *statistics );

    return VS_CODE_OK;
}

VSSnapInfoClient::TEnumDevicesArray VSSnapInfoClient::enumDevices( size_t wait_msec, size_t max_devices_amount ) const {
    TEnumDevicesArray devices;
    size_t devices_amount;
    vs_status_e ret_code;

    devices.resize( max_devices_amount );
    ret_code = vs_snap_info_enum_devices( snapProtocol().netIf(), devices.data(), devices.size(), &devices_amount, wait_msec );

    devices.resize( devices_amount );

    return devices;
}

bool VSSnapInfoClient::changePolling( size_t pooling_element, uint16_t period_seconds, bool enable, const VSMac &device_mac) const {
    vs_mac_addr_t mac = device_mac;

    if( vs_snap_info_set_polling( snapProtocol().netIf(), &mac, pooling_element, enable, period_seconds ) != VS_CODE_OK ) {
        VS_LOG_ERROR( "Unable to setup info polling" );
        return false;
    }

    return true;
}
