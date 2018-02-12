#include "simulator.h"
#include <QDebug>

SimulatorNode::SimulatorNode(Apx::Client *client, QObject *parent) : QObject(parent), mClient(client), mTestSignal2Id(-1)
{
   if(mClient != 0)
   {
      mTestSignal2Id = mClient->findProvidePortId("TestSignal2");
   }

}

void SimulatorNode::onRequirePortData(int portId, QString &portName, QVariant &value)
{
   (void) portId;
   if (portName == "TestSignal1")
   {
      if (mClient != 0)
      {
         mClient->setProvidePortValue(mTestSignal2Id, value);
      }
   }
   qDebug()  << portName << value.toString();
}
