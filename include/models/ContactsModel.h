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

#ifndef VM_CONTACTSMODEL_H
#define VM_CONTACTSMODEL_H

#include "Contact.h"
#include "ListModel.h"

#include <QUrl>

namespace vm
{
class ContactAvatarLoader;
class ContactsProxyModel;

class ContactsModel : public ListModel
{
    Q_OBJECT

public:
    enum Roles
    {
        IdRole = Qt::UserRole,
        UsernameRole,
        NameRole,
        DetailsRole,
        AvatarUrlRole,
        FilterRole,
        SortRole,
        //
        UserRole
    };

    explicit ContactsModel(QObject *parent, bool createProxy = true);

    void setContacts(const Contacts &contacts);
    const Contacts &getContacts() const;
    int getContactsCount() const;

    Contact createContact(const QString &username) const;
    const Contact &getContact(const int row) const;
    bool hasContact(const QString &contactId) const;

    void addContact(const Contact &contact);
    void removeContact(const QString &contactId);
    void removeContactsByRows(const int startRow, const int endRow);
    void updateContact(const Contact &contact, int row);

signals:
    void avatarUrlNotFound(const QString &contactId, QPrivateSignal) const;

protected:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    std::optional<int> findRowByContactName(const QString &contactId) const;

private:
    void loadAvatarUrl(const QString &contactId);
    void setAvatarUrl(const Contact &contact, const QUrl &url);

    Contacts m_contacts;
    ContactAvatarLoader *m_avatarLoader;
};
}

#endif // VM_CONTACTSMODEL_H
