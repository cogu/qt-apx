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

    char header[HEADER_LEN_LONG];
    QCOMPARE(NumHeader::encode32(&header[0], (int) sizeof(header), long_msg_len), (int) HEADER_LEN_LONG);



}

}
