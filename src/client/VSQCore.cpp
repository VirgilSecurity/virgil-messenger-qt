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

#include "client/VSQCore.h"

#include <QJsonDocument>
#include <QJsonObject>

#include "VSQSettings.h"
#include "VSQUtils.h"
#include "android/VSQAndroid.h"

Q_LOGGING_CATEGORY(lcCore, "core")

using namespace VirgilIoTKit;

VSQCore::VSQCore(VSQSettings *settings)
    : m_settings(settings)
    , m_deviceId(VSQUtils::createUuid())
{}

VSQCore::~VSQCore()
{
#ifdef VS_DEVMODE
    qCDebug(lcDev) << "~Core";
#endif
}

bool VSQCore::signIn(const QString &userWithEnv)
{
    // Initialization
    if (!initialize())
        return false;

    setUser(userWithEnv);
    qCDebug(lcCore) << "Trying to Sign In:" << m_user;

    Credentials creds;
    memset(&creds, 0, sizeof (creds));
    if (!loadCredentials(m_user, creds)) {
        m_lastErrorText = QLatin1String("Cannot load user credentials");
        return false;
    }

    // Sign In user, using Virgil Service
    if (VS_CODE_OK != vs_messenger_virgil_sign_in(&creds)) {
        m_lastErrorText = QLatin1String("Cannot Sign In user");
        return false;
    }

    return true;
}

void VSQCore::signOut()
{
    m_user.clear();
    m_envType = EnvironmentType::DEFAULT;
    m_xmppPassword.clear();
    vs_messenger_virgil_logout();
}

bool VSQCore::signUp(const QString &userWithEnv)
{
    if (userWithEnv.isEmpty()) {
        m_lastErrorText = QLatin1String("Empty username");
        return false;
    }
    setUser(userWithEnv);
    qInfo() << "Trying to sign up:" << userWithEnv;
    if (m_user.isEmpty()) {
        m_lastErrorText = QLatin1String("Incorrect username");
        return false;
    }

    // HACK(fpohtmeh): doesn't work without init
    if (!initialize())
        return false;

    Credentials creds;
    memset(&creds, 0, sizeof (creds));

    const auto status = vs_messenger_virgil_sign_up(m_user.toStdString().c_str(), &creds);
    if (status != VS_CODE_OK) {
        m_lastErrorText = QLatin1String("Username is already taken");
        return false;
    }

    qInfo() << "User has been successfully signed up:" << userWithEnv;
    saveCredentials(m_user, creds);
    return true;
}

bool VSQCore::backupKey(const QString &password)
{
    // Upload current user key to the cloud
    if (VS_CODE_OK != vs_messenger_virgil_set_sign_in_password(password.toStdString().c_str())) {
        m_lastErrorText = QLatin1String("Cannot set sign in password");
        return false;
    }
    return true;
}

bool VSQCore::signInWithKey(const QString &userWithEnv, const QString &password)
{
    setUser(userWithEnv);
    Credentials creds;
    memset(&creds, 0, sizeof (creds));
    // Download private key from the cloud
    if (VS_CODE_OK != vs_messenger_virgil_sign_in_with_password(m_user.toStdString().c_str(), password.toStdString().c_str(), &creds)) {
        m_lastErrorText = QLatin1String("Cannot set sign in password");
        return false;
    }
    // Save credentials
    saveCredentials(m_user, creds);
    return true;
}

