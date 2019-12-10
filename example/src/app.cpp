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

#include <app.h>
#include <controller.h>
#include <VSQFeatures.h>
#include <VSQImplementations.h>
#include <VSQAppConfig.h>
#include <VSQManufactureId.h>
#include <VSQUdpBroadcast.h>
#include <VSQDeviceType.h>
#include <VSQDeviceSerial.h>
#include <VSQDeviceRoles.h>
#include <VSQIoTKitFacade.h>

GuiApplication::GuiApplication(int argc, char *argv[]) : app(argc, argv), m_udpBroadcast(4100) {

    auto features = VSQFeatures() << VSQFeatures::SNAP_INFO_CLIENT;
    auto impl = VSQImplementations() << &m_udpBroadcast;
    auto roles = VSQDeviceRoles() << VirgilIoTKit::VS_SNAP_DEV_CONTROL;
    auto app_config = VSQAppConfig() << VSQManufactureId() << VSQDeviceType() << VSQDeviceSerial()
                                     << VirgilIoTKit::VS_LOGLEV_DEBUG << roles;

    if (!VSQIoTKitFacade::init(features, impl, roles, app_config))
        throw std::runtime_error("Unable to initialize Virgil IoT KIT");
}

int
GuiApplication::run() {
    Controller controller;

    controller.setupUI();

    return app.exec();
}
