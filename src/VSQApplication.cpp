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

#include "Platform.h"
#include "PlatformUpdates.h"
#include "CustomerEnv.h"
#include "Utils.h"
#include "VSQApplication.h"
#include "VSQClipboardProxy.h"
#include "VSQCustomer.h"
#include "Logging.h"
#include "VSQUiHelper.h"

#include <QDesktopServices>
#include <QFont>
#include <QGuiApplication>
#include <QtCore>
#include <QtQml>

using namespace vm;
using namespace platform;

using Self = VSQApplication;

/******************************************************************************/
Self::VSQApplication()
    : m_settings(this),
      m_engine(new QQmlApplicationEngine(this)),
      m_validator(new Validator(this)),
      m_messenger(&m_settings, m_validator),
      m_userDatabase(new UserDatabase(m_settings.databaseDir(), nullptr)),
      m_models(&m_messenger, &m_settings, m_userDatabase, m_validator, this),
      m_databaseThread(new QThread()),
      m_controllers(&m_messenger, &m_settings, &m_models, m_userDatabase, this),
      m_keyboardEventFilter(new KeyboardEventFilter(this)),
      m_applicationStateManager(&m_messenger, &m_controllers, &m_models, m_validator, this)
{
    m_settings.print();

    qRegisterMetaType<qsizetype>("qsizetype");
    qRegisterMetaType<KeyboardEventFilter *>("KeyboardEventFilter*");

    connect(&m_models, &Models::notificationCreated, this, &Self::notificationCreated);
    connect(&m_controllers, &Controllers::notificationCreated, this, &Self::notificationCreated);

    QThread::currentThread()->setObjectName("MainThread");
    m_userDatabase->moveToThread(m_databaseThread);
    m_databaseThread->setObjectName("DatabaseThread");
    m_databaseThread->start();
}

Self::~VSQApplication()
{
    m_databaseThread->quit();
    m_databaseThread->wait();
    delete m_userDatabase;
    delete m_databaseThread;
}

/******************************************************************************/
void Self::initialize()
{
    // Attributes
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

    // Organization params
    QCoreApplication::setOrganizationName(Customer::OrganizationName);
    QCoreApplication::setOrganizationDomain(Customer::OrganizationDomain);

    // Application params
    QCoreApplication::setApplicationName(Customer::ApplicationName);
    QGuiApplication::setApplicationDisplayName(Customer::ApplicationDisplayName);

    // Loggers
#if !ENABLE_XMPP_LOGS && !ENABLE_XMPP_EXTRA_LOGS
    QLoggingCategory::setFilterRules("core-messenger-xmpp.debug=false");
#endif

#if (VSQ_WEBDRIVER_DEBUG)
    wd_setup(argc, argv);
#endif
}

/******************************************************************************/
int Self::run(const QString &basePath, Logging *logging)
{

    VSQUiHelper uiHelper;

    QUrl url;
    if (basePath.isEmpty()) {
        url = QStringLiteral("qrc:/qml/");
    } else {
        url = "file://" + basePath + "/qml/";
    }
    m_engine->setBaseUrl(url);
    m_engine->addImportPath(url.toString());

    QQmlContext *context = m_engine->rootContext();
    context->setContextProperty("UiHelper", &uiHelper);
    context->setContextProperty("app", this);
    context->setContextProperty("clipboard", new VSQClipboardProxy(QGuiApplication::clipboard()));
    context->setContextProperty("messenger", &m_messenger);
    context->setContextProperty("logging", logging);
    context->setContextProperty("crashReporter", m_messenger.crashReporter());
    context->setContextProperty("settings", &m_settings);
    context->setContextProperty("controllers", &m_controllers);
    context->setContextProperty("models", &m_models);

    QFont fon(QGuiApplication::font());
    fon.setPointSize(1.5 * QGuiApplication::font().pointSize());
    QGuiApplication::setFont(fon);

    connect(qApp, &QGuiApplication::applicationStateChanged, this, &Self::onApplicationStateChanged);
    connect(qApp, &QGuiApplication::aboutToQuit, this, &Self::onAboutToQuit);

    reloadQml();

    if (!Platform::instance().init()) {
        // TODO: Improve - show error and then quit.
        qCritical("Unable to prepare platform");
        return -1;
    }

    PlatformUpdates::instance().startChecking();

    const auto result = QGuiApplication::instance()->exec();
    m_engine.reset(); // Engine must be deleted first since it uses most classes
    return result;
}

void Self::reloadQml()
{
    const QUrl url(QStringLiteral("main.qml"));
    m_engine->clearComponentCache();
    m_engine->load(url);
}

bool Self::isUpdatesSupported()
{
    return PlatformUpdates::instance().isSupported();
}

void Self::checkUpdates()
{
    PlatformUpdates::instance().checkNow();
}

QString Self::organizationDisplayName() const
{
    return Customer::OrganizationDisplayName;
}

QString Self::applicationDisplayName() const
{
    return Customer::ApplicationDisplayName;
}

QString Self::currentVersion() const
{
    return CustomerEnv::version();
}

void Self::uiInit()
{
    Platform::instance().uiInit();
}

bool Self::isIosSimulator() const
{
#if VS_IOS_SIMULATOR
    return true;
#else
    return false;
#endif
}

Qt::KeyboardModifiers Self::keyboardModifiers() const
{
    return qApp->keyboardModifiers();
}

void Self::onApplicationStateChanged(Qt::ApplicationState state)
{
    qDebug() << state;
    m_messenger.setApplicationActive(Qt::ApplicationState::ApplicationActive == state);

    if (Qt::ApplicationState::ApplicationSuspended == state) {
        m_messenger.suspend();
    }
}

void Self::onAboutToQuit()
{
    qDebug() << "Application about to quit";
    m_messenger.setApplicationActive(false);
    m_settings.setRunFlag(false);
}

ApplicationStateManager *Self::stateManager()
{
    return &m_applicationStateManager;
}
