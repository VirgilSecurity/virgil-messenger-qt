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

#include <virgil/iot/qt/VSQIoTKit.h>

#include <VSQMessenger.h>
#include <QtConcurrent>


const QString VSQMessenger::kOrganization = "VirgilSecurity";
const QString VSQMessenger::kApp = tr("IoTKit Messenger");


/******************************************************************************/
VSQMessenger::VSQMessenger() {
    if (VS_CODE_OK != vs_messenger_virgil_init(_virgilURL().toStdString().c_str())) {
        qCritical() << "Cannot initialize low level messenger";
    }

    // Connect XMPP signals
    connect(&m_xmpp, SIGNAL(connected()), this, SLOT(onConnected()));
    connect(&m_xmpp, SIGNAL(disconnected()), this, SLOT(onDisconnected()));
    connect(&m_xmpp, SIGNAL(error(QXmppClient::Error)), this, SLOT(onError(QXmppClient::Error)));
    connect(&m_xmpp, SIGNAL(messageReceived(const QXmppMessage &)), this, SLOT(onMessageReceived(const QXmppMessage &)));
    connect(&m_xmpp, SIGNAL(presenceReceived(const QXmppPresence &)), this, SLOT(onPresenceReceived(const QXmppPresence &)));
    connect(&m_xmpp, SIGNAL(iqReceived(const QXmppIq &)), this, SLOT(onIqReceived(const QXmppIq &)));
    connect(&m_xmpp, SIGNAL(sslErrors(const QList<QSslError> &)), this, SLOT(onSslErrors(const QList<QSslError> &)));
    connect(&m_xmpp, SIGNAL(stateChanged(QXmppClient::State)), this, SLOT(onStateChanged(QXmppClient::State)));
}

/******************************************************************************/
void
VSQMessenger::_connect(QString user) {
    const size_t _pass_buf_sz = 512;
    char pass[_pass_buf_sz];
    QString jid = user + "@" + _xmppURL();

    // Get XMPP password
    if (VS_CODE_OK != vs_messenger_virgil_get_xmpp_pass(pass, _pass_buf_sz)) {
        emit fireError(tr("Cannot get XMPP password"));
        return;
    }

    // Connect to XMPP
    emit fireConnecting();
    m_xmpp.connectToServer(jid, QString::fromLatin1(pass));
}

/******************************************************************************/
void
VSQMessenger::signIn(QString user) {
    QtConcurrent::run([=]() {
        qDebug() << "Trying to Sign In: " << user;

        vs_messenger_virgil_user_creds_t creds;
        memset(&creds, 0, sizeof (creds));

        // Load User Credentials
        if (!_loadCredentials(user, creds)) {
            emit fireError(tr("Cannot load user credentials"));
            return;
        }

        // Sign In user, using Virgil Service
        if (VS_CODE_OK != vs_messenger_virgil_sign_in(&creds)) {
            emit fireError(tr("Cannot Sign In user"));
            return;
        }

        // Connect over XMPP
        _connect(user);
    });
}

/******************************************************************************/
void
VSQMessenger::signUp(QString user) {
    QtConcurrent::run([=]() {
        qDebug() << "Trying to Sign Up: " << user;

        vs_messenger_virgil_user_creds_t creds;
        memset(&creds, 0, sizeof (creds));

        // Sign Up user, using Virgil Service
        if (VS_CODE_OK != vs_messenger_virgil_sign_up(user.toStdString().c_str(), &creds)) {
            emit fireError(tr("Cannot Sign Up user"));
            return;
        }

        // Save credentials
        _saveCredentials(user, creds);

        // Connect over XMPP
        _connect(user);
    });
}

/******************************************************************************/
QString
VSQMessenger::_virgilURL() {
    return "https://messenger-stg.virgilsecurity.com";
}

/******************************************************************************/
QString
VSQMessenger::_xmppURL() {
    return "xmpp-stg.virgilsecurity.com";
}

/******************************************************************************/
uint16_t
VSQMessenger::_xmppPort() {
    return 5222;
}

/******************************************************************************/

// TODO: Use SecBox
bool
VSQMessenger::_saveCredentials(const QString &user, const vs_messenger_virgil_user_creds_t &creds) {
    QByteArray baCred(reinterpret_cast<const char*>(&creds), sizeof(creds));
    QSettings settings(kOrganization, kApp);
    settings.setValue(user, baCred.toBase64());
    return true;
}

/******************************************************************************/
bool
VSQMessenger::_loadCredentials(const QString &user, vs_messenger_virgil_user_creds_t &creds) {
    QSettings settings(kOrganization, kApp);

    auto credBase64 = settings.value(user, QString("")).toString();
    auto baCred = QByteArray::fromBase64(credBase64.toUtf8());

    if (baCred.size() != sizeof(creds)) {
        VS_LOG_WARNING("Cannot load credentials for : %s", user.toStdString().c_str());
        return false;
    }

    memcpy(&creds, baCred.data(), static_cast<size_t> (baCred.size()));
    return true;
}

/******************************************************************************/
void
VSQMessenger::logout() {
    qDebug() << "Logout";
    m_xmpp.disconnectFromServer();
}

/******************************************************************************/
void
VSQMessenger::deleteUser(QString user) {
    logout();
}

/******************************************************************************/
void VSQMessenger::onConnected() {
    emit fireReady();
}

/******************************************************************************/
void VSQMessenger::onDisconnected() {
    VS_LOG_DEBUG("onDisconnected");
#if 0
    emit fireError(tr("Disconnected ..."));
#endif
}

/******************************************************************************/
void VSQMessenger::onError(QXmppClient::Error) {
    VS_LOG_DEBUG("onError");
    emit fireError(tr("Connection error ..."));
}

/******************************************************************************/
void VSQMessenger::onMessageReceived(const QXmppMessage &message) {
    VS_LOG_DEBUG("onMessageReceived");
}

/******************************************************************************/
void VSQMessenger::onPresenceReceived(const QXmppPresence &presence) {
    VS_LOG_DEBUG("onPresenceReceived");
}

/******************************************************************************/
void VSQMessenger::onIqReceived(const QXmppIq &iq) {
    VS_LOG_DEBUG("onIqReceived");
}

/******************************************************************************/
void VSQMessenger::onSslErrors(const QList<QSslError> &errors) {
    emit fireError(tr("Secure connection error ..."));
}

/******************************************************************************/
void VSQMessenger::onStateChanged(QXmppClient::State state) {
    if (QXmppClient::ConnectingState == state) {
        emit fireConnecting();
    }
}

/******************************************************************************/
