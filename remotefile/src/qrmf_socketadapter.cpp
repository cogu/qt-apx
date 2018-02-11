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
   mReceiveBuffer.resize(RMF_SOCKET_ADAPTER_MIN_BUF_LEN);
#ifndef UNIT_TEST
   mReconnectTimer.setSingleShot(true);
   QObject::connect(&mReconnectTimer,SIGNAL(timeout(void)),this,SLOT(onReconnectTimeout(void)));
#endif
#ifdef UNIT_TEST
   mMockSocket = NULL;
#endif
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
   else if (mSocketType == RMF_SOCKET_TYPE_TCP)
   {
      if (mTcpSocket == NULL)
      {
         mTcpSocket = new QTcpSocket(this);
      }
      m_isAcknowledgeSeen=false;
      mTcpSocket->connectToHost(mTcpAddress, mTcpPort);
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

#ifdef UNIT_TEST
int SocketAdapter::connectMock(MockSocket *socket)
{
   if (mSocketType == RMF_SOCKET_TYPE_NONE)
   {
      mSocketType = RMF_SOCKET_TYPE_MOCK;
      mMockSocket = socket;
      m_isAcknowledgeSeen=false;
      mMockSocket->setConnectionState(true);
      return 0;
   }
   return -1;
}
#endif

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


void SocketAdapter::onConnected()
{
   m_isAcknowledgeSeen=false;
   sendGreetingHeader();
   //qDebug()<<"SocketAdapter::onConnected";
   emit connected();
}

void SocketAdapter::onDisconnected()
{
   //qDebug()<<"SocketAdapter::onDisconnected";
   mReconnectTimer.start(RMF_SOCKET_ADAPTER_RECONNECT_TIMER_MS);
   emit disconnected();
}


void SocketAdapter::onReadyread()
{   

   qint64 readAvail = getSocketReadAvail();
   while(readAvail > 0)
   {
      bool result = readHandler((quint32)readAvail);
      if (result == false)
      {
         break;
      }
      readAvail = getSocketReadAvail();
   }
   if (readAvail < 0)
   {
      qDebug() << "getSocketReadAvail failed\n";
   }
}

void SocketAdapter::onReconnectTimeout(void)
{
   qDebug() << "[RMF_SOCKET_ADAPTER] onReconnectTimeout";
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

   if(mRxPending+readLen > (size_t) mReceiveBuffer.length())
   {
      mReceiveBuffer.resize(mRxPending+readLen);
   }
   return mReceiveBuffer.data()+mRxPending;
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

qint64 SocketAdapter::getSocketReadAvail()
{
   qint64 retval;
   switch(mSocketType)
   {
   case RMF_SOCKET_TYPE_NONE:
      retval= -1;
   case RMF_SOCKET_TYPE_TCP:
      retval = mTcpSocket->bytesAvailable();
      break;
   case RMF_SOCKET_TYPE_LOCAL:
      retval = mLocalSocket->bytesAvailable();
      break;
#ifdef UNIT_TEST
   case RMF_SOCKET_TYPE_MOCK:
      retval = mMockSocket->bytesAvailable();
      break;
#endif
   default:
      retval=-1;
   }
   return retval;
}

qint64 SocketAdapter::readSocket(char *pDest, quint32 readLen)
{
   qint64 retval;
   if ((pDest == 0) || (readLen == 0) )
   {
      retval = -1;
   }
   else
   {
      switch(mSocketType)
      {
      case RMF_SOCKET_TYPE_NONE:
         retval= -1;
      case RMF_SOCKET_TYPE_TCP:
         retval = mTcpSocket->read(pDest, (qint64) readLen);
         break;
      case RMF_SOCKET_TYPE_LOCAL:
         retval = mLocalSocket->read(pDest, (qint64) readLen);
         break;
   #ifdef UNIT_TEST
      case RMF_SOCKET_TYPE_MOCK:
         retval = mMockSocket->read(pDest, (qint64) readLen);
         break;
   #endif
      default:
         retval=-1;
      }
   }
   return retval;
}

bool SocketAdapter::readHandler(quint32 readAvail)
{
   bool retval = true;
   char *pDest = prepareReceive(readAvail);
   if(pDest != 0)
   {
      qint64 result = readSocket(pDest, readAvail);

      if (result > 0)
      {
         char *pStartOfReceiveBuffer = mReceiveBuffer.data();
         const char *pBegin = pStartOfReceiveBuffer;
         const char *pEnd = pDest+result;
         const char *pNext = parseRemoteFileData(pBegin, pEnd);
         if (pNext == NULL)
         {
            qCritical() << "[RMF_SOCKET_ADAPTER] Error: Invalid data";
            retval = false;
         }
         else if (pNext > pEnd)
         {
            qCritical() << "[RMF_SOCKET_ADAPTER] Error: Parsed beyond buffer size";
            retval = false;
         }
         else
         {
            size_t unparsedLen = (size_t) (pEnd-pNext);
            if (unparsedLen > 0)
            {
               if (pNext > pBegin)
               {
                  memmove(pStartOfReceiveBuffer,pNext,unparsedLen);
               }
               mRxPending=unparsedLen;
            }
            else
            {
               mRxPending=0;
            }
         }
      }
      else
      {
         qCritical() << "readSocket failed";
         retval = false;
      }
   }
   else
   {
      qCritical() << "Unable to allocate memory for receive buffer. size:" << readAvail;
      retval=false;
   }
   return retval;
}

const char *SocketAdapter::parseRemoteFileData(const char *pBegin, const char *pEnd)
{
   if ( (pBegin == 0) || (pEnd == 0) )
   {
      return NULL;
   }
   /**
    * pBegin is moved forward until there is no more data to parse or there is an incomplete message in buffer
    * It returns pBegin which could be pEnd in case ALL data between pBegin and pEnd has been parsed.
    */
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
      }
      if(headerLen==0)
      {
         break; //partial header
      }
      else
      {
         const char *pNext = pBegin+headerLen;
         quint32 remain =(quint32) (pEnd-pNext);
         if(remain>=(quint32)msgLen)
         {
            if (m_isAcknowledgeSeen == false)
            {
               if (msgLen == RMF_CMD_TYPE_LEN+RMF_HIGH_ADDRESS_SIZE)
               {
                  quint32 address;
                  quint32 cmdType;
                  address = qFromBigEndian<quint32>((const uchar*)pNext);
                  cmdType = qFromLittleEndian<quint32>( ((const uchar*) pNext)+RMF_HIGH_ADDRESS_SIZE);
                  if ( (address == (RMF_CMD_START_ADDR | RMF_CMD_HIGH_BIT) ) && (cmdType == RMF_CMD_ACK) )
                  {
                     m_isAcknowledgeSeen=true;
                     if (mReceiveHandler != 0)
                     {
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



}
