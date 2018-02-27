#include <QCoreApplication>
#include "qapx_client.h"
#include "stressnode.h"

int main(int argc, char *argv[])
{
   QCoreApplication a(argc, argv);

   const char *apxText =
   "APX/1.2\n"
   "N\"QStressNode\"\n"
   "P\"QTestSignal1\"S:=0\n"
   "P\"QTestSignal2\"S:=0\n"
   "P\"QTestSignal3\"S:=0\n"
   "R\"PTestSignal1\"S:=0\n"
   "R\"PTestSignal2\"S:=0\n"
   "R\"PTestSignal3\"S:=0\n"
   "\n";


   Apx::Client client;

   client.createLocalNode(apxText);
   client.connectTcp(QHostAddress::LocalHost, 5000);
   StressNode stressNode(&client, &a);
   stressNode.start();

   return a.exec();
}
