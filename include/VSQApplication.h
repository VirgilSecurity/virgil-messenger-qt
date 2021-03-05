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

#ifndef VIRGIL_IOTKIT_QT_DEMO_VSQAPP_H
#define VIRGIL_IOTKIT_QT_DEMO_VSQAPP_H

#include <QtCore>
#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include "KeyboardEventFilter.h"
#include "Validator.h"
#include "Messenger.h"
#include "Settings.h"
#if VS_MACOS
#    include <macos/VSQMacos.h>
#endif // VS_MACOS
#include "Controllers.h"
#include "UserDatabase.h"
#include "Models.h"
#include "ApplicationStateManager.h"

class QNetworkAccessManager;

class VSQLogging;

class VSQApplication : public QObject
{
    Q_OBJECT
    Q_PROPERTY(vm::ApplicationStateManager *stateManager READ stateManager CONSTANT)
    Q_PROPERTY(QString organizationDisplayName READ organizationDisplayName CONSTANT)
    Q_PROPERTY(QString applicationDisplayName READ applicationDisplayName CONSTANT)
    Q_PROPERTY(vm::KeyboardEventFilter *keyboardEventFilter MEMBER m_keyboardEventFilter CONSTANT)
    Q_PROPERTY(vm::Validator *validator MEMBER m_validator CONSTANT)

public:
    VSQApplication();
    ~VSQApplication() override;

    static void initialize();

    int run(const QString &basePath, VSQLogging *logging);

    Q_INVOKABLE
    void reloadQml();

    Q_INVOKABLE
    void checkUpdates();

    Q_INVOKABLE QString currentVersion() const;

    // Names

    QString organizationDisplayName() const;
    QString applicationDisplayName() const;

    Q_INVOKABLE bool isIosSimulator() const;
    Q_INVOKABLE Qt::KeyboardModifiers keyboardModifiers() const;

signals:
    void notificationCreated(const QString &notification, const bool error);

private:
    void onApplicationStateChanged(Qt::ApplicationState state);
    void onAboutToQuit();

    vm::ApplicationStateManager *stateManager();

    static const QString kVersion;
    Settings m_settings;
    QScopedPointer<QQmlApplicationEngine> m_engine;
    vm::Validator *m_validator;
    vm::Messenger m_messenger;
    vm::UserDatabase *m_userDatabase;
    vm::Models m_models;
    QThread *m_databaseThread;
    vm::Controllers m_controllers;
    vm::KeyboardEventFilter *m_keyboardEventFilter;
    vm::ApplicationStateManager m_applicationStateManager;
};

#endif // VSQApplication
