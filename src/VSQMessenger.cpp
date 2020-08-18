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

#include <android/VSQAndroid.h>
#include <cstring>
#include <cstdlib>

#include <VSQDownload.h>
#include <VSQMessenger.h>
#include <VSQPushNotifications.h>
#include <VSQSqlConversationModel.h>
#include <VSQUtils.h>
#include <VSQSettings.h>

#include <android/VSQAndroid.h>
#include <android/VSQAndroid.h>

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

#ifndef USE_XMPP_LOGS
#define USE_XMPP_LOGS 0
#endif


const QString VSQMessenger::kOrganization = "VirgilSecurity";
const QString VSQMessenger::kApp = "VirgilMessenger";
const QString VSQMessenger::kUsers = "Users";
const QString VSQMessenger::kProdEnvPrefix = "prod";
const QString VSQMessenger::kStgEnvPrefix = "stg";
const QString VSQMessenger::kDevEnvPrefix = "dev";
const QString VSQMessenger::kPushNotificationsProxy = "push-notifications-proxy";
const QString VSQMessenger::kPushNotificationsNode = "node";
const QString VSQMessenger::kPushNotificationsService = "service";
const QString VSQMessenger::kPushNotificationsFCM = "fcm";
const QString VSQMessenger::kPushNotificationsDeviceID = "device_id";
const QString VSQMessenger::kPushNotificationsFormType = "FORM_TYPE";
const QString VSQMessenger::kPushNotificationsFormTypeVal = "http://jabber.org/protocol/pubsub#publish-options";
const int VSQMessenger::kConnectionWaitMs = 10000;
const int VSQMessenger::kKeepAliveTimeSec = 10;

/******************************************************************************/
VSQMessenger::VSQMessenger(QNetworkAccessManager *networkAccessManager, VSQSettings *settings)
    : QObject()
    , m_xmpp()
    , m_settings(settings)
    , m_transferManager(new VSQCryptoTransferManager(&m_xmpp, networkAccessManager, m_settings, this))
    , m_attachmentBuilder(m_transferManager, this)
{
    // Register QML typess
    qmlRegisterType<VSQMessenger>("MesResult", 1, 0, "Result");
    QuickFuture::registerType<VSQMessenger::EnResult>([](VSQMessenger::EnResult res) -> QVariant {
        return QVariant(static_cast<int>(res));
    });

    // Connect to Database
    _connectToDatabase();

    m_sqlConversations = new VSQSqlConversationModel(this);
    m_sqlConversations->connectTransferManager(m_transferManager);
    m_sqlChatModel = new VSQSqlChatModel(this);

    // Add receipt messages extension
    m_xmppReceiptManager = new QXmppMessageReceiptManager();
    m_xmppCarbonManager = new QXmppCarbonManager();
    m_xmppDiscoveryManager = new VSQDiscoveryManager(&m_xmpp, this);

    m_xmpp.addExtension(m_xmppReceiptManager);
    m_xmpp.addExtension(m_xmppCarbonManager);

    // Signal connection
    connect(this, SIGNAL(fireReadyToAddContact(QString)), this, SLOT(onAddContactToDB(QString)));
    connect(this, SIGNAL(fireError(QString)), this, SLOT(disconnect()));

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

    // Uploader
    connect(m_transferManager, &VSQCryptoTransferManager::fireReadyToUpload, this,  &VSQMessenger::onReadyToUpload, Qt::QueuedConnection);

    // Use Push notifications
#if VS_PUSHNOTIFICATIONS
    VSQPushNotifications::instance().startMessaging();
#endif
}

