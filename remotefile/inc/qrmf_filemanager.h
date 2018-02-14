#ifndef QRMF_FILEMANAGER_H
#define QRMF_FILEMANAGER_H

#include <QtGlobal>
#include <QObject>
#include "qrmf_filemap2.h"
#include "qrmf_msg.h"
#include "qrmf_base.h"


namespace RemoteFile
{
   class FileManager : public QObject, public RemoteFile::ReceiveHandler
   {
      Q_OBJECT
   public:
      FileManager(RemoteFile::FileMap2 *localFileMap=NULL, RemoteFile::FileMap2 *remoteFileMap=NULL);
      ~FileManager();
      void attachLocalFile(RemoteFile::File *file);
      void requestRemoteFile(RemoteFile::File *file);
      void onConnected(RemoteFile::TransmitHandler *transmitHandler);
      void onDisconnected();
      bool onMsgReceived(const char *msgData, int msgLen);
      void outPortDataWriteNotify(File *file, const quint8 *pSrc, quint32 offset, quint32 length);
   protected:
      void processCmd(const char *pBegin, const char *pEnd);
      void processFileWrite(quint32 address, bool more_bit, const char *data, quint32 dataLen);
   protected:
      RemoteFile::TransmitHandler *mTransmitHandler;
      RemoteFile::FileMap2 *mLocalFileMap;
      RemoteFile::FileMap2 *mRemoteFileMap;
      QList<RemoteFile::File*> mRequestedFiles;

   private slots:
      void onMessage(const RemoteFile::Msg& msg);
   signals:
      void message(const RemoteFile::Msg& msg);
      void remoteFileFullWrite(const QString& fileName);
   };
}

#endif // QRMF_FILEMANAGER_H
