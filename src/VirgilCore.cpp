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

#include "VirgilCore.h"

#include <QJsonDocument>
#include <QJsonObject>

#include <QXmppMessage.h>

#include "Settings.h"
#include "android/VSQAndroid.h"

using namespace VirgilIoTKit;

VirgilCore::VirgilCore(Settings *settings)
    : m_settings(settings)
{}

bool VirgilCore::signIn(const QString &userWithEnv)
{
    // Initialization
    if (!init())
        return false;

    setUser(userWithEnv);
    qDebug() << "Trying to Sign In: " << m_user;

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

void VirgilCore::signOut()
{
    m_user.clear();
    m_envType = EnvironmentType::DEFAULT;
    m_xmppPassword.clear();
    vs_messenger_virgil_logout();
}

bool VirgilCore::signUp(const QString &userWithEnv)
{
    if (userWithEnv.isEmpty()) {
        m_lastErrorText = QLatin1String("Empty username");
        return false;
    }
    setUser(userWithEnv);
    qInfo() << "Trying to sign up: " << userWithEnv;
    if (m_user.isEmpty()) {
        m_lastErrorText = QLatin1String("Incorrect username");
        return false;
    }

    // HACK(fpohtmeh): doesn't work without init
    if (!init())
        return false;

    Credentials creds;
    memset(&creds, 0, sizeof (creds));

    const auto status = vs_messenger_virgil_sign_up(m_user.toStdString().c_str(), &creds);
    if (status != VS_CODE_OK) {
        m_lastErrorText = QLatin1String("Username is already taken");
        return false;
    }

    qInfo() << "User has been successfully signed up: " << userWithEnv;
    saveCredentials(m_user, creds);
    return true;
}

bool VirgilCore::backupKey(const QString &password)
{
    // Upload current user key to the cloud
    if (VS_CODE_OK != vs_messenger_virgil_set_sign_in_password(password.toStdString().c_str())) {
        m_lastErrorText = QLatin1String("Cannot set sign in password");
        return false;
    }
    return true;
}

bool VirgilCore::signInWithKey(const QString &userWithEnv, const QString &password)
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

Optional<QXmppMessage> VirgilCore::encryptMessage(const StMessage &message)
{
    static const size_t _encryptedMsgSzMax = 20 * 1024;
    uint8_t encryptedMessage[_encryptedMsgSzMax];
    size_t encryptedMessageSz = 0;

    // Create JSON-formatted message to be sent
    QJsonObject payloadObject;
    payloadObject.insert("body", message.message);

    QJsonObject mainObject;
    mainObject.insert("type", "text");
    mainObject.insert("payload", payloadObject);

    QJsonDocument doc(mainObject);
    QString internalJson = doc.toJson(QJsonDocument::Compact);
    qDebug() << internalJson;

    // Encrypt message
    if (VS_CODE_OK != vs_messenger_virgil_encrypt_msg(
                     message.recipient.toStdString().c_str(),
                     internalJson.toStdString().c_str(),
                     encryptedMessage,
                     _encryptedMsgSzMax,
                     &encryptedMessageSz)) {
        VS_LOG_WARNING("Cannot encrypt message to be sent");
        m_lastErrorText = QLatin1String("Cannot encrypt message to be sent");
        return NullOptional;
    }

    // Send encrypted message
    QString toJID = message.recipient + "@" + xmppURL();
    QString fromJID = m_user + "@" + xmppURL();
    QString encryptedStr = QString::fromLatin1(reinterpret_cast<char*>(encryptedMessage));

    QXmppMessage msg(fromJID, toJID, encryptedStr);
    msg.setReceiptRequested(true);
    msg.setId(message.id);
    return msg;
}

Optional<QString> VirgilCore::decryptMessage(const QString &sender, const QString &message)
{
    static const size_t _decryptedMsgSzMax = 10 * 1024;
    uint8_t decryptedMessage[_decryptedMsgSzMax];
    size_t decryptedMessageSz = 0;

    qDebug() << "Sender            : " << sender;
    qDebug() << "Encrypted message : " << message;

    // Decrypt message
    // DECRYPTED_MESSAGE_SZ_MAX - 1  - This is required for a Zero-terminated string
    if (VS_CODE_OK !=
            vs_messenger_virgil_decrypt_msg(
                sender.toStdString().c_str(),
                message.toStdString().c_str(),
                decryptedMessage, decryptedMessageSz - 1,
                &decryptedMessageSz)) {
        VS_LOG_WARNING("Received message cannot be decrypted");
        m_lastErrorText = QLatin1String("Received message cannot be decrypted");
        return NullOptional;
    }

    // Add Zero termination
    decryptedMessage[decryptedMessageSz] = 0;

    // Get message from JSON
    QByteArray baDecr(reinterpret_cast<char *> (decryptedMessage), static_cast<int> (decryptedMessageSz));
    QJsonDocument jsonMsg(QJsonDocument::fromJson(baDecr));
    QString decryptedString = jsonMsg["payload"]["body"].toString();

    VS_LOG_DEBUG("Received message: <%s>", decryptedString.toStdString().c_str());
    return decryptedString;
}

bool VirgilCore::userExists(const QString &user) const
{
    if (VS_CODE_OK != vs_messenger_virgil_search(user.toStdString().c_str()))
        return false;
    return true;
}

QString VirgilCore::user() const
{
    return m_user;
}

bool VirgilCore::isSignedIn() const
{
    return vs_messenger_virgil_is_signed_in();
}

QString VirgilCore::lastErrorText() const
{
    return m_lastErrorText;
}

QString VirgilCore::xmppJID() const
{
    return m_user + QLatin1Char('@') + xmppURL();
}

QString VirgilCore::xmppURL() const
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
    VS_LOG_DEBUG("XMPP URL: %s", res.toStdString().c_str());
    return res;
}

Optional<QString> VirgilCore::xmppPassword()
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

uint16_t VirgilCore::xmppPort() const
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
    VS_LOG_DEBUG("XMPP PORT: %d", static_cast<int> (res));
    return res;
}

bool VirgilCore::init()
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

void VirgilCore::setUser(const QString &userWithEnv)
{
    const UserEnv userEnv = parseUserWithEnv(userWithEnv);
    m_user = userEnv.first;
    m_envType = userEnv.second;
}

VirgilCore::UserEnv VirgilCore::parseUserWithEnv(const QString &userWithEnv) const
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

bool VirgilCore::loadCredentials(const QString &user, Credentials &creds)
{
    const auto baCred = m_settings->userCredential(user);
    if (baCred.size() != sizeof(creds)) {
        VS_LOG_WARNING("Cannot load credentials for : %s", user.toStdString().c_str());
        return false;
    }
    memcpy(&creds, baCred.data(), static_cast<size_t> (baCred.size()));
    return true;
}

void VirgilCore::saveCredentials(const QString &user, const VirgilCore::Credentials &creds)
{
    // TODO: Use SecBox
    QByteArray baCred(reinterpret_cast<const char*>(&creds), sizeof(creds));
    m_settings->setUserCredential(user, baCred.toBase64());
}

QString VirgilCore::caBundleFile() const
{
#if VS_ANDROID
    return VSQAndroid::caBundlePath();
#else
    return qgetenv("VS_CURL_CA_BUNDLE");
#endif
}

QString VirgilCore::virgilURL() const
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
    VS_LOG_DEBUG("Virgil URL: %s", res.toStdString().c_str());
    return res;
}
