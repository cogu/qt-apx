#include "qapxsimpleport.h"
#include <QByteArray>

QApxSimplePort::QApxSimplePort(const char *name, const char *dsg, const char *attr):
   mName(0),mDsg(0),mAttr(0)
{
   if (name != 0)
   {
      mName = qstrdup(name);
   }
   if (dsg != 0)
   {
      mDsg = qstrdup(dsg);
   }
   if( attr != 0)
   {
      mAttr = qstrdup(attr);
   }
}

QApxSimplePort::~QApxSimplePort()
{
   if (mName != 0)
   {
      delete[] mName;
   }
   if (mDsg != 0)
   {
      delete[] mDsg;
   }
   if (mAttr != 0)
   {
      delete[] mAttr;
   }
}

QApxSimpleRequirePort::QApxSimpleRequirePort(const char *name, const char *dsg, const char *attr):
   QApxSimplePort(name,dsg,attr)
{

}

QApxSimpleProvidePort::QApxSimpleProvidePort(const char *name, const char *dsg, const char *attr):
   QApxSimplePort(name,dsg,attr)
{

}
