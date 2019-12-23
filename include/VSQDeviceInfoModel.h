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

#ifndef VIRGIL_IOTKIT_QT_DEMO_VSQ_DEVICE_INFO_MODEL_H
#define VIRGIL_IOTKIT_QT_DEMO_VSQ_DEVICE_INFO_MODEL_H

#include <virgil/iot/qt/protocols/snap/VSQSnapINFOClient.h>
#include <QObject>

class VSQDeviceInfoModel : public QObject {
    Q_OBJECT

    Q_PROPERTY(QString macDeviceRoles READ macDeviceRoles NOTIFY fireMacDeviceRoles)
    Q_PROPERTY(QString deviceState READ deviceState NOTIFY fireDeviceState)
    Q_PROPERTY(QString manufactureIdDeviceType READ manufactureIdDeviceType NOTIFY fireManufactureIdDeviceType)
    Q_PROPERTY(QString fwTlVer READ fwTlVer NOTIFY fireFwTlVer)
    Q_PROPERTY(QString statistics READ statistics NOTIFY fireStatistics)
    Q_PROPERTY(QString lastTimestamp READ lastTimestamp NOTIFY fireLastTimestamp)

public:
    VSQDeviceInfoModel() {
    }
    VSQDeviceInfoModel(const VSQDeviceInfo &deviceInfo) {
        m_deviceInfo = deviceInfo;
    }

    const VSQDeviceInfo &
    deviceInfo() const {
        return m_deviceInfo;
    }
    QString
    macDeviceRoles() const {
        return QString("%1\n%2\n")
                .arg(QString(m_deviceInfo.m_mac.description()))
                .arg(QString(m_deviceInfo.m_deviceRoles));
    }
    QString
    deviceState() const;
    QString
    manufactureIdDeviceType() const {
        return m_deviceInfo.m_hasGeneralInfo
                       ? QString("%1\n%2").arg(m_deviceInfo.m_manufactureId).arg(m_deviceInfo.m_deviceType)
                       : "---";
    }
    QString
    fwTlVer() const {
        return m_deviceInfo.m_hasGeneralInfo ? QString("%1\n%2").arg(m_deviceInfo.m_fwVer).arg(m_deviceInfo.m_tlVer)
                                             : "---";
    }
    QString
    statistics() const {
        return m_deviceInfo.m_hasStatistics ? QString("%1\n%2").arg(m_deviceInfo.m_sent).arg(m_deviceInfo.m_received)
                                            : "---";
    }
    QString
    lastTimestamp() const {
        return m_deviceInfo.m_lastTimestamp.toString("hh:mm:ss");
    }

    void
    change(const VSQDeviceInfo &deviceInfo);

signals:

    void
    fireMacDeviceRoles();
    void
    fireDeviceState();
    void
    fireManufactureIdDeviceType();
    void
    fireFwTlVer();
    void
    fireStatistics();
    void
    fireLastTimestamp();

private:
    VSQDeviceInfo m_deviceInfo;
};

#endif // VIRGIL_IOTKIT_QT_DEMO_VSQ_DEVICE_INFO_MODEL_H
