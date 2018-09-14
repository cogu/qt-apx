/*
 * This file implements an APX stream parser. It was written early in 2015 for APX/1.1 and it has a lot things implemented
 * that are not in use since APX/1.2. However, the parser works fine for parsing APX/1.2 files.
 * This file should be cleaned up and parts of it should be removed.
 */
#include "qapx_stream.h"
#include "qscan.h"
#include <QtEndian>
#include <QDebug>
#include <ctype.h>
#include "qapxbase.h"

QApxIStreamBuf::QApxIStreamBuf():mEventHandler(0),mIsOpen(0),
   mLastMsgType(APX_MSG_TYPE_NONE),mException(APX_EXCEPTION_NO_EXCEPTION),mLine(0)
{
   reset();
}

void QApxIStreamBuf::open()
{
   mIsOpen = true;
   mLine=1;
   if (mEventHandler != 0)
   {
      mEventHandler->apx_istream_open();
   }   
}

void QApxIStreamBuf::close()
{
   mIsOpen = false;
   if (mEventHandler != 0)
   {
      mEventHandler->apx_istream_close();
   }
}


void QApxIStreamBuf::parseBuffer()
{

}

void QApxIStreamBuf::write(const QByteArray &chunk)
{
   const quint8 *pBegin;
   const quint8 *pEnd;
   const quint8 *pNext;
   const quint8 *pResult;

   //qDebug() << "buffering" << chunk.length();
   m_buf.append(chunk);
   pBegin = (const quint8*) m_buf.constData();
   pEnd = pBegin + m_buf.length();
   pNext = pBegin;
   pResult = 0;


   while (pNext < pEnd)
   {
      quint8 firstByte;
      firstByte =  *pNext; //do not move pNext forward yet, it could be a single '\n' character
      if( (firstByte & APX_BINARY_FLAG) == 0)
      {
         //ASCII character (0-127, 7-bit ASCII)
         //wait to parse until a complete line has been seen. lines end with a single \n (not with \r\n as in HTML)         
         const quint8 *pLineEnd = 0;
         const quint8 *pLineBegin = pNext;

         pLineEnd = qscan_searchUntil(pNext,pEnd,(quint8) '\n');
         if(pLineEnd == pLineBegin)
         {
            //if line is empty it means end of APX announcement.
            pNext = pLineEnd+1;
            if(mEventHandler != 0)
            {
               mParseState = APX_PARSE_STATE_NONE;
               mEventHandler->apx_istream_endTextMsg();               
            }
            break;
         }
         else if (pLineEnd == 0)
         {
            //parse entire line
            pLineEnd=pEnd;
         }
         if(pLineEnd > pLineBegin)
         {            
            bool success = parseLine(pLineBegin,pLineEnd);
            if (success == false)
            {
               qDebug("[APX] parse failure (%d)",mLine);
               return;
            }
            pNext = pLineEnd+1;
            mLine++;
         }
         else
         {
            qDebug("[PLUGIN] parse failure, line=%d",mLine);
            Q_ASSERT(0);
         }
      }
      else
      {
         //The APX data message flag is set. This marks the beginning of an APX data message         
         pResult = parseDataMsg(pNext,pEnd);
         if (pResult > pNext)
         {
            //we parsed a full message, check for another message in the buffer
            pNext=pResult;
         }
         else
         {
            if (pResult == 0)
            {
               qDebug() << "[PLUGIN] APX parse error"; //invalid message header
            }
            else
            {
               //we have not yet received a complete message, try again later
            }
            break;
         }
      }
   }
   //update m_buf
   int len = (int) (pNext-pBegin);
   QByteArray tmp = m_buf.right(m_buf.length()-len);
   m_buf.swap(tmp);
}

void QApxIStreamBuf::reset()
{
   resetParseState();
   m_buf.clear();
}

void QApxIStreamBuf::resetParseState()
{
   mParseState = APX_PARSE_STATE_NONE;
   mLastMsgType=APX_MSG_TYPE_NONE;
   mException = APX_EXCEPTION_NO_EXCEPTION;
}

