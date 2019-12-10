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

#include <VSQNetifBase.h>
#include <VSQSnapServiceBase.h>
#include <VSQIoTKitFacade.h>
#include <VSQLogger.h>

bool
VSQIoTKitFacade::init(const VSQFeatures &features, const VSQImplementations &impl, const VSQAppConfig &appConfig) {
    using namespace VirgilIoTKit;

    instance().m_features = features;
    instance().m_impl = impl;
    instance().m_appConfig = appConfig;

    vs_logger_init(appConfig.logLevel());

    try {
        Q_CHECK_PTR(instance().m_impl.netif());

        // SNAP entities
        if (features.hasSnap()) {
            if (vs_snap_init(instance().m_impl.netif(),
                             appConfig.manufactureId(),
                             appConfig.deviceType(),
                             appConfig.deviceSerial(),
                             appConfig.deviceRoles()) != VirgilIoTKit::VS_CODE_OK) {
                throw QString("Unable to initialize SNAP");
            }

            instance().registerService(VSQFeatures::SNAP_INFO_CLIENT, {VS_SNAP_DEV_CONTROL, VS_SNAP_DEV_DEBUGGER});
        }
        return true;

    } catch (QString &descr) {
        VSLogCritical("Error during Virgil IoT KIT initialization : ", descr.toStdString());
        return false;
    }
}

void
VSQIoTKitFacade::registerService(VSQFeatures::EFeature feature, VSQDeviceRoles::TRolesList &&roles) {

    if (!m_features.hasFeature(feature) || !m_appConfig.deviceRoles().hasRoles(roles)) {
        return;
    }

    for (const auto &service : m_impl.services()) {
        if (service->serviceFeature() == feature) {
            if (vs_snap_register_service(service->serviceInterface()) != VirgilIoTKit::VS_CODE_OK) {
                throw QString("Unable to register SNAP's %1 service").arg(service->serviceName());
            }
            return;
        }
    }

    throw QString("No appropriate service has been found in the services list");
}
