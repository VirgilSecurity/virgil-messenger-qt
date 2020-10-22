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

#include <VSQContactManager.h>
#include <VSQCustomer.h>
#include <VSQDiscoveryManager.h>
#include <VSQDownload.h>
#include <VSQLastActivityManager.h>
#include <VSQMessenger.h>
#include <VSQSqlConversationModel.h>
#include <VSQUpload.h>
#include <VSQUtils.h>
#include <VSQSettings.h>
#include <android/VSQAndroid.h>
#include <helpers/FutureWorker.h>

#if VS_PUSHNOTIFICATIONS
#include "PushNotifications.h"
#include "XmppPushNotifications.h"
using namespace notifications;
using namespace notifications::xmpp;
#endif // VS_PUSHNOTIFICATIONS

#include <qxmpp/QXmppMessage.h>
#include <qxmpp/QXmppUtils.h>
#include <qxmpp/QXmppPushEnableIq.h>
#include <qxmpp/QXmppMessageReceiptManager.h>
#include <qxmpp/QXmppCarbonManager.h>

#include <QtConcurrent>
#include <QStandardPaths>
#include <QSqlDatabase>
#include <QSqlError>
#include <QtQml>
#include <QUuid>

#include <QuickFuture>
Q_DECLARE_METATYPE(VSQMessenger::EnStatus)
Q_DECLARE_METATYPE(VSQMessenger::EnResult)
Q_DECLARE_METATYPE(QFuture<VSQMessenger::EnResult>)

#include <cstring>
#include <cstdlib>

#ifndef USE_XMPP_LOGS
#define USE_XMPP_LOGS 0
#endif

const QString VSQMessenger::kProdEnvPrefix = "prod";
const QString VSQMessenger::kStgEnvPrefix = "stg";
const QString VSQMessenger::kDevEnvPrefix = "dev";
const int VSQMessenger::kConnectionWaitMs = 10000;
const int VSQMessenger::kKeepAliveTimeSec = 10;

using namespace VSQ;

Q_LOGGING_CATEGORY(lcMessenger, "messenger")

using namespace VSQ;

// Helper struct to categorize transfers
struct TransferId
{
    enum class Type
    {
        File,
        Thumbnail
    };

    TransferId() = default;
    TransferId(const QString &messageId, const Type type)
        : messageId(messageId)
        , type(type)
    {}
    ~TransferId() = default;

    static TransferId parse(const QString &rawTransferId)
    {
        const auto parts = rawTransferId.split(';');
        return TransferId(parts.front(), (parts.back() == "thumb") ? Type::Thumbnail : Type::File);
    }

    operator QString() const
    {
        return messageId + ((type == Type::Thumbnail) ? QLatin1String(";thumb") : QLatin1String(";file"));
    }

    QString messageId;
    Type type = Type::File;
};


/******************************************************************************/
VSQMessenger::VSQMessenger(QNetworkAccessManager *networkAccessManager, VSQSettings *settings, Validator *validator)
    : QObject()
    , m_xmpp()
    , m_settings(settings)
    , m_validator(validator)
    , m_transferManager(new VSQCryptoTransferManager(&m_xmpp, networkAccessManager, m_settings, this))
    , m_attachmentBuilder(settings, this)
{
    // Register QML typess
    qmlRegisterType<VSQMessenger>("MesResult", 1, 0, "Result");
    QuickFuture::registerType<VSQMessenger::EnResult>([](VSQMessenger::EnResult res) -> QVariant {
        return QVariant(static_cast<int>(res));
    });

    qRegisterMetaType<QXmppClient::Error>();

    // Connect to Database
    _connectToDatabase();
    m_sqlConversations = new VSQSqlConversationModel(validator, this);
    m_sqlChatModel = new VSQSqlChatModel(this);

    // Add receipt messages extension
    m_xmppReceiptManager = new QXmppMessageReceiptManager();
    m_xmppCarbonManager = new QXmppCarbonManager();
    m_discoveryManager = new VSQDiscoveryManager(&m_xmpp, this);
    m_contactManager = new VSQContactManager(&m_xmpp, this);
    m_lastActivityManager = new VSQLastActivityManager(m_settings, this);

    m_xmpp.addExtension(m_xmppReceiptManager);
    m_xmpp.addExtension(m_xmppCarbonManager);
    m_xmpp.addExtension(m_lastActivityManager);

    // Signal connection
    connect(this, SIGNAL(fireReadyToAddContact(QString)), this, SLOT(onAddContactToDB(QString)));
    connect(this, SIGNAL(fireError(QString)), this, SLOT(disconnect()));
    connect(this, &VSQMessenger::downloadThumbnail, this, &VSQMessenger::onDownloadThumbnail);

    // Uploading
    connect(m_transferManager, &VSQCryptoTransferManager::fireReadyToUpload, this,  &VSQMessenger::onReadyToUpload, Qt::QueuedConnection);
    connect(m_transferManager, &VSQCryptoTransferManager::statusChanged, this, &VSQMessenger::onAttachmentStatusChanged);
    connect(m_transferManager, &VSQCryptoTransferManager::progressChanged, this, &VSQMessenger::onAttachmentProgressChanged);
    connect(m_transferManager, &VSQCryptoTransferManager::fileDecrypted, this, &VSQMessenger::onAttachmentDecrypted);

    // Connect XMPP signals
    connect(&m_xmpp, SIGNAL(connected()), this, SLOT(onConnected()), Qt::QueuedConnection);
    connect(&m_xmpp, SIGNAL(disconnected()), this, SLOT(onDisconnected()));
    connect(&m_xmpp, &QXmppClient::error, this, &VSQMessenger::onError);
    connect(&m_xmpp, SIGNAL(messageReceived(const QXmppMessage &)), this, SLOT(onMessageReceived(const QXmppMessage &)));
    connect(&m_xmpp, SIGNAL(presenceReceived(const QXmppPresence &)), this, SLOT(onPresenceReceived(const QXmppPresence &)));
    connect(&m_xmpp, SIGNAL(iqReceived(const QXmppIq &)), this, SLOT(onIqReceived(const QXmppIq &)));
    connect(&m_xmpp, SIGNAL(sslErrors(const QList<QSslError> &)), this, SLOT(onSslErrors(const QList<QSslError> &)));
    connect(&m_xmpp, SIGNAL(stateChanged(QXmppClient::State)), this, SLOT(onStateChanged(QXmppClient::State)));

    // messages sent to our account (forwarded from another client)
    connect(m_xmppCarbonManager, &QXmppCarbonManager::messageReceived, &m_xmpp, &QXmppClient::messageReceived);
    // messages sent from our account (but another client)
    connect(m_xmppCarbonManager, &QXmppCarbonManager::messageSent, &m_xmpp, &QXmppClient::messageReceived);
    connect(m_xmppReceiptManager, &QXmppMessageReceiptManager::messageDelivered, this, &VSQMessenger::onMessageDelivered);

    // Network Analyzer
    connect(&m_networkAnalyzer, &VSQNetworkAnalyzer::fireStateChanged, this, &VSQMessenger::onProcessNetworkState, Qt::QueuedConnection);
    connect(&m_networkAnalyzer, &VSQNetworkAnalyzer::fireHeartBeat, this, &VSQMessenger::checkState, Qt::QueuedConnection);

    // Push notifications
#if VS_PUSHNOTIFICATIONS
    auto& pushNotifications  = PushNotifications::instance();
    connect(&pushNotifications, &PushNotifications::tokenUpdated, this, &VSQMessenger::onPushNotificationTokenUpdate);
#endif // VS_PUSHNOTIFICATIONS
}

