////////////////////////////////////////////////////////////////////////////
// MiniFt And MiniFt HD
//
// 20131018-1-0843	David Whalen-Robinson	First Linux Smart Tool
//	Code should be considered a prototype.  All libraries are local project files.  All libraries and code locations
//  should be considered in temporary locations and are likely to experience subsequent revisions

//don't have this covered
#include "CrawlerConfigScript.h"


#if 0


//List of Compile Options

//Build Generation
//Motor Control Selection
//DevMode(s)
//Vision Support
//and further down:
//Probe Default ATOB BTOA
//Drill Default ATOB BTOA SAME REVERSE

//Build Generation

//The build version must match the platform
//Uncomment one of these build options.
//#define GEN20
//#define GEN25
//#define GENHD3
//#define GENHD4
#define GENHD5
//#define GENFD
//#define GENFLOORBEAM
//#define GENCIRCMFT2

//RIVET SUPPORT WAS REMOVED SINCE THE LAST VERSION
//If needed it can be restored in a better way
//GENCIRCMFT1 Support for the original GENCIRCMFT1 was removed
//If it returns, it would be implemented as a variant of the newer GENCIRCMFTX code.

//GEN3 : Refers to GENHD3
//GEN4 : Refers to GENHD4

//FIXME0000000000 at a good time, swap the type string to "MiniFT"
#define SMARTTOOL_TYPE_STRING "MiniFt"
#define SYSTEM_VERSION_STRING "20131003-1-x"

#ifdef GEN20
#define SMARTTOOL_SUBTYPE_STRING "MiniFT G2"
#endif
#ifdef GEN25
#define SMARTTOOL_SUBTYPE_STRING "MiniFT G2.5"
#endif
#ifdef GENHD3
#define SMARTTOOL_SUBTYPE_STRING "MiniFT-HD3"
#endif
#ifdef GENHD4
#define SMARTTOOL_SUBTYPE_STRING "MiniFT-HD4"
#endif
#ifdef GENHD5
#define SMARTTOOL_SUBTYPE_STRING "MiniFT-HD5"
#endif
#ifdef GENFD
#define SMARTTOOL_SUBTYPE_STRING "MiniFT-FD"
#endif
#ifdef GENFLOORBEAM
#define SMARTTOOL_SUBTYPE_STRING "MiniFT-FloorBeam"
#endif
#ifdef GENCIRCMFT1
#define SMARTTOOL_SUBTYPE_STRING "MiniFT-Circ1"
#endif
#ifdef GENCIRCMFT2
#define SMARTTOOL_SUBTYPE_STRING "MiniFT-Circ2"
#endif

#ifndef SMARTTOOL_SUBTYPE_STRING
#fatal "ERROR: Build generations or SMARTTOOL_SUBTYPE_STRING not defined"
#endif

//And Setting for OID COM_VERSION
#define COM_VERSION 2




//Control Which Set of code updates compiles
#define MINIFT_DEV_CODE
#define MINIFT_TEST_CODE
//After code passes testing, it will not need any of these ifdefs around it.

//FIXME0 retest when testing HD
//This activates the second calibration offset and the system to use it.
//You can test this distinctly by turning this on
//#define LS_USE_DUAL_PROBE_Y_COMP

#define MINIFT_TRIAL_STATION_CODE_A0
//do not try this yet
//#define MINIFT_TRIAL_STATION_CODE_USE_FIRST_HOLE_OP



#define DRILL_DIRECT_READY
#define DRILL_DIRECT_HOMESYS

//Working
#define DRILL_DIRECT_CUTTER_OIDS

//Working
#define DRILL_DIRECT_HOME_AFTER_HOME_BACK_SET


//This is now hardcoded
//DRILL_DIRECT_PROCESS_AND_OVERRIDE

//when we learn that drill is not home, then report as axis and and MiniFT is not homed....
//    that may work, but consider all the side effects of homed.....
//next then think about what athome does..............
/*For now, dont disable any athome.... just add some features for drill state to pendant for float and clamp_clamp.

AtHome is as simple as saying : Drill Clear and Drill IDLE is the only case this is true...

AtHome Turns Move Lock on...
AtHome Enabled Unclamp button along with other things... prevents any clamp operations if drillis not at home...

AtHome Prevents Float
Prevent Clamp

A disconnect assumes not at home.
Athome may have been used to select when to requery home status of drill after MiniFT Home...
Athome was used for fast home-->idle when minift wanted home
Athome was effected by drill modes..

AtHome controls the display of the Home button on the drill page...
>>>>>>>>>>>>>>>>>>>>
	Should show when drill is idle not home

*/
//	consider all side effects....
//then continue with another item wave....

//use this for clear code??????????????????????????????
//then see what else mode used for on pendant....

#ifdef GENHD3
#undef DRILL_DIRECT_READY
#undef DRILL_DIRECT_HOMESYS
#undef DRILL_DIRECT_CUTTER_OIDS
#undef DRILL_DIRECT_HOME_AFTER_HOME_BACK_SET
#endif

#ifdef GENCIRCMFT1
#undef DRILL_DIRECT_READY
#define GENCIRCMFTX
#endif
#ifdef GENCIRCMFT2
#undef DRILL_DIRECT_READY
#define GENCIRCMFTX
#endif

//For Building Laser Vision and Bypassing CAM vision on HDGEN3
#define GENHD3WITHLASER

//For Building GENHD3 with ESTOP 1 support
#define GENHD3WITHESTOP1



//Define Common Include Codes
//Defaults defined prior to Complication Options controlled by machine generation

//All Machines Except those undefining and defining an alternate use ELMO WHISTLE MC
#define WHISTLEMC 1
#define RABBITBRAKES

//include to use vision systems
//Currently we do this based on build.
//#define CENTERVISION
//#define CENTERVISION_ANALOG_POINT_LASER
//OR #define CENTERVISION_LINE_SCANNER
//OR #define CENTERVISION_CAM
//#define USE_HYSTERESIS_FROM_CENTERVISION

//All Whistle Machines except those that undefine and define an alternate use PX
#define PRIMARY_ENCODER_PX

#define DEFAULT_TOOLVERIFY TOOLVERIFY_ACTION

//Complication Options controlled by machine generation

#ifdef GEN20
#define GEN_ORIGINAL_STYLE
#endif
#ifdef GEN25
#define GEN_ORIGINAL_STYLE
#endif
#ifdef GEN_ORIGINAL_STYLE
#define GALIL 1
#define KEYPRESSERGALIL
#define MCBRAKES
#define READ_TEMP
#undef DEFAULT_TOOLVERIFY
#define DEFAULT_TOOLVERIFY TOOLVERIFY_PRE
#define OLD_SYSTEM_SPEED_DEFAULTS
#undef DRILL_DIRECT_READY
#undef DRILL_DIRECT_HOMESYS
#undef DRILL_DIRECT_CUTTER_OIDS
#undef DRILL_DIRECT_HOME_AFTER_HOME_BACK_SET
#define UNIBUTTON
#define LEDSYSTEM
#define BEEPSYSTEM
#endif

#ifdef GENHD3
#define GENHDSPEEDS
#define ORIENTATION_SENSORS
#define CLAMP_SYSTEM
#define CLAMP_SYSTEM_NAC_STP
#define HD_RAIL_STP
#define SMARTDRILL_STP
#define DRILLFILL_STP
#define CENTERVISION
#ifdef GENHD3WITHLASER
#define CENTERVISION_ANALOG_POINT_LASER
//#define USE_HYSTERESIS_FROM_CENTERVISION
#else
#define CENTERVISION_CAM
#define CAM_ORIENATION_STANDARD
#endif
#define UNIBUTTON
#ifdef GENHD3WITHESTOP1
//gen3 machines not yet modified to be ESTOPV2
#else
#define ESTOPV2
#endif
#define GEN4LUBE
#endif

#ifdef GENHD4
#define GENHDSPEEDS
#define ORIENTATION_SENSORS
#define CLAMP_SYSTEM
#define CLAMP_SYSTEM_NAC_STP
#define HD_RAIL_STP
#define SMARTDRILL_STP
#define CENTERVISION
#define CENTERVISION_CAM
#define CAM_ORIENATION_STANDARD
#define USE_RFID
#define USE_RFID_OMRON
#define ESTOPV2
#define DELAY_AFTER_ESTOP_DEFAULT 1500
#define GEN4LUBE
#endif

#ifdef GENHD5
#define GENHDSPEEDS
#define ORIENTATION_SENSORS
#define CLAMP_SYSTEM
#define CLAMP_SYSTEM_NAC_STP
#define HD_RAIL_STP
#define SMARTDRILL_STP
#define CENTERVISION
#define CENTERVISION_CAM
#define CAM_ORIENATION_STANDARD
#define USE_RFID
#define USE_RFID_MIFARE
#define ESTOPV2
#define DELAY_AFTER_ESTOP_DEFAULT 1500
#define USE_FINDWINDOW
//#define SEAL_SYSTEM

#define FASTENER_SYSTEM
#define FASTENER_STP
#define FASTENER_TRAY_STP
#endif

#ifdef GENFD
//HD Supports Clamping Generally, except GENFD
#define SMARTDRILL_STP
#define HD_RAIL_STP
//no center vision currently
//#define CENTERVISION
//#define CENTERVISION_ANALOG_POINT_LASER
//#define USE_HYSTERESIS_FROM_CENTERVISION
////use PY
//#undef PRIMARY_ENCODER_PX
//#define PRIMARY_ENCODER_PY
#warnt "GENFD must use Primary Encoder until Aux issue is resolved"
#define ESTOPV2
#endif

#ifdef GENFLOORBEAM
#define GENHDSPEEDS
//#define ORIENTATION_SENSORS
#define ORIENTATION_PERMANENT
#define CLAMP_SYSTEM
#define CLAMP_SYSTEM_NAC_STP
#define HD_RAIL_STP
#define SMARTDRILL_STP
#define AIR_CLEAR
#define CENTERVISION
#define CENTERVISION_CAM
#define CAM_ORIENATION_CXMXN
#define USE_RFID
#define USE_RFID_MIFARE
#define ESTOPV2
#define DELAY_AFTER_ESTOP_DEFAULT 1500
#define USE_FINDWINDOW
#endif

#ifdef GENCIRCMFT1
#define CENTERVISION
#define CENTERVISION_ANALOG_POINT_LASER
//#define USE_HYSTERESIS_FROM_CENTERVISION
#define ESTOPV2
#define DELAY_AFTER_ESTOP_DEFAULT 500
#define LEDSYSTEM
#define BEEPSYSTEM
#endif

#ifdef GENCIRCMFT2
#define CLAMP_SYSTEM
#define CENTERVISION
#define CENTERVISION_ANALOG_POINT_LASER
//#define USE_HYSTERESIS_FROM_CENTERVISION
#define USE_RFID
#define USE_RFID_OMRON
#define ESTOPV2
#define DELAY_AFTER_ESTOP_DEFAULT 500
#define LEDSYSTEM
#define BEEPSYSTEM
#endif

//More Build Options

//Turn this on for Everything
#define AUTO_MOVE_FIRST_POSITION

#ifndef GALIL
#ifndef WHISTLEMC
#fatal "ERROR: GALIL or WHISTLEMC must be defined"
#endif
#endif

#ifdef GALIL
#ifdef WHISTLEMC
#error "ERROR: Both GALIL and WHISTLEMC are defined"
#endif
#endif

#ifdef GENCIRCMFTX
#define CLAMP_LOOSE_OR_UNCLAMP CLAMP_UNCLAMP
#endif

#ifdef CLAMP_SYSTEM
#ifndef CLAMP_LOOSE_OR_UNCLAMP
#define CLAMP_LOOSE_OR_UNCLAMP CLAMP_LOOSE
#endif
#endif


//Include pressure sensor
#ifdef GENCIRCMFT1
#define PRESSURESENSOR
#endif

//EEOPTION Build Features
//This prevents the eeoption from being changed at all
//#define EEOPTION_PERMANENT

// EEOPTION_DEFAULT
#ifdef GENHD3
#define EEOPTION_DEFAULT EEHDGEN3RAIL
#endif
#ifdef GENHD4
#define EEOPTION_DEFAULT EEHDGEN4RAIL
#endif
#ifdef GENHD5
#define EEOPTION_DEFAULT EEHDGEN5RAIL
#endif
#ifdef GENFD
#define EEOPTION_DEFAULT EEFD1
#endif
#ifdef GENFLOORBEAM
#define EEOPTION_DEFAULT EEFLOORBEAM
#endif
#ifdef GENCIRCMFT1
#define EEOPTION_DEFAULT EECIRCMFT1
#define EEOPTION_PERMANENT
#endif
#ifdef GENCIRCMFT2
#define EEOPTION_DEFAULT EECIRCMFT2
#define EEOPTION_PERMANENT
#endif
#ifndef EEOPTION_DEFAULT
#define EEOPTION_DEFAULT EENONE
#endif



//Brake on during stops
//This is now the default for all machines
//NOTE: Brake will turn on for certain errors, even if this define is not set
#define BRAKE_ON_WHEN_STOPPED

//Define this to activate a break on after motion is over
//The feature uses OID g_ConfigData.uiBrakeOnTimeout to set the number of ms
#define BRAKE_ON_TIMEOUT

//Primary Estop System defined by model above
#ifndef ESTOPV2
#ifndef ESTOPV1B
#fatal "Define ESTOPV2 or ESTOPV1B"
#endif
#endif

//Don't leave estop until power supply to drive is stable
#ifndef DELAY_AFTER_ESTOP_DEFAULT
#ifdef ESTOPV1B
#define DELAY_AFTER_ESTOP_DEFAULT 50
#endif
#ifdef ESTOPV2
#define DELAY_AFTER_ESTOP_DEFAULT 500
#endif
#endif















//	The following is needed for NEMS pull (HTTP server), NEMS push (NetMgr) and FTP server.

//STP LIB

#include "STP.h"

////////////////////////////////////////////////////////////////////////////////
//	DEFINES - Application specific
////////////////////////////////////////////////////////////////////////////////

#define TCP_CONNECTION_TIMEOUT_SEC 5

//	using STP
#define STP_SERVICE_THROTTLE_MS	20	//	this is application-specific...not inherently part of STP.LIB
////////////////////////////////////////////////////////////////////////////////
//	DEFINES
////////////////////////////////////////////////////////////////////////////////

//edit
//	maximum num of chars in a global temporary string (used for things like sprintf's to send build msgs)
//#define TEMPSTRING_MAXSIZE	512
//#define INI_STRING_MAXSIZE	33
//#define INI_LINE_MAXSIZE	129

//DIO

//	physical channel numbers of digital outputs
//edit
#define DIGOUT_SUSPENSION 4		//May or may not be used.
#define DIGOUT_RAIL_AIR  3 	//Apply Air to Rail
#define DIGOUT_GRIPPER_TOOL	2

#define DIGOUT_LOWER_TOOL  0
#define DIGOUT_LIFT_TOOL  1

//FIXME VHIGH  Crawler HW stop lines NOT BAD IDEA FOR CRAWLER
//#define DIGOUT_LINE_STOP 	4
//May need to implement so that crawler doens't ever move after stop.

//Allocate Digin channels:
#define DIGIN_ESTOP 2             //	Logical state. (NEW in GEN4)
#define DIGIN_FLOAT	3
//FIXME SEVERE resolve labels and dual up and dual down sensors
// Should be like this, where P is one side of the machine, and N is the other side
// I currently don't know if 4 or 7 is P or N and if 5 or 6 is P or N
// DIGIN_RAILDOWN_WHEELSUP_P 4
// DIGIN_RAILDOWN_WHEELSUP_N 7
// DIGIN_RAILUP_WHEELSDOWN_P 5
// DIGIN_RAILUP_WHEELSDOWN_N 6
//
#define DIGIN_RAILDOWN 	4
#define DIGIN_WHEELSDOWN 5
#define DIGIN_RAILUP	6
#define DIGIN_WHEELSUP	7

