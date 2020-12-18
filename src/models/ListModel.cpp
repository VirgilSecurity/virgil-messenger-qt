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

#include <QSortFilterProxyModel>

#include "models/ListSelectionModel.h"

using namespace vm;

ListModel::ListModel(QObject *parent)
    : QAbstractListModel(parent)
    , m_proxy(new QSortFilterProxyModel(this))
    , m_selection(new ListSelectionModel(this))
{
    qRegisterMetaType<QSortFilterProxyModel *>("QSortFilterProxyModel*");
    qRegisterMetaType<ListSelectionModel *>("ListSelectionModel*");

    m_proxy->setSourceModel(this);
    m_proxy->setFilterKeyColumn(0);
    m_proxy->setFilterCaseSensitivity(Qt::CaseInsensitive);

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

QHash<int, QByteArray> ListModel::unitedRoleNames(const QHash<int, QByteArray> &names) const
{
    auto result = ListModel::roleNames();
    for (auto k : names.keys()) {
        result.insert(k, names.value(k));
    }
    return result;
}

const QSortFilterProxyModel *ListModel::proxy() const
{
    return m_proxy;
}

QSortFilterProxyModel *ListModel::proxy()
{
    return m_proxy;
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
