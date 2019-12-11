#ifndef __VSInfoGeneralController_H__
#define __VSInfoGeneralController_H__

#include "VSQInfoGeneralModel.h"

#include <QtCore>
#include <QtQml/QQmlListProperty>

class VSQDeviceInfo;

class VSQInfoGeneralController : public QObject {
    Q_OBJECT
    Q_PROPERTY(QQmlListProperty<VSQInfoGeneralModel> vsInfoGeneralList READ getVSInfoGeneralList)

public:
    explicit VSQInfoGeneralController(QObject *parent = nullptr) : QObject(parent) {
    }
    virtual ~VSQInfoGeneralController();

    QQmlListProperty<VSQInfoGeneralModel>
    getVSInfoGeneralList();

public slots:
    void
    newDevice(const VSQDeviceInfo &deviceInfo);

    void
    changeDeviceState(const VSQDeviceInfo &deviceInfo);

private:
    QList<VSQInfoGeneralModel *> m_deviceModelsList;
};

#endif // __VSInfoGeneralController_H__
