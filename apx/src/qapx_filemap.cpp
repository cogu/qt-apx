#include "qapx_filemap.h"

/**
 * @brief default address assigner for APX file names
 * @param file
 * @return
 */
bool Apx::FileMap::assignFileAddressDefault(RemoteFile::File *file)
{
   if (file->mName.endsWith(".in") || file->mName.endsWith(".out") )
   {
      return assignFileAddress(file, PORT_DATA_START, DEFINITION_START, PORT_DATA_BOUNDARY);
   }
   else if (file->mName.endsWith(".apx") )
   {
      return assignFileAddress(file, DEFINITION_START, USER_DATA_START, DEFINITION_BOUNDARY);
   }
   else
   {
      return assignFileAddress(file, USER_DATA_START, USER_DATA_END, USER_DATA_BOUNDARY);
   }
}

Apx::File *Apx::FileMap::findByAddress(quint32 address)
{
   RemoteFile::File *file = RemoteFile::FileMap2::findByAddress(address);
   return (Apx::File*) file;
}

/**
 * @brief not implemented
 * @param name
 * @return
 */
Apx::File *Apx::FileMap::findByName(const char *name)
{
   (void) name;
   return NULL;
}
