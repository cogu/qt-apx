#include "test_qapx_filemap.h"
#include "qapx_filemap.h"
#include "qapx_file.h"

void TestApxFileMap::assignFileAddress()
{
   Apx::FileMap map;
   Apx::File f1("test.out",6840);
   bool result = map.assignFileAddress(&f1, 0, 65536, 1024);
   QVERIFY(result == true);
   QCOMPARE(f1.mAddress, (quint32)0);
   result = map.insert(&f1);
   QVERIFY(result == true);
   QVERIFY(map.size() == 1);

   Apx::File f2("test.in",64);
   result = map.assignFileAddress(&f2, 0, 65536, 1024);
   QVERIFY(result == true);
   QCOMPARE(f2.mAddress, (quint32)1024*7);
   result = map.insert(&f2);
   QVERIFY(result == true);
   QVERIFY(map.size() == 2);

   Apx::File f3("test.txt",64);
   result = map.assignFileAddress(&f3, 65536, 131072, 4096);
   QVERIFY(result == true);
   QCOMPARE(f3.mAddress, (quint32)65536);
   result = map.insert(&f3);
   QVERIFY(result == true);
   QVERIFY(map.size() == 3);

   Apx::File f4("test.bin",100);
   result = map.assignFileAddress(&f4,0,65536,1024);
   QVERIFY(result == true);
   QCOMPARE(f4.mAddress, (quint32)1024*8);
   result = map.insert(&f4);
   QVERIFY(result == true);
   QVERIFY(map.size() == 4);

   Apx::File f5("test2.bin",8000);
   result = map.assignFileAddress(&f5,65536,131072,4096);
   QVERIFY(result == true);
   QCOMPARE(f5.mAddress, (quint32)65536+4096);
   result = map.insert(&f5);
   QVERIFY(result == true);
   QVERIFY(map.size() == 5);

   Apx::File f6("test3.bin",4000);
   result = map.assignFileAddress(&f6,65536,131072,4096);
   QVERIFY(result == true);
   QCOMPARE(f6.mAddress, (quint32)65536+4096*3);
   result = map.insert(&f6);
   QVERIFY(result == true);
   QVERIFY(map.size() == 6);

   Apx::File f7("test4.in",128);
   result = map.assignFileAddress(&f7, 0, 65536, 1024);
   QVERIFY(result == true);
   QCOMPARE(f7.mAddress, (quint32)1024*9);
   result = map.insert(&f7);
   QVERIFY(result == true);
   QVERIFY(map.size() == 7);

}

void TestApxFileMap::test_findByAddress()
{
   bool result;
   Apx::FileMap map;
   Apx::File f1("test.out",6840);
   result = map.insert(&f1);
   QVERIFY(result == true);
   QCOMPARE(f1.mAddress, (quint32)0);

   Apx::File f2("test.in",64);
   result = map.insert(&f2);
   QVERIFY(result == true);
   QCOMPARE(f2.mAddress, (quint32)1024*7);

   Apx::File f3("test.txt",64);
   result = map.insert(&f3);
   QVERIFY(result == true);
   QCOMPARE(f3.mAddress, (quint32)USER_DATA_START);

   Apx::File f4("test.bin",100);
   result = map.insert(&f4);
   QVERIFY(result == true);
   QCOMPARE(f4.mAddress, (quint32)USER_DATA_START+USER_DATA_BOUNDARY);

   Apx::File f5("test2.bin",8000);
   result = map.insert(&f5);
   QVERIFY(result == true);
   QCOMPARE(f5.mAddress, (quint32)USER_DATA_START+USER_DATA_BOUNDARY*2);

   Apx::File f6("test3.bin",4000);
   result = map.insert(&f6);
   QVERIFY(result == true);
   QCOMPARE(f6.mAddress, (quint32)USER_DATA_START+USER_DATA_BOUNDARY*3);

   Apx::File f7("test4.in",128);
   result = map.insert(&f7);
   QVERIFY(result == true);
   QCOMPARE(f7.mAddress, (quint32)1024*8);

   Apx::File *file = map.findByAddress(8192);
   QVERIFY(file == &f7);
   file = map.findByAddress(7228);
   QVERIFY(file == &f2);
   file = map.findByAddress(0);
   QVERIFY(file == &f1);
   file = map.findByAddress(10000);
   QVERIFY(file == nullptr);

}
