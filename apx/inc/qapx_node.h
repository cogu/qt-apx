#ifndef QAPX_NODE_H
#define QAPX_NODE_H

#include "qapxdatatype.h"
#include <QList>
#include <QString>
#include <QMap>
#include "qapxsimpleport.h"

namespace Apx
{
   class Node
   {
   public:
      Node();
      ~Node();
      void appendType(QApxDataType *dataType);
      QApxDataType *getTypeById(int i);
      void appendRequirePort(QApxSimpleRequirePort *port);
      void appendProvidePort(QApxSimpleProvidePort *port);
      QApxSimplePort *getRequirePortById(int i);
      QApxSimplePort *getProvidePortById(int i);
      int getNumRequirePorts(){return mRequirePorts.size();}
      int getNumProvidePorts(){return mProvidePorts.size();}
      QApxSimplePort *findPortByName(const char *name);
      void setName(const QString &name);
      QString &getName();

   protected:
      QString mName;
      QList<QApxDataType*> mDataTypes;
      QList<QApxSimplePort*> mRequirePorts;
      QList<QApxSimplePort*> mProvidePorts;
      QMap<QString,QApxSimplePort*> mPortMap;
      typedef QList<QApxSimplePort*>::iterator PortListIterator;
      typedef QList<QApxDataType*>::iterator DataTypeListIterator;
      typedef QMap<QString,QApxSimplePort*>::iterator PortMapItertator;
   };
}
#endif //QAPX_NODE_H