VSQMessenger::~VSQMessenger()
{
}

void VSQMessenger::signIn(const QString &userId)
{
    FutureWorker::run(signInAsync(userId), [=](const FutureResult &result) {
        switch (result) {
        case MRES_OK:
            m_settings->setLastSignedInUserId(userId);
            emit signedIn(userId);
            break;
        case MRES_ERR_NO_CRED:
            emit signInErrorOccured(tr("Cannot load credentials"));
            break;
        case MRES_ERR_SIGNIN:
            emit signInErrorOccured(tr("Cannot sign-in user"));
            break;
        default:
            emit signInErrorOccured(tr("Unknown sign-in error"));
            break;
        }
    });
}

void VSQMessenger::signOut()
{
    FutureWorker::run(logoutAsync(), [=](const FutureResult &) {
        m_settings->setLastSignedInUserId(QString());
        emit signedOut();
    });
}

void VSQMessenger::signUp(const QString &userId)
{
    FutureWorker::run(signUpAsync(userId), [=](const FutureResult &result) {
        switch (result) {
        case MRES_OK:
            m_settings->setLastSignedInUserId(userId);
            emit signedUp(userId);
            break;
        case MRES_ERR_USER_ALREADY_EXISTS:
            emit signUpErrorOccured(tr("Username is already taken"));
            break;
        default:
            emit signUpErrorOccured(tr("Unknown sign-up error"));
            break;
        }
    });
}

void VSQMessenger::addContact(const QString &userId)
{
    FutureWorker::run(addContactAsync(userId), [=](const FutureResult &result) {
        if (result == MRES_OK) {
            emit contactAdded(userId);
        }
        else {
            emit addContactErrorOccured(tr("Contact not found"));
        }
    });
}

void VSQMessenger::backupKey(const QString &password, const QString &confirmedPassword)
{
    if (password.isEmpty()) {
        emit backupKeyFailed(tr("Password cannot be empty"));
    }
    else if (password != confirmedPassword) {
        emit backupKeyFailed(tr("Passwords do not match"));
    }
    else {
        FutureWorker::run(backupKeyAsync(password), [=](const FutureResult &result) {
            if (result == MRES_OK) {
                emit keyBackuped(m_userId);
            }
            else {
                emit backupKeyFailed(tr("Backup private key error"));
            }
        });
    }
}

void VSQMessenger::downloadKey(const QString &userId, const QString &password)
{
    if (password.isEmpty()) {
        emit downloadKeyFailed(tr("Password cannot be empty"));
    }
    else {
        FutureWorker::run(signInWithBackupKeyAsync(userId, password), [=](const FutureResult &result) {
            if (result == MRES_OK) {
                emit keyDownloaded(userId);
            }
            else {
                emit downloadKeyFailed(tr("Private key download error"));
            }
        });
    }
}

void VSQMessenger::sendMessage(const QString &to, const QString &message, const QVariant &attachmentUrl, const Enums::AttachmentType attachmentType)
{
    FutureWorker::run(sendMessageAsync(to, message, attachmentUrl, attachmentType), [=](const FutureResult &result) {
        if (result == MRES_OK) {
            emit messageSent();
        }
    });
}

void
VSQMessenger::onMessageDelivered(const QString& to, const QString& messageId) {

    m_sqlConversations->setMessageStatus(messageId, StMessage::Status::MST_RECEIVED);

    // QMetaObject::invokeMethod(m_sqlConversations, "setMessageStatus",
    //                          Qt::QueuedConnection, Q_ARG(const QString &, messageId),
    //                          Q_ARG(const StMessage::Status, StMessage::Status::MST_RECEIVED));

    qDebug() << "Message with id: '" << messageId << "' delivered to '" << to << "'";
}

/******************************************************************************/
void
VSQMessenger::_connectToDatabase() {
    QSqlDatabase database = QSqlDatabase::database();
    if (!database.isValid()) {
        database = QSqlDatabase::addDatabase("QSQLITE");
        if (!database.isValid())
            qFatal("Cannot add database: %s", qPrintable(database.lastError().text()));
    }

    const QDir writeDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (!writeDir.mkpath("."))
        qFatal("Failed to create writable directory at %s", qPrintable(writeDir.absolutePath()));

    // Ensure that we have a writable location on all devices.
    const QString fileName = writeDir.absolutePath() + "/chat-database.sqlite3";
    // When using the SQLite driver, open() will create the SQLite database if it doesn't exist.
    database.setDatabaseName(fileName);
    if (!database.open()) {
        QFile::remove(fileName);
        qFatal("Cannot open database: %s", qPrintable(database.lastError().text()));
    }
}

/******************************************************************************/
QString
VSQMessenger::_xmppPass() {
    char pass[VS_MESSENGER_VIRGIL_TOKEN_SZ_MAX];

    // Get XMPP password
    if (VS_CODE_OK != vs_messenger_virgil_get_xmpp_pass(pass, VS_MESSENGER_VIRGIL_TOKEN_SZ_MAX)) {
        emit fireError(tr("Cannot get XMPP password"));
        return "";
    }

    return QString::fromLatin1(pass);
}

