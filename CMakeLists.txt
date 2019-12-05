
cmake_minimum_required(VERSION 3.16 FATAL_ERROR)

project(Virgil-IoT-INFO-client VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

set(VIRGIL_IOT_CONFIG_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/ext/virgil-iotkit/config/pc CACHE STRING "Path to the PC configuration")

include(${CMAKE_CURRENT_LIST_DIR}/ext/virgil-iotkit/cmake/helpers.cmake)

add_subdirectory(ext)
add_subdirectory(iotkit-qt)
add_subdirectory(info-client)
