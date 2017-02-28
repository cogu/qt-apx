#include "test_qapx_file.h"
#include "qapx_file.h"


void TestApxFile::inputFile()
{
   Apx::InputFile file("test.in",42);
   QVERIFY(file.mLength == 42);
}

void TestApxFile::outputFile()
{
   Apx::OutputFile file("test.out",100);
   QVERIFY(file.mLength == 100);
}
