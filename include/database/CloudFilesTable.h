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

#ifndef VM_CLOUDFILESTABLE_H
#define VM_CLOUDFILESTABLE_H

#include "core/DatabaseTable.h"
#include "CloudFile.h"
#include "CloudFilesUpdate.h"

class QSqlQuery;

namespace vm
{
class CloudFilesTable : public DatabaseTable
{
    Q_OBJECT

public:
    explicit CloudFilesTable(Database *database);

signals:
    void fetch(const CloudFileHandler &folder);
    void updateCloudFiles(const CloudFilesUpdate &update);

    void errorOccurred(const QString &errorText);
    void fetched(const CloudFileHandler &folder, const ModifiableCloudFiles &cloudFiles);

private:
    bool create() override;

    bool createFiles(const ModifiableCloudFiles &cloudFiles);
    bool createFile(const CloudFileHandler &cloudFile);
    bool updateFiles(const CloudFiles &cloudFiles, const CloudFileUpdateSource source);
    bool updateFile(const CloudFileHandler &cloudFile, const CloudFileUpdateSource source);
    bool updateDownloadedFile(const DownloadCloudFileUpdate &update);
    bool deleteFiles(const CloudFiles &cloudFiles);

    QStringList getSubFoldersIds(const QString &folderId);

    void onFetch(const CloudFileHandler &folder);
    void onUpdateCloudFiles(const CloudFilesUpdate &update);
};
}

#endif // VM_CLOUDFILESTABLE_H
