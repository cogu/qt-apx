#include <cstring>
#include "mock_socket.h"

MockSocket::MockSocket()
{

}

void MockSocket::receive(const char *data, int len)
{
   mPendingReceive.append(data, len);
}

qint64 MockSocket::bytesAvailable() const
{
   return mPendingReceive.length();
}

qint64 MockSocket::read(char *data, qint64 maxSize)
{
   if (maxSize>=mPendingReceive.length())
   {
      qint64 retval = (qint64) mPendingReceive.length();
      memcpy(data, mPendingReceive.constData(), mPendingReceive.length());
      mPendingReceive.clear();
      return retval;
   }
   return 0;
}