/******************************************************************************/
Q_DECLARE_METATYPE(QXmppConfiguration)
bool
VSQMessenger::_connect(QString userWithEnv, QString deviceId, QString userId, bool forced) {
    if (forced) {
        m_connectGuard.lock();
    } else if (!m_connectGuard.tryLock()) {
        return false;
    }

    static int cnt = 0;

    const int cur_val = cnt++;

    qCDebug(lcNetwork) << ">>>>>>>>>>> _connect: START " << cur_val;

    // Update users list
    m_settings->addUserToList(userWithEnv);

    // Connect to XMPP
    emit fireConnecting();

    QString jid = userId + "@" + _xmppURL() + "/" + deviceId;
    m_conf.setJid(jid);
    m_conf.setHost(_xmppURL());
    m_conf.setPassword(_xmppPass());
    m_conf.setAutoReconnectionEnabled(false);
    m_conf.setAutoAcceptSubscriptions(true);
#if VS_ANDROID
    m_conf.setKeepAliveInterval(kKeepAliveTimeSec);
    m_conf.setKeepAliveTimeout(kKeepAliveTimeSec - 1);
#endif
    qDebug() << "SSL: " << QSslSocket::supportsSsl();

    auto logger = QXmppLogger::getLogger();
    logger->setLoggingType(QXmppLogger::SignalLogging);
    logger->setMessageTypes(QXmppLogger::AnyMessage);

#if USE_XMPP_LOGS
    connect(logger, &QXmppLogger::message, [=](QXmppLogger::MessageType, const QString &text){
        qDebug() << text;
    });

    m_xmpp.setLogger(logger);
#endif
    if (m_xmpp.isConnected()) {
        // Wait for disconnection
        QTimer timer;
        timer.setSingleShot(true);
        QEventLoop loop;
        connect(&m_xmpp, &QXmppClient::connected, &loop, &QEventLoop::quit);
        connect(&m_xmpp, &QXmppClient::disconnected, &loop, &QEventLoop::quit);
        connect(&m_xmpp, &QXmppClient::error, &loop, &QEventLoop::quit);
        connect( &timer, &QTimer::timeout, &loop, &QEventLoop::quit);

        QMetaObject::invokeMethod(&m_xmpp, "disconnectFromServer", Qt::QueuedConnection);

        timer.start(5000);
        loop.exec();
    }

    // Wait for connection
    QTimer timer;
    timer.setSingleShot(true);
    QEventLoop loop;
    connect(&m_xmpp, &QXmppClient::connected, &loop, &QEventLoop::quit);
    connect(&m_xmpp, &QXmppClient::disconnected, &loop, &QEventLoop::quit);
    connect(&m_xmpp, &QXmppClient::error, &loop, &QEventLoop::quit);
    connect( &timer, &QTimer::timeout, &loop, &QEventLoop::quit);

    qRegisterMetaType<QXmppConfiguration>("QXmppConfiguration");
    QMetaObject::invokeMethod(&m_xmpp, "connectToServer", Qt::QueuedConnection, Q_ARG(QXmppConfiguration, m_conf));

    timer.start(kConnectionWaitMs);
    loop.exec();

    const bool connected = m_xmpp.isConnected();
    qCDebug(lcNetwork) << "<<<<<<<<<<< _connect: FINISH connected = " << connected << "  " << cur_val;

    qDebug() << "Carbons " << (connected ? "enable" : "disable");
    m_xmppCarbonManager->setCarbonsEnabled(connected);
    m_connectGuard.unlock();
    return connected;
}

/******************************************************************************/
void
VSQMessenger::onProcessNetworkState(bool online) {
    if (online) {
        _reconnect();
    } else {
        emit fireError("No internet connection");
    }
}

/******************************************************************************/
QString
VSQMessenger::_caBundleFile() {
#if VS_ANDROID
    return VSQAndroid::caBundlePath();
#else
    return qgetenv("VS_CURL_CA_BUNDLE");
#endif
}

/******************************************************************************/
QString
VSQMessenger::_prepareLogin(const QString &user) {
    QString userId = user;
    m_envType = _defaultEnv;

    // NOTE(vova.y): deprecated logic, user can't contain @
    // Check required environment
    QStringList pieces = user.split("@");
    if (2 == pieces.size()) {
        userId = pieces.at(1);
        if (kProdEnvPrefix == pieces.first()) {
            m_envType = PROD;
        } else if (kStgEnvPrefix == pieces.first()) {
            m_envType = STG;
        }
        else if (kDevEnvPrefix == pieces.first()) {
            m_envType = DEV;
        }
    }

    vs_messenger_virgil_logout();
    char *cCABundle = strdup(_caBundleFile().toStdString().c_str());
    if (VS_CODE_OK != vs_messenger_virgil_init(_virgilURL().toStdString().c_str(), cCABundle)) {
        qCritical() << "Cannot initialize low level messenger";
    }
    free(cCABundle);

    m_crashReporter->setVirgilUrl(_virgilURL());
    m_crashReporter->setkApp(Customer::ApplicationName);
    m_crashReporter->setkOrganization(Customer::OrganizationName);

    // Set current user
    m_user = userId;
    m_sqlConversations->setUser(userId);
    m_sqlChatModel->init(userId);

    // Inform about user activation
    emit fireCurrentUserChanged();

    return userId;
}

/******************************************************************************/
QString VSQMessenger::currentUser() const {
    return m_user;
}

/******************************************************************************/
QString VSQMessenger::currentRecipient() const {
    return m_recipient;
}

/******************************************************************************/
QFuture<VSQMessenger::EnResult>
VSQMessenger::backupKeyAsync(QString password) {
    // m_userId = _prepareLogin(user);
    return QtConcurrent::run([=]() -> EnResult {

        // Upload current user key to the cloud
        if (VS_CODE_OK != vs_messenger_virgil_set_sign_in_password(password.toStdString().c_str())) {
            emit fireError(tr("Cannot set sign in password"));
            return VSQMessenger::EnResult::MRES_ERR_ENCRYPTION;
        }

        return MRES_OK;
    });
}

/******************************************************************************/
QFuture<VSQMessenger::EnResult>
VSQMessenger::signInWithBackupKeyAsync(QString username, QString password) {
    m_userId = _prepareLogin(username);
    return QtConcurrent::run([=]() -> EnResult {

        vs_messenger_virgil_user_creds_t creds;
        memset(&creds, 0, sizeof (creds));

        // Download private key from the cloud
        if (VS_CODE_OK != vs_messenger_virgil_sign_in_with_password(username.toStdString().c_str(), password.toStdString().c_str(), &creds)) {
            emit fireError(tr("Cannot set sign in password"));
            return MRES_ERR_SIGNUP;
        }

        // Save credentials
        _saveCredentials(username, creds);

        return _connect(m_user, m_settings->deviceId(), m_userId, true) ? MRES_OK : MRES_ERR_SIGNIN;;
    });
}

