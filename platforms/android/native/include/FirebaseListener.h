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

#ifndef VIRGIL_MESSENGER_NOTIFICATIONS_ANDROID_FIREBASE_LISTENER_H_INCLUDED
#define VIRGIL_MESSENGER_NOTIFICATIONS_ANDROID_FIREBASE_LISTENER_H_INCLUDED

#include <firebase/messaging.h>
#include <firebase/app.h>
#include <firebase/util.h>

#include <QtCore>

class QAndroidJniEnvironment;

namespace vm {
namespace notifications {

class FirebaseListener : public firebase::messaging::Listener
{
public:
    static FirebaseListener &instance();

    void init();

    void OnTokenReceived(const char *token) override;

    void OnMessage(const firebase::messaging::Message &message) override;

private:
    FirebaseListener();

    void showNotification(QString title, QString message);

private:
    QAndroidJniEnvironment *m_jniEnv;
    firebase::App *m_app;
    firebase::ModuleInitializer m_initializer;
};

} // notifications
} // vm

#endif // VIRGIL_MESSENGER_NOTIFICATIONS_ANDROID_FIREBASE_LISTENER_H_INCLUDED
