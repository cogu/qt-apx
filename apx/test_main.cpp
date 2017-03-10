#include <QtTest/QtTest>
#include "test_qapx_filemap.h"
#include "test_qapx_bytearrayparser.h"
#include "test_qapx_file.h"
#include "test_qapx_nodedata.h"
#include "test_compiler.h"
#include "test_deserializer.h"
#include "test_qapx_datavm.h"

int main(int argc, char *argv[])
{
   int status = 0;

   {
      TestApxFileMap t1;
      status |= QTest::qExec(&t1, argc, argv);
      TestApxByteParser tc2;
      status |= QTest::qExec(&tc2, argc, argv);
      TestApxFile tc3;
      status |= QTest::qExec(&tc3, argc, argv);
      TestApxNodeData tc4;
      status |= QTest::qExec(&tc4, argc, argv);
      TestDeserializer tc5;
      status |= QTest::qExec(&tc5, argc, argv);
      TestCompiler tc6;
      status |= QTest::qExec(&tc6, argc, argv);
      TestDataVM tc7;
      status |= QTest::qExec(&tc7, argc, argv);
   }
   return status;
}