//Allocate analog channels for comunication with IR sensing.
#define DIGIN_CLIFFDETECT1 0
#define DIGIN_CLIFFDETECT2 1
#define DIGIN_CLIFFDETECT3 4
#define DIGIN_CLIFFDETECT4 5

//Bypass definition for bypassing errors in the stance machine.
#define BYPASSON 1
#define BYPASSOFF 0

//NOT USING IN CRAWLER YET
//#define DIGIN_WHISTLE3_MSG 4
//#define DIGIN_WHISTLE4_MSG 5

//NOT USING IN CRAWLER YET
//May NEVER USE FOR WHISTLE UDP
//#define DIGIN_WHISTLE1_MSG 6
//#define DIGIN_WHISTLE2_MSG 7

#define DIGIN_DEBOUNCE_HISTORY_COUNT 8

//	physical digital input channels (0 through 7 for MFI-RCM4200)

//State of Digital input valves.
#define Valve_On_State  1
#define Valve_Off_State 0

////////////////////////////////////////////////////////////////////////////////
//	Motion Control Defs
////////////////////////////////////////////////////////////////////////////////

#include "WhistleUDP.h"
#include "CrawlerWhistle.h"

//	States for generic state machine
#define STATEMACHINE_FAIL -1
#define STATEMACHINE_DONE 0
#define STATEMACHINE_BUSY 1

//defines for verbosity of msgs that are echo'd out from the whistle
#define MCCOMMECHO_OFF	0
#define MCCOMMECHO_ALL	1
#define MCCOMMECHO_DBG	2

#define OFF	0
#define ON	1
//Used for when the one of the 5 input sensors is sensing.
#define DETECTED 0

//Used for when the Cliff sensor  is sensing.
#define CLIFFVOLTS 2.5

//edit
//#define USERBLOCK_ACCESS_STACK	1
//#define USERBLOCK_ACCESS_LAYERS	2
//#define USERBLOCK_ACCESS_NUMLAYERS	3
//#define USERBLOCK_ACCESS_MAINTDATA	4
//#define USERBLOCK_ACCESS_HOLEDATA	5
//#define USERBLOCK_ACCESS_CUTTER	6

////////////////////////////////////////////////////////////////////////////////
//	Globals
////////////////////////////////////////////////////////////////////////////////

long g_pipc;
int g_ibootipc;

//Bypass variables.
int g_iMoveBypass;
int g_iOperateBypass;
int g_iParkBypass;

//Timer globals
unsigned long g_ulStartStanceTime;
unsigned long g_ulTempTime;
unsigned long g_ulStanceTime;

//Timers that store the time for an action to take place.

//Engage/Operate timer variables
unsigned long g_ulOperateActionTime1;
int g_iStartOperateTimer;

//Move timer variables
unsigned long g_ulMoveActionTime1;
int g_iStartMoveTimer;

//Park timer variables
unsigned long g_ulParkActionTime1;
int g_iStartParkTimer;

//Timeout integers for ActionTime variables
int iActionTimeout1;
int iActionTimeout2;
int iActionTimeout3;
int iActionTimeout4;
int iActionTimeout5;

unsigned long g_ulTempCounter;
//Other non OID globals
char g_cModeStatePrev;

char g_cStanceGoalLast;
char g_cStanceStateLast;

#ifdef EDGEDETECT_SYSTEM
char g_cUseEdgeDetect;
int g_iEdgeDetect;
#endif

//Time of Last g_DateTime update
unsigned long g_ulDateTimeMS;

char g_cSentModeState;

//edit
char g_cLastClampLegsGoal;
unsigned long g_ulClampOperationStart;

//char g_cBugPosition; //used for debug

//edit
unsigned long g_ulLastJogPosnQuery;

//edit
unsigned long g_ulClampStart;

//edit
char g_uiClampLegsError;

//DIO
char g_cLegsLock;
char g_cLegsExtend;
char g_cLegsRetract;

unsigned long g_ulClampLegsRetract;
unsigned long g_ulClampLegsExtend;
unsigned long g_ulClampLegsUnlock;
unsigned long g_ulClampLegsLock;

char g_cBackfeetUp;

//DigIn Estop
char g_cDigInEstopXCount;
char g_cDigInEstop;

typedef struct
{
	char bEthernetIsUp;
	char bLoadProfileError;  //	0 = success,  Non-zero = failure
} td_SystemStatus;

td_SystemStatus g_SystemStatus;

//	DCD file support

//edit
//char	g_szTemp[TEMPSTRING_MAXSIZE];  //	used for temporary strings when building complex strings for msgs

#define MC_ASSERTED_ESTOP
char g_cEstopMCAsserted;

//For use by EthernetDownloaderEnabled
char g_cEthernetDownloaderEnabled;
//Generic Flag for activating temporary testing features
char g_cAuxCtrlFlag;
//For Use with ShowIO
char g_cShowIO;
char g_cShowAna;

//Hold
char g_cHold;

//Last Sent Home Values
char g_cSentClampHomed;

char * g_szModeNames[ModeDisEngage + 1];

//Edit

////////////////////////////////////////////////////////////////////////////////
//	Function Prototypes
////////////////////////////////////////////////////////////////////////////////
int main(void);
void InitVars(void);

#ifdef EDGEDETECT_SYSTEM
void EdgeFoundStop(void);
char EdgeMoveAway(float fx, float fy, float fr);
char EdgeDetected();
#endif

void ModeStateMachine(void);
void StanceStateMachine(void);

//edit
//void InitClampLegsStateMachine();
//void ClampLegsStateMachine( void );
void CrawlerHomeMachine(void);

//	Digital I/O functions
void InitDIO(void);
void ReadDigitalInputs(void);
float anaInConvertVolts(unsigned int uiRawValue, unsigned int channel, unsigned int gaincode);

void EstopEngageActions(void);
void EstopContinueActions(void);
void EstopDisengageActions(void);
void FailAnyHomeRunning(void);

void GripperChange(int i_GripState);

void RailAirChange(int i_AirState);

//FIXME0 consider new position
void SmartToolMsgCrawlerMessageCode(td_STPsessions * p_STPSession, unsigned int uiOID, unsigned int uiMessageCode);

//	User Block access functions - config defaults are stored in User Block of flash
void LoadConfig(void);
void StoreConfig(td_STPsessions * p_STPSession);
void RecallConfig(td_STPsessions * p_STPSession, int irecall);

//	The following is used only for the NEMS PUSH implementation of DCD files.
//	int DataToHtml( char *sBufOut, char *sBufIn, int iLen );

//	int strpos(char * s, int c);
//extern int _catoerror;
int catoi(char * pstr);
long catol(char * pstr);
float catof(char * pstr);

int strnpos(char * s, int c, int iLen);

int atobool(char *s);

void AlertModeChanges(td_STPsessions * p_STPSession);

void SendMoveStatus(td_STPsessions * p_STPSession);

/////////////////////////////////////////
//	TIMERB FUNCTION PROTOTYPES
/////////////////////////////////////////
#ifdef USE_TIMERB
void TimerBInit(void );
void TimerBStart(void);
int TimerBRead(void);
int TimerBReset(void);

void timerb_isr();
void KillBTimer(void);
#endif

void ClearShowIO();
void ShowIO();
void ClearShowAna();
void ShowAna();


#ifdef EDGEDETECT_SYSTEM
	g_cUseEdgeDetect = EDGEDETECTION_DEFAULT; //off by default for now
	g_iEdgeDetect = 0;
#endif

	//Bypass vars
	g_iMoveBypass = 0;
	g_iOperateBypass = 0;
	g_iParkBypass = 0;

}

//encoder ratio calc
//Now Using 20.1 as wheel circumfrence estimate
//400000/20.1 rounded is  19900 counts per inch

////////////////////////////////////////////////////////////////////////////////
//	InitDIO
////////////////////////////////////////////////////////////////////////////////
void InitDIO(void)
{
//edit
//	g_cLegsExtend = LEGS_OFF;
//	g_cLegsRetract = LEGS_RETRACT;//For consistent behavior with past and across 1 or 2 valve systems.
//	g_cLegsLock = LEGSLOCK_OFF;

//	g_cBackfeetUp = BACKFEET_NOT_UP;

	g_cDigInEstopXCount = 1;
	g_cDigInEstop = ESTOP_SIGNAL; // normally high, pulled low.

	//Set the Valve so that it does not get hot.
	digOut(DIGOUT_LOWER_TOOL, Valve_Off_State);
	digOut(DIGOUT_LIFT_TOOL, Valve_Off_State);

	//Set the Gripper to Open on Startup.
	GripperChange(OFF);

	//Set the RailAir to OFF on Startup.
	RailAirChange(OFF);
//	g_cClampHomeSensor=0;
}

////////////////////////////////////////////////////////////////////////////////
//	ReadDigitalInputs Debounce / DIO / ADC
////////////////////////////////////////////////////////////////////////////////
void ReadDigitalInputs(void)
{
	// debounces digital inputs...best run as a periodic interrupt, but can be called
	// from a round-robin state machine (debounce times will vary if run from a round-robin state machine)
	// current version keeps the count of consecutive inputs opposite to the current value.
	// If this number reaches the DIGIN_DEBOUNCE_HISTORY_COUNT, then the input is flipped.

	//NOTE: This Method may seem longer and slower than the 2 or 3 other old methods, but
	// it's actually very efficient on the CPU, especially in the case there is no change.
	// It takes care of the history gathering and double latch code that was required after
	// the history gathering such as if hist==0  or hist=BIT_WINDOW that the previous had to do.
	//Taking the code out of the loop makes variable access by label more efficient, and
	//reduces indexing code which might take as much space as just listing this out.

	char cv;

	cv = digIn(DIGIN_ESTOP);
	if (cv != g_cDigInEstop)
	{
		//disagreement ... less common path
		g_cDigInEstopXCount++;
		if (g_cDigInEstopXCount >= DIGIN_DEBOUNCE_HISTORY_COUNT)
		{
			//now reach the count... flip
			g_cDigInEstop = cv;
			g_cDigInEstopXCount = 0;
		}
	}
	else
	{
		//agreement
		g_cDigInEstopXCount = 0;
	}

}

//	Convert raw ADC counts to volts.
float anaInConvertVolts(unsigned int uiRawValue, unsigned int channel, unsigned int gaincode)
{
	float fRetVal;
//FIXME SEVERE Need Analog lib
#warning "no analog volt conversion built for linux version"
	fRetVal = 0;
	return fRetVal;
}

#define ADC_GAIN 1
#define FAULTCODE_ADC_OVERFLOW	-4096

////////////////////////////////////////////////////////////////////////////////
// MAC Valve Control
//
// MacVal1
//  iLockSw =1 (SupportFeet Lock)
//           0 (SupportFeed Unlock)
// MacVal2
//  iLockSw =1 (SupportFeed extend)
//           0 (SupportFeed retract)
// MacVal3
//  iLockSw =1 (coolant On)
//           0 (coolant Off)
////////////////////////////////////////////////////////////////////////////////

void EstopEngageActions(void)
{
	MCRunEstopEngage();
//edit
//edit	g_cClampGoal=CLAMP_OFF;
//edit	g_cClampState=CLAMP_HOLD;
}
void EstopContinueActions(void)
{
	FailAnyHomeRunning();
//edit	g_cClampGoal=CLAMP_OFF;
}
void EstopDisengageActions(void)
{
	MCRunEstopDisengage();
	FailAnyHomeRunning();
}

void FailAnyHomeRunning(void)
{
//edit
//	if (g_cClampHomed == HOME_RUNNING)
//	{
//		g_cClampHomed = HOME_FAILURE;
//	}
}

////////////////////////////////////////////////////////////////////////////////
// Socket Console
////////////////////////////////////////////////////////////////////////////////

