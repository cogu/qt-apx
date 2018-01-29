QT += core network testlib
QT -= gui

CONFIG += c++11

TARGET = apx_unit
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app


SOURCES += \
    $$PWD/src/qapx_client.cpp \
    $$PWD/src/qapx_compiler.cpp \
    $$PWD/src/qapxdataelement.cpp \
    $$PWD/src/qapxdatatype.cpp \
    $$PWD/src/qapxfileparser.cpp \
    $$PWD/src/qapx_node.cpp \
    $$PWD/src/qapxsimpleport.cpp \
    $$PWD/src/qapx_stream.cpp \
    $$PWD/src/qapxutil.cpp \
    $$PWD/src/qapx_datavm.cpp \
    $$PWD/src/qapx_bytearrayparser.cpp \
    $$PWD/test_main.cpp \
    $$PWD/test/test_qapx_filemap.cpp \
    src/qapx_filemap.cpp \
    src/qapx_file.cpp \
    test/test_qapx_file.cpp \
    src/qapx_nodedata.cpp \
    test/test_qapx_nodedata.cpp \
    test/test_qapx_bytearrayparser.cpp \
    test/test_deserializer.cpp \
    test/test_compiler.cpp \
    test/test_qapx_datavm.cpp \
    test/test_qapx_node.cpp \
    test/test_qapx_client.cpp

HEADERS += \
    $$PWD/inc/qapx_client.h \
    $$PWD/inc/qapx_vmbase.h \
    $$PWD/inc/qapx_compiler.h \
    $$PWD/inc/qapxdataelement.h \
    $$PWD/inc/qapxdatatype.h \
    $$PWD/inc/qapxfileparser.h \
    $$PWD/inc/qapx_node.h \
    $$PWD/inc/qapxsimpleport.h \
    $$PWD/inc/qapx_stream.h \
    $$PWD/inc/qapxutil.h \
    $$PWD/inc/qapx_datavm.h \
    $$PWD/inc/qapx_bytearrayparser.h \
    inc/qapx_filemap.h \
    inc/qapx_file.h \
    test/test_qapx_filemap.h \
    test/test_qapx_file.h \
    inc/qapx_nodedata.h \
    inc/qapx_exception.h \
    test/test_qapx_nodedata.h \
    test/test_qapx_bytearrayparser.h \
    test/test_deserializer.h \
    test/test_compiler.h \
    test/test_qapx_datavm.h \
    test/test_qapx_node.h \
    test/test_qapx_client.h

INCLUDEPATH += \
    $$PWD/inc \
    $$PWD/test

DEFINES += \
    APX_UNIT_TEST\

include(../remotefile/remotefile.pri)
include(../util/util.pri)
