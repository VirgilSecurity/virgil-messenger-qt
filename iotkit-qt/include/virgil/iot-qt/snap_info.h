//
// Created by Oleksandr Nemchenko on 23.11.2019.
//

#ifndef VIRGIL_IOT_INFO_CLIENT_KIT_SNAP_INFO_SERVICE_H_
#define VIRGIL_IOT_INFO_CLIENT_KIT_SNAP_INFO_SERVICE_H_

#include <vector>

#include <QtCore>

#include <virgil/iot-qt/snap_protocol.h>
#include <virgil/iot-qt/snap_service.h>

#include <virgil/iot/protocols/snap/info/info-structs.h>
#include <virgil/iot/protocols/snap/info/info-client.h>

class VSSnapInfoClient : public QObject, public VSSnapService {
    Q_OBJECT

public:
    VSSnapInfoClient();
    ~VSSnapInfoClient() override;

    using TEnumDevicesArray = std::vector<VirgilIoTKit::vs_snap_info_device_t>;

    const VirgilIoTKit::vs_snap_service_t *serviceInterface() override    { return _snapService; }
    const std::string &serviceName() const override;

    TEnumDevicesArray enumDevices( size_t wait_msec, size_t max_devices_amount = 1000 ) const;
    bool changePolling( size_t polling_element, uint16_t period_seconds, bool enable, const VSMac &device_mac = _broadcastMac ) const;
    bool setPolling(    size_t polling_element, uint16_t period_seconds, const VSMac &device_mac = _broadcastMac ) const { return changePolling( polling_element, period_seconds, true,  device_mac ); }
    bool resetPoling(   size_t polling_element, uint16_t period_seconds, const VSMac &device_mac = _broadcastMac ) const { return changePolling( polling_element, period_seconds, false, device_mac ); }

signals:
    void deviceStarted( VirgilIoTKit::vs_snap_info_device_t &device );
    void deviceGeneralInfo( VirgilIoTKit::vs_info_general_t &general_data );
    void deviceStatistics( VirgilIoTKit::vs_info_statistics_t &statistics );

private:
    static VSSnapInfoClient *_instance;
    static const VirgilIoTKit::vs_snap_service_t * _snapService;
    mutable VirgilIoTKit::vs_snap_info_client_service_t _snapInfoImpl;
    struct DeviceInfo{};
    std::vector<DeviceInfo> _devices;

    static VirgilIoTKit::vs_status_e startNotify( VirgilIoTKit::vs_snap_info_device_t *device );
    static VirgilIoTKit::vs_status_e generalInfo( VirgilIoTKit::vs_info_general_t *general_data );
    static VirgilIoTKit::vs_status_e statistics( VirgilIoTKit::vs_info_statistics_t *statistics );

};

#endif // VIRGIL_IOT_INFO_CLIENT_KIT_SNAP_INFO_SERVICE_H_
