#include <QByteArray>
#include "test_qapx_bytearrayparser.h"
#include "qapxsimplenode.h"
#include "qapxbytearrayparser.h"

void TestApxByteParser::parseNormal()
{
   QApxByteArrayParser apxByteArrayParser;
   const char *apx_str =
   "APX/1.2\n"
   "N\"Simulator\"\n"
   "T\"InactiveActive_T\"C(0,3)\n"
   "P\"VehicleSpeed\"S:=65535\n"
   "P\"MainBeam\"T[0]:=3\n"
   "P\"FuelLevel\"C\n"
   "P\"ParkBrakeActive\"T[0]:=3\n"
   "R\"RheostatLevel\"C:=255\n";

   QByteArray bytes(apx_str);
   QApxSimpleNode *node = apxByteArrayParser.parseNode(bytes);
   QVERIFY(node != NULL);
   delete node;
}

/**
 * @brief test case for broken/partial APX file
 */
void TestApxByteParser::parseBroken()
{
   QApxByteArrayParser apxByteArrayParser;
   const char *apx_str =
   "APX/1.2\n"
   "N\"Simulator\"\n"
   "T\"InactiveActive_T\"C(0,3)\n"
   "P\"VehicleSpeed\"S:=65535\n"
   "P\"MainBeam\"T[0]:=3\n"
   "P\"FuelLevel\"C\n"
   "P\"ParkBrakeActive\"T[0]:=3\n"
   "R\"Rheost"; //this is a broken APX line

   QByteArray bytes(apx_str);
   QApxSimpleNode *node = apxByteArrayParser.parseNode(bytes);
   QVERIFY(node == NULL);
}
