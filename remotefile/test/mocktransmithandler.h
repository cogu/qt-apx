#ifndef MOCKTRANSMITHANDLER_H
#define MOCKTRANSMITHANDLER_H

#include <QObject>
#include <QList>
#include <QByteArray>
#include "qrmf_base.h"


namespace RemoteFile
{

class MockTransmitHandler : public QObject, public RemoteFile::TransmitHandler
{
   Q_OBJECT
public:
   MockTransmitHandler(QObject *parent);
   ~MockTransmitHandler()
   {
   }

   int getSendAvail()
   {
      return mSendBuffer.size();
   }

   char* getSendBuffer(int msgLen)
   {
      if (getSendAvail()>=msgLen)
      {
         return mSendBuffer.data();
      }
      return nullptr;
   }

   int send(int offset, int msgLen);

   QList<QByteArray> &getMessages(){return mMessages;}

protected:
   QByteArray mSendBuffer;
   QList<QByteArray> mMessages;

signals:
   void messageReceived();
};

}
#endif // MOCKTRANSMITHANDLER_H
