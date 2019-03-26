QT       += testlib

QT       += gui

TARGET = QASTCTests
CONFIG   += console

TEMPLATE = app

CONFIG += warn_off
unix {
    QMAKE_CXXFLAGS_WARN_OFF -= -w
    QMAKE_CXXFLAGS += -Wall
}

DEFINES += SRCDIR=\\\"$$PWD/\\\"

SOURCES += \
    main.cpp \
    Tests.cpp

HEADERS += \
    Tests.h
