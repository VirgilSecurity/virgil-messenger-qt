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

#include <app.h>

#include <virgil/iot-qt/snap_udp.h>
#include <virgil/iot-qt/snap_info.h>
#include <virgil/iot-qt/logger.h>
#include <virgil/iot/protocols/snap/snap-structs.h>
#include <main_window.h>

using namespace VirgilIoTKit;

void InitSnap( VSSnapUdp *snap_udp, VSSnapInfoClient *snap_info_client, VSMainWindow *main_window ) {
    using namespace VirgilIoTKit;

    VSManufactureId manufacture_id;
    VSDeviceType device_type;
    VSDeviceSerial device_serial;
    quint16 udp_bind_port = 4100;
    VirgilIoTKit::vs_snap_device_role_e device_role = VirgilIoTKit::VS_SNAP_DEV_SNIFFER;

    snap_udp->setBindPort( udp_bind_port );

    if( !snap_udp->init( manufacture_id, device_type, device_serial, device_role, [main_window]( const std::string &notify_message ) {
        main_window->changeConnectionState( notify_message );
    } )) {
        throw std::runtime_error( "SNAP through UDP initialization error" );
    }

    if( !snap_udp->registerService( *snap_info_client )) {
        throw std::runtime_error( "Unable to initialize SNAP:INFO Client service" );
    }

    if( !QObject::connect( snap_info_client, &VSSnapInfoClient::deviceStarted, main_window, &VSMainWindow::deviceStarted )) {
        throw std::runtime_error( "Unable to connect VSSnapInfoClient::deviceStarted to VSMainWindow::deviceStarted" );
    }

    if( !QObject::connect( snap_info_client, &VSSnapInfoClient::deviceGeneralInfo, main_window, &VSMainWindow::deviceGeneralInfo )) {
        throw std::runtime_error( "Unable to connect VSSnapInfoClient::deviceGeneralInfo to VSMainWindow::deviceGeneralInfo" );
    }

}

void InitInfoClient( VSSnapInfoClient *info_client, VSMainWindow *main_window ) {
    constexpr auto polling_period = 1;
    constexpr auto polling_mask = VirgilIoTKit::VS_SNAP_INFO_GENERAL | VirgilIoTKit::VS_SNAP_INFO_STATISTICS;

    if( !info_client->setPolling( polling_mask, polling_period )){
        throw std::runtime_error( "Unable to set data polling" );
    }

    if( !QObject::connect( info_client, &VSSnapInfoClient::deviceStatistics, main_window, &VSMainWindow::deviceStatistics )) {
        throw std::runtime_error( "Unable to connect VSSnapInfoClient::deviceStatistics to VSMainWindow::deviceStatistics" );
    }

    if( !QObject::connect( info_client, &VSSnapInfoClient::deviceStarted, [info_client]( VirgilIoTKit::vs_snap_info_device_t &device ) {
        info_client->setPolling( polling_mask, polling_period, device.mac );
    } )) {
        throw std::runtime_error( "Unable to connect VSSnapInfoClient::deviceStatistics to VSMainWindow::deviceStatistics" );
    }
}

int main( int argc, char *argv[] )
{
    QApplication app( argc, argv );
    int app_ret_code;

    VirgilIoTKit::vs_logger_init( VirgilIoTKit::VS_LOGLEV_DEBUG );
    VS_LOG_INFO( "Start Virgil IoT network analyzer" );
//    VS_LOG_INFO( "Git commit ", COMPILE_GIT_HASH );


    try{
        VSMainWindow main_window;
        VSSnapUdp snap_udp;
        VSSnapInfoClient snap_info_client;

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
