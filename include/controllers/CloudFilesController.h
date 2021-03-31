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

#ifndef VM_CLOUDFILESCONTROLLER_H
#define VM_CLOUDFILESCONTROLLER_H

#include <QObject>

#include "CloudFileObject.h"
#include "CloudFileSystem.h"
#include "CloudFilesUpdate.h"

class Settings;

namespace vm {
class CloudFilesModel;
class Messenger;
class Models;
class UserDatabase;

class CloudFilesController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString displayPath READ displayPath NOTIFY displayPathChanged)
    Q_PROPERTY(bool isRoot READ isRoot NOTIFY isRootChanged)
    Q_PROPERTY(bool isListUpdating MEMBER m_isListUpdating NOTIFY isListUpdatingChanged)
    Q_PROPERTY(CloudFileObject *currentFolder MEMBER m_cloudFolderObject CONSTANT)

public:
    CloudFilesController(Messenger *messenger, Models *models, UserDatabase *userDatabase, QObject *parent);

    CloudFilesModel *model();
    void switchToRootFolder();
    void clearFiles();

    Q_INVOKABLE void openFile(const QVariant &proxyRow);
    Q_INVOKABLE void switchToFolder(const QVariant &proxyRow);
    Q_INVOKABLE void switchToParentFolder();
    Q_INVOKABLE void refresh();

    Q_INVOKABLE void addFiles(const QVariant &fileUrls);
    Q_INVOKABLE void deleteFiles();
    void createFolder(const QString &name, const CloudFileMembers &members);

    void loadCloudFileMembers();
    void addMembers(const CloudFileMembers &members);
    Q_INVOKABLE void removeSelectedMembers();

signals:
    void openUrlRequested(const QUrl &url);

    void updateCloudFiles(const CloudFilesUpdate &update);

    void notificationCreated(const QString &notification, const bool error) const;
    void errorOccurred(const QString &errorText);

    void displayPathChanged(const QString &path);
    void isRootChanged(bool isRoot);
    void isListUpdatingChanged(bool isUpdating);

private:
    using FoldersHierarchy = ModifiableCloudFiles;

    void switchToHierarchy(const FoldersHierarchy &hierarchy);
    void refreshIfOnline();

    QString displayPath() const;
    bool isRoot() const;
    ModifiableCloudFileHandler rootFolder() const;
    CloudFileHandler parentFolder() const;

    void onOnlineListingFailed();
    void onUpdateCloudFiles(const CloudFilesUpdate &update);

    QPointer<Messenger> m_messenger;
    QPointer<Models> m_models;
    QPointer<UserDatabase> m_userDatabase;
    QPointer<CloudFileSystem> m_cloudFileSystem;
    QPointer<CloudFileObject> m_cloudFolderObject;

    FoldersHierarchy m_hierarchy;
    FoldersHierarchy m_requestedHierarchy;
    bool m_isListUpdating = false;
    bool m_onlineRefreshNeeded = false;
};
} // namespace vm

#endif // VM_CLOUDFILESCONTROLLER_H
