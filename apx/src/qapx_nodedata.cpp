#include <cstring>
#include <QDebug>
#include "qapx_bytearrayparser.h"
#include "qapx_nodedata.h"
#include "qapx_exception.h"
#include "qapx_compiler.h"

using namespace std;

/**
 * @brief NodeData constructir
 * @param nodeText a text string containing the node definition in APX/Text format (see http://apx.readthedocs.io/en/latest/apx_text.html)
 */
Apx::NodeData::NodeData(const char *apxText):
   mNode(nullptr), mInPortDataFile(nullptr), mOutPortDataFile(nullptr), mDefinitionFile(nullptr),
   mInPortDataMap(nullptr),mOutPortDataMap(nullptr),mInPortDataMapLen(0),mOutPortDataMapLen(0),
   mNodeHandler(nullptr)
{
   if (apxText != nullptr)
   {
    QByteArray bytes(apxText);
    mNode = parseNode(bytes);
    processNode(bytes);
   }
}

Apx::NodeData::NodeData(QString &apxText):
   mNode(nullptr), mInPortDataFile(nullptr), mOutPortDataFile(nullptr), mDefinitionFile(nullptr),
   mInPortDataMap(nullptr),mOutPortDataMap(nullptr),mInPortDataMapLen(0),mOutPortDataMapLen(0),
   mNodeHandler(nullptr)
{
   QByteArray bytes(apxText.toLatin1());
   mNode = parseNode(bytes);
   processNode(bytes);
}

Apx::Node* Apx::NodeData::load(const char *apxText)
{
   cleanup();
   QByteArray bytes(apxText);
   mNode = parseNode(bytes);
   processNode(bytes);
   return mNode;
}

Apx::Node* Apx::NodeData::load(QString &apxText)
{
   cleanup();
   QByteArray bytes(apxText.toLatin1());
   mNode = parseNode(bytes);
   processNode(bytes);
   return mNode;
}

Apx::NodeData::~NodeData()
{
   cleanup();
}

bool Apx::NodeData::getRequirePortValue(const QApxSimplePort *port, QVariant &value)
{
   if (port != nullptr)
   {
      return getRequirePortValue(port->getPortIndex(), value);
   }
   return false;
}

bool Apx::NodeData::getRequirePortValue(int portIndex, QVariant &value)
{
   if ((mNode != nullptr) && (portIndex>=0) && (portIndex < mNode->getNumRequirePorts()))
   {
      Q_ASSERT(mInPortDataElements.length() == mNode->getNumRequirePorts());
      const PortDataElement &dataElement = mInPortDataElements.at(portIndex);
      quint32 offset = dataElement.offset;
      quint32 length = dataElement.length;
      QByteArray tmpData;
      tmpData.resize(length);
      if ((int)length == mInPortDataFile->read((quint8*) tmpData.data(), offset, length))
      {
         return getRequirePortValueInternal(portIndex, tmpData, value);
      }
   }
   return false;
}




/**
 * @brief called from ApxInputFile after a file was written to
 * @param offset
 * @param length
 */
