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
#include "FileUtils.h"
#include "operations/DecryptFileOperation.h"
#include "operations/DownloadFileOperation.h"

using namespace vm;

DownloadDecryptFileOperation::DownloadDecryptFileOperation(NetworkOperation *parent, Messenger *messenger, const Settings *settings,
                                                           const QUrl &url, quint64 bytesTotal, const QString &filePath,
                                                           const QByteArray& decryptionKey, const UserId &senderId)
    : NetworkOperation(parent)
    , m_messenger(messenger)
    , m_settings(settings)
    , m_url(url)
    , m_bytesTotal(bytesTotal)
    , m_filePath(filePath)
    , m_decryptionKey(decryptionKey)
    , m_senderId(senderId)
{
    setName(QLatin1String("DownloadDecrypt"));
}

bool DownloadDecryptFileOperation::populateChildren()
{
    m_tempPath = m_settings->attachmentCacheDir().filePath(Utils::createUuid());

    auto downOp = new DownloadFileOperation(this, m_messenger->fileLoader(), m_url, m_bytesTotal, m_tempPath);
    connect(downOp, &DownloadFileOperation::progressChanged, this, &DownloadDecryptFileOperation::progressChanged);
    connect(downOp, &DownloadFileOperation::downloaded, this, &DownloadDecryptFileOperation::downloaded);
    appendChild(downOp);

    auto decryptOp = new DecryptFileOperation(this, m_messenger, m_tempPath, m_filePath,
                                              m_decryptionKey, m_senderId);
    connect(decryptOp, &DecryptFileOperation::decrypted, this, &DownloadDecryptFileOperation::decrypted);
    appendChild(decryptOp);

    return true;
}

void DownloadDecryptFileOperation::cleanup()
{
    FileUtils::removeFile(m_tempPath);
    Operation::cleanup();
}
