#include "qapxfileparser.h"
#include <QFile>
#include <QDebug>
#include <QTextStream>
#include <QtGlobal>
#include "qscan.h"
#include "qapx_node.h"

QApxFileParser::QApxFileParser():mApxInStream(),mNode(nullptr)
{
   mApxInStream.setInputHandler(this);
}

QApxFileParser::~QApxFileParser()
{
}

Apx::Node* QApxFileParser::parseNode(const char *filepath)
{
   mNode = new Apx::Node();
   QFile file(filepath);
   if(!file.open(QIODevice::ReadOnly)) {
       qDebug("[PLUGIN] failed to open file '%s'",filepath);
   }
   else
   {
      QTextStream in(&file);
      mApxInStream.open();
      QString line;
      while(in.readLineInto(&line))
      {
         mApxInStream.write(line.toLatin1());
      }
      mApxInStream.close();
   }
   Apx::Node *retval = mNode;
   mNode = nullptr;
   return retval;
}

void QApxFileParser::apx_istream_open()
{

}

void QApxFileParser::apx_istream_close()
{

}

void QApxFileParser::apx_istream_nodeNameRqst()
{

}

void QApxFileParser::apx_istream_nodeNameRsp(const QByteArray &name)
{
   (void)name;
}

void QApxFileParser::apx_istream_nodeQueryRqst(const QByteArray &name)
{
   (void)name;
}

void QApxFileParser::apx_istream_nodeQueryRspError(const QByteArray &name)
{
   (void)name;
}

void QApxFileParser::apx_istream_nodeQueryRspStart(const QByteArray &name, const QByteArray &dim)
{
   (void)name;
   (void) dim;
}

const QApxDataType* QApxFileParser::getDataTypeFromDsg(const quint8 *pNext, const quint8 *pEnd)
{
   if (pNext[0] == '[')
   {
      const quint8 *pMark = pNext;
      pNext=qscan_matchPair(pMark,pEnd,'[',']','\\');
      if (pNext > pMark)
      {
         int value;
         if (qscan_toInt(pMark+1,pNext,&value) == nullptr)
         {
            qDebug("[PLUGIN] qscan_toInt failed to get type id");
            return nullptr;
         }
         return mNode->getTypeById(value);
      }
   }
   return nullptr;
}

void QApxFileParser::apx_istream_require(const QByteArray &name, const QByteArray &dsg, const QByteArray &attr)
{
   if (mNode != nullptr)
   {
      const quint8 *pNext = reinterpret_cast<const quint8*>(dsg.constData());
      const quint8 *pEnd = pNext + dsg.size();

      if ( (pNext + 1) < pEnd)
      {
         if (pNext[0]=='T')
         {
            const QApxDataType* dataType = getDataTypeFromDsg(&pNext[1], pEnd);
            if (dataType != nullptr)
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
}

void QApxFileParser::apx_istream_provide(const QByteArray &name, const QByteArray &dsg, const QByteArray &attr)
{
   if (mNode != nullptr)
   {
      const quint8 *pNext = reinterpret_cast<const quint8*>(dsg.constData());
      const quint8 *pEnd = pNext + dsg.size();
      if ( (pNext + 1) < pEnd)
      {
         if (pNext[0]=='T')
         {
            const QApxDataType* dataType = getDataTypeFromDsg(&pNext[1], pEnd);
            if (dataType != nullptr)
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
}

void QApxFileParser::apx_istream_typedef(const QByteArray &name, const QByteArray &dsg, const QByteArray &attr)
{
   (void)name;
   if (mNode != nullptr)
   {
      QApxDataType *datatype = new QApxDataType(name.constData(),dsg.constData(),attr.constData());
      mNode->appendType(datatype);
   }
}

void QApxFileParser::apx_istream_requirePortConnect(quint16 portId, const quint32 dataLen)
{
   (void)portId;
   (void)dataLen;
}

void QApxFileParser::apx_istream_requirePortDisconnect(quint16 portId)
{
   (void)portId;
}

void QApxFileParser::apx_istream_providePortConnect(quint16 portId, const quint32 dataLen)
{
   (void)portId;
   (void)dataLen;
}

void QApxFileParser::apx_istream_providePortDisconnect(quint16 portId)
{
   (void)portId;
}

void QApxFileParser::apx_istream_requirePortError(quint16 portId)
{
   (void)portId;
}

void QApxFileParser::apx_istream_providePortError(quint16 portId)
{
   (void)portId;
}

void QApxFileParser::apx_istream_endTextMsg()
{

}

void QApxFileParser::apx_istream_dataRead(quint16 portId)
{
   (void)portId;
}

void QApxFileParser::apx_istream_dataWrite(quint16 portId, const QByteArray &msg)
{
   (void)portId;
   (void)msg;
}

void QApxFileParser::apx_istream_opInvoke(quint16 portId, const QByteArray &msg)
{   
   (void)portId;
   (void)msg;
}

void QApxFileParser::apx_istream_opReturn(quint16 portId, const QByteArray &msg)
{
   (void)portId;
   (void)msg;
}

