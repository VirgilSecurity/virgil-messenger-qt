//
// Created by Oleksandr Nemchenko on 26.11.2019.
//

#include <chrono>
#include <iostream>
#include <thread>
#include <QCoreApplication>

extern "C" bool
vs_logger_output_hal(const char *buffer){
    std::cout << buffer;

    return true;
}

extern "C" void
vs_impl_msleep(size_t msec){
    auto start = std::chrono::high_resolution_clock::now();
    do {
        QCoreApplication::processEvents( QEventLoop::AllEvents, msec );
    }
    while( std::chrono::high_resolution_clock::now() - start <= std::chrono::milliseconds( msec ));
}