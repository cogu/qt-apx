#include <QtTest/QtTest>
#include "test_numheader.h"
#include "test_qrmf_proto.h"
#include "test_qrmf_filemanager.h"


int main(int argc, char *argv[])
{
   int status = 0;
   {
      QCoreApplication app(argc, argv);
      TestNumHeader tc1;
      status |= QTest::qExec(&tc1, argc, argv);

      TestRemoteFileProtocol tc2;
      status |= QTest::qExec(&tc2, argc, argv);

      RemoteFile::TestFileManager tc3;
      status |= QTest::qExec(&tc3, argc, argv);
   }
   return status;
}
