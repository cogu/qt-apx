#ifndef TEST_QREMOTEFILE_H
#define TEST_QREMOTEFILE_H

#include <QtTest/QtTest>

#define RMF_CMD_ACK                (quint32) 0   //reserved for future use
#define RMF_CMD_NACK               (quint32) 1   //reserved for future use
#define RMF_CMD_EOT                (quint32) 2   //reserved for future use
#define RMF_CMD_FILE_INFO          (quint32) 3   //serialized file info data structure
#define RMF_CMD_REVOKE_FILE        (quint32) 4   //used by server to tell clients that the file is no longer available
//range reserved for future use
#define RMF_CMD_FILE_OPEN         (quint32) 10  //opens a file
#define RMF_CMD_FILE_CLOSE        (quint32) 11  //closes a file

class TestRemoteFileProtocol : public QObject
{
   Q_OBJECT
private slots:
   void test_packHeader();
   void test_unpackHeader();
   void test_packFileInfo();
   void test_unpackFileInfo();
   void test_packFileOpen();
   void test_unpackFileOpen();
   void test_packFileClose();
   void test_unpackFileClose();
};

#endif // TEST_QREMOTEFILE_H
