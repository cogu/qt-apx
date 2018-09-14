#include <QtEndian>
#include "qnumheader.h"

namespace NumHeader {

/**
 * @brief encode16
 * @param pDest
 * @param destLimit
 * @param value
 * @return Number of bytes written into pDest. Valid values are 0,1 and 2.
 *         0 Is returned when destLimit was set to 1 but encode16 needs 2 bytes in pDest to encode the number.
 *         -1 is returned when value is outside valid range.
 */
int encode16(char *pDest, int destLimit, quint16 value)
{
   int retval=0; //default is to write 0 bytes intp pDest
   uchar *p = (uchar*) pDest;
   if (value <= NUMHEADER_SHORT_MSG_LIMIT)
   {
      if (destLimit>=(int)sizeof(quint8))
      {
         retval=1; //write 1 intp pDest
         *p = (uchar) value;
      }
   }
   else if ( value <= NUMHEADER16_LONG_MSG_LIMIT )
   {
      if (destLimit>=(int)sizeof(quint16))
      {
         retval=(int)sizeof(quint16);
         if (value >= 32768u)
         {
            //this is special handling for reinterpreting the range 0-127 as 32768-32895 when long_bit is set to true
            value -= 32768u;
         }
         qToBigEndian<quint16>(value, p);
         p[0]|=0x80u; //activate long_bit
      }
   }
   else
   {
      //value argument is out of valid range
      retval = -1;
   }
   return retval;
}

/**
 * @brief decode16
 * @param pBegin
 * @param pEnd
 * @param value
 * @return number of bytes read from pBegin. Valid values are 0, 1 and 2. -1 is returned when arguments are invalid (e.g. if value is nullptr or pEnd<pBegin)
 */
int decode16(const char* const pBegin, const char* const pEnd, quint16 *value)
{
   int retval = 0; //default is to read 0 bytes starting from pBegin
   if( (pBegin != nullptr) && (pBegin<pEnd) && (value != nullptr) )
   {
      const uchar c = *(const uchar* const)pBegin;
      if(c & 0x80u) //is long_bit set?
      {
         if(pBegin+sizeof(quint16)<=pEnd)
         {
            retval = (int) sizeof(quint16);
            quint16 tmp = qFromBigEndian<quint16>(pBegin);
            tmp&=(quint16)0x7FFFu; //clear the long bit
            if(tmp<128u)
            {
               tmp+=32768u; //interpret range 0-127 as range 32768-32895 when long_bit was set to 1
            }
            *value=tmp;
         }
      }
      else
      {
         retval = (int) sizeof(quint8);
         *value=(const quint8) c;
      }
   }
   else
   {
      //one or more arguments are invalid
      retval = -1;
   }
   return retval;
}

int encode32(char *pDest, int destLimit, quint32 value)
{
   int retval=0; //default is to write 0 bytes intp pDest
   uchar *p = (uchar*) pDest;
   if (value <= NUMHEADER_SHORT_MSG_LIMIT)
   {
      if (destLimit>=(int)sizeof(quint8))
      {
         retval=1; //write 1 intp pDest
         *p = (uchar) value;
      }
   }
   else if ( value <= NUMHEADER32_LONG_MSG_LIMIT )
   {
      if (destLimit>=(int)sizeof(quint32))
      {
         retval=(int)sizeof(quint32);
         qToBigEndian<quint32>(value, p);
         p[0]|=0x80u; //activate long_bit
      }
   }
   else
   {
      //value argument is out of valid range
      retval = -1;
   }
   return retval;
}

int decode32(const char* const pBegin, const char* const pEnd, quint32 *value)
{
   int retval = 0; //default is to read 0 bytes starting from pBegin
   if( (pBegin != 0) && (pBegin<pEnd) && (value != nullptr) )
   {
      const uchar c = *(const uchar* const)pBegin;
      if(c & 0x80u) //is long_bit set?
      {
         if(pBegin+sizeof(quint32)<=pEnd)
         {
            retval = (int) sizeof(quint32);
            quint32 tmp = qFromBigEndian<quint32>(pBegin);
            tmp&=(quint32)0x7FFFFFFFu; //clear the long bit
            *value=tmp;
         }
      }
      else
      {
         retval = (int) sizeof(quint8);
         *value=(const quint8) c;
      }
   }
   else
   {
      //one or more arguments are invalid
      retval = -1;
   }
   return retval;
}


} //namespace
