#ifndef __CONTROLLER_H__
#define __CONTROLLER_H__

#include <VSQDeviceInfoController.h>
#include <VSQDeviceInfoModel.h>


// Qt headers
#include <QObject>

class QQuickView;

class VSQController : public QObject {
    Q_OBJECT

public:
    explicit VSQController(QObject *parent = 0);
    ~VSQController() = default;
    void
    setupUI();

public slots:
    void
    onDeviceInfo(const VSQDeviceInfo &deviceInfo);

    void
    onNewDevice(const VSQDeviceInfo &deviceInfo);

private:
    QSharedPointer<QQuickView> m_context;
    VSQDeviceInfoController *m_deviceInfoController;
};

#endif // __CONTROLLER_H__
