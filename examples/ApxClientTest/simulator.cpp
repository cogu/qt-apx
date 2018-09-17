#include "simulator.h"
#include <QDebug>

SimulatorNode::SimulatorNode(Apx::Client *client, QObject *parent) : QObject(parent), mClient(client), mTestSignal2Id(-1)
{
   if(mClient != nullptr)
   {
      mTestSignal2Id = mClient->findProvidePortId("TestSignal2");
   }

}

void SimulatorNode::onRequirePortData(int portId, const QString &portName, const QVariant &value)
{
   (void) portId;
   QVariant tmpVal = value;
   if (portName == "TestSignal1")
   {
      if (mClient != nullptr)
      {
         mClient->setProvidePortValue(mTestSignal2Id, tmpVal);
      }
   }
   qDebug()  << portName << value.toString();
}
