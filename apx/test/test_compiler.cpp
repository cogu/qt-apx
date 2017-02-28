#include <QList>
#include "test_compiler.h"
#include "qapxdataelement.h"
#include "qapx_compiler.h"
#include "qapx_datavm.h"


void TestCompiler::test_packBaseTypes()
{
   QList<QApxDataElement*> elements;
   QApxDataElementParser parser;
   Apx::DataCompiler compiler;
   elements.append(parser.parseDataSignature((quint8*)"C"));
   QCOMPARE(elements.last()->baseType, QAPX_BASE_TYPE_UINT8);

   QByteArray program;

   int result = compiler.genPackData(program, elements.last());
   QVERIFY(result == 0);
   QCOMPARE(program.length(),7);
   QCOMPARE(program, QByteArray("\x01\x00\x01\x00\x00\x01\x0F",7));

   program.clear();
   elements.append(parser.parseDataSignature((quint8*)"S"));
   QCOMPARE(elements.last()->baseType, QAPX_BASE_TYPE_UINT16);
   result = compiler.genPackData(program, elements.last());
   QVERIFY(result == 0);
   QCOMPARE(program, QByteArray("\x01\x00\x01\x00\x00\x02\x10",7));


   program.clear();
   elements.append(parser.parseDataSignature((quint8*)"L"));
   QCOMPARE(elements.last()->baseType, QAPX_BASE_TYPE_UINT32);
   result = compiler.genPackData(program, elements.last());
   QVERIFY(result == 0);
   QCOMPARE(program, QByteArray("\x01\x00\x01\x00\x00\x04\x11",7));


   program.clear();
   elements.append(parser.parseDataSignature((quint8*)"c"));
   QCOMPARE(elements.last()->baseType, QAPX_BASE_TYPE_SINT8);
   result = compiler.genPackData(program, elements.last());
   QVERIFY(result == 0);
   QCOMPARE(program, QByteArray("\x01\x00\x01\x00\x00\x01\x12",7));

   program.clear();
   elements.append(parser.parseDataSignature((quint8*)"s"));
   QCOMPARE(elements.last()->baseType, QAPX_BASE_TYPE_SINT16);
   result = compiler.genPackData(program, elements.last());
   QVERIFY(result == 0);
   QCOMPARE(program, QByteArray("\x01\x00\x01\x00\x00\x02\x13",7));

   program.clear();
   elements.append(parser.parseDataSignature((quint8*)"l"));
   QCOMPARE(elements.last()->baseType, QAPX_BASE_TYPE_SINT32);
   result = compiler.genPackData(program, elements.last());
   QVERIFY(result == 0);
   QCOMPARE(program, QByteArray("\x01\x00\x01\x00\x00\x04\x14",7));

   program.clear();
   elements.append(parser.parseDataSignature((quint8*)"a[10]"));
   QCOMPARE(elements.last()->baseType, QAPX_BASE_TYPE_STRING);
   result = compiler.genPackData(program, elements.last());
   QVERIFY(result == 0);
   QCOMPARE(program, QByteArray("\x01\x00\x01\x00\x00\x0a\x15\x00\x0a",9));

   for(int i=0;i<elements.length();i++)
   {
      delete elements[i];
   }
}

