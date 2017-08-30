#ifndef QAPX_CLIENT_H
#define QAPX_CLIENT_H

#include <QObject>
#include <QString>
#include <QVariant>
#include "qapx_nodedata.h"
#include "qremotefile.h"
#include <QHostAddress>
#include "qapx_filemap.h"
#include "qremotefile.h"
#include "qapx_datavm.h"

namespace Apx
{

   class Client : public QObject, public NodeHandler
   {
      Q_OBJECT
   public:
      Client(QObject *parent=NULL);
      virtual ~Client();

      void createLocalNode(const char *apxText);
      void createLocalNode(QString &apxText);
      void connectTcp(QHostAddress address,quint16 port);
      void close();

      //NodeHandler API
      void inPortDataNotification(NodeData *nodeData, QApxSimplePort *port, QVariant &value);

      //client user API
      Q_DECL_DEPRECATED void setProvidePort(int portId, QVariant &value);
      void setProvidePortValue(int portId, QVariant &value);
      int findProvidePortId(QString &name);
      int findProvidePortId(const char *name);
      int findRequirePortId(QString &name);
      int findRequirePortId(const char *name);

      Apx::NodeData *getNodeData() {return &mNodeData;}

   protected:
      NodeData mNodeData;
      Apx::FileMap mLocalFileMap;
      Apx::FileMap mRemoteFileMap;
      RemoteFile::FileManager *mFileManager;
      RemoteFile::SocketAdapter *mSocketAdapter;
      Apx::DataVM mUnpackVM; //virtual machine used for unpacking data
      Apx::DataVM mPackVM; //virtual machine used for packing data (in case we are running in a multi-threaded environment)

private slots:
      void onConnected();
      void onDisconnected();

  signals:
      void requirePortData(int portId, QString &portName, QVariant &value);
      void connected();
      void disconnected();
   };
}

#endif // QAPX_CLIENT_H
