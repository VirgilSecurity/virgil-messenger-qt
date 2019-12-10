//  Copyright (C) 2015-2020 Virgil Security, Inc.
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

#ifndef VSQIOTKITFACADE_H
#define VSQIOTKITFACADE_H

#include <QObject>
#include <VSQSingleton.h>
#include <virgil/iot/logger/logger.h>
#include <virgil/iot/provision/provision-structs.h>

class VSQNetifBase;
class VSQSnapServiceBase;

class VSQIoTKitFacade : public QObject, public VSQSingleton<VSQIoTKitFacade> {
    Q_OBJECT

public:
    bool
    init(VirgilIoTKit::vs_device_manufacture_id_t manufacturer_id,
         VirgilIoTKit::vs_device_type_t device_type,
         VirgilIoTKit::vs_device_serial_t device_serial,
         VirgilIoTKit::vs_log_level_t log_level,
         uint32_t device_roles);

    VSQIoTKitFacade &
    operator<<(VSQNetifBase &netif);

    VSQIoTKitFacade &
    operator<<(VSQSnapServiceBase &snap_service);

private:
    bool m_initialized = false;
    VirgilIoTKit::vs_device_manufacture_id_t m_manufacturer_id;
    VirgilIoTKit::vs_device_type_t m_device_type;
    VirgilIoTKit::vs_device_serial_t m_device_serial;
    uint32_t m_device_roles;
};

#endif // VSQIOTKITFACADE_H
