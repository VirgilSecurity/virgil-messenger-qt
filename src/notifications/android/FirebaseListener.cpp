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

#include "FirebaseListener.h"

#include "PushNotifications.h"

#include <QtAndroid>
#include <QAndroidJniEnvironment>
#include <QAndroidJniObject>

#include <QDebug>

Q_DECLARE_LOGGING_CATEGORY(lcFirebaseListener);
Q_LOGGING_CATEGORY(lcFirebaseListener, "firebase");

using namespace notifications::android;
using Self = FirebaseListener;

static firebase::InitResult _InitializeMessaging(firebase::App *app, void *context)
{
    return firebase::messaging::Initialize(*app, static_cast<firebase::messaging::Listener *>(context));
}

Self &Self::instance()
{
    static Self instance;
    return instance;
}

Self::FirebaseListener()
{
    qCDebug(lcFirebaseListener) << "PushDelegate. Creating JNI Env";
    m_jniEnv = new QAndroidJniEnvironment();

    qCDebug(lcFirebaseListener) << "Getting Android Activity";
    QAndroidJniObject jniObject = QtAndroid::androidActivity();

    qCDebug(lcFirebaseListener) << "Creating Firebase App";
    firebase::App *instance = firebase::App::GetInstance();

    if (instance) {
        qCDebug(lcFirebaseListener) << "App instance already exists";
        m_app = instance;
    } else {
        qCDebug(lcFirebaseListener) << "Creating app instance";
        m_app = firebase::App::Create(*m_jniEnv, jniObject.object<jobject>());
    }
}

void Self::init()
{
    qCDebug(lcFirebaseListener) << "Initializing Firebase module";
    m_initializer.Initialize(m_app, this, _InitializeMessaging);
    qCDebug(lcFirebaseListener) << "Module initialized. Waiting on messaging initialization";
}

void Self::showNotification(QString title, QString message)
{
    QAndroidJniObject javaNotificationTitle = QAndroidJniObject::fromString(title);
    QAndroidJniObject javaNotificationMessage = QAndroidJniObject::fromString(message);
    QAndroidJniObject::callStaticMethod<void>(
            "org/virgil/notification/NotificationClient", "notify",
            "(Landroid/content/Context;Ljava/lang/String;Ljava/lang/String;)V", QtAndroid::androidContext().object(),
            javaNotificationTitle.object<jstring>(), javaNotificationMessage.object<jstring>());
}

void Self::OnTokenReceived(const char *token)
{
    qCDebug(lcFirebaseListener) << "Token received: [" << token << "]";
    auto deviceToken = QString::fromUtf8(token);
    PushNotifications::instance().registerToken(deviceToken);
}

void Self::OnMessage(const firebase::messaging::Message &message)
{
    firebase::messaging::Message mes = message;
    qCDebug(lcFirebaseListener) << "Received message: ";
    qCDebug(lcFirebaseListener) << "to    : " << QString::fromStdString(mes.to);
    qCDebug(lcFirebaseListener) << "from  : " << QString::fromStdString(mes.from);
    // qCDebug(lcFirebaseListener) << "body  : " << QString::fromStdString(mes.notification->body);

#if 0
    const QString sender = QString::fromStdString(mes.data["title"]);
    const QString ciphertext = QString::fromStdString(mes.data["ciphertext"]);
    const QString decryptedMessage = Messenger::decryptMessage(sender, ciphertext);

    showNotification(sender, decryptedMessage);
#endif
}
