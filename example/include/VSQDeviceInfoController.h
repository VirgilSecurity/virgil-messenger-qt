#ifndef __VSInfoGeneralController_H__
#define __VSInfoGeneralController_H__

#include "VSQDeviceInfoModel.h"

#include <QtCore>
#include <QtQml/QQmlListProperty>

class VSQDeviceInfoController : public QObject {
    Q_OBJECT
    Q_PROPERTY(QQmlListProperty<VSQDeviceInfoModel> deviceInfoList READ getDeviceInfoList NOTIFY deviceInfoListChanged)

public:
    explicit VSQDeviceInfoController(QObject *parent = NULL);
    virtual ~VSQDeviceInfoController();

    QQmlListProperty<VSQDeviceInfoModel>
    getDeviceInfoList();

    void
    change(const VSQDeviceInfo &deviceInfo);

signals:
    void
    deviceInfoListChanged();

private:
    QList<VSQDeviceInfoModel *> m_deviceInfoList;
    VSQDeviceInfoModel *m_currentDeviceInfoModel;
};


#endif // __VSInfoGeneralController_H__
