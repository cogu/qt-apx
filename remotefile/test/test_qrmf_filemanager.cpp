#include "test_qrmf_filemanager.h"
#include "qrmf_filemanager.h"
#include "qrmf_proto.h"
#include "mocktransmithandler.h"
#include <QSignalSpy>


namespace RemoteFile
{


void TestFileManager::test_start_stop()
{
   RemoteFile::FileMap *localFileMap = new RemoteFile::FileMapDefault;
   RemoteFile::FileMap *remoteFileMap = new RemoteFile::FileMapDefault;
   RemoteFile::FileManager manager(localFileMap, remoteFileMap);

   delete localFileMap;
   delete remoteFileMap;
}

void TestFileManager::test_onConnect()
{
   MockTransmitHandler transmitHandler(NULL);
   QSignalSpy spy(&transmitHandler, SIGNAL(messageReceived()));
   RemoteFile::FileMap *localFileMap = new RemoteFile::FileMapDefault;
   RemoteFile::FileMap *remoteFileMap = new RemoteFile::FileMapDefault;
   RemoteFile::FileManager *manager  = new RemoteFile::FileManager(localFileMap, remoteFileMap);
   RemoteFile::File f1("test1.txt",100);
   RemoteFile::File f2("test_data.bin",200);
   RemoteFile::File f3("test2.bin",400);

   manager->attachLocalFile(&f1);
   manager->attachLocalFile(&f2);
   manager->attachLocalFile(&f3);
   manager->onConnected(&transmitHandler);

   spy.wait(250);
   QCOMPARE(spy.count(), 3);

   delete manager;
   delete localFileMap;
   delete remoteFileMap;


}

void TestFileManager::onMessageReceived()
{

}

}
