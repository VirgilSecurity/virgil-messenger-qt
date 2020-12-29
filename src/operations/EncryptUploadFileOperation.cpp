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

#include "operations/EncryptUploadFileOperation.h"

#include "FileUtils.h"
#include "Settings.h"
#include "Utils.h"
#include "operations/EncryptFileOperation.h"
#include "operations/UploadFileOperation.h"

using namespace vm;

EncryptUploadFileOperation::EncryptUploadFileOperation(NetworkOperation *parent, const Settings *settings, FileLoader *fileLoader, const QString &sourcePath, const UserId &recipientId)
    : NetworkOperation(parent)
    , m_fileLoader(fileLoader)
    , m_sourcePath(sourcePath)
    , m_tempPath(settings->attachmentCacheDir().filePath(Utils::createUuid()))
    , m_recipientId(recipientId)
{
    setName(QLatin1String("EncryptUpload"));
}

void EncryptUploadFileOperation::setSourcePath(const QString &sourcePath)
{
    m_sourcePath = sourcePath;
}

bool EncryptUploadFileOperation::populateChildren()
{
    auto encryptOp = new EncryptFileOperation(this, m_sourcePath, m_tempPath, m_recipientId);
    connect(encryptOp, &EncryptFileOperation::encrypted, this, &EncryptUploadFileOperation::encrypted);
    appendChild(encryptOp);

    auto uploadOp = new UploadFileOperation(this, m_fileLoader, m_tempPath);
    connect(uploadOp, &UploadFileOperation::progressChanged, this, &EncryptUploadFileOperation::progressChanged);
    connect(uploadOp, &UploadFileOperation::uploadSlotReceived, this, &EncryptUploadFileOperation::uploadSlotReceived);
    connect(uploadOp, &UploadFileOperation::uploaded, this, &EncryptUploadFileOperation::uploaded);
    appendChild(uploadOp);

    return true;
}

void EncryptUploadFileOperation::cleanup()
{
    Operation::cleanup();
    FileUtils::removeFile(m_tempPath);
}
