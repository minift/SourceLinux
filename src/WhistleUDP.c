//WhistleUDP
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
#include <stdarg.h>
#include <errno.h>
#include "SmartTool.h"
#include "SocketConsole.h"
#include "hwio.h"
#include "SmartToolUtil.h"
#include "WhistleUDP.h"

//FIXME PORTLOW
//This is defined in STP.c, but I don't really want to include STP headers here....
void SendGenericMessage(char * messagebuffer);

//FIXME VHDJWRRECHECK PORT RECHECK
#define USE_OUTPUT

//FIXME PORTLOW   how to set this option VS what I must do...
#define WHISTLE_HEARTBEAT_TIMEOUT 4000

//globals
char g_cRxBuff[WHISTLE_BUFFER_SIZE + 2];

//All Whistle Axis Libs can share these globals
char g_cTxBuffer[WHISTLE_TX_BUFFER_SIZE];
char g_cTxBuffer2[WHISTLE_TX_BUFFER_SIZE];

void DUMPARRAY(char * label, char * buffer, int count);

void initWhistleUDP(char * name, char cAxis, char * localIP, int localport, char * remoteIP, int remoteport,
					char * comMsgCmd, char * comMsgClearCmd, td_WhistleUDP * p_WhistleUDP)
{
	p_WhistleUDP->name = name;
	p_WhistleUDP->cAxis = cAxis;
	p_WhistleUDP->localIP = localIP;
	p_WhistleUDP->localport = localport;
	p_WhistleUDP->remoteIP = remoteIP;
	p_WhistleUDP->remoteport = remoteport;
	p_WhistleUDP->iOpen = 0;
	p_WhistleUDP->cCommFlag = 2;
	p_WhistleUDP->uiComMsgRequested = 0;
	p_WhistleUDP->comMsgCmd = comMsgCmd;
	p_WhistleUDP->comMsgCmdLen = strlen(comMsgCmd);
	p_WhistleUDP->comMsgClearCmd = comMsgClearCmd;
	p_WhistleUDP->comMsgClearCmdLen = strlen(comMsgClearCmd);
	p_WhistleUDP->cCommTimeout = 0;
	p_WhistleUDP->lasti = 0;
	WhistleUDPMonReset(&p_WhistleUDP->WhistleUDPMonitor);
	p_WhistleUDP->cCommEnable = 1;
	p_WhistleUDP->cCommDisplay = 0;
	p_WhistleUDP->cCommDisplayRequired = COMM_DISPLAY_TX;
	//Heartbeat
	p_WhistleUDP->HBsentTime = 0;
	p_WhistleUDP->HBlastTime = 0;
	p_WhistleUDP->cHBsent = FALSE;
	p_WhistleUDP->cHBTimeout = 0;
//edit
//void WhistleUDPMonUpdate(td_WhistleUDP * p_WhistleUDP);

	int sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sock == -1) /* if socket failed to initialize, exit */
	{
		logf("Error Creating Socket");
		sock = 0;
	}
	p_WhistleUDP->sock = sock;

	if (sock > 0)
	{
		memset(&p_WhistleUDP->localsockaddr, 0, sizeof(struct sockaddr_in));
		//The address is ipv4
		p_WhistleUDP->localsockaddr.sin_family = AF_INET;

		//ip_v4 adresses is a uint32_t, convert a string representation of the octets to the appropriate value
		if (p_WhistleUDP->localIP != 0)
		{
			p_WhistleUDP->localsockaddr.sin_addr.s_addr = inet_addr(p_WhistleUDP->localIP);
		}
		else
		{
			p_WhistleUDP->localsockaddr.sin_addr.s_addr = htonl(INADDR_ANY );
		}

		//sockets are unsigned shorts, htons(x) ensures x is in network byte order, set the port to 7654
		p_WhistleUDP->localsockaddr.sin_port = htons(p_WhistleUDP->localport);

		if (bind(sock, (struct sockaddr *) &p_WhistleUDP->localsockaddr, sizeof(p_WhistleUDP->localsockaddr)) < 0)
		{
			perror("bind failed");
			p_WhistleUDP->sock = 0;
		}

		memset(&p_WhistleUDP->remotesockaddr, 0, sizeof(struct sockaddr_in));
		//The address is ipv4
		p_WhistleUDP->remotesockaddr.sin_family = AF_INET;

		//ip_v4 adresses is a uint32_t, convert a string representation of the octets to the appropriate value
		p_WhistleUDP->remotesockaddr.sin_addr.s_addr = inet_addr(p_WhistleUDP->remoteIP);
		p_WhistleUDP->remotesockaddr.sin_port = htons(p_WhistleUDP->remoteport);

		//Turn on non-blocking
		int flags = fcntl(sock, F_GETFL, 0);
		if (flags < 0)
		{
			perror("error fcntl get failed");
			close(sock);
			exit(EXIT_FAILURE);
		}
		if (fcntl(sock, F_SETFL, flags | O_NONBLOCK) < 0)
		{
			perror("error fcntl set failed");
			close(sock);
			exit(EXIT_FAILURE);
		}
	}
	return;
}

