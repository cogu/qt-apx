#include "mock_receive_handler.h"

namespace RemoteFile
{

bool MockReceiveHandler::onMsgReceived(const char *msgData, int msgLen)
{
   QByteArray *msg = new QByteArray(msgData, msgLen);
   appendMessage(msg);
   return mParseResult;
}

void MockReceiveHandler::onConnected(TransmitHandler *transmitHandler)
{
   this->transmitHandler = transmitHandler;
}

void MockReceiveHandler::onDisconnected()
{

}

void MockReceiveHandler::appendMessage(QByteArray *msg)
{
   messages.append(msg);
}

void MockReceiveHandler::clearMessages()
{
   int len = messages.length();
   for(int i=0;i<len;i++)
   {
      delete messages.at(i);
   }
   messages.clear();
}

}
