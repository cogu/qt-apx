#ifndef QAPXDATATYPE_H
#define QAPXDATATYPE_H



class QApxDataType
{
public:
   QApxDataType(const char *name, const char *dsg, const char *attr);
   ~QApxDataType();
   const char* getName() {return mName;}
   const char* getDataSignature() {return mDsg;}
   const char* getAttr() {return mAttr;}
protected:
   char *mName;
   char *mDsg;
   char *mAttr;
};

#endif // QAPXDATATYPE_H
