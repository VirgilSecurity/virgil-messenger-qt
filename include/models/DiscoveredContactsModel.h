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

#include "ListModel.h"
#include "VSQCommon.h"

namespace vm
{
class Validator;

class DiscoveredContactsModel : public ListModel
{
    Q_OBJECT
    Q_PROPERTY(bool newContactFiltered MEMBER m_newContactFiltered NOTIFY newContactFilteredChanged)

public:
    enum Roles
    {
        NameRole = Qt::UserRole,
        DetailsRole,
        AvatarUrlRole,
        LastSeenActivityRole,
        FilterRole
    };

    DiscoveredContactsModel(Validator *validator, QObject *parent);

    void reload();

signals:
    void contactsPopulated(const Contacts &contacts, QPrivateSignal);
    void newContactFilteredChanged(const bool filtered);

private:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setContacts(const Contacts &contacts);
    void checkNewContactFiltered();

    Validator *m_validator;
    Contacts m_contacts;
    bool m_newContactFiltered = false;
};
}

#endif // VM_DISCOVEREDCONTACTSMODEL_H
