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
    connect(this, &AttachmentsTable::createAttachment, this, &AttachmentsTable::processCreateAttachment);
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

void AttachmentsTable::processCreateAttachment(const Attachment &attachment)
{
    ScopedConnection connection(*database());
    const DatabaseUtils::BindValues values {
        { ":id", attachment.id },
        { ":messageId", attachment.messageId },
        { ":type", static_cast<int>(attachment.type) },
        { ":status", static_cast<int>(attachment.status) },
        { ":filename", attachment.fileName },
        { ":size", attachment.size },
        { ":localPath", attachment.localPath },
        { ":url", attachment.url },
        { ":extras", Utils::extrasToJson(attachment.extras, attachment.type) }
    };
    const auto query = DatabaseUtils::readExecQuery(database(), QLatin1String("insertAttachment"), values);
    if (!query) {
        qCCritical(lcDatabase) << "AttachmentsTable::processCreateAttachment error";
        emit errorOccurred(tr("Failed to insert attachment"));
        return;
    }
    qCDebug(lcDatabase) << "Attachment was inserted into table" << attachment.localPath;
}
