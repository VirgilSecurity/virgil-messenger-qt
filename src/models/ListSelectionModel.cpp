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

#include "models/ListSelectionModel.h"

#include "models/ListModel.h"

using namespace vm;

ListSelectionModel::ListSelectionModel(ListModel *source)
    : QItemSelectionModel(source)
    , m_sourceModel(source)
{
    qRegisterMetaType<ListSelectionModel *>("ListSelectionModel*");

    setModel(source);

    connect(this, &QItemSelectionModel::selectionChanged, this, &ListSelectionModel::onChanged);
}

void ListSelectionModel::setSelected(const QVariant &proxyRow, bool selected)
{
    if (!m_multiSelect && selected) {
        clear();
    }
    const auto index = m_sourceModel->sourceIndex(proxyRow.toInt());
    const auto flag = selected ? QItemSelectionModel::Select : QItemSelectionModel::Deselect;
    QItemSelectionModel::select(index, flag);
}

void ListSelectionModel::toggle(const QVariant &proxyRow)
{
    const auto sourceIndex = m_sourceModel->sourceIndex(proxyRow.toInt());
    toggle(sourceIndex);
}

void ListSelectionModel::toggle(const QModelIndex &sourceIndex)
{
    if (!m_multiSelect && !isSelected(sourceIndex)) {
        clear();
    }
    QItemSelectionModel::select(sourceIndex, QItemSelectionModel::Toggle);
}

void ListSelectionModel::clear()
{
    QItemSelectionModel::clearSelection();
}

void ListSelectionModel::setMultiSelect(const bool multiSelect)
{
    if (multiSelect == m_multiSelect) {
        return;
    }
    m_multiSelect = multiSelect;
    emit multiSelectChanged(multiSelect);
}

bool ListSelectionModel::hasSelection() const
{
    return m_hasSelection;
}

void ListSelectionModel::onChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    emit changed(selected.indexes() + deselected.indexes());
    if (m_hasSelection != QItemSelectionModel::hasSelection()) {
        m_hasSelection = !m_hasSelection;
        emit hasSelectionChanged(m_hasSelection);
    }
}
