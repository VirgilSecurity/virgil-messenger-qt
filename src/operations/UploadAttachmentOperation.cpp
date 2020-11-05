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
#include "operations/CreateAttachmentThumbnailOperation.h"
#include "operations/CreateThumbnailOperation.h"
#include "operations/EncryptUploadFileOperation.h"
#include "operations/MessageOperation.h"
#include "operations/MessageOperationFactory.h"

using namespace vm;

UploadAttachmentOperation::UploadAttachmentOperation(MessageOperation *parent, const Settings *settings)
    : Operation(QString("UploadAttachment"), parent)
    , m_parent(parent)
    , m_settings(settings)
{
}

bool UploadAttachmentOperation::populateChildren()
{
    const auto message = m_parent->message();
    const auto attachment = m_parent->attachment();
    auto factory = m_parent->factory();
    const QString preffix = name() + QChar('/');
    //  Create picture preview
    if (attachment->type == Attachment::Type::Picture) {
        const auto filePath = m_settings->generateThumbnailPath();
        factory->populateCreateAttachmentPreview(m_parent, this, attachment->localPath, filePath);
    }
    // Encrypt/Upload attachment file
    if (!attachment->url.isValid())
    {
        auto op = factory->populateEncryptUpload(preffix + QString("EncryptUpload"), this, attachment->localPath, message->recipientId);
        connect(op, &EncryptUploadFileOperation::progressChanged, m_parent, &MessageOperation::setAttachmentProgress);
        connect(op, &EncryptUploadFileOperation::uploaded, m_parent, &MessageOperation::setAttachmentUrl);
    }
    // Process picture
    if (attachment->type == Attachment::Type::Picture) {
        // Create thumbnail
        const auto filePath = m_settings->generateThumbnailPath();
        factory->populateCreateAttachmentThumbnail(m_parent, this, filePath);

        // Encrypt/upload thumbnail
        auto uplOp = factory->populateEncryptUpload(preffix + QString("EncryptUploadThumbnail"), this, filePath, message->recipientId);
        connect(uplOp, &EncryptUploadFileOperation::progressChanged, m_parent, &MessageOperation::setAttachmentProgress);
        connect(uplOp, &EncryptUploadFileOperation::uploaded, m_parent, &MessageOperation::setAttachmentThumbnailUrl);
    }
    // Connection to loading statuses
    connect(this, &Operation::started, m_parent, std::bind(&MessageOperation::setAttachmentStatus, m_parent, Attachment::Status::Loading));
    connect(this, &Operation::finished, m_parent, std::bind(&MessageOperation::setAttachmentStatus, m_parent, Attachment::Status::Loaded));
    return hasChildren();
}
