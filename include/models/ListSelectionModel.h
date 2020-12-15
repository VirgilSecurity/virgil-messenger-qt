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

#ifndef VM_LISTSELECTIONMODEL_H
#define VM_LISTSELECTIONMODEL_H

#include <QItemSelectionModel>

namespace vm
{
class ListModel;

class ListSelectionModel : public QItemSelectionModel
{
    Q_OBJECT
    Q_PROPERTY(bool hasSelection READ hasSelection NOTIFY hasSelectionChanged)
    Q_PROPERTY(bool multiSelect MEMBER m_multiSelect WRITE setMultiSelect NOTIFY multiSelectChanged)

public:
    explicit ListSelectionModel(ListModel *source);

    Q_INVOKABLE void setSelected(const QVariant &proxyRow, bool selected);
    Q_INVOKABLE void toggle(const QVariant &proxyRow);
    void toggle(const QModelIndex &sourceIndex);
    Q_INVOKABLE void clear();

    void setMultiSelect(const bool multiSelect);

    bool hasSelection() const;
    std::vector<QVariant> items() const;

signals:
    void changed(const QList<QModelIndex> &indices);
    void hasSelectionChanged(const bool changed);
    void multiSelectChanged(const bool multiselect);

private:
    void onChanged(const QItemSelection &selected, const QItemSelection &deselected);

    ListModel *m_sourceModel;
    bool m_hasSelection = false;
    bool m_multiSelect = false;
};
}

#endif // VM_LISTSELECTIONMODEL_H
