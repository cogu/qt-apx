#include <QByteArray>
#include <cstring>
#include "test_numheader.h"
#include "qnumheader.h"

using namespace std;

void TestNumHeader::test_encode16()
{
   int result;
   QByteArray array(2,0); //create byte array of 2 bytes, initialize with zeros
   result=NumHeader::encode16(array.data(), array.length(), 0);
   QVERIFY( result == 1);
   QCOMPARE(array, QByteArray("\x00\x00",2));

   memset(array.data(),0, 2);
   result=NumHeader::encode16(array.data(), array.length(), 127);
   QVERIFY( result == 1);
   QCOMPARE(array, QByteArray("\x7F\x00",2));

   memset(array.data(),0, 2);
   result=NumHeader::encode16(array.data(), array.length(), 128);
   QVERIFY( result == 2);
   QCOMPARE(array, QByteArray("\x80\x80",2));

   memset(array.data(),0, 2);
   result=NumHeader::encode16(array.data(), array.length(), 32767);
   QVERIFY( result == 2);
   QCOMPARE(array, QByteArray("\xFF\xFF",2));

   memset(array.data(),0, 2);
   result=NumHeader::encode16(array.data(), array.length(), 32768);
   QVERIFY( result == 2);
   QCOMPARE(array, QByteArray("\x80\x00",2));

   memset(array.data(),0, 2);
   result=NumHeader::encode16(array.data(), array.length(), 32895);
   QVERIFY( result == 2);
   QCOMPARE(array, QByteArray("\x80\x7F",2));

   memset(array.data(),0, 2);
   result=NumHeader::encode16(array.data(), array.length(), 32896);
   QVERIFY( result == -1);
   QCOMPARE(array, QByteArray("\x00\x00",2));
}

void TestNumHeader::test_encode32()
{
   int result;
   QByteArray array(4,0); //create byte array of 4 bytes, initialize with zeros
   result=NumHeader::encode32(array.data(), array.length(), 0);
   QVERIFY( result == 1);
   QCOMPARE(array, QByteArray("\x00\x00\x00\x00",4));

   memset(array.data(),0, 4);
   result=NumHeader::encode32(array.data(), array.length(), 127);
   QVERIFY( result == 1);
   QCOMPARE(array, QByteArray("\x7F\x00\x00\x00",4));

   memset(array.data(),0, 4);
   result=NumHeader::encode32(array.data(), array.length(), 128);
   QVERIFY( result == 4);
   QCOMPARE(array, QByteArray("\x80\x00\x00\x80",4));

   memset(array.data(),0, 4);
   result=NumHeader::encode32(array.data(), array.length(), 32767);
   QVERIFY( result == 4);
   QCOMPARE(array, QByteArray("\x80\x00\x7F\xFF",4));

   memset(array.data(),0, 4);
   result=NumHeader::encode32(array.data(), array.length(), 32768);
   QVERIFY( result == 4);
   QCOMPARE(array, QByteArray("\x80\x00\x80\x00",4));

   memset(array.data(),0, 4);
   result=NumHeader::encode32(array.data(), array.length(), 32895);
   QVERIFY( result == 4);
   QCOMPARE(array, QByteArray("\x80\x00\x80\x7F",4));

   memset(array.data(),0, 4);
   result=NumHeader::encode32(array.data(), array.length(), 2147483647);
   QVERIFY( result == 4);
   QCOMPARE(array, QByteArray("\xFF\xFF\xFF\xFF",4));

   memset(array.data(),0, 4);
   result=NumHeader::encode32(array.data(), array.length(), 2147483648);
   QVERIFY( result == -1);
   QCOMPARE(array, QByteArray("\x00\x00\x00\x00",4));

}

