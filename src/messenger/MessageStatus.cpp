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


#include "MessageStatus.h"


using namespace vm;


MessageStatus vm::MessageStatusFromString(const QString& statusString) {
    if (statusString == QLatin1String("new")) {
        return MessageStatus::New;
    }
    else if (statusString == QLatin1String("processing")) {
        return MessageStatus::Processing;
    }
    else if (statusString == QLatin1String("succeed")) {
        return MessageStatus::Succeed;
    }
    else if (statusString == QLatin1String("failed")) {
        return MessageStatus::Failed;
    }
    else if (statusString == QLatin1String("broken")) {
        return MessageStatus::Broken;
    }
    else {
        throw std::logic_error("Invalid MessageStatus string");
    }
}


QString vm::MessageStatusToString(MessageStatus status) {
    switch (status) {
        case MessageStatus::New:
            return QLatin1String("new");

        case MessageStatus::Processing:
            return QLatin1String("processing");

        case MessageStatus::Succeed:
            return QLatin1String("succeed");

        case MessageStatus::Failed:
            return QLatin1String("failed");

        case MessageStatus::Broken:
            return QLatin1String("broken");

        default:
            throw std::logic_error("Invalid MessageStatus");
    }
}
