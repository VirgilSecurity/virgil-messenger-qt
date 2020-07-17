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
#include <QFuture>
#include <QObject>
#include <QSemaphore>

#include <qxmpp/QXmppClient.h>

#include <virgil/iot/messenger/messenger.h>

#include "VSQCommon.h"

using namespace VirgilIoTKit;

class QXmppMessageReceiptManager;

class VSQAttachmentsModel;
class VSQContactsModel;
class VSQSettings;
class VSQChatsModel;
class VSQConversationsModel;

class VSQMessenger : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString currentUser READ currentUser NOTIFY fireCurrentUserChanged)
    Q_PROPERTY(QString currentRecipient WRITE setCurrentRecipient READ currentRecipient NOTIFY fireCurrentRecipientChanged)

    enum VSQEnvType { PROD, STG, DEV };

public:
    enum EnResult
    {
        MRES_OK,
        MRES_ERR_NO_CRED,
        MRES_ERR_SIGNIN,
        MRES_ERR_SIGNUP,
        MRES_ERR_USER_NOT_FOUND,
        MRES_ERR_USER_ALREADY_EXISTS,
        MRES_ERR_ENCRYPTION
    };
    Q_ENUMS(EnResult)

    enum EnStatus
    {
        MSTATUS_ONLINE,
        MSTATUS_UNAVAILABLE
    };
    Q_ENUMS(EnStatus)

    explicit VSQMessenger(VSQSettings *settings, QObject *parent = nullptr);
    VSQMessenger() = default; // NOTE(fpohtmeh): needed for QmlPrivate template
    ~VSQMessenger() override = default;

    QString currentUser() const;
    QString currentRecipient() const;

    VSQConversationsModel &modelConversations();
    VSQChatsModel &getChatModel();
    
    static QString decryptMessage(const QString &sender, const QString &message);

    Q_INVOKABLE void markAsRead(const QString &chatId);

public slots:
    Q_INVOKABLE QFuture<VSQMessenger::EnResult>
    signIn(QString user);

    Q_INVOKABLE QFuture<VSQMessenger::EnResult>
    backupUserKey(QString password);

    Q_INVOKABLE QFuture<VSQMessenger::EnResult>
    signInWithBackupKey(QString username, QString password);

    Q_INVOKABLE QFuture<VSQMessenger::EnResult>
    signUp(QString user);

    Q_INVOKABLE QFuture<VSQMessenger::EnResult>
    logout();

    Q_INVOKABLE QFuture<VSQMessenger::EnResult>
    disconnect();

    Q_INVOKABLE QFuture<VSQMessenger::EnResult>
    deleteUser(QString user);

    Q_INVOKABLE QStringList
    usersList();

    Q_INVOKABLE void
    checkState();

    Q_INVOKABLE QFuture<VSQMessenger::EnResult>
    addContact(QString contact);

    Q_INVOKABLE QFuture<VSQMessenger::EnResult> sendMessage(const QString &messageText, const QVariant &attachmentUrl, Enums::AttachmentType attachmentType);
    QFuture<VSQMessenger::EnResult> sendMessage(bool createNew, const StMessage &message);

    Q_INVOKABLE void
    setStatus(VSQMessenger::EnStatus status);

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

    void fireCurrentUserChanged();
    void fireCurrentRecipientChanged();

private slots:
    void onConnected();
    void onMessageDelivered(const QString&, const QString&);
    void onDisconnected();
    void onError(QXmppClient::Error);
    void onMessageReceived(const QXmppMessage &message);
    void onPresenceReceived(const QXmppPresence &presence);
    void onIqReceived(const QXmppIq &iq);
    void onSslErrors(const QList<QSslError> &errors);
    void onStateChanged(QXmppClient::State state);

    void
    onAddContactToDB(QString contact);

    Q_INVOKABLE void
    onSubscribePushNotifications(bool enable);

private:
    void setCurrentRecipient(const QString &recipient);

    VSQSettings *m_settings;
    QXmppClient m_xmpp;
    QXmppMessageReceiptManager* m_xmppReceiptManager;

    VSQContactsModel *m_contacts;
    VSQChatsModel *m_sqlChatModel;
    VSQAttachmentsModel *m_attachments;
    VSQConversationsModel *m_sqlConversations;

    QString m_user;
    QString m_userId;
    QString m_recipient;
    QString m_xmppPass;
    VSQEnvType m_envType;
    static const VSQEnvType _defaultEnv = PROD;
    QXmppConfiguration conf;

    static const QString kOrganization;
    static const QString kApp;
    static const QString kUsers;
    static const QString kProdEnvPrefix;
    static const QString kStgEnvPrefix;
    static const QString kDevEnvPrefix;
    static const QString kPushNotificationsProxy;
    static const QString kPushNotificationsNode;
    static const QString kPushNotificationsService;
    static const QString kPushNotificationsFCM;
    static const QString kPushNotificationsDeviceID;
    static const QString kPushNotificationsFormType;
    static const QString kPushNotificationsFormTypeVal;
    static const int kConnectionWaitMs;
    static const int kKeepAliveTimeSec;

    void
    _connectToDatabase();

    bool
    _connect(QString userWithEnv, QString userId);

    QString
    _xmppPass();

    QString
    _virgilURL();

    QString
    _xmppURL();

    uint16_t
    _xmppPort();

    void
    _reconnect();

    bool
    _saveCredentials(const QString &user, const vs_messenger_virgil_user_creds_t &creds);

    bool
    _loadCredentials(const QString &user, vs_messenger_virgil_user_creds_t &creds);

    void
    _addToUsersList(const QString &user);

    QString
    _prepareLogin(const QString &user);

    QString
    _caBundleFile();

    void _sendFailedMessages();
};

#endif // VIRGIL_IOTKIT_QT_MESSENGER_H
