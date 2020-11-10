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

#include "Settings.h"
#include "Utils.h"
#include "operations/EncryptFileOperation.h"
#include "operations/UploadFileOperation.h"

using namespace vm;

EncryptUploadFileOperation::EncryptUploadFileOperation(const QString &name, QObject *parent, const Settings *settings, const QString &sourcePath,
                                                       const Contact::Id &recipientId, FileLoader *fileLoader)
    : Operation(name, parent)
    , m_settings(settings)
    , m_sourcePath(sourcePath)
    , m_recipientId(recipientId)
    , m_fileLoader(fileLoader)
{}

void EncryptUploadFileOperation::setSourcePath(const QString &path)
{
    m_sourcePath = path;
}

bool EncryptUploadFileOperation::populateChildren()
{
    m_tempPath = m_settings->attachmentCacheDir().filePath(Utils::createUuid());
    const auto preffix = name() + QChar('/');

    auto encryptOp = new EncryptFileOperation(preffix + QString("Encrypt"), this, m_sourcePath, m_tempPath, m_recipientId);
    connect(encryptOp, &EncryptFileOperation::bytesCalculated, this, &EncryptUploadFileOperation::bytesCalculated);
    appendChild(encryptOp);

    auto uploadOp = new UploadFileOperation(preffix + QString("Upload"), this, m_tempPath, m_fileLoader);
    connect(uploadOp, &UploadFileOperation::progressChanged, this, &EncryptUploadFileOperation::progressChanged);
    connect(uploadOp, &UploadFileOperation::uploaded, this, &EncryptUploadFileOperation::uploaded);
    appendChild(uploadOp);

    return hasChildren();
}

void EncryptUploadFileOperation::cleanup()
{
    Utils::removeFile(m_tempPath);
    Operation::cleanup();
}
