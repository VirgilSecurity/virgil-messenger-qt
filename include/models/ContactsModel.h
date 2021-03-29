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

namespace vm {
class ContactAvatarLoader;
class ContactsProxyModel;

class ContactsModel : public ListModel
{
    Q_OBJECT

public:
    enum Roles {
        IdRole = Qt::UserRole,
        UsernameRole,
        NameRole,
        DisplayNameRole,
        DetailsRole,
        AvatarUrlRole,
        FilterRole,
        SortRole,
        //
        UserRole
    };

    explicit ContactsModel(QObject *parent, bool createProxy = true);

    void setContacts(Contacts contacts);
    [[nodiscard]] Contacts getContacts() const;
    [[nodiscard]] int getContactsCount() const;

    [[nodiscard]] ContactHandler createContact(const QString &username) const;
    [[nodiscard]] ContactHandler getContact(int row) const;
    [[nodiscard]] bool hasContact(const QString &contactUsername) const;

    void addContact(ContactHandler contact);
    void removeContact(const QString &contactUsername);
    void removeContactByRow(int row);
    void removeContactsByRows(int startRow, int endRow);
    void updateContact(ContactHandler contact, int row);

    [[nodiscard]] Contacts selectedContacts() const;

    Q_INVOKABLE virtual void toggleByUsername(const QString &contactUsername);

signals:
    void avatarUrlNotFound(const QString &contactName, QPrivateSignal) const;

protected:
    [[nodiscard]] int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    [[nodiscard]] QVariant data(const QModelIndex &index, int role) const override;
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

    [[nodiscard]] QModelIndex findByUsername(const QString &contactUsername) const;
    [[nodiscard]] QModelIndex findByUserId(const UserId &userId) const;

private:
    void loadAvatarUrl(const QString &contactUsername);
    void setAvatarUrl(const ContactHandler &contact, const QUrl &url);

    Contacts m_contacts;
    ContactAvatarLoader *m_avatarLoader;
};
} // namespace vm

#endif // VM_CONTACTSMODEL_H
