#ifndef __CONTROLLER_H__
#define __CONTROLLER_H__

#include "VSQInfoGeneralController.h"
#include "VSQInfoGeneralModel.h"


// Qt headers
#include <QObject>

class QQuickView;

class VSQController : public QObject {
    Q_OBJECT

public:
    explicit VSQController(QObject *parent = 0);
    ~VSQController();
    void
    setupUI();


private:
    QQuickView *m_pUIContext;
    VSQInfoGeneralController *m_pVSInfoGeneralController;
};

#endif // __CONTROLLER_H__