/**
 * @brief QApxIStreamBuf::parseLine
 * @param pBegin
 * @param pEnd
 * @return true on success, false on failure
 */
bool QApxIStreamBuf::parseLine(const quint8 *pBegin,const quint8 *pEnd)
{
   ApxHeaderLine *header;
   ApxDeclarationLine *decl;
   const quint8 *pResult = 0;   

   quint8 firstChar;
   quint8 secondChar;
   quint8 thirdChar;

   firstChar = (pBegin<pEnd)? *pBegin : (quint8) '\0';
   secondChar = ((pBegin+1)<pEnd)? *(pBegin+1) : (quint8)'\0';
   thirdChar = ((pBegin+2)<pEnd)? *(pBegin+2) : (quint8)'\0';

   switch(mParseState)
   {
   case APX_PARSE_STATE_NONE:
      header = new ApxHeaderLine;
      pResult=parseApxHeaderLine(pBegin,pEnd,header);
      if (pResult == pEnd) //entire line was parsed
      {
         if ( (header->majorVersion==1) && (header->minorVersion>=2))
         {
            mParseState = APX_PARSE_STATE_HEADER;
         }
         else
         {
            reset();
         }
      }
      else
      {
         Q_ASSERT(0);
         reset();
      }
      delete header;
      break;
   case APX_PARSE_STATE_HEADER:
      if ( (firstChar =='?') && (secondChar=='N') )
      {
         if (thirdChar == '*')
         {
            mParseState = APX_PARSE_STATE_NODENAME_RQST;
            if (mEventHandler != 0)
            {
               mEventHandler->apx_istream_nodeNameRqst();
            }
         }
         else if(thirdChar=='\"')
         {
            mParseState = APX_PARSE_STATE_NODEQUERY_RQST;
            if (mEventHandler != 0)
            {
               const quint8 *pNext=pBegin+2;
               pResult=qscan_matchPair(pNext,pEnd,'"','"','\\');
               if (pResult > pNext)
               {
                  QByteArray name = qscan_toByteArray(pNext+1,pResult-1);
                  mEventHandler->apx_istream_nodeQueryRqst(name);
               }
            }
         }
         else
         {
            //syntax error
            Q_ASSERT(0);
         }
      }
      else if( (firstChar == '+') && (secondChar == 'N') && (thirdChar == '\"'))
      {
         mParseState = APX_PARSE_STATE_NODENAME_RSP;
         qDebug()<<"[PLUGIN] discovery response APX_PARSE_STATE_HEADER";
      }
      else if(isalpha(firstChar))
      {
         decl = new ApxDeclarationLine;
         mParseState = APX_PARSE_STATE_NODEQUERY_RSP;
         mLastMsgType = APX_MSG_TYPE_NODEQUERY_RSP;
         pResult=splitDeclarationLine(pBegin,pEnd,decl);
         if (pResult != 0)
         {
            parseDeclaration(decl);
         }
         else
         {
            Q_ASSERT(0);
            reset();
         }
         delete decl;
      }
      break;
   case APX_PARSE_STATE_NODENAME_RQST:
      //only message end is allowed here
      Q_ASSERT(0);
      reset();
      break;
   case APX_PARSE_STATE_NODENAME_RSP:
      if( (firstChar == '+') && (secondChar == 'N') && (thirdChar == '\"'))
      {
         qDebug()<<"[PLUGIN] discovery response";
      }
      else
      {
         Q_ASSERT(0);
         reset();
      }
      break;
   case APX_PARSE_STATE_NODEQUERY_RQST:
      if ( (firstChar =='?') && (secondChar == 'N') && (thirdChar == '"') )
      {
         if (mEventHandler != 0)
         {
            const quint8 *pNext=pBegin+2;
            pResult=qscan_matchPair(pNext,pEnd,'"','"','\\');
            if (pResult > pNext)
            {
               QByteArray name = qscan_toByteArray(pNext+1,pResult-1);
               mEventHandler->apx_istream_nodeQueryRqst(name);
            }
         }
      }
      else
      {
         Q_ASSERT(0);
         reset();
      }
      break;
   case APX_PARSE_STATE_NODEQUERY_RSP:
      if (isalpha(firstChar))
      {
         decl = new ApxDeclarationLine;
         pResult=splitDeclarationLine(pBegin,pEnd,decl);
         if (pResult != 0)
         {
            parseDeclaration(decl);
            delete decl;
         }
         else
         {
            mException = APX_EXCEPTION_INVALID_DECLARATION_LINE;
            delete decl;
            return false;
         }         
      }
      break;
   default:
      Q_ASSERT(0);
      reset();
   }
   return true;
}


