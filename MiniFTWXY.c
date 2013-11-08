//MiniFTWhistle Library

//Use the new model with monitor thread and no kill/starts
//#define WHISTLE_CONTINUOUS

// MiniFT Whistle Gold Motion Library
//
// The Library is based on new methods rather than a direct port of the entire old MiniFT Whistle Library
// This library is based on the Whistle Gold UDP communication library.
// Many Features have been implemented differently or have not been implemented in the new library because they are not needed.
//
// Anything Related to the old library init and serial sub systems has been replaced.
// MemInitMC and InitMC .  The New library has one call which inits everything.
// void MiniFTWhistleInit(void);
// void MiniFTWhistleService(void);
// Implement the main Init and Service calls.
// The Serial Write and Read Loops have not been ported. (Uses the WhistleUDP lib)
//
//FIXME PORTMED5 "revise"
//
// Heartbeat and Timeouts were ported, but are different
//
//
// These items were not ported at all.
//
// DRIVETHROUGHBACKLASH .   This could be done again using the old as a reference if ever needed.
//
// RunProbeHome.  This was really used for Hard Stop Homing and has
//


//FIXME PORTMED
//HAVE plan for timeout detection
//When moving, monitor at higher rate.... timeout activate
//When not moving monitor at different rate, but anytime an unanwered query codes

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "MiniFTDefs.h"
#include "SocketConsole.h"
#include "WhistleUDP.h"
#include "MiniFTWXY.h"
#include "STP.h"
#include "hwio.h"
#include "SmartToolUtil.h"
#include "CommonSmartTool.h"
#include "MiniFTSmartTool.h"
#include "MiniFTIO.h"

//control some whistle specific debug output

//FIXME PORTMED check use_output ... remove generic...
#define USE_OUTPUT
#define OUTPUT_WHISTLE_MESSAGE_STATES
#define OUTPUT_PX_AND_I_OUTPUT
#define OUTPUT_WHISTLE_HEARTBEAT
#define OUTPUT_WHISTLE_RUNANDSTOPS
#define OUTPUT_MVN_START
#define OUTPUT_MVN_ECHO

#define OUTPUT_RX_SHOW_UNMATCHED
#define OUTPUT_RX_SHOW_IGNORED
//#define OUTPUT_RX_SHOW_IGNORED_VERBOSE

//FIXME PORTMED  want even newer jog method that is like crawer with local monitor...

#if 0
//A reference to the Functions that sent status back in Crawler
extern void SendMoveStatus(void);
#endif

char *p_szaMsgCodes[] =
{
	"NoMessage",
	"GravCompRun",
	"GravCompOK",
	"GravCompFail",
	"PositionRun",
	"PositionOK",
	"PositionFail",
	"PositionStop",
	"Unused 8",
	"JogRun",
	"PositionStart",
	"Unused 11",
	"HomeRun",
	"HomeOK",
	"HomeFail",
	"Unused 15",
	"AutoPERR",
	"AutoER",
	"Unused 18",
	"Unused 19",
	"Unused 20",
	"Unused 21",
	"Unused 22",
	"Unused 23",
	"ForceLimitCurrent 24",
	"FindWinRun 25",
	"FindWinOK 26",
	"FindWinFail 27",
	"FindWinNone 28",

};

#ifdef PRIMARY_ENCODER_PX
#ifdef PRIMARY_ENCODER_PY
#error "FATAL: Defined both PX and PY as primary encoder"
#endif
#endif

//In Elmo Gold, Single And Aux are both UM=5 and PX reports the position.
#define PX "px"
#define PXXUM "xum=5;\r"
#ifdef PRIMARY_ENCODER_PX
#else
#ifdef PRIMARY_ENCODER_PY
#else
#error "FATAL: Must define either PRIMARY_ENCODER to be PX or PY"
#endif
#endif

//////////////////////////
//Functions
//////////////////////////

void MiniFTWhistleInit(void);
void MiniFTWhistleService(void);
//FIXME PORTMED   ways to set hb timeout off
void MCSetCommDisplayX(byte c);
void MCSetCommDisplayY(byte c);
void MCSetCommEnableX(byte cEnable);
void MCSetCommEnableY(byte cEnable);
void MCparseWhistleUDPMsg(td_WhistleUDP * p_WhistleUDP);
void MCHBTimeout(td_WhistleUDP * p_WhistleUDP);
void MCSetConfigParams(void);
void MCSetEncoderRatios();
void MCSetErrorLimits();
void MCSetCurrentLimits(float fx, float fy, float fpx, float fpy);
void MCSetPositionLimits(byte bAllowLimitSetNow);
void MCSetVelmonAndBoostParameters(void);
void MCSetSD();
void MCClearMotionSet();
void MCRunRestart();
void RunServoHereNowW();
void RunMotorOffNow();
void RunMotorOffNowWYOnly();
void RestartMainWhistleThread();
void MCRunEstopWithPower();
void MCRunEstopEngage();
void MCRunEstopDisengage();
void RunGravComp();
void RunFloat();
void RunUnfloat();
void RunLoadYRail(byte c);
//void ZeroPosition_DEPRECATED();
//void SetPosition_PENDING(float fx, float fy);
void MCGetPosition();
void MCGetPositionCounts();
void RunProbeK();
void RunProbeAdjust();

void ProbeTeachPosition();
void RunJogX(byte cx, float fx);
void RunJogY(byte cy, float fy);
void StopJogX();
void StopJogY();
void RunHomeX();
void RunHomeY();
void RunFindWindowX(float x);
void MCSetMoveSpeedParams(float fsx, float fsy, float facx, float facy, float fdcx, float fdcy);
void MCSetMoveSpeedParamsEven(float fs, float facdc);
void MCSetMoveSpeedParamsX(float fsx, float facx, float fdcx);
void MCSetMoveSpeedParamsY(float fsy, float facy, float fdcy);
void MCSetMoveParams(float fErrLev, byte cMoveOpt);
void MCRunPosition(float x, float y);
void MCStopPosition();
void MCStopPositionAdvanced();
void MCStopAndFailMove();
void SetForceLimitingCurrents(void);
void SetForceLimitingCurrentsX(void);
void SetForceLimitingCurrentsY(void);
void UpdateForceLimitingCurrents(void);
void MiniFTSendWhistleMsgX(char * s);
void MiniFTSendWhistleMsgXL(char * s, int l);
void MiniFTSendWhistleMsgY(char * s);
void MiniFTSendWhistleMsgYL(char * s, int l);
void MiniFTSendWhistleMsgXY(char * s);
void MiniFTSendWhistleMsgXYL(char * s, int l);
void RunMotorActionBrakeTimeoutCheck();
void RunMotorActionAirClear();
void SetMotorActionDirectly();

//FIXME PORTLOW
//These headers are for functions in main which have no better header for include at this time:
void SendActivePremove(td_STPsessions * p_STPSession, unsigned int uiMsgType);

//FIXME PORTMEDHIGH
// lib still contains remnants of crawler system, but eventually must clearn these vars to be perfect
// and based on the MiniFT Linux + Gold sub system

//Public Variables
byte g_cMCRunningMsgGoal = NoMessage;
byte g_cMCWindowFound = 0;

//Private Variables

td_WhistleUDP g_WhistleUDPX;
td_WhistleUDP g_WhistleUDPY;

byte g_cMCMsg = 0;

byte g_cMCMotionSet = 0;

//long g_lMClastsp = 0;
//long g_lMClastac = 0;
//long g_lMClastdc = 0;

//Crawler Had one number all motors
//uint16 g_uiMCMoveNumSeq = 1;
//uint16 g_uiMCMoveNum = 0;
//uint16 g_uiMCMoveNumEcho = 0;
//int32 g_iMCpgl = 0;

//For Crawler Style Move monitor
//uint32 g_uiMCMoveStart = 0;
//uint32 g_uiMonitorMoveLastCheck = 0;
byte g_cMoveX = 0;
byte g_cMoveY = 0;

uint32 g_uiMCBootTime = 0;
byte g_cBootComms = 0;

//Motor Action Flag
byte g_cMotorAction = 0;
byte g_cLastMotorAction = 0;
uint32 g_uiMotorActionTime = 0;

//Cralwer style
//byte g_cMCFreshCurPosn = GETPOSN_CLEAR;
//float g_fMCLastKnownPosn = 0;

byte g_cGravCompProgress = 0;
byte g_cPositionProgress = 0;
byte g_cProbeHomeProgress = 0;
int g_cIgnoreStaleStatusX = 0;
int g_cIgnoreStaleStatusY = 0;

byte g_cMotionSet = UnknownMotionSet; //Clear Motion Set

float g_fLastJogFactorX = 1.01;
float g_fLastJogFactorY = 1.01;

int g_ipglx = 0; //FIXME PORTMED  implement a protection around the distance limit
int g_ipgly = 0;

int g_ipglechox = 0;
int g_ipglechoy = 0;

byte g_cGravCompDir = 0; //flag helps track rx of direction values.

unsigned int g_uiMoveNumSeq = 1;
unsigned int g_uiMoveNum = 0;
unsigned int g_uiMoveNumEchoX = 0;
unsigned int g_uiMoveNumEchoY = 0;

#ifdef WHISTLE_CONTINUOUS
//Does not run jog monitor each time but has a permanent monitory
#else
byte g_cJogRunXFlag = 0;
byte g_cJogRunYFlag = 0;
#endif

//for pxt
int g_iPXTPendingX = 0;
int g_iPXTPendingY = 0;

uint32 g_uiPacketNumberX=0;
uint32 g_uiPacketNumberY=0;
uint32 g_uiLastJVZeroPacketX=0;
uint32 g_uiLastJVZeroPacketY=0;

#define XCOUNTS(X) ((int32)(X*g_ConfigData.EncoderRatio.fX))
#define YCOUNTS(Y) ((int32)(Y*g_ConfigData.EncoderRatio.fY))

#define sendWhistleMsgX(s) sendWhistleUDPMsgW(&g_WhistleUDPX,s)
#define sendWhistleMsgXL(s,l) sendWhistleUDPMsgWL(&g_WhistleUDPX,s,l)
#define sendWhistleMsgY(s) sendWhistleUDPMsgW(&g_WhistleUDPY,s)
#define sendWhistleMsgYL(s,l) sendWhistleUDPMsgWL(&g_WhistleUDPY,s,l)

#define sendWhistleMsgXY(s) sendWhistleMsgX(s);sendWhistleMsgY(s);
#define sendWhistleMsgXYL(s,l) sendWhistleMsgXL(s,l);sendWhistleMsgYL(s,l);

//FIXME PORTLOW do real fabs
#define fabsinline(fvar) ((fvar)<0 ? (-(fvar)) : (fvar))

void MiniFTWhistleInit(void)
{
	char * msg;
	//uint32 ui;

	logf("MiniFTWhistleInit\r\n");

	//just clear variables that need to be maintained
	g_cMoveDone = MOVEDONE_TRUE;

	g_uiMCBootTime = MS_TIMER;

	initWhistleUDP("wx", X_AXIS, 0, 5051, "192.168.0.21", 5001, "i;\r", "i=0;\r", &g_WhistleUDPX);
	initWhistleUDP("wy", Y_AXIS, 0, 5052, "192.168.0.22", 5001, "i;\r", "i=0;\r", &g_WhistleUDPY);

	//turn all whistle output on during this boot
	MCSetCommDisplayX(31); //special code is 11111 which is like turning on all output plus an extra bit to show it was from boot time
	MCSetCommDisplayY(31);
	g_cBootComms = 1;

	//Serivce Ports,which may help ensure they are really open now
//FIXME PORTHIGH ensure that the ports are ready and that init will make it...

//FIXME PORTHIGH REVIEW THE ENTIRE SETUP HERE  AND CONSIDER NEW MANAGEMENT OF ALL THE PARAMETERS  (moving to the proper new system)

	//This May be overkill, but it resolves the problems
	//ui = MS_TIMER;
	usleep(500);
	msg = "ver;\r";
	sendWhistleMsgXY(msg);
	usleep(500);
	msg = "kl;\r";
	sendWhistleMsgXY(msg);
	usleep(500);

	sendWhistleMsgXY(PXXUM);
	usleep(500);

	MCSetConfigParams(); //Set the parameters which are not set at the start of each operation.
}


void MiniFTWhistleService(void)
{
	//Service X
	ServiceWhistleUDP(&g_WhistleUDPX, DI_COM_FLAG_X, &MCparseWhistleUDPMsg);
	ServiceWhistleUDPHeartbeat(&g_WhistleUDPX, &MCHBTimeout);
	WhistleUDPMonUpdate(&g_WhistleUDPX);

	//Service Y
	ServiceWhistleUDP(&g_WhistleUDPY, DI_COM_FLAG_Y, &MCparseWhistleUDPMsg);
	ServiceWhistleUDPHeartbeat(&g_WhistleUDPX, &MCHBTimeout);
	WhistleUDPMonUpdate(&g_WhistleUDPY);

	//FIXME PORTLOW this would do the Crawler style move monitor... doesn't make sense for MiniFT completely,
	//but a number of updates we want to do might benefit from that pattern
	//MCMonitorMove();

	if (g_cBootComms == 1)
	{
		if ((MS_TIMER - g_uiMCBootTime) > 5000)
		{
//FIXME PORTLOW  Turn Book Com into a more formal feature and clean up
			if (g_WhistleUDPX.cCommDisplay == 31)
			{
				g_WhistleUDPX.cCommDisplay = 0;
			}
			if (g_WhistleUDPY.cCommDisplay == 31)
			{
				g_WhistleUDPY.cCommDisplay = 0;
			}
			g_cBootComms = 0;
		}
	}

#ifdef BRAKE_ON_TIMEOUT
	RunMotorActionBrakeTimeoutCheck();
#endif
#ifdef AIR_CLEAR
	RunMotorActionAirClear();
#endif

	//FIXME PORTLOW
	//combine timeout with monitoring
	//make a not moving move monitor that checks statut

}

void MCSetCommDisplayX(byte c)
{
	g_WhistleUDPX.cCommDisplay = c;
}
void MCSetCommDisplayY(byte c)
{
	g_WhistleUDPY.cCommDisplay = c;
}

void MCSetCommEnableX(byte cEnable)
{
	g_WhistleUDPX.cCommEnable = cEnable;
}
void MCSetCommEnableY(byte cEnable)
{
	g_WhistleUDPY.cCommEnable = cEnable;
}

////////////////////////////////////////////////////////////////////////////////
// Whistle:  MCparseWhistleUDPMsg
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Whistle:  parseWhistleMsg
////////////////////////////////////////////////////////////////////////////////

