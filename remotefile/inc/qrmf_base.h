#ifndef QRMF_BASE_H
#define QRMF_BASE_H

#include <QtGlobal>
#define RMF_GREETING_START "RMFP/1.0\n"
#define RMF_NUMHEADER_FORMAT "NumHeader-Format:" //allows values are 16 or 32. This tells other side to expect NumHeader16 or NumHeader32 respectively

#define RMF_CMD_ACK                (quint32) 0   //reserved for future use
#define RMF_CMD_NACK               (quint32) 1   //reserved for future use
#define RMF_CMD_EOT                (quint32) 2   //reserved for future use
#define RMF_CMD_FILE_INFO          (quint32) 3   //serialized file info data structure
#define RMF_CMD_REVOKE_FILE        (quint32) 4   //used by server to tell clients that the file is no longer available
//range 5-9 reserved for future use
#define RMF_CMD_FILE_OPEN         (quint32) 10  //opens a file
#define RMF_CMD_FILE_CLOSE        (quint32) 11  //closes a file

#define RMF_DIGEST_SIZE          32u //32 bytes is suitable for storing a sha256 hash
#define RMF_MAXLEN_FILE_NAME     255u

#define RMF_DATA_LOW_MIN_ADDR ((quint32)0x0)               //0
#define RMF_DATA_LOW_MAX_ADDR ((quint32)0x3FFF)            //16KB
#define RMF_DATA_HIGH_START_ADDR ((quint32)0x4000)         //16KB
#define RMF_DATA_HIGH_MAX_ADDR ((quint32)0x3FFFFBFF)       //1GB
#define RMF_CMD_START_ADDR      ((quint32) 0x3FFFFC00)
#define RMF_CMD_END_ADDR        ((quint32) 0x3FFFFFFF)
#define RMF_INVALID_ADDRESS     0xFFFFFFFF
#define RMF_CMD_HIGH_BIT ((quint32) 0x80000000)
#define RMF_CMD_MORE_BIT ((quint32) 0x40000000)

#define RMF_LOW_ADDRESS_SIZE       2u
#define RMF_HIGH_ADDRESS_SIZE      4u
#define RMF_FILEINFO_BASE_LEN      48
#define RMF_FILE_OPEN_LEN          8
#define RMF_FILE_CLOSE_LEN         8
#define RMF_CMD_TYPE_LEN           4

namespace RemoteFile
{

class TransmitHandler
{
public:
   TransmitHandler(){}
   virtual ~TransmitHandler(){}
   virtual int getSendAvail() = 0; //transmitHandler shall return how many bytes can be provided by getSendBuffer.
   virtual char* getSendBuffer(int msgLen) = 0; //transmitHandler shall attempt to allocate a buffer of appropriate length. Returns NULL on failure.
   virtual int send(int offset, int msgLen) = 0; //buffer is provided by transmit handler. offset is the offset from return-value of getSendBuffer where data was written
};

class ReceiveHandler
{
public:
   ReceiveHandler(){}
   virtual ~ReceiveHandler(){}
   virtual void onMsgReceived(const char *msgData, int msgLen) = 0;
   virtual void onConnected(RemoteFile::TransmitHandler *transmitHandler) = 0;
};

}

#endif // QRMF_BASE_H
