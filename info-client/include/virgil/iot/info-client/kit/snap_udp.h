//
// Created by Oleksandr Nemchenko on 23.11.2019.
//

#ifndef VIRGIL_IOT_INFO_CLIENT_KIT_SNAP_UDP_H_
#define VIRGIL_IOT_INFO_CLIENT_KIT_SNAP_UDP_H_

#include <app.h>

#include <kit/snap_protocol.h>

namespace VirgilIoTKit {

    class VSSnapUdp : public QObject, public VSSnapProtocol {
        Q_OBJECT

    public:
        VSSnapUdp() : _port( 0 ) {}
        ~VSSnapUdp() override;

        void setBindPort( quint16 bind_port )   { _port = bind_port; }
        const VSMac& ownMacAddress() const override;

    private:
        vs_status_e initInterface() override;
        vs_status_e destroy() override;
        vs_status_e sendRawData( TData &&data ) override;
        TMacResponse ownMac() const override;

        std::unique_ptr<QUdpSocket> _socket;
        quint16 _port;

        void hasInputData();
    };

} // namespace VirgilIoTKit

#endif // VIRGIL_IOT_INFO_CLIENT_KIT_SNAP_UDP_H_