void MCparseWhistleUDPMsg(td_WhistleUDP * p_WhistleUDP)
{
	char * p_cRxBuff;
	char c;
	char* p_cParse;
	//char* p_c;
	int i;
	float fp, fn;
	float fx, fy;
	//long lx, ly;
	//int ipgl;
	unsigned int ui;
	int pxt;

	//unsigned long ul; //temp unsigned long is just ul
	long lTemp;
	long l1, lpgl;
	char cNoClear;
///////

	p_cRxBuff = g_cRxBuff; //UDP Packet stored here

	//FIXME PORTMED  Catox ??? use numer parsing system
	//not using catox lib yet
	//_ctoerror=0; //clear the error state, because it may not get cleared and it may be set

	p_cParse = p_cRxBuff;

	char cWhistleAxis = p_WhistleUDP->cAxis;
	if (cWhistleAxis == X_AXIS)
	{
		g_uiPacketNumberX++;
	}
	else
	{
		g_uiPacketNumberY++;
	}

	c = *p_cParse;

	if (c == 'o')
	{
		if (memcmp(p_cParse, "ob[1]=", 6) == 0)
		{
			goto case_ignore_quiet;
			//ignore
		}
	}
	else if (c == 'i')
	{
		if (memcmp(p_cParse, "i;", 2) == 0)
		{
			// i is variable on Whistle that holds the msg code from the Whistle
			p_cParse += 2;
			i = atoi(p_cParse);

#ifdef OUTPUT_WHISTLE_MESSAGE_STATES
			//Display the Message State Name
			if (i != p_WhistleUDP->lasti)
			{
				char * p_szMsg = "Unknown";
				if (i<sizeof(p_szaMsgCodes))
				{
					p_szMsg = p_szaMsgCodes[i];
				}
				logf("%s to %d-%s\r\n", p_WhistleUDP->name, i, p_szMsg);
				p_WhistleUDP->lasti = i;
			}
#endif

			//Check in advance so that items below that use this can use fast local cEchoMatched
			char cEchoMatched = 1;
			if (cWhistleAxis == X_AXIS)
			{
				if (g_uiMoveNumEchoX != g_uiMoveNum)
				{
					cEchoMatched = 0;
				}
			}
			else
			{
				if (g_uiMoveNumEchoY != g_uiMoveNum)
				{
					cEchoMatched = 0;
				}
			}

			cNoClear = 0;
			switch (i)
			{
			case NoMessage:
				cNoClear = 1;
				break;
			case GravCompRun:
				cNoClear = 1;
				//ignore
				break;
			case GravCompOK:
				if (g_cMCRunningMsgGoal == GravCompOK)
				{
					sendWhistleUDPMsgWL(p_WhistleUDP, "gcp;\r", 5);
					sendWhistleUDPMsgWL(p_WhistleUDP, "gcn;\r", 5);
					//FIXME PORTLOW check that these actually arrive and THEN gc is complete.
					g_cGravCompProgress |= cWhistleAxis;
					if (g_cGravCompProgress == X_AND_Y)
					{
						//clear flag
						g_cMCRunningMsgGoal = NoMessage;
						//success on both
						g_cGravCompStatus = GRAVCOMP_PASS;
						g_cFloatStatus = FLOATSTAT_FLOAT;
						//blink
						LEDCount(1, 500, 500);
						//beep
						Beep();
						SmartToolMsg(0, STP_ALERT, MINIFT_OID_GRAVCOMP_STATUS, 1, (char *)&g_cGravCompStatus);
						SmartToolMsg(0, STP_ALERT, MINIFT_OID_FLOAT_STATUS, 1, (char *)&g_cFloatStatus);
						//Get the Results
						g_cGravCompDir = 0;
						//Motion
						g_cMotorAction = 0;
					}
				}
				break;
			case GravCompFail:
				if (g_cMCRunningMsgGoal == GravCompOK)
				{
					//only raise error if other has not been raised 1st.
					SmartToolMsgMiniFtMessageCode(0, MINIFT_OID_MCERR, MINIFTMC_GRAVCOMP_FAILURE);
					//must now set to ERROR
					if (g_cGravCompStatus == GRAVCOMP_RUNNING)
					{
						//only record the 1st error while running... either x or y
						if (cWhistleAxis == X_AXIS)
						{
							g_cGravCompStatus = GRAVCOMP_FAILX;
						}
						else
						{
							g_cGravCompStatus = GRAVCOMP_FAILY;
						}
					}
					g_cFloatStatus = FLOATSTAT_NOFLOAT;
					//beep
					BeepMCERR()
					;
					SmartToolMsg(0, STP_ALERT, MINIFT_OID_GRAVCOMP_STATUS, 1, (char *)&g_cGravCompStatus);
					SmartToolMsg(0, STP_ALERT, MINIFT_OID_FLOAT_STATUS, 1, (char *)&g_cFloatStatus);

					//clear flag
					g_cMCRunningMsgGoal = NoMessage;
					//ensure that neither is going to complete grav comp or be in float
					#ifdef WHISTLE_CONTINUOUS
					//FIXME PORTLOW no grav comp support
					#else
					sendWhistleMsgXYL("er[3]=pem;\r", 11);
					sendWhistleMsgXYL("kl;\r", 4);
					sendWhistleMsgXYL("XQ##UFLT;\r", 10);
					#endif
					//Motion
					g_cMotorAction = 0;
				}
				break;
			case PositionStart:
				//Starting Position but have not seen flag indicating motion yet
				cNoClear = 1;
				break;
			case PositionRun:
				cNoClear = 1;
				//ignore
				break;
			case PositionOK:
				if (cEchoMatched == 0)
				{
					//We haven't seen the echo for our last move yet,
					//which means this might be the completion of a previous move.
					//This also means that we have started a new move, so we don't want to
					//report that the move is done yet.
					break;
				}
				if (g_cMCRunningMsgGoal == PositionOK)
				{
					if (cWhistleAxis == X_AXIS)
					{
						ProfilePoint("X DPOSN");
						logf("X*\r\n");
					}
					else
					{
						ProfilePoint("Y DPOSN");
						logf("Y*\r\n");
					}
					g_cPositionProgress |= cWhistleAxis;
					if (g_cPositionProgress == X_AND_Y)
					{
						//clear flag
						g_cMCRunningMsgGoal = NoMessage;
						g_cMoveDone = MOVEDONE_TRUE;  // send ack to MODE_POSN state machine
						//Let Main system apply brake after done moving
						//#ifdef BRAKE_ON_WHEN_STOPPED
						//BrakeOn();
						//#endif
#ifdef OUTPUT_POSITION_STATES
						logf("Done:Posn\r\n");
#endif
						ProfilePoint("DPOSN");
						g_cMotorAction = 0;
					}
				}
				else if (g_cMCRunningMsgGoal == PositionStop)
				{
					//They were waiting for a stop
					g_cPositionProgress |= cWhistleAxis;
					if (g_cPositionProgress == X_AND_Y)
					{
						logf("Trt DONE as STOP\r\n");
						//clear flag
						g_cMCRunningMsgGoal = NoMessage;
						g_cMoveDone = MOVEDONE_STOP;  // send ack to MODE_POSN state machine
						g_uiMoveNum = 0;
#ifdef BRAKE_ON_WHEN_STOPPED
						BrakeOn();
#endif
#ifdef OUTPUT_POSITION_STATES
						logf("Done:Posn via Stop\r\n");
#endif
					}
				}
				break;
			case PositionFail:
				if (cEchoMatched == 0)
				{
					//We haven't seen the echo for our last move yet,
					//which means this might be the completion of a previous move.
					//This also means that we have started a new move, so we don't want to
					//report that the move is done yet.
					break;
				}
				if (g_cMCRunningMsgGoal == PositionOK)
				{
					if (g_cMoveDone != MOVEDONE_TRUE && g_cMoveDone != MOVEDONE_ERROR)
					{
						//only raise error if other has not been raised 1st. //FIXME Enhancement... instead implement error stacking on pendant
						if (cWhistleAxis == X_AXIS)
						{
							SmartToolMsgMiniFtMessageCode(0, MINIFT_OID_MCERR, MINIFTMC_BEYOND_TOLERANCE_X);
						}
						else
						{
							SmartToolMsgMiniFtMessageCode(0, MINIFT_OID_MCERR, MINIFTMC_BEYOND_TOLERANCE_Y);
						}
						//must now set to ERROR
						g_cMoveDone = MOVEDONE_ERROR;  // send error back to MODE_POSN state machine
						//Since this is an error... always throw brake on
						BrakeOn();

						//clear flag
						g_cMCRunningMsgGoal = NoMessage;
						g_uiMoveNum = 0;
						//get last moves info for special test measurements
						//sendWhistleXYMsg("tm;tms;tm-tms;\r"); //Gather these variables for analysis //FIXMENOWNOWNOW later remove
						//old ways              //ensure that neither is going to complete the move
						//RunServoHereNowW(); //FIXMENOWNOWNOW Test this out, and consider only stopping the OTHER one....
						//Don't Use Brake Here... main move will take care of final brake
						//Motion
						g_cMotorAction = 0;
					}
					BeepMCERR()
					;
				}
				else if (g_cMCRunningMsgGoal == PositionStop)
				{
					//They were waiting for a stop
					logf("Trt Posn Err as STOP\r\n");
					//clear flag
					g_cMCRunningMsgGoal = NoMessage;
					g_cMoveDone = MOVEDONE_STOP;  // send ack to MODE_POSN state machine
					g_uiMoveNum = 0;
#ifdef BRAKE_ON_WHEN_STOPPED
					BrakeOn();
#endif
#ifdef OUTPUT_POSITION_STATES
					logf("Done:Posn via Stop\r\n");
#endif
					//Motion
					g_cMotorAction = 0;
				}
				//FIMXENOWNOW TEST : Recheck : Can this pattern guarantee that reported error
				// will kill possibillity of being on position with a flase positive indication?
				break;
			case PositionStop:
				if (cEchoMatched == 0)
				{
					//We haven't seen the echo for our last move yet,
					//which means this might be the completion of a previous move.
					//This also means that we have started a new move, so we don't want to
					//report that the move is done yet.
					break;
				}
				//Last Echo matches the last command
				if (g_cMCRunningMsgGoal == PositionStop)
				{
					g_cPositionProgress |= cWhistleAxis;
					if (g_cPositionProgress == X_AND_Y)
					{
						//clear flag
						g_cMCRunningMsgGoal = NoMessage;
						g_cMoveDone = MOVEDONE_STOP;  // send error back to MODE_POSN state machine
						g_uiMoveNum = 0;
						//Now that stop is confirmed, brake
#ifdef BRAKE_ON_WHEN_STOPPED
						BrakeOn();
#endif
#ifdef OUTPUT_POSITION_STATES
						logf("Done:Posn via Stop\r\n");
#endif
						//Motion
						g_cMotorAction = 0;
					}
				}
				else if (g_cMCRunningMsgGoal != NoMessage)
				{
					logf("Stop/NS?\r\n"); //Got Stop but not waiting for stop???
				}
				break;
			case JogRun:
				break;
			case HomeRun:
				#ifdef HOMESYSTEM
				if (g_cMCRunningMsgGoal == HomeOK)
				{
#ifdef HOMESYSTEM_X
					if (g_cHomedX != HOME_RUNNING)
#endif
#ifdef HOMESYSTEM_Y
						if (g_cHomedY != HOME_RUNNING)
						#endif
						{
							//clear flag
							g_cMCRunningMsgGoal = NoMessage;
						}
				}
#endif
				break;
			case HomeOK:
				#ifdef HOMESYSTEM
				if (g_cMCRunningMsgGoal == HomeOK)
				{
					if (cWhistleAxis == X_AXIS)
					{
#ifdef HOMESYSTEM_X
						g_cHomedX = HOME_DONE;
						g_cMotorAction &= (0xFF - X_AXIS);
#endif
					}
					else
					{
#ifdef HOMESYSTEM_Y
						g_cHomedY = HOME_DONE;
						g_cMotorAction &= (0xFF - Y_AXIS);
#endif
					}
#ifdef HOMESYSTEM_X
					if (g_cHomedX != HOME_RUNNING)
#endif
#ifdef HOMESYSTEM_Y
						if (g_cHomedY != HOME_RUNNING)
						#endif
						{
							//clear flag
							g_cMCRunningMsgGoal = NoMessage;
						}
				}
#endif
				break;
			case HomeFail:
				#ifdef HOMESYSTEM
				if (g_cMCRunningMsgGoal == HomeOK)
				{
					if (cWhistleAxis == X_AXIS)
					{
#ifdef HOMESYSTEM_X
						g_cHomedX = HOME_FAILURE;
						g_cMotorAction &= (0xFF - X_AXIS);
#endif
					}
					else
					{
#ifdef HOMESYSTEM_Y
						g_cHomedY = HOME_FAILURE;
						g_cMotorAction &= (0xFF - Y_AXIS);
#endif
					}
#ifdef HOMESYSTEM_X
					if (g_cHomedX != HOME_RUNNING)
#endif
#ifdef HOMESYSTEM_Y
						if (g_cHomedY != HOME_RUNNING)
						#endif
						{
							//clear flag
							g_cMCRunningMsgGoal = NoMessage;
						}
				}
#endif
				break;
			case FindWinRun:
				cNoClear = 1;
				//ignore
				break;
			case FindWinOK:
				//Implemented only for X Axis
				if (cWhistleAxis != X_AXIS)
				{
					break;
				}
				if (cEchoMatched == 0)
				{
					//We haven't seen the echo for our last move yet,
					//which means this might be the completion of a previous move.
					//This also means that we have started a new move, so we don't want to
					//report that the move is done yet.
					break;
				}
				if (g_cMCRunningMsgGoal == FindWinOK)
				{
					logf("FWOK\r\n");
					//Set Window Found Flag
					g_cMCWindowFound = 1;
					//Ask for Position right now
					MCGetPositionCounts();
					//clear flag
					g_cMCRunningMsgGoal = NoMessage;
					g_cMoveDone = MOVEDONE_TRUE;  // send ack to MODE_POSN state machine
#ifdef OUTPUT_POSITION_STATES
					logf("Done:FW\r\n");
#endif
					g_cMotorAction = 0;
				}
				break;
			case FindWinNone:
				//Implemented only for X Axis
				if (cWhistleAxis != X_AXIS)
				{
					break;
				}
				//Clear this no matter what
				g_cMCWindowFound = 0;
				if (cEchoMatched == 0)
				{
					//We haven't seen the echo for our last move yet,
					//which means this might be the completion of a previous move.
					//This also means that we have started a new move, so we don't want to
					//report that the move is done yet.
					break;
				}
				if (g_cMCRunningMsgGoal == FindWinOK)
				{
					logf("FWN\r\n");
					//clear flag
					g_cMCRunningMsgGoal = NoMessage;
					g_cMoveDone = MOVEDONE_TRUE;  // send ack to MODE_POSN state machine
#ifdef OUTPUT_POSITION_STATES
					logf("Done:FW\r\n");
#endif
					g_cMotorAction = 0;
				}
				break;
			case FindWinFail:
				//Implemented only for X Axis
				if (cWhistleAxis != X_AXIS)
				{
					break;
				}
				if (cEchoMatched == 0)
				{
					//We haven't seen the echo for our last move yet,
					//which means this might be the completion of a previous move.
					//This also means that we have started a new move, so we don't want to
					//report that the move is done yet.
					break;
				}
				if (g_cMCRunningMsgGoal == FindWinOK)
				{
					if (g_cMoveDone != MOVEDONE_TRUE && g_cMoveDone != MOVEDONE_ERROR)
					{
						//only raise error if other has not been raised 1st. //FIXME Enhancement... instead implement error stacking on pendant
						SmartToolMsgMiniFtMessageCode(0, MINIFT_OID_MCERR, MINIFTMC_BEYOND_TOLERANCE_X);
						//must now set to ERROR
						g_cMoveDone = MOVEDONE_ERROR;  // send error back to MODE_POSN state machine
						//Since this is an error... always throw brake on
						BrakeOn();

						//clear flag
						g_cMCRunningMsgGoal = NoMessage;
						g_uiMoveNum = 0;
						//get last moves info for special test measurements
						//sendWhistleXYMsg("tm;tms;tm-tms;\r"); //Gather these variables for analysis //FIXMENOWNOWNOW later remove
						//old ways              //ensure that neither is going to complete the move
						//RunServoHereNowW(); //FIXMENOWNOWNOW Test this out, and consider only stopping the OTHER one....
						//Don't Use Brake Here... main move will take care of final brake
						//Motion
						g_cMotorAction = 0;
					}
					BeepMCERR()
					;
				}
				break;
			case AutoPERR:
				//Always print out this state
				logf("@PERR\r\n");
				SmartToolMsgMiniFtMessageCode(0, MINIFT_OID_MCERR, MINIFTMC_UNABLE_TO_MOVE); //FIXMENOWNOWNOW Better Error ?? ?? ??

				sendWhistleUDPMsgWL(p_WhistleUDP, "ec;\r", 4);
				sendWhistleUDPMsgWL(p_WhistleUDP, "mf;\r", 4);
				sendWhistleUDPMsgWL(p_WhistleUDP, "i=0;\r", 5);

				//When Running A Position Movement, this has a clear effect.
				//When Waiting, this also will have an effect of stopping on position.
				//must now set to ERROR
				g_cMoveDone = MOVEDONE_ERROR;  // send error back to MODE_POSN state machine
				//Since this is an error... always throw brake on
				BrakeOn();
				//clear flag
				g_cMCRunningMsgGoal = NoMessage;
				g_uiMoveNum = 0;
				//beep
				BeepMCERR()
				;
#ifdef FORCE_LIMIT_WHISTLE_CODE
				//FOR Now I am giong to try not to kill anything...
#else
				//Unless using new system, continue to do what old did.
				if (g_cModeState != MODE_ESTOP)
				{
					//stop both x and y
					RunServoHereNowW();
					//FIXME PORTHIGH  must check for GOLD differences and also must have fall protection Velocity limit for case like a kl and then network fail
				}
				else
				{
					//just send kl to both x and y
					sendWhistleMsgXYL("kl;\r", 4);
				}
#endif
				//Esure Jog is clear
				g_cJogX = JOGSTOP; //state is now stopped
				g_cJogY = JOGSTOP;
				g_cJogGoalX = JOGSTOP;
				g_cJogGoalY = JOGSTOP;
				//Homing
#ifdef HOMESYSTEM_X
				if (cWhistleAxis == X_AXIS && g_cHomedX == HOME_RUNNING)
				{
					g_cHomedX = HOME_FAILURE;
				}
#endif
#ifdef HOMESYSTEM_Y
				if (cWhistleAxis == Y_AXIS && g_cHomedY == HOME_RUNNING)
				{
					g_cHomedY = HOME_FAILURE;
				}
#endif
				#ifdef WHISTLE_CONTINUOUS
				//Does not run jog monitor each time but has a permanent monitory
				#else
				g_cJogRunXFlag = 0;
				g_cJogRunYFlag = 0;
				#endif
				//Motion
				g_cMotorAction = 0;
				break;
			case AutoER:
				//Always print out this state
				logf("@ER\r\n");
				//ProfilePoint("HIT");
				//PrintProfilePoints();
				//ClearProfiling(); //erase buffer.. start again

				sendWhistleUDPMsgWL(p_WhistleUDP, "lmf;\r", 5);
				sendWhistleUDPMsgWL(p_WhistleUDP, "mf;\r", 4);

				//For Position, Grav Comp, and anything else
				//you want stop and turn the motor on in AUTOER.
				//In the message handling, you need to stop the other axis.
				//For Home Probe you want to stop and turn the motor on in AUTOER, but
				//you do not want to stop the other axis, and you don't really want the message here.
				//In order to make this happen Home Probe will bypass code here

#ifdef GENRIVET
				if (g_cMCRunningMsgGoal!=ProbeHomeOK)
				{
#endif

				//issue warning and set MCObstructionEvent, if not in ESTOP.
				if (g_cModeState != MODE_ESTOP)
				{
					SmartToolMsgMiniFtMessageCode(0, MINIFT_OID_MCERR, MINIFTMC_UNABLE_TO_MOVE);
					g_cMCObstructionEvent |= cWhistleAxis;
				}
				//stop both x and y
				MCStopAndFailMove();

#ifdef GENRIVET
			}
			else
			{
				logf("PrbHm...\r\n");
			}
#endif

				//New Home System
				//Homing
#ifdef HOMESYSTEM_X
				if (cWhistleAxis == X_AXIS && g_cHomedX == HOME_RUNNING)
				{
					logf("%s MFHM\r\n", p_WhistleUDP->name);
					g_cHomedX = HOME_FAILURE;
				}
#endif
#ifdef HOMESYSTEM_Y
				if (cWhistleAxis == Y_AXIS && g_cHomedY == HOME_RUNNING)
				{
					logf("%s MFHM %c\r\n", p_WhistleUDP->name);
					g_cHomedY = HOME_FAILURE;
				}
#endif
				#ifdef WHISTLE_CONTINUOUS
				//Does not run jog monitor each time but has a permanent monitory
				#else
				g_cJogRunXFlag = 0;
				g_cJogRunYFlag = 0;
				#endif
				//Motion
				g_cMotorAction = 0;
				break;
				//ALWAYS INCLUDE THIS: Some settings even on a machine that does not have Force Limiting On could cause this error.
				//#ifdef FORCE_LIMIT_WHISTLE_CODE
			case ForceLimitCurrent:
				//Always print out this state
				logf("@FLC\r\n");

				//For Position, Grav Comp, and anything else
				//you want stop and turn the motor on in AUTOER.
				//In the message handling, you need to stop the other axis.

				//issue warning and set MCObstructionEvent, if not in ESTOP.
				if (g_cModeState != MODE_ESTOP)
				{
					SmartToolMsgMiniFtMessageCode(0, MINIFT_OID_MCERR, MINIFTMC_UNABLE_TO_MOVE);
					g_cMCObstructionEvent |= cWhistleAxis;
				}
				//stop both x and y
				MCStopAndFailMove();

				//When Running A Position Movement, this has a clear effect.
				//When Waiting, this also will have an effect of stopping on position.
				//must now set to ERROR
				g_cMoveDone = MOVEDONE_ERROR;  // send error back to MODE_POSN state machine
				//Since this is an error... always throw brake on
				BrakeOn();
				//clear flag
				g_cMCRunningMsgGoal = NoMessage;
				g_uiMoveNum = 0;
				//beep
				BeepMCERR()
				;
				if (g_cModeState != MODE_ESTOP)
				{
					//stop both x and y
					RunServoHereNowW();
				}
				else
				{
					//just send kl to both x and y
					sendWhistleMsgXYL("kl;\r", 4);
				}
				//Esure Jog is clear
				g_cJogX = JOGSTOP; //state is now stopped
				g_cJogY = JOGSTOP;
				g_cJogGoalX = JOGSTOP;
				g_cJogGoalY = JOGSTOP;
				//Homing
#ifdef HOMESYSTEM_X
				if (cWhistleAxis == X_AXIS && g_cHomedX == HOME_RUNNING)
				{
					g_cHomedX = HOME_FAILURE;
				}
#endif
#ifdef HOMESYSTEM_Y
				if (cWhistleAxis == Y_AXIS && g_cHomedY == HOME_RUNNING)
				{
					g_cHomedY = HOME_FAILURE;
				}
#endif
				#ifdef WHISTLE_CONTINUOUS
				//Does not run jog monitor each time but has a permanent monitory
				#else
				g_cJogRunXFlag = 0;
				g_cJogRunYFlag = 0;
				#endif
				//Motion
				g_cMotorAction = 0;
				break;
				//#endif
			default:
				logf("Msg?=%d\r\n", i);
				break;
			}
			if (i > 0 && cNoClear == 0) //ALWAYS CLEAR THE CODE, unless it's an intermediate code. && g_cMCRunningMsgGoal==NoMessage)
			{
				//Safe to clear the code because there should be no additional operation results waiting
				sendWhistleUDPMsgWL(p_WhistleUDP, "i=0;\r", 5);
				sendWhistleUDPMsgWL(p_WhistleUDP, "ob[1]=0;\r", 9);
			}
		}
		else if (memcmp(p_cParse, "i=", 2) == 0)
		{
			goto case_ignore_quiet;
			//ignore
		}
		else
		{
			goto case_unmatched_whistle_messages;
		}
		//End Letter 'i'
	}
	else if (c == 'p')
	{
		if (memcmp(p_cParse, "pxt=", 4) == 0)
		{
			p_cParse += 4;
			i = atoi(p_cParse);
			if (cWhistleAxis == X_AXIS)
			{
				g_iPXTPendingX = i;
			}
			else
			{
				g_iPXTPendingY = i;
			}
		}
		else if (memcmp(p_cParse, PX ";", 3) == 0)
		{
			// The position string
			//logf("    OK %s " PX ": \"%s\"\r\n", p_WhistleUDP->name, p_cRxBuff);
			p_cParse += 3;
			lTemp = atol(p_cParse);  // an "int" on the Whistle is a 32-bit signed integer (long)
			if (cWhistleAxis == X_AXIS)
			{
				fx = ((float) lTemp) / ((float) g_ConfigData.EncoderRatio.fX);
			}
			else
			{
				fy = ((float) lTemp) / ((float) g_ConfigData.EncoderRatio.fY);
			}

			//
			if (cWhistleAxis == X_AXIS)
			{
				pxt = g_iPXTPendingX;
				g_iPXTPendingX = -1; //clear this
			}
			else
			{
				pxt = g_iPXTPendingY;
				g_iPXTPendingY = -1; //clear this
			}
			if (pxt == -1)
			{
				//look like this was not requested with pxt first
				pxt = 1;
			}
			if (pxt == 1)
			{
				//GetPosition
				g_PosnMode.cFreshCurPosn |= cWhistleAxis;
				if (cWhistleAxis == X_AXIS)
				{
					g_PosnMode.fLastKnownPosnX = fx;
					g_PosnMode.fPosnX = fx;
//logf("px was %f\r\n",fx);
				}
				else
				{
					g_PosnMode.fLastKnownPosnY = fy;
					g_PosnMode.fPosnY = fy;
//logf("px was %f\r\n",fy);
				}
				//When this is complete it will cause a message to be generated below.
				if (g_PosnMode.cFreshCurPosn == X_AND_Y)
				{
					//for testing only
					g_PosnMode.cFreshCurPosnSend = 1;
				}
				else
				{
					g_PosnMode.cFreshCurPosnSend = 0; //don't send partial...
				}
			}
			else if (pxt == 2)
			{
				//ProbeTeachPosition
				g_Probe.cGotTeachCoords |= cWhistleAxis;
				if (cWhistleAxis == X_AXIS)
				{
					//so copy last value
					if (fx == 0.0)
					{
						fx = 0.00001;
					} //FIXME Minor Not sure why we avoid 0,0
					g_TeachMachinePosn.fX = fx;
				}
				else
				{
					//so copy last value
					if (fy == 0.0)
					{
						fy = 0.00001;
					} //FIXME Minor Not sure why we avoid 0,0
					g_TeachMachinePosn.fY = fy;
				}
				if (g_Probe.cGotTeachCoords == X_AND_Y)
				{
					logf("got Teach: x=%f,y=%f\r\n", g_TeachMachinePosn.fX, g_TeachMachinePosn.fY); //debug
				}
			}
			else if (pxt == 3)
			{
				//ProbeK
				g_cProbeFlag |= cWhistleAxis;
				if (cWhistleAxis == X_AXIS)
				{
					//so copy last value
					g_fProbeX = fx - g_MachineOffset.fX;
				}
				else
				{
					//so copy last value
					g_fProbeY = fy - g_MachineOffset.fY;
				}
				if (g_cProbeFlag == X_AND_Y)
				{
					logf("got K: x=%f,y=%f\r\n", g_fProbeX, g_fProbeY); //debug
				}
			}
			else if (pxt == 5)
			{
				//ProbeAdjust
				g_Probe.cGotProbeAdjust |= cWhistleAxis;
				if (cWhistleAxis == X_AXIS)
				{
					//so copy last value
					g_Probe.fProbeAdjustX = fx; //do not use machine offset, which is applied during probe, but then after part of rotation.
				}
				else
				{
					//so copy last value
					g_Probe.fProbeAdjustY = fy; //do not use machine offset, which is applied during probe, but then after part of rotation.
				}
				if (g_Probe.cGotProbeAdjust == X_AND_Y)
				{
					logf("got ProbeAdjust: x=%f,y=%f\r\n", g_Probe.fProbeAdjustX, g_Probe.fProbeAdjustY); //debug
				}
			}
			else if (pxt == 6)
			{
				//ProbeYHomeCapture  Use the same probe home place holder as lap rivet, but
				// for circ rivet this is a different processs... just capture posn and store it
				// in same probe home place.
				g_Probe.cGotHome = 0;
				if (cWhistleAxis == Y_AXIS)
				{
					g_Probe.fHomeY = fy; //do not use machine offset
					g_Probe.fHomeX = 0.0; //because X has no home

					g_Probe.cGotHome = X_AND_Y;
					logf("got ProbeHome: y=%f\r\n", g_Probe.fHomeY); //debug
				}
			}
			else if (pxt == 7)
			{
				//GetPositionCounts... like GetPosition, but also sets the special FreshPosn Flag
				g_PosnMode.cFreshCurPosn |= cWhistleAxis;
				g_PosnMode.cFreshPosn = g_PosnMode.cFreshCurPosn; //set only by this path
				if (cWhistleAxis == X_AXIS)
				{
					g_PosnMode.fLastKnownPosnX = fx;
					g_PosnMode.fPosnX = fx;
				}
				else
				{
					g_PosnMode.fLastKnownPosnY = fy;
					g_PosnMode.fPosnY = fy;
				}
				//When this is complete it will cause a message to be generated below.
				if (g_PosnMode.cFreshCurPosn == X_AND_Y)
				{
					g_PosnMode.ulFreshPosnTime = MS_TIMER;
					g_PosnMode.cFreshCurPosnSend = 1;
				}
				else
				{
					g_PosnMode.cFreshCurPosnSend = 0; //don't send partial...
				}
			}
			else	//	Undefine PXT value.
			{
				logf("@pxt=%d not expctd\r\n", pxt);
				// FIXMENOWNOWNOWNOWNOW
			}
			//do nothing else here
		}
		else if (memcmp(p_cParse, "pgl=", 4) == 0)
		{
			p_cParse += 4;
			ui = (int) atol(p_cParse);
			if (cWhistleAxis == X_AXIS)
			{
				if (g_uiMoveNumEchoX != g_uiMoveNum)
				{
					lpgl = atol(p_cParse);
					if (lpgl != g_ipglx)
					{
						MCStopAndFailMove();
						sprintf(g_cTxBuffer2, "FAIL: .: echo lpgl=%d BUT g_ipgl=%d : %s\r\n", (int) lpgl, g_ipglx, p_cRxBuff);
						g_cTxBuffer2[6] = 'X';
						logf(g_cTxBuffer2);
						SmartToolMsgStr(0, STP_ALERT, MINIFT_OID_MCERR, g_cTxBuffer2);
					}
					g_ipglechox = lpgl;
				}
			}
			else
			{
				if (g_uiMoveNumEchoY != g_uiMoveNum)
				{
					lpgl = atol(p_cParse);
					if (lpgl != g_ipgly)
					{
						MCStopAndFailMove();
						sprintf(g_cTxBuffer2, "FAIL: .: echo lpgl=%d BUT g_ipgl=%d : %s\r\n", (int) lpgl, g_ipgly, p_cRxBuff);
						g_cTxBuffer2[6] = 'Y';
						logf(g_cTxBuffer2);
						SmartToolMsgStr(0, STP_ALERT, MINIFT_OID_MCERR, g_cTxBuffer2);
					}
					g_ipglechoy = lpgl;
				}
			}
		}
		else
		{
			goto case_unmatched_whistle_messages;
		}
		//End Letter 'p'
	}
	else if (memcmp(p_cParse, "mvn=", 4) == 0)
	{
		//Check Position Echo
		p_cParse += 4;
		ui = (int) atol(p_cParse);
		if (ui == 0)
		{
			//they set it to 0 ... don't overwrite the movenumecho though
		}
		else if (cWhistleAxis == X_AXIS)
		{
			g_uiMoveNumEchoX = ui; //The Echo is Talking about this move num
#ifdef OUTPUT_MVN_ECHO
			logf("mvn echo x = %u\r\n", ui);
#endif
		}
		else
		{
			g_uiMoveNumEchoY = ui; //The Echo is Talking about this move num
#ifdef OUTPUT_MVN_ECHO
			logf("mvn echo y = %u\r\n", ui);
#endif
		}
	}
	else if (memcmp(p_cParse, "ver;", 4) == 0)
	{
		logf("%s Ver: \"%s\"\r\n", p_WhistleUDP->name, p_cRxBuff);
	}
	else if (memcmp(p_cParse, "mf;", 3) == 0)
	{
		p_cParse += 3;
		goto parse_lmf_and_mf;
	}
	else if (memcmp(p_cParse, "lmf;", 4) == 0)
	{
		p_cParse += 4;
		parse_lmf_and_mf:
		l1 = atol(p_cParse);
		if (l1 != 0)
		{
			if (l1 & 0x1000)
			{
				//special motor category
				if (l1 & 0x2000)
				{
					//Motor Power Failure
#ifdef MC_ASSERTED_ESTOP
					//If estop mode is already present, but the logical signal is NOT present, then don't resignal this:
					//It might be that we are waiting to leave ESTOP, and this could be delayed info anyway.
					if (g_cModeState == MODE_ESTOP && g_cDigInEstopSignal != ESTOP_SIGNALED)
					{
						//do NOT signal this in this case
					}
					else
					{
						g_cEstopMCAsserted = 1;
					}
#endif
				}
			}
			//if (l1 & 0x10000)
			//{
			//	g_cNeedMotorCom = 1; //after estop it will com again...
			//	logf("FE\r\n");
			//}
			logf("@@@%s:lmf:%s\r\n", p_WhistleUDP->name, p_cParse);
			//FIXMENOWNOWNOWNOWNOWNOWNOWNOWNOW
			//consider brake if this is not something we have overcome...
			//if (cWhistleAxis==X_AXIS)
			//{
			//    sprintf(g_cTxBuffer2,"X:%s",p_cParse);
			//}
			//else
			//{
			//    sprintf(g_cTxBuffer2,"Y:%s",p_cParse);
			//}
			//SmartToolMsgStr(STP_ALERT, OID_MCERR, g_cTxBuffer2);
		}
	}
	else if (memcmp(p_cParse, "mo;", 2) == 0)
	{
		//	Whistle heartbeat echo received.
		//	FIXME: Debug the following new code:
		//	Check time since last heartbeat prompt was sent.
		if (p_WhistleUDP->cHBsent)
		{
			p_WhistleUDP->HBlastTime = (MS_TIMER - p_WhistleUDP->HBsentTime);
			if (p_WhistleUDP->HBlastTime>=4)
			{
				logf("whbt=%u\r\n", p_WhistleUDP->HBlastTime);
			}
			p_WhistleUDP->cHBsent = FALSE;	// Cancel WD response pending flag.
		}
	}
	else if (memcmp(p_cParse, "c=", 2) == 0)
	{
		goto case_ignore;
		//ignore
	}
	else if (memcmp(p_cParse, "st;", 3) == 0)
	{
		alternate_jog_path:
		if (cWhistleAxis == X_AXIS)
		{
			if (g_cJogX != JOGSTOP || g_cJogGoalX != JOGSTOP)
			{
				g_cJogX = JOGSTOP; //state is now stopped
				g_cJogGoalX = JOGSTOP;
				if (g_cJogY == JOGSTOP && g_cJogGoalY == JOGSTOP)
				{
					g_cMotorAction = 0;
				}
			}
		}
		else
		{
			if (g_cJogY != JOGSTOP || g_cJogGoalY != JOGSTOP)
			{
				g_cJogY = JOGSTOP; //state is now stopped
				g_cJogGoalY = JOGSTOP;
				if (g_cJogX == JOGSTOP && g_cJogGoalX == JOGSTOP)
				{
					g_cMotorAction = 0;
				}
			}
		}
		goto case_ignore;
		//ignore
	}
	else if (memcmp(p_cParse, "bg;", 3) == 0)
	{
		if (cWhistleAxis == X_AXIS)
		{
			if (g_uiLastJVZeroPacketX==g_uiPacketNumberX - 1)
			{
				logf("SAW JOG STOP X\r\n");
				//got an echo of jv=0 followed by this bg... this is a stop jog
				goto alternate_jog_path;
			}
		}
		else
		{
			if (g_uiLastJVZeroPacketY==g_uiPacketNumberY - 1)
			{
				logf("SAW JOG STOP Y\r\n");
				//got an echo of jv=0 followed by this bg... this is a stop jog
				goto alternate_jog_path;
			}
		}
		goto case_ignore;
	}
	else if (memcmp(p_cParse, "jv=", 3) == 0)
	{
		if (memcmp(p_cParse, "jv=0;", 5) == 0)
		{
			if (cWhistleAxis == X_AXIS)
			{
				g_uiLastJVZeroPacketX=g_uiPacketNumberX;
			}
			else
			{
				g_uiLastJVZeroPacketY=g_uiPacketNumberY;
			}
			goto case_ignore;
		}
	}
	#ifdef WHISTLE_CONTINUOUS
	//Does not run jog monitor each time but has a permanent monitor
	else if (memcmp(p_cParse, "kl;", 3) == 0)
	{
		//FIXME PORTHIGH do I need to watch for this as sense that thread is dead?
		if (cWhistleAxis == X_AXIS)
		{
		}
		else
		{
		}
		goto case_ignore;
		//ignore
	}
	#else
	else if (memcmp(p_cParse, "XQ##JOG", 7) == 0) //JOGM JOGMX etc...
	{
		if (cWhistleAxis == X_AXIS)
		{
			g_cJogRunXFlag = 1;
		}
		else
		{
			g_cJogRunYFlag = 1;
		}
		goto case_ignore;
	}
	else if (memcmp(p_cParse, "kl;", 3) == 0)
	{
		if (cWhistleAxis == X_AXIS)
		{
			g_cJogRunXFlag = 0;
		}
		else
		{
			g_cJogRunYFlag = 0;
		}
		goto case_ignore;
		//ignore
	}
	#endif
	else if (memcmp(p_cParse, "cmd=", 4) == 0)
	{
		//FIXME PORTWHISTLE
		//FIXEM PORTFATAL
		logf("PARSING: %s : \r\n", p_cParse );
		i=0;
		p_cParse += 4;
		char cDigit= *p_cParse++;
		if (cDigit>='0' && cDigit<='9') { i+=cDigit-'0'; }
		cDigit= *p_cParse++;
		if (cDigit>='0' && cDigit<='9') { i=i*10; i+=cDigit-'0'; }
		logf("i=%d \r\n", i );
		//FIXME PORTFATAL
	}
	else if (memcmp(p_cParse, "XQ##POSN", 8) == 0)
	{
		if (cWhistleAxis == X_AXIS)
		{
			if (g_ipglechox!=g_ipglx)
			{
				//x echo didn't happen.... can't trust that this move will be to the right location.
				MCStopAndFailMove();
				sprintf(g_cTxBuffer2, "FAIL: .: no echo of ipgl %d\r\n", g_ipglx);
				g_cTxBuffer2[6] = 'X';
				logf(g_cTxBuffer2);
				SmartToolMsgStr(0, STP_ALERT, MINIFT_OID_MCERR, g_cTxBuffer2);
			}
		}
		else
		{
			if (g_ipglechoy!=g_ipgly)
			{
				//y echo didn't happen.... can't trust that this move will be to the right location.
				MCStopAndFailMove();
				sprintf(g_cTxBuffer2, "FAIL: .: no echo of ipgl %d\r\n", g_ipgly);
				g_cTxBuffer2[6] = 'Y';
				logf(g_cTxBuffer2);
				SmartToolMsgStr(0, STP_ALERT, MINIFT_OID_MCERR, g_cTxBuffer2);
			}
		}
	}
	else if (memcmp(p_cParse, "sp=", 3) == 0)
	{
		goto case_ignore;
		//ignore
	}
	else if (memcmp(p_cParse, "gcp;", 4) == 0)
	{
		p_cParse += 4;
		fp = atof(p_cParse);
		logf("gcp parse to %f\r\n", fp);
		if (cWhistleAxis == X_AXIS)
		{
			g_GravCompResults.fxp = fp;
		}
		else
		{
			g_GravCompResults.fyp = fp;
		}
	}
	else if (memcmp(p_cParse, "gcn;", 4) == 0)
	{
		p_cParse += 4;
		fn = atof(p_cParse);
		logf("gcn parse to %f\r\n", fn);
		if (fabsinline(g_GravCompResults.fxp) >= fabsinline(fn))
		{
			i = 1;
		}
		else
		{
			i = -1;
		}
		if (cWhistleAxis == X_AXIS)
		{
			g_GravCompResults.fxn = fn;
			g_GravComp.iDirX = i;
			logf("     XWhistle GCD: %d\r\n", i);
		}
		else
		{
			g_GravCompResults.fyn = fn;
			g_GravComp.iDirY = i;
			logf("     YWhistle GCD: %d\r\n", i);
		}
		g_cGravCompDir |= cWhistleAxis;
		if (g_cGravCompDir == X_AND_Y)
		{
			SendActivePremove(0,STP_ALERT);
		}
	}
	else
	{
		case_unmatched_whistle_messages:
		//an unknown value
#ifdef OUTPUT_RX_SHOW_UNMATCHED
		logf("%s Unmatched: \"%s\"\r\n", p_WhistleUDP->name, p_cRxBuff);
#endif
		goto complete_parsing;
	}
	//FIXMENOWNOWNOWNOW improve on parsing numbers for speed and for security
	complete_parsing:
	return;
	case_ignore:
#ifdef OUTPUT_RX_SHOW_IGNORED
	logf("%s ignored: \"%s\"\r\n", p_WhistleUDP->name, p_cRxBuff);
#endif
	return;
	case_ignore_quiet:
#ifdef OUTPUT_RX_SHOW_IGNORED_VERBOSE
	logf("%s ignored: \"%s\"\r\n", p_WhistleUDP->name, p_cRxBuff);
#endif
	return;
}

