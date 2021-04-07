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

#include "CustomerEnv.h"
#include "VSQCustomer.h"
#include "Utils.h"
#include "FileUtils.h"

#include <QStandardPaths>
#include <QLoggingCategory>

static const QString kUsersGroup = "Users";
static const QString kUsersList = "UsersList";
static const QString kCredenitalsGroup = "Credentials";
static const QString kUsersInfoGroup = "UsersInfo";
static const QString kChatsLastSyncDateGroup = "ChatsLastSyncDateGroup";

static const QString kDeviceId = "DeviceId";

static const QString kLastSessionGroup = "LastSession";
static const QString kWindowGeometryId = "WindowGeometry";
static const QString kSessionId = "SessionId";
static const QString kSignedInUsername = "SignedInUsername";

using namespace vm;

QDir Settings::m_logsDir = QDir();
bool Settings::m_logsDirInitialized = false;

Q_LOGGING_CATEGORY(lcSettings, "settings")

Settings::Settings(QObject *parent)
    : QSettings(settingsFileName(), settingsFormat(), parent), m_sessionId(Utils::createUuid())
{

    qCDebug(lcSettings) << "Settings are written to: " << fileName();

    if (deviceId().isEmpty()) {
        createDeviceId();
        removeGroup(kUsersGroup);
        removeGroup(kCredenitalsGroup);
    }

    m_databaseDir.setPath(CustomerEnv::appDataLocation().filePath(QLatin1String("database")));
    const QDir cacheDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    m_attachmentCacheDir.setPath(cacheDir.filePath(QLatin1String("attachments")));
    m_thumbnaisDir.setPath(cacheDir.filePath(QLatin1String("thumbnails")));
    m_downloadsDir.setPath(QStandardPaths::writableLocation(QStandardPaths::DownloadLocation));
    m_cloudFilesCacheDir.setPath(cacheDir.filePath(QLatin1String("cloudFiles")));
}

Settings::~Settings() { }

void Settings::print()
{
    qCDebug(lcSettings) << "Device id:" << deviceId();
    qCDebug(lcSettings) << "Database dir:" << databaseDir().absolutePath();
    qCDebug(lcSettings) << "Attachment cache dir:" << attachmentCacheDir().absolutePath();
    qCDebug(lcSettings) << "Attachment max file size:" << attachmentMaxFileSize();
    qCDebug(lcSettings) << "Thumbnails dir:" << thumbnailsDir().absolutePath();
    qCDebug(lcSettings) << "Thumbnail max size:" << attachmentMaxFileSize();
    qCDebug(lcSettings) << "Downloads dir:" << downloadsDir().absolutePath();
    if (devMode()) {
        qCDebug(lcSettings) << "Dev mode:" << true;
    }
}

void Settings::setLastSignedInUser(const QString &username)
{
    if (lastSignedInUser() == username) {
        return;
    }
    setGroupValue(kLastSessionGroup, kSignedInUsername, username);
    sync();
    emit lastSignedInUserChanged(username);

    if (!username.isEmpty()) {
        addUserToList(username);
    }
}

QString Settings::lastSignedInUser() const
{
    return groupValue(kLastSessionGroup, kSignedInUsername).toString();
}

void Settings::setUsersList(const QStringList &users)
{
    if (usersList() == users) {
        return;
    }
    setGroupValue(kUsersGroup, kUsersList, users);
    sync();
    emit usersListChanged(users);
}

QStringList Settings::usersList() const
{
    return groupValue(kUsersGroup, kUsersList).toStringList();
}

void Settings::addUserToList(const QString &user)
{
    auto users = usersList();
    // FIFO
    if (users.contains(user)) {
        users.removeOne(user);
    }
    setUsersList(QStringList(user) + users);
}

QString Settings::settingsFileName()
{
    QString ext;
#if VS_MACOS || VS_IOS
    ext = QLatin1String(".plist");
#elif VS_LINUX || VS_WINDOWS
    ext = QLatin1String(".ini");
#endif
    return CustomerEnv::appDataLocation().filePath(QLatin1String("settings") + ext);
}

QSettings::Format Settings::settingsFormat()
{
#if VS_WINDOWS
    return Format::IniFormat;
#else
    return Format::NativeFormat;
#endif
}

QString Settings::userCredential(const QString &username) const
{
    return groupValue(kCredenitalsGroup, username).toString();
}

void Settings::setUserCredential(const QString &username, const QString &credential)
{
    setGroupValue(kCredenitalsGroup, username, credential);
    sync();
}

QString Settings::userInfo(const QString &username) const
{
    return groupValue(kUsersInfoGroup, username).toString();
}

void Settings::setUserInfo(const QString &username, const QString &userInfo)
{
    setGroupValue(kUsersInfoGroup, username, userInfo);
    sync();
}

QString Settings::deviceId() const
{
    return value(kDeviceId).toString();
}

bool Settings::runFlag() const
{
    auto lastSessionId = groupValue(kLastSessionGroup, kSessionId).toString();
    return !lastSessionId.isEmpty() && lastSessionId != m_sessionId;
}

