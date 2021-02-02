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

#ifndef VM_FILESPROGRESSMODEL_H
#define VM_FILESPROGRESSMODEL_H

#include "CloudFilesUpdate.h"
#include "ListModel.h"

namespace vm
{
class FilesProgressModel : public ListModel
{
    Q_OBJECT
    Q_PROPERTY(int activeCount MEMBER m_activeCount NOTIFY activeCountChanged)

public:
    using TransferType = TransferCloudFileUpdate::Type;

    explicit FilesProgressModel(QObject *parent);
    ~FilesProgressModel() override;

    void add(const QString &id, const QString &name, const quint64 bytesTotal, const TransferType transferType);
    void setProgress(const QString &id, const quint64 bytesLoaded, const quint64 bytesTotal);
    void remove(const QString &id);
    void markAsFailed(const QString &id);

signals:
    void interrupt(const QString &id);
    void activeCountChanged(const int count);

private:
    enum Roles
    {
        NameRole = Qt::UserRole,
        BytesLoadedRole,
        BytesTotalRole,
        DisplayProgressRole,
        IsFailedRole,
        IsCompletedRole
    };

    struct Item
    {
        QString id;
        QString name;
        quint64 bytesLoaded = 0;
        quint64 bytesTotal = 0;
        TransferType transferType = TransferType::Upload;
        bool isFailed = false;
    };

    QModelIndex findById(const QString &id) const;
    static QString displayedProgress(const Item &item);
    static bool isItemCompleted(const Item &item);

    int calculateActiveCount() const;
    void setActiveCount(const int activeCount);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    QList<Item> m_items;
    int m_activeCount = 0;
};
} // namespace vm

#endif // VM_FILESPROGRESSMODEL_H
