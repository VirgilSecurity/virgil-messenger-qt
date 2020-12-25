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

#include <QtConcurrent>

#include "Settings.h"
#include "Validator.h"
#include "Utils.h"
#include "controllers/UsersController.h"
#include "models/DiscoveredContactsProxyModel.h"
#include "models/ListSelectionModel.h"

using namespace vm;

DiscoveredContactsModel::DiscoveredContactsModel(Validator *validator, QObject *parent)
    : ContactsModel(parent, false)
    , m_validator(validator)
    , m_selectedContacts(new ContactsModel(this, false))
{
    qRegisterMetaType<Contacts>("Contacts");
    qRegisterMetaType<DiscoveredContactsModel *>("DiscoveredContactsModel*");

    setProxy(new DiscoveredContactsProxyModel(this));
    m_selectedContacts->setProxy(new ContactsProxyModel(m_selectedContacts, false));

    connect(selection(), &ListSelectionModel::changed, this, &DiscoveredContactsModel::onSelectionChanged);
    connect(this, &DiscoveredContactsModel::filterChanged, this, &DiscoveredContactsModel::updateDiscoveredContacts);
    connect(this, &DiscoveredContactsModel::fixedContactsPopulated, this, &DiscoveredContactsModel::onDeviceContactsPopulated);
}

void DiscoveredContactsModel::reload()
{
    m_selectedContacts->setContacts(Contacts());
    setFilter(QString());

    QtConcurrent::run([=]() {
        const auto contacts = Utils::getDeviceContacts(getContacts());
        emit fixedContactsPopulated(contacts, QPrivateSignal());
    });
}

int DiscoveredContactsModel::fixedContactsCount() const
{
    return m_fixedContactsCount;
}

void DiscoveredContactsModel::toggleById(const Contact::Id &contactId)
{
    const auto row = findRowByContactId(contactId);
    if (row && *row < m_fixedContactsCount) {
        selection()->toggle(index(*row));
    }
    else {
        updateSelectedContacts(contactId);
    }
}

QString DiscoveredContactsModel::firstContactId() const
{
    if (proxy()->rowCount() > 0) {
        return proxy()->data(proxy()->index(0, 0), IdRole).toString();
    }
    return QString();
}

QVariant DiscoveredContactsModel::data(const QModelIndex &index, int role) const
{
    switch (role) {
    case SortRole:
    {
        const auto row = index.row();
        const auto preffix = (row < m_fixedContactsCount) ? QLatin1Char('1') : QLatin1Char('0');
        return preffix + getContact(row).name;
    }
    case SectionRole:
        return (index.row() < m_fixedContactsCount) ? tr("Phone contacts") : tr("Discovered contacts");
    case IsSelectedRole:
    {
        if (index.row() >= m_fixedContactsCount) {
            const auto contactId = getContact(index.row()).id;
            return m_selectedContacts->hasContact(contactId);
        }
        break;
    }
    default:
        break;
    }
    return ContactsModel::data(index, role);
}

QHash<int, QByteArray> DiscoveredContactsModel::roleNames() const
{
    return unitedRoleNames(ContactsModel::roleNames(), {
        { SectionRole, "section" }
    });
}

Contacts DiscoveredContactsModel::findContactsByFilter() const
{
    Contacts contacts;
    for (const Contact::Id &id : { filter() }) {
        // TODO(fpohtmeh): exclude current user
        if (m_validator->isValidUsername(id)) {
            contacts.push_back(createContact(id));
        }
    }
    return contacts;
}

void DiscoveredContactsModel::invalidateIsSelectedRole(int startRow, int endRow)
{
    emit dataChanged(index(startRow), index(endRow), { IsSelectedRole });
}

void DiscoveredContactsModel::updateDiscoveredContacts()
{
    const auto contacts = findContactsByFilter();
    int contactsCount = m_fixedContactsCount;
    for (auto &c : contacts) {
        auto row = findRowByContactId(c.id);
        if (row && *row < m_fixedContactsCount) {
            continue; // Contact already exists
        }
        // Replace or add contact
        if (contactsCount < getContactsCount()) {
            updateContact(c, contactsCount);
        }
        else {
            addContact(c);
        }
        ++contactsCount;
    }
    // Remove unused contacts
    if (getContactsCount() > contactsCount) {
        removeContactsByRows(contactsCount, getContactsCount() - 1);
    }
    // Invalidate selection for discovered contacts
    if (getContactsCount() > m_fixedContactsCount) {
        invalidateIsSelectedRole(m_fixedContactsCount, getContactsCount() - 1);
    }
}

void DiscoveredContactsModel::updateSelectedContacts(const Contact::Id &contactId, const Contact *contact)
{
    if (m_selectedContacts->hasContact(contactId)) {
        m_selectedContacts->removeContact(contactId);
    }
    else {
        m_selectedContacts->addContact(contact ? *contact : createContact(contactId));
    }
    // Invalidate selection for discovered contacts
    const auto row = findRowByContactId(contactId);
    if (row && *row >= m_fixedContactsCount) {
        invalidateIsSelectedRole(*row, *row);
    }
}

void DiscoveredContactsModel::onDeviceContactsPopulated(const Contacts &contacts)
{
    setContacts(contacts);
    m_fixedContactsCount = getContactsCount();
    updateDiscoveredContacts();
}

void DiscoveredContactsModel::onSelectionChanged(const QList<QModelIndex> &indices)
{
    for (const auto &i : indices) {
        if (i.row() < m_fixedContactsCount) {
            const auto &contact = getContact(i.row());
            updateSelectedContacts(contact.id, &contact);
        }
    }
}