void HandleSocketConsoleCommand(char * scmd, int icmdlen)
{
	char c;
	char lc;
	int i;
	char * sarg;
	char * sarg2;

	if (_cConsoleSockCommandMode >= 1)
	{
		// Console Command mode 1 goes to mc whistle
		// Console Command mode 2 goes to mca whistle
		//Check for special "~" command that will exit whistle mode
		if (icmdlen == 1 && *scmd == '~')
		{
			_cConsoleSockCommandMode = 0;
			return;
		}
		//send to drive
		scmd[icmdlen++] = '\r';	//OK to replace this character (the original end of command)
		c = scmd[icmdlen];	//record this character so we can restore this one.
		scmd[icmdlen] = 0;		//place temporary null
		if (_cConsoleSockCommandMode == 1)
		{
			CrawlerSendWhistleMsg1L(scmd, icmdlen);
		}
		else if (_cConsoleSockCommandMode == 2)
		{
			CrawlerSendWhistleMsg2L(scmd, icmdlen);
		}
		else if (_cConsoleSockCommandMode == 3)
		{
			CrawlerSendWhistleMsg3L(scmd, icmdlen);
		}
		else if (_cConsoleSockCommandMode == 4)
		{
			CrawlerSendWhistleMsg4L(scmd, icmdlen);
		}
		scmd[icmdlen] = c; //restore this character
		return;
	}
	sarg = 0;
	sarg2 = 0;
	i = 0;
	lc = 0;
	while (i < icmdlen)
	{
		c = scmd[i];
		if (c == ' ')
		{
			scmd[i] = 0;
		}
		else if (lc == ' ')
		{
			//last was space, but this is not
			if (sarg == 0)
			{
				sarg = scmd + i;
			}
			else if (sarg2 == 0)
			{
				sarg2 = scmd + i;
			}
		}
		lc = c;
		i++;
	}

	if (strcmp(scmd, "update") == 0)
	{
		g_cEthernetDownloaderEnabled = 1;
		logf("firmware update enabled\r\n");
	}
	else if (strcmp(scmd, "exit") == 0)
	{
		SocketConsoleClose();
	}
	else if (strcmp(scmd, "eu") == 0)
	{
		logf("eu=%d\r\n", g_SystemStatus.bEthernetIsUp);
	}
	else if (strcmp(scmd, "euon") == 0)
	{
		g_SystemStatus.bEthernetIsUp = TRUE;
	}
	//Crawler Test Commands
	else if (strcmp(scmd, "poperate") == 0)
	{
		g_cModeState = ModePrepareOperate;
		logf("Mode: Prepare To Operate tool.\r\n");

	}
	else if (strcmp(scmd, "pmove") == 0)
	{
		g_cModeState = ModePrepareToMove;
		logf("Mode: Prepare To Move.\r\n");

	}
	else if (strcmp(scmd, "lifteron") == 0)
	{
		digOut(DIGOUT_LOWER_TOOL, Valve_On_State);
		digOut(DIGOUT_LIFT_TOOL, Valve_On_State);
		logf("Mode: Lifter On.\r\n");
	}
	else if (strcmp(scmd, "lifteroff") == 0)
	{
		digOut(DIGOUT_LOWER_TOOL, Valve_Off_State);
		digOut(DIGOUT_LIFT_TOOL, Valve_Off_State);
		logf("Mode: Lifter Off.\r\n");
	}
	else if (strcmp(scmd, "lift") == 0)
	{
		digOut(DIGOUT_LOWER_TOOL, Valve_Off_State);
		digOut(DIGOUT_LIFT_TOOL, Valve_On_State);
		logf("Mode: Lift Tool.\r\n");
	}
	else if (strcmp(scmd, "lower") == 0)
	{
		digOut(DIGOUT_LOWER_TOOL, Valve_On_State);
		digOut(DIGOUT_LIFT_TOOL, Valve_Off_State);
		logf("Mode: Lower Tool\r\n");
	}
	else if (strcmp(scmd, "park") == 0)
	{
		g_cModeState = ModePark;
		logf("Mode: Park\r\n");

	}
	else if (strcmp(scmd, "init") == 0)
	{
		g_cModeState = ModeInit;
		logf("Mode: Init\r\n");

	}
	else if (strcmp(scmd, "engage") == 0)
	{
		g_cModeState = ModeEngage;
		logf("Mode: Engage.\r\n");

	}
	else if (strcmp(scmd, "move") == 0)
	{
		g_cModeState = ModeMove;
		logf("Mode: Move.\r\n");
	}
	//Stance Test Commands
	else if (strcmp(scmd, "so") == 0)
	{
		g_cStanceGoal = StanceOff;
	}
	else if (strcmp(scmd, "railoff") == 0)
	{
		RailAirChange(OFF);
	}
	else if (strcmp(scmd, "railon") == 0)
	{
		RailAirChange(ON);
	}
	else if (strcmp(scmd, "gripoff") == 0)
	{
		GripperChange(OFF);
	}
	else if (strcmp(scmd, "gripon") == 0)
	{
		GripperChange(ON);
	}
	else if (strcmp(scmd, "checkvars") == 0)
	{
		logf("ModeState is %d \r\n", g_cModeState);
		logf("StanceGoal is %d \r\n", g_cStanceGoal);
	}
#ifdef EDGEDETECT_SYSTEM
	else if (strcmp(scmd, "checkedge") == 0)
	{
		logf("UseEdgeDetect=%d\r\n",g_cUseEdgeDetect);
		logf("ch1 v: %f \r\n", anaInVolts(DIGIN_CLIFFDETECT1, 3));
		logf("ch2 v: %f \r\n", anaInVolts(DIGIN_CLIFFDETECT2, 3));
		logf("ch3 v: %f \r\n", anaInVolts(DIGIN_CLIFFDETECT3, 3));
		logf("ch4 v: %f \r\n", anaInVolts(DIGIN_CLIFFDETECT4, 3));
		logf("iEdgeDetect is %d \r\n", g_iEdgeDetect);
	}
	else if (strcmp(scmd, "edon") == 0)
	{
		g_cUseEdgeDetect=1;
	}
	else if (strcmp(scmd, "edoff") == 0)
	{
		g_cUseEdgeDetect=0;
	}
#endif
	else if (strcmp(scmd, "bypassall") == 0)
	{
		g_iMoveBypass = BYPASSON;
		g_iOperateBypass = BYPASSON;
		g_iParkBypass = BYPASSON;

	}
	else if (strcmp(scmd, "nobypass") == 0)
	{
		g_iMoveBypass = BYPASSOFF;
		g_iOperateBypass = BYPASSOFF;
		g_iParkBypass = BYPASSOFF;

	}
	else if (strcmp(scmd, "movebypasson") == 0)
	{
		g_iMoveBypass = BYPASSON;
	}
	else if (strcmp(scmd, "movebypassoff") == 0)
	{
		g_iMoveBypass = BYPASSOFF;
	}
	else if (strcmp(scmd, "operatebypasson") == 0)
	{
		g_iOperateBypass = BYPASSON;
	}
	else if (strcmp(scmd, "operatebypassoff") == 0)
	{
		g_iOperateBypass = BYPASSOFF;
	}
	else if (strcmp(scmd, "parkbypasson") == 0)
	{
		g_iParkBypass = BYPASSON;
	}
	else if (strcmp(scmd, "parkbypassoff") == 0)
	{
		g_iParkBypass = BYPASSOFF;
	}
	else if (strcmp(scmd, "sm") == 0)
	{
		g_cStanceGoal = StanceMove;

	}
	else if (strcmp(scmd, "sp") == 0)
	{
		g_cStanceGoal = StancePark;

	}
	else if (strcmp(scmd, "se") == 0)
	{
		g_cStanceGoal = StanceEngage;

	}
	else if (strcmp(scmd, "exit") == 0)
	{
		SocketConsoleClose();
	}
	//Whistle Console Modes
	//To Save Code and Data Space, entering the whistle mode will also turn on echo for that whistle.
	// Leaving the whistle mode with "~" will not actualy turn off the whistle echo.
	//The 3rd command "wq" meaning "whistle quiet" will turn off the echo again.
	//This is intentional so that we don't need many more commands.

	else if (strcmp(scmd, "w1") == 0)	//Whistle Clamp Axis Console Mode
	{
		//mc direct
		_cConsoleSockCommandMode = 1;
		i = 3;	//default is rx and tx on
		if (sarg != 0)
		{
			i = atoi(sarg);
		}
		MCSetCommDisplay1(i);
	}
	else if (strcmp(scmd, "w2") == 0) //Whistle Clamp Axis Console Mode
	{
		//mc direct
		_cConsoleSockCommandMode = 2;
		i = 3; //default is rx and tx on
		if (sarg != 0)
		{
			i = atoi(sarg);
		}
		MCSetCommDisplay2(i);
	}
	else if (strcmp(scmd, "w3") == 0) //Whistle Clamp Axis Console Mode
	{
		//mc direct
		_cConsoleSockCommandMode = 3;
		i = 3; //default is rx and tx on
		if (sarg != 0)
		{
			i = atoi(sarg);
		}
		MCSetCommDisplay3(i);
	}
	else if (strcmp(scmd, "w4") == 0) //Whistle Clamp Axis Console Mode
	{
		//mc direct
		_cConsoleSockCommandMode = 4;
		i = 3; //default is rx and tx on
		if (sarg != 0)
		{
			i = atoi(sarg);
		}
		MCSetCommDisplay4(i);
	}
	else if (strcmp(scmd, "wq") == 0) //Whistle Quiet will stop the tx rx echo
	{
		MCSetCommDisplay1(0);
		MCSetCommDisplay2(0);
		MCSetCommDisplay3(0);
		MCSetCommDisplay4(0);
	}
	else if (strcmp(scmd, "wen1") == 0)
	{
		//cut or restore communication to a axis 0 = no coms 1 = normal coms
		i = atobool(sarg);
		MCSetCommEnable1((char) i);
	}
	else if (strcmp(scmd, "wen2") == 0)
	{
		//cut or restore communication to a axis 0 = no coms 1 = normal coms
		i = atobool(sarg);
		MCSetCommEnable2((char) i);
	}
	else if (strcmp(scmd, "wen3") == 0)
	{
		//cut or restore communication to a axis 0 = no coms 1 = normal coms
		i = atobool(sarg);
		MCSetCommEnable3((char) i);
	}
	else if (strcmp(scmd, "wen4") == 0)
	{
		//cut or restore communication to a axis 0 = no coms 1 = normal coms
		i = atobool(sarg);
		MCSetCommEnable4((char) i);
	}
	else if (strcmp(scmd, "wena") == 0)
	{
		//cut or restore communication to a axis 0 = no coms 1 = normal coms
		i = atobool(sarg);
		MCSetCommEnable1((char) i);
		MCSetCommEnable2((char) i);
		MCSetCommEnable3((char) i);
		MCSetCommEnable4((char) i);
	}
#if 0
	//	Back feet lock
	else if (strcmp(scmd, "bfl") == 0)
	{
		i=atobool(sarg);
		g_cLegsLock=i;
		digOut(DIGOUT_LINE_BACKFEET_LOCK, i);
	}
	//	Back feet extend
	else if (strcmp(scmd, "bfe") == 0)
	{
		i=atobool(sarg);
		g_cLegsExtend=i;
		digOut(DIGOUT_LINE_BACKFEET_EXTEND, i);
	}
	//	Back feet retract
	else if (strcmp(scmd, "bfr") == 0)
	{
		i=atobool(sarg);
		g_cLegsRetract=i;
		digOut(DIGOUT_LINE_BACKFEET_RETRACT, i); //May not do anything if USE_DI
	}
#endif
	//	Back feet up
	else if (strcmp(scmd, "bu") == 0)
	{
		/*edit
		 g_cClampLegsState=CLAMP_OFF;
		 g_cClampLegsGoal=CLAMP_UNCLAMP;
		 //FORCE up directly
		 g_cLegsExtend = LEGS_EXTEND; //Force state indicator to OPOSITE of desired current state, which will make CLAMP_UNCLAMP code execute
		 g_cLegsRetract = LEGS_OFF; //Force state to OPOSITE
		 g_cLegsLock = LEGSLOCK_ON;
		 //and truely force values to change
		 digOut(DIGOUT_LINE_BACKFEET_LOCK, 0);
		 digOut(DIGOUT_LINE_BACKFEET_EXTEND, LEGS_OFF);
		 #ifdef USE_DISTINCT_EXTEND_AND_RETRACT
		 digOut(DIGOUT_LINE_BACKFEET_RETRACT, LEGS_RETRACT);//Only Set if really present,otherwise EXTEND LEGS OFF is actually dong LEGS_RETRACT
		 #endif
		 */
	}

	/*
	 //	Back feed Mode Adjust


	 else if (strcmp(scmd, "bfa") == 0)
	 {
	 if (sarg!=0)
	 {
	 i=atoi(sarg);
	 g_ConfigData.cClampLegsMode = i;
	 }
	 logf("bfa=%d\r\n",g_ConfigData.cClampLegsMode);
	 }
	 else if (strcmp(scmd, "bfz") == 0)
	 {
	 if (sarg!=0)
	 {
	 i=atoi(sarg);
	 g_ConfigData.ClampLegsCalibration.iCountsAtZero = i;
	 }
	 logf("bfz=%d\r\n",g_ConfigData.ClampLegsCalibration.iCountsAtZero);
	 }
	 */
	//	Set / get Crawler serial number.
	else if (strcmp(scmd, "sn") == 0)
	{
		if (sarg != 0)
		{
			strcpy(g_szSerialNumber, sarg);
			StoreConfig(0);
		}
		logf("s/n %s\r\n", g_szSerialNumber);
	}
	else if (strcmp(scmd, "store") == 0)
	{
		StoreConfig(0);
	}
	else if (strcmp(scmd, "mode") == 0)
	{
		if (sarg != 0)
		{
			g_cModeState = atoi(sarg);
		}
		logf("Mode=%s\r\n", g_szModeNames[(int) g_cModeState]);
	}
	else if (strcmp(scmd, "pos") == 0)
	{
//edit
	}
	else if (strcmp(scmd, "chome") == 0)
	{
		//start clamp home
		//Home will run while it's in this UNCLAMP state, and when home is done
		//it will complete the UNCLAMP and be in the UNCLAMP state.
		/*edit
		 g_cClampHomed = HOME_RUNNING;
		 g_cHomeStateGoal = HOME_CLAMP_INIT;
		 g_cClampGoal = CLAMP_UNCLAMP; //Needs to do Unclamp After First Part Of Home
		 g_cClampState = CLAMP_UNKNOWN;
		 g_cClampGoalLastShown = 0xFF; //so it will def show whatever it is next time it starts clamp
		 */
	}
	else if (strcmp(scmd, "home") == 0)
	{
		//alternate way to run A Home
//edit		g_cMCAState = MCA_HOME;
//		MCAFindHome();
	}
	else if (strcmp(scmd, "forcehome") == 0)
	{
		//alternate way to run A Home
//edit        logf("Homed was %d. Forced to HOME_DONE\r\n",g_cClampHomed);
//edit		g_cClampHomed=HOME_DONE;
	}
	else if (strcmp(scmd, "clearhome") == 0)
	{
		//alternate way to run A Home
//edit        logf("Homed was %d. Cleared to HOME_NOT_DONE\r\n",g_cClampHomed);
//edit		g_cClampHomed=HOME_NOT_DONE;
	}
	else if (strcmp(scmd, "st") == 0)
	{
		MCStopPosition();
//edit       	g_cMotorStateGoal = MOTOR_OFF;
//edit		MCAStopPosition();
//edit		g_cMCAState = MCA_SERVO;
	}
	else if (strcmp(scmd, "aux") == 0)
	{
		g_cAuxCtrlFlag = (char) atoi(sarg);
	}
	else if (strcmp(scmd, "hold!") == 0)
	{
		g_cHold = !g_cHold;
		if (g_cHold == 1)
		{
			logf("On Hold\r\n");
		}
	}
	else if (strcmp(scmd, "io") == 0)
	{
		ClearShowIO();
		g_cShowIO = !g_cShowIO;
	}
	else if (strcmp(scmd, "ana") == 0)
	{
		ClearShowAna();
		if (sarg != 0)
		{
			//use this like a mask
			g_cShowAna = atoi(sarg);
		}
		else if (g_cShowAna == 0)
		{
			g_cShowAna = 255;
		}
		else
		{
			g_cShowAna = 0;
		}
		logf("ana %d\r\n", g_cShowAna);
	}
	else if (strcmp(scmd, "oid") == 0)
	{
		//FIXME HIGH PORT.... make this do output to confirm functon of numbers and outputs...
//		//Show OID Label Alignment
//		logf("OID_MAX_NUMBER_Crawler = %d  Label = %s\r\n",
//OLD				OID_MAX_NUMBER_Crawler, DisplayOIDName(OID_MAX_NUMBER_Crawler));//edit and test
	}
	else if (strcmp(scmd, "tm") == 0)
	{
		ServiceDateTime();
		logf("date %d %d %d %d %d %d %d\r\n", g_DateTime.uiyear,
				g_DateTime.cmonth, g_DateTime.cdayOfMonth, g_DateTime.chour,
				g_DateTime.cminute, g_DateTime.csecond,
				g_DateTime.uimillisecond);
	}
	else
	{
		//unmatched_command:
		logf("Unmatched Command: \"%s\"\r\n", scmd);
	}
	return;
}

void SocketConsoleShowIdentity()
{
	logf("%s %s sn %s\r\n", SMARTTOOL_SUBTYPE_STRING, SYSTEM_VERSION_STRING, g_szSerialNumber);
	//logf("Welcome to the system.\r\n");
}

////////////////////////////////////////////////////////////////////////////////
//	E-STOP HANDLING
////////////////////////////////////////////////////////////////////////////////

//	FIXME:	Add when a E-STOP state signal is supplied to this processor.

////////////////////////////////////////////////////////////////////////////////
// Config functions
////////////////////////////////////////////////////////////////////////////////

#warning "REWRITE TO USE CONFIG SCRIPT FUNCTIONALITY"

void create_hash(char * data, char * hashbuffer, int size)
{
	//not a real hash, but does detect size change
	//FIXME LOW  IMPLEMENT HASH... but low priority compared to script config
	memset(hashbuffer, 0, 16);
	sprintf(hashbuffer, "%d", size);
}

#define iimagesize (4 + 64 + sizeof(td_ConfigData) + 16)

int g_cLoadConfig = 0;

void LoadConfig(void)
{
	//FIXME HIGH Convt to script loading when that API is done

	// read config data from user block on flash and store it in g_ConfigData
	// if the read fails, this function will NOT destroy what was previously in g_ConfigData

	int iRetVal;
	int i;
	unsigned long ulHeader;
	td_ConfigData ConfigDataTemp;
	char hashstoredInFlash[16];
	char hashofData[16];
	char userblockbuffer[iimagesize];

	// RetVal keeps track of errors
	iRetVal = 0;

	//Read user block file
	int fd = open("userblock.bin", O_RDONLY);
	int ib = 0;
	while (ib < iimagesize)
	{
		i = read(fd, userblockbuffer + ib, sizeof(userblockbuffer) - ib);
		if (i < 0)
		{
			logf("Error reading userblock:%d\r\n", errno);
			close(fd);
			iRetVal = -1;
			goto LoadConfigShowResult;
		}
		ib += i;
		if (i == 0)
		{
			if (ib < iimagesize)
			{
				logf(
						"Error reading userblock: did not get enough bytes: %d < %d \r\n",
						ib, iimagesize);
				close(fd);
				iRetVal = -1;
				goto LoadConfigShowResult;
			}
			break;
		}
	}
	close(fd);

	// check for the unique header (0x55AA55AB) to signify that actual data exists
	memcpy((char*) &ulHeader, userblockbuffer, 4);
	// load serial number
	memcpy(g_szSerialNumber, userblockbuffer + 4, 64);
	g_szSerialNumber[63] = 0;

	if (ulHeader != 0x55AA55AB)
	{
		g_szSerialNumber[0] = 0;
		iRetVal = -1;
		goto LoadConfigShowResult;
	}

	memcpy(&ConfigDataTemp, userblockbuffer + 68, sizeof(td_ConfigData));
	memcpy(hashstoredInFlash, userblockbuffer + 68 + sizeof(td_ConfigData), 16);

	create_hash((char *) &ConfigDataTemp, hashofData, sizeof(td_ConfigData));

	for (i = 0; i < 16; i++)
	{
		if (hashofData[i] != hashstoredInFlash[i])
		{
			iRetVal = -1;
			goto LoadConfigShowResult;
		}
	}

	LoadConfigShowResult:
	if (iRetVal < 0)
	{
#ifdef USE_OUTPUT
		logf("UBreadConfig:Config Load Failed\r\n");
#endif
		g_cLoadConfig = 0;
		return;
	}
#ifdef USE_OUTPUT
	logf("UBreadConfig:Config Loaded\r\n");
#endif
	g_ConfigData = ConfigDataTemp; //copy entire structure
	g_cLoadConfig = 1;
	return;
}

