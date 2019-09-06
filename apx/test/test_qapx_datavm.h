#ifndef TEST_QAPX_TESTDATAVM_H
#define TEST_QAPX_TESTDATAVM_H
#include <QtTest/QtTest>

class TestDataVM : public QObject
{
   Q_OBJECT
private slots:
   void test_parseArrayLen();
   void test_packUnpackRecord();
   void test_packUnpackS8();
   void test_packUnpackS16();
   void test_packUnpackS32();
   void test_packUnpackU8();
   void test_packUnpackU16();
   void test_packUnpackU32();
   void test_packUnpackU8Array();
   void test_unpackShortU8Array();
   void test_unpackShortU16Array();
   void test_unpackPatchU8Array();
   void test_unpackProgErrorsU8Array();
   void test_packUnpackU16Array();
   void test_packUnpackU32Array();
   void test_packUnpackString();


};

#endif // TEST_QAPX_TESTDATAVM_H
