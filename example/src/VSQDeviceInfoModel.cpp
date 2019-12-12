#include <VSQDeviceInfoModel.h>

void
VSQDeviceInfoModel::change(const VSQDeviceInfo &deviceInfo) {

    m_deviceInfo = deviceInfo;

    emit deviceStateChanged();
    emit macDeviceRolesChanged();
    emit manufactureIdDeviceTypeChanged();
    emit fwTlVerChanged();
    emit sentReceivedChanged();
    emit lastTimestampChanged();
}

QString
VSQDeviceInfoModel::deviceState() const {
    if (!m_deviceInfo.m_isActive) {
        return "Inactive";
    } else if (!m_deviceInfo.m_hasGeneralInfo) {
        return "Not initialized";
    } else if (!m_deviceInfo.m_hasStatistics) {
        return "Started";
    } else {
        return "Active";
    }
}