#include <QtGlobal>
#include <QDebug>
#include <cstring>
#include "qapx_file.h"
#include "qapx_nodedata.h"

using namespace std;

Apx::File::File(QString name, quint32 length):RemoteFile::File(name,length),mNodeDataHandler(NULL)
{
   mData.resize((int)length);
   Q_ASSERT(mData.length() == (int)length);
   memset(mData.data(),0, length);
}

Apx::File::~File()
{

}

int Apx::File::read(quint8 *pDest, quint32 offset, quint32 length)
{
   int retval = -1;
   if ( offset+length <= (quint32) mData.length() )
   {
      mDataLock.lock();
      memcpy(pDest, (const quint8*) mData.constData()+offset, length);
      mDataLock.unlock();
      retval = length;
   }
   return retval;
}

int Apx::File::write(const quint8 *pSrc, quint32 offset, quint32 length)
{
   int retval = -1;   
   if ( offset+length <= (quint32) mData.length() )
   {
      mDataLock.lock();
      memcpy(mData.data()+offset, (void*)pSrc, length);
      mDataLock.unlock();
      retval = length;
   }
   return retval;
}


Apx::InputFile::InputFile(QString name, quint32 length, NodeDataHandler *handler) : Apx::File(name, length)
{

}

int Apx::InputFile::write(const quint8 *pSrc, quint32 offset, quint32 length)
{
   int result = Apx::File::write(pSrc, offset, length);
   if ( (result>0) && (mNodeDataHandler != NULL))
   {
      QByteArray data((const char*)pSrc,(int) length);
      mNodeDataHandler->inPortDataWriteNotify(offset,data);
   }
   return result;
}

Apx::OutputFile::OutputFile(QString name, quint32 length): Apx::File(name, length)
{

}

Apx::OutputFile::~OutputFile()
{

}

int Apx::OutputFile::write(const quint8 *pSrc, quint32 offset, quint32 length)
{
   int result = Apx::File::write(pSrc, offset, length);
   if ( (result > 0) && (mFileManager != NULL))
   {
      mFileManager->outPortDataWriteNotify(this,pSrc, offset,length);
   }
   return result;
}