const quint8 * QApxIStreamBuf::splitDeclarationLine(const quint8 *pBegin,const quint8 *pEnd, ApxDeclarationLine *data)
{
   const quint8 *pNext = (quint8*) pBegin;
   const quint8 *pResult = 0;
   int len;
   if (pNext < pEnd)
   {
      data->lineType = *pNext++;
      if (pNext < pEnd)
      {
         quint8 c = (quint8) *pNext;
         if (c == '"')
         {
            pResult = qscan_matchPair(pNext,pEnd,'"','"','\\');
            if (pResult > pNext)
            {
               len = (int) (pResult-pNext-2); //compensate for the two '"' characters
               data->name.clear();
               data->name.insert(0,(const char*) (pNext+1),len); //do not include the first '"" character in string
               pNext = pResult;
               pResult = qscan_searchUntil(pNext,pEnd,':');
               if (pResult > pNext)
               {

                  len = (int) (pResult-pNext);
                  data->dsg.clear();
                  data->dsg.insert(0,(const char*) pNext,len);
                  pNext = pResult;
                  if (pNext<pEnd)
                  {
                     Q_ASSERT(':'==*pNext++);
                     if (pNext<pEnd)
                     {
                        len = (int) (pEnd-pNext);
                        data->attr.clear();
                        data->attr.insert(0,(const char*) pNext,len);
                        pNext=pEnd;
                     }
                     else
                     {
                        Q_ASSERT(0); //deal with this error later
                     }
                  }
                  else
                  {
                     data->attr.clear();
                  }
                  return pNext;
               }
               else
               {
                  len = (int) (pEnd-pNext);
                  data->dsg.clear();
                  data->dsg.insert(0,(const char*) pNext,len);
                  pNext=pEnd;
                  return pNext;
               }
            }
         }
      }
   }
   return nullptr; //parse failure
}


const quint8 *QApxIStreamBuf::parseApxHeaderLine(const quint8 *pBegin, const quint8 *pEnd, ApxHeaderLine *data)
{
   const quint8 *pNext = pBegin;
   const quint8 *pResult = nullptr;
   const char *str = "APX/";
   int len = strlen(str);
   pResult = qscan_matchStr(pNext,pEnd,(const quint8*) str,((const quint8*) str)+len);
   if ( (pResult > pNext) && (pNext+len == pResult))
   {
      long number;
      pNext=pResult;
      number = strtol((const char*)pNext,(char **) &pResult,10);
      if (pResult > pNext)
      {
         data->majorVersion=(int) number;
         pNext=pResult;
         if (pNext<pEnd)
         {
            char c = (char) *pNext++;
            if ( (c == '.') && (pNext<pEnd) )
            {
               number = strtol((const char*)pNext,(char **) &pResult,10);
               if (pResult > pNext)
               {
                  data->minorVersion=(int) number;
                  pNext=pResult;
                  return pNext;
               }
            }
         }
      }
   }
   return nullptr;
}

