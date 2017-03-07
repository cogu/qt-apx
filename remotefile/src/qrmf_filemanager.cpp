#include <QDebug>
#include <QtEndian>
#include "qrmf_filemanager.h"
#include "qrmf_proto.h"

#define DEFAULT_PACK_BUF_LEN 1024

namespace RemoteFile
{

FileManagerWorker::FileManagerWorker(): mTransmitHandler(NULL)
{

}

void FileManagerWorker::onMessage(Msg msg)
{
   switch(msg.msgType)
   {
   case RMF_MSG_CONNECT:
      mTransmitHandler = (RemoteFile::TransmitHandler*) msg.msgData3;
      break;
   case RMF_MSG_FILEINFO:      
      {
         RemoteFile::File *file = (RemoteFile::File*) msg.msgData3;
         if( (mTransmitHandler != NULL) && (file != NULL) )
         {
            int msgLen = ((int)RMF_ADDR_LEN) + ((int)msg.msgData1);
            char *sendBuffer = mTransmitHandler->getSendBuffer(msgLen);
            if (sendBuffer != NULL)
            {
               char *p=sendBuffer;
               int headerLen = RemoteFile::packHeader(p, msgLen,RMF_CMD_START_ADDR,false);
               p+=headerLen;
               int payloadLen = RemoteFile::packFileInfo(p, msgLen-headerLen, *file);
               Q_ASSERT(payloadLen>0);
               mTransmitHandler->send(0,headerLen+payloadLen);
            }
         }
         delete file;
      }
      break;
   case RMF_MSG_FILEOPEN:
      {
         int maxMsgLen = (int) RMF_ADDR_LEN + RMF_FILE_OPEN_LEN;
         char *sendBuffer = mTransmitHandler->getSendBuffer(maxMsgLen);
         if (sendBuffer != NULL)
         {
            char *p=sendBuffer;
            int headerLen = RemoteFile::packHeader(p, maxMsgLen,RMF_CMD_START_ADDR,false);
            p+=headerLen;
            int payloadLen = RemoteFile::packFileOpen(p, maxMsgLen-headerLen,msg.msgData1);
            Q_ASSERT(payloadLen>0);
            mTransmitHandler->send(0,headerLen+payloadLen);
         }
      }
      break;
   case RMF_MSG_WRITE_DATA:
      {
         quint32 address = (quint32) msg.msgData1;
         QByteArray *dataBytes = (QByteArray*) msg.msgData3;
         if( (mTransmitHandler != NULL) && (dataBytes != NULL) )
         {
            int maxMsgLen = ((int)RMF_ADDR_LEN) + dataBytes->length();
            char *sendBuffer = mTransmitHandler->getSendBuffer(maxMsgLen);
            if (sendBuffer != NULL)
            {
               char *p=sendBuffer;
               int headerLen = RemoteFile::packHeader(p, maxMsgLen,address,false);
               p+=headerLen;
               memcpy(p, dataBytes->constData(), dataBytes->length());
               //note: headerLen can be shorter here than RMF_ADDR_LEN.
               mTransmitHandler->send(0,headerLen+dataBytes->length());
            }
         }
         if (dataBytes !=NULL)
         {
            delete dataBytes;
         }
      }
      break;
   default:
      qDebug() << "Unhandled message" << msg.msgType;
   }
}

FileManager::FileManager(FileMap2 *localFileMap, FileMap2 *remoteFileMap):
   mLocalFileMap(localFileMap),
   mRemoteFileMap(remoteFileMap)
{
   QObject::connect(this, &FileManager::message, &mWorkerThread, &FileManagerWorker::onMessage);
   mWorkerThread.start();
}

FileManager::~FileManager()
{
   mWorkerThread.quit();
   mWorkerThread.wait();
}

void FileManager::attachLocalFile(File *file)
{
   mLocalFileMap->insert(file);
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
      if (file == NULL)
      {
         break;
      }      
      RemoteFile::File *copy = new RemoteFile::File(*file);
      RemoteFile::Msg msg(RMF_MSG_FILEINFO,RMF_FILEINFO_BASE_LEN+copy->mName.size()+1,0, copy);
      emit message(msg);
   }
}

void FileManager::onMsgReceived(const char *msgData, int msgLen)
{
   const char *pNext = msgData;
   const char *pEnd = msgData+msgLen;
   quint32 address;
   bool more_bit;
   int headerLen = RemoteFile::unpackHeader(pNext, pEnd, &address, &more_bit);

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
   }
}

void FileManager::processCmd(const char *pBegin, const char *pEnd)
{
   quint32 cmdType;
   if(pBegin+sizeof(quint32)>pEnd)
   {
      qDebug("[FILEMANAGER] processCmd: invalid message length:%d",(int) (pEnd-pBegin) );
      return;
   }
   cmdType = qFromLittleEndian<quint32>((uchar*)pBegin);
   switch(cmdType)
   {
   case RMF_CMD_FILE_INFO:
      {
         RemoteFile::File *file = new File();
         int result = RemoteFile::unpackFileInfo(pBegin, pEnd, *file); ///TODO: this needs to be deleted later
         if (result <= 0)
         {
            qDebug("[FILEMANAGER] unpackFileInfo failed with :%d",(int) result );            
         }
         else
         {
            int numRequesteFiles = mRequestedFiles.length();
            int index=-1;
            for (int i=0;i<numRequesteFiles;i++)
            {
               if (mRequestedFiles[i]->mName == file->mName)
               {
                  if (mRequestedFiles[i]->mLength != file->mLength)
                  {
                     qDebug("[FILEMANAGER] Requested file \"%s\" but length does not match. Expected %d, got %d",
                            file->mName.toLatin1().constData(), mRequestedFiles[i]->mLength, file->mLength);
                  }
                  else
                  {
                     //this file was previously requested. move information from file pointer to requestedFile pointer
                     RemoteFile::File *requestedFile = mRequestedFiles[i];
                     requestedFile->mAddress=file->mAddress;
                     requestedFile->mFileType=file->mFileType;
                     requestedFile->mDigestType=file->mDigestType;
                     memcpy(&requestedFile->mDigestData[0], &file->mDigestData[0], RMF_DIGEST_SIZE);
                     delete file;
                     file = NULL;
                     requestedFile->isOpen=true;
                     RemoteFile::Msg msg(RMF_MSG_FILEOPEN, requestedFile->mAddress, 0, 0);
                     emit message(msg);
                     index=i;
                     break;
                  }
               }
            }
            if (index >= 0)
            {
               //move item from requested list to remoteFile list
               RemoteFile::File *requestedFile = mRequestedFiles[index];
               mRequestedFiles.removeAt(index);
               mRemoteFileMap->insert(requestedFile);
            }
            else
            {
               mRemoteFileMap->insert(file);
               file->isWeakRef=false; //mark object for delayed deletion
               file = NULL; //prevent early deletion
            }
         }         
         if (file != NULL)
         {
            //set file to NULL to prevent it from getting deleted here
            delete file;
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
            if (file != NULL)
            {
               ///TODO: set flag that file is open here
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
   RemoteFile::File *file = mRemoteFileMap->findByAddress(address);
   if ( (file != 0) && (file->isOpen == true) )
   {
      quint32 offset = address - file->mAddress;
      file->write((const quint8*) data, offset, dataLen);
   }
}

}

