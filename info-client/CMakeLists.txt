
cmake_minimum_required(VERSION 3.16 FATAL_ERROR)

project(info-client VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5Core CONFIG REQUIRED)
find_package(Qt5Network CONFIG REQUIRED)
find_package(Qt5Widgets CONFIG REQUIRED)

add_executable(info-client)

execute_process(
        COMMAND git rev-parse HEAD
        OUTPUT_VARIABLE COMPILE_GIT_HASH
)

string(STRIP ${COMPILE_GIT_HASH} COMPILE_GIT_HASH)

target_compile_definitions(info-client
        PRIVATE
        COMPILE_GIT_HASH="${COMPILE_GIT_HASH}"
        )

target_precompile_headers(info-client
    PRIVATE
        include/virgil/iot-qt/info-client/app.h
        )

target_sources(info-client
        PRIVATE
        include/virgil/iot-qt/info-client/app.h
        include/virgil/iot-qt/info-client/main_window.h

        src/main.cpp
        src/main_window.cpp
        )

target_include_directories(info-client
        PRIVATE
        include/virgil/iot-qt/info-client
        )

target_link_libraries(info-client
        PRIVATE

        virgil-iotkit-qt
        virgil-iot-status-code
        vs-module-provision
        vs-module-snap-factory

        Qt5::Core
        Qt5::Network
        Qt5::Widgets
        )

