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

#import "NotificationService.h"

#include "CoreMessenger.h"
#include "Settings.h"

#include <memory>

using namespace vm;

@interface NotificationService ()

@property (nonatomic, strong) void (^contentHandler)(UNNotificationContent* contentToDeliver);
@property (nonatomic, strong) UNMutableNotificationContent* bestAttemptContent;

@end

@implementation NotificationService

- (void)didReceiveNotificationRequest:(UNNotificationRequest*)request
                   withContentHandler:(void (^)(UNNotificationContent* _Nonnull))contentHandler
{
    self.contentHandler = contentHandler;
    self.bestAttemptContent = [request.content mutableCopy];

    //
    //  Replace Base64 with something readable.
    //
    self.bestAttemptContent.title = @"<...>";
    self.bestAttemptContent.body = @"encrypted message";

    //
    //  Extract encrypted message.
    //

    //
    //  Try to decrypt encrypted message.
    //
    auto settings = std::make_unique<Settings>();
    CoreMessenger messenger(settings.get());
}

- (void)serviceExtensionTimeWillExpire
{
    // Called just before the extension will be terminated by the system.
    // Use this as an opportunity to deliver your "best attempt" at modified content, otherwise the original push
    // payload will be used.
    self.contentHandler(self.bestAttemptContent);
}

@end
