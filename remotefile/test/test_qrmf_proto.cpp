#include <cstring>
#include "test_qrmf_proto.h"
#include "qrmf_proto.h"

using namespace std;

void TestRemoteFileProtocol::test_packHeader()
{
   int result;
   QByteArray array(4,0);
   result=RemoteFile::packHeader(array.data(), array.length(), 0, false);
   QVERIFY(result == 2);
   QCOMPARE(array, QByteArray("\x00\x00\x00\x00",4));

   memset(array.data(),0, array.length());
   result=RemoteFile::packHeader(array.data(), array.length(), 0, true);
   QVERIFY(result == 2);
   QCOMPARE(array, QByteArray("\x40\x00\x00\x00",4));

   memset(array.data(),0, array.length());
   result=RemoteFile::packHeader(array.data(), array.length(), 16383, false);
   QVERIFY(result == 2);
   QCOMPARE(array, QByteArray("\x3F\xFF\x00\x00",4));

   memset(array.data(),0, array.length());
   result=RemoteFile::packHeader(array.data(), array.length(), 16383, true);
   QVERIFY(result == 2);
   QCOMPARE(array, QByteArray("\x7F\xFF\x00\x00",4));

   memset(array.data(),0, array.length());
   result=RemoteFile::packHeader(array.data(), array.length(), 16384, false);
   QVERIFY(result == 4);
   QCOMPARE(array, QByteArray("\x80\x00\x40\x00",4));

   memset(array.data(),0, array.length());
   result=RemoteFile::packHeader(array.data(), array.length(), 16384, true);
   QVERIFY(result == 4);
   QCOMPARE(array, QByteArray("\xC0\x00\x40\x00",4));

   memset(array.data(),0, array.length());
   result=RemoteFile::packHeader(array.data(), array.length(), 1073741823, false);
   QVERIFY(result == 4);
   QCOMPARE(array, QByteArray("\xBF\xFF\xFF\xFF",4));

   memset(array.data(),0, array.length());
   result=RemoteFile::packHeader(array.data(), array.length(), 1073741823, true);
   QVERIFY(result == 4);
   QCOMPARE(array, QByteArray("\xFF\xFF\xFF\xFF",4));

}

void TestRemoteFileProtocol::test_unpackHeader()
{
   int result;
   quint32 address;
   bool more_bit;
   QByteArray array;

   array = QByteArray("\x00",1);
   result=RemoteFile::unpackHeader(array.data(), array.data()+array.length(), &address, &more_bit);
   QCOMPARE(result, (int) 0);

   array = QByteArray("\x00\x00",2);
   result=RemoteFile::unpackHeader(array.data(), array.data()+array.length(), &address, &more_bit);
   QVERIFY(result == 2);
   QCOMPARE(address, (quint32) 0);
   QCOMPARE(more_bit, false);

   array = QByteArray("\x40\x00",2);
   result=RemoteFile::unpackHeader(array.data(), array.data()+array.length(), &address, &more_bit);
   QVERIFY(result == 2);
   QCOMPARE(address, (quint32) 0);
   QCOMPARE(more_bit, true);


   array = QByteArray("\x3F\xFF",2);
   result=RemoteFile::unpackHeader(array.data(), array.data()+array.length(), &address, &more_bit);
   QVERIFY(result == 2);
   QCOMPARE(address, (quint32) 16383);
   QCOMPARE(more_bit, false);

   array = QByteArray("\x7F\xFF",2);
   result=RemoteFile::unpackHeader(array.data(), array.data()+array.length(), &address, &more_bit);
   QVERIFY(result == 2);
   QCOMPARE(address, (quint32) 16383);
   QCOMPARE(more_bit, true);


   array = QByteArray("\x80",1);
   result=RemoteFile::unpackHeader(array.data(), array.data()+array.length(), &address, &more_bit);
   QVERIFY(result == 0); //buffer too short

   array = QByteArray("\x80\x00",2);
   result=RemoteFile::unpackHeader(array.data(), array.data()+array.length(), &address, &more_bit);
   QVERIFY(result == 0); //buffer (still) too short

   array = QByteArray("\x80\x00\x40",3);
   result=RemoteFile::unpackHeader(array.data(), array.data()+array.length(), &address, &more_bit);
   QVERIFY(result == 0); //buffer (still) too short

   array = QByteArray("\x80\x00\x40\x00",4);
   result=RemoteFile::unpackHeader(array.data(), array.data()+array.length(), &address, &more_bit);
   QVERIFY(result == 4);
   QCOMPARE(address, (quint32) 16384);
   QCOMPARE(more_bit, false);

   array = QByteArray("\xC0\x00\x40\x00",4);
   result=RemoteFile::unpackHeader(array.data(), array.data()+array.length(), &address, &more_bit);
   QVERIFY(result == 4);
   QCOMPARE(address, (quint32) 16384);
   QCOMPARE(more_bit, true);

   array = QByteArray("\xBF\xFF\xFF\xFF",4);
   result=RemoteFile::unpackHeader(array.data(), array.data()+array.length(), &address, &more_bit);
   QVERIFY(result == 4);
   QCOMPARE(address, (quint32) 1073741823);
   QCOMPARE(more_bit, false);

   array = QByteArray("\xFF\xFF\xFF\xFF",4);
   result=RemoteFile::unpackHeader(array.data(), array.data()+array.length(), &address, &more_bit);
   QVERIFY(result == 4);
   QCOMPARE(address, (quint32) 1073741823);
   QCOMPARE(more_bit, true);


}

