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

#ifndef VM_CLOUD_FILE_MEMBERS_MODEL_H
#define VM_CLOUD_FILE_MEMBERS_MODEL_H

#include <QPointer>

#include "CloudFileMember.h"
#include "ContactsModel.h"

namespace vm {
class Messenger;

class CloudFileMembersModel : public ContactsModel
{
    Q_OBJECT
    Q_PROPERTY(bool isReadOnly MEMBER m_isReadOnly NOTIFY isReadOnlyChanged)
    Q_PROPERTY(bool isOwnedByUser MEMBER m_isOwnedByUser NOTIFY isOwnedByUserChanged)

public:
    CloudFileMembersModel(Messenger *messenger, QObject *parent);

    void setMembers(const CloudFileMembers &members);
    CloudFileMembers members() const;

    CloudFileMembers selectedMembers() const;

signals:
    void isReadOnlyChanged(bool isReadOnly);
    void isOwnedByUserChanged(bool isOwned);

private:
    QVariant data(const QModelIndex &index, int role) const override;

    QPointer<Messenger> m_messenger;
    CloudFileMembers m_members;
    bool m_isReadOnly = true;
    bool m_isOwnedByUser = false;
};
} // namespace vm

#endif // VM_CLOUD_FILE_MEMBERS_MODEL_H