void StoreConfig(td_STPsessions * p_STPSession)
{

	unsigned long ulHeader;
	char hashresult[16];
	int iRetVal;
	int ib;
	unsigned int uiMessageCode;

	iRetVal = 0;

	// write a unique header (0x55AA55AB) to signify that actual data exists
	ulHeader = 0x55AA55AB;
	// write size of current

	create_hash((char *) &g_ConfigData, hashresult, sizeof(td_ConfigData));

	int fd = open("userblock.bin", O_WRONLY | O_CREAT, 0644);
	if (fd == -1)
	{
		logf("failed open: %d", errno);
		iRetVal = -1;
		goto StoreConfigShowResult;

	}
	ib = write(fd, &ulHeader, 4);
	if (ib < 4)
	{
		iRetVal = -1;
		goto StoreConfigShowResult;
	}
	ib = write(fd, g_szSerialNumber, 64);
	if (ib < 64)
	{
		iRetVal = -1;
		goto StoreConfigShowResult;
	}
	ib = write(fd, &g_ConfigData, sizeof(td_ConfigData));
	if (ib < 64)
	{
		iRetVal = -1;
		goto StoreConfigShowResult;
	}
	ib = write(fd, hashresult, 16);
	if (ib < 16)
	{
		iRetVal = -1;
		goto StoreConfigShowResult;
	}

	StoreConfigShowResult: if (fd >= 0)
	{
		close(fd);
	}
	if (iRetVal == 0)
	{
		uiMessageCode = CrawlerMC_STOREDEFAULT_SUCCESS;
		logf("success\r\n");
	}
	else
	{
		uiMessageCode = CrawlerMC_STOREDEFAULT_FAILURE;
		logf("failure\r\n");
	}
// provide user with a message to act as feedback that the config store worked (or didn't)

	SmartToolMsgCrawlerMessageCode(p_STPSession, CRAWLER_OID_STOREDEFAULT_CONFIG, uiMessageCode);
}

void RecallConfig(td_STPsessions * p_STPSession, int irecall)
{
	unsigned int uiMessageCode;
	if (irecall == 0)
	{
		//factor defaults : reload parameters
		InitConfig();
		uiMessageCode = CrawlerMC_LOADFACTORYSETTINGS_SUCCESS;
	}
	else if (irecall == 1)
	{
		LoadConfig();
		//user block recall
		if (g_cLoadConfig == 1)
		{
			uiMessageCode = CrawlerMC_LOADDEFAULT_SUCCESS;
		}
		else
		{
			uiMessageCode = CrawlerMC_LOADDEFAULT_FAILURE;
		}
	}
	else
	{
		uiMessageCode = CrawlerMC_INVALID_PARAMETER;
	}
// provide user with a message to act as feedback that the config store worked (or didn't)
	SmartToolMsgCrawlerMessageCode(p_STPSession, CRAWLER_OID_RECALL_CONFIG, uiMessageCode);
	MCSetConfigParams(); //Set the parameters which are not set at the start of each operation.
//edit  more to init set????
}

///////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//	Mode State Machine
////////////////////////////////////////////////////////////////////////////////
#define DELAY_AFTER_ESTOP 500

void ModeStateMachine(void)
{
	char cEntryState;
	static unsigned long s_ulEstopTime = 127;
	static char s_cEstopPrev = 0;  // monitors prior estop state
//Currently no Mode State Logic
// except for estop

	cEntryState = g_cModeState;

#ifdef GEN_BC
	g_cDigInEstop = 0; //FORCE CLEAR
#endif	

//Check For Estop
	if (g_cDigInEstop == ESTOP_SIGNAL || g_cEstopMCAsserted == 1) //estop signalled, or estop asserted by MC
	{
		//Estop is signalled
		s_ulEstopTime = MS_TIMER;
		//Estop is not clear
		// ALWAYS put us into ESTOP mode when any Estop button is engaged...
		// even if the Rabbit software is trying to command us into a different state somewhere else in this application
		if (s_cEstopPrev != TRUE)
		{
			// do these actions only once when estop was pressed
			EstopEngageActions();
#ifdef USE_OUTPUT
			logf("ESTOP mode\r\n");
			logf("ESTOP_SIGNAL define = %d  DIN2=%d\r\n", ESTOP_SIGNAL, g_cDigInEstop);
#endif
		}
		s_cEstopPrev = TRUE;
		g_cModeState = ModeESTOP;
		//If the main estop is signalled, then we can clear the MCAsserted Estop signal...
		//It is no longer just asserted by the MC system
		if (g_cDigInEstop == ESTOP_SIGNAL)
		{
			g_cEstopMCAsserted = 0;
		}
		goto label_past_estop;
	}
//Estop is not signalled
	if (s_cEstopPrev != FALSE)
	{
		//Estop was previously signalled: can we exit?
		if ((MS_TIMER - s_ulEstopTime) < DELAY_AFTER_ESTOP)
		{
			//they have not been clear of one of the previous things long enough
			s_cEstopPrev = TRUE;
			g_cModeState = ModeESTOP;
			goto label_past_estop;
		}
		// estop cleared
		EstopDisengageActions();
#ifdef USE_OUTPUT
		logf("ESTOP cleared\r\n");
		logf("ESTOP_SIGNAL define = %d  DIN2=%d\r\n", ESTOP_SIGNAL, g_cDigInEstop);
#endif
		//go to ModeInit, which will then go to the right follow up mode
		g_cModeState = ModeInit;
		s_cEstopPrev = FALSE;
		//edit if (g_cClampState!=CLAMP_UNCLAMP)
		//{
		//	g_cClampGoal=CLAMP_HOLD;
		//}
	}
	label_past_estop:

	AlertModeChanges(0);

//edit
//    if (g_cSentClampHomed != g_cClampHomed)
//    {
//		logf("Sending %c Home Status %d\r\n",'C', g_cClampHomed);
//		SmartToolMsgChar(p_STPSession, STP_ALERT, OID_CLAMP_HOMED, g_cClampHomed);
//    	g_cSentClampHomed = g_cClampHomed;
//    }

//Mode Based logic
	switch ((int) g_cModeState)
	{
	case ModeUnknown:

		break;

	case ModeInit:
		//FIXME HIGH  many issues to deal with verify entry... compare with default state functions
//Why do railairchange here, and not at init of air or default of air
//where that code exists...????
//What about ESTOP wiping out INIT????
//??When ESTOP gets triggered should gripper change?

		if (g_cModeState != g_cModeStatePrev)
		{
			GripperChange(OFF); //Also called in InitDIO to set the gripper on startup.
			RailAirChange(OFF);
			g_cStanceGoal = StanceOff;
			logf("ModeInit changed g_cStanceGoal to %d \r\n", g_cStanceGoal);
		}

		break;

	case ModeESTOP:
		EstopContinueActions();
		break;

	case ModeReady:
		//Ready to move.
#warning "should it do mode specific monitoring here????"
		break;

	case ModeMove:
		//FIXME HIGH consider doing checking on stance so that failures will prevent ModeReady from continuing"
		if (g_cModeState != g_cModeStatePrev)
		{
			// if(g_cStanceGoal!=StanceMove)
			// {
			//	g_cStanceGoal=StanceMove;
			// }
			//GripperChange(ON);
			// Turn Suspension ON here when we want to use it.
		}
		if (g_cMCMoveDone != MOVEDONE_FALSE)
		{
			logf("returned to ModeReady\r\n");
			g_cModeState = ModeReady;
		}
		break;
	case ModeEngage:

		break;

	case ModePark:

		if (g_cModeState != g_cModeStatePrev)
		{
			if (g_cStanceGoal != StancePark)
			{
				g_cStanceGoal = StancePark;
			}

		}
		if (g_cStanceState == StancePark)
		{
			//Success!
			//g_cModeStatePrevious = ModePark;
			g_cModeState = ModePark;
		}
		break;

	case ModeSleep:
		if (g_cModeState != g_cModeStatePrev)
		{
			//future sleep mode development.
		}
		break;

	case ModeHalt:
		break;

	case ModeFault:
		break;

	case ModePrepareOperate:

		if (g_cModeState != g_cModeStatePrev)
		{
			//Balance is done here;
			GripperChange(ON);
			if (g_cStanceGoal != StanceEngage)  //&&(g_cStanceGoal!=StanceError))
			{
				g_cStanceGoal = StanceEngage;
			}
		}
		if (g_cStanceState == StanceEngage)
		{
			//Made it to StanceEngage
			//g_cModeStatePrevious = ModeEngage;
			g_cModeState = ModeEngage;
		}
		break;

	case ModePrepareToMove:
		if (g_cModeState != g_cModeStatePrev)
		{
			if (g_cStanceGoal != StanceMove) //&&(g_cStanceGoal!=StanceError))
			{
				g_cStanceGoal = StanceMove;
			}

		}
		if (g_cStanceState == StanceMove)
		{
			//Success!
			//g_cModeStatePrevious = ModeMove;
			g_cModeState = ModeReady;
		}
		break;

	case ModeDisEngage:
		if (g_cModeState != g_cModeStatePrev)
		{
//FIXME MED Crawler Cell Controller needs to Center ande Balance MiniFT but we didn't do that yet
			//Center done here. This should be done by the Device Controller in the Cell Controller
			//Balance done here. This should be done by the Device Controller in the Cell Controller
			if (g_cStanceGoal != StanceOff)
			{
				g_cStanceGoal = StanceOff;
			}
		}
		if (g_cStanceState == StanceOff)
		{
			//Success!
			//g_cModeStatePrevious = ModeDisEngage;
			g_cModeState = ModeDisEngage;
		}
		break;

	default:
		//	should never get here...reset to ModeUnknown
		logf("We have fallen through mode state machine\r\n");
		g_cModeState = ModeUnknown;
		break;

	}

	g_cModeStatePrev = cEntryState;
} //ModeStateMachine

////////////////////////////////////////////////////////////////////////////////
//	GripperChange
///////////////////////////////////////////////////////////////////////////////
void GripperChange(int i_GripState)
{
	if (i_GripState != ON)
	{
		i_GripState = OFF;
	}

	digOut(DIGOUT_GRIPPER_TOOL, i_GripState);
}
////////////////////////////////////////////////////////////////////////////////
//	RailAirChange
///////////////////////////////////////////////////////////////////////////////
void RailAirChange(int i_AirState)
{
	if (i_AirState != ON)
	{
		i_AirState = OFF;
	}

	digOut(DIGOUT_RAIL_AIR, i_AirState);
}

////////////////////////////////////////////////////////////////////////////////
//	Clamp axis motion control
////////////////////////////////////////////////////////////////////////////////

#define cosapprox(x) (1-(((x)*(x))/2))

void CrawlerHomeMachine()
{
#define OUTPUT_CHOME_DETAIL
//HOMING
//continue to watch these until state changes
//edit
//    while (g_cClampHomed == HOME_RUNNING) //using while only for "break" exit pattern
//    {
//	}

}

#ifdef EDGEDETECT_SYSTEM
nodebug void EdgeFoundStop()
{

	if(((anaInVolts(DIGIN_CLIFFDETECT1, 3) > CLIFFVOLTS)) && (g_iEdgeDetect == 0 ))
	{
		logf("channel 1 volts: %f \r\n", anaInVolts(DIGIN_CLIFFDETECT1, 3));
		MCStopPosition();
		g_iEdgeDetect++;
	}

	if(((anaInVolts(DIGIN_CLIFFDETECT2, 3) > CLIFFVOLTS)) && (g_iEdgeDetect == 0 ))
	{
		logf("channel 2 volts: %f \r\n", anaInVolts(DIGIN_CLIFFDETECT2, 3));
		MCStopPosition();
		g_iEdgeDetect++;

	}

	if(((anaInVolts(DIGIN_CLIFFDETECT3, 3) > CLIFFVOLTS)) && (g_iEdgeDetect == 0 ))
	{
		logf("channel 3 volts: %f \r\n", anaInVolts(DIGIN_CLIFFDETECT3, 3));
		MCStopPosition();
		g_iEdgeDetect++;
	}

	//logf("channel 4 volts: %f \r\n", anaInVolts(DIGIN_CLIFFDETECT4, 3));
	if(((anaInVolts(DIGIN_CLIFFDETECT4, 3) > CLIFFVOLTS)) && (g_iEdgeDetect == 0 ))
	{
		MCStopPosition();
		g_iEdgeDetect++;
	}

} //EdgeFoundStop

nodebug char EdgeMoveAway(float fx, float fy, float fr)
{
	float fNorthWestSensor;
	float fNorthEastSensor;
	float fSouthEastSensor;
	float fSouthWestSensor;

	char cBlockXP;
	char cBlockXN;
	char cBlockYP;
	char cBlockYN;
	char cBlockR;
	char cBlocked;

	fNorthWestSensor = anaInVolts(DIGIN_CLIFFDETECT1, 3);
	fNorthEastSensor = anaInVolts(DIGIN_CLIFFDETECT2, 3);
	fSouthWestSensor = anaInVolts(DIGIN_CLIFFDETECT3, 3);
	fSouthEastSensor = anaInVolts(DIGIN_CLIFFDETECT4, 3);

	cBlockXP=0;
	cBlockXN=0;
	cBlockYP=0;
	cBlockYN=0;
	cBlockR=0;
	cBlocked=0;

	//fNorthWestSensor is the XPYP sensor
	if(fNorthWestSensor > CLIFFVOLTS)
	{

		cBlockXP=1;
		cBlockYP=1;
		cBlockR=1;
	}
	//fNorthEastSensor is the XPYN sensor
	if(fNorthEastSensor > CLIFFVOLTS)
	{
		cBlockXP=1;
		cBlockYN=1;
		cBlockR=1;
	}
	//fSouthWestSensor is the XNYP sensor
	if (fSouthWestSensor > CLIFFVOLTS)
	{
		cBlockXN=1;
		cBlockYP=1;
		cBlockR=1;
	}

	//fSouthEastSensor is the XNYN sensor
	if (fSouthEastSensor > CLIFFVOLTS)
	{
		cBlockXN=1;
		cBlockYN=1;
		cBlockR=1;
	}

	if ((cBlockXN==1 && fx<0)||(cBlockXP==1 && fx>0)||
			(cBlockYN==1 && fy<0)||(cBlockYP==1 && fy>0)||(cBlockR==1 && fr!=0))
	{
		cBlocked=1;
	}
	return cBlocked;

} //EdgeMoveAway

