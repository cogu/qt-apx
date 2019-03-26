#include "test_qapx_datavm.h"
#include "qapx_datavm.h"

class TestVM : public Apx::DataVM {
    public:
    const char* wrap_parseArrayLen(const char *pBegin, const char *pEnd, int &arrayLen){return parseArrayLen(pBegin, pEnd, arrayLen); }
};

void TestDataVM::test_parseArrayLen()
{
   TestVM vm;
   QByteArray len("\x00\x00", 2);
   int decLen = -1;
   const char* startAddr = len.constData();
   const char* endAddr = startAddr + len.length();
   const char* ret = vm.wrap_parseArrayLen(startAddr, endAddr, decLen);
   QCOMPARE(decLen, 0);
   QVERIFY(ret>len.constData());

   decLen = -1;
   ret = vm.wrap_parseArrayLen(startAddr, endAddr+1, decLen);
   QCOMPARE(decLen, 0);
   QVERIFY(ret>len.constData());

   decLen = -1;
   ret = vm.wrap_parseArrayLen(startAddr, endAddr-1, decLen);
   QCOMPARE(decLen, -1);
   QVERIFY(ret == len.constData());
}

void TestDataVM::test_packUnpackRecord()
{
   QByteArray pack_prog(  "\x01\x00\x02\x00\x00\x03\x1D""elem1\0\x0F\x1D""elem2\0\x10",22); //compiled program of pack({"elem1"C"elem2"S})
   QByteArray unpack_prog("\x01\x01\x02\x00\x00\x03\x1D""elem1\0\x02\x1D""elem2\0\x03",22); //compiled program of unpack({"elem1"C"elem2"S})
   QVariantMap input;
   input.insert(QString("elem1"), QVariant((int) 29));
   input.insert(QString("elem2"), QVariant((int) 32702));
   QVariantMap output;
   Apx::DataVM vm;
   QByteArray serializedData;

   int result = vm.exec(pack_prog, serializedData, input);
   QVERIFY(result == 0);
   QCOMPARE(serializedData, QByteArray("\x1D\xBE\x7F"));

   result = vm.exec(unpack_prog, serializedData, output);
   QVERIFY(result == 0);
   QCOMPARE(output, input);
}


void TestDataVM::test_packUnpackS8()
{
   QByteArray pack_prog(  "\x01\x00\x01\x00\x00\x01\x12",7); //pack S8 from QVaríant(scalar)
   QByteArray unpack_prog("\x01\x01\x01\x00\x00\x01\x05",7); //unpack S8 to QVaríant(scalar)

   QVariant input(127);
   QVariant output(0);
   QByteArray serializedData;

   Apx::DataVM vm;
   int result = vm.exec(pack_prog, serializedData, input);
   QVERIFY(result == 0);
   QCOMPARE(serializedData, QByteArray("\x7f",1));
   result = vm.exec(unpack_prog, serializedData, output);
   QVERIFY(result == 0);
   QCOMPARE(output,input);

   input = QVariant(-128);
   result = vm.exec(pack_prog, serializedData, input);
   QVERIFY(result == 0);
   QCOMPARE(serializedData, QByteArray("\x80",1));
   result = vm.exec(unpack_prog, serializedData, output);
   QVERIFY(result == 0);
   QCOMPARE(output,input);
}

