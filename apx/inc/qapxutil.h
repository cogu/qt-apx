#ifndef APX_UTIL_H
#define APX_UTIL_H
#include <QtGlobal>
#include <QVector>

/*
struct u16MapElem{
   quint16 key;
   void *val;
   u16MapElem(quint16 _key, void *_val):key(_key),val(_val){}
};

class u16Map {
public:
    u16Map();
    ~u16Map();
    void insert(quint16 key, void *val);
    void remove(u16MapElem *pElem);
    u16MapElem* find(quint16 key);
    u16MapElem* findExact(quint16 key, void *val);
    u16MapElem* iterInit(u16MapElem *pElem);
    u16MapElem* iterNext();
protected:
    QVector<u16MapElem*> mData;
    int mIter;
};
*/
const char* QApx_parseStr(const char *begin, const char *end, char **str);
char *QApx_makeStr(const char *begin, const char *end);
//char *apx_makestr_x(const char *begin, const char *end, int startOffset, int endOffset);

#endif // APX_UTIL_H
