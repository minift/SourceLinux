#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include "hwio.h"
#include "SmartTool.h"
#include "SmartToolUtil.h"
#include "CommonSmartTool.h"
#include "SocketConsole.h"
#include "STP.h"

//	PACKET_FLOOD_MONITOR is a debug tool to watch for flooding of Ethernet bandwidth.
//	#define PACKET_FLOOD_MONITOR 1

//Timeout Defines
//  The correct number must be in sync with the pendant
//  software being used.
#define STP_TXMSG_TIMEOUT_MS 1000
#define STP_RXMSG_DELAY_TIMEOUT_MS 1500
#define STP_RXMSG_CLOSE_TIMEOUT_MS 2500
//#define STP_STREAMING_OID_MODE
#define STP_RX_MAX_REPEAT_READ_MS 50
//FIXME HIGH PORT options
#define STP_STREAMING_OID_MODE
#define STP_RX_REPEAT_READ
#define STP_RX_MAX_REPEAT_READS 20
#define STP_RX_MAX_REPEAT_READ_MS 50
#define DO_NOT_USE_NTOHF
#define EXCLUDE_OID_POSNMODE_CURXY
#define EXCLUDE_OID_VISION_DATA

#define PACKET_FLOOD_MONITOR

//Number of client sessions in the main group
#define MAXNUM_STP_SESSIONS 3

////////////////////////////////////////////////////////////////////////////////
//	Global variables
////////////////////////////////////////////////////////////////////////////////
td_STPmsg g_STPtxMsg;  //global used for holding tx STP message
int g_iActiveSessions = 0;
//Private
int _STPServerSocket = 0;
td_STPsessions _STPsessions[MAXNUM_STP_SESSIONS];
uint16 g_hton_STP_VERSION;

#ifdef PACKET_FLOOD_MONITOR
//Network Flood Detection
uint32 g_uiLastSend = 0;
unsigned int g_uiFloodCount = 0;
#endif

//local Globals to support output
char * g_szSystemSmartToolType = 0;
//td_SmartTool * g_pSystemSmartTool = 0;
//td_SmartTool * g_pSystemSmartToolModule1 = 0;
td_SmartTool * g_pSystemSmartTool = 0;
td_SmartTool * g_pSystemSmartToolModule1 = 0;

char * g_szaMessageType[6] =
{
		"Unknown",
		"Get",
		"Get-Resp",
		"Set",
		"Alert",
		"BroadCast"
};

//FIXME PORTMED  STP Sends Can occur BEFORE the init is done, and packets bring with bad names.... this is minor, but should be better

////////////////////////////////////////////////////////////////////////////////
//STPInit
//Must be called prior to using any other library functions
void STPInit(char * szSystemSmartToolType, td_SmartTool * pSystemSmartTool, td_SmartTool * pSystemSmartToolModule1)
{
#ifdef STP_OUTPUT
	logf("Protocol STP_VERSION %d\r\n", STP_VERSION);
#endif
	g_hton_STP_VERSION = htons(STP_VERSION);

	g_szSystemSmartToolType = szSystemSmartToolType;

	//Start TCP Server Socket
	STPServerInit();

	//Clear all sessions
	STPInitAllSessions( szSystemSmartToolType, pSystemSmartTool, pSystemSmartToolModule1);

	return;
}

//Start Server Socket for STP connections
int STPServerInit()
{
	struct sockaddr_in stSockAddr;
	_STPServerSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (-1 == _STPServerSocket)
	{
		perror("can not create socket");
		exit(EXIT_FAILURE);
	}

	memset(&stSockAddr, 0, sizeof(stSockAddr));

	stSockAddr.sin_family = AF_INET;
	stSockAddr.sin_port = htons(STP_LISTENPORT);
	stSockAddr.sin_addr.s_addr = htonl(INADDR_ANY );
    logf( "STPServerInit: sock = %d; port = %d; IPAddr = 0x%8x (%s:%d).\r\n", _STPServerSocket, stSockAddr.sin_port,
                    stSockAddr.sin_addr.s_addr, INADDR_ANY, STP_LISTENPORT );

	if (-1 == bind(_STPServerSocket, (struct sockaddr *) &stSockAddr, sizeof(stSockAddr)))
	{
		perror("error bind failed");
		close(_STPServerSocket);
		exit(EXIT_FAILURE);
	}
	if (-1 == listen(_STPServerSocket, 2))
	{
		perror("error listen failed");
		close(_STPServerSocket);
		exit(EXIT_FAILURE);
	}
	int flags = fcntl(_STPServerSocket, F_GETFL, 0);
	if (flags < 0)
	{
		perror("error fcntl get failed");
		close(_STPServerSocket);
		exit(EXIT_FAILURE);
	}
	if (fcntl(_STPServerSocket, F_SETFL, flags | O_NONBLOCK) < 0)
	{
		perror("error fcntl set failed");
		close(_STPServerSocket);
		exit(EXIT_FAILURE);
	}
	return 1;
}
////////////////////////////////////////////////////////////////////////////////

//	Call this function ONLY at system start-up.
//	NOTE: In the new logic, this function is called by StpInit(); Applications should NOT call it.
void STPInitAllSessions(char * szSystemSmartToolType, td_SmartTool * pSystemSmartTool, td_SmartTool * pSystemSmartToolModule1)
{
	int i;
	//int iSessionCnt;

	//iSessionCnt = 0;  //	will keep track of # of open sessions
	for (i = 0; i < MAXNUM_STP_SESSIONS; i++)
	{
		td_STPsessions * p_STPSession = &_STPsessions[i];
		//	The following encapsulates tcp_listen() logic, and provides the returned value from tcp_listen().
		p_STPSession->iSessionNum = i; //for reference, store the session number into the session object
		p_STPSession->p_szConnectIP = (char*) 0; //the globals are all servers currently
		p_STPSession->p_szSmartToolType = szSystemSmartToolType;
		p_STPSession->p_SmartTool = pSystemSmartTool;
		p_STPSession->p_SmartToolModule1 = pSystemSmartToolModule1;
		STPSessionInit(&_STPsessions[i]);
	}

	return;
}

////////////////////////////////////////////////////////////////////////////////

int STPClientSessionInit(td_STPsessions * p_STPSession, int iSessionNum, char * szConnectIP, char * szSmartToolType, td_SmartTool * pSmartTool, td_SmartTool * pSmartToolModule1)
{
	p_STPSession->iSessionNum = iSessionNum;
	p_STPSession->p_szConnectIP = (char*) szConnectIP;
	p_STPSession->p_szSmartToolType = szSmartToolType;
	p_STPSession->p_SmartTool = pSmartTool;
	p_STPSession->p_SmartToolModule1 = pSmartToolModule1;
	//copy the system information into this session
	return STPSessionInit(p_STPSession);
}

////////////////////////////////////////////////////////////////////////////////