void QApxIStreamBuf::parseDeclaration(ApxDeclarationLine *decl)
{
   if (decl != nullptr)
   {
      switch(decl->lineType)
      {
      case 'N':
         if (mEventHandler!=nullptr)
         {
            mEventHandler->apx_istream_nodeQueryRspStart(decl->name,decl->dsg);
         }
         break;
      case 'T':
         if (mEventHandler!=nullptr)
         {
            mEventHandler->apx_istream_typedef(decl->name,decl->dsg,decl->attr);
         }
         break;
      case 'P':
         if (mEventHandler!=nullptr)
         {
            mEventHandler->apx_istream_provide(decl->name,decl->dsg,decl->attr);
         }
         break;
      case 'R':
         if (mEventHandler!=nullptr)
         {
            mEventHandler->apx_istream_require(decl->name,decl->dsg,decl->attr);
         }
         break;
      }
   }
}

const quint8 *QApxIStreamBuf::parseDataMsg(const quint8 *pBegin, const quint8 *pEnd)
{
   const quint8 *pNext = pBegin;

   while (pNext < pEnd)
   {
      quint8 headerByte = ((quint8)*pNext++);
      quint8 msgType = headerByte & APX_MSG_MASK;
      quint8 lValType = (headerByte >> APX_LEN_STARTBIT) & APX_LEN_MASK;
      quint32 msgLen = 0;

      //check if message is complete before consuming it from buffer
      //1. Determine message length based on the lValType found in headerByte
      switch(lValType)
      {
      case APX_LEN_INHERIT:
         //assume entire buffer contains the message. Another transfer protocol must have
         //already handled the message length before this call
         msgLen = (quint32) (pEnd-pBegin);
         break;
      case APX_LEN_BYTE:
         //length is located in next byte as uint8
         if (pNext < pEnd)
         {
            msgLen = ((quint32) *pNext++);
         }
         else
         {
            return pBegin; //header not complete
         }
         break;
      case APX_LEN_SHORT:
         //length is located in next 2 byte as uint16 (big endian)
         if ( (pNext + 1) < pEnd)
         {
            msgLen = ((quint32) *pNext++) << 8;
            msgLen |= ((quint32) *pNext++);
         }
         else
         {
            return pBegin; //header not complete
         }
         break;
      case APX_LEN_LONG:
         //length is located in next 4 byte as uint32 (big endian)
         if ( (pNext + 3) < pEnd)
         {
            msgLen = ((quint32) *pNext++) << 24;
            msgLen |= ((quint32) *pNext++) << 16;
            msgLen |= ((quint32) *pNext++) << 8;
            msgLen |= ((quint32) *pNext++);
         }
         else
         {
            return pBegin; //header not complete
         }
         break;
      default:
         msgLen=lValType; //1:1 byte, 2: 2 bytes, 3: 3 bytesm 4: 4 bytes
         break;
      }
      if (msgLen > 0)
      {
         //is message complete?
         if( msgLen <= ((quint32)(pEnd - pNext))   )
         {
            //consume message
            const quint8 *pMark=pNext+msgLen;
            QByteArray msg;
            quint16 portId=0;
            quint32 dataLen; //used for requirePortConnect and providePortConnect
            if (msgLen >= 2) //assume that first 2 data bytes is a portId
            {
               portId = ((quint16) *pNext++) << 8;
               portId |= (quint16) *pNext++;
               msgLen-=2;
            }
            if(msgLen>0)
            {
               msg.setRawData((const char *) pNext,(uint) msgLen);
            }
            switch(msgType)
            {
            case APX_MSG_RESERVED:
               break;
            case APX_MSG_WRITE:
               mEventHandler->apx_istream_dataWrite(portId,msg);
               break;
            case APX_MSG_READ:
               mEventHandler->apx_istream_dataRead(portId);
               break;
            case APX_MSG_INVOKE:
               mEventHandler->apx_istream_opInvoke(portId,msg);
               break;
            case APX_MSG_RETURN:
               mEventHandler->apx_istream_opReturn(portId,msg);
               break;
            case APX_MSG_RPORT_CONNECT:
               dataLen = parseDataLen(msg);
               mEventHandler->apx_istream_requirePortConnect(portId,dataLen);
               break;
            case APX_MSG_RPORT_DISCONNECT:
               mEventHandler->apx_istream_requirePortDisconnect(portId);
               break;
            case APX_MSG_PPORT_CONNECT:
               dataLen = parseDataLen(msg);
               mEventHandler->apx_istream_providePortConnect(portId,dataLen);
               break;
            case APX_MSG_PPORT_DISCONNECT:
               mEventHandler->apx_istream_providePortDisconnect(portId);
               break;
            case APX_MSG_RPORT_ERROR:
               mEventHandler->apx_istream_requirePortError(portId);
               break;
            case APX_MSG_PPORT_ERROR:
               mEventHandler->apx_istream_providePortError(portId);
               break;
            }
            pNext=pMark;
         }
         else
         {
            //still waiting for message to complete
            return pBegin;
         }
      }
      else
      {
         //just a header with no data bytes?
         return nullptr; //mark parse failure
      }
   }
   return pNext;
}

