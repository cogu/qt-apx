/*
 * This file was written early 2015 and was originally an implementation of APX/1.1
 * It works for APX/1.2 but it has lots of unused parts/feature.
 */

///TODO: Clean up this mess of a file

#ifndef QAPXSTREAM_H
#define QAPXSTREAM_H

#include <QtGlobal>
#include <QObject>
#include <QByteArray>
#include <QIODevice>

#define APX_PARSE_STATE_NONE              0
#define APX_PARSE_STATE_HEADER            1
#define APX_PARSE_STATE_NODENAME_RQST     2
#define APX_PARSE_STATE_NODENAME_RSP      3
#define APX_PARSE_STATE_NODEQUERY_RQST    4
#define APX_PARSE_STATE_NODEQUERY_RSP     5
#define APX_PARSE_STATE_BINARY            6

#define APX_MSG_TYPE_NONE           0
#define APX_MSG_TYPE_NODEQUERY_RSP  1

#define APX_EXCEPTION_NO_EXCEPTION                 0
#define APX_EXCEPTION_NOT_IMPLEMENTED              1
#define APX_EXCEPTION_INVALID_DECLARATION_LINE     2


struct ApxDeclarationLine {
   QByteArray name;
   QByteArray dsg;
   QByteArray attr;
   quint8 lineType;
};

struct ApxHeaderLine {
  int majorVersion;
  int minorVersion;
};


class QApxIStreamEventHandler
{
public:
    QApxIStreamEventHandler(){}
    virtual ~QApxIStreamEventHandler(){}
    virtual void apx_istream_open()=0;
    virtual void apx_istream_close()=0;
    virtual void apx_istream_nodeNameRqst()=0;
    virtual void apx_istream_nodeNameRsp(const QByteArray &name)=0;
    virtual void apx_istream_nodeQueryRqst(const QByteArray &name)=0;
    virtual void apx_istream_nodeQueryRspError(const QByteArray &name)=0;
    virtual void apx_istream_nodeQueryRspStart(const QByteArray &name,const QByteArray &dim)=0;
    virtual void apx_istream_require(const QByteArray &name,const QByteArray &dsg, const QByteArray &attr)=0;
    virtual void apx_istream_provide(const QByteArray &name,const QByteArray &dsg, const QByteArray &attr)=0;
    virtual void apx_istream_typedef(const QByteArray &name,const QByteArray &dsg, const QByteArray &attr)=0;
    virtual void apx_istream_requirePortConnect(quint16 portId,const quint32 dataLen)=0;
    virtual void apx_istream_requirePortDisconnect(quint16 portId)=0;
    virtual void apx_istream_providePortConnect(quint16 portId,const quint32 dataLen)=0;
    virtual void apx_istream_providePortDisconnect(quint16 portId)=0;
    virtual void apx_istream_requirePortError(quint16 portId)=0;
    virtual void apx_istream_providePortError(quint16 portId)=0;
    virtual void apx_istream_endTextMsg()=0;
    virtual void apx_istream_dataRead(quint16 portId)=0;
    virtual void apx_istream_dataWrite(quint16 portId,const QByteArray &msg)=0;
    virtual void apx_istream_opInvoke(quint16 portId,const QByteArray &msg)=0;
    virtual void apx_istream_opReturn(quint16 portId,const QByteArray &msg)=0;
};

class QApxIStreamBuf //: public QObject
{
//    Q_OBJECT
public:
   QApxIStreamBuf();
   void setInputHandler(QApxIStreamEventHandler *handler){mEventHandler = handler;}
   void open();
   void close();
   bool isOpen(){return mIsOpen;}
   int getLastMsgType(){return mLastMsgType;}
   void parseBuffer();
   void write(QByteArray &chunk);
   int getException(){return mException;}

protected:
   void reset();
   void resetParseState();
   bool parseLine(const quint8 *pBegin, const quint8 *pEnd);
   const quint8 * splitDeclarationLine(const quint8 *pBegin,const quint8 *pEnd, ApxDeclarationLine *data);
   const quint8 *parseApxHeaderLine(const quint8 *pBegin, const quint8 *pEnd, ApxHeaderLine *data);
   void parseDeclaration(ApxDeclarationLine *decl);
   const quint8 *parseDataMsg(const quint8 *pBegin,const quint8 *pEnd);
   quint32 parseDataLen(const QByteArray &msg);

protected:
   int mParseState;
   QByteArray m_buf;
   QApxIStreamEventHandler *mEventHandler;
   bool mIsOpen;
   int mLastMsgType;
   int mException;
   int mLine;
/*
public slots:
    void open();
    void close();
    */
};


