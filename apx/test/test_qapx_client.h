#ifndef TEST_QAPX_CLIENT_H
#define TEST_QAPX_CLIENT_H

#include <QtTest/QtTest>

class TestApxClient : public QObject
{
   Q_OBJECT
   private slots:
      void createClient();
      void connectToServer(); //WARNING! This, test requires a localhost server running at port 5000
};

#endif // TEST_QAPX_CLIENT_H
