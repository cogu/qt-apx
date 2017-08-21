#include <cstring>
#include <QDebug>
#include "qapxbytearrayparser.h"
#include "qapx_nodedata.h"
#include "qapx_exception.h"
#include "qapx_compiler.h"

using namespace std;

/**
 * @brief NodeData constructir
 * @param nodeText a text string containing the node definition in APX/Text format (see http://apx.readthedocs.io/en/latest/apx_text.html)
 */
Apx::NodeData::NodeData(const char *apxText):
   mNode(NULL), mInPortDataFile(NULL), mOutPortDataFile(NULL), mDefinitionFile(NULL),
   mInPortDataMap(NULL),mOutPortDataMap(NULL),mInPortDataMapLen(0),mOutPortDataMapLen(0),
   mNodeHandler(NULL)
{
   if (apxText != NULL)
   {
      parse(apxText);
   }
}

Apx::NodeData::NodeData(QString &apxText):
   mNode(NULL), mInPortDataFile(NULL), mOutPortDataFile(NULL), mDefinitionFile(NULL),
   mInPortDataMap(NULL),mOutPortDataMap(NULL),mInPortDataMapLen(0),mOutPortDataMapLen(0),
   mNodeHandler(NULL)
{
   parse(apxText);
}

Apx::NodeData::~NodeData()
{
   cleanup();
}

void Apx::NodeData::parse(const char *apxText)
{
   QByteArray bytes(apxText);
   cleanup();
   processNode(bytes);
}

void Apx::NodeData::parse(QString &apxText)
{
   QByteArray bytes(apxText.toLatin1());
   cleanup();
   processNode(bytes);
}

/**
 * @brief called from ApxInputFile after a file was written to
 * @param offset
 * @param length
 */
void Apx::NodeData::inPortDataWriteNotify(quint32 offset, QByteArray &data)
{

   if ( offset+data.length() <= (quint32)mInPortDataMapLen)
   {
      quint32 endOffset=offset+(quint32) data.length();
      while(offset<endOffset)
      {
         PortDataElement* dataElement = mInPortDataMap[offset];
         if (dataElement != NULL)
         {
            int portIndex = dataElement->port->getPortIndex();
            QVariant value;
            QVariantMap map;
            QVariantList list;
            PackUnpackProg unpackInfo = mInPortUnpackProg.at(portIndex);
            int exception = VM_EXCEPTION_NO_EXCEPTION;

            switch(unpackInfo.vtype)
            {
            case VTYPE_SCALAR:
               exception = mUnpackVM.exec(unpackInfo.prog,data,value);
               break;
            case VTYPE_MAP:
               exception = mUnpackVM.exec(unpackInfo.prog,data,map);
               if(exception == VM_EXCEPTION_NO_EXCEPTION)
               {
                  value = map;
               }
               break;
            case VTYPE_LIST:
               exception = mUnpackVM.exec(unpackInfo.prog,data,list);
               if(exception == VM_EXCEPTION_NO_EXCEPTION)
               {
                  value = list;
               }
               break;
            default:
               break;
            }
            if (exception != VM_EXCEPTION_NO_EXCEPTION)
            {
               qDebug("[APX] exception caught in DataVM: %d",exception);
            }
            else
            {
               ///TODO: can this be achieved with the extra copy to value?
               if (mNodeHandler != NULL)
               {
                  mNodeHandler->inPortDataNotification(this, dataElement->port, value);
               }
            }
            offset+=dataElement->length;
         }
         else
         {
            break;
         }
      }
   }
}

/**
 * @brief returns the correspondng id of provide port with matching name
 * @param name
 * @return non-negative id of provide port. -1 on failure.
 */
int Apx::NodeData::findProvidePortId(const char *name) const
{
   if ( (mNode == NULL) || (name == NULL) )
   {
      return -1;
   }
   int numProvidePorts = mNode->getNumProvidePorts();
   for (int i= 0;i<numProvidePorts;i++)
   {
      QApxSimplePort *port = mNode->getProvidePortById(i);
      Q_ASSERT(port != NULL);
      if(strcmp(name, port->getName())==0)
      {
         return i;
      }
   }
   return -1;
}


/**
 * @brief writes value to provide port with corresponding port id
 * @param portId
 * @param value
 */
