#include <VSQController.h>

#include <QtGui/QGuiApplication>
#include <QtQml>
#include <QFont>
#include <QStringList>
#include <QQuickView>

VSQController::VSQController(QObject *parent) : QObject(parent),
    m_qmlEngine(new QQmlApplicationEngine(this)) {
    m_deviceInfoController = new VSQDeviceInfoController();
}

void
VSQController::setupUI() {
    qmlRegisterType<VSQDeviceInfoModel>("demo-iotkit-qt", 1, 0, "VSQDeviceInfoModel");

    QQmlContext *rootContext = m_qmlEngine->rootContext();
    rootContext->setContextProperty("application", this);
    rootContext->setContextProperty("VSQDeviceInfoController", QVariant::fromValue(m_deviceInfoController));
    m_qmlEngine->load(QUrl(QStringLiteral("qrc:/main.qml")));
}

void
VSQController::onDeviceInfo(const VSQDeviceInfo &deviceInfo) {
    Q_CHECK_PTR(m_deviceInfoController);
    m_deviceInfoController->change(deviceInfo);
}

void
VSQController::onNewDevice(const VSQDeviceInfo &deviceInfo) {
    Q_CHECK_PTR(m_deviceInfoController);
    m_deviceInfoController->change(deviceInfo);
}
