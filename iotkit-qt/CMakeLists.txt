
cmake_minimum_required(VERSION 3.16 FATAL_ERROR)

project(virgil-iotkit-qt VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5Core CONFIG REQUIRED)
find_package(Qt5Network CONFIG REQUIRED)

add_library(virgil-iotkit-qt)

target_sources(virgil-iotkit-qt
        PRIVATE
        include/virgil/iot-qt/logger.h
        include/virgil/iot-qt/snap_info.h
        include/virgil/iot-qt/snap_protocol.h
        include/virgil/iot-qt/snap_service.h
        include/virgil/iot-qt/snap_udp.h

        src/hal.cpp
        src/snap_info.cpp
        src/snap_protocol.cpp
        src/snap_service.cpp
        src/snap_udp.cpp
        )

target_include_directories(virgil-iotkit-qt
        PUBLIC
        include
        )

target_link_libraries(virgil-iotkit-qt
        PRIVATE

        vs-module-snap-factory
        vs-module-logger
        virgil-iot-status-code

        Qt5::Core
        Qt5::Network
        )

