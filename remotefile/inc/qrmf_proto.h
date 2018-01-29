#ifndef QRMF_PROTO_H
#define QRMF_PROTO_H

#include <QtGlobal>
#include "qrmf_file.h"

namespace RemoteFile
{
   int packHeader(char *pDest, int destLimit, quint32 address, bool more_bit=false);
   int unpackHeader(const char *pBegin, const char *pEnd, quint32 *address, bool *more_bit);

   int packFileInfo(char *pDest, int destLimit, const RemoteFile::File &file);
   int unpackFileInfo(const char *pBegin, const char *pEnd, RemoteFile::File &file, bool networkByteOrder=false);

   int packFileOpen(char *pDest, int destLimit, quint32 address);
   int unpackFileOpen(const char *pBegin, const char *pEnd, quint32 &address, bool networkByteOrder=false);

   int packFileClose(char *pDest, int destLimit, quint32 address);
   int unpackFileClose(const char *pBegin, const char *pEnd, quint32 &address, bool networkByteOrder=false);

} //namespace

#endif // QRMF_PROTO_H
