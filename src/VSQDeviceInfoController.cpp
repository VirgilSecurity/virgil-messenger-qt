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

#include <QWindow>
#include <VSQDeviceInfoController.h>

VSQDeviceInfoController::VSQDeviceInfoController(QObject *parent)
    : QObject(parent), m_deviceInfoList(), m_currentDeviceInfoModel(nullptr) {
}

VSQDeviceInfoController::~VSQDeviceInfoController() {
    for (auto model : m_deviceInfoList) {
        delete model;
    }

    delete m_currentDeviceInfoModel;
}

QQmlListProperty<VSQDeviceInfoModel>
VSQDeviceInfoController::getDeviceInfoList() {
    return QQmlListProperty<VSQDeviceInfoModel>(this, m_deviceInfoList);
}

void
VSQDeviceInfoController::change(const VSQDeviceInfo &deviceInfo) {

    // Change existing device info
    for (auto device : m_deviceInfoList) {
        if (device->deviceInfo().m_mac == deviceInfo.m_mac) {
            device->change(deviceInfo);
            return;
        }
    }

    // Add new device
    VSQDeviceInfoModel *deviceInfoModel = new VSQDeviceInfoModel(deviceInfo);
    m_deviceInfoList.append(deviceInfoModel);
    emit deviceInfoListChanged();
}

int
VSQDeviceInfoController::visibilityMode() const {
#if defined(Q_OS_ANDROID) || defined(Q_OS_IOS) || defined(Q_OS_TVOS) || defined(Q_OS_WATCHOS)
    return QWindow::FullScreen;
#else
    return QWindow::Windowed;
#endif
}
