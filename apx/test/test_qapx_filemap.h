#ifndef TEST_QAPX_FILEMAP_H
#define TEST_QAPX_FILEMAP_H

#include <QtTest/QtTest>

class TestApxFileMap: public QObject
{
    Q_OBJECT
private slots:
    void assignFileAddress();
    void test_findByAddress();
};

#endif // TEST_QAPX_FILEMAP_H
