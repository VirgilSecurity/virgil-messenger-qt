#ifndef __CONTROLLER_H__
#define __CONTROLLER_H__

#include "VSInfoGeneralController.h"
#include "VSInfoGeneralModel.h"


// Qt headers
#include <QObject>

class QQuickView;

class Controller : public QObject {
    Q_OBJECT

public:
    explicit Controller(QObject *parent = 0);
    ~Controller();
    void
    setupUI();


private:
    QQuickView *m_pUIContext;
    VSInfoGeneralController *m_pVSInfoGeneralController;
};

#endif // __CONTROLLER_H__
