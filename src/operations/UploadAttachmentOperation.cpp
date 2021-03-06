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

#include "operations/UploadAttachmentOperation.h"

#include "FileUtils.h"
#include "operations/CalculateAttachmentFingerprintOperation.h"
#include "operations/ConvertImageFormatOperation.h"
#include "operations/CreateAttachmentPreviewOperation.h"
#include "operations/CreateAttachmentThumbnailOperation.h"
#include "operations/EncryptUploadFileOperation.h"
#include "operations/MessageOperation.h"
#include "operations/MessageOperationFactory.h"

using namespace vm;
using Self = UploadAttachmentOperation;

Self::UploadAttachmentOperation(MessageOperation *parent, const Settings *settings)
    : LoadAttachmentOperation(parent, settings), m_parent(parent)
{
}

bool Self::populateChildren()
{
    const auto content = m_parent->message()->content();
    if (std::holds_alternative<MessageContentFile>(content)) {
        populateFileOperations();
    } else if (std::holds_alternative<MessageContentPicture>(content)) {
        populatePictureOperations();
    } else {
        throw std::logic_error("Invalid attachment content");
    }
    return true;
}

void Self::populateFileOperations()
{
    const auto message = m_parent->message();
    const auto attachment = std::get_if<MessageContentFile>(&message->content());
    const auto factory = m_parent->factory();

    // Fingerprint
    auto fingerprintOp = factory->populateCalculateAttachmentFingerprint(m_parent, this, attachment->localPath());
    connect(fingerprintOp, &Operation::finished, [this]() {
        // Stage update
        updateStage(MessageContentUploadStage::Preprocessed);
    });

    populateEncryptUpload();
}

void Self::populatePictureOperations()
{
    const auto message = m_parent->message();
    const auto attachment = std::get_if<MessageContentPicture>(&message->content());
    const auto factory = m_parent->factory();

    // Convert image to preffered format
    const auto fileName = QLatin1String("conv-%2").arg(attachment->id());
    auto convertOp = factory->populateConvertImageFormatOperation(this, attachment->localPath(), fileName);
    connect(convertOp, &ConvertImageFormatOperation::fileCreated, [this, message](const QString &filePath) {
        MessageAttachmentLocalPathUpdate update;
        update.messageId = message->id();
        update.attachmentId = message->contentAsAttachment()->id();
        update.localPath = filePath;
        m_parent->apply(update);
    });

    // Create picture preview
    const auto previewFilePath = m_settings->makeThumbnailPath(attachment->id(), true);
    auto createPreviewOp =
            factory->populateCreateAttachmentPreview(m_parent, this, attachment->localPath(), previewFilePath);
    connect(convertOp, &ConvertImageFormatOperation::converted, createPreviewOp,
            &CreateAttachmentPreviewOperation::setSourcePath);

    // Create thumbnail
    const auto thumbnailFilePath = m_settings->makeThumbnailPath(attachment->id(), false);
    auto createThumbnailOp =
            factory->populateCreateAttachmentThumbnail(m_parent, this, attachment->localPath(), thumbnailFilePath);
    connect(convertOp, &ConvertImageFormatOperation::converted, createThumbnailOp,
            &CreateAttachmentThumbnailOperation::setSourcePath);

    // Encrypt/Upload thumbnail
    auto encUploadThumbOp = factory->populateEncryptUpload(this, thumbnailFilePath);
    connect(encUploadThumbOp, &EncryptUploadFileOperation::progressChanged, this,
            &LoadAttachmentOperation::setLoadOperationProgress);
    connect(encUploadThumbOp, &EncryptUploadFileOperation::encrypted,
            [this, message](const QFileInfo &file, const QByteArray &decryptionKey, const QByteArray &signature) {
                startLoadOperation(file.size());
                // Thumbnail encrypted size update
                MessagePictureThumbnailEncryptionUpdate update;
                update.messageId = message->id();
                update.attachmentId = message->contentAsAttachment()->id();
                update.encryptedSize = file.size();
                update.decryptionKey = decryptionKey;
                update.signature = signature;
                m_parent->apply(update);
            });

    connect(encUploadThumbOp, &EncryptUploadFileOperation::uploaded, [this, message](const QUrl &url) {
        // Thumbnail remote url update
        MessagePictureThumbnailRemoteUrlUpdate urlUpdate;
        urlUpdate.messageId = message->id();
        urlUpdate.attachmentId = message->contentAsAttachment()->id();
        urlUpdate.remoteUrl = url;
        m_parent->apply(urlUpdate);
    });

    // Calculate attachment fingerprint
    auto fingerprintOp = factory->populateCalculateAttachmentFingerprint(m_parent, this, attachment->localPath());
    connect(convertOp, &ConvertImageFormatOperation::converted, fingerprintOp,
            &CalculateAttachmentFingerprintOperation::setSourcePath);
    connect(fingerprintOp, &Operation::finished, [this]() {
        // Stage update
        updateStage(MessageContentUploadStage::Preprocessed);
    });

    // Encrypt/Upload attachment
    auto encUploadOp = populateEncryptUpload();
    connect(convertOp, &ConvertImageFormatOperation::converted, encUploadOp,
            &EncryptUploadFileOperation::setSourcePath);
}

EncryptUploadFileOperation *Self::populateEncryptUpload()
{
    const auto message = m_parent->message();
    const auto attachment = message->contentAsAttachment();

    // Encrypt/Upload
    auto encUploadOp = m_parent->factory()->populateEncryptUpload(this, attachment->localPath());
    connect(encUploadOp, &EncryptUploadFileOperation::progressChanged, this,
            &LoadAttachmentOperation::setLoadOperationProgress);
    connect(encUploadOp, &EncryptUploadFileOperation::encrypted,
            [this, message](const QFileInfo &file, const QByteArray &decryptionKey, const QByteArray &signature) {
                startLoadOperation(file.size());
                // Encrypted size update
                MessageAttachmentEncryptionUpdate update;
                update.messageId = message->id();
                update.attachmentId = message->contentAsAttachment()->id();
                update.encryptedSize = file.size();
                update.decryptionKey = decryptionKey;
                update.signature = signature;
                m_parent->apply(update);
                // Stage update
                updateStage(MessageContentUploadStage::Encrypted);
            });
    connect(encUploadOp, &EncryptUploadFileOperation::uploadSlotReceived, [this]() {
        // Stage update
        updateStage(MessageContentUploadStage::GotUploadingSlot);
    });
    connect(encUploadOp, &EncryptUploadFileOperation::uploaded, [this, message](const QUrl &url) {
        // Remote url update
        MessageAttachmentRemoteUrlUpdate urlUpdate;
        urlUpdate.messageId = message->id();
        urlUpdate.attachmentId = message->contentAsAttachment()->id();
        urlUpdate.remoteUrl = url;
        m_parent->apply(urlUpdate);
        // Stage update
        updateStage(MessageContentUploadStage::Uploaded);
    });

    return encUploadOp;
}

void Self::updateStage(MessageContentUploadStage uploadStage)
{
    const auto message = m_parent->message();

    MessageAttachmentUploadStageUpdate stageUpdate;
    stageUpdate.messageId = message->id();
    stageUpdate.attachmentId = message->contentAsAttachment()->id();
    stageUpdate.uploadStage = uploadStage;
    m_parent->apply(stageUpdate);
}
