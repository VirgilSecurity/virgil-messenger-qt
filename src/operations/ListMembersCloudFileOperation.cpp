//  Copyright (C) 2015-2021 Virgil Security, Inc.
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

#include "ListMembersCloudFileOperation.h"

#include "ContactsTable.h"
#include "CloudFileOperation.h"
#include "CloudFileSystem.h"
#include "CloudFilesUpdate.h"
#include "UserDatabase.h"

#include <algorithm>

using namespace vm;
using Self = ListMembersCloudFileOperation;

Self::ListMembersCloudFileOperation(CloudFileOperation *parent, CloudFileHandler file, CloudFileHandler parentFolder,
                                    UserDatabase *userDatabase)
    : Operation(QLatin1String("ShareCloudFiles"), parent),
      m_parent(parent),
      m_file(std::move(file)),
      m_parentFolder(std::move(parentFolder)),
      m_userDatabase(userDatabase),
      m_requestId(0)
{
    connect(m_parent->cloudFileSystem(), &CloudFileSystem::membersFetched, this, &Self::onListFetched);
    connect(m_parent->cloudFileSystem(), &CloudFileSystem::fetchMembersErrorOccurred, this,
            &Self::onListFetchErrorOccured);
    connect(m_userDatabase->contactsTable(), &ContactsTable::fetched, this, &Self::onContactsFetched);
}

void Self::run()
{
    m_requestId = m_parent->cloudFileSystem()->fetchMembers(m_file);
}

void Self::onListFetched(const CloudFileRequestId &requestId, const CloudFileHandler &file, CloudFileMembers members)
{
    if (m_requestId == requestId) {
        m_file = file;

        std::transform(members.begin(), members.end(), std::inserter(m_members, m_members.end()),
                       [](auto &&member) { return std::make_pair(member->memberId(), member); });

        QStringList memberIds;
        std::transform(m_members.cbegin(), m_members.cend(), std::back_inserter(memberIds),
                       [](const auto &member) { return QString(member.second->memberId()); });

        m_userDatabase->contactsTable()->fetch(m_requestId, memberIds);
    }
}

void Self::onListFetchErrorOccured(CloudFileRequestId requestId, const QString &errorText)
{
    if (m_requestId == requestId) {
        failAndNotify(errorText);
    }
}

void Self::onContactsFetched(const quint64 requestId, MutableContacts fetchedContacts)
{
    if (m_requestId == requestId) {
        //
        //  Update contacts in members.
        //
        CloudFileMembers updatedMembers;
        for (auto &&contact : fetchedContacts) {
            auto currentMember = m_members.find(contact->userId());
            if (currentMember != m_members.end()) {
                currentMember->second->setContact(contact);
                updatedMembers.push_back(currentMember->second);
                m_members.erase(currentMember);
            }
        }

        std::transform(m_members.begin(), m_members.end(), std::back_inserter(updatedMembers),
                       [](auto &&member) { return std::move(member.second); });

        m_members.clear();

        //
        //  Emit update and finish.
        //
        ListMembersCloudFileUpdate update;
        update.parentFolder = m_parentFolder;
        update.file = std::move(m_file);
        update.members = std::move(updatedMembers);
        m_parent->updateCloudFiles(update);

        finish();
    }
}
