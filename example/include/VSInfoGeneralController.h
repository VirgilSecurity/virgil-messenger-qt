#ifndef __VSInfoGeneralController_H__
#define __VSInfoGeneralController_H__

#include "VSInfoGeneralModel.h"

#include <QObject>
#include <QtCore/QList>
#include <QtQml/QQmlListProperty>

class VSInfoGeneralController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QQmlListProperty<VSInfoGeneralModel> vsInfoGeneralList READ getVSInfoGeneralList NOTIFY vsInfoGeneralListChanged)
    Q_PROPERTY(VSInfoGeneralModel* currentVSInfoGeneral READ getCurrentVSInfoGeneral NOTIFY currentVSInfoGeneralChanged)
public:

    explicit VSInfoGeneralController( QObject* parent = NULL );
    virtual ~VSInfoGeneralController();

    QQmlListProperty<VSInfoGeneralModel> getVSInfoGeneralList();
    VSInfoGeneralModel* getCurrentVSInfoGeneral();

signals:
    void vsInfoGeneralListChanged();
    void currentVSInfoGeneralChanged();

private:
    QList<VSInfoGeneralModel*> m_VSInfoGeneralList;
    VSInfoGeneralModel* m_CurrentVSInfoGeneralModel;
};

#endif // __VSInfoGeneralController_H__
