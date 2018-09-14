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
      QApxDataType *getTypeById(int i) const;
      void appendRequirePort(QApxSimpleRequirePort *port);
      void appendProvidePort(QApxSimpleProvidePort *port);
      QApxSimplePort *getRequirePortById(int i) const;
      QApxSimplePort *getProvidePortById(int i) const;
      int getNumRequirePorts() const {return mRequirePorts.size();}
      int getNumProvidePorts() const {return mProvidePorts.size();}
      QApxSimplePort *findPortByName(const char *name) const;
      void setName(const QString &name);
      const QString &getName() const;

   protected:
      QString mName;
      QList<QApxDataType*> mDataTypes;
      QList<QApxSimplePort*> mRequirePorts;
      QList<QApxSimplePort*> mProvidePorts;
      QMap<QString,QApxSimplePort*> mPortMap;
      typedef QList<QApxSimplePort*>::const_iterator PortListIterator;
      typedef QList<QApxDataType*>::const_iterator DataTypeListIterator;
      typedef QMap<QString,QApxSimplePort*>::const_iterator PortMapItertator;
   };
}
#endif //QAPX_NODE_H
