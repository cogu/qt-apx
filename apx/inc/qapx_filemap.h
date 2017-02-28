#ifndef QAPX_FILEMAP_H
#define QAPX_FILEMAP_H
#include "qrmf_filemap2.h"
#include "qapx_file.h"

//constants
#define PORT_DATA_START     0x0
#define PORT_DATA_BOUNDARY  0x400 //1KB, this must be a power of 2
#define DEFINITION_START    0x4000000 //64MB, this must be a power of 2
#define DEFINITION_BOUNDARY 0x100000 //1MB, this must be a power of 2
#define USER_DATA_START     0x20000000 //512MB, this must be a power of 2
#define USER_DATA_END       0x3FFFFC00 //Start of remote file cmd message area
#define USER_DATA_BOUNDARY  0x100000 //1MB, this must be a power of 2

namespace Apx
{
   class FileMap : public RemoteFile::FileMap2
   {
   public:
      FileMap() {}
      virtual ~FileMap() {}
      Apx::File *findByAddress(quint32 address);
      Apx::File *findByName(const char *name);
   protected:
      bool assignFileAddressDefault(RemoteFile::File *file);
   };
}

#endif // QAPX_FILEMAP_H
