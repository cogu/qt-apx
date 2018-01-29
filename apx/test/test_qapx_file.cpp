#include "test_qapx_file.h"
#include "qapx_file.h"


void TestApxFile::inputFile()
{
   Apx::InputFile file("test.in",42);
   quint8 buf[50];
   int retval;
   QVERIFY(file.mLength == 42);
   retval = file.read(buf, 0, 0);
   QVERIFY(retval == 0);
   retval = file.read(buf, 41, 1);
   QVERIFY(retval == 1);
   retval = file.read(buf, 0, 42);
   QVERIFY(retval == 42);
   retval = file.read(buf, 1, 42);
   QVERIFY(retval == -1);
   retval = file.read(buf, 0, 43);
   QVERIFY(retval == -1);
}

void TestApxFile::outputFile()
{
   Apx::OutputFile file("test.out",100);
   QVERIFY(file.mLength == 100);
}
