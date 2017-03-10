#ifndef QAPXSIMPLEPORT_H
#define QAPXSIMPLEPORT_H

class QApxSimplePort
{
public:
   QApxSimplePort(const char *name, const char *dsg, const char *attr);
   virtual ~QApxSimplePort();
   const char *getName(){return mName;}
   const char *getDataSignature(){return mDsg;}
   const char *getAttr(){return mAttr;}
   virtual bool isRequire() = 0;
   int getPortIndex(){return mPortIndex;}
   void setPortIndex(int value){mPortIndex=value;}
protected:
   char *mName; //strong reference to name
   char *mDsg; //strong reference to data signature
   char *mAttr; //strong reference to port attribute string
   int mPortIndex;
};

class QApxSimpleRequirePort : public QApxSimplePort
{
public:
   QApxSimpleRequirePort(const char *name, const char *dsg, const char *attr);
   bool isRequire(){return true;}
};

class QApxSimpleProvidePort : public QApxSimplePort
{
public:
   QApxSimpleProvidePort(const char *name, const char *dsg, const char *attr);
   bool isRequire(){return false;}
};


#endif // QAPXSIMPLEPORT_H
