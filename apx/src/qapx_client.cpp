#include "qapx_client.h"
#include <QtGlobal>

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
   createLocalNode(apxText.toUtf8().constData());
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
   (void) nodeData;
   QString name(port->getName());
   emit requirePortData(port->getPortIndex(), name, value);
}

void Client::setProvidePort(int portId, QVariant &value)
{
   mNodeData.setProvidePort(portId,value);
}

int Client::findProvidePortId(QString &name)
{
   return mNodeData.findProvidePortId(qUtf8Printable(name));
}

int Client::findProvidePortId(const char *name)
{
   return mNodeData.findProvidePortId(name);
}

int Client::findRequirePortId(QString &name)
{
   return mNodeData.findRequirePortId(qUtf8Printable(name));
}

int Client::findRequirePortId(const char *name)
{
   return mNodeData.findRequirePortId(name);
}


}