int STPSessionInit(td_STPsessions * p_STPSession)
{
	struct sockaddr_in stSockAddr;
	int sock;
	int iResult;

	//	Set all session struct data to initial values.
	p_STPSession->cState = STP_RX_STATE_CLOSED;

	p_STPSession->iRxLen = 0;

	//	do not initialize p_cRxBuff nor p_cTxBuff arrays within the STPsessions structure...
	//	these arrays will be populated once data is received or prepared to transmit
	p_STPSession->uiLastTxTimeMs = 0;
	p_STPSession->uiLastRxTimeMs = MS_TIMER;
	p_STPSession->bDelayed = 0;
	p_STPSession->Sock = 0; //no connection to this session yet.

	if (p_STPSession->p_szConnectIP == 0)
	{
		//This will hold client sockets that are accepted
		return 1;
	}
	//This is a client socket, which means that a connection must be opened now.

	while (1) //for break only
	{
		logf(
				"FATAL: The client side of STP has not been tested yet, so must be tested in more detail.\r\n");
		exit(1);
		int Res;
		sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

		if (-1 == sock)
		{
			perror("error: failed to allocate socket");
			sock = 0;
			break;
		}

		memset(&stSockAddr, 0, sizeof(stSockAddr));

		stSockAddr.sin_family = AF_INET;
		stSockAddr.sin_port = htons(p_STPSession->iConnectPort);
		Res = inet_pton(AF_INET, p_STPSession->p_szConnectIP, &stSockAddr.sin_addr);

		if (0 > Res)
		{
			perror("error: first parameter is not a valid address family");
			close(sock);
			sock = 0;
			break;
		}
		if (0 == Res)
		{
			perror("char string (second parameter does not contain valid ipaddress)");
			close(sock);
			sock = 0;
			break;
		}
		int flags = fcntl(_STPServerSocket, F_GETFL, 0);
		if (flags < 0)
		{
			perror("error fcntl get failed");
			close(sock);
			sock = 0;
			break;
		}
		if (fcntl(_STPServerSocket, F_SETFL, flags | O_NONBLOCK) < 0)
		{
			perror("error fcntl set failed");
			close(sock);
			sock = 0;
			break;
		}
		Res = connect(sock, (struct sockaddr *) &stSockAddr, sizeof(stSockAddr));
		if (Res < 0)
		{
			//FIXME VHDJWRRECHECK PORT RECHECK

//FIXME PORTFATAL   How to use Connect in non-blocking mode??? important to clean this up  wouldblcok
			perror("connect failed:");
			logf("Res=%d", Res);
			close(sock);
			sock = 0;
			break;
		}
		//success so far
		break;
	}
	if (sock > 0)
	{
		p_STPSession->Sock = sock;
		p_STPSession->cState = STP_RX_STATE_CLEAR;	//Success!
		p_STPSession->uiLastRxTimeMs = MS_TIMER;	//reset timeout
#ifdef STP_OUTPUT
		logf("STP: STPSessionInit connecting to %s:%d on s# %d.\r\n",
				p_STPSession->p_szConnectIP, p_STPSession->iConnectPort,
				p_STPSession->iSessionNum);
#endif
	}
	else
	{
#ifdef STP_OUTPUT
		logf("STP: STPSessionInit failed to connect to %s:%d on s# %d.\r\n",
				p_STPSession->p_szConnectIP, p_STPSession->iConnectPort,
				p_STPSession->iSessionNum);
#endif
	}
	iResult = 1;
	return (iResult);
}

////////////////////////////////////////////////////////////////////////////////

/* START FUNCTION DESCRIPTION ********************************************
 STPSessionTick  <STP.LIB>
 SYNTAX: StpSessionTick();
 KEYWORDS:
 DESCRIPTION:	Refresh connection status for STP sessions
 PARAMETERS:		None
 RETURN VALUE:	None
 PURPOSE:		Call periodically when appication is executing a blocking function
 to keep active STP sessions from timing out.
 NOTES:			Originally The Drill was using this as it's main service call, but the call has been
 modified to suit it's intended purpose of keeping connections alive and servicing the library, but
 not handling new stp messages.
 */
//FIXME VHDJWRRECHECK PORT RECHECK

//FIXME PORTHIGH  external setting
#define STP_SERVICE_THROTTLE_MS   20
void STPSessionTick(void)
{
	static int s_iSession = 0;
	td_STPsessions * p_STPSession;

	p_STPSession = &_STPsessions[s_iSession];
	if ((MS_TIMER - p_STPSession->uiLastTxTimeMs) > STP_SERVICE_THROTTLE_MS)
	{
		//	sequence to follow for servicing STP: 1)check connection, 2)check for msgs, 3)handle msgs
		//NO LONGER USED g_iSTPSessionNum = s_iSession;	//	For tracking current session through callbacks.
		CheckSTPSession(p_STPSession);	//	check connection status

		//	To keep all sessions alive, each successive pass must tend to the next session number.
		s_iSession++;	//	Advance to the next session.
		if (s_iSession >= MAXNUM_STP_SESSIONS)
		{
			s_iSession = 0;
		}
	}	//	Endif throttle timer has expired
}

////////////////////////////////////////////////////////////////////////////////

/* START FUNCTION DESCRIPTION ********************************************
 ServiceSTP  <STP.LIB>
 SYNTAX: ServiceSTP();
 KEYWORDS:
 DESCRIPTION:	Refresh connection status for STP sessions
 PARAMETERS:		None
 RETURN VALUE:	None
 PURPOSE:		For MiniFt: Service STP Lib: Maintain connections, operate timeouts,
 Read Incoming STP packets.
 NOTES:			Drill Systems have been using STPSessionTick()
 */
void ServiceSTP()
{
	int i;
	uint32 uiTime;
	td_STPsessions * p_STPSession;

	uiTime = MS_TIMER;
	AcceptConnections();
	uiTime = MS_TIMER - uiTime;
#ifdef STP_OUTPUT
	if (uiTime > 40)
	{
		logf("Accept=%u\r\n", uiTime);
	}
#endif

	g_iActiveSessions = 0; //reset count

	i = 0;
	while (i < MAXNUM_STP_SESSIONS)
	{
		p_STPSession = &_STPsessions[i];
		if (p_STPSession->Sock == 0)
		{
			//not an active connection
			i++;
			continue;
		}
		uiTime = MS_TIMER;

		CheckSTPSession(p_STPSession);
		uiTime = MS_TIMER - uiTime;
#ifdef STP_OUTPUT
		if (uiTime > 40)
		{
			logf("Check=%u\r\n", uiTime);
		}
#endif
		uiTime = MS_TIMER;

		RxSmartToolMsg(p_STPSession);

		uiTime = MS_TIMER - uiTime;
#ifdef STP_OUTPUT
		if (uiTime > 40)
		{
			logf("s#%d Eth=%u\r\n", i, uiTime);
		}
#endif
		if (p_STPSession->Sock != 0)
		{
			g_iActiveSessions++;
		}
		i++;
	}
	return;
}

////////////////////////////////////////////////////////////////////////////////


//FIXME MED PORT add headers in for this and org

