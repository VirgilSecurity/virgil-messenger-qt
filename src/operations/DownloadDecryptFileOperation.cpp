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

#include "operations/DownloadDecryptFileOperation.h"

#include "Settings.h"
#include "Utils.h"
#include "operations/DownloadFileOperation.h"
#include "operations/DecryptFileOperation.h"

using namespace vm;

DownloadDecryptFileOperation::DownloadDecryptFileOperation(const QString &name, QObject *parent, const Settings *settings, FileLoader *fileLoader,
                                                           const QUrl &url, const DataSize &bytesTotal, const QString &filePath, const Contact::Id &senderId)
    : Operation(name, parent)
    , m_settings(settings)
    , m_fileLoader(fileLoader)
    , m_url(url)
    , m_bytesTotal(bytesTotal)
    , m_filePath(filePath)
    , m_senderId(senderId)
{}

bool DownloadDecryptFileOperation::populateChildren()
{
    m_tempPath = m_settings->attachmentCacheDir().filePath(Utils::createUuid());
    const auto preffix = name() + QChar('/');

    auto downOp = new DownloadFileOperation(preffix + QString("Download"), this, m_fileLoader, m_url, m_bytesTotal, m_tempPath);
    connect(downOp, &DownloadFileOperation::progressChanged, this, &DownloadDecryptFileOperation::progressChanged);
    appendChild(downOp);

    auto decryptOp = new DecryptFileOperation(preffix + QString("Decrypt"), this, m_tempPath, m_filePath, m_senderId);
    connect(decryptOp, &DecryptFileOperation::decrypted, this, &DownloadDecryptFileOperation::decrypted);
    appendChild(decryptOp);

    return hasChildren();
}

void DownloadDecryptFileOperation::cleanup()
{
    Utils::removeFile(m_tempPath);
    Operation::cleanup();
}
