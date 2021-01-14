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

#ifndef VM_CLOUDFILE_H
#define VM_CLOUDFILE_H

#include <QDateTime>
#include <QFileInfo>
#include <QString>

namespace vm
{
class CloudFile
{
public:
    CloudFile() = default;
    explicit CloudFile(const QFileInfo &info);
    explicit CloudFile(const QString &path);
    CloudFile(const QString &name, const bool isDirectory, const qsizetype size, const QDateTime &createdAt);
    virtual ~CloudFile() noexcept = default;

    QString name() const noexcept;
    bool isDirectory() const noexcept;
    qsizetype size() const noexcept;
    QDateTime createdAt() const noexcept;

    void setLocalPath(const QString &path);
    QString localPath() const;

    QString relativePath(const CloudFile &dir) const;
    QString parentPath() const;
    void cdUp();
    CloudFile child(const QString &name) const;

private:
    QString m_name;
    bool m_isDirectory = false;
    qsizetype m_size = 0;
    QDateTime m_createdAt;
    QString m_localPath;
};

using CloudFileHandler = std::shared_ptr<const CloudFile>;
using CloudFiles = std::vector<CloudFileHandler>;

}

#endif // VM_CLOUDFILE_H