void AcceptConnections(void)
{
	int i;

	//Find the lowest empty slot
	i = 0;
	td_STPsessions * p_STPSession = 0;
	while (i < MAXNUM_STP_SESSIONS)
	{
		p_STPSession = &_STPsessions[i];
		if (p_STPSession->Sock == 0)
		{
			//this one is the next available
			break;
		}
		i++;
	}
	if (i == MAXNUM_STP_SESSIONS)
	{
		return;
	}

	//Accept any new connections
	int sock = accept(_STPServerSocket, NULL, NULL );
	if (sock < 0)
	{
		if (errno == EAGAIN)
		{
			//Nothing trying to connect
			return;
		}
		logf("accept failed: errno=%d\r\n", errno);
		perror("error accept failed");
		return;
	}

    int flags = fcntl(sock, F_GETFL, 0);
    if (flags < 0)
    {
        perror("error fcntl get failed");
    	close(sock);
    	//still not connected
    	return;
    }
    if ( fcntl(sock, F_SETFL, flags | O_NONBLOCK) < 0)
    {
        perror("error fcntl set failed");
    	close(sock);
    	//still not connected
    	return;
    }

    //Use the session we found
	p_STPSession->Sock = sock;
	p_STPSession->cState = STP_RX_STATE_CLEAR;	//Success!
	p_STPSession->uiLastRxTimeMs = MS_TIMER;	//reset timeout
	logf("stp con new session assigned.\r\n");
#ifdef STP_OUTPUT
	logf("STP: s#%d open\r\n", p_STPSession->iSessionNum); //FIXME use true description with options for other outputs
#endif
//FIXME SEVERE when to init the session??? should I reclear here??? no on close????
	SessionStarting(p_STPSession); //callback
}

/* START FUNCTION DESCRIPTION ********************************************
 CheckSTPSession  <STP.LIB>
 SYNTAX: int CheckSTPSession(td_STPsessions * p_STPSession);
 KEYWORDS:
 DESCRIPTION:	Check and refresh connection status for the specified STP session
 PARAMETERS:		STP session
 RETURN VALUE:  0:OK, -1:connection problem
 MINIFT NOTES:   Use ServiceSTP generally (in transition)
 SMART DRILL NOTES: User application should call STPSessionTick() regularly within the application main loop.
 User applications should NOT call this function directly.
 SEE ALSO:
 END DESCRIPTION **********************************************************/

//FIXME   consider for many of these often called routines
void CheckSTPSession(td_STPsessions * p_STPSession)
{
	//FIXME this now only does the timeout check and prevention

	//    that part may not even be needed in the future, but for backwards compatibility
	//	should be there?????  OPTION

	//also don't have clear idea for close and relist
	//could do it on check, or when sending....  how do you know????

	if (p_STPSession->Sock == 0)
	{
		//when there is no socket, just continue...
		logf("Warning in CheckSTP sock was 0\r\n");
		return;
	}

	//	Check if last transmission was recent enough to avoid a timeout.
	//We must send enough so that the client doesn't have to ping us.
	//The system avoids pings, and in the future may not even operate with them.
	uint32 ui = MS_TIMER - p_STPSession->uiLastTxTimeMs;
	if (ui > STP_TXMSG_TIMEOUT_MS)
	{
		//#ifdef STP_OUTPUT
		//logf("STP: s#%d tx timeout %lu>%lu... sending\r\n",p_STPSession->iSessionNum,ul,(unsigned long)STP_TXMSG_TIMEOUT_MS);
		//#endif
		SmartToolMsg(p_STPSession, STP_ALERT, COMMON_OID_NULLOID, 0, g_STPtxMsg.p_cObjectValue);
	}
//FIXME DEPRECATE THE global session library with legacy enable feature

	//	Check if socket has not been heard from recently enough
	ui = MS_TIMER - p_STPSession->uiLastRxTimeMs;
	if (ui > STP_RXMSG_DELAY_TIMEOUT_MS)
	{
		if (p_STPSession->bDelayed == 0)
		{
			p_STPSession->bDelayed = 1;
#ifdef PING_AFTER_TIMEOUT
			//Send the ping when we are first delayed
			SmartToolMsg(p_STPSession, STP_GET, COMMON_OID_NULLOID, 0, g_STPtxMsg.p_cObjectValue);
#endif

			//And Call SessionDelay
#ifdef STP_OUTPUT
			logf("STP: s#%d delay\r\n", p_STPSession->iSessionNum);
#endif
			SessionDelay(p_STPSession); //callback

		}
		if (ui > STP_RXMSG_CLOSE_TIMEOUT_MS)
		{
			//	Connection did not respond... Connection is likely closed but not shown as closed yet somehow.
			//Close socket and Listen.
#ifdef STP_OUTPUT
			logf("STP: s#%d timeout...closing\r\n", p_STPSession->iSessionNum);
#endif
			g_STPtxMsg.p_cObjectValue[0] = CLOSE_TIMEOUT;
			SmartToolMsg(p_STPSession, STP_ALERT, COMMON_OID_STP_CLOSE, 1, g_STPtxMsg.p_cObjectValue);
			STPSessionClose(p_STPSession);
		}
	}
	else
	{
		p_STPSession->bDelayed = 0;
	}
}

void STPSessionClose(td_STPsessions * p_STPSession)
{
	if (p_STPSession->Sock != 0)
	{
		close(p_STPSession->Sock);
		p_STPSession->Sock = 0;
	}
	SessionClosing(p_STPSession);			//callback //run any code for the tool required when a connection drops
	STPSessionInit(p_STPSession); //clears the values back to closed
}

// START FUNCTION DESCRIPTION ********************************************
// Monitor and display sent STP messages when the sending frequency gets high
// END DESCRIPTION *******************************************************
void PreventSTPTimeout()
{
	int i;
	td_STPsessions * p_STPSession;
	i = 0;
	while (i < MAXNUM_STP_SESSIONS)	//	Min duration 650 usec
	{					//	Typical duration 750-800 usec. Worst case ~1.5msec.
		p_STPSession = &_STPsessions[i];
		p_STPSession->uiLastRxTimeMs = MS_TIMER;
		i++;
	}
	return;
}

void STPShutdown()
{
	int i;
	td_STPsessions * p_STPSession;

	logf("STPShutdown\r\n");
	i = 0;
	while (i < MAXNUM_STP_SESSIONS)
	{
		p_STPSession = &_STPsessions[i];
		if (p_STPSession->Sock == 0)
		{
			//not an active connection
			i++;
			continue;
		}
		close(p_STPSession->Sock);
		p_STPSession->Sock = 0;
		i++;
	}


	if (_STPServerSocket != 0)
	{
		close(_STPServerSocket);
		_STPServerSocket = 0;
	}
}

//GetSessionByNumber(int i)
td_STPsessions * GetSessionByNumber(unsigned int i)
{
	if (i<MAXNUM_STP_SESSIONS)
	{
		return &_STPsessions[i];
	}
	return 0;
}


