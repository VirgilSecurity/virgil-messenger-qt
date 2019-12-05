//
// Created by Oleksandr Nemchenko on 23.11.2019.
//

#ifndef _VIRGIL_IOTKIT_QT_SNAP_UDP_H_
#define _VIRGIL_IOTKIT_QT_SNAP_UDP_H_

#include <QtCore>
#include <QtNetwork>

#include <virgil/iot-qt/netif.h>

class VSNetifUdp : public QObject, public VSNetif {
    Q_OBJECT

public:
    VSNetifUdp( quint16 bind_port = 0 );
    ~VSNetifUdp() override;

    void bindPort( quint16 bind_port )  { _port = bind_port; }
    VirgilIoTKit::vs_netif_t* getImplementation() override  { return &_netif; }

signals:
    void stateChanged( QAbstractSocket::SocketState connectionState, const std::string &description );

private:
    static VSNetifUdp* _instance;

    std::unique_ptr<QUdpSocket> _socket;
    quint16 _port;
    VirgilIoTKit::vs_netif_t _netif;

    static VirgilIoTKit::vs_status_e netifDeinit();
    static VirgilIoTKit::vs_status_e netifInit( const VirgilIoTKit::vs_netif_rx_cb_t rx_cb, const VirgilIoTKit::vs_netif_process_cb_t process_cb );
    static VirgilIoTKit::vs_status_e netifTx( const uint8_t* data, const uint16_t data_sz );
    static VirgilIoTKit::vs_status_e netifMacAddr( VirgilIoTKit::vs_mac_addr_t* mac_addr );

    VirgilIoTKit::vs_netif_rx_cb_t _netifRxCallback;
    VirgilIoTKit::vs_netif_process_cb_t _netifRxProcessCallback;

private slots:
    void connectionStateChanged( QAbstractSocket::SocketState );
    void hasInputData();
};

#endif // _VIRGIL_IOTKIT_QT_SNAP_UDP_H_
