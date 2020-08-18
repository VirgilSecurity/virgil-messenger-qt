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

#include <qlogging.h>
#include <VSQLogging.h>
#include <VSQMessenger.h>
#include <virgil/iot/qt/VSQIoTKit.h>
#include <virgil/iot/logger/logger.h>

//using namespace VirgilIoTKit;

// Private variables
const QString VSQLogging::endpointSendReport = "/send-logs";

VSQLogging *VSQLogging::m_instance = nullptr;

// Public methods
//***********************************************************************************************************
VSQLogging::VSQLogging(QNetworkAccessManager *networkAccessManager) : QObject(nullptr), manager(networkAccessManager)
{
    if (m_instance) {
        qFatal("Instance of logging already exists!");
    }
    else {
        m_instance = this;
    }
}

VSQLogging::~VSQLogging() {
    resetRunFlag();
    m_instance = nullptr;
}
//***********************************************************************************************************
void VSQLogging::setVirgilUrl(QString VirgilUrl) {
    currentVirgilUrl = VirgilUrl;
    qDebug("Send report URL set to [%s]", qPrintable(currentVirgilUrl));
}
//***********************************************************************************************************
void VSQLogging::setkVersion(QString AppVersion){
    kVersion = AppVersion;
}

//***********************************************************************************************************
void VSQLogging::setkOrganization(QString strkOrganization) {
    kOrganization = strkOrganization;
}

//***********************************************************************************************************
void VSQLogging::setkApp(QString strkApp) {
    kApp = strkApp;
}

//***********************************************************************************************************
void VSQLogging::checkAppCrash() {
    qDebug("Checking previus run flag...");
    if(checkRunFlag()) {
        qCritical("Previus application run is crashed ! Sending log files...");
        emit crashReportRequested();
    } else {
        qDebug("Set run flag to true");
        setRunFlag(true); // Set run flag
    }
}

//***********************************************************************************************************
void VSQLogging::resetRunFlag() {
    qDebug("Reset run flag");
    setRunFlag(false); // Reset run flag
}

// Private methods
//***********************************************************************************************************
bool VSQLogging::checkRunFlag() {
    QSettings settings(kOrganization, kApp);
    return settings.value("RunFlag", false).toBool();
}

//***********************************************************************************************************
void VSQLogging::setRunFlag(bool runState) {
    QSettings settings(kOrganization, kApp);
    settings.setValue("RunFlag", runState);
}

//***********************************************************************************************************
bool VSQLogging::sendLogFiles() {
    QByteArray fileData;
    const QDir writeDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (!writeDir.exists()) {
        qFatal("Directory [%s] not exist.", qPrintable(writeDir.absolutePath()));
        return false;
    }

    QDirIterator fileIterator(writeDir.absolutePath(), QStringList() << "virgil-messenger.log*");
    while (fileIterator.hasNext()) {
        QFile readFile(fileIterator.next());
        if ( readFile.open( QIODevice::ReadOnly | QIODevice::Text ) ) {
            qDebug() << "Read file:" << readFile.fileName() << endl;
            fileData.append(readFile.readAll());
        }
        else
            qDebug() << "Can't open " << readFile.fileName() << readFile.errorString() << endl;
    }

    sendFileToBackendRequest(fileData);
    return true;
}
//***********************************************************************************************************
bool VSQLogging::sendFileToBackendRequest(QByteArray fileData) {
    QNetworkRequest req;
    char *buffBearer = (char *) malloc(1024);
    size_t sizeBearer = 1024;

    vs_messenger_virgil_get_auth_token(buffBearer ,sizeBearer);
    qDebug("Backend token [%s]", buffBearer);

    QString strEndpoint = currentVirgilUrl + endpointSendReport;
    req.setUrl(QUrl(strEndpoint));
    qDebug("Send report to endpoint : [%s]",qPrintable(strEndpoint));
    req.setHeader(QNetworkRequest::ContentTypeHeader,  QString("application/json"));
    req.setRawHeader(QString("Authorization").toUtf8(), buffBearer);
    req.setRawHeader(QString("Version").toUtf8(), qPrintable(kVersion));
    req.setRawHeader(QString("Platform").toUtf8(), qPrintable(QSysInfo::kernelType()));
    QNetworkReply* reply = manager->post(req, fileData);
    connect(reply, SIGNAL(finished()), this, SLOT(endpointReply()));

    return true;
}
// Slots
//***********************************************************************************************************
void VSQLogging::endpointReply(){
    QNetworkReply *reply= qobject_cast<QNetworkReply *>(sender());
    if (reply->error() == QNetworkReply::NoError)
      {
        qDebug("Send report OK");
        emit reportSent("Report send OK");
      }
      else {
        qDebug("Error sending report [%s]", qPrintable(reply->errorString()));
        emit reportSentErr("Report send error: [" + reply->errorString() + "]" );
      }
    reply->deleteLater();
    qDebug("Sending finished");
}
// End class VSQLogging
//***********************************************************************************************************

// Functions
//***********************************************************************************************************
void VSQLogging::logger_qt_redir(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
#ifdef VS_DEVMODE
    emit VSQLogging::m_instance->newMessage(QString("[%1]: %2").arg(context.category, msg));
#endif
    QByteArray localMsg = msg.toLocal8Bit();
    switch (type) {
    case QtDebugMsg:
        vs_logger_message(VS_LOG_MAKE_LEVEL(VS_LOGLEV_DEBUG), context.file, context.line,localMsg.constData());
        //VS_LOG_DEBUG(localMsg.constData());
        break;
    case QtInfoMsg:
        vs_logger_message(VS_LOG_MAKE_LEVEL(VS_LOGLEV_INFO), context.file, context.line,localMsg.constData());
        //VS_LOG_INFO(localMsg.constData());
        break;
    case QtWarningMsg:
        vs_logger_message(VS_LOG_MAKE_LEVEL(VS_LOGLEV_WARNING), context.file, context.line,localMsg.constData());
        //VS_LOG_WARNING(localMsg.constData());
        break;
    case QtCriticalMsg:
        vs_logger_message(VS_LOG_MAKE_LEVEL(VS_LOGLEV_CRITICAL), context.file, context.line,localMsg.constData());
        //VS_LOG_CRITICAL(localMsg.constData());
        break;
    case QtFatalMsg:
        vs_logger_message(VS_LOG_MAKE_LEVEL(VS_LOGLEV_FATAL), context.file, context.line,localMsg.constData());
        //VS_LOG_FATAL(localMsg.constData());
        abort();
    }
}