////////////////////////////////////////////////////////////////////////////////
// START FUNCTION DESCRIPTION ********************************************
// Monitor and display sent STP messages when the sending frequency gets high
// END DESCRIPTION *******************************************************
void CheckPacketFlood(unsigned int uiOID)
{
#ifdef PACKET_FLOOD_MONITOR
	uint32 ui;

	ui = MS_TIMER - g_uiLastSend;
	g_uiLastSend = MS_TIMER;
	if (ui < 20)
	{
		g_uiFloodCount++;
	}
	else
	{
		g_uiFloodCount = 0;
	}

	if (g_uiFloodCount > 10)
	{
		if (uiOID != 285)					//for laser system there are many packets, but it's needed.
		{
			//Not Covered under the normal STP_OUTPUT
			logf("!!PNF:%u:%u\r\n", g_uiFloodCount, uiOID);
		}
	}
#endif
}

////////////////////////////////////////////////////////////////////////////////
/* START FUNCTION DESCRIPTION ********************************************
 SmartToolMsg  <STP.LIB>
 SYNTAX: void SmartToolMsg(td_STPsessions * p_STPSession, uint16 uiMsgType, uint16 uiOID, uint16 uiValueLength, char* p_cObjValue)

 KEYWORDS:
 DESCRIPTION:
 PARAMETERS:
 RETURN VALUE:  no value
 NOTES:
 SEE ALSO:
 END DESCRIPTION **********************************************************/
void SmartToolMsg(td_STPsessions * p_STPSession, uint16 uiMsgType, uint16 uiOID, uint16 uiValueLength, char* p_cObjValue)
{
	uint16 ui;

#ifdef STP_OUTPUT
	// Change this if needed to display output packets of interest. There is a heavy time cost to the print out though.
	// WARNING: This disply will greatly increase the time this function takes.
	//			This will slow down your main loop time, and
	//			decrease the sending rate of packets, which
	//			will change the behavior of the code being tested.

	if (uiOID != COMMON_OID_NULLOID)
	#ifdef EXCLUDE_OID_VISION_DATA
//FIXME PORTHIGH FUTURE display arrays
//	if( uiOID != MINIFT_OID_VISION_DATA )
#endif
#ifdef EXCLUDE_OID_POSNMODE_CURXY
//	if( uiOID != MINIFT_OID_POSNMODE_CURXY )
#endif
	{
		//	logf("STP msg: %d-%s %d-%s %d \r\n", uiMsgType, DisplayMessageTypeName(uiMsgType), uiOID, DisplayOIDName(uiOID), uiValueLength);
		logf("STP msg: %d-%s\r\n", uiOID, DisplayOIDName(p_STPSession, uiOID));
	}
#endif

#ifdef PACKET_FLOOD_MONITOR
	CheckPacketFlood(uiOID);
#endif

	if (uiValueLength == STP_FINDLENGTH)
	{
		uiValueLength = strlen(p_cObjValue); //does not include null, which is correct way to pass the string over STP for string type.
	}
	if (uiValueLength > STP_OBJVALUE_MAXSIZE)
	{
#ifdef STP_OUTPUT
		logf("ERROR:SmartToolMsg:uiValueLength overflow.\r\n");
#endif
		return;
	}

	g_STPtxMsg.uiVersion = g_hton_STP_VERSION;
	g_STPtxMsg.uiMsgType = htons(uiMsgType);
	g_STPtxMsg.uiOID = htons(uiOID);

	g_STPtxMsg.uiValueLength = htons(uiValueLength);

	//if there is data, copy the data into the message,
	// unless the data buffer is the same as what was sent down.
	if (uiValueLength > 0 && p_cObjValue != g_STPtxMsg.p_cObjectValue)
	{
		memcpy(g_STPtxMsg.p_cObjectValue, p_cObjValue, uiValueLength);
	}

	ui = uiValueLength + STP_HEADERSIZE;
	SendSTPmsg(p_STPSession, &g_STPtxMsg, ui);
}

////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

//
void SmartToolMsgInt16(td_STPsessions * p_STPSession, uint16 uiMsgType, uint16 uiOID, int16 iValue)
{
	uint16 ui;
#ifdef PACKET_FLOOD_MONITOR
	CheckPacketFlood(uiOID);
#endif

#ifdef STP_OUTPUT_TX
	logf("STP tx uint: %d-%s %d-%s %u\r\n", uiMsgType, DisplayMessageTypeName(uiMsgType), uiOID, DisplayOIDName(p_STPSession, uiOID), iValue);
#endif

	g_STPtxMsg.uiVersion = g_hton_STP_VERSION;
	g_STPtxMsg.uiMsgType = htons(uiMsgType);
	g_STPtxMsg.uiOID = htons(uiOID);

	g_STPtxMsg.uiValueLength = htons(2);

	writeHInt16ToNBuffer(g_STPtxMsg.p_cObjectValue, iValue);

	ui = (2 + STP_HEADERSIZE);
	SendSTPmsg(p_STPSession, &g_STPtxMsg, ui);
}

//
void SmartToolMsgUInt16(td_STPsessions * p_STPSession, uint16 uiMsgType, uint16 uiOID, uint16 uiValue)
{
	uint16 ui;
#ifdef PACKET_FLOOD_MONITOR
	CheckPacketFlood(uiOID);
#endif

#ifdef STP_OUTPUT_TX
	logf("STP tx uint: %d-%s %d-%s %u\r\n", uiMsgType, DisplayMessageTypeName(uiMsgType), uiOID, DisplayOIDName(p_STPSession, uiOID), uiValue);
#endif

	g_STPtxMsg.uiVersion = g_hton_STP_VERSION;
	g_STPtxMsg.uiMsgType = htons(uiMsgType);
	g_STPtxMsg.uiOID = htons(uiOID);

	g_STPtxMsg.uiValueLength = htons(2);

	writeHInt16ToNBuffer(g_STPtxMsg.p_cObjectValue, uiValue);

	ui = (2 + STP_HEADERSIZE);
	SendSTPmsg(p_STPSession, &g_STPtxMsg, ui);
}

//
void SmartToolMsgInt32(td_STPsessions * p_STPSession, uint16 uiMsgType, uint16 uiOID, int32 lValue)
{
	uint16 ui;
#ifdef PACKET_FLOOD_MONITOR
	CheckPacketFlood(uiOID);
#endif

#ifdef STP_OUTPUT_TX
	logf("STP tx ulong: %d-%s %d-%s %u\r\n", uiMsgType, DisplayMessageTypeName(uiMsgType), uiOID, DisplayOIDName(p_STPSession, uiOID), lValue);
#endif

	g_STPtxMsg.uiVersion = g_hton_STP_VERSION;
	g_STPtxMsg.uiMsgType = htons(uiMsgType);
	g_STPtxMsg.uiOID = htons(uiOID);

	g_STPtxMsg.uiValueLength = htons(4);

	writeHInt32ToNBuffer(g_STPtxMsg.p_cObjectValue, lValue);

	ui = (4 + STP_HEADERSIZE);
	SendSTPmsg(p_STPSession, &g_STPtxMsg, ui);
}

