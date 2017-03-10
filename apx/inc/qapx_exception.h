/*
 * Declares APX-related C++ exceptions
 */
#ifndef QAPX_EXCEPTION_H
#define QAPX_EXCEPTION_H
#include <exception>

namespace Apx
{
   class ParseException : public std::runtime_error
   {
   public:
      ParseException(const char *message): std::runtime_error(message){}
   };

   class CompilerException : public std::runtime_error
   {
   public:
      CompilerException(const char *message): std::runtime_error(message){}
   };

   class VariantTypeException : public std::runtime_error
   {
   public:
      VariantTypeException(const char *message): std::runtime_error(message){}
   };

   class VMException : public std::runtime_error
   {
   public:
      VMException(const char *message): std::runtime_error(message){}
   };
}


#endif // QAPX_EXCEPTION_H