bool Apx::NodeData::setProvidePort(int portId, QVariant &value)
{
   bool retval = false;
   QByteArray serializedData;
   if ( (portId >= 0) && (portId < mOutPortPackProg.length()) )
   {
      const PackUnpackProg &progInfo = mOutPortPackProg.at(portId);
      int exception = VM_EXCEPTION_INVALID_VARIANT_TYPE;
      switch(progInfo.vtype)
      {
      case VTYPE_SCALAR:
         exception = mPackVM.exec(progInfo.prog,serializedData,value);
         break;
      case VTYPE_MAP:
         if (value.canConvert<QVariantMap>())
         {
            QVariantMap map = value.toMap();
            exception = mPackVM.exec(progInfo.prog,serializedData,map);
         }
         break;
      case VTYPE_LIST:
         if (value.canConvert<QVariantMap>())
         {
            QVariantList list = value.toList();
            exception = mPackVM.exec(progInfo.prog,serializedData,list);
         }
         break;
      default:
         break;
      }
      if (exception == VM_EXCEPTION_INVALID_VARIANT_TYPE)
      {
         QString message("invalid variant type");
         message.append(value.typeName());
         throw Apx::VariantTypeException(message.toLatin1().constData());
      }
      else if(exception != VM_EXCEPTION_NO_EXCEPTION)
      {
         QString message("error code=");
         message.append(QString::number(exception));
         throw Apx::VMException(message.toLatin1().constData());
      }
      else
      {         
         retval = true;
         //successfully serialized data into serializedData array
         writeProvidePortRaw(portId, serializedData.constData(), serializedData.length());
      }
   }
   return retval;
}

/**
 * @brief returns the correspondng id of the require port with matching name
 * @param name
 * @return non-negative id of require port. -1 on failure.
 */
int Apx::NodeData::findRequirePortId(const char *name) const
{
   if ( (mNode == NULL) || (name == NULL) )
   {
      return -1;
   }
   int numRequirePorts = mNode->getNumRequirePorts();
   for (int i= 0;i<numRequirePorts;i++)
   {
      QApxSimplePort *port = mNode->getRequirePortById(i);
      Q_ASSERT(port != NULL);
      if(strcmp(name, port->getName())==0)
      {
         return i;
      }
   }
   return -1;
}

int Apx::NodeData::getNumRequirePorts() const
{
   if (mNode != NULL)
   {
      return mNode->getNumRequirePorts();
   }
   return -1;
}

int Apx::NodeData::getNumProvidePorts() const
{
   if (mNode != NULL)
   {
      return mNode->getNumProvidePorts();
   }
   return -1;
}

QApxSimplePort *Apx::NodeData::getRequirePortById(int id) const
{
   if (mNode != NULL)
   {
      return mNode->getRequirePortById(id);
   }
   return (QApxSimplePort*) NULL;
}

QApxSimplePort *Apx::NodeData::getProvidePortById(int id) const
{
   if (mNode != NULL)
   {
      return mNode->getProvidePortById(id);
   }
   return (QApxSimplePort*) NULL;
}


void Apx::NodeData::processNode(QByteArray &bytes)
{
   QApxByteArrayParser byteArrayParser;
   QApxDataElementParser dataElementParser;
   Apx::DataCompiler compiler;
   mNode = byteArrayParser.parseNode(bytes);
   if (mNode == NULL)
   {
      throw Apx::ParseException("syntax error");
   }
   int numRequirePorts = mNode->getNumRequirePorts();
   int numProvidePorts = mNode->getNumProvidePorts();
   int i;
   int inputLen=0;
   int outputLen=0;
   for (i=0;i<numRequirePorts;i++)
   {
      QApxSimplePort *port = mNode->getRequirePortById(i);
      Q_ASSERT(port != NULL);
      port->setPortIndex(i);
      QApxDataElement *pElement = dataElementParser.parseDataSignature((const quint8*) port->getDataSignature());
      Q_ASSERT(pElement != NULL);
      PortDataElement elem(port, (quint32) inputLen, (quint32) pElement->packLen);
      mInPortDataElements.append(elem);
      inputLen+=pElement->packLen;
      QByteArray unpack_prog;
      int result = compiler.genUnpackData(unpack_prog, pElement);
      if (result == 0)
      {
         mInPortUnpackProg.append(PackUnpackProg(pElement, unpack_prog));
      }
      else
      {
         QString msg("failed to generate byte code for port ");
         msg.append(port->getName());
         throw Apx::CompilerException(msg.toLatin1().constData());
      }
      delete pElement;
   }   
   for (i=0;i<numProvidePorts;i++)
   {
      QApxSimplePort *port = mNode->getProvidePortById(i);
      Q_ASSERT(port != NULL);
      port->setPortIndex(i);
      QApxDataElement *pElement = dataElementParser.parseDataSignature((const quint8*) port->getDataSignature());
      Q_ASSERT(pElement != NULL);      
      mOutPortDataElements.append(PortDataElement(port, (quint32) outputLen, (quint32) pElement->packLen));
      outputLen+=pElement->packLen;      
      QByteArray pack_prog;
      int result = compiler.genPackData(pack_prog, pElement);
      if (result == 0)
      {
         mOutPortPackProg.append(PackUnpackProg(pElement, pack_prog));
      }
      else
      {
         QString msg("failed to generate byte code for port ");
         msg.append(port->getName());
         throw Apx::CompilerException(msg.toLatin1().constData());
      }
      delete pElement;
   }
   if (inputLen > 0)
   {
      mInPortDataFile = new Apx::InputFile(mNode->getName()+QString(".in"), inputLen);
      mInPortDataFile->setNodeDataHandler(this);
   }
   if (outputLen > 0)
   {
      mOutPortDataFile = new Apx::OutputFile(mNode->getName()+QString(".out"), outputLen);
   }
   mDefinitionFile = new Apx::OutputFile(mNode->getName()+QString(".apx"), bytes.length());
   mDefinitionFile->write((const quint8*) bytes.constData(), 0, (quint32) bytes.length());

   populatePortDataMap();

}

