QT += core network
QT -= gui

CONFIG += c++11

TARGET = ApxStressTest
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    stressnode.cpp


include(../../util/util.pri)
include(../../remotefile/remotefile.pri)
include(../../apx/apx.pri)

HEADERS += \
    stressnode.h
