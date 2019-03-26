#include <QCoreApplication>
#include <QDebug>
#include "qapx_nodedata.h"
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
   const char *apx_str =
   "APX/1.2\n"
   "N\"Simulator\"\n"
   "T\"InactiveActive_T\"C(0,3)\n"
   "R\"VehicleSpeed\"S:=65535\n"
   "R\"MainBeam\"T[0]:=3\n"
   "R\"FuelLevel\"C\n"
   "R\"ParkBrakeActive\"T[0]:=3\n"
   "R\"RheostatLevel\"C:=255\n";
   QByteArray apx_text(apx_str);
   Apx::NodeData *nodeData = new Apx::NodeData(apx_text);
   Apx::File *outDataFile = nodeData->getOutPortDataFile();
   Apx::File *inDataFile = nodeData->getInPortDataFile();
   if (inDataFile != nullptr)
   {
      qDebug() << inDataFile->mLength;
   }
   if (outDataFile != nullptr)
   {
      qDebug() << outDataFile->mLength;
   }
   QByteArray dummyData(6, 0);
   qDebug() << dummyData.length();
   quint8 *pNext = (quint8*) dummyData.data();
   for (int i=0;i<6; i++)
   {
      *pNext++ = i;
   }
   inDataFile->write((quint8*) dummyData.data(), 0, dummyData.length());
   for (int i=0; i<5;i++)
   {
      QVariant value;
      bool success = nodeData->getRequirePortValue(i, value);
      if (success)
      {
         qDebug() << value;
      }
   }


   delete nodeData;
   //return a.exec();
}