void TestCompiler::test_packArrayTypes()
{
   QList<QApxDataElement*> elements;
   QApxDataElementParser parser;
   Apx::DataCompiler compiler;
   elements.append(parser.parseDataSignature((quint8*)"C[6]"));
   QCOMPARE(elements.last()->baseType, QAPX_BASE_TYPE_UINT8);

   QByteArray program;

   int result = compiler.genPackData(program, elements.last());
   QVERIFY(result == 0);
   QCOMPARE(program, QByteArray("\x01\x00\x03\x00\x00\x06\x16\x00\x06",9));

   program.clear();
   elements.append(parser.parseDataSignature((quint8*)"S[3]"));
   QCOMPARE(elements.last()->baseType, QAPX_BASE_TYPE_UINT16);
   result = compiler.genPackData(program, elements.last());
   QVERIFY(result == 0);
   QCOMPARE(program, QByteArray("\x01\x00\x03\x00\x00\x06\x17\x00\x03",9));


   program.clear();
   elements.append(parser.parseDataSignature((quint8*)"L[4]"));
   QCOMPARE(elements.last()->baseType, QAPX_BASE_TYPE_UINT32);
   result = compiler.genPackData(program, elements.last());
   QVERIFY(result == 0);
   QCOMPARE(program, QByteArray("\x01\x00\x03\x00\x00\x10\x18\x00\x04",9));


   program.clear();
   elements.append(parser.parseDataSignature((quint8*)"c[2]"));
   QCOMPARE(elements.last()->baseType, QAPX_BASE_TYPE_SINT8);
   result = compiler.genPackData(program, elements.last());
   QVERIFY(result == 0);
   QCOMPARE(program, QByteArray("\x01\x00\x03\x00\x00\x02\x19\x00\02",9));

   program.clear();
   elements.append(parser.parseDataSignature((quint8*)"s[130]"));
   QCOMPARE(elements.last()->baseType, QAPX_BASE_TYPE_SINT16);
   result = compiler.genPackData(program, elements.last());
   QVERIFY(result == 0);
   QCOMPARE(program, QByteArray("\x01\x00\x03\x00\x01\x04\x1a\x00\x82",9));

   program.clear();
   elements.append(parser.parseDataSignature((quint8*)"l[14]"));
   QCOMPARE(elements.last()->baseType, QAPX_BASE_TYPE_SINT32);
   result = compiler.genPackData(program, elements.last());
   QVERIFY(result == 0);
   QCOMPARE(program, QByteArray("\x01\x00\x03\x00\x00\x38\x1b\x00\x0e",9));


   for(int i=0;i<elements.length();i++)
   {
      delete elements[i];
   }
}

void TestCompiler::test_packRecordTypes()
{
   QList<QApxDataElement*> elements;
   QApxDataElementParser parser;
   Apx::DataCompiler compiler;
   elements.append(parser.parseDataSignature((quint8*)"{\"elem1\"C\"elem2\"S[4]\"elem3\"a[21]}"));
   QCOMPARE(elements.last()->baseType, QAPX_BASE_TYPE_RECORD);

   QByteArray program;

   int result = compiler.genPackData(program, elements.last());
   QVERIFY(result == 0);
   QCOMPARE(program, QByteArray("\x01\x00\x02\x00\x00\x1E\x1D""elem1\0\x0F\x1D""elem2\0\x17\x00\04\x1D""elem3\0\x15\x00\x15",34));

   program.clear();
   elements.append(parser.parseDataSignature((quint8*)"{\"elem1\"C\"elem2\"S}"));
   result = compiler.genPackData(program, elements.last());
   QVERIFY(result == 0);
   QCOMPARE(program, QByteArray("\x01\x00\x02\x00\x00\x03\x1D""elem1\0\x0F\x1D""elem2\0\x10",22));

   for(int i=0;i<elements.length();i++)
   {
      delete elements[i];
   }
}

/** test that QVariant -> serialzier -> deserializer -> (back to) QVariant produces same value **/

void TestCompiler::test_combindedRecordType()
{
   QList<QApxDataElement*> elements;
   QApxDataElementParser parser;
   Apx::DataCompiler compiler;
   elements.append(parser.parseDataSignature((quint8*)"{\"elem1\"C\"elem2\"S}"));
   QCOMPARE(elements.last()->baseType, QAPX_BASE_TYPE_RECORD);

   QByteArray program;

   int result = compiler.genPackData(program, elements.last());
   QVERIFY(result == 0);
   QVariantMap input;
   input.insert(QString("elem1"), QVariant((int) 29));
   input.insert(QString("elem2"), QVariant((int) 32702));

   Apx::DataVM vm;
   QByteArray serializedData;

   result = vm.exec(program, serializedData, input);
   QVERIFY(result == 0);

   for(int i=0;i<elements.length();i++)
   {
      delete elements[i];
   }
}

void TestCompiler::test_compileString()
{
   QList<QApxDataElement*> elements;
   QApxDataElementParser parser;
   Apx::DataCompiler compiler;
   elements.append(parser.parseDataSignature((quint8*)"a[16]"));
   QCOMPARE(elements.last()->baseType, QAPX_BASE_TYPE_STRING);

   QByteArray pack_prog;
   QByteArray unpack_prog;

   int result = compiler.genPackData(pack_prog, elements.last());
   QVERIFY(result == 0);
   QCOMPARE(pack_prog, QByteArray("\x01\x00\x01\x00\x00\x10\x15\x00\x10",9));

   result = compiler.genUnpackData(unpack_prog, elements.last());
   QVERIFY(result == 0);
   QCOMPARE(unpack_prog, QByteArray("\x01\x01\x01\x00\x00\x10\x08\x00\x10",9));

   for(int i=0;i<elements.length();i++)
   {
      delete elements[i];
   }

}
