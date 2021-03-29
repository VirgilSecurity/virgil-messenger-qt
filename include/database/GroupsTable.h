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

#ifndef VM_GROUPS_TABLE_H
#define VM_GROUPS_TABLE_H

#include "core/DatabaseTable.h"
#include "GroupUpdate.h"
#include "Chat.h"
#include "Group.h"

#include <QString>

class QSqlQuery;

namespace vm {
class GroupsTable : public DatabaseTable
{
    Q_OBJECT

public:
    explicit GroupsTable(Database *database);

    static GroupHandler readGroup(const QSqlQuery &query, const QString &preffix = QString());

signals:
    //
    //  Control signals.
    //
    void add(const GroupHandler &group);
    void fetch();
    void updateGroup(const GroupUpdate &groupUpdate);
    void deleteGroup(const GroupId &groupId);

    //
    //  Notification signals.
    //
    void fetched(const Groups &groups);
    void errorOccurred(const QString &errorText);

private:
    void onAdd(const GroupHandler &group);
    void onFetch();
    void onUpdateGroup(const GroupUpdate &groupUpdate);
    void onDeleteGroup(const GroupId &groupId);

    void insertGroup(const GroupId &groupId, const UserId &superOwnerId);
    void updateGroupName(const GroupId &groupId, const QString &name);
    void updateGroupCache(const GroupId &groupId, const QString &cache);
    void updateGroupInvitation(const GroupId &groupId, GroupInvitationStatus status);

    bool create() override;
};
} // namespace vm

#endif // VM_GROUPS_TABLE_H
