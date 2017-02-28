#include "qapxdatatype.h"
#include <QByteArray>

QApxDataType::QApxDataType(const char *name, const char *dsg, const char *attr):
   mName(0),mDsg(0),mAttr(0)
{
   if (name != 0)
   {
      mName=qstrdup(name);
   }
   if (dsg != 0)
   {
      mDsg=qstrdup(dsg);
   }
   if (attr != 0)
   {
      mAttr=qstrdup(attr);
   }
}

QApxDataType::~QApxDataType()
{
   if (mName != 0) { delete[] mName;}
   if (mDsg != 0) { delete[] mDsg;}
   if (mAttr != 0) { delete[] mAttr;}
}

