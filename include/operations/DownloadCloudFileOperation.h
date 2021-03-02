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

#ifndef VM_DOWNLOAD_CLOUD_FILE_OPERATION_H
#define VM_DOWNLOAD_CLOUD_FILE_OPERATION_H

#include "CloudFilesUpdate.h"
#include "CloudFileRequestId.h"
#include "DownloadFileOperation.h"

namespace vm {
class CloudFileOperation;

class DownloadCloudFileOperation : public DownloadFileOperation
{
    Q_OBJECT

public:
    DownloadCloudFileOperation(CloudFileOperation *parent, const CloudFileHandler &file,
                               const CloudFileHandler &parentFolder);

    void run() override;
    CloudFileId cloudFileId() const;

private:
    void cleanup() override;

    QString tempFilePath() const;

    void onDownloadInfoGot(CloudFileRequestId requestId, const CloudFileHandler &file, const QUrl &url,
                           const QByteArray &encryptionKey);
    void onGetDownloadInfoErrorOccurred(CloudFileRequestId requestId, const QString &errorText);
    void onProgressChanged(quint64 bytesLoaded, quint64 bytesTotal);
    void onDownloaded();
    void sendFailedTransferUpdate();

    void transferUpdate(TransferCloudFileUpdate::Stage stage, quint64 bytesLoaded);
    bool createLocalDir();

    CloudFileOperation *m_parent;
    CloudFileHandler m_file;
    CloudFileHandler m_parentFolder;
    CloudFileRequestId m_requestId;
    QByteArray m_encryptionKey;
};
} // namespace vm

#endif // VM_DOWNLOAD_CLOUD_FILE_OPERATION_H