void MCHBTimeout(td_WhistleUDP * p_WhistleUDP)
{
	if (g_cMotorAction != 0)
	{
		MCStopAndFailMove();
	}
	else
	{
		BrakeOn(); //FIXME0 do better and stop the entire system
	}
}

void MCSetConfigParams(void)
{
	//This method is only called during whistle init. //FIXME PORTLOW VERIFY this statement
	MCSetEncoderRatios(); //set any vars which need adjustment based on encoder ratios
	MCSetErrorLimits();
	MCSetCurrentLimits(g_ConfigData.MCCurrentLimit.fX, g_ConfigData.MCCurrentLimit.fY, g_ConfigData.MCPeakCurrentLimit.fX, g_ConfigData.MCPeakCurrentLimit.fY);
	MCSetPositionLimits(0); //Set only the variables, but not the limits yet, because it hasn't been homed yet.
	MCSetVelmonAndBoostParameters();
	MCSetSD();
	return;
}

void MCSetEncoderRatios()
{
//set any vars which need adjustment based on encoder ratios
}

void MCSetErrorLimits()
{
	//Set Machine Following Error limit directly
	//Whistle code will set the general following error for move failure detection using this OID.

	sprintf(g_cTxBuffer, "pem=%d;\r", XCOUNTS(g_ConfigData.PosnErrLimit.fX) );
	sendWhistleMsgX(g_cTxBuffer);
	sprintf(g_cTxBuffer, "pem=%d;\r", YCOUNTS(g_ConfigData.PosnErrLimit.fY) );
	sendWhistleMsgY(g_cTxBuffer);

	///????? should I do this or continue to use PEM?
	//sprintf(g_cTxBuffer, "ER[3]=%d;\r", XCOUNTS(g_ConfigData.PosnErrLimit.fX) );
	//sendWhistleMsgX(g_cTxBuffer);
	//sprintf(g_cTxBuffer, "ER[3]=%d;\r", YCOUNTS(g_ConfigData.PosnErrLimit.fY) );
	//sendWhistleMsgY(g_cTxBuffer);

#ifdef FORCE_LIMIT_WHISTLE_CODE
	//FIXME PORTFATAL  This code and its applicability would need to be ported
	//It has not been touched yet
	sprintf(g_cTxBuffer,"mxs=%d;\r", XCOUNTS(g_ConfigData.fMaxSpeedX));
	sendWhistleMsgX(g_cTxBuffer);

	sprintf(g_cTxBuffer,"mxs=%d;\r", YCOUNTS(g_ConfigData.fMaxSpeedY));
	sendWhistleMsgY(g_cTxBuffer);

	sendWhistleXMsg("LL[2]=-mxs;HL[2]=mxs;\rVL[2]=-mxs;VH[2]=mxs;\r", FINDLENGTH);
	sendWhistleYMsg("LL[2]=-mxs;HL[2]=mxs;\rVL[2]=-mxs;VH[2]=mxs;\r", FINDLENGTH);

	//FIXME PORTFATAL  VEL ERR MUST BE IN PLACE EITHER OLDER OR NEWER STYLE...
	//Does anything even have vef vet anymore?
	sprintf(g_cTxBuffer,"ER[2]=%d;\r", XCOUNTS(g_ConfigData.VelErrLimit.fVErrorX));
	sendWhistleMsgX(g_cTxBuffer);
	sprintf(g_cTxBuffer,"ER[2]=%d;\r", YCOUNTS(g_ConfigData.VelErrLimit.fVErrorY));
	sendWhistleMsgY(g_cTxBuffer);
#else
	//FIXME PORTFATAL  the older version of limiting is also not in use....
#if 0
	sprintf(g_cTxBuffer, "vef=%f;vet=%d;\r", g_ConfigData.VelErrLimit.fVLimitMarginX, (long) (1000 * g_ConfigData.VelErrLimit.fVErrorX));
	sendWhistleMsgX(g_cTxBuffer);
	sprintf(g_cTxBuffer, "vef=%f;vet=%d;\r", g_ConfigData.VelErrLimit.fVLimitMarginY, (long) (1000 * g_ConfigData.VelErrLimit.fVErrorY));
	sendWhistleMsgY(g_cTxBuffer);
#endif
#endif

	//FIXME PORTFATAL  Must Set Error Limits... Possibly Alter Parameters
	return;
}

