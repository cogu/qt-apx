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
   void test_onReadyRead_partial_short_msg();
   void test_onReadyRead_partial_long_msg();
   void test_getSocketReadAvail_single();
   void test_getSocketReadAvail_multi();
   void test_getSocketReadAvail_not_connected();
};

}
#endif // TEST_QRMF_SOCKETADAPTER_H
