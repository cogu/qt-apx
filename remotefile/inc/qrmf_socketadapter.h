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
#define RMF_SOCKET_ADAPTER_RECONNECT_TIMER_MS 5000

#define RMF_SOCKET_TYPE_NONE           0
#define RMF_SOCKET_TYPE_TCP            1
#define RMF_SOCKET_TYPE_LOCAL          2
#define RMF_SOCKET_TYPE_MOCK           3

namespace RemoteFile
{

class SocketAdapter : public QObject,public RemoteFile::TransmitHandler
{
   Q_OBJECT
public:
   explicit SocketAdapter(int numHeaderBits=32,QObject *parent = 0);
   virtual ~SocketAdapter();
   int connectTcp(QHostAddress address,quint16 port);
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
   char *mSendBufPtr;

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
   bool readHandler(quint32 readAvail);

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
