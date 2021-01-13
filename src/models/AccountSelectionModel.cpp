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

#include "models/AccountSelectionModel.h"

#include "Settings.h"

using namespace vm;

AccountSelectionModel::AccountSelectionModel(Settings *settings, QObject *parent)
    : ListModel(parent)
    , m_settings(settings)
{
    qRegisterMetaType<AccountSelectionModel *>("AccountSelectionModel*");

    connect(m_settings, &Settings::usersListChanged, this, &AccountSelectionModel::reload);
}

int AccountSelectionModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    const auto usersList = m_settings->usersList();
    return (usersList.size() + m_chunkSize - 1) / m_chunkSize;
}

QVariant AccountSelectionModel::data(const QModelIndex &index, int role) const
{
    Q_UNUSED(role)
    const auto usersList = m_settings->usersList();
    const auto group = index.row();
    const auto start(m_chunkSize * group);
    const auto end(qMin(m_chunkSize * (group + 1), usersList.size()));
    return QStringList(usersList.cbegin() + start, usersList.cbegin() + end);
}

QHash<int, QByteArray> AccountSelectionModel::roleNames() const
{
    QHash<int, QByteArray> names;
    names[Qt::DisplayRole] = "modelData";
    return names;
}

void AccountSelectionModel::reload()
{
    beginResetModel();
    endResetModel();
}
