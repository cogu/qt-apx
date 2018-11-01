#include <QDebug>
#include <QtEndian>
#include "qrmf_filemanager.h"
#include "qrmf_proto.h"

namespace RemoteFile
{

void FileManager::onMessage(const Msg& msg)
{
   switch(msg.msgType)
   {
   case RMF_MSG_CONNECT:
      mTransmitHandler = (RemoteFile::TransmitHandler*) msg.msgData3;
      break;
   case RMF_MSG_FILEINFO:
      {
         const RemoteFile::File* const file = (RemoteFile::File*) msg.msgData3;
         if( (mTransmitHandler != nullptr) && (file != nullptr) )
         {
            const int msgLen = ((int)RMF_HIGH_ADDRESS_SIZE) + ((int)msg.msgData1);
            char *sendBuffer = mTransmitHandler->getSendBuffer(msgLen);
            if (sendBuffer != nullptr)
            {
               int headerLen = RemoteFile::packHeader(sendBuffer, msgLen,RMF_CMD_START_ADDR,false);
               sendBuffer+=headerLen;
               int payloadLen = RemoteFile::packFileInfo(sendBuffer, msgLen-headerLen, *file);
               Q_ASSERT(payloadLen>0);
               mTransmitHandler->send(0,headerLen+payloadLen);
            }
         }
         if (file != nullptr)
         {
            delete file;
         }
      }
      break;
   case RMF_MSG_FILEOPEN:
      {
         const int maxMsgLen = (int) RMF_HIGH_ADDRESS_SIZE + RMF_FILE_OPEN_LEN;
         char *sendBuffer = mTransmitHandler->getSendBuffer(maxMsgLen);
         if (sendBuffer != nullptr)
         {
            const int headerLen = RemoteFile::packHeader(sendBuffer, maxMsgLen,RMF_CMD_START_ADDR,false);
            sendBuffer+=headerLen;
            const int payloadLen = RemoteFile::packFileOpen(sendBuffer, maxMsgLen-headerLen,msg.msgData1);
            Q_ASSERT(payloadLen>0);
            mTransmitHandler->send(0,headerLen+payloadLen);
         }
      }
      break;
   case RMF_MSG_WRITE_DATA:
      {
         quint32 address = (quint32) msg.msgData1;
         const QByteArray* const dataBytes = (QByteArray*) msg.msgData3;
         if( (mTransmitHandler != nullptr) && (dataBytes != nullptr) )
         {
            const int payloadLen = dataBytes->length();
            const int maxMsgLen = ((int)RMF_HIGH_ADDRESS_SIZE) + payloadLen;
            char *sendBuffer = mTransmitHandler->getSendBuffer(maxMsgLen);
            if (sendBuffer != nullptr)
            {
               char *p=sendBuffer;
               int headerLen = RemoteFile::packHeader(p, maxMsgLen,address,false);
               p+=headerLen;
               memcpy(p, dataBytes->constData(), payloadLen);
               //note: headerLen can be shorter here than RMF_ADDR_LEN.
               const int result = mTransmitHandler->send(0,headerLen+payloadLen);
               if (result<0)
               {
                  qWarning() << "[RMF_FILE_MANAGER] send error" << result;
               }
            }
         }
         if (dataBytes !=nullptr)
         {
            delete dataBytes;
         }
      }
      break;
   default:
      qDebug() << "[RMF_FILE_MANAGER] Unhandled message" << msg.msgType;
   }
}

FileManager::FileManager(FileMap2 *localFileMap, FileMap2 *remoteFileMap):
   mTransmitHandler(nullptr),
   mLocalFileMap(localFileMap),
   mRemoteFileMap(remoteFileMap),
   mRequestedFiles()
{
   QObject::connect(this, &FileManager::message, this, &FileManager::onMessage);
}

FileManager::~FileManager()
{
}

void FileManager::attachLocalFile(File *file)
{
   mLocalFileMap->insert(file);
   file->mFileManager = this;
}

void FileManager::requestRemoteFile(File *file)
{
   mRequestedFiles.append(file);
}

void FileManager::onConnected(TransmitHandler *transmitHandler)
{   
   mLocalFileMap->iterInit();
   RemoteFile::Msg connectMsg(RMF_MSG_CONNECT,0,0, (void*) transmitHandler);
   emit message(connectMsg);
   while(true)
   {
      RemoteFile::File *file = mLocalFileMap->next();
      if (file == nullptr)
      {
         break;
      }
      RemoteFile::File *copy = new RemoteFile::File(*file);
      RemoteFile::Msg msg(RMF_MSG_FILEINFO,RMF_FILEINFO_BASE_LEN+copy->mName.size()+1,0, copy);
      emit message(msg);
   }
}

void FileManager::onDisconnected()
{
   mRemoteFileMap->clear();
}

bool FileManager::onMsgReceived(const char *msgData, int msgLen)
{
   bool retval = false;
   const char *pNext = msgData;
   const char *pEnd = msgData+msgLen;
   quint32 address;
   bool more_bit;
   const int headerLen = RemoteFile::unpackHeader(pNext, pEnd, &address, &more_bit);

   if (headerLen > 0)
   {
      pNext+=headerLen;
      int dataLen = msgLen-headerLen;
      if (address == RMF_CMD_START_ADDR)
      {
         processCmd(pNext,pEnd);
      }
      else if (address < RMF_CMD_START_ADDR)
      {
         processFileWrite(address,more_bit,pNext,dataLen);
      }
      retval = true;
   }
   return retval;
}

void FileManager::outPortDataWriteNotify(RemoteFile::File *file, const quint8 *pSrc, quint32 offset, quint32 length)
{
   QByteArray *buf = new QByteArray((const char*)pSrc,length);
   RemoteFile::Msg msg(RMF_MSG_WRITE_DATA,file->mAddress+offset,0,(void*) buf);
   emit message(msg);
}

void FileManager::processCmd(const char *pBegin, const char *pEnd)
{
   quint32 cmdType;
   if(pBegin+sizeof(quint32)>pEnd)
   {
      qDebug("[RMF_FILE_MANAGER] processCmd: invalid message length:%d",(int) (pEnd-pBegin) );
      return;
   }
   cmdType = qFromLittleEndian<quint32>((uchar*)pBegin);
   switch(cmdType)
   {
   case RMF_CMD_FILE_INFO:
      {
         RemoteFile::File *remoteFile = new File();
         int result = RemoteFile::unpackFileInfo(pBegin, pEnd, *remoteFile);
         if (result <= 0)
         {
            qDebug("[RMF_FILE_MANAGER] unpackFileInfo failed with :%d",(int) result );
            delete remoteFile;
         }
         else
         {
            int numRequesteFiles = mRequestedFiles.length();
            for (int i=0;i<numRequesteFiles;i++)
            {
               RemoteFile::File* requestedFile = mRequestedFiles[i];
               if (requestedFile->mName == remoteFile->mName)
               {
                  if (requestedFile->mLength != remoteFile->mLength)
                  {
                     qDebug("[FILEMANAGER] Requested file \"%s\" but length does not match. Expected %d, got %d",
                            remoteFile->mName.toLatin1().constData(), requestedFile->mLength, remoteFile->mLength);
                  }
                  else
                  {
                     requestedFile->mAddress=remoteFile->mAddress;
                     requestedFile->mFileType=remoteFile->mFileType;
                     requestedFile->mDigestType=remoteFile->mDigestType;
                     //switch out remoteFile and requestedFile (requestedFile is of an inherited class)
                     requestedFile->isWeakRef=true; //prevents deletion by mRemoteFileMap object
                     delete remoteFile; //switch out remoteFile
                     remoteFile = requestedFile; //switch in requestedFile
                     remoteFile->isOpen=true;
                     RemoteFile::Msg msg(RMF_MSG_FILEOPEN, remoteFile->mAddress, 0, nullptr);
                     emit message(msg);
                     break;
                  }
               }
            }
            mRemoteFileMap->insert(remoteFile);
         }
      }
      break;
   case RMF_CMD_FILE_OPEN:
      {
         quint32 startAddress;
         int result = RemoteFile::unpackFileOpen(pBegin, pEnd, startAddress);
         if (result <= 0)
         {
            qDebug("[RMF_FILE_MANAGER] unpackFileOpen failed with :%d",(int) result );
         }
         else
         {
            RemoteFile::File *file = mLocalFileMap->findByAddress(startAddress);
            if (file != nullptr)
            {
               file->isOpen=true;
               QByteArray *fileContent = new QByteArray(file->mLength,0);
               int result = file->read((quint8*) fileContent->data(), 0, (quint32) file->mLength);
               if (result != (int) file->mLength)
               {
                  Q_ASSERT(0);
               }
               RemoteFile::Msg msg(RMF_MSG_WRITE_DATA,file->mAddress,0,(void*) fileContent);
               qDebug("[RMF_FILE_MANAGER] remoteNode opened file \"%s\"@%08X, length=%d",file->mName.toLatin1().constData(),file->mAddress, file->mLength);
               emit message(msg);
            }
         }
      }
      break;
   case RMF_CMD_FILE_CLOSE:
      break;
   }
}

void FileManager::processFileWrite(quint32 address, bool more_bit, const char *data, quint32 dataLen)
{
   Q_UNUSED(more_bit);
   RemoteFile::File *file = mRemoteFileMap->findByAddress(address);
   if ( (file != nullptr) && (file->isOpen == true) )
   {
      quint32 offset = address - file->mAddress;
      const int write_result = file->write((const quint8*) data, offset, dataLen);
      if (write_result!=(int)dataLen)
      {
         qDebug("[RMF_FILE_MANAGER] Incomplete write for address @%08X, %d %u", address, write_result, dataLen);
      }
      else if (write_result == (int)file->mLength)
      {
         emit remoteFileFullWrite(file->mName);
      }
   }
   else
   {
      qDebug("[RMF_FILE_MANAGER] Ignoring write for address not in opened file @%08X", address);
   }
}

}

