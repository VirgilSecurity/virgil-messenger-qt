QT += core network qml quick

CONFIG += c++14

TARGET = iotkit-qt-example

#
#   Include IoTKit Qt wrapper
#

include(ext/virgil-iotkit/integration/qt/iotkit.pri)

#
#   Defines
#

DEFINES += QT_DEPRECATED_WARNINGS \
        INFO_CLIENT=1

#
#   Headers
#

HEADERS += \
        include/VSQApp.h \
        include/VSQController.h \
        include/VSQDeviceInfoController.h \
        include/VSQDeviceInfoModel.h

#
#   Sources
#

SOURCES += \
        src/hal.cpp \
        src/main.cpp \
        src/VSQApp.cpp \
        src/VSQController.cpp \
        src/VSQDeviceInfoController.cpp \
        src/VSQDeviceInfoModel.cpp

#
#   Resources
#

RESOURCES += src/qml/resources.qrc

#
#   Include path
#

INCLUDEPATH +=  include

#
#   Default rules for deployment
#
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

#
#   Depencies
#

DEPENDPATH += $${INCLUDEPATH}

