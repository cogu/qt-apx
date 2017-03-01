QT += core network
QT -= gui

CONFIG += c++11

TARGET = ApxClientTest
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    simulator.cpp


include(../../util/util.pri)
include(../../remotefile/remotefile.pri)
include(../../apx/apx.pri)

HEADERS += \
    simulator.h
