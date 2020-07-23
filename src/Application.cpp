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

#include "Application.h"

#include <QDesktopServices>
#include <QFont>
#include <QQmlContext>

#include "ClipboardProxy.h"
#include "Messenger.h"
#include "QmlEngine.h"
#include "Settings.h"
#include "ui/VSQUiHelper.h"
#include "macos/VSQMacos.h"

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

#if defined(VERSION)
const QString Application::kVersion = QString(TOSTRING(VERSION));
#else
const QString Application::kVersion = "unknown";
#endif

Application::Application(int &argc, char **argv)
    : ApplicationBase(argc, argv)
    , m_settings(new Settings(this))
    , m_messenger(new Messenger(m_settings, this))
    , m_engine(new QmlEngine(argc, argv, this))
{
    setupFonts();
    setupConnections();
    setupEngine();

#if (MACOS)
    VSQMacos::instance().startUpdatesTimer();
#endif
}

void Application::initialize()
{
    // TODO(fpohtmeh): add application name, display name and version
    // Organization params
    ApplicationBase::setOrganizationName("VirgilSecurity");
    ApplicationBase::setOrganizationDomain("virgil.net");
    // Attributes
    ApplicationBase::setAttribute(Qt::AA_EnableHighDpiScaling);
    ApplicationBase::setAttribute(Qt::AA_UseHighDpiPixmaps);
}

void Application::reloadQml()
{
    m_engine->reloadQml();
}

void Application::checkUpdates()
{
#if (MACOS)
    VSQMacos::instance().checkUpdates();
#endif
}

QString Application::currentVersion() const
{
    return kVersion + "-alpha";
}

void Application::sendReport()
{
    QDesktopServices::openUrl(QUrl("mailto:?to=kutashenko@gmail.com&subject=Virgil Messenger Report&body=Here is some email body text", QUrl::TolerantMode));
}

void Application::setupFonts()
{
    QFont font(ApplicationBase::font());
    font.setPointSize(1.5 * ApplicationBase::font().pointSize());
    setFont(font);
}

void Application::setupConnections()
{
    connect(this, &Application::applicationStateChanged, this, &Application::onApplicationStateChanged);
    connect(m_messenger, &Messenger::quitRequested, this, &Application::quit);
}

void Application::setupEngine()
{
    QQmlContext *context = m_engine->rootContext();
    context->setContextProperty("app", this);
    context->setContextProperty("UiHelper", new VSQUiHelper(this));
    context->setContextProperty("clipboard", new ClipboardProxy(clipboard()));
    context->setContextProperty("messenger", m_messenger);
    context->setContextProperty("settings", m_settings);

    reloadQml();
}

void Application::onApplicationStateChanged(Qt::ApplicationState state)
{
    qDebug() << "Application state:" << state;
#if VS_PUSHNOTIFICATIONS
    static bool _deactivated = false;

    if (Qt::ApplicationInactive == state) {
        _deactivated = true;
        m_messenger.setOnlineStatus(false);
    }

    if (Qt::ApplicationActive == state && _deactivated) {
        _deactivated = false;
        QTimer::singleShot(200, m_messenger, &Messenger::checkConnectionState);
    }
#endif // VS_PUSHNOTIFICATIONS
}
