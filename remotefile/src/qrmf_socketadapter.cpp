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
// Define to get qDebug socket status
//#define RMF_SOCKET_VERBOSE

namespace RemoteFile
{

SocketAdapter::SocketAdapter(int numHeaderBits, QObject *parent) :
   QObject(parent),mSocketType(RMF_SOCKET_TYPE_NONE),mRxPending(0),mTcpSocket(NULL),
   mLocalSocket(NULL),mReconnectTimer(parent), mReceiveHandler(NULL),m_isAcknowledgeSeen(false),
   mSendBufPtr(NULL),mErrorCode(RMF_ERR_NONE),mLastSocketError(QAbstractSocket::UnknownSocketError)
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
      if(mTcpSocket->state() != QAbstractSocket::UnconnectedState)
      {
         QObject::disconnect(mTcpSocket, SIGNAL(connected(void)), this, SLOT(onConnected(void)));
         QObject::disconnect(mTcpSocket, SIGNAL(disconnected(void)), this, SLOT(onDisconnected(void)));
         QObject::disconnect(mTcpSocket, SIGNAL(readyRead(void)), this, SLOT(onReadyread(void)));
      }
      break;
   case RMF_SOCKET_TYPE_LOCAL:
      if(mLocalSocket->state() != QLocalSocket::UnconnectedState)
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


int SocketAdapter::connectTcp(const QHostAddress& address, quint16 port)
{
   if (mSocketType == RMF_SOCKET_TYPE_NONE)
   {
      mTcpAddress = address;
      mTcpPort = port;
      mSocketType = RMF_SOCKET_TYPE_TCP;
      mTcpSocket = new QTcpSocket(this);
      mLastSocketError=QAbstractSocket::UnknownSocketError;
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
      mLastSocketError=QAbstractSocket::UnknownSocketError;
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
      mLastSocketError=QAbstractSocket::UnknownSocketError;
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
   int ret_val = 0;
   if (mSendBufPtr == NULL) //has getSendBuffer not been called?
   {
      ret_val = -2; // Sequence error
   }
   else
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
      totalSize=headerLen+msgLen;
      if(mNumHeaderBits == 16)
      {
         int result = NumHeader::encode16(pDest, headerLen, (quint16) msgLen);
         Q_ASSERT(result == headerLen); (void)result;
      }
      else
      {
         int result = NumHeader::encode32(pDest, headerLen, (quint32) msgLen);
         Q_ASSERT(result == headerLen); (void)result;
      }
      switch(mSocketType)
      {
      case RMF_SOCKET_TYPE_NONE:
         break;
      case RMF_SOCKET_TYPE_TCP:
         if (totalSize != mTcpSocket->write((const char*) pDest, totalSize))
         {
            ret_val = -3; // Socket did not accept the full write
         }
         break;
      case RMF_SOCKET_TYPE_LOCAL:
         if (totalSize != mLocalSocket->write((const char*) pDest, totalSize))
         {
            ret_val = -3; // Socket did not accept the full write
         }
         break;
      default:
         break;
      }
      mSendBufPtr=NULL; //set mSendBufPtr back to NULL. The client must make another call to getSendBuffer before making another send
   }
   return ret_val;
}


void SocketAdapter::onConnected()
{
   mErrorCode = RMF_ERR_NONE;
   m_isAcknowledgeSeen=false;
   sendGreetingHeader();
#ifdef RMF_SOCKET_VERBOSE
   qDebug()<<"[RMF_SOCKET_ADAPTER] onConnected";
#endif
   emit connected();
}

void SocketAdapter::onDisconnected()
{
#ifdef RMF_SOCKET_VERBOSE
   qDebug()<<"[RMF_SOCKET_ADAPTER] onDisconnected";
#endif
   mReconnectTimer.start(RMF_SOCKET_ADAPTER_RECONNECT_TIMER_MS);
   if (mReceiveHandler != 0)
   {
      mReceiveHandler->onDisconnected();
   }
   emit disconnected();
}


void SocketAdapter::onReadyread()
{
   qint64 readAvail = getSocketReadAvail();
   while( (readAvail > 0) && (mErrorCode == RMF_ERR_NONE))
   {
      readHandler((quint32)readAvail);
      readAvail = getSocketReadAvail();
   }
   if ( (mErrorCode==RMF_ERR_NONE) && (readAvail < 0) )
   {
      qDebug() << "getSocketReadAvail failed\n";
      setError(RMF_ERR_SOCKET_READ_AVAIL_FAIL);
   }
   if (mErrorCode != RMF_ERR_NONE)
   {
      mReconnectTimer.start(RMF_SOCKET_ADAPTER_RECONNECT_TIMER_MS);
      close();
   }
}

void SocketAdapter::onReconnectTimeout(void)
{
#ifdef RMF_SOCKET_VERBOSE
   qDebug() << "[RMF_SOCKET_ADAPTER] onReconnectTimeout";
#endif
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
   setError(RMF_ERR_SOCKET_EVENT, (qint64) error);
   mReconnectTimer.start(RMF_SOCKET_ADAPTER_RECONNECT_TIMER_MS);
   close();
}

void SocketAdapter::onLocalSocketError(QLocalSocket::LocalSocketError error)
{
   setError(RMF_ERR_SOCKET_EVENT, (qint64) error);
   mReconnectTimer.start(RMF_SOCKET_ADAPTER_RECONNECT_TIMER_MS);
   close();
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
      break;
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
         break;
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

void SocketAdapter::readHandler(quint32 readAvail)
{
   char *pDest = prepareReceive(readAvail);
   if(pDest != NULL)
   {
      qint64 result = readSocket(pDest, readAvail);

      if (result > 0)
      {
         char* const pStartOfReceiveBuffer = mReceiveBuffer.data();
         const char* const pEnd = pDest+result;
         const char* pNext = parseRemoteFileData(pStartOfReceiveBuffer, pEnd);
         if (pNext == NULL)
         {
            setError(RMF_ERR_BAD_MSG);
         }
         else if (pNext > pEnd)
         {
            setError(RMF_ERR_INVALID_PARSE);
         }
         else
         {
            size_t unparsedLen = (size_t) (pEnd-pNext);
            if (unparsedLen > 0)
            {
               if (pNext > pStartOfReceiveBuffer)
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
         setError(RMF_ERR_SOCKET_READ_FAIL);
      }
   }
   else
   {
      setError(RMF_ERR_BAD_ALLOC, (qint64) readAvail);
   }
}


const char *SocketAdapter::parseRemoteFileData(const char* pBegin, const char* const pEnd)
{
   if ( (pBegin == 0) || (pEnd == 0) )
   {
      return NULL; //invalid arguments
   }
   /**
    * pBegin is moved forward until there is no more data to parse or there is an incomplete message in buffer
    * It returns pBegin which could be pEnd in case ALL data between pBegin and pEnd has been parsed.
    */
   while(pBegin<pEnd)
   {
      int headerLen = 0;
      int msgLen = 0;
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
         Q_ASSERT(headerLen>0);
         Q_ASSERT(msgLen>0);
         if(remain>=(quint32)msgLen)
         {
            if (m_isAcknowledgeSeen == false)
            {
               if (msgLen == RMF_CMD_TYPE_LEN+RMF_HIGH_ADDRESS_SIZE)
               {
                  const quint32 address = qFromBigEndian<quint32>((const uchar*)pNext);
                  const quint32 cmdType = qFromLittleEndian<quint32>( ((const uchar*) pNext)+RMF_HIGH_ADDRESS_SIZE);
                  if ( (address == (RMF_CMD_START_ADDR | RMF_CMD_HIGH_BIT) ) && (cmdType == RMF_CMD_ACK) )
                  {
                     m_isAcknowledgeSeen=true;
                     if (mReceiveHandler != 0)
                     {
                        mReceiveHandler->onConnected(this);
                     }
                  }
               }
               else
               {
                   qWarning() << "[RMF_SOCKET_ADAPTER] Expected an ack, but got bad message size" << msgLen;
               }
            }
            else
            {
               if (mReceiveHandler != 0)
               {
                  bool result = mReceiveHandler->onMsgReceived(pNext, msgLen);
                  if (result == false)
                  {
                     setError(RMF_ERR_BAD_MSG);
                  }
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

void SocketAdapter::setError(quint32 error, qint64 errorExtra)
{
   mErrorCode = error;

   switch(error)
   {
   case RMF_ERR_NONE:
      break;
   case RMF_ERR_SOCKET_READ_FAIL:
      qCritical() << "[RMF_SOCKET_ADAPTER] Error while reading from socket";
      break;
   case RMF_ERR_SOCKET_EVENT:
      mLastSocketError = (QAbstractSocket::SocketError) errorExtra;
      qCritical() << "[RMF_SOCKET_ADAPTER] Error event from socket: " << ((int) mLastSocketError);
      break;
   case RMF_ERR_INVALID_PARSE:
      qCritical() << "[RMF_SOCKET_ADAPTER] Error: Parsed beyond buffer size";
      break;
   case RMF_ERR_BAD_MSG:
      qCritical() << "[RMF_SOCKET_ADAPTER] Error: Bad message";
      break;
   case RMF_ERR_BAD_ALLOC:
      qCritical() << "[RMF_SOCKET_ADAPTER] Error: Unable to allocate memory for buffer " << (int) errorExtra;
      break;
   }
}

}