VSQMessenger::~VSQMessenger()
{
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
    if (!m_xmppPass.isEmpty()) {
        return m_xmppPass;
    }

    const size_t _pass_buf_sz = 512;
    char pass[_pass_buf_sz];

    // Get XMPP password
    if (VS_CODE_OK != vs_messenger_virgil_get_xmpp_pass(pass, _pass_buf_sz)) {
        emit fireError(tr("Cannot get XMPP password"));
        return "";
    }

    m_xmppPass = QString::fromLatin1(pass);

    return m_xmppPass;
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
    _addToUsersList(userWithEnv);

    // Connect to XMPP
    emit fireConnecting();

    QString jid = userId + "@" + _xmppURL() + "/" + deviceId;
    conf.setJid(jid);
    conf.setHost(_xmppURL());
    conf.setPassword(_xmppPass());
    conf.setAutoReconnectionEnabled(false);
#if VS_ANDROID
    conf.setKeepAliveInterval(kKeepAliveTimeSec);
    conf.setKeepAliveTimeout(kKeepAliveTimeSec - 1);
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

        timer.start(2000);
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
    QMetaObject::invokeMethod(&m_xmpp, "connectToServer", Qt::QueuedConnection, Q_ARG(QXmppConfiguration, conf));

    timer.start(kConnectionWaitMs);
    loop.exec();

    const bool connected = m_xmpp.isConnected();
    qDebug() << "<<<<<<<<<<< _connect: FINISH connected = " << connected << "  " << cur_val;

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

    m_logging->setVirgilUrl(_virgilURL());
    m_logging->setkApp(kApp);
    m_logging->setkOrganization(kOrganization);

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
VSQMessenger::backupUserKey(QString password) {
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
VSQMessenger::signInWithBackupKey(QString username, QString password) {
    m_userId = _prepareLogin(username);
    return QtConcurrent::run([=]() -> EnResult {

        vs_messenger_virgil_user_creds_t creds;
        memset(&creds, 0, sizeof (creds));

        // Download private key from the cloud
        if (VS_CODE_OK != vs_messenger_virgil_sign_in_with_password(username.toStdString().c_str(), password.toStdString().c_str(), &creds)) {
            emit fireError(tr("Cannot set sign in password"));
            return MRES_ERR_SIGNUP;
        }

        m_deviceId = QUuid::createUuid().toString(QUuid::WithoutBraces).toLower();

        // Save credentials
        _saveCredentials(username, m_deviceId, creds);

        return _connect(m_user, m_deviceId, m_userId, true) ? MRES_OK : MRES_ERR_SIGNIN;;
    });
}

/******************************************************************************/
QFuture<VSQMessenger::EnResult>
VSQMessenger::signIn(QString user) {
    m_userId = _prepareLogin(user);
    return QtConcurrent::run([=]() -> EnResult {
        qDebug() << "Trying to Sign In: " << m_userId;

        vs_messenger_virgil_user_creds_t creds;
        memset(&creds, 0, sizeof (creds));

        // Load User Credentials
        if (!_loadCredentials(m_userId, m_deviceId, creds)) {
            emit fireError(tr("Cannot load user credentials"));
            qDebug() << "Cannot load user credentials";
            return MRES_ERR_NO_CRED;
        }

        // Sign In user, using Virgil Service
        if (VS_CODE_OK != vs_messenger_virgil_sign_in(&creds)) {
            emit fireError(tr("Cannot Sign In user"));
            qDebug() << "Cannot Sign In user";
            return MRES_ERR_SIGNIN;
        }

        // Check previus run is crashed
        m_logging->checkAppCrash();

        // Connect over XMPP
        return _connect(m_user, m_deviceId, m_userId, true) ? MRES_OK : MRES_ERR_SIGNIN;
    });
}



/******************************************************************************/
QFuture<VSQMessenger::EnResult>
VSQMessenger::signUp(QString user) {
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

        m_deviceId = QUuid::createUuid().toString(QUuid::WithoutBraces).toLower();

        // Save credentials
        _saveCredentials(m_userId, m_deviceId, creds);

        // Connect over XMPP
        return _connect(m_user, m_deviceId, m_userId, true) ? MRES_OK : MRES_ERR_SIGNUP;
    });
}

/******************************************************************************/
QFuture<VSQMessenger::EnResult>
VSQMessenger::addContact(QString contact) {
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
    m_sqlChatModel->createPrivateChat(contact);
    // m_sqlContacts->addContact(contact);
    emit fireAddedContact(contact);
}

