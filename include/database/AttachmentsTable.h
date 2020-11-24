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

#ifndef VM_ATTACHMENTSTABLE_H
#define VM_ATTACHMENTSTABLE_H

#include "core/DatabaseTable.h"

namespace vm
{
class AttachmentsTable : public DatabaseTable
{
    Q_OBJECT

public:
    explicit AttachmentsTable(Database *database);

signals:
    void createAttachment(const Attachment &attachment);
    void updateStatus(const Attachment::Id &attachmentId, const Attachment::Status &status);
    void updateUrl(const Attachment::Id &attachmentId, const QUrl &url);
    void updateLocalPath(const Attachment::Id &attachmentId, const QString &localPath);
    void updateFingerprint(const Attachment::Id &attachmentId, const QString &fingerprint);
    void updateExtras(const Attachment::Id &attachmentId, const Attachment::Type &type, const QVariant &extras);
    void updateEncryptedSize(const Attachment::Id &attachmentId, const DataSize &size);

    void errorOccurred(const QString &errorText);

private:
    bool create() override;

    void onCreateAttachment(const Attachment &attachment);
    void onUpdateStatus(const Attachment::Id &attachmentId, const Attachment::Status &status);
    void onUpdateUrl(const Attachment::Id &attachmentId, const QUrl &url);
    void onUpdateLocalPath(const Attachment::Id &attachmentId, const QString &localPath);
    void onUpdateFingerprint(const Attachment::Id &attachmentId, const QString &fingerprint);
    void onUpdateExtras(const Attachment::Id &attachmentId, const Attachment::Type &type, const QVariant &extras);
    void onUpdateEncryptedSize(const Attachment::Id &attachmentId, const DataSize &size);
};
}

#endif // VM_ATTACHMENTSTABLE_H
