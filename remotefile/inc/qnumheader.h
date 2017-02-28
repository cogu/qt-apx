#ifndef QNUMHEADER_H
#define QNUMHEADER_H
#include <QtGlobal>

#define NUMHEADER_SHORT_MSG_LIMIT ((quint16) 127u)
#define NUMHEADER16_LONG_MSG_LIMIT ((quint16) 32895u) //128+32767
#define NUMHEADER32_LONG_MSG_LIMIT ((quint32) 2147483647) //2^31-1

namespace NumHeader {

int encode16(char *pDest, int destLimit, quint16 value);
int decode16(const char *pBegin, const char *pEnd, quint16 *value);
int encode32(char *pDest, int destLimit, quint32 value);
int decode32(const char *pBegin, const char *pEnd, quint32 *value);


}
#endif // QNUMHEADER_H
