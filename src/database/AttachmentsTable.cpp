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

#include "database/AttachmentsTable.h"

#include "Utils.h"
#include "database/core/Database.h"
#include "database/core/DatabaseUtils.h"
#include "MessageContentType.h"

using namespace vm;
using Self = AttachmentsTable;

Self::AttachmentsTable(Database *database)
    : DatabaseTable(QLatin1String("attachments"), database)
{
    connect(this, &Self::addAttachment, this, &Self::onAddAttachment);
    connect(this, &Self::updateAttachment, this, &Self::onUpdateAttachment);
}

bool Self::create()
{
    if (DatabaseUtils::readExecQueries(database(), QLatin1String("createAttachments"))) {
        qCDebug(lcDatabase) << "Attachments table was created";
        return true;
    }
    qCCritical(lcDatabase) << "Failed to create attachments table";
    return false;
}

void Self::onAddAttachment(MessageHandler message)
{
    const auto attachment = message->contentAsAttachment();

    ScopedConnection connection(*database());
    const auto extrasJson = attachment->extrasToJson(true);
    const DatabaseUtils::BindValues values {
        { ":id", QString(attachment->id()) },
        { ":messageId", QString(message->id()) },
        { ":type",  MessageContentTypeToString(message->contentType()) },
        { ":fingerprint", attachment->fingerprint() },
        { ":decryptionKey", attachment->decryptionKey() },
        { ":signature", attachment->signature() },
        { ":filename", attachment->fileName() },
        { ":localPath", attachment->localPath() },
        { ":url", attachment->remoteUrl() },
        { ":size", attachment->size() },
        { ":encryptedSize", attachment->encryptedSize() },
        { ":extras", extrasJson },
        { ":uploadStage", MessageContentUploadStageToString(attachment->uploadStage()) },
        { ":downloadStage", MessageContentDownloadStageToString(attachment->downloadStage()) },
    };
    const auto query = DatabaseUtils::readExecQuery(database(), QLatin1String("insertAttachment"), values);
    if (!query) {
        qCCritical(lcDatabase) << "AttachmentsTable::onAddAttachment error";
        emit errorOccurred(tr("Failed to insert attachment"));
        return;
    }
    qCDebug(lcDatabase) << "Attachment was inserted into table: " << attachment->id();
}

static std::tuple<QString, DatabaseUtils::BindValues> createDatabaseBindings(const MessageUpdate &attachmentUpdate) {
    if (const auto arg = std::get_if<MessageAttachmentUploadStageUpdate>(&attachmentUpdate)) {
        return {"updateAttachmentUploadStage", {
            { ":id", QString(arg->attachmentId) },
            { ":uploadStage", MessageContentUploadStageToString(arg->uploadStage) }
        }};
    }

    if (const auto arg = std::get_if<MessageAttachmentDownloadStageUpdate>(&attachmentUpdate)) {
        return {"updateAttachmentDownloadStage", {
            { ":id", QString(arg->attachmentId) },
            { ":downloadStage", MessageContentDownloadStageToString(arg->downloadStage) }
        }};
    }

    if (const auto arg = std::get_if<MessageAttachmentFingerprintUpdate>(&attachmentUpdate)) {
        return {"updateAttachmentFingerprint", {
            { ":id", QString(arg->attachmentId) },
            { ":fingerprint", arg->fingerprint }
        }};
    }

    if (const auto arg = std::get_if<MessageAttachmentRemoteUrlUpdate>(&attachmentUpdate)) {
        return {"updateAttachmentRemoteUrl", {
            { ":id", QString(arg->attachmentId) },
            { ":url",  arg->remoteUrl }
        }};
    }

    if (const auto arg = std::get_if<MessageAttachmentEncryptionUpdate>(&attachmentUpdate)) {
        return {"updateAttachmentEncryption", {
            { ":id", QString(arg->attachmentId) },
            { ":encryptedSize", arg->encryptedSize },
            { ":decryptionKey", arg->decryptionKey },
            { ":signature", arg->signature }
        }};
    }

    if (const auto arg = std::get_if<MessageAttachmentLocalPathUpdate>(&attachmentUpdate)) {
        return {"updateAttachmentLocalPath", {
            { ":id", QString(arg->attachmentId) },
            { ":localPath", arg->localPath }
        }};
    }

    if (const auto arg = MessageUpdateToAttachmentExtrasUpdate(attachmentUpdate)) {
        return {"updateAttachmentExtras", {
            { ":id", QString(arg->attachmentId) },
            { ":extras", arg->extrasToJson() }
        }};
    }

    return {};
}

void Self::onUpdateAttachment(const MessageUpdate &attachmentUpdate) {
    auto [queryId, bindValues] = createDatabaseBindings(attachmentUpdate);
    if (queryId.isEmpty()) {
        // Nothing to update.
        return;
    }

    const auto query = DatabaseUtils::readExecQuery(database(), queryId, bindValues);
    if (query) {
        qCDebug(lcDatabase) << "Attachment was updated" << bindValues.front().second << bindValues.back();
    } else {
        qCCritical(lcDatabase) << "Self::onUpdateAttachment error";
        emit errorOccurred(tr("Failed to update attachment"));
    }
}