void TestRemoteFileProtocol::test_packFileInfo()
{
   const char *name1 = "file1.txt";
   RemoteFile::File file1(name1, 1000);
   file1.mAddress = 0x12345678;
   QByteArray array(RMF_FILEINFO_BASE_LEN+(int)strlen(name1)+1,'\0');
   int result;

   result = RemoteFile::packFileInfo(array.data(), array.length(), file1);
   QCOMPARE(result, (int)RMF_FILEINFO_BASE_LEN+(int)strlen(name1)+1);

   QByteArray expected = QByteArray("\x03\x00\x00\x00\x78\x56\x34\x12\xe8\x03\x00\x00\x00\x00\x00\x00" //16
                                    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //32
                                    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //48
                                    "file1.txt"
                                    ,RMF_FILEINFO_BASE_LEN+(int)strlen(name1)+1);
   QCOMPARE(array, expected);

}

void TestRemoteFileProtocol::test_unpackFileInfo()
{
   const char *name1 = "file1.txt";
   QByteArray array = QByteArray(   "\x03\x00\x00\x00\x78\x56\x34\x12\xe8\x03\x00\x00\x00\x00\x00\x00" //16
                                    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //32
                                    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //48
                                    "file1.txt"
                                    ,RMF_FILEINFO_BASE_LEN+(int)strlen(name1)+1);
   RemoteFile::File file1;
   int result = RemoteFile::unpackFileInfo(array.constData(), array.constData()+array.length(), file1);
   QCOMPARE(result, (int)RMF_FILEINFO_BASE_LEN+(int)strlen(name1)+1);
   QCOMPARE(file1.mName, QString("file1.txt"));
   QCOMPARE(file1.mAddress, (quint32) 0x12345678);
   QCOMPARE(file1.mLength, (quint32) 1000);
   QCOMPARE(file1.mFileType, RemoteFile::RMF_FILE_TYPE_FIXED);
   QCOMPARE(file1.mDigestType, RemoteFile::RMF_DIGEST_TYPE_NONE);

   //it should also work if null-terminator is forgotten
   result = RemoteFile::unpackFileInfo(array.constData(), array.constData()+array.length()-1, file1);
   QCOMPARE(result, (int)RMF_FILEINFO_BASE_LEN+(int)strlen(name1));
   QCOMPARE(file1.mName, QString("file1.txt"));
   QCOMPARE(file1.mAddress, (quint32) 0x12345678);
   QCOMPARE(file1.mLength, (quint32) 1000);
   QCOMPARE(file1.mFileType, RemoteFile::RMF_FILE_TYPE_FIXED);
   QCOMPARE(file1.mDigestType, RemoteFile::RMF_DIGEST_TYPE_NONE);

   //what if array cuts of directly after RMF_FILEINFO_BASE_LEN bytes?
   result = RemoteFile::unpackFileInfo(array.constData(), array.constData()+RMF_FILEINFO_BASE_LEN, file1);
   QCOMPARE(result, (int)RMF_FILEINFO_BASE_LEN);
   QCOMPARE(file1.mName, QString(""));
   QCOMPARE(file1.mAddress, (quint32) 0x12345678);
   QCOMPARE(file1.mLength, (quint32) 1000);
   QCOMPARE(file1.mFileType, RemoteFile::RMF_FILE_TYPE_FIXED);
   QCOMPARE(file1.mDigestType, RemoteFile::RMF_DIGEST_TYPE_NONE);

   //FileInfo struct with null-terminator directly after RMF_FILEINFO_BASE_LEN bytes
   array = QByteArray(   "\x03\x00\x00\x00\x78\x56\x34\x12\xe8\x03\x00\x00\x00\x00\x00\x00" //16
                         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //32
                         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //48
                         "\x00",RMF_FILEINFO_BASE_LEN+1); //49
   result = RemoteFile::unpackFileInfo(array.constData(), array.constData()+array.length(), file1);
   QCOMPARE(result, (int)RMF_FILEINFO_BASE_LEN+1);
   QCOMPARE(file1.mName, QString(""));
   QCOMPARE(file1.mAddress, (quint32) 0x12345678);
   QCOMPARE(file1.mLength, (quint32) 1000);
   QCOMPARE(file1.mFileType, RemoteFile::RMF_FILE_TYPE_FIXED);
   QCOMPARE(file1.mDigestType, RemoteFile::RMF_DIGEST_TYPE_NONE);


}

