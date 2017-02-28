#ifndef QRMF_MSG_H
#define QRMF_MSG_H

#include <QtGlobal>

#define RMF_MSG_CONNECT              0 //msgData3=TransmitHandler*
#define RMF_MSG_FILEINFO             1 //msgData1=size, msgData3=char[]                     usage: used to send FileInfo to remoteNode
#define RMF_MSG_FILEOPEN             2 //msgData1=(virtual file) address                    usage: used to send FileOpen to remoteNode
#define RMF_MSG_FILECLOSE            3 //msgData1=(virtual file) address                    usage: used to send FileClose to remoteNode
#define RMF_MSG_WRITE_DATA           4 //msgData1=writeAddress, msgData3=QByteArray*        usage: used to write file data at specified address

namespace RemoteFile
{
   struct Msg
   {
      quint32 msgType;      
      quint32 msgData1;
      quint32 msgData2; //two generic quint32 values can be sent in one message
      void *msgData3;   //a void* pointer can also be sent in a message

      Msg(quint32 _msgType, quint32 _msgData1, quint32 _msgData2, void *_msgData3):
         msgType(_msgType),msgData1(_msgData1),msgData2(_msgData2),msgData3(_msgData3){}
   };
}

#endif // QRMF_MSG_H
