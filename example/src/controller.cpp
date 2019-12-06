#include <controller.h>

#include <QtGui/QGuiApplication>
#include <QtQml>
#include <QFont>
#include <QStringList>
#include <QQuickView>

Controller::Controller( QObject* parent )
    : QObject( parent )
    , m_pUIContext ( nullptr ) {

    m_pVSInfoGeneralController = new VSInfoGeneralController();
}

Controller::~Controller() {
    delete m_pUIContext;
    delete m_pVSInfoGeneralController;
}

void Controller::setupUI() {
    m_pUIContext = new QQuickView;
    m_pUIContext->setTitle( QString("demo-iotkit-qt - ")+QString( VERSION ) );

    QQmlContext* rootContext = m_pUIContext->rootContext();
    rootContext->setContextProperty("application", this);
    rootContext->setContextProperty("vsInfoGeneralController", QVariant::fromValue(m_pVSInfoGeneralController));

    qmlRegisterType<VSInfoGeneralModel>("demo-iotkit-qt", 1, 0, "VSInfoGeneralModel");

    QFont f = qApp->font();
    f.setPixelSize(12);
    qApp->setFont( f );
    m_pUIContext->setResizeMode(QQuickView::SizeRootObjectToView);
    m_pUIContext->setSource(QUrl("qrc:/main.qml"));

    m_pUIContext->show();
}
