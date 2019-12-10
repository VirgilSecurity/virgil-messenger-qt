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

#include <VSQUdpBroadcast.h>
#include <VSQLogger.h>

VSQUdpBroadcast::VSQUdpBroadcast(quint16 port) : m_port(port) {
}

bool
VSQUdpBroadcast::init() {
    if (!connect(&m_socket, &QUdpSocket::stateChanged, this, &VSQUdpBroadcast::onConnectionStateChanged)) {
        VSLogError(
                "Unable to connect VSQUdpBroadcast::stateChanged signal to the socket's "
                "VSQUdpBroadcast::onConnectionStateChanged slot");
        return false;
    }

    if (!m_socket.bind(m_port, QUdpSocket::ReuseAddressHint)) {
        VSLogError(
                "Unable to bind LocalHost:", m_port, ". Last error : ", m_socket.errorString().toStdString().c_str());
        return false;
    }

    if (!connect(&m_socket, &QUdpSocket::readyRead, this, &VSQUdpBroadcast::onHasInputData)) {
        VSLogError("Unable to connect QUdpSocket::readyRead signal to the VSQUdpBroadcast::onHasInputData slot");
        return false;
    }

    return true;
}

void
VSQUdpBroadcast::onConnectionStateChanged(QAbstractSocket::SocketState socket_state) {
    emit fireConnectionStateChanged(socket_state);
}

bool
VSQUdpBroadcast::deinit() {
    m_socket.disconnectFromHost();
    return true;
}

bool
VSQUdpBroadcast::tx(const QByteArray &data) {
    auto data_sz = data.size();
    Q_ASSERT(m_socket.state() == QAbstractSocket::ConnectedState && "Socket must be connected before this call");

    auto sent_bytes = m_socket.writeDatagram(data, QHostAddress::Broadcast, m_port);

    if (sent_bytes != data_sz) {
        VSLogError("Sent bytes : ",
                   sent_bytes,
                   ", data bytes to send : ",
                   data.size(),
                   ". Last error : ",
                   m_socket.errorString().toStdString().c_str());
        return false;
    }

    return true;
}

QString
VSQUdpBroadcast::macAddr() {
    return m_socket.multicastInterface().hardwareAddress();
}

void
VSQUdpBroadcast::onHasInputData() {
    processData(m_socket.receiveDatagram().data());
}
