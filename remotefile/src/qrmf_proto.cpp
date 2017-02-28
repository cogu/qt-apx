#include "qrmf_proto.h"
#include "qrmf_base.h"
#include <QtEndian>


namespace RemoteFile
{

/**
 * @brief packs address and more_bit into buffer
 * @param pDest
 * @param destLimit
 * @param more_bit
 * @return number of bytes written into pDest
 */
int packHeader(char *pDest, int destLimit, quint32 address, bool more_bit)
{
   int retval = 0; //default is to write 0 bytes intp pDest
   uchar *p = (uchar*) pDest;
   if(address<16384)
   {
      //address will fit into 16 bits
      if (destLimit>=(int)sizeof(quint16))
      {
         retval = (int)sizeof(quint16);
         qToBigEndian<quint16>((quint16) address, p);
         if (more_bit == true)
         {
            p[0]|=0x40u; //activate MORE_BIT
         }
      }
   }
   else if(address < 1073741824)
   {
      if (destLimit>=(int)sizeof(quint32))
      {
         retval = (int)sizeof(quint32);
         qToBigEndian<quint32>(address, p);
         p[0]|=0x80u; //activate LONG_BIT
         if (more_bit == true)
         {
            p[0]|=0x40u; //activate MORE_BIT
         }
      }
   }
   else
   {
      //one or more arguments are invalid
      retval = -1;
   }
   return retval;
}


/**
 * @brief unpacks address and more_bit from buffer
 * @param pBegin
 * @param pEnd
 * @param address
 * @param more_bit
 * @return number of bytes parsed. Possible return values are -1 (invalid arguments), 0 (buffer too small), 2 and 4.
 */
int unpackHeader(const char *pBegin, const char *pEnd, quint32 *address, bool *more_bit)
{
   int retval = 0; //default is to assume that the buffer is too small to parse
   const char *pNext=pBegin;
   if( (pBegin != 0) && (pEnd != 0) && (address != NULL) && (more_bit != NULL))
   {
      if (pBegin+1<pEnd)
      {
         char c = *pNext;
         pNext+=2;
         if(c & 0x80u) //is high_bit set?
         {
            if(pNext+1<pEnd) //at least 2 more bytes available?
            {
               retval = (int) sizeof(quint32);
               quint32 tmp = qFromBigEndian<quint32>((const uchar*) pBegin);
               tmp&=(quint32)0x3FFFFFFFu; //clear the high_bit and more_bit (we still have a copy of more_bit in char c above)
               *address=tmp;
            }
         }
         else
         {
            retval = (int) sizeof(quint16);
            quint32 tmp = (quint32) qFromBigEndian<quint16>((const uchar*) pBegin);
            tmp &=0x3FFF;
            *address=tmp;
         }
         *more_bit = (c & 0x40)? true : false; //is more_bit set?
      }
   }
   else
   {
      //one or more arguments are invalid
      retval = -1;
   }
   return retval;
}

/**
 * @brief packs FILEINFO struct into buffer
 * @param pDest
 * @param destLimit
 * @param file
 * @return number of bytes written into pDest. special values are -1 (invalid arguments) and 0 (destLimit is too small)
 */
int packFileInfo(char *pDest, int destLimit, File &file)
{
   int retval = 0;
   if ( (pDest != 0) && (destLimit >= 0) )
   {
      int nameLen = file.mName.length();
      int needed = RMF_FILEINFO_BASE_LEN + nameLen+1; //+1 for null-terminator
      if (needed <= destLimit)
      {
         uchar *p = (uchar*) pDest;
         qToLittleEndian<quint32>(RMF_CMD_FILE_INFO, p); p+=sizeof(quint32);
         qToLittleEndian<quint32>(file.mAddress, p); p+=sizeof(quint32);
         qToLittleEndian<quint32>(file.mLength, p); p+=sizeof(quint32);
         qToLittleEndian<quint16>(file.mFileType, p); p+=sizeof(quint16);
         qToLittleEndian<quint16>(file.mDigestType, p); p+=sizeof(quint16);
         memcpy(p,file.mDigestData,RMF_DIGEST_SIZE); p+=RMF_DIGEST_SIZE;
         memcpy(p, file.mName.toLatin1().data(), nameLen); p+=nameLen;
         p[0]=0;
         return needed;
      }
   }
   else
   {
      retval = -1;
   }
   return retval;
}

/**
 * @brief unpacks FileInfo struct from byte array. It stores parsed data in a File object
 * @param pBegin
 * @param pEnd
 * @param file
 * @return number of bytes parsed. Special return values are: -2 (invalid struct type),  -1 (invalid arguments), and 0 (buffer too small)
 */
int unpackFileInfo(const char *pBegin, const char *pEnd, File &file, bool networkByteOrder)
{
   int retval = 0; //default is to assume that the buffer is too small to parse
   const uchar *pNext = (uchar*) pBegin;
   const uchar *puEnd = (const uchar*) pEnd;
   if( (pBegin != 0) && (pEnd != 0) )
   {
      if (pBegin+RMF_FILEINFO_BASE_LEN<=pEnd)
      {
         if (networkByteOrder)
         {
            quint32 cmdType = qFromBigEndian<quint32>(pNext); pNext+=sizeof(quint32);
            if (cmdType == RMF_CMD_FILE_INFO)
            {
               file.mAddress = qFromBigEndian<quint32>(pNext); pNext+=sizeof(quint32);
               file.mLength = qFromBigEndian<quint32>(pNext); pNext+=sizeof(quint32);
               file.mFileType = (RemoteFile::FileType) qFromBigEndian<quint16>(pNext); pNext+=sizeof(quint16);
               file.mDigestType = (RemoteFile::DigestType) qFromBigEndian<quint16>(pNext); pNext+=sizeof(quint16);
            }
            else
            {
               retval = -2; //not a RMF_CMD_FILE_INFO struct
            }
         }
         else
         {
            quint32 cmdType = qFromLittleEndian<quint32>(pNext); pNext+=sizeof(quint32);
            if (cmdType == RMF_CMD_FILE_INFO)
            {
               file.mAddress = qFromLittleEndian<quint32>(pNext); pNext+=sizeof(quint32);
               file.mLength = qFromLittleEndian<quint32>(pNext); pNext+=sizeof(quint32);
               file.mFileType = (RemoteFile::FileType) qFromLittleEndian<quint16>(pNext); pNext+=sizeof(quint16);
               file.mDigestType = (RemoteFile::DigestType) qFromLittleEndian<quint16>(pNext); pNext+=sizeof(quint16);
            }
            else
            {
               retval = -2; //not a RMF_CMD_FILE_INFO struct
            }
         }
         if (retval >= 0)
         {
            int remain=0;
            int nameLen=0;
            const uchar *pMark;
            memcpy(&file.mDigestData[0],pNext,RMF_DIGEST_SIZE); pNext+=RMF_DIGEST_SIZE;
            //treat remaining bytes as the file name
            pMark = pNext;
            remain = (int) (puEnd-pNext);
            if (remain>(int)RMF_MAXLEN_FILE_NAME)
            {
               nameLen=RMF_MAXLEN_FILE_NAME;
               pNext+=RMF_MAXLEN_FILE_NAME+1;
            }
            else if (remain>0)
            {
               pNext=puEnd-1;
            }
            else
            {
               pNext=puEnd;
            }
            if (remain>0)
            {
               nameLen=(int)(pNext-pMark);
               if (*pNext != 0)
               {
                  nameLen+=1; //character instead of null-terminator
               }
               pNext++;
            }
            file.mName = QString::fromLatin1((const char*) pMark, nameLen);
            retval = RMF_FILEINFO_BASE_LEN+(int)(pNext-pMark);
         }
      }
   }
   else
   {
      retval = -1;
   }
   return retval;
}

/**
 * @brief packs FileOpen struct into pDest
 * @param pDest
 * @param destLimit
 * @param address
 * @return number of bytes written into pDest. special values are -1 (invalid arguments) and 0 (destLimit is too small)
 */
int packFileOpen(char *pDest, int destLimit, quint32 address)
{
   int retval = 0;
   if ( pDest != 0 )
   {
      int needed = (int) sizeof(quint32)*2;
      if (needed <= destLimit)
      {
         uchar *pNext = (uchar*) pDest;
         qToLittleEndian<quint32>(RMF_CMD_FILE_OPEN, pNext); pNext+=sizeof(quint32);
         qToLittleEndian<quint32>(address, pNext);
         retval=needed;
      }
   }
   else
   {
      retval = -1;
   }
   return retval;
}

/**
 * @brief unpacks FileClose struct from byte array.
 * @param pBegin
 * @param pEnd
 * @param file
 * @return number of bytes parsed. Special return values are: -2, (invalid struct type),  -1 (invalid arguments), and 0 (buffer too small)
 */
int unpackFileOpen(const char *pBegin, const char *pEnd, quint32 &address, bool networkByteOrder)
{
   int retval = 0;
   const uchar *pNext = (uchar*) pBegin;
   if( (pBegin != 0) && (pEnd != 0) )
   {
      if (pBegin+RMF_FILE_OPEN_LEN<=pEnd)
      {
         if (networkByteOrder)
         {
            quint32 cmdType = qFromBigEndian<quint32>(pNext); pNext+=sizeof(quint32);
            if (cmdType == RMF_CMD_FILE_OPEN)
            {
               address = qFromBigEndian<quint32>(pNext); pNext+=sizeof(quint32);
            }
            else
            {
               retval = -2; //not a RMF_CMD_FILE_OPEN struct
            }
         }
         else
         {
            quint32 cmdType = qFromLittleEndian<quint32>(pNext); pNext+=sizeof(quint32);
            if (cmdType == RMF_CMD_FILE_OPEN)
            {
               address = qFromLittleEndian<quint32>(pNext); pNext+=sizeof(quint32);
            }
            else
            {
               retval = -2; //not a RMF_CMD_FILE_OPEN struct
            }
         }
         if (retval>=0)
         {
            retval = RMF_FILE_OPEN_LEN;
         }
      }
   }
   else
   {
      retval = -1;
   }
   return retval;
}

/**
 * @brief packs FileClose struct into pDest
 * @param pDest
 * @param destLimit
 * @param address
 * @return number of bytes written into pDest. special values are -1 (invalid arguments) and 0 (destLimit is too small)
 */
int packFileClose(char *pDest, int destLimit, quint32 address)
{
   int retval = 0;
   if ( pDest != 0 )
   {
      int needed = (int) sizeof(quint32)*2;
      if (needed <= destLimit)
      {
         uchar *pNext = (uchar*) pDest;
         qToLittleEndian<quint32>(RMF_CMD_FILE_CLOSE, pNext); pNext+=sizeof(quint32);
         qToLittleEndian<quint32>(address, pNext);
         retval=needed;
      }
   }
   else
   {
      retval = -1;
   }
   return retval;
}

/**
 * @brief unpacks FileClose struct from byte array.
 * @param pBegin
 * @param pEnd
 * @param file
 * @return number of bytes parsed. Special return values are: -2, (invalid struct type),  -1 (invalid arguments), and 0 (buffer too small)
 */
int unpackFileClose(const char *pBegin, const char *pEnd, quint32 &address, bool networkByteOrder)
{
   int retval = 0;
   const uchar *pNext = (uchar*) pBegin;
   if( (pBegin != 0) && (pEnd != 0) )
   {
      if (pBegin+RMF_FILE_CLOSE_LEN<=pEnd)
      {
         if (networkByteOrder)
         {
            quint32 cmdType = qFromBigEndian<quint32>(pNext); pNext+=sizeof(quint32);
            if (cmdType == RMF_CMD_FILE_CLOSE)
            {
               address = qFromBigEndian<quint32>(pNext); pNext+=sizeof(quint32);
            }
            else
            {
               retval = -2; //not a RMF_CMD_FILE_CLOSE struct
            }
         }
         else
         {
            quint32 cmdType = qFromLittleEndian<quint32>(pNext); pNext+=sizeof(quint32);
            if (cmdType == RMF_CMD_FILE_CLOSE)
            {
               address = qFromLittleEndian<quint32>(pNext); pNext+=sizeof(quint32);
            }
            else
            {
               retval = -2; //not a RMF_CMD_FILE_CLOSE struct
            }
         }
         if (retval>=0)
         {
            retval = RMF_FILE_CLOSE_LEN;
         }
      }
   }
   else
   {
      retval = -1;
   }
   return retval;
}

} //namespace