class QApxOStreamBuf : public QObject
{
   Q_OBJECT
public:
   QApxOStreamBuf(QObject *parent=0);
   ~QApxOStreamBuf();
   //APX header

   //APX text messages
   void apxStartTextMsg();
   void apxEndTextMsg();
   void apxNodeNameRqst();
   int apxNodeNameRsp(const QByteArray &name);
   int apxNodeQueryRqst(const QByteArray &name);
   int apxNodeQueryRspStart(const QByteArray &name,int numTypedefs, int numProvidePorts, int numRequirePorts);
   int apxNodeRequire(const QByteArray &name,const QByteArray &dsg, const QByteArray &attr);
   int apxNodeProvide(const QByteArray &name,const QByteArray &dsg, const QByteArray &attr);
   int apxNodeTypedef(const QByteArray &name,const QByteArray &dsg, const QByteArray &attr);

   //APX binary messages   
   void apxRequirePortConnect(quint16 portId, quint32 dataLen);
   void apxRequirePortDisconnect(quint16 portId);
   void apxProvidePortConnect(quint16 portId, quint32 dataLen);
   void apxProvidePortDisconnect(quint16 portId);
   void apxRequirePortError(quint16 portId);
   void apxProvidePortError(quint16 portId);
   void apxDataRead(quint16 portId);
   void apxDataWrite(quint16 portId, const QByteArray &data);
   void apxOpInvoke(quint16 portId, const QByteArray &data);
   void apxOpReturn(quint16 portId,const QByteArray &msg);

   //utility functions
   void clear();
   void transmit();
   int length() const {return mBuf.length();}
protected:
   QByteArray nodeDim(int numTypedefs, int numProvidePorts, int numRequirePorts);
   int apxDeclarationLine(const char firstChar, const QByteArray &name, const QByteArray &dsg, const QByteArray &attr);
   void apxDataMsg(quint8 msgId, quint16 portId, const QByteArray &data);
protected:
   QByteArray mBuf;
   static const int mMajorVersion=1;
   static const int mMinorVersion=2;
signals:
   void write(QByteArray chunk);
};

/*
class QApxInStream
{
public:
   void SetDevice(QIODevice *d);
protected:
};

class QApxOutStream
{
public:
   QApxOutStream();
   QApxOutStream(QIODevice *d);
   ~QApxOutStream();
   //APX text messages
   void apxStartTextMsg();
   void apxEndTextMsg();
   void apxNodeNameRqst();
   int apxNodeNameRsp(const QByteArray &name);
   int apxNodeQueryRqst(const QByteArray &name);
   int apxNodeQueryRspStart(const QByteArray &name,int numTypedefs, int numProvidePorts, int numRequirePorts);
   int apxNodeRequire(const QByteArray &name,const QByteArray &dsg, const QByteArray &attr);
   int apxNodeProvide(const QByteArray &name,const QByteArray &dsg, const QByteArray &attr);
   int apxNodeTypedef(const QByteArray &name,const QByteArray &dsg, const QByteArray &attr);

   //APX binary messages
   void apxRequirePortConnect(quint16 portId);
   void apxRequirePortDisconnect(quint16 portId);
   void apxProvidePortConnect(quint16 portId);
   void apxProvidePortDisconnect(quint16 portId);
   void apxRequirePortError(quint16 portId);
   void apxProvidePortError(quint16 portId);
   void apxDataRead(quint16 portId);
   void apxDataWrite(quint16 portId, const QByteArray &data);
   void apxOpInvoke(quint16 portId, const QByteArray &data);
   void apxOpReturn(quint16 portId,const QByteArray &msg);

   void SetDevice(QIODevice *d){mDevice=d;}
protected:
   QByteArray nodeDim(int numTypedefs, int numProvidePorts, int numRequirePorts);
   int apxDeclarationLine(const char firstChar, const QByteArray &name, const QByteArray &dsg, const QByteArray &attr);
   void apxDataMsg(quint8 msgId, quint16 portId, const QByteArray &data);
protected:
   static const int mMajorVersion=1;
   static const int mMinorVersion=2;
   QIODevice *mDevice;
protected:
};
*/
#endif // QAPXSTREAM_H