void MCSetCurrentLimits(float fx, float fy, float fpx, float fpy)
{
	//units in amps
	logf("xa=%f ya=%f \r\n", fx, fy);

	sprintf(g_cTxBuffer, "CL[1]=%f;\r", fx);
	sendWhistleMsgX(g_cTxBuffer); //Sample of "Compact Style" whistle send chain
	sprintf(g_cTxBuffer, "CL[1]=%f;\r", fy);
	sendWhistleMsgY(g_cTxBuffer);
	sprintf(g_cTxBuffer, "PL[1]=%f;\r", fpx);
	sendWhistleMsgX(g_cTxBuffer);
	sprintf(g_cTxBuffer, "PL[1]=%f;\r", fpy);
	sendWhistleMsgY(g_cTxBuffer);

	//FIXME PORTFATAL FORCELIMITS
#ifdef FORCE_LIMIT_WHISTLE_CODE
	sendWhistleMsgXY("scl=CL[1];\r");
	sendWhistleMsgXY("spl=PL[1];\r");
#else
	//no force limits, but set these variables (assume they exist, but set then out of the way)
	sendWhistleMsgXY("scl=CL[1];\r");
	sendWhistleMsgXY("spl=PL[1];\r");
	sendWhistleMsgXY("iqml=-spl-1;\r");
	sendWhistleMsgXY("iqmh=spl+1;\r");
	sendWhistleMsgXY("impl=spl;\r");
	sendWhistleMsgXY("imcl=scl;\r");
#endif
	return;
}

