#ifndef QAPX_CLIENT_H
#define QAPX_CLIENT_H

#include <QObject>
#include <QString>
#include "qapx_nodedata.h"
#include "qremotefile.h"
#include <QHostAddress>
#include "qapx_filemap.h"
#include "qremotefile.h"

namespace Apx
{

   class Client : public QObject
   {
      Q_OBJECT
   public:
      Client(QObject *parent=NULL);
      virtual ~Client(){}

      void createLocalNode(const char *apxText);
      void createLocalNode(QString &apxText);
      void connectTcp(QHostAddress address,quint16 port);

   protected:
      NodeData mNodeData;
      Apx::FileMap mLocalFileMap;
      Apx::FileMap mRemoteFileMap;
      RemoteFile::FileManager *mFileManager;
      RemoteFile::SocketAdapter *mSocketAdapter;

  signals:
      void requirePortData(int portId, QString &portName, QByteArray &portData);

   public slots:
      void onRequirePortData(int portIndex);

   };
}

#endif // QAPX_CLIENT_H
