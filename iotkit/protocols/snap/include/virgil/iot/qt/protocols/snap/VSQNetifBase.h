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

#ifndef VIRGIL_IOTKIT_QT_VSQNETIFBASE_H
#define VIRGIL_IOTKIT_QT_VSQNETIFBASE_H

#include <QObject>
#include <QAbstractSocket>

#include <virgil/iot/protocols/snap.h>

class VSQNetifBase: public QObject {
    Q_OBJECT
public:
    VSQNetifBase();
    VSQNetifBase(VSQNetifBase const &) = delete;
    VSQNetifBase &operator=(VSQNetifBase const &) = delete;

    virtual ~VSQNetifBase() = default;

signals:
    // Do not use these signals directly from your implementation of network interface.
    void fireReceivedBytes(const QByteArray &data);
    void fireReceivedPacket(const QByteArray &packet);
    void fireStateChanged(QAbstractSocket::SocketState connectionState);

protected:
    virtual bool init() = 0;
    virtual bool deinit() = 0;
    virtual bool tx(const QByteArray &data) = 0;
    virtual QString macAddr() = 0;

    // This method must be called by implementation of network interface.
    // It uses low level callbacks and sends data using signals
    bool processData(const QByteArray &data);

private:
    friend struct VSQLowLevelNetif;

    // TODO: Try to give access to vs_netif_t only
    friend class VSQSnap;

    VirgilIoTKit::vs_netif_t m_lowLevelNetif;
    VirgilIoTKit::vs_netif_rx_cb_t m_lowLevelRxCall;
    VirgilIoTKit::vs_netif_process_cb_t m_lowLevelPacketProcess;


};

#endif //VIRGIL_IOTKIT_QT_VSQNETIFBASE_H