/******************************************************************************/
QFuture<VSQMessenger::EnResult>
VSQMessenger::signInAsync(QString user) {
    m_userId = _prepareLogin(user);
    return QtConcurrent::run([=]() -> EnResult {
        qDebug() << "Trying to sign in: " << m_userId;

        vs_messenger_virgil_user_creds_t creds;
        memset(&creds, 0, sizeof (creds));

        // Load User Credentials
        if (!_loadCredentials(m_userId, creds)) {
            qDebug() << "Sign-in error, code" << MRES_ERR_NO_CRED;
            return MRES_ERR_NO_CRED;
        }

        // Sign In user, using Virgil Service
        if (VS_CODE_OK != vs_messenger_virgil_sign_in(&creds)) {
            qDebug() << "Sign-in error, code:" << MRES_ERR_SIGNIN;
            return MRES_ERR_SIGNIN;
        }

        // Check previous run is crashed
        m_crashReporter->checkAppCrash();

        // Connect over XMPP
        return _connect(m_user, m_settings->deviceId(), m_userId, true) ? MRES_OK : MRES_ERR_SIGNIN;
    });
}



/******************************************************************************/
QFuture<VSQMessenger::EnResult>
VSQMessenger::signUpAsync(QString user) {
    m_userId = _prepareLogin(user);
    return QtConcurrent::run([=]() -> EnResult {
        qInfo() << "Trying to sign up: " << m_userId;

        vs_messenger_virgil_user_creds_t creds;
        memset(&creds, 0, sizeof (creds));

        VirgilIoTKit::vs_status_e status = vs_messenger_virgil_sign_up(m_userId.toStdString().c_str(), &creds);

        if (status != VS_CODE_OK) {
            emit fireError(tr("Cannot sign up user"));
            return MRES_ERR_USER_ALREADY_EXISTS;
        }

        qInfo() << "User has been successfully signed up: " << m_userId;

        // Save credentials
        _saveCredentials(m_userId, creds);

        // Connect over XMPP
        return _connect(m_user, m_settings->deviceId(), m_userId, true) ? MRES_OK : MRES_ERR_SIGNUP;
    });
}

/******************************************************************************/
QFuture<VSQMessenger::EnResult>
VSQMessenger::addContactAsync(QString contact) {
    return QtConcurrent::run([=]() -> EnResult {
        // Sign Up user, using Virgil Service
        if (VS_CODE_OK != vs_messenger_virgil_search(contact.toStdString().c_str())) {
            auto errorText = tr("User is not registered : ") + contact;
            emit fireInform(errorText);
            return MRES_ERR_USER_NOT_FOUND;
        }

        emit fireReadyToAddContact(contact);
        return MRES_OK;
    });
}

/******************************************************************************/
void
VSQMessenger::onAddContactToDB(QString contact) {
    if (!m_contactManager->addContact(contact + "@" + _xmppURL(), contact, QString())) {
        emit fireError(m_contactManager->lastErrorText());
    }
    else {
        m_sqlChatModel->createPrivateChat(contact);
        emit fireAddedContact(contact);
    }
}

void VSQMessenger::onDownloadThumbnail(const StMessage message, const QString sender)
{
    qCDebug(lcTransferManager) << "Downloading of thumbnail for message:" << message.messageId;
    auto attachment = *message.attachment;
    if (attachment.thumbnailPath.isEmpty()) {
        attachment.thumbnailPath = m_attachmentBuilder.generateThumbnailFileName();
    }
    auto future = QtConcurrent::run([=]() {
        const TransferId id(message.messageId, TransferId::Type::Thumbnail);
        auto download = m_transferManager->startCryptoDownload(id, attachment.remoteThumbnailUrl, attachment.thumbnailPath, sender);
        bool success = false;
        QEventLoop loop;
        connect(download, &VSQDownload::ended, [&](bool failed) {
            success = !failed;
            loop.quit();
        });
        loop.exec();
        if(!success) {
            m_sqlConversations->setAttachmentStatus(message.messageId, Attachment::Status::Created);
        }
    });
}

void VSQMessenger::onAttachmentStatusChanged(const QString &uploadId, const Enums::AttachmentStatus status)
{
    const auto id = TransferId::parse(uploadId);
    if (id.type == TransferId::Type::File) {
        m_sqlConversations->setAttachmentStatus(id.messageId, status);
    }
}

void VSQMessenger::onAttachmentProgressChanged(const QString &uploadId, const DataSize bytesReceived, const DataSize bytesTotal)
{
    const auto id = TransferId::parse(uploadId);
    if (id.type == TransferId::Type::File) {
        m_sqlConversations->setAttachmentProgress(id.messageId, bytesReceived, bytesTotal);
    }
}

void VSQMessenger::onAttachmentDecrypted(const QString &uploadId, const QString &filePath)
{
    const auto id = TransferId::parse(uploadId);
    if (id.type == TransferId::Type::File) {
        m_sqlConversations->setAttachmentFilePath(id.messageId, filePath);
    }
    else if (id.type == TransferId::Type::Thumbnail) {
        m_sqlConversations->setAttachmentThumbnailPath(id.messageId, filePath);
    }
}

/******************************************************************************/
QString
VSQMessenger::_virgilURL() {
    QString res = qgetenv("VS_MSGR_VIRGIL");
    if (res.isEmpty()) {
        switch (m_envType) {
        case PROD:
            res = Customer::MessengerUrlTemplate.arg(QString());
            break;

        case STG:
            res = Customer::MessengerUrlTemplate.arg(QLatin1String("-stg"));
            break;

        case DEV:
            res = Customer::MessengerUrlTemplate.arg(QLatin1String("-dev"));
            break;
        }
    }

    qDebug("%s URL: [%s]", qPrintable(Customer::OrganizationName), qPrintable(res));
    return res;
}

/******************************************************************************/
QString
VSQMessenger::_xmppURL() {
    QString res = qgetenv("VS_MSGR_XMPP_URL");

    if (res.isEmpty()) {
        switch (m_envType) {
        case PROD:
            res = Customer::XmppUrlTemplate.arg(QString());;
            break;

        case STG:
            res = Customer::XmppUrlTemplate.arg(QLatin1String("-stg"));
            break;

        case DEV:
            res = Customer::XmppUrlTemplate.arg(QLatin1String("-dev"));
            break;
        }
    }

    qDebug("XMPP URL: %s", res.toStdString().c_str());
    return res;
}

/******************************************************************************/
uint16_t
VSQMessenger::_xmppPort() {
    uint16_t res = 5222;
    QString portStr = qgetenv("VS_MSGR_XMPP_PORT");

    if (!portStr.isEmpty()) {
        bool ok;
        int port = portStr.toInt(&ok);
        if (ok) {
            res = static_cast<uint16_t> (port);
        }
    }

    qDebug("XMPP PORT: %d", static_cast<int> (res));

    return res;
}

