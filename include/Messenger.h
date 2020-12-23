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


#ifndef VIRGIL_IOTKIT_QT_MESSENGER_H
#define VIRGIL_IOTKIT_QT_MESSENGER_H

#include "Validator.h"
#include "User.h"
#include "CoreMessenger.h"
#include "CrashReporter.h"
#include "MessageSender.h"
#include "FileLoader.h"

#include <QObject>
#include <QPointer>


Q_DECLARE_LOGGING_CATEGORY(lcMessenger)

namespace vm {
class Messenger : public MessageSender {
    Q_OBJECT

public:

    Messenger(Settings *settings, Validator *validator);
    Messenger() = default; // QML engine requires default constructor
    virtual ~Messenger() noexcept = default;

    //
    //  Status.
    //
    //
    //  Return true if messenger has Internet connection with all services.
    //
    bool isOnline() const noexcept;

    //
    //  Messages.
    //
    virtual bool sendMessage(MessageHandler message) override;

    //
    // User control.
    //
    void signIn(const QString &username);
    void signOut();
    void signUp(const QString &username);
    void backupKey(const QString &password, const QString &confirmedPassword);
    void downloadKey(const QString &username, const QString &password);

    //
    //  Attachment control.
    //
    QString requestUploadSlot(const QString &filePath);

    //
    // Find users.
    //
    UserHandler currentUser() const;
    UserHandler findUserByUsername(const QString &username) const;
    UserHandler findUserById(const UserId &id) const;

    //
    //  Helpers.
    //
    void setApplicationActive(bool active);

    QPointer<CrashReporter> crashReporter() noexcept;
    QPointer<FileLoader> fileLoader() noexcept;

public slots:
    void setCurrentRecipient(const QString &recipient);

    bool subscribeToUser(const UserId &contactId);

signals:
    //--
    // Sign-in.
    //
    void signedIn(const QString &username);
    void signInErrorOccured(const QString &errorText);
    void signedUp(const QString &username);
    void signUpErrorOccured(const QString &errorText);
    void signedOut();
    //--

    //--
    //  Key management.
    //
    void keyBackuped(const QString &username);
    void backupKeyFailed(const QString &errorText);
    void keyDownloaded(const QString &username);
    void downloadKeyFailed(const QString &errorText);
    //--

    //--
    //  Messages and user activity.
    //
    void updateMessage(const MessageUpdate &messageUpdate);
    void messageSent(MessageHandler message);
    void messageReceived(ModifiableMessageHandler message);
    void lastActivityTextChanged(const QString& text);
    //--

    //--
    // Connection.
    //
    void onlineStatusChanged(bool isOnline);
    //--

    //--
    //  File upload.
    //
    void uploadServiceFound(bool found);
    void uploadSlotReceived(const QString &slotId, const QUrl &putUrl, const QUrl &getUrl);
    void uploadSlotErrorOcurred(const QString &slotId, const QString &errorText);
    //--

private slots:
    void onPushNotificationTokenUpdate();
    void onConnectionStateChanged(CoreMessenger::ConnectionState state);
    void onMessageReceived(ModifiableMessageHandler message);

private:
    QPointer<Settings> m_settings;
    QPointer<Validator> m_validator;
    QPointer<CoreMessenger> m_coreMessenger;
    QPointer<CrashReporter> m_crashReporter;
    QPointer<FileLoader> m_fileLoader;
    bool m_isOnline = false;
};
} // namespace vm

#endif // VIRGIL_IOTKIT_QT_MESSENGER_H
