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

#ifndef VM_DELETE_CLOUD_FILES_OPERATION_H
#define VM_DELETE_CLOUD_FILES_OPERATION_H

#include "CloudFile.h"
#include "CloudFileRequestId.h"
#include "Operation.h"

namespace vm {
class CloudFileOperation;

class DeleteCloudFilesOperation : public Operation
{
    Q_OBJECT

public:
    DeleteCloudFilesOperation(CloudFileOperation *parent, const CloudFiles &files);

    void run() override;

private:
    void incProcessedCount();

    void onFileDeleted(CloudFileRequestId requestId, const CloudFileHandler &file);
    void onDeleteFileErrorOccured(CloudFileRequestId requestId, const QString &errorText);

    void deleteLocalFiles();

    CloudFileOperation *m_parent;
    const CloudFiles m_files;
    CloudFileRequestId m_requestId;
    size_t m_processedCount;
    CloudFiles m_deletedFiles;
};
} // namespace vm

#endif // VM_DELETE_CLOUD_FILES_OPERATION_H
