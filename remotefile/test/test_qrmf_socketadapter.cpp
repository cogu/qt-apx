#include "test_qrmf_socketadapter.h"
#include "mock_receive_handler.h"
#include "qrmf_socketadapter.h"
#include "mock_socket.h"
#include "qnumheader.h"
#include <cstdio>
#include <cstring>

#define ACK_MSG_LEN 9
#define HEADER_LEN_LONG 4

static const char * acknowledge_msg = "\x08\xbF\xFF\xFC\x00\x00\x00\x00\x00";
static const char * ipsum_lorem_msg = "Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum";


namespace RemoteFile
{

void TestSocketAdapter::test_acknowledge()
{
   MockReceiveHandler receiveHandler;
   SocketAdapter sockAdapter;
   sockAdapter.setReceiveHandler(&receiveHandler);
   sockAdapter.onConnected();
   MockSocket socket;
   QCOMPARE(sockAdapter.connectMock(&socket), 0);
   QCOMPARE(receiveHandler.transmitHandler, (RemoteFile::TransmitHandler*)NULL);
   socket.receive(acknowledge_msg, ACK_MSG_LEN);
   sockAdapter.onReadyread();
   QCOMPARE(receiveHandler.transmitHandler, &sockAdapter);
}

void TestSocketAdapter::test_onReadyRead()
{

}

void TestSocketAdapter::test_onReadyRead_partial_short_msg()
{
   MockReceiveHandler receiveHandler;
   SocketAdapter sockAdapter;
   sockAdapter.setReceiveHandler(&receiveHandler);
   sockAdapter.onConnected();
   MockSocket socket;
   QCOMPARE(sockAdapter.connectMock(&socket), 0);   
   QCOMPARE(receiveHandler.messages.length(), 0);
   socket.receive(acknowledge_msg, ACK_MSG_LEN);
   sockAdapter.onReadyread();
   QCOMPARE(receiveHandler.messages.length(), 0);
   socket.receive("\x0E\xb0\x66\x00\x00\x01\x02\x03",8);
   sockAdapter.onReadyread();
   QCOMPARE(receiveHandler.messages.length(), 0);
   socket.receive("\x04\x05\x06\x07\x08\x09\x0A",7);
   sockAdapter.onReadyread();
   QCOMPARE(receiveHandler.messages.length(), 1);
   QCOMPARE(*receiveHandler.messages.at(0), QByteArray("\xb0\x66\x00\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0A", 14));
   receiveHandler.clearMessages();
   QCOMPARE(sockAdapter.getRxPending(), 0);


   //try again but this time feed it 1 character at a time
   const char *full_msg = "\x0E\xb0\x66\x00\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0A";
   int full_msg_len = 15;
   for (int i=0;i<full_msg_len-1;i++)
   {
       socket.receive(&full_msg[i],1);
       sockAdapter.onReadyread();
       QCOMPARE(receiveHandler.messages.length(), 0);
   }
   socket.receive(&full_msg[full_msg_len-1],1);
   sockAdapter.onReadyread();
   QCOMPARE(receiveHandler.messages.length(), 1);
   QCOMPARE(*receiveHandler.messages.at(0), QByteArray("\xb0\x66\x00\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0A", 14));
}

void TestSocketAdapter::test_onReadyRead_partial_long_msg()
{
   MockReceiveHandler receiveHandler;
   SocketAdapter sockAdapter;
   sockAdapter.setReceiveHandler(&receiveHandler);
   sockAdapter.onConnected();
   MockSocket socket;
   QCOMPARE(sockAdapter.connectMock(&socket), 0);
   socket.receive(acknowledge_msg, ACK_MSG_LEN);
   sockAdapter.onReadyread();
   QCOMPARE(receiveHandler.messages.length(), 0);
   quint32 long_msg_len = (quint32) strlen(ipsum_lorem_msg);

   char *msg = new char[long_msg_len+HEADER_LEN_LONG];
   QCOMPARE(NumHeader::encode32(&msg[0], (int) HEADER_LEN_LONG, long_msg_len), (int) HEADER_LEN_LONG);
   quint32 total_msg_len = long_msg_len+HEADER_LEN_LONG;
   memcpy(&msg[HEADER_LEN_LONG], &ipsum_lorem_msg[0], long_msg_len);
   QCOMPARE(msg[total_msg_len-1], 'm');
   for (quint32 i=0;i<total_msg_len-1;i++)
   {
       socket.receive(&msg[i],1);
       sockAdapter.onReadyread();
       QCOMPARE(receiveHandler.messages.length(), 0);
   }
   socket.receive(&msg[total_msg_len-1],1);
   sockAdapter.onReadyread();
   QCOMPARE(receiveHandler.messages.length(), 1);
   QCOMPARE(*receiveHandler.messages.at(0), QByteArray(ipsum_lorem_msg));

   delete[] msg;
}

void TestSocketAdapter::test_getSocketReadAvail_single()
{
   MockReceiveHandler receiveHandler;
   SocketAdapter sockAdapter;
   sockAdapter.setReceiveHandler(&receiveHandler);
   sockAdapter.onConnected();
   MockSocket socket;
   QCOMPARE(sockAdapter.connectMock(&socket), 0);
   QCOMPARE(sockAdapter.getSocketReadAvail(), (qint64) 0);
   socket.receive(acknowledge_msg, ACK_MSG_LEN);
   QCOMPARE(sockAdapter.getSocketReadAvail(), (qint64) ACK_MSG_LEN);
}

void TestSocketAdapter::test_getSocketReadAvail_multi()
{
   MockReceiveHandler receiveHandler;
   SocketAdapter sockAdapter;
   sockAdapter.setReceiveHandler(&receiveHandler);
   sockAdapter.onConnected();
   MockSocket socket;
   const char *msg1="abcdefgh";
   const char *msg2="ijklmn";
   const char *msg3="opqr";
   QCOMPARE(sockAdapter.connectMock(&socket), 0);
   QCOMPARE(sockAdapter.getSocketReadAvail(), (qint64) 0);
   socket.receive(msg1, (int) strlen(msg1));
   socket.receive(msg2, (int) strlen(msg2));
   socket.receive(msg3, (int) strlen(msg3));
   QCOMPARE(sockAdapter.getSocketReadAvail(), (qint64) strlen(msg1));
   socket.dropOne();
   QCOMPARE(sockAdapter.getSocketReadAvail(), (qint64) strlen(msg2));
   socket.dropOne();
   QCOMPARE(sockAdapter.getSocketReadAvail(), (qint64) strlen(msg3));
   socket.dropOne();
   QCOMPARE(sockAdapter.getSocketReadAvail(), (qint64) 0);
}

void TestSocketAdapter::test_getSocketReadAvail_not_connected()
{
   MockReceiveHandler receiveHandler;
   SocketAdapter sockAdapter;
   sockAdapter.setReceiveHandler(&receiveHandler);
   sockAdapter.onConnected();
   MockSocket socket;
   const char *msg1="abcdefgh";
   QCOMPARE(sockAdapter.connectMock(&socket), 0);
   QCOMPARE(sockAdapter.getSocketReadAvail(), (qint64) 0);
   socket.receive(msg1, (int) strlen(msg1));
   socket.setConnectionState(false); //some kind of error occured here that disconnected the socket
   QCOMPARE(sockAdapter.getSocketReadAvail(), (qint64) -1);
}

void TestSocketAdapter::test_bad_message()
{
   MockReceiveHandler receiveHandler;
   SocketAdapter sockAdapter;
   sockAdapter.setReceiveHandler(&receiveHandler);
   sockAdapter.onConnected();
   MockSocket socket;
   QCOMPARE(sockAdapter.connectMock(&socket), 0);
   QCOMPARE(receiveHandler.transmitHandler, (RemoteFile::TransmitHandler*)NULL);
   socket.receive(acknowledge_msg, ACK_MSG_LEN);
   sockAdapter.onReadyread();
   QCOMPARE(receiveHandler.messages.length(), 0);
   const char *msg1="\04abcd";
   socket.receive(msg1, (int) strlen(msg1));
   receiveHandler.setParseResult(false);
   QCOMPARE(sockAdapter.getError(), RMF_ERR_NONE);
   sockAdapter.onReadyread();
   QCOMPARE(sockAdapter.getError(), RMF_ERR_BAD_MSG);
}

}
