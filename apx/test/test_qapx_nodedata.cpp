#include <cstring>
#include "test_qapx_nodedata.h"
#include "qapx_nodedata.h"

using namespace std;

void TestApxNodeData::test_createNodeDataDualNewline()
{
   const char *apxText =
   "APX/1.2\n"
   "N\"Simulator\"\n"
   "T\"InactiveActive_T\"C(0,3)\n"
   "P\"VehicleSpeed\"S:=65535\n"
   "P\"MainBeam\"C(0,3):=3\n"
   "P\"FuelLevel\"C\n"
   "R\"TurnIndicator_StalkSLevel\"C(0,7):=7\n"
   "P\"ParkBrakeFault\"T[0]:=3\n"
   "R\"RheostatLevel\"C:=255\n"
   "\n";

   Apx::NodeData nodeData = Apx::NodeData(apxText);

   Apx::File *inputFile = nodeData.getInPortDataFile();
   Apx::File *outputFile = nodeData.getOutPortDataFile();
   Apx::File *definitionFile = nodeData.getDefinitionFile();
   QVERIFY (inputFile != nullptr);
   QVERIFY (outputFile != nullptr);
   QVERIFY (definitionFile != nullptr);
   QCOMPARE(inputFile->mLength, (quint32) 2);
   QCOMPARE(outputFile->mLength, (quint32) 5);
   QCOMPARE(definitionFile->mLength, (quint32) strlen(apxText));
}

void TestApxNodeData::test_createNodeData()
{
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

   Apx::NodeData *nodeData = new Apx::NodeData(apxText);

   Apx::File *inputFile = nodeData->getInPortDataFile();
   Apx::File *outputFile = nodeData->getOutPortDataFile();
   Apx::File *definitionFile = nodeData->getDefinitionFile();
   QVERIFY (inputFile != nullptr);
   QVERIFY (outputFile != nullptr);
   QVERIFY (definitionFile != nullptr);
   QCOMPARE(inputFile->mLength, (quint32) 2);
   QCOMPARE(outputFile->mLength, (quint32) 5);
   QCOMPARE(definitionFile->mLength, (quint32) strlen(apxText));

   QByteArray definitionFileContent(definitionFile->mLength,0);
   int result = definitionFile->read((quint8*) definitionFileContent.data(), 0, (quint32) definitionFile->mLength);
   QCOMPARE(result, (int) strlen(apxText));
   QCOMPARE(definitionFileContent, QByteArray(apxText));
   QCOMPARE(nodeData->mInPortDataElements.length(), 2);
   QCOMPARE(nodeData->mOutPortDataElements.length(), 4);

   QCOMPARE(nodeData->mInPortDataMapLen, 2);
   QCOMPARE(nodeData->mOutPortDataMapLen, 5);
   QVERIFY(nodeData->mInPortDataMap[0] == &nodeData->mInPortDataElements[0]);
   QVERIFY(nodeData->mInPortDataMap[1] == &nodeData->mInPortDataElements[1]);
   QVERIFY(nodeData->mOutPortDataMap[0] == &nodeData->mOutPortDataElements[0]);
   QVERIFY(nodeData->mOutPortDataMap[1] == &nodeData->mOutPortDataElements[0]); // Two byte speed
   QVERIFY(nodeData->mOutPortDataMap[2] == &nodeData->mOutPortDataElements[1]);
   QVERIFY(nodeData->mOutPortDataMap[3] == &nodeData->mOutPortDataElements[2]);
   QVERIFY(nodeData->mOutPortDataMap[4] == &nodeData->mOutPortDataElements[3]);


   delete nodeData;

}

void TestApxNodeData::test_getRequirePortValue()
{
   const char *apx_str =
      "APX/1.2\n"
      "N\"Simulator\"\n"
      "T\"InactiveActive_T\"C(0,3)\n"
      "R\"VehicleSpeed\"S:=65535\n"
      "R\"MainBeam\"T[0]:=3\n"
      "R\"FuelLevel\"C\n"
      "R\"ParkBrakeActive\"T[0]:=3\n"
      "R\"RheostatLevel\"C:=255";

      QByteArray apx_text(apx_str);
      Apx::NodeData *nodeData = new Apx::NodeData(apx_text);
      Apx::File *outDataFile = nodeData->getOutPortDataFile();
      Apx::File *inDataFile = nodeData->getInPortDataFile();
      Apx::Node *node = nodeData->getNode();
      QVERIFY(node != nullptr);
      QVERIFY(outDataFile == nullptr);
      QVERIFY(inDataFile != nullptr);
      QCOMPARE(inDataFile->mLength, (quint32) 6);
      QCOMPARE(nodeData->mInPortDataElements.length(),5);

      QVariant value;
      bool success = nodeData->getRequirePortValue(node->findPortByName("VehicleSpeed"), value);
      QVERIFY(success == true);
      QVERIFY(value == QVariant((uint) 0)); // Definition init value is not auto set... QVERIFY(value == QVariant((uint) 65535));
      success = nodeData->getRequirePortValue(node->findPortByName("MainBeam"), value);
      QVERIFY(success == true);
      QVERIFY(value == QVariant((uint) 0)); // Definition init value is not auto set... QVERIFY(value == QVariant((uint) 3));
      success = nodeData->getRequirePortValue(node->findPortByName("FuelLevel"), value);
      QVERIFY(success == true);
      QVERIFY(value == QVariant((uint) 0)); // Definition init value is not auto set... QVERIFY(value == QVariant((uint) 3));
      success = nodeData->getRequirePortValue(node->findPortByName("ParkBrakeActive"), value);
      QVERIFY(success == true);
      QVERIFY(value == QVariant((uint) 0)); // Definition init value is not auto set... QVERIFY(value == QVariant((uint) 255));
      success = nodeData->getRequirePortValue(node->findPortByName("RheostatLevel"), value);
      QVERIFY(success == true);
      QVERIFY(value == QVariant((uint) 0)); // Definition init value is not auto set... QVERIFY(value == QVariant((uint) 5));
      success = nodeData->getRequirePortValue(node->findPortByName("NoSuchSignal"), value);
      QVERIFY(success == false);
      success = nodeData->getRequirePortValue(-1, value);
      QVERIFY(success == false);


      QByteArray testData(6, 0);
      quint8 *pNext = (quint8*) testData.data();
      for (int i=0;i<6; i++)
      {
         *pNext++ = i;
      }
      inDataFile->write((quint8*) testData.data(), 0, testData.length());
      success = nodeData->getRequirePortValue(node->findPortByName("VehicleSpeed"), value);
      QVERIFY(success == true);
      QVERIFY(value == QVariant((uint) 256));
      success = nodeData->getRequirePortValue(node->findPortByName("MainBeam"), value);
      QVERIFY(success == true);
      QVERIFY(value == QVariant((uint) 2));
      success = nodeData->getRequirePortValue(node->findPortByName("FuelLevel"), value);
      QVERIFY(success == true);
      QVERIFY(value == QVariant((uint) 3));
      success = nodeData->getRequirePortValue(node->findPortByName("ParkBrakeActive"), value);
      QVERIFY(success == true);
      QVERIFY(value == QVariant((uint) 4));
      success = nodeData->getRequirePortValue(node->findPortByName("RheostatLevel"), value);
      QVERIFY(success == true);
      QVERIFY(value == QVariant((uint) 5));
      success = nodeData->getRequirePortValue(node->findPortByName("NoSuchSignal"), value);
      QVERIFY(success == false);
      success = nodeData->getRequirePortValue(-1, value);
      QVERIFY(success == false);


      delete nodeData;
}