/******************************************************************************/
QString
VSQMessenger::_virgilURL() {
    QString res = qgetenv("VS_MSGR_VIRGIL");
    if (res.isEmpty()) {
        switch (m_envType) {
        case PROD:
            res = "https://messenger.virgilsecurity.com";
            break;

        case STG:
            res = "https://messenger-stg.virgilsecurity.com";
            break;

        case DEV:
            res = "https://messenger-dev.virgilsecurity.com";
            break;
        }
    }

    VS_LOG_DEBUG("Virgil URL: [%s]", qPrintable(res));
    return res;
}

/******************************************************************************/
QString
VSQMessenger::_xmppURL() {
    QString res = qgetenv("VS_MSGR_XMPP_URL");

    if (res.isEmpty()) {
        switch (m_envType) {
        case PROD:
            res = "xmpp.virgilsecurity.com";
            break;

        case STG:
            res = "xmpp-stg.virgilsecurity.com";
            break;

        case DEV:
            res = "xmpp-dev.virgilsecurity.com";
            break;
        }
    }

    VS_LOG_DEBUG("XMPP URL: %s", res.toStdString().c_str());
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

    VS_LOG_DEBUG("XMPP PORT: %d", static_cast<int> (res));

    return res;
}

/******************************************************************************/

void
VSQMessenger::_addToUsersList(const QString &user) {
    // Save known users
    auto knownUsers = usersList();
    knownUsers.removeAll(user);
    knownUsers.push_front(user);
    _saveUsersList(knownUsers);
}

/******************************************************************************/
// TODO: Use SecBox
bool
VSQMessenger::_saveCredentials(const QString &user, const QString &deviceId, const vs_messenger_virgil_user_creds_t &creds) {
    // Save credentials
    QByteArray baCred(reinterpret_cast<const char*>(&creds), sizeof(creds));

    QJsonObject jsonObject;
    jsonObject.insert("device_id", deviceId);
    jsonObject.insert("creds", QJsonValue::fromVariant(baCred.toBase64()));

    QJsonDocument doc(jsonObject);
    QString json = doc.toJson(QJsonDocument::Compact);

    qInfo() << "Saving user credentails: " << json;

    QSettings settings(kOrganization, kApp);
    settings.setValue(user, json);

    return true;
}

/******************************************************************************/
bool
VSQMessenger::_loadCredentials(const QString &user, QString &deviceId, vs_messenger_virgil_user_creds_t &creds) {
    QSettings settings(kOrganization, kApp);

    auto settingsJson = settings.value(user, QString("")).toString();
    QJsonDocument json(QJsonDocument::fromJson(settingsJson.toUtf8()));
    deviceId = json["device_id"].toString();
    auto baCred = QByteArray::fromBase64(json["creds"].toString().toUtf8());

    if (baCred.size() != sizeof(creds)) {
        VS_LOG_WARNING("Cannot load credentials for : %s", user.toStdString().c_str());
        return false;
    }

    memcpy(&creds, baCred.data(), static_cast<size_t> (baCred.size()));
    return true;
}

/******************************************************************************/
void
VSQMessenger::_saveUsersList(const QStringList &users) {
    QSettings settings(kOrganization, kApp);
    settings.setValue(kUsers, users);
}

/******************************************************************************/
QStringList
VSQMessenger::usersList() {
    QSettings settings(kOrganization, kApp);
    qDebug() << settings.fileName();
    return settings.value(kUsers, QStringList()).toStringList();
}

