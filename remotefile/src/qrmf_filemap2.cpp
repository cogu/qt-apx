#include "qrmf_filemap2.h"
#include <QDebug>

namespace RemoteFile
{

bool FileMap2::insert(RemoteFile::File *file)
{
   if (file->mAddress == RMF_INVALID_ADDRESS)
   {
      bool result = assignFileAddressDefault(file);
      if (result == false)
      {
         return false;
      }
   }
   insertDefault(file);
   return true;
}

/**
 * @brief not yet implementend
 * @param file
 * @return true on success, false on failure
 */
bool FileMap2::remove(RemoteFile::File *file)
{
   (void) file;
   return false;
}

void FileMap2::clear()
{
   ListIterator it = mFiles.begin();
   while(it != mFiles.end())
   {
      RemoteFile::File *file = *it;
      if ( (file != NULL) && (file->isWeakRef==false))
      {
         delete file;
      }
      it++;
   }
   mFiles.clear();
}

/**
 * @brief RemoteFile::FileMap2::assignFileAddress
 * @param file
 * @param startAddress
 * @param endAddress
 * @param addressBoundary
 * @return true on success, false on failure
 */
bool FileMap2::assignFileAddress(RemoteFile::File *file, quint32 startAddress, quint32 endAddress, quint32 addressBoundary)
{
   int numFiles = mFiles.length();
   int i;
   int a=-1;
   int b=-1;
   for(i=0;i<numFiles;i++)
   {
      if (mFiles[i]->mAddress>=startAddress)
      {
         a=i;
         break;
      }
   }
   if (a >= 0)
   {
      for(i=a+1;i<numFiles;i++)
      {
         b=i;
         if (mFiles[i]->mAddress>=endAddress)
         {
            break;
         }
         a=i;
         b=-1;
      }
   }
   if ( a != -1 )
   {
      //insert after a
      if ( (startAddress & (startAddress-1))==0 && ((addressBoundary & (addressBoundary-1))==0) )
      {
         quint32 prevEndAddress = mFiles[a]->mAddress+mFiles[a]->mLength;
         file->mAddress = (prevEndAddress + (addressBoundary-1)) & (~(addressBoundary-1));
      }
      else
      {
         //use slower version
         ///TODO: implement slower version to support cases where the two arguments are not powers of 2.
         Q_ASSERT(0);
      }
   }
   else
   {
      file->mAddress = startAddress;
   }
   if ( b != -1)
   {
      endAddress=mFiles[b]->mAddress;
   }
   return verifyFileAddress(file, startAddress, endAddress);
}

void FileMap2::iterInit()
{
   mIterator = mFiles.begin();
}

File *FileMap2::next()
{
   File *retval = NULL;
   if (mIterator != mFiles.end())
   {
      retval = *mIterator;
      mIterator++;
   }
   return retval;
}

File *FileMap2::findByAddress(quint32 address)
{
   if (mLastFileIndex>=0)
   {
      if ( (mFiles[mLastFileIndex]->mAddress<=address) && (address < mFiles[mLastFileIndex]->mAddress + mFiles[mLastFileIndex]->mLength ) )
      {
         return mFiles[mLastFileIndex];
      }
   }
   mLastFileIndex=-1;
   int i;
   int numItems=mFiles.length();

   for (i=0;i<numItems;i++)
   {
      if ( (mFiles[i]->mAddress<=address) && (address < mFiles[i]->mAddress + mFiles[i]->mLength ) )
      {
         mLastFileIndex=i;
         return mFiles[i];
      }
   }
   return NULL;
}

File *FileMap2::findByName(const char *name)
{
   ListIterator it;
   ListIterator end = mFiles.end();
   QString qname(name);
   for (it=mFiles.begin(); it!=end; it++)
   {
      if ((*it)->mName == qname)
      {
         return *it;
      }
   }
   return NULL;
}



/**
 * @brief verifies that file fits within the range bound by startAddress and endAddress
 * @param startAddress
 * @param endAddress
 * @return true if successful, false on failure
 */
bool RemoteFile::FileMap2::verifyFileAddress(RemoteFile::File *file, quint32 startAddress, quint32 endAddress)
{
   quint32 fileStartAddress = file->mAddress;
   quint32 fileEndAddress = file->mAddress+file->mLength;
   if ( (fileStartAddress<startAddress) || (fileEndAddress > endAddress) )
   {
      file->mAddress = RMF_INVALID_ADDRESS;
      return false; //file does not fit within bounds
   }
   return true;
}

void RemoteFile::FileMap2::insertDefault(RemoteFile::File *file)
{
   Q_ASSERT(file->mAddress != RMF_INVALID_ADDRESS);
   ///TODO: reimplement this using binary search algorithm. Linear search will work fine for now.
   RemoteFile::FileMap2::ListIterator it = mFiles.begin();
   RemoteFile::FileMap2::ListIterator end = mFiles.end();
   while(it != end)
   {
      if ((*it)->mAddress > file->mAddress)
      {
         mFiles.insert(it,file);
         return;
      }
      it++;
   }
   //we are at the end of the list, insert at end
   mFiles.append(file);
}

/**
 * @brief default address assigner. It tries to find any free slot (equal to or) above 16KiB.
 * @param file
 * @return true on success, false on failure
 */
bool FileMapDefault2::assignFileAddressDefault(File *file)
{
   return assignFileAddress(file, RMF_DATA_HIGH_START_ADDR, RMF_CMD_START_ADDR, mAddressBoundary);
}


}
