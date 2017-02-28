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
}


#endif // QAPX_EXCEPTION_H
