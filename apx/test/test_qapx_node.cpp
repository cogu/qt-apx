#include "test_qapx_node.h"
#include "qapx_node.h"

void TestApxNode::test_createNode()
{
   QApxDataType *datatype;
   Apx::Node *node = new Apx::Node();
   QByteArray name("TestNode");
   node->setName(name);
   datatype = new QApxDataType("InactiveActive_T","C(0,3))","");
   node->appendType(datatype);

   delete node;
}
