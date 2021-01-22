//  Copyright (C) 2015-2021 Virgil Security, Inc.
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

#ifndef VM_CLOUD_FILE_SYSTEM_H
#define VM_CLOUD_FILE_SYSTEM_H

#include <QObject>
#include <QLoggingCategory>
#include <QPointer>

#include "CloudFile.h"
#include "CoreMessengerCloudFs.h"
#include "Settings.h"

Q_DECLARE_LOGGING_CATEGORY(lcCloudFileSystem)

namespace vm
{
class CoreMessenger;
class Messenger;

class CloudFileSystem : public QObject
{
    Q_OBJECT

public:
    CloudFileSystem(CoreMessenger *coreMessenger, Messenger *messenger);

    void signIn();
    void signOut();

    void fetchList(const CloudFileHandler &parentFolder);
    void createFile(const QString &filePath, const CloudFileHandler &parentFolder);
    void createFolder(const QString &name, const CloudFileHandler &parentFolder);
    void deleteFiles(const CloudFiles &files);

    QDir downloadsDir() const;

signals:
    void downloadsDirChanged(const QDir &downloadsDir);

    void listFetched(const ModifiableCloudFileHandler &parentFolder, const ModifiableCloudFiles &files);
    void fetchListErrorOccured(const QString &errorText);

    void fileCreated(const ModifiableCloudFileHandler &cloudFile, const QString &encryptedFilePath, const QUrl &uploadUrl);
    void createFileErrorOccurred(const QString &errorText);

    void folderCreated(const ModifiableCloudFileHandler &parentFolder);
    void createFolderErrorOccured(const QString &errorText);

    void fileDeleted(const CloudFileHandler &file);
    void deleteFileErrorOccured(const QString &errorText);

private:
    ModifiableCloudFileHandler createParentFolderFromInfo(const CloudFsFolder &fsFolder, const CloudFileHandler &oldFolder) const;
    ModifiableCloudFileHandler createFolderFromInfo(const CloudFsFolderInfo &info, const CloudFileId &parentId, const QString &localPath) const;
    ModifiableCloudFileHandler createFileFromInfo(const CloudFsFileInfo &info, const CloudFileId &parentId, const QString &localPath) const;

    QPointer<CoreMessenger> m_coreMessenger;
    Messenger *m_messenger;
    std::optional<CoreMessengerCloudFs> m_coreFs;
    QDir m_downloadsDir;
};
}

#endif // VM_CLOUD_FILE_SYSTEM_H
