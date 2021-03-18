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

#ifndef VM_GROUP_MEMBERS_TABLE_H
#define VM_GROUP_MEMBERS_TABLE_H

#include "core/DatabaseTable.h"
#include "GroupUpdate.h"
#include "GroupMember.h"
#include "Message.h"

#include <QString>
#include <QSqlQuery>

#include <optional>

namespace vm {
class GroupMembersTable : public DatabaseTable
{
    Q_OBJECT

public:
    explicit GroupMembersTable(Database *database);

signals:
    //
    //  Control signals.
    //
    void add(const GroupMembers &groupMembers);
    void fetch(const GroupId &groupId);
    void deleteGroupMembers(const GroupId &groupId);
    void updateGroup(const GroupUpdate &groupUpdate);

    //
    //  Notification signals.
    //
    void errorOccurred(const QString &errorText);
    void fetched(const GroupId &groupId, const GroupMembers &groupMembers);

private:
    void onAdd(const GroupMembers &groupMembers);
    void onFetch(const GroupId &groupId);
    void onDeleteGroupMembers(const GroupId &groupId);
    void onUpdateGroup(const GroupUpdate &groupUpdate);

    bool create() override;

    GroupMemberHandler readGroupMember(const QSqlQuery &query);
};
} // namespace vm

#endif // VM_GROUP_MEMBERS_TABLE_H
