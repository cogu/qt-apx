#include "simulator.h"
#include <QDebug>

SimulatorNode::SimulatorNode(QObject *parent) : QObject(parent)
{

}

void SimulatorNode::onRequirePortData(int portId, QString &portName, QVariant &value)
{   
   qDebug()  << portName << value.toString();
}
