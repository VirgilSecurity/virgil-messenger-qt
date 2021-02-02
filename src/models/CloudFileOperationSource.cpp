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

#include "CloudFileOperationSource.h"

#include "CloudFile.h"

using namespace vm;

CloudFileOperationSource::CloudFileOperationSource(Type type)
    : OperationSource()
    , m_type(type)
{
}

CloudFileOperationSource::Type CloudFileOperationSource::type() const
{
    return m_type;
}

CloudFileHandler CloudFileOperationSource::folder() const
{
    return m_folder;
}

void CloudFileOperationSource::setFolder(const CloudFileHandler &folder)
{
    m_folder = folder;
}

CloudFiles CloudFileOperationSource::files() const
{
    return m_files;
}

void CloudFileOperationSource::setFiles(const CloudFiles &files)
{
    m_files = files;
}

QString CloudFileOperationSource::filePath() const
{
    return m_filePath;
}

void CloudFileOperationSource::setFilePath(const QString &path)
{
    m_filePath = path;
}

QString CloudFileOperationSource::name() const
{
    return m_name;
}

void CloudFileOperationSource::setName(const QString &name)
{
    m_name = name;
}

OperationSource::PostFunction CloudFileOperationSource::postFunction() const
{
    return m_postFunction;
}

void CloudFileOperationSource::setPostFunction(const PostFunction &func)
{
    m_postFunction = func;
}

bool CloudFileOperationSource::isValid() const
{
    return true;
}

QString CloudFileOperationSource::toString() const
{
    const QString str("CloudFileOperationSource(%1)");
    switch (m_type) {
    case Type::CreateFolder:
        return str.arg(QLatin1String("CreateFolder"));
    case Type::Upload:
        return str.arg(QLatin1String("Upload"));
    case Type::Download:
        return str.arg(QLatin1String("Download"));
    case Type::Delete:
        return str.arg(QLatin1String("Delete"));
    default:
        return str.arg(QLatin1String("???"));
    }
}
