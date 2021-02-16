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

#include "GroupsTable.h"

#include "core/Database.h"
#include "core/DatabaseUtils.h"

using namespace vm;
using Self = GroupsTable;

Self::GroupsTable(Database *database)
    : DatabaseTable(QLatin1String("groups"), database)
{
    connect(this, &Self::updateGroup, this, &Self::onUpdateGroup);
    connect(this, &Self::addGroupForChat, this, &Self::onAddGroupForChat);
    connect(this, &Self::deleteGroup, this, &Self::onDeleteGroup);
}


bool Self::create()
{
    if (DatabaseUtils::readExecQueries(database(), QLatin1String("createGroups"))) {
        qCDebug(lcDatabase) << "Table 'groups' was created.";
        return true;

    } else {
        qCCritical(lcDatabase) << "Failed to create table 'groups'.";
        return false;
    }
}


void Self::onAddGroupForChat(const ChatHandler& chat) {
    onUpdateGroup(AddGroupUpdate{ GroupId(chat->id()) });
}


void Self::onUpdateGroup(const GroupUpdate& groupUpdate)
{
    DatabaseUtils::BindValues bindValues;
    QLatin1String queryId;

    if (auto update = std::get_if<AddGroupUpdate>(&groupUpdate)) {
        queryId = QLatin1String("insertGroup");
        bindValues.push_back({ ":id", QString(update->groupId) });
    }

    if (queryId.isEmpty()) {
        return;
    }

    ScopedConnection connection(*database());
    const auto query = DatabaseUtils::readExecQuery(database(), queryId, bindValues);
    if (query) {
        qCDebug(lcDatabase) << "Group was updated: " << bindValues.front().second;
    }
    else {
        qCCritical(lcDatabase) << "GroupsTable::onUpdateGroup error";
        emit errorOccurred(tr("Failed to update groups table"));
    }
}

void Self::onDeleteGroup(const GroupId &groupId)
{
    const DatabaseUtils::BindValues values {{ ":id", QString(groupId) }};
    const auto query = DatabaseUtils::readExecQuery(database(), QLatin1String("deleteGroupById"), values);
    if (query) {
        qCDebug(lcDatabase) << "Group was removed, id:" << groupId;
    }
    else {
        qCCritical(lcDatabase) << "GroupsTable::onDeleteGroup deletion error";
        emit errorOccurred(tr("Failed to delete group"));
    }
}
