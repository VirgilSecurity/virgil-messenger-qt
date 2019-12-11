#include <VSQInfoGeneralController.h>

void
VSQInfoGeneralController::newDevice(const VSQDeviceInfo &deviceInfo) {
    auto deviceModel = new VSQInfoGeneralModel;

    m_deviceModelsList.append(deviceModel);
}

void
VSQInfoGeneralController::changeDeviceState(const VSQDeviceInfo &deviceInfo) {

    for (auto &deviceModel : m_deviceModelsList) {
        if (deviceModel->mac() == deviceInfo.m_mac.description()) {
            deviceModel->update(deviceInfo);
            return;
        }
    }

    Q_ASSERT(false && "changeDeviceState was called for device that is not present in the list");
}

QQmlListProperty<VSQInfoGeneralModel>
VSQInfoGeneralController::getVSInfoGeneralList() {
    return QQmlListProperty<VSQInfoGeneralModel>(this, m_deviceModelsList);
}