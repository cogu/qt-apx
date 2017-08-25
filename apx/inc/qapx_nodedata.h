#ifndef QAPX_NODEDATA_H
#define QAPX_NODEDATA_H
#include <QtGlobal>
#include <QString>
#include <QByteArray>
#include <QList>
#include "qapx_file.h"
#include "qapx_node.h"
#include "qapx_vmbase.h"
#include "qapxdataelement.h"
#include "qapx_datavm.h"

namespace Apx
{

   //forward declaration
   class NodeData;

    /**
    * @brief Data Information about a single port
    */
   class PortDataElement
   {
   public:
      QApxSimplePort *port;
      quint32 offset;
      quint32 length;
      PortDataElement(QApxSimplePort *_port,  quint32 _offset, quint32 _length);
   };

   struct PackUnpackProg
   {
      VariantType vtype; //what QVariant it expects
      QByteArray  prog; //byte code program
      PackUnpackProg(QApxDataElement *pElement, QByteArray _prog);
   };

   /**
    * @brief The InPortDataHandler represents the application layer
    */
   class NodeDataHandler
   {
   public:
      virtual void inPortDataWriteNotify(quint32 offset, QByteArray &data) = 0;
      ///TODO: add more handlers here for when ports are connected/disconnected etc.
   };

   /**
    * @brief The NodeHandler is used by Apx::Client to register as a listener of one or more nodes
    */
   class NodeHandler
   {
   public:
      virtual void inPortDataNotification(NodeData *nodeData, QApxSimplePort *port, QVariant &value) = 0;
   };


   /**
    * @brief The NodeData class is the container for the APX files.
    * The APX Files are:
    *   - nodename.in : Apx::InputFile (stored in mInPortDataFile)
    *   - nodename.out : Apx::OutputFile (mOutPortDataFile)
    *   - nodename.apx : Apx::OutputFile (mNodeDefinitionFile)
    *
    * The member mInPortDataFile is NULL after construction if the node has no require ports
    * The member mOutPortDataFile is NULL after construction if the node has no provide ports
    * mNodeDefinitionFile is created in constructor using the constructor argument
    */
   class NodeData : public NodeDataHandler
   {
   public:
      NodeData(const char *apxText=NULL);
      NodeData(QString &apxText);
      virtual ~NodeData();

      void parse(const char *apxText);
      void parse(QString &apxText);

      //NodeDataHandler API
      void inPortDataWriteNotify(quint32 offset, QByteArray &data);

      //Getters and setters
      Apx::InputFile *getInPortDataFile() {return mInPortDataFile;}
      Apx::OutputFile *getOutPortDataFile() {return mOutPortDataFile;}
      Apx::OutputFile *getDefinitionFile() {return mDefinitionFile;}
      void setNodeHandler(NodeHandler *handler){mNodeHandler=handler;}
      NodeHandler *getNodeHandler(){return mNodeHandler;}
      int findProvidePortId(const char *name) const;
      bool setProvidePort(int portId, QVariant &value);
      int findRequirePortId(const char *name) const;
      int getNumRequirePorts() const;
      int getNumProvidePorts() const;
      QApxSimplePort *getRequirePortById(int id) const; //NOTE: QApxSimplePort will soon be refactored. The new name will be Apx::Port
      QApxSimplePort *getProvidePortById(int id) const; //NOTE: QApxSimplePort will soon be refactored. The new name will be Apx::Port


   protected:
      void processNode(QByteArray &bytes);
      void cleanup();
      void populatePortDataMap();
      void writeProvidePortRaw(int portId,const char *pSrc, int length);
#ifdef UNIT_TEST
   public:
#else
   protected:
#endif
      Apx::Node *mNode;
      Apx::InputFile *mInPortDataFile; //represents the .in file of the node. Is set to NULL in case this node does not have any require ports.
      Apx::OutputFile *mOutPortDataFile; //represents the .out file of the node. Is set to NULL in case this node does not have any provide ports.
      Apx::OutputFile *mDefinitionFile; //represents the .apx file of the node. this is constructed from the nodeText string durint construction.
      QList<PortDataElement> mInPortDataElements; //one PortDataElement per require port of the node
      QList<PortDataElement> mOutPortDataElements; //one PortDataElement per provide port of the node
      PortDataElement** mInPortDataMap; //offset to PortDataElement* mapping of mInDataElements
      PortDataElement** mOutPortDataMap; //offset to PortDataElement* mapping of mOutDataElements
      int mInPortDataMapLen; //number of elements in mInPortDataMap
      int mOutPortDataMapLen; //number of elements in mOutPortDataMap
      QList<PackUnpackProg> mInPortUnpackProg; //byte code programs for unpacking require port data to QVariant (one program per port)
      QList<PackUnpackProg> mOutPortPackProg; //byte code programs for packing provide port data to QVariant (one program per port)
      Apx::DataVM mUnpackVM; //virtual machine used for unpacking data
      Apx::DataVM mPackVM; //virtual machine used for packing data (in case we are running in a multi-threaded environment)
      NodeHandler *mNodeHandler;

   };



} //namespace


#endif // QAPX_NODEDATA_H
