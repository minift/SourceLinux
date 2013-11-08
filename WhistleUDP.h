//WhistleUDP

#ifndef WHISTLEUDP_H_
#define WHISTLEUDP_H_

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "SmartTool.h"

//Should never delay when reading... get what is there...
#define WHISTLE_RXTIMEOUT_MS 0
#define WHISTLE_MAXNUM_WRITES 25  // used in sendWhistleMsg (number of consecutive attempted writes)
#define WHISTLE_BUFFER_SLIDEBACK_SIZE 256

#define WHISTLE_BUFFER_SIZE 512
#define WHISTLE_TX_BUFFER_SIZE 256

#define COMM_DISPLAY_RX 1
#define COMM_DISPLAY_TX 2
#define COMM_DISPLAY_RX_SYSTEM 4
#define COMM_DISPLAY_TX_SYSTEM 8
#define COMM_DISPLAY_RX_PX 16
#define COMM_DISPLAY_TX_PX 32

typedef struct {
	unsigned long ulSMTime;
	unsigned int bytessent;
	unsigned int bytesrecd;
	unsigned int avgsent;
	unsigned int avgrecd;
	unsigned int peaksent;
	unsigned int peakrecd;
} td_WhistleUDPMonitor;

typedef struct {
	char * name;
	char cAxis;
	char * localIP;
	int localport;
	struct sockaddr_in localsockaddr;
	char * remoteIP;
	int remoteport;
	struct sockaddr_in remotesockaddr;
	int sock;
	int iOpen;
	char cCommFlag;
	//Whistle Message System
	uint32 uiComMsgRequested;
	char * comMsgCmd;
	int comMsgCmdLen;
	char * comMsgClearCmd;
	int comMsgClearCmdLen;
	char cCommTimeout;
	int lasti;
	//Traffic Monitor
	td_WhistleUDPMonitor WhistleUDPMonitor;
	//A pointer to the parse function
	int (*parseWhistleMsg) ();
	char cCommEnable;
	char cCommDisplay;
	char cCommDisplayRequired;
	//Heartbeat VERSION 1 (based on old tech... should be improved with idea)
	uint32 HBsentTime;
	uint32 HBlastTime;
	char cHBsent;
	char cHBTimeout;
} td_WhistleUDP;

void initWhistleUDP(char * name, char cAxis, char * localIP, int localport, char * remoteIP, int remoteport, char * comMsgCmd, char * comMsgClearCmd, td_WhistleUDP * p_WhistleUDP);
void CheckWhistleUDPPort(td_WhistleUDP * p_WhistleUDP);
void ServiceWhistleUDP(td_WhistleUDP * p_WhistleUDP, int digin_chnum_comm_flag, void (*fxParseWhistleUDPMsg)());
//Heartbeat VERSION 1 (based on old tech... should be improved with idea)
void ServiceWhistleUDPHeartbeat(td_WhistleUDP * p_WhistleUDP, void (*fxHBTimeout)()); //heartbeat v1

void ParseWhistleUDPMsg(td_WhistleUDP * p_WhistleUDP);
int sendWhistleUDPMsgW(td_WhistleUDP * p_WhistleUDP, char * p_cTxBuff);
int sendWhistleUDPMsgWL(td_WhistleUDP * p_WhistleUDP, char * p_cTxBuff, int iLen);
//Traffic Monitor Functions
void WhistleUDPMonReset(td_WhistleUDPMonitor * p_sm);
void WhistleUDPMonUpdate(td_WhistleUDP * p_WhistleUDP);


extern char g_cRxBuff[];
extern char g_cTxBuffer[];
extern char g_cTxBuffer2[];



#endif