void TestNumHeader::test_decode16()
{
   int result;
   quint16 value;
   QByteArray array;

   array = QByteArray("\x00",1);
   result=NumHeader::decode16(array.data(), array.data()+array.length(), &value);
   QVERIFY(result == 1);
   QCOMPARE(value, (quint16) 0);

   array = QByteArray("\x00\x00",2);
   result=NumHeader::decode16(array.data(), array.data()+array.length(), &value);
   QVERIFY(result == 1);
   QCOMPARE(value, (quint16) 0);

   array = QByteArray("\x7F\x00",2);
   result=NumHeader::decode16(array.data(), array.data()+array.length(), &value);
   QVERIFY(result == 1);
   QCOMPARE(value, (quint16) 127);

   array = QByteArray("\x80",1);
   result=NumHeader::decode16(array.data(), array.data()+array.length(), &value);
   QVERIFY(result == 0);

   array = QByteArray("\x80\x80",2);
   result=NumHeader::decode16(array.data(), array.data()+array.length(), &value);
   QVERIFY(result == 2);
   QCOMPARE(value, (quint16) 128);

   array = QByteArray("\xFF\xFF",2);
   result=NumHeader::decode16(array.data(), array.data()+array.length(), &value);
   QVERIFY(result == 2);
   QCOMPARE(value, (quint16) 32767);

   array = QByteArray("\x80\x00",2);
   result=NumHeader::decode16(array.data(), array.data()+array.length(), &value);
   QVERIFY(result == 2);
   QCOMPARE(value, (quint16) 32768);

   array = QByteArray("\x80\x7F",2);
   result=NumHeader::decode16(array.data(), array.data()+array.length(), &value);
   QVERIFY(result == 2);
   QCOMPARE(value, (quint16) 32895);
}

void TestNumHeader::test_decode32()
{
   int result;
   quint32 value;
   QByteArray array;

   array = QByteArray("\x00",1);
   result=NumHeader::decode32(array.data(), array.data()+array.length(), &value);
   QVERIFY(result == 1);
   QCOMPARE(value, (quint32) 0);

   array = QByteArray("\x00\x00\x00\x00",4);
   result=NumHeader::decode32(array.data(), array.data()+array.length(), &value);
   QVERIFY(result == 1);
   QCOMPARE(value, (quint32) 0);


   array = QByteArray("\x7F",1);
   result=NumHeader::decode32(array.data(), array.data()+array.length(), &value);
   QVERIFY(result == 1);
   QCOMPARE(value, (quint32) 127);

   array = QByteArray("\x80",1);
   result=NumHeader::decode32(array.data(), array.data()+array.length(), &value);
   QVERIFY(result == 0); //buffer too small

   array = QByteArray("\x80\x00",2);
   result=NumHeader::decode32(array.data(), array.data()+array.length(), &value);
   QVERIFY(result == 0); //buffer too small

   array = QByteArray("\x80\x00\x00",3);
   result=NumHeader::decode32(array.data(), array.data()+array.length(), &value);
   QVERIFY(result == 0); //buffer too small

   array = QByteArray("\x80\x00\x00\x80",4);
   result=NumHeader::decode32(array.data(), array.data()+array.length(), &value);
   QVERIFY(result == 4);
   QCOMPARE(value, (quint32) 128);

   array = QByteArray("\x80\x00\x7F\xFF",4);
   result=NumHeader::decode32(array.data(), array.data()+array.length(), &value);
   QVERIFY(result == 4);
   QCOMPARE(value, (quint32) 32767);

   array = QByteArray("\x80\x00\x80\x00",4);
   result=NumHeader::decode32(array.data(), array.data()+array.length(), &value);
   QVERIFY(result == 4);
   QCOMPARE(value, (quint32) 32768);

   array = QByteArray("\x80\x00\x80\x7F",4);
   result=NumHeader::decode32(array.data(), array.data()+array.length(), &value);
   QVERIFY(result == 4);
   QCOMPARE(value, (quint32) 32895);

   array = QByteArray("\xFF\xFF\xFF\xFF",4);
   result=NumHeader::decode32(array.data(), array.data()+array.length(), &value);
   QVERIFY(result == 4);
   QCOMPARE(value, (quint32) 2147483647);

   array = QByteArray("\x80\x00\x00\x00",4);
   result=NumHeader::decode32(array.data(), array.data()+array.length(), &value);
   QVERIFY(result == 4);
   QCOMPARE(value, (quint32) 0);

}
