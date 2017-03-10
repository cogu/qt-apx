#include <cstring>
#include "qrmf_file.h"
#include "qrmf_filemanager.h"

namespace RemoteFile
{

File::File(): mName(""),
   mAddress(RMF_INVALID_ADDRESS),
   mLength(0),
   mFileType(RMF_FILE_TYPE_FIXED), //other file types will be implemented later on
   mDigestType(RMF_DIGEST_TYPE_NONE),
   isWeakRef(true),
   isOpen(false),
   mFileManager(NULL)

{
   memset(&mDigestData[0],0,RMF_DIGEST_SIZE);
}

File::File(QString &name, quint32 length) : mName(name),
   mAddress(RMF_INVALID_ADDRESS),
   mLength(length),
   mFileType(RMF_FILE_TYPE_FIXED), //other file types will be implemented later on
   mDigestType(RMF_DIGEST_TYPE_NONE),
   isWeakRef(true),
   isOpen(false),
   mFileManager(NULL)
{
   memset(&mDigestData[0],0,RMF_DIGEST_SIZE);
}

File::File(const char *name, quint32 length)
   : mName(name),
     mAddress(RMF_INVALID_ADDRESS),
     mLength(length),
     mFileType(RMF_FILE_TYPE_FIXED), //other file types will be implemented later on
     mDigestType(RMF_DIGEST_TYPE_NONE),
     isWeakRef(true),
     isOpen(false),
     mFileManager(NULL)
  {
     memset(&mDigestData[0],0,RMF_DIGEST_SIZE);
  }

File::~File()
{

}

/**
 * @brief This is not implemented in the base class but can be overriden in child classes
 * @param pDest
 * @param offset
 * @param length
 * @return returns -1 on failure
 */
int File::read(quint8 *pDest, quint32 offset, quint32 length)
{
   (void) pDest; (void) offset; (void) length;
   return -1;
}

/**
 * @brief This is not implemented in the base class but can be overriden by child classes
 * @param pDest
 * @param offset
 * @param length
 * @return returns -1 on failure
 */
int File::write(const quint8 *pSrc, quint32 offset, quint32 length)
{
   (void) pSrc; (void) offset; (void) length;
   return -1;
}

}
