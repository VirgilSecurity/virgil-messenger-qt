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

#include "VSQSettings.h"

#include <QStandardPaths>

#include "VSQUtils.h"

static const QString kLastSignedInUser = "LastSignedInUser";
static const QString kUsers = "Users";
static const QString kSavedSessionId = "SavedSessionId";

Q_LOGGING_CATEGORY(lcSettings, "settings")

VSQSettings::VSQSettings(QObject *parent)
    : QSettings("VirgilSecurity", "VirgilMessenger", parent) // organization, application name
{
    m_sessionId = VSQUtils::createUuid();
    m_appDataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    m_attachmentCacheDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + QLatin1String("/attachments");
    for (auto dir : { m_appDataDir, m_attachmentCacheDir })
        if (!dir.exists() && !dir.mkpath(dir.absolutePath()))
            qFatal("Failed to create writable directory at %s", qPrintable(dir.absolutePath()));

    qCDebug(lcSettings) << "Settings";
    qCDebug(lcSettings) << "- session id:" << m_sessionId;
    qCDebug(lcSettings) << "- settings filename:" << fileName();
    qCDebug(lcSettings) << "- database filename:" << databaseFileName();
    qCDebug(lcSettings) << "- last signed-in user:" << lastSignedInUser();
    qCDebug(lcSettings) << "- attachment cache dir:" << attachmentCacheDir().absolutePath();
    qCDebug(lcSettings) << "- attachment max size:" << attachmentMaxSize();
}

VSQSettings::~VSQSettings()
{
#ifdef VS_DEVMODE
    qCDebug(lcDev) << "~Settings";
#endif
}

void VSQSettings::setLastSignedInUser(const QString &user)
{
    if (lastSignedInUser() == user)
        return;
    setValue(kLastSignedInUser, user);
    sync();
    emit lastSignedInUserChanged(user);
}

QString VSQSettings::lastSignedInUser() const
{
    return value(kLastSignedInUser).toString();
}

void VSQSettings::setUsersList(const QStringList &users)
{
    if (usersList() == users)
        return;
    setValue(kUsers, users);
    sync();
    emit usersListChanged(users);
}

QStringList VSQSettings::usersList() const
{
    return value(kUsers, QStringList()).toStringList();
}

void VSQSettings::addUserToList(const QString &user)
{
    auto users = usersList();
    if (!users.contains(user))
        setUsersList(users << user);
}

QString VSQSettings::userCredential(const QString &user) const
{
    return value(user, QString()).toString();
}

void VSQSettings::setUserCredential(const QString &user, const QString &credential)
{
    setValue(user, credential);
    sync();
}

QString VSQSettings::databaseFileName() const
{
    return m_appDataDir.filePath("chat-database.sqlite3");
}

int VSQSettings::attachmentMaxSize() const
{
    return 50 * 1024 * 1024;
}

QDir VSQSettings::attachmentCacheDir() const
{
    return m_attachmentCacheDir;
}

QSize VSQSettings::previewMaxSize() const
{
    return QSize(600, 400);
}

bool VSQSettings::devMode() const
{
#ifdef VS_DEVMODE
    return true;
#else
    return false;
#endif // VS_DEVMODE
}

bool VSQSettings::runFlag() const
{
    auto savedSessionId = value(kSavedSessionId).toString();
    return !savedSessionId.isEmpty() && savedSessionId != m_sessionId;
}

void VSQSettings::setRunFlag(bool flag)
{
    if (flag) {
        qCDebug(lcSettings) << "Save session id" << m_sessionId;
        setValue(kSavedSessionId, m_sessionId);
    }
    else {
        qCDebug(lcSettings) << "Reset session id";
        remove(kSavedSessionId);
    }
}
