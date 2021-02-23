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

#ifndef VM_LISTMODEL_H
#define VM_LISTMODEL_H

#include <QAbstractListModel>

namespace vm {
class ListProxyModel;
class ListSelectionModel;

class ListModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int count MEMBER m_count NOTIFY countChanged)
    Q_PROPERTY(ListProxyModel *proxy MEMBER m_proxy NOTIFY proxyChanged)
    Q_PROPERTY(ListSelectionModel *selection MEMBER m_selection CONSTANT)
    Q_PROPERTY(QString filter MEMBER m_filter WRITE setFilter NOTIFY filterChanged)

public:
    enum Roles { IsSelectedRole = Qt::CheckStateRole };

    using RoleNames = QHash<int, QByteArray>;

    explicit ListModel(QObject *parent, bool createProxy = true);

    QString filter() const;
    void setFilter(const QString &filter);
    void clearFilter();

    QModelIndex sourceIndex(const int proxyRow) const;
    QModelIndex proxyIndex(const int sourceRow) const;

    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;
    static RoleNames unitedRoleNames(const RoleNames &a, const RoleNames &b);

    const ListProxyModel *proxy() const;
    ListProxyModel *proxy();
    void setProxy(ListProxyModel *proxy);

    const ListSelectionModel *selection() const;
    ListSelectionModel *selection();

signals:
    void countChanged(int count);
    void filterChanged(const QString &filter);
    void proxyChanged(ListProxyModel *proxy);

private:
    void onSelectionChanged(const QList<QModelIndex> &indices);
    void onRowCountChanged();

    int m_count = 0;
    ListProxyModel *m_proxy;
    ListSelectionModel *m_selection;
    QString m_filter;
};
} // namespace vm

#endif // VM_LISTMODEL_H
