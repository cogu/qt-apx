#ifndef APX_BASE_TYPES_H
#define APX_BASE_TYPES_H
#include <QtGlobal>
#include <QString>

#define APX_F_IS_CONNECTED       1
#define APX_F_NO_DATA            2

#define APX_MSG_RESERVED         0 //(RESERVED)
#define APX_MSG_READ             1 //RECEIVE
#define APX_MSG_WRITE            2 //SEND
#define APX_MSG_INVOKE           3 //REQUEST
#define APX_MSG_RETURN           4 //RESPONSE
#define APX_MSG_RPORT_CONNECT    5 //+R
#define APX_MSG_RPORT_DISCONNECT 6 //-R
#define APX_MSG_PPORT_CONNECT    7 //+P
#define APX_MSG_PPORT_DISCONNECT 8 //-P
#define APX_MSG_RPORT_ERROR      9 //!R
#define APX_MSG_PPORT_ERROR      10 //!P
#define APX_MSG_MASK             0x0F

#define APX_BINARY_FLAG          0x80

#define APX_MODE_MASTER 0
#define APX_MODE_SLAVE  1

#define APX_LEN_INHERIT    0
#define APX_LEN_BYTE       5
#define APX_LEN_SHORT      6
#define APX_LEN_LONG       7
#define APX_LEN_MASK       7
#define APX_LEN_STARTBIT   4




void QApxBuildPortSignature(QString &result,const char *name,const char *dsg);
#endif // APX_BASE_TYPES_H