/******************************************************************************/
// TODO: Use SecBox
bool
VSQMessenger::_saveCredentials(const QString &user, const vs_messenger_virgil_user_creds_t &creds) {
    // Save credentials
    const QByteArray baCred(reinterpret_cast<const char*>(&creds), sizeof(creds));

    QJsonObject jsonObject;
    jsonObject.insert("creds", QJsonValue::fromVariant(baCred.toBase64()));

    const QJsonDocument doc(jsonObject);
    const QString json = doc.toJson(QJsonDocument::Compact);

    qDebug() << "Saving user credentails: " << json;

    m_settings->setUserCredential(user, json);

    return true;
}

/******************************************************************************/
bool
VSQMessenger::_loadCredentials(const QString &user, vs_messenger_virgil_user_creds_t &creds) {
    const auto settingsJson = m_settings->userCredential(user);
    const QJsonDocument json(QJsonDocument::fromJson(settingsJson.toUtf8()));
    const auto baCred = QByteArray::fromBase64(json["creds"].toString().toUtf8());

    if (baCred.size() != sizeof(creds)) {
        qWarning("Cannot load credentials for : %s", user.toStdString().c_str());
        return false;
    }

    memcpy(&creds, baCred.data(), static_cast<size_t> (baCred.size()));
    return true;
}

/******************************************************************************/
QFuture<VSQMessenger::EnResult>
VSQMessenger::logoutAsync() {
    return QtConcurrent::run([=]() -> EnResult {
        qDebug() << "Logout";
        QMetaObject::invokeMethod(this, "deregisterFromNotifications", Qt::BlockingQueuedConnection);
        QMetaObject::invokeMethod(&m_xmpp, "disconnectFromServer", Qt::BlockingQueuedConnection);
        vs_messenger_virgil_logout();
        m_user = "";
        m_userId = "";
        return MRES_OK;
    });
}

/******************************************************************************/
QFuture<VSQMessenger::EnResult> VSQMessenger::disconnect() {
    bool connected = m_xmpp.isConnected();
    return QtConcurrent::run([=]() -> EnResult {
        qDebug() << "Disconnect";
        if (connected) {
            QMetaObject::invokeMethod(&m_xmpp, "disconnectFromServer", Qt::BlockingQueuedConnection);
        }
        return MRES_OK;
    });
}

/******************************************************************************/
VSQSqlConversationModel &
VSQMessenger::modelConversations() {
    return *m_sqlConversations;
}

VSQSqlChatModel &
VSQMessenger::getChatModel() {
    return *m_sqlChatModel;
}

VSQLastActivityManager *VSQMessenger::lastActivityManager()
{
    return m_lastActivityManager;
}

void VSQMessenger::setCrashReporter(VSQCrashReporter *crashReporter)
{
    m_crashReporter = crashReporter;
}

/******************************************************************************/
void
VSQMessenger::onPushNotificationTokenUpdate() {
    registerForNotifications();
}

void
VSQMessenger::registerForNotifications() {
#if VS_PUSHNOTIFICATIONS

    if (!m_xmpp.isConnected()) {
        qInfo() << "Can not subscribe for push notifications, no connection. Will try it later.";
        return;
    }

    auto xmppPush = XmppPushNotifications::instance().buildEnableIq();

    xmppPush.setNode(currentUser() + "@" + _xmppURL() + "/" + m_settings->deviceId());

#ifdef QT_DEBUG
    QString xml;
    QXmlStreamWriter xmlWriter(&xml);
    xmlWriter.setAutoFormatting(true);
    xmppPush.toXml(&xmlWriter);
    qDebug().noquote() << "Subscribe XMPP request:" << xml;
#endif

    const bool sentStatus = m_xmpp.sendPacket(xmppPush);

    qInfo() << "Subscribe for push notifications status: " << (sentStatus ? "sucess" : "failed");
#endif // VS_PUSHNOTIFICATIONS
}

void
VSQMessenger::deregisterFromNotifications() {
#if VS_PUSHNOTIFICATIONS

    if (!m_xmpp.isConnected()) {
        qInfo() << "Can not unsubscribe from push notifications, no connection.";
        return;
    }

    auto xmppPush = XmppPushNotifications::instance().buildDisableIq();

    xmppPush.setNode(currentUser() + "@" + _xmppURL() + "/" + m_settings->deviceId());

#ifdef QT_DEBUG
    QString xml;
    QXmlStreamWriter xmlWriter(&xml);
    xmlWriter.setAutoFormatting(true);
    xmppPush.toXml(&xmlWriter);
    qDebug().noquote() << "Unsubscribe XMPP request:" << xml;
#endif

    const bool sentStatus = m_xmpp.sendPacket(xmppPush);

    qInfo() << "Unsubscribe from push notifications status: " << (sentStatus ? "sucess" : "failed");
#endif // VS_PUSHNOTIFICATIONS
}

/******************************************************************************/
void
VSQMessenger::onConnected() {
    registerForNotifications();
}

/******************************************************************************/
void
VSQMessenger::onReadyToUpload() {
    emit fireReady();
    _sendFailedMessages();
}

/******************************************************************************/
void
VSQMessenger::_sendFailedMessages() {
   auto messages = m_sqlConversations->getMessages(m_user, StMessage::Status::MST_FAILED);
   QtConcurrent::run([=]() {
       for (int i = 0; i < messages.length(); i++) {
           const auto &msg = messages[i];
           if (MRES_OK != _sendMessageInternal(false, msg.messageId, msg.recipient, msg.message, msg.attachment)) {
               break;
           }
       }
   });
}

QString VSQMessenger::createJson(const QString &message, const OptionalAttachment &attachment)
{
    QJsonObject mainObject;
    QJsonObject payloadObject;
    if (!attachment) {
        mainObject.insert("type", "text");
        payloadObject.insert("body", message);
    }
    else {
        if (attachment->type == Attachment::Type::Picture) {
            mainObject.insert("type", "picture");
            payloadObject.insert("thumbnailUrl", attachment->remoteThumbnailUrl.toString());
            payloadObject.insert("thumbnailWidth", attachment->thumbnailSize.width());
            payloadObject.insert("thumbnailHeight", attachment->thumbnailSize.height());
        }
        else {
            mainObject.insert("type", "file");
        }
        payloadObject.insert("url", attachment->remoteUrl.toString());
        payloadObject.insert("fileName", attachment->fileName);
        payloadObject.insert("displayName", attachment->displayName);
        payloadObject.insert("bytesTotal", attachment->bytesTotal);
    }
    mainObject.insert("payload", payloadObject);

    QJsonDocument doc(mainObject);
    return doc.toJson(QJsonDocument::Compact);
}

