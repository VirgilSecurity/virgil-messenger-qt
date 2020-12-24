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

#include "operations/DownloadAttachmentOperation.h"

#include "Settings.h"
#include "Utils.h"
#include "FileUtils.h"
#include "operations/MessageOperation.h"
#include "operations/MessageOperationFactory.h"
#include "operations/DownloadDecryptFileOperation.h"
#include "MessageUpdate.h"

using namespace vm;
using Self = DownloadAttachmentOperation;

Self::DownloadAttachmentOperation(MessageOperation *parent, const Settings *settings, const Parameter &parameter)
    : LoadAttachmentOperation(parent)
    , m_parent(parent)
    , m_settings(settings)
    , m_parameter(parameter)
{
    setName((parameter.type == Parameter::Type::Full) ? QLatin1String("DownloadAttachment") : QLatin1String("PreloadAttachment"));
}

bool Self::populateChildren()
{
    const auto message = m_parent->message();
    auto factory = m_parent->factory();

    if (m_parameter.type == Parameter::Type::Preload) {
        if (auto picture = std::get_if<MessageContentPicture>(&message->content())) {
            // Create preview from original file
            if (!FileUtils::fileExists(picture->previewPath()) && FileUtils::fileExists(picture->localPath())) {
                const auto filePath = m_settings->makeThumbnailPath(picture->id(), true);
                factory->populateCreateAttachmentPreview(m_parent, this, picture->localPath(), filePath);
            }
            // Otherwise download/decrypt thumbnail
            else if (!FileUtils::fileExists(picture->thumbnail().localPath())) {
                const auto filePath = m_settings->makeThumbnailPath(picture->id(), false);
                auto op = factory->populateDownloadDecrypt(this, picture->thumbnail().remoteUrl(), picture->thumbnail().encryptedSize(), filePath, message->senderId());
                op->setName(op->name() + QLatin1String("Thumbnail"));
                connect(op, &Operation::started, this, std::bind(&LoadAttachmentOperation::startLoadOperation, this, picture->thumbnail().encryptedSize()));
                connect(op, &DownloadDecryptFileOperation::progressChanged, this, &LoadAttachmentOperation::setLoadOperationProgress);
                connect(op, &DownloadDecryptFileOperation::decrypted, [parent = m_parent, message](const QString &filePath) {
                    MessagePictureThumbnailPathUpdate update;
                    update.messageId = message->id();
                    update.attachmentId = message->contentAsAttachment()->id();
                    update.thumbnailPath = filePath;
                    parent->messageUpdate(update);
                });
            }
        }
    }
    else if (auto attachment = std::get_if<MessageContentPicture>(&message->content())) {
        // Download/decrypt file
        auto downloadPath = m_parameter.filePath;
        if (!FileUtils::fileExists(downloadPath)) {
            qCDebug(lcOperation) << "Download attachment to:" << downloadPath;
            auto op = factory->populateDownloadDecrypt(this, attachment->remoteUrl(), attachment->encryptedSize(), downloadPath, message->senderId());
            connect(op, &Operation::started, this, std::bind(&LoadAttachmentOperation::startLoadOperation, this, attachment->encryptedSize()));
            connect(op, &DownloadDecryptFileOperation::progressChanged, this, &LoadAttachmentOperation::setLoadOperationProgress);
            connect(op, &DownloadDecryptFileOperation::decrypted, [parent = m_parent, message](const QString &filePath) {
                MessageAttachmentLocalPathUpdate update;
                update.messageId = message->id();
                update.attachmentId = message->contentAsAttachment()->id();
                update.localPath = filePath;
                parent->messageUpdate(update);
            });
        }
        // Create picture preview
        if (auto picture = std::get_if<MessageContentPicture>(&message->content())) {
            if (!FileUtils::fileExists(picture->previewPath())) {
                const auto filePath = m_settings->makeThumbnailPath(picture->id(), true);
                factory->populateCreateAttachmentPreview(m_parent, this, downloadPath, filePath);
            }
        }
    }
    return hasChildren();
}
