QT += core network qml quick

CONFIG += c++14
CONFIG += debug

TARGET = iotkit-qt-example
#
#   Defines
#

DEFINES += QT_DEPRECATED_WARNINGS \
        INFO_CLIENT=1

#
#   Headers
#

HEADERS += \
        example/include/VSQApp.h \
        example/include/VSQController.h \
        example/include/VSQDeviceInfoController.h \
        example/include/VSQDeviceInfoModel.h \
        \
        ext/virgil-iotkit/default-impl/qt-udp-broadcast/include/virgil/iot/qt-udp-broadcast/VSQUdpBroadcast.h \
        ext/virgil-iotkit/helpers/iotkit-qt/include/virgil/iot/qt-helpers/VSQAppConfig.h \
        ext/virgil-iotkit/helpers/iotkit-qt/include/virgil/iot/qt-helpers/VSQDeviceRoles.h \
        ext/virgil-iotkit/helpers/iotkit-qt/include/virgil/iot/qt-helpers/VSQDeviceSerial.h \
        ext/virgil-iotkit/helpers/iotkit-qt/include/virgil/iot/qt-helpers/VSQDeviceType.h \
        ext/virgil-iotkit/helpers/iotkit-qt/include/virgil/iot/qt-helpers/VSQFeatures.h \
        ext/virgil-iotkit/helpers/iotkit-qt/include/virgil/iot/qt-helpers/VSQFileVersion.h \
        ext/virgil-iotkit/helpers/iotkit-qt/include/virgil/iot/qt-helpers/VSQImplementations.h \
        ext/virgil-iotkit/helpers/iotkit-qt/include/virgil/iot/qt-helpers/VSQIoTKit.h \
        ext/virgil-iotkit/helpers/iotkit-qt/include/virgil/iot/qt-helpers/VSQIoTKitFacade.h \
        ext/virgil-iotkit/helpers/iotkit-qt/include/virgil/iot/qt-helpers/VSQMac.h \
        ext/virgil-iotkit/helpers/iotkit-qt/include/virgil/iot/qt-helpers/VSQManufactureId.h \
        ext/virgil-iotkit/helpers/iotkit-qt/include/virgil/iot/qt-helpers/VSQSingleton.h \
        ext/virgil-iotkit/modules/protocols/qt-snap/include/virgil/iot/protocols/qt-snap/VSQNetifBase.h \
        ext/virgil-iotkit/modules/protocols/qt-snap/include/virgil/iot/protocols/qt-snap/VSQSnapServiceBase.h \
        ext/virgil-iotkit/modules/protocols/qt-snap/services/info-client/include/virgil/iot/protocols/qt-snap/info-client/VSQSnapINFOClient.h

#
#   Sources
#

SOURCES += \
        example/src/hal.cpp \
        example/src/main.cpp \
        example/src/VSQApp.cpp \
        example/src/VSQController.cpp \
        example/src/VSQDeviceInfoController.cpp \
        example/src/VSQDeviceInfoModel.cpp \
        \
        ext/virgil-iotkit/default-impl/qt-udp-broadcast/src/VSQUdpBroadcast.cpp \
        ext/virgil-iotkit/helpers/iotkit-qt/src/VSQDeviceRoles.cpp \
        ext/virgil-iotkit/helpers/iotkit-qt/src/VSQDeviceSerial.cpp \
        ext/virgil-iotkit/helpers/iotkit-qt/src/VSQDeviceType.cpp \
        ext/virgil-iotkit/helpers/iotkit-qt/src/VSQFileVersion.cpp \
        ext/virgil-iotkit/helpers/iotkit-qt/src/VSQIoTKitFacade.cpp \
        ext/virgil-iotkit/helpers/iotkit-qt/src/VSQMac.cpp \
        ext/virgil-iotkit/helpers/iotkit-qt/src/VSQManufactureId.cpp \
        ext/virgil-iotkit/modules/protocols/qt-snap/src/VSQNetifBase.cpp \
        ext/virgil-iotkit/modules/protocols/qt-snap/services/info-client/src/VSQSnapINFOClient.cpp

