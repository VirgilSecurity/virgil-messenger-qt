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

#ifndef VM_LIST_CLOUD_FOLDER_OPERATION_H
#define VM_LIST_CLOUD_FOLDER_OPERATION_H

#include <QPointer>

#include "CloudFile.h"
#include "CloudFileRequestId.h"
#include "Operation.h"

namespace vm
{
class CloudFileOperation;
class UserDatabase;

class ListCloudFolderOperation : public Operation
{
    Q_OBJECT

public:
    ListCloudFolderOperation(CloudFileOperation *parent, const CloudFileHandler &parentFolder, UserDatabase *userDatabase);

    void run() override;

private:
    void onDatabaseListFetched(const CloudFileHandler &parentFolder, const ModifiableCloudFiles &cloudFiles);
    void onCloudListFetched(CloudFileRequestId requestId, const ModifiableCloudFileHandler &parentFolder, const ModifiableCloudFiles &files);
    void onCloudListFetchErrorOccurred(CloudFileRequestId requestId, const QString &errorText);

    static bool fileIdLess(const ModifiableCloudFileHandler &lhs, const ModifiableCloudFileHandler &rhs);
    static bool fileUpdated(const ModifiableCloudFileHandler &lhs, const ModifiableCloudFileHandler &rhs);

    CloudFileOperation *m_parent;
    CloudFileHandler m_parentFolder;
    QPointer<UserDatabase> m_userDatabase;

    ModifiableCloudFiles m_cachedFiles;
    CloudFileRequestId m_requestId = 0;
};
}

#endif // VM_LIST_CLOUD_FOLDER_OPERATION_H
