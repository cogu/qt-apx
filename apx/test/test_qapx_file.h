#ifndef TEST_QAPX_FILE_H
#define TEST_QAPX_FILE_H

#include <QtTest/QtTest>

class TestApxFile : public QObject
{
   Q_OBJECT   
   private slots:
      void inputFile();
      void outputFile();
};

#endif // TEST_QAPX_FILE_H
