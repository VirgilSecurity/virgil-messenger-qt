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

#include "CloudFilesQueueListeners.h"

#include <QFileInfo>

using namespace vm;

Q_LOGGING_CATEGORY(lcCloudFilesQueueListener, "cloudfiles-queue-listener");

CloudFilesQueueListener::~CloudFilesQueueListener()
{
}

bool CloudFilesQueueListener::preRunCloudFile(CloudFileOperationSource *source)
{
    Q_UNUSED(source)
    return true;
}

void CloudFilesQueueListener::postRunCloudFile(CloudFileOperationSource *source)
{
    Q_UNUSED(source)
}

bool CloudFilesQueueListener::preRun(OperationSourcePtr source)
{
    const auto fileSource = dynamic_cast<CloudFileOperationSource *>(source.get());
    return preRunCloudFile(fileSource);
}

void CloudFilesQueueListener::postRun(OperationSourcePtr source)
{
    const auto fileSource = dynamic_cast<CloudFileOperationSource *>(source.get());
    postRunCloudFile(fileSource);
}

bool UniqueCloudFileFilter::preRunCloudFile(CloudFileOperationSource *source)
{
    const auto id = createUniqueId(source);
    if (id.isEmpty()) {
        return true;
    }

    QMutexLocker locker(&m_mutex);
    const auto it = std::find(m_ids.begin(), m_ids.end(), id);
    if (it == m_ids.end()) {
        m_ids.push_back(id);
        return true;
    }

    qCDebug(lcCloudFilesQueueListener) << "Skipped duplicated operation:" << source->toString();
    notifyNotUnique(source);
    return false;
}

void UniqueCloudFileFilter::postRunCloudFile(CloudFileOperationSource *source)
{
    const auto id = createUniqueId(source);
    if (id.isEmpty()) {
        return;
    }

    QMutexLocker locker(&m_mutex);
    const auto it = std::find(m_ids.begin(), m_ids.end(), id);
    if (it != m_ids.end()) {
        m_ids.erase(it);
    }
}

void UniqueCloudFileFilter::clear()
{
    QMutexLocker locker(&m_mutex);
    m_ids.clear();
}

QString UniqueCloudFileFilter::createUniqueId(CloudFileOperationSource *source) const
{
    switch (source->type()) {
        case SourceType::Download:
            return source->file()->id();
        case SourceType::Upload:
            return source->folder()->id() + QLatin1Char('/') + QFileInfo(source->filePath()).fileName();
        case SourceType::CreateFolder:
            return source->folder()->id() + QLatin1Char('/') + source->name();
        default:
            return QString();
    }
}

void UniqueCloudFileFilter::notifyNotUnique(CloudFileOperationSource *source)
{
    switch (source->type()) {
        case SourceType::Download:
            emit notificationCreated(tr("File is downloading"), false);
            break;
        case SourceType::Upload:
            emit notificationCreated(tr("File name is not unique"), true);
            break;
        case SourceType::CreateFolder:
            emit notificationCreated(tr("Folder name is not unique"), true);
            break;
        default:
            break;
    }
}
