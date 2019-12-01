
#include <app.h>

#include <kit/snap_udp.h>
#include <kit/snap_info.h>
#include <kit/logger.h>
#include <main_window.h>
#include <statistics.h>

using namespace VirgilIoTKit;

CAppSettings _appSettings;

void InitSnap( VirgilIoTKit::CSnapUdp *snap_udp, VirgilIoTKit::CSnapInfoClient *snap_info_client, CMainWindow *main_window ) {
    using namespace VirgilIoTKit;

    CManufactureId manufacture_id;
    CDeviceType device_type;
    CDeviceSerial device_serial;
    quint16 udp_bind_port = 4100;
    vs_snap_device_role_e device_role = VS_SNAP_DEV_SNIFFER;

    snap_udp->setBindPort( udp_bind_port );

    if( !snap_udp->init( manufacture_id, device_type, device_serial, device_role, [main_window]( const std::string &notify_message ) {
        main_window->changeConnectionState( notify_message );
    } )) {
        throw std::runtime_error( "SNAP through UDP initialization error" );
    }

    if( !snap_udp->registerService( *snap_info_client )) {
        throw std::runtime_error( "Unable to initialize SNAP:INFO Client service" );
    }

    if( !QObject::connect( snap_info_client, &CSnapInfoClient::deviceStarted, main_window, &CMainWindow::deviceStarted )) {
        throw std::runtime_error( "Unable to connect CSnapInfoClient::deviceStarted to CMainWindow::deviceStarted" );
    }

    if( !QObject::connect( snap_info_client, &CSnapInfoClient::deviceGeneralInfo, main_window, &CMainWindow::deviceGeneralInfo )) {
        throw std::runtime_error( "Unable to connect CSnapInfoClient::deviceGeneralInfo to CMainWindow::deviceGeneralInfo" );
    }

}

void InitInfoClient( VirgilIoTKit::CSnapInfoClient *info_client, CMainWindow *main_window ) {
    constexpr auto polling_period = 1;
    constexpr auto polling_mask = VS_SNAP_INFO_GENERAL | VS_SNAP_INFO_STATISTICS;

    if( !info_client->setPolling( polling_mask, polling_period )){
        throw std::runtime_error( "Unable to set data polling" );
    }

    if( !QObject::connect( info_client, &CSnapInfoClient::deviceStatistics, main_window, &CMainWindow::deviceStatistics )) {
        throw std::runtime_error( "Unable to connect CSnapInfoClient::deviceStatistics to CMainWindow::deviceStatistics" );
    }

    if( !QObject::connect( info_client, &CSnapInfoClient::deviceStarted, [info_client]( vs_snap_info_device_t &device ) {
        info_client->setPolling( polling_mask, polling_period, device.mac );
    } )) {
        throw std::runtime_error( "Unable to connect CSnapInfoClient::deviceStatistics to CMainWindow::deviceStatistics" );
    }
}

int main( int argc, char *argv[] )
{
    QApplication app( argc, argv );
    int app_ret_code;

    VirgilIoTKit::vs_logger_init( VirgilIoTKit::VS_LOGLEV_DEBUG );
    VS_LOG_INFO( "Start Virgil IoT network analyzer" );
    VS_LOG_INFO( "Git commit ", COMPILE_GIT_HASH );

    try{
        CMainWindow main_window;
        VirgilIoTKit::CSnapUdp snap_udp;
        VirgilIoTKit::CSnapInfoClient snap_info_client;
        CStatistics statistics;

        statistics.init();

        main_window.show();

        InitSnap( &snap_udp, &snap_info_client, &main_window );
        InitInfoClient( &snap_info_client, &main_window );

        app_ret_code = app.exec();
    }
    catch( const std::runtime_error &error ){
        VS_LOG_FATAL( "Runtime error has been happened : ", error.what() );
        app_ret_code = -1;
    }
    catch( ... ){
        VS_LOG_FATAL( "Unknown error has been happened" );
        app_ret_code = -2;
    }

    return app_ret_code;
}
