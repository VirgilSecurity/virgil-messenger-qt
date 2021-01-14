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

#ifndef VM_CLOUDFILESMODEL_H
#define VM_CLOUDFILESMODEL_H

#include <QDateTime>
#include <QFileInfoList>
#include <QTimer>

#include "CloudFile.h"
#include "ListModel.h"

class Settings;

namespace vm
{
class CloudFilesModel : public ListModel
{
    Q_OBJECT

public:
    enum Roles
    {
        FilenameRole = Qt::UserRole,
        IsDirRole,
        DisplayDateTimeRole,
        DisplayFileSize,
        SortRole
    };

    CloudFilesModel(const Settings *settings, QObject *parent);

    void setDirectory(const CloudFileHandler &cloudDir);
    void setEnabled(bool enabled);

    CloudFileHandler getFile(const int proxyRow) const;
    CloudFiles getSelectedFiles() const;

signals:
    void listReady(const QFileInfoList &list, QPrivateSignal);

private:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setList(const QFileInfoList &list);
    void invalidateDateTime();

    const Settings *m_settings;
    CloudFiles m_list;
    QDateTime m_now;
    QTimer m_updateTimer;
    int m_debugCounter = 0;
};
}

#endif // VM_CLOUDFILESMODEL_H
