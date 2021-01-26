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


#include "GroupInvitationStatus.h"


using namespace vm;


GroupInvitationStatus vm::GroupInvitationStatusFromString(const QString& stageString) {
    if (stageString == QLatin1String("none")) {
        return GroupInvitationStatus::None;
    }
    else if (stageString == QLatin1String("invited")) {
        return GroupInvitationStatus::Invited;
    }
    else if (stageString == QLatin1String("accepted")) {
        return GroupInvitationStatus::Accepted;
    }
    else if (stageString == QLatin1String("rejected")) {
        return GroupInvitationStatus::Rejected;
    }
    else {
        throw std::logic_error("Invalid GroupInvitationStatus string");
    }
}


QString vm::GroupInvitationStatusToString(GroupInvitationStatus stage) {
    switch (stage) {
        case GroupInvitationStatus::None:
            return QLatin1String("none");

        case GroupInvitationStatus::Invited:
            return QLatin1String("invited");

        case GroupInvitationStatus::Accepted:
            return QLatin1String("accepted");

        case GroupInvitationStatus::Rejected:
            return QLatin1String("rejected");

        default:
            throw std::logic_error("Invalid GroupInvitationStatus");
    }
}