////////////////////////////////////////////////////////////////////////////////
// void ServiceWhistleUDP(td_WhistleUDP * p_WhistleUDP)
// Handle Com Signal
// Read input
// Parse input
// Pass input to further parsing handler(s)
//
//
////////////////////////////////////////////////////////////////////////////////

int last_ack;

int plack(char *s)
{
	int i;
	while (*s != 0)
	{
		if (*s != 'a')
		{
			s++;
			continue;
		}
		s++;
		if (*s != 'c')
		{
			continue;
		}
		s++;
		if (*s != 'k')
		{
			continue;
		}
		s++;
		i = atoi(s);
		return i;
	}
	return 0;
}

void ServiceWhistleUDP(td_WhistleUDP * p_WhistleUDP, int di_com_flag, void (*fxParseWhistleUDPMsg)())
{
	int ibrx; //bytes to read
	int i;
	int rxcnt;
	char c;
	char cWhistleFlag;
	char * s;
	char fmtbuf[128];
	int ilen;
	char cFailExp = '?';
	byte b;


	uint32 uistart;
//edit MAKE WITH FLAG OPTION LATER

	//Whistle Message Check Interval

	//FIXME PORTLOW  new whistle patterns should be considered but aren't needed for 1st port release

	//FIXME PORTLOW  create the more permanent system, but for now depend on comflags as soon as possible

	if ((MS_TIMER - p_WhistleUDP->uiComMsgRequested) > 1000)
	{
		//Show it if showing system bits
		p_WhistleUDP->cCommDisplayRequired = COMM_DISPLAY_TX_SYSTEM;
		goto whistleudp_query_message;
	}

	//Whistle Input Flag
	if (di_com_flag >= 0)
	{
		cWhistleFlag=digIn( di_com_flag );
		if (cWhistleFlag==1)
		{
			if (p_WhistleUDP->cCommFlag!=1)
			{
				p_WhistleUDP->cCommDisplayRequired = COMM_DISPLAY_TX_SYSTEM;
				whistleudp_query_message:
				//Get and Clear the Message, and clear the signal line.
				//Only display this tx if system tx is turned on for display...

				//Request when it goes high, or if enough time passes and it's still high.
				sendWhistleUDPMsgWL(p_WhistleUDP, p_WhistleUDP->comMsgCmd, p_WhistleUDP->comMsgCmdLen);
				sendWhistleUDPMsgWL(p_WhistleUDP, p_WhistleUDP->comMsgClearCmd, p_WhistleUDP->comMsgClearCmdLen);
				//Set Signal Low If using com flags
				if (di_com_flag >= 0)
				{
					sendWhistleUDPMsgWL(p_WhistleUDP, "ob[1]=0;\r", 9);
				}
				//restore cCommDisplayRequired
				p_WhistleUDP->cCommDisplayRequired = COMM_DISPLAY_TX;
				//update com msg reqest time
				p_WhistleUDP->uiComMsgRequested=MS_TIMER;
			}
		}
		if (p_WhistleUDP->cCommFlag!=cWhistleFlag)
		{
			//logf("WF=%d ch=%d\r\n", cWhistleFlag, di_com_flag);
			p_WhistleUDP->cCommFlag=cWhistleFlag;
		}
	}

	//Whistle Communication
	uistart = MS_TIMER;
	//how will command erros show?
	//>>>>>>>>>>>>>>>>>>>>>>>>
	while (1)
	{
		//FIXME PORTHIGH1
		//need a pattern than will bring back check and open them if they are gone instead there.....?????
		if (p_WhistleUDP->sock > 0)
		{
			//logf("read from %s \r\n", p_WhistleUDP->name);

			//rxcnt = recv(p_WhistleUDP->sock, g_cRxBuff, WHISTLE_BUFFER_SIZE, 0);

			struct sockaddr_in remotesockaddr;
			socklen_t fromlen;

			memset(&remotesockaddr, 0, sizeof(remotesockaddr));
			fromlen = sizeof(remotesockaddr);

			rxcnt = recvfrom(p_WhistleUDP->sock, g_cRxBuff, WHISTLE_BUFFER_SIZE, 0, (struct sockaddr *) &remotesockaddr, &fromlen);
			if (rxcnt < 0)
			{
				if (errno == EAGAIN)
				{
					//would block
					break;
				}
				logf("udp_recv err %d\r\n", errno);
				close(p_WhistleUDP->sock);
				p_WhistleUDP->sock = 0;
				break;
			}
			if (rxcnt == 0)
			{
				logf("gn\r\n");
				logf("udp_recv err %d\r\n", errno);
				break;
			}
			//logf("fromlen=%d\r\n", fromlen);
			//unsigned char *a = (char *) &remotesockaddr.sin_addr.s_addr;
			//logf("%d.%d.%d.%d : %u\r\n", a[0], a[1], a[2], a[3], (ntohs(remotesockaddr.sin_port)));

			if (rxcnt >= WHISTLE_BUFFER_SIZE)
			{
				logf("udp_recv PACKET  TRUNCATED\r\n");
			}
			p_WhistleUDP->WhistleUDPMonitor.bytesrecd += rxcnt;

			g_cRxBuff[rxcnt] = 0;

			//Default
			p_WhistleUDP->cCommDisplayRequired = COMM_DISPLAY_RX;

			//Turn up the display requirement to system bits if this is a system command
			if (g_cRxBuff[0]=='p' && g_cRxBuff[1]=='x')
			{
				// px* queries are echos of system calls and happen often so only show if watching
				p_WhistleUDP->cCommDisplayRequired = COMM_DISPLAY_RX_PX;
			}
			else if (g_cRxBuff[0]=='i')
			{
				if (g_cRxBuff[1]==';')
				{
					p_WhistleUDP->cCommDisplayRequired = COMM_DISPLAY_RX_SYSTEM;
				}
				else if (g_cRxBuff[1]=='=')
				{
					p_WhistleUDP->cCommDisplayRequired = COMM_DISPLAY_RX_SYSTEM;
					// i=* queries are echos of system calls and happen often so only show if watching
				}
			}
			else if (memcmp(g_cRxBuff,"ob[1]=0",7)==0)
			{
				p_WhistleUDP->cCommDisplayRequired = COMM_DISPLAY_RX_SYSTEM;
			}
			b = p_WhistleUDP->cCommDisplay & p_WhistleUDP->cCommDisplayRequired;
			//This test code block helps debug commdisplay issues
			//if (b==0)
			//{
			//	//logf("no b: %d %d", p_WhistleUDP->cCommDisplay, p_WhistleUDP->cCommDisplayRequired);
			//	b=1;
			//}
			//Search the Whistle Response to see if it has an '?' which indicates a bad command
			for(i = rxcnt - 1 ; i >= 1 ; i--)
			{
				if(g_cRxBuff[i] == cFailExp)
				{
					byte errorcode = (byte)g_cRxBuff[i-1];
					logf("%s Bad command: e=%d\r\n", p_WhistleUDP->name, errorcode);
					char messagebuffer[256];
					snprintf(messagebuffer, 256, "%s Bad command: e=%d : %s",g_cRxBuff, errorcode, g_cRxBuff);
					SendGenericMessage(messagebuffer); //FIXME PORTLOW future should not use this oid for messages
					b = 1; //show this response that included the error
					break;
				}
			}

			//Display the Whistle Response
			if (b != 0)
			{
				s = fmtbuf;
				i = 0;
				while (g_cRxBuff[i] != 0)
				{
					*s = g_cRxBuff[i];
					if (*s == '\r')
					{
						*s++ = '\\';
						*s = 'r';
					}
					s++;
					i++;
				}
				*s = 0;
				logf("rx%s:%s\r\n", p_WhistleUDP->name, fmtbuf);
				//DUMPARRAY("\r\ng_cRxBuff\r\n",g_cRxBuff,rxcnt);
			}
			fxParseWhistleUDPMsg(p_WhistleUDP); //assume message is in g_cRxBuff due to face this is single threaded
			if (MS_TIMER - uistart < 5)
			{
				//read_another_packet now (read until nothing to read)
				continue;
			}
		}
		//do not check for another packet now
		break;
	}
	//restore echo to basic level
	p_WhistleUDP->cCommDisplayRequired = COMM_DISPLAY_TX;
}