StMessage VSQMessenger::parseJson(const QJsonDocument &json)
{
    const auto type = json["type"].toString();
    const auto payload = json["payload"];
    StMessage message;
    if (type == QLatin1String("text")) {
        message.message = payload["body"].toString();
        return message;
    }
    Attachment attachment;
    attachment.id = VSQUtils::createUuid();
    attachment.remoteUrl = payload["url"].toString();
    attachment.fileName = payload["fileName"].toString();
    attachment.displayName = payload["displayName"].toString();
    attachment.bytesTotal = payload["bytesTotal"].toInt();
    message.message = attachment.displayName;
    if (type == QLatin1String("picture")) {
        attachment.type = Attachment::Type::Picture;
        attachment.remoteThumbnailUrl = payload["thumbnailUrl"].toString();
        attachment.thumbnailSize = QSize(payload["thumbnailWidth"].toInt(), payload["thumbnailHeight"].toInt());
    }
    else {
        attachment.type = Attachment::Type::File;
    }
    message.attachment = attachment;
    return message;
}

OptionalAttachment VSQMessenger::uploadAttachment(const QString messageId, const QString recipient, const Attachment &attachment)
{
    if (!m_transferManager->isReady()) {
        qCDebug(lcMessenger) << "Transfer manager is not ready";
        setFailedAttachmentStatus(messageId);
        return NullOptional;
    }

    Attachment uploadedAttachment = attachment;

    bool thumbnailUploadNeeded = attachment.type == Attachment::Type::Picture && attachment.remoteThumbnailUrl.isEmpty();
    if (thumbnailUploadNeeded) {
        qCDebug(lcMessenger) << "Thumbnail uploading...";
        const TransferId uploadId(messageId, TransferId::Type::Thumbnail);
        if (m_transferManager->hasTransfer(uploadId)) {
            qCCritical(lcMessenger) << "Thumbnail upload for" << messageId << "already exists";
            return NullOptional;
        }
        else if (auto upload = m_transferManager->startCryptoUpload(uploadId, attachment.thumbnailPath, recipient)) {
            m_sqlConversations->setAttachmentStatus(messageId, Attachment::Status::Loading);
            QEventLoop loop;
            QMutex guard;
            auto con = connect(upload, &VSQUpload::ended, [&](bool failed) {
                QMutexLocker l(&guard);
                if (failed) {
                    setFailedAttachmentStatus(messageId);
                }
                else {
                    m_sqlConversations->setAttachmentThumbnailRemoteUrl(messageId, *upload->remoteUrl());
                    uploadedAttachment.remoteThumbnailUrl = *upload->remoteUrl();
                    thumbnailUploadNeeded = false;
                }
                loop.quit();
            });
            connect(upload, &VSQUpload::connectionChanged, &loop, &QEventLoop::quit);
            qCDebug(lcMessenger) << "Upload waiting: start";
            loop.exec();
            QObject::disconnect(con);
            QMutexLocker l(&guard);
            qCDebug(lcMessenger) << "Upload waiting: end";
            if (!thumbnailUploadNeeded) {
                qCDebug(lcMessenger) << "Thumbnail was uploaded";
            }
        }
        else  {
            qCDebug(lcMessenger) << "Unable to start upload";
            setFailedAttachmentStatus(messageId);
            return NullOptional;
        }
    }

    bool attachmentUploadNeeded = attachment.remoteUrl.isEmpty();
    if (attachmentUploadNeeded) {
        qCDebug(lcMessenger) << "Attachment uploading...";
        const TransferId id(messageId, TransferId::Type::File);
        if (m_transferManager->hasTransfer(messageId)) {
            qCCritical(lcMessenger) << "Upload for" << messageId << "already exists";
            return NullOptional;
        }
        else if (auto upload = m_transferManager->startCryptoUpload(id, attachment.filePath, recipient)) {
            m_sqlConversations->setAttachmentStatus(messageId, Attachment::Status::Loading);
            uploadedAttachment.bytesTotal = upload->fileSize();
            m_sqlConversations->setAttachmentBytesTotal(messageId, upload->fileSize());
            QEventLoop loop;
            QMutex guard;
            auto con = connect(upload, &VSQUpload::ended, [&](bool failed) {
                QMutexLocker locker(&guard);
                if (failed) {
                    setFailedAttachmentStatus(messageId);
                }
                else {
                    m_sqlConversations->setAttachmentRemoteUrl(messageId, *upload->remoteUrl());
                    uploadedAttachment.remoteUrl = *upload->remoteUrl();
                    attachmentUploadNeeded = false;
                }
                loop.quit();
            });
            connect(upload, &VSQUpload::connectionChanged, &loop, &QEventLoop::quit);
            qCDebug(lcMessenger) << ": start";
            loop.exec();
            QObject::disconnect(con);
            QMutexLocker locker(&guard);
            qCDebug(lcMessenger) << "Upload waiting: end";
            if (!attachmentUploadNeeded) {
                qCDebug(lcMessenger) << "Attachment was uploaded";
            }
        }
        else {
            qCDebug(lcMessenger) << "Unable to start upload";
            setFailedAttachmentStatus(messageId);
            return NullOptional;
        }
    }

    if (thumbnailUploadNeeded || attachmentUploadNeeded) {
        qCDebug(lcMessenger) << "Thumbnail or/and attachment were not uploaded";
        return NullOptional;
    }
    qCDebug(lcMessenger) << "Everything was uploaded";
    m_sqlConversations->setAttachmentStatus(messageId, Attachment::Status::Loaded);
    uploadedAttachment.status = Attachment::Status::Loaded;
    return uploadedAttachment;
}

void VSQMessenger::setFailedAttachmentStatus(const QString &messageId)
{
    m_sqlConversations->setMessageStatus(messageId, StMessage::Status::MST_FAILED);
    m_sqlConversations->setAttachmentStatus(messageId, Attachment::Status::Failed);
}

/******************************************************************************/
void
VSQMessenger::checkState() {
    if (vs_messenger_virgil_is_signed_in() && (m_xmpp.state() == QXmppClient::DisconnectedState)) {
        qCDebug(lcNetwork) << "We should be connected, but it's not so. Let's try to reconnect.";
#if VS_ANDROID
        emit fireError(tr("Disconnected ..."));
#endif
        _reconnect();
    } else {
#if 0
        qCDebug(lcNetwork) << "Connection is ok";
#endif
    }
}

