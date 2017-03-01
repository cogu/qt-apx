#ifndef SIMULATORNODE_H
#define SIMULATORNODE_H

#include <QObject>
#include <QVariant>


class SimulatorNode : public QObject
{
   Q_OBJECT
public:
   explicit SimulatorNode(QObject *parent = 0);

signals:

public slots:
   void onRequirePortData(int portId, QString &portName, QVariant &value);
};

#endif // SIMULATORNODE_H