void TestRemoteFileProtocol::test_packFileOpen()
{
   quint32 address;
   QByteArray array(RMF_FILE_OPEN_LEN,0);
   int result;

   address=0x00000000;
   result = RemoteFile::packFileOpen(array.data(), array.length(), address);
   QCOMPARE(result, array.length());
   QCOMPARE(array, QByteArray("\x0a\x00\x00\x00\x00\x00\x00\x00",RMF_FILE_OPEN_LEN));

   address=0x12345678;
   result = RemoteFile::packFileOpen(array.data(), array.length(), address);
   QCOMPARE(result, array.length());
   QCOMPARE(array, QByteArray("\x0a\x00\x00\x00\x78\x56\x34\x12",RMF_FILE_OPEN_LEN));

   address=0xffffffff;
   result = RemoteFile::packFileOpen(array.data(), array.length(), address);
   QCOMPARE(result, array.length());
   QCOMPARE(array, QByteArray("\x0a\x00\x00\x00\xff\xff\xff\xff",RMF_FILE_OPEN_LEN));

}

void TestRemoteFileProtocol::test_unpackFileOpen()
{
   QByteArray array = QByteArray(  "\x0a\x00\x00\x00\x00\x00\x00\x00",RMF_FILE_OPEN_LEN);
   quint32 address;
   int result;
   for (int i=0; i < array.length();i++)
   {
      result = RemoteFile::unpackFileOpen(array.constData(),array.constData()+i,address);
      QCOMPARE(result, 0);
   }
   result = RemoteFile::unpackFileOpen(array.constData(),array.constData()+array.length(),address);
   QCOMPARE(result, (int)RMF_FILE_OPEN_LEN);
   QCOMPARE(address, (quint32)0x00000000);

   array = QByteArray(  "\x0a\x00\x00\x00\x78\x56\x34\x12",RMF_FILE_OPEN_LEN);
   result = RemoteFile::unpackFileOpen(array.constData(),array.constData()+array.length(),address);
   QCOMPARE(result, (int)RMF_FILE_OPEN_LEN);
   QCOMPARE(address, (quint32)0x12345678);

   //verify invalid cmdType check
   array = QByteArray(  "\x0b\x00\x00\x00\x78\x56\x34\x12",RMF_FILE_OPEN_LEN);
   result = RemoteFile::unpackFileOpen(array.constData(),array.constData()+array.length(),address);
   QCOMPARE(result, -2);

}

void TestRemoteFileProtocol::test_packFileClose()
{
   quint32 address;
   QByteArray array(RMF_FILE_OPEN_LEN,0);
   int result;

   address=0x00000000;
   result = RemoteFile::packFileClose(array.data(), array.length(), address);
   QCOMPARE(result, array.length());
   QCOMPARE(array, QByteArray("\x0b\x00\x00\x00\x00\x00\x00\x00",RMF_FILE_CLOSE_LEN));

   address=0x12345678;
   result = RemoteFile::packFileClose(array.data(), array.length(), address);
   QCOMPARE(result, array.length());
   QCOMPARE(array, QByteArray("\x0b\x00\x00\x00\x78\x56\x34\x12",RMF_FILE_CLOSE_LEN));

   address=0xffffffff;
   result = RemoteFile::packFileClose(array.data(), array.length(), address);
   QCOMPARE(result, array.length());
   QCOMPARE(array, QByteArray("\x0b\x00\x00\x00\xff\xff\xff\xff",RMF_FILE_CLOSE_LEN));

}

void TestRemoteFileProtocol::test_unpackFileClose()
{
   QByteArray array = QByteArray(  "\x0b\x00\x00\x00\x00\x00\x00\x00",RMF_FILE_CLOSE_LEN);
   quint32 address;
   int result;
   for (int i=0; i < array.length();i++)
   {
      result = RemoteFile::unpackFileClose(array.constData(),array.constData()+i,address);
      QCOMPARE(result, 0);
   }
   result = RemoteFile::unpackFileClose(array.constData(),array.constData()+array.length(),address);
   QCOMPARE(result, (int)RMF_FILE_CLOSE_LEN);
   QCOMPARE(address, (quint32)0x00000000);

   array = QByteArray(  "\x0b\x00\x00\x00\x78\x56\x34\x12",RMF_FILE_CLOSE_LEN);
   result = RemoteFile::unpackFileClose(array.constData(),array.constData()+array.length(),address);
   QCOMPARE(result, (int)RMF_FILE_CLOSE_LEN);
   QCOMPARE(address, (quint32)0x12345678);

   //verify invalid cmdType check
   array = QByteArray(  "\x0a\x00\x00\x00\x78\x56\x34\x12",RMF_FILE_CLOSE_LEN);
   result = RemoteFile::unpackFileClose(array.constData(),array.constData()+array.length(),address);
   QCOMPARE(result, -2);
}
