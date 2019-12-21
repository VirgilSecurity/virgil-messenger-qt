#ifndef _DEMO_IOTKIT_QT_INFO_GENERAL_MODEL_H
#define _DEMO_IOTKIT_QT_INFO_GENERAL_MODEL_H

#include <virgil/iot/qt/protocols/snap/VSQSnapINFOClient.h>
#include <QObject>

class VSQDeviceInfoModel : public QObject {
    Q_OBJECT

    Q_PROPERTY(QString macDeviceRoles READ macDeviceRoles NOTIFY fireMacDeviceRoles)
    Q_PROPERTY(QString deviceState READ deviceState NOTIFY fireDeviceState)
    Q_PROPERTY(QString manufactureIdDeviceType READ manufactureIdDeviceType NOTIFY fireManufactureIdDeviceType)
    Q_PROPERTY(QString fwTlVer READ fwTlVer NOTIFY fireFwTlVer)
    Q_PROPERTY(QString statistics READ statistics NOTIFY fireStatistics)
    Q_PROPERTY(QString lastTimestamp READ lastTimestamp NOTIFY fireLastTimestamp)

public:
    VSQDeviceInfoModel() {
    }
    VSQDeviceInfoModel(const VSQDeviceInfo &deviceInfo) {
        m_deviceInfo = deviceInfo;
    }

    const VSQDeviceInfo &
    deviceInfo() const {
        return m_deviceInfo;
    }
    QString
    macDeviceRoles() const {
        return QString("%1\n%2\n")
                .arg(QString(m_deviceInfo.m_mac.description()))
                .arg(QString(m_deviceInfo.m_deviceRoles));
    }
    QString
    deviceState() const;
    QString
    manufactureIdDeviceType() const {
        return m_deviceInfo.m_hasGeneralInfo
                       ? QString("%1\n%2").arg(m_deviceInfo.m_manufactureId).arg(m_deviceInfo.m_deviceType)
                       : "---";
    }
    QString
    fwTlVer() const {
        return m_deviceInfo.m_hasGeneralInfo ? QString("%1\n%2").arg(m_deviceInfo.m_fwVer).arg(m_deviceInfo.m_tlVer)
                                             : "---";
    }
    QString
    statistics() const {
        return m_deviceInfo.m_hasStatistics ? QString("%1\n%2").arg(m_deviceInfo.m_sent).arg(m_deviceInfo.m_received)
                                            : "---";
    }
    QString
    lastTimestamp() const {
        return m_deviceInfo.m_lastTimestamp.toString("hh:mm:ss");
    }

    void
    change(const VSQDeviceInfo &deviceInfo);

signals:

    void
    fireMacDeviceRoles();
    void
    fireDeviceState();
    void
    fireManufactureIdDeviceType();
    void
    fireFwTlVer();
    void
    fireStatistics();
    void
    fireLastTimestamp();

private:
    VSQDeviceInfo m_deviceInfo;
};

#endif // _DEMO_IOTKIT_QT_INFO_GENERAL_MODEL_H
