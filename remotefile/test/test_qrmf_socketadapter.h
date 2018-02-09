#ifndef TEST_QRMF_SOCKETADAPTER_H
#define TEST_QRMF_SOCKETADAPTER_H

#include <QtTest/QtTest>
namespace RemoteFile
{

class TestSocketAdapter : public QObject
{
   Q_OBJECT
private slots:
   void test_acknowledge();
   void test_onReadyRead();
   void test_onReadyReadPartial();
};

}
#endif // TEST_QRMF_SOCKETADAPTER_H