void TestDataVM::test_packUnpackS16()
{
   QByteArray pack_prog(  "\x01\x00\x01\x00\x00\x02\x13",7); //pack S16 from QVaríant(scalar)
   QByteArray unpack_prog("\x01\x01\x01\x00\x00\x02\x06",7); //unpack S16 to QVaríant(scalar)

   QVariant input(0);
   QVariant output(0x7fff);
   QByteArray serializedData;
   serializedData.reserve(4);
   QVERIFY(serializedData.length()==0);
   memset(serializedData.data(),0xaau,4);
   uint8_t raw_init_expected[4] = {0xaau ,0xaau ,0xaau, 0xaau};
   QVERIFY(memcmp(serializedData.data(),raw_init_expected,4) == 0);
   uint8_t raw_init_resized_expected[4] = {0xaau ,0xaau ,'\0', 0xaau};
   const int preCapacity = serializedData.capacity();
   serializedData.resize(2);
   QVERIFY(memcmp(serializedData.data(),raw_init_resized_expected,4) == 0);

   Apx::DataVM vm;
   int result = vm.exec(pack_prog, serializedData, input);
   // Even if byte array is resized (reduced 4->2) the capacity of the underlying data should not change
   QVERIFY(preCapacity == serializedData.capacity());
   QVERIFY(result == 0);
   QVERIFY(serializedData.length()==2);
   QCOMPARE(serializedData, QByteArray("\x00\x00",2));
   uint8_t raw_expected[4] = {0,0,'\0', 0xaau};
   QVERIFY(memcmp(serializedData.data(),raw_expected,4) == 0);

   result = vm.exec(unpack_prog, serializedData, output);
   QVERIFY(result == 0);
   QCOMPARE(output,input);

   input = QVariant(0x1234);
   serializedData.clear();
   result = vm.exec(pack_prog, serializedData, input);
   QVERIFY(result == 0);
   QVERIFY(serializedData.length()==2);
   QCOMPARE(serializedData, QByteArray("\x34\x12",2));
   result = vm.exec(unpack_prog, serializedData, output);
   QVERIFY(result == 0);
   QCOMPARE(output,input);

   input = QVariant(-1);
   serializedData.clear();
   result = vm.exec(pack_prog, serializedData, input);
   QVERIFY(result == 0);
   QVERIFY(serializedData.length()==2);
   QCOMPARE(serializedData, QByteArray("\xff\xff",2));
   result = vm.exec(unpack_prog, serializedData, output);
   QVERIFY(result == 0);
   QCOMPARE(output,input);
}

void TestDataVM::test_packUnpackS32()
{
   QByteArray pack_prog(  "\x01\x00\x01\x00\x00\x04\x14",7); //pack S32 from QVaríant(scalar)
   QByteArray unpack_prog("\x01\x01\x01\x00\x00\x04\x07",7); //unpack S32 to QVaríant(scalar)

   QVariant input(0);
   QVariant output(0xffff);
   QByteArray serializedData;

   Apx::DataVM vm;
   int result = vm.exec(pack_prog, serializedData, input);
   QVERIFY(result == 0);
   QCOMPARE(serializedData, QByteArray("\x00\x00\x00\x00",4));
   result = vm.exec(unpack_prog, serializedData, output);
   QVERIFY(result == 0);
   QCOMPARE(output,input);


   input = QVariant(0x12345678);
   serializedData.clear();
   result = vm.exec(pack_prog, serializedData, input);
   QVERIFY(result == 0);
   QVERIFY(serializedData.length()==4);
   QCOMPARE(serializedData, QByteArray("\x78\x56\x34\x12",4));
   result = vm.exec(unpack_prog, serializedData, output);
   QVERIFY(result == 0);
   QCOMPARE(output,input);

   input = QVariant(0x7fffffff);
   serializedData.clear();
   result = vm.exec(pack_prog, serializedData, input);
   QVERIFY(result == 0);
   QVERIFY(serializedData.length()==4);
   QCOMPARE(serializedData, QByteArray("\xff\xff\xff\x7f",4));
   result = vm.exec(unpack_prog, serializedData, output);
   QVERIFY(result == 0);
   QCOMPARE(output,input);

   input = QVariant(-1);
   serializedData.clear();
   result = vm.exec(pack_prog, serializedData, input);
   QVERIFY(result == 0);
   QVERIFY(serializedData.length()==4);
   QCOMPARE(serializedData, QByteArray("\xff\xff\xff\xff",4));
   result = vm.exec(unpack_prog, serializedData, output);
   QVERIFY(result == 0);
   QCOMPARE(output,input);
}

void TestDataVM::test_packUnpackU8()
{
   QByteArray pack_prog(  "\x01\x00\x01\x00\x00\x01\x0F",7); //pack U8 from QVaríant(scalar)
   QByteArray unpack_prog("\x01\x01\x01\x00\x00\x01\x02",7); //unpack U8 to QVaríant(scalar)

   QVariant input(250);
   QVariant output(0);
   QByteArray serializedData;

   Apx::DataVM vm;
   int result = vm.exec(pack_prog, serializedData, input);
   QVERIFY(result == 0);
   QCOMPARE(serializedData, QByteArray("\xFA",1));
   result = vm.exec(unpack_prog, serializedData, output);
   QVERIFY(result == 0);
   QCOMPARE(output,input);
}

void TestDataVM::test_packUnpackU16()
{
   QByteArray pack_prog(  "\x01\x00\x01\x00\x00\x02\x10",7); //pack U16 from QVaríant(scalar)
   QByteArray unpack_prog("\x01\x01\x01\x00\x00\x02\x03",7); //unpack U16 to QVaríant(scalar)

   QVariant input(0);
   QVariant output(0xffff);
   QByteArray serializedData;
   serializedData.reserve(4);
   QVERIFY(serializedData.length()==0);
   memset(serializedData.data(),0xaau,4);
   uint8_t raw_init_expected[4] = {0xaau ,0xaau ,0xaau, 0xaau};
   QVERIFY(memcmp(serializedData.data(),raw_init_expected,4) == 0);
   uint8_t raw_init_resized_expected[4] = {0xaau ,0xaau ,'\0', 0xaau};
   const int preCapacity = serializedData.capacity();
   serializedData.resize(2);
   QVERIFY(memcmp(serializedData.data(),raw_init_resized_expected,4) == 0);

   Apx::DataVM vm;
   int result = vm.exec(pack_prog, serializedData, input);
   // Even if byte array is resized (reduced 4->2) the capacity of the underlying data should not change
   QVERIFY(preCapacity == serializedData.capacity());
   QVERIFY(result == 0);
   QVERIFY(serializedData.length()==2);
   QCOMPARE(serializedData, QByteArray("\x00\x00",2));
   uint8_t raw_expected[4] = {0,0,'\0', 0xaau};
   QVERIFY(memcmp(serializedData.data(),raw_expected,4) == 0);

   result = vm.exec(unpack_prog, serializedData, output);
   QVERIFY(result == 0);
   QCOMPARE(output,input);

   input = QVariant(0x1234);
   serializedData.clear();
   result = vm.exec(pack_prog, serializedData, input);
   QVERIFY(result == 0);
   QVERIFY(serializedData.length()==2);
   QCOMPARE(serializedData, QByteArray("\x34\x12",2));
   result = vm.exec(unpack_prog, serializedData, output);
   QVERIFY(result == 0);
   QCOMPARE(output,input);

}

void TestDataVM::test_packUnpackU32()
{
   QByteArray pack_prog(  "\x01\x00\x01\x00\x00\x04\x11",7); //pack U32 from QVaríant(scalar)
   QByteArray unpack_prog("\x01\x01\x01\x00\x00\x04\x04",7); //unpack U32 to QVaríant(scalar)

   QVariant input(0);
   QVariant output(0xffff);
   QByteArray serializedData;

   Apx::DataVM vm;
   int result = vm.exec(pack_prog, serializedData, input);
   QVERIFY(result == 0);
   QCOMPARE(serializedData, QByteArray("\x00\x00\x00\x00",4));
   result = vm.exec(unpack_prog, serializedData, output);
   QVERIFY(result == 0);
   QCOMPARE(output,input);


   input = QVariant(0x12345678);
   serializedData.clear();
   result = vm.exec(pack_prog, serializedData, input);
   QVERIFY(result == 0);
   QVERIFY(serializedData.length()==4);
   QCOMPARE(serializedData, QByteArray("\x78\x56\x34\x12",4));
   result = vm.exec(unpack_prog, serializedData, output);
   QVERIFY(result == 0);
   QCOMPARE(output,input);

   input = QVariant(0xffffffff);
   serializedData.clear();
   result = vm.exec(pack_prog, serializedData, input);
   QVERIFY(result == 0);
   QVERIFY(serializedData.length()==4);
   QCOMPARE(serializedData, QByteArray("\xff\xff\xff\xff",4));
   result = vm.exec(unpack_prog, serializedData, output);
   QVERIFY(result == 0);
   QCOMPARE(output,input);

}

void TestDataVM::test_packUnpackU8Array()
{
   QByteArray pack_prog(  "\x01\x00\x03\x00\x00\x03\x16\x00\x03",9); //pack U8 from QVaríantList
   QByteArray unpack_prog("\x01\x01\x03\x00\x00\x03\x09\x00\x03",9); //unpack U8 to QVaríantList

   QVariantList input;
   input.append(QVariant(1));
   input.append(QVariant(2));
   input.append(QVariant(3));
   QVariantList output;
   QByteArray serializedData;

   Apx::DataVM vm;
   int result = vm.exec(pack_prog, serializedData, input);
   QVERIFY(result == 0);
   QCOMPARE(serializedData, QByteArray("\x01\x02\x03",3));
   result = vm.exec(unpack_prog, serializedData, output);
   QVERIFY(result == 0);
   QCOMPARE(output,input);
}

void TestDataVM::test_unpackShortU8Array()
{
   QByteArray unpack_prog_both ("\x01\x01\x03\x00\x00\x03\x09\x00\x03",9); //unpack U8 to QVaríantList
   QByteArray unpack_prog_extra("\x01\x01\x03\x00\x00\x03\x09\x00\x02",9);
   QByteArray unpack_prog_arr  ("\x01\x01\x03\x00\x00\x02\x09\x00\x03",9);
   Apx::DataVM vm;
   QVariantList output;

   // Unpack short data
   QByteArray shortSerializedData = QByteArray("\x01\x02",2);
   int result = vm.exec(unpack_prog_both, shortSerializedData, output);
   QVERIFY(output.isEmpty());
   QVERIFY(result == VM_EXCEPTION_DATA_LEN_TOO_SHORT);
   result = vm.exec(unpack_prog_extra, shortSerializedData, output);
   QVERIFY(output.isEmpty());
   QVERIFY(result == VM_EXCEPTION_DATA_LEN_TOO_SHORT);
   result = vm.exec(unpack_prog_arr, shortSerializedData, output);
   QVERIFY(output.isEmpty());
   QVERIFY(result == VM_EXCEPTION_DATA_LEN_TOO_SHORT);

   QByteArray unpack_short_prog("\x01\x01\x03\x00\x00\x02\x09\x00\x02",9);
   result = vm.exec(unpack_short_prog, shortSerializedData, output);
   QVERIFY(result == 0);

   QVariantList input;
   input.append(QVariant(1));
   input.append(QVariant(2));
   QCOMPARE(output,input);
}

void TestDataVM::test_unpackPatchU8Array()
{
  Apx::DataVM vm;
  QVariantList output;

  // Unpack short data
  QByteArray shortSerializedData = QByteArray("\x01\x02",2);
  QByteArray unpack_short_prog("\x01\x01\x03\x00\x00\x02\x09\x00\x02",9);
  int result = vm.exec(unpack_short_prog, shortSerializedData, output);
  QVERIFY(result == 0);

  QVariantList input;
  input.append(QVariant(1));
  input.append(QVariant(2));
  QCOMPARE(output,input);

   QByteArray tinySerializedData = QByteArray("\x07",1);
   QByteArray unpack_tiny_prog("\x01\x01\x03\x00\x00\x01\x09\x00\x01",9);
   // only modify first list member
   result = vm.exec(unpack_tiny_prog, tinySerializedData, output);
   QVERIFY(result == 0);
   QVariantList input2;
   input2.append(QVariant(7));
   input2.append(QVariant(2));
   QCOMPARE(output,input2);
}

void TestDataVM::test_unpackProgErrorsU8Array()
{
   Apx::DataVM vm;
   QVariantList output;
   QByteArray shortSerializedData = QByteArray("\x01\x02",2);

   // Test extraargs not followed by op code
   QByteArray unpack_short_extraargs_prog("\x01\x01\x03\x00\x00\x01",6);
   int result = vm.exec(unpack_short_extraargs_prog, shortSerializedData, output);
   // TODO should it not be an error that extraargs is not followed by an op code?
   QVERIFY(result == 0);

   // Test short extraargs prog
   unpack_short_extraargs_prog.chop(1);
   result = vm.exec(unpack_short_extraargs_prog, shortSerializedData, output);
   QVERIFY(result == VM_EXCEPTION_PROGRAM_PARSE_ERROR);

   // Test missmatching extraargs and array size
   output.clear();
   QByteArray unpack_short_missmatch1_prog("\x01\x01\x03\x00\x00\x01\x09\x00\x02",9);
   result = vm.exec(unpack_short_missmatch1_prog, shortSerializedData, output);
   // TODO is it good that input validation is not done on data actally used?
   QVERIFY(result == 0);
   QVariantList input;
   input.append(QVariant(1));
   input.append(QVariant(2));
   QCOMPARE(output,input);

   QByteArray unpack_short_missmatch2_prog("\x01\x01\x03\x00\x00\x02\x09\x00\x01",9);
   result = vm.exec(unpack_short_missmatch2_prog, shortSerializedData, output);
   // TODO should input validation be sanity checked?
   QVERIFY(result == 0);

   // Test short array decode prog
   unpack_short_missmatch2_prog.chop(1);
   result = vm.exec(unpack_short_missmatch2_prog, shortSerializedData, output);
   QVERIFY(result == VM_EXCEPTION_PROGRAM_PARSE_ERROR);
}

void TestDataVM::test_packUnpackU16Array()
{
   QByteArray pack_prog(  "\x01\x00\x03\x00\x00\x08\x17\x00\x04",9); //pack U16 array from QVaríantList
   QByteArray unpack_prog("\x01\x01\x03\x00\x00\x08\x0a\x00\x04",9); //unpack U16 array to QVaríantList

   QVariantList input;
   input.append(QVariant(100));
   input.append(QVariant(200));
   input.append(QVariant(300));
   input.append(QVariant(400));
   QVariantList output;
   QByteArray serializedData;

   Apx::DataVM vm;
   int result = vm.exec(pack_prog, serializedData, input);
   QVERIFY(result == 0);
   QCOMPARE(serializedData, QByteArray("\x64\x00\xC8\x00\x2C\x01\x90\x01",8));
   result = vm.exec(unpack_prog, serializedData, output);
   QVERIFY(result == 0);
   QCOMPARE(output,input);
}

void TestDataVM::test_packUnpackU32Array()
{
   QByteArray pack_prog(  "\x01\x00\x03\x00\x00\x14\x18\x00\x05",9); //pack U32 array from QVaríantList
   QByteArray unpack_prog("\x01\x01\x03\x00\x00\x14\x0b\x00\x05",9); //unpack U32 array to QVaríantList

   QVariantList input;
   input.append(QVariant(20000));
   input.append(QVariant(40000));
   input.append(QVariant(80000));
   input.append(QVariant(160000));
   input.append(QVariant(320000));
   QVariantList output;
   QByteArray serializedData;

   Apx::DataVM vm;
   int result = vm.exec(pack_prog, serializedData, input);
   QVERIFY(result == 0);
   QCOMPARE(serializedData, QByteArray("\x20\x4e\x00\x00\x40\x9c\x00\x00\x80\x38\x01\x00\x00\x71\x02\x00\x00\xe2\x04\x00",20));
   result = vm.exec(unpack_prog, serializedData, output);
   QVERIFY(result == 0);
   QCOMPARE(output,input);
}

void TestDataVM::test_packUnpackString()
{
   QByteArray pack_prog(  "\x01\x00\x01\x00\x00\x10\x15\x00\x10",9); //pack string from QVariant, 16 characters maximum
   QByteArray unpack_prog("\x01\x01\x01\x00\x00\x10\x08\x00\x10",9); //unpack string array to QVaríant, 16 characters maximum

   QVariant input(QString("Hello World!"));
   QVariant output;
   QByteArray serializedData;

   Apx::DataVM vm;
   int result = vm.exec(pack_prog, serializedData, input);
   QVERIFY(result == 0);
   QCOMPARE(serializedData, QByteArray("Hello World!\x00\x00\x00\x00",16));
   result = vm.exec(unpack_prog, serializedData, output);
   QVERIFY(result == 0);
   QCOMPARE(output.toString().length(), input.toString().length());
   QCOMPARE(output,input);


   //test too long input string
   input = QVariant(QString("A Really Long String"));
   result = vm.exec(pack_prog, serializedData, input);
   QVERIFY(result == VM_EXCEPTION_DATA_LEN_TOO_SHORT);

   //test exactly the allowed length (with no null terminator)
   input = QVariant(QString("A Normal String1"));
   result = vm.exec(pack_prog, serializedData, input);
   QVERIFY(result == VM_EXCEPTION_NO_EXCEPTION);
   QCOMPARE(serializedData, QByteArray("A Normal String1",16));
   result = vm.exec(unpack_prog, serializedData, output);
   QVERIFY(result == VM_EXCEPTION_NO_EXCEPTION);
   QCOMPARE(output.toString().length(), input.toString().length());
   QCOMPARE(output,input);

   //test string which uses null-terminator as last character
   input = QVariant(QString("A Normal String"));
   result = vm.exec(pack_prog, serializedData, input);
   QVERIFY(result == VM_EXCEPTION_NO_EXCEPTION);
   QCOMPARE(serializedData, QByteArray("A Normal String\x00",16));
   result = vm.exec(unpack_prog, serializedData, output);
   QVERIFY(result == VM_EXCEPTION_NO_EXCEPTION);
   QCOMPARE(output.toString().length(), input.toString().length());
   QCOMPARE(output,input);

}
