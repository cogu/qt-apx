#include "stressnode.h"
#include <QDebug>

StressNode::StressNode(Apx::Client *client, QObject *parent) : QObject(parent), mClient(client)
{
   if(mClient != nullptr)
   {
      mQTestSignal1Id = mClient->findProvidePortId("QTestSignal1");
      mQTestSignal2Id = mClient->findProvidePortId("QTestSignal2");
      mQTestSignal3Id = mClient->findProvidePortId("QTestSignal3");
   }
   mQTestSignal1Value = 0;
   mQTestSignal2Value = 0;
   mQTestSignal3Value = 0;

   mTimer = new QTimer(parent);
   QObject::connect(mClient, &Apx::Client::requirePortData, this, &StressNode::onRequirePortData);
   QObject::connect(mTimer, SIGNAL(timeout()), this, SLOT(onTimerTimeout()));
}

void StressNode::start()
{
   mTimer->start(5);
}

void StressNode::onRequirePortData(int portId, QString &portName, QVariant &value)
{
   (void) portId;
   //qDebug()  << portName << value.toString();
}

void StressNode::onTimerTimeout()
{
   QVariant tmp;
   mQTestSignal1Value = (mQTestSignal1Value + 1) & 0xFFFF;
   mQTestSignal2Value = (mQTestSignal2Value + 1) & 0xFFFF;
   mQTestSignal3Value = (mQTestSignal3Value + 1) & 0xFFFF;
   tmp = mQTestSignal1Value;
   mClient->setProvidePortValue(mQTestSignal1Id, tmp);
   tmp = mQTestSignal2Value;
   mClient->setProvidePortValue(mQTestSignal2Id, tmp);
   tmp = mQTestSignal3Value;
   mClient->setProvidePortValue(mQTestSignal3Id, tmp);   
}
