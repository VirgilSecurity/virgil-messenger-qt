//
// Created by Oleksandr Nemchenko on 23.11.2019.
//

#ifndef VIRGIL_IOT_INFO_CLIENT_KIT_SNAP_INFO_SERVICE_H_
#define VIRGIL_IOT_INFO_CLIENT_KIT_SNAP_INFO_SERVICE_H_

#include <kit/snap_protocol.h>
#include <kit/snap_service.h>

#include <virgil/iot/protocols/snap/info/info-structs.h>
#include <virgil/iot/protocols/snap/info/info-client.h>

namespace VirgilIoTKit {

    class CSnapInfoClient : public QObject, public CSnapService {
        Q_OBJECT

    public:
        CSnapInfoClient();
        ~CSnapInfoClient() override;

        using TEnumDevicesArray = std::vector<vs_snap_info_device_t>;

        const vs_snap_service_t *serviceInterface() const override;
        const std::string &serviceName() const override;

        TEnumDevicesArray enumDevices( size_t wait_msec, size_t max_devices_amount = 1000 ) const;
        bool changePolling( size_t polling_element, uint16_t period_seconds, bool enable, const CMac &device_mac = _broadcastMac ) const;
        bool setPolling(    size_t polling_element, uint16_t period_seconds, const CMac &device_mac = _broadcastMac ) const { return changePolling( polling_element, period_seconds, true,  device_mac ); }
        bool resetPoling(   size_t polling_element, uint16_t period_seconds, const CMac &device_mac = _broadcastMac ) const { return changePolling( polling_element, period_seconds, false, device_mac ); }

    signals:
        void deviceStarted( vs_snap_info_device_t &device );
        void deviceGeneralInfo( vs_info_general_t &general_data );
        void deviceStatistics( vs_info_statistics_t &statistics );

    private:
        static CSnapInfoClient *_instance;
        static const vs_snap_service_t * _snapService;
        mutable vs_snap_info_client_service_t _snapInfoImpl;
        struct DeviceInfo{};
        std::vector<DeviceInfo> _devices;

        static vs_status_e startNotify( vs_snap_info_device_t *device );
        static vs_status_e generalInfo( vs_info_general_t *general_data );
        static vs_status_e statistics(  vs_info_statistics_t *statistics );

    };

} // namespace VirgilIoTKit

#endif // VIRGIL_IOT_INFO_CLIENT_KIT_SNAP_INFO_SERVICE_H_
