#ifndef QAPX_NODEDATA_H
#define QAPX_NODEDATA_H
#include <QtGlobal>
#include <QString>
#include <QByteArray>
#include <QList>
#include "qapx_file.h"
#include "qapxsimplenode.h"

namespace Apx
{

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

   protected:
      void processNode(QByteArray &bytes);
      void cleanup();
      void populatePortDataMap();
#ifdef UNIT_TEST
   public:
#else
   protected:
#endif
      QApxSimpleNode *mNode;
      Apx::InputFile *mInPortDataFile; //represents the .in file of the node. Is set to NULL in case this node does not have any require ports.
      Apx::OutputFile *mOutPortDataFile; //represents the .out file of the node. Is set to NULL in case this node does not have any provide ports.
      Apx::OutputFile *mDefinitionFile; //represents the .apx file of the node. this is constructed from the nodeText string durint construction.
      QList<PortDataElement> mInPortDataElements; //one PortDataElement per require port of the node
      QList<PortDataElement> mOutPortDataElements; //one PortDataElement per provide port of the node
      PortDataElement** mInPortDataMap; //offset to PortDataElement* mapping of mInDataElements
      PortDataElement** mOutPortDataMap; //offset to PortDataElement* mapping of mOutDataElements
      int mInPortDataMapLen;
      int mOutPortDataMapLen;
   };
}


#endif // QAPX_NODEDATA_H
