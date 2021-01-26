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

#include "GroupMembersTable.h"

#include "core/Database.h"
#include "core/DatabaseUtils.h"

using namespace vm;
using Self = GroupMembersTable;

Self::GroupMembersTable(Database *database)
    : DatabaseTable(QLatin1String("groupMembers"), database)
{
    connect(this, &Self::updateGroup, this, &Self::onUpdateGroup);
}


bool Self::create()
{
    if (DatabaseUtils::readExecQueries(database(), QLatin1String("createGroupMembers"))) {
        qCDebug(lcDatabase) << "Table 'groupMembers' was created.";
        return true;

    } else {
        qCCritical(lcDatabase) << "Failed to create table 'groupMembers'.";
        return false;
    }
}


void Self::onUpdateGroup(const GroupUpdate& groupUpdate)
{
    std::list<DatabaseUtils::BindValues> bindValuesCollection;
    QLatin1String queryId;

    if (auto update = std::get_if<AddGroupOwnersUpdate>(&groupUpdate)) {
        queryId = QLatin1String("insertGroupMember");

        for (const auto& member : update->owners) {
            DatabaseUtils::BindValues bindValues;
            bindValues.push_back({ ":groupId", QString(update->groupId) });
            bindValues.push_back({ ":memberId", QString(member->id()) });
            bindValues.push_back({ ":memberNickname", QString(member->username()) });
            bindValues.push_back({ ":memberAffiliation", "owner" });

            bindValuesCollection.push_back(std::move(bindValues));
        }
    }

    if (auto update = std::get_if<AddGroupMembersUpdate>(&groupUpdate)) {
        queryId = QLatin1String("insertGroupMember");

        for (const auto& member : update->members) {
            DatabaseUtils::BindValues bindValues;
            bindValues.push_back({ ":groupId", QString(update->groupId) });
            bindValues.push_back({ ":memberId", QString(member->id()) });
            bindValues.push_back({ ":memberNickname", QString(member->username()) });
            bindValues.push_back({ ":memberAffiliation", "member" });

            bindValuesCollection.push_back(std::move(bindValues));
        }
    }

    if (auto update = std::get_if<GroupMemberInvitationUpdate>(&groupUpdate)) {
        queryId = QLatin1String("updateGroupMemberInvitationStatus");

        DatabaseUtils::BindValues bindValues;
        bindValues.push_back({ ":groupId", QString(update->groupId) });
        bindValues.push_back({ ":memberId", QString(update->memberId) });
        bindValues.push_back({ ":invitationStatus", GroupInvitationStatusToString(update->invitationStatus) });

        bindValuesCollection.push_back(std::move(bindValues));
    }

    if (queryId.isEmpty()) {
        return;
    }

    ScopedConnection connection(*database());
    ScopedTransaction transaction(*database());
    for (const auto& bindValues : bindValuesCollection) {

        const auto query = DatabaseUtils::readExecQuery(database(), queryId, bindValues);
        if (query) {
            qCDebug(lcDatabase) << "GroupMembers was updated: " << bindValues.front().second;
        }
        else {
            transaction.rollback();
            qCCritical(lcDatabase) << "GroupMembersTable::onUpdateGroup error";
            emit errorOccurred(tr("Failed to update group members table"));
        }
    }
}
