#ifndef SIMULATORNODE_H
#define SIMULATORNODE_H

#include <QObject>
#include <QVariant>
#include "qapx_client.h"


class SimulatorNode : public QObject
{
   Q_OBJECT
public:
   explicit SimulatorNode(Apx::Client *client, QObject *parent = 0);

protected:
   Apx::Client *mClient;
   int mTestSignal2Id;

signals:

public slots:
   void onRequirePortData(int portId, const QString &portName, const QVariant &value);

};

#endif // SIMULATORNODE_H
