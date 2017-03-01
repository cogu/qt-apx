#include "qapx_client.h"

namespace Apx
{

Client::Client(QObject *parent) : QObject(parent),
   mFileManager(NULL), mSocketAdapter(NULL)
{
   mFileManager = new RemoteFile::FileManager(&mLocalFileMap, &mRemoteFileMap);
}

void Client::createLocalNode(const char *apxText)
{
   mNodeData.parse(apxText);
   InputFile *inPortDataFile = mNodeData.getInPortDataFile();
   OutputFile *outPortDataFile = mNodeData.getOutPortDataFile();
   OutputFile *definitionDataFile = mNodeData.getDefinitionFile();
   if (outPortDataFile != NULL)
   {
      mFileManager->attachLocalFile(outPortDataFile);
   }
   if (definitionDataFile != NULL)
   {
      mFileManager->attachLocalFile(definitionDataFile);
   }
   if (inPortDataFile != NULL)
   {
      mFileManager->requestRemoteFile(inPortDataFile);
   }
   mNodeData.setNodeHandler(this);
}

void Client::createLocalNode(QString &apxText)
{
   mNodeData.parse(apxText);
}

void Client::connectTcp(QHostAddress address, quint16 port)
{
   if (mSocketAdapter == NULL)
   {
      mSocketAdapter = new RemoteFile::SocketAdapter(32,this);
      mSocketAdapter->setReceiveHandler(mFileManager);
   }
   mSocketAdapter->connectTcp(address, port);
}

void Client::inPortDataNotification(NodeData *nodeData, QApxSimplePort *port, QVariant &value)
{
   QString name(port->getName());
   emit requirePortData(port->getPortIndex(), name, value);
}

void Client::setProvidePort(int portId, QVariant &value)
{
   mNodeData.setProvidePort(portId,value);
}

int Client::getProvidePortId(QString &name)
{
   return getProvidePortId(name.toLatin1().constData());
}

int Client::getProvidePortId(const char *name)
{
   return mNodeData.getProvidePortId(name);
}


}
