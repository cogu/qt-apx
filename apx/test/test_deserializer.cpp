#include "test_deserializer.h"
#include "qapxdataelement.h"


void TestDeserializer::test_simple()
{
   QApxDataElementParser parser;
   QApxDataElement *dataElement = parser.parseDataSignature((quint8*)"C");
   QCOMPARE(dataElement->baseType, QAPX_BASE_TYPE_UINT8);
   delete dataElement;
}
