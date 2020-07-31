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

#ifndef VSQ_CORE_H
#define VSQ_CORE_H

#include "VSQCommon.h"

#include <virgil/iot/messenger/messenger.h>

class VSQSettings;

Q_DECLARE_LOGGING_CATEGORY(lcCore)

class VSQCore
{
public:
    explicit VSQCore(VSQSettings *settings);
    ~VSQCore();

    bool signIn(const QString &userWithEnv);
    void signOut();
    bool signUp(const QString &userWithEnv);
    bool backupKey(const QString &password);
    bool signInWithKey(const QString &userWithEnv, const QString &password);

    Optional<QString> encryptMessageBody(const QString &contact, const QString &body);
    Optional<QString> decryptMessageBody(const QString &contact, const QString &encrypedBody);

    bool userExists(const QString &user) const;

    QString user() const;
    bool isSignedIn() const;
    QString lastErrorText() const;

    QString xmppJID() const;
    QString xmppURL() const;
    Optional<QString> xmppPassword();
    uint16_t xmppPort() const;
    QString virgilURL() const;

private:
    enum class EnvironmentType
    {
        PROD,
        STG,
        DEV,
        DEFAULT = PROD
    };
    using UserEnv = std::pair<QString, EnvironmentType>;
    using Credentials = VirgilIoTKit::vs_messenger_virgil_user_creds_t;

    bool initialize();
    void setUser(const QString &userWithEnv);
    UserEnv parseUserWithEnv(const QString &userWithEnv) const;

    bool loadCredentials(const QString &user, Credentials &creds);
    void saveCredentials(const QString &user, const Credentials &creds);

    QString caBundleFile() const;

    VSQSettings *m_settings;
    QString m_user;
    EnvironmentType m_envType = EnvironmentType::DEFAULT;
    QString m_deviceId;
    QString m_lastErrorText;
    QString m_xmppPassword;
};

#endif // VSQ_CORE_H
