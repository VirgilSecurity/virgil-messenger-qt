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
#include "operations/CreateAttachmentPreviewOperation.h"
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
    setName((parameter.type == Parameter::Type::Download) ? QLatin1String("DownloadAttachment") : QLatin1String("PreloadAttachment"));
}

bool Self::populateChildren()
{
    if (m_parameter.type == Parameter::Type::Preload) {
        populatePreload();
    }
    else if (m_parameter.type == Parameter::Type::Download) {
        populateDownload();
    }

    return hasChildren();
}

void Self::populateDownload()
{
    const auto message = m_parent->message();
    const auto attachment = message->contentAsAttachment();
    const auto factory = m_parent->factory();

    // Download/decrypt file
    auto downloadPath = m_parameter.filePath;
    if (!FileUtils::fileExists(downloadPath)) {
        qCDebug(lcOperation) << "Download attachment to:" << downloadPath;

        auto downloadDecOp = factory->populateDownloadDecrypt(this, attachment->remoteUrl(), attachment->encryptedSize(),
                                                   downloadPath, attachment->decryptionKey(), attachment->signature(), message->senderId());

        connect(downloadDecOp, &DownloadDecryptFileOperation::progressChanged, this, &LoadAttachmentOperation::setLoadOperationProgress);

        connect(downloadDecOp, &Operation::started, this, [this, encryptedSize = attachment->encryptedSize()]() {
            startLoadOperation(encryptedSize);
            updateStage(MessageContentDownloadStage::Downloading);
        });

        connect(downloadDecOp, &DownloadDecryptFileOperation::downloaded, [this]() {
            updateStage(MessageContentDownloadStage::Downloaded);
        });

        connect(downloadDecOp, &DownloadDecryptFileOperation::decrypted, [this, message](const QFileInfo &file) {
            MessageAttachmentLocalPathUpdate update;
            update.messageId = message->id();
            update.attachmentId = message->contentAsAttachment()->id();
            update.localPath = file.absoluteFilePath();
            m_parent->messageUpdate(update);

            updateStage(MessageContentDownloadStage::Decrypted);
        });
    }

    // Create picture preview
    const auto picture = std::get_if<MessageContentPicture>(&message->content());
    if (picture && !FileUtils::fileExists(picture->previewPath())) {
        const auto previewPath = m_settings->makeThumbnailPath(picture->id(), true);
        auto createPreviewOp = factory->populateCreateAttachmentPreview(m_parent, this, downloadPath, previewPath);
        connect(createPreviewOp, &Operation::finished, [this]() {
            updateStage(MessageContentDownloadStage::Preloaded);
            updateStage(MessageContentDownloadStage::Decrypted); // TODO(fpohtmeh): don't use this stage as final?
        });
    }
}

void Self::populatePreload()
{
    const auto message = m_parent->message();
    const auto factory = m_parent->factory();

    const auto picture = std::get_if<MessageContentPicture>(&message->content());
    if (!picture) {
        throw std::logic_error("Invalid attachment content in preload operation");
    }

    if (FileUtils::fileExists(picture->previewPath())) {
        return;
    }

    // Create preview from original file
    if (FileUtils::fileExists(picture->localPath())) {
        const auto previewPath = m_settings->makeThumbnailPath(picture->id(), true);
        factory->populateCreateAttachmentPreview(m_parent, this, picture->localPath(), previewPath);
    }
    else {
        // Check if thumbnail exists
        const auto thumbnail = picture->thumbnail();
        if (FileUtils::fileExists(thumbnail.localPath())) {
            return;
        }

        // Download/decrypt thumbnail
        const auto thumbnailPath = m_settings->makeThumbnailPath(picture->id(), false);

        auto downloadDecOp = factory->populateDownloadDecrypt(this, thumbnail.remoteUrl(), thumbnail.encryptedSize(),
                                                              thumbnailPath, thumbnail.decryptionKey(), thumbnail.signature(), message->senderId());
        downloadDecOp->setName(QLatin1String("DownloadDecryptThumbnail"));
        connect(downloadDecOp, &DownloadDecryptFileOperation::progressChanged, this, &LoadAttachmentOperation::setLoadOperationProgress);
        connect(downloadDecOp, &Operation::started, [this, encryptedSize = thumbnail.encryptedSize()]() {
            startLoadOperation(encryptedSize);
        });
        connect(downloadDecOp, &DownloadDecryptFileOperation::decrypted, [this, message](const QFileInfo &file) {
            const auto extrasToJson = [message]() {
                const auto picture = std::get_if<MessageContentPicture>(&message->content());
                return picture->extrasToJson(true);
            };

            MessagePictureThumbnailPathUpdate update;
            update.messageId = message->id();
            update.attachmentId = message->contentAsAttachment()->id();
            update.extrasToJson = extrasToJson;
            update.thumbnailPath = file.absoluteFilePath();
            m_parent->messageUpdate(update);
        });
    }

    // Update stages
    connect(this, &Operation::started, [this]() {
        updateStage(MessageContentDownloadStage::Preloading);
    });

    connect(this, &Operation::finished, [this]() {
        updateStage(MessageContentDownloadStage::Preloaded);
    });
}

void DownloadAttachmentOperation::updateStage(MessageContentDownloadStage downloadStage)
{
    const auto message = m_parent->message();

    MessageAttachmentDownloadStageUpdate stageUpdate;
    stageUpdate.messageId = message->id();
    stageUpdate.attachmentId = message->contentAsAttachment()->id();
    stageUpdate.downloadStage = downloadStage;
    m_parent->messageUpdate(stageUpdate);
}