void Apx::NodeData::cleanup()
{
   if (mDefinitionFile != NULL) {delete mDefinitionFile;}
   if (mOutPortDataFile != NULL) {delete mOutPortDataFile;}
   if (mInPortDataFile != NULL) {delete mInPortDataFile;}
   if (mNode != NULL) {delete mNode;}
   if (mInPortDataMap != NULL) {delete[] mInPortDataMap;}
   if (mOutPortDataMap != NULL) {delete[] mOutPortDataMap;}
}

void Apx::NodeData::populatePortDataMap()
{
   if (mNode != 0)
   {
      if (mInPortDataFile != NULL)
      {
         mInPortDataMapLen = mInPortDataFile->mLength; //number of elements, not number of bytes
         size_t numBytes = mInPortDataMapLen*sizeof(PortDataElement**);
         mInPortDataMap = new PortDataElement*[mInPortDataMapLen];
         PortDataElement **ppNext = mInPortDataMap;
         Q_ASSERT(mInPortDataMap != NULL);
         PortDataElement **ppEnd = mInPortDataMap+numBytes;
         for (int i=0;i<mInPortDataElements.length();i++)
         {            
            Q_ASSERT(ppNext<=ppEnd);
            for (quint32 j=0; j<mInPortDataElements[i].length; j++)
            {
               *ppNext++=&mInPortDataElements[i];
               Q_ASSERT(ppNext<=ppEnd);
            }
         }
      }
      if (mOutPortDataFile != NULL)
      {
         mOutPortDataMapLen = mOutPortDataFile->mLength; //number of elements, not number of bytes
         size_t numBytes = mOutPortDataMapLen*sizeof(PortDataElement**);
         mOutPortDataMap = new PortDataElement*[mOutPortDataMapLen];
         PortDataElement **ppNext = mOutPortDataMap;
         Q_ASSERT(mOutPortDataMap != NULL);
         PortDataElement **ppEnd = mOutPortDataMap+numBytes;
         for (int i=0;i<mOutPortDataElements.length();i++)
         {
            for (quint32 j=0; j<mOutPortDataElements[i].length; j++)
            {
               *ppNext++=&mOutPortDataElements[i];
               Q_ASSERT(ppNext<=ppEnd);
            }
         }
      }
   }
}

/**
 * @brief writes serialized bytes directly to the output file
 * @param portId
 * @param pSrc
 * @param length
 */
void Apx::NodeData::writeProvidePortRaw(int portId, const char *pSrc, int length)
{   
   if ( (portId >= 0) && (portId < mOutPortDataElements.length()) )
   {
      const PortDataElement &dataElement = mOutPortDataElements.at(portId);
      if ( dataElement.length == (quint32)length)
      {
         mOutPortDataFile->write((const quint8*) pSrc, dataElement.offset, dataElement.length);
      }
      else
      {
         Q_ASSERT(0); ///maybe throw an exception here?
      }
   }
}


Apx::PortDataElement::PortDataElement(QApxSimplePort *_port, quint32 _offset, quint32 _length):
   port(_port), offset(_offset),length(_length){}

Apx::PackUnpackProg::PackUnpackProg(QApxDataElement *pElement, QByteArray _prog)
   :prog(_prog)
{
   Q_ASSERT(pElement != 0);
   if(pElement->baseType == QAPX_BASE_TYPE_NONE)
   {
      throw Apx::ParseException("invalid baseType");
   }
   else if(pElement->baseType == QAPX_BASE_TYPE_STRING)
   {
      vtype = VTYPE_SCALAR;
   }
   else if(pElement->baseType == QAPX_BASE_TYPE_RECORD)
   {
      vtype = VTYPE_MAP;
   }
   else
   {
      if (pElement->arrayLen>0)
      {
         vtype = VTYPE_LIST;
      }
      else
      {
         vtype = VTYPE_SCALAR;
      }
   }
}