//Heartbeat VERSION 1 (based on old tech... should be improved with idea)
void ServiceWhistleUDPHeartbeat(td_WhistleUDP * p_WhistleUDP, void (*fxHBTimeout)())
{
	char c;
	//FIXME PORTLOW  wish to redo how heartbeat works, but for now, this is what we have.

	//Run Heartbeat
	if (MS_TIMER - p_WhistleUDP->HBsentTime > WHISTLE_HEARTBEAT_TIMEOUT)
	{
		if (p_WhistleUDP->cHBsent == TRUE)
		{
			//was not cleared
			logf("Whistle %s COMM timeout...\r\n", p_WhistleUDP->name);
			p_WhistleUDP->cHBTimeout = 1; //signal timeout
			if (fxHBTimeout!=0)
			{
				fxHBTimeout(p_WhistleUDP);
			}
		}

		//Only display this tx if system tx is turned on for display...
		//system tx = bit 4 so for this special message shift the comm display down 2 bits, and restore display afterwards

		p_WhistleUDP->cCommDisplayRequired = COMM_DISPLAY_TX_SYSTEM;

		//Now Send No matter what
		sendWhistleUDPMsgWL(p_WhistleUDP, "mo;\r", 4);
		p_WhistleUDP->HBsentTime = MS_TIMER;
		p_WhistleUDP->cHBsent = TRUE;

		//restore echo to basic level
		p_WhistleUDP->cCommDisplayRequired = COMM_DISPLAY_TX;
	}
}

