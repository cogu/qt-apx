#include <QDebug>
#include <QtGlobal> // for qWarning
#include <qendian.h>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include "qrmf_proto.h"
#include "qrmf_socketadapter.h"
#include "qnumheader.h"
#include "qscan.h"

#define RMF_SOCKET_BUFFER_SIZE_LIMIT 32 * 1024

namespace RemoteFile
{

SocketAdapter::SocketAdapter(int numHeaderBits, QObject *parent) :
   QObject(parent),mSocketType(RMF_SOCKET_TYPE_NONE),mRxPending(0),mTcpSocket(NULL),
   mLocalSocket(NULL),mReconnectTimer(parent), mReceiveHandler(NULL),m_isAcknowledgeSeen(false),
   mSendBufPtr(NULL)
{
   if (numHeaderBits==16)
   {
      mNumHeaderBits=16;
      mMaxNumHeaderLen=(int) sizeof(quint16);
   }
   else
   {
      mNumHeaderBits=32;
      mMaxNumHeaderLen=(int) sizeof(quint32);
   }
   mTotalReceived=0;   
   mReconnectTimer.setSingleShot(true);
   QObject::connect(&mReconnectTimer,SIGNAL(timeout(void)),this,SLOT(onReconnectTimeout(void)));
   mReceiveBuffer.resize(RMF_SOCKET_ADAPTER_MIN_BUF_LEN);
}

SocketAdapter::~SocketAdapter()
{
   switch(mSocketType)
   {
   case RMF_SOCKET_TYPE_NONE:
      break;
   case RMF_SOCKET_TYPE_TCP:
      if(mTcpSocket->state() != 0)
      {
         QObject::disconnect(mTcpSocket, SIGNAL(connected(void)), this, SLOT(onConnected(void)));
         QObject::disconnect(mTcpSocket, SIGNAL(disconnected(void)), this, SLOT(onDisconnected(void)));
         QObject::disconnect(mTcpSocket, SIGNAL(readyRead(void)), this, SLOT(onReadyread(void)));
      }
	  break;
   case RMF_SOCKET_TYPE_LOCAL:
      if(mLocalSocket->state() != 0)
      {
         QObject::disconnect(mLocalSocket, SIGNAL(connected(void)), this, SLOT(onConnected(void)));
         QObject::disconnect(mLocalSocket, SIGNAL(disconnected(void)), this, SLOT(onDisconnected(void)));
         QObject::disconnect(mLocalSocket, SIGNAL(readyRead(void)), this, SLOT(onReadyread(void)));
      }
      break;
   default:
      break;
   }

   mReceiveHandler=NULL;
   close();

}


int SocketAdapter::connectTcp(QHostAddress address, quint16 port)
{
   if (mSocketType == RMF_SOCKET_TYPE_NONE)
   {
      mTcpAddress = address;
      mTcpPort = port;
      mSocketType = RMF_SOCKET_TYPE_TCP;
      mTcpSocket = new QTcpSocket(this);
      QObject::connect(mTcpSocket, SIGNAL(error(QAbstractSocket::SocketError)),this, SLOT(onTcpSocketError(QAbstractSocket::SocketError)));
      QObject::connect(mTcpSocket, SIGNAL(connected(void)),this, SLOT(onConnected(void)));
      QObject::connect(mTcpSocket, SIGNAL(disconnected(void)),this, SLOT(onDisconnected(void)));
      QObject::connect(mTcpSocket, SIGNAL(readyRead(void)),this, SLOT(onReadyread(void)));
      m_isAcknowledgeSeen=false;
      mTcpSocket->connectToHost(mTcpAddress, mTcpPort);
      return 0;
   }
   return -1;
}

int SocketAdapter::connectLocal(const char *filename)
{
   if (mSocketType == RMF_SOCKET_TYPE_NONE)
   {
      mSocketType = RMF_SOCKET_TYPE_LOCAL;
      mLocalSocketName=QString(filename);
      mLocalSocket = new QLocalSocket(this);
      mLocalSocket->setReadBufferSize(RMF_SOCKET_BUFFER_SIZE_LIMIT);
      QObject::connect(mLocalSocket, SIGNAL(error(QLocalSocket::LocalSocketError)),this, SLOT(onLocalSocketError(QLocalSocket::LocalSocketError)));
      QObject::connect(mLocalSocket, SIGNAL(connected(void)),this, SLOT(onConnected(void)));
      QObject::connect(mLocalSocket, SIGNAL(disconnected(void)),this, SLOT(onDisconnected(void)));
      QObject::connect(mLocalSocket, SIGNAL(readyRead(void)),this, SLOT(onReadyread(void)));
      m_isAcknowledgeSeen=false;
      mLocalSocket->connectToServer(mLocalSocketName);
      return 0;
   }
   return -1;
}

void SocketAdapter::close()
{
   switch(mSocketType)
   {
   case RMF_SOCKET_TYPE_NONE:
      break;
   case RMF_SOCKET_TYPE_TCP:
      mTcpSocket->close();
      break;
   case RMF_SOCKET_TYPE_LOCAL:
      mLocalSocket->close();
      break;
   default:
      break;
   }
}

int SocketAdapter::getSendAvail()
{
   return RMF_SOCKET_ADAPTER_MAX_BUF_LEN;
}

char *SocketAdapter::getSendBuffer(int msgLen)
{
   if (msgLen>RMF_SOCKET_ADAPTER_MAX_BUF_LEN)
   {
      return NULL; //never accept too large size request
   }
   if (msgLen>mSendBuffer.length())
   {
      mSendBuffer.resize(msgLen+mMaxNumHeaderLen);
   }
   mSendBufPtr = mSendBuffer.data()+mMaxNumHeaderLen;
   return mSendBufPtr; //always reserve 2 or 4 bytes before the buffer given to requester. This will give us space for writing numHeader in send function
}

int SocketAdapter::send(int offset, int msgLen)
{
   if (mSendBufPtr != NULL) //have getSendBuffer been called?
   {
      char *pDest;
      int headerLen;

      qint64 totalSize;
      if ( (offset<0) || (mMaxNumHeaderLen+offset+msgLen>mSendBuffer.length()) )
      {
         return -1; //invalid arguments
      }
      pDest=mSendBufPtr+offset;
      if(msgLen<=NUMHEADER_SHORT_MSG_LIMIT)
      {
         headerLen=(int) sizeof(quint8);
      }
      else
      {
         headerLen=mMaxNumHeaderLen;
      }
      pDest-=headerLen;
      if(mNumHeaderBits == 16)
      {
         int result = NumHeader::encode16(pDest, headerLen, (quint16) msgLen);
         Q_ASSERT(result == headerLen);
      }
      else
      {
         int result = NumHeader::encode32(pDest, headerLen, (quint32) msgLen);
         Q_ASSERT(result == headerLen);
      }
      totalSize=headerLen+msgLen;
      switch(mSocketType)
      {
      case RMF_SOCKET_TYPE_NONE:
         break;
      case RMF_SOCKET_TYPE_TCP:
         mTcpSocket->write((const char*) pDest, totalSize);
         break;
      case RMF_SOCKET_TYPE_LOCAL:
         mLocalSocket->write((const char*) pDest, totalSize);
         break;
      default:
         break;
      }
      mSendBufPtr=NULL; //set mSendBufPtr back to NULL. The client must make another call to getSendBuffer before making another send
   }
   return 0;
}

const char *SocketAdapter::parseData(const char *pBegin, const char *pEnd)
{   
   /**
    * pBegin is moved forward until there is no more data to parse or there is an incomplete message in buffer
    * It returns pBegin which could be pEnd in case ALL data between pBegin and pEnd has been parsed.
    */

   const char *pNext;
   while(pBegin<pEnd)
   {
      pNext = parseRemoteFileData(pBegin, pEnd);
      if (pNext==NULL)
      {
         qDebug() << "[RMF_SOCKET_ADAPTER] parseRemoteFileData parse failure";
      }
      else if (pNext==pBegin)
      {
         break; //not enough data in buffer to parse next message
      }
      else if (pNext>pBegin)
      {
         pBegin=pNext;
      }
      else
      {
         Q_ASSERT(0); //this must never happen
      }
   }
   return pBegin;
}

void SocketAdapter::onConnected()
{
   m_isAcknowledgeSeen=false;
   sendGreetingHeader();
   //qDebug()<<"SocketAdapter::onConnected";
}

void SocketAdapter::onDisconnected()
{
   //qDebug()<<"SocketAdapter::onDisconnected";
   mReconnectTimer.start(RMF_SOCKET_ADAPTER_RECONNECT_TIMER_MS);
}

void SocketAdapter::onReadyread()
{
   quint32 readLen;
   switch(mSocketType)
   {
   case RMF_SOCKET_TYPE_NONE:
      return;
   case RMF_SOCKET_TYPE_TCP:
      readLen = mTcpSocket->bytesAvailable();
      break;
   case RMF_SOCKET_TYPE_LOCAL:
      readLen = mLocalSocket->bytesAvailable();
      break;
   default:
      return;
   }

   if(readLen>0)
   {      
      char *pDest = prepareReceive(readLen);
      if(pDest != 0)
      {
         const char *pBegin;
         const char *pNext = NULL;
         const char *pEnd;
         qint64 result;

         if(mSocketType == RMF_SOCKET_TYPE_TCP)
         {
            result = mTcpSocket->read(pDest,readLen);
         }
         else
         {
            result = mLocalSocket->read(pDest,readLen);
         }
         if (result > 0)
         {
            mRxPending+=(int)result;
            pBegin = mReceiveBuffer.constData();
            pEnd = pBegin+mRxPending;
            while(pBegin<pEnd)
            {
               pNext = parseData(pBegin,pEnd);
               if(pNext > pBegin)
               {
                  pBegin=pNext;
               }
               else if (pNext == NULL)
               {
                  qDebug("[RMF_SOCKET_ADAPTER] parse error detected");
                  break;
               }
               else
               {
                  break;
               }
            }
            if( (pNext!= NULL) && (pNext<pEnd) )
            {
               //unparsed bytes in array, move to start of array before next reception
               mRxPending=(int) (pEnd-pNext);
               memmove(mReceiveBuffer.data(),pNext,mRxPending);
            }
            else{
               //all bytes parsed or parse error
               mRxPending=0;
            }
         }
      }
   }
   else
   {
      qDebug() << "[RMF_SOCKET_ADAPTER] socket read failed";
   }
}

void SocketAdapter::onReconnectTimeout(void)
{
   switch(mSocketType)
   {
   case RMF_SOCKET_TYPE_NONE:
      return;
   case RMF_SOCKET_TYPE_TCP:
      this->connectTcp(mTcpAddress, mTcpPort);
      break;
   case RMF_SOCKET_TYPE_LOCAL:
      this->connectLocal(mLocalSocketName.toLatin1().constData());
      break;
   default:
      return;
   }
}


void SocketAdapter::onTcpSocketError(QAbstractSocket::SocketError error)
{
   qDebug("[RMF_SOCKET_ADAPTER] TcpSocketError %d",error);
   mReconnectTimer.start(RMF_SOCKET_ADAPTER_RECONNECT_TIMER_MS);
}

void SocketAdapter::onLocalSocketError(QLocalSocket::LocalSocketError error)
{
   qDebug("[RMF_SOCKET_ADAPTER] LocalSocketError %d",error);
   mReconnectTimer.start(RMF_SOCKET_ADAPTER_RECONNECT_TIMER_MS);
}   

char *SocketAdapter::prepareReceive(quint32 readLen)
{

   if(mRxPending+(int)readLen > mReceiveBuffer.length())
   {
      mReceiveBuffer.resize(mRxPending+readLen);
   }
   return mReceiveBuffer.data()+mRxPending;
}

const char *SocketAdapter::parseRemoteFileData(const char *pBegin, const char *pEnd)
{
   /**
    * pBegin is moved forward until there is no more data to parse or there is an incomplete message in buffer
    * It returns pBegin which could be pEnd in case ALL data between pBegin and pEnd has been parsed.
    */
   const char *pNext;
   while(pBegin<pEnd)
   {
      int headerLen;
      int msgLen;
      if (mNumHeaderBits==16)
      {
         quint16 tmp;
         headerLen = NumHeader::decode16(pBegin,pEnd,&tmp);
         if (headerLen>0)
         {
            msgLen=(int) tmp;
         }
      }
      else
      {
         quint32 tmp;
         headerLen = NumHeader::decode32(pBegin,pEnd,&tmp);
         if (headerLen>0)
         {
            msgLen=(int) tmp;
         }
         else
         {
            return NULL; //parse failure
         }
      }
      if (headerLen<0)
      {
         return NULL; //parse failure
      }
      else if(headerLen==0)
      {
         break; //partial header
      }
      else
      {
         pNext=pBegin+headerLen;
         quint32 remain =(quint32) (pEnd-pNext);
         if(remain>=(quint32)msgLen)
         {
            qDebug() << "here";
            if (m_isAcknowledgeSeen == false)
            {
               if (msgLen == RMF_CMD_TYPE_LEN+RMF_HIGH_ADDRESS_SIZE)
               {
                  quint32 address;
                  quint32 cmdType;
                  address = qFromBigEndian<quint32>(pNext);
                  cmdType = qFromLittleEndian<quint32>(pNext+RMF_HIGH_ADDRESS_SIZE);
                  qDebug() << "here 2";
                  if ( (address == (RMF_CMD_START_ADDR | RMF_CMD_HIGH_BIT) ) && (cmdType == RMF_CMD_ACK) )
                  {
                     m_isAcknowledgeSeen=true;
                     if (mReceiveHandler != 0)
                     {
                        qDebug() << "here 3";
                        mReceiveHandler->onConnected(this);
                     }
                  }
               }
            }
            else
            {
               if (mReceiveHandler != 0)
               {
                  mReceiveHandler->onMsgReceived(pNext, msgLen);
               }
            }
            pNext+=msgLen;
            pBegin=pNext; //move pBegin forward to mark the message as consumed
         }
         else
         {
            break;
         }
      }
   }
   return pBegin;
}


/**
 * @brief sends the RemoteFile greeting header
 */
void SocketAdapter::sendGreetingHeader()
{
   QByteArray greeting;   
   greeting.append(RMF_GREETING_START);
   greeting.append(RMF_NUMHEADER_FORMAT);
   greeting.append("32\n"); //use NumHeader32 format
   greeting.append("\n"); //emtpy line at end to terminate header
   if (mSocketType == RMF_SOCKET_TYPE_TCP)
   {
      //prepend greeting with NumHeader32. In most cases the header is in short form, prepending only a single byte.
      char numHeader[sizeof(quint32)];
      int headerLen=NumHeader::encode32(&numHeader[0],sizeof(numHeader),greeting.length());
      greeting.prepend(&numHeader[0],headerLen);
      mTcpSocket->write(greeting);
   }
}

}
