#include <VSQInfoGeneralModel.h>

VSQInfoGeneralModel::VSQInfoGeneralModel(const VSQDeviceInfo &deviceInfo) {
    m_deviceInfo = deviceInfo;

    mac(m_deviceInfo.m_mac);
    deviceRoles(m_deviceInfo.m_deviceRoles);
    manufactureId(m_deviceInfo.m_manufactureId);
    deviceType(m_deviceInfo.m_deviceType);
    fwVer(m_deviceInfo.m_fwVer);
    tlVer(m_deviceInfo.m_tlVer);
    sent(m_deviceInfo.m_sent);
    received(m_deviceInfo.m_received);
    active(m_deviceInfo.m_isActive ? "Active" : "NOT active");
    lastTimestamp(m_deviceInfo.m_lastTimestamp.toString(Qt::SystemLocaleShortDate));
}

void
VSQInfoGeneralModel::deviceStateChanged(const VSQDeviceInfo &deviceInfo) {

    bool deviceRolesChanged = false;
    bool manufactureIdChanged = false;
    bool deviceTypeChanged = false;
    bool fwVerChanged = false;
    bool tlVerChanged = false;
    bool sentChanged = false;
    bool receivedChanged = false;
    bool activeChanged = false;
    bool lastTimestampChanged = false;

    if (m_deviceInfo == deviceInfo)
        return;

    deviceRolesChanged = m_deviceInfo.m_deviceRoles != deviceInfo.m_deviceRoles;
    manufactureIdChanged = m_deviceInfo.m_manufactureId != deviceInfo.m_manufactureId;
    deviceTypeChanged = m_deviceInfo.m_deviceType != deviceInfo.m_deviceType;
    fwVerChanged = m_deviceInfo.m_fwVer != deviceInfo.m_fwVer;
    tlVerChanged = m_deviceInfo.m_tlVer != deviceInfo.m_tlVer;
    sentChanged = m_deviceInfo.m_sent != deviceInfo.m_sent;
    receivedChanged = m_deviceInfo.m_received != deviceInfo.m_received;
    activeChanged = m_deviceInfo.m_isActive != deviceInfo.m_isActive;
    lastTimestampChanged = m_deviceInfo.m_lastTimestamp != deviceInfo.m_lastTimestamp;
}

QString
VSQInfoGeneralModel::mac() const{

}

QString
VSQInfoGeneralModel::deviceRoles() const{

}

QString
VSQInfoGeneralModel::manufactureId() const{

}

QString
VSQInfoGeneralModel::deviceType() const{

}

QString
VSQInfoGeneralModel::fwVer() const{

}

QString
VSQInfoGeneralModel::tlVer() const{

}

quint32
VSQInfoGeneralModel::sent() const{

}

quint32
VSQInfoGeneralModel::received() const{

}

bool
VSQInfoGeneralModel::active() const{

}

QString
VSQInfoGeneralModel::lastTimestamp() const{

}


void
VSQInfoGeneralModel::mac(QString mac){

}

void
VSQInfoGeneralModel::deviceRoles(QString deviceRoles){

}

void
VSQInfoGeneralModel::manufactureId(QString manufactureId){

}

void
VSQInfoGeneralModel::deviceType(QString deviceType){

}

void
VSQInfoGeneralModel::fwVer(QString fwVer){

}

void
VSQInfoGeneralModel::tlVer(QString tlVer){

}

void
VSQInfoGeneralModel::sent(quint32 sent){

}

void
VSQInfoGeneralModel::received(quint32 received){

}

void
VSQInfoGeneralModel::active(bool active){

}

void
VSQInfoGeneralModel::lastTimestamp(QString lastTimestamp){

}

