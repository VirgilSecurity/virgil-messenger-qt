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


#ifndef VIRGIL_IOTKIT_QT_MESSENGER_H
#define VIRGIL_IOTKIT_QT_MESSENGER_H

#include <QtCore>

#include <virgil/iot/qt/VSQIoTKit.h>
#include <qxmpp/QXmppClient.h>

using namespace VirgilIoTKit;
#include <virgil/iot/messenger/messenger.h>

#include "VSQSqlContactModel.h"
#include "VSQSqlConversationModel.h"

class VSQMessenger final : public QObject {

    Q_OBJECT

public:
    VSQMessenger();
    virtual ~VSQMessenger() = default;

    Q_INVOKABLE void
    signIn(QString user);

    Q_INVOKABLE void
    signUp(QString user);

    Q_INVOKABLE void
    logout();

    Q_INVOKABLE void
    deleteUser(QString user);

    Q_INVOKABLE QStringList
    usersList();

    Q_INVOKABLE void
    addContact(QString contact);

    VSQSqlContactModel &
    modelContacts();

    VSQSqlConversationModel &
    modelConversations();

signals:
    void
    fireError(QString errorText);

    void
    fireInform(QString informText);

    void
    fireConnecting();

    void
    fireReady();

    void
    fireAddedContact(QString contact);

    void
    fireReadyToAddContact(QString contact);

    void
    fireNewMessage(QString from, QString message);


private slots:
    void onConnected();
    void onDisconnected();
    void onError(QXmppClient::Error);
    void onMessageReceived(const QXmppMessage &message);
    void onPresenceReceived(const QXmppPresence &presence);
    void onIqReceived(const QXmppIq &iq);
    void onSslErrors(const QList<QSslError> &errors);
    void onStateChanged(QXmppClient::State state);

    void
    onAddContactToDB(QString contact);

private:
    QXmppClient m_xmpp;
    VSQSqlContactModel *m_sqlContacts;
    VSQSqlConversationModel *m_sqlConversations;

    static const QString kOrganization;
    static const QString kApp;
    static const QString kUsers;

    void
    _connectToDatabase();

    void
    _connect(QString user);

    QString
    _virgilURL();

    QString
    _xmppURL();

    uint16_t
    _xmppPort();

    bool
    _saveCredentials(const QString &user, const vs_messenger_virgil_user_creds_t &creds);

    bool
    _loadCredentials(const QString &user, vs_messenger_virgil_user_creds_t &creds);

    void
    _addToUsersList(const QString &user);

    void
    _saveUsersList(const QStringList &users);
};

#endif // VIRGIL_IOTKIT_QT_MESSENGER_H
