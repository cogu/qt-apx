#include "qapx_datavm.h"
#include "qapx_vmbase.h"
#include <QtEndian>
#include <QDebug>

namespace Apx
{

QVariantPtr::QVariantPtr():
scalar(nullptr),map(nullptr),list(nullptr),type(VTYPE_INVALID)
{

}

QVariantPtr::QVariantPtr(QVariant *s, QVariantMap *m, QVariantList *l):
   scalar(s),map(m),list(l)
{
   if (s != nullptr)
   {
      type=VTYPE_SCALAR;
   }
   else if(m!=nullptr)
   {
      type=VTYPE_MAP;
   }
   else if (l!=nullptr)
   {
      type=VTYPE_LIST;
   }
   else
   {
      type=VTYPE_INVALID;
   }
}


DataVM::State::State():
   arrayIndex(-1),value()
{
}

DataVM::State::State(QVariant *v):arrayIndex(-1),value(v,nullptr,nullptr)
{
}

DataVM::State::State(QVariantMap *v):arrayIndex(-1),value(nullptr,v,nullptr)
{
}

DataVM::State::State(QVariantList *v):arrayIndex(-1),value(nullptr,nullptr,v)
{

}

void DataVM::State::cleanup()
{
   switch(value.type)
   {
   case VTYPE_INVALID:
      break;
   case VTYPE_SCALAR:
      if(value.scalar!=nullptr) { delete value.scalar; }
      break;
   case VTYPE_MAP:
      if(value.map!=nullptr) { delete value.map; }
      break;
   case VTYPE_LIST:
      if(value.list!=nullptr) { delete value.list; }
      break;
   }
}



DataVM::DataVM():
   mRawData(nullptr),mReadBegin(nullptr),mReadEnd(nullptr),mReadNext(nullptr),mWriteNext(nullptr),mWriteEnd(nullptr),mMode(PROG_TYPE_PACK)
{

}

int DataVM::exec(const QByteArray &prog, QByteArray &rawData, QVariant &value)
{
   mState = DataVM::State(&value);
   mRawData=&rawData;
   return execProg(prog);
}

/**
 * @brief QApxDataVM::exec
 * @param prog
 * @param rawData
 * @param value
 * @return 0 on success or exception code on failure
 */
int DataVM::exec(const QByteArray &prog, QByteArray &rawData, QVariantMap &value)
{   
   mState = DataVM::State(&value);
   mRawData=&rawData;
   return execProg(prog);
}

int DataVM::execProg(const QByteArray &prog)
{
   int exception = VM_EXCEPTION_NO_EXCEPTION;
   const char *pBegin=prog.constData();
   const char *pEnd=pBegin+prog.length();
   const char *pNext=pBegin;

   while(pNext<pEnd)
   {
      int opCode;
      const char* const pMark=pNext;
      pNext=parseOpCode(pNext,pEnd,opCode);
      if (pNext>pMark)
      {
         pNext = execOperation(opCode,pNext,pEnd,exception);
         if ( exception != VM_EXCEPTION_NO_EXCEPTION )
         {
            break;
         }
      }
      else
      {
         exception = VM_EXCEPTION_PROGRAM_PARSE_ERROR;
         break;
      }
   }
   return exception;
}

int DataVM::exec(const QByteArray &prog, QByteArray &rawData, QVariantList &value)
{
   mState = DataVM::State(&value);
   mRawData=&rawData;
   return execProg(prog);
}

const char *DataVM::exceptionToStr(int exception)
{
   switch(exception)
   {
   case VM_EXCEPTION_NO_EXCEPTION: return "VM_EXCEPTION_NO_EXCEPTION";
   case VM_EXCEPTION_NOT_IMPLEMENTED: return "VM_EXCEPTION_NOT_IMPLEMENTED";
   case VM_EXCEPTION_INVALID_OP_CODE: return "VM_EXCEPTION_INVALID_OP_CODE";
   case VM_EXCEPTION_PROGRAM_PARSE_ERROR: return "VM_EXCEPTION_PROGRAM_PARSE_ERROR";
   case VM_EXCEPTIOM_INVALID_PROG_TYPE: return "VM_EXCEPTIOM_INVALID_PROG_TYPE";
   case VM_EXCEPTION_UNKNOWN_VARIANT_TYPE: return "VM_EXCEPTION_UNKNOWN_VARIANT_TYPE";
   case VM_EXCEPTION_INVALID_VARIANT_TYPE: return "VM_EXCEPTION_INVALID_VARIANT_TYPE";
   case VM_EXCEPTION_DATA_LEN_TOO_SHORT: return "VM_EXCEPTION_DATA_LEN_TOO_SHORT";
   case VM_EXCEPTION_INVALID_DATA_PARSE_PTR: return "VM_EXCEPTION_INVALID_DATA_PARSE_PTR";
   case VM_EXCEPTION_INVALID_DATA_PTR: return "VM_EXCEPTION_INVALID_DATA_PTR";
   case VM_EXCEPTION_INTERNAL_ERROR: return "VM_EXCEPTION_INTERNAL_ERROR";
   case VM_EXCEPTION_INVALID_FIELD_NAME: return "VM_EXCEPTION_INVALID_FIELD_NAME";
   }

   return nullptr;
}

/**
 * @brief QApxDataVM::parseOpCode
 * @param pBegin start of parse array
 * @param pEnd end of parse array (subtracting pBegin from pEnd always yields the length of the enclosed array)
 * @param opCode parsed token
 * @return nullptr in case of critical error, pBegin in lack of available bytes ina parse array, else it returns a pointer between pBegin+1 and pEnd where the parsing was stopped
 */
const char *DataVM::parseOpCode(const char *pBegin, const char *pEnd, int &opCode)
{
   const char *pNext=pBegin;
   if (pBegin < pEnd)
   {
      opCode = (int) *pNext++;
   }
   return pNext;
}

/**
 * @brief QApxDataVM::parseArrayLen
 * @param pBegin
 * @param pEnd
 * @param arrayLen
 * @return nullptr in case of critical error, pBegin in lack of available bytes ina parse array, else it returns a pointer between pBegin+1 and pEnd where the parsing was stopped
 */
const char *DataVM::parseArrayLen(const char *pBegin, const char *pEnd, int &arrayLen)
{
   if( (pBegin==nullptr) || (pEnd==nullptr) || (pBegin>pEnd))
   {
      //invalud argument
      return nullptr;
   }
   if(pBegin+2<=pEnd)
   {
      arrayLen  = ((int) pBegin[0]) << 8u;
      arrayLen |= (int) pBegin[1];
      return pBegin+2;
   }
   return pBegin;
}

/**
 * @brief QApxDataVM::execOperation
 * @param opCode
 * @return 0 on success or exception id in case of exception
 *
 * executes the operation in opCode. OpCodes are defined in qapxdata.h
 */
const char *DataVM::execOperation(int opCode,const char *pBegin, const char *pEnd, int &exception)
{
   const char *pNext=pBegin;

   exception = VM_EXCEPTION_NO_EXCEPTION;
   switch(opCode)
   {
   case OPCODE_NOP:
      break;      
   case OPCODE_ARGS:
      {
         int progType;
         int typeId;
         int packLen;
         const char *pMark=pNext;
         pNext=parseArgsExtra(pNext,pEnd,progType,typeId,packLen);
         if (pNext>pMark)
         {
            exception = execArgs(progType,typeId,packLen);
         }
         else
         {
            exception = VM_EXCEPTION_PROGRAM_PARSE_ERROR;
         }
      }
      break;
   case OPCODE_UNPACK_U8:
      exception=execUnpackU8();
      break;
   case OPCODE_UNPACK_U16:
      exception=execUnpackU16();
      break;
   case OPCODE_UNPACK_U32:
      exception=execUnpackU32();
      break;
   case OPCODE_UNPACK_S8:
      exception=execUnpackS8();
      break;
   case OPCODE_UNPACK_S16:
      exception=execUnpackS16();
      break;
   case OPCODE_UNPACK_S32:
      exception=execUnpackS32();
      break;
   case OPCODE_UNPACK_STR:
      {
         int strLen;
         const char *pMark=pNext;
         pNext=parseArrayLen(pNext,pEnd,strLen);
         if (pNext>pMark)
         {
            exception = execUnpackString(strLen);
         }
         else
         {
            exception = VM_EXCEPTION_PROGRAM_PARSE_ERROR;
         }
      }
      break;
   case OPCODE_UNPACK_U8AR: /*intentional fall-through*/
   case OPCODE_UNPACK_U16AR: /*intentional fall-through*/
   case OPCODE_UNPACK_U32AR:
      {
         int arrayLen;
         const char *pMark=pNext;
         pNext=parseArrayLen(pNext,pEnd,arrayLen);
         if (pNext>pMark)
         {
            if (opCode == OPCODE_UNPACK_U8AR)
            {
               exception = execUnpackU8Array(arrayLen);
            }
            else if (opCode == OPCODE_UNPACK_U16AR)
            {
               exception = execUnpackU16Array(arrayLen);
            }
            else if (opCode == OPCODE_UNPACK_U32AR)
            {
               exception = execUnpackU32Array(arrayLen);
            }
         }
         else
         {
            exception = VM_EXCEPTION_PROGRAM_PARSE_ERROR;
         }
      }
      break;
   case OPCODE_PACK_U8:
      exception=execPackU8();
      break;
   case OPCODE_PACK_U16:
      exception=execPackU16();
      break;
   case OPCODE_PACK_U32:
      exception=execPackU32();
      break;
   case OPCODE_PACK_S8:
      exception=execPackS8();
      break;
   case OPCODE_PACK_S16:
      exception=execPackS16();
      break;
   case OPCODE_PACK_S32:
      exception=execPackS32();
      break;
   case OPCODE_PACK_STR:
      {
         int strLen;
         const char *pMark=pNext;
         pNext=parseArrayLen(pNext,pEnd,strLen);
         if (pNext>pMark)
         {
            exception = execPackString(strLen);
         }
         else
         {
            exception = VM_EXCEPTION_PROGRAM_PARSE_ERROR;
         }
      }
      break;
   case OPCODE_PACK_U8AR: /*intentional fall-through*/
   case OPCODE_PACK_U16AR: /*intentional fall-through*/
   case OPCODE_PACK_U32AR:
      {
         int arrayLen;
         const char *pMark=pNext;
         pNext=parseArrayLen(pNext,pEnd,arrayLen);
         if (pNext>pMark)
         {
            if (opCode == OPCODE_PACK_U8AR)
            {
               exception = execPackU8Array(arrayLen);
            }
            else if (opCode == OPCODE_PACK_U16AR)
            {
               exception = execPackU16Array(arrayLen);
            }
            else if (opCode == OPCODE_PACK_U32AR)
            {
               exception = execPackU32Array(arrayLen);
            }
         }
         else
         {
            exception = VM_EXCEPTION_PROGRAM_PARSE_ERROR;
         }
      }
      break;
   case OPCODE_RECORD_SELECT:
      {
         const char *fieldName;
         const char *pMark=pNext;
         pNext=parseRecordSelectExtra(pNext,pEnd,fieldName);
         if (pNext>pMark)
         {
            exception = execRecordSelect(fieldName);
         }
         else
         {
            exception = VM_EXCEPTION_PROGRAM_PARSE_ERROR;
         }
      }
      break;
   default:
      qDebug("[APX] opcode not implemented: %d,",opCode);
      exception = VM_EXCEPTION_INVALID_OP_CODE;
      return nullptr;
   }
   return pNext;
}
/**
 * @brief QApxDataVM::parseArgsExtra
 * @param pBegin start of parse array
 * @param pEnd end of parse array
 * @param variantType parsed token
 * @param packLen parsed token
 * @return nullptr in case of critical error, pBegin in lack of available bytes ina parse rray, else it returns a pointer between pBegin+1 and pEnd where the parsing was stopped
 */
const char *DataVM::parseArgsExtra(const char *pBegin, const char *pEnd, int &progType, int &typeId, int &packLen)
{
   if(pBegin+5<=pEnd)
   {      
      progType = (int) pBegin[0];
      typeId   = (int) pBegin[1];
      packLen  =((int) pBegin[2])<<((int)16);
      packLen |=((int) pBegin[3])<<((int)8);
      packLen |=((int) pBegin[4]);
      return pBegin+5; //consumed 5 bytes from pBegin
   }
   return pBegin; //not enough bytes in buffer, return pBegin
}

/**
 * @brief QApxDataVM::parseRecordSelectExtra
 * @param pBegin start of parse array
 * @param pEnd end of parse array
 * @param name parsed token
 * @return nullptr in case of critical error, pBegin in lack of available bytes ina parse rray, else it returns a pointer between pBegin+1 and pEnd where the parsing was stopped
 */
const char *DataVM::parseRecordSelectExtra(const char *pBegin, const char *pEnd, const char *&name)
{
   const char *pNext = pBegin;
   name=nullptr;
   if ( (pBegin == nullptr) || (pEnd==nullptr) || (pBegin >pEnd) )
   {
      //invalid arguments
      return nullptr;
   }
   while(pNext<pEnd)
   {
      char c=*pNext++;
      if (c=='\0')
      {
         name=pBegin;
         break;
      }
   }
   if(name == nullptr)
   {
      //null-termination not found before pEnd (not enough bytes in buffer)
      return pBegin;
   }
   return pNext;
}

/**
 * @brief QApxDataVM::execArgs
 * @param progType
 * @param typeId
 * @param packLen
 * @return 0 on success or exception state on failure
 */

int DataVM::execArgs(int progType, int typeId, int packLen)
{
   int exception = VM_EXCEPTION_NO_EXCEPTION;
   if( (progType == PROG_TYPE_UNPACK) || (progType == PROG_TYPE_PACK) )
   {
      //1. verify that the variantType we got is of correct type
      if ( (typeId == VTYPE_SCALAR) || (typeId == VTYPE_LIST) || (typeId == VTYPE_MAP) )
      {
         if (typeId != mState.value.type)
         {
            exception = VM_EXCEPTION_INVALID_VARIANT_TYPE;
         }
      }
      else
      {
         exception = VM_EXCEPTION_UNKNOWN_VARIANT_TYPE;
      }
      if (exception == VM_EXCEPTION_NO_EXCEPTION)
      {
         if (progType == PROG_TYPE_UNPACK)
         {
            //2a. verify that the buffer we got has enough data
            //how many bytes is in the buffer?
            if ( mRawData == nullptr )
            {
               exception = VM_EXCEPTION_DATA_LEN_TOO_SHORT;
            }
            else
            {
               const int rawDataLen = mRawData->length();
               if (rawDataLen < packLen)
               {
                  exception = VM_EXCEPTION_DATA_LEN_TOO_SHORT;
               }
               else
               {
                  //3. setup the data parse pointers and VM Mode
                  mReadBegin=mRawData->constData();
                  mReadEnd=mReadBegin+rawDataLen;
                  mReadNext=mReadBegin;
                  mMode = PROG_TYPE_UNPACK;
               }
            }
         }
         else //pack program
         {
            //2b. setup the data parse pointers and VM Mode
            mRawData->resize(packLen);
            mWriteNext=(quint8*) mRawData->data();
            mWriteEnd=(quint8*) mWriteNext+packLen;
            Q_ASSERT(mRawData->length()==packLen);
            mMode = PROG_TYPE_PACK;
         }
      }
   }
   else
   {
      exception = VM_EXCEPTIOM_INVALID_PROG_TYPE;
   }
   return exception;
}

/**
 * @brief QApxDataVM::execRecordSelect
 * @param fieldName
 * @return 0 on success, exception code on error
 */
int DataVM::execRecordSelect(const char *fieldName)
{
   int exception = VM_EXCEPTION_NO_EXCEPTION;
   if (mState.value.map == nullptr)
   {
      exception = VM_EXCEPTION_INVALID_VARIANT_TYPE;
   }
   else
   {
      mState.fieldName=fieldName;
   }
   return exception;
}


int DataVM::execUnpackU8Array(int arrayLen)
{
   return unpackUnsignedArray<quint8>(arrayLen);
}

int DataVM::execUnpackU16Array(int arrayLen)
{
   return unpackUnsignedArray<quint16>(arrayLen);
}

int DataVM::execUnpackU32Array(int arrayLen)
{
   return unpackUnsignedArray<quint32>(arrayLen);
}


int DataVM::execUnpackU8()
{
   return unpackUnsigned<quint8>();
}

int DataVM::execUnpackU16()
{
   return unpackUnsigned<quint16>();
}

int DataVM::execUnpackU32()
{
   return unpackUnsigned<quint32>();
}

int DataVM::execUnpackS8()
{
   return unpackUnsigned<qint8>();
}

int DataVM::execUnpackS16()
{
   return unpackUnsigned<qint16>();
}

int DataVM::execUnpackS32()
{
   return unpackUnsigned<qint32>();
}

int DataVM::execUnpackString(int strLen)
{
   //1. verify that data pointers are setup correctly
   if( (mReadNext == nullptr) || (mReadEnd==nullptr) || (mReadNext>mReadEnd) )
   {
      return VM_EXCEPTION_INVALID_DATA_PTR;
   }
   //2. check that enough data is available in buffer   
   if(mReadNext+strLen>mReadEnd)
   {
      return VM_EXCEPTION_DATA_LEN_TOO_SHORT;
   }
   //3. unpack based on QVariantPtr type
   int actualStrLen=strLen; //search for null-terminator in data, if found before strLen bytes, use that as actualStrLen, otherwise use strLen
   for (int i=0;i<strLen;i++)
   {
      if (mReadNext[i]=='\0')
      {
         actualStrLen=i;
         break;
      }
   }
   Q_ASSERT(actualStrLen<=strLen);
   QString tmp=QString::fromLocal8Bit(mReadNext,actualStrLen);
   mReadNext+=strLen;
   if( (mState.value.type == VTYPE_SCALAR) && (mState.value.scalar != nullptr))
   {
      //4.1 unpack string
      mState.value.scalar->setValue(tmp);
   }
   else if( (mState.value.type == VTYPE_MAP) && (mState.value.map != nullptr))
   {
      mState.value.map->insert(mState.fieldName,QVariant(tmp));
   }
   else
   {
      return VM_EXCEPTION_INVALID_VARIANT_TYPE;
   }
   return VM_EXCEPTION_NO_EXCEPTION;
}

int DataVM::execPackU8Array(int arrayLen)
{
   return packUnsignedArray<quint8>(arrayLen);
}

int DataVM::execPackU16Array(int arrayLen)
{
   return packUnsignedArray<quint16>(arrayLen);
}

int DataVM::execPackU32Array(int arrayLen)
{
   return packUnsignedArray<quint32>(arrayLen);
}

int DataVM::execPackU8()
{
   return packUnsignedInteger<quint8>();
}

int DataVM::execPackU16()
{
   return packUnsignedInteger<quint16>();
}

int DataVM::execPackU32()
{
   return packUnsignedInteger<quint32>();
}

int DataVM::execPackS8()
{
   return packSignedInteger<qint8>();
}

int DataVM::execPackS16()
{
   return packSignedInteger<qint16>();
}

int DataVM::execPackS32()
{
   return packSignedInteger<qint32>();
}

int DataVM::execPackString(int strLen)
{
   int exception = VM_EXCEPTION_NO_EXCEPTION;
   //1. verify that data pointers are setup correctly
   if( (mWriteNext == nullptr) || (mWriteEnd==nullptr) || (mWriteNext>mWriteEnd) )
   {
      return VM_EXCEPTION_INVALID_DATA_PTR;
   }
   //2. check that enough data is available in buffer
   if(mWriteNext+strLen>mWriteEnd)
   {
      return VM_EXCEPTION_DATA_LEN_TOO_SHORT;
   }
   //3. pack based on QVariantPtr type
   QString tmp;
   mReadNext+=strLen;
   if( (mState.value.type == VTYPE_SCALAR) && (mState.value.scalar != nullptr))
   {
      //4.1 unpack string
      tmp = mState.value.scalar->toString();
   }
   else if( (mState.value.type == VTYPE_MAP) && (mState.value.map != nullptr))
   {
      tmp = (*mState.value.map)[mState.fieldName].toString();
   }
   else
   {
      exception = VM_EXCEPTION_INVALID_VARIANT_TYPE;
   }
   if (exception == VM_EXCEPTION_NO_EXCEPTION)
   {
      int actualLen = tmp.length();
      if (actualLen>strLen)
      {
         exception = VM_EXCEPTION_DATA_LEN_TOO_SHORT;
      }
      else
      {
         //first set entire area allocated to string data to NULL
         memset(mWriteNext, 0, strLen);
         //then copy string data into the nullified array. If the actual string is shorter than the allocated area, the null-bytes will act as null-terminators.
         memcpy(mWriteNext,tmp.toLocal8Bit().constData(),actualLen);
      }
   }
   return exception;
}

int DataVM::storeUnpackedValue(uint value)
{
   return storeUnpackedInteger<uint>(value);
}

int DataVM::storeUnpackedValue(int value)
{
   return storeUnpackedInteger<int>(value);
}

template<typename T> int DataVM::storeUnpackedInteger(T value)
{
   int exception = VM_EXCEPTION_NO_EXCEPTION;
   if (mState.value.type == VTYPE_SCALAR)
   {
      mState.value.scalar->setValue(value);
   }
   else if (mState.value.type == VTYPE_LIST)
   {
      if(mState.arrayIndex < 0)
      {
         exception = VM_EXCEPTION_INTERNAL_ERROR;
      }
      else
      {
         if (mState.arrayIndex < mState.value.list->size())
         {
            mState.value.list->replace(mState.arrayIndex,QVariant(value));
         }
         else
         {
            mState.value.list->append(QVariant(value));
         }
      }
   }
   else if (mState.value.type == VTYPE_MAP)
   {
      if(mState.fieldName.length()==0)
      {
         exception = VM_EXCEPTION_INVALID_FIELD_NAME;
      }
      mState.value.map->insert(mState.fieldName,QVariant(value));
   }
   else
   {
      exception = VM_EXCEPTION_INVALID_VARIANT_TYPE;
   }
   return exception;
}

template<typename T> int DataVM::packUnsignedInteger()
{
   int exception = VM_EXCEPTION_NO_EXCEPTION;
   bool ok=false;
   uint value;
   switch(mState.value.type)
   {
   case VTYPE_SCALAR:
      if (mState.value.scalar != nullptr)
      {
         value = mState.value.scalar->toUInt(&ok);
      }
      else
      {
         exception = VM_EXCEPTION_INTERNAL_ERROR;
      }
      break;
   case VTYPE_LIST:
      if( (mState.arrayIndex < 0) || (mState.arrayIndex >= mState.value.list->size()) )
      {
         exception = VM_EXCEPTION_INTERNAL_ERROR;
      }
      else
      {
         value = mState.value.list->at(mState.arrayIndex).toUInt(&ok);
      }
      break;
   case VTYPE_MAP:
      if(mState.fieldName.length()==0)
      {
         exception = VM_EXCEPTION_INVALID_FIELD_NAME;
      }
      value = (*mState.value.map)[mState.fieldName].toUInt(&ok);
      break;
   default:
      exception = VM_EXCEPTION_INVALID_VARIANT_TYPE;
   }
   if (exception == VM_EXCEPTION_NO_EXCEPTION)
   {
      if ( (ok == true) && (mWriteNext!=nullptr) && ((mWriteNext+sizeof(T))<=mWriteEnd) )
      {
         qToLittleEndian<T>(value, mWriteNext);
         mWriteNext+=sizeof(T);
      }
      else
      {
         exception = VM_EXCEPTION_DATA_CONVERSION_FAILURE;
      }
   }
   return exception;
}

/**
 * T1 is the type given to qToLittleEndian. valid values are (quint8, quint16, quint32, qint8, qint16, quint32)
 * This is the specialization used for signed integers.
 */
template<typename T> int DataVM::packSignedInteger()
{
   int exception = VM_EXCEPTION_NO_EXCEPTION;
   bool ok=false;
   int value;
   switch(mState.value.type)
   {
   case VTYPE_SCALAR:
      if (mState.value.scalar != nullptr)
      {
         value = mState.value.scalar->toInt(&ok);
      }
      else
      {
         exception = VM_EXCEPTION_INTERNAL_ERROR;
      }
      break;
   case VTYPE_LIST:
      if( (mState.arrayIndex < 0) || (mState.arrayIndex >= mState.value.list->size()) )
      {
         exception = VM_EXCEPTION_INTERNAL_ERROR;
      }
      else
      {
         value = mState.value.list->at(mState.arrayIndex).toInt(&ok);
      }
      break;
   case VTYPE_MAP:
      if(mState.fieldName.length()==0)
      {
         exception = VM_EXCEPTION_INVALID_FIELD_NAME;
      }
      value = (*mState.value.map)[mState.fieldName].toInt(&ok);
      break;
   default:
      exception = VM_EXCEPTION_INVALID_VARIANT_TYPE;
   }
   if (exception == VM_EXCEPTION_NO_EXCEPTION)
   {
      if ( (ok == true) && (mWriteNext!=nullptr) && ((mWriteNext+sizeof(T))<=mWriteEnd) )
      {
         qToLittleEndian<T>(value, mWriteNext);
         mWriteNext+=sizeof(T);
      }
      else
      {
         exception = VM_EXCEPTION_DATA_CONVERSION_FAILURE;
      }
   }
   return exception;
}

template<typename T> int DataVM::packUnsignedArray(int arrayLen)
{
   int exception = VM_EXCEPTION_NO_EXCEPTION;
   //1. verify that data pointers are setup correctly
   if( (mWriteNext == nullptr) || (mWriteEnd==nullptr) || (mWriteNext>=mWriteEnd) )
   {
      return VM_EXCEPTION_INVALID_DATA_PTR;
   }
   //2. check that enough data is available in buffer
   int requiredDataBytes=sizeof(T)*arrayLen;
   if(mWriteNext+requiredDataBytes>mWriteEnd)
   {
      return VM_EXCEPTION_DATA_LEN_TOO_SHORT;
   }
   //3. pack based on QVariantPtr type
   if(mState.value.type == VTYPE_LIST)
   {
      //4.1 pack list
      for(mState.arrayIndex=0;mState.arrayIndex<arrayLen;mState.arrayIndex++)
      {
         exception = packUnsignedInteger<T>();
         if(exception != VM_EXCEPTION_NO_EXCEPTION)
         {
            return exception;
         }
      }
   }
   else if( (mState.value.type == VTYPE_MAP) && (mState.value.map != nullptr))
   {
      //4.2 unpack as an element in a QVariantMap, this reqires that a field name has been selected previously.
      if (mState.fieldName.length()==0)
      {
         exception = VM_EXCEPTION_INVALID_FIELD_NAME;
      }
      else
      {
         QVariantList list;
         list.reserve(arrayLen);
         DataVM::State state(&list);
         mStateStack.push(mState);
         mState=state;
         for(mState.arrayIndex=0;mState.arrayIndex<arrayLen;mState.arrayIndex++)
         {
            exception = packUnsignedInteger<T>();
            if(exception != VM_EXCEPTION_NO_EXCEPTION)
            {
               return exception;
            }
         }
         mState=mStateStack.pop();
         mState.value.map->insert(mState.fieldName,list);
      }
   }
   else
   {
      exception = VM_EXCEPTION_INVALID_VARIANT_TYPE;
   }
   return exception;
}

/**
 * For some unexplicable reason C++ refuses to allow this to be a template function with int/uint as type arguments
 * When tested it fails in compilation with message "template argument deduction/substitution failed.".
 */
template<typename T> int DataVM::packSignedArray(int arrayLen)
{
   int exception = VM_EXCEPTION_NO_EXCEPTION;
   //1. verify that data pointers are setup correctly
   if( (mWriteNext == nullptr) || (mWriteEnd==nullptr) || (mWriteNext>=mWriteEnd) )
   {
      return VM_EXCEPTION_INVALID_DATA_PTR;
   }
   //2. check that enough data is available in buffer
   int requiredDataBytes=sizeof(T)*arrayLen;
   if(mWriteNext+requiredDataBytes>mWriteEnd)
   {
      return VM_EXCEPTION_DATA_LEN_TOO_SHORT;
   }
   //3. pack based on QVariantPtr type
   if(mState.value.type == VTYPE_LIST)
   {
      //4.1 pack list
      for(mState.arrayIndex=0;mState.arrayIndex<arrayLen;mState.arrayIndex++)
      {
         exception = packSignedInteger<T>();
         if(exception != VM_EXCEPTION_NO_EXCEPTION)
         {
            return exception;
         }
      }
   }
   else if( (mState.value.type == VTYPE_MAP) && (mState.value.map != nullptr))
   {
      //4.2 unpack as an element in a QVariantMap, this reqires that a field name has been selected previously.
      if (mState.fieldName.length()==0)
      {
         exception = VM_EXCEPTION_INVALID_FIELD_NAME;
      }
      else
      {
         QVariantList list;
         list.reserve(arrayLen);
         DataVM::State state(&list);
         mStateStack.push(mState);
         mState=state;
         for(mState.arrayIndex=0;mState.arrayIndex<arrayLen;mState.arrayIndex++)
         {
            exception = packSignedInteger<T>();
            if(exception != VM_EXCEPTION_NO_EXCEPTION)
            {
               return exception;
            }
         }
         mState=mStateStack.pop();
         mState.value.map->insert(mState.fieldName,list);
      }
   }
   else
   {
      exception = VM_EXCEPTION_INVALID_VARIANT_TYPE;
   }
   return exception;
}

template<typename T> int DataVM::unpackUnsigned()
{
   int exception = VM_EXCEPTION_NO_EXCEPTION;
   //1. verify that data pointers are setup correctly
   if( (mReadNext == nullptr) || (mReadEnd==nullptr) || (mReadNext>mReadEnd) )
   {
      return VM_EXCEPTION_INVALID_DATA_PTR;
   }
   //2. unpack data value
   if(mReadNext+sizeof(T)<=mReadEnd)
   {
      uint value = (uint) qFromLittleEndian<T>((const uchar*)mReadNext);
      mReadNext+=sizeof(T);
      exception = storeUnpackedValue(value);
   }
   else
   {
      exception = VM_EXCEPTION_DATA_LEN_TOO_SHORT;
   }
   return exception;
}

template<typename T> int DataVM::unpackSigned()
{
   int exception = VM_EXCEPTION_NO_EXCEPTION;
   //1. verify that data pointers are setup correctly
   if( (mReadNext == nullptr) || (mReadEnd==nullptr) || (mReadNext>mReadEnd) )
   {
      return VM_EXCEPTION_INVALID_DATA_PTR;
   }
   //2. unpack data value
   if(mReadNext+sizeof(T)<=mReadEnd)
   {
      int value = (int) qFromLittleEndian<T>((const uchar*)mReadNext);
      mReadNext+=sizeof(T);
      exception = storeUnpackedValue(value);
   }
   else
   {
      exception = VM_EXCEPTION_DATA_LEN_TOO_SHORT;
   }
   return exception;
}

template<typename T> int DataVM::unpackUnsignedArray(int arrayLen)
{
   //1. verify that data pointers are setup correctly
   if( (mReadNext == nullptr) || (mReadEnd==nullptr) || (mReadNext>mReadEnd) )
   {
      return VM_EXCEPTION_INVALID_DATA_PTR;
   }
   //2. check that enough data is available in buffer
   int requiredDataBytes=sizeof(quint8)*arrayLen;
   if(mReadNext+requiredDataBytes>mReadEnd)
   {
      return VM_EXCEPTION_DATA_LEN_TOO_SHORT;
   }
   //3. unpack based on QVariantPtr type
   if( (mState.value.type == VTYPE_LIST) && (mState.value.list != nullptr))
   {
      //4.1 unpack as list
      for(mState.arrayIndex=0;mState.arrayIndex<arrayLen;mState.arrayIndex++)
      {
         int exception = unpackUnsigned<T>();
         if(exception != VM_EXCEPTION_NO_EXCEPTION)
         {
            return exception;
         }
      }
   }
   else if( (mState.value.type == VTYPE_MAP) && (mState.value.map != nullptr))
   {
      //4.2 unpack as an element in a QVariantMap, this reqires that a field name has been selected previously.
      QVariantList list;
      list.reserve(arrayLen);
      DataVM::State state(&list);
      mStateStack.push(mState);
      mState=state;
      for(mState.arrayIndex=0;mState.arrayIndex<arrayLen;mState.arrayIndex++)
      {
         int exception = unpackUnsigned<T>();
         if(exception != VM_EXCEPTION_NO_EXCEPTION)
         {
            return exception;
         }
      }
      mState=mStateStack.pop();
      mState.value.map->insert(mState.fieldName,list);
   }
   else
   {
      return VM_EXCEPTION_INVALID_VARIANT_TYPE;
   }
   return VM_EXCEPTION_NO_EXCEPTION;
}

template<typename T> int DataVM::unpackSignedArray(int arrayLen)
{
   //1. verify that data pointers are setup correctly
   if( (mReadNext == nullptr) || (mReadEnd==nullptr) || (mReadNext>mReadEnd) )
   {
      return VM_EXCEPTION_INVALID_DATA_PTR;
   }
   //2. check that enough data is available in buffer
   int requiredDataBytes=sizeof(quint8)*arrayLen;
   if(mReadNext+requiredDataBytes>mReadEnd)
   {
      return VM_EXCEPTION_DATA_LEN_TOO_SHORT;
   }
   //3. unpack based on QVariantPtr type
   if( (mState.value.type == VTYPE_LIST) && (mState.value.list != nullptr))
   {
      //4.1 unpack as list
      return VM_EXCEPTION_NOT_IMPLEMENTED;
   }
   else if( (mState.value.type == VTYPE_MAP) && (mState.value.map != nullptr))
   {
      //4.2 unpack as an element in a QVariantMap, this reqires that a field name has been selected previously.
      QVariantList list;
      list.reserve(arrayLen);
      DataVM::State state(&list);
      mStateStack.push(mState);
      mState=state;
      for(mState.arrayIndex=0;mState.arrayIndex<arrayLen;mState.arrayIndex++)
      {
         int exception = unpackSigned<T>();
         if(exception != VM_EXCEPTION_NO_EXCEPTION)
         {
            return exception;
         }
      }
      mState=mStateStack.pop();
      mState.value.map->insert(mState.fieldName,list);
   }
   else
   {
      return VM_EXCEPTION_INVALID_VARIANT_TYPE;
   }
   return VM_EXCEPTION_NO_EXCEPTION;
}



} //namespace
