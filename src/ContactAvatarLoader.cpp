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

#include "ContactAvatarLoader.h"

#include <QtConcurrent>

#include "Utils.h"

using namespace vm;

ContactAvatarLoader::ContactAvatarLoader(QObject *parent)
    : QObject(parent)
{
    m_timer.setSingleShot(true);
    m_timer.setInterval(100);
    connect(&m_timer, &QTimer::timeout, this, &ContactAvatarLoader::processLoad);
}

void ContactAvatarLoader::load(Contact &contact)
{
    // FIXME: Re-implement this.
    // if (canLoad(contact)) {
    //     m_contacts.push_back(contact);
    //     if (!m_timer.isActive()) {
    //         m_timer.start();
    //     }
    // }
}

void ContactAvatarLoader::load(Contacts &contacts, int maxLimit)
{
    // FIXME: Re-implement this.
    // const auto s = qMin<int>(maxLimit, m_contacts.size());
    // for (int i = 0; i < s; ++i) {
    //     load(contacts[i]);
    // }
}

bool ContactAvatarLoader::canLoad(Contact &contact)
{
    // FIXME: Re-implement this.
    // if (contact.avatarUrlRetryCount == 0) {
    //     return false;
    // }
    // --contact.avatarUrlRetryCount;
    // for (auto &c : m_contacts) {
    //     if (c.id == contact.id) {
    //         return false;
    //     }
    // }
    return true;
}

void ContactAvatarLoader::processLoad()
{
    // FIXME: Re-implement this.
    // Contacts contacts;
    // std::swap(contacts, m_contacts);
    // QtConcurrent::run([this, contacts = std::move(contacts)]() {
    //     for (auto &contact : contacts) {
    //         const auto url = Utils::getContactAvatarUrl(contact);
    //         if (!url.isEmpty()) {
    //             emit loaded(contact, url);
    //         }
    //     }
    // });
}
