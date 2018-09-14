#ifndef QAPX_PROGRAM_BASE_H
#define QAPX_PROGRAM_BASE_H

//The opcode is 1 byte in length
                                    //extra bytes                                                  comments
#define OPCODE_NOP               0
#define OPCODE_ARGS              1  //5: progTypeByte1, variantTypeByte1, lengthByte1-lengthByte3  progTypeByte1 tells whether we are packing (0) or unpacking (1) data. variantTypeByte1 is  an integer of type QVariant::Type, lengthBytes1-3 is a 24-bit integer that tells the total number of bytes to pack or unpack
#define OPCODE_UNPACK_U8         2  //                                                             unpack single uint8 (1 byte)
#define OPCODE_UNPACK_U16        3  //                                                             unpack single uint16 (2 bytes)
#define OPCODE_UNPACK_U32        4  //                                                             unpack single uint32 (4 bytes)
#define OPCODE_UNPACK_S8         5  //                                                             unpack single sint8 (4 bytes)
#define OPCODE_UNPACK_S16        6  //                                                             unpack single sint16 (2 bytes)
#define OPCODE_UNPACK_S32        7  //                                                             unpack single sint32 (4 bytes)
#define OPCODE_UNPACK_STR        8  //2: lengthByte1, lengthByte2                                  length=0-65535, number of array elements
#define OPCODE_UNPACK_U8AR       9  //2: lengthByte1, lengthByte2                                  length=0-65535, number of array elements
#define OPCODE_UNPACK_U16AR      10 //2: lengthByte1, lengthByte2                                  length=0-65535, number of array elements
#define OPCODE_UNPACK_U32AR      11 //2: lengthByte1, lengthByte2                                  length=0-65535, number of array elements
#define OPCODE_UNPACK_S8AR       12 //2: lengthByte1, lengthByte2                                  length=0-65535, number of array elements
#define OPCODE_UNPACK_S16AR      13 //2: lengthByte1, lengthByte2                                  length=0-65535, number of array elements
#define OPCODE_UNPACK_S32AR      14 //2: lengthByte1, lengthByte2                                  length=0-65535, number of array elements
#define OPCODE_PACK_U8           15  //                                                            pack single uint8 (1 byte)
#define OPCODE_PACK_U16          16  //                                                            pack single uint16 (2 bytes)
#define OPCODE_PACK_U32          17  //                                                            pack single uint32 (4 bytes)
#define OPCODE_PACK_S8           18  //                                                            pack single sint8 (4 bytes)
#define OPCODE_PACK_S16          19  //                                                            pack single sint16 (2 bytes)
#define OPCODE_PACK_S32          20  //                                                            pack single sint32 (4 bytes)
#define OPCODE_PACK_STR          21 //2: lengthByte1, lengthByte2                                  length=0-65535, number of array elements
#define OPCODE_PACK_U8AR         22 //2: lengthByte1, lengthByte2                                  length=0-65535, number of array elements
#define OPCODE_PACK_U16AR        23 //2: lengthByte1, lengthByte2                                  length=0-65535, number of array elements
#define OPCODE_PACK_U32AR        24 //2: lengthByte1, lengthByte2                                  length=0-65535, number of array elements
#define OPCODE_PACK_S8AR         25 //2: lengthByte1, lengthByte2                                  length=0-65535, number of array elements
#define OPCODE_PACK_S16AR        26 //2: lengthByte1, lengthByte2                                  length=0-65535, number of array elements
#define OPCODE_PACK_S32AR        27 //2: lengthByte1, lengthByte2                                  length=0-65535, number of array elements
#define OPCODE_RECORD_ENTER      28 //                                                             used for record inside of record or record inside of array
#define OPCODE_RECORD_SELECT     29 //null-terminated string containing field element
#define OPCODE_RECORD_LEAVE      30
#define OPCODE_ARRAY_ENTER       31 //                                                        used only for array of records
#define OPCODE_ARRAY_NEXT        32
#define OPCODE_ARRAY_LEAVE       33


/*
 * All programs must start with an OPCPODE_ARGS stating what QVariant::Type it will operate on as well as
 * the total number of bytes it requires from the data buffer. Note that the maximum data buffer currently supported is
 * 2^24-1 bytes (No vehicle signal can exceed this limit)
 * After the OPCODE_ARGS, any number of operations are allowed in any order until the program ends.
 * Programs are normally stored in QByteArrays.
 * the code uses VLE (Variable Length Encoding) meaning that the start of an OPCODE can be completely unaligned with
 * respect to word-boundaries.
 */


#define PROG_TYPE_PACK           0    //value of progTypeByte1
#define PROG_TYPE_UNPACK         1    //value of progTypeByte1


//don't to rely on QVariant::Type here in case QT team changes the enum definitions in QVariant.h
enum VariantType
{
   VTYPE_INVALID,          //nullptr
   VTYPE_SCALAR,           //QVariant
   VTYPE_MAP,              //QVariantMap
   VTYPE_LIST              //QVariantList
};


#endif // QAPX_PROGRAM_BASE_H
