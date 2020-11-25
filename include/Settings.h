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

#ifndef VM_SETTINGS_H
#define VM_SETTINGS_H

#include <QDir>
#include <QRect>
#include <QSettings>
#include <QSize>

#include "VSQCommon.h"

Q_DECLARE_LOGGING_CATEGORY(lcSettings)

class Settings : public QSettings
{
    Q_OBJECT
    Q_PROPERTY(QString lastSignedInUserId READ lastSignedInUserId WRITE setLastSignedInUserId NOTIFY lastSignedInUserIdChanged)
    Q_PROPERTY(QStringList usersList READ usersList WRITE setUsersList NOTIFY usersListChanged)
    Q_PROPERTY(bool devMode READ devMode CONSTANT)
    Q_PROPERTY(bool fileCloudEnabled READ fileCloudEnabled CONSTANT)
    Q_PROPERTY(QRect windowGeometry READ windowGeometry WRITE setWindowGeometry NOTIFY windowGeometryChanged)

public:
    explicit Settings(QObject *parent);
    ~Settings();

    void print();

    // Users

    void setLastSignedInUserId(const QString &userId);
    QString lastSignedInUserId() const;

    void setUsersList(const QStringList &users);
    QStringList usersList() const;
    void addUserToList(const QString &user);

    QString userCredential(const QString &user) const;
    void setUserCredential(const QString &user, const QString &userCredential);

    // Device id, run flags
    QString deviceId() const;
    bool runFlag() const;
    void setRunFlag(bool run);

    // Attachments

    QDir databaseDir() const;
    DataSize attachmentMaxFileSize() const;
    QDir attachmentCacheDir() const;
    QDir thumbnailsDir() const;
    QDir downloadsDir() const;

    QString makeThumbnailPath(const vm::Attachment::Id &attachmentId, bool isPreview) const;
    QSize thumbnailMaxSize() const;
    QSize previewMaxSize() const;

    // Modes / features
    bool devMode() const;
    bool fileCloudEnabled() const;
    bool autoSendCrashReport() const;

    // Window
    QRect windowGeometry() const;
    void setWindowGeometry(const QRect &geometry);

    // Short interval for elapsed seconds that means now
    Seconds nowInterval() const;

signals:
    void lastSignedInUserIdChanged(const QString &);
    void usersListChanged(const QStringList &);
    void windowGeometryChanged(const QRect &); // Required by QML, not used

private:
    QString makeGroupKey(const QString &group, const QString &key) const;
    void setGroupValue(const QString &group, const QString &key, const QVariant &value);
    QVariant groupValue(const QString &group, const QString &key, const QVariant &defaultValue = QVariant()) const;
    void removeGroupKey(const QString &group, const QString &key);
    void removeGroup(const QString &group);

    void createDeviceId();

    QString m_sessionId;
    QDir m_databaseDir;
    QDir m_attachmentCacheDir;
    QDir m_thumbnaisDir;
    QDir m_downloadsDir;
};

#endif // VM_SETTINGS_H
