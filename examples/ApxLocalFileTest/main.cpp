#include <QCoreApplication>
#include <QDebug>
#include "qapx_node.h"
#include "qapx_bytearrayparser.h"
#ifdef _MSC_VER
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif



int main(int argc, char *argv[])
{
#ifdef _MSC_VER
   _CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif
   //QCoreApplication a(argc, argv);
   Apx::Node *node;
   const char *apx_str =
   "APX/1.2\n"
   "N\"Simulator\"\n"
   "T\"InactiveActive_T\"C(0,3)\n"
   "P\"VehicleSpeed\"S:=65535\n"
   "P\"MainBeam\"T[0]:=3\n"
   "P\"FuelLevel\"C\n"
   "P\"ParkBrakeActive\"T[0]:=3\n"
   "R\"RheostatLevel\"C:=255\n";
   QByteArray definition(apx_str);

   Apx::ByteArrayParser parser;
   node = parser.parseNode(definition);
   delete node;
   //return a.exec();
}
