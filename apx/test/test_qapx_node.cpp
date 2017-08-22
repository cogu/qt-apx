#include "test_qapx_node.h"
#include "qapxsimplenode.h"

void TestApxNode::test_createNode()
{
   QApxDataType *datatype;
   QApxSimpleNode *node = new QApxSimpleNode();
   QByteArray name("TestNode");
   node->setName(name);
   datatype = new QApxDataType("InactiveActive_T","C(0,3))","");
   node->appendType(datatype);

   delete node;
}
