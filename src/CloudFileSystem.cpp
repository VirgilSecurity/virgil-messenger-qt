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

#include "CloudFileSystem.h"

#include "CoreMessenger.h"
#include "FutureWorker.h"

using namespace vm;

CloudFileSystem::CloudFileSystem(CoreMessenger *coreMessenger, QObject *parent)
    : QObject(parent)
    , m_coreMessenger(coreMessenger)
{
}

void CloudFileSystem::signIn()
{
    m_coreFs = m_coreMessenger->cloudFs();
}

void CloudFileSystem::signOut()
{
    m_coreFs = {};
}

void CloudFileSystem::fetchList(const CloudFileHandler &folder)
{
    // FIXME(fpohtmeh): implement
//    FutureWorker::run(m_coreFs->listFolder(folder->id().toCoreFolderId()), [this, folder = folder](const CloudFsFutureResult &result) {
//        if (auto fsFolder = std::get_if<CloudFsFolder>(&result)) {
//            // Build list
//            ModifiableCloudFiles list;
//            for (auto &f : fsFolder->folders) {
//                list.push_back(createFolder(f));
//            }
//            for (auto &f : fsFolder->files) {
//                list.push_back(createFile(f));
//            }
//            emit listFetched(folder, list);
//        }
//        else {
//            emit fetchListErrorOccured(tr("Cloud files listing error"));
//        }
//    });
}

ModifiableCloudFileHandler CloudFileSystem::createFolder(const QString &name, const CloudFileHandler &parentFolder)
{
    // FIXME(fpohtmeh): implement
    Q_UNUSED(name)
    Q_UNUSED(parentFolder)
//    const auto parentFolderId = parentFolder->id();
//    const auto folderId = std::get_if<CloudFsFolderId>(&parentFolderId);
//    const auto isRoot = !folderId->isValid();
//    auto future = isRoot ? m_cloudFs.createFolder(name) : m_cloudFs.createFolder(name, *folderId, parentFolder->publicKey());
    return {};
}

bool CloudFileSystem::deleteFiles(const CloudFiles &files)
{
    // FIXME(fpohtmeh): implement
    return false;
}

ModifiableCloudFileHandler CloudFileSystem::createFolder(const CloudFsFolderInfo &info) const
{
    auto folder = std::make_shared<CloudFile>();
    return folder;
}

ModifiableCloudFileHandler CloudFileSystem::createFile(const CloudFsFileInfo &info) const
{
    auto file = std::make_shared<CloudFile>();
    return file;
}
