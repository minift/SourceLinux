//STP.h
////////////////////////////////////////////////////////////////////////////////

#ifndef STP_H_
#define STP_H_

//Includes needed for common SmartTool Type
#include "SmartTool.h"

//External Defines
//  These must be set before using the library
//Optional External Defines
#ifdef STP_RX_REPEAT_READ
#ifndef STP_RX_MAX_REPEAT_READS
#fatal "STP_RX_REPEAT_READ requires that STP_RX_MAX_REPEAT_READS is defined"
#endif
#ifndef STP_RX_MAX_REPEAT_READ_MS
#fatal "STP_RX_REPEAT_READ requires that STP_RX_MAX_REPEAT_READ_MS is defined"
#endif
#endif

//Optional External Print Out Controlling Defines

//Show Basic STP Lib Output
#define STP_OUTPUT
//#define STP_OUTPUT_VERBOSE

//Show Incomming STP info
//#define STP_OUTPUT_PARSED
//Show the nulloid also, which is used for keep alive sometimes.
//#define STP_OUTPUT_PARSED_NULLOID
//You may also consider implementing something in the handler such as
//	MiniFt has implemented OUTPUT_RXSTP
//#define STP_OUTPUT_CSTATE

//Show Output STP info
#define STP_OUTPUT_TX

//Internal Defines
//  Some defines can be set by the main program to replace the default

#ifndef STP_LISTENPORT
#define STP_LISTENPORT 5555
#endif

#define STP_VERSION 0x0001

// STP message types
#define STP_GET 0x0001
#define STP_GET_RESP 0x0002
#define STP_SET 0x0003
#define STP_ALERT 0x0004
#define STP_BROADCAST 0x0005
#define STP_MESSAGE_TYPE_MAX 0x0006

// sizes of various components of an STP message
#define STP_HEADERSIZE  8
#define STP_EXTHEADERSIZE 4
#define STP_OBJVALUE_MAXSIZE (STP_EXTHEADERSIZE + 1024) // physical allocation space for STP message
#define STP_WHOLEMSG_MAXSIZE (STP_HEADERSIZE + STP_OBJVALUE_MAXSIZE)
#define STP_RX_BUFFER_SIZE 2560

#define STP_FINDLENGTH 0x7FFF  //When passed as length to build function, function will calculate the length based on a string interpretation of the buffer.
//No OID Defs : See Top Note
// states for STP Packet rx system
#define STP_RX_STATE_CLOSED 0
#define STP_RX_STATE_CLEAR 1
#define STP_RX_STATE_START 2

#define TCP_MAXNUM_FASTWRITE_ERRORS 10

// Macros
//just add this one they don't have in the series they call "intel"

#define writeHInt32ToNBuffer(p_c,l) *((int32_t *)p_c)=htonl( l )
#define writeHInt16ToNBuffer(p_c,x) *((int16_t *)p_c)=htons( x )
#define writeFloatToBuffer(p_c,f) *((float *)p_c)=f
#define writeDoubleToBuffer(p_c,d) *((double *)p_c)=d
//For use on non NTOHF machines when talking to legacy NTOHF machines
#define writeFloatToBufferNTOHFC(p_c,f)     *((float *)p_c) = htonfc( f )

#define WriteBString(x,l,s) \
	*x++=l;\
	memcpy(x,s,l);\
	x+=l;

#define ReadBString(x,l,s,m) \
	ilen=*x++;\
	if (ilen>m){ilen=m;}\
	memcpy(s,x,ilen);\
	x += ilen;\
	s[ilen]=0;\
	l=ilen;

#define WriteBArray(x,l,s) \
	WriteBString(x,l,s)

#define ReadBArray(x,l,s,m) \
	ReadBString(x,l,s,m)

typedef struct
{
	uint16 uiVersion;  // version # of protocol
	uint16 uiMsgType;  // command: GET, GETRESP, SET, ALERT
	uint16 uiOID;
	uint16 uiValueLength;  // size in octets
	char p_cObjectValue[STP_OBJVALUE_MAXSIZE];
} td_STPmsg;

//if a message is of size 0xFFFF, then the true size is stored
// in the td_STPmsgStream structed located at the start of the data payload.a
typedef struct
{
	uint32 ulValueLength;
} td_STPmsgStream;
//FIXME VHDJWRRECHECK PORT RECHECK
typedef struct
{
	int Sock;
	// this is the raw TCP buffer used for calls to sock_fastread...represents the entire STP message struct
	td_STPmsg STPrxMsg; //the memory for the structure is also used as a raw buffer for input before the data is aligned into the structure.
	char xxbuffernullspace; //follow the buffer with an extra space to allow a null to be writen after the buffer if needed.
	char xxbuffernullspace2;
	unsigned int iRxLen; //iRxLen is the number of bytes currently written into this memory buffer.
	//values used for streaming input
	//THESE ARE NOT USED ANYMORE
	//unsigned int iPos;  //position of stream reader in the handler
	//unsigned int iEnd;  //end of data available for stream reader
	char cState; // STATE_CLOSED=0 not open CLEAR=1 no header yet parsed  START=2 = has header
	//FIXME PORTMED  recheck the usage of the next 3
	unsigned int uiLastTxTimeMs;
	unsigned int uiLastRxTimeMs;
	char bDelayed;
	int iSessionNum;
	char * p_szConnectIP;
	int iConnectPort;
	//FIXME PORTMED  VHDJWRRECHECK PORT RECHECK
	//FIXME PORTMED  INCOMPLETE NEED TO TEST client connections
	char * p_szSmartToolType; //used for Smart Tool Identification when displaying session details and OIDs
	//Primary Smart Tool ID Name Set + Module1
	td_SmartTool * p_SmartTool;
	td_SmartTool * p_SmartToolModule1;
} td_STPsessions;

