#include "qapxbytearrayparser.h"
#include <QDebug>
#include <QtGlobal>
#include "qscan.h"
#include "qapxsimplenode.h"


QApxByteArrayParser::QApxByteArrayParser()
{
   mApxInStream.setInputHandler(this);
}

/**
 * @brief parses APX Text from QByteArray
 * @param data
 * @return pointer to parsed node. The returned pointer is owned by (and must be deleted by) the caller of this method
 */
QApxSimpleNode *QApxByteArrayParser::parseNode(QByteArray &data)
{
   QApxSimpleNode *retval = NULL;
   mNode = new QApxSimpleNode();
   mApxInStream.open();
   mApxInStream.write(data);
   mApxInStream.close();
   int exception = mApxInStream.getException();
   if (exception == APX_EXCEPTION_NO_EXCEPTION)
   {
      retval = mNode;
   }
   else
   {
      delete mNode;
   }
   mNode=NULL;
   return retval;
}

void QApxByteArrayParser::apx_istream_open()
{

}

void QApxByteArrayParser::apx_istream_close()
{

}

void QApxByteArrayParser::apx_istream_nodeNameRqst()
{

}

void QApxByteArrayParser::apx_istream_nodeNameRsp(const QByteArray &name)
{
   (void) name;
}

void QApxByteArrayParser::apx_istream_nodeQueryRqst(const QByteArray &name)
{
   (void)name;
}

void QApxByteArrayParser::apx_istream_nodeQueryRspError(const QByteArray &name)
{
   (void)name;
}

void QApxByteArrayParser::apx_istream_nodeQueryRspStart(const QByteArray &name, const QByteArray &dim)
{
   if (mNode != NULL)
   {
      mNode->setName(name);
   }
   (void) dim;
}

void QApxByteArrayParser::apx_istream_require(const QByteArray &name, const QByteArray &dsg, const QByteArray &attr)
{
   if (mNode != NULL)
   {
      const quint8 *pBegin = (const quint8*) dsg.constData();
      const quint8 *pEnd = pBegin + dsg.size();
      const quint8 *pNext=pBegin;
      const quint8 *pMark;
      if ( pNext < pEnd)
      {
         if (pNext[0]=='T')
         {
            if (pNext[1] == '[')
            {
               pMark = pNext+1;
               pNext=qscan_matchPair(pMark,pEnd,'[',']','\\');
               if (pNext > pMark)
               {
                  int value;
                  if (qscan_toInt(pMark+1,pNext,&value) == 0)
                  {
                     qDebug("[PLUGIN] qscan_toInt failed");
                     return;
                  }
                  //qDebug("[PLUGIN] %s: %d",name.constData(),value);
                  QApxDataType *dataType = mNode->getTypeById(value);
                  if (dataType != NULL)
                  {
                     QApxSimpleRequirePort *port = new QApxSimpleRequirePort(name.constData(),dataType->getDataSignature(),attr.constData());
                     mNode->appendRequirePort(port);
                  }
                  else
                  {
                     qDebug("[PLUGIN] no datatype found for %s",name.constData());
                  }
               }
            }
         }
         else
         {
            QApxSimpleRequirePort *port = new QApxSimpleRequirePort(name.constData(),dsg.constData(),attr.constData());
            mNode->appendRequirePort(port);
         }
      }
   }
}

void QApxByteArrayParser::apx_istream_provide(const QByteArray &name, const QByteArray &dsg, const QByteArray &attr)
{
   if (mNode != NULL)
   {
      const quint8 *pBegin = (const quint8*) dsg.constData();
      const quint8 *pEnd = pBegin + dsg.size();
      const quint8 *pNext=pBegin;
      const quint8 *pMark;
      if ( pNext < pEnd)
      {
         if (pNext[0]=='T')
         {
            if (pNext[1] == '[')
            {
               pMark = pNext+1;
               pNext=qscan_matchPair(pMark,pEnd,'[',']','\\');
               if (pNext > pMark)
               {
                  int value;
                  if (qscan_toInt(pMark+1,pNext,&value) == 0)
                  {
                     qDebug("[PLUGIN] qscan_toInt failed");
                     return;
                  }
                  QApxDataType *dataType = mNode->getTypeById(value);
                  if (dataType != NULL)
                  {
                     QApxSimpleProvidePort *port = new QApxSimpleProvidePort(name.constData(),dataType->getDataSignature(),attr.constData());
                     mNode->appendProvidePort(port);
                  }
                  else
                  {
                     qDebug("[PLUGIN] no datatype found for %s",name.constData());
                  }
               }
            }
         }
         else
         {
            QApxSimpleProvidePort *port = new QApxSimpleProvidePort(name.constData(),dsg.constData(),attr.constData());
            mNode->appendProvidePort(port);
         }
      }
   }
}

void QApxByteArrayParser::apx_istream_typedef(const QByteArray &name, const QByteArray &dsg, const QByteArray &attr)
{
   (void)name;
   if (mNode != NULL)
   {
      QApxDataType *datatype = new QApxDataType(name.constData(),dsg.constData(),attr.constData());
      mNode->appendType(datatype);
   }
}

void QApxByteArrayParser::apx_istream_requirePortConnect(quint16 portId, const quint32 dataLen)
{
   (void)portId;
   (void)dataLen;
}

void QApxByteArrayParser::apx_istream_requirePortDisconnect(quint16 portId)
{
   (void)portId;
}

void QApxByteArrayParser::apx_istream_providePortConnect(quint16 portId, const quint32 dataLen)
{
   (void)portId;
   (void)dataLen;
}

void QApxByteArrayParser::apx_istream_providePortDisconnect(quint16 portId)
{
   (void)portId;
}

void QApxByteArrayParser::apx_istream_requirePortError(quint16 portId)
{
   (void)portId;
}

void QApxByteArrayParser::apx_istream_providePortError(quint16 portId)
{
   (void)portId;
}

void QApxByteArrayParser::apx_istream_endTextMsg()
{

}

void QApxByteArrayParser::apx_istream_dataRead(quint16 portId)
{
   (void)portId;
}

void QApxByteArrayParser::apx_istream_dataWrite(quint16 portId, const QByteArray &msg)
{
   (void)portId;
   (void)msg;
}

void QApxByteArrayParser::apx_istream_opInvoke(quint16 portId, const QByteArray &msg)
{
   (void)portId;
   (void)msg;
}

void QApxByteArrayParser::apx_istream_opReturn(quint16 portId, const QByteArray &msg)
{
   (void)portId;
   (void)msg;
}