Optional<QString> VSQCore::encryptMessageBody(const QString &contact, const QString &body)
{
    static const size_t _encryptedMsgSzMax = 20 * 1024;
    uint8_t encryptedMessage[_encryptedMsgSzMax];
    size_t encryptedMessageSz = 0;

    // Create JSON-formatted message to be sent
    QJsonObject payloadObject;
    payloadObject.insert("body", body);

    QJsonObject mainObject;
    mainObject.insert("type", "text");
    mainObject.insert("payload", payloadObject);

    QJsonDocument doc(mainObject);
    QString internalJson = doc.toJson(QJsonDocument::Compact);
    qCDebug(lcCore) << internalJson;

    // Encrypt message
    if (VS_CODE_OK != vs_messenger_virgil_encrypt_msg(
                     contact.toStdString().c_str(),
                     internalJson.toStdString().c_str(),
                     encryptedMessage,
                     _encryptedMsgSzMax,
                     &encryptedMessageSz)) {
        m_lastErrorText = QLatin1String("Cannot encrypt message to be sent");
        qCWarning(lcCore) << m_lastErrorText;
        return NullOptional;
    }
    return QString::fromLatin1(reinterpret_cast<char*>(encryptedMessage));
}

Optional<QString> VSQCore::decryptMessageBody(const QString &contact, const QString &encrypedBody)
{
    static const size_t decryptedBodyMaxSize = 10 * 1024;
    uint8_t decryptedBody[decryptedBodyMaxSize];
    size_t decryptedBodySize = 0;

    qCDebug(lcCore) << "Sender         :" << contact;
    qCDebug(lcCore) << "Encrypted body :" << encrypedBody;

    // Decrypt message
    // DECRYPTED_MESSAGE_SZ_MAX - 1  - This is required for a Zero-terminated string
    if (VS_CODE_OK !=
            vs_messenger_virgil_decrypt_msg(
                contact.toStdString().c_str(),
                encrypedBody.toStdString().c_str(),
                decryptedBody, decryptedBodySize - 1,
                &decryptedBodySize)) {
        m_lastErrorText = QLatin1String("Received message cannot be decrypted");
        qCWarning(lcCore) << m_lastErrorText;
        return NullOptional;
    }

    // Add Zero termination
    decryptedBody[decryptedBodySize] = 0;

    // Get message from JSON
    QByteArray baDecr(reinterpret_cast<char *> (decryptedBody), static_cast<int> (decryptedBodySize));
    QJsonDocument jsonMsg(QJsonDocument::fromJson(baDecr));
    QString decryptedString = jsonMsg["payload"]["body"].toString();

    qCWarning(lcCore) << "Received message:" << decryptedString;
    return decryptedString;
}

bool VSQCore::userExists(const QString &user) const
{
    if (VS_CODE_OK != vs_messenger_virgil_search(user.toStdString().c_str()))
        return false;
    return true;
}

QString VSQCore::user() const
{
    return m_user;
}

bool VSQCore::isSignedIn() const
{
    return vs_messenger_virgil_is_signed_in();
}

QString VSQCore::lastErrorText() const
{
    return m_lastErrorText;
}

QString VSQCore::xmppJID() const
{
    return m_user + QLatin1Char('@') + xmppURL() + QLatin1Char('/') + m_deviceId;
}

QString VSQCore::xmppURL() const
{
    QString res = qgetenv("VS_MSGR_XMPP_URL");
    if (res.isEmpty()) {
        switch (m_envType) {
        case EnvironmentType::PROD:
            res = "xmpp.virgilsecurity.com";
            break;
        case EnvironmentType::STG:
            res = "xmpp-stg.virgilsecurity.com";
            break;
        case EnvironmentType::DEV:
            res = "xmpp-dev.virgilsecurity.com";
            break;
        }
    }
    qCDebug(lcCore) << "XMPP URL:" << res;
    return res;
}

Optional<QString> VSQCore::xmppPassword()
{
    if (!m_xmppPassword.isEmpty())
        return m_xmppPassword;

    const size_t _pass_buf_sz = 512;
    char pass[_pass_buf_sz];
    // Get XMPP password
    if (VS_CODE_OK != vs_messenger_virgil_get_xmpp_pass(pass, _pass_buf_sz)) {
        m_lastErrorText = QLatin1String("Cannot get XMPP password");
        return NullOptional;
    }
    m_xmppPassword = QString::fromLatin1(pass);
    return m_xmppPassword;
}

