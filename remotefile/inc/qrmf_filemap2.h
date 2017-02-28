/**
  QT adaptation of RemoteFile Layer
*/
#ifndef QRMF_FILEMAP_H
#define QRMF_FILEMAP_H
#include <QString>
#include <QtGlobal>
#include <QList>
#include "qrmf_file.h"

namespace RemoteFile {

/**
 * @brief Abstract base class for RemoteFile::FileMap2. Concrete classes needs to implement the assignFileAddressDefault method
 */
class FileMap2
{
public:
   FileMap2():mLastFileIndex(-1) {}
   virtual ~FileMap2() {}
   virtual bool insert(RemoteFile::File *file);
   virtual bool remove(RemoteFile::File *file);
   int size() {return mFiles.size();}
   bool assignFileAddress(RemoteFile::File *file, quint32 startAddress, quint32 endAddress, quint32 addressBoundary);
   void iterInit(void);
   RemoteFile::File *next();
   RemoteFile::File *findByAddress(quint32 address);
   RemoteFile::File *findByName(const char *name);
protected:
   virtual bool assignFileAddressDefault(RemoteFile::File *file) = 0;
   bool verifyFileAddress(RemoteFile::File*, quint32 startAddress, quint32 endAddress);
   void insertDefault(RemoteFile::File *file);
protected:
   QList<RemoteFile::File*> mFiles; //weak or strong reference to File*. strength of reference is decided by property
   typedef QList<RemoteFile::File*>::iterator ListIterator;
   ListIterator mIterator;
   int mLastFileIndex;
};

class FileMapDefault2: public FileMap2
{
public:
   FileMapDefault2():mAddressBoundary(4096){}
   ~FileMapDefault2(){}
protected:
   bool assignFileAddressDefault(RemoteFile::File *file);
protected:
   quint32 mAddressBoundary;
};


}
#endif // QRMF_FILEMAP_H
#ifndef QRMF_FILEMAP2_H
#define QRMF_FILEMAP2_H

#endif // QRMF_FILEMAP2_H
