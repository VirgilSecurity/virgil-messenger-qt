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

using namespace vm;

AttachmentsTable::AttachmentsTable(Database *database)
    : DatabaseTable(QLatin1String("attachments"), database)
{
    connect(this, &AttachmentsTable::createAttachment, this, &AttachmentsTable::onCreateAttachment);
    connect(this, &AttachmentsTable::updateStatus, this, &AttachmentsTable::onUpdateStatus);
    connect(this, &AttachmentsTable::updateUrl, this, &AttachmentsTable::onUpdateUrl);
    connect(this, &AttachmentsTable::updateExtras, this, &AttachmentsTable::onUpdateExtras);
    connect(this, &AttachmentsTable::updateLocalPath, this, &AttachmentsTable::onUpdateLocalPath);
    connect(this, &AttachmentsTable::updateFingerprint, this, &AttachmentsTable::onUpdateFingerprint);
    connect(this, &AttachmentsTable::updateEncryptedSize, this, &AttachmentsTable::onUpdateEncryptedSize);
}

bool AttachmentsTable::create()
{
    if (DatabaseUtils::readExecQueries(database(), QLatin1String("createAttachments"))) {
        qCDebug(lcDatabase) << "Attachments table was created";
        return true;
    }
    qCCritical(lcDatabase) << "Unable to create attachments table";
    return false;
}

void AttachmentsTable::onCreateAttachment(const Attachment &attachment)
{
    ScopedConnection connection(*database());
    const auto extrasJson = Utils::extrasToJson(attachment.extras, attachment.type, false);
    const DatabaseUtils::BindValues values {
        { ":id", attachment.id },
        { ":messageId", attachment.messageId },
        { ":type", static_cast<int>(attachment.type) },
        { ":status", static_cast<int>(attachment.status) },
        { ":filename", attachment.fileName },
        { ":size", attachment.size },
        { ":localPath", attachment.localPath },
        { ":fingerprint", attachment.fingerprint },
        { ":url", attachment.url },
        { ":encryptedSize", attachment.encryptedSize },
        { ":extras", extrasJson }
    };
    const auto query = DatabaseUtils::readExecQuery(database(), QLatin1String("insertAttachment"), values);
    if (!query) {
        qCCritical(lcDatabase) << "AttachmentsTable::onCreateAttachment error";
        emit errorOccurred(tr("Failed to insert attachment"));
        return;
    }
    qCDebug(lcDatabase) << "Attachment was inserted into table" << attachment.id;
}

void AttachmentsTable::onUpdateStatus(const Attachment::Id &attachmentId, const Attachment::Status &status)
{
    ScopedConnection connection(*database());
    const DatabaseUtils::BindValues values {
        { ":id", attachmentId },
        { ":status", static_cast<int>(status) }
    };
    const auto query = DatabaseUtils::readExecQuery(database(), QLatin1String("updateAttachmentStatus"), values);
    if (query) {
        qCDebug(lcDatabase) << "Attachment status was updated" << attachmentId << "status" << status;
    }
    else {
        qCCritical(lcDatabase) << "AttachmentsTable::onUpdateStatus error";
        emit errorOccurred(tr("Failed to update attachment status"));
    }
}

void AttachmentsTable::onUpdateUrl(const Attachment::Id &attachmentId, const QUrl &url)
{
    ScopedConnection connection(*database());
    const DatabaseUtils::BindValues values {
        { ":id", attachmentId },
        { ":url", url }
    };
    const auto query = DatabaseUtils::readExecQuery(database(), QLatin1String("updateAttachmentUrl"), values);
    if (query) {
        qCDebug(lcDatabase) << "Attachment url was updated" << attachmentId << "url filename" << url.fileName();
    }
    else {
        qCCritical(lcDatabase) << "AttachmentsTable::onUpdateUrl error";
        emit errorOccurred(tr("Failed to update attachment url"));
    }
}

void AttachmentsTable::onUpdateLocalPath(const Attachment::Id &attachmentId, const QString &localPath)
{
    ScopedConnection connection(*database());
    const DatabaseUtils::BindValues values {
        { ":id", attachmentId },
        { ":localPath", localPath }
    };
    const auto query = DatabaseUtils::readExecQuery(database(), QLatin1String("updateAttachmentLocalPath"), values);
    if (query) {
        qCDebug(lcDatabase) << "Attachment localPath was updated" << attachmentId
                            << "localPath" << Utils::fileName(localPath);
    }
    else {
        qCCritical(lcDatabase) << "AttachmentsTable::onUpdateLocalPath error";
        emit errorOccurred(tr("Failed to update attachment localPath"));
    }
}

void AttachmentsTable::onUpdateFingerprint(const Attachment::Id &attachmentId, const QString &fingerprint)
{
    ScopedConnection connection(*database());
    const DatabaseUtils::BindValues values {
        { ":id", attachmentId },
        { ":fingerprint", fingerprint }
    };
    const auto query = DatabaseUtils::readExecQuery(database(), QLatin1String("updateAttachmentFingerprint"), values);
    if (query) {
        qCDebug(lcDatabase) << "Attachment fingerprint was updated" << attachmentId
                            << "fingerprint" << fingerprint;
    }
    else {
        qCCritical(lcDatabase) << "AttachmentsTable::onUpdateFingerprint error";
        emit errorOccurred(tr("Failed to update attachment fingerprint"));
    }
}

void AttachmentsTable::onUpdateExtras(const Attachment::Id &attachmentId, const Attachment::Type &type, const QVariant &extras)
{
    ScopedConnection connection(*database());
    const auto extrasJson = Utils::extrasToJson(QVariant::fromValue(extras), type, false);
    const DatabaseUtils::BindValues values {
        { ":id", attachmentId },
        { ":extras",  extrasJson}
    };
    const auto query = DatabaseUtils::readExecQuery(database(), QLatin1String("updateAttachmentExtras"), values);
    if (query) {
        qCDebug(lcDatabase) << "Attachment extras was updated" << attachmentId;
    }
    else {
        qCCritical(lcDatabase) << "AttachmentsTable::onUpdatePictureExtras error";
        emit errorOccurred(tr("Failed to update attachment extras"));
    }
}

void AttachmentsTable::onUpdateEncryptedSize(const Attachment::Id &attachmentId, const DataSize &encryptedSize)
{
    ScopedConnection connection(*database());
    const DatabaseUtils::BindValues values {
        { ":id", attachmentId },
        { ":encryptedSize", encryptedSize }
    };
    const auto query = DatabaseUtils::readExecQuery(database(), QLatin1String("updateAttachmentEncryptedSize"), values);
    if (query) {
        qCDebug(lcDatabase) << "Attachment encryptedSize was updated" << attachmentId << "encryptedSize" << encryptedSize;
    }
    else {
        qCCritical(lcDatabase) << "AttachmentsTable::onUpdateEncryptedSize error";
        emit errorOccurred(tr("Failed to update attachment encryptedSize"));
    }
}
