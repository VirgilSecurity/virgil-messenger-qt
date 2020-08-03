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

#if (VS_ANDROID)

#include "android/VSQFirebaseListener.h"

#include <QtAndroid>
#include <QAndroidJniEnvironment>
#include <QAndroidJniObject>

#include <QDebug>

#include <VSQMessenger.h>

/******************************************************************************/
VSQFirebaseListener::VSQFirebaseListener() {
    qDebug() << "PushDelegate. Creating JNI Env";
    _jniEnv = new QAndroidJniEnvironment();

    qDebug() << "Getting Android Activity";
    QAndroidJniObject jniObject = QtAndroid::androidActivity();

    qDebug() << "Creating Firebase App";
    ::firebase::App *instance = ::firebase::App::GetInstance();

    if (instance) {
        qDebug() << "App instance already exists";
        _app = instance;
    } else {
        qDebug() << "Creating app instance";
        _app = ::firebase::App::Create(*_jniEnv, jniObject.object<jobject>());
    }
}

/******************************************************************************/
static ::firebase::InitResult _InitializeMessaging(::firebase::App *app, void *context) {
    return ::firebase::messaging::Initialize(*app, static_cast<::firebase::messaging::Listener *>(context));
}

/******************************************************************************/
void
VSQFirebaseListener::initMessaging() {
    qDebug() << "Initializing Firebase module";
    _initializer.Initialize(_app, this, _InitializeMessaging);
    qDebug() << "Module initialized. Waiting on messaging initialization";
}

/******************************************************************************/
void
VSQFirebaseListener::showNotification(QString title, QString message) {
    QAndroidJniObject javaNotificationTitle = QAndroidJniObject::fromString(title);
    QAndroidJniObject javaNotificationMessage = QAndroidJniObject::fromString(message);
    QAndroidJniObject::callStaticMethod<void>(
            "org/virgil/notification/NotificationClient",
            "notify",
            "(Landroid/content/Context;Ljava/lang/String;Ljava/lang/String;)V",
            QtAndroid::androidContext().object(),
            javaNotificationTitle.object<jstring>(),
            javaNotificationMessage.object<jstring>());
}

/******************************************************************************/
void
VSQFirebaseListener::OnTokenReceived(const char *token) {
    qDebug() << "Token received: [" << token << "]";
    m_token = QString::fromUtf8(token);
}

/******************************************************************************/
const QString &VSQFirebaseListener::token() const {
    return m_token;
}

/******************************************************************************/
void
VSQFirebaseListener::OnMessage(const ::firebase::messaging::Message &message) {
    ::firebase::messaging::Message mes = message;
    qDebug() << "Received message: ";
    qDebug() << "to    : " << QString::fromStdString(mes.to);
    qDebug() << "from  : " << QString::fromStdString(mes.from);
    //qDebug() << "body  : " << QString::fromStdString(mes.notification->body);

#if 0
    const QString sender = QString::fromStdString(mes.data["title"]);
    const QString ciphertext = QString::fromStdString(mes.data["ciphertext"]);
    const QString decryptedMessage = Messenger::decryptMessage(sender, ciphertext);

    showNotification(sender, decryptedMessage);
#endif
}

/******************************************************************************/

#endif // VS_ANDROID
