#ifndef QRMF_SOCKET_ADAPTER_H
#define QRMF_SOCKET_ADAPTER_H

#include <QtGlobal>
#include <QObject>
#include <QString>
#include <QTimer>
#include <QHostAddress>
#include <QLocalSocket>
#include <QTcpSocket>
#include "qrmf_base.h"
#ifdef UNIT_TEST
#include "mock_socket.h"
#endif
#define RMF_SOCKET_ADAPTER_MIN_BUF_LEN 16384     //16KiB
#define RMF_SOCKET_ADAPTER_MAX_BUF_LEN 0x4000000 //64MiB

#ifndef RMF_SOCKET_ADAPTER_RECONNECT_TIMER_MS
#define RMF_SOCKET_ADAPTER_RECONNECT_TIMER_MS 5000
#endif

#define RMF_SOCKET_TYPE_NONE           0
#define RMF_SOCKET_TYPE_TCP            1
#define RMF_SOCKET_TYPE_LOCAL          2
#define RMF_SOCKET_TYPE_MOCK           3


#define RMF_ERR_NONE                   0u
#define RMF_ERR_SOCKET_READ_FAIL       1u
#define RMF_ERR_SOCKET_READ_AVAIL_FAIL 2u
#define RMF_ERR_SOCKET_EVENT           3u
#define RMF_ERR_INVALID_PARSE          4u
#define RMF_ERR_BAD_MSG                5u
#define RMF_ERR_BAD_ALLOC              6u
#define RMF_ERR_MSG_LEN_TOO_LONG       7u

namespace RemoteFile
{

class SocketAdapter : public QObject,public RemoteFile::TransmitHandler
{
   Q_OBJECT
public:
   explicit SocketAdapter(int numHeaderBits=32,QObject *parent = 0);
   virtual ~SocketAdapter();
   int connectTcp(const QHostAddress& address,quint16 port);
   int connectLocal(const char *filename); //connects to local socket in unix, named pipe on Windows
#ifdef UNIT_TEST
   int connectMock(MockSocket *socket);
#endif
   void close();
   //TransmitHandler Interface
   int getSendAvail();
   char *getSendBuffer(int msgLen);
   int send(int offset, int msgLen);
   void setReceiveHandler(RemoteFile::ReceiveHandler *handler){mReceiveHandler=handler;}
   int getRxPending(){return mRxPending;}
   quint32 getError(){return mErrorCode;}


protected:
   int mSocketType;
   size_t mRxPending;
   int mNumHeaderBits;
   int mMaxNumHeaderLen;
   //used for tcp socket
   quint16 mTcpPort;
   QHostAddress mTcpAddress;
   QTcpSocket *mTcpSocket;
   //used for local socket
   QString mLocalSocketName;
   QLocalSocket *mLocalSocket;
#ifdef UNIT_TEST
   MockSocket *mMockSocket;
#endif

   QByteArray mReceiveBuffer;
   QByteArray mSendBuffer;
   quint8 mStatus;
   int mTotalReceived;
   QTimer mReconnectTimer;
   RemoteFile::ReceiveHandler *mReceiveHandler;
   bool m_isAcknowledgeSeen;
   bool m_isServerConnectedOnce;
   char *mSendBufPtr;
   quint32 mErrorCode;
   QAbstractSocket::SocketError mLastSocketError;

#ifdef UNIT_TEST
public:
#else
protected:
#endif
   char *prepareReceive(quint32 readLen);
   const char* parseRemoteFileData(const char *pBegin, const char *pEnd);   
   void sendGreetingHeader();
   qint64 getSocketReadAvail();
   qint64 readSocket(char *pDest, quint32 readLen);
   void readHandler(quint32 readAvail);
   void setError(quint32 error, qint64 errorExtra = -1);

   void disconnectSocket();

signals:
   void connected(void);
   void disconnected(void);

#ifdef UNIT_TEST
public:
#else
public slots:
#endif
   void onConnected(void);
   void onDisconnected(void);
   void onReadyread(void);
   void onReconnectTimeout(void);
   void onTcpSocketError(QAbstractSocket::SocketError error);
   void onLocalSocketError(QLocalSocket::LocalSocketError error);

};

}
#endif // QRMF_SOCKET_ADAPTER_H
