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

#ifndef VM_FILECLOUDUPLOADER_H
#define VM_FILECLOUDUPLOADER_H

#include <QObject>

namespace vm
{
class FileCloudUploader : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex NOTIFY currentIndexChanged)
    Q_PROPERTY(int currentProcessedBytes READ currentProcessedBytes WRITE setCurrentProcessedBytes NOTIFY currentProcessedBytesChanged)
    Q_PROPERTY(int currentTotalBytes READ currentTotalBytes WRITE setCurrentTotalBytes NOTIFY currentTotalBytesChanged)

public:
    FileCloudUploader(QObject *parent);

    int currentIndex() const;
    int currentProcessedBytes() const;
    int currentTotalBytes() const;

public slots:
    void setCurrentIndex(const int &);
    void setCurrentProcessedBytes(const int &);
    void setCurrentTotalBytes(const int &);

signals:
    void currentIndexChanged(const int &index);
    void currentProcessedBytesChanged(const int &bytes);
    void currentTotalBytesChanged(const int &bytes);

private:
    QStringList m_fileNames;
    int m_currentIndex;
    int m_currentProcessedBytes;
    int m_currentTotalBytes;
};
}

#endif // VM_FILECLOUDUPLOADER_H
