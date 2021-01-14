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

#ifndef VS_CLOUDFILESQUEUE_H
#define VS_CLOUDFILESQUEUE_H

#include <QLoggingCategory>

#include "CloudFile.h"
#include "OperationQueue.h"

Q_DECLARE_LOGGING_CATEGORY(lcCloudFilesQueue);

namespace vm
{
class CloudFilesQueue : public OperationQueue
{
    Q_OBJECT

public:
    explicit CloudFilesQueue(QObject *parent);
    ~CloudFilesQueue() override;

signals:
    void pushCreateDirectory(const QString &name, const CloudFileHandler &parentDir);
    void pushUploadFile(const QString &filePath, const CloudFileHandler &parentDir);
    void pushDeleteFiles(const CloudFiles &files);

private:
    Operation *createOperation(OperationSourcePtr source) override;
    void invalidateOperation(OperationSourcePtr source) override;

    void onPushCreateDirectory(const QString &name, const CloudFileHandler &parentDir);
    void onPushUploadFile(const QString &filePath, const CloudFileHandler &parentDir);
    void onPushDeleteFiles(const CloudFiles &files);
};
}

#endif // VS_CLOUDFILESQUEUE_H
