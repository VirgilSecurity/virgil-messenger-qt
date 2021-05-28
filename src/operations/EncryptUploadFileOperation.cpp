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
#include "Messenger.h"
#include "Settings.h"
#include "UidUtils.h"
#include "operations/EncryptFileOperation.h"
#include "operations/UploadFileOperation.h"

using namespace vm;

EncryptUploadFileOperation::EncryptUploadFileOperation(NetworkOperation *parent, Messenger *messenger,
                                                       const QString &sourcePath)
    : NetworkOperation(parent),
      m_messenger(messenger),
      m_sourcePath(sourcePath),
      m_tempPath(messenger->settings()->attachmentCacheDir().filePath(UidUtils::createUuid()))
{
    setName(QLatin1String("EncryptUpload"));
}

void EncryptUploadFileOperation::setSourcePath(const QString &sourcePath)
{
    m_sourcePath = sourcePath;
}

bool EncryptUploadFileOperation::populateChildren()
{
    auto encryptOp = new EncryptFileOperation(this, m_messenger, m_sourcePath, m_tempPath);
    connect(encryptOp, &EncryptFileOperation::encrypted, this, &EncryptUploadFileOperation::encrypted);
    appendChild(encryptOp);

    auto uploadOp = new UploadFileOperation(this, m_messenger->fileLoader(), m_tempPath);
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