quint32 QApxIStreamBuf::parseDataLen(const QByteArray &msg)
{
   int len = msg.length();
   quint32 retval = 0;
   const quint8 *pNext = (const quint8*) msg.constData();
   if (len==1)
   {
      //size 1: contains uint8 value
      retval=(quint32) *pNext;
   }
   else if(len==2)
   {
      //size 2: contains uint16 value
      retval = ((quint32) *pNext++) << 8;
      retval |= ((quint32) *pNext);
   }
   else if(len==4)
   {
      //size 4: contains uint32 value
      retval = ((quint32) *pNext++) << 24;
      retval |= ((quint32) *pNext++) << 16;
      retval |= ((quint32) *pNext++) << 8;
      retval |= ((quint32) *pNext);
   }
   else
   {

   }

   return retval;
}

QApxOStreamBuf::QApxOStreamBuf(QObject *parent):QObject(parent)
{

}

QApxOStreamBuf::~QApxOStreamBuf()
{

}


//announce messages
void QApxOStreamBuf::apxStartTextMsg()
{
   mBuf.clear();
   mBuf.append(QStringLiteral("APX/"));
   mBuf.append(QByteArray::number(mMajorVersion));
   mBuf.append('.');
   mBuf.append(QByteArray::number(mMinorVersion));
   mBuf.append('\n');
}

void QApxOStreamBuf::apxEndTextMsg()
{
   mBuf.append('\n');
}

void QApxOStreamBuf::apxNodeNameRqst()
{
   mBuf.append(QStringLiteral("?N*\n"));
}

int QApxOStreamBuf::apxNodeNameRsp(const QByteArray &name)
{
   if(name.length()>0)
   {
      mBuf.append(QStringLiteral("+N\""));
      mBuf.append(name);
      mBuf.append(QStringLiteral("\"\n"));
      return 0;
   }
   return -1;
}

int QApxOStreamBuf::apxNodeQueryRqst(const QByteArray &name)
{
   if(name.length()>0)
   {
      mBuf.append(QStringLiteral("?N\""));
      mBuf.append(name);
      mBuf.append(QStringLiteral("\"\n"));
      return 0;
   }
   return -1;
}

int QApxOStreamBuf::apxNodeQueryRspStart(const QByteArray &name,int numTypedefs, int numProvidePorts, int numRequirePorts)
{
   QByteArray dim = nodeDim(numTypedefs,numProvidePorts,numRequirePorts);
   QByteArray attr;
   return apxDeclarationLine('N',name,dim,attr);
}

int QApxOStreamBuf::apxNodeRequire(const QByteArray &name,const QByteArray &dsg, const QByteArray &attr)
{
   return apxDeclarationLine('R',name,dsg,attr);
}

int QApxOStreamBuf::apxNodeProvide(const QByteArray &name,const QByteArray &dsg, const QByteArray &attr)
{
   return apxDeclarationLine('P',name,dsg,attr);
}