nodebug char EdgeDetected()
{
	float fNorthWestSensor;
	float fNorthEastSensor;
	float fSouthEastSensor;
	float fSouthWestSensor;

	char cBlocked;

	fNorthWestSensor = anaInVolts(DIGIN_CLIFFDETECT1, 3);
	fNorthEastSensor = anaInVolts(DIGIN_CLIFFDETECT2, 3);
	fSouthWestSensor = anaInVolts(DIGIN_CLIFFDETECT3, 3);
	fSouthEastSensor = anaInVolts(DIGIN_CLIFFDETECT4, 3);

	cBlocked=0;

	//fNorthWestSensor is the XPYP sensor
	if(fNorthWestSensor > CLIFFVOLTS)
	{
		cBlocked=1;
	}
	//fNorthEastSensor is the XPYN sensor
	else if(fNorthEastSensor > CLIFFVOLTS)
	{
		cBlocked=1;
	}
	//fSouthWestSensor is the XNYP sensor
	else if (fSouthWestSensor > CLIFFVOLTS)
	{
		cBlocked=1;
	}

	//fSouthEastSensor is the XNYN sensor
	else if (fSouthEastSensor > CLIFFVOLTS)
	{
		cBlocked=1;
	}

	return cBlocked;
}

#endif //END EDGEDETECT_SYSTEM
////////////////////////////////////////////////////////////////////////////////
// Stance state machine.
////////////////////////////////////////////////////////////////////////////////

void StanceStateMachine()
{
	if (g_cStanceState != g_cStanceGoal)
	{
		if (g_cStanceGoal != g_cStanceGoalLast)
		{
			//if(g_cStanceState!=StanceError)
			//{
			g_ulStartStanceTime = MS_TIMER;
			g_cStanceState = StanceTransition;
			//}
		}

		switch ((int) g_cStanceGoal)
		{
		case StanceUnknown:
			//uiStanceError=0;
			break;

		case StanceOff:
			if (g_cStanceGoalLast != StanceOff)
			{
				logf("Goal StanceOff\r\n");
				g_cStanceGoalLast = g_cStanceGoal;
			}
			digOut(DIGOUT_LOWER_TOOL, Valve_Off_State);
			digOut(DIGOUT_LIFT_TOOL, Valve_Off_State);

			g_ulStanceTime = MS_TIMER - g_ulStartStanceTime;
			if (digIn(DIGIN_FLOAT) == DETECTED) //read digital-in.
			{

				g_cStanceState = StanceOff;
				logf("Stance State Now = %d   We made it turn off.\r\n", g_cStanceState);
			}
			else if (g_ulStanceTime > g_ConfigData.uiStanceTimeout)
			{
				g_cStanceState = StanceError;
				g_cStanceGoal = StanceError;
				g_ulStanceTime = 0;  //reset the stance timer.
				logf("Stance State Now = %d   We didn't make it to off.\r\n", g_cStanceState);
			}
			break;

		case StanceMove:
			if (g_cStanceGoalLast != StanceMove)
			{
				logf("Goal StanceMove\r\n");
				g_cStanceGoalLast = g_cStanceGoal;
			}
			if (g_iStartMoveTimer == 0)
			{
				g_ulMoveActionTime1 = MS_TIMER;
				g_iStartMoveTimer++;
			}

			if (MS_TIMER - g_ulMoveActionTime1 > iActionTimeout1)
			{
				GripperChange(ON);
			}

			if (MS_TIMER - g_ulMoveActionTime1 > iActionTimeout2)
			{
				RailAirChange(OFF);
			}

			if (MS_TIMER - g_ulMoveActionTime1 > iActionTimeout4)
			{
				//Wheels are down and Rail is up.
				digOut(DIGOUT_LOWER_TOOL, Valve_Off_State);
				digOut(DIGOUT_LIFT_TOOL, Valve_On_State);
				g_ulTempTime = MS_TIMER;
				g_ulStanceTime = g_ulTempTime - g_ulStartStanceTime;
			}

			if (((MS_TIMER - g_ulMoveActionTime1 > iActionTimeout5)
					&& (digIn(DIGIN_RAILUP) == DETECTED)
					&& (digIn(DIGIN_WHEELSDOWN) == DETECTED))
					|| (g_iMoveBypass == BYPASSON)) //read digital-in.
			{
				g_cStanceState = StanceMove;
				g_iStartMoveTimer = 0;
				logf("Stance State Now = %d  We made it Move\r\n", g_cStanceState);

			}
			else if (g_ulStanceTime > g_ConfigData.uiStanceTimeout)
			{
				g_cStanceState = StanceError;
				g_cStanceGoal = StanceError;
				g_iStartMoveTimer = 0;
				g_ulStanceTime = 0;
				logf("Stance State Now = %d   We didn't make it to Move\r\n", g_cStanceState);
			}

			break;

		case StanceEngage:
			if (g_cStanceGoalLast != StanceEngage)
			{
				logf("Goal StanceEngage\r\n");
				g_cStanceGoalLast = g_cStanceGoal;
			}
			if (g_iStartOperateTimer == 0)
			{
				g_ulOperateActionTime1 = MS_TIMER;
				g_iStartOperateTimer++;
			}

			if ((MS_TIMER - g_ulOperateActionTime1) > iActionTimeout1)
			{
				digOut(DIGOUT_LOWER_TOOL, Valve_On_State);
				digOut(DIGOUT_LIFT_TOOL, Valve_Off_State);
				g_ulTempTime = MS_TIMER;
				g_ulStanceTime = g_ulTempTime - g_ulStartStanceTime;
			}

			if ((((MS_TIMER - g_ulOperateActionTime1) > iActionTimeout3)
					&& (digIn(DIGIN_RAILDOWN) == DETECTED)
					&& (digIn(DIGIN_WHEELSUP) == DETECTED))
					|| (g_iOperateBypass == BYPASSON))
			{

				if ((MS_TIMER - g_ulOperateActionTime1) > iActionTimeout5)
				{
					logf("Stance State Now = %d  We Made it Engage\r\n", g_cStanceState);
					GripperChange(OFF);
					RailAirChange(ON);
					g_cStanceState = StanceEngage;
					g_iStartOperateTimer = 0;
				}

			}
			else if (g_ulStanceTime > g_ConfigData.uiStanceTimeout)
			{
				g_cStanceState = StanceError;
				g_cStanceGoal = StanceError;
				g_iStartOperateTimer = 0;
				g_ulStanceTime = 0;
			}
			break;

		case StancePark:
			if (g_cStanceGoalLast != StancePark)
			{
				logf("Goal StancePark\r\n");
				g_cStanceGoalLast = g_cStanceGoal;
			}
			if (g_iStartParkTimer == 0)
			{
				g_ulParkActionTime1 = MS_TIMER;
				g_iStartParkTimer++;
			}

			if (MS_TIMER - g_ulParkActionTime1 > iActionTimeout1)
			{
				digOut(DIGOUT_LOWER_TOOL, Valve_Off_State);
				digOut(DIGOUT_LIFT_TOOL, Valve_Off_State);
				RailAirChange(OFF);
				g_ulTempTime = MS_TIMER;
				g_ulStanceTime = g_ulTempTime - g_ulStartStanceTime;
			}

			if (MS_TIMER - g_ulParkActionTime1 > iActionTimeout4)
			{
				GripperChange(OFF);
				//Turn off suspension here.

			}

			if (((MS_TIMER - g_ulParkActionTime1 > iActionTimeout5)
					&& (digIn(DIGIN_FLOAT) == DETECTED))
					|| (g_iParkBypass == BYPASSON))
			{
				g_cStanceState = StancePark;
				g_iStartParkTimer = 0;
				logf("Stance State Now = %d  We made it park\r\n", g_cStanceState);
			}
			else if (g_ulStanceTime > g_ConfigData.uiStanceTimeout)
			{
				g_iStartParkTimer = 0;
				g_ulStanceTime = 0;
				g_cStanceState = StanceError;
				g_cStanceGoal = StanceError;
			}

			break;

		case StanceTransition:
			if (g_cStanceGoalLast != StanceTransition)
			{
				logf("Goal StanceTransition\r\n");
				g_cStanceGoalLast = g_cStanceGoal;
			}
			break;

		case StanceError:
			if (g_cStanceGoalLast != StanceTransition)
			{
				logf("Goal StanceError! \r\n");
				g_cStanceGoalLast = g_cStanceGoal;
			}
			g_cStanceState = StanceError;
			break;
		}

	}
	else if (g_cStanceState == g_cStanceGoal)
	{
		switch ((int) g_cStanceGoal)
		{
		case StanceUnknown:
			if (g_cStanceStateLast != StanceUnknown)
			{
				logf("StanceUnknown\r\n");
				g_cStanceStateLast = g_cStanceState;
			}
			break;

		case StanceOff:
			if (g_cStanceStateLast != StanceOff)
			{
				logf("StanceOff\r\n");
				g_cStanceStateLast = g_cStanceState;
			}
			if (digIn(DIGIN_FLOAT) != DETECTED) //read digital-in.
			{
				g_cStanceGoal = StanceError;
				logf("!!Off Fail\r\n");
			}

			break;

		case StanceMove:
			if (g_cStanceStateLast != StanceMove)
			{
				logf("StanceMove\r\n");
				g_cStanceStateLast = g_cStanceState;
			}
			if ((digIn(DIGIN_RAILUP) != DETECTED) || (digIn(DIGIN_WHEELSDOWN) != DETECTED)) //read digital-in.
			{
				g_cStanceGoal = StanceError;
				logf("!!Move Fail\r\n");
			}

			break;

		case StanceEngage:
			if (g_cStanceStateLast != StanceEngage)
			{
				logf("StanceMove\r\n");
				g_cStanceStateLast = g_cStanceState;
			}
			if ((digIn(DIGIN_RAILDOWN) != DETECTED) || (digIn(DIGIN_WHEELSUP) != DETECTED)) //read digital-in.
			{
				g_cStanceGoal = StanceError;
				logf("!!Engage Fail\r\n");
			}

			break;

		case StancePark:
			if (g_cStanceStateLast != StancePark)
			{
				logf("StancePark\r\n");
				g_cStanceStateLast = g_cStanceState;
			}
			if ((digIn(DIGIN_FLOAT) != DETECTED))
			{
				g_cStanceGoal = StanceError;
				logf("!!Park Fail\r\n");
			}

			break;

		case StanceTransition:
			if (g_cStanceStateLast != StanceTransition)
			{
				logf("StanceTransition\r\n");
				g_cStanceStateLast = g_cStanceState;
			}
			break;

		case StanceError:
			if (g_cStanceStateLast != StanceError)
			{
				logf("StanceError!!!!!!!!!!!\r\n");
				g_cStanceStateLast = g_cStanceState;
			}
			g_cStanceState = StanceError;
			break;

		default:
			//	should never get here...reset to ModeUnknown
			g_cStanceState = ModeUnknown;
			logf("We have fallen through the stance state machine. \r\n");
			break;
		}
	}

}

////////////////////////////////////////////////////////////////////////////////
//	Main
////////////////////////////////////////////////////////////////////////////////
int mainSAMPLE(void)
{
//int i;
//int iResult;
//long sz;
	uint32 uiMainLoopTime;

	InitSocketConsole();

	InitDIO();
//Due to boot int with legs CLAMP_OFF and legs goal CLAMP_OFF, no change occurs, which is what we want.
//but we need to set these to these values so that the 1st command that occurs will be honored
	g_cLegsExtend = 0xFF;
	g_cLegsRetract = 0xFF;
	g_cLegsLock = 0xFF;

	logf(SMARTTOOL_SUBTYPE_STRING " " SYSTEM_VERSION_STRING);

	STPInit(SmartToolTypeCrawler, CRAWLER_OID_MAX_NUMBER, pszOIDNamesCrawler);

	InitListenSocketConsole();
//wait several sec or until we have a SocketConsole
#if 0
	logf("Wait up to 10 sec for SocketConsole\r\n");
	SocketConsoleWaitConnect(10000);
#endif

	logf(SMARTTOOL_SUBTYPE_STRING " " SYSTEM_VERSION_STRING);

	InitVars();
	InitConfig();

//	Load Config
	LoadConfig();
//FIXME Load Config will be updated to read the script later... currently reads a file that is like user block

//	whatever does NOT load in LoadProfile will retain "factory defaults" as previously specified in InitVars()

//Init Motion Control System
	CrawlerWhistleInit();

	g_cModeState = ModeInit; //still init until it goes to Estop or 1st call to ModeStateMachine

//edit	InitClampLegsStateMachine();
	int ms_hits[100];
	int msover_hits = 0;
	memset(ms_hits, 0, 100 * sizeof(int));
	uint32 uimaxdiff = -1;
	uint32 uiStatDisplayTime = MS_TIMER;
	while (1)
	{
		uiMainLoopTime = MS_TIMER;

		ServiceSTP();

		ReadDigitalInputs();

		CheckSocketConsole();
		RxSocketConsole();
		if (g_cHold)
		{
			goto show_io;
		}

		ModeStateMachine();
		//edit ClampLegsStateMachine();

		StanceStateMachine();
#ifdef EDGEDETECT_SYSTEM
		if (g_cUseEdgeDetect==1)
		{
			EdgeFoundStop();
		}
#endif

		CrawlerHomeMachine();

		CrawlerWhistleService();

		show_io: if (g_cShowIO != 0)
		{
			ShowIO();
		}
		if (g_cShowAna != 0)
		{
			ShowAna();
		}

		uint32 uidiff = MS_TIMER - uiMainLoopTime;
		if (uidiff < 100)
		{
			ms_hits[uidiff]++;
		}
		else
		{
			msover_hits++;
			if (uimaxdiff < uidiff)
			{
				uimaxdiff = uidiff;
			}
			logf("!ml %u m %u n>=100 %d\r\n", uidiff, uimaxdiff, msover_hits);
		}
		//FIXME HIGH  use new tiered timer system that allows more accuracy for measurement.
		//make program to measure timing accuracy...

		//EVERY MINUTE SHOW THE STATS FOR LOOP TIMES < 100 sec
		if ((MS_TIMER - uiStatDisplayTime) > 60000)
		{
			uiStatDisplayTime = MS_TIMER;
			int ind = 0;
			while (ind < 100)
			{
				if (ms_hits[ind] > 0)
				{
					printf("%d\t%d\r\n", ind, ms_hits[ind]);
				}
				ms_hits[ind] = 0;
				ind++;
			}
			if (msover_hits > 0)
			{
				printf("%d >100 <=since boot\r\n", msover_hits);
			}
		}
	}
}

//UTILS
int _catoerror;

int catoi(char * pstr)
{
	char **tail;
	int i;	//FIXMEPORT better
	i = (int) strtol(pstr, tail, 10);
	return i;
}

long catol(char * pstr)
{
	char **tail;
	long l;
	l = strtol(pstr, tail, 10);
	return l;
}

unsigned long catoul(char * pstr)
{
	char **tail;
	unsigned long ul;
	ul = (unsigned long) strtol(pstr, tail, 10);
	return ul;
}

float catof(char * pstr)
{
	char **tail;
	float f;
#ifndef USE_NEW_IO
	f = strtod(pstr, tail);
#else
	f = _n_strtod(pstr,tail);
#endif
	return f;
}

//	Find offset of a character in string of specified length; return -1 if not found.
int strnpos(char *s, int c, int iLen)
{
	int i;
	int ipos;

	i = 0;
	ipos = -1;
	while ((i < iLen) && (ipos < 0))
	{
		if ((*s) == c)
			ipos = i;
		s++;
		i++;
	}
	return (ipos);
}

////////////////////////////////////////////////////////////////////////////////
//	STP Smart Tool Protocol callback functions
////////////////////////////////////////////////////////////////////////////////

//	Don't Delete.  Required by STP.LIB
void SessionStarting(td_STPsessions * p_STPSession)
{
	logf("sstart\r\n");
}

//	Don't Delete.  Required by STP.LIB
void SessionDelay(td_STPsessions * p_STPSession)
{
//edit the one case where I still want to stop a jog	StopJog();
	logf("sdelay\r\n");
}

//	Don't Delete.  Required by STP.LIB
void SessionClosing(td_STPsessions * p_STPSession)
{
//edit the one case where I still want to stop a jog	StopJog();
	logf("sclosing\r\n");
}

