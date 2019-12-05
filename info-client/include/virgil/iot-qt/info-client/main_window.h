//
// Created by Oleksandr Nemchenko on 25.11.2019.
//

#ifndef HELPERS_MAIN_WINDOW_H
#define HELPERS_MAIN_WINDOW_H

#include <app.h>

#include <virgil/iot-qt/snap_protocol.h>
#include <virgil/iot/protocols/snap/info/info-structs.h>

class VSMainWindow : public QObject  {
    Q_OBJECT

public:
    VSMainWindow();
    ~VSMainWindow();

    void show();
    void changeConnectionState( const std::string &connection_state );

public slots:
    void deviceStarted( VirgilIoTKit::vs_snap_info_device_t &device );
    void deviceGeneralInfo( VirgilIoTKit::vs_info_general_t &general_data );
    void deviceStatistics( VirgilIoTKit::vs_info_statistics_t &statistics );

private:
    enum class EColumn{ MAC = 0, DevRoles, Manufacture, Type, FWVersion, TLVersion, ReceivedAmount, SentAmount, ColumnsAmount };
    struct SDeviceInfo {
        SDeviceInfo(): _hasGeneralInfo( false ), _hasStatistics( false )    {}

        VSMac _mac;
        VirgilIoTKit::vs_snap_device_role_e _roles;
        VSManufactureId _manufactureId;
        VSDeviceType _deviceType;
        VirgilIoTKit::vs_file_version_unpacked_t _fwVer;
        VirgilIoTKit::vs_file_version_unpacked_t _tlVer;
        uint32_t _sent;
        uint32_t _received;

        bool _hasGeneralInfo;
        bool _hasStatistics;
    };

    QMainWindow _wnd;
    QTableWidget _tbl;
    QStatusBar _status;
    std::vector<SDeviceInfo> _devices;
    QString _connectionState;

    void changeStatusBar();
    void updateTable();

    SDeviceInfo& device( const VSMac &mac );
};

#endif //HELPERS_MAIN_WINDOW_H
