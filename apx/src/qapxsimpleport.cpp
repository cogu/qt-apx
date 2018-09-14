#include "qapxsimpleport.h"
#include <QByteArray>
#include <string.h>

static const char attrSeparator = ',';
static const char attrInitMarker = '=';
static const char attrInitGroupEndBracket = '}';
static const char attrInitStringEndMarker = '"';

static const char * buildAttrInitRaw(const char *attr)
{
   if (attr==nullptr)
   {
      return nullptr;
   }
   char * ret_val = nullptr;
   const char * initStart = strchr(attr, attrInitMarker);
   const char * initLastChar = nullptr;
   if (nullptr != initStart)
   {
      // Init value starts on the char after the attrInitMarker
      initStart = initStart + 1;
      // Check if signal group
      initLastChar = strrchr(initStart, attrInitGroupEndBracket);
      if (nullptr == initLastChar)
      {
         // Not a group - check if string
         initLastChar = strrchr(initStart, attrInitStringEndMarker);
         if (nullptr == initLastChar)
         {
            // Not a string or group - find init attribute ending if not last attribute
            initLastChar = strchr(initStart, attrSeparator);
            if (nullptr != initLastChar)
            {
               // End is at char before the attrSeparator
               initLastChar = initLastChar - 1;
            }
         }
      }
      if (nullptr == initLastChar)
      {
         // Simple init attribute is last - so duplicate the string
         ret_val = qstrdup(initStart);
      }
      else
      {
         const size_t initSize = initLastChar - initStart + 1;
         ret_val = new char[initSize + 1];
         memcpy(ret_val, initStart, initSize);
         ret_val[initSize] ='\0';
      }
   }
   return ret_val;
}

QApxSimplePort::QApxSimplePort(const char *name, const char *dsg, const char *attr):
   mAttrInit(nullptr),mPortIndex(-1)
{
   // qstrdup of nullptr will return nullptr (http://doc.qt.io/qt-5/qbytearray.html#qstrdup)
   mName = qstrdup(name);
   mDsg = qstrdup(dsg);
   mAttrFull = qstrdup(attr);
   mAttrInit = buildAttrInitRaw(attr);
}

QApxSimplePort::~QApxSimplePort()
{
   if (mName != nullptr)
   {
      delete[] mName;
   }
   if (mDsg != nullptr)
   {
      delete[] mDsg;
   }
   if (mAttrFull != nullptr)
   {
      delete[] mAttrFull;
   }
   if (mAttrInit != nullptr)
   {
      delete[] mAttrInit;
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
