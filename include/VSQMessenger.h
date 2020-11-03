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
#include <QXmppCarbonManager.h>

#include <qxmpp/QXmppClient.h>
#include <qxmpp/QXmppMessageReceiptManager.h>

#include <virgil/iot/messenger/messenger.h>

#include "Validator.h"
#include "VSQCrashReporter.h"
#include <VSQNetworkAnalyzer.h>

class QJsonDocument;

class VSQContactManager;
class VSQDiscoveryManager;
class VSQLastActivityManager;

Q_DECLARE_LOGGING_CATEGORY(lcMessenger)

using namespace VirgilIoTKit;
using namespace vm;

class VSQMessenger : public QObject {

    Q_OBJECT

    enum VSQEnvType { PROD, STG, DEV };

    Q_ENUMS(EnResult)

    Q_ENUMS(EnStatus)

public:
    enum EnResult
    {
        MRES_OK,
        MRES_ERR_NO_CRED,
        MRES_ERR_SIGNIN,
        MRES_ERR_SIGNUP,
        MRES_ERR_USER_NOT_FOUND,
        MRES_ERR_USER_ALREADY_EXISTS,
        MRES_ERR_ENCRYPTION,
        MRES_ERR_ATTACHMENT
    };

    enum EnStatus
    {
        MSTATUS_ONLINE,
        MSTATUS_UNAVAILABLE
    };

    Q_PROPERTY(QString currentUser READ currentUser NOTIFY fireCurrentUserChanged)

    VSQMessenger(QNetworkAccessManager *networkAccessManager, VSQSettings *settings, Validator *validator);
    VSQMessenger() = default; // QML engine requires default constructor
    virtual ~VSQMessenger();

    Q_INVOKABLE QString currentUser() const;
    Q_INVOKABLE QString currentRecipient() const;

    QXmppClient *xmpp();
    QString xmppURL() const;
    VSQLastActivityManager *lastActivityManager();

    void setApplicationActive(bool active);

    // New methods

    void signIn(const QString &userId);
    void signOut();
    void signUp(const QString &userId);
    void backupKey(const QString &password, const QString &confirmedPassword);
    void downloadKey(const QString &userId, const QString &password);

public slots:
    Q_INVOKABLE QFuture<VSQMessenger::EnResult>
    signInAsync(QString user);

    Q_INVOKABLE QFuture<VSQMessenger::EnResult>
    backupKeyAsync(QString password);

    Q_INVOKABLE QFuture<VSQMessenger::EnResult>
    signInWithBackupKeyAsync(QString username, QString password);

    Q_INVOKABLE QFuture<VSQMessenger::EnResult>
    signUpAsync(QString user);

    Q_INVOKABLE QFuture<VSQMessenger::EnResult>
    logoutAsync();

    Q_INVOKABLE QFuture<VSQMessenger::EnResult>
    disconnect();

    Q_INVOKABLE void
    checkState();

    Q_INVOKABLE void
    setStatus(VSQMessenger::EnStatus status);

    void setCrashReporter(VSQCrashReporter *crashReporter);

    void setCurrentRecipient(const QString &recipient);

    bool subscribeToContact(const Contact::Id &contactId);

signals:
    void
    fireError(QString errorText);

    void
    fireInform(QString informText);

    void
    fireWarning(QString warningText);

    void
    fireConnecting();

    void
    fireReady();

    void chatEntryRequested(const QString &contact);

    void
    fireNewMessage(QString from, QString message);

    void
    fireCurrentUserChanged();

    void openPreviewRequested(const QUrl &url);
    void informationRequested(const QString &message);

    void downloadThumbnail(const MessageV0 message, const QString sender, QPrivateSignal);

    // New signals

    void signedIn(const QString &userId);
    void signInErrorOccured(const QString &errorText);
    void signedUp(const QString &userId);
    void signUpErrorOccured(const QString &errorText);
    void signedOut();

    void keyBackuped(const QString &userId);
    void backupKeyFailed(const QString &errorText);
    void keyDownloaded(const QString &userId);
    void downloadKeyFailed(const QString &errorText);

    void messageStatusChanged(const Message::Id &messageId, const Contact::Id &contactId, const Message::Status &status);

private slots:
    void onConnected();
    void onDisconnected();
    void onError(QXmppClient::Error);
    void onPresenceReceived(const QXmppPresence &presence);
    void onIqReceived(const QXmppIq &iq);
    void onSslErrors(const QList<QSslError> &errors);
    void onStateChanged(QXmppClient::State state);
    void onProcessNetworkState(bool online);
    void onDownloadThumbnail(const MessageV0 message, const QString sender);
    void onAttachmentStatusChanged(const QString &uploadId, const Enums::AttachmentStatus status);
    void onAttachmentProgressChanged(const QString &uploadId, const DataSize bytesReceived, const DataSize bytesTotal);
    void onAttachmentDecrypted(const QString &uploadId, const QString &filePath);

    void registerForNotifications();
    void deregisterFromNotifications();
    void onPushNotificationTokenUpdate();

private:
    QXmppClient m_xmpp;
    QXmppMessageReceiptManager* m_xmppReceiptManager;
    QXmppCarbonManager* m_xmppCarbonManager;
    VSQDiscoveryManager* m_discoveryManager;
    VSQContactManager *m_contactManager;
    VSQLastActivityManager* m_lastActivityManager;

    VSQCrashReporter *m_crashReporter;
    VSQNetworkAnalyzer m_networkAnalyzer;
    VSQSettings *m_settings;
    Validator *m_validator;

    QMutex m_connectGuard;
    QString m_user;
    QString m_userId;
    QString m_recipient;
    VSQEnvType m_envType;
    static const VSQEnvType _defaultEnv = PROD;
    QXmppConfiguration m_conf;
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
    _connect(QString userWithEnv, QString deviceId, QString userId, bool forced = false);

    QString
    _xmppPass();

    QString
    _virgilURL();

    uint16_t
    _xmppPort();

    void
    _reconnect();

    bool
    _saveCredentials(const QString &user, const vs_messenger_virgil_user_creds_t &creds);

    bool
    _loadCredentials(const QString &user, vs_messenger_virgil_user_creds_t &creds);

    QString
    _prepareLogin(const QString &user);

    QString
    _caBundleFile();

    using Function = std::function<void (const MessageV0 &message)>;
    void downloadAndProcess(MessageV0 message, const Function &func);
};

Q_DECLARE_METATYPE(QXmppClient::Error)

#endif // VIRGIL_IOTKIT_QT_MESSENGER_H
