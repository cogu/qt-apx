#include "qapxdatatype.h"
#include <QByteArray>

QApxDataType::QApxDataType(const char *name, const char *dsg, const char *attr):
   mName(nullptr),mDsg(nullptr),mAttr(nullptr)
{
   if (name != nullptr)
   {
      mName=qstrdup(name);
   }
   if (dsg != nullptr)
   {
      mDsg=qstrdup(dsg);
   }
   if (attr != nullptr)
   {
      mAttr=qstrdup(attr);
   }
}

QApxDataType::~QApxDataType()
{
   if (mName != nullptr) { delete[] mName;}
   if (mDsg != nullptr) { delete[] mDsg;}
   if (mAttr != nullptr) { delete[] mAttr;}
}

