//  Copyright (C) 2015-2021 Virgil Security, Inc.
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

#ifndef VM_CLOUD_FILE_H
#define VM_CLOUD_FILE_H

#include <memory>

#include <QByteArray>
#include <QDateTime>

#include "CloudFileId.h"
#include "CloudFileUpdateSource.h"
#include "UserId.h"

namespace vm
{
class CloudFile
{
public:
    CloudFileId id() const noexcept;
    void setId(const CloudFileId &id);
    CloudFileId parentId() const noexcept;
    void setParentId(const CloudFileId &id);
    QString name() const noexcept;
    void setName(const QString &name);
    bool isFolder() const noexcept;
    void setIsFolder(bool isFolder);
    QString type() const;
    void setType(const QString &type);
    quint64 size() const noexcept;
    void setSize(quint64 size);
    QDateTime createdAt() const noexcept;
    void setCreatedAt(const QDateTime &dateTime);
    QDateTime updatedAt() const noexcept;
    void setUpdatedAt(const QDateTime &dateTime);
    UserId updatedBy() const noexcept;
    void setUpdatedBy(const UserId &userId);
    QByteArray encryptedKey() const noexcept;
    void setEncryptedKey(const QByteArray &key);
    QByteArray publicKey() const noexcept;
    void setPublicKey(const QByteArray &key);
    QString localPath() const noexcept;
    void setLocalPath(const QString &path);
    QString fingerprint() const noexcept;
    void setFingerprint(const QString &fingerprint);

    void update(const CloudFile &file, CloudFileUpdateSource source);

private:
    CloudFileId m_id;
    CloudFileId m_parentId;
    QString m_name;
    bool m_isFolder;
    QString m_type;
    quint64 m_size;
    QDateTime m_createdAt;
    QDateTime m_updatedAt;
    UserId m_updatedBy;
    QByteArray m_encryptedKey;
    QByteArray m_publicKey;
    QString m_localPath;
    QString m_fingerprint;
};

using CloudFileHandler = std::shared_ptr<const CloudFile>;
using ModifiableCloudFileHandler = std::shared_ptr<CloudFile>;
using CloudFiles = std::vector<CloudFileHandler>;
using ModifiableCloudFiles = std::vector<ModifiableCloudFileHandler>;
}

#endif // VM_CLOUD_FILE_H
