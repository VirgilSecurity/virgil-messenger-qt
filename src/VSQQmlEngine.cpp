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

#include "VSQQmlEngine.h"

#include "VSQCommon.h"

Q_LOGGING_CATEGORY(lcQmlengine, "qmlengine")

VSQQmlEngine::VSQQmlEngine(int &argc, char **argv, QObject *parent)
    : QQmlApplicationEngine(parent)
{
    parseArgs(argc, argv);
    registerCommonTypes();
}

VSQQmlEngine::~VSQQmlEngine()
{
#ifdef VS_DEVMODE
    qCDebug(lcDev) << "~QmlEngine";
#endif
}

void VSQQmlEngine::reloadQml()
{
    const QUrl url(QStringLiteral("main.qml"));
    clearComponentCache();
    load(url);

#if !defined(Q_OS_ANDROID) && !defined(Q_OS_IOS) && !defined(Q_OS_WATCHOS)
    {
        QObject *rootObject(rootObjects().first());
        rootObject->setProperty("width", 800);
        rootObject->setProperty("height", 640);
    }
#endif
}

void VSQQmlEngine::parseArgs(int &argc, char **argv)
{
    QString basePath;
    if (argc == 2 && argv[1] && argv[1][0]) {
        basePath = QString::fromLocal8Bit(argv[1]);
        qCDebug(lcQmlengine) << "Custom QML base path:" << basePath;
    }
    if (basePath.isEmpty())
        setBaseUrl(QUrl(QLatin1String("qrc:/qml/")));
    else
        setBaseUrl(QUrl(QLatin1String("file://") + basePath + QLatin1String("/qml/")));
}
