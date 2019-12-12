#include <VSQController.h>

#include <QtGui/QGuiApplication>
#include <QtQml>
#include <QFont>
#include <QStringList>
#include <QQuickView>

VSQController::VSQController(QObject *parent) : QObject(parent) {
    m_deviceInfoController = new VSQDeviceInfoController();
}

void
VSQController::setupUI() {
    m_context = decltype(m_context)::create();
    m_context->setTitle(QString("demo-iotkit-qt - ") + QString(VERSION));

    QQmlContext *rootContext = m_context->rootContext();
    rootContext->setContextProperty("application", this);
    rootContext->setContextProperty("VSQDeviceInfoController", QVariant::fromValue(m_deviceInfoController));

    qmlRegisterType<VSQDeviceInfoModel>("demo-iotkit-qt", 1, 0, "VSQDeviceInfoModel");

    QFont f = qApp->font();
    f.setPixelSize(12);
    qApp->setFont(f);
    m_context->setResizeMode(QQuickView::SizeRootObjectToView);
    m_context->setSource(QUrl("qrc:/main.qml"));

    m_context->show();
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
