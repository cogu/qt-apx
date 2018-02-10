#ifndef MOCK_RECEIVE_HANDLER_H
#define MOCK_RECEIVE_HANDLER_H

#include "qrmf_base.h"
#include <QList>
#include <QByteArray>

namespace RemoteFile
{

class MockReceiveHandler : public ReceiveHandler
{
public:
   MockReceiveHandler(): transmitHandler(NULL){}
   virtual ~MockReceiveHandler(){
      clearMessages();
   }
   void onMsgReceived(const char *msgData, int msgLen);
   void onConnected(RemoteFile::TransmitHandler *transmitHandler);
   void onDisconnected(RemoteFile::TransmitHandler *transmitHandler);
   void clearMessages();

   QList<QByteArray*> messages;
   TransmitHandler *transmitHandler;

protected:
   void appendMessage(QByteArray *msg);


};

}

#endif // MOCK_RECEIVE_HANDLER_H
