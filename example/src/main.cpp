#include "controller.h"
#include <QGuiApplication>

int
main(int argc, char *argv[]) {
    QGuiApplication a(argc, argv);

    Controller controller;
    controller.setupUI();

    return a.exec();
}
