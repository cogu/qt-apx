#include "qapxsimplenode.h"

QApxSimpleNode::QApxSimpleNode()
{

}

QApxSimpleNode::~QApxSimpleNode()
{
   DataTypeListIterator it;
   PortListIterator it2;
   for(it2=mRequirePorts.begin();it2!=mRequirePorts.end();it2++)
   {
      QApxSimplePort *port = *it2;
      if(port != NULL)
      {
         delete port;
      }
   }
   for(it2=mProvidePorts.begin();it2!=mProvidePorts.end();it2++)
   {
      QApxSimplePort *port = *it2;
      if(port != NULL)
      {
         delete port;
      }
   }
   for (it=mDataTypes.begin(); it!= mDataTypes.end(); it++)
   {
      QApxDataType* dataType = *it;
      if (dataType != NULL)
      {
         delete dataType;
      }
   }
}

void QApxSimpleNode::appendType(QApxDataType *dataType)
{
   mDataTypes.append(dataType);
}

QApxDataType *QApxSimpleNode::getTypeById(int i)
{
   if (i<mDataTypes.size())
   {
      return mDataTypes.at(i);
   }
   return (QApxDataType*) NULL;
}

void QApxSimpleNode::appendRequirePort(QApxSimpleRequirePort *port)
{
   mRequirePorts.append((QApxSimplePort*) port);
   mPortMap.insert(QString(port->getName()),dynamic_cast<QApxSimplePort*>(port));
}

void QApxSimpleNode::appendProvidePort(QApxSimpleProvidePort *port)
{
   mProvidePorts.append((QApxSimplePort*) port);
   mPortMap.insert(QString(port->getName()),dynamic_cast<QApxSimplePort*>(port));
}

QApxSimplePort *QApxSimpleNode::getRequirePortById(int i)
{
   if (i< mRequirePorts.size())
   {
      return mRequirePorts.at(i);
   }
   return (QApxSimplePort*) NULL;
}

QApxSimplePort *QApxSimpleNode::getProvidePortById(int i)
{
   if (i< mProvidePorts.size())
   {
      return mProvidePorts.at(i);
   }
   return (QApxSimplePort*) NULL;
}

QApxSimplePort *QApxSimpleNode::findPortByName(const char *name)
{
   PortMapItertator it = mPortMap.find(QString(name));
   if (it != mPortMap.end())
   {
      return it.value();
   }
   return (QApxSimplePort*) NULL;
}

void QApxSimpleNode::setName(const QString &name)
{
   mName=name;
}

QString &QApxSimpleNode::getName()
{
   return mName;
}

