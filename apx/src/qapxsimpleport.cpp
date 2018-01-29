#include "qapxsimpleport.h"
#include <QByteArray>
#include <string.h>

static const char attrSeparator = ',';
static const char attrInitMarker = '=';
static const char attrInitGroupEndBracket = '}';
static const char attrInitStringEndMarker = '"';

static const char * buildAttrInitRaw(const char *attr)
{
   if (attr==NULL)
   {
      return NULL;
   }
   char * ret_val = NULL;
   const char * initStart = strchr(attr, attrInitMarker);
   const char * initLastChar = NULL;
   if (NULL != initStart)
   {
      // Init value starts on the char after the attrInitMarker
      initStart = initStart + 1;
      // Check if signal group
      initLastChar = strrchr(initStart, attrInitGroupEndBracket);
      if (NULL == initLastChar)
      {
         // Not a group - check if string
         initLastChar = strrchr(initStart, attrInitStringEndMarker);
         if (NULL == initLastChar)
         {
            // Not a string or group - find init attribute ending if not last attribute
            initLastChar = strchr(initStart, attrSeparator);
            if (NULL != initLastChar)
            {
               // End is at char before the attrSeparator
               initLastChar = initLastChar - 1;
            }
         }
      }
      if (NULL == initLastChar)
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
   mAttrInit(NULL),mPortIndex(-1)
{
   // qstrdup of null pointer will return null (http://doc.qt.io/qt-5/qbytearray.html#qstrdup)
   mName = qstrdup(name);
   mDsg = qstrdup(dsg);
   mAttrFull = qstrdup(attr);
   mAttrInit = buildAttrInitRaw(attr);
}

QApxSimplePort::~QApxSimplePort()
{
   if (mName != NULL)
   {
      delete[] mName;
   }
   if (mDsg != NULL)
   {
      delete[] mDsg;
   }
   if (mAttrFull != NULL)
   {
      delete[] mAttrFull;
   }
   if (mAttrInit != NULL)
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
