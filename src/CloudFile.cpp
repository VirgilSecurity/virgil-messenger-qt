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

using namespace vm;

CloudFile::CloudFile(const QString &name, const bool isDirectory, const qsizetype size, const QDateTime &createdAt)
    : m_name(name)
    , m_isDirectory(isDirectory)
    , m_size(size)
    , m_createdAt(createdAt)
{
}

CloudFile::CloudFile(const QFileInfo &info)
    : m_name(info.fileName())
    , m_isDirectory(info.isDir())
    , m_size(info.size())
    , m_createdAt(info.fileTime(QFile::FileBirthTime))
{
    setLocalPath(info.absoluteFilePath());
}

CloudFile::CloudFile(const QString &path)
    : CloudFile(QFileInfo(path))
{
}

QString CloudFile::name() const noexcept
{
    return m_name;
}

bool CloudFile::isDirectory() const noexcept
{
    return m_isDirectory;
}

qsizetype CloudFile::size() const noexcept
{
    return m_size;
}

QDateTime CloudFile::createdAt() const noexcept
{
    return m_createdAt;
}

void CloudFile::setLocalPath(const QString &path)
{
    m_localPath = path;
}

QString CloudFile::localPath() const
{
    return m_localPath;
}

QString CloudFile::relativePath(const CloudFile &dir) const
{
    auto child(*this);
    QStringList parts;
    while (child.localPath() != dir.localPath()) {
        parts.push_front(child.name());
        child.cdUp();
    }
    return parts.join(QLatin1Char('/'));
}

QString CloudFile::parentPath() const
{
    return QFileInfo(m_localPath).absolutePath();
}

void CloudFile::cdUp()
{
    const auto dir = QFileInfo(m_localPath).dir();
    m_name = dir.dirName();
    m_isDirectory = true;
    m_size = 0;
    m_localPath = dir.absolutePath();
}

CloudFile CloudFile::child(const QString &name) const
{
    return CloudFile(QDir(localPath()).filePath(name));
}
