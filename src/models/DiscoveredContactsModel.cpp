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
#include "models/ListSelectionModel.h"

using namespace vm;

DiscoveredContactsModel::DiscoveredContactsModel(Validator *validator, QObject *parent)
    : ContactsModel(true, parent)
    , m_validator(validator)
    , m_selectedContacts(new ContactsModel(false, this))
{
    qRegisterMetaType<DiscoveredContactsModel *>("DiscoveredContactsModel*");

    connect(this, &ContactsModel::contactsChanged, this, &DiscoveredContactsModel::checkFilterHasNewContact);
    connect(selection(), &ListSelectionModel::changed, this, &DiscoveredContactsModel::onSelectionChanged);
    connect(this, &DiscoveredContactsModel::filterChanged, this, &DiscoveredContactsModel::checkFilterHasNewContact);
    connect(this, &DiscoveredContactsModel::contactsPopulated, this, &DiscoveredContactsModel::setContacts);
}

void DiscoveredContactsModel::setUserId(const UserId &userId)
{
    m_userId = userId;
}

void DiscoveredContactsModel::reload()
{
    m_selectedContacts->setContacts(Contacts());
    setFilter(QString());

    QtConcurrent::run([=]() {
        const auto contacts = Utils::getDeviceContacts(getContacts());
        emit contactsPopulated(contacts, QPrivateSignal());
    });
}

void DiscoveredContactsModel::toggleById(const Contact::Id &contactId)
{
    if (auto row = findRowByContactId(contactId)) {
        selection()->toggle(index(*row));
    }
    else if (m_selectedContacts->hasContact(contactId)) {
        m_selectedContacts->removeContact(contactId);
    }
    else {
        m_selectedContacts->addContact(contactId);
    }
}

void DiscoveredContactsModel::checkFilterHasNewContact()
{
    const auto filter = this->filter();
    const bool has = filter != m_userId && m_validator->isValidUsername(filter) && !hasContact(filter);
    if (has == m_filterHasNewContact) {
        return;
    }
    m_filterHasNewContact = has;
    emit filterHasNewContactChanged(has);
}

void DiscoveredContactsModel::onSelectionChanged(const QList<QModelIndex> &indices)
{
    for (const auto &i : indices) {
        const auto &c = getContact(i.row());
        if (m_selectedContacts->hasContact(c.id)) {
            m_selectedContacts->removeContact(c.id);
        }
        else {
            m_selectedContacts->addContact(c);
        }
    }
}
