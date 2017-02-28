QT += core network testlib
QT -= gui

CONFIG += c++11

TARGET = remotefile_unit
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += \
    $$PWD/src/qrmf_file.cpp \
    $$PWD/src/qrmf_filemap.cpp \
    $$PWD/src/qrmf_filemap2.cpp \
    $$PWD/test_main.cpp \
    $$PWD/test/test_numheader.cpp \
    $$PWD/src/qremotefile.cpp \
    $$PWD/src/qnumheader.cpp \
    $$PWD/src/qrmf_filemanager.cpp \
    $$PWD/src/qrmf_base.cpp \
    $$PWD/test/test_qrmf_filemanager.cpp \
    $$PWD/test/test_qrmf_proto.cpp \
    $$PWD/src/qrmf_proto.cpp \
    $$PWD/test/mocktransmithandler.cpp

HEADERS += \
    $$PWD/inc/qrmf_file.h \
    $$PWD/inc/qrmf_filemap.h \
    $$PWD/inc/qrmf_filemap2.h \
    $$PWD/test/test_numheader.h \
    $$PWD/inc/qremotefile.h \
    $$PWD/inc/qnumheader.h \
    $$PWD/inc/qrmf_filemanager.h \
    $$PWD/inc/qrmf_msg.h \
    $$PWD/inc/qrmf_base.h \
    $$PWD/test/test_qrmf_filemanager.h \
    $$PWD/inc/qrmf_proto.h \
    $$PWD/test/test_qrmf_proto.h \
    $$PWD/test/mocktransmithandler.h

INCLUDEPATH += \
    $$PWD/inc  \
    $$PWD/test


