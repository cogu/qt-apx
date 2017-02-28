#include <QByteArray>
#include <algorithm>
#include "qapxutil.h"

//bool sortByKey(u16MapElem* lhs, u16MapElem* rhs) { return lhs->key < rhs->key; }

/*
u16Map::u16Map():mIter(-1){

}

u16Map::~u16Map(){
    int len;
    int i;
    len=mData.length();
    for(i=0;i<len;i++){
        delete mData[i];
    }
}

void u16Map::insert(quint16 key, void *val){
    u16MapElem *pElem = new u16MapElem(key,val);
    mData.push_back(pElem);
    std::stable_sort(mData.begin(),mData.end(),sortByKey);
}

void u16Map::remove(u16MapElem *pElem){
    int i = mData.indexOf(pElem);
    if(i>=0){
        mData.remove(i);
    }
}

u16MapElem* u16Map::find(quint16 key){
    int len;
    int i;
    len=mData.length();
    for(i=0;i<len;i++){
        u16MapElem *pElem = (u16MapElem*) mData.at(i);
        if(pElem->key == key){
            return pElem;
        }
    }
    return 0;
}

u16MapElem* u16Map::findExact(quint16 key, void *val){
    int len;
    int i;
    len=mData.length();
    for(i=0;i<len;i++){
        u16MapElem *pElem = (u16MapElem*) mData.at(i);
        if( (pElem->key == key) && (pElem->val == val)){
            return pElem;
        }
    }
    return 0;
}

u16MapElem* u16Map::iterInit(u16MapElem *pElem){
    mIter = mData.indexOf(pElem);
    if( (mIter>=0) && (mIter<mData.length())){
        return (u16MapElem*) mData.at(mIter);
    }
    return 0;
}

u16MapElem* u16Map::iterNext(){
    if( (mIter>=0) && (mIter<mData.length())){
        mIter++;
        if( mIter<mData.length()){
            return (u16MapElem*) mData.at(mIter);
        }
    }
    return 0;
}
*/
#if 0
const char* QApx_parseStr(const char *begin, const char *end, char **str)
{
    const char *next = begin;
    if( (begin != 0) && (end != 0) && (str != 0)){
       if(next<end){
          char c = '\0';
          char strType = *next++;
          const char *mark;
          if( (strType == '"') || (strType == '\'')){
             mark = next;
             while(next<end){
                c = *next++;
                if(c == strType){
                   break;
                }
             }
             if( (c == '"') || (c == '\'')){
                *str = QApx_makeStr(mark,next-1);
                return next;
             }
          }
       }
       return begin;
    }
    return 0;
}

char *QApx_makeStr(const char *begin, const char *end)
{
    if( (begin != 0) && (end != 0) && (begin<end)){
       int len = ((int) (end-begin));
       QByteArray tmp (begin,len);
       return qstrdup(tmp.data());
    }
    return 0;
}
#endif

/*
char *apx_makestr_x(const char *begin, const char *end, int startOffset, int endOffset){
    if( (begin != 0) && (end != 0) && (begin<end)){
       int allocLen;
       int strLen = (int) (end-begin);
       allocLen = strLen+startOffset+endOffset;
       QByteArray tmp(allocLen,' ');
       int i=startOffset;
       const char *next = begin;
       while(next<end){
           tmp[i++]=*next++;
       }
       return qstrdup(tmp.data());
    }
    return 0;
}*/

