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

#include "CloudFile.h"

#include <QDir>

#include "Utils.h"

using namespace vm;

CloudFileId CloudFile::id() const noexcept
{
    return m_id;
}

void CloudFile::setId(const CloudFileId &id)
{
    m_id = id;
}

CloudFileId CloudFile::parentId() const noexcept
{
    return m_parentId;
}

void CloudFile::setParentId(const CloudFileId &id)
{
    m_parentId = id;
}

QString CloudFile::name() const noexcept
{
    return m_name;
}

void CloudFile::setName(const QString &name)
{
    m_name = name;
}

bool CloudFile::isFolder() const noexcept
{
    return m_isFolder;
}

void CloudFile::setIsFolder(const bool isFolder)
{
    m_isFolder = isFolder;
}

QString CloudFile::type() const
{
    return m_type;
}

void CloudFile::setType(const QString &type)
{
    m_type = type;
}

quint64 CloudFile::size() const noexcept
{
    return m_size;
}

void CloudFile::setSize(const quint64 size)
{
    m_size = size;
}

QDateTime CloudFile::createdAt() const noexcept
{
    return m_createdAt;
}

void CloudFile::setCreatedAt(const QDateTime &dateTime)
{
    m_createdAt = dateTime;
}

QDateTime CloudFile::updatedAt() const noexcept
{
    return m_updatedAt;
}

void CloudFile::setUpdatedAt(const QDateTime &dateTime)
{
    m_updatedAt = dateTime;
}

UserId CloudFile::updatedBy() const noexcept
{
    return m_updatedBy;
}

void CloudFile::setUpdatedBy(const UserId &userId)
{
    m_updatedBy = userId;
}

QByteArray CloudFile::encryptedKey() const noexcept
{
    return m_encryptedKey;
}

void CloudFile::setEncryptedKey(const QByteArray &key)
{
    m_encryptedKey = key;
}

QByteArray CloudFile::publicKey() const noexcept
{
    return m_publicKey;
}

void CloudFile::setPublicKey(const QByteArray &key)
{
    m_publicKey = key;
}

QString CloudFile::localPath() const noexcept
{
    return m_localPath;
}

void CloudFile::setLocalPath(const QString &path)
{
    m_localPath = path;
}

QString CloudFile::fingerprint() const noexcept
{
    return m_fingerprint;
}

void CloudFile::setFingerprint(const QString &fingerprint)
{
    m_fingerprint = fingerprint;
}

bool CloudFile::isRoot() const
{
    return m_isFolder && m_id == CloudFileId::root();
}

void CloudFile::update(const CloudFile &file, const CloudFileUpdateSource source)
{
    if (file.id() != id()) {
        throw std::logic_error("Failed to update cloud file: source id is different");
    }
    if (isFolder() != file.isFolder()) {
        throw std::logic_error("Failed to update cloud file: used file and folder");
    }
    setParentId(file.parentId()); // update always

    if (source == CloudFileUpdateSource::ListedParent) {
        setName(file.name());
        setCreatedAt(file.createdAt());
        setUpdatedAt(file.updatedAt());
        setUpdatedBy(file.updatedBy());
        setLocalPath(file.localPath());
        if (isFolder()) {
            setEncryptedKey(file.encryptedKey());
            setPublicKey(file.publicKey());
        }
    }
    else if (source == CloudFileUpdateSource::ListedChild) {
        setName(file.name());
        setCreatedAt(file.createdAt());
        setUpdatedAt(file.updatedAt());
        setUpdatedBy(file.updatedBy());
        setLocalPath(file.localPath());
        if (isFolder()) {
            if (file.updatedAt() > updatedAt()) {
                setEncryptedKey(QByteArray());
                setPublicKey(QByteArray());
            }
        }
        else {
            setType(file.type());
            setSize(file.size());
            if (file.updatedAt() > updatedAt()) {
                setFingerprint(QString());
            }
        }
    }
    else if (source == CloudFileUpdateSource::Download) {
        if (!isFolder()) {
            setFingerprint(file.fingerprint());
        }
    }
}
