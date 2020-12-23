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


#ifndef VM_MESSAGE_GROUP_CHAT_INFO_H
#define VM_MESSAGE_GROUP_CHAT_INFO_H

#include "MessageContent.h"

namespace vm {

//
//  Handles information about group chat the message belongs to.
//
class MessageGroupChatInfo  {
public:
    MessageGroupChatInfo(QString groupId, QString senderGroupNickname, QString recipientGroupNickname, bool isPrivate);

    //
    // Return group unique identifier the message belongs to.
    //
    QString groupId() const;

    //
    // Return sender "nickname" - a group unique name of the sender.
    //
    QString senderGroupNickname() const;

    //
    // Return sender "nickname" - a group unique name of the recipient.
    //
    QString recipientGroupNickname() const;

    //
    // Return true if message was send personally (privately).
    //
    bool isPrivate() const;

private:
    QString m_groupId;
    QString m_senderGroupNickname;
    QString m_recipientGroupNickname;
    bool m_isPrivate;

};
} // namespace vm

#endif // VM_MESSAGE_GROUP_CHAT_INFO_H