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

#include "PushHandlerService.h"

#include "Logging.h"
#include "Settings.h"
#include "CoreMessenger.h"

#include <QDebug>
#include <QAndroidJniObject>
#include <QAndroidService>

#include <android/log.h>

using namespace vm;

static constexpr const char *const lcPushHandlerService = "PushHandlerService";

jobject Java_org_virgil_notification_PushHandlerService_decryptNotification(JNIEnv *jenv, jclass, jstring jRecipientJid,
                                                                            jstring jSenderJid, jstring jCiphertext)
{
    Settings settings;

    QAndroidJniObject recipientJid(jRecipientJid);
    QAndroidJniObject senderJid(jSenderJid);
    QAndroidJniObject ciphertext(jCiphertext);

    jclass jResultClass = jenv->FindClass("org/virgil/notification/DecryptedNotification");
    Q_ASSERT(jResultClass);

    __android_log_print(ANDROID_LOG_DEBUG, lcPushHandlerService, "Start to decrypt notification...");

    auto decryptResult = CoreMessenger::decryptStandaloneMessage(settings, recipientJid.toString(),
                                                                 senderJid.toString(), ciphertext.toString());

    if (auto result = std::get_if<CoreMessengerStatus>(&decryptResult)) {
        __android_log_print(ANDROID_LOG_WARN, lcPushHandlerService, "Failed to decrypt notification.");

        jmethodID jResultCtor = jenv->GetMethodID(jResultClass, "<init>", "()V");
        jobject jResult = jenv->NewObject(jResultClass, jResultCtor);

        return jResult;
    }

    __android_log_print(ANDROID_LOG_DEBUG, lcPushHandlerService, "Notification was decrypted...");

    auto message = std::move(*std::get_if<MessageHandler>(&decryptResult));
    QString title = message->senderUsername();

    QString body;
    if (auto text = std::get_if<MessageContentText>(&message->content())) {
        body = text->text();

    } else if (auto file = std::get_if<MessageContentFile>(&message->content())) {
        body = QString("File: %1").arg(file->fileName());

    } else if (auto picture = std::get_if<MessageContentPicture>(&message->content())) {
        body = "Picture";

    } else if (auto groupInvitation = std::get_if<MessageContentGroupInvitation>(&message->content())) {
        body = "Group Invitation";

    } else {
        __android_log_print(ANDROID_LOG_WARN, lcPushHandlerService, "Unexpected message content.");
    }

    jstring jTitle = jenv->NewStringUTF(title.toStdString().c_str());
    jstring jBody = jenv->NewStringUTF(body.toStdString().c_str());

    jmethodID jResultCtor = jenv->GetMethodID(jResultClass, "<init>", "(Ljava/lang/String;Ljava/lang/String;)V");
    jobject jResult = jenv->NewObject(jResultClass, jResultCtor, jTitle, jBody);

    __android_log_print(ANDROID_LOG_DEBUG, lcPushHandlerService,
                        "Right before decrypted notification will be returned.");

    return jResult;
}

int main(int argc, char *argv[])
{
    __android_log_print(ANDROID_LOG_DEBUG, lcPushHandlerService, "JNI Service started.");

    QAndroidService app(argc, argv);

    return app.exec();
}
