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

#include "Settings.h"
#include "Utils.h"
#include "FileUtils.h"
#include "operations/CalculateAttachmentFingerprintOperation.h"
#include "operations/ConvertToPngOperation.h"
#include "operations/CreateAttachmentPreviewOperation.h"
#include "operations/CreateAttachmentThumbnailOperation.h"
#include "operations/CreateThumbnailOperation.h"
#include "operations/EncryptUploadFileOperation.h"
#include "operations/MessageOperation.h"
#include "operations/MessageOperationFactory.h"

using namespace vm;

UploadAttachmentOperation::UploadAttachmentOperation(MessageOperation *parent, const Settings *settings)
    : LoadAttachmentOperation(parent)
    , m_parent(parent)
    , m_settings(settings)
{
    setName(QString("UploadAttachment"));
}

bool UploadAttachmentOperation::populateChildren()
{
    const auto message = m_parent->message();
    const auto attachment = m_parent->attachment();
    auto factory = m_parent->factory();
    const bool isPicture = attachment->type == Attachment::Type::Picture;

    // Convert to png
    ConvertToPngOperation *convertOp = nullptr;
    if (isPicture) {
        convertOp = factory->populateConvertToPngOperation(this, attachment->localPath);
        connect(convertOp, &ConvertToPngOperation::fileCreated, this, &UploadAttachmentOperation::setTempPngPath);
    }
    //  Create picture preview
    if (isPicture) {
        const auto filePath = m_settings->makeThumbnailPath(attachment->id, true);
        auto op = factory->populateCreateAttachmentPreview(m_parent, this, attachment->localPath, filePath);
        if (convertOp) {
            connect(convertOp, &ConvertToPngOperation::imageRead, op, &CreateAttachmentPreviewOperation::setSourceImage);
            connect(convertOp, &ConvertToPngOperation::converted, op, &CreateAttachmentPreviewOperation::setSourcePath);
        }
    }
    // Calculate attachment fingerprint
    if (attachment->fingerprint.isEmpty()) {
        auto op = factory->populateCalculateAttachmentFingerprint(m_parent, this, attachment->localPath);
        if (convertOp) {
            connect(convertOp, &ConvertToPngOperation::converted, op, &CalculateAttachmentFingerprintOperation::setSourcePath);
        }
    }
    // Encrypt/Upload attachment file
    if (!attachment->url.isValid())
    {
        auto op = factory->populateEncryptUpload(this, attachment->localPath, message->recipientId);
        connect(op, &EncryptUploadFileOperation::bytesCalculated, this, std::bind(&LoadAttachmentOperation::startLoadOperation, this, args::_1));
        connect(op, &EncryptUploadFileOperation::progressChanged, this, &LoadAttachmentOperation::setLoadOperationProgress);
        connect(op, &EncryptUploadFileOperation::uploaded, m_parent, &MessageOperation::setAttachmentUrl);
        connect(op, &EncryptUploadFileOperation::bytesCalculated, m_parent, &MessageOperation::setAttachmentEncryptedSize);
        if (convertOp) {
            connect(convertOp, &ConvertToPngOperation::converted, op, &EncryptUploadFileOperation::setSourcePath);
        }
    }
    // Process picture
    if (isPicture) {
        // Create thumbnail
        const auto filePath = m_settings->makeThumbnailPath(attachment->id, false);
        auto op = factory->populateCreateAttachmentThumbnail(m_parent, this, attachment->localPath, filePath);
        if (convertOp) {
            connect(convertOp, &ConvertToPngOperation::imageRead, op, &CreateAttachmentThumbnailOperation::setSourceImage);
            connect(convertOp, &ConvertToPngOperation::converted, op, &CreateAttachmentThumbnailOperation::setSourcePath);
        }

        // Encrypt/upload thumbnail
        auto op2 = factory->populateEncryptUpload(this, filePath, message->recipientId);
        op2->setName(op2->name() + QLatin1String("Thumbnail"));
        connect(op2, &EncryptUploadFileOperation::bytesCalculated, this, std::bind(&LoadAttachmentOperation::startLoadOperation, this, args::_1));
        connect(op2, &EncryptUploadFileOperation::progressChanged, this, &LoadAttachmentOperation::setLoadOperationProgress);
        connect(op2, &EncryptUploadFileOperation::uploaded, m_parent, &MessageOperation::setAttachmentThumbnailUrl);
        connect(op2, &EncryptUploadFileOperation::bytesCalculated, m_parent, &MessageOperation::setAttachmentEncryptedThumbnailSize);
    }
    // Connection to loading statuses
    connect(this, &Operation::started, m_parent, std::bind(&MessageOperation::setAttachmentStatus, m_parent, Attachment::Status::Loading));
    connect(this, &Operation::finished, m_parent, std::bind(&MessageOperation::setAttachmentStatus, m_parent, Attachment::Status::Loaded));
    return hasChildren();
}

void UploadAttachmentOperation::cleanup()
{
    FileUtils::removeFile(m_tempPngPath);
}

void UploadAttachmentOperation::setTempPngPath(const QString &path)
{
    m_tempPngPath = path;
}
