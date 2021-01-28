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

#ifndef VM_CLOUDFILESUPLOADER_H
#define VM_CLOUDFILESUPLOADER_H

#include <QObject>

namespace vm
{
class CloudFilesUploader : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int currentIndex READ currentIndex NOTIFY currentIndexChanged)
    Q_PROPERTY(qint64 currentProcessedBytes READ currentProcessedBytes WRITE setCurrentProcessedBytes NOTIFY currentProcessedBytesChanged)
    Q_PROPERTY(qint64 currentTotalBytes READ currentTotalBytes NOTIFY currentTotalBytesChanged)
    Q_PROPERTY(QStringList fileNames READ fileNames NOTIFY fileNamesChanged)

public:
    CloudFilesUploader(QObject *parent);

    int currentIndex() const;
    qint64 currentProcessedBytes() const;
    qint64 currentTotalBytes() const;
    QStringList fileNames() const;

    void setCurrentIndex(const int index);
    void setCurrentProcessedBytes(const qint64 bytes);
    void setCurrentTotalBytes(const qint64 bytes);

signals:
    void currentIndexChanged(const int &index);
    void currentProcessedBytesChanged(const qint64 bytes);
    void currentTotalBytesChanged(const qint64 bytes);
    void fileNamesChanged(const QStringList &fileNames);

private:
    QStringList m_fileNames;
    int m_currentIndex;
    int m_currentProcessedBytes;
    int m_currentTotalBytes;
};
}

#endif // VM_CLOUDFILESUPLOADER_H
