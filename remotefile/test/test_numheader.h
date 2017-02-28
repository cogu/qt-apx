#ifndef TESTNUMHEADER_H
#define TESTNUMHEADER_H

#include <QtTest/QtTest>

class TestNumHeader : public QObject
{
   Q_OBJECT
private slots:
   void test_encode16();
   void test_encode32();
   void test_decode16();
   void test_decode32();
};

#endif // TESTNUMHEADER_H
