#ifndef TESTAPXNODEDATA_H
#define TESTAPXNODEDATA_H

#include <QtTest/QtTest>

class TestApxNodeData : public QObject
{
   Q_OBJECT
private slots:
   void test_createNodeDataDualNewline();
   void test_createNodeData();
   void test_getRequirePortValue();
};

#endif // TESTAPXNODEDATA_H
