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

#include "CloudFileOperationSource.h"

using namespace vm;
using Self = CloudFilesQueue;

Q_LOGGING_CATEGORY(lcCloudFilesQueue, "cloudfiles-queue");

Self::CloudFilesQueue(QObject *parent)
    : OperationQueue(lcCloudFilesQueue(), parent)
{
    connect(this, &Self::pushCreateFolder, this, &Self::onPushCreateFolder);
    connect(this, &Self::pushUploadFile, this, &Self::onPushUploadFile);
    connect(this, &Self::pushDeleteFiles, this, &Self::onPushDeleteFiles);
}

Self::~CloudFilesQueue()
{
}

Operation *Self::createOperation(OperationSourcePtr source)
{
    return nullptr;
}

void Self::invalidateOperation(OperationSourcePtr source)
{
    Q_UNUSED(source)
}

void Self::onPushCreateFolder(const QString &name, const CloudFileHandler &parentFolder)
{
    Q_UNUSED(name)
}

void Self::onPushUploadFile(const QString &filePath, const CloudFileHandler &parentFolder)
{
    auto source = std::make_shared<CloudFileOperationSource>(CloudFileOperationSource::Type::Upload);
    source->setFolder(parentFolder);
    source->setUploadFilePath(filePath);
    addSource(source);
}

void Self::onPushDeleteFiles(const CloudFiles &files)
{
    Q_UNUSED(files)
}
