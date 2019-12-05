//
// Created by Oleksandr Nemchenko on 25.11.2019.
//

#include <app.h>

#include <main_window.h>

VSMainWindow::VSMainWindow() {
}

VSMainWindow::~VSMainWindow() {
}

void VSMainWindow::show() {
    static const std::map<EColumn, QString> tbl_headers = { { EColumn::MAC, "MAC" }, { EColumn::DevRoles, "Device Roles" }, { EColumn::Manufacture, "Manufacture" }, { EColumn::Type, "Type" }, { EColumn::FWVersion, "Firmware Version" }, { EColumn::TLVersion, "Trust List Version" }, { EColumn::ReceivedAmount, "Receives" }, { EColumn::SentAmount, "Sends" } };

    _tbl.setColumnCount( static_cast<int>( EColumn::ColumnsAmount ));
    _tbl.setAlternatingRowColors( true );

    for( const auto& [ column_id, title ] : tbl_headers )
        _tbl.setHorizontalHeaderItem( static_cast<int>( column_id ), new QTableWidgetItem( title ));

    _wnd.setCentralWidget( &_tbl );
    _wnd.showNormal();

    _connectionState = "No connection";
    changeStatusBar();
}

void VSMainWindow::changeConnectionState( const std::string &connection_state ) {
    _connectionState = QString::fromStdString( connection_state );
    changeStatusBar();
}

void VSMainWindow::changeStatusBar() {
    QString str;

    if( _devices.empty() )
        str = "No devices";
    else if ( _devices.size() == 1 )
        str = "1 device";
    else
        str = QString( "%1 devices" ). arg( _devices.size() );

    str += " *** ";
    str += _connectionState;

    _wnd.statusBar()->showMessage( str );
}

VSMainWindow::SDeviceInfo& VSMainWindow::device( const VirgilIoTKit::VSMac &mac ) {
    for( auto &device : _devices ){
        if( device._mac == mac ) {
            return device;
        }
    }

    _devices.emplace_back();
    return _devices.back();
}

void VSMainWindow::deviceStarted( VirgilIoTKit::vs_snap_info_device_t &started_device ){

    SDeviceInfo& dev = device( started_device.mac );

    dev._mac = started_device.mac;
    dev._roles = static_cast<VirgilIoTKit::vs_snap_device_role_e>( started_device.device_roles );

    updateTable();
}

void VSMainWindow::deviceGeneralInfo( VirgilIoTKit::vs_info_general_t &general_data ){
    SDeviceInfo& dev = device( general_data.default_netif_mac );

    dev._manufactureId = general_data.manufacture_id;
    dev._deviceType = general_data.device_type;
    dev._roles = static_cast<VirgilIoTKit::vs_snap_device_role_e>( general_data.device_roles );
    dev._fwVer = general_data.fw_ver;
    dev._tlVer = general_data.tl_ver;
    dev._hasGeneralInfo = true;

    updateTable();
}

void VSMainWindow::deviceStatistics( VirgilIoTKit::vs_info_statistics_t &statistics ){
    SDeviceInfo& dev = device( statistics.default_netif_mac );

    dev._received = statistics.received;
    dev._sent = statistics.sent;
    dev._hasStatistics = true;

    updateTable();
}

void VSMainWindow::updateTable() {
    size_t row = 0;

    _tbl.setRowCount(0);
    _tbl.setRowCount( _devices.size() );

    for( const auto &device : _devices ) {
        QString text;

        text = QString::fromStdString( device._mac );

        _tbl.setItem( row, static_cast<int>( EColumn::MAC ), new QTableWidgetItem( text ));

        text = "";
        if( device._roles & VirgilIoTKit::VS_SNAP_DEV_GATEWAY ) text += ", Gateway";
        if( device._roles & VirgilIoTKit::VS_SNAP_DEV_THING ) text += ", Thing";
        if( device._roles & VirgilIoTKit::VS_SNAP_DEV_CONTROL ) text += ", Control";
        if( device._roles & VirgilIoTKit::VS_SNAP_DEV_LOGGER ) text += ", Logger";
        if( device._roles & VirgilIoTKit::VS_SNAP_DEV_SNIFFER ) text += ", Sniffer";
        if( device._roles & VirgilIoTKit::VS_SNAP_DEV_DEBUGGER ) text += ", Debugger";
        if( device._roles & VirgilIoTKit::VS_SNAP_DEV_INITIALIZER ) text += ", Initializer";
        text.remove(0, 2);
        _tbl.setItem( row, static_cast<int>( EColumn::DevRoles ), new QTableWidgetItem( text ));

        text = "";
        if( device._hasGeneralInfo ) {
            text = QString::fromStdString( device._manufactureId );
        }
        _tbl.setItem( row, static_cast<int>( EColumn::Manufacture ), new QTableWidgetItem( text ));

        text = "";
        if( device._hasGeneralInfo ) {
            text = QString::fromStdString( device._deviceType );
        }
        _tbl.setItem( row, static_cast<int>( EColumn::Type ), new QTableWidgetItem( text ));

        text = "";
        if( device._hasGeneralInfo ) {
            text = QString( "%1.%2.%3.%4" ).
                    arg( static_cast<int>(device._fwVer.major) ).
                    arg( static_cast<int>(device._fwVer.minor) ).
                    arg( static_cast<int>(device._fwVer.patch) ).
                    arg( static_cast<int>(device._fwVer.build) );
        }
        _tbl.setItem( row, static_cast<int>( EColumn::FWVersion ), new QTableWidgetItem( text ));

        text = "";
        if( device._hasGeneralInfo ) {
            text = QString( "%1.%2.%3.%4" ).
                    arg( static_cast<int>(device._tlVer.major) ).
                    arg( static_cast<int>(device._tlVer.minor) ).
                    arg( static_cast<int>(device._tlVer.patch) ).
                    arg( static_cast<int>(device._tlVer.build) );
        }
        _tbl.setItem( row, static_cast<int>( EColumn::TLVersion ), new QTableWidgetItem( text ));

        text = "";
        if( device._hasStatistics ) {
            text = QString( "%1" ). arg(  device._sent );
        }
        _tbl.setItem( row, static_cast<int>( EColumn::SentAmount ), new QTableWidgetItem( text ));

        text = "";
        if( device._hasStatistics ) {
            text = QString( "%1" ). arg(  device._received );
        }
        _tbl.setItem( row, static_cast<int>( EColumn::ReceivedAmount ), new QTableWidgetItem( text ));

        ++row;
    }

    changeStatusBar();
}
