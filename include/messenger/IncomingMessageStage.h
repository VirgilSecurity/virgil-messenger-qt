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

#ifndef VM_INCOMING_MESSAGE_STAGE_H
#define VM_INCOMING_MESSAGE_STAGE_H

#include "MessageStatus.h"

#include <QString>

namespace vm {
//
//  Denotes incoming message processing stage.
//
enum class IncomingMessageStage {
    Received, // A message was locally received from a sender.
    Decrypted, // A message was decrypted, so content handles specific message content: text, picture, etc.
    Read, // A message was read by a recipient.
    Broken // A message is broken and can't be processed.
};

//
//  Return incoming stage from a given string.
//  Throws if correspond stage is not found.
//
IncomingMessageStage IncomingMessageStageFromString(const QString &stageString);

//
//  Return string from a given incoming stage.
//
QString IncomingMessageStageToString(IncomingMessageStage stage);

//
// Converts incoming message stage to message status
//
MessageStatus IncomingMessageStageToMessageStatus(IncomingMessageStage stage);
} // namespace vm

#endif // VM_INCOMING_MESSAGE_STAGE_H