////////////////////////////////////////////////////////////////////////////////
// Whistle: sendWhistleUDPMsg
////////////////////////////////////////////////////////////////////////////////

int sendWhistleUDPMsgW(td_WhistleUDP * p_WhistleUDP, char * p_cTxBuff)
{
	int iLen;
	iLen = strlen(p_cTxBuff);
	return sendWhistleUDPMsgWL(p_WhistleUDP, p_cTxBuff, iLen);
}

int sendWhistleUDPMsgWL(td_WhistleUDP * p_WhistleUDP, char * p_cTxBuff, int iLen)
{
	int i;
	int itx;
	int iNumWrites;
	int retval;
	char c;
	byte b;

	if (p_WhistleUDP->cCommEnable != 1)
	{
		return 0;
	}

#ifdef USE_OUTPUT
	b = p_WhistleUDP->cCommDisplay & p_WhistleUDP->cCommDisplayRequired;
	//This test code block helps debug commdisplay issues
	//if (b==0)
	//{
	//	logf("no b: %d %d", p_WhistleUDP->cCommDisplay, p_WhistleUDP->cCommDisplayRequired);
	//	b=1;
	//}
	if (b != 0)
	{
		c = p_cTxBuff[iLen - 1];
		if (c == '\r')
		{
			logf("tx%s:%s\n", p_WhistleUDP->name, p_cTxBuff);
		}
		else
		{
			logf("tx%s:%s\r\n", p_WhistleUDP->name, p_cTxBuff);
		}
	}
#endif

//Note: Currently there is no prevention here if connection is not working
	if (iLen >= WHISTLE_TX_BUFFER_SIZE)
	{
#ifdef USE_OUTPUT
		logf("@@@%s:FATAL:msg longer than buffer. buffer was overrun.\r\n",
				p_WhistleUDP->name);
#endif
		iLen = WHISTLE_TX_BUFFER_SIZE;
		return 0;
	}

	if (p_WhistleUDP->sock > 0)
	{
		//sendto(int socket, char data, int dataLength, flags, destinationAddress, int destinationStructureLength)
		retval = sendto(p_WhistleUDP->sock, p_cTxBuff, iLen, 0,
				(struct sockaddr*) &p_WhistleUDP->remotesockaddr,
				sizeof(struct sockaddr_in));
		if (retval < 0)
		{
			printf("Error sending packet: %s\n", strerror(errno));
			close(p_WhistleUDP->sock);
			p_WhistleUDP->sock = 0;
			retval = 0;
		}
	}

	p_WhistleUDP->WhistleUDPMonitor.bytessent += iLen;

	return retval;
}



