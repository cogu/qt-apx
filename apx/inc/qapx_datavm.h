#ifndef QAPX_DATAVM_H
#define QAPX_DATAVM_H

#include <QByteArray>
#include <QVariant>
#include <QStack>
#include "qapx_vmbase.h"

//these are the exceptions that the VM can generate                                                             OP_CODES that can generate
#define VM_EXCEPTION_NO_EXCEPTION            0  //                                                              any
#define VM_EXCEPTION_NOT_IMPLEMENTED         1  //                                                              any
#define VM_EXCEPTION_INVALID_OP_CODE         2  //                                                              any
#define VM_EXCEPTION_PROGRAM_PARSE_ERROR     3  //the VM couldn't parse the program code                        any
#define VM_EXCEPTIOM_INVALID_PROG_TYPE       4  //the program type in is unknown/unspported                     OPCODE_ARGS
#define VM_EXCEPTION_UNKNOWN_VARIANT_TYPE    5  //the QVariant that we will operate on is of an unknown type    OPCODE_ARGS
#define VM_EXCEPTION_INVALID_VARIANT_TYPE    6  //the QVariant has an unexpected/incorrect type                 OPCODE_ARGS
#define VM_EXCEPTION_DATA_LEN_TOO_SHORT      7  //the rawData byte array that the VM was given is too short     OPCODE_ARGS
#define VM_EXCEPTION_INVALID_DATA_PARSE_PTR  8  //internal variable mDataNext has invalid value                 OPCODE_UNPACK_*
#define VM_EXCEPTION_INVALID_DATA_PTR        9  //internal variables mDataNext and/or mDataEnd is incorrect     OPCODE_UNPACK_*
#define VM_EXCEPTION_INTERNAL_ERROR          10 //internal error
#define VM_EXCEPTION_INVALID_FIELD_NAME      11 //unpacking into QVariantMap but no field name was selected     OPCODE_UNPACK_*
#define VM_EXCEPTION_DATA_CONVERSION_FAILURE 13 //Failed while reading value from QVariant using e.g. ToUint()  OPCODE_PACK_*
namespace Apx
{

struct QVariantPtr
{
   QVariant *scalar;
   QVariantMap *map;
   QVariantList *list;
   VariantType type;
   QVariantPtr();
   QVariantPtr(QVariant *s,QVariantMap *m,QVariantList *l);
};


/**
 * @brief The QApxDataVM class
 *
 * This is a special purpose virtual machine that can execute the QAPX data byte code defined qapx_vmbase.h
 * It is a stack-based machine that can be used to unpack raw bytes from QByteArray into QVariant or vice-versa.
 * The byte code programs are stored as "machine code" in QByteArray that are given to the VM in the exec function to execute
 */
class DataVM
{
public:

   struct State
   {
      QString fieldName;
      int arrayIndex;
      QVariantPtr value;
      QVariantMap::Iterator mapIterator;
      State();      
      State(QVariant *v);
      State(QVariantMap *v);
      State(QVariantList *v);      
      void cleanup();
   };

   DataVM();
   int exec(const QByteArray &prog, QByteArray &rawData, QVariant &value);
   int exec(const QByteArray &prog, QByteArray &rawData, QVariantMap &value);
   int exec(const QByteArray &prog, QByteArray &rawData, QVariantList &value);
static const char *exceptionToStr(int exception);

protected:
   const char *parseOpCode(const char *pBegin, const char *pEnd, int &opCode);
   const char *parseArrayLen(const char *pBegin, const char *pEnd, int &arrayLen);
   const char *parseArgsExtra(const char *pBegin, const char *pEnd, int &progType, int &typeId, int &packLen);
   const char *parseRecordSelectExtra(const char *pBegin, const char *pEnd, const char *&name);

   int execProg(const QByteArray &prog);
   const char *execOperation(int opCode, const char *pBegin, const char *pEnd, int &exception);
   int execArgs(int progType, int typeId, int packLen);
   int execRecordSelect(const char *fieldName);

   int execUnpackU8Array(int arrayLen);
   int execUnpackU16Array(int arrayLen);
   int execUnpackU32Array(int arrayLen);

   int execUnpackU8();
   int execUnpackU16();
   int execUnpackU32();
   int execUnpackS8();
   int execUnpackS16();
   int execUnpackS32();
   int execUnpackString(int strLen);

   int execPackU8Array(int arrayLen);
   int execPackU16Array(int arrayLen);
   int execPackU32Array(int arrayLen);
   int execPackU8();
   int execPackU16();
   int execPackU32();
   int execPackS8();
   int execPackS16();
   int execPackS32();
   int execPackString(int strLen);


   template<typename T> int unpackUnsigned();
   template<typename T> int unpackSigned();
   template<typename T> int storeUnpackedInteger(T value);
   template<typename T> int packUnsignedInteger();
   template<typename T> int packSignedInteger();
   template<typename T> int packUnsignedArray(int arrayLen);
   template<typename T> int packSignedArray(int arrayLen);
   template<typename T> int unpackUnsignedArray(int arrayLen);
   template<typename T> int unpackSignedArray(int arrayLen);

   QStack<DataVM::State> mStateStack;
   DataVM::State mState;
   QByteArray *mRawData;
   const char *mReadBegin;
   const char *mReadEnd;
   const char *mReadNext;
   quint8 *mWriteNext;
   quint8 *mWriteEnd;
   int mMode;
};

} //namespace

#endif // QAPXDATAVM_H
