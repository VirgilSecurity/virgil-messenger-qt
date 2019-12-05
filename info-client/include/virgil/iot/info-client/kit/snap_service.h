//
// Created by Oleksandr Nemchenko on 23.11.2019.
//

#ifndef VIRGIL_IOT_INFO_CLIENT_KIT_SNAP_SERVICE_H_
#define VIRGIL_IOT_INFO_CLIENT_KIT_SNAP_SERVICE_H_

#include <app.h>

#include <kit/snap_protocol.h>

#include <virgil/iot/status_code/status_code.h>
#include <virgil/iot/protocols/snap/snap-structs.h>

namespace VirgilIoTKit {

    class VSSnapProtocol;

    class VSSnapService {
    public:
        virtual ~VSSnapService();

        virtual const vs_snap_service_t *serviceInterface() = 0;
        virtual const std::string &serviceName() const = 0;

        void setSnapProtocol( const VSSnapProtocol *protocol )   { _protocol = protocol; }

        const VSSnapProtocol &snapProtocol() const               { return *_protocol; }

    private:
        const VSSnapProtocol * _protocol = nullptr;
    };

} // namespace VirgilIoTKit

#endif // VIRGIL_IOT_INFO_CLIENT_KIT_SNAP_SERVICE_H_