//
void SmartToolMsgUInt32(td_STPsessions * p_STPSession, uint16 uiMsgType, uint16 uiOID, uint32 lValue)
{
	uint16 ui;
#ifdef PACKET_FLOOD_MONITOR
	CheckPacketFlood(uiOID);
#endif

#ifdef STP_OUTPUT_TX
	logf("STP tx ulong: %d-%s %d-%s %u\r\n", uiMsgType, DisplayMessageTypeName(uiMsgType), uiOID, DisplayOIDName(p_STPSession, uiOID), lValue);
#endif

	g_STPtxMsg.uiVersion = g_hton_STP_VERSION;
	g_STPtxMsg.uiMsgType = htons(uiMsgType);
	g_STPtxMsg.uiOID = htons(uiOID);

	g_STPtxMsg.uiValueLength = htons(4);

	writeHInt32ToNBuffer(g_STPtxMsg.p_cObjectValue, lValue);

	ui = (4 + STP_HEADERSIZE);
	SendSTPmsg(p_STPSession, &g_STPtxMsg, ui);
}

//
void SmartToolMsgFloat(td_STPsessions * p_STPSession, uint16 uiMsgType, uint16 uiOID, float fvalue)
{
	uint16 ui;
#ifdef PACKET_FLOOD_MONITOR
	CheckPacketFlood(uiOID);
#endif

#ifdef STP_OUTPUT_TX
	logf("STP tx float: %d-%s %d-%s %.6f\r\n", uiMsgType, DisplayMessageTypeName(uiMsgType), uiOID, DisplayOIDName(p_STPSession, uiOID), fvalue);
#endif

	g_STPtxMsg.uiVersion = g_hton_STP_VERSION;
	g_STPtxMsg.uiMsgType = htons(uiMsgType);
	g_STPtxMsg.uiOID = htons(uiOID);

	g_STPtxMsg.uiValueLength = htons(4);

	writeFloatToBuffer(g_STPtxMsg.p_cObjectValue, fvalue);

	ui = (4 + STP_HEADERSIZE);
	SendSTPmsg(p_STPSession, &g_STPtxMsg, ui);
}

//
void SmartToolMsgDouble(td_STPsessions * p_STPSession, uint16 uiMsgType, uint16 uiOID, double dvalue)
{
	uint16 ui;
#ifdef PACKET_FLOOD_MONITOR
	CheckPacketFlood(uiOID);
#endif

#ifdef STP_OUTPUT_TX
	logf("STP tx float: %d-%s %d-%s %f\r\n", uiMsgType, DisplayMessageTypeName(uiMsgType), uiOID, DisplayOIDName(p_STPSession, uiOID), dvalue);
#endif

	g_STPtxMsg.uiVersion = g_hton_STP_VERSION;
	g_STPtxMsg.uiMsgType = htons(uiMsgType);
	g_STPtxMsg.uiOID = htons(uiOID);

	g_STPtxMsg.uiValueLength = htons(8);

	writeDoubleToBuffer(g_STPtxMsg.p_cObjectValue, dvalue);

	ui = (8 + STP_HEADERSIZE);
	SendSTPmsg(p_STPSession, &g_STPtxMsg, ui);
}

//
void SmartToolMsgChar(td_STPsessions * p_STPSession, uint16 uiMsgType, uint16 uiOID, char cvalue)
{
	uint16 ui;
#ifdef PACKET_FLOOD_MONITOR
	CheckPacketFlood(uiOID);
#endif

#ifdef STP_OUTPUT_TX
	logf("STP tx char: %d-%s %d-%s %u\r\n", uiMsgType, DisplayMessageTypeName(uiMsgType), uiOID, DisplayOIDName(p_STPSession, uiOID), (unsigned int) cvalue);
#endif

	g_STPtxMsg.uiVersion = g_hton_STP_VERSION;
	g_STPtxMsg.uiMsgType = htons(uiMsgType);
	g_STPtxMsg.uiOID = htons(uiOID);

	g_STPtxMsg.uiValueLength = htons(1);

	g_STPtxMsg.p_cObjectValue[0] = cvalue;

	ui = (1 + STP_HEADERSIZE);
	SendSTPmsg(p_STPSession, &g_STPtxMsg, ui);
}

/* START FUNCTION DESCRIPTION ********************************************
 SmartToolMsgFromUInt  <STP.LIB>
 SYNTAX: void SmartToolMsgEmpty(td_STPsessions * p_STPSession, unsigned int uiMsgType, unsigned int uiOID, char cvalue)
 KEYWORDS:
 DESCRIPTION:
 PARAMETERS:
 RETURN VALUE:
 NOTES:
 SEE ALSO:
 END DESCRIPTION **********************************************************/
void SmartToolMsgEmpty(td_STPsessions * p_STPSession, uint16 uiMsgType, uint16 uiOID)
{
	uint16 ui;
#ifdef PACKET_FLOOD_MONITOR
	CheckPacketFlood(uiOID);
#endif

#ifdef STP_OUTPUT_TX
	logf("STP tx empty: %d-%s %d-%s \r\n", uiMsgType, DisplayMessageTypeName(uiMsgType), uiOID, DisplayOIDName(p_STPSession, uiOID));
#endif

	g_STPtxMsg.uiVersion = g_hton_STP_VERSION;
	g_STPtxMsg.uiMsgType = htons(uiMsgType);
	g_STPtxMsg.uiOID = htons(uiOID);
	g_STPtxMsg.uiValueLength = 0;

	ui = STP_HEADERSIZE;
	SendSTPmsg(p_STPSession, &g_STPtxMsg, ui);
}

////////////////////////////////////////////////////////////////////////////////
/* START FUNCTION DESCRIPTION ********************************************
 SmartToolMsgStr  <STP.LIB>
 SYNTAX: int SmartToolMsgStr(unsigned int uiMsgType, unsigned int uiOID, char* p_cObjValue);
 KEYWORDS:
 DESCRIPTION:
 PARAMETERS:
 RETURN VALUE:  no value
 NOTES:
 SEE ALSO:
 END DESCRIPTION **********************************************************/
