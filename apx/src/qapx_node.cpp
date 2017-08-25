#include "qapx_node.h"
namespace Apx
{
   Node::Node()
   {

   }

   Node::~Node()
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

   void Node::appendType(QApxDataType *dataType)
   {
      mDataTypes.append(dataType);
   }

   QApxDataType *Node::getTypeById(int i)
   {
      if (i<mDataTypes.size())
      {
         return mDataTypes.at(i);
      }
      return (QApxDataType*) NULL;
   }

   void Node::appendRequirePort(QApxSimpleRequirePort *port)
   {
      mRequirePorts.append((QApxSimplePort*) port);
      mPortMap.insert(QString(port->getName()),dynamic_cast<QApxSimplePort*>(port));
   }

   void Node::appendProvidePort(QApxSimpleProvidePort *port)
   {
      mProvidePorts.append((QApxSimplePort*) port);
      mPortMap.insert(QString(port->getName()),dynamic_cast<QApxSimplePort*>(port));
   }

   QApxSimplePort *Node::getRequirePortById(int i)
   {
      if (i< mRequirePorts.size())
      {
         return mRequirePorts.at(i);
      }
      return (QApxSimplePort*) NULL;
   }

   QApxSimplePort *Node::getProvidePortById(int i)
   {
      if (i< mProvidePorts.size())
      {
         return mProvidePorts.at(i);
      }
      return (QApxSimplePort*) NULL;
   }

   QApxSimplePort *Node::findPortByName(const char *name)
   {
      PortMapItertator it = mPortMap.find(QString(name));
      if (it != mPortMap.end())
      {
         return it.value();
      }
      return (QApxSimplePort*) NULL;
   }

   void Node::setName(const QString &name)
   {
      mName=name;
   }

   QString &Node::getName()
   {
      return mName;
   }
}
