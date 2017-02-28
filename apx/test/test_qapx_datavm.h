#ifndef TEST_QAPX_TESTDATAVM_H
#define TEST_QAPX_TESTDATAVM_H
#include <QtTest/QtTest>

class TestDataVM : public QObject
{
   Q_OBJECT
private slots:
   void test_packUnpackRecord();
   void test_packUnpackU8();
   void test_packUnpackU16();
   void test_packUnpackU32();
   void test_packUnpackU8Array();
   void test_packUnpackU16Array();
   void test_packUnpackU32Array();
   void test_packUnpackString();


};

#endif // TEST_QAPX_TESTDATAVM_H