void MCSetPositionLimits(byte bAllowLimitSetNow)
{
	//send position limits
	sprintf(g_cTxBuffer, "minp=%d;\r", XCOUNTS(g_fPositionMinX) );
	sendWhistleMsgX(g_cTxBuffer);
	sprintf(g_cTxBuffer, "minp=%d;\r", YCOUNTS(g_fPositionMinY) );
	sendWhistleMsgY(g_cTxBuffer);
	sprintf(g_cTxBuffer, "maxp=%d;\r", XCOUNTS(g_fPositionMaxX) );
	sendWhistleMsgX(g_cTxBuffer);
	sprintf(g_cTxBuffer, "maxp=%d;\r", YCOUNTS(g_fPositionMaxY) );
	sendWhistleMsgY(g_cTxBuffer);
	if (bAllowLimitSetNow == 1)
	{
		//do not set limits during this home operation, or if no home was done yet
		sprintf(g_cTxBuffer, "VL[3]=%d;\r", XCOUNTS(g_fPositionMinX) );
		sendWhistleMsgX(g_cTxBuffer);
		sprintf(g_cTxBuffer, "VL[3]=%d;\r", YCOUNTS(g_fPositionMinY) );
		sendWhistleMsgY(g_cTxBuffer);
		sprintf(g_cTxBuffer, "VH[3]=%d;\r", XCOUNTS(g_fPositionMaxX) );
		sendWhistleMsgX(g_cTxBuffer);
		sprintf(g_cTxBuffer, "VH[3]=%d;\r", YCOUNTS(g_fPositionMaxY) );
		sendWhistleMsgY(g_cTxBuffer);
	}
	return;
}

void MCSetVelmonAndBoostParameters(void)
{
	//FIXME PORTLOW  these parameters could be recycled as the lib is perfected
	//					g_ConfigData.fVelMonSpeed1,
	//					g_ConfigData.fVelMonSpeed2,
	//					g_ConfigData.fVelMonKDV1,
	//					g_ConfigData.fVelMonKDV2,
	//					g_ConfigData.fVelMonBoostFactor,
	//					g_ConfigData.fVelMonBoostThreshold);
	return;
}

void MCSetSD()
{
	//FIXME PORTLOW	 not on minift?
	//	sprintf(g_cTxBuffer, "sd=%d;\r", WCOUNTS(g_ConfigData.fMaxStopDec));
	//	sendWhistleMsgALL(g_cTxBuffer);
}

void MCClearMotionSet()
{
	g_cMotionSet = UnknownMotionSet; //Clear Motion Set
	return;
}

void MCRunRestart()
{
//FIXME Minor Only needed to support OID_RESET_MC
	return;
}

#if 0
void MCClearPositionLimits()
{
	sendWhistleMsgALL("VL[3]=-40000000;\r");
	sendWhistleMsgALL("VH[3]=40000000;\r");
}
#endif

//Common Internal State Changes Used in many places:
void RunServoHereNowW()
{
	#ifdef WHISTLE_CONTINUOUS
	usleep(1000);
	sendWhistleMsgXYL("er[3]=pem;\r", 11);
	usleep(1000);
	sendWhistleMsgXYL("mo=1;\r", 6);
	usleep(1000);
	sendWhistleMsgXYL("st;\r", 4);
	usleep(1000);
	sprintf(g_cTxBuffer, "cmd=%d;\r", ServoHereNow); sendWhistleMsgXY(g_cTxBuffer);
	usleep(1000);
	#else
	usleep(1000);
	sendWhistleMsgXYL("er[3]=pem;\r", 11);
	usleep(1000);
	sendWhistleMsgXYL("kl;\r", 4);
	usleep(1000);
	sendWhistleMsgXYL("i=0;\r", 5);
	usleep(1000);
	sendWhistleMsgXYL("XQ##SHNW;\r", 10);
	#endif
}
void RunMotorOffNow()
{
	#ifdef WHISTLE_CONTINUOUS
	usleep(1000);
	sendWhistleMsgXYL("er[3]=pem;\r", 11);
	usleep(1000);
	sendWhistleMsgXYL("mo=0;\r", 6);
	usleep(1000);
	sendWhistleMsgXYL("st;\r", 4);
	usleep(1000);
	sprintf(g_cTxBuffer, "cmd=%d;\r", MotorOffNow); sendWhistleMsgXY(g_cTxBuffer);
	usleep(1000);
	#else
	usleep(1000);
	sendWhistleMsgXYL("er[3]=pem;\r", 11);
	usleep(1000);
	sendWhistleMsgXYL("kl;\r", 4);
	usleep(1000);
	sendWhistleMsgXYL("i=0;\r", 5);
	usleep(1000);
	sendWhistleMsgXYL("XQ##MON;\r",9); //Run Motor Off Now
	#endif
}
void RunMotorOffNowWYOnly()
{
	#ifdef WHISTLE_CONTINUOUS
	usleep(1000);
	sendWhistleMsgYL("er[3]=pem;\r", 11);
	usleep(1000);
	sendWhistleMsgYL("mo=0;\r", 6);
	usleep(1000);
	sendWhistleMsgYL("st;\r", 4);
	usleep(1000);
	sprintf(g_cTxBuffer, "cmd=%d;\r", MotorOffNow); sendWhistleMsgY(g_cTxBuffer);
	usleep(1000);
	#else
	usleep(1000);
	sendWhistleMsgYL("er[3]=pem;\r", 11);
	usleep(1000);
	sendWhistleMsgYL("kl;\r", 4);
	usleep(1000);
	sendWhistleMsgYL("i=0;\r", 5);
	usleep(1000);
	sendWhistleMsgYL("XQ##MON;\r",9); //Run Motor Off Now
	#endif
}

void RestartMainWhistleThread()
{
#ifdef WHISTLE_CONTINUOUS
	usleep(1000);
	sendWhistleMsgXYL("er[3]=pem;\r", 11);
	usleep(1000);
	sendWhistleMsgXYL("kl;\r", 4);
	usleep(1000);
	sendWhistleMsgXYL("i=0;\r", 5);
	usleep(1000);
	//asdfasdfasdf this is the temporary //FIXME PORTWHISTLE
	//FIXME PORTFATAL
	sendWhistleMsgXYL("XQ##SHNW;\r", 10);
	usleep(1000);
	sprintf(g_cTxBuffer, "cmd=%d;\r", ServoHereNow); sendWhistleMsgXY(g_cTxBuffer);
	usleep(1000);
#else
#endif
}


void MCRunEstopWithPower()
{
	g_cMCRunningMsgGoal = NoMessage;
	g_cGravCompProgress = 0;
	g_cPositionProgress = 0;
	g_cProbeHomeProgress = 0;
	g_cMotionSet = UnknownMotionSet; //Clear Motion Set
	RunServoHereNowW();
	return;
}

void MCRunEstopEngage()
{
	g_cMCRunningMsgGoal = NoMessage;
	g_cGravCompProgress = 0;
	g_cPositionProgress = 0;
	g_cProbeHomeProgress = 0;
	g_cMotionSet = UnknownMotionSet; //Clear Motion Set
	RunMotorOffNow();
	return;
//FIXME PORTMED  idea from crawler is good for SD set???
//	//takes effect after next estop
//	MCSetSD();
}

void MCRunEstopDisengage()
{
	//FIXME PORTMED	MC Assert ESTOP on Gold Question
	//This line from crawler... is it good for MiniFT to clear that estop flag here, or is it cleared???
//	g_cEstopMCAsserted = 0; //Clear this condition

	//The Brake should remain on until action:
	BrakeOn(); //FIXME PORTHIGH test this paradigm
	g_cMCRunningMsgGoal = NoMessage;
	sendWhistleMsgXY(PXXUM);
	RunServoHereNowW();
	//And for Gold... Start the Main Thread Up again. (for Gold series, the Run Servo Here Now W does not start a thread.
	#ifdef WHISTLE_CONTINUOUS
	RestartMainWhistleThread();
	#endif
	return;
	//NOTE: for GENCIRCMFT2, there was an option GENCIRCMFTX_RUN_XCM
	//which ran XCM instead of SHNW to do a special commutation.
	//This Option has been remove from this code version.  It would have to be added
	//again a drive in the future needed a special manual commutation routine.
	//this also made use of the byte g_cNeedMotorCom = 1; variable which was set to 1 when
	//the error handlers detected faults related to commutation
	//The code to run XCM was removed from this function except for this message
}

void RunGravComp()
{
	char * p_cCmd;

	g_cMCRunningMsgGoal = GravCompOK;
	g_cGravCompProgress = 0;

	//GravComp is a motor action
	g_cMotorAction = X_AND_Y;

	//send grav comp parameters
	sprintf(g_cTxBuffer, "gctf=%.3f;\r", g_ConfigData.GravCompTriggerFactor.fX);
	sendWhistleMsgX(g_cTxBuffer);
	sprintf(g_cTxBuffer, "gctf=%.3f;\r", g_ConfigData.GravCompTriggerFactor.fY);
	sendWhistleMsgY(g_cTxBuffer);
	sprintf(g_cTxBuffer, "gcm=%d;\r", XCOUNTS(g_ConfigData.GravCompMoveDist.fX) );
	sendWhistleMsgX(g_cTxBuffer);
	sprintf(g_cTxBuffer, "gcm=%d;\r", YCOUNTS(g_ConfigData.GravCompMoveDist.fY) );
	sendWhistleMsgY(g_cTxBuffer);

	//Several Grav Comp Parameters are currently not used at lower level, or not used at all

	//send down gravcomp acc/dec parameters

	sprintf(g_cTxBuffer, "sp=%d;\r", XCOUNTS(g_ConfigData.GravCompSpeed.fX) );
	sendWhistleMsgX(g_cTxBuffer);
	sprintf(g_cTxBuffer, "ac=%d;\r", XCOUNTS(g_ConfigData.GravCompAcc.fX) );
	sendWhistleMsgX(g_cTxBuffer);
	sprintf(g_cTxBuffer, "dc=%d;\r", XCOUNTS(g_ConfigData.GravCompDec.fX) );
	sendWhistleMsgX(g_cTxBuffer);

	sprintf(g_cTxBuffer, "sp=%d;\r", YCOUNTS(g_ConfigData.GravCompSpeed.fY) );
	sendWhistleMsgY(g_cTxBuffer);
	sprintf(g_cTxBuffer, "ac=%d;\r", YCOUNTS(g_ConfigData.GravCompAcc.fY) );
	sendWhistleMsgY(g_cTxBuffer);
	sprintf(g_cTxBuffer, "dc=%d;\r", YCOUNTS(g_ConfigData.GravCompDec.fY) );
	sendWhistleMsgY(g_cTxBuffer);

	g_cMotionSet = GravMotionSet; //speed and accel settings from GravComp

	sendWhistleMsgXYL("er[3]=pem;\r", 11);

#ifdef WHISTLE_CONTINUOUS
	//FIXME PORTLOW no grav comp support
#else
	p_cCmd = "XQ##RGC;\r";

	if (g_ConfigData.cGravCompAxes == 0)
	{
		sendWhistleMsgXYL("kl;\r", 4);
		sendWhistleMsgXYL(p_cCmd, 9);
	}
	else if (g_ConfigData.cGravCompAxes == 1)
	{
		sendWhistleMsgXL("kl;\r", 4);
		sendWhistleMsgXL(p_cCmd, 9);
		//Leave Y alone
	}
	else if (g_ConfigData.cGravCompAxes == 2)
	{
		sendWhistleMsgYL("kl;\r", 4);
		sendWhistleMsgYL(p_cCmd, 9);
		//Leave X alone
	}
	//Turn Brake Off
	BrakeOff(); //In the case of a failure, the brake should be turned back on.
#endif
	return;
}

