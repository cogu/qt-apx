#ifndef TEST_QAPX_BYTEPARSER_H
#define TEST_QAPX_BYTEPARSER_H


#include <QtTest/QtTest>


class TestApxByteParser : public QObject
{
   Q_OBJECT
   private slots:
      void parseNormal();
      void parseBroken();
};

#endif // TEST_QAPX_BYTEPARSER_H
