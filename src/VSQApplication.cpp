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

#include "VSQApplication.h"

#include <QDesktopServices>
#include <QFont>
#include <QQmlContext>

#include <virgil/iot/qt/VSQIoTKit.h>

#include "VSQClipboardProxy.h"
#include "VSQLogging.h"
#include "VSQMessenger.h"
#include "VSQQmlEngine.h"
#include "VSQSettings.h"
#include "ui/VSQUiHelper.h"
#include "macos/VSQMacos.h"

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

Q_LOGGING_CATEGORY(application, "application")

VSQApplication::VSQApplication(int &argc, char **argv)
    : ApplicationBase(argc, argv)
    , m_settings(new VSQSettings(this))
    , m_messenger(new VSQMessenger(m_settings, this))
    , m_engine(new VSQQmlEngine(argc, argv, this))
    , m_logging(new VSQLogging(this))
{
    setupCore();
    setupLogging();
    setupFonts();
    setupConnections();
    setupEngine();

#if (MACOS)
    VSQMacos::instance().startUpdatesTimer();
#endif
}

void VSQApplication::initialize()
{
    // Organization params
    ApplicationBase::setApplicationName("VirgilMessenger");
    ApplicationBase::setOrganizationName("VirgilSecurity");
    ApplicationBase::setOrganizationDomain("virgil.net");
#if defined(VERSION)
    ApplicationBase::setApplicationVersion(TOSTRING(VERSION));
#else
    ApplicationBase::setApplicationVersion("unknown");
#endif
    // Attributes
    ApplicationBase::setAttribute(Qt::AA_EnableHighDpiScaling);
    ApplicationBase::setAttribute(Qt::AA_UseHighDpiPixmaps);

#ifdef VS_DEVMODE
    qSetMessagePattern("[%{category}.%{type}]: %{message} [at %{file} #%{line}]");
#endif
}

void VSQApplication::reloadQml()
{
    m_engine->reloadQml();
}

void VSQApplication::checkUpdates()
{
#if (MACOS)
    VSQMacos::instance().checkUpdates();
#endif
}

QString VSQApplication::currentVersion() const
{
    return applicationVersion() + "-alpha";
}

void VSQApplication::sendReport()
{
    m_logging->sendLogFiles();
}

void VSQApplication::setupCore()
{
    // TODO(fpohtmeh): refactor
    const auto features = VSQFeatures();
    const auto impl = VSQImplementations();
    const auto appConfig = VSQAppConfig() << VirgilIoTKit::VS_LOGLEV_DEBUG;
    if (!VSQIoTKitFacade::instance().init(features, impl, appConfig))
        VS_LOG_CRITICAL("Unable to initialize Virgil IoT KIT");
}

void VSQApplication::setupLogging()
{
    m_messenger->setLogging(m_logging);
    m_logging->setkApp(applicationName());
    m_logging->setkOrganization(organizationName());
    m_logging->setkVersion(applicationVersion());
}

void VSQApplication::setupFonts()
{
    QFont font(ApplicationBase::font());
    font.setPointSize(1.5 * ApplicationBase::font().pointSize());
    setFont(font);
}

void VSQApplication::setupConnections()
{
    connect(this, &VSQApplication::applicationStateChanged, this, &VSQApplication::onApplicationStateChanged);
    connect(m_messenger, &VSQMessenger::quitRequested, this, &VSQApplication::quit);
}

void VSQApplication::setupEngine()
{
    QQmlContext *context = m_engine->rootContext();
    context->setContextProperty("app", this);
    context->setContextProperty("UiHelper", new VSQUiHelper(this));
    context->setContextProperty("clipboard", new VSQClipboardProxy(clipboard()));
    context->setContextProperty("SnapInfoClient", &VSQSnapInfoClientQml::instance());
    context->setContextProperty("SnapSniffer", VSQIoTKitFacade::instance().snapSniffer().get());
    context->setContextProperty("messenger", m_messenger);
    context->setContextProperty("settings", m_settings);
    context->setContextProperty("logging", m_logging);

    reloadQml();
}

void VSQApplication::onApplicationStateChanged(Qt::ApplicationState state)
{
    qCDebug(application) << "Application state:" << state;
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