/******************************************************************************/
QFuture<VSQMessenger::EnResult>
VSQMessenger::logout() {
    return QtConcurrent::run([=]() -> EnResult {
        qDebug() << "Logout";
        m_user = "";
        m_userId = "";
        m_xmppPass = "";
        QMetaObject::invokeMethod(this, "onSubscribePushNotifications", Qt::BlockingQueuedConnection, Q_ARG(bool, false));
        QMetaObject::invokeMethod(&m_xmpp, "disconnectFromServer", Qt::BlockingQueuedConnection);
        vs_messenger_virgil_logout();
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
QFuture<VSQMessenger::EnResult>
VSQMessenger::deleteUser(QString user) {
    return QtConcurrent::run([=]() -> EnResult {
        Q_UNUSED(user)
        logout();
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

void VSQMessenger::setLogging(VSQLogging *loggingPtr) {
    m_logging = loggingPtr;
}


/******************************************************************************/
void
VSQMessenger::onSubscribePushNotifications(bool enable) {
    #if VS_PUSHNOTIFICATIONS

    // Subscribe Form Type
    QXmppDataForm::Field subscribeFormType;
    subscribeFormType.setKey(kPushNotificationsFormType);
    subscribeFormType.setValue(kPushNotificationsFormTypeVal);

    // Subscribe service
    QXmppDataForm::Field subscribeService;
    subscribeService.setKey(kPushNotificationsService);
    subscribeService.setValue(kPushNotificationsFCM);

    // Subscribe device
    QXmppDataForm::Field subscribeDevice;
    subscribeDevice.setKey(kPushNotificationsDeviceID);
    subscribeDevice.setValue(VSQPushNotifications::instance().token());

    // Create a Data Form
    QList<QXmppDataForm::Field> fields;
    fields << subscribeFormType << subscribeService << subscribeDevice;

    QXmppDataForm dataForm;
    dataForm.setType(QXmppDataForm::Submit);
    dataForm.setFields(fields);

    // Create request
    QXmppPushEnableIq xmppPush;
    xmppPush.setType(QXmppIq::Set);
    xmppPush.setMode(enable ? QXmppPushEnableIq::Enable : QXmppPushEnableIq::Disable);
    xmppPush.setJid(kPushNotificationsProxy);

    // xmppPush.setNode(kPushNotificationsNode);
    xmppPush.setNode(currentUser() + "@" + _xmppURL() + "/" + m_deviceId);
    xmppPush.setDataForm(dataForm);

    m_xmpp.sendPacket(xmppPush);
#else
    Q_UNUSED(enable)
#endif // VS_PUSHNOTIFICATIONS
}

/******************************************************************************/
void
VSQMessenger::onConnected() {
    onSubscribePushNotifications(true);
//#if !defined (Q_OS_ANDROID)
    _sendFailedMessages();
//#endif
    emit fireReady();
}

/******************************************************************************/
void
VSQMessenger::onReadyToUpload() {
//#if defined (Q_OS_ANDROID)
//    _sendFailedMessages();
//#endif
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
    else if (attachment->type == Attachment::Type::Picture) {
        mainObject.insert("type", "picture");
        payloadObject.insert("url", attachment->remoteUrl.toString());
        payloadObject.insert("displayName", attachment->displayName);
        payloadObject.insert("thumbnailUrl", attachment->remoteThumbnailUrl.toString());
        payloadObject.insert("bytesTotal", attachment->bytesTotal);
    }
    else {
        mainObject.insert("type", "file");
        payloadObject.insert("url", attachment->remoteUrl.toString());
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
    attachment.displayName = payload["displayName"].toString();
    attachment.bytesTotal = static_cast<qint64>(payload["bytesTotal"].toDouble());
    message.message = attachment.displayName;
    if (type == QLatin1String("picture")) {
        attachment.type = Attachment::Type::Picture;
        attachment.remoteThumbnailUrl = payload["thumbnailUrl"].toString();
    }
    else {
        attachment.type = Attachment::Type::File;
    }
    message.attachment = attachment;
    return message;
}

/******************************************************************************/
void
VSQMessenger::checkState() {
    if (m_xmpp.state() == QXmppClient::DisconnectedState) {
        qCDebug(lcNetwork) << "We should be connected, but it's not so. Let's try to reconnect.";
#if VS_ANDROID
        emit fireError(tr("Disconnected ..."));
#endif
        _reconnect();
    } else {
        qCDebug(lcNetwork) << "Connection is ok";
    }
}

/******************************************************************************/
void
VSQMessenger::_reconnect() {
    if (!m_user.isEmpty() && !m_userId.isEmpty() && vs_messenger_virgil_is_signed_in()) {
        QtConcurrent::run([=]() {
            _connect(m_user, m_deviceId, m_userId);
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
    VS_LOG_DEBUG("onError");
    qDebug() << "onError : " << err << "   state:" << m_xmpp.state();
    emit fireError(tr("Connection error ..."));
}

/******************************************************************************/
Optional<StMessage> VSQMessenger::decryptMessage(const QString &sender, const QString &message) {
    static const size_t _decryptedMsgSzMax = 10 * 1024;
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
        VS_LOG_WARNING("Received message cannot be decrypted");
        return NullOptional;
    }

    // Add Zero termination
    decryptedMessage[decryptedMessageSz] = 0;

    // Get message from JSON
    QByteArray baDecr(reinterpret_cast<char *> (decryptedMessage), static_cast<int> (decryptedMessageSz));
    QJsonDocument jsonMsg(QJsonDocument::fromJson(baDecr));

    auto msg = parseJson(jsonMsg);
    VS_LOG_DEBUG("Received message: <%s>", msg.message.toStdString().c_str());
    return msg;
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

    if (sender == currentUser()) {
        QString recipient = message.to().split("@").first();
        m_sqlConversations->createMessage(recipient, msg->message, message.id(), msg->attachment);
        m_sqlConversations->setMessageStatus(message.id(), StMessage::Status::MST_SENT);
        // ensure private chat with recipient exists
        m_sqlChatModel->createPrivateChat(recipient);
        emit fireNewMessage(sender, msg->message);
        return;
    }

    // Add sender to contact
    // m_sqlContacts->addContact(sender);
    m_sqlChatModel->createPrivateChat(sender);
    // Save message to DB
    m_sqlConversations->receiveMessage(message.id(), sender, msg->message, msg->attachment);
    m_sqlChatModel->updateLastMessage(sender, msg->message);
    if (sender != m_recipient) {
        m_sqlChatModel->updateUnreadMessageCount(sender);
    }

    if (msg->attachment) {
        auto &attachment = *msg->attachment;
        if (attachment.type == Attachment::Type::Picture) {
            qCDebug(lcTransferManager) << "Thumbnail downloading for message:" << message.id();
            if (attachment.thumbnailPath.isEmpty()) {
                attachment.thumbnailPath = m_attachmentBuilder.generateThumbnailFileName();
            }
            const auto id = message.id() + QLatin1String("-thumb");
            auto dl = m_transferManager->startCryptoDownload(id, attachment.remoteThumbnailUrl, attachment.thumbnailPath, sender);
            connect(dl, &VSQDownload::ended, [=]() {
                m_sqlConversations->setAttachmentThumbnailPath(message.id(), attachment.thumbnailPath);
            });
        }
    }

    // Inform system about new message
    emit fireNewMessage(sender, msg->message);
}

/******************************************************************************/

VSQMessenger::EnResult
VSQMessenger::_sendMessageInternal(bool createNew, const QString &messageId, const QString &to, const QString &message, const OptionalAttachment &attachment)
{
    QMutexLocker _guard(&m_messageGuard);
    static const size_t _encryptedMsgSzMax = 20 * 1024;
    uint8_t encryptedMessage[_encryptedMsgSzMax];
    size_t encryptedMessageSz = 0;

    // Create JSON-formatted message to be sent
    const QString internalJson = createJson(message, attachment);
    qDebug() << "json for encryption:" << internalJson;

    // Encrypt message
    auto plaintext = internalJson.toStdString();
    if (VS_CODE_OK != vs_messenger_virgil_encrypt_msg(
                     to.toStdString().c_str(),
                     reinterpret_cast<const uint8_t*>(plaintext.c_str()),
                     plaintext.length(),
                     encryptedMessage,
                     _encryptedMsgSzMax,
                     &encryptedMessageSz)) {
        VS_LOG_WARNING("Cannot encrypt message to be sent");
        return MRES_ERR_ENCRYPTION;
    }

    // Send encrypted message
    QString toJID = to + "@" + _xmppURL();
    QString fromJID = currentUser() + "@" + _xmppURL();
    QString encryptedStr = QString::fromLatin1(reinterpret_cast<char*>(encryptedMessage), encryptedMessageSz);

    QXmppMessage msg(fromJID, toJID, encryptedStr);
    msg.setReceiptRequested(true);
    msg.setId(messageId);

    if(createNew) {
        m_sqlConversations->createMessage(to, message, messageId, attachment);
    }
    m_sqlChatModel->updateLastMessage(to, message);

    if (m_xmpp.sendPacket(msg)) {
        m_sqlConversations->setMessageStatus(msg.id(), StMessage::Status::MST_SENT);
    } else {
        m_sqlConversations->setMessageStatus(msg.id(), StMessage::Status::MST_FAILED);
    }
    return MRES_OK;
}

void VSQMessenger::downloadAndProcess(StMessage msg, const Function &func)
{
    if (!msg.attachment) {
        qCDebug(lcTransferManager) << "Message has no attachment:" << msg.messageId;
        return;
    }
    auto &attachment = *msg.attachment;
    if (QFile::exists(attachment.filePath)) {
        func(msg);
        return;
    }
    // Update attachment filePath
    auto &filePath = attachment.filePath;
    if (filePath.isEmpty() || QFile::exists(filePath)) {
        filePath = VSQUtils::findUniqueFileName(m_settings->downloadsDir().filePath(attachment.displayName));
    }
    auto upload = m_transferManager->startCryptoDownload(msg.messageId, attachment.remoteUrl, filePath, msg.recipient);
    connect(m_transferManager, &VSQCryptoTransferManager::fileDownloadedAndDecrypted, upload, [=](const QString &id, const QString &filePath) {
        if (id == msg.messageId) {
            func(msg);
        }
    });
}

/******************************************************************************/
QFuture<VSQMessenger::EnResult>
VSQMessenger::createSendMessage(bool createNew, const QString &messageId, const QString &to, const QString &message) {
    return QtConcurrent::run([=]() -> EnResult {
        return _sendMessageInternal(createNew, messageId, to, message, NullOptional);
    });
}

QFuture<VSQMessenger::EnResult>
VSQMessenger::createSendAttachment(bool createNew, const QString &messageId, const QString &to,
                                   const QUrl &url, const Enums::AttachmentType attachmentType)
{
    return QtConcurrent::run([=]() -> EnResult {
        QString warningText;
        auto attachment = m_attachmentBuilder.build(url, attachmentType, messageId, to, warningText);
        if (!attachment) {
            qCWarning(lcAttachment) << warningText;
            fireWarning(warningText);
            return MRES_ERR_ATTACHMENT;
        }
        return _sendMessageInternal(createNew, messageId, to, attachment->displayName, attachment);
    });
}

/******************************************************************************/
QFuture<VSQMessenger::EnResult>
VSQMessenger::sendMessage(const QString &to, const QString &message,
                          const QVariant &attachmentUrl, const Enums::AttachmentType attachmentType)
{
    const auto url = attachmentUrl.toUrl();
    if (m_attachmentBuilder.isValidUrl(url)) {
        return createSendAttachment(true, VSQUtils::createUuid(), to, url, attachmentType);
    }
    else {
        return createSendMessage(true, VSQUtils::createUuid(), to, message);
    }
    QFuture<VSQMessenger::EnResult> future;
    return future;
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
}

void VSQMessenger::saveAttachmentAs(const QString &messageId, const QVariant &fileUrl)
{
    auto message = m_sqlConversations->getMessage(messageId);
    if (!message) {
        return;
    }
    const auto dest = fileUrl.toUrl().toLocalFile();
    qCDebug(lcTransferManager) << "Saving of attachment as" << dest;
    downloadAndProcess(*message, [=](const StMessage &msg) {
        qCDebug(lcTransferManager) << "Message attachment saved as:" << dest;
        QFile::copy(msg.attachment->filePath, dest);
    });
}

void VSQMessenger::downloadAttachment(const QString &messageId)
{
    auto message = m_sqlConversations->getMessage(messageId);
    if (!message) {
        return;
    }
    qCDebug(lcTransferManager) << "Downloading of attachment:" << messageId;
    downloadAndProcess(*message, [](const StMessage &msg) {
        qCDebug(lcTransferManager) << QString("Message '%1' attachment was downloaded to: %2").arg(msg.messageId, msg.attachment->filePath);
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
        const auto url = QUrl::fromLocalFile(msg.attachment->filePath).toString();
        qCDebug(lcTransferManager) << "Opening of message attachment:" << url;
        emit openUrlExternallyRequested(url);
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