void Settings::setRunFlag(bool run)
{
    if (run) {
        qCDebug(lcSettings) << "Save session id" << m_sessionId;
        setGroupValue(kLastSessionGroup, kSessionId, m_sessionId);
    } else {
        qCDebug(lcSettings) << "Reset session id";
        removeGroupKey(kLastSessionGroup, kSessionId);
    }
}

QDir Settings::logsDir()
{
    if (!m_logsDirInitialized) {
        m_logsDir.setPath(CustomerEnv::appDataLocation().filePath(QLatin1String("logs")));
        m_logsDirInitialized = true;
    }
    if (!m_logsDir.exists()) {
        qCDebug(lcSettings) << "Create logs dir:" << m_logsDir.absolutePath();
        QDir().mkpath(m_logsDir.absolutePath());
    }
    return m_logsDir;
}

QDir Settings::databaseDir() const
{
    if (!m_databaseDir.exists()) {
        FileUtils::forceCreateDir(m_databaseDir.absolutePath());
    }
    return m_databaseDir;
}

quint64 Settings::attachmentMaxFileSize() const
{
    return 25 * 1024 * 1024;
}

QDir Settings::attachmentCacheDir() const
{
    if (!m_attachmentCacheDir.exists()) {
        FileUtils::forceCreateDir(m_attachmentCacheDir.absolutePath());
    }
    return m_attachmentCacheDir;
}

QDir Settings::thumbnailsDir() const
{
    if (!m_thumbnaisDir.exists()) {
        FileUtils::forceCreateDir(m_thumbnaisDir.absolutePath());
    }
    return m_thumbnaisDir;
}

QDir Settings::downloadsDir() const
{
    if (!m_downloadsDir.exists()) {
        FileUtils::forceCreateDir(m_downloadsDir.absolutePath());
    }
    return m_downloadsDir;
}

QDir Settings::cloudFilesDownloadsDir(const QString &userName) const
{
    const QDir dir = m_downloadsDir.filePath(QLatin1String("VirgilCloudFiles/") + userName);
    if (!dir.exists()) {
        FileUtils::forceCreateDir(dir.absolutePath());
    }
    qCDebug(lcSettings) << "Cloud files dir:" << dir.absolutePath();
    return dir;
}

QDir Settings::cloudFilesCacheDir() const
{
    if (!m_cloudFilesCacheDir.exists()) {
        FileUtils::forceCreateDir(m_cloudFilesCacheDir.absolutePath());
    }
    return m_cloudFilesCacheDir;
}

QString Settings::makeThumbnailPath(const AttachmentId &attachmentId, bool isPreview) const
{
    return thumbnailsDir().filePath((isPreview ? QLatin1String("p-") : QLatin1String("t-")) + attachmentId
                                    + QLatin1String(".png"));
}

QSize Settings::thumbnailMaxSize() const
{
    return QSize(100, 80);
}

QSize Settings::previewMaxSize() const
{
    return 3 * thumbnailMaxSize();
}

bool Settings::devMode() const
{
    return false;
}

bool Settings::autoSendCrashReport() const
{
    return !devMode();
}

QRect Settings::windowGeometry() const
{
    return groupValue(kLastSessionGroup, kWindowGeometryId).toRect();
}

void Settings::setWindowGeometry(const QRect &geometry)
{
    setGroupValue(kLastSessionGroup, kWindowGeometryId, geometry);
    sync();
}

std::chrono::seconds Settings::nowInterval() const
{
    return std::chrono::seconds(5);
}

QString Settings::makeGroupKey(const QString &group, const QString &key) const
{
    return group + '/' + key;
}

void Settings::setGroupValue(const QString &group, const QString &key, const QVariant &value)
{
    setValue(makeGroupKey(group, key), value);
    sync();
}

QVariant Settings::groupValue(const QString &group, const QString &key, const QVariant &defaultValue) const
{
    return value(makeGroupKey(group, key), defaultValue);
}

void Settings::removeGroupKey(const QString &group, const QString &key)
{
    remove(makeGroupKey(group, key));
    sync();
}

void Settings::removeGroup(const QString &group)
{
    beginGroup(group);
    remove(QString());
    endGroup();
}

void Settings::createDeviceId()
{
    setValue(kDeviceId, Utils::createUuid());
    sync();
}

QDateTime Settings::chatHistoryLastSyncDate(const QString &chatId) const
{
    if (chatId.isEmpty()) {
        return groupValue(kChatsLastSyncDateGroup, lastSignedInUser()).toDateTime();
    } else {
        return groupValue(makeGroupKey(kChatsLastSyncDateGroup, lastSignedInUser()), chatId).toDateTime();
    }
}

void Settings::setChatHistoryLastSyncDate(const QString &chatId)
{
    if (chatId.isEmpty()) {
        setGroupValue(kChatsLastSyncDateGroup, lastSignedInUser(), QDateTime::currentDateTimeUtc());
    } else {
        setGroupValue(makeGroupKey(kChatsLastSyncDateGroup, lastSignedInUser()), chatId,
                      QDateTime::currentDateTimeUtc());
    }

    sync();
}
