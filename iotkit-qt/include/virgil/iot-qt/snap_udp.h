//
// Created by Oleksandr Nemchenko on 23.11.2019.
//

#ifndef _VIRGIL_IOTKIT_QT_SNAP_UDP_H_
#define _VIRGIL_IOTKIT_QT_SNAP_UDP_H_

#include <QtCore>
#include <QtNetwork>

#include <virgil/iot-qt/snap_protocol.h>

class VSSnapUdp : public QObject, public VSSnapProtocol {
    Q_OBJECT

public:
    VSSnapUdp() : _port( 0 ) {}
    ~VSSnapUdp() override;

    void setBindPort( quint16 bind_port )   { _port = bind_port; }
    const VSMac& ownMacAddress() const override;

private:
    VirgilIoTKit::vs_status_e initInterface() override;
    VirgilIoTKit::vs_status_e destroy() override;
    VirgilIoTKit::vs_status_e sendRawData( TData &&data ) override;
    TMacResponse ownMac() const override;

    std::unique_ptr<QUdpSocket> _socket;
    quint16 _port;

    void hasInputData();
};

#endif // _VIRGIL_IOTKIT_QT_SNAP_UDP_H_