void RunFloat()
{
	g_cMCRunningMsgGoal = NoMessage;
	g_cGravCompProgress = 0;

	//Float is a motor action
	g_cMotorAction = X_AND_Y;
	SetForceLimitingCurrents();

#ifdef WHISTLE_CONTINUOUS
	//FIXME PORTLOW no grav comp support
#else
	sendWhistleMsgXYL("er[3]=pem;\r", 11);
	sendWhistleMsgXYL("kl;\r", 4);
	sendWhistleMsgXYL("XQ##FLT;\r", 9);
#endif
	//Turn Brake Off
	BrakeOff(); //In the case of a failure, the brake should be turned back on.

	//Assume Success
	g_cFloatStatus = FLOATSTAT_FLOAT;
	SmartToolMsg(0, STP_ALERT, MINIFT_OID_FLOAT_STATUS, 1, (char *)&g_cFloatStatus);
	if (g_cModeState == MODE_POSN)
	{     // beep while floating, but only in position mode
		BeepFloat();
	}

	//FIXMENOWNOWNOW Is This Good Enough, or do we need a motor check? (or set a max velocity??)
	return;
}

void RunUnfloat()
{
	g_cMCRunningMsgGoal = NoMessage;
	g_cGravCompProgress = 0;

#ifdef WHISTLE_CONTINUOUS
	//FIXME PORTLOW no grav comp support
#else
	sendWhistleMsgXYL("er[3]=pem;\r", 11);
	sendWhistleMsgXYL("kl;\r", 4);
	sendWhistleMsgXYL("XQ##UFLT;\r", 10);
#endif

	//There is no reason to turn the brake off in this case.
	//It can remain on until a move or other status happens
	//OLD	//Turn Brake Off
	//OLD	BrakeOff(); //In the case of a failure, the brake should be turned back on.

	//Assume Success
	g_cFloatStatus = FLOATSTAT_NOFLOAT;
	SmartToolMsg(0, STP_ALERT, MINIFT_OID_FLOAT_STATUS, 1, (char *)&g_cFloatStatus);
	// beeper beeps while floating, during MODE_POSN only, but let's turn it off always when we come out of float
	BeepOff()
	;
	//FIXMENOWNOWNOW Is This Good Enough, or do we need a motor check? (or set a max velocity??)
	//Despite Brake Off, this qualifies as no action
	g_cMotorAction = 0;
	return;
}

void RunLoadYRail(byte c)
{
	g_cMCRunningMsgGoal = NoMessage;
	//Qualifies as a motor Action
	g_cMotorAction = Y_AXIS;
	RunServoHereNowW();
	if (c == 1)
	{
		//FIXME PORTMEDHIGH  great idea:  have MsgXL mode that checks to see that message are really want they say they are...
		// run that for a test...
		RunMotorOffNowWYOnly();
	}
	//Nothing to Check... just assume success... no state knowledge used as currently designed
	return;
}

//Former ZeroPosition() Should Not Be Used

#if 0
//Set Absolute position.
//Doesn't even include machine offsets.
//Designed for external input absolute system position.
void SetPosition_PENDING(float fx, float fy)
{
	float fYdelta;
	g_cMCRunningMsgGoal = NoMessage;

	BrakeOn(); //Due to potential issue after probe. Use Brake to guarantee hold

	//FIXME PORTMED  This was only used on once place, and I'm seeing if that is every used
	sendWhistleXYLMsg("er[3]=pem;\r", 11);

	//send down X-axis position
	sprintf(g_cTxBuffer, "kl;" PX "=%d;mo=1;st;\r" PX ";pxt=1;\r", XCOUNTS(fx) );
	sendWhistleMsgX(g_cTxBuffer);

	//send down Y-axis position
	sprintf(g_cTxBuffer, "kl;" PX "=%d;mo=1;st;\r" PX ";pxt=1;\r", YCOUNTS(fy) );
	sendWhistleMsgY(g_cTxBuffer);

	fYdelta = fy - g_PosnMode.fLastKnownPosnY;
	//This is a temporary solution to preserving MachineBaseExtension
	g_fMachineBaseExtension += fYdelta;
#ifdef PROBE_HOME_Y_HOME_SENSOR
	//For Circ Rivet Also preserve Y Home Position across
	logf("HomeY Before: %f\r\n",g_Probe.fProbeHomeY);
	g_Probe.fProbeHomeY += fYdelta;
	logf("HomeY After: %f\r\n",g_Probe.fProbeHomeY);
#endif
	//stopped after this
	g_cMotorAction = 0;
	return;
}
#endif

void MCGetPosition()
{
	//don't clear running code because this could happen during another operation
	g_PosnMode.cFreshCurPosn = 0; //Rabbit main system needs us to clear this inside here
	g_PosnMode.cFreshPosn = 0;

	g_WhistleUDPX.cCommDisplayRequired = COMM_DISPLAY_TX_PX;
	g_WhistleUDPY.cCommDisplayRequired = COMM_DISPLAY_TX_PX;
	//Now Send this PX query
	sendWhistleMsgXYL(PX ";\r", 4);
	//PX Must always be 2 characters, but the only 2 things it could be would be "px" or "py"
	g_WhistleUDPX.cCommDisplayRequired = COMM_DISPLAY_TX;
	g_WhistleUDPY.cCommDisplayRequired = COMM_DISPLAY_TX;
	return;
}
void MCGetPositionCounts()
{
	//don't clear running code because this could happen during another operation
	g_PosnMode.cFreshCurPosn = 0; //Rabbit main system needs us to clear this inside here
	g_PosnMode.cFreshPosn = 0; //this only gets set by GetPositionCounts
	usleep(4000); //network clog prevention
	sendWhistleMsgXYL("pxt=7;\r", 7); //set the fresh posn flag as well for this type of probe
	usleep(2000); //network clog prevention
	sendWhistleMsgXYL(PX ";\r", 4);
	usleep(2000); //network clog prevention
	return;
//FIXME PORTLOW remove pxts that are not used????
}

void RunProbeK()
{
	g_cMCRunningMsgGoal = NoMessage;
	usleep(4000); //network clog prevention
	sendWhistleMsgXYL("pxt=3;\r", 7);
	usleep(2000); //network clog prevention
	sendWhistleMsgXYL(PX ";\r", 4);
	usleep(2000); //network clog prevention
	//FIXME PORTLOW ensure these are properly operational with test
	return;
}

void RunProbeAdjust()
{
	g_cMCRunningMsgGoal = NoMessage;
	sendWhistleMsgXYL("pxt=5;\r", 7);
	sendWhistleMsgXYL(PX ";\r", 4);
	return;
}

void ProbeTeachPosition()
{
	g_cMCRunningMsgGoal = NoMessage;
	sendWhistleMsgXYL("pxt=2;\r", 7);
	sendWhistleMsgXYL(PX ";\r", 4);
	return;
}

void RunJogX(byte cx, float fx)
{
	float f;
	if (cx == JOGSTOP)
	{
		return; //no point in calling this function in this case
	}
	//Jog Motion
	g_cMCRunningMsgGoal = NoMessage;
	g_cMotorAction |= X_AXIS;

	//send jog parameters
	//send down jog acc/dec parameters
	if (g_cMotionSet != JogMotionSet)
	{
		//ensure both will write next time they are used even if only one will be used now
		g_fLastJogFactorX = 1.01;
		g_fLastJogFactorY = 1.01;
	}

	SetForceLimitingCurrentsX();

	f = fabsinline(fx);
	if (g_fLastJogFactorX != fx)
	{
		//Since jog is often called in repeat, only set this if it was not set.
		//update both, if either is updated
		sprintf(g_cTxBuffer, "sp=%d;\r", XCOUNTS(f*g_ConfigData.JogSpeed.fX) );
		sendWhistleMsgX(g_cTxBuffer);
		usleep(600); //network clog prevention
		sprintf(g_cTxBuffer, "ac=%d;\r", XCOUNTS(f*g_ConfigData.JogAcc.fX) );
		sendWhistleMsgX(g_cTxBuffer);
		usleep(600); //network clog prevention
		sprintf(g_cTxBuffer, "dc=%d;\r", XCOUNTS(g_ConfigData.JogDec.fX) );
		sendWhistleMsgX(g_cTxBuffer); //use full decel
		g_cMotionSet = JogMotionSet; //speed and accel settings from Jog
		g_fLastJogFactorX = fx;
		usleep(600); //network clog prevention
	}
	//USE NEW JOGM
	if (cx == JOGNEG)
	{
		//flip f
		f = -f;
	}
	#ifdef WHISTLE_CONTINUOUS
	//Does not run jog monitor each time but has a permanent monitory
	#else
	//Jogm was run when checking motion sets....
	if (g_cJogRunXFlag != 1) //set by command echo and kill...
	{
		//must start the jog monitor
		usleep(300); //network clog prevention
		sendWhistleMsgXL("er[3]=pem;\r", 11);
		usleep(300); //network clog prevention
		sendWhistleMsgXL("kl;\r", 4);
		usleep(300); //network clog prevention
		sendWhistleMsgX(PXXUM);
		usleep(300); //network clog prevention
		sendWhistleMsgXL("mo=1;\r", 6);
		usleep(2000); //network clog prevention
		sendWhistleMsgXL("XQ##JOGM;\r", 10);
		usleep(300); //network clog prevention

		logf("%c Start JogM\r\n", 'X');
	}
	usleep(1000); //network clog prevention
	#endif
	//run jog command
	sprintf(g_cTxBuffer, "jv=%d;\r", XCOUNTS(f*g_ConfigData.JogSpeed.fX) );
	sendWhistleMsgX(g_cTxBuffer);
	usleep(500); //network clog prevention
	sendWhistleMsgXL("bg;\r", 4);

	//Turn Brake Off
	BrakeOff(); //In the case of a failure, the brake should be turned back on.
	return;
}

void RunJogY(byte cy, float fy)
{
	float f;
	if (cy == JOGSTOP)
	{
		return; //no point in calling this function in this case
	}
	//Jog Motion
	g_cMCRunningMsgGoal = NoMessage;
	g_cMotorAction |= Y_AXIS;

	//send jog parameters
	//send down jog acc/dec parameters
	if (g_cMotionSet != JogMotionSet)
	{
		//ensure both will write next time they are used even if only one will be used now
		g_fLastJogFactorX = 1.01;
		g_fLastJogFactorY = 1.01;
	}

	SetForceLimitingCurrentsY();

	f = fabsinline(fy);
	if (g_fLastJogFactorY != fy)
	{
		//Since jog is often called in repeat, only set this if it was not set.
		//update both, if either is updated
		sprintf(g_cTxBuffer, "sp=%d;\r", YCOUNTS(f*g_ConfigData.JogSpeed.fY) );
		sendWhistleMsgY(g_cTxBuffer);
		usleep(600); //network clog prevention
		sprintf(g_cTxBuffer, "ac=%d;\r", YCOUNTS(f*g_ConfigData.JogAcc.fY) );
		sendWhistleMsgY(g_cTxBuffer);
		usleep(600); //network clog prevention
		sprintf(g_cTxBuffer, "dc=%d;\r", YCOUNTS(g_ConfigData.JogDec.fY) );
		sendWhistleMsgY(g_cTxBuffer); //use full decel
		g_cMotionSet = JogMotionSet; //speed and accel settings from Jog
		g_fLastJogFactorY = fy;
		usleep(600); //network clog prevention
	}
	//USE NEW JOGM
	if (cy == JOGNEG)
	{
		//flip f
		f = -f;
	}
	#ifdef WHISTLE_CONTINUOUS
	//Does not run jog monitor each time but has a permanent monitory
	#else
	//Jogm was run when checking motion sets....
	if (g_cJogRunYFlag != 1) //set by command echo and kill...
	{
		//must start the jog monitor
		usleep(300); //network clog prevention
		sendWhistleMsgYL("er[3]=pem;\r", 11);
		usleep(300); //network clog prevention
		sendWhistleMsgYL("kl;\r", 4);
		usleep(300); //network clog prevention
		sendWhistleMsgY(PXXUM);
		usleep(300); //network clog prevention
		sendWhistleMsgYL("mo=1;\r", 6);
		usleep(2000); //network clog prevention
		sendWhistleMsgYL("XQ##JOGM;\r", 10);
		usleep(300); //network clog prevention

		logf("%c Start JogM\r\n", 'Y');
	}
	usleep(1000); //network clog prevention
	#endif
	//run jog command
	sprintf(g_cTxBuffer, "jv=%d;\r", YCOUNTS(f*g_ConfigData.JogSpeed.fY) );
	sendWhistleMsgY(g_cTxBuffer);
	usleep(500); //network clog prevention
	sendWhistleMsgYL("bg;\r", 4);

	//Turn Brake Off
	BrakeOff(); //In the case of a failure, the brake should be turned back on.
	return;
}

void StopJogX()
{
	g_cMCRunningMsgGoal = NoMessage;
	//USE NEW JOG CODE
	//NO CANT DO THIS BECAUSE OF CURRENT FLAG USE	g_cMCRunningMsgGoal=NoMessage;
	sendWhistleMsgXL("jv=0;\r", 6);
	sendWhistleMsgXL("bg;\r", 4);
	usleep(2000); //network clog prevention
	sendWhistleMsgXL("jv=0;\r", 6);
	usleep(1000); //network clog prevention
	sendWhistleMsgXL("bg;\r", 4);
	g_cMotorAction &= (0xFF - X_AXIS);
	return;
}

void StopJogY()
{
	g_cMCRunningMsgGoal = NoMessage;
	//USE NEW JOG CODE
	//NO CANT DO THIS BECAUSE OF CURRENT FLAG USE	g_cMCRunningMsgGoal=NoMessage;
	sendWhistleMsgYL("jv=0;\r", 6);
	sendWhistleMsgYL("bg;\r", 4);
	usleep(2000); //network clog prevention
	sendWhistleMsgYL("jv=0;\r", 6);
	usleep(1000); //network clog prevention
	sendWhistleMsgYL("bg;\r", 4);
	g_cMotorAction &= (0xFF - Y_AXIS);
	return;
}

