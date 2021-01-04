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

#include "models/ListModel.h"

#include "models/ListProxyModel.h"
#include "models/ListSelectionModel.h"

using namespace vm;

ListModel::ListModel(QObject *parent, bool createProxy)
    : QAbstractListModel(parent)
    , m_proxy(createProxy ? new ListProxyModel(this) : nullptr)
    , m_selection(new ListSelectionModel(this))
{
    connect(m_selection, &ListSelectionModel::changed, this, &ListModel::onSelectionChanged);
}

QString ListModel::filter() const
{
    return m_filter;
}

void ListModel::setFilter(const QString &filter)
{
    if (m_filter == filter) {
        return;
    }
    m_proxy->setFilterFixedString(filter);
    m_filter = filter;
    emit filterChanged(filter);
}

void ListModel::clearFilter()
{
    setFilter(QString());
}

QModelIndex ListModel::sourceIndex(const int proxyRow) const
{
    return proxy()->mapToSource(proxy()->index(proxyRow, 0));
}

QModelIndex ListModel::proxyIndex(const int sourceRow) const
{
    return proxy()->mapFromSource(index(sourceRow));
}

QVariant ListModel::data(const QModelIndex &index, int role) const
{
    if (role == IsSelectedRole) {
        return m_selection->isSelected(index);
    }
    return QVariant();
}

QHash<int, QByteArray> ListModel::roleNames() const
{
    return {
        { IsSelectedRole, "isSelected" }
    };
}

ListModel::RoleNames ListModel::unitedRoleNames(const ListModel::RoleNames &a, const ListModel::RoleNames &b)
{
    auto result = a;
    for (auto k : b.keys()) {
        result.insert(k, b.value(k));
    }
    return result;
}

const ListProxyModel *ListModel::proxy() const
{
    return m_proxy;
}

ListProxyModel *ListModel::proxy()
{
    return m_proxy;
}

void ListModel::setProxy(ListProxyModel *proxy)
{
    m_proxy = proxy;
    emit proxyChanged(proxy);
}

const ListSelectionModel *ListModel::selection() const
{
    return m_selection;
}

ListSelectionModel *ListModel::selection()
{
    return m_selection;
}

void ListModel::onSelectionChanged(const QList<QModelIndex> &indices)
{
    for (auto &i : indices) {
        emit dataChanged(i, i, { IsSelectedRole });
    }
}