// STP UTIL SECTION

////////////////////////////////////////////////////////////////////////////////
// STP: BuildCrawlerMessageCode
// builds an STP_ALERT using OID_Crawler_MESSAGE_CODE
////////////////////////////////////////////////////////////////////////////////
void SmartToolMsgCrawlerMessageCode(td_STPsessions * p_STPSession, unsigned int uiOID, unsigned int uiMessageCode)
{
	unsigned int ui;
	td_oid_crawler_message_code * p_oid_crawler_message_code;

#ifdef USE_OUTPUT
	logf("STP CrawlerMC: src=%s mc=%d\r\n", DisplayOIDName(p_STPSession, uiOID), uiMessageCode);
#endif
#ifdef PACKET_FLOOD_MONITOR
	CheckPacketFlood(OID_MINIFT_MESSAGE_CODE);
#endif

	g_STPtxMsg.uiVersion = htons(STP_VERSION);
	g_STPtxMsg.uiMsgType = htons(STP_ALERT);
	g_STPtxMsg.uiOID = htons(CRAWLER_OID_CRAWLER_MESSAGE_CODE);
	g_STPtxMsg.uiValueLength = htons(sizeof(td_oid_crawler_message_code));

	p_oid_crawler_message_code = (td_oid_crawler_message_code *) &g_STPtxMsg.p_cObjectValue;
	p_oid_crawler_message_code->uiOID = htons(uiOID);
	p_oid_crawler_message_code->uiCode = htons(uiMessageCode);

	ui = (STP_HEADERSIZE + sizeof(td_oid_crawler_message_code));
	SendSTPmsg(p_STPSession, &g_STPtxMsg, ui);
}

//#define SHOW_CALC_VERBOSE

void temp_move_code(unsigned int uimovenum, float dx, float dy, float frotate, float fspeed, float frotateSpeed, float facceleration)
{
	float d;
	//float mx;
	//float my;
	float m1;
	float m2;
	float m3;
	float m4;
	float mmax;
	float maxspeedlimit;
	float ratio;
	float s1, s2, s3, s4;
	float ac, ac1, ac2, ac3, ac4;
	float dc, dc1, dc2, dc3, dc4;
	float x;
	float y;
	char caxis;

//move dx dy inches total.
//do this at the speed indicated...
	x = dx;
	y = dy;
#ifdef SHOW_CALC_VERBOSE
	logf("xy %f %f\r\n", x, y);
#endif
	if (dx != 0 || dy != 0)
	{
		//find vector length and then normalize for the purpose or relative motor velocity
		d = sqrt(dx * dx + dy * dy);
		if (d > 0)
		{
			//normalize this vector for relative motor speeds
			x = dx / d;
			y = dy / d;
		}
		else
		{
		}
	}

//to drive in x, just rotate that way
	m1 = x;
	m2 = x;
//to drive in y+  m1 goes -, but m2 goes +
	m1 -= y;
	m2 += y;
#ifdef SHOW_CALC_VERBOSE
	logf("a %f %f %f\r\n", m1, m2, fspeed);
#endif
	m1 = m1 * fspeed;
	m2 = m2 * fspeed;

//For XY Motion without rotation (so far)
//m3 is the same as m2 and m4 is the same as m1
//   ( The Diagonal motors move the same speed )
//   M1    M2
//   M3    M4
	m3 = m2;
	m4 = m1;

//Add Rotation
//Convert Degrees per sec to inches per sec at rotation radius
//360 degress/sec = PI*2*R inches sec around circumfrence
// X dps / 360 degrees = rotations / sec
// rotations / sec = PI*2*R inches sec
#ifdef SHOW_CALC_VERBOSE
	logf("r %f\r\n",frotateSpeed);
#endif
	frotateSpeed = (3.14159 / 180) * g_ConfigData.fRotationRadius * frotateSpeed;
#ifdef SHOW_CALC_VERBOSE
	logf("r %f\r\n",frotateSpeed);
#endif
//To Rotate Positive m2 and m4 go forward, but m1 and m3 go backwards
//To Rotate Negative m2 and m4 go backwards, but m1 and m3 go forwards
	m2 += frotate * frotateSpeed;
	m4 += frotate * frotateSpeed;
	m1 -= frotate * frotateSpeed;
	m3 -= frotate * frotateSpeed;

//Speeds must be scaled so that no motor moves more than the max speed
	mmax = fabs(m1);
	if (mmax < fabs(m2))
	{
		mmax = fabs(m2);
	}
	if (mmax < fabs(m3))
	{
		mmax = fabs(m3);
	}
	if (mmax < fabs(m4))
	{
		mmax = fabs(m4);
	}

	maxspeedlimit = g_ConfigData.fMaxSpeed;
	if (mmax > maxspeedlimit)
	{
		ratio = (maxspeedlimit / mmax);
		m1 *= ratio;
		m2 *= ratio;
		m3 *= ratio;
		m4 *= ratio;
	}

	if (facceleration == 0)
	{
		ac = g_ConfigData.fMaxAcc;
		dc = g_ConfigData.fMaxDec;
	}
	else
	{
		ac = fabs(facceleration);
		dc = ac;
		if (ac > g_ConfigData.fMaxAcc)
		{
			ac = g_ConfigData.fMaxAcc;
		}
		if (dc > g_ConfigData.fMaxDec)
		{
			dc = g_ConfigData.fMaxDec;
		}
	}

//Do do a move like this, let m1 be distance (signed) and fabs(m1) be the speed

//logf("b %f %f %f %f %f\r\n",m1,m2,m3,m4,mf);
	s1 = fabs(m1);
	s2 = fabs(m2);
	s3 = fabs(m3);
	s4 = fabs(m4);

	ac1 = ac * (s1 / mmax);
	ac2 = ac * (s2 / mmax);
	ac3 = ac * (s3 / mmax);
	ac4 = ac * (s4 / mmax);

	dc1 = dc * (s1 / mmax);
	dc2 = dc * (s2 / mmax);
	dc3 = dc * (s3 / mmax);
	dc4 = dc * (s4 / mmax);

#ifdef SHOW_CALC_VERBOSE
	logf("d %f %f %f %f \r\n", m1, m2, m3, m4);
	logf("s %f %f %f %f \r\n", s1, s2, s3, s4);
	logf("a %f %f %f %f \r\n", ac1, ac2, ac3, ac4);
	logf("d %f %f %f %f \r\n", dc1, dc2, dc3, dc4);
#endif

//Start Move With Primary Move Command Function MCRunMove
	MCRunMove(uimovenum, m1, m2, m3, m4,
			s1, s2, s3, s4,
			ac1, ac2, ac3, ac4,
			dc1, dc2, dc3, dc4);

//find longest distance for status (in this case s1/sx is proportional to d1/dx execpt s1 is >=0)
	d = s1;
	caxis = 0;
	if (d < s2)
	{
		d = s2;
		caxis = 1;
	}
	else if (d < s3)
	{
		d = s3;
		caxis = 2;
	}
	else if (d < s4)
	{
		d = s4;
		caxis = 3;
	}

//Send Start Move Status right now
//Also stop sending back any status for any moves prior to this move now. (see SendMoveStatus code in CrawlerWhistle.lib)
	g_MoveStatus.uimoveNumber = uimovenum;
	g_MoveStatus.caxis = caxis;
	g_MoveStatus.cstage = Start;
	g_MoveStatus.ftime = 0;
	g_MoveStatus.feta = 0; //no estimate yet
	g_MoveStatus.fdistRemain = d; //no value yet
	g_MoveStatus.fdistTotal = d; //no value yet
	g_uiMoveNumEchoM1 = 0;
	g_uiMoveNumEchoM2 = 0;
	g_uiMoveNumEchoM3 = 0;
	g_uiMoveNumEchoM4 = 0;
	SendMoveStatus(0);
}

void temp_move_axis_code(td_oid_move_axis_float * p_oid_move_axis_float)
{
//	typedef struct {
//		char cmoveType;
//		char caux;
//		unsigned int uimovenumber;
//		float fdist1;
//		float fdist2;
//		float fdist3;
//		float fdist4;
//		float fspeed;
//		float facceleration;
//	} td_oid_move_axis_float;

	float d1, d2, d3, d4;
	float s, s1, s2, s3, s4;
	float ac, ac1, ac2, ac3, ac4;
	float dc, dc1, dc2, dc3, dc4;
	float md;
	float r1, r2, r3, r4;
	char caxis;

	d1 = p_oid_move_axis_float->fdist1;
	d2 = p_oid_move_axis_float->fdist2;
	d3 = p_oid_move_axis_float->fdist3;
	d4 = p_oid_move_axis_float->fdist4;
	s = p_oid_move_axis_float->fspeed;
	ac = p_oid_move_axis_float->facceleration;

//Limit Max Speed
	s = fabs(s);
	if (s > g_ConfigData.fMaxSpeed)
	{
		s = g_ConfigData.fMaxSpeed;
	}

	if (ac == 0)
	{
		ac = g_ConfigData.fMaxAcc;
		dc = g_ConfigData.fMaxDec;
	}
	else
	{
		ac = fabs(ac);
		dc = ac;
		if (ac > g_ConfigData.fMaxAcc)
		{
			ac = g_ConfigData.fMaxAcc;
		}
		if (dc > g_ConfigData.fMaxDec)
		{
			dc = g_ConfigData.fMaxDec;
		}
	}

//Find Longest Distance
	md = fabs(d1);
	caxis = 0;
	if (md < fabs(d2))
	{
		md = fabs(d2);
		caxis = 1;
	}
	if (md < fabs(d3))
	{
		md = fabs(d3);
		caxis = 2;
	}
	if (md < fabs(d4))
	{
		md = fabs(d4);
		caxis = 3;
	}
//Speeds must be scaled so that longest move goes at speed s, and others complete at the same time.
	r1 = fabs(d1 / md);
	r2 = fabs(d2 / md);
	r3 = fabs(d3 / md);
	r4 = fabs(d4 / md);

	s1 = s * r1;
	s2 = s * r2;
	s3 = s * r3;
	s4 = s * r4;

	ac1 = ac * r1;
	ac2 = ac * r2;
	ac3 = ac * r3;
	ac4 = ac * r4;

	dc1 = dc * r1;
	dc2 = dc * r2;
	dc3 = dc * r3;
	dc4 = dc * r4;

#ifdef SHOW_CALC_VERBOSE
	logf("d %f %f %f %f \r\n", d1, d2, d3, d4);
	logf("s %f %f %f %f \r\n", s1, s2, s3, s4);
	logf("a %f %f %f %f \r\n", ac1, ac2, ac3, ac4);
	logf("d %f %f %f %f \r\n", dc1, dc2, dc3, dc4);
#endif

	MCRunMove(p_oid_move_axis_float->uimoveNumber,
			d1, d2, d3, d4,
			s1, s2, s3, s4,
			ac1, ac2, ac3, ac4,
			dc1, dc2, dc3, dc4);

//Send Start Move Status right now
//Also stop sending back any status for any moves prior to this move now. (see SendMoveStatus code in CrawlerWhistle.lib)
	g_MoveStatus.uimoveNumber = p_oid_move_axis_float->uimoveNumber;
	g_MoveStatus.caxis = caxis;
	g_MoveStatus.cstage = Start;
	g_MoveStatus.ftime = 0;
	g_MoveStatus.feta = 0; //no estimate yet
	g_MoveStatus.fdistRemain = md;
	g_MoveStatus.fdistTotal = md;
	g_uiMoveNumEchoM1 = 0;
	g_uiMoveNumEchoM2 = 0;
	g_uiMoveNumEchoM3 = 0;
	g_uiMoveNumEchoM4 = 0;
	SendMoveStatus(0);
}

