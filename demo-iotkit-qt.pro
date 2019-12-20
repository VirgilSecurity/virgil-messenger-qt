QT += core network qml quick

CONFIG += c++14

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
        example/include/VSQDeviceInfoModel.h

#
#   Sources
#

SOURCES += \
        example/src/hal.cpp \
        example/src/main.cpp \
        example/src/VSQApp.cpp \
        example/src/VSQController.cpp \
        example/src/VSQDeviceInfoController.cpp \
        example/src/VSQDeviceInfoModel.cpp

#
#   Resources
#

RESOURCES += example/src/qml/resources.qrc

#
#   Include path
#

INCLUDEPATH +=  example/include

#
#   Default rules for deployment
#
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

########################
#
#   IoTKit
#
########################

#
#   Project relative path
#

VIRGIL_IOTKIT_SOURCE_PATH = $${PWD}/ext/virgil-iotkit
VIRGIL_IOTKIT_BUILD_PATH_BASE = $${VIRGIL_IOTKIT_SOURCE_PATH}

CONFIG(debug, debug|release) {
    VIRGIL_IOTKIT_BUILD_PATH_BASE = $${VIRGIL_IOTKIT_SOURCE_PATH}/debug
}
CONFIG(release, debug|release) {
    VIRGIL_IOTKIT_BUILD_PATH_BASE = $${VIRGIL_IOTKIT_SOURCE_PATH}/release
}

unix:mac:      VIRGIL_IOTKIT_BUILD_PATH = $${VIRGIL_IOTKIT_BUILD_PATH_BASE}.mac
linux:android: VIRGIL_IOTKIT_BUILD_PATH = $${VIRGIL_IOTKIT_BUILD_PATH_BASE}.android

message("Virgil IoTKIT libraries : $${VIRGIL_IOTKIT_BUILD_PATH}")

#
#   Headers
#

HEADERS += \
        $${VIRGIL_IOTKIT_SOURCE_PATH}/default-impl/qt-udp-broadcast/include/virgil/iot/qt-udp-broadcast/VSQUdpBroadcast.h \
        $${VIRGIL_IOTKIT_SOURCE_PATH}/helpers/iotkit-qt/include/virgil/iot/qt-helpers/VSQAppConfig.h \
        $${VIRGIL_IOTKIT_SOURCE_PATH}/helpers/iotkit-qt/include/virgil/iot/qt-helpers/VSQDeviceRoles.h \
        $${VIRGIL_IOTKIT_SOURCE_PATH}/helpers/iotkit-qt/include/virgil/iot/qt-helpers/VSQDeviceSerial.h \
        $${VIRGIL_IOTKIT_SOURCE_PATH}/helpers/iotkit-qt/include/virgil/iot/qt-helpers/VSQDeviceType.h \
        $${VIRGIL_IOTKIT_SOURCE_PATH}/helpers/iotkit-qt/include/virgil/iot/qt-helpers/VSQFeatures.h \
        $${VIRGIL_IOTKIT_SOURCE_PATH}/helpers/iotkit-qt/include/virgil/iot/qt-helpers/VSQFileVersion.h \
        $${VIRGIL_IOTKIT_SOURCE_PATH}/helpers/iotkit-qt/include/virgil/iot/qt-helpers/VSQImplementations.h \
        $${VIRGIL_IOTKIT_SOURCE_PATH}/helpers/iotkit-qt/include/virgil/iot/qt-helpers/VSQIoTKit.h \
        $${VIRGIL_IOTKIT_SOURCE_PATH}/helpers/iotkit-qt/include/virgil/iot/qt-helpers/VSQIoTKitFacade.h \
        $${VIRGIL_IOTKIT_SOURCE_PATH}/helpers/iotkit-qt/include/virgil/iot/qt-helpers/VSQMac.h \
        $${VIRGIL_IOTKIT_SOURCE_PATH}/helpers/iotkit-qt/include/virgil/iot/qt-helpers/VSQManufactureId.h \
        $${VIRGIL_IOTKIT_SOURCE_PATH}/helpers/iotkit-qt/include/virgil/iot/qt-helpers/VSQSingleton.h \
        $${VIRGIL_IOTKIT_SOURCE_PATH}/modules/protocols/qt-snap/include/virgil/iot/protocols/qt-snap/VSQNetifBase.h \
        $${VIRGIL_IOTKIT_SOURCE_PATH}/modules/protocols/qt-snap/include/virgil/iot/protocols/qt-snap/VSQSnapServiceBase.h \
        $${VIRGIL_IOTKIT_SOURCE_PATH}/modules/protocols/qt-snap/services/info-client/include/virgil/iot/protocols/qt-snap/info-client/VSQSnapINFOClient.h

#
#   Sources
#

