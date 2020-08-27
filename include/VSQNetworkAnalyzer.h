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

#ifndef VSQNETWORKANALYZER_H
#define VSQNETWORKANALYZER_H

#include <QObject>
#include <QNetworkConfigurationManager>
#include <QNetworkSession>
#include <QMap>
#include <QTimer>

#include "VSQCommon.h"

Q_DECLARE_LOGGING_CATEGORY(lcNetwork);

#ifdef Q_OS_MACOS
// TODO: Remove after fixing of deprecated functionality
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

class VSQNetworkAnalyzer : public QObject {
    Q_OBJECT

    typedef QMap<int, QString> VSQNetworkInterfaceData;

public:
    VSQNetworkAnalyzer(QObject *parent = nullptr);
    virtual ~VSQNetworkAnalyzer();

    bool
    isOnline() const;

signals:
    void
    fireStateChanged(bool online);

    void
    fireHeartBeat();

protected slots:
    void
    onUpdateCompleted();

    void
    onAnalyzeNetwork();

protected:
    void
    printConfiguration(const QNetworkConfiguration &configuration) const;

    void
    printSession(const QNetworkSession &session) const;

    void
    printNetworkInterface(const QNetworkInterface &interface) const;

    void
    printMap(const VSQNetworkInterfaceData &networkInterfaceData) const;

    static const int kTimerInterval = 5000;
    static const int kSessionTimeoutMs = 1000;

    QNetworkConfigurationManager m_nwManager;
    bool m_connectedState;
    VSQNetworkInterfaceData m_networkInterfaceData;
    QTimer m_timer;

private slots:
    void
    onStart();

private:
    QThread *m_thread;

    bool
    checkIsNeedStop();
};

#ifdef Q_OS_MACOS
#pragma GCC diagnostic pop
#endif

#endif // VSQNETWORKANALYZER_H
