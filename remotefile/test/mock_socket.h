#ifndef MOCK_SOCKET_H
#define MOCK_SOCKET_H

#include <QByteArray>

class MockSocket
{
public:
   MockSocket();
   void receive(const char *data, int len);
   qint64 bytesAvailable() const;
   qint64 read(char *data, qint64 maxSize);

protected:
   QByteArray mPendingReceive;

};

#endif // MOCK_SOCKET_H
