#include "qapxdataelement.h"
#include <cstring>
#include "qapxutil.h"
#include "qscan.h"

using namespace std;

QApxDataElement::~QApxDataElement()
{
   if (pElementList != NULL)
   {
      int len = pElementList->length();
      for (int i=0;i<len;i++)
      {
         delete pElementList->at(i);
      }
      delete pElementList;
   }
}


QApxDataElementParser::QApxDataElementParser()
{

}

QApxDataElement *QApxDataElementParser::parseDataSignature(const quint8 *dsg)
{
   const quint8 *pNext=dsg;
   const quint8 *pEnd=pNext+strlen((const char*)dsg);
   const quint8 *pMark;
   QApxDataElement *pElement = new QApxDataElement;
   char c = (char) *pNext;

   if (c =='{')
   {
      const quint8 *pRecordBegin = pNext;
      const quint8 *pRecordEnd=qscan_matchPair(pRecordBegin,pEnd,'{','}','\\');
      if (pRecordEnd > pRecordBegin)
      {
         pRecordEnd--; //pRecordEnd points to the character after '}'. make it point exactly at '}'
         pElement->baseType = QAPX_BASE_TYPE_RECORD;
         pElement->pElementList = new QApxDataElementList;
         pNext = pRecordBegin+1;
         while (pNext<pRecordEnd)
         {
            pMark=pNext;
            QApxDataElement *pChildElement = new QApxDataElement;
            pNext = parseDataElement(pNext,pRecordEnd,pChildElement);
            if (pNext > pMark)
            {
               pElement->pElementList->append(pChildElement);
            }
            else
            {
               qDebug("[PLUGIN] record parse failure");
               delete pChildElement;
               delete pElement;
               return 0;
            }
         }
         calcLen(pElement);
      }
   }
   else
   {
      pMark=pNext;
      pNext = parseDataElement(pNext,pEnd,pElement);
      if (pNext <= pMark)
      {
         qDebug("[PLUGIN] element parse failure");
      }
   }
   return pElement;
}

const quint8 *QApxDataElementParser::parseDataElement(const quint8 *pBegin, const quint8 *pEnd, QApxDataElement *pElement)
{
   const quint8 *pNext=pBegin;

   pNext = parseName(pNext,pEnd,pElement);
   if (pNext == 0)
   {
      return NULL;
   }
   pNext = parseType(pNext,pEnd,pElement);
   if (pNext == 0)
   {
      return NULL;
   }
   pNext = parseArrayLength(pNext,pEnd,pElement);
   if (pNext == 0)
   {
      return NULL;
   }
   pNext = parseLimit(pNext,pEnd,pElement);
   if (pNext == 0)
   {
      return NULL;
   }
   calcLen(pElement);
   return pNext;
}

const quint8 *QApxDataElementParser::parseName(const quint8 *pBegin, const quint8 *pEnd, QApxDataElement *pElement)
{
   const quint8 *pNext=pBegin;
   if (pNext < pEnd)
   {
      char c = (char) *pNext;
      if(c == '\"')
      {
         const quint8 *pMark=pNext;
         pNext=qscan_matchPair(pMark,pEnd,'"','"','\\');
         if (pNext>pMark)
         {
            int len=(int) (pNext-pMark-2); //pNext points one character beyond the closing '"' character
            pElement->name.append((char *)pMark+1,len);
         }
         else
         {
            qDebug("[PLUGIN] failed to parse string");
            return 0;
         }
      }
   }
   return pNext;
}

const quint8 *QApxDataElementParser::parseType(const quint8 *pBegin, const quint8 *pEnd, QApxDataElement *pElement)
{
   (void) pEnd;
   const quint8 *pNext=pBegin;
   char c = (char) *pNext++;
   //check for name (name is optional)
   switch(c)
   {
   case 'C':
      pElement->baseType=QAPX_BASE_TYPE_UINT8;
      break;
   case 'S':
      pElement->baseType=QAPX_BASE_TYPE_UINT16;
      break;
   case 'L':
      pElement->baseType=QAPX_BASE_TYPE_UINT32;
      break;
   case 'a':
      pElement->baseType=QAPX_BASE_TYPE_STRING;
      break;
   case 'c':
      pElement->baseType=QAPX_BASE_TYPE_SINT8;
      break;
   case 's':
      pElement->baseType=QAPX_BASE_TYPE_SINT16;
      break;
   case 'l':
      pElement->baseType=QAPX_BASE_TYPE_SINT32;
      break;
   default:
      return NULL;
   }
   return pNext;
}

const quint8 *QApxDataElementParser::parseArrayLength(const quint8 *pBegin, const quint8 *pEnd, QApxDataElement *pElement)
{
   const quint8 *pNext=pBegin;
   if (pNext < pEnd)
   {
      char c = (char) *pNext;
      if(c == '[')
      {
         const quint8 *pMark=pNext;
         pNext=qscan_matchPair(pMark,pEnd,'[',']','\\');
         if (pNext>pMark)
         {
            if (qscan_toInt(pMark+1,pNext,&pElement->arrayLen) == 0)
            {
               qDebug("[PLUGIN] qscan_toInt failed");
               return NULL;
            }
         }
         else
         {
            qDebug("[PLUGIN] failed to parse array length");
            return NULL;
         }
      }
   }
   return pNext;
}

const quint8 *QApxDataElementParser::parseLimit(const quint8 *pBegin, const quint8 *pEnd, QApxDataElement *pElement)
{
   (void) pElement;
   const quint8 *pNext=pBegin;
   if (pNext < pEnd)
   {
      char c = (char) *pNext;
      if(c == '(')
      {
         const quint8 *pMark=pNext;
         pNext=qscan_matchPair(pMark,pEnd,'(',')','\\');
         if (pNext>pMark)
         {
            //TODO: implement limit parsing
         }
      }
   }
   return pNext;
}

void QApxDataElementParser::calcLen(QApxDataElement *pElement)
{
   if (pElement->baseType == QAPX_BASE_TYPE_RECORD)
   {
      int packLen=0;
      int end=pElement->pElementList->length();
      for(int i=0;i<end;i++)
      {
         const QApxDataElement *childElement = pElement->pElementList->at(i);
         packLen+=childElement->packLen;
      }
      pElement->packLen = packLen;
   }
   else
   {
      int elemLen=0;
      switch(pElement->baseType)
      {
      case QAPX_BASE_TYPE_NONE:
         break;
      case QAPX_BASE_TYPE_UINT8:
         elemLen=1;
         break;
      case QAPX_BASE_TYPE_UINT16:
         elemLen=2;
         break;
      case QAPX_BASE_TYPE_UINT32:
         elemLen=4;
         break;
      case QAPX_BASE_TYPE_SINT8:
         elemLen=1;
         break;
      case QAPX_BASE_TYPE_SINT16:
         elemLen=2;
         break;
      case QAPX_BASE_TYPE_SINT32:
         elemLen=4;
         break;
      case QAPX_BASE_TYPE_STRING:
         elemLen=1;
         break;
      default:
         break;
      }
      if (elemLen > 0)
      {
         if (pElement->arrayLen > 0)
         {
            pElement->packLen=elemLen*pElement->arrayLen;
         }
         else
         {
            pElement->packLen=elemLen;
         }
      }
   }
}
