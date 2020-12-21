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


#ifndef VM_MESSAGE_CONTENT_ATTACHMENT_H
#define VM_MESSAGE_CONTENT_ATTACHMENT_H

#include "AttachmentId.h"
#include "MessageUpdateable.h"
#include "MessageContentUploadStage.h"
#include "MessageContentDownloadStage.h"

#include <QUrl>

namespace vm {

//
//  Base class for all message contents that handle attachment,
//  for instance image, file, voice record, etc.
//
class MessageContentAttachment : public MessageUpdateable {

public:
    virtual ~MessageContentAttachment() noexcept = default;

    //
    //  Defines processing stages for outgoing attachment.
    //
    using UploadStage =  MessageContentUploadStage;

    //
    //  Defines processing stages for incoming attachment.
    //
    using DownloadStage =  MessageContentDownloadStage;

    //
    //  Apply file specific update.
    //
    bool applyUpdate(const MessageUpdate& update) override;

    //
    //  Return attachment unique identifier.
    //
    AttachmentId id() const;

    //
    //  Set attachment unique identifier.
    //
    void setId(AttachmentId id);

    //
    //  Return attachment file name without path.
    //
    QString filename() const;

    //
    //  Set attachment file name without path.
    //
    void setFilename(QString fileName);

    //
    //  Return attachment fingerprint.
    //
    QString fingerprint() const;

    //
    //  Set attachment fingerprint.
    //  Note, can be calculated later.
    //
    void setFingerprint(QString fingerprint);

    //
    //  Return attachment size.
    //
    qint64 size() const;

    //
    //  Set attachment size.
    //
    void setSize(qint64 size);

    //
    //  Return attachment URL that should be used for uploading or downloading.
    //
    QUrl remoteUrl() const;

    //
    //  Set attachment URL that should be used for uploading or downloading.
    //  Note, if attachment is outgoing remote URL is known when uploading slot is received.
    //  Note, if attachment is incoming remote URL is known from message.
    //
    void setRemoteUrl(QUrl remoteUrl);

    //
    //  Return attachment absolute path.
    //
    QString localPath() const;

    //
    //  Set attachment absolute path.
    //  Note, if attachment is outgoing local path is known immediately.
    //  Note, if attachment is incoming local path is known when attachment is downloaded and decrypted.
    //
    void setLocalPath(QString localPath);

    //
    //  Return attachment encrypted size.
    //
    qint64 encryptedSize() const;

    //
    //  Set attachment encrypted size.
    //  Note, if attachment is outgoing encrypted size is known only after encryption.
    //  Note, if attachment is incoming encrypted size is known from message.
    //
    void setEncryptedSize(qint64 encryptedSize);

    //
    //  Return attachment processed size.
    //  Note, this property should not be persist.
    //
    qint64 processedSize() const;

    //
    //  Set attachment processed size.
    //  Note, this property should not be persist.
    //
    void setProcessedSize(qint64 processedSize);

    //
    //  Return processing stages for outgoing attachment.
    //  Note, for incoming attachment the returned value is always UploadStage::Uploaded.
    //
    UploadStage uploadStage() const noexcept;

    //
    //  Set processing stages for outgoing attachment.
    //  Note, for incoming attachment this operation has no sense, but if called behavior is undefined.
    //
    void setUploadStage(UploadStage uploadStage);

    //
    //  Return processing stages for incoming attachment.
    //  Note, for outgoing attachment the returned value is usually DownloadStage::Decrypted,
    //  but if message was cached and initial attachment was deleted, the it can be any.
    //
    DownloadStage downloadStage() const noexcept;

    //
    //  Set processing stages for incoming attachment.
    //  Note, for outgoing attachment this operation can be used for message content was deleted and we want to
    //  download it from a cloud.
    //
    void setDownloadStage(DownloadStage downloadStage);

    //
    //  Return JSON string with extra attachment attributes.
    //
    virtual QString extrasToJson() const;

private:
    AttachmentId m_id;
    QString m_fileName;
    QString m_fingerprint;
    quint64 m_size = 0;
    QUrl m_remoteUrl;
    QString m_localPath;
    quint64 m_encryptedSize = 0;
    quint64 m_processedSize = 0;
    UploadStage m_uploadStage;
    DownloadStage m_downloadStage;

};
} // namespace vm

#endif // VM_MESSAGE_CONTENT_ATTACHMENT_H