uint16_t VSQCore::xmppPort() const
{
    uint16_t res = 5222;
    QString portStr = qgetenv("VS_MSGR_XMPP_PORT");
    if (!portStr.isEmpty()) {
        bool ok;
        int port = portStr.toInt(&ok);
        if (ok) {
            res = static_cast<uint16_t> (port);
        }
    }
    qCDebug(lcCore) << "XMPP PORT:" << res;
    return res;
}

bool VSQCore::initialize()
{
    if (vs_messenger_virgil_is_init())
        return true;

    char *cCABundle = strdup(caBundleFile().toStdString().c_str());
    if (VS_CODE_OK != vs_messenger_virgil_init(virgilURL().toStdString().c_str(), cCABundle)) {
        m_lastErrorText = QLatin1String("Cannot initialize low level messenger");
        return false;
    }
    free(cCABundle);
    return true;
}

void VSQCore::setUser(const QString &userWithEnv)
{
    const UserEnv userEnv = parseUserWithEnv(userWithEnv);
    m_user = userEnv.first;
    m_envType = userEnv.second;
}

VSQCore::UserEnv VSQCore::parseUserWithEnv(const QString &userWithEnv) const
{
    UserEnv userEnv;
    const QStringList pieces = userWithEnv.split("@");
    if (pieces.size() == 1) {
        userEnv.first = userWithEnv;
        userEnv.second = EnvironmentType::DEFAULT;
    }
    else {
        userEnv.first = pieces.back();
        const QMap<QString, EnvironmentType> types {
            { QLatin1String("prod"), EnvironmentType::PROD },
            { QLatin1String("stg"), EnvironmentType::STG },
            { QLatin1String("dev"), EnvironmentType::DEV }
        };
        userEnv.second = types.value(pieces.front(), EnvironmentType::DEFAULT);
    }
    return userEnv;
}

bool VSQCore::loadCredentials(const QString &user, Credentials &creds)
{
    const auto settingsJson = m_settings->userCredential(user).toUtf8();
    const auto json = QJsonDocument::fromJson(settingsJson);
    const auto deviceId = json["device_id"].toString();
    const auto baCred = QByteArray::fromBase64(json["creds"].toString().toUtf8());
    if (baCred.size() != sizeof(creds)) {
        qCWarning(lcCore) << "Cannot load credentials for:" << user;
        return false;
    }
    m_deviceId = deviceId;
    memcpy(&creds, baCred.data(), static_cast<size_t> (baCred.size()));
    return true;
}

void VSQCore::saveCredentials(const QString &user, const Credentials &creds)
{
    // TODO(fpohtmeh): save one device id for all users
    // TODO: Use SecBox
    QByteArray baCred(reinterpret_cast<const char*>(&creds), sizeof(creds));
    QJsonObject jsonObject;
    jsonObject.insert("device_id", m_deviceId);
    jsonObject.insert("creds", QJsonValue::fromVariant(baCred.toBase64()));

    QString json = QJsonDocument(jsonObject).toJson(QJsonDocument::Compact);
    qCInfo(lcCore) << "Saving user credentails:" << json;
    m_settings->setUserCredential(user, json);
}

QString VSQCore::caBundleFile() const
{
#if VS_ANDROID
    return VSQAndroid::caBundlePath();
#else
    return qgetenv("VS_CURL_CA_BUNDLE");
#endif
}

QString VSQCore::virgilURL() const
{
    QString res = qgetenv("VS_MSGR_VIRGIL");
    if (res.isEmpty()) {
        switch (m_envType) {
        case EnvironmentType::PROD:
            res = "https://messenger.virgilsecurity.com";
            break;
        case EnvironmentType::STG:
            res = "https://messenger-stg.virgilsecurity.com";
            break;
        case EnvironmentType::DEV:
            res = "https://messenger-dev.virgilsecurity.com";
            break;
        }
    }
    qCDebug(lcCore) << "Virgil URL:" << res;
    return res;
}
