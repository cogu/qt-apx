/**
  QT adaptation of RemoteFile Layer
*/
#ifndef QRMF_FILE_H
#define QRMF_FILE_H
#include <QtGlobal>
#include <QString>
#include "qrmf_base.h"


namespace RemoteFile {

class FileManager;

enum FileType {
   RMF_FILE_TYPE_FIXED,
   RMF_FILE_TYPE_DYNAMIC,
   RMF_FILE_TYPE_STREAM
};

enum DigestType {
   RMF_DIGEST_TYPE_NONE,
   RMF_DIGEST_TYPE_SHA1,
   RMF_DIGEST_TYPE_SHA256
};

/**
 * @brief This is a (hopefully better) version of the rmf_file_info_t struct from the C implementation of remotefile
 */
class File
{
public:
   QString mName;                      //name of the file, default=""
   quint32 mAddress;                   //address of the file, default=RMF_INVALID_ADDRESS
   quint32 mLength;                    //default=0
   RemoteFile::FileType mFileType;     //default=RMF_FILE_TYPE_FIXED
   RemoteFile::DigestType mDigestType; //default=RMF_DIGEST_TYPE_NONE
   quint8 mDigestData[RMF_DIGEST_SIZE];
   bool isWeakRef;                     //default true
   bool isOpen;                        //default false
   RemoteFile::FileManager *mFileManager;


   File();   
   File(QString &name, quint32 length);
   File(const char *name, quint32 length);
   virtual ~File();

   virtual int read(quint8 *pDest, quint32 offset, quint32 length); //this can be overriden by inherited classes
   virtual int write(const quint8 *pSrc, quint32 offset, quint32 length);
};

}

#endif // RMF_FILE_H
