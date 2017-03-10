/**
* QT version of the apx_file_t struct found in C implementation of APX
*/
#ifndef QAPX_FILE_H
#define QAPX_FILE_H
#include <QByteArray>
#include <QMutex>
#include "qrmf_file.h"
#include "qrmf_filemanager.h"

namespace Apx
{

   //forward declarations
   class NodeDataHandler;

   /**
    * @brief APX File base class
    */
   class File : public RemoteFile::File
   {
   public:      
      File(QString name, quint32 length);
      virtual ~File();
      QByteArray &getFileData(){return mData;}
      virtual int read(quint8 *pDest, quint32 offset, quint32 length);
      virtual int write(const quint8 *pSrc, quint32 offset, quint32 length);
   protected:
      QByteArray mData; //this is the raw data array. It's length is specified by length argument in the File constructor. Access to it is protected by dataLock variable
      QMutex mDataLock; //read/write lock for the data member variable
      NodeDataHandler *mNodeDataHandler;

   };

   /**
    * @brief InputFile represents input files like nodename.in
    * They exports a write interface for the Apx::FileManager to use and notifies application layer when data has changed
    * From FileManager perspective these are called remote files
    */
   class InputFile : public Apx::File
   {
   public:
      InputFile(QString name, quint32 length, NodeDataHandler *handler = NULL);
      virtual ~InputFile(){}
      void setNodeDataHandler(NodeDataHandler *handler) {mNodeDataHandler = handler;}
      NodeDataHandler *getNodeDataHandler() {return mNodeDataHandler;}
      virtual int write(const quint8 *pSrc, quint32 offset, quint32 length);
   };


   /**
    * @brief OutputFile represents output files like nodename.out and nodename.apx
    * They exports a read interface for the Apx::FileManager to use and allows application layer to write to it using a write interface.
    * from FileManager perspective, these are called local files
    */
   class OutputFile : public Apx::File
   {
   public:
      OutputFile(QString name, quint32 length);
      virtual ~OutputFile();
      virtual int write(const quint8 *pSrc, quint32 offset, quint32 length);
   };


}


#endif // QAPX_FILE_H
