QT += core network testlib
QT -= gui

CONFIG += c++11

TARGET = remotefile_unit
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

DEFINES += \
    UNIT_TEST\


SOURCES += \
    $$PWD/src/qrmf_file.cpp \
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
    $$PWD/test/mocktransmithandler.cpp \
    $$PWD/src/qrmf_socketadapter.cpp \
    test/test_qrmf_socketadapter.cpp \
    test/mock_receive_handler.cpp \
    test/mock_socket.cpp

HEADERS += \
    $$PWD/inc/qrmf_file.h \
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
    $$PWD/test/mocktransmithandler.h \
    $$PWD/inc/qrmf_socketadapter.h \
    test/test_qrmf_socketadapter.h \
    test/mock_receive_handler.h \
    test/mock_socket.h

INCLUDEPATH += \
    $$PWD/inc  \
    $$PWD/test

include(../util/util.pri)


