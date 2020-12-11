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
#include "CommKitUser.h"
#include "CommKitMessenger.h"
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

    Q_PROPERTY(QString currentUser READ currentUser NOTIFY fireCurrentUserChanged)

    Messenger(Settings *settings, Validator *validator);
    Messenger() = default; // QML engine requires default constructor
    virtual ~Messenger() noexcept = default;

    //
    //  Info.
    //
    Q_INVOKABLE QString currentUser() const;
    Q_INVOKABLE QString currentRecipient() const;
    Q_INVOKABLE bool isOnline() const noexcept;

    //
    //  Messages.
    //
    virtual bool sendMessage(const GlobalMessage& message) override;

    //
    // User control.
    //
    void signIn(const QString &userId);
    void signOut();
    void signUp(const QString &userId);
    void backupKey(const QString &password, const QString &confirmedPassword);
    void downloadKey(const QString &userId, const QString &password);

    //
    // Find users.
    //
    QSharedPointer<CommKitUser> findUserByUsername(const QString &username);
    QSharedPointer<CommKitUser> findUserById(const QString &id);

    //
    //  Helpers.
    //
    void setApplicationActive(bool active);

    QPointer<CrashReporter> crashReporter() noexcept;
    QPointer<FileLoader> fileLoader() noexcept;

public slots:
    void setCurrentRecipient(const QString &recipient);

    bool subscribeToContact(const Contact::Id &contactId);

signals:
    void
    fireError(QString errorText);

    void
    fireNewMessage(QString from, QString message);

    void
    fireCurrentUserChanged();

    //
    // Sign-in statuses.
    //
    void signedIn(const QString &userId);
    void signInErrorOccured(const QString &errorText);
    void signedUp(const QString &userId);
    void signUpErrorOccured(const QString &errorText);
    void signedOut();

    //
    // Connection statuses.
    //
    void onlineStatusChanged(bool isOnline);

    //
    //  Key management statuses.
    //
    void keyBackuped(const QString &userId);
    void backupKeyFailed(const QString &errorText);
    void keyDownloaded(const QString &userId);
    void downloadKeyFailed(const QString &errorText);

    //
    //  Messages and user activity statuses.
    //
    void messageStatusChanged(const Message::Id &messageId, const QString &recipeintId, const Message::Status &status);
    void messageReceived(const GlobalMessage& message);
    void messageDelivered(const QString& userId, const QString& messageId);

    void lastActivityTextChanged(const QString& text);

    //--
    //  File upload.
    //
public:
    QString requestUploadSlot(const QString &filePath);

signals:
    void uploadServiceFound(bool found);
    void uploadSlotReceived(const QString &slotId, const QUrl &putUrl, const QUrl &getUrl);
    void uploadSlotErrorOcurred(const QString &slotId, const QString &errorText);
    //--

private slots:
    void onPushNotificationTokenUpdate();
    void onConnectionStateChanged(CommKitMessenger::ConnectionState state);
    void onMessageReceived(const CommKitMessage& message);

private:
    QPointer<Settings> m_settings;
    QPointer<Validator> m_validator;
    QPointer<CommKitMessenger> m_commKitMessenger;
    QPointer<CrashReporter> m_crashReporter;
    QPointer<FileLoader> m_fileLoader;
    QString m_currentRecipient;
};
} // namespace vm

#endif // VIRGIL_IOTKIT_QT_MESSENGER_H
