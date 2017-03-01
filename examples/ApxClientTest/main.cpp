#include <QCoreApplication>
#include "qapx_client.h"
#include "simulator.h"

int main(int argc, char *argv[])
{
   QCoreApplication a(argc, argv);

   const char *apxText =
   "APX/1.2\n"
   "N\"Simulator\"\n"
   "T\"InactiveActive_T\"C(0,3)\n"
   "P\"VehicleSpeed\"S:=65535\n"
   "P\"MainBeam\"C(0,3):=3\n"
   "P\"FuelLevel\"C\n"
   "P\"ParkBrakeFault\"T[0]:=3\n"
   "R\"TurnIndicator_StalkSLevel\"C(0,7):=7\n"
   "R\"RheostatLevel\"C:=255\n";

   Apx::Client client;

   client.createLocalNode(apxText);
   client.connectTcp(QHostAddress::LocalHost, 5000);
   SimulatorNode simulator;
   QObject::connect(&client, Apx::Client::requirePortData, &simulator, SimulatorNode::onRequirePortData);

   return a.exec();
}
