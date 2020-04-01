#!/bin/bash

SCRIPT_FOLDER="$( cd "$( dirname "$0" )" && pwd )"

${SCRIPT_FOLDER}/build-qxmpp.sh ${HOME}/Work/Qt/5.12.6/mingw32 " \
        -DCMAKE_TOOLCHAIN_FILE=/home/roman/Work/demo-iotkit-qt/ext/virgil-iotkit/sdk/cmake/mingw32.toolchain.cmake  \
        -DAUTOMOC_EXECUTABLE=/home/roman/Work/Qt/5.12.6/mingw32/bin/moc \
        -DQT_MOC_EXECUTABLE=/home/roman/Work/Qt/5.12.6/mingw32/bin/moc
        -DCYGWIN=1\
    "