//Whistle UDP Monitor Functions

void WhistleUDPMonReset(td_WhistleUDPMonitor * p_mon)
{
	p_mon->ulSMTime = 0;
	p_mon->bytessent = 0;
	p_mon->bytesrecd = 0;
	p_mon->avgsent = 0;
	p_mon->avgrecd = 0;
	p_mon->peaksent = 240; //so that it won't show at the start
	p_mon->peakrecd = 0;
}

void WhistleUDPMonUpdate(td_WhistleUDP * p_WhistleUDP)
{
	unsigned long ulMSTime;
	unsigned long ulMS;
	char * name;
	td_WhistleUDPMonitor * p_mon;

	name = p_WhistleUDP->name;
	p_mon = &p_WhistleUDP->WhistleUDPMonitor;

	ulMSTime = MS_TIMER;
	ulMS = ulMSTime - p_mon->ulSMTime;
	if (ulMS < 250)
	{
		//do not process this sample yet
		return;
	}
	p_mon->ulSMTime = ulMSTime; //update time for next sample

//FIXME LOW   REDO MONITOR     BYTE LIMIT ISSUE FOR MONITOR<<<<<<<<<<<<<<<<<<<<<<<<<

//	Calculate average traffic for the last sample.
	p_mon->avgsent = (unsigned int) (((unsigned long) (p_mon->bytessent)) * 1000L / ulMS); //Find the true bytes per second
	p_mon->avgrecd = (unsigned int) (((unsigned long) (p_mon->bytesrecd)) * 1000L / ulMS);

//	Clear for Next Sample
	p_mon->bytessent = 0;
	p_mon->bytesrecd = 0;

//	Update peak traffic count since power up.
	if (p_mon->avgsent > p_mon->peaksent)
	{
		p_mon->peaksent = p_mon->avgsent;
		logf("@@@%s: peak tx %u\r\n", name, p_mon->peaksent);
	}
	else if (p_mon->avgsent > (p_mon->peaksent - p_mon->peaksent / 8))
	{
		logf("@@@%s: last tx %u peak tx %u\r\n", name, p_mon->avgsent, p_mon->peaksent);
	}
	if (p_mon->avgrecd > p_mon->peakrecd)
	{
		p_mon->peakrecd = p_mon->avgrecd;
		logf("@@@%s: peak rx %u\r\n", name, p_mon->peakrecd);
	}
	else if (p_mon->avgrecd > (p_mon->peakrecd - p_mon->peakrecd / 8))
	{
		p_mon->peakrecd = p_mon->avgrecd;
		logf("@@@%s: last rx %u peak rx %u\r\n", name, p_mon->avgrecd, p_mon->peakrecd);
	}

}

void DUMPARRAY(char * label, char * buffer, int count)
{
	int i;
	char c;

	logf(label);
	i = 0;
	while (i < count)
	{
		c = buffer[i];
		if (c >= 0x20)
		{
			logf("%d  %d '%c'\r\n", i, c, c);
		}
		else
		{
			logf("%d  %d .\r\n", i, c);
		}
		i++;
	}
}