extern td_STPmsg g_STPtxMsg;  //global used for holding tx STP message
extern int g_iActiveSessions;

//Predeclarations of these functions, which are used in many places prior to calling STP library functions.
uint32 ntohl(uint32 x);
uint16 ntohs(uint16 x);
uint32 htonl(uint32 x);
uint16 htons(uint16 x);

//STP Functions
void STPInit(char * szSystemSmartToolType, td_SmartTool * pSystemSmartTool, td_SmartTool * pSystemSmartToolModule1);
int STPServerInit();
void STPInitAllSessions(char * szSystemSmartToolType, td_SmartTool * pSystemSmartTool, td_SmartTool * pSystemSmartToolModule1);
int STPClientSessionInit(td_STPsessions * p_STPSession, int iSessionNum, char * szConnectIP, char * szSmartToolType, td_SmartTool * pSmartTool, td_SmartTool * pSmartToolModule1);
int STPSessionInit(td_STPsessions * p_STPSession);
void STPSessionTick(void);
void ServiceSTP();
void AcceptConnections(void);
void CheckSTPSession(td_STPsessions * p_STPSession);
void STPSessionClose(td_STPsessions * p_STPSession);
void PreventSTPTimeout();
void STPShutdown();
td_STPsessions * GetSessionByNumber(unsigned int i);
void CheckPacketFlood(unsigned int uiOID);

void SmartToolMsg(td_STPsessions * p_STPSession, uint16 uiMsgType, uint16 uiOID, uint16 uiValueLength, char* p_cObjValue);

void SmartToolMsgInt16(td_STPsessions * p_STPSession, uint16 uiMsgType, uint16 uiOID, int16 iValue);
void SmartToolMsgUInt16(td_STPsessions * p_STPSession, uint16 uiMsgType, uint16 uiOID, uint16 iValue);

void SmartToolMsgInt32(td_STPsessions * p_STPSession, uint16 uiMsgType, uint16 uiOID, int32 iValue);
void SmartToolMsgUInt32(td_STPsessions * p_STPSession, uint16 uiMsgType, uint16 uiOID, uint32 uiValue);

void SmartToolMsgFloat(td_STPsessions * p_STPSession, uint16 uiMsgType, uint16 uiOID, float fvalue);
void SmartToolMsgDouble(td_STPsessions * p_STPSession, uint16 uiMsgType, uint16 uiOID, double dvalue);

void SmartToolMsgChar(td_STPsessions * p_STPSession, uint16 uiMsgType, uint16 uiOID, char cvalue);
void SmartToolMsgEmpty(td_STPsessions * p_STPSession, uint16 uiMsgType, uint16 uiOID);
void SmartToolMsgStr(td_STPsessions * p_STPSession, uint16 uiMsgType, uint16 uiOID, char* p_cObjValue);

void SmartToolMsgCommonMessageCode(td_STPsessions * p_STPSession, uint16 uiOID, uint16 uiMessageCode);
void RxSmartToolMsg(td_STPsessions * p_STPSession);

int ParseSmartToolMsgHeader(td_STPmsg* p_STPrxMsg);

void STPSend(int sock, char* p_cTxBuff, int iMsgLength);
void SendSTPmsg(td_STPsessions * p_STPSession, td_STPmsg* p_STPrxMsg, int iMsgLength);
void SendRaw(td_STPsessions * p_STPSession, char * buffer, int iMsgLength);

//void SendArrayOfStringsAsBSTRARRAY(char ** szarray, int i, int max);
//void SendXmemArrayOfStringRefObjectsAsBSTRARRAY(long xp, int i, int max);

char * DisplayMessageTypeName(uint16 ui);

char * DisplayOIDName(td_STPsessions * p_STPSession, uint16 ui);

////////////////////////////////////////////////////////////////////////////////
//	Callback Function Prototypes
////////////////////////////////////////////////////////////////////////////////
//	These functions MUST be defined by the application code that uses this library.

extern void SessionStarting(td_STPsessions * p_STPSession);
extern void SessionDelay(td_STPsessions * p_STPSession);
extern void SessionClosing(td_STPsessions * p_STPSession);
extern int HandleSmartToolMsg(td_STPmsg* p_STPrxMsg, td_STPsessions * p_STPSession);

#define MAXNUM_STP_SESSIONS 3

//not truely part of STP.h
//FIXME PORTMED  put this in right places later.
void SmartToolMsgMiniFtMessageCode(td_STPsessions * p_STPSession,unsigned int uiOID, unsigned int uiMessageCode);
void SendActivePremove(td_STPsessions * p_STPSession, unsigned int uiMsgType);

#endif //STP_H_
