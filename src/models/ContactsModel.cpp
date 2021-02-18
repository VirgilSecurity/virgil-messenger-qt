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

#include "ContactsModel.h"

#include "ContactAvatarLoader.h"
#include "ContactsProxyModel.h"
#include "ListSelectionModel.h"

using namespace vm;

ContactsModel::ContactsModel(QObject *parent, bool createProxy)
    : ListModel(parent, false)
    , m_avatarLoader(new ContactAvatarLoader(this))
{
    qRegisterMetaType<ContactsModel *>("ContactsModel*");

    if (createProxy) {
        setProxy(new ContactsProxyModel(this));
    }

    connect(this, &ContactsModel::avatarUrlNotFound, this, &ContactsModel::loadAvatarUrl);
    connect(m_avatarLoader, &ContactAvatarLoader::loaded, this, &ContactsModel::setAvatarUrl);
}

void ContactsModel::setContacts(const Contacts &contacts)
{
    beginResetModel();
    m_contacts = contacts;
    endResetModel();
    m_avatarLoader->load(m_contacts, 10);
}

const Contacts &ContactsModel::getContacts() const
{
    return m_contacts;
}

int ContactsModel::getContactsCount() const
{
    return m_contacts.size();
}

ContactHandler ContactsModel::createContact(const QString &username) const
{
    auto contact = std::make_shared<Contact>();
    contact->setUsername(username);
    contact->setName(username);
    return contact;
}

const ContactHandler ContactsModel::getContact(const int row) const
{
    return m_contacts[row];
}

bool ContactsModel::hasContact(const QString &contactUsername) const
{
    return findByUsername(contactUsername).isValid();
}

void ContactsModel::addContact(const ContactHandler contact)
{
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    m_contacts.push_back(contact);
    endInsertRows();
    m_avatarLoader->load(m_contacts.back());
}

void ContactsModel::removeContact(const QString &contactUsername)
{
    if (const auto index = findByUsername(contactUsername); index.isValid()) {
        beginRemoveRows(QModelIndex(), index.row(), index.row());
        m_contacts.erase(m_contacts.begin() + index.row());
        endRemoveRows();
    }
}

void ContactsModel::removeContactsByRows(const int startRow, const int endRow)
{
    beginRemoveRows(QModelIndex(), startRow, endRow);
    m_contacts.erase(m_contacts.begin() + startRow, m_contacts.begin() + endRow + 1);
    endRemoveRows();
}

void ContactsModel::updateContact(const ContactHandler contact, int row)
{
    m_contacts[row] = contact;
    const auto rowIndex = index(row);
    emit dataChanged(rowIndex, rowIndex);
}

Contacts ContactsModel::selectedContacts() const
{
    Contacts contacts;
    const auto indices = selection()->selectedIndexes();
    for (const auto &i : indices) {
        contacts.push_back(m_contacts[i.row()]);
    }
    return contacts;
}

void ContactsModel::toggleByUsername(const QString &contactUsername)
{
    if (const auto index = findByUsername(contactUsername); index.isValid()) {
        selection()->toggle(index.row());
    }
}

QModelIndex ContactsModel::findByUsername(const QString &contactUsername) const
{
    const auto it = std::find_if(m_contacts.begin(), m_contacts.end(), [&contactUsername](auto contact) {
        return contact->username() == contactUsername;
    });
    if (it != m_contacts.end()) {
        return index(std::distance(m_contacts.begin(), it));
    }
    return QModelIndex();
}

void ContactsModel::loadAvatarUrl(const QString &contactUsername)
{
    if (const auto index = findByUsername(contactUsername); index.isValid()) {
        m_avatarLoader->load(m_contacts[index.row()]);
    }
}

void ContactsModel::setAvatarUrl(const ContactHandler contact, const QUrl &url)
{
    if (const auto index = findByUsername(contact->username()); index.isValid()) {
        m_contacts[index.row()]->setAvatarLocalPath(url.toLocalFile());
        emit dataChanged(index, index, { AvatarUrlRole });
    }
}

int ContactsModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_contacts.size();
}

QVariant ContactsModel::data(const QModelIndex &index, int role) const
{
    const auto &contact = m_contacts[index.row()];
    switch (role) {
    case IdRole:
        return QString(contact->userId());

    case UsernameRole:
        return contact->username();

    case NameRole:
        return contact->name();

    case DisplayNameRole:
        return contact->displayName();

    case DetailsRole:
    {
        if (contact->email().isEmpty()) {
            return contact->phone().isEmpty() ? tr("No phone/email") : contact->phone();
        }
        else {
            return contact->phone().isEmpty() ? contact->email() : (contact->phone() + QLatin1String(" / ") + contact->email());
        }
    }
    case AvatarUrlRole:
    {
        const auto &url = contact->avatarLocalPath();
        if (url.isEmpty()) {
            emit avatarUrlNotFound(contact->username(), QPrivateSignal());
        }
        return url;
    }
    case FilterRole:
        return contact->name() + QLatin1Char('\n') + contact->email() + QLatin1Char('\n') + contact->phone();

    case SortRole:
        return contact->name();

    default:
        return ListModel::data(index, role);
    }
}

QHash<int, QByteArray> ContactsModel::roleNames() const
{
    return unitedRoleNames(ListModel::roleNames(), {
        { IdRole, "userId" },
        { UsernameRole, "username" },
        { NameRole, "name" },
        { DisplayNameRole, "displayName" },
        { DetailsRole, "details" },
        { AvatarUrlRole, "avatarUrl" }
    });
}