void SmartToolMsgStr(td_STPsessions * p_STPSession, uint16 uiMsgType,	uint16 uiOID, char* p_cObjValue)
{
	uint16 ui;
	uint16 uiValueLength;

#ifdef STP_OUTPUT
	// Change this if needed to display output packets of interest. There is a heavy time cost to the print out though.
	// WARNING: This disply will greatly increase the time this function takes.
	//			This will slow down your main loop time, and
	//			decrease the sending rate of packets, which
	//			will change the behavior of the code being tested.

	if (uiOID != COMMON_OID_NULLOID)
	{
		//	logf("STP msg: %d-%s %d-%s %d \r\n", uiMsgType, DisplayMessageTypeName(uiMsgType), uiOID, DisplayOIDName(uiOID), uiValueLength);
		logf("STP tx: %d-%s\r\n", uiOID, DisplayOIDName(p_STPSession, uiOID));
	}
#endif

#ifdef PACKET_FLOOD_MONITOR
	CheckPacketFlood(uiOID);
#endif

	uiValueLength = strlen(p_cObjValue); //does not include null, which is correct way to pass the string over STP for string type.
	if (uiValueLength > STP_OBJVALUE_MAXSIZE)
	{
#ifdef STP_OUTPUT
		logf("ERROR:SmartToolMsg:uiValueLength overflow.\r\n");
#endif
		return;
	}

	g_STPtxMsg.uiVersion = g_hton_STP_VERSION;
	g_STPtxMsg.uiMsgType = htons(uiMsgType);
	g_STPtxMsg.uiOID = htons(uiOID);

	g_STPtxMsg.uiValueLength = htons(uiValueLength);

	//if there is data, copy the data into the message,
	// unless the data buffer is the same as what was sent down.
	if (uiValueLength > 0 && p_cObjValue != g_STPtxMsg.p_cObjectValue)
	{
		memcpy(g_STPtxMsg.p_cObjectValue, p_cObjValue, uiValueLength);
	}

	ui = uiValueLength + STP_HEADERSIZE;
	SendSTPmsg(p_STPSession, &g_STPtxMsg, ui);
}

////////////////////////////////////////////////////////////////////////////////
/* START FUNCTION DESCRIPTION ********************************************
 SmartToolMsgCommonMessageCode  <STP.LIB>
 SYNTAX: void SmartToolMsgCommonMessageCode(unsigned int uiOID, unsigned int uiMessageCode)
 KEYWORDS:
 DESCRIPTION:
 PARAMETERS:
 RETURN VALUE:
 NOTES:
 SEE ALSO:
 END DESCRIPTION **********************************************************/
void SmartToolMsgCommonMessageCode(td_STPsessions * p_STPSession, uint16 uiOID, uint16 uiMessageCode)
{
	uint16 ui;
	td_oid_common_message_code * p_oid_common_message_code;

#ifdef STP_OUTPUT_TX
	logf("STP txCMC: src=%s mc=%d\r\n", DisplayOIDName(p_STPSession, uiOID), uiMessageCode);
#endif
#ifdef PACKET_FLOOD_MONITOR
	CheckPacketFlood(COMMON_OID_COMMON_MESSAGE_CODE);
#endif

	g_STPtxMsg.uiVersion = g_hton_STP_VERSION;
	g_STPtxMsg.uiMsgType = htons(STP_ALERT);
	g_STPtxMsg.uiOID = htons(COMMON_OID_COMMON_MESSAGE_CODE);
	g_STPtxMsg.uiValueLength = htons(sizeof(td_oid_common_message_code));

	p_oid_common_message_code = (td_oid_common_message_code *) &g_STPtxMsg.p_cObjectValue;
	p_oid_common_message_code->uiOID = htons(uiOID);
	p_oid_common_message_code->uiCode = htons(uiMessageCode);

	ui = (STP_HEADERSIZE + sizeof(td_oid_common_message_code));
	SendSTPmsg(p_STPSession, &g_STPtxMsg, ui);
}

////////////////////////////////////////////////////////////////////////////////
/* START FUNCTION DESCRIPTION ********************************************
 RxSmartToolMsg  <STP.LIB>
 SYNTAX: void RxSmartToolMsg(int iSessionNum)
 KEYWORDS:
 DESCRIPTION:
 PARAMETERS:
 RETURN VALUE:
 NOTES:
 SEE ALSO:
 END DESCRIPTION **********************************************************/

//#define STP_RX_REPEAT_READ
//#define STP_RX_MAX_REPEAT_READS 20
void RxSmartToolMsg(td_STPsessions * p_STPSession)
{
	unsigned long ulStartReads;
	int inb;
	int irxgoal;
	int irx;
	int iresult;
	//int ipos;
	//int idata;
	//int iend;
	char cstate;
	int sock;
	td_STPmsg * p_STPrxMsg;
	char * p_cRxBuff;
	//td_STPmsgStream * p_STPmsgStream;
	unsigned long ul;

	char cPacketsHandledSinceRead = 0;

	sock = (p_STPSession->Sock); //local reference to socket
	if (sock == 0)
	{
		return;
	}

	p_STPrxMsg = &p_STPSession->STPrxMsg;
	p_cRxBuff = (char *) p_STPrxMsg; //to treat this memory like a buffer for reading the socket

	inb = p_STPSession->iRxLen; //how many bytes are waiting in buffer
	cstate = p_STPSession->cState;
	if (cstate == STP_RX_STATE_CLOSED)
	{
		//Shouldn't get here because socket is still assigned, yet the state is closed
		logf("state was closed\r\n");
		return;
	}
	ulStartReads = MS_TIMER;
	read_again_now:

	//NEW PATTERN: Try to ready max buffer size in one read
	//parse through....
	//when parsed through as far as possible, if 0 remain exit.
	//when a partial packet remains, repeat reading up to X times.

	irxgoal = STP_WHOLEMSG_MAXSIZE - inb;
	if (irxgoal == 0)
	{
		logf("noroom\r\n");
	}
	if (irxgoal > 0)
	{
		errno = 12345;
		irx = read(sock, p_cRxBuff + inb, irxgoal);
		if (irx < 0)
		{
			if (errno == EAGAIN)
			{
				//would block
				//since there is no new data, and the code pattern has handled all the data
				//that could be handled, it is time to exit
				goto save_and_exit;
			}
			//another error?
#ifdef STP_OUTPUT
			logf("STP data read error (s#%d) errno=%d\r\n", p_STPSession->iSessionNum, errno); //debug code
#endif
			//Close socket
			STPSessionClose(p_STPSession);
			goto exit_directly;
		}
		if (irx == 0) //means it's closed....
		{
			if (errno == EAGAIN)
			{
				logf("0 EAGAIN");
			}
			else
			{
				logf("0 not EAGAIN");
			}
			//Close socket
			STPSessionClose(p_STPSession);
			goto exit_directly;
		}
#ifdef STP_OUTPUT_VERBOSE
		logf("STP data read (s#%d, %d bytes)\r\n", p_STPSession->iSessionNum,irx);  //debug code
#endif
		inb += irx;
		p_STPSession->uiLastRxTimeMs = MS_TIMER; //update the last rx time
		p_STPSession->bDelayed = 0;
		cPacketsHandledSinceRead = 0;
	}
	//buffer is now as full as possible

	while (inb >= STP_HEADERSIZE)
	{
		if (cstate == STP_RX_STATE_CLEAR)
		{
			//just found it
			iresult = ParseSmartToolMsgHeader(p_STPrxMsg);
			if (iresult == 0)
			{
				logf("FAILURE:RxSmartToolMsg OUT OF SYNC: inb=%d\r\n", inb);
				//memchardump("stream:",p_cRxBuff,inb);
				//Close socket
				STPSessionClose(p_STPSession);
				goto exit_directly;
			}
			if (p_STPrxMsg->uiValueLength > STP_OBJVALUE_MAXSIZE)
			{
				logf("packet value length > %d bytes.\r\n", STP_OBJVALUE_MAXSIZE);
				//Close socket
				STPSessionClose(p_STPSession);
				goto exit_directly;
			}
			//found valid header
			cstate = STP_RX_STATE_START;

			//reset the buffer used variable
			//ipos = STP_HEADERSIZE;
			//iend = STP_HEADERSIZE; //used to pass usable length to handler
			//continue for processing
		}
		//parsed this header now
		int imessagesize = STP_HEADERSIZE + p_STPrxMsg->uiValueLength;
		//logf("%d %d \r\n", inb, imessagesize);
		if (inb >= imessagesize)
		{
			//have enough to process this message
			//Now handle the STP packet
			HandleSmartToolMsg(p_STPrxMsg, p_STPSession);
			cPacketsHandledSinceRead = 1;

			cstate = STP_RX_STATE_CLEAR; //clear signal that header has been processed

			//free buffer space
			inb -= imessagesize;
			if (inb > 0)
			{
				//Copy the data to the start of the buffer.
				memcpy(p_cRxBuff, p_cRxBuff + imessagesize, inb);
			}
			//Check if there is another headers worth of data left
			continue;
		}
		//not enough for this message
		break;
	}
	if (inb > 0)
	{
		//not an entire header, or entire message left
		if (cPacketsHandledSinceRead == 1)
		{
			//Since some packets have been handled since we started the loop
			//there is a good change that there is more room and more data has arrived
			//for this incomplete message.  Most Messages are sent all at once.
			//Only read again if we are not taking too long
			ul = MS_TIMER - ulStartReads;
			if (ul < STP_RX_MAX_REPEAT_READ_MS)
			{
				//not too long. time to read again
				goto read_again_now;
			}
#ifdef STP_OUTPUT
			logf("MR:ms\r\n");
#endif
			goto save_and_exit;
		}
		//no packets were handled since the last read...
		//it's not likely that new data is now available
		goto save_and_exit;
		//must return later to try to get more
	}
	save_and_exit:

	p_STPSession->iRxLen = inb; //save the number of bytes in the buffer;
	p_STPSession->cState = cstate;
	exit_directly:
	return;
}