SOURCES += \
        $${VIRGIL_IOTKIT_SOURCE_PATH}/default-impl/qt-udp-broadcast/src/VSQUdpBroadcast.cpp \
        $${VIRGIL_IOTKIT_SOURCE_PATH}/helpers/iotkit-qt/src/VSQDeviceRoles.cpp \
        $${VIRGIL_IOTKIT_SOURCE_PATH}/helpers/iotkit-qt/src/VSQDeviceSerial.cpp \
        $${VIRGIL_IOTKIT_SOURCE_PATH}/helpers/iotkit-qt/src/VSQDeviceType.cpp \
        $${VIRGIL_IOTKIT_SOURCE_PATH}/helpers/iotkit-qt/src/VSQFileVersion.cpp \
        $${VIRGIL_IOTKIT_SOURCE_PATH}/helpers/iotkit-qt/src/VSQIoTKitFacade.cpp \
        $${VIRGIL_IOTKIT_SOURCE_PATH}/helpers/iotkit-qt/src/VSQMac.cpp \
        $${VIRGIL_IOTKIT_SOURCE_PATH}/helpers/iotkit-qt/src/VSQManufactureId.cpp \
        $${VIRGIL_IOTKIT_SOURCE_PATH}/modules/protocols/qt-snap/src/VSQNetifBase.cpp \
        $${VIRGIL_IOTKIT_SOURCE_PATH}/modules/protocols/qt-snap/services/info-client/src/VSQSnapINFOClient.cpp

#
#   Libraries
#

defineReplace(add_virgiliotkit_library) {
    LIBRARY_PATH = $$1
    LIBRARY_NAME = $$2
    !exists($${VIRGIL_IOTKIT_BUILD_PATH}/$${LIBRARY_PATH}/*$${LIBRARY_NAME}.*): error("Library $${LIBRARY_NAME} has not been found in $${VIRGIL_IOTKIT_BUILD_PATH}/$${LIBRARY_PATH}. Rebuild VirgilIoTKIT library")
    return ("-L$${VIRGIL_IOTKIT_BUILD_PATH}/$${LIBRARY_PATH} -l$${LIBRARY_NAME}")
}

LIBS += $$add_virgiliotkit_library("modules/logger",         "vs-module-logger")
LIBS += $$add_virgiliotkit_library("modules/provision",      "vs-module-provision")
LIBS += $$add_virgiliotkit_library("modules/protocols/snap", "vs-module-snap-control")

#
#   Include path
#

INCLUDEPATH +=  $${VIRGIL_IOTKIT_SOURCE_PATH}/default-impl/qt-udp-broadcast/include \
                $${VIRGIL_IOTKIT_SOURCE_PATH}/helpers/iotkit-qt/include \
                $${VIRGIL_IOTKIT_SOURCE_PATH}/modules/protocols/qt-snap/include \
                $${VIRGIL_IOTKIT_SOURCE_PATH}/modules/protocols/qt-snap/services/info-client/include \
                \
                $${VIRGIL_IOTKIT_SOURCE_PATH}/modules/logger/include \
                $${VIRGIL_IOTKIT_SOURCE_PATH}/modules/provision/include \
                $${VIRGIL_IOTKIT_SOURCE_PATH}/modules/provision/trust_list/include \
                $${VIRGIL_IOTKIT_SOURCE_PATH}/modules/protocols/snap/include \
                $${VIRGIL_IOTKIT_SOURCE_PATH}/modules/crypto/secmodule/include \
                $${VIRGIL_IOTKIT_SOURCE_PATH}/helpers/status_code/include \
                $${VIRGIL_IOTKIT_SOURCE_PATH}/helpers/macros/include \
                $${VIRGIL_IOTKIT_SOURCE_PATH}/helpers/storage_hal/include \
                $${VIRGIL_IOTKIT_SOURCE_PATH}/helpers/update/include \
                $${VIRGIL_IOTKIT_SOURCE_PATH}/config/pc

#
#   Depencies
#

DEPENDPATH += $${INCLUDEPATH}

#
#   CMake arguments
#

VIRGIL_IOTKIT_CMAKE_ARGS += "-DVIRGIL_IOT_CONFIG_DIRECTORY=$${VIRGIL_IOTKIT_ABSOLUTE_PATH}/config/pc"

#
#   Dependencies
#

unix: LIB_EXT = a
PRE_TARGETDEPS += $${VIRGIL_IOTKIT_BUILD_PATH}/modules/logger/libvs-module-logger.$${LIB_EXT} \
                  $${VIRGIL_IOTKIT_BUILD_PATH}/modules/provision/libvs-module-provision.$${LIB_EXT} \
                  $${VIRGIL_IOTKIT_BUILD_PATH}/modules/protocols/snap/libvs-module-snap-control.$${LIB_EXT}