#
#   Resources
#

RESOURCES += example/src/qml/resources.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH = example/src/qml

# Additional import path used to resolve QML modules just for Qt Quick Designer
QML_DESIGNER_IMPORT_PATH = example/src/qml

CONFIG(debug) {
    BUILD_DIR = $$PWD/debug
} else {
    BUILD_DIR = $$PWD/release
}

#
#   Default rules for deployment
#
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

#
#   IoTKit Base path
#

IOTKIT_BASE_PATH = ext/virgil-iotkit
message("IOTKIT_BASE_PATH = $${IOTKIT_BASE_PATH}")
#
#   IoTKit build directory
#

unix:mac: IOTKIT_BUILD_DIR_PATH = $${BUILD_DIR}/CLion.Desktop/$${IOTKIT_BASE_PATH}
#else:win32: IOTKIT_BUILD_DIR_PATH = $${BUILD_DIR}/iotkit/win32
#else:unix:iphonesimulator: IOTKIT_BUILD_DIR_PATH = $${BUILD_DIR}/iotkit/ios-simulator
#else:unix:iphone: IOTKIT_BUILD_DIR_PATH = $${BUILD_DIR}/iotkit/ios
else:unix:android: IOTKIT_BUILD_DIR_PATH = $${BUILD_DIR}/Shell.Android/$${IOTKIT_BASE_PATH}
#else:unix:linux:!android: IOTKIT_BUILD_DIR_PATH = $${BUILD_DIR}/iotkit/linux
message("IOTKIT_BUILD_DIR_PATH = $${IOTKIT_BUILD_DIR_PATH}")

#
#   External libraries
#

LIBS += -L$${IOTKIT_BUILD_DIR_PATH}/modules/logger/ -lvs-module-logger \
        -L$${IOTKIT_BUILD_DIR_PATH}/modules/provision/ -lvs-module-provision \
        -L$${IOTKIT_BUILD_DIR_PATH}/modules/protocols/snap -lvs-module-snap-control

#
#   Include path
#

INCLUDEPATH +=  $${PWD}/example/include \
                $${IOTKIT_BASE_PATH}/default-impl/qt-udp-broadcast/include \
                $${IOTKIT_BASE_PATH}/helpers/iotkit-qt/include \
                $${IOTKIT_BASE_PATH}/modules/protocols/qt-snap/include \
                $${IOTKIT_BASE_PATH}/modules/protocols/qt-snap/services/info-client/include \
                \
                $${IOTKIT_BASE_PATH}/modules/logger/include \
                $${IOTKIT_BASE_PATH}/modules/provision/include \
                $${IOTKIT_BASE_PATH}/modules/provision/trust_list/include \
                $${IOTKIT_BASE_PATH}/modules/protocols/snap/include \
                $${IOTKIT_BASE_PATH}/modules/crypto/secmodule/include \
                $${IOTKIT_BASE_PATH}/helpers/status_code/include \
                $${IOTKIT_BASE_PATH}/helpers/macros/include \
                $${IOTKIT_BASE_PATH}/helpers/storage_hal/include \
                $${IOTKIT_BASE_PATH}/helpers/update/include \
                $${IOTKIT_BASE_PATH}/config/pc

DEPENDPATH += $${INCLUDEPATH}

#
#   Dependencies
#

#unix: LIB_EXT = a
#PRE_TARGETDEPS += $${IOTKIT_BUILD_DIR_PATH}/modules/logger/libvs-module-logger.$${LIB_EXT} \
#                  $${IOTKIT_BUILD_DIR_PATH}/modules/provision/libvs-module-provision.$${LIB_EXT} \
#                  $${IOTKIT_BUILD_DIR_PATH}/modules/protocols/snap/libvs-module-snap-control.$${LIB_EXT}