////////////////////////////////////////////////////////////////////////////////
/* START FUNCTION DESCRIPTION ********************************************
 ParseSmartToolMsgHeader  <STP.LIB>
 SYNTAX: int ParseSmartToolMsgHeader(td_STPmsg* p_STPrxMsg)
 KEYWORDS:
 DESCRIPTION:
 PARAMETERS:
 RETURN VALUE:
 NOTES:
 SEE ALSO:
 END DESCRIPTION **********************************************************/

int ParseSmartToolMsgHeader(td_STPmsg* p_STPrxMsg)
{
	int iresult;

	// Rabbit internal representation is little endian (LSB first)...network is big endian (MSB first)
	uint16 uiVersion;
	uint16 uiMsgType;
	uint16 uiOID;
	uint16 uiValueLength;

	uiVersion = ntohs(p_STPrxMsg->uiVersion);
	uiMsgType = ntohs(p_STPrxMsg->uiMsgType);
	uiOID = ntohs(p_STPrxMsg->uiOID);
	uiValueLength = ntohs(p_STPrxMsg->uiValueLength);

	iresult = 1;

	if (uiVersion >= 0x0010 || uiVersion == 0)
	{
		//if version as a little off, then we could parse it, but if the number is this far off, then it's likely not a version number at all
		iresult = 0; //a fatal error for stream sync
	}

	if (uiMsgType >= 0x0020 || uiMsgType == 0)
	{
		//if message type was a little off, then we could try to parse it, but if the number is this far off, then it's likely not a type at all
		iresult = 0; //a fatal error for stream sync
	}

	if (iresult > 0)
	{
#ifdef STP_OUTPUT_PARSED
#ifndef STP_OUTPUT_PARSED_NULLOID
		if (p_STPrxMsg->uiOID==OID_NULLOID)
		{
#endif
			logf("STP Parsed msg : v%d %d-%s %d-%s %d \r\n", uiVersion, uiMsgType, DisplayMessageTypeName(uiMsgType), uiOID, DisplayOIDName(uiOID), uiValueLength);
#ifndef STP_OUTPUT_PARSED_NULLOID
		}
#endif
#endif
		//save host order data back into memory
		p_STPrxMsg->uiVersion = uiVersion;
		p_STPrxMsg->uiMsgType = uiMsgType;
		p_STPrxMsg->uiOID = uiOID;
		p_STPrxMsg->uiValueLength = uiValueLength;
	}
	else
	{
#ifdef STP_OUTPUT
		logf("STP FAILED parse header (ver,msg,OID,len)=%d,%d,%d,%d\r\n", uiVersion, uiMsgType, uiOID, uiValueLength);
#endif
	}
	return iresult;
}

////////////////////////////////////////////////////////////////////////////////
/* START FUNCTION DESCRIPTION ********************************************
 STPSend  <STP.LIB>
 SYNTAX:			See prototype above.
 KEYWORDS:
 DESCRIPTION:	PRIVATE function - for internal use only. NOT to be called by application code.
 Low level function to tx a string to a specified socket.
 PARAMETERS:
 RETURN VALUE:
 NOTES:
 SEE ALSO:
 END DESCRIPTION **********************************************************/

void STPSend(int sock, char* p_cTxBuff, int iMsgLength)
{
	int inb;
	int itx;
	int irx;
	char cNumFastwriteErrs;

#ifdef STP_OUTPUT
	//    logf("Send g_pSocket=%lp p_Socket=%lp\r\n",g_pSocket,p_Socket);
#endif

	inb = 0;  // inb will keep track of # of bytes sent out
	cNumFastwriteErrs = 0; // track how many attempts so we can bail out if needed
	while (inb < iMsgLength)
	{
		itx = iMsgLength - inb; //just try to send the whole thing
		irx = write(sock, p_cTxBuff + inb, itx);
		if (irx < 0)
		{ // error handling
#ifdef STP_OUTPUT
			logf("STPSend:error\r\n");
#endif
			irx = 0; //assume that in the error case... 0 bytes are sent.
			cNumFastwriteErrs++;
			if (cNumFastwriteErrs >= TCP_MAXNUM_FASTWRITE_ERRORS)
			{
				break;
			}
			continue;
		}
		inb += irx;
		if (irx < itx) //Update 20091002 I don't like doing this tick that often, but seems to be needed.
		{
			cNumFastwriteErrs++;
			if (cNumFastwriteErrs >= TCP_MAXNUM_FASTWRITE_ERRORS)
			{
				break;
			}
			continue; //try again now
		}
	}
	if (inb < iMsgLength)
	{
		//message not fully sent
#ifdef STP_OUTPUT
		logf("SendSTPmsg:Failed to send complete message\r\n");
#endif
		//Close this socket rather than complete the message and be out of sync
		close(sock);
//FIXME PORT FATAL  must ensure that after this, the sock will be reopenned killed sock....
	}
	return;
}