void Apx::NodeData::inPortDataWriteNotify(quint32 offset, QByteArray &data)
{
   quint32 dataLen = (quint32) data.length();
   const quint32 endOffset=offset+dataLen;
   if ( endOffset <= (quint32)mInPortDataMapLen)
   {
      QVariant value;
      while(offset<endOffset)
      {
         const PortDataElement* const dataElement = mInPortDataMap[offset];
         if (dataElement != nullptr)
         {
            if (dataElement->length <= dataLen)
            {
               const int portIndex = dataElement->port->getPortIndex();
               const bool result = getRequirePortValueInternal(portIndex, data, value);
               if ( (result == true) && (mNodeHandler != nullptr) )
               {
                  mNodeHandler->inPortDataNotification(this, dataElement->port, value);
               }
               if ( dataLen > dataElement->length)
               {
                  data.remove(0, dataElement->length);
                  dataLen -= dataElement->length;
               }
               offset+=dataElement->length;
            }
            else
            {
               break; //QByteArray is shorter than offset
            }
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
int Apx::NodeData::findProvidePortId(const char* const name) const
{
   if ( (mNode == nullptr) || (name == nullptr) )
   {
      return -1;
   }
   int numProvidePorts = mNode->getNumProvidePorts();
   for (int i= 0;i<numProvidePorts;i++)
   {
      QApxSimplePort *port = mNode->getProvidePortById(i);
      Q_ASSERT(port != nullptr);
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
bool Apx::NodeData::setProvidePortValue(int portId, QVariant &value)
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
         if (value.canConvert<QVariantList>())
         {
            QVariantList list = value.toList();
            exception = mPackVM.exec(progInfo.prog,serializedData,list);
         }
         break;
      case VTYPE_INVALID: // Intentional fallthru
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
int Apx::NodeData::findRequirePortId(const char* const name) const
{
   if ( (mNode == nullptr) || (name == nullptr) )
   {
      return -1;
   }
   int numRequirePorts = mNode->getNumRequirePorts();
   for (int i= 0;i<numRequirePorts;i++)
   {
      QApxSimplePort *port = mNode->getRequirePortById(i);
      Q_ASSERT(port != nullptr);
      if(strcmp(name, port->getName())==0)
      {
         return i;
      }
   }
   return -1;
}

int Apx::NodeData::getNumRequirePorts() const
{
   if (mNode != nullptr)
   {
      return mNode->getNumRequirePorts();
   }
   return -1;
}

int Apx::NodeData::getNumProvidePorts() const
{
   if (mNode != nullptr)
   {
      return mNode->getNumProvidePorts();
   }
   return -1;
}

QApxSimplePort *Apx::NodeData::getRequirePortById(int id) const
{
   if (mNode != nullptr)
   {
      return mNode->getRequirePortById(id);
   }
   return nullptr;
}

QApxSimplePort *Apx::NodeData::getProvidePortById(int id) const
{
   if (mNode != nullptr)
   {
      return mNode->getProvidePortById(id);
   }
   return nullptr;
}

Apx::Node* Apx::NodeData::parseNode(QByteArray &bytes)
{
   ByteArrayParser byteArrayParser;
   Apx::Node *node = byteArrayParser.parseNode(bytes);
   if (node == nullptr)
   {
      throw Apx::ParseException("syntax error");
   }
   return node;
}


void Apx::NodeData::processNode(QByteArray &bytes)
{
   QApxDataElementParser dataElementParser;
   Apx::DataCompiler compiler;
   const int numRequirePorts = mNode->getNumRequirePorts();
   const int numProvidePorts = mNode->getNumProvidePorts();
   int i;
   int inputLen=0;
   int outputLen=0;
   QByteArray pack_unpack_prog;

   mInPortUnpackProg.reserve(numRequirePorts);
   mInPortDataElements.reserve(numRequirePorts);
   for (i=0;i<numRequirePorts;i++)
   {
      QApxSimplePort *port = mNode->getRequirePortById(i);
      Q_ASSERT(port != nullptr);
      port->setPortIndex(i);
      QApxDataElement *pElement = dataElementParser.parseDataSignature((const quint8*) port->getDataSignature());
      Q_ASSERT(pElement != nullptr);
      PortDataElement elem(port, (quint32) inputLen, (quint32) pElement->packLen);
      mInPortDataElements.append(elem);
      inputLen+=pElement->packLen;
      pack_unpack_prog.clear();
      int result = compiler.genUnpackData(pack_unpack_prog, pElement);
      if (result == 0)
      {
         mInPortUnpackProg.append(PackUnpackProg(pElement, pack_unpack_prog));
      }
      delete pElement;
      if (result != 0)
      {
         QString msg("failed to generate byte code for port ");
         msg.append(port->getName());
         throw Apx::CompilerException(msg.toLatin1().constData());
      }
   }
   mOutPortPackProg.reserve(numProvidePorts);
   mOutPortDataElements.reserve(numProvidePorts);
   for (i=0;i<numProvidePorts;i++)
   {
      QApxSimplePort *port = mNode->getProvidePortById(i);
      Q_ASSERT(port != nullptr);
      port->setPortIndex(i);
      QApxDataElement *pElement = dataElementParser.parseDataSignature((const quint8*) port->getDataSignature());
      Q_ASSERT(pElement != nullptr);
      mOutPortDataElements.append(PortDataElement(port, (quint32) outputLen, (quint32) pElement->packLen));
      outputLen+=pElement->packLen;
      pack_unpack_prog.clear();
      int result = compiler.genPackData(pack_unpack_prog, pElement);
      if (result == 0)
      {
         mOutPortPackProg.append(PackUnpackProg(pElement, pack_unpack_prog));
      }
      delete pElement;
      if (result != 0)
      {
         QString msg("failed to generate byte code for port ");
         msg.append(port->getName());
         throw Apx::CompilerException(msg.toLatin1().constData());
      }
   }
   if (inputLen > 0)
   {
      mInPortDataFile = new Apx::InputFile(mNode->getName()+Apx::File::cInSuffix, inputLen);
      mInPortDataFile->setNodeDataHandler(this);
   }
   if (outputLen > 0)
   {
      mOutPortDataFile = new Apx::OutputFile(mNode->getName()+Apx::File::cOutSuffix, outputLen);
   }
   mDefinitionFile = new Apx::OutputFile(mNode->getName()+Apx::File::cDefinitionSuffix, bytes.length());
   mDefinitionFile->write((const quint8*) bytes.constData(), 0, (quint32) bytes.length());

   populatePortDataMap();
}

void Apx::NodeData::cleanup()
{
   if (mDefinitionFile != nullptr) {delete mDefinitionFile; mDefinitionFile = nullptr;}
   if (mOutPortDataFile != nullptr) {delete mOutPortDataFile; mOutPortDataFile = nullptr;}
   if (mInPortDataFile != nullptr) {delete mInPortDataFile; mInPortDataFile = nullptr;}
   if (mNode != nullptr) {delete mNode; mNode = nullptr;}
   if (mInPortDataMap != nullptr) {delete[] mInPortDataMap; mInPortDataMap = nullptr;}
   if (mOutPortDataMap != nullptr) {delete[] mOutPortDataMap; mOutPortDataMap = nullptr;}
}


void Apx::NodeData::populatePortDataMap()
{
   if (mNode != nullptr)
   {
      if (mInPortDataFile != nullptr)
      {
         mInPortDataMapLen = mInPortDataFile->mLength; //number of elements, not number of bytes
         mInPortDataMap = new PortDataElement*[mInPortDataMapLen];
         PortDataElement **ppNext = mInPortDataMap;
         Q_ASSERT(mInPortDataMap != nullptr);
         for (int i=0;i<mInPortDataElements.length();i++)
         {
            PortDataElement& currPortElem = mInPortDataElements[i];
            for (quint32 j=0; j<currPortElem.length; j++)
            {
               Q_ASSERT(ppNext<(mInPortDataMap+mInPortDataMapLen*sizeof(PortDataElement**)));
               *ppNext++=&currPortElem;
            }
         }
      }
      if (mOutPortDataFile != nullptr)
      {
         mOutPortDataMapLen = mOutPortDataFile->mLength; //number of elements, not number of bytes
         mOutPortDataMap = new PortDataElement*[mOutPortDataMapLen];
         PortDataElement **ppNext = mOutPortDataMap;
         Q_ASSERT(mOutPortDataMap != nullptr);
         for (int i=0;i<mOutPortDataElements.length();i++)
         {
            PortDataElement& currPortElem = mOutPortDataElements[i];
            for (quint32 j=0; j<currPortElem.length; j++)
            {
               Q_ASSERT(ppNext<(mOutPortDataMap+mOutPortDataMapLen*sizeof(PortDataElement**)));
               *ppNext++=&currPortElem;
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

Apx::PackUnpackProg::PackUnpackProg(QApxDataElement *pElement, const QByteArray& _prog)
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

bool Apx::NodeData::getRequirePortValueInternal(int portIndex, QByteArray &data, QVariant &value)
{
   QVariantMap* tempMap = nullptr;
   QVariantList* tempList = nullptr;
   if ((mNode != nullptr) && (portIndex >= 0) && (portIndex < mNode->getNumRequirePorts()))
   {
      const PackUnpackProg& unpackInfo = mInPortUnpackProg.at(portIndex);
      int exception = VM_EXCEPTION_INVALID_VARIANT_TYPE;

      switch(unpackInfo.vtype)
      {
      case VTYPE_SCALAR:
         exception = mUnpackVM.exec(unpackInfo.prog,data,value);
         break;
      case VTYPE_MAP:
         tempMap = new QVariantMap();
         exception = mUnpackVM.exec(unpackInfo.prog,data,*tempMap);
         if(exception == VM_EXCEPTION_NO_EXCEPTION)
         {
            value = *tempMap;
         }
         delete tempMap;
         break;
      case VTYPE_LIST:
         tempList = new QVariantList();
         exception = mUnpackVM.exec(unpackInfo.prog,data,*tempList);
         if(exception == VM_EXCEPTION_NO_EXCEPTION)
         {
            value = *tempList;
         }
         delete tempList;
         break;
      case VTYPE_INVALID: // Intentional fallthru
      default:
         break;
      }
      if (exception != VM_EXCEPTION_NO_EXCEPTION)
      {
         qDebug("[APX] exception caught in DataVM: %d",exception);
         return false;
      }
      return true;
   }
   return false;
}
