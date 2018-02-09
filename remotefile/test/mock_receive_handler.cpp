#include "mock_receive_handler.h"

namespace RemoteFile
{

void MockReceiveHandler::onMsgReceived(const char *msgData, int msgLen)
{
   QByteArray *msg = new QByteArray(msgData, msgLen);
   appendMessage(msg);
}

void MockReceiveHandler::onConnected(TransmitHandler *transmitHandler)
{
   this->transmitHandler = transmitHandler;
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
