#include "qapx_bytearrayparser.h"
#include <QDebug>
#include <QtGlobal>
#include "qscan.h"
#include "qapx_node.h"


namespace Apx {

   ByteArrayParser::ByteArrayParser()
   {
      mApxInStream.setInputHandler(this);
   }

   /**
    * @brief parses APX Text from QByteArray
    * @param data
    * @return pointer to parsed node. The returned pointer is owned by (and must be deleted by) the caller of this method
    */
   Apx::Node *ByteArrayParser::parseNode(QByteArray &data)
   {
      Apx::Node *retval = nullptr;
      mNode = new Apx::Node();
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
      mNode=nullptr;
      return retval;
   }

   void ByteArrayParser::apx_istream_open()
   {

   }

   void ByteArrayParser::apx_istream_close()
   {

   }

   void ByteArrayParser::apx_istream_nodeNameRqst()
   {

   }

   void ByteArrayParser::apx_istream_nodeNameRsp(const QByteArray &name)
   {
      (void) name;
   }

   void ByteArrayParser::apx_istream_nodeQueryRqst(const QByteArray &name)
   {
      (void)name;
   }

   void ByteArrayParser::apx_istream_nodeQueryRspError(const QByteArray &name)
   {
      (void)name;
   }

   void ByteArrayParser::apx_istream_nodeQueryRspStart(const QByteArray &name, const QByteArray &dim)
   {
      if (mNode != nullptr)
      {
         mNode->setName(name);
      }
      (void) dim;
   }

   const QApxDataType* ByteArrayParser::getDataTypeFromDsg(const quint8 *pNext, const quint8 *pEnd)
   {
      if ((pNext < pEnd) && (pNext[0] == '['))
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

   void ByteArrayParser::apx_istream_require(const QByteArray &name, const QByteArray &dsg, const QByteArray &attr)
   {
      if (mNode != nullptr)
      {
         const quint8 *pNext = reinterpret_cast<const quint8*>(dsg.constData());
         const quint8 *pEnd = pNext + dsg.size();
         QApxSimpleRequirePort* requrePort = nullptr;
         if ( pNext < pEnd)
         {
            if (pNext[0]=='T')
            {
               const QApxDataType *dataType = getDataTypeFromDsg(&pNext[1], pEnd);
               if (dataType != nullptr)
               {
                  requrePort = new QApxSimpleRequirePort(name.constData(),dataType->getDataSignature(),attr.constData());
               }
               else
               {
                  qDebug("[PLUGIN] no datatype found for %s",name.constData());
               }
            }
            else
            {
               requrePort = new QApxSimpleRequirePort(name.constData(),dsg.constData(),attr.constData());
            }
            if (requrePort != nullptr)
            {
               mNode->appendRequirePort(requrePort);
            }
         }
      }
   }

   void ByteArrayParser::apx_istream_provide(const QByteArray &name, const QByteArray &dsg, const QByteArray &attr)
   {
      if (mNode != nullptr)
      {
         const quint8 *pNext = reinterpret_cast<const quint8*>(dsg.constData());
         const quint8 *pEnd = pNext + dsg.size();
         QApxSimpleProvidePort *providerPort = nullptr;
         if ( pNext < pEnd)
         {
            if (pNext[0]=='T')
            {
               const QApxDataType *dataType = getDataTypeFromDsg(&pNext[1], pEnd);
               if (dataType != nullptr)
               {
                  providerPort = new QApxSimpleProvidePort(name.constData(),dataType->getDataSignature(),attr.constData());
               }
               else
               {
                  qDebug("[PLUGIN] no datatype found for %s",name.constData());
               }
            }
            else
            {
               providerPort = new QApxSimpleProvidePort(name.constData(),dsg.constData(),attr.constData());
            }
         }
         if (providerPort != nullptr)
         {
            mNode->appendProvidePort(providerPort);
         }
      }
   }

   void ByteArrayParser::apx_istream_typedef(const QByteArray &name, const QByteArray &dsg, const QByteArray &attr)
   {
      (void)name;
      if (mNode != nullptr)
      {
         QApxDataType *datatype = new QApxDataType(name.constData(),dsg.constData(),attr.constData());
         mNode->appendType(datatype);
      }
   }

   void ByteArrayParser::apx_istream_requirePortConnect(quint16 portId, const quint32 dataLen)
   {
      (void)portId;
      (void)dataLen;
   }

   void ByteArrayParser::apx_istream_requirePortDisconnect(quint16 portId)
   {
      (void)portId;
   }

   void ByteArrayParser::apx_istream_providePortConnect(quint16 portId, const quint32 dataLen)
   {
      (void)portId;
      (void)dataLen;
   }

   void ByteArrayParser::apx_istream_providePortDisconnect(quint16 portId)
   {
      (void)portId;
   }

   void ByteArrayParser::apx_istream_requirePortError(quint16 portId)
   {
      (void)portId;
   }

   void ByteArrayParser::apx_istream_providePortError(quint16 portId)
   {
      (void)portId;
   }

   void ByteArrayParser::apx_istream_endTextMsg()
   {

   }

   void ByteArrayParser::apx_istream_dataRead(quint16 portId)
   {
      (void)portId;
   }

   void ByteArrayParser::apx_istream_dataWrite(quint16 portId, const QByteArray &msg)
   {
      (void)portId;
      (void)msg;
   }

   void ByteArrayParser::apx_istream_opInvoke(quint16 portId, const QByteArray &msg)
   {
      (void)portId;
      (void)msg;
   }

   void ByteArrayParser::apx_istream_opReturn(quint16 portId, const QByteArray &msg)
   {
      (void)portId;
      (void)msg;
   }



}
