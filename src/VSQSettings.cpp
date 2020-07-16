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

#include <QDebug>
#include <QStandardPaths>

static const QString kUsers = "Users";

VSQSettings::VSQSettings(QObject *parent)
    : QSettings("VirgilSecurity", "VirgilMessenger", parent) // organization, application name
{
    m_appDataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    m_attachmentCacheDir = QDir(QStandardPaths::writableLocation(QStandardPaths::CacheLocation)).filePath("attachments");
    for (auto dir : { m_appDataDir, m_attachmentCacheDir })
        if (!dir.exists() && !dir.mkpath(dir.absolutePath()))
            qFatal("Failed to create writable directory at %s", qPrintable(dir.absolutePath()));

    qDebug() << "Settings";
    qDebug() << "- settings filename:" << fileName();
    qDebug() << "- database filename:" << databaseFileName();
    qDebug() << "- attachment cache dir:" << attachmentCacheDir().absolutePath();
    qDebug() << "- attachment max size:" << attachmentMaxSize();
}

VSQSettings::~VSQSettings()
{}

void VSQSettings::setUsersList(const QStringList &users)
{
    setValue(kUsers, users);
    sync();
}

QStringList VSQSettings::usersList() const
{
    return value(kUsers, QStringList()).toStringList();
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
