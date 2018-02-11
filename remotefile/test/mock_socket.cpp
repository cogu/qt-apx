#include <cstring>
#include "mock_socket.h"

MockSocket::MockSocket()
{

}

void MockSocket::receive(const char *data, int len)
{
   QByteArray *packet = new QByteArray(data, len);
   mPendingReceive.append(packet);
}

qint64 MockSocket::bytesAvailable() const
{
   qint64 retval = -1;
   int numPackets = mPendingReceive.length();
   if (mConnectionState != false)
   {
      if (numPackets == 0)
      {
         retval = 0;
      }
      else
      {
         retval = mPendingReceive.at(0)->length();
      }
   }
   return retval;
}

qint64 MockSocket::read(char *dest, qint64 maxSize)
{
   qint64 retval = -1;
   if (mConnectionState != false)
   {
      if (mPendingReceive.isEmpty())
      {
         retval = 0; //nothing more to read
      }
      else
      {
         QByteArray *firstPacket = mPendingReceive.at(0);
         int packetLen = firstPacket->length();
         if (maxSize>=packetLen)
         {
            memcpy(dest, firstPacket->constData(), packetLen);
            //remove packet from list and free memory
            mPendingReceive.removeAt(0);
            delete firstPacket;
            retval = (qint64) packetLen;
         }
         else
         {
            //partial read not implemented
         }
      }
   }
   return retval;
}

void MockSocket::setConnectionState(bool state)
{
   mConnectionState = state;
}

void MockSocket::dropOne()
{
   if (mPendingReceive.isEmpty() == false)
   {
      QByteArray *firstPacket = mPendingReceive.takeAt(0);
      delete firstPacket;
   }
}
