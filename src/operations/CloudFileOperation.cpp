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

#include "CloudFileOperation.h"

#include "Messenger.h"

using namespace vm;

qsizetype CloudFileOperation::m_counter = 0;

CloudFileOperation::CloudFileOperation(Messenger *messenger, QObject *parent)
    : NetworkOperation(parent, messenger->isOnline())
    , m_messenger(messenger)
{
    setName(QLatin1String("CloudFile(%1)").arg(QString::number(++m_counter)));
}

Settings *CloudFileOperation::settings()
{
    return m_messenger->settings();
}

CloudFileSystem *CloudFileOperation::cloudFileSystem()
{
    return m_messenger->cloudFileSystem();
}

FileLoader *CloudFileOperation::fileLoader()
{
    return m_messenger->fileLoader();
}

bool CloudFileOperation::waitForFolderKeys(const CloudFileHandler &cloudFolder)
{
    const auto folderId = cloudFolder->id().coreFolderId();
    if (const auto isRoot = !folderId.isValid()) {
        // We don't need keys
        return true;
    }

    // Wait a second for fetched keys
    QEventLoop loop;
    connect(cloudFileSystem(), &CloudFileSystem::listFetched, &loop, &QEventLoop::quit);
    connect(cloudFileSystem(), &CloudFileSystem::fetchListErrorOccured, &loop, &QEventLoop::quit);
    QTimer::singleShot(1000, &loop, &QEventLoop::quit);
    loop.exec();

    return !cloudFolder->publicKey().isEmpty() && !cloudFolder->encryptedKey().isEmpty();
}
