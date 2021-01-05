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

#ifndef VSQ_CUSTOMER_H
#define VSQ_CUSTOMER_H

#include <QString>

namespace Customer
{
    static QString OrganizationName = "Braes Capital";
    static QString OrganizationDisplayName = "Braes";

    static QString ApplicationName = "Braes";
    static QString ApplicationDisplayName = "Braes Secure Messenger";

    static QString OrganizationDomain = "braescapital.com";
    static QString MessengerUrlTemplate = "https://messenger%1-va.virgilsecurity.com";
    static QString ContactDiscoveryUrlTemplate = "https://disco%1-va.virgilsecurity.com";
    static QString XmppUrlTemplate = "xmpp%1-va.virgilsecurity.com";
    static QString XmppDomainTemplate = "xmpp%1.virgilsecurity.com";

    static const QString kPushNotificationsProxy = "braes-push-notifications-proxy";
    static const QString kPushNotificationsTopic = "com.virgil.BraesMessenger2";

    static const QString kSecurityApplicationGroupIdentifier = "group.com.virgil.BraesMessenger";
}

#endif // VSQ_CUSTOMER_H
