#include "test_qrmf_socketadapter.h"
#include "mock_receive_handler.h"
#include "qrmf_socketadapter.h"
#include "mock_socket.h"

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
   socket.receive("\x08\xbF\xFF\xFC\x00\x00\x00\x00\x00", 9);
   sockAdapter.onReadyread();
   QCOMPARE(receiveHandler.transmitHandler, &sockAdapter);
}

void TestSocketAdapter::test_onReadyRead()
{

}

void TestSocketAdapter::test_onReadyReadPartial()
{
   MockReceiveHandler receiveHandler;
   SocketAdapter sockAdapter;
   sockAdapter.setReceiveHandler(&receiveHandler);
   sockAdapter.onConnected();
   MockSocket socket;
   QCOMPARE(sockAdapter.connectMock(&socket), 0);
   QCOMPARE(receiveHandler.transmitHandler, (RemoteFile::TransmitHandler*)NULL);
   QCOMPARE(receiveHandler.messages.length(), 0);
   socket.receive("\x08\xbF\xFF\xFC\x00\x00\x00\x00\x00", 9);
   sockAdapter.onReadyread();
   QCOMPARE(receiveHandler.messages.length(), 0);
   socket.receive("\x0E\xb0\x66\x00\x00\x01\x02\x03",8);
   sockAdapter.onReadyread();
   QCOMPARE(receiveHandler.messages.length(), 0);
   socket.receive("\x04\x05\x06\x07\x08\x09\x0A",7);
   sockAdapter.onReadyread();
   QCOMPARE(receiveHandler.messages.length(), 1);
   QCOMPARE(*receiveHandler.messages.at(0), QByteArray("\xb0\x66\x00\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0A", 14));
}

}
