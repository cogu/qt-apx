#ifndef QSCAN_H
#define QSCAN_H
#include <QtGlobal>
#include <QByteArray>

const quint8 *qscan_searchUntil(const quint8 *pBegin, const quint8 *pEnd, quint8 val);
const quint8 *qscan_matchPair(const quint8 *pBegin, const quint8 *pEnd, const quint8 left, const quint8 right, const quint8 escapeChar);
const quint8 *qscan_matchStr(const quint8 *pBegin, const quint8 *pEnd,const quint8 *pStrBegin, const quint8 *pStrEnd);
const quint8 *qscan_digit(const quint8 *pBegin, const quint8 *pEnd);
const quint8 *qscan_toInt(const quint8 *pBegin, const quint8 *pEnd,int *data);
QByteArray qscan_toByteArray(const quint8 *pBegin, const quint8 *pEnd);

#endif // QSCAN_H
