#ifndef TEST_SERIALIZER_H
#define TEST_SERIALIZER_H

#include <QtTest/QtTest>

class TestCompiler : public QObject
{
   Q_OBJECT
private slots:
   void test_packBaseTypes();
   void test_packArrayTypes();
   void test_packRecordTypes();
   void test_combindedRecordType();
   void test_compileString();
};

#endif // TEST_SERIALIZER_H
