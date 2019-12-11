#ifndef _DEMO_IOTKIT_QT_INFO_GENERAL_MODEL_H
#define _DEMO_IOTKIT_QT_INFO_GENERAL_MODEL_H

#include <VSQSnapINFOClient.h>
#include <QObject>

class VSQInfoGeneralModel : public QObject {
    Q_OBJECT

    Q_PROPERTY(QString mac READ mac WRITE mac NOTIFY fireMacChanged)
    Q_PROPERTY(QString deviceRoles READ deviceRoles NOTIFY fireDeviceRolesChanged)
    Q_PROPERTY(QString manufactureId READ manufactureId WRITE manufactureId NOTIFY fireManufactureIdChanged)
    Q_PROPERTY(QString deviceType READ deviceType WRITE deviceType NOTIFY fireDeviceTypeChanged)
    Q_PROPERTY(QString fwVer READ fwVer WRITE fwVer NOTIFY fireFwVerChanged)
    Q_PROPERTY(QString tlVer READ tlVer WRITE tlVer NOTIFY fireTlVerChanged)
    Q_PROPERTY(quint32 sent READ sent WRITE sent NOTIFY fireSentChanged)
    Q_PROPERTY(quint32 received READ received WRITE received NOTIFY fireReceivedChanged)
    Q_PROPERTY(bool active READ active WRITE active NOTIFY fireActiveChanged)
    Q_PROPERTY(QString lastTimestamp READ lastTimestamp WRITE lastTimestamp NOTIFY fireLastTimestampChanged)

public:
    VSQInfoGeneralModel() = default;
    VSQInfoGeneralModel(const VSQDeviceInfo &deviceInfo);

    void
    update(const VSQDeviceInfo &deviceInfo);

    QString
    mac() const;
    QString
    deviceRoles() const;
    QString
    manufactureId() const;
    QString
    deviceType() const;
    QString
    fwVer() const;
    QString
    tlVer() const;
    quint32
    sent() const;
    quint32
    received() const;
    bool
    active() const;
    QString
    lastTimestamp() const;

    void
    mac(QString mac);
    void
    deviceRoles(QString deviceRoles);
    void
    manufactureId(QString manufactureId);
    void
    deviceType(QString deviceType);
    void
    fwVer(QString fwVer);
    void
    tlVer(QString tlVer);
    void
    sent(quint32 sent);
    void
    received(quint32 received);
    void
    active(bool active);
    void
    lastTimestamp(QString lastTimestamp);

signals:
    void
    fireMacChanged();
    void
    fireDeviceRolesChanged();
    void
    fireManufactureIdChanged();
    void
    fireDeviceTypeChanged();
    void
    fireFwVerChanged();
    void
    fireTlVerChanged();
    void
    fireSentChanged();
    void
    fireReceivedChanged();
    void
    fireActiveChanged();
    void
    fireLastTimestampChanged();

public slots:
    void
    deviceStateChanged(const VSQDeviceInfo &deviceInfo);

private:
    VSQDeviceInfo m_deviceInfo;
};

#endif // _DEMO_IOTKIT_QT_INFO_GENERAL_MODEL_H
