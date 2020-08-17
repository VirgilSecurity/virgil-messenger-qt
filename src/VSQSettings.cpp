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
    : QObject(parent)
{
    m_attachmentCacheDir.setPath(QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + QLatin1String("/attachments"));
    m_thumbnaisDir.setPath(QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + QLatin1String("/thumbnails"));
    m_downloadsDir.setPath(QStandardPaths::writableLocation(QStandardPaths::DownloadLocation) + QLatin1String("/Virgil"));
    for (auto dir : { m_attachmentCacheDir, m_thumbnaisDir, m_downloadsDir }) {
        if (!dir.exists() && !dir.mkpath(dir.absolutePath())) {
            qFatal("Failed to create writable directory at %s", qPrintable(dir.absolutePath()));
        }
    }

    qCDebug(lcSettings) << "Settings";
    qCDebug(lcSettings) << "Attachment cache dir:" << attachmentCacheDir().absolutePath();
    qCDebug(lcSettings) << "Attachment max file size:" << attachmentMaxFileSize();
    qCDebug(lcSettings) << "Thumbnails dir:" << thumbnailsDir().absolutePath();
    qCDebug(lcSettings) << "Thumbnail max size:" << attachmentMaxFileSize();
    qCDebug(lcSettings) << "Downloads dir:" << downloadsDir().absolutePath();
    if (devMode()) {
        qCDebug(lcSettings) << "Dev mode:" << true;
    }
}

VSQSettings::~VSQSettings()
{
#ifdef VS_DEVMODE
    qCDebug(lcDev) << "~Settings";
#endif
}

DataSize VSQSettings::attachmentMaxFileSize() const
{
    return 25 * 1024 * 1024;
}

QDir VSQSettings::attachmentCacheDir() const
{
    return m_attachmentCacheDir;
}

QDir VSQSettings::thumbnailsDir() const
{
    return m_thumbnaisDir;
}

QDir VSQSettings::downloadsDir() const
{
    return m_downloadsDir;
}

QSize VSQSettings::thumbnailMaxSize() const
{
    return QSize(100, 80);
}

bool VSQSettings::devMode() const
{
#ifdef VS_DEVMODE
    return true;
#else
    return false;
#endif // VS_DEVMODE
}