void RunHomeX()
{
#ifdef HOMESYSTEM_X_MC
	char * p_cCmd;
	g_cMCRunningMsgGoal=HomeOK;
	//Motion
	g_cMotorAction |= X_AXIS;

	SetForceLimitingCurrentsX();

	//send parameters for the purpose of home
	usleep(500);
	sprintf(g_cTxBuffer,"xsp=%d;\r", XCOUNTS(g_ConfigData.HomeSpeed.fX) ); sendWhistleMsgX(g_cTxBuffer);
	usleep(500);
	sprintf(g_cTxBuffer,"xfsp=%d;\r", XCOUNTS(g_ConfigData.HomeFineSpeed.fX) ); sendWhistleMsgX(g_cTxBuffer);
	usleep(500);
	sprintf(g_cTxBuffer,"ac=%d;\r", XCOUNTS(g_ConfigData.HomeAcc.fX) ); sendWhistleMsgX(g_cTxBuffer);
	usleep(500);
	sprintf(g_cTxBuffer,"dc=%d;\r", XCOUNTS(g_ConfigData.HomeDec.fX) ); sendWhistleMsgX(g_cTxBuffer);
	usleep(500);
	sprintf(g_cTxBuffer,"hmp=%d;\r", XCOUNTS(g_fHomePositionX) ); sendWhistleMsgX(g_cTxBuffer);
	usleep(500);
	sprintf(g_cTxBuffer,"minp=%d;\r", XCOUNTS(g_fPositionMinX) ); sendWhistleMsgX(g_cTxBuffer);
	usleep(500);
	sprintf(g_cTxBuffer,"maxp=%d;\r", XCOUNTS(g_fPositionMaxX) ); sendWhistleMsgX(g_cTxBuffer);
	usleep(500);
	sprintf(g_cTxBuffer,"hprm=%d;\r", XCOUNTS(HOMESYSTEM_X_HPRM) ); sendWhistleMsgX(g_cTxBuffer);
	usleep(500);

#ifdef HOMESYSTEM_X_POS_PROC_SENSOR
	sendWhistleMsgX(HOMESYSTEM_X_POS_PROC_SENSOR);
	usleep(500);
#endif
#ifdef HOMESYSTEM_X_NEG_PROC_SENSOR
	sendWhistleMsgX(HOMESYSTEM_X_NEG_PROC_SENSOR);
	usleep(500);
#endif

#ifdef HOMESYSTEM_X_HIB
	sendWhistleMsgX(HOMESYSTEM_X_HIB);
	usleep(500);
#endif
#ifdef HOMESYSTEM_X_HIC
	sendWhistleMsgX(HOMESYSTEM_X_HIC);
	usleep(500);
#endif

	g_cMotionSet = HomeMotionSet; //speed and accel settings from HomeMotionSet

#ifdef WHISTLE_CONTINUOUS
	sprintf(g_cTxBuffer, "cmd=%d;\r", HomeRun); sendWhistleMsgX(g_cTxBuffer);
#else
	sendWhistleMsgXL("er[3]=pem;\r",11);
	usleep(500);
	sendWhistleMsgXL("kl;\r", 4);
	usleep(2000);
	sendWhistleMsgXL("XQ##HMIY;\r", 10);
#endif

	//Turn Brake Off
	BrakeOff();//In the case of a failure, the brake should be turned back on.
	return;
#endif
}

//FIXME PORTFATAL  Home should be explicitly tested with arguments tested.
void RunHomeY()
{
#ifdef HOMESYSTEM_Y_MC
	g_cMCRunningMsgGoal = HomeOK;
	//Motion
	g_cMotorAction |= Y_AXIS;

	SetForceLimitingCurrentsY();

	//send parameters for the purpose of home
	usleep(500);
	sprintf(g_cTxBuffer, "xsp=%d;\r", YCOUNTS(g_ConfigData.HomeSpeed.fY) );	sendWhistleMsgY(g_cTxBuffer);	usleep(2000); //network clog prevention
	usleep(500);
	sprintf(g_cTxBuffer, "xfsp=%d;\r", YCOUNTS(g_ConfigData.HomeFineSpeed.fY) ); sendWhistleMsgY(g_cTxBuffer); usleep(2000); //network clog prevention
	usleep(500);
	sprintf(g_cTxBuffer, "ac=%d;\r", YCOUNTS(g_ConfigData.HomeAcc.fY) ); sendWhistleMsgY(g_cTxBuffer); usleep(2000); //network clog prevention
	usleep(500);
	sprintf(g_cTxBuffer, "dc=%d;\r", YCOUNTS(g_ConfigData.HomeDec.fY) ); sendWhistleMsgY(g_cTxBuffer); usleep(2000); //network clog prevention
	usleep(500);
	sprintf(g_cTxBuffer, "hmp=%d;\r", YCOUNTS(g_fHomePositionY) ); sendWhistleMsgY(g_cTxBuffer); usleep(2000); //network clog prevention
	usleep(500);
	sprintf(g_cTxBuffer, "minp=%d;\r", YCOUNTS(g_fPositionMinY) ); sendWhistleMsgY(g_cTxBuffer); usleep(2000); //network clog prevention
	usleep(500);
	sprintf(g_cTxBuffer, "maxp=%d;\r", YCOUNTS(g_fPositionMaxY) ); sendWhistleMsgY(g_cTxBuffer); usleep(2000); //network clog prevention
	usleep(500);
	sprintf(g_cTxBuffer, "hprm=%d;\r", YCOUNTS(HOMESYSTEM_Y_HPRM) ); sendWhistleMsgY(g_cTxBuffer); usleep(2000); //network clog prevention
	usleep(500);

#ifdef HOMESYSTEM_Y_POS_PROC_SENSOR
	sendWhistleMsgY(HOMESYSTEM_Y_POS_PROC_SENSOR);
	usleep(500);
#endif
#ifdef HOMESYSTEM_Y_NEG_PROC_SENSOR
	sendWhistleMsgY(HOMESYSTEM_Y_NEG_PROC_SENSOR);
	usleep(500);
#endif

#ifdef HOMESYSTEM_Y_HIB
	sendWhistleMsgY(HOMESYSTEM_Y_HIB);
	usleep(500);
#endif
#ifdef HOMESYSTEM_Y_HIC
	sendWhistleMsgY(HOMESYSTEM_Y_HIC);
	usleep(500);
#endif

	g_cMotionSet = HomeMotionSet; //speed and accel settings from HomeMotionSet

#ifdef WHISTLE_CONTINUOUS
	sprintf(g_cTxBuffer, "cmd=%d;\r", HomeRun); sendWhistleMsgY(g_cTxBuffer);
#else
	sendWhistleMsgYL("er[3]=pem;\r", 11);
	usleep(500);
	sendWhistleMsgYL("kl;\r", 4);
	usleep(2000);
	sendWhistleMsgYL("XQ##HMIY;\r", 10);
#endif
	//Turn Brake Off
	BrakeOff(); //In the case of a failure, the brake should be turned back on.
	return;
#endif
}

void RunFindWindowX(float x)
{
#ifdef USE_FINDWINDOW
	//Use Standard Move Speeds and Acceleration
#ifdef OUTPUT_WHISTLE_RUNANDSTOPS
	logf("RunFW\r\n");
#endif
	g_cMoveDone = MOVEDONE_FALSE;  //reset flag that indicates when a move has completed
	//Uses these message codes:
	// FindWinRun
	// FindWinOK
	// FindWinFail
	g_cMCRunningMsgGoal=FindWinOK;
	//Motion
	g_cMotorAction |= X_AXIS;

	g_uiMoveNum=g_uiMoveNumSeq++;//move it up
	if (g_uiMoveNumSeq==0)//wrap arround to 0 so skip to 1
	{
		g_uiMoveNumSeq=1; //reset to 1
	}
	g_cPositionProgress = 0;
	//Clear this before move
	g_cMCWindowFound=0;
	SetForceLimitingCurrentsX();

	//send parameters
	usleep(500);
	sprintf(g_cTxBuffer,"minws=%d;\r", XCOUNTS(g_ConfigData.RFIDConfig.fminWindowSize) ); sendWhistleMsgX(g_cTxBuffer);
	usleep(500);
	sprintf(g_cTxBuffer,"maxws=%d;\r", XCOUNTS(g_ConfigData.RFIDConfig.fmaxWindowSize)); sendWhistleMsgX(g_cTxBuffer);

	usleep(500);
	sendWhistleMsgXL("er[3]=pem;\r",11);
	g_ipglx=XCOUNTS(x);
	usleep(500);
	sprintf(g_cTxBuffer, "mvn=%d;\r", g_uiMoveNum); sendWhistleMsgX(g_cTxBuffer);
	usleep(500);
	sprintf(g_cTxBuffer, "pgl=%d;\r", g_ipglx); sendWhistleMsgX(g_cTxBuffer);
#ifdef WHISTLE_CONTINUOUS
	usleep(500);
	sprintf(g_cTxBuffer, "cmd=%d;\r", FindWinRun); sendWhistleMsgX(g_cTxBuffer);
#else
	usleep(500);
	sendWhistleMsgXL("kl;\r",4);
	usleep(2000);
	sendWhistleMsgXL("XQ##FINDW;\r",11);
#endif

	//Turn Brake Off
	BrakeOff();//In the case of a failure, the brake should be turned back on.
#ifdef OUTPUT_MVN_START
	logf("mvn start = %u\r\n", g_uiMoveNum);
#endif

	g_cMotionSet = HomeMotionSet; //speed and accel settings from HomeMotionSet
#endif
	return;
}

void MCSetMoveSpeedParams(float fsx, float fsy, float facx, float facy, float fdcx, float fdcy)
{
	sprintf(g_cTxBuffer, "sp=%d;\r", XCOUNTS(fsx) );	sendWhistleMsgX(g_cTxBuffer);
	usleep(500);
	sprintf(g_cTxBuffer, "ac=%d;\r", XCOUNTS(facx) );	sendWhistleMsgX(g_cTxBuffer);
	usleep(500);
	sprintf(g_cTxBuffer, "dc=%d;\r", XCOUNTS(fdcx) );	sendWhistleMsgX(g_cTxBuffer);
	usleep(500);

	sprintf(g_cTxBuffer, "sp=%d;\r", YCOUNTS(fsy) );	sendWhistleMsgY(g_cTxBuffer);
	usleep(500);
	sprintf(g_cTxBuffer, "ac=%d;\r", YCOUNTS(facy) );	sendWhistleMsgY(g_cTxBuffer);
	usleep(500);
	sprintf(g_cTxBuffer, "dc=%d;\r", YCOUNTS(fdcy) );	sendWhistleMsgY(g_cTxBuffer);
	usleep(500);

	g_cMotionSet = PositionMotionSet; //speed and accel settings from Position.
	//NOTE: These are dynamic and the only purpose in setting this is to indicate that it's NOT the current Jog settings.
	return;
}

void MCSetMoveSpeedParamsEven(float fs, float facdc)
{
	sprintf(g_cTxBuffer, "sp=%d;\r", XCOUNTS(fs) );	sendWhistleMsgXY(g_cTxBuffer);
	usleep(500);
	sprintf(g_cTxBuffer, "ac=%d;\r", XCOUNTS(facdc) );	sendWhistleMsgXY(g_cTxBuffer);
	usleep(500);
	sprintf(g_cTxBuffer, "dc=%d;\r", XCOUNTS(facdc) );	sendWhistleMsgXY(g_cTxBuffer);
	usleep(500);

	g_cMotionSet = PositionMotionSet; //speed and accel settings from Position.
	//NOTE: These are dynamic and the only purpose in setting this is to indicate that it's NOT the current Jog settings.
	return;
}

void MCSetMoveSpeedParamsX(float fsx, float facx, float fdcx)
{
	sprintf(g_cTxBuffer, "sp=%d;\r", XCOUNTS(fsx) );	sendWhistleMsgX(g_cTxBuffer);
	usleep(500);
	sprintf(g_cTxBuffer, "ac=%d;\r", XCOUNTS(facx) );	sendWhistleMsgX(g_cTxBuffer);
	usleep(500);
	sprintf(g_cTxBuffer, "dc=%d;\r", XCOUNTS(fdcx) );	sendWhistleMsgX(g_cTxBuffer);
	usleep(500);

	g_cMotionSet = PositionMotionSet; //speed and accel settings from Position.
	//NOTE: These are dynamic and the only purpose in setting this is to indicate that it's NOT the current Jog settings.
	return;
}

void MCSetMoveSpeedParamsY(float fsy, float facy, float fdcy)
{
	sprintf(g_cTxBuffer, "sp=%d;\r", YCOUNTS(fsy) );	sendWhistleMsgY(g_cTxBuffer);
	usleep(500);
	sprintf(g_cTxBuffer, "ac=%d;\r", YCOUNTS(facy) );	sendWhistleMsgY(g_cTxBuffer);
	usleep(500);
	sprintf(g_cTxBuffer, "dc=%d;\r", YCOUNTS(fdcy) );	sendWhistleMsgY(g_cTxBuffer);
	usleep(500);

	g_cMotionSet = PositionMotionSet; //speed and accel settings from Position.
	//NOTE: These are dynamic and the only purpose in setting this is to indicate that it's NOT the current Jog settings.
	return;
}

//FIXME PORTLOW  is MoveOpt still needed?  should we just set tr??????

void MCSetMoveParams(float fErrLev, byte cMoveOpt)
{
#ifdef GENHD4
	long l;
//FIXME PORTFATAL  if this is true consider just setting the GOLD elmo code so that 100 is not the wide default...
	l=XCOUNTS(fErrLev);
	if (l<100)
	{
		cMoveOpt=1; //because on GENHD4 counts are lower and some of the existing tolerances end up being less than 100...
					//the hole will fail to move withing tolerance because tr will be set to 100...  This option will set tr to the value l
	}
#endif
	//FIXME PORTMED4  must ensure proper tr for both paths is default
	sprintf(g_cTxBuffer, "maxerr=%d;\r", XCOUNTS(fErrLev) );	sendWhistleMsgX(g_cTxBuffer);
	usleep(500);
	sprintf(g_cTxBuffer, "maxerr=%d;\r", YCOUNTS(fErrLev) );	sendWhistleMsgY(g_cTxBuffer);
	usleep(500);
	sprintf(g_cTxBuffer, "mopt=%d;\r", cMoveOpt);	sendWhistleMsgXY(g_cTxBuffer);
	return;
}

