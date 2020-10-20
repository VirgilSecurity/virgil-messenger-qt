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

#include "VSQCustomer.h"
#include "VSQUtils.h"

static const QString kUsersGroup = "Users";
static const QString kUsersList = "UsersList";
static const QString kCredenitalsGroup = "Credentials";

static const QString kDeviceId = "DeviceId";

static const QString kLastSessionGroup = "LastSession";
static const QString kWindowGeometryId = "WindowGeometry";
static const QString kSessionId = "SessionId";
static const QString kSignedInUserId = "SignedInUserId";

Q_LOGGING_CATEGORY(lcSettings, "settings")

VSQSettings::VSQSettings(QObject *parent)
    : QSettings(Customer::OrganizationName, Customer::ApplicationName, parent)
    , m_sessionId(VSQUtils::createUuid())
{
    if (deviceId().isEmpty()) {
        createDeviceId();
        removeGroup(kUsersGroup);
        removeGroup(kCredenitalsGroup);
    }

    m_attachmentCacheDir.setPath(QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + QLatin1String("/attachments"));
    m_thumbnaisDir.setPath(QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + QLatin1String("/thumbnails"));
    m_downloadsDir.setPath(QStandardPaths::writableLocation(QStandardPaths::DownloadLocation));
}

VSQSettings::~VSQSettings()
{
#ifdef VS_DEVMODE
    qCDebug(lcDev) << "~Settings";
#endif
}

void VSQSettings::print()
{
    qCDebug(lcSettings) << "Settings";
    qCDebug(lcSettings) << "Device id:" << deviceId();
    qCDebug(lcSettings) << "Attachment cache dir:" << attachmentCacheDir().absolutePath();
    qCDebug(lcSettings) << "Attachment max file size:" << attachmentMaxFileSize();
    qCDebug(lcSettings) << "Thumbnails dir:" << thumbnailsDir().absolutePath();
    qCDebug(lcSettings) << "Thumbnail max size:" << attachmentMaxFileSize();
    qCDebug(lcSettings) << "Downloads dir:" << downloadsDir().absolutePath();
    if (devMode()) {
        qCDebug(lcSettings) << "Dev mode:" << true;
    }
}

void VSQSettings::setLastSignedInUserId(const QString &userId)
{
    if (lastSignedInUserId() == userId) {
        return;
    }
    setGroupValue(kLastSessionGroup, kSignedInUserId, userId);
    sync();
    emit lastSignedInUserIdChanged(userId);
}

QString VSQSettings::lastSignedInUserId() const
{
    return groupValue(kLastSessionGroup, kSignedInUserId).toString();
}

void VSQSettings::setUsersList(const QStringList &users)
{
    if (usersList() == users) {
        return;
    }
    setGroupValue(kUsersGroup, kUsersList, users);
    sync();
    emit usersListChanged(users);
}

QStringList VSQSettings::usersList() const
{
    return groupValue(kUsersGroup, kUsersList).toStringList();
}

void VSQSettings::addUserToList(const QString &user)
{
    auto users = usersList();
    if (!users.contains(user)) {
        setUsersList(users << user);
    }
}

QString VSQSettings::userCredential(const QString &user) const
{
    return groupValue(kCredenitalsGroup, user).toString();
}

void VSQSettings::setUserCredential(const QString &user, const QString &credential)
{
    setGroupValue(kCredenitalsGroup, user, credential);
    sync();
}

QString VSQSettings::deviceId() const
{
    return value(kDeviceId).toString();
}

bool VSQSettings::runFlag() const
{
    auto lastSessionId = groupValue(kLastSessionGroup, kSessionId).toString();
    return !lastSessionId.isEmpty() && lastSessionId != m_sessionId;
}

void VSQSettings::setRunFlag(bool run)
{
    if (run) {
        qCDebug(lcSettings) << "Save session id" << m_sessionId;
        setGroupValue(kLastSessionGroup, kSessionId, m_sessionId);
    }
    else {
        qCDebug(lcSettings) << "Reset session id";
        removeGroupKey(kLastSessionGroup, kSessionId);
    }
}

DataSize VSQSettings::attachmentMaxFileSize() const
{
    return 25 * 1024 * 1024;
}

QDir VSQSettings::attachmentCacheDir() const
{
    if(!m_attachmentCacheDir.exists()) {
        VSQUtils::forceCreateDir(m_attachmentCacheDir.absolutePath());
    }
    return m_attachmentCacheDir;
}

QDir VSQSettings::thumbnailsDir() const
{
    if(!m_thumbnaisDir.exists()) {
        VSQUtils::forceCreateDir(m_thumbnaisDir.absolutePath());
    }
    return m_thumbnaisDir;
}

QDir VSQSettings::downloadsDir() const
{
    if(!m_downloadsDir.exists()) {
        VSQUtils::forceCreateDir(m_downloadsDir.absolutePath());
    }
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

QRect VSQSettings::windowGeometry() const
{
    return groupValue(kLastSessionGroup, kWindowGeometryId).toRect();
}

void VSQSettings::setWindowGeometry(const QRect &geometry)
{
    setGroupValue(kLastSessionGroup, kWindowGeometryId, geometry);
    sync();
}

Seconds VSQSettings::lastSeenActivityInterval() const
{
    return 5;
}

QString VSQSettings::makeGroupKey(const QString &group, const QString &key) const
{
    return group + '/' + key;
}

void VSQSettings::setGroupValue(const QString &group, const QString &key, const QVariant &value)
{
    setValue(makeGroupKey(group, key), value);
    sync();
}

QVariant VSQSettings::groupValue(const QString &group, const QString &key, const QVariant &defaultValue) const
{
    return value(makeGroupKey(group, key), defaultValue);
}

void VSQSettings::removeGroupKey(const QString &group, const QString &key)
{
    remove(makeGroupKey(group, key));
}

void VSQSettings::removeGroup(const QString &group)
{
    beginGroup(group);
    remove(QString());
    endGroup();
}

void VSQSettings::createDeviceId()
{
    setValue(kDeviceId, VSQUtils::createUuid());
    sync();
}