int HandleSmartToolMsg(td_STPmsg* p_STPrxMsg, td_STPsessions * p_STPSession)
{
//	a reference to the socket
//	tcp_Socket* p_Socket;
//	declare variables for transmitting STP messages (responses, alerts)
	char* p_cSTPobjValBuff;

	unsigned int uiOID;
	unsigned int ui;

#ifdef EDGEDETECT_SYSTEM
	//cBlockedMove is used to determine if a move can be made based on direction of edge.
	char cBlockedMove;
#endif

	int i;
	float f;
	//char c;

	//FIXME GET AND SET
//MakeOID-generated:: GET AND SET DECLARATIONS (oid complete)
	td_StpStatus * p_StpStatus;
//td_oid_common_message_code * p_oid_common_message_code;
//	td_oid_direct_dout * p_oid_direct_dout;
//	td_oid_direct_din * p_oid_direct_din;
//td_oid_crawler_message_code * p_oid_crawler_message_code;
	td_VelErrLimit * p_VelErrLimit;
	td_Velocity * p_Velocity;
	td_Fault * p_Fault;
	td_oid_move_float * p_oid_move_float;
	td_oid_move_axis_float * p_oid_move_axis_float;
	td_oid_move_float_beta * p_oid_move_float_beta;
	td_MoveStatus * p_MoveStatus;
//MakeOID-generated::END

//FIXME0000000000 must compare this to MiniFt Version and see if I need more code.
//Since the core of Crawler is based on the MiniFT STP pattern, I must make sure than
//any additional details the MiniFT takes care of are taken care of here.

	p_cSTPobjValBuff = g_STPtxMsg.p_cObjectValue; //for output value, reference the value location in STP tx

	uiOID = p_STPrxMsg->uiOID;

#ifdef OUTPUT_RXSTP
#ifndef SHOW_STP_HEARTBEAT
	if (uiOID != OID_NULLOID)
#endif
	{
//      logf("Handle STP msg : v%d %d-%s %d-%s %d \r\n", p_STPrxMsg->uiVersion, p_STPrxMsg->uiMsgType, DisplayMessageTypeName(p_STPrxMsg->uiMsgType), p_STPrxMsg->uiOID, DisplayOIDName(p_STPrxMsg->uiOID), p_STPrxMsg->uiValueLength);
		logf("Handle STP msg : %d-%s %d-%s %d \r\n", p_STPrxMsg->uiMsgType,
				DisplayMessageTypeName(p_STPrxMsg->uiMsgType),
				p_STPrxMsg->uiOID, "X", p_STPrxMsg->uiValueLength);
		//logf("STPrx %u\r\n",uiOID);
	}
#endif

	switch (p_STPrxMsg->uiMsgType)
	{
////////////////////////////////////////////////////////////////////////
//	GET GET GET GET GET GET GET GET GET GET GET GET GET GET GET GET GET
////////////////////////////////////////////////////////////////////////
	case STP_GET:
		switch (uiOID)
		{
		case COMMON_OID_NULLOID:
			SmartToolMsgEmpty(p_STPSession, STP_GET_RESP, uiOID);
			break;
		case COMMON_OID_DEBUG:
			break;
		case COMMON_OID_SMARTTOOL_TYPE:
			SmartToolMsgStr(p_STPSession, STP_GET_RESP, uiOID, SMARTTOOL_TYPE_STRING);
			break;
		case COMMON_OID_SMARTTOOL_SUBTYPE:
			SmartToolMsgStr(p_STPSession, STP_GET_RESP, uiOID, SMARTTOOL_SUBTYPE_STRING);
			break;
		case COMMON_OID_SYSTEM_VERSION:
			SmartToolMsgStr(p_STPSession, STP_GET_RESP, uiOID, SMARTTOOL_SUBTYPE_STRING " " SYSTEM_VERSION_STRING);
			break;
		case COMMON_OID_SERIAL_NUMBER:
			SmartToolMsgStr(p_STPSession, STP_GET_RESP, uiOID, g_szSerialNumber);
			break;
		case COMMON_OID_SCRIPT_TRANSFER:
			break;
		case COMMON_OID_GENERICMESSAGE:
			break;
		case COMMON_OID_DEBUGMESSAGE:
			break;
		case COMMON_OID_STPSTATUS:
			p_StpStatus = (td_StpStatus *) p_cSTPobjValBuff;
			p_StpStatus->uiOID = htons(g_StpStatus.uiOID);
			p_StpStatus->uiStatus = htons(g_StpStatus.uiStatus);
			SmartToolMsg(p_STPSession, STP_GET_RESP, uiOID, sizeof(td_StpStatus), p_cSTPobjValBuff);
			break;
		case COMMON_OID_COMMON_MESSAGE_CODE:
			break;
		case COMMON_OID_DIRECT_DOUT:
			{
			//td_oid_direct_dout * p_oid_direct_dout = (td_oid_direct_dout *) p_cSTPobjValBuff;
			//FIXME FillStructWithData(p_oid_direct_dout);
			//p_oid_direct_dout->cchannel = p_oid_direct_dout->cchannel;
			//p_oid_direct_dout->cvalue = p_oid_direct_dout->cvalue;
			//SmartToolMsg(p_STPSession, STP_GET_RESP, uiOID, sizeof(td_oid_direct_dout), p_cSTPobjValBuff);
		}
			break;
		case COMMON_OID_DIRECT_DIN:
			{
			td_oid_direct_din * p_oid_direct_din = (td_oid_direct_din *) p_cSTPobjValBuff;
			p_oid_direct_din->cchannel = 1;
			p_oid_direct_din->cvalue = digIn(p_oid_direct_din->cchannel);
			SmartToolMsg(p_STPSession, STP_GET_RESP, uiOID, sizeof(td_oid_direct_din), p_cSTPobjValBuff);
		}
			break;
		case COMMON_OID_COM_VERSION:
			SmartToolMsgUInt16(p_STPSession, STP_GET_RESP, uiOID, COM_VERSION);
			break;
			//case COMMON_OID_STREAM_SYNC: NO GET, but can alert
			//case COMMON_OID_STP_CLOSE: NO GET, but can alert
		case COMMON_OID_DATE_TIME:
			HandleCommonOIDDateTimeGet(p_STPSession);
			break;
			//end common OIDs
			//Crawler OIDs
		case CRAWLER_OID_MODE:
			SmartToolMsgChar(p_STPSession, STP_GET_RESP, uiOID, g_cModeState);
			break;
		case CRAWLER_OID_MODE_GOAL:
			SmartToolMsgChar(p_STPSession, STP_GET_RESP, uiOID, g_cModeStateGoal);
			break;
		case CRAWLER_OID_ACTION:
			SmartToolMsgChar(p_STPSession, STP_GET_RESP, uiOID, g_cAction);
			break;
		case CRAWLER_OID_CRAWLER_MESSAGE_CODE:
			break;
		case CRAWLER_OID_LIMITS_AND_OBSTRUCTIONS:
			//FIXME FUTURE MINOR consider putting this in
			//					SmartToolMsgULong(STP_GET_RESP, uiOID, xvarname);
			break;
		case CRAWLER_OID_ESTOP_CLEAR_DELAY:
			SmartToolMsgUInt16(p_STPSession, STP_GET_RESP, uiOID, g_ConfigData.uiEStopClearDelay);
			break;
		case CRAWLER_OID_STANCE_STATE:
			SmartToolMsgChar(p_STPSession, STP_GET_RESP, uiOID, g_cStanceState);
			break;
		case CRAWLER_OID_STANCE_GOAL:
			SmartToolMsgChar(p_STPSession, STP_GET_RESP, uiOID, g_cStanceGoal);
			break;
		case CRAWLER_OID_STANCE_TIMEOUT:
			SmartToolMsgUInt16(p_STPSession, STP_GET_RESP, uiOID, g_ConfigData.uiStanceTimeout);
			break;
		case CRAWLER_OID_AUX_DATA_TRANSFER_PORT:
			//edit					SmartToolMsgUInt(p_STPSession, STP_GET_RESP, uiOID, xvarname);
			break;
		case CRAWLER_OID_ENCODER_RATIO:
			SmartToolMsgFloat(p_STPSession, STP_GET_RESP, uiOID, g_ConfigData.fEncoderRatio);
			break;
		case CRAWLER_OID_CURRENT_LIMITS:
			SmartToolMsgFloat(p_STPSession, STP_GET_RESP, uiOID, g_ConfigData.fCurrentLimit);
			break;
		case CRAWLER_OID_PEAK_CURRENT_LIMITS:
			SmartToolMsgFloat(p_STPSession, STP_GET_RESP, uiOID, g_ConfigData.fPeakCurrentLimit);
			break;
		case CRAWLER_OID_MAX_SPEED:
			SmartToolMsgFloat(p_STPSession, STP_GET_RESP, uiOID, g_ConfigData.fMaxSpeed);
			break;
		case CRAWLER_OID_MAX_ACCEL:
			SmartToolMsgFloat(p_STPSession, STP_GET_RESP, uiOID, g_ConfigData.fMaxAcc);
			break;
		case CRAWLER_OID_MAX_DECEL:
			SmartToolMsgFloat(p_STPSession, STP_GET_RESP, uiOID, g_ConfigData.fMaxDec);
			break;
		case CRAWLER_OID_MAX_STOP_DECEL:
			SmartToolMsgFloat(p_STPSession, STP_GET_RESP, uiOID, g_ConfigData.fMaxStopDec);
			break;
		case CRAWLER_OID_POSNERR_LIMIT:
			SmartToolMsgFloat(p_STPSession, STP_GET_RESP, uiOID, g_ConfigData.fPosnErrLimit);
			break;
		case CRAWLER_OID_VELERR_LIMIT:
			p_VelErrLimit = (td_VelErrLimit *) p_cSTPobjValBuff;
			p_VelErrLimit->fVLimitMargin = g_ConfigData.VelErrLimit.fVLimitMargin;
			p_VelErrLimit->fVError = g_ConfigData.VelErrLimit.fVError;
			SmartToolMsg(p_STPSession, STP_GET_RESP, uiOID, sizeof(td_VelErrLimit), p_cSTPobjValBuff);
			break;
		case CRAWLER_OID_POSNMODE_TOLERANCE:
			SmartToolMsgFloat(p_STPSession, STP_GET_RESP, uiOID, g_ConfigData.fPosnTolerance);
			break;
		case CRAWLER_OID_MAX_ROTATION_SPEED:
			SmartToolMsgFloat(p_STPSession, STP_GET_RESP, uiOID, g_ConfigData.fMaxRotationSpeed);
			break;
		case CRAWLER_OID_ROTATION_RADIUS:
			SmartToolMsgFloat(p_STPSession, STP_GET_RESP, uiOID, g_ConfigData.fRotationRadius);
			break;
		case CRAWLER_OID_BRAKE_ON_TIMEOUT:
			SmartToolMsgUInt16(p_STPSession, STP_GET_RESP, uiOID, g_ConfigData.uiBrakeOnTimeout);
			break;
		case CRAWLER_OID_VELOCITY:
			p_Velocity = (td_Velocity *) p_cSTPobjValBuff;
			p_Velocity->fdX = g_Velocity.fdX;
			p_Velocity->fdY = g_Velocity.fdY;
			p_Velocity->fCurve = g_Velocity.fCurve;
			p_Velocity->fdHeading = g_Velocity.fdHeading;
			SmartToolMsg(p_STPSession, STP_GET_RESP, uiOID, sizeof(td_Velocity), p_cSTPobjValBuff);
			break;
		case CRAWLER_OID_JOG_ENABLE_TIMEOUT:
			SmartToolMsgUInt16(p_STPSession, STP_GET_RESP, uiOID, g_ConfigData.uiJogEnableTimeout);
			break;
		case CRAWLER_OID_FAULT:
			p_Fault = (td_Fault *) p_cSTPobjValBuff;
			p_Fault->lerrorCode = htonl(g_Fault.lerrorCode);
			p_Fault->lmotorfaultCode = htonl(g_Fault.lmotorfaultCode);
			p_Fault->caxis = g_Fault.caxis;
			p_Fault->cseverity = g_Fault.cseverity;
			SmartToolMsg(p_STPSession, STP_GET_RESP, uiOID, sizeof(td_Fault), p_cSTPobjValBuff);
			break;
		case CRAWLER_OID_MOVE_STATUS:
			p_MoveStatus = (td_MoveStatus *) p_cSTPobjValBuff;
			p_MoveStatus->uimoveNumber = htons(g_MoveStatus.uimoveNumber);
			p_MoveStatus->caxis = g_MoveStatus.caxis;
			p_MoveStatus->cstage = g_MoveStatus.cstage;
			p_MoveStatus->ftime = g_MoveStatus.ftime;
			p_MoveStatus->feta = g_MoveStatus.feta;
			p_MoveStatus->fdistRemain = g_MoveStatus.fdistRemain;
			p_MoveStatus->fdistTotal = g_MoveStatus.fdistTotal;
			SmartToolMsg(p_STPSession, STP_GET_RESP, uiOID, sizeof(td_MoveStatus), p_cSTPobjValBuff);
			break;
//MakeOID-generated::END

		}
		break;
		////////////////////////////////////////////////////////////////////////
		//	SET SET SET SET SET SET SET SET SET SET SET SET SET SET SET SET SET
		////////////////////////////////////////////////////////////////////////
	case STP_SET:
		switch (p_STPrxMsg->uiOID)
		{
		case COMMON_OID_NULLOID:
			break;
		case COMMON_OID_DEBUG:
			break;
		case COMMON_OID_SERIAL_NUMBER:
			ui = p_STPrxMsg->uiValueLength;
			if (ui > 64 - 1)
			{
				SmartToolMsgCommonMessageCode(p_STPSession, uiOID, COMMONMC_BADLENGTH);
				break;
			}
			if (ui == 1 && p_STPrxMsg->p_cObjectValue[0] == 'c')
			{
				//special code means to clear the serial number
				g_szSerialNumber[0] = 0;
			}
			else if (g_szSerialNumber[0] == 0)
			{
				//if the serial number is clear, we can write anything into it
				memcpy(g_szSerialNumber, p_STPrxMsg->p_cObjectValue, ui);
				g_szSerialNumber[ui] = 0;
			}
			SmartToolMsgStr(0, STP_ALERT, COMMON_OID_SERIAL_NUMBER, g_szSerialNumber);
			break;
		case COMMON_OID_RESET_SYSTEM:
			break;
		case COMMON_OID_SCRIPT_TRANSFER:
			break;
		case COMMON_OID_GENERICMESSAGE:
			break;
		case COMMON_OID_DEBUGMESSAGE:
			break;
		case COMMON_OID_CONTROL_DELAY:
			break;
		case COMMON_OID_DIRECT_DOUT:
			{
			td_oid_direct_dout * p_oid_direct_dout;
			p_oid_direct_dout = (td_oid_direct_dout *) p_STPrxMsg->p_cObjectValue;
			if (p_oid_direct_dout->cchannel < 16)
			{
				if (p_oid_direct_dout->cvalue != 0)
				{
					p_oid_direct_dout->cvalue = 1;
				}
				logf("S%d=%d\r\n", p_oid_direct_dout->cchannel, p_oid_direct_dout->cvalue);
				digOut(p_oid_direct_dout->cchannel, p_oid_direct_dout->cvalue);
			}
		}
			break;
		case COMMON_OID_DIRECT_DIN:
			{
			td_oid_direct_din * p_oid_direct_din;
			p_oid_direct_din = (td_oid_direct_din *) p_STPrxMsg->p_cObjectValue;
			p_oid_direct_din->cvalue = digIn(p_oid_direct_din->cchannel);
			logf("I%d=%d\r\n", p_oid_direct_din->cchannel, p_oid_direct_din->cvalue);
		}
			break;
		case COMMON_OID_STREAM_SYNC:
			//Echo Back same value
			SmartToolMsg(p_STPSession, STP_ALERT, uiOID, 2, p_STPrxMsg->p_cObjectValue);
			//NTOHS value for display here
			ui = (unsigned int) ntohs(*((int*) (p_STPrxMsg->p_cObjectValue)));
			logf("STREAM_SYNC %u\r\n", ui);
			break;
		case COMMON_OID_STP_CLOSE:
			i = p_STPrxMsg->p_cObjectValue[0];
			//FIXME MED should close this side now.
			logf("STP_CLOSE %d\r\n", i);
			break;
		case COMMON_OID_STP_IDENTIFY:
			//The New Identify
			SmartToolMsgStr(p_STPSession, STP_ALERT, COMMON_OID_SMARTTOOL_TYPE, SMARTTOOL_TYPE_STRING);
			SmartToolMsgStr(p_STPSession, STP_ALERT, COMMON_OID_SMARTTOOL_SUBTYPE, SMARTTOOL_SUBTYPE_STRING);
			SmartToolMsgStr(p_STPSession, STP_ALERT, COMMON_OID_SYSTEM_VERSION, SMARTTOOL_SUBTYPE_STRING " " SYSTEM_VERSION_STRING);
			SmartToolMsgStr(p_STPSession, STP_ALERT, COMMON_OID_SERIAL_NUMBER, g_szSerialNumber);
			SmartToolMsgUInt16(p_STPSession, STP_ALERT, COMMON_OID_COM_VERSION, COM_VERSION);
			SmartToolMsgEmpty(p_STPSession, STP_ALERT, COMMON_OID_STP_IDENTIFY);

			SmartToolMsgFloat(p_STPSession, STP_ALERT, CRAWLER_OID_MAX_SPEED, g_ConfigData.fMaxSpeed);
			SmartToolMsgFloat(p_STPSession, STP_ALERT, CRAWLER_OID_MAX_ROTATION_SPEED, g_ConfigData.fMaxRotationSpeed);
			break;
		case COMMON_OID_DATE_TIME:
			HandleCommonOIDDateTimeSet((td_DateTime *) p_STPrxMsg->p_cObjectValue);
			break;
		case CRAWLER_OID_MODE:
			case CRAWLER_OID_MODE_GOAL:
			//Setting Mode Sets the Goal...
			//For Some modes, we allow it to directly set
			g_cModeStateGoal = p_STPrxMsg->p_cObjectValue[0];
			g_cModeState = g_cModeStateGoal; //by default, go right to mode

			//This may not always be the best option, but for now I want to try
			g_cModeStatePrev = 0; //allow the actions associated with entering this mode to trigger again

			if (g_cModeStateGoal == ModeEngage)
			{
				//not allowed to directly set this mode...
				g_cModeState = ModePrepareOperate;
			}
			else if (g_cModeStateGoal == ModeMove)
			{
				//not allowed to directly set this mode...
				g_cModeState = ModePrepareToMove;
			}
			break;
			//important JLIME
//If using a goal style machine, like stance, then you would handle like this on it's own.
//				case CRAWLER_OID_MODE_GOAL:
//					g_cModeStateGoal = p_STPrxMsg->p_cObjectValue[0];
//					break;
		case CRAWLER_OID_STOREDEFAULT_CONFIG:
			StoreConfig(p_STPSession);
			break;
		case CRAWLER_OID_RECALL_CONFIG:
			//edit.... removed factory recal
			RecallConfig(p_STPSession, p_STPrxMsg->p_cObjectValue[0]);
			break;
		case CRAWLER_OID_ESTOP_CLEAR_DELAY:
			g_ConfigData.uiEStopClearDelay = (unsigned int) ntohs(
					*((int*) (p_STPrxMsg->p_cObjectValue)));
			break;
		case CRAWLER_OID_STANCE_GOAL:
			g_cStanceGoal = p_STPrxMsg->p_cObjectValue[0];
			break;
		case CRAWLER_OID_STANCE_TIMEOUT:
			g_ConfigData.uiStanceTimeout = (unsigned int) ntohs(*((int*) (p_STPrxMsg->p_cObjectValue)));
			break;
		case CRAWLER_OID_AUX_DATA_TRANSFER_PORT:
			//edit=(unsigned int)ntohs( *((int*)(p_STPrxMsg->p_cObjectValue)) );
			break;
		case CRAWLER_OID_ENCODER_RATIO:
			g_ConfigData.fEncoderRatio = *(float *) p_STPrxMsg->p_cObjectValue;
			break;
		case CRAWLER_OID_CURRENT_LIMITS:
			f = *(float *) p_STPrxMsg->p_cObjectValue;
			if (f > 2.0)
			{
				logf("must be <= %f\r\n", 2.0);
			}
			g_ConfigData.fCurrentLimit = f;
			MCSetConfigParams();
			break;
		case CRAWLER_OID_PEAK_CURRENT_LIMITS:
			f = *(float *) p_STPrxMsg->p_cObjectValue;
			if (f > 2.0)
			{
				logf("must be <= %f\r\n", 2.0);
			}
			g_ConfigData.fPeakCurrentLimit = f;
			MCSetConfigParams();
			break;
		case CRAWLER_OID_MAX_SPEED:
			g_ConfigData.fMaxSpeed = *(float *) p_STPrxMsg->p_cObjectValue;
			break;
		case CRAWLER_OID_MAX_ACCEL:
			g_ConfigData.fMaxAcc = *(float *) p_STPrxMsg->p_cObjectValue;
			break;
		case CRAWLER_OID_MAX_DECEL:
			g_ConfigData.fMaxDec = *(float *) p_STPrxMsg->p_cObjectValue;
			break;
		case CRAWLER_OID_MAX_STOP_DECEL:
			g_ConfigData.fMaxStopDec = *(float *) p_STPrxMsg->p_cObjectValue;
			break;
		case CRAWLER_OID_POSNERR_LIMIT:
			g_ConfigData.fPosnErrLimit = *(float *) p_STPrxMsg->p_cObjectValue;
			MCSetConfigParams();
			break;
		case CRAWLER_OID_VELERR_LIMIT:
			p_VelErrLimit = (td_VelErrLimit *) p_STPrxMsg->p_cObjectValue;
			g_ConfigData.VelErrLimit.fVLimitMargin = p_VelErrLimit->fVLimitMargin;
			g_ConfigData.VelErrLimit.fVError = p_VelErrLimit->fVError;
//FIXME LOW  Should Crawler have this?
//MCASetConfigParams();
			break;
//					MCASetMoveSpeedParams();

//FOR STOP DECEL???
//takes effect after next A Axis Restart. (Power Cycle is the easiest way to force.)

//POSITION LIMITS??????????????
//
//////////////////////////////////////////////////////////////////////////////////////////

//homing
		case CRAWLER_OID_POSNMODE_TOLERANCE:
			g_ConfigData.fPosnTolerance = *(float *) p_STPrxMsg->p_cObjectValue;
			break;
		case CRAWLER_OID_MAX_ROTATION_SPEED:
			g_ConfigData.fMaxRotationSpeed = *(float *) p_STPrxMsg->p_cObjectValue;
			break;
		case CRAWLER_OID_ROTATION_RADIUS:
			g_ConfigData.fRotationRadius = *(float *) p_STPrxMsg->p_cObjectValue;
			break;
		case CRAWLER_OID_RESET_MC:
			//FIXME MEDLOW Implement OID_RESET_MC action.
			break;
		case CRAWLER_OID_BRAKE_ON_TIMEOUT:
			g_ConfigData.uiBrakeOnTimeout = (unsigned int) ntohs(*((int*) (p_STPrxMsg->p_cObjectValue)));
			break;
		case CRAWLER_OID_JOG_ENABLE_TIMEOUT:
			g_ConfigData.uiJogEnableTimeout = (unsigned int) ntohs(*((int*) (p_STPrxMsg->p_cObjectValue)));
			break;
		case CRAWLER_OID_MOVE_FLOAT:
			p_oid_move_float = (td_oid_move_float *) p_STPrxMsg->p_cObjectValue;
			//NTOH the field that needs it
			p_oid_move_float->uimoveNumber = ntohs(p_oid_move_float->uimoveNumber);
			//Structure for reference
			/*
			 typedef struct {
			 char cmoveType;
			 char crelative;
			 unsigned int uimoveNumber;
			 float fx;
			 float fy;
			 float fr;
			 float fc;
			 float fspeed;
			 float frotateSpeed;
			 float facceleration;
			 } td_oid_move_float;
			 */

#ifdef EDGEDETECT_SYSTEM
			cBlockedMove = 0;
			if (g_cUseEdgeDetect==1)
			{
				cBlockedMove = EdgeMoveAway(p_oid_move_float->fx, p_oid_move_float->fy, p_oid_move_float->fr);
				logf("BlockedMove: %d\r\n", cBlockedMove);
			}
#endif

			if (g_cModeState != ModeReady)
			{
				if (g_cModeState != ModeMove)
				{
					//Can't do a move
					logf("mode does not allow move start\r\n");
					break;
				}
				else
				{
					//may continue move
					//important JLIME

				}

			}
			if (p_oid_move_float->cmoveType == MoveTypeNew)
			{
				if (p_oid_move_float->crelative == 2)
				{
#ifdef EDGEDETECT_SYSTEM
					if(cBlockedMove != 1)
					{
						g_iEdgeDetect = 0;
#endif
					temp_move_code(p_oid_move_float->uimoveNumber,
							p_oid_move_float->fx,
							p_oid_move_float->fy,
							p_oid_move_float->fr,
							p_oid_move_float->fspeed,
							p_oid_move_float->frotateSpeed,
							p_oid_move_float->facceleration);
					g_cModeState = ModeMove;
					AlertModeChanges(p_STPSession); //do right away
#ifdef EDGEDETECT_SYSTEM
						}
#endif
				}
			}
			break;
		case CRAWLER_OID_MOVE_AXIS_FLOAT:
			p_oid_move_axis_float = (td_oid_move_axis_float *) p_STPrxMsg->p_cObjectValue;
			//p_oid_move_axis_float->cmoveType=p_oid_move_axis_float->cmoveType;
			//p_oid_move_axis_float->caux = p_oid_move_axis_float->caux;
			p_oid_move_axis_float->uimoveNumber = ntohs(p_oid_move_axis_float->uimoveNumber);
			//p_oid_move_axis_float->fdist1=p_oid_move_axis_float->fdist1;
			//p_oid_move_axis_float->fdist2=p_oid_move_axis_float->fdist2;
			//p_oid_move_axis_float->fdist3=p_oid_move_axis_float->fdist3;
			//p_oid_move_axis_float->fdist4=p_oid_move_axis_float->fdist4;
			//p_oid_move_axis_float->fspeed=p_oid_move_axis_float->fspeed;
			//p_oid_move_axis_float->facceleration=p_oid_move_axis_float->facceleration;
			/*
			 typedef struct {
			 char cmoveType;
			 char caux;
			 unsigned int uimoveNumber;
			 float fdist1;
			 float fdist2;
			 float fdist3;
			 float fdist4;
			 float fspeed;
			 float facceleration;
			 } td_oid_move_axis_float;
			 */
			if (g_cModeState != ModeReady)
			{
				if (g_cModeState != ModeMove)
				{
					//Can't do a move
					logf("mode does not allow move start\r\n");
					break;
				}
				else
				{
					//may continue move
					//important JLIME

				}
			}
#ifdef EDGEDETECT_SYSTEM
			cBlockedMove = EdgeDetected();
			if (cBlockedMove == 1)
			{
				//Not allowed to move
				logf("BlockedMove: %d\r\n", cBlockedMove);
				break;
			}
			//Clear edge detect
			g_iEdgeDetect = 0;
#endif
			temp_move_axis_code(p_oid_move_axis_float);
			g_cModeState = ModeMove;
			AlertModeChanges(p_STPSession); //do right away
			break;
		case CRAWLER_OID_STOP:
			MCStopPosition();
			break;

		case CRAWLER_OID_MOVE_FLOAT_BETA:
			p_oid_move_float_beta = (td_oid_move_float_beta *) p_STPrxMsg->p_cObjectValue;
			//p_oid_move_float_beta->cmoveType = p_oid_move_float_beta->cmoveType;
			//p_oid_move_float_beta->crelative = p_oid_move_float_beta->crelative;
			//p_oid_move_float_beta->cstop = p_oid_move_float_beta->cstop;
			//p_oid_move_float_beta->caux = p_oid_move_float_beta->caux;
			//p_oid_move_float_beta->fx = p_oid_move_float_beta->fx;
			//p_oid_move_float_beta->fy = p_oid_move_float_beta->fy;
			//p_oid_move_float_beta->fr = p_oid_move_float_beta->fr;
			//p_oid_move_float_beta->fc = p_oid_move_float_beta->fc;
			//p_oid_move_float_beta->fspeed = p_oid_move_float_beta->fspeed;
			//p_oid_move_float_beta->frotatespeed = p_oid_move_float_beta->frotatespeed;
			//p_oid_move_float_beta->facceleration = p_oid_move_float_beta->facceleration;
#ifdef EDGEDETECT_SYSTEM
			cBlockedMove = 0;
			if (g_cUseEdgeDetect==1)
			{
				cBlockedMove = EdgeMoveAway(p_oid_move_float_beta->fx, p_oid_move_float_beta->fy, p_oid_move_float_beta->fr);
				logf("BlockedMove %d\r\n", cBlockedMove);
			}
#endif

			if (g_cModeState != ModeReady)
			{
				if (g_cModeState != ModeMove)
				{
					//Can't do a move
					logf("mode does not allow move start\r\n");
					break;
				}
				else
				{
					//may continue move
					//important JLIME
				}
			}
			if (p_oid_move_float_beta->cmoveType == MoveTypeNew)
			{
				if (p_oid_move_float_beta->crelative == 2)
				{
#ifdef EDGEDETECT_SYSTEM
					if(cBlockedMove != 1)
					{
						g_iEdgeDetect = 0;
#endif
					temp_move_code(1,
							p_oid_move_float_beta->fx,
							p_oid_move_float_beta->fy,
							p_oid_move_float_beta->fr,
							p_oid_move_float_beta->fspeed,
							p_oid_move_float_beta->frotateSpeed,
							p_oid_move_float_beta->facceleration);
#ifdef EDGEDETECT_SYSTEM
				}
#endif

					g_cModeState = ModeMove;
					AlertModeChanges(p_STPSession); //do right away
				}
			}
			break;

		case CRAWLER_OID_MOVE_BYPASS:
			g_cMoveByPass = p_STPrxMsg->p_cObjectValue[0];
			g_iMoveBypass = g_cMoveByPass;
			break;
		case CRAWLER_OID_OPERATE_BYPASS:
			g_cOperateByPass = p_STPrxMsg->p_cObjectValue[0];
			g_iOperateBypass = g_cOperateByPass;
			break;
		case CRAWLER_OID_PARK_BYPASS:
			g_cParkByPass = p_STPrxMsg->p_cObjectValue[0];
			g_iParkBypass = g_cParkByPass;
			break;

//MakeOID-generated::END
		default:
			#ifdef USE_OUTPUT
			logf("rx'd unsupported STP OID %d for SET\r\n", p_STPrxMsg->uiOID);
#endif
		}
		break;
	case STP_ALERT:
		//	currently, the rabbit doesn't need to deal with received alerts
		break;
	case STP_GET_RESP:
		//	currently, the rabbit doesn't need to deal with received get responses
		break;
	default:
		break;
	}
	return 0;
}

