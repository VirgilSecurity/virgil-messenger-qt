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

#include <virgil/iot-qt/snap_udp.h>

using namespace VirgilIoTKit;

VSSnapUdp::~VSSnapUdp(){

}

vs_status_e VSSnapUdp::initInterface() {
    assert( _port != 0 );

    VS_LOG_DEBUG( "Set UDP broadcast mode for LocalHost:", _port );
    _socket.reset( new QUdpSocket );

    if( !connect( _socket.get(), &QAbstractSocket::stateChanged, [this](QAbstractSocket::SocketState socketState )
    {
        static const std::map<QAbstractSocket::SocketState, std::string> state_descr{
                { QAbstractSocket::SocketState::UnconnectedState, "Unconnected" },
                { QAbstractSocket::SocketState::HostLookupState, "Host lookup" },
                { QAbstractSocket::SocketState::ConnectingState, "Connecting" },
                { QAbstractSocket::SocketState::ConnectedState, "Connected" },
                { QAbstractSocket::SocketState::BoundState, "Bound" },
                { QAbstractSocket::SocketState::ListeningState, "Listening" },
                { QAbstractSocket::SocketState::ClosingState, "Closing" } };

        сhangeStateNotify( std::string( "LocalHost:" ) + std::to_string(_port) + " : " + state_descr.at(socketState) );

        VS_LOG_CRITICAL( "Socket LocalHost:", _port, " has been changed its state. New state : ", state_descr.at(socketState).c_str() );
    } )) {
        VS_LOG_ERROR( "Unable to connect VSSnapUdp::initInterface to the socket's QAbstractSocket::stateChanged signal" );
        return VS_CODE_ERR_SOCKET;
    }

    if( !_socket->bind( _port, QAbstractSocket::ReuseAddressHint )) {
        VS_LOG_ERROR( "Unable to bind LocalHost:", _port, ". Last error : ", _socket->errorString().toStdString().c_str() );
        return VS_CODE_ERR_SOCKET;
    }

    if( !connect( _socket.get(), &QUdpSocket::readyRead, this, &VSSnapUdp::hasInputData )) {
        VS_LOG_ERROR( "Unable to connect QUdpSocket::readyRead to VSSnapUdp::hasInputData " );
        return VS_CODE_ERR_SOCKET;
    }

    return VS_CODE_OK;
}

void VSSnapUdp::hasInputData() {
    TData data;
    auto data_size = _socket->pendingDatagramSize();
    QHostAddress sender_address;
    quint16 port = 0;

    data.resize( data_size );
    auto read_size = _socket->readDatagram( reinterpret_cast<char*>( data.data() ), data.capacity(), &sender_address, &port );

    processRxData( std::move( data ));
}

const VSMac& VSSnapUdp::ownMacAddress() const {
    static const VSMac mac( 10, 20, 30, 40, 50, 60 );

    if( !_socket ){
        VS_LOG_CRITICAL( "Snap UDP implementation is not initialized" );
        return mac;
    }

    return mac;
}

vs_status_e VSSnapUdp::destroy() {
    _socket.reset();

    return VS_CODE_OK;
}

vs_status_e VSSnapUdp::sendRawData( TData &&data ) {
    auto data_size = data.size();
    auto sent_bytes = _socket->writeDatagram( QByteArray( reinterpret_cast<const char *>(data.data()), data_size ), QHostAddress::Broadcast, _port );
    //auto sent_bytes = _socket->write( reinterpret_cast<const char *>( data.data() ), data_size );

    if( !_socket ){
        VS_LOG_CRITICAL( "Snap UDP implementation is not initialized" );
        return VS_CODE_ERR_CTX_NOT_READY;
    }

    if( sent_bytes != data_size ) {
        VS_LOG_ERROR( "Sent bytes : ", sent_bytes, ", data bytes to send : ", data_size, ". Last error : ", _socket->errorString().toStdString().c_str() );
        return VS_CODE_ERR_REQUEST_SEND;
    }

    return VS_CODE_OK;
}

VSSnapUdp::TMacResponse VSSnapUdp::ownMac() const {
    TMacResponse response;
    auto &[ ret_code, mac ] = response;

    ret_code = VS_CODE_OK;
    mac = ownMacAddress();

    return response;
}
