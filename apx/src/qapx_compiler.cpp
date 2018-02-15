#include "qapx_compiler.h"
#include <limits>

#define MAX_PACK_LEN 16777215 //should be equal to 2^24-1

using namespace std;

namespace Apx
{

DataCompiler::DataCompiler()
{

}

/**
 * @brief DataCompiler::genUnpackData
 * @param prog reference to an empty QByteArray where the program opcodes will be written.
 * @param pElement pointer to QApxDataElement.
 * @param pStack internal variable used for recursive calls, when this is set to zero it is treated as the initial call to this function.
 * @return 0 on success, non-zero on error
 */
int DataCompiler::genUnpackData(QByteArray &prog, const QApxDataElement *pElement, CompilerStack *pStack)
{
   const QApxDataElement *pTopElement=NULL;
   if (pStack==0)
   {
      mStack.clear();
      pStack = &mStack;
      prog.reserve(6);
      setArgs(prog,PROG_TYPE_UNPACK,pElement);
   }
   else if (pStack->size()>0)
   {
      pTopElement = pStack->top();
   }
   if (pElement->baseType == QAPX_BASE_TYPE_RECORD)
   {
      if( (pTopElement != pElement) && (pElement->arrayLen>0) )
      {
         //array of record
         bool first=true;
         arrayEnter(prog,pElement,pStack);
         for (int i=0;i<pElement->arrayLen;i++)
         {
            if (!first)
            {
               first=false;
               arrayNext(prog);
            }
            this->genUnpackData(prog,pElement,pStack);
         }
         arrayLeave(prog,pStack);
      }
      else
      {
         int end = pElement->pElementList->length();
         for (int i=0;i<end;i++)
         {
            const QApxDataElement* const& pChildElement = pElement->pElementList->at(i);
            Q_ASSERT(pChildElement != 0);
            recordSelect(prog,pChildElement->name.constData());
            if (pChildElement->baseType == QAPX_BASE_TYPE_RECORD)
            {
               recordEnter(prog,NULL,pStack); //FIXME: have to use NULL-pointer here since it can mess with (pTopElement != pElement) in if-statement above
               genUnpackData(prog, pChildElement, pStack);
               recordLeave(prog,pStack);
            }
            else
            {
               genUnpackData(prog, pChildElement, pStack);
            }
         }
      }
   }
   else
   {
      switch(pElement->baseType)
      {
      case QAPX_BASE_TYPE_UINT8:
         unpackU8(prog,pElement->arrayLen);
         break;
      case QAPX_BASE_TYPE_UINT16:
         unpackU16(prog,pElement->arrayLen);
         break;
      case QAPX_BASE_TYPE_UINT32:
         unpackU32(prog,pElement->arrayLen);
         break;
      case QAPX_BASE_TYPE_SINT8:
         unpackS8(prog,pElement->arrayLen);
         break;
      case QAPX_BASE_TYPE_SINT16:
         unpackS16(prog,pElement->arrayLen);
         break;
      case QAPX_BASE_TYPE_SINT32:
         unpackS32(prog,pElement->arrayLen);
         break;
      case QAPX_BASE_TYPE_STRING:
         unpackString(prog,pElement->arrayLen);
         break;
      default:
         break;
      }
   }
   return 0;
}

int DataCompiler::genPackData(QByteArray &prog, const QApxDataElement *pElement, CompilerStack *pStack)
{
   const QApxDataElement *pTopElement=NULL;
   if (pStack==0)
   {
      mStack.clear();
      pStack = &mStack;
      prog.reserve(6);
      setArgs(prog,PROG_TYPE_PACK,pElement);
   }
   else if (pStack->size()>0)
   {
      pTopElement = pStack->top();
   }
   if (pElement->baseType == QAPX_BASE_TYPE_RECORD)
   {
      if( (pTopElement != pElement) && (pElement->arrayLen>0) )
      {
         //array of record
         bool first=true;
         arrayEnter(prog,pElement,pStack);
         for (int i=0;i<pElement->arrayLen;i++)
         {
            if (!first)
            {
               first=false;
               arrayNext(prog);
            }
            this->genPackData(prog,pElement,pStack);
         }
         arrayLeave(prog,pStack);
      }
      else //just a record (not an array of records)
      {
         int end = pElement->pElementList->length();
         for (int i=0;i<end;i++)
         {
            const QApxDataElement* const& pChildElement = pElement->pElementList->at(i);
            Q_ASSERT(pChildElement != 0);
            recordSelect(prog,pChildElement->name.constData());
            if (pChildElement->baseType == QAPX_BASE_TYPE_RECORD)
            {
               recordEnter(prog,NULL,pStack); //FIXME: have to use NULL-pointer here since it can mess with (pTopElement != pElement) in if-statement above
               genPackData(prog, pChildElement, pStack);
               recordLeave(prog,pStack);
            }
            else
            {
               genPackData(prog, pChildElement, pStack);
            }
         }
      }
   }
   else
   {
      switch(pElement->baseType)
      {
      case QAPX_BASE_TYPE_UINT8:
         packU8(prog,pElement->arrayLen);
         break;
      case QAPX_BASE_TYPE_UINT16:
         packU16(prog,pElement->arrayLen);
         break;
      case QAPX_BASE_TYPE_UINT32:
         packU32(prog,pElement->arrayLen);
         break;
      case QAPX_BASE_TYPE_SINT8:
         packS8(prog,pElement->arrayLen);
         break;
      case QAPX_BASE_TYPE_SINT16:
         packS16(prog,pElement->arrayLen);
         break;
      case QAPX_BASE_TYPE_SINT32:
         packS32(prog,pElement->arrayLen);
         break;
      case QAPX_BASE_TYPE_STRING:
         packString(prog,pElement->arrayLen);
         break;
      default:
         break;
      }
   }
   return 0;
}

int DataCompiler::setArgs(QByteArray &prog, int progType, const QApxDataElement *pElement)
{  
   /* 1. Examine properties of pElement to determine what kind of QVariant we expect to operate on
    * Options are:
    * QVariant (scalar)
    * QVariantMap (map)
    * QVariantList (list)
   */
   VariantType requiredType = VTYPE_INVALID;
   if (pElement->baseType == QAPX_BASE_TYPE_RECORD)
   {
      requiredType = VTYPE_MAP;
   }
   else
   {
      if ( (pElement->arrayLen>0) && (pElement->baseType != QAPX_BASE_TYPE_STRING))
      {
         requiredType = VTYPE_LIST;
      }
      else
      {
         switch(pElement->baseType)
         {
         case QAPX_BASE_TYPE_UINT8:  //intentional fall-through
         case QAPX_BASE_TYPE_UINT16: //intentional fall-through
         case QAPX_BASE_TYPE_UINT32: //intentional fall-through
         case QAPX_BASE_TYPE_SINT8:  //intentional fall-through
         case QAPX_BASE_TYPE_SINT16: //intentional fall-through
         case QAPX_BASE_TYPE_SINT32: //intentional fall-through
         case QAPX_BASE_TYPE_STRING: //intentional fall-through
            requiredType = VTYPE_SCALAR;
            break;         
         default:
            break;
         }
      }
   }
   if (requiredType == VTYPE_INVALID)
   {
      return -1; //unknown/unsupported element type
   }
   //2. check that we can support the number of bytes to be packed/unpacked
   if( (pElement->packLen==0) || (pElement->packLen > MAX_PACK_LEN) )
   {
      return -1;
   }
   //3. append necessary bytes to prog array
   prog.append((char) OPCODE_ARGS);                   //opcode
   prog.append((char) progType);                      //extra byte 1
   prog.append((char) requiredType);                  //extra byte 2
   prog.append((char) (pElement->packLen >> 16));     //extra byte 3 //use 3 bytes to store length of the data element (as a 24-bit integer)
   prog.append((char) (pElement->packLen >> 8));      //extra byte 4
   prog.append((char) (pElement->packLen) );          //extra byte 5
   return 0;
}

/******************************* Protected Methods *******************************************/

void DataCompiler::arrayEnter(QByteArray &prog, const QApxDataElement *pElement, CompilerStack *pStack)
{
   prog.append((char) OPCODE_ARRAY_ENTER);
   pStack->push(pElement);
}

void DataCompiler::arrayNext(QByteArray &prog)
{
   prog.append((char) OPCODE_ARRAY_NEXT);
}

void DataCompiler::arrayLeave(QByteArray &prog, CompilerStack *pStack)
{
   prog.append((char) OPCODE_ARRAY_LEAVE);
   pStack->pop();
}

void DataCompiler::recordEnter(QByteArray &prog, const QApxDataElement *pElement, DataCompiler::CompilerStack *pStack)
{
   prog.append((char) OPCODE_RECORD_ENTER);
   pStack->push(pElement);
}

void DataCompiler::recordSelect(QByteArray &prog, const char *name)
{
   prog.append((char) OPCODE_RECORD_SELECT);
   prog.append(name);
   prog.append('\0');
}

void DataCompiler::recordLeave(QByteArray &prog, DataCompiler::CompilerStack *pStack)
{
   prog.append((char) OPCODE_RECORD_LEAVE);
   pStack->pop();
}

void DataCompiler::unpackU8(QByteArray &prog, int arrayLen)
{
   Q_ASSERT(arrayLen <= numeric_limits<quint16>::max());
   if (arrayLen > 0)
   {
      prog.append((char) OPCODE_UNPACK_U8AR);
      prog.append((char) (arrayLen >> 8)); //high byte
      prog.append((char) (arrayLen & 0xFF)); //low byte
   }
   else
   {
      prog.append((char) OPCODE_UNPACK_U8);
   }
}

void DataCompiler::unpackU16(QByteArray &prog, int arrayLen)
{
   Q_ASSERT(arrayLen <= numeric_limits<quint16>::max());
   if (arrayLen > 0)
   {
      prog.append((char) OPCODE_UNPACK_U16AR);
      prog.append((char) (arrayLen >> 8)); //high byte
      prog.append((char) (arrayLen & 0xFF)); //low byte
   }
   else
   {
      prog.append((char) OPCODE_UNPACK_U16);
   }
}

void DataCompiler::unpackU32(QByteArray &prog, int arrayLen)
{
   Q_ASSERT(arrayLen <= numeric_limits<quint16>::max());
   if (arrayLen > 0)
   {
      prog.append((char) OPCODE_UNPACK_U32AR);
      prog.append((char) (arrayLen >> 8)); //high byte
      prog.append((char) (arrayLen & 0xFF)); //low byte
   }
   else
   {
      prog.append((char) OPCODE_UNPACK_U32);
   }
}

void DataCompiler::unpackS8(QByteArray &prog, int arrayLen)
{
   Q_ASSERT(arrayLen <= numeric_limits<quint16>::max());
   if (arrayLen > 0)
   {
      prog.append((char) OPCODE_UNPACK_S8AR);
      prog.append((char) (arrayLen >> 8)); //high byte
      prog.append((char) (arrayLen & 0xFF)); //low byte
   }
   else
   {
      prog.append((char) OPCODE_UNPACK_S8);
   }
}

void DataCompiler::unpackS16(QByteArray &prog, int arrayLen)
{
   Q_ASSERT(arrayLen <= numeric_limits<quint16>::max());
   if (arrayLen > 0)
   {
      prog.append((char) OPCODE_UNPACK_S16AR);
      prog.append((char) (arrayLen >> 8)); //high byte
      prog.append((char) (arrayLen & 0xFF)); //low byte
   }
   else
   {
      prog.append((char) OPCODE_UNPACK_S16);
   }
}

void DataCompiler::unpackS32(QByteArray &prog, int arrayLen)
{
   Q_ASSERT(arrayLen <= numeric_limits<quint16>::max());
   if (arrayLen > 0)
   {
      prog.append((char) OPCODE_UNPACK_S32AR);
      prog.append((char) (arrayLen >> 8)); //high byte
      prog.append((char) (arrayLen & 0xFF)); //low byte
   }
   else
   {
      prog.append((char) OPCODE_UNPACK_S32);
   }
}

void DataCompiler::unpackString(QByteArray &prog, int strLen)
{
   Q_ASSERT(strLen <= numeric_limits<quint16>::max());
   prog.append((char) OPCODE_UNPACK_STR);
   prog.append((char) (strLen >> 8)); //high byte
   prog.append((char) (strLen & 0xFF)); //low byte
}

void DataCompiler::packU8(QByteArray &prog, int arrayLen)
{
   Q_ASSERT(arrayLen <= numeric_limits<quint16>::max());
   if (arrayLen > 0) //array of U8?
   {
      prog.append((char) OPCODE_PACK_U8AR);
      prog.append((char) (arrayLen >> 8)); //high byte
      prog.append((char) (arrayLen & 0xFF)); //low byte
   }
   else
   {
      prog.append((char) OPCODE_PACK_U8);
   }
}

void DataCompiler::packU16(QByteArray &prog, int arrayLen)
{
   Q_ASSERT(arrayLen <= numeric_limits<quint16>::max());
   if (arrayLen > 0) //array of U16?
   {
      prog.append((char) OPCODE_PACK_U16AR);
      prog.append((char) (arrayLen >> 8)); //high byte
      prog.append((char) (arrayLen & 0xFF)); //low byte
   }
   else
   {
      prog.append((char) OPCODE_PACK_U16);
   }
}

void DataCompiler::packU32(QByteArray &prog, int arrayLen)
{
   Q_ASSERT(arrayLen <= numeric_limits<quint16>::max());
   if (arrayLen > 0) //array of U32?
   {
      prog.append((char) OPCODE_PACK_U32AR);
      prog.append((char) (arrayLen >> 8)); //high byte
      prog.append((char) (arrayLen & 0xFF)); //low byte
   }
   else
   {
      prog.append((char) OPCODE_PACK_U32);
   }
}

void DataCompiler::packS8(QByteArray &prog, int arrayLen)
{
   Q_ASSERT(arrayLen <= numeric_limits<quint16>::max());
   if (arrayLen > 0) //array of S8?
   {
      prog.append((char) OPCODE_PACK_S8AR);
      prog.append((char) (arrayLen >> 8)); //high byte
      prog.append((char) (arrayLen & 0xFF)); //low byte
   }
   else
   {
      prog.append((char) OPCODE_PACK_S8);
   }
}

void DataCompiler::packS16(QByteArray &prog, int arrayLen)
{
   Q_ASSERT(arrayLen <= numeric_limits<quint16>::max());
   if (arrayLen > 0) //array of S16?
   {
      prog.append((char) OPCODE_PACK_S16AR);
      prog.append((char) (arrayLen >> 8)); //high byte
      prog.append((char) (arrayLen & 0xFF)); //low byte
   }
   else
   {
      prog.append((char) OPCODE_PACK_S16);
   }
}

void DataCompiler::packS32(QByteArray &prog, int arrayLen)
{
   Q_ASSERT(arrayLen <= numeric_limits<quint16>::max());
   if (arrayLen > 0) //array of S32?
   {
      prog.append((char) OPCODE_PACK_S32AR);
      prog.append((char) (arrayLen >> 8)); //high byte
      prog.append((char) (arrayLen & 0xFF)); //low byte
   }
   else
   {
      prog.append((char) OPCODE_PACK_S32);
   }
}

void DataCompiler::packString(QByteArray &prog, int strLen)
{
   Q_ASSERT(strLen <= numeric_limits<quint16>::max());
   prog.append((char) OPCODE_PACK_STR);
   prog.append((char) (strLen >> 8)); //high byte
   prog.append((char) (strLen & 0xFF)); //low byte
}



}
