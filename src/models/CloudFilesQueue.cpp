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

#include "CloudFilesQueue.h"

#include "CreateCloudFolderOperation.h"
#include "CloudFileOperation.h"
#include "DeleteCloudFilesOperation.h"
#include "DownloadCloudFileOperation.h"
#include "ListCloudFolderOperation.h"
#include "Messenger.h"
#include "ShareCloudFilesOperation.h"
#include "UploadCloudFileOperation.h"
#include "UserDatabase.h"

using namespace vm;
using Self = CloudFilesQueue;

Q_LOGGING_CATEGORY(lcCloudFilesQueue, "cloudfiles-queue");

Self::CloudFilesQueue(Messenger *messenger, UserDatabase *userDatabase, QObject *parent)
    : OperationQueue(lcCloudFilesQueue(), parent)
    , m_messenger(messenger)
    , m_userDatabase(userDatabase)
    , m_watcher(new CloudFolderUpdateWatcher(this))
{
    connect(m_messenger, &Messenger::signedOut, this, &CloudFilesQueue::stop);
    connect(m_userDatabase, &UserDatabase::opened, this, &Self::start);
    connect(this, &Self::pushListFolder, this, &Self::onPushListFolder);
    connect(this, &Self::pushCreateFolder, this, &Self::onPushCreateFolder);
    connect(this, &Self::pushUploadFile, this, &Self::onPushUploadFile);
    connect(this, &Self::pushDownloadFile, this, &Self::onPushDownloadFile);
    connect(this, &Self::pushDeleteFiles, this, &Self::onPushDeleteFiles);
    connect(this, &Self::pushShareFiles, this, &Self::onPushShareFiles);
    connect(this, &Self::updateCloudFiles, this, &Self::onUpdateCloudFiles);

    addCloudFileListener(m_watcher.data());
    addCloudFileListener(new UniqueCloudFileFilter(this));
}

Self::~CloudFilesQueue()
{
}

void Self::addCloudFileListener(CloudFilesQueueListenerPtr listener)
{
    m_cloudFileListeners.push_back(listener);
    addListener(listener.data());
}

Operation *Self::createOperation(OperationSourcePtr source)
{
    const auto cloudFileSource = dynamic_cast<CloudFileOperationSource *>(source.get());

    auto *op = new CloudFileOperation(m_messenger, m_watcher, nullptr);
    connect(op, &Operation::notificationCreated, this, &Self::notificationCreated);
    connect(op, &CloudFileOperation::cloudFilesUpdate, this, &Self::updateCloudFiles);

    switch (cloudFileSource->type()) {
        case SourceType::ListFolder:
            op->appendChild(new ListCloudFolderOperation(op, cloudFileSource->folder(), m_userDatabase));
            break;
        case SourceType::CreateFolder:
            op->appendChild(new CreateCloudFolderOperation(op, cloudFileSource->name(), cloudFileSource->folder()));
            break;
        case SourceType::Upload: {
            auto uploadOp = new UploadCloudFileOperation(op, cloudFileSource->filePath(), cloudFileSource->folder());
            connect(this, &Self::interruptFileOperation, uploadOp, [uploadOp](auto cloudFileId) {
                if (cloudFileId == uploadOp->cloudFileId()) {
                    uploadOp->interrupt();
                }
            });
            op->appendChild(uploadOp);
            break;
        }
        case SourceType::Download: {
            auto downloadOp = new DownloadCloudFileOperation(op, cloudFileSource->files().front(), cloudFileSource->folder());
            connect(this, &Self::interruptFileOperation, downloadOp, [downloadOp](auto cloudFileId) {
                if (cloudFileId == downloadOp->cloudFileId()) {
                    downloadOp->interrupt();
                }
            });
            op->appendChild(downloadOp);
            break;
        }
        case SourceType::Delete:
            op->appendChild(new DeleteCloudFilesOperation(op, cloudFileSource->files()));
            break;
        case SourceType::Share:
            op->appendChild(new ShareCloudFilesOperation(op, cloudFileSource->files(), cloudFileSource->contacts()));
            break;
        default:
            throw std::logic_error("CloudFilesQueue::createOperation is not fully implemented");
    }

    return op;
}

void Self::invalidateOperation(OperationSourcePtr source)
{
    Q_UNUSED(source)
}

qsizetype Self::maxAttemptCount() const
{
    return 0;
}

void Self::onPushListFolder(const CloudFileHandler &parentFolder)
{
    auto source = std::make_shared<CloudFileOperationSource>(SourceType::ListFolder);
    source->setPriority(OperationSource::Priority::Highest);
    source->setFolder(parentFolder);
    addSource(std::move(source));
}

void Self::onPushCreateFolder(const QString &name, const CloudFileHandler &parentFolder)
{
    auto source = std::make_shared<CloudFileOperationSource>(SourceType::CreateFolder);
    source->setPriority(OperationSource::Priority::Highest);
    source->setName(name);
    source->setFolder(parentFolder);
    addSource(std::move(source));
}

void Self::onPushUploadFile(const QString &filePath, const CloudFileHandler &parentFolder)
{
    auto source = std::make_shared<CloudFileOperationSource>(SourceType::Upload);
    source->setFolder(parentFolder);
    source->setFilePath(filePath);
    addSource(std::move(source));
}

void Self::onPushDownloadFile(const CloudFileHandler &file, const CloudFileHandler &parentFolder, const PostFunction &func)
{
    auto source = std::make_shared<CloudFileOperationSource>(SourceType::Download);
    source->setFolder(parentFolder);
    source->setFiles({ file });
    source->setPostFunction(func);
    addSource(std::move(source));
}

void Self::onPushDeleteFiles(const CloudFiles &files)
{
    auto source = std::make_shared<CloudFileOperationSource>(SourceType::Delete);
    source->setPriority(OperationSource::Priority::Highest);
    source->setFiles(files);
    addSource(std::move(source));
}

void Self::onPushShareFiles(const CloudFiles &files, const Contacts &contacts)
{
    auto source = std::make_shared<CloudFileOperationSource>(SourceType::Share);
    source->setPriority(OperationSource::Priority::Highest);
    source->setFiles(files);
    source->setContacts(contacts);
    addSource(std::move(source));
}

void CloudFilesQueue::onUpdateCloudFiles(const CloudFilesUpdate &update)
{
    for (auto &listener : m_cloudFileListeners) {
        listener->updateCloudFiles(update);
    }
}
