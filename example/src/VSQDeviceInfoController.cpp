#include <VSQDeviceInfoController.h>

VSQDeviceInfoController::VSQDeviceInfoController(QObject *parent)
    : QObject(parent), m_deviceInfoList(), m_currentDeviceInfoModel(nullptr) {
}

VSQDeviceInfoController::~VSQDeviceInfoController() {
    for (auto model : m_deviceInfoList) {
        delete model;
    }

    delete m_currentDeviceInfoModel;
}

QQmlListProperty<VSQDeviceInfoModel>
VSQDeviceInfoController::getDeviceInfoList() {
    return QQmlListProperty<VSQDeviceInfoModel>(this, m_deviceInfoList);
}

void
VSQDeviceInfoController::change(const VSQDeviceInfo &deviceInfo) {

    // Change existing device info
    for (auto device : m_deviceInfoList) {
        if (device->deviceInfo().m_mac == deviceInfo.m_mac) {
            device->change(deviceInfo);
            return;
        }
    }

    // Add new device
    VSQDeviceInfoModel *deviceInfoModel = new VSQDeviceInfoModel(deviceInfo);
    m_deviceInfoList.append(deviceInfoModel);
    emit deviceInfoListChanged();
}
