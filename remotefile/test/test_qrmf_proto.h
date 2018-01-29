#ifndef TEST_QREMOTEFILE_H
#define TEST_QREMOTEFILE_H

#include <QtTest/QtTest>

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