void MCRunPosition(float x, float y)
{
#ifdef OUTPUT_WHISTLE_RUNANDSTOPS
	logf("RunPos\r\n");
#endif
	g_ipglx = XCOUNTS(x);
	g_ipgly = YCOUNTS(y);

	double dtest;
	dtest = ((double) g_ipglx / (double) g_ConfigData.EncoderRatio.fX);
	if (fabsinline(dtest-x) > 0.0001)
	{
		logf("position accuracy issue %f => %d => %f\r\n", x, g_ipglx, dtest);
		g_cMoveDone = MOVEDONE_ERROR;  //reset flag that indicates when a move has completed
		g_cMCRunningMsgGoal = PositionOK;
		return;
	}
	dtest = ((double) g_ipgly / (double) g_ConfigData.EncoderRatio.fY);
	if (fabsinline(dtest-y) > 0.0001)
	{
		logf("position accuracy issue %f => %d => %f\r\n", y, g_ipgly, dtest);
		g_cMoveDone = MOVEDONE_ERROR;  //reset flag that indicates when a move has completed
		g_cMCRunningMsgGoal = PositionOK;
		return;
	}

	g_cMoveDone = MOVEDONE_FALSE;  //reset flag that indicates when a move has completed
	g_cMCRunningMsgGoal = PositionOK;
	//Motion
	g_cMotorAction = X_AND_Y;
	g_uiMoveNum = g_uiMoveNumSeq++; //move it up
	if (g_uiMoveNumSeq == 0) //wrap arround to 0 so skip to 1
	{
		g_uiMoveNumSeq = 1; //reset to 1
	}
	g_cPositionProgress = 0;

	SetForceLimitingCurrents();

	usleep(500);
	sendWhistleMsgXYL("er[3]=pem;\r", 11);

	usleep(500);
	sprintf(g_cTxBuffer, "mvn=%d;\r", g_uiMoveNum); sendWhistleMsgXY(g_cTxBuffer);
	usleep(500);
	sprintf(g_cTxBuffer, "pgl=%d;\r", g_ipglx); sendWhistleMsgX(g_cTxBuffer);
	sprintf(g_cTxBuffer, "pgl=%d;\r", g_ipgly); sendWhistleMsgY(g_cTxBuffer);
	usleep(500);
#ifdef WHISTLE_CONTINUOUS
	#if PositionRun!=4
	#error "Correct hardcoded cmd=4 to match PositionRun's current value"
	#endif
	sendWhistleMsgXYL("cmd=4;\r", 7);
#else
	sendWhistleMsgXYL("kl;\r", 4);
	usleep(2000);
	sendWhistleMsgXYL("XQ##POSN;\r", 11);
#endif
	//Turn Brake Off
	BrakeOff(); //In the case of a failure, the brake should be turned back on.
#ifdef OUTPUT_MVN_START
	logf("mvn start = %u\r\n", g_uiMoveNum);
#endif
	return;
}
//FIXMEHIGH test implemented echo confirm for motion target

void MCStopPosition()
{
#ifdef OUTPUT_WHISTLE_RUNANDSTOPS
	logf("StopPos\r\n");
#endif
	g_cMCRunningMsgGoal = PositionStop;
	g_cPositionProgress = 0;
	//sendWhistleXYLMsg("er[3]=pem;\r",11);
	sendWhistleMsgXYL("mvn=0;\r", 7);
	sendWhistleMsgXYL("st;\r", 4);
	//Rabbit takes care of jumping to stop case, but we must stop the movement, and set the flag
	if (g_cMoveDone == MOVEDONE_FALSE)
	{
		//must now set to true, but only if it's not MOVEDONE_ERROR... retain error state for Rabbit to see and clear
		g_cMoveDone = MOVEDONE_STOP;
	}
	//Do not apply brake until it's stopped (and the confirmation is back), UNLESS there is a major err...
	//In the event of a fault, the brake would be applied
	//Motion
	g_cMotorAction = 0;
	return;
}

void MCStopPositionAdvanced() //do not set movedone to stop... use that only after it has stopped
{
#ifdef OUTPUT_WHISTLE_RUNANDSTOPS
	logf("StopPos\r\n");
#endif
	g_cMCRunningMsgGoal = PositionStop;
	g_cPositionProgress = 0;
	//sendWhistleXYLMsg("er[3]=pem;\r",11);
	sendWhistleMsgXYL("mvn=0;\r", 7);
	sendWhistleMsgXYL("st;\r", 4);
	//Motion
	g_cMotorAction = 0;
	return;
}

void MCStopAndFailMove()
{
#ifdef OUTPUT_WHISTLE_RUNANDSTOPS
	logf("StopAndFail\r\n");
#endif
	//stop both x and y
	sendWhistleMsgXYL("er[3]=pem;\r", 11);
	usleep(500);
	sendWhistleMsgXYL("mvn=0;\r", 7);
	usleep(500);
	sendWhistleMsgXYL("st;\r", 4);
	usleep(500);
	sendWhistleMsgXYL("ec;\r", 4);
	usleep(500);
	sendWhistleMsgXYL("mf;\r", 4);
	//When Running A Position Movement, this has a clear effect.
	//When Waiting, this also will have an effect of stopping on position.
	//must now set to ERROR
	g_cMoveDone = MOVEDONE_ERROR;  // send error back to MODE_POSN state machine
	//Since this is an error... always throw brake on

	BrakeOn();
	//beep
	BeepMCERR();
	//Esure Jog is clear
	g_cJogX = JOGSTOP; //state is now stopped
	g_cJogY = JOGSTOP;
	g_cJogGoalX = JOGSTOP;
	g_cJogGoalY = JOGSTOP;
	//Motion
	g_cMotorAction = 0;
	return;
}

//SetForceLimitingCurrents

void SetForceLimitingCurrents(void)
{
	//iqml   low limit for current monitor
	//iqmh   high limit for current monitor
	//imcl   cl for move...
	//impl   pl for move...
#ifdef FORCE_LIMIT_WHISTLE_CODE
	SetForceLimitingCurrentsX();
	SetForceLimitingCurrentsY();
#endif
}

void SetForceLimitingCurrentsX(void)
{
#ifdef FORCE_LIMIT_WHISTLE_CODE
	float fg;
	float f;
	float iqmh;
	float iqml;
	float imcl;
	float impl;

	if (g_ConfigData.ForceLimits.cActive==0)
	{
		//don't force limit...  use standard limits
		imcl=g_ConfigData.MCCurrentLimit.fX;
		impl=g_ConfigData.MCPeakCurrentLimit.fX;
		iqmh=impl;
		iqml=-impl;
		iqmh+=1;//move these out a little because it hits them by mistake
		iqml-=1;
	}
	else
	{

		if (g_ForceSensor.cErrFlag==1)
		{
			//can't set these right now
			//leave current settings, and let motion error handle any issues
			return;
		}

		//This was unused because the update rate needed is slow and there is no harm
		//g_ConfigData.ForceLimits.uiMinUpdateDelta

		//Calculate the current limits to use for X
		fg=g_ForceSensor.fX;
		if (fg>1.0)
		{
			fg=1.0; //never greater than 1 g should be used
		}
		//the sensor will always be aligned with the machine by calibration.
		//so a positive G means positive amps in general.
		f=fg*((float)g_ConfigData.ForceLimits.uiFullGravX)/1000.0;
		if (fg>=0)
		{
			iqmh=f+((float)g_ConfigData.ForceLimits.uiCurrentOverX)/1000.0;
			iqml=f-((float)g_ConfigData.ForceLimits.uiCurrentUnderX)/1000.0;
			imcl=iqmh;
			impl=iqmh;
			iqmh+=1; //move it 1 amp up because t's really the limit that takes care of this direction's limit in this gravity orientation
		}
		else
		{
			iqml=f-((float)g_ConfigData.ForceLimits.uiCurrentOverX)/1000.0;
			iqmh=f+((float)g_ConfigData.ForceLimits.uiCurrentUnderX)/1000.0;
			imcl=-iqml;
			impl=-iqml;
			iqml-=1; //move it 1 amp down because t's really the limit that takes care of this direction's limit in this gravity orientation
		}

		if (imcl>g_ConfigData.MCCurrentLimit.fX)
		{
			imcl=g_ConfigData.MCCurrentLimit.fX;
		}
		if (impl>g_ConfigData.MCPeakCurrentLimit.fX)
		{
			impl=g_ConfigData.MCPeakCurrentLimit.fX;
		}
	}

	sprintf(g_cTxBuffer,"iqml=%.2f;\r",iqml); sendWhistleMsgX(g_cTxBuffer);
	sprintf(g_cTxBuffer,"iqmh=%.2f;\r",iqmh); sendWhistleMsgX(g_cTxBuffer);
	sprintf(g_cTxBuffer,"impl=%.2f;\r",impl); sendWhistleMsgX(g_cTxBuffer);
	sprintf(g_cTxBuffer,"imcl=%.2f;\r",imcl); sendWhistleMsgX(g_cTxBuffer);
#endif
}

void SetForceLimitingCurrentsY(void)
{
#ifdef FORCE_LIMIT_WHISTLE_CODE
	float fg;
	float f;
	float iqmh;
	float iqml;
	float imcl;
	float impl;

	if (g_ConfigData.ForceLimits.cActive==0)
	{
		//don't force limit...  use standard limits
		imcl=g_ConfigData.MCCurrentLimit.fY;
		impl=g_ConfigData.MCPeakCurrentLimit.fY;
		iqmh=impl;
		iqml=-impl;
		iqmh+=1;//move these out a little because it hits them by mistake
		iqml-=1;
	}
	else
	{

		if (g_ForceSensor.cErrFlag==1)
		{
			//can't set these right now
			//leave current settings, and let motion error handle any issues
			return;
		}

		//This was unused because the update rate needed is slow and there is no harm
		//g_ConfigData.ForceLimits.uiMinUpdateDelta

		//Calculate the current limits to use for Y
		fg=g_ForceSensor.fY;
		if (fg>1.0)
		{
			fg=1.0; //never greater than 1 g should be used
		}
		//the sensor will always be aligned with the machine by calibration.
		//so a positive G means positive amps in general.
		f=fg*((float)g_ConfigData.ForceLimits.uiFullGravY)/1000.0;
		if (fg>=0)
		{
			iqmh=f+((float)g_ConfigData.ForceLimits.uiCurrentOverY)/1000.0;
			iqml=f-((float)g_ConfigData.ForceLimits.uiCurrentUnderY)/1000.0;
			imcl=iqmh;
			impl=iqmh;
			iqmh+=1; //move it 1 amp up because t's really the limit that takes care of this direction's limit in this gravity orientation
		}
		else
		{
			iqml=f-((float)g_ConfigData.ForceLimits.uiCurrentOverY)/1000.0;
			iqmh=f+((float)g_ConfigData.ForceLimits.uiCurrentUnderY)/1000.0;
			imcl=-iqml;
			impl=-iqml;
			iqml-=1; //move it 1 amp down because t's really the limit that takes care of this direction's limit in this gravity orientation
		}
		if (imcl>g_ConfigData.MCCurrentLimit.fY)
		{
			imcl=g_ConfigData.MCCurrentLimit.fY;
		}
		if (impl>g_ConfigData.MCPeakCurrentLimit.fY)
		{
			impl=g_ConfigData.MCPeakCurrentLimit.fY;
		}
	}
	sprintf(g_cTxBuffer,"iqml=%.2f;\r",iqml); sendWhistleMsgY(g_cTxBuffer);
	sprintf(g_cTxBuffer,"iqmh=%.2f;\r",iqmh); sendWhistleMsgY(g_cTxBuffer);
	sprintf(g_cTxBuffer,"impl=%.2f;\r",impl); sendWhistleMsgY(g_cTxBuffer);
	sprintf(g_cTxBuffer,"imcl=%.2f;\r",imcl); sendWhistleMsgY(g_cTxBuffer);
#endif
}

void UpdateForceLimitingCurrents(void)
{
#ifdef FORCE_LIMIT_WHISTLE_CODE
	//Only call the values update here if we know the motor is in a motor action, which
	//wmeans that it's doing a motion where brakes are off, and where these limits are still applicable.
	//There may be some special motor actions where the motor is on and the brakes are on, but
	//any action that 1st sets the limits, also sets this flag... so only update if the flag is set.
	//
	if (g_cMotorAction != 0)
	{
		//Move may still be running... update the limit varriables
		SetForceLimitingCurrents();
	}
#endif
}

////////////////////////////////////////////////////////////////////////
////continue here down..................................................

//Allow pass through of commands only for Console Mode

///////////////////

void MiniFTSendWhistleMsgX(char * s)
{
	sendWhistleMsgX(s);
}
void MiniFTSendWhistleMsgXL(char * s, int l)
{
	sendWhistleMsgXL(s, l);
}
void MiniFTSendWhistleMsgY(char * s)
{
	sendWhistleMsgY(s);
}
void MiniFTSendWhistleMsgYL(char * s, int l)
{
	sendWhistleMsgYL(s, l);
}

void MiniFTSendWhistleMsgXY(char * s)
{
	sendWhistleMsgXY(s);
}
void MiniFTSendWhistleMsgXYL(char * s, int l)
{
	sendWhistleMsgXYL(s, l);
}

//PRECHECKED
void RunMotorActionBrakeTimeoutCheck()
{
#ifdef BRAKE_ON_TIMEOUT
	unsigned long ul;
	if (g_cMotorAction == 0)
	{
		if (g_cBrakeReleased == 1) //Brake is off
		{
			if (g_cLastMotorAction != 0)
			{
				g_uiMotorActionTime = MS_TIMER;
			}
			ul = MS_TIMER - g_uiMotorActionTime;
			if (ul > g_ConfigData.uiBrakeOnTimeout)
			{
				//Turn On Brake
				BrakeOn();
				logf("Brake On Timeout!\r\n");
			}
		}
	}
	g_cLastMotorAction = g_cMotorAction;
	//FIXME minor  consider making a value that turns this off instead of always having it on
	return;
#endif
}

//PRECHECKED
void RunMotorActionAirClear()
{
#ifdef AIR_CLEAR
	byte cx;
	byte cy;
	cx=0;
	cy=0;
	if (g_cMotorAction!=0 && g_ConfigData.uiAirClear==1)
	{
		if ((g_cMotorAction & X_AXIS) != 0)
		{
			cx=1;
		}
		if ((g_cMotorAction & Y_AXIS) != 0)
		{
			cy=1;
		}
	}
	if (g_cAirBlastX!=cx)
	{
		g_cAirBlastX=cx;
		digOut(DIGOUT_CHNUM_AIRBLASTX, g_cAirBlastX );
		logf("abx%d\r\n",g_cAirBlastX);
	}
	if (g_cAirBlastY!=cy)
	{
		g_cAirBlastY=cy;
		digOut(DIGOUT_CHNUM_AIRBLASTY, g_cAirBlastY );
		logf("aby%d\r\n",g_cAirBlastY);
	}
#endif
}

//prechecked
void SetMotorActionDirectly()
{
	//For testing purposes indicate that there is a motor action
	//Used for Avoiding Brake Timeout
	g_cMotorAction = X_AND_Y;
}

#undef WCOUNTS

