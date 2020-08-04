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
#include "VSQCrashReporter.h"
#include "VSQLogging.h"
#include "VSQMessenger.h"
#include "VSQQmlEngine.h"
#include "VSQSettings.h"
#include "ui/VSQUiHelper.h"
#include "macos/VSQMacos.h"

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

Q_LOGGING_CATEGORY(lcApplication, "application")

VSQApplication::VSQApplication(int &argc, char **argv)
    : ApplicationBase(argc, argv)
{
    setupCore();

    m_settings = new VSQSettings(this);
    m_crashReporter = new VSQCrashReporter(m_settings, this);
    m_messenger = new VSQMessenger(m_settings, m_crashReporter, this);
    m_engine = new VSQQmlEngine(argc, argv, this);

    setupFonts();
    setupConnections();
    setupEngine();

#if (MACOS)
    VSQMacos::instance().startUpdatesTimer();
#endif
}

VSQApplication::~VSQApplication()
{
    // Delete objects in another order, so settings can be used in destructors
    delete m_engine;
    delete m_messenger;
    delete m_crashReporter;
    delete m_settings;

#ifdef VS_DEVMODE
    qCDebug(lcDev) << "~Application";
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

void VSQApplication::setupCore()
{
    const auto appConfig = VSQAppConfig() << VirgilIoTKit::VS_LOGLEV_DEBUG;
    if (!VSQIoTKitFacade::instance().init(VSQFeatures(), VSQImplementations(), appConfig))
        qCCritical(lcApplication) << "Unable to initialize Virgil IoT KIT";

    VSQLogging::instance()->installMessageHandler();
    connect(VSQLogging::instance(), &VSQLogging::fatal, this, &VSQApplication::quit);
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
    connect(m_messenger, &VSQMessenger::signedIn, m_crashReporter, &VSQCrashReporter::check);
    connect(m_crashReporter, &VSQCrashReporter::checked, m_settings, std::bind(&VSQSettings::setRunFlag, m_settings, true));
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
    context->setContextProperty("crashReporter", m_crashReporter);
    context->setContextProperty("logging", VSQLogging::instance());

    reloadQml();
}

void VSQApplication::onApplicationStateChanged(Qt::ApplicationState state)
{
    qCDebug(lcApplication) << "Application state:" << state;
#if VS_PUSHNOTIFICATIONS
    static bool _deactivated = false;

    if (Qt::ApplicationInactive == state) {
        _deactivated = true;
        m_messenger->setOnlineStatus(false);
    }

    if (Qt::ApplicationActive == state && _deactivated) {
        _deactivated = false;
        QTimer::singleShot(200, m_messenger, &VSQMessenger::checkConnectionState);
    }
#endif // VS_PUSHNOTIFICATIONS
}
