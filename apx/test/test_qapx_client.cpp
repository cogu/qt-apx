#include "test_qapx_client.h"
#include "qapx_client.h"
#include <QSignalSpy>
#include <QDebug>
#include <QtCore>

void TestApxClient::createClient()
{
   const char *apx_str =
   "APX/1.2\n"
   "N\"Simulator\"\n"
   "T\"InactiveActive_T\"C(0,3)\n"
   "P\"VehicleSpeed\"S:=65535\n"
   "P\"MainBeam\"T[0]:=3\n"
   "P\"FuelLevel\"C\n"
   "P\"ParkBrakeActive\"T[0]:=3\n"
   "R\"RheostatLevel\"C:=255\n";
   Apx::Client *client = new Apx::Client();
   client->createLocalNode(apx_str);
   Apx::NodeData *nodeData = client->getNodeData();
   QVERIFY(nodeData != 0);

   delete client;
}

void TestApxClient::connectToServer()
{
   const char *apx_str =
   "APX/1.2\n"
   "N\"Simulator\"\n"
   "T\"InactiveActive_T\"C(0,3)\n"
   "P\"VehicleSpeed\"S:=65535\n"
   "P\"MainBeam\"T[0]:=3\n"
   "P\"FuelLevel\"C\n"
   "P\"ParkBrakeActive\"T[0]:=3\n"
   "R\"RheostatLevel\"C:=255\n";
   int argc = 0;
   QCoreApplication a(argc, 0);
   Apx::Client *client = new Apx::Client(&a);
   QSignalSpy connectedSpy(client, SIGNAL(connected()));
   QSignalSpy disconnectSpy(client, SIGNAL(disconnected()));
   client->createLocalNode(apx_str);
   Apx::NodeData *nodeData = client->getNodeData();
   QVERIFY(nodeData != 0);
   client->connectTcp(QHostAddress::LocalHost, 5000);
   connectedSpy.wait(5000);
   QCOMPARE(connectedSpy.count(), 1);
   client->close();
   disconnectSpy.wait(5000);
   QCOMPARE(disconnectSpy.count(), 1);

}
