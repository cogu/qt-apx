#ifndef QAPXDATASIGNATURE_H
#define QAPXDATASIGNATURE_H

#include <QList>
#include <QByteArray>
#include <QtGlobal>

#define QAPX_BASE_TYPE_NONE     -1
#define QAPX_BASE_TYPE_UINT8     0 //'C'
#define QAPX_BASE_TYPE_UINT16    1 //'S'
#define QAPX_BASE_TYPE_UINT32    2 //'L'
#define QAPX_BASE_TYPE_UINT64    3 //'U'
#define QAPX_BASE_TYPE_SINT8     4 //'c'
#define QAPX_BASE_TYPE_SINT16    5 //'s'
#define QAPX_BASE_TYPE_SINT32    6 //'l'
#define QAPX_BASE_TYPE_SINT64    6 //'u'
#define QAPX_BASE_TYPE_STRING    7 //'a'
#define QAPX_BASE_TYPE_RECORD    8 //"{}"

class QApxRecordSignature;

struct QApxDataElement;
typedef QList<QApxDataElement*> QApxDataElementList;

struct QApxDataElement
{
   QByteArray name;
   int baseType;
   int arrayLen;
   int packLen;
   union {
      quint32 u32;
      qint32  s32;
   }min;
   union {
      quint32 u32;
      qint32  s32;
   }max;
   QApxDataElementList *pElementList; //nullptr for all cases except when baseType is exactly == QAPX_BASE_TYPE_RECORD
   QApxDataElement():name(),baseType(QAPX_BASE_TYPE_NONE),arrayLen(0),packLen(0),pElementList(nullptr){min.u32=0;max.u32=0;}
   ~QApxDataElement();
};




class QApxDataElementParser
{
public:
   QApxDataElementParser();
   QApxDataElement *parseDataSignature(const quint8 *dsg);   
protected:
   const quint8 *parseDataElement(const quint8 *pBegin, const quint8 *pEnd, QApxDataElement *pElement);
   const quint8 *parseName(const quint8 *pBegin, const quint8 *pEnd, QApxDataElement *pElement);
   const quint8 *parseType(const quint8 *pBegin, const quint8 *pEnd, QApxDataElement *pElement);
   const quint8 *parseArrayLength(const quint8 *pBegin, const quint8 *pEnd, QApxDataElement *pElement);
   const quint8 *parseLimit(const quint8 *pBegin, const quint8 *pEnd, QApxDataElement *pElement);
   void calcLen(QApxDataElement *pElement);
};

class QApxRecordSignature
{
public:
   QApxRecordSignature();
protected:
   QList<QApxDataElementParser> mElements;
};



#endif // QAPXDATASIGNATURE_H
