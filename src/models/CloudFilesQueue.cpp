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
#include "CloudFileOperationSource.h"
#include "DeleteCloudFilesOperation.h"
#include "Messenger.h"
#include "UploadCloudFileOperation.h"

using namespace vm;
using Self = CloudFilesQueue;

Q_LOGGING_CATEGORY(lcCloudFilesQueue, "cloudfiles-queue");

Self::CloudFilesQueue(Messenger *messenger, QObject *parent)
    : OperationQueue(lcCloudFilesQueue(), parent)
    , m_messenger(messenger)
{
    connect(m_messenger, &Messenger::signedOut, this, &CloudFilesQueue::stop);
    connect(this, &Self::pushCreateFolder, this, &Self::onPushCreateFolder);
    connect(this, &Self::pushUploadFile, this, &Self::onPushUploadFile);
    connect(this, &Self::pushDeleteFiles, this, &Self::onPushDeleteFiles);
}

Self::~CloudFilesQueue()
{
}

Operation *Self::createOperation(OperationSourcePtr source)
{
    const auto cloudFileSource = dynamic_cast<CloudFileOperationSource *>(source.get());

    auto *op = new CloudFileOperation(m_messenger->currentUser()->id(), nullptr);
    connect(op, &Operation::notificationCreated, this, &Self::notificationCreated);
    connect(op, &CloudFileOperation::cloudFileUpdate, this, &Self::updateCloudFile);

    switch (cloudFileSource->type()) {
        case CloudFileOperationSource::Type::CreateFolder:
            op->appendChild(new CreateCloudFolderOperation(op, cloudFileSource->name(), cloudFileSource->folder()));
            break;
        case CloudFileOperationSource::Type::Upload:
            op->appendChild(new UploadCloudFileOperation(op, cloudFileSource->filePath(), cloudFileSource->folder()));
            break;
        case CloudFileOperationSource::Type::Delete:
            op->appendChild(new DeleteCloudFilesOperation(op, cloudFileSource->files()));
            break;
        default:
            throw std::logic_error("CloudFilesQueue::createOperation is not fully implemented");
            break;
    }

    return op;
}

void Self::invalidateOperation(OperationSourcePtr source)
{
    Q_UNUSED(source)
}

void Self::onPushCreateFolder(const QString &name, const CloudFileHandler &parentFolder)
{
    auto source = std::make_shared<CloudFileOperationSource>(CloudFileOperationSource::Type::CreateFolder);
    source->setPriority(OperationSource::Priority::Highest);
    source->setName(name);
    source->setFolder(parentFolder);
    addSource(source);
}

void Self::onPushUploadFile(const QString &filePath, const CloudFileHandler &parentFolder)
{
    auto source = std::make_shared<CloudFileOperationSource>(CloudFileOperationSource::Type::Upload);
    source->setFolder(parentFolder);
    source->setFilePath(filePath);
    addSource(source);
}

void Self::onPushDeleteFiles(const CloudFiles &files)
{
    auto source = std::make_shared<CloudFileOperationSource>(CloudFileOperationSource::Type::Delete);
    source->setPriority(OperationSource::Priority::Highest);
    source->setFiles(files);
    addSource(source);
}