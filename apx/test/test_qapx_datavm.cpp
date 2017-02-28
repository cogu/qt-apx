#include "test_qapx_datavm.h"
#include "qapx_datavm.h"

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

   Apx::DataVM vm;
   int result = vm.exec(pack_prog, serializedData, input);
   QVERIFY(result == 0);
   QVERIFY(serializedData.length()==2);
   QCOMPARE(serializedData, QByteArray("\x00\x00",2));
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
