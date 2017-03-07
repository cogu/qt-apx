#ifndef QRMF_PROTO_H
#define QRMF_PROTO_H

#include <QtGlobal>
#include "qrmf_file.h"

#define RMF_GREETING_START "RMFP/1.0\n"
#define RMF_NUMHEADER_FORMAT "NumHeader-Format:" //allows values are 16 or 32. This tells other side to expect NumHeader16 or NumHeader32 respectively

#define RMF_ADDR_LEN 4 //the maximum address can be 4 bytes long
#define RMF_FILEINFO_BASE_LEN      48
#define RMF_FILE_OPEN_LEN          8
#define RMF_FILE_CLOSE_LEN         8


namespace RemoteFile
{
   int packHeader(char *pDest, int destLimit, quint32 address, bool more_bit=false);
   int unpackHeader(const char *pBegin, const char *pEnd, quint32 *address, bool *more_bit);

   int packFileInfo(char *pDest, int destLimit, RemoteFile::File &file);
   int unpackFileInfo(const char *pBegin, const char *pEnd, RemoteFile::File &file, bool networkByteOrder=false);

   int packFileOpen(char *pDest, int destLimit, quint32 address);
   int unpackFileOpen(const char *pBegin, const char *pEnd, quint32 &address, bool networkByteOrder=false);

   int packFileClose(char *pDest, int destLimit, quint32 address);
   int unpackFileClose(const char *pBegin, const char *pEnd, quint32 &address, bool networkByteOrder=false);

} //namespace

#endif // QRMF_PROTO_H
