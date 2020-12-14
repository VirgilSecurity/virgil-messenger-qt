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

#include "models/DiscoveredContactsModel.h"

#include <QSortFilterProxyModel>
#include <QtConcurrent>

#include "ContactAvatarLoader.h"
#include "Settings.h"
#include "Validator.h"
#include "Utils.h"

using namespace vm;

DiscoveredContactsModel::DiscoveredContactsModel(Validator *validator, QObject *parent)
    : ListModel(parent)
    , m_validator(validator)
    , m_avatarLoader(new ContactAvatarLoader(this))
{
    qRegisterMetaType<DiscoveredContactsModel *>("DiscoveredContactsModel*");

    proxy()->setSortRole(NameRole);
    proxy()->sort(0, Qt::AscendingOrder);
    proxy()->setFilterRole(FilterRole);

    connect(this, &DiscoveredContactsModel::filterChanged, this, &DiscoveredContactsModel::checkNewContactFiltered);
    connect(this, &DiscoveredContactsModel::contactsPopulated, this, &DiscoveredContactsModel::setContacts);
    connect(this, &DiscoveredContactsModel::avatarUrlNotFound, this, &DiscoveredContactsModel::loadAvatarUrl);
    connect(m_avatarLoader, &ContactAvatarLoader::loaded, this, &DiscoveredContactsModel::setAvatarUrl);
}

void DiscoveredContactsModel::reload()
{
    QtConcurrent::run([=]() {
        const auto contacts = Utils::getDeviceContacts(m_contacts);
        emit contactsPopulated(contacts, QPrivateSignal());
    });
}

int DiscoveredContactsModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_contacts.size();
}

QVariant DiscoveredContactsModel::data(const QModelIndex &index, int role) const
{
    const auto &info = m_contacts[index.row()];
    switch (role) {
    case NameRole:
        return info.name;
    case DetailsRole:
    {
        if (info.email.isEmpty()) {
            return info.phoneNumber.isEmpty() ? tr("No phone/email") : info.phoneNumber;
        }
        else {
            return info.phoneNumber.isEmpty() ? info.email : (info.phoneNumber + QLatin1String(" / ") + info.email);
        }
    }
    case AvatarUrlRole:
    {
        const auto &url = info.avatarUrl;
        if (url.isEmpty()) {
            emit avatarUrlNotFound(info.id, QPrivateSignal());
        }
        return url;
    }
    case LastSeenActivityRole:
        return info.lastSeenActivity;
    case FilterRole:
        return info.name + QLatin1Char('\n') + info.email + QLatin1Char('\n') + info.phoneNumber;
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> DiscoveredContactsModel::roleNames() const
{
    return {
        { NameRole, "name" },
        { DetailsRole, "details" },
        { AvatarUrlRole, "avatarUrl" },
        { LastSeenActivityRole, "lastSeenActivity" },
        // Search role is hidden
    };
}

void DiscoveredContactsModel::setContacts(const Contacts &contacts)
{
    beginResetModel();
    m_contacts = contacts;
    endResetModel();
    checkNewContactFiltered();
    m_avatarLoader->load(m_contacts, 10);
}

void DiscoveredContactsModel::checkNewContactFiltered()
{
    const auto filter = this->filter();
    bool filtered = m_validator->isValidUsername(filter);
    if (filtered) {
        for (auto &contact : m_contacts) {
            if (contact.name == filter) {
                filtered = false;
                break;
            }
        }
    }
    if (filtered == m_newContactFiltered) {
        return;
    }
    // FIXME(fpohtmeh): compare with current userId
    m_newContactFiltered = filtered;
    emit newContactFilteredChanged(filtered);
}

void DiscoveredContactsModel::loadAvatarUrl(const Contact::Id &contactId)
{
    if (const auto row = findRowByContactId(contactId)) {
        m_avatarLoader->load(m_contacts[*row]);
    }
}

void DiscoveredContactsModel::setAvatarUrl(const Contact &contact, const QUrl &url)
{
    if (const auto row = findRowByContactId(contact.id)) {
        m_contacts[*row].avatarUrl = url;
        const auto idx = index(*row);
        emit dataChanged(idx, idx, { AvatarUrlRole });
    }
}

Optional<int> DiscoveredContactsModel::findRowByContactId(const Contact::Id &contactId) const
{
    int row = -1;
    for (auto &info : m_contacts) {
        ++row;
        if (info.id == contactId) {
            return row;
        }
    }
    return NullOptional;
}
