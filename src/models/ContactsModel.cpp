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

#include "models/ContactsModel.h"

#include "models/ContactsProxyModel.h"
#include "ContactAvatarLoader.h"

#include <stdexcept>

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

Contact ContactsModel::createContact(const QString &username) const
{
    // TODO: Re-implement this.
    Contact contact;
    contact.setUsername(username);
    contact.setName(username);
    return contact;
}

const Contact &ContactsModel::getContact(const int row) const
{
    return m_contacts[row];
}

bool ContactsModel::hasContact(const QString &contactId) const
{
    return findRowByContactName(contactId) != std::nullopt;
}

void ContactsModel::addContact(const Contact &contact)
{
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    m_contacts.push_back(contact);
    endInsertRows();
    m_avatarLoader->load(m_contacts.back());
}

void ContactsModel::removeContact(const QString &contactId)
{
    if (auto row = findRowByContactName(contactId)) {
        beginRemoveRows(QModelIndex(), *row, *row);
        m_contacts.erase(m_contacts.begin() + *row);
        endRemoveRows();
    }
}

void ContactsModel::removeContactsByRows(const int startRow, const int endRow)
{
    beginRemoveRows(QModelIndex(), startRow, endRow);
    m_contacts.erase(m_contacts.begin() + startRow, m_contacts.begin() + endRow + 1);
    endRemoveRows();
}

void ContactsModel::updateContact(const Contact &contact, int row)
{
    m_contacts[row] = contact;
    const auto rowIndex = index(row);
    emit dataChanged(rowIndex, rowIndex);
}

std::optional<int> ContactsModel::findRowByContactName(const QString &contactId) const
{
    int row = -1;
    for (auto &info : m_contacts) {
        ++row;
        if (info.name() == contactId) {
            return row;
        }
    }
    return std::nullopt;
}

void ContactsModel::loadAvatarUrl(const QString &contactId)
{
    if (const auto row = findRowByContactName(contactId)) {
        m_avatarLoader->load(m_contacts[*row]);
    }
}

void ContactsModel::setAvatarUrl(const Contact &contact, const QUrl &url)
{
    if (const auto row = findRowByContactName(contact.name())) {
        m_contacts[*row].setAvatarLocalPath(url.toLocalFile());
        const auto idx = index(*row);
        emit dataChanged(idx, idx, { AvatarUrlRole });
    }
}

int ContactsModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_contacts.size();
}

QVariant ContactsModel::data(const QModelIndex &index, int role) const
{
    const auto &info = m_contacts[index.row()];
    switch (role) {
    case IdRole:
        return QString(info.userId());

    case UsernameRole:
        return info.username();

    case NameRole:
        return info.name();

    case DetailsRole:
    {
        if (info.email().isEmpty()) {
            return info.phone().isEmpty() ? tr("No phone/email") : info.phone();
        }
        else {
            return info.phone().isEmpty() ? info.email() : (info.phone() + QLatin1String(" / ") + info.email());
        }
    }
    case AvatarUrlRole:
    {
        const auto &url = info.avatarLocalPath();
        if (url.isEmpty()) {
            emit avatarUrlNotFound(info.userId(), QPrivateSignal());
        }
        return url;
    }
    case FilterRole:
        return info.name() + QLatin1Char('\n') + info.email() + QLatin1Char('\n') + info.phone();

    case SortRole:
        return info.name();

    default:
        return ListModel::data(index, role);
    }
}

QHash<int, QByteArray> ContactsModel::roleNames() const
{
    return unitedRoleNames(ListModel::roleNames(), {
        { IdRole, "contactId" }, // TODO: Rename to 'userId'
        { UsernameRole, "username" },
        { NameRole, "name" },
        { DetailsRole, "details" },
        { AvatarUrlRole, "avatarUrl" }
    });
}
