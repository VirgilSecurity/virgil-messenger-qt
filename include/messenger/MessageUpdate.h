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

#ifndef VM_MESSAGE_UPDATE_H
#define VM_MESSAGE_UPDATE_H

#include "MessageId.h"
#include "MessageStatus.h"
#include "AttachmentId.h"
#include "IncomingMessageStage.h"
#include "OutgoingMessageStage.h"
#include "MessageContentUploadStage.h"
#include "MessageContentDownloadStage.h"

#include <QtCore>
#include <QString>
#include <QUrl>

#include <optional>
#include <variant>

namespace vm {

struct MessageUpdateBase
{
    MessageId messageId;
};

struct MessageAttachmentUpdateBase : public MessageUpdateBase
{
    AttachmentId attachmentId;
};

struct IncomingMessageStageUpdate : public MessageUpdateBase
{
    IncomingMessageStage stage;
};

struct OutgoingMessageStageUpdate : public MessageUpdateBase
{
    OutgoingMessageStage stage;
};

struct MessageAttachmentUploadStageUpdate : public MessageAttachmentUpdateBase
{
    MessageContentUploadStage uploadStage;
};

struct MessageAttachmentDownloadStageUpdate : public MessageAttachmentUpdateBase
{
    MessageContentDownloadStage downloadStage;
};

struct MessageAttachmentFingerprintUpdate : public MessageAttachmentUpdateBase
{
    QString fingerprint;
};

struct MessageAttachmentRemoteUrlUpdate : public MessageAttachmentUpdateBase
{
    QUrl remoteUrl;
};

struct MessageAttachmentLocalPathUpdate : public MessageAttachmentUpdateBase
{
    QString localPath;
};

struct MessageAttachmentEncryptionUpdate : public MessageAttachmentUpdateBase
{
    quint64 encryptedSize;
    QByteArray decryptionKey;
    QByteArray signature;
};

struct MessageAttachmentProcessedSizeUpdate : public MessageAttachmentUpdateBase
{
    quint64 processedSize;
};

struct MessageAttachmentExtrasUpdate : public MessageAttachmentUpdateBase
{
    std::function<QString()> extrasToJson; // TODO(fpohtmeh): find better way to get extras in update
};

struct MessagePictureThumbnailPathUpdate : public MessageAttachmentExtrasUpdate
{
    QString thumbnailPath;
};

struct MessagePictureThumbnailSizeUpdate : public MessageAttachmentExtrasUpdate
{
    QSize thumbnailSize;
};

struct MessagePictureThumbnailEncryptionUpdate : public MessageAttachmentExtrasUpdate
{
    qint64 encryptedSize;
    QByteArray decryptionKey;
    QByteArray signature;
};

struct MessagePictureThumbnailRemoteUrlUpdate : public MessageAttachmentExtrasUpdate
{
    QUrl remoteUrl;
};

struct MessagePicturePreviewPathUpdate : public MessageAttachmentExtrasUpdate
{
    QString previewPath;
};

using MessageUpdate = std::variant<
        IncomingMessageStageUpdate, OutgoingMessageStageUpdate, MessageAttachmentUploadStageUpdate,
        MessageAttachmentDownloadStageUpdate, MessageAttachmentFingerprintUpdate, MessageAttachmentRemoteUrlUpdate,
        MessageAttachmentEncryptionUpdate, MessageAttachmentLocalPathUpdate, MessageAttachmentProcessedSizeUpdate,
        MessagePictureThumbnailPathUpdate, MessagePictureThumbnailSizeUpdate, MessagePictureThumbnailEncryptionUpdate,
        MessagePictureThumbnailRemoteUrlUpdate, MessagePicturePreviewPathUpdate>;

//
//  Return message unique identifier the update relates to.
//
MessageId MessageUpdateGetMessageId(const MessageUpdate &update);

//
//  Convert to message attachment extras update if possible.
//
const MessageAttachmentExtrasUpdate *MessageUpdateToAttachmentExtrasUpdate(const MessageUpdate &update);
} // namespace vm

#endif // VM_MESSAGE_UPDATE_H