int atobool(char *s)
{
	char c;
	while (*s == ' ')
	{
		s++;
	}
	c = *s;
	if (c == '1')
	{
		return 1;
	}
	return 0;
}

void AlertModeChanges(td_STPsessions * p_STPSession)
{
	if (g_cSentModeState != g_cModeState)
	{
		logf("Mode=%s\r\n", g_szModeNames[(int) g_cModeState]);
		SmartToolMsgChar(p_STPSession, STP_ALERT, CRAWLER_OID_MODE, g_cModeState);
		g_cSentModeState = g_cModeState;
	}
}

void SendMoveStatus(td_STPsessions * p_STPSession)
{
	td_MoveStatus * p_MoveStatus;
	p_MoveStatus = (td_MoveStatus *) g_STPtxMsg.p_cObjectValue;
	p_MoveStatus->uimoveNumber = htons(g_MoveStatus.uimoveNumber);
	p_MoveStatus->caxis = g_MoveStatus.caxis;
	p_MoveStatus->cstage = g_MoveStatus.cstage;
	p_MoveStatus->ftime = g_MoveStatus.ftime;
	p_MoveStatus->feta = g_MoveStatus.feta;
	p_MoveStatus->fdistRemain = g_MoveStatus.fdistRemain;
	p_MoveStatus->fdistTotal = g_MoveStatus.fdistTotal;
	SmartToolMsg(p_STPSession, STP_ALERT, CRAWLER_OID_MOVE_STATUS,
			sizeof(td_MoveStatus), g_STPtxMsg.p_cObjectValue);

	logf("m%u a=%d s=%d t=%f dist %f %f\r\n", g_MoveStatus.uimoveNumber,
			g_MoveStatus.caxis, g_MoveStatus.cstage, g_MoveStatus.ftime,
			g_MoveStatus.fdistRemain, g_MoveStatus.fdistTotal);
}

#define SHOWIO_DIN 8
#define SHOWIO_ANA 8
char g_clastdin[SHOWIO_DIN];
int g_ilastanain[SHOWIO_ANA];
float g_flastanain[SHOWIO_ANA];
float g_fminanain[SHOWIO_ANA];
float g_fmaxanain[SHOWIO_ANA];

void ClearShowIO()
{
	int i;
	i = 0;
	while (i < SHOWIO_DIN)
	{
		g_clastdin[i++] = 2;
	}
	i = 0;
	return;
}

void ShowIO()
{
	char ci;
	char c;
	char lc;
	ci = 0;
	while (ci < SHOWIO_DIN)
	{
		c = digIn((int) ci);
		lc = g_clastdin[(int) ci];
		g_clastdin[(int) ci] = c;
		if (c != lc)
		{
			if (c > 0)
			{
				c = '1';
			}
			else
			{
				c = '0';
			}
			logf("DIN%d=%c\r\n", ci, c);
		}
		ci++;
	}
	return;
}

void ClearShowAna()
{
	int i;
	i = 0;
	while (i < SHOWIO_ANA)
	{
		g_ilastanain[i] = 0;
		g_flastanain[i] = 1024;
		g_fminanain[i] = 100;
		g_fmaxanain[i] = -100;
		i++;
	}
	return;
}

void ShowAna()
{
	char ci;
//char c;
//char lc;
	float f;
	float lf;
	int iana;
	int liana;
	float fmin;
	float fmax;
	char cdelta;
	char cbit;

	ci = 0;
	cbit = 1;
	while (ci < SHOWIO_ANA)
	{
		if ((cbit & g_cShowAna) > 0)
		{
			liana = g_ilastanain[(int) ci];
			iana = anaIn((int) ci);
			g_ilastanain[(int) ci] = iana;

			f = anaInConvertVolts(iana, (int) ci, 0);
			f = anaInVolts((int) ci, 1);
			lf = g_flastanain[(int) ci];

			cdelta = 0;
			fmin = g_fminanain[(int) ci];
			fmax = g_fmaxanain[(int) ci];
			if (f < fmin)
			{
				fmin = f;
				g_fminanain[(int) ci] = f;
				cdelta = 1;
			}
			if (f > fmax)
			{
				fmax = f;
				g_fmaxanain[(int) ci] = f;
				cdelta = 1;
			}

			if (fabs(f - lf) > 0.2 || (iana > 2047 && iana != liana)
					|| cdelta == 1)
			{
				logf("anaIn%d = %d counts = %.4f v   %.4f to %.4f\r\n", ci,
						iana, f, fmin, fmax);
				g_flastanain[(int) ci] = f; //this is the last SHOWN value
			}
		}
		ci++;
		cbit *= 2;
		//if (ci==2) { ci=4; } //skip over 2 and 3 which are not analog on this board....  My analog 2 and 3 show up on 4 and 5
	}
	return;
}

#endif

//EOF
