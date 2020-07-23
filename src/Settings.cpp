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

#include "Settings.h"

#include <QStandardPaths>

static const QString kLastSignedInUser = "LastSignedInUser";
static const QString kUsers = "Users";

Settings::Settings(QObject *parent)
    : QSettings("VirgilSecurity", "VirgilMessenger", parent) // organization, application name
{
    m_appDataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    m_attachmentCacheDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + QLatin1String("/attachments");
    for (auto dir : { m_appDataDir, m_attachmentCacheDir })
        if (!dir.exists() && !dir.mkpath(dir.absolutePath()))
            qFatal("Failed to create writable directory at %s", qPrintable(dir.absolutePath()));

    qDebug() << "Settings";
    qDebug() << "- settings filename:" << fileName();
    qDebug() << "- database filename:" << databaseFileName();
    qDebug() << "- last signed-in user:" << lastSignedInUser();
    qDebug() << "- attachment cache dir:" << attachmentCacheDir().absolutePath();
    qDebug() << "- attachment max size:" << attachmentMaxSize();
}

Settings::~Settings()
{}

void Settings::setLastSignedInUser(const QString &user)
{
    if (lastSignedInUser() == user)
        return;
    setValue(kLastSignedInUser, user);
    sync();
    emit lastSignedInUserChanged(user);
}

QString Settings::lastSignedInUser() const
{
    return value(kLastSignedInUser).toString();
}

void Settings::setUsersList(const QStringList &users)
{
    if (usersList() == users)
        return;
    setValue(kUsers, users);
    sync();
    emit usersListChanged(users);
}

QStringList Settings::usersList() const
{
    return value(kUsers, QStringList()).toStringList();
}

void Settings::addUserToList(const QString &user)
{
    auto users = usersList();
    if (!users.contains(user))
        setUsersList(users << user);
}

QByteArray Settings::userCredential(const QString &user) const
{
    const auto credBase64 = value(user, QString()).toString();
    return QByteArray::fromBase64(credBase64.toUtf8());
}

void Settings::setUserCredential(const QString &user, const QByteArray &credential)
{
    setValue(user, credential);
    sync();
}

QString Settings::databaseFileName() const
{
    return m_appDataDir.filePath("chat-database.sqlite3");
}

int Settings::attachmentMaxSize() const
{
    return 50 * 1024 * 1024;
}

QDir Settings::attachmentCacheDir() const
{
    return m_attachmentCacheDir;
}

QSize Settings::previewMaxSize() const
{
    return QSize(600, 400);
}
