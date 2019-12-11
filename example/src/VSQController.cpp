#include <VSQController.h>

#include <QtGui/QGuiApplication>
#include <QtQml>
#include <QFont>
#include <QStringList>
#include <QQuickView>

VSQController::VSQController(QObject *parent) : QObject(parent), m_pUIContext(nullptr) {

    m_pVSInfoGeneralController = new VSQInfoGeneralController();
}

VSQController::~VSQController() {
    delete m_pUIContext;
    delete m_pVSInfoGeneralController;
}

void
VSQController::setupUI() {
    m_pUIContext = new QQuickView;
    m_pUIContext->setTitle(QString("demo-iotkit-qt - ") + QString(VERSION));

    QQmlContext *rootContext = m_pUIContext->rootContext();
    rootContext->setContextProperty("application", this);
    rootContext->setContextProperty("vsInfoGeneralController", QVariant::fromValue(m_pVSInfoGeneralController));

    qmlRegisterType<VSQInfoGeneralModel>("demo-iotkit-qt", 1, 0, "VSQInfoGeneralModel");

    QFont f = qApp->font();
    f.setPixelSize(12);
    qApp->setFont(f);
    m_pUIContext->setResizeMode(QQuickView::SizeRootObjectToView);
    m_pUIContext->setSource(QUrl("qrc:/main.qml"));

    m_pUIContext->show();
}
