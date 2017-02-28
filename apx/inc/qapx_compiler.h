#ifndef QAPX_COMPILER_H
#define QAPX_COMPILER_H

#include <QVariant>
#include <QStack>
#include <QtGlobal>
#include <QByteArray>
#include "qapxdataelement.h"
#include "qapx_vmbase.h"


namespace Apx
{
class DataCompiler
{

public:   
   typedef QStack<const QApxDataElement*> CompilerStack;
   DataCompiler();
   int genUnpackData(QByteArray &prog, const QApxDataElement *pElement, CompilerStack *pStack=0);
   int genPackData(QByteArray &prog, const QApxDataElement *pElement, CompilerStack *pStack=0);
protected:

   CompilerStack mStack; //stack of states
   int setArgs(QByteArray &prog, int progType, const QApxDataElement *pElement);
   void arrayEnter(QByteArray &prog, const QApxDataElement *pElement,CompilerStack *pStack);
   void arrayNext(QByteArray &prog);
   void arrayLeave(QByteArray &prog, CompilerStack *pStack=0);
   void recordEnter(QByteArray &prog, const QApxDataElement *pElement,CompilerStack *pStack);
   void recordSelect(QByteArray &prog, const char *name);
   void recordLeave(QByteArray &prog,CompilerStack *pStack);
   void unpackU8(QByteArray &prog, int arrayLen);
   void unpackU16(QByteArray &prog, int arrayLen);
   void unpackU32(QByteArray &prog, int arrayLen);
   void unpackS8(QByteArray &prog, int arrayLen);
   void unpackS16(QByteArray &prog, int arrayLen);
   void unpackS32(QByteArray &prog, int arrayLen);
   void unpackString(QByteArray &prog, int strLen);
   void packU8(QByteArray &prog, int arrayLen);
   void packU16(QByteArray &prog, int arrayLen);
   void packU32(QByteArray &prog, int arrayLen);
   void packS8(QByteArray &prog, int arrayLen);
   void packS16(QByteArray &prog, int arrayLen);
   void packS32(QByteArray &prog, int arrayLen);
   void packString(QByteArray &prog, int strLen);
};

} //namespace

#endif // QAPX_COMPILER_H
