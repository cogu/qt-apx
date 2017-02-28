#ifndef TEST_QRMF_FILEMANAGER_H
#define TEST_QRMF_FILEMANAGER_H

#include <QtTest/QtTest>

namespace RemoteFile
{
   class TestFileManager : public QObject
   {
      Q_OBJECT
   private slots:
      void test_start_stop();
      void test_onConnect();
   public slots:
      void onMessageReceived();
   };
}


#endif // TEST_QRMF_FILEMANAGER_H
