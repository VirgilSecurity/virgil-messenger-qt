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

#include "CustomerEnv.h"

#include "VSQCustomer.h"

#if VS_ANDROID
#    include "android/VSQAndroid.h"
#endif

#if VS_WINDOWS
#    include "windows/WindowsPlatform.h"
#endif

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

#if defined(VERSION)
static const QString kVersion = QString(TOSTRING(VERSION)) + "-alpha";
#else
static const QString kVersion = "unknown";
#endif

using namespace vm;
using Self = vm::CustomerEnv;

QString Self::messengerServiceUrl()
{
#if VS_MSGR_ENV_DEV
    return Customer::MessengerUrlTemplate.arg(QLatin1String("-dev"));
#elif VS_MSGR_ENV_STG
    return Customer::MessengerUrlTemplate.arg(QLatin1String("-stg"));
    Í
#else
    return Customer::MessengerUrlTemplate.arg(QString());
#endif
}

QString Self::xmppServiceUrl()
{
#if VS_MSGR_ENV_DEV
    return Customer::XmppUrlTemplate.arg(QLatin1String("-dev"));
#elif VS_MSGR_ENV_STG
    return Customer::XmppUrlTemplate.arg(QLatin1String("-stg"));
#else
    return Customer::XmppUrlTemplate.arg(QString());
#endif
}

QString Self::xmppServiceDomain()
{
#if VS_MSGR_ENV_DEV
    return Customer::XmppDomainTemplate.arg(QLatin1String("-dev"));
#elif VS_MSGR_ENV_STG
    return Customer::XmppDomainTemplate.arg(QLatin1String("-stg"));
#else
    return Customer::XmppDomainTemplate.arg(QString());
#endif
}

QString Self::contactDiscoveryServiceUrl()
{
#if VS_MSGR_ENV_DEV
    return Customer::ContactDiscoveryUrlTemplate.arg(QLatin1String("-dev"));
#elif VS_MSGR_ENV_STG
    return Customer::ContactDiscoveryUrlTemplate.arg(QLatin1String("-stg"));
#else
    return Customer::ContactDiscoveryUrlTemplate.arg(QString());
#endif
}

QString Self::caBundlePath()
{
#if VS_ANDROID
    return VSQAndroid::caBundlePath();
#elif VS_WINDOWS
    return WindowsPlatform::caBundlePath();
#else
    return qgetenv("VS_CURL_CA_BUNDLE");
#endif
}

QString Self::version()
{
    return kVersion;
}