int QApxOStreamBuf::apxNodeTypedef(const QByteArray &name,const QByteArray &dsg, const QByteArray &attr)
{
   return apxDeclarationLine('T',name,dsg,attr);
}

//data messages

void QApxOStreamBuf::apxRequirePortConnect(quint16 portId, quint32 dataLen)
{
   QByteArray data;
   if (dataLen<256)
   {
      //will fit into one byte
      data.resize(1);
      qToBigEndian<quint8>((quint8)dataLen,(uchar*) data.data());
   }
   else if (dataLen<65536)
   {
      //will fit into two bytes
      data.resize(2);
      qToBigEndian<quint16>((quint16)dataLen,(uchar*) data.data());
   }
   else
   {
      //we need 4 bytes
      data.resize(4);
      qToBigEndian<quint32>(dataLen,(uchar*) data.data());
   }
   apxDataMsg(APX_MSG_RPORT_CONNECT,portId,data);
}

void QApxOStreamBuf::apxRequirePortDisconnect(quint16 portId)
{
   QByteArray data;
   apxDataMsg(APX_MSG_RPORT_DISCONNECT,portId,data);
}

void QApxOStreamBuf::apxProvidePortConnect(quint16 portId, quint32 dataLen)
{
   QByteArray data;
   if (dataLen<256)
   {
      //will fit into one byte
      data.resize(1);
      qToBigEndian<quint8>((quint8)dataLen,(uchar*) data.data());
   }
   else if (dataLen<65536)
   {
      //will fit into two bytes
      data.resize(2);
      qToBigEndian<quint16>((quint16)dataLen,(uchar*) data.data());
   }
   else
   {
      //we need 4 bytes
      data.resize(4);
      qToBigEndian<quint32>(dataLen,(uchar*) data.data());
   }
   apxDataMsg(APX_MSG_PPORT_CONNECT,portId,data);
}

void QApxOStreamBuf::apxProvidePortDisconnect(quint16 portId)
{
   QByteArray data;
   apxDataMsg(APX_MSG_PPORT_DISCONNECT,portId,data);
}

void QApxOStreamBuf::apxRequirePortError(quint16 portId)
{
   QByteArray data;
   apxDataMsg(APX_MSG_RPORT_ERROR,portId,data);
}

void QApxOStreamBuf::apxProvidePortError(quint16 portId)
{
   QByteArray data;
   apxDataMsg(APX_MSG_PPORT_ERROR,portId,data);
}



void QApxOStreamBuf::apxDataRead(quint16 portId)
{
   QByteArray data;
   apxDataMsg(APX_MSG_READ,portId,data);
}

void QApxOStreamBuf::apxDataWrite(quint16 portId,const QByteArray &data)
{
   apxDataMsg(APX_MSG_WRITE,portId,data);
}

void QApxOStreamBuf::apxOpInvoke(quint16 portId,const QByteArray &data)
{
   apxDataMsg(APX_MSG_WRITE,portId,data);
}

void QApxOStreamBuf::apxOpReturn(quint16 portId,const QByteArray &data)
{
   apxDataMsg(APX_MSG_WRITE,portId,data);
}

void QApxOStreamBuf::clear()
{
   mBuf.clear();
}

void QApxOStreamBuf::transmit()
{
   emit write(mBuf);
}


QByteArray QApxOStreamBuf::nodeDim(int numTypedefs, int numProvidePorts, int numRequirePorts)
{
   QByteArray result;
   result.append('(');
   result.append(QByteArray::number(numTypedefs));
   result.append(',');
   result.append(QByteArray::number(numProvidePorts));
   result.append(',');
   result.append(QByteArray::number(numRequirePorts));
   result.append(')');
   return result;
}

int QApxOStreamBuf::apxDeclarationLine(const char firstChar,const QByteArray &name, const QByteArray &dsg, const QByteArray &attr)
{
   if ( (name.length()>0) && (dsg.length()>0) )
   {
      mBuf.append(firstChar);
      mBuf.append('"');
      mBuf.append(name);
      mBuf.append('"');
      mBuf.append(dsg);
      if (attr.length() > 0)
      {
         mBuf.append(':');
         mBuf.append(attr);
      }
      mBuf.append('\n');
      return 1;
   }
   return -1;
}

