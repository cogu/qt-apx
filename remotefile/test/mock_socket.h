#ifndef MOCK_SOCKET_H
#define MOCK_SOCKET_H

#include <QByteArray>
#include <QList>

class MockSocket
{
public:   
   MockSocket();
   void receive(const char *data, int len);
   qint64 bytesAvailable() const;
   qint64 read(char *dest, qint64 maxSize);
   void setConnectionState(bool state);
   void dropOne();

protected:
   QList<QByteArray*> mPendingReceive;
   bool mConnectionState;
};

#endif // MOCK_SOCKET_H
