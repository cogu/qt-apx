#include "qapx_client.h"
#include <QtGlobal>

namespace Apx
{

Client::Client(QObject *parent, bool inPortNotifyWithName)
  : QObject(parent)
  , mInPortNotifyWithName(inPortNotifyWithName)
  , mInPortDataNotifications(0)
  , mNodeData()
  , mLocalFileMap()
  , mRemoteFileMap()
  , mSocketAdapter(nullptr)
  , mUnpackVM()
  , mPackVM()
{
   mFileManager = new RemoteFile::FileManager(&mLocalFileMap, &mRemoteFileMap);
}

Client::~Client()
{
   delete mFileManager;
   if (mSocketAdapter != nullptr)
   {
      delete mSocketAdapter;
   }
}

void Client::createLocalNode(const char *apxText)
{
   mNodeData.load(apxText);
   InputFile *inPortDataFile = mNodeData.getInPortDataFile();
   OutputFile *outPortDataFile = mNodeData.getOutPortDataFile();
   OutputFile *definitionDataFile = mNodeData.getDefinitionFile();
   if (outPortDataFile != nullptr)
   {
      mFileManager->attachLocalFile(outPortDataFile);
   }
   if (definitionDataFile != nullptr)
   {
      mFileManager->attachLocalFile(definitionDataFile);
   }
   if (inPortDataFile != nullptr)
   {
      mFileManager->requestRemoteFile(inPortDataFile);
      QObject::connect(mFileManager, SIGNAL(remoteFileFullWrite(const QString&)), this, SLOT(onRemoteFileFullWrite(const QString&)));
   }
   mNodeData.setNodeHandler(this);
}

void Client::createLocalNode(const QString &apxText)
{
   createLocalNode(apxText.toUtf8().constData());
}

void Client::connectTcp(const QHostAddress& address, quint16 port)
{
   if (mSocketAdapter == nullptr)
   {
      mSocketAdapter = new RemoteFile::SocketAdapter(32,this);
      mSocketAdapter->setReceiveHandler(mFileManager);
      QObject::connect(mSocketAdapter, SIGNAL(connected()), this, SLOT(onConnected()));
      QObject::connect(mSocketAdapter, SIGNAL(disconnected()), this, SLOT(onDisconnected()));
   }
   mSocketAdapter->connectTcp(address, port);
}

void Client::close()
{
   if (mSocketAdapter != nullptr)
   {
      mSocketAdapter->close();
   }
}

void Client::inPortDataNotification(NodeData *nodeData, QApxSimplePort *port, const QVariant &value)
{
   (void) nodeData;
   mInPortDataNotifications++;
   if (mInPortNotifyWithName)
   {
      emit requirePortData(port->getPortIndex(), QString(port->getName()), value);
   }
   else
   {
      emit requirePortDataIdOnly(port->getPortIndex(), value);
   }
}

Q_DECL_DEPRECATED void Client::setProvidePort(int portId, QVariant &value)
{
   setProvidePortValue(portId, value);
}

void Client::setProvidePortValue(int portId, QVariant &value)
{
   mNodeData.setProvidePortValue(portId,value);
}

int Client::findProvidePortId(const QString &name) const
{
   return mNodeData.findProvidePortId(qUtf8Printable(name));
}

int Client::findProvidePortId(const char* const name) const
{
   return mNodeData.findProvidePortId(name);
}

int Client::findRequirePortId(const QString &name) const
{
   return mNodeData.findRequirePortId(qUtf8Printable(name));
}

int Client::findRequirePortId(const char* const name) const
{
   return mNodeData.findRequirePortId(name);
}

void Client::onConnected()
{
   emit connected();
}

void Client::onDisconnected()
{
   emit disconnected();
}

void Client::onRemoteFileFullWrite(const QString& fileName)
{
   Q_UNUSED(fileName);
   emit requirePortsFullyRefreshed();
}


}