////////////////////////////////////////////////////////////////////////////////
/* START FUNCTION DESCRIPTION ********************************************
 SendSTPmsg  <STP.LIB>
 SYNTAX:			See prototype above.
 KEYWORDS:
 DESCRIPTION:	PRIVATE function - for internal use only. NOT to be called by application code.
 Send a message to one or more STP sessions.
 To determine which sessions are recipients, see logic and comments below.
 PARAMETERS:
 RETURN VALUE:
 NOTES:
 SEE ALSO:
 END DESCRIPTION **********************************************************/

void SendSTPmsg(td_STPsessions * p_STPSession, td_STPmsg* p_STPrxMsg, int iMsgLength)
{
	//	Send a msg out to one or all STP sessions
	//	If a specific session is specified, the message is sent only to that session.
	//	If no specific session is flagged, then the msg is sent to all established STP sessions.

	int i;

	//	If we are working with a specific STP session number, ...

	if (p_STPSession != 0)
	{
		p_STPSession->uiLastTxTimeMs = MS_TIMER;
		STPSend(p_STPSession->Sock, (char *) p_STPrxMsg, iMsgLength);
	}
	else
	{
		i = 0; //have to go through all sockets
		while (i < MAXNUM_STP_SESSIONS)
		{
			_STPsessions[i].uiLastTxTimeMs = MS_TIMER;
			STPSend(_STPsessions[i].Sock, (char *) p_STPrxMsg, iMsgLength);
			i++;
		}
	}
	return;
}

////////////////////////////////////////////////////////////////////////////////
/* START FUNCTION DESCRIPTION ********************************************
 SendRaw  <STP.LIB>
 SYNTAX: void SendRaw(char * buffer, int iMsgLength)
 KEYWORDS:
 DESCRIPTION:	used only by STP.LIB::SendArrayOfStringsAsBSTRARRAY and
 STP.LIB::SendXmemArrayOfStringRefObjectsAsBSTRARRAY
 PARAMETERS:
 RETURN VALUE:
 NOTES:
 SEE ALSO:
 END DESCRIPTION **********************************************************/

void SendRaw(td_STPsessions * p_STPSession, char * buffer, int iMsgLength)
{
	//use this function with only a cast... this could be a macro, but that is less clear
	SendSTPmsg(p_STPSession, (td_STPmsg*) buffer, iMsgLength);
	return;
}

////////////////////////////////////////////////////////////////////////////////
/* START FUNCTION DESCRIPTION ********************************************
 SendArrayOfStringsAsBSTRARRAY  <STP.LIB>
 SYNTAX: void SendArrayOfStringsAsBSTRARRAY(char ** szarray,int i,int max)
 KEYWORDS:
 DESCRIPTION:
 PARAMETERS:
 RETURN VALUE:
 NOTES:
 SEE ALSO:
 END DESCRIPTION **********************************************************/

void SendArrayOfStringsAsBSTRARRAY(td_STPsessions * p_STPSession,
		char ** szarray, int i, int max)
{
	char * p_c;
	char * s;
	int len;
	p_c = g_STPtxMsg.p_cObjectValue;
	while (i < max)
	{
		s = szarray[i];
		len = strlen(s);
		*p_c = (char) len; //store char length as 8 bit length
		strcpy(p_c + 1, s);
		len++;
		SendRaw(p_STPSession, p_c, len);
		i++;
	}
	*p_c = 1;
	p_c[1] = 0; //1 byte string with null as terminator
	SendRaw(p_STPSession, p_c, 2);
	return;
}

////////////////////////////////////////////////////////////////////////////////
/* START FUNCTION DESCRIPTION ********************************************
 SendXmemArrayOfStringRefObjectsAsBSTRARRAY  <STP.LIB>
 SYNTAX: void SendXmemArrayOfStringRefObjectsAsBSTRARRAY(long xp,int i,int max)
 KEYWORDS:
 DESCRIPTION:
 PARAMETERS:
 RETURN VALUE:
 NOTES:
 SEE ALSO:
 END DESCRIPTION **********************************************************/
#if 0
void SendXmemArrayOfStringRefObjectsAsBSTRARRAY(long xp, int i, int max)
{
	char * p_c;
	char * s;
	int sr;
	int len;
	p_c = g_STPtxMsg.p_cObjectValue;
	while (i < max)
	{
		sr = xgetint(xp + 2 * i);
		getXmemString(sr, p_c + 1, 255);
		len = strlen(p_c + 1);
		*p_c = (char) len;
		len++;
		SendRaw(p_c, len);
		i++;
	}
	*p_c = 1;
	p_c[1] = 0; //1 byte string with null as terminator
	SendRaw(p_c, 2);
}
#endif
//STP Output Functions

//OID message type and display
char * DisplayMessageTypeName(uint16 ui)
{
	if (ui >= STP_MESSAGE_TYPE_MAX)
	{
		ui = 0;
	} //0 prints Unknown
	return g_szaMessageType[ui];
}

char * DisplayOIDName(td_STPsessions * p_STPSession, uint16 ui)
{
	char ** p_szaOIDNameArray;
	if (ui < 100)
	{
		if (ui > COMMON_OID_MAX_NUMBER)
		{
			return "Unknown COMMON OID";
		}
		return pszOIDNamesCommon[ui];
	}
	if (p_STPSession == 0)
	{
		//When session is passed in as zero, use the main default Smart Tool OID set on this machine
		p_STPSession = &_STPsessions[0];
	}
	td_SmartTool * st = p_STPSession->p_SmartTool;
	if (st!=0)
	{
		//logf("got tool\r\n");
		//logf("range %d %d\r\n",st->iOIDBase , st->iOIDMax);
		if (ui >= st->iOIDBase && ui <= st->iOIDMax)
		{
			//logf("in range\r\n");
			p_szaOIDNameArray=st->szaOIDNames;
			if (p_szaOIDNameArray!=0)
			{
				//logf("got array\r\n");
				return p_szaOIDNameArray[ui - st->iOIDBase];
			}
		}
	}
	st = p_STPSession->p_SmartToolModule1;
	if (st!=0)
	{
		//logf("got tool module\r\n");
		//logf("range %d %d\r\n",st->iOIDBase , st->iOIDMax);
		if (ui >= st->iOIDBase && ui <= st->iOIDMax)
		{
			//logf("in range\r\n");
			p_szaOIDNameArray=st->szaOIDNames;
			if (p_szaOIDNameArray!=0)
			{
				//logf("got array\r\n");
				return p_szaOIDNameArray[ui - st->iOIDBase];
			}
		}
	}
	return "Unknown OID";
}

void SendGenericMessage(char * messagebuffer)
{
	SmartToolMsgStr(0, STP_ALERT, COMMON_OID_GENERICMESSAGE, messagebuffer);
}