void QApxOStreamBuf::apxDataMsg(quint8 msgType, quint16 portId, const QByteArray &msgData)
{
   quint8 headerByte = (APX_BINARY_FLAG | (msgType & APX_MSG_MASK));
   int dataLen = sizeof(portId) + msgData.length();
   if (dataLen >= 0) // TODO always true
   {
      quint8 lengthValType;
      if(dataLen<5)
      {
         lengthValType = dataLen;
      }
      else if (dataLen < 256)
      {
         lengthValType=APX_LEN_BYTE;
      }
      else if (dataLen < 65536)
      {
         lengthValType=APX_LEN_SHORT;
      }
      else
      {
         lengthValType=APX_LEN_LONG;
      }
      headerByte|= (quint8) ((lengthValType&APX_LEN_MASK)<<APX_LEN_STARTBIT);
      mBuf.append(headerByte);
      const int preLenTypeSize = mBuf.size();
      int tmpSize = preLenTypeSize;
      //Right after headerByte, inser length bytes (if applicable)
      switch(lengthValType)
      {
      case APX_LEN_BYTE:
         tmpSize += 1;
         mBuf.resize(tmpSize);
         qToBigEndian<quint8>((quint8)dataLen, &(mBuf.data())[preLenTypeSize]);
         break;
      case APX_LEN_SHORT:
         tmpSize += 2;
         mBuf.resize(tmpSize);
         qToBigEndian<quint16>((quint16)dataLen, &(mBuf.data())[preLenTypeSize]);
         break;
      case APX_LEN_LONG:
         tmpSize += 4;
         mBuf.resize(tmpSize);
         qToBigEndian<quint32>((quint32)dataLen, &(mBuf.data())[preLenTypeSize]);
         break;
      default:
         //Not applicable
         break;
      }
      mBuf.resize(tmpSize+sizeof(portId));
      qToBigEndian<quint16>(portId, &(mBuf.data())[tmpSize]);
      if (msgData.length() > 0)
      {
         mBuf.append(msgData);
      }
   }

}


