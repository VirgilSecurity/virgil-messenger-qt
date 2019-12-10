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

#include <type_traits>
#include <VSQIoTKitFacade.h>
#include <virgil/iot/logger/logger.h>

#include <VSQNetifBase.h>

bool
VSQIoTKitFacade::init(VirgilIoTKit::vs_device_manufacture_id_t manufacturer_id,
                      VirgilIoTKit::vs_device_type_t device_type,
                      VirgilIoTKit::vs_device_serial_t device_serial,
                      VirgilIoTKit::vs_log_level_t log_level,
                      uint32_t device_roles) {
    VirgilIoTKit::vs_logger_init(log_level);

    std::copy(m_manufacturer_id, manufacturer_id, sizeof(manufacturer_id));
    std::copy(m_device_type, device_type, sizeof(device_type));
    std::copy(m_device_serial, device_serial, sizeof(device_serial));
    m_device_roles = device_roles;

    m_initialized = true;
}

VSQIoTKitFacade &
VSQIoTKitFacade::operator<<(VSQNetifBase &netif) {
    Q_ASSERT(m_initialized);

    if (VirgilIoTKit::vs_snap_init(netif.netif(), m_manufacturer_id, m_device_type, m_device_serial, m_device_roles) !=
        VirgilIoTKit::VS_CODE_OK) {
        VsLogError("Unable to initialize SNAP module");
    }

    return *this;
}

VSQIoTKitFacade &
VSQIoTKitFacade::operator<<(VSQSnapServiceBase &snap_service) {
    Q_ASSERT(m_initialized);

    if (VirgilIoTKit::vs_snap_register_service(snap_service.serviceInterface()) != VirgilIoTKit::VS_CODE_OK) {
        VsLogError("Unable to initialize service ", snap_service.serviceName());
    }

    return *this;
}
