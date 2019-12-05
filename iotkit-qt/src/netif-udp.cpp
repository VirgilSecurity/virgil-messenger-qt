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

#include <virgil/iot-qt/logger.h>
#include <virgil/iot-qt/netif-udp.h>

using namespace VirgilIoTKit;

/* static */ VSNetifUdp* VSNetifUdp::_instance = nullptr;

VSNetifUdp::VSNetifUdp( quint16 bind_port ) {
    assert( !_instance && "Only one instance is allowed" );

    _instance = this;

    _netif.user_data = this;

    _netif.deinit = netifDeinit;
    _netif.init = netifInit;
    _netif.mac_addr = netifMacAddr;
    _netif.tx = netifTx;

    bindPort( bind_port );
}

VSNetifUdp::~VSNetifUdp() {
    _instance = nullptr;
}

/* static */ vs_status_e VSNetifUdp::netifInit( const VirgilIoTKit::vs_netif_rx_cb_t rx_cb, const VirgilIoTKit::vs_netif_process_cb_t process_cb ) {
    assert( _instance && "Constructor must be called before this call" );
    assert( _instance->_port != 0 && "Port must not be zero" );

    VSLogDebug( "Set UDP broadcast mode for LocalHost:", _instance->_port );
    _instance->_socket.reset( new QUdpSocket );

    if( !connect( _instance->_socket.get(), &QAbstractSocket::stateChanged, _instance, &VSNetifUdp::connectionStateChanged )) {
        VSLogError( "Unable to connect VSSnapUdp::initInterface to the socket's QAbstractSocket::stateChanged signal" );
        return VS_CODE_ERR_SOCKET;
    }

    if( !_instance->_socket->bind( _instance->_port, QAbstractSocket::ReuseAddressHint )) {
        VSLogError( "Unable to bind LocalHost:", _instance->_port, ". Last error : ", _instance->_socket->errorString().toStdString().c_str() );
        return VS_CODE_ERR_SOCKET;
    }

    if( !connect( _instance->_socket.get(), &QUdpSocket::readyRead, _instance, &VSNetifUdp::hasInputData )) {
        VSLogError( "Unable to connect QUdpSocket::readyRead to VSSnapUdp::hasInputData " );
        return VS_CODE_ERR_SOCKET;
    }

    _instance->_netifRxCallback = rx_cb;
    _instance->_netifRxProcessCallback = process_cb;

    return VS_CODE_OK;
}

void VSNetifUdp::connectionStateChanged( QAbstractSocket::SocketState socketState ) {
    static const std::map<QAbstractSocket::SocketState, std::string> state_descr{
            { QAbstractSocket::SocketState::UnconnectedState, "Unconnected" },
            { QAbstractSocket::SocketState::HostLookupState, "Host lookup" },
            { QAbstractSocket::SocketState::ConnectingState, "Connecting" },
            { QAbstractSocket::SocketState::ConnectedState, "Connected" },
            { QAbstractSocket::SocketState::BoundState, "Bound" },
            { QAbstractSocket::SocketState::ListeningState, "Listening" },
            { QAbstractSocket::SocketState::ClosingState, "Closing" } };

    emit stateChanged( socketState, std::string( "LocalHost:" ) + std::to_string(_port) + " : " + state_descr.at( socketState ) );
    VSLogCritical( "Socket LocalHost:", _port, " has been changed its state. New state : ", state_descr.at( socketState ).c_str() );

}

void VSNetifUdp::hasInputData() {
    TData data;
    auto data_size = _socket->pendingDatagramSize();
    QHostAddress sender_address;
    quint16 port = 0;

    data.resize( data_size );
    auto read_size = _socket->readDatagram( reinterpret_cast<char*>( data.data() ), data.capacity(), &sender_address, &port );

    if( !_netifRxCallback )
        return;

    const uint8_t *packet_data = nullptr;
    uint16_t packet_data_sz = 0;

    if( _netifRxCallback( &_instance->_netif, data.data(), data.size(), &packet_data, &packet_data_sz ) != VirgilIoTKit::VS_CODE_OK )
        return;

    if( !_netifRxProcessCallback )
        return;

    if( _netifRxProcessCallback( &_instance->_netif, packet_data, packet_data_sz ) != VirgilIoTKit::VS_CODE_OK ) {
        VSLogError( "Unable to process received packet" );
    }
}

/* static */ VirgilIoTKit::vs_status_e VSNetifUdp::netifDeinit() {
    assert( _instance && "Constructor must be called before this call" );

    _instance->_socket.reset();

    return VirgilIoTKit::VS_CODE_OK;
}

/* static */ VirgilIoTKit::vs_status_e VSNetifUdp::netifTx( const uint8_t* data_buf, const uint16_t data_sz ) {
    assert( _instance && "Constructor must be called before this call" );

    TData data( data_buf, data_buf + data_sz );
    auto sent_bytes = _instance->_socket->writeDatagram( QByteArray( reinterpret_cast<const char *>(data.data()), data_sz ), QHostAddress::Broadcast, _instance->_port );

    if( !_instance->_socket ){
        VSLogCritical( "Snap UDP implementation is not initialized" );
        return VS_CODE_ERR_CTX_NOT_READY;
    }

    if( sent_bytes != data_sz ) {
        VSLogError( "Sent bytes : ", sent_bytes, ", data bytes to send : ", data_sz, ". Last error : ", _instance->_socket->errorString().toStdString().c_str() );
        return VS_CODE_ERR_REQUEST_SEND;
    }

    return VS_CODE_OK;

}

/* static */ VirgilIoTKit::vs_status_e VSNetifUdp::netifMacAddr( VirgilIoTKit::vs_mac_addr_t* mac_addr ) {
    assert( _instance && "Constructor must be called before this call" );
    assert( mac_addr && "mac_addr pointer must not be nullptr" );

    // TODO : to fix !!!
    mac_addr->bytes[0] = 1;
    mac_addr->bytes[1] = 2;
    mac_addr->bytes[2] = 2;
    mac_addr->bytes[3] = 3;
    mac_addr->bytes[4] = 4;
    mac_addr->bytes[5] = 5;

    return VS_CODE_OK;
}