/*
QApxOutStream::QApxOutStream():mDevice(0)
{

}

QApxOutStream::QApxOutStream(QIODevice *d):mDevice(d)
{

}

void QApxOutStream::apxStartTextMsg()
{
   if( (mDevice != 0) && (mDevice->isWritable()) )
   {
      mDevice->write("APX/");
      mDevice->write(QByteArray::number(mMajorVersion));
      mDevice->write(".");
      mDevice->write(QByteArray::number(mMinorVersion));
      mDevice->write("\n");
   }
}

void QApxOutStream::apxEndTextMsg()
{
   if( (mDevice != 0) && (mDevice->isWritable()) )
   {
      mDevice->write("\n");
   }
}

void QApxOutStream::apxNodeNameRqst()
{
   if( (mDevice != 0) && (mDevice->isWritable()) )
   {
      mDevice->write("?N*\n");
   }
}

int QApxOutStream::apxNodeNameRsp(const QByteArray &name)
{
   if( (mDevice != 0) && (mDevice->isWritable()) )
   {
      if(name.length()>0)
      {
         mDevice->write("+N\"");
         mDevice->write(name);
         mDevice->write("\"\n");
         return 0;
      }
   }
   return -1;
}

int QApxOutStream::apxNodeQueryRqst(const QByteArray &name)
{
   if( (mDevice != 0) && (mDevice->isWritable()) )
   {
      if(name.length()>0)
      {
         mDevice->write("?N\"");
         mDevice->write(name);
         mDevice->write("\"\n");
         return 0;
      }
   }
   return -1;
}

int QApxOutStream::apxNodeQueryRspStart(const QByteArray &name,int numTypedefs, int numProvidePorts, int numRequirePorts)
{
   QByteArray dim = nodeDim(numTypedefs,numProvidePorts,numRequirePorts);
   QByteArray attr;
   return apxDeclarationLine('N',name,dim,attr);
}

int QApxOutStream::apxNodeRequire(const QByteArray &name,const QByteArray &dsg, const QByteArray &attr)
{
   return apxDeclarationLine('R',name,dsg,attr);
}

int QApxOutStream::apxNodeProvide(const QByteArray &name,const QByteArray &dsg, const QByteArray &attr)
{
   return apxDeclarationLine('P',name,dsg,attr);
}

int QApxOutStream::apxNodeTypedef(const QByteArray &name,const QByteArray &dsg, const QByteArray &attr)
{
   return apxDeclarationLine('T',name,dsg,attr);
}

//data messages

void QApxOutStream::apxRequirePortConnect(quint16 portId)
{
   QByteArray data;
   apxDataMsg(APX_MSG_RPORT_CONNECT,portId,data);
}

void QApxOutStream::apxRequirePortDisconnect(quint16 portId)
{
   QByteArray data;
   apxDataMsg(APX_MSG_RPORT_DISCONNECT,portId,data);
}

void QApxOutStream::apxProvidePortConnect(quint16 portId)
{
   QByteArray data;
   apxDataMsg(APX_MSG_PPORT_CONNECT,portId,data);
}

void QApxOutStream::apxProvidePortDisconnect(quint16 portId)
{
   QByteArray data;
   apxDataMsg(APX_MSG_PPORT_DISCONNECT,portId,data);
}

void QApxOutStream::apxRequirePortError(quint16 portId)
{
   QByteArray data;
   apxDataMsg(APX_MSG_RPORT_ERROR,portId,data);
}

void QApxOutStream::apxProvidePortError(quint16 portId)
{
   QByteArray data;
   apxDataMsg(APX_MSG_PPORT_ERROR,portId,data);
}



void QApxOutStream::apxDataRead(quint16 portId)
{
   QByteArray data;
   apxDataMsg(APX_MSG_READ,portId,data);
}

void QApxOutStream::apxDataWrite(quint16 portId,const QByteArray &data)
{
   apxDataMsg(APX_MSG_WRITE,portId,data);
}

void QApxOutStream::apxOpInvoke(quint16 portId,const QByteArray &data)
{
   apxDataMsg(APX_MSG_WRITE,portId,data);
}

void QApxOutStream::apxOpReturn(quint16 portId,const QByteArray &data)
{
   apxDataMsg(APX_MSG_WRITE,portId,data);
}



QByteArray QApxOStreamBuf::nodeDim(int numTypedefs, int numProvidePorts, int numRequirePorts)
{
   QByteArray result;
   result.append('(');
   result.append(QByteArray::number(numTypedefs));
   result.append(',');
   result.append(QByteArray::number(numProvidePorts));
   result.append(',');
   result.append(QByteArray::number(numRequirePorts));
   result.append(')');
   return result;
}

int QApxOStreamBuf::apxDeclarationLine(const char firstChar,const QByteArray &name, const QByteArray &dsg, const QByteArray &attr)
{
   if ( (name.length()>0) && (dsg.length()>0) )
   {
      QByteArray tmp;
      tmp.append(firstChar);
      tmp.append('"');
      tmp.append(name);
      tmp.append('"');
      tmp.append(dsg);
      if (attr.length() > 0)
      {
         tmp.append(":");
         tmp.append(attr);
      }
      tmp.append('\n');
      emit write(tmp);
      return 1;
   }
   return -1;
}

void QApxOStreamBuf::apxDataMsg(quint8 msgType, quint16 portId, const QByteArray &data)
{
   QByteArray tmp;
   QByteArray msg;
   tmp.resize(2);
   msg.append(msgType);
   qToBigEndian<quint16>(portId,(uchar*) tmp.data());
   msg.append(tmp);
   if( data.length()>0)
   {
      msg.append(data);
   }
   emit write(tmp);
}
*/