/******************************************************************************/
void
VSQMessenger::_reconnect() {
    if (!m_user.isEmpty() && !m_userId.isEmpty() && vs_messenger_virgil_is_signed_in()) {
        QtConcurrent::run([=]() {
            _connect(m_user, m_settings->deviceId(), m_userId);
        });
    }
}

/******************************************************************************/
void
VSQMessenger::onDisconnected() {
    qDebug() << "onDisconnected  state:" << m_xmpp.state();
    qDebug() << "Carbons disable";
    m_xmppCarbonManager->setCarbonsEnabled(false);
}

/******************************************************************************/
void
VSQMessenger::onError(QXmppClient::Error err) {
    qDebug("onError");
    qDebug() << "onError : " << err << "   state:" << m_xmpp.state();
    emit fireError(tr("Connection error ..."));
}

/******************************************************************************/
Optional<StMessage> VSQMessenger::decryptMessage(const QString &sender, const QString &message) {
    const size_t _decryptedMsgSzMax = VSQUtils::bufferSizeForDecryption(message.size());
    uint8_t decryptedMessage[_decryptedMsgSzMax];
    size_t decryptedMessageSz = 0;

    qDebug() << "Sender            : " << sender;
    qDebug() << "Encrypted message : " << message.length() << " bytes";

    // Decrypt message
    // DECRYPTED_MESSAGE_SZ_MAX - 1  - This is required for a Zero-terminated string
    if (VS_CODE_OK != vs_messenger_virgil_decrypt_msg(
                sender.toStdString().c_str(),
                message.toStdString().c_str(),
                decryptedMessage, decryptedMessageSz - 1,
                &decryptedMessageSz)) {
        qWarning("Received message cannot be decrypted");
        return NullOptional;
    }

    // Add Zero termination
    decryptedMessage[decryptedMessageSz] = 0;

    // Get message from JSON
    QByteArray baDecr(reinterpret_cast<char *> (decryptedMessage), static_cast<int> (decryptedMessageSz));
    QJsonDocument jsonMsg(QJsonDocument::fromJson(baDecr));

    qCDebug(lcMessenger) << "JSON for parsing:" << jsonMsg;
    auto msg = parseJson(jsonMsg);
    qCDebug(lcMessenger) << "Received message: " << msg.message;
    return msg;
}

void VSQMessenger::setApplicationActive(bool active)
{
    m_lastActivityManager->setEnabled(active);
}

/******************************************************************************/
void
VSQMessenger::onMessageReceived(const QXmppMessage &message) {

    QString sender = message.from().split("@").first();
    QString recipient = message.to().split("@").first();

    qInfo() << "Sender: " << sender << " Recipient: " << recipient;

    // Decrypt message
    auto msg = decryptMessage(sender, message.body());
    if (!msg)
        return;

    msg->messageId = message.id();
    if (sender == currentUser()) {
        QString recipient = message.to().split("@").first();
        m_sqlConversations->createMessage(recipient, msg->message, msg->messageId, msg->attachment);
        m_sqlConversations->setMessageStatus(msg->messageId, StMessage::Status::MST_SENT);
        // ensure private chat with recipient exists
        m_contactManager->addContact(recipient + "@" + _xmppURL(), recipient, QString());
        m_sqlChatModel->createPrivateChat(recipient);
        emit fireNewMessage(sender, msg->message);
        return;
    }

    // Add sender to contact
    m_contactManager->addContact(sender + "@" + _xmppURL(), sender, QString());
    m_sqlChatModel->createPrivateChat(sender);
    // Save message to DB
    m_sqlConversations->receiveMessage(msg->messageId, sender, msg->message, msg->attachment);
    m_sqlChatModel->updateLastMessage(sender, msg->message);
    if (sender != m_recipient) {
        m_sqlChatModel->updateUnreadMessageCount(sender);
    }

    if (msg->attachment && msg->attachment->type == Attachment::Type::Picture) {
        emit downloadThumbnail(*msg, sender, QPrivateSignal());
    }

    // Inform system about new message
    emit fireNewMessage(sender, msg->message);
}

/******************************************************************************/

VSQMessenger::EnResult
VSQMessenger::_sendMessageInternal(bool createNew, const QString &messageId, const QString &to, const QString &message, const OptionalAttachment &attachment)
{
    // Write to database
    if(createNew) {
        m_sqlConversations->createMessage(to, message, messageId, attachment);
    }
    m_sqlChatModel->updateLastMessage(to, message);

    OptionalAttachment updloadedAttacment;
    if (attachment) {
        qCDebug(lcMessenger) << "Trying to upload the attachment";
        updloadedAttacment = uploadAttachment(messageId, to, *attachment);
        if (!updloadedAttacment) {
            qCDebug(lcMessenger) << "Attachment was NOT uploaded";
            return MRES_OK; // don't send message
        }
        qCDebug(lcMessenger) << "Everything was uploaded. Continue to send message";
    }

    // Create JSON-formatted message to be sent
    const QString internalJson = createJson(message, updloadedAttacment);
    qDebug() << "Json for encryption:" << internalJson;

    // Encrypt message
    QMutexLocker _guard(&m_messageGuard);
    const auto plaintext = internalJson.toStdString();
    const size_t _encryptedMsgSzMax = VSQUtils::bufferSizeForEncryption(plaintext.size());
    uint8_t encryptedMessage[_encryptedMsgSzMax];
    size_t encryptedMessageSz = 0;

    if (VS_CODE_OK != vs_messenger_virgil_encrypt_msg(
                     to.toStdString().c_str(),
                     reinterpret_cast<const uint8_t*>(plaintext.c_str()),
                     plaintext.length(),
                     encryptedMessage,
                     _encryptedMsgSzMax,
                     &encryptedMessageSz)) {
        qWarning("Cannot encrypt message to be sent");

        // Mark message as failed
        m_sqlConversations->setMessageStatus(messageId, StMessage::Status::MST_FAILED);
        return MRES_ERR_ENCRYPTION;
    }

    // Send encrypted message
    QString toJID = to + "@" + _xmppURL();
    QString fromJID = currentUser() + "@" + _xmppURL();
    QString encryptedStr = QString::fromLatin1(reinterpret_cast<char*>(encryptedMessage), encryptedMessageSz);

    QXmppMessage msg(fromJID, toJID, encryptedStr);
    msg.setReceiptRequested(true);
    msg.setId(messageId);

    // Send message and update status
    if (m_xmpp.sendPacket(msg)) {
        m_sqlConversations->setMessageStatus(messageId, StMessage::Status::MST_SENT);
    } else {
        m_sqlConversations->setMessageStatus(messageId, StMessage::Status::MST_FAILED);
    }
    qCDebug(lcMessenger) << "Message sent:" << messageId;
    return MRES_OK;
}

