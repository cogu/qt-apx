#ifndef QAPXBYTEARRAYPARSER_H
#define QAPXBYTEARRAYPARSER_H

#include "qapx_stream.h"

namespace Apx
{
   class Node;

   class ByteArrayParser : public QApxIStreamEventHandler
   {
   public:
      ByteArrayParser();
      Apx::Node *parseNode (QByteArray &data);

      //QApxIstreamEventHandler API
      void apx_istream_open();
      void apx_istream_close();
      void apx_istream_nodeNameRqst();
      void apx_istream_nodeNameRsp(const QByteArray &name);
      void apx_istream_nodeQueryRqst(const QByteArray &name);
      void apx_istream_nodeQueryRspError(const QByteArray &name);
      void apx_istream_nodeQueryRspStart(const QByteArray &name,const QByteArray &dim);
      void apx_istream_require(const QByteArray &name,const QByteArray &dsg, const QByteArray &attr);
      void apx_istream_provide(const QByteArray &name,const QByteArray &dsg, const QByteArray &attr);
      void apx_istream_typedef(const QByteArray &name,const QByteArray &dsg, const QByteArray &attr);
      void apx_istream_requirePortConnect(quint16 portId,const quint32 dataLen);
      void apx_istream_requirePortDisconnect(quint16 portId);
      void apx_istream_providePortConnect(quint16 portId,const quint32 dataLen);
      void apx_istream_providePortDisconnect(quint16 portId);
      void apx_istream_requirePortError(quint16 portId);
      void apx_istream_providePortError(quint16 portId);
      void apx_istream_endTextMsg();
      void apx_istream_dataRead(quint16 portId);
      void apx_istream_dataWrite(quint16 portId,const QByteArray &msg);
      void apx_istream_opInvoke(quint16 portId,const QByteArray &msg);
      void apx_istream_opReturn(quint16 portId,const QByteArray &msg);

   protected:
      QApxIStreamBuf mApxInStream;
      Apx::Node *mNode;

   };
}





#endif // QAPXBYTEARRAYPARSER_H
