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
      Client(QObject *parent=NULL, bool inPortNotifyWithName=true);
      virtual ~Client();

      void createLocalNode(const char *apxText);
      void createLocalNode(const QString &apxText);
      void connectTcp(const QHostAddress& address,quint16 port);
      void close();

      //NodeHandler API
      void inPortDataNotification(NodeData *nodeData, QApxSimplePort *port, const QVariant &value);

      //client user API
      Q_DECL_DEPRECATED void setProvidePort(int portId, QVariant &value);
      void setProvidePortValue(int portId, QVariant &value);
      int findProvidePortId(const QString &name) const;
      int findProvidePortId(const char* const name) const;
      int findRequirePortId(const QString &name) const;
      int findRequirePortId(const char* const name) const;

      Apx::NodeData *getNodeData() {return &mNodeData;}

   protected:
      bool mInPortNotifyWithName;
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
      void onRemoteFileFullWrite(const QString& fileName);

  signals:
      void requirePortData(int portId, const QString &portName, const QVariant &value);
      void requirePortDataIdOnly(int portId, const QVariant &value);
      void connected();
      void disconnected();
      // Emitted when all ports have been updated in one message
      void requirePortsFullyRefreshed();
   };
}

#endif // QAPX_CLIENT_H