void VSQMessenger::downloadAndProcess(StMessage message, const Function &func)
{
    if (!message.attachment) {
        qCDebug(lcTransferManager) << "Message has no attachment:" << message.messageId;
        return;
    }
    auto future = QtConcurrent::run([=]() {
        auto msg = message;
        auto &attachment = *msg.attachment;
        auto &filePath = attachment.filePath;
        if (QFile::exists(filePath)) {
            func(msg);
            return;
        }
        // Update attachment filePath
        const auto downloads = m_settings->downloadsDir();
        if (filePath.isEmpty() || QFileInfo(filePath).dir() != downloads) {
            filePath = VSQUtils::findUniqueFileName(downloads.filePath(attachment.fileName));
        }
        const TransferId id(msg.messageId, TransferId::Type::File);
        auto download = m_transferManager->startCryptoDownload(id, attachment.remoteUrl, filePath, msg.sender);
        bool success = false;
        QEventLoop loop;
        QMutex guard;
        auto con = connect(download, &VSQDownload::ended, [&](bool failed) {
            QMutexLocker l(&guard);
            success = !failed;
            loop.quit();
        });
        connect(m_transferManager, &VSQCryptoTransferManager::fileDecrypted, download, [=](const QString &id, const QString &filePath) {
            qCDebug(lcTransferManager) << "Comparing of downloaded file with requested..." << filePath;
            if (TransferId::parse(id).messageId == msg.messageId) {
                auto msgWithPath = msg;
                msgWithPath.attachment->filePath = filePath;
                func(msgWithPath);
            }
        });
        loop.exec();
        QObject::disconnect(con);
        QMutexLocker l(&guard);

        if(!success) {
            m_sqlConversations->setAttachmentStatus(message.messageId, Attachment::Status::Created);
        }
    });
}

QFuture<VSQMessenger::EnResult>
VSQMessenger::createSendMessage(const QString messageId, const QString to, const QString message)
{
    return QtConcurrent::run([=]() -> EnResult {
        qCDebug(lcMessenger) << "Message creation started:" << messageId;
        return _sendMessageInternal(true, messageId, to, message, NullOptional);
    });
}

QFuture<VSQMessenger::EnResult>
VSQMessenger::createSendAttachment(const QString messageId, const QString to,
                                   const QUrl url, const Enums::AttachmentType attachmentType)
{
    return QtConcurrent::run([=]() -> EnResult {
        qCDebug(lcMessenger) << "Message creation started:" << messageId << "Attachment type:" << attachmentType;
        QString warningText;
        auto attachment = m_attachmentBuilder.build(url, attachmentType, warningText);
        if (!attachment) {
            qCWarning(lcAttachment) << warningText;
            fireWarning(warningText);
            return MRES_ERR_ATTACHMENT;
        }
        return _sendMessageInternal(true, messageId, to, attachment->displayName, attachment);
    });
}

/******************************************************************************/
QFuture<VSQMessenger::EnResult>
VSQMessenger::sendMessageAsync(const QString &to, const QString &message,
                               const QVariant &attachmentUrl, const Enums::AttachmentType attachmentType)
{
    const auto url = attachmentUrl.toUrl();
    if (VSQUtils::isValidUrl(url)) {
        return createSendAttachment(VSQUtils::createUuid(), to, url, attachmentType);
    }
    else {
        return createSendMessage(VSQUtils::createUuid(), to, message);
    }
}

/******************************************************************************/
void
VSQMessenger::setStatus(VSQMessenger::EnStatus status) {
    QXmppPresence::Type presenceType = VSQMessenger::MSTATUS_ONLINE == status ?
                QXmppPresence::Available : QXmppPresence::Unavailable;
    m_xmpp.setClientPresence(QXmppPresence(presenceType));
}

/******************************************************************************/
void VSQMessenger::setCurrentRecipient(const QString &recipient)
{
    m_recipient = recipient;
    if (recipient.isEmpty()) {
        m_lastActivityManager->setCurrentJid(QString());
    }
    else {
        m_lastActivityManager->setCurrentJid(recipient + "@" + _xmppURL());
    }
}

void VSQMessenger::saveAttachmentAs(const QString &messageId, const QVariant &fileUrl)
{
    auto message = m_sqlConversations->getMessage(messageId);
    if (!message) {
        return;
    }
    const auto dest = VSQUtils::urlToLocalFile(fileUrl.toUrl());
    qCDebug(lcTransferManager) << "Saving of attachment as" << dest;
    downloadAndProcess(*message, [=](const StMessage &msg) {
        qCDebug(lcTransferManager) << "Message attachment saved as:" << dest;
        const auto src = msg.attachment->filePath;
        QFile::copy(src, dest);
    });
}

void VSQMessenger::downloadAttachment(const QString &messageId)
{
    auto message = m_sqlConversations->getMessage(messageId);
    if (!message) {
        return;
    }
    qCDebug(lcTransferManager) << "Downloading of attachment:" << messageId;
    downloadAndProcess(*message, [this](const StMessage &msg) {
        qCDebug(lcTransferManager) << QString("Message '%1' attachment was downloaded").arg(msg.messageId);
        emit informationRequested("Saved to downloads");
    });
}

void VSQMessenger::openAttachment(const QString &messageId)
{
    auto message = m_sqlConversations->getMessage(messageId);
    if (!message) {
        return;
    }
    qCDebug(lcTransferManager) << "Opening of attachment:" << messageId;
    downloadAndProcess(*message, [this](const StMessage &msg) {
        const auto url = VSQUtils::localFileToUrl(msg.attachment->filePath);
        qCDebug(lcTransferManager) << "Opening of message attachment:" << url;
        emit openPreviewRequested(url);
    });
}

/******************************************************************************/
void
VSQMessenger::onPresenceReceived(const QXmppPresence &presence) {
    Q_UNUSED(presence)
}

/******************************************************************************/
void
VSQMessenger::onIqReceived(const QXmppIq &iq) {
    Q_UNUSED(iq)
}

/******************************************************************************/
void
VSQMessenger::onSslErrors(const QList<QSslError> &errors) {
    Q_UNUSED(errors)
    emit fireError(tr("Secure connection error ..."));
}

/******************************************************************************/
void
VSQMessenger::onStateChanged(QXmppClient::State state) {
    if (QXmppClient::ConnectingState == state) {
        emit fireConnecting();
    }
}

/******************************************************************************/
