#include "mocktransmithandler.h"

#define MIN_TRANSMIT_SIZE 4096

namespace RemoteFile
{


MockTransmitHandler::MockTransmitHandler(QObject *parent) : QObject(parent)
{
   mSendBuffer.resize(MIN_TRANSMIT_SIZE);
}

int MockTransmitHandler::send(int offset, int msgLen)
{
   Q_ASSERT( (offset>=0) && (offset+msgLen<mSendBuffer.size()));
   mMessages.append(mSendBuffer); //append a copy of mSendBuffer
   memset(mSendBuffer.data(),0,mSendBuffer.size());
   emit messageReceived();
   return msgLen;
}

}
