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
   MockReceiveHandler(): transmitHandler(nullptr),mParseResult(true){}
   virtual ~MockReceiveHandler(){
      clearMessages();
   }
   bool onMsgReceived(const char *msgData, int msgLen);
   void onConnected(RemoteFile::TransmitHandler *transmitHandler);
   void onDisconnected();
   void clearMessages();
   void setParseResult(bool result) {mParseResult = result;}

   QList<QByteArray*> messages;
   TransmitHandler *transmitHandler;

protected:
   void appendMessage(QByteArray *msg);
   bool mParseResult;


};

}

#endif // MOCK_RECEIVE_HANDLER_H
