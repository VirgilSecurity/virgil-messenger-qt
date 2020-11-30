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

#include "models/FileCloudUploader.h"

#include <QObject>

using namespace vm;

FileCloudUploader::FileCloudUploader(QObject *parent)
    : QObject(parent)
    , m_fileNames({"First", "Second", "Third"})
{
}

int FileCloudUploader::currentIndex() const
{
    return m_currentIndex;
}

int FileCloudUploader::currentProcessedBytes() const
{
    return m_currentProcessedBytes;
}

int FileCloudUploader::currentTotalBytes() const
{
    return m_currentTotalBytes;
}

void FileCloudUploader::setCurrentIndex(const int &index)
{
    if (m_currentIndex != index) {
        m_currentIndex = index;
        emit currentIndexChanged(m_currentIndex);
    }
}

void FileCloudUploader::setCurrentProcessedBytes(const int &bytes)
{
    if (m_currentProcessedBytes != bytes) {
        m_currentProcessedBytes = bytes;
        emit currentProcessedBytesChanged(m_currentProcessedBytes);
    }
}

void FileCloudUploader::setCurrentTotalBytes(const int &bytes)
{
    if (m_currentTotalBytes != bytes) {
        m_currentTotalBytes = bytes;
        emit currentTotalBytesChanged(m_currentTotalBytes);
    }
}

