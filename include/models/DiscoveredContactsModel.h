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

#ifndef VM_DISCOVEREDCONTACTSMODEL_H
#define VM_DISCOVEREDCONTACTSMODEL_H

#include "ContactsModel.h"

namespace vm
{
class Validator;

class DiscoveredContactsModel : public ContactsModel
{
    Q_OBJECT
    Q_PROPERTY(ContactsModel *selectedContacts MEMBER m_selectedContacts CONSTANT)

public:
    enum Roles {
        SectionRole = UserRole
    };

    DiscoveredContactsModel(Validator *validator, QObject *parent);

    void reload();

    Q_INVOKABLE void toggleById(const Contact::Id &contactId);

    int fixedContactsCount() const;

signals:
    void fixedContactsPopulated(const Contacts &contacts, QPrivateSignal);

private:
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    Contacts findContactsByFilter() const;
    void invalidateIsSelectedRole(int startRow, int endRow);
    void updateDiscoveredContacts();
    void updateSelectedContacts(const Contact::Id &contactId, const Contact *contact = nullptr);

    void onDeviceContactsPopulated(const Contacts &contacts);
    void onSelectionChanged(const QList<QModelIndex> &indices);

    Validator *m_validator;
    ContactsModel *m_selectedContacts;

    int m_fixedContactsCount = 0;
};
}

Q_DECLARE_METATYPE(vm::Contacts)

#endif // VM_DISCOVEREDCONTACTSMODEL_H
