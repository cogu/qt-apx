#include <QtTest/QtTest>
#include "test_qapx_filemap.h"
#include "test_qapx_bytearrayparser.h"
#include "test_qapx_file.h"
#include "test_qapx_nodedata.h"
#include "test_compiler.h"
#include "test_deserializer.h"
#include "test_qapx_datavm.h"
#include "test_qapx_node.h"
#include "test_qapx_client.h"
#ifdef _MSC_VER
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

//The test below requires that an instance of APX server is running on localhost
#define ENABLE_CLIENT_TEST 0


int main(int argc, char *argv[])
{
#ifdef _MSC_VER
   _CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif
   int status = 0;

   {
      TestApxFileMap t1;
      status |= QTest::qExec(&t1, argc, argv);
      TestApxByteParser tc2;
      status |= QTest::qExec(&tc2, argc, argv);
      TestApxFile tc3;
      status |= QTest::qExec(&tc3, argc, argv);
      TestApxNode tc4;
      status |= QTest::qExec(&tc4, argc, argv);
      TestApxNodeData tc5;
      status |= QTest::qExec(&tc5, argc, argv);
      TestDeserializer tc6;
      status |= QTest::qExec(&tc6, argc, argv);
      TestCompiler tc7;
      status |= QTest::qExec(&tc7, argc, argv);
      TestDataVM tc8;
      status |= QTest::qExec(&tc8, argc, argv);
#if ENABLE_CLIENT_TEST
      TestApxClient tc_client;
      status |= QTest::qExec(&tc_client, argc, argv);
#endif
   }
   return status;
}

