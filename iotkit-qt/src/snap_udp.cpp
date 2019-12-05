//
// Created by Oleksandr Nemchenko on 23.11.2019.
//

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
