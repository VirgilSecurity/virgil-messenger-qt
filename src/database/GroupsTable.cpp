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

Self::GroupsTable(Database *database) : DatabaseTable(QLatin1String("groups"), database)
{
    connect(this, &Self::add, this, &Self::onAdd);
    connect(this, &Self::fetch, this, &Self::onFetch);
    connect(this, &Self::deleteGroup, this, &Self::onDeleteGroup);
    connect(this, &Self::updateGroup, this, &Self::onUpdateGroup);
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

GroupHandler Self::readGroup(const QSqlQuery &query, const QString &preffix)
{
    auto queryValue = [&query, &preffix](auto &name) {
        if (preffix.isEmpty()) {
            return query.value(name);
        }
        QString newName(name);
        newName[0] = newName[0].toUpper();
        return query.value(preffix + newName);
    };

    auto id = GroupId(queryValue("id").toString());
    auto superOwnerId = UserId(queryValue("superOwnerId").toString());
    auto name = queryValue("name").toString();
    auto cache = queryValue("cache").toString();
    auto invitationStatus = GroupInvitationStatusFromString(queryValue("invitationStatus").toString());

    if (!id.isValid() || !superOwnerId.isValid()) {
        return nullptr;
    }

    return std::make_shared<Group>(std::move(id), std::move(superOwnerId), std::move(name), invitationStatus,
                                   std::move(cache));
}

void Self::onAdd(const GroupHandler &group)
{
    ScopedConnection connection(*database());

    const QVariant groupCache = !group->cache().isEmpty() ? group->cache() : QVariant("");

    const DatabaseUtils::BindValues bindValues { { ":id", QString(group->id()) },
                                                 { ":superOwnerId", QString(group->superOwnerId()) },
                                                 { ":name", QString(group->name()) },
                                                 { ":invitationStatus",
                                                   GroupInvitationStatusToString(group->invitationStatus()) },
                                                 { ":cache", groupCache } };

    const auto query = DatabaseUtils::readExecQuery(database(), QLatin1String("insertGroup"), bindValues);
    if (query) {
        qCDebug(lcDatabase) << "Group was added: " << bindValues.front().second;
        emit added(group);
    } else {
        qCCritical(lcDatabase) << "GroupsTable::insertGroup error";
        emit errorOccurred(tr("Failed to update groups table"));
    }
}

void Self::onFetch()
{
    ScopedConnection connection(*database());
    auto query = DatabaseUtils::readExecQuery(database(), QLatin1String("selectGroups"));
    if (query) {
        qCDebug(lcDatabase) << "Groups where fetched";

        Groups groups;
        while (query->next()) {
            if (auto group = readGroup(*query)) {
                groups.push_back(std::move(group));
            }
        }

        emit fetched(groups);
    } else {
        qCCritical(lcDatabase) << "GroupsTable::onFetch error";
    }
}

void Self::onUpdateGroup(const GroupUpdate &groupUpdate)
{
    if (auto update = std::get_if<GroupNameUpdate>(&groupUpdate)) {
        updateGroupName(update->groupId, update->name);

    } else if (auto update = std::get_if<GroupCacheUpdate>(&groupUpdate)) {
        updateGroupCache(update->groupId, update->cache);
    } else if (auto update = std::get_if<GroupInvitationUpdate>(&groupUpdate)) {
        updateGroupInvitation(update->groupId, update->invitationStatus);
    }
}

void Self::onDeleteGroup(const GroupId &groupId)
{
    ScopedConnection connection(*database());
    const DatabaseUtils::BindValues values { { ":id", QString(groupId) } };
    const auto query = DatabaseUtils::readExecQuery(database(), QLatin1String("deleteGroupById"), values);
    if (query) {
        qCDebug(lcDatabase) << "Group was removed, id:" << groupId;
    } else {
        qCCritical(lcDatabase) << "GroupsTable::onDeleteGroup deletion error";
        emit errorOccurred(tr("Failed to delete group"));
    }
}

void Self::updateGroupName(const GroupId &groupId, const QString &name)
{
    ScopedConnection connection(*database());
    const DatabaseUtils::BindValues bindValues { { ":id", QString(groupId) }, { ":name", name } };
    const auto query = DatabaseUtils::readExecQuery(database(), QLatin1String("updateGroupName"), bindValues);
    if (query) {
        qCDebug(lcDatabase) << "Group name was updated: " << bindValues.front().second;
    } else {
        qCCritical(lcDatabase) << "GroupsTable::updateGroupName error";
        emit errorOccurred(tr("Failed to update groups table"));
    }
}

void Self::updateGroupCache(const GroupId &groupId, const QString &cache)
{
    ScopedConnection connection(*database());
    const DatabaseUtils::BindValues bindValues { { ":id", QString(groupId) }, { ":cache", cache } };
    const auto query = DatabaseUtils::readExecQuery(database(), QLatin1String("updateGroupCache"), bindValues);
    if (query) {
        qCDebug(lcDatabase) << "Group cache was updated: " << bindValues.front().second;
    } else {
        qCCritical(lcDatabase) << "GroupsTable::updateGroupCache error";
        emit errorOccurred(tr("Failed to update groups table"));
    }
}

void Self::updateGroupInvitation(const GroupId &groupId, GroupInvitationStatus status)
{
    ScopedConnection connection(*database());
    const DatabaseUtils::BindValues bindValues { { ":id", QString(groupId) },
                                                 { ":invitationStatus", GroupInvitationStatusToString(status) } };
    const auto query = DatabaseUtils::readExecQuery(database(), QLatin1String("updateGroupInvitation"), bindValues);
    if (query) {
        qCDebug(lcDatabase) << "Group invitation was updated: " << bindValues.front().second;
    } else {
        qCCritical(lcDatabase) << "GroupsTable::updateGroupInvitation error";
        emit errorOccurred(tr("Failed to update groups table"));
    }
}
