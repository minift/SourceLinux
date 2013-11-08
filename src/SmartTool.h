//

#ifndef SMARTTOOL_H_
#define SMARTTOOL_H_

//Standard Posix Integral Types
#include <stdint.h>

//Standard Integral Types for Smart Tools
typedef int8_t int8;
typedef unsigned char uint8; //The default uint8 type is char, but we want an explicitly unsigned char, because some compilers seem to think char is signed by default.
typedef int16_t int16;
typedef uint16_t uint16;
typedef int32_t int32;
typedef uint32_t uint32;
typedef int64_t int64;
typedef uint64_t uint64;

//A few more common ST type abbreviations
typedef unsigned int uint;
typedef unsigned long ulong;

//Boolean
typedef uint8 bool;
//Byte
typedef uint8 byte;

#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif

#define htonul(x) ((uint32)htonl((int32)x))
#define ntohul(x) ((uint32)ntohl((int32)x))

typedef struct struct_SmartTool
{
	char * szSmartToolType;
	int iOIDBase;
	int iOIDMax;
	char ** szaOIDNames;
} td_SmartTool;

#endif /* SMARTTOOL_H_ */
