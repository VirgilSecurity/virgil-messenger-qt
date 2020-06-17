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

#include <QtCore>
#include <QtQml>

#include <VSQApplication.h>
#include <VSQClipboardProxy.h>
#include <ui/VSQUiHelper.h>
#include <virgil/iot/logger/logger.h>

#include <QGuiApplication>
#include <QFont>
#include <QDesktopServices>

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

#if defined(VERSION)
const QString VSQApplication::kVersion = QString(TOSTRING(VERSION));
#else
const QString VSQApplication::kVersion = "unknown";
#endif

/******************************************************************************/
VSQApplication::VSQApplication() {
#if (MACOS)
    VSQMacos::instance().startUpdatesTimer();
#endif
}

/******************************************************************************/
int
VSQApplication::run(const QString &basePath) {

    VSQUiHelper uiHelper;

    QQmlContext *context = m_engine.rootContext();
    if (basePath.isEmpty()) {
        m_engine.setBaseUrl(QUrl(QStringLiteral("qrc:/qml/")));
    } else {
        QUrl url("file://" + basePath + "/qml/");
        m_engine.setBaseUrl(url);
    }

    context->setContextProperty("UiHelper", &uiHelper);
    context->setContextProperty("app", this);
    context->setContextProperty("clipboard", new VSQClipboardProxy(QGuiApplication::clipboard()));
    context->setContextProperty("Messenger", &m_messenger);
    context->setContextProperty("ConversationsModel", &m_messenger.modelConversations());
    context->setContextProperty("ChatModel", &m_messenger.getChatModel());

    QFont fon(QGuiApplication::font());
    fon.setPointSize(1.5 * QGuiApplication::font().pointSize());
    QGuiApplication::setFont(fon);

    connect(QGuiApplication::instance(),
            SIGNAL(applicationStateChanged(Qt::ApplicationState)),
            this,
            SLOT(onApplicationStateChanged(Qt::ApplicationState)));

    reloadQml();

    return QGuiApplication::instance()->exec();
}

/******************************************************************************/
void VSQApplication::reloadQml() {
    const QUrl url(QStringLiteral("main.qml"));
    m_engine.clearComponentCache();
    m_engine.load(url);

#if !defined(Q_OS_ANDROID) && !defined(Q_OS_IOS) && !defined(Q_OS_WATCHOS)
    {
        QObject *rootObject(m_engine.rootObjects().first());
        rootObject->setProperty("width", 800);
        rootObject->setProperty("height", 640);
    }
#endif
}

/******************************************************************************/
void VSQApplication::checkUpdates() {
#if (MACOS)
    VSQMacos::instance().checkUpdates();
#endif
}

/******************************************************************************/
QString
VSQApplication::currentVersion() const {
    return kVersion + "-alpha";
}

/******************************************************************************/
void
VSQApplication::sendReport() {
    QDesktopServices::openUrl(QUrl("mailto:?to=kutashenko@gmail.com&subject=Virgil Messenger Report&body=Here is some email body text", QUrl::TolerantMode));
}

/******************************************************************************/
void
VSQApplication::onApplicationStateChanged(Qt::ApplicationState state) {
    static bool _deactivated = false;
    qDebug() << state;

    if (Qt::ApplicationInactive == state) {
        _deactivated = true;
        m_messenger.setStatus(VSQMessenger::MSTATUS_UNAVAILABLE);
    }

    if (Qt::ApplicationActive == state && _deactivated) {
        _deactivated = false;
        QTimer::singleShot(200, [this]() {
            this->m_messenger.checkState();
        });
    }
}


/******************************************************************************/
