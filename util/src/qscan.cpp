#include "qscan.h"
#include <ctype.h>

/**
 * @brief searches for character \param val in byte array bounded by pBegin and pEnd (pEnd excluded)
 * @param pBegin lower bound
 * @param pEnd upper bound
 * @param val
 * @return NULL on failure (value not found). pBegin in case pBegin==pEnd. Otherwise it returns a pointer between pBegin and pEnd where val was found
 */
const quint8 *qscan_searchUntil(const quint8 *pBegin, const quint8 *pEnd, quint8 val){
    const quint8 *pNext = pBegin;
    if ( (pBegin != 0) && (pEnd != 0) && (pEnd>=pBegin) )
    {
       if (pBegin==pEnd)
       {
          return pBegin; // no actual data to parse
       }
       while(pNext < pEnd){
          quint8 c = *pNext;
          if(c == val){
             return pNext;
          }
          pNext++;
       }
    }
    return NULL; //invalid arguments or value not found
}

const quint8 *qscan_matchPair(const quint8 *pBegin, const quint8 *pEnd, const quint8 left, const quint8 right, const quint8 escapeChar){
    const quint8 *pNext = pBegin;
     if (pNext < pEnd){
        if (*pNext == left){
           pNext++;
           if (escapeChar != 0){
              quint8 isEscape = 0;
              while (pNext < pEnd){
                 quint8 c = *pNext++;
                 if (isEscape != 0){
                    //ignore this char
                     isEscape = 0;
                    continue;
                 }
                 else{
                    if ( c == escapeChar ){
                       isEscape = 1;
                    }
                    else  if (*pNext == right){
                       return pNext+1;
                    }
                 }
              }
           }
           else{
              while (pNext < pEnd){
                 if (*pNext++ == right){
                    return pNext+1;
                 }
              }
           }
        }
     }
     return 0;
}

/**
 * @brief compares characters in string bound by pStrBegin and pStrEnd in buffer bound by pBegin and pEnd
 * @param pBegin start of buffer
 * @param pEnd end of buffer
 * @param pStrBegin start of string to be matched
 * @param pStrEnd end of string to matched
 * @return pointer in buffer where the match stopped. If no characters was matched it will return pBegin.
 */
const quint8 *qscan_matchStr(const quint8 *pBegin, const quint8 *pEnd, const quint8 *pStrBegin, const quint8 *pStrEnd)
{
   const quint8 *pNext = pBegin;
   const quint8 *pStrNext = pStrBegin;
   while(pNext < pEnd){
      if (pStrNext < pStrEnd)
      {
         if (*pNext != *pStrNext)
         {
            return pNext;
         }
      }
      else
      {
         //all characters in str has been successfully matched
         return pNext;
      }
      pNext++;
      pStrNext++;
   }
   return pNext;
}

const quint8 *qscan_digit(const quint8 *pBegin, const quint8 *pEnd)
{
   const quint8 *pNext = pBegin;
   while (pNext < pEnd)
   {
      char c = (char) *pNext;
      if (!isdigit(c)){
         break;
      }
      pNext++;
   }
   return pNext;
}

const quint8 *qscan_toInt(const quint8 *pBegin, const quint8 *pEnd, int *data)
{
   const quint8 *pResult=qscan_digit(pBegin,pEnd);
   if (pResult > pBegin)
   {
      int radix=1;
      int len=(pResult-pBegin);
      if (len == 0)
      {
         return pBegin;
      }
      for (int i=1;i<len;i++)
      {
         radix*=10;
      }
      *data=0;
      for(int i=0;i<len;i++)
      {
         *data+=(pBegin[i]-0x30)*radix;
         radix/=10;
      }
   }
   return pResult;
}

bool qscan_startsWith(const quint8 *pBegin, const quint8 *pEnd, const char *str)
{
   const char *p=str;
   const quint8 *pNext=pBegin;
   while (*p!=0)
   {
      if (pNext<pEnd)
      {
         if (*pNext++ == (quint8) (*p++))
         {
            continue;
         }
      }
      return false;
   }
   return true;
}


QByteArray qscan_toByteArray(const quint8 *pBegin, const quint8 *pEnd)
{
   QByteArray result;
   int len = pEnd-pBegin;
   result.insert(0,(const char*)pBegin,len);
   return result;
}
