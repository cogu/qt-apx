#ifndef STRESS_NODE_H
#define STRESS_NODE_H

#include <QObject>
#include <QVariant>
#include <QTimer>
#include "qapx_client.h"


class StressNode : public QObject
{
   Q_OBJECT
public:
   explicit StressNode(Apx::Client *client, QObject *parent = 0);

   void start();

protected:
   Apx::Client *mClient;
   QTimer *mTimer;
   int mQTestSignal1Id;
   int mQTestSignal2Id;
   int mQTestSignal3Id;
   int mQTestSignal1Value;
   int mQTestSignal2Value;
   int mQTestSignal3Value;


signals:

public slots:
   void onRequirePortData(int portId, QString &portName, QVariant &value);
   void onTimerTimeout();

};

#endif // STRESS_NODE_H
