////////////////////////////////////////////////////////////////////////////
// MiniFt And MiniFt HD
//
// 20131018-1-0843	David Whalen-Robinson	First Linux Smart Tool
//	Code should be considered a prototype.  All libraries are local project files.  All libraries and code locations
//  should be considered in temporary locations and are likely to experience subsequent revisions
////////////////////////////////////////////////////////////////////////////////
//FIXME PORTFATAL find and adjust all fixme priorities

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <math.h>
#include <errno.h>

#include "MiniFTDefs.h"
#include "SmartTool.h"
#include "CommonSmartTool.h"
#include "MiniFTSmartTool.h"
#include "ToolManagementSmartTool.h"
#include "CrawlerConfigScript.h"
#include "STP.h"
#include "SmartToolUtil.h"
#include "SocketConsole.h"
#include "MiniFTWXY.h"
#include "hwio.h"
#include "MiniFTIO.h"
#include "AngleSensor.h"
#define MD5HASH
#ifdef MD5HASH
//FIXME PORTHIGH3 TEST  try this as a replacement for MD5
#include <openssl/md5.h>
#else
//FIXME PORTMED allows bypass of md5 in case it's hard to make it work
#define BYPASS_MD5_MUST_MATCH
#endif

//FIXME PORTLOW SEVERE RailSTP
//STP Rail LIB
#ifdef HD_RAIL_STP
//FIXME PORTLOW  RailSTP  (will be high later)
//#use "MiniFtRailSTP.lib"
#endif

//CenterVision
#ifdef CENTERVISION
#ifdef CENTERVISION_LINE_SCANNER
//#use "MiniFtVision.LIB"
#error "lib not ported to linux"
#endif
#ifdef CENTERVISION_ANALOG_POINT_LASER
//#use "MiniFtVisionPoint.LIB"
#error "lib not ported to linux"
#endif
#ifdef CENTERVISION_CAM
//FIXME PORTMED  how to include exclude the C file based on triggers????? better way to configure
#include "CenterVisionCam.h"
#endif
#endif

//FIXME PORTMED move this lower and then fix comments
//FIXME MED place these near other non-oid tool items... These work with
// Tool Management oids but are actually closer to g_LoadedTool.
//In the Future this coule  be organized better.
byte g_szToolIDlen;
char g_szToolID[257]; //256 + 1
byte g_szToolTypeCodelen;
char g_szToolTypeCode[65]; //64 + 1

//FIXME PORTMED... stp libs....  libs and local vs options
//#define SHOW_STP_HEARTBEAT
//THIS gets moved to a debug option
//#define PACKET_FLOOD_MONITOR
//this one seems to be on all the time now

//RFID Lib
#ifdef USE_RFID_OMRON
#fatal "OMRON NOT PORTED AND NOT SUPPORTED"
//MUST CONVERT THIS TO USE NEW?????
#endif

//RFID Lib
#ifdef USE_RFID_MIFARE
//#define RFID_SERVICE_PERIOD_MS		35
#include "StrongLinkSL031Serial.h"
#endif

#ifdef USE_RFID_OMRON
//The main format from 2011,2012 and 2013 before HD5
#define RFID_FORMAT_MS00
#define RFID_FMS00_DATA_SIZE 48
#endif

#ifdef USE_RFID_MIFARE
//New Shorter Format
#define RFID_FORMAT_0
#define RFID_F0_DATA_SIZE 16
//also use this because we have code that needs to send this to CELL
#define RFID_FMS00_DATA_SIZE 48
typedef struct
{
	byte cFormat;
	byte cRailType;
	byte cGroup;
	byte cSegmentAndRailSide;
	unsigned long ulSerialNumber;
	unsigned long ulPosition;
	unsigned long ulSegmentPosition;
} td_RFID_F0;
#endif

//CLOSEST_HOLE_SYSTEM
//Hole Location Using Ordered by X
//Using a binary search, find a the hole closes to your hole in X.
//Once you find the closest hole in X, or a hole with your same X,
//Calculate your distance D.
//Scan to X = YOUR X - D  up to YOUR X + D
//You should scan starting at X and working to X-D and then from X to X+D
//If at any point you find a closer Hole, then this would narrow the min and max
//X you need to check.

//Types for Part Program system

typedef struct
{
//FIXME HIGH
//continue cleanup of these variable locations
	byte cLocked;
	byte cTeachModeData;
	byte cMiniFtFormat; //indicate that new format is being used.
	byte cChecksum;
	byte p_cChecksum16[16];
	int iNumVisited; //total visited since load

	//special program parameters
	float K1OriginDistX; // in inches, X offset of K1 from track origin bumper
	float K1EndDistX;		// in inches, X offset of K1 from track end bumper
	float K1OriginDistY; 	// in inches, Y offset of K1 from center of X track

	byte cErrorCount;
	unsigned int p_uiErrorMessages[PARTPGM_MAX_ERROR_MESSAGES];

} td_PartPgmInfo;

//New Program
//FIXME000000000000000 consolidate all the new as I clean

typedef struct
{
	unsigned int diameter;
	byte ops; //FIXME000000000000 Ops variable width issues...
} td_tool_data_fileformat;

typedef struct
{
	byte layers;
	byte material;
	unsigned int clamplbs;
	byte ops; //FIXME000000000000 Ops variable width issues...
	unsigned int countersinkDepth;
	//the following fields are only set when the larger format row is sent
	byte proclayers;
	unsigned int clampwarnlbs;
	unsigned int clampalarmlbs;
	unsigned int procminbreak_not_used; //d2u format //no longer used, but needed for proper record spacing
	unsigned int hardstopamps; //Times 100 for 2 digits of decimal
	byte procstyle;
} td_process_data;

typedef struct
{
	byte flatx;
	byte _flatxb2;
	byte _flatxb3;
	byte flaty;
	byte _flatyb2;
	byte _flatyb3;
	byte ki_primary;
	byte ki_secondary;
	unsigned int diameter;
	byte tooltype;
	byte process;
	byte fastener;
	byte pattern;
	byte ops;
	unsigned int depthstack;
	int countersink_adj;
} td_hole_data_fileformat;

//FIXME00000000000000000000000000
typedef struct
{
	byte cKInd;
	byte cKPri;
	byte cKSec;
	byte cTool;
	byte cProcess;
	byte cFastener;
	byte cPattern;
	unsigned int uiOps; //array of bits used to indicate what operations are specified for this hole (optional for some input formats)
	unsigned int uiStack;
	int iCountersinkAdjust;
	unsigned int d2uDiameter;
} td_HoleData;

//Smart Drill Struct Used for Layer Defs
typedef struct
{
	char Name[32];
	byte cLayerNumber;
	byte cCoolantType;
	byte cPeckType;
	byte cShiftRetract; //36
	byte cLcUseSlope;
	byte cH2SPeck;
	byte cCountersinkLayer;
	byte cUseHardStop; //40
	byte cBurstInterval; //10ms units  0=0ms 1=10ms//cSpeedShift;
	unsigned int uiRpm;
	unsigned int uiPeckDwell; //45
	int iLubeDurationMs; //47
	float fMicroPeckRot;
	float fMicroPeckSetback;
	float fAbsoluteEnd;
	float fThicknessMax;
	float fIpr; //67
	float fPeckIncrement;
	float fSetback;
	float fEarlyShiftDistance;
	float fThrustBaselineDistance;
	float fThrustMin; //87
	float fThrustMax;
	float fLcDeltaThrust; //95
} td_Layers;

//Smart Drill Struct Used for OVERRIDE

typedef struct
{
	byte cLayerNumber;
	byte cOverrideActive;
	byte cUseThisLayer;
	byte cUseHardStop;
	float fAbsoluteEnd;
	float fThickness;
} td_LayerOverride;

//Function Predeclarations

void InitVars(void);

void StopInterfaceTasks();
#ifdef DRILL_DIRECT_READY
void ClearDrillSync();
#endif

void EstopEngageActions(void);
void EstopContinueActions(void);
void EstopDisengageActions(void);

#ifdef TOOL_IN_RAM
void SaveToolToRam();
void LoadToolFromRam();
#endif
void LoadToolToDrill(void);
void LoadToolHomeBackToDrill(void);
void RepositionDrill(void);
void ModeStateMachine(void);
void RFIDRun();
void SetupRFIDSeekMove(float fmovedelta, char * label);
#ifdef HOMESYSTEM
void ClearAllHomeStatus();
void ClearSentHomeStatus();
void SetAllHomeStatusPending();
void AlertHomeStatus();
void AlertHomeReport(byte axis_code, byte status, byte status_reason);
#endif
void SelectHomePositionAndPositionLimits(void);
#ifdef SEAL_SYSTEM
int LookupDiameterPrimeDelay(float fdiameter);
#endif

int main(void);

void ServiceNow();

// Probe Distance Check
void CheckKLocationDistances(byte ck, byte mark);

// functions associated with Modes
void ResetNearestPosition();
int FindNearestPosition();
int MoveAllowed(unsigned int uiSource);
int SpecifyGotoPosn(int iGotoPosn, int iIndex);
void SpecifyGotoPosnAgain();
void SetPressureBasedOnProcess(byte cProcess);
#ifdef CLAMP_SYSTEM_HD_PISTON
void SetClampPressureLBS();
#endif
//float CalcDistanceSquared(float fX1, float fY1, float fX2, float fY2);
//MACHINEPOINTS
void CalcProbeHome();
int CalcProbeAdjust();
//ClearGolbalRotation(); //just clear two variables
void SetGlobalRotation(byte cpk, byte csk);
void StartCreateMachineDataSet();
void CreateMachineDataSet();
void CreatePosnHashList();
#ifdef OUTPUT_POSITION_INFO
//instead of listing the program detail all at once,
//this can put it out over time after the program is done loading
void ShowProgramRotationOutput();
#endif
byte CreateMachineDataSetForOneHole(unsigned int i, float * fpX, float * fpY);
byte RotateVecDataSetToMachine(unsigned int i, float * fpX, float * fpY);
byte RotateVecMachineToDataSet(unsigned int i, float * fpX, float * fpY);
void ProbeModeSelectAndSetGlobalRotation();
void RotateMachineToDataset(float fX, float fY, float* p_fRotatedX,
		float* p_fRotatedY);
void ApplyMachineOffset(float * p_fX, float * p_fY);
void RemoveMachineOffset(float * p_fX, float * p_fY);
void SetToolOffset(float fToolOffsetX, float fToolOffsetY);
void SetToolOffsetWithYComp(float fToolOffset1X, float fToolOffset1Y,
		float fToolOffset1YExtension, float fToolOffset2X, float fToolOffset2Y,
		float fToolOffset2YExtension);
void ClearToolOffset();
void CalculatePreMove(float* p_fPreMoveX, float* p_fPreMoveY);
void InitPosnMode(void);
void SetMoveSpeeds(byte bMakeLinear, byte bFinalMove, float fdx, float fdy);
void DoFloat(byte cAction);

// Part Program Loading / Parsing functions
void ClearPartProgram();
void ParsePartProgramStart();
void ParsePartProgramContinue(byte cMaxLinesPerContinue);
void PartProgramShowStatus();
void PartProgramShowStatusPart2();
void LogParseError(unsigned int uiMessageCode);
void CreateProbeEvaluationOrder(); //probing move to probe lib???
void RecalculateLocatingDirectives(byte ki);
void PreviewDisplayProbeCommandDataBase();
void LoadProbeCommand(byte chprobe);
void ClearProbeCommand();
void ParseProbeCommand(byte ckprobe, char * sbuf, int i);
void ClearPositionsDuringProbe(byte ki);
void RecalculatePositionsDuringProbe(byte ki);

void CaptureTeachPosition();
void DeletePosition();
void CompleteTeachPosition();
void LoadLinearProgram(); //an alternate
void ClearOpHistory(void);
void AddOpHistory(int index, unsigned int uiOperation);
void SetOpHistory(int index, unsigned int uiOperations);
unsigned int GetOpHistory(int index);
void SetDrillDir(int iProbeDir);

// Job Selection
void StartOver();
void StartSendPartProgramData(int iSession, uint32 uiStart);
void ContinueSendPartProgramData();
void SendProbeStatus(unsigned int uiMsgType, byte cKIndex);
void SendAllProbeStatus(unsigned int uiMsgType);
void SendProbeStart(unsigned int uiMsgType, byte cKIndex);
void SendAllProbeStart(unsigned int uiMsgType);
void AlertProbeStatusUpdate();

//void SendKHoleDistance(unsigned int uiMsgType,float fExpected,float fFound);
void SendXYDataID(td_STPsessions * p_STPSession, unsigned int uiMsgType);
void StartSendXYData(int iSession, unsigned int uiStart);
void ContinueSendXYData();
void StartSendOpHistory(int iSession, unsigned long ulAfterTime);
void ContinueSendOpHistory();
void SendRFIDData(td_STPsessions * p_STPSession, unsigned int uiMsgType);
void SendCurPosnInd(td_STPsessions * p_STPSession, unsigned int uiMsgType);
void SendNearPosnInd(td_STPsessions * p_STPSession, unsigned int uiMsgType);
void SendGoalPosnInd(td_STPsessions * p_STPSession, unsigned int uiMsgType);
void SendCurXY(td_STPsessions * p_STPSession, unsigned int uiMsgType);
void SendNearXY(td_STPsessions * p_STPSession, unsigned int uiMsgType);
void SendActivePremove(td_STPsessions * p_STPSession, unsigned int uiMsgType);
void SendSystemComponents(td_STPsessions * p_STPSession, unsigned int uiMsgType);
void LoadHoleParameters();
void SendHoleParameters(td_STPsessions * p_STPSession, unsigned int uiMsgType);
#ifdef SMARTDRILL_STP
void CalculateLayerOverride();
void SendLayerOverride();
void ShowLayerOverride();
#endif
void SendReqToolSearch(unsigned int uiMsgType);
void SendToolMGMT(td_STPsessions * p_STPSession, unsigned int uiMsgType, byte c_op, byte carg1, unsigned int ui, char *s, int ilen);
void SendTool(td_STPsessions * p_STPSession, unsigned int utMsgType, byte cOperation);
byte CheckObstructionsAndMotionLimits(float fX, float fY);
void AlertObstructionCode(td_STPsessions * p_STPSession);
void AlertObstructionWarningCode(td_STPsessions * p_STPSession);

//Prbe System
void InitProbeValues();
void ResetProbeValues();
void SendProbeValues();
void CheckProbeComplete();

//Stations
void UpdateStationPlan();
void NextStation();
void AdvanceStations();
void PrepareStations();

void StopProcess(void);

//Tool Lookup
byte LookupToolTypeCode(char * tooltype);
void LookupToolTypeString(char * tooltypeout, byte ctool_search);

//Tool Sync
void VerifyAndAlertTool();
void VerifyAndAlertProcess();
void LoadProcessAndOverride();
#ifdef SMARTDRILL_STP
void LoadProcess();
#endif

//Maintain g_DateTime
void ServiceDateTime();

//System Function

//Global Storage

//FIXME PORTVHIGH makeoid handlers are still integrated and require massive adjust....
//    they are not even done integration

//FIXME PORTMED move this memory allocation???
byte g_cSmartToolShutdown = 0;

//Action Sent
byte g_cActionSent;

//Hold
byte g_cHold;

#ifdef HOMESYSTEM_X_RFID
float g_fRFIDMachineX;
float g_fRFIDRailX;
int g_iRFIDRailOrientation;
#endif

//assisting variables to globals

byte g_cEstopPrevMode;
byte g_cPosnModeState;
byte g_cStartProcess;
byte g_cStationGoal;
byte g_cStartStation;
byte g_cStationPlanDrill;
byte g_cStationPlanInspect;
#ifdef SEAL_SYSTEM
byte g_cStationPlanSeal;
#endif
#ifdef FASTENER_SYSTEM
byte g_cStationPlanFill;
byte g_cStationPlanRemove;
#endif
byte g_cSentStation;
byte g_cSentStationGoal;
char *g_cStationCode;

unsigned int g_uiActionCycles;
int g_iActionHoleIndex;

//FIXME0000: are these 2 used really????
byte g_cDrillStatePrev;
unsigned long g_ulDrillStateStartTime;

//Temp Quick Implementation for this
byte g_cSafeUnclamp; //Signal used by Drill and Fastener
byte g_cSawDrillMode;
byte g_cTestOpt;
byte g_cLubeBypass;

byte g_cFastDoneFlag;
byte g_cPendingFastenerFaultUpdate;

int g_iPrimeDelay;

byte g_cCutterDetected;
byte g_cLastCutterDetected;
unsigned int g_uiMessageCode;

//Tool
td_oid_tool_rec g_LoadedTool;

td_STPsessions * g_ToolServerSTPSession;

//Part Program Data

//Primary Program Global Storage
float g_fRawDataX[MAXNUM_POSNS];
float g_fRawDataY[MAXNUM_POSNS];
float g_fRawDataMX[MAXNUM_POSNS];
float g_fRawDataMY[MAXNUM_POSNS];
float g_fRawDataR[MAXNUM_POSNS]; //radius to halfway mark to nearest hole.

//allocate memory for HoleData Structures
td_HoleData g_HoleDataArray[MAXNUM_POSNS]; //VERY LARGE ARRAY

byte g_cMaterialData[MAXSIZE_MATERIALS];
byte g_cStackData[MAXSIZE_STACKDATA];

//memory for operation history bit flag array.
uint16 g_uiOpHistory[MAXNUM_POSNS];

//CLOSEST_HOLE_SYSTEM
int16 g_iMPosnHashList[MAXNUM_POSNS]; //linked list of buckets

//buffer for PP Data that we need to be able to send out later
char g_cPartProgramData[MAX_PARTPGM_STORAGE];

//And initialize the length to 0
uint32 g_uiPartProgramDataLength = 0;

//Not Allocated but a reference...
char * g_pcToolNames = 0; //not allocated but a pointer into the part program data
uint16 g_uiToolNamesLen = 0;

char * g_pcProcessLayerDataBase = 0; //not allocated but a pointer into the part program data
char * g_pcProbeCommandDataBase = 0; //probe control field

td_PartPgmInfo g_PartPgmInfo; // supporting only one PP at a time, so this is not an array

//additional parse state, start by start and used in continue
byte g_cBeingParsed;
//used for direct data
char * g_sPPDirect;
int g_iPPDirectPos;
int g_iPPDirectEnd;
byte g_cPPDirectFinalCall;
int g_iProgramLoadingSession;

//Part Program
byte g_cFormatMinorVersion;
byte g_cToolCount;
byte g_cProcessCount;
byte g_cFastenerCount;
byte g_cPatternCount;
int g_iHoleCount;
byte g_cKHoleCount;

//NEW_MINIFT_FORMAT
//PROCESS DATA
byte g_cProcessLayerCount[MAXNUM_PROCESSES];
byte g_cProcessMaterials[MAXNUM_PROCESSES];
byte g_cProcessProcLayerCount[MAXNUM_PROCESSES];
unsigned int g_uiProcessPounds[MAXNUM_PROCESSES];
unsigned int g_uiProcessPoundsWarning[MAXNUM_PROCESSES];
unsigned int g_uiProcessPoundsAbort[MAXNUM_PROCESSES];
byte g_cProcessOps[MAXNUM_PROCESSES];
unsigned int g_uiProcessCountersink[MAXNUM_PROCESSES];
unsigned int g_uiProcessHardstopAmps[MAXNUM_PROCESSES];
byte g_cProcessStyle[MAXNUM_PROCESSES];
unsigned int g_uiProcessLayerDataIndex[MAXNUM_PROCESSES];
//(7*2+4)= 18 bytes per proc * 64 procs = 1152 bytes

//HOLE DATA

//Program Options
float g_fAssumeApproxDataSetOrientation;
byte g_cAssumeApproxPositionsFromDataset;
byte g_cRequireParameterRecognition;

//Prbe - NOT A TYPO : New system is called Prbe until the old is 100% removed
byte g_cProbeFlag;
float g_fProbeX; //Hold The Probe Position Until it is placed into the arrays
float g_fProbeY;
int g_iKHoleHoleIndex[MAXNUM_KHOLES + 2];
float g_fKHolePrbeX[MAXNUM_KHOLES + 2];
float g_fKHolePrbeY[MAXNUM_KHOLES + 2]; //MAXNUM_KHOLES + 2];
byte g_cKHolePrbeStatus[MAXNUM_KHOLES + 2];
byte g_cKHolePrbeStatusWarnings[MAXNUM_KHOLES + 2]; //goes out in HIGH HALF of method
byte g_cKHolePrbeStatusDistance[MAXNUM_KHOLES + 2]; //used only for distance check loop
//ProbeStart
byte g_cKHolePrbeStart[MAXNUM_KHOLES + 2];
float g_fKHolePrbeStartX[MAXNUM_KHOLES + 2];
float g_fKHolePrbeStartY[MAXNUM_KHOLES + 2]; //MAXNUM_KHOLES + 2];
byte g_cKHoleHoleIndexSet;
byte g_cDistanceErrorFlagSent;
byte g_cDistanceErrorShown;
byte g_cProbeDistanceErrors;

byte g_cKHolePrbeEvalDCount[MAXNUM_KHOLES + 2];
byte g_cKHolePrbeEvalOrder[MAXNUM_KHOLES + 2];
//Probe Command Field Probe Parameters for Probe Control
byte g_cProbeCommand;
byte g_cProbeCommandMessage;
float g_fProbeVectorX;
float g_fProbeVectorY;
float g_fProbeMachineVectorX;
float g_fProbeMachineVectorY;
byte g_cProbeExtraOffsetGiven;
byte g_cProbeExtraMachineOffsetGiven;
float g_fProbeExtraOffsetX;
float g_fProbeExtraOffsetY;
float g_fProbeExtraMachineOffsetX;
float g_fProbeExtraMachineOffsetY;
float g_fProbeMaxDistShift;
byte g_cProbeShiftLimX;
float g_fProbeShiftLimXMin;
float g_fProbeShiftLimXMax;
byte g_cProbeShiftLimY;
float g_fProbeShiftLimYMin;
float g_fProbeShiftLimYMax;
float g_fProbeExpectedDiameter;

//Probe Command for new runtime probe system
byte g_cKHolePrbeCommand;
byte g_cKHolePrbeCommandInput;
//Hole Being Probed by new probe system
byte g_cKHolePrbeIndex;
byte g_cKHolePrbeIndexInput; //input value used with trigger
//New Trigger
byte g_cKHolePrbeTrigger;
//Complete and Calculated
byte g_cProbeComplete;
byte g_cProbeCalculated;
//Reapply Feature
float g_fKHoleLastPrbeX[MAXNUM_KHOLES_REPROBE + 2];
float g_fKHoleLastPrbeY[MAXNUM_KHOLES_REPROBE + 2];
byte g_cKHoleLastPrbeStatus[MAXNUM_KHOLES_REPROBE + 2];
byte g_cKHoleLastCount;

//RFID HOME
float g_fStartX;
float g_fStartY;
float g_fTargetX;
byte g_cTagState;
byte g_cMoveDir;
unsigned long g_ulCenterStart;
float g_fTagStart;
float g_fTagEnd;
byte g_cTagP;
byte g_cTagN;
#define TAG_NONE 0
#define TAG_STARTED 1
#define TAG_FOUND 2

byte g_cImmediateFakeRfid;

//XYDATA_ID
uint32 g_ulMachineDataSetIDTimeSec;
uint32 g_ulMachineDataSetIDTime;

//OpHistory
#define OPHISTORYBLOCKCOUNT 40
byte g_cBlockCount;
byte g_cOpHistoryBlockOrder[OPHISTORYBLOCKCOUNT]; // 40 > 5000/128
uint32 g_ulOpHistoryBlockTime[OPHISTORYBLOCKCOUNT];

int g_iProbedIndex;

//HD Clamp Status
#ifdef CLAMP_SYSTEM
byte g_cClampState;
byte g_cClampGoal;
byte g_cClampStateSent;
byte g_cClampGoalSent;
#endif
uint32 g_ulClampStart;
uint32 g_ulClampPressureZero;
uint32 g_ulClampPressureLow;
uint32 g_ulClampPressureHigh;
uint32 g_ulClampLegsLock;
uint32 g_ulClampLegsUnlock;
uint32 g_ulClampLegsUp;
uint32 g_ulClampLegsDown;
uint32 g_ulClampALock;
uint32 g_ulClampAUnlock;
uint16 g_uiClampPressure;
uint16 g_uiClampPressureWarning;
uint16 g_uiClampPressureAbort;
char * g_szClampMessage;

//Pending Ops
byte g_cClear;
#ifdef CLAMP_SYSTEM_NAC_STP
byte g_cNACClear;
float g_fNACClampZOffset;
#endif
//Jog
byte g_cJogX; //The status
byte g_cJogY; //The status
byte g_cJogGoalX; //The goal
byte g_cJogGoalY; //The goal
float g_fJogX; //The status speed factor
float g_fJogY; //The status speed factor
float g_fJogGoalX; //The goal speed factor
float g_fJogGoalY; //The goal speed factor
uint32 g_uiJogStopX;
uint32 g_uiJogStopY;
#ifdef JOG_ENABLE_TIME
uint32 g_uiJogEnableTime;
#endif

//Position Update
uint32 g_uiPositionUpdateTime;
uint32 g_uiPositionUpdateThrottle;
uint32 g_uiPositionSendTime;
uint32 g_uiPositionSendThrottle;
float g_fPosnLSCD; //Least significant change distance: Changes smaller than this should not be signalled back

//Position Limit
//Loaded from Config at the time needed
float g_fHomePositionX;
float g_fHomePositionY;
float g_fPositionMinX;
float g_fPositionMaxX;
float g_fPositionMinY;
float g_fPositionMaxY;

td_TeachMachinePosn g_TeachMachinePosn;

//FIXME PORTLOW make ARC4 work
#ifdef MD5HASH
MD5_CTX g_md5_context;
#endif

//FIXME MED Pattern
// bring this back some day when I use the new patterh feature
// this is really a reminder to build the new pattern feature
//int g_iChosenPattern;

//Tool and Process Check
byte g_cRequiredTool;
byte g_cRequiredProcess;
byte g_cLoadedTool; //the code of the loaded tool (or 0 if it has none in the map)
byte g_cToolLoaded; //bool
byte g_cProcessLoaded; //bool
byte g_cOverrideCalculated;
byte g_cOverrideSent;
byte g_cLayerOverrides;
td_LayerOverride g_LayerOverrides[8];

byte g_cLastSearchedRequiredTool;

//DRILL_DIRECT_PROCESS_AND_OVERRIDE
byte g_cDrillLoadProcessAndOverride;
#define DRILL_LOAD_PROCESS_NOT_NEEDED 0
#define DRILL_LOAD_PROCESS_NEEDED 1
#define DRILL_LOAD_PROCESS_WAIT 2
#define DRILL_LOAD_PROCESS_READY 3
#define DRILL_LOAD_PROCESS_DONE 4

#define DRILL_SYNC 3
#define DRILL_SYNC_BIT1 1
#define DRILL_SYNC_BIT2 2
byte g_cDrillSync;
float g_fHomeBack;
float g_fLastSentHomeBack;

byte g_iDrillDir; //hold the effective drill direction ( may not be the same as the default )

byte g_cAllowKVisit;
byte g_cAutoMove;
uint32 g_uiAutoTime;
uint32 g_uiStartAutoTime;
byte g_cAutoRepeat;
byte g_cLastAutoMove;

//Float
byte g_cFloatGoal; //used only by DoFloat to indicate what the last request was
byte g_cFloatExitModePosnState;

uint16 g_uiModeFlags;

//HOMESYSTEM
#ifdef HOMESYSTEM
byte g_cHomed;	//overall system home status
byte g_cSentHomed;
#ifdef HOMESYSTEM_X
byte g_cHomedX;	//X Axis
byte g_cSentHomedX;
#endif
#ifdef HOMESYSTEM_Y
byte g_cHomedY;	//Y Axis
byte g_cSentHomedY;
#endif
#ifdef HOMESYSTEM_DRILL
byte g_cDrillHomed;	//an external system
byte g_cSentDrillHomed;
#endif
#ifdef HOMESYSTEM_AAXIS
byte g_cNACAAxisHomed;	//an external system
byte g_cSentNACAAxisHomed;
#endif
#ifdef HOMESYSTEM_CLAMP
byte g_cNACClampHomed;	//an external system
byte g_cSentNACClampHomed;
#endif
#ifdef HOMESYSTEM_FASTENER
byte g_cFastenerHomed; //an external system
byte g_cSentFastenerHomed;
#endif
#ifdef HOMESYSTEM_FASTENERTRAY
byte g_cFastenerTrayHomed; //an external system
byte g_cSentFastenerTrayHomed;
#endif
//And also create this OR def
#ifdef HOMESYSTEM_AAXIS
#ifndef HOMESYSTEM_AAXIS_OR_CLAMP
#define HOMESYSTEM_AAXIS_OR_CLAMP
#endif
#endif
//check clamp axis for OR def
#ifdef HOMESYSTEM_CLAMP
#ifndef HOMESYSTEM_AAXIS_OR_CLAMP
#define HOMESYSTEM_AAXIS_OR_CLAMP
#endif
#endif
//done creating or def
#endif

//DFINT
byte g_cDrillStateGoal;
byte g_cDrillStateGoalSent;
byte g_cDrillHomeWasStarted;
byte g_cDrillStateGoalCommanded;
byte g_cDrillStateDFGoal;

byte g_cEstopMCAsserted = 0;
byte g_cEstopFullyEngaged = 1;

//ADC DAC
uint16 g_uiClampPressureLBS;
uint16 g_uiClampPressureLBSLastSet;

//state variables for specific modes
byte g_cCurrentJogDir;

// struct for holding gravcomp results and handshakes
td_GravComp g_GravComp;
// grav comp timer
uint32 g_uiGravCompTime;

// struct for holding probe results and handshakes
td_ProbeMode g_Probe;

// struct for holding Position Mode results and handshakes
td_PosnMode g_PosnMode;

// struct for holding loaded position hole information
td_HoleData g_HoleData;
// hole current holes effective operations
//	(this is the result of hole ops & process ops & tool ops)
unsigned int g_uiHoleOps;

//Global Rotation System set values
byte g_cRotationKP;
byte g_cRotationKS;
byte g_cRotationContext;
float g_fPKMX;
float g_fPKMY;
float g_fSKMX;
float g_fSKMY;
float g_fPKX;
float g_fPKY;
float g_fSKX;
float g_fSKY;
float g_fCosTheta;
float g_fSinTheta;
#ifdef LINEAR_SCALE_OPTION
float g_fCosScale;
float g_fSinScale;
float g_fScale;
float g_fUnscale;
#endif

//SendPPData System
byte g_cSendPPDataSessions;
byte g_cSendPPDataSession[MAXNUM_STP_SESSIONS]; //indicate sending to this one
uint32 g_uiSendPPDataIndex[MAXNUM_STP_SESSIONS];
uint32 g_uiSendPPDataMax[MAXNUM_STP_SESSIONS];

//SendXYData System
byte g_cSendXYDataSessions; //not a count but single flag indicating something is set
byte g_cSendXYDataSession[MAXNUM_STP_SESSIONS]; //indicate sending to this one
unsigned int g_uiSendXYDataIndex[MAXNUM_STP_SESSIONS];

//OpHistory
byte g_cSendOpHistory;
byte g_cSendOpHistoryBlockOrderIndex[MAXNUM_STP_SESSIONS];
unsigned long g_ulSendOpHistoryAfterTime[MAXNUM_STP_SESSIONS];

//Scaling Flag
byte g_cScaleBasedOnProbe;
//Flag For Linear Program Mode number
int g_iPartPgmLinearHoles;

//Inspection Flag
byte g_cInspectMethod;
//Inspection Progress Flag
byte g_cPositionInspection;

unsigned int g_uiPositionOpsCache;
byte g_cShowPrevDrilledMessage;

byte g_cFastenerMode;
byte g_cFastenerLoaded;
byte g_cFastenerState;

//FIXME FAS
#warning "FAS  DEMO FASTENER SHORTCUTS"
//There are a number of demo fastener shortcuts to reeval
//  Some has NO #ifdef control....  Some are temporary code
byte g_cUseFastenerTray;
float g_fFastenerTrayPickupPosition;
float g_fFastenerTrayClampPickupHeight;

//Additional Time Measure
unsigned long g_ulClampUnlockTime;

float g_fMachineBaseExtension; //make this an OID????

//MACHINEPOINTS
//CLOSEST_HOLE_SYSTEM
#define MPOSNHASHSIZE 256
typedef uint16 t_MPOSNBUCKETTYPE;
long xp_uiMPosnHashList;
float g_fMPosnHashMinBucket;
float g_fMPosnHashMaxBucket;
float g_fMPosnHashBucketsize;
int16 g_iMPosnBucket[MPOSNHASHSIZE];

unsigned long g_ulCreateMachineDataSetStartTime;
unsigned int g_uiCreateMachineDataSetIndex;
unsigned int g_uiCreateMachineDataSetHashIndex;
#ifdef OUTPUT_POSITION_INFO
unsigned int g_uiProgramRotationOutputIndex;
#endif
uint32 g_ulArrive;
uint32 g_ulPastTPC;
uint32 g_ulLastMoveStart;
uint32 g_ulMoveStart;
uint32 g_ulMoveEnd;
uint32 g_ulProcStart;
uint32 g_ulClampStart2;
uint32 g_ulClampDone;
uint32 g_ulProcPassed;
uint32 g_ulDrillStart;
uint32 g_ulFastenerStart;
uint32 g_ulUnclampStart;
uint32 g_ulUnclampDone;
uint32 g_ulDrillDone;
uint32 g_ulFastenerDone;
uint32 g_ulFinalTime;
uint32 g_ulDrillEchoStart;
//Start of newer cycle based system... not ready to replace old system but adding for LD, LO, Spinup etc...
uint32 g_ulLPR;
uint32 g_ulLD;
uint32 g_ulLO;
uint32 g_ulSpinUp;
unsigned int g_uiCutterAir;
byte g_cPrintAuth;

#warning "Check this buffer use and need"
char temppploadbuffer[2024];

//For use by macro
unsigned long g_ultemp;
char * p_cultemp;

//For Use with ShowIO
byte g_cShowIO;
byte g_cShowAna;

unsigned long g_ulCutterAirTotal = 1880 * 64;

////////////////////////////////////////////////////////////////////////////////
// InitVars
// Description : Initialize All Global variables
////////////////////////////////////////////////////////////////////////////////
void InitVars(void)
{
	// gravity compensation mode, float, and jog
	g_cFloatStatus = FLOATSTAT_NOFLOAT;
	g_cGravCompStatus = GRAVCOMP_NOTDONE;
	g_GravComp.iDirX = 0;
	g_GravComp.iDirY = 0;

	//Probe mode
	g_Probe.cCaptureTrig = FALSE;

	// Probe home and Probe Adjust modes
	g_Probe.cHomeTrig = FALSE;
	g_Probe.fHomeX = 0.0;
	g_Probe.fHomeY = 0.0; //FIXME Minor  May not need these older home system values
	g_Probe.cGotHome = 0;
	g_Probe.cXBumperDirection = X_BUMP_UNKNOWN;
	g_Probe.cYBumperDirection = Y_BUMP_UNKNOWN;
	g_Probe.cRegistration = REGISTRATION_UNKNOWN;
	g_Probe.cProbeAdjustTrig = FALSE;
	g_Probe.fProbeAdjustX = 0.0;
	g_Probe.fProbeAdjustY = 0.0;
	g_Probe.cGotProbeAdjust = 0;

	// Probe Teach
//Save mem since nothing is using this now
//	strnull(g_Probe.szTeachPosnName);
	g_Probe.cTeachCaptureTrig = FALSE;
	g_Probe.cGotTeachCoords = 0;

	g_cSendPPDataSessions = 0;
	g_cSendXYDataSessions = 0;
	//Taken Care of by ClearProgram  g_cSendXYDataSession[i] = 0;
	g_cSendOpHistory = 0;

	g_cScaleBasedOnProbe = 0;
	g_iPartPgmLinearHoles = 0;

	g_cInspectMethod = 0;
	g_cPositionInspection = 0;

	g_uiPositionOpsCache = 0;
	g_cShowPrevDrilledMessage = 0;

	g_cFastenerMode = 0;
	g_cFastenerLoaded = 0;
	g_cFastenerState = 0;

	g_cUseFastenerTray = 1; //start on by default
	g_fFastenerTrayPickupPosition = 0; //zero is default
	g_fFastenerTrayClampPickupHeight = 0; //zero is default
	//Positioning Mode
	g_PosnMode.iStartPosnIndex = 0;
	g_PosnMode.iCurPosnIndex = 0;
	g_PosnMode.cOnCurPosn = 0;
	g_PosnMode.iGotoPosnIndex = -1; // "next" position to go to...could be next, previous, or random
	g_PosnMode.cDoMoveTrig = FALSE;
	g_cMoveDone = MOVEDONE_TRUE; // initialize to true because carriage is not moving (it is "done moving") at power-up
	g_PosnMode.cFreshCurPosn = 0;
	g_PosnMode.cFreshCurPosnSend = 0;
	g_PosnMode.fLastKnownPosnX = 0.0;
	g_PosnMode.fLastKnownPosnY = 0.0;
	g_PosnMode.fPosnX = 0.0;
	g_PosnMode.fPosnY = 0.0;
	g_PosnMode.cFreshPosn = 0;
	g_PosnMode.iNearestPosn = -1;
	g_PosnMode.iLastSentNearestPosn = -1;

	// general
	g_uiModeFlags = 0;
	g_cModeState = MODE_IDLE;

//MakeOID-generated:: GLOBAL DEFAULTS (oid complete)
	g_StpStatus.uiOID = 0;
	g_StpStatus.uiStatus = STPSTAT_OK;
	g_DateTime.ulticksMSW = 0;
	g_DateTime.ulticksLSW = 0;
	g_DateTime.uiyear = 0;
	g_DateTime.cmonth = 0;
	g_DateTime.cdayOfMonth = 0;
	g_DateTime.chour = 0;
	g_DateTime.cminute = 0;
	g_DateTime.csecond = 0;
	g_DateTime.uimillisecond = 0;
	g_cModeState = 0;
	g_cAction = ACTION_IDLE;
	g_cEEOption = EEOPTION_DEFAULT;
	g_HoleParam.cProcess = 0;
	g_HoleParam.cToolType = 0;
	g_HoleParam.cFastenerType = 0;
	g_HoleParam.cLayers = 0;
	g_HoleParam.cCountersink = 0;
	g_HoleParam.uiOperations = 0;
	g_HoleParam.fDiameter = 0;
	g_HoleParam.fProcessCountersink = 0;
	g_HoleParam.fHoleCountersinkAdjust = 0;
	g_HoleParam.flayer1 = 0;
	g_HoleParam.flayer2 = 0;
	g_HoleParam.flayer3 = 0;
	g_HoleParam.flayer4 = 0;
	g_HoleParam.flayer5 = 0;
	g_HoleParam.flayer6 = 0;
	g_HoleParam.flayer7 = 0;
	g_HoleParam.flayer8 = 0;
	g_HoleParam.cmat1 = 0;
	g_HoleParam.cmat2 = 0;
	g_HoleParam.cmat3 = 0;
	g_HoleParam.cmat4 = 0;
	g_HoleParam.cmat5 = 0;
	g_HoleParam.cmat6 = 0;
	g_HoleParam.cmat7 = 0;
	g_HoleParam.cmat8 = 0;
	g_cLoadedProcess = 0;
	g_fGaugeLength = 0;
	strnull(g_szPartPgmFilename);
	g_cPartPgmStatus = 0;
	g_cGravCompStatus = 0;
	g_cFloatStatus = 0;
	g_GravCompResults.fxp = 0;
	g_GravCompResults.fxn = 0;
	g_GravCompResults.fyp = 0;
	g_GravCompResults.fyn = 0;
	g_fXRailSurfaceOffset = 2;
	g_cProbeMethod = 0;
	g_cPattern = 0;
	g_cMoveDone = 1;
	g_MachineOffset.fX = 0;
	g_MachineOffset.fY = 0;
	g_cMachineOffsetCompensationAdjustment = 0;
	g_MachineOffset1.fX = 0;
	g_MachineOffset1.fY = 0;
	g_MachineOffset1.fYExtension = 0;
	g_MachineOffset2.fX = 0;
	g_MachineOffset2.fY = 0;
	g_MachineOffset2.fYExtension = 0;
	g_cStation = STATION_UNSPEC;
	g_JogFactor.fX = 1.0;
	g_JogFactor.fY = 1.0;
	g_cALockMode = 0;
	memset(&g_VisionInspectResults, 0, sizeof(td_VisionInspectResults));
#ifdef GENCIRCMFTX
	g_cDrillState = DRILLSTATE_IDLE;
#else
//FIXME deal with rivet dif here.... don't want to have ot ifdef all this
#ifndef GENRIVET_ALL
	g_cDrillState = DRILLSTATE_INIT;
#endif
#endif
	g_cDrillExplanation = 0;
	g_cSealState = 0;
	g_cFillState = 0;
	g_cFastenerArrived = 0;
	g_cAccelArm_ = 0;
	g_cAccelTrigger_ = 0;
	g_FastenerFault.cDevice = 0;
	g_FastenerFault.cSeverity = 0;
	g_FastenerFault.lFaultCode = 0;
	g_DrillFault.cDevice = 0;
	g_DrillFault.cSeverity = 0;
	g_DrillFault.lFaultCode = 0;
	g_HoleResultData.iHoleNumber = -1;
	g_HoleResultData.iHoleResult = HR_UNKNOWN;
	g_RFIDData.cstate = RFID_INIT;
	g_RFIDData.ccontext = RFID_CONTEXT_SEEK;
	g_RFIDData.cseekstate = RFID_SEEK_NULL;
	g_RFIDData.ultimestamp = 0;
	g_RFIDData.ulrfidtimestamp = 0;
	g_RFIDData.fposition = 0;
	g_szTagDatalen = 0;
	g_szTagData[0] = 0;
	g_RFIDData.uicrc16 = 0;
	g_RFIDData.uiendcode = 0;
	g_RFIDData.ulseektime = 0;
	//g_RFIDData.fsstart = ;
	//g_RFIDData.fpstart = ;
	//g_RFIDData.fpend = ;
	//g_RFIDData.fnstart = ;
	//g_RFIDData.fnend = ;
	//g_RFIDData.fhs1 = ;
	//g_RFIDData.fhs2 = ;
	//g_RFIDData.fhsf = ;
	g_cReadRFID = RFID_READ_OFF;
	g_ForceSensor.fX = 0;
	g_ForceSensor.fY = 0;
	g_ForceSensor.fZ = 0;
	g_ForceSensor.cErrFlag = 0;
	//Tool Management OID globals
	g_szToolIDlen = 0;
	g_szToolID[0] = 0;
	g_szToolTypeCodelen = 0;
	g_szToolTypeCode[0] = 0;
//MakeOID-generated::END

	g_cActionSent = 0xFF;
	g_cHold = 0;

#ifdef HOMESYSTEM_X_RFID
	g_fRFIDMachineX = 0;
	g_fRFIDRailX = 0;
	g_iRFIDRailOrientation = 1;
#endif

	g_cPosnModeState = POSNMODE_INIT;
	g_cEstopPrevMode = MODE_IDLE;

	g_uiActionCycles = 0;
	g_iActionHoleIndex = -1;
	g_cSafeUnclamp = 0;
	g_cSawDrillMode = 0;
	g_cTestOpt = 0;
	g_cLubeBypass = 0;
	g_cFastDoneFlag = 0;
	g_cCutterDetected = CUTTER_UNKNOWN;
	g_cLastCutterDetected = CUTTER_UNKNOWN;
	g_uiMessageCode = 0;

#ifdef TOOL_IN_RAM
	//don't over write this ram... see if it's value remains after init
#else
	g_LoadedTool.cOperation = 0;
	g_LoadedTool.cToolStatus = 0;
	g_LoadedTool.cHardstop = 0;
	g_LoadedTool.fDiameter = 0;
	g_LoadedTool.fLength = 0;
	g_LoadedTool.fMinBreak = 0;
	g_LoadedTool.fLengthCountersink = 0;
	g_LoadedTool.fCountersinkAdjust = 0;
	g_LoadedTool.ulDTimeTicksMSW = 0;
	g_LoadedTool.ulDTimeTicksLSW = 0;
	g_LoadedTool.uiDCount = 0;
	g_LoadedTool.uiDWarnCount = 0;
	g_LoadedTool.uiDLimitCount = 0;
#endif
	g_ToolServerSTPSession = (td_STPsessions *) 0;

	g_cToolCount = 0; //clear
	g_cProcessCount = 0;
	g_cFastenerCount = 0;
	g_cPatternCount = 0;
	g_iHoleCount = 0;
	g_cKHoleCount = 0;

	ClearProbeCommand();

	g_cKHolePrbeCommand = 0;
	g_cKHolePrbeCommandInput = 0;
	g_cKHolePrbeTrigger = 0;
	g_cProbeComplete = 0;
	g_cProbeCalculated = 0;
	g_cKHoleLastCount = 0;
	//ClearGolbalRotation(); //just clear two variables
	g_cRotationKP = 0;
	g_cRotationKS = 0;
	g_cRotationContext = 0;

	g_cLoadedTool = 0; //FIXME NOW if tool ram, then exlucde this
	g_cLastSearchedRequiredTool = 0;

	g_cProcessLayerCount[0] = 1; //Default Process
	g_cProcessMaterials[0] = 0; //Should Really not supprt this for drilling, but
	g_uiProcessPounds[0] = 50; //on simple machines there is no point in requiring process at all.
	g_uiProcessPoundsWarning[0] = 0;
	g_uiProcessPoundsAbort[0] = 0;
	g_cProcessOps[0] = 255;
	g_uiProcessCountersink[0] = 500;

	g_ulMachineDataSetIDTimeSec = 0;
	g_ulMachineDataSetIDTime = 0;
	g_cBlockCount = 0;

	g_cStartProcess = 0;
	g_cStationGoal = STATION_UNSPEC;
	g_cStartStation = STATION_DRILL;
	g_cStationPlanDrill = 1;
	g_cStationPlanInspect = 1;
#ifdef SEAL_SYSTEM
	g_cStationPlanSeal = 1;
#endif
#ifdef FASTENER_SYSTEM
	g_cStationPlanFill = 1;
	g_cStationPlanRemove = 0;
#endif
	g_cStationCode = "udspfmil##"; //For showing current station

	g_cDrillStatePrev = 255;
	g_ulDrillStateStartTime = 0;

	g_TeachMachinePosn.fX = 0.0;
	g_TeachMachinePosn.fY = 0.0;

	g_sPPDirect = (char*) 0;
	g_iPPDirectPos = 0;
	g_iPPDirectEnd = 0;
	g_cPPDirectFinalCall = 0;

	//HD + All Clamping Systems
#ifdef CLAMP_SYSTEM
#ifdef CLAMP_SYSTEM_NAC_STP
	//only set state unknown
	g_cClampState=CLAMP_UNKNOWN;
	g_cClampGoal=CLAMP_UNKNOWN;
#else
	g_cClampState = CLAMP_UNKNOWN;
	g_cClampGoal = CLAMP_LOOSE_OR_UNCLAMP;
#endif
	g_cClampStateSent = 0xFF;
	g_cClampGoalSent = 0xFF;
#endif

	//HD
	g_uiClampPressure = 100;
	g_uiClampPressureWarning = 0;
	g_uiClampPressureAbort = 0;

#ifdef CENTERVISION
	g_cCenterVisionResult = CENTERVISION_OFF;
#endif

	//DFINT
	g_cDrillStateGoal = DRILLSTATE_IDLE;
	g_cDrillStateGoalSent = 0xFF;
	g_cDrillHomeWasStarted = 0;
	g_cDrillStateGoalCommanded = 0xFF;
	g_cDrillStateDFGoal = 0xFF;
#ifdef SEAL_SYSTEM
	g_cSealantApply = SEALANT_APPLY_OFF;
	g_cSealantPressure = SEALANT_PRESSURE_OFF;
	g_cSealantPinch = SEALANT_PINCH_ON;
#endif

	//HOMESYSTEM  //FIXME0000000 fix common-order hierarchy
#ifdef HOMESYSTEM
	g_cHomed = HOME_NOT_DONE;	//overall system home status
	g_cSentHomed = 0xFF;
	ClearAllHomeStatus();
	ClearSentHomeStatus();
#endif

	g_cImmediateFakeRfid = 0; //temp feature may be added in nicer way later

	//Pending Ops
	g_cClear = 0;
#ifdef CLAMP_SYSTEM_NAC_STP
	g_cNACClear = 0;
#endif
//	g_cAction = 0;

	g_cJogX = 0;
	g_cJogY = 0;
	g_cJogGoalX = 0;
	g_cJogGoalY = 0;
	g_fJogX = 0; //The status speed factor
	g_fJogY = 0; //The status speed factor
	g_fJogGoalX = 0; //The goal speed factor
	g_fJogGoalY = 0; //The goal speed factor
	g_uiJogStopX = 0;
	g_uiJogStopY = 0;

	//Position Update
	g_uiPositionUpdateTime = 0;
	g_uiPositionSendTime = 0;
	g_uiPositionUpdateThrottle = POSITION_UPDATE_THROTTLE_DEFAULT;
	g_uiPositionSendThrottle = POSITION_SEND_THROTTLE_DEFAULT;
	g_fPosnLSCD = 0.00025;

	g_fHomePositionX = 0;
	g_fHomePositionY = 0;
	g_fPositionMinX = -1000;
	g_fPositionMaxX = 1000;
	g_fPositionMinY = -1000;
	g_fPositionMaxY = 1000;

	//FIXME MED Pattern
	//g_iChosenPattern=0;

	g_cDrillLoadProcessAndOverride = DRILL_LOAD_PROCESS_NOT_NEEDED;
	g_cDrillSync = 0;
	g_fLastSentHomeBack = 0.0000001;

	g_iDrillDir = DRILLDIR_ATOB;

	ResetNearestPosition();

	g_cAllowKVisit = 0;
	g_cAutoMove = 0;
	g_uiAutoTime = 0;
	g_uiStartAutoTime = 0;
	g_cAutoRepeat = 0;
	g_cLastAutoMove = 3;
	g_cFloatGoal = 255;
	g_cFloatExitModePosnState = 0;
	g_iProbedIndex = -1;
	//Tool Sync
	g_cToolLoaded = 0;
	g_cProcessLoaded = 0;
	g_cOverrideCalculated = 0;
	g_cOverrideSent = 0;
	g_cLayerOverrides = 0;

#ifdef OUTPUT_POSITION_INFO
	g_uiProgramRotationOutputIndex = 1;
#endif

	g_uiCreateMachineDataSetHashIndex = 0; //0 indicates that we have not created the hash and avoids getting stuck in the nearest posn calc routines

	g_cShowIO = 0;
	g_cShowAna = 0;

	//Call Function to init Part Program Vars
	ClearPartProgram();

	g_fMachineBaseExtension = 0;

	p_cultemp = (char *) &g_ultemp;
	return;
}

////////////////////////////////////////////////////////////////////////////////
// SessionStarting
////////////////////////////////////////////////////////////////////////////////
void SessionStarting(td_STPsessions * p_STPSession)
{
	int iSession;
#ifdef OUTPUT_SESSION_EVENTS
	logf("S Strt\r\n");
#endif
	iSession = p_STPSession->iSessionNum;
#ifdef CLAMP_SYSTEM_NAC_STP
	if ( iSession == -1)
	{
		//special NAC connection
		//MiniFt is the client and the STP Server is NAC.
		//IDENTIFY
		NACIdentify();
		return;
	}
#endif
#ifdef DRILL_DIRECT_READY
	if ( iSession == -2)
	{
		logf("SD R\r\n");
		ClearDrillSync();
		SmartDrillResync();
		LoadToolToDrill();
		return;
	}
#endif
	if (iSession == -3)
	{
		//df
		g_cDrillSync = DRILL_SYNC; //no sync requirements for df
		return;
	}
	else if (iSession == -4)
	{
#ifdef FASTENER_STP
		FastenerResync();
#endif
	}
	else if (iSession == -5)
	{
#ifdef FASTENER_TRAY_STP
		FastenerTrayResync();
#endif
	}
}

////////////////////////////////////////////////////////////////////////////////
// SessionDelay
////////////////////////////////////////////////////////////////////////////////

void SessionDelay(td_STPsessions * p_STPSession)
{
	int iSession;
	iSession = p_STPSession->iSessionNum;
//	if (iSession != -2)
//	{
//If Drill has delay, it is only due to timing differences in the settings.
//We could repair this someday.
//Ideally all of the links could support the same highly responsive timing parameters
//FIXME MINOR  session delay timing improvement
#ifdef OUTPUT_SESSION_EVENTS
	logf("S Dly\r\n");
#endif
//	}

	if (iSession >= 0)
	{
		//Stop Interface Tasks 1st.
		StopInterfaceTasks();
	}
}

////////////////////////////////////////////////////////////////////////////////
// SessionClosing
////////////////////////////////////////////////////////////////////////////////

void SessionClosing(td_STPsessions * p_STPSession)
{
	int iSession;
#ifdef OUTPUT_SESSION_EVENTS
	logf("S Cls\r\n");
#endif

	iSession = p_STPSession->iSessionNum;
	if (iSession < 0)
	{
#ifdef CLAMP_SYSTEM_NAC_STP
		if ( iSession == -1)
		{
			//special NAC connection
			//MiniFt is the client and the STP Server is NAC.
			//NAC has close the session???
			g_cClampState = CLAMP_UNKNOWN;
			g_szRailSerialNumber[0]=0;
			return;
		}
#endif
#ifdef DRILL_DIRECT_READY
		if ( iSession == -2)
		{
			//smartdrill
			ClearDrillSync();
			return;
		}
#endif
		if (iSession == -3)
		{
			//df
			g_cDrillSync = 0;
			//Must Load Process Again
			if (g_cDrillLoadProcessAndOverride > DRILL_LOAD_PROCESS_NEEDED)
			{
				g_cDrillLoadProcessAndOverride = DRILL_LOAD_PROCESS_NEEDED;
			}
			return;
		}
	}
	//Stop Interface Tasks 1st.
	StopInterfaceTasks();

	//Stop Any Auto Move
	g_cAutoMove = 0;
	g_uiStartAutoTime = 0;
	g_cAutoRepeat = 0;

	//FIXME DISABLE  Rivet had a nice disable system that could disable the entire thing by a call to one function
	//	MiniFT might need a disable/enable like that some day, but it would be different in some ways.
	//If I do it I should review RivetDisable from the archives

	g_cSendPPDataSessions = 0; //FIXME SEVERE  really need to stop ONLY this session
	g_cSendXYDataSession[iSession] = 0;
	g_cSendOpHistoryBlockOrderIndex[iSession] = 0xFF;

	if (p_STPSession == g_ToolServerSTPSession)
	{
		g_ToolServerSTPSession = (td_STPsessions *) 0; //clear this
	}
}

////////////////////////////////////////////////////////////////////////////////
// Stop Interface High Speed Communication Dependant Tasks
////////////////////////////////////////////////////////////////////////////////

void StopInterfaceTasks()
{
	//Stop Jog
	logf("si\r\n");
	g_cJogGoalY = JOGSTOP;
	g_cJogGoalX = JOGSTOP;
}

#ifdef DRILL_DIRECT_READY
//Resent Drill Sync Variables
void ClearDrillSync()
{
	//Clear Sync
	g_cDrillSync = 0;
	g_fLastSentHomeBack = 0.0000001;
	//Must Load Process Again
	if (g_cDrillLoadProcessAndOverride>DRILL_LOAD_PROCESS_NEEDED)
	{
		g_cDrillLoadProcessAndOverride = DRILL_LOAD_PROCESS_NEEDED;
	}
	//Always Erase These
	g_cOverrideCalculated = 0;
	g_cLoadedProcess = 0;
}
#endif

void EstopEngageActions(void)
{
	BrakeOn();
	if (g_cEstopMCAsserted == 1)
	{
		//No Motor Power, so run EstopEngage Right Now
		MCRunEstopEngage();
		g_cEstopFullyEngaged = 1;
#ifdef OUTPUT_ESTOP
		logf("Estop Full\r\n");
#endif
	}
	else
	{
		//Still Have Motor Power, so Run Servo Here
		MCRunEstopWithPower();
		g_cEstopFullyEngaged = 0;
#ifdef OUTPUT_ESTOP
		logf("Estop W Pow\r\n");
#endif
	}

	g_cAction = ACTION_ESTOP;

	ClearToolOffset(); //FIXME00000 is this a problem for returning to position mode???
	SetDefaultValveState();

#ifdef CLAMP_SYSTEM_NAC_STP
	//Do almost nothing in the case of Estop, but do cancel any goal. NAC will set state to hold in most cases also
	if (g_cClampGoal!=g_cClampState)
	{
		g_cClampGoal=CLAMP_HOLD;
	}
#endif

	g_PosnMode.cOnCurPosn = 0;	//moving actions always mean not on position
	SendCurPosnInd(0, STP_ALERT); //Send Cleared Cur Posn Index

	g_cMoveDone = MOVEDONE_TRUE;  // not moving now.
	SmartToolMsgChar(0, STP_ALERT, MINIFT_OID_POSNMODE_MOVEDONE, g_cMoveDone);
	g_cFloatStatus = FLOATSTAT_NOFLOAT;
	SmartToolMsgChar(0, STP_ALERT, MINIFT_OID_FLOAT_STATUS, g_cFloatStatus);
#ifdef BEEPSYSTEM
	if (g_cBeepMode < BEEPSIGNALS)
	{
		BeepOff()
		;
	}
#endif

#ifdef HOMESYSTEM
	//If Home was running, must assume now that it's not and that it failed.
	if (g_cHomed == HOME_RUNNING)
	{
		g_cHomed = HOME_NOT_DONE;
	}
#ifdef HOMESYSTEM_DRILL
	if (g_cDrillHomed == HOME_RUNNING || g_cDrillHomed == HOME_PENDING)
	{
		g_cDrillHomed = HOME_NOT_DONE;
		g_cDrillStateGoal = DRILLSTATE_IDLE;
	}
#endif
#ifdef HOMESYSTEM_CLAMP
	if (g_cNACClampHomed == HOME_RUNNING || g_cNACClampHomed == HOME_PENDING)
	{
		g_cNACClampHomed = HOME_NOT_DONE;
	}
#endif
#ifdef HOMESYSTEM_FASTENER
	if (g_cFastenerHomed == HOME_RUNNING || g_cFastenerHomed == HOME_PENDING)
	{
		g_cFastenerHomed = HOME_NOT_DONE;
	}
#endif
#ifdef HOMESYSTEM_X_LINK
	if (g_cHomedX == HOME_RUNNING || g_cHomedX == HOME_PENDING)
	{
		g_cHomedX = HOME_NOT_DONE;
	}
#endif
#ifdef HOMESYSTEM_Y_LINK
	if (g_cHomedY == HOME_RUNNING || g_cHomedY == HOME_PENDING)
	{
		g_cHomedY = HOME_NOT_DONE;
	}
#endif
#ifdef HOMESYSTEM_AAXIS
	if (g_cNACAAxisHomed == HOME_RUNNING || g_cNACAAxisHomed == HOME_PENDING)
	{
		logf("nag=>nd\r\n");
		g_cNACAAxisHomed = HOME_NOT_DONE;
	}
#endif
#ifdef HOMESYSTEM_FASTENERTRAY
	if (g_cFastenerTrayHomed == HOME_RUNNING || g_cFastenerTrayHomed == HOME_PENDING)
	{
		g_cFastenerTrayHomed = HOME_NOT_DONE;
	}
#endif
#ifdef HOMESYSTEM_X_RFID
	if (g_cHomedX == HOME_RUNNING || g_cHomedX == HOME_PENDING)
	{
		g_cHomedX = HOME_NOT_DONE;
	}
	if (g_RFIDData.cseekstate < RFID_SEEK_TERMINAL_STATE)
	{
		g_RFIDData.cseekstate = RFID_SEEK_NULL;
	}
#endif
#endif

	//force them to grav comp again by deciding it has not been done yet.
	g_cGravCompStatus = GRAVCOMP_NOTDONE;
	SmartToolMsgChar(0, STP_ALERT, MINIFT_OID_GRAVCOMP_STATUS, g_cGravCompStatus);

#ifdef CENTERVISION
	g_cCenterVisionResult = CENTERVISION_OFF;
#endif

#ifdef GENCIRCMFTX
	//DO NOT RESET DRILL STATE	g_cDrillState = DRILLSTATE_IDLE;
	g_cDrillButton = START_DRILL_OFF;
	digOut(DO_START_DRILL, g_cDrillButton);
#endif

	g_cStartProcess = 0;
	//Clear Any Triggered Move
	g_PosnMode.cDoMoveTrig = FALSE;
	g_cDrillState = DRILLSTATE_ESTOP;
#ifdef SEAL_SYSTEM
	g_cSealState=SEALSTATE_OFF;
#endif
#ifdef FASTENER_SYSTEM
	g_cFillState=FILLSTATE_OFF;
	g_cFastenerArrived=0; //allow clearance, but hope it will redetect it
	//Fastener Abort
	FastenerSTPSet(FASTENER_OID_ABORT);
#ifdef FASTENER_TRAY_STP
	FastenerTraySTPSet(FASTENERTRAY_OID_ABORT);
#endif
#endif
}

void EstopContinueActions(void)
{
	if (g_cEstopFullyEngaged != 1)
	{
		//Still using Power
		if (g_cEstopMCAsserted == 1)
		{
			//No Motor Power, so run EstopEngage Right Now
			MCRunEstopEngage();
			g_cEstopFullyEngaged = 1;
#ifdef OUTPUT_ESTOP
			logf("Estop Full\r\n");
#endif
		}
	}
	if (g_cEstopMCAsserted == 1)
	{
		//After g_cEstopMCAsserted has had it's effect
		//it can be cleared, but only if they put have the main estop signaled
		//Both Estop Switch and Estop Asserted
		if (g_cDigInEstopSignal == ESTOP_SIGNALED)
		{
			//Estop is signaled
			g_cEstopMCAsserted = 0;
		}
	}

	g_cAction = ACTION_ESTOP;

	return;
}

void EstopDisengageActions(void)
{
	MCRunEstopDisengage();
#ifdef GENCIRCMFTX
	g_cDrillState = DRILLSTATE_INIT;
#endif
	g_cEstopFullyEngaged = 0;
}

void PressureLossActions(void)
{
	return; //FIXME0 Want to test later.......
//FIXME00000000000 because when it was on there was a major issue......
//  BUT I NEED SOME OF THIS TO BE SAFE.... TEST LATER....

	BrakeOn();

	g_PosnMode.cOnCurPosn = 0;	//moving actions always mean not on position
	SendCurPosnInd(0, STP_ALERT); //Send Cleared Cur Posn Index

	//stop motion
	//Ensure it will stop in posn mode
	if (g_cModeState == MODE_POSN)
	{
		if (g_cPosnModeState > POSNMODE_WAITNEXTPOSN)
		{
			g_cPosnModeState = POSNMODE_STOP_MOVEMENT;
		}
	}
#ifdef BEEPSYSTEM
	if (g_cBeepMode < BEEPSIGNALS)
	{
		BeepOff()
		;
	}
#endif
	//stop float
	if (g_cFloatStatus == FLOATSTAT_FLOAT)
	{
		DoFloat(FLOAT_UNFLOAT_STOP);
	}
	g_cFloatExitModePosnState = 0;
	//stop jog
	g_cJogGoalX = JOGSTOP;
	g_cJogGoalY = JOGSTOP;

	//force them to grav comp again by deciding it has not been done yet.
	g_cGravCompStatus = GRAVCOMP_NOTDONE;
	SmartToolMsgChar(0, STP_ALERT, MINIFT_OID_GRAVCOMP_STATUS,
			g_cGravCompStatus);

#ifdef CENTERVISION
	g_cCenterVisionResult = CENTERVISION_OFF;
#endif

#ifdef GENCIRCMFTX
	g_cDrillButton = START_DRILL_OFF;
	digOut(DO_START_DRILL, g_cDrillButton);
#endif

	g_cStartProcess = 0;
	//Clear Any Triggered Move
	g_PosnMode.cDoMoveTrig = FALSE;
	g_cDrillStateGoal = DRILLSTATE_IDLE;
#ifdef SEAL_SYSTEM
	g_cSealState=SEALSTATE_OFF;
#endif
#ifdef FASTENER_SYSTEM
	g_cFillState=FILLSTATE_OFF;
	//Fastener Abort
	FastenerSTPSet(FASTENER_OID_ABORT);
#ifdef FASTENER_TRAY_STP
	FastenerTraySTPSet(FASTENERTRAY_OID_ABORT);
#endif
#endif
}

//STP MiniFt Definitions

////////////////////////////////////////////////////////////////////////////////
// STP: BuildMiniFtMessageCode
// builds an STP_ALERT using MINIFT_OID_MINIFT_MESSAGE_CODE
////////////////////////////////////////////////////////////////////////////////
//FIXME0 headers
void SmartToolMsgMiniFtMessageCode(td_STPsessions * p_STPSession, unsigned int uiOID, unsigned int uiMessageCode)
{
	unsigned int ui;
	td_oid_minift_message_code * p_oid_minift_message_code;

#ifdef OUTPUT_MiniFtMC
	logf("STP MiniFtMC:s=%s mc=%d\r\n", DisplayOIDName(p_STPSession, uiOID), uiMessageCode);
#endif
#ifdef PACKET_FLOOD_MONITOR
	CheckPacketFlood(OID_MINIFT_MESSAGE_CODE);
#endif

	g_STPtxMsg.uiVersion = htons(STP_VERSION);
	g_STPtxMsg.uiMsgType = htons(STP_ALERT);
	g_STPtxMsg.uiOID = htons(MINIFT_OID_MINIFT_MESSAGE_CODE);
	g_STPtxMsg.uiValueLength = htons(sizeof(td_oid_minift_message_code));

	p_oid_minift_message_code =
			(td_oid_minift_message_code *) &g_STPtxMsg.p_cObjectValue;
	p_oid_minift_message_code->uiOID = htons(uiOID);
	p_oid_minift_message_code->uiCode = htons(uiMessageCode);

	ui = (STP_HEADERSIZE + sizeof(td_oid_minift_message_code));
	SendSTPmsg(p_STPSession, &g_STPtxMsg, ui);
}

////////////////////////////////////////////////////////////////////////////////
// STP: HandleSmartToolMsg
////////////////////////////////////////////////////////////////////////////////
int HandleSmartToolMsg(td_STPmsg* p_STPrxMsg, td_STPsessions * p_STPSession)
{
	// a reference to the socket
	// declare variables for transmitting STP messages (responses, alerts)
	char* p_cSTPobjValBuff;
	char* p_cSTPobjVal; //for use, when copy is not needed

	unsigned int uiOID;
	unsigned int uiMessageCode;

	int i;
	float f; //temp float, just called "f"
	unsigned long ul; //temp unsigned long, just called "ul"
	char* p_c;
	//char* p_c_end;
	char* s;
	long* p_l;
	float* p_f;
	int iTemp;
	int iPos;
	int sr;
	unsigned int ui;
	long lTemp;
	int ilen;
	byte c, ccode;
	int iNextPosn, iPrevPosn;
	char * p_szSmartTool; //because a few things can be turned on without main OUTPUT_RXSTP
#ifdef OUTPUT_RXSTP
	char * p_szOIDName;
#endif
	//char * p_cFIXME;   //this helps test compiling when using new makeoid code, but must commented out after.

	//MakeOID-generated:: GET AND SET DECLARATIONS (oid complete)

	td_StpStatus * p_StpStatus;
	//td_oid_common_message_code * p_oid_common_message_code;
	td_oid_direct_dout * p_oid_direct_dout;
	td_oid_direct_din * p_oid_direct_din;
	td_DateTime * p_DateTime;
	//td_oid_minift_message_code * p_oid_minift_message_code;
	td_EncoderRatio * p_EncoderRatio;
	td_MCCurrentLimit * p_MCCurrentLimit;
	td_MCPeakCurrentLimit * p_MCPeakCurrentLimit;
	td_oid_system_monitor * p_oid_system_monitor;
	//td_HoleParam * p_HoleParam;
	td_GravCompSpeed * p_GravCompSpeed;
	td_GravCompAcc * p_GravCompAcc;
	td_GravCompDec * p_GravCompDec;
	td_GravCompMoveDist * p_GravCompMoveDist;
	td_GravCompNoiseLimit * p_GravCompNoiseLimit;
	td_GravCompTriggerFactor * p_GravCompTriggerFactor;
	td_GravCompResults * p_GravCompResults;
	td_ProbeOffset * p_ProbeOffset;
	//td_oid_khole_distance * p_oid_khole_distance;
	td_oid_probe * p_oid_probe;
	td_oid_probe_position * p_oid_probe_position;
	td_oid_probe_status * p_oid_probe_status;
	//td_oid_probe_start * p_oid_probe_start;
	td_HomeSpeed * p_HomeSpeed;
	td_HomeAcc * p_HomeAcc;
	td_HomeDec * p_HomeDec;
	td_HomeMoveDist * p_HomeMoveDist;
	td_HomeFineSpeed * p_HomeFineSpeed;
	//td_oid_home_report * p_oid_home_report;
	//td_oid_posnmode_curxy * p_oid_posnmode_curxy;
	//td_oid_posnmode_nearxy * p_oid_posnmode_nearxy;
	td_PreMove * p_PreMove;
	td_PosnSpeed * p_PosnSpeed;
	td_PosnAcc * p_PosnAcc;
	td_PosnDec * p_PosnDec;
	td_PosnFinalSpeed * p_PosnFinalSpeed;
	td_PosnErrLimit * p_PosnErrLimit;
	td_VelErrLimit * p_VelErrLimit;
	td_LongDistance * p_LongDistance;
	td_LongSpeed * p_LongSpeed;
	td_MachineOffset * p_MachineOffset;
	td_MachineOffset1 * p_MachineOffset1;
	td_MachineOffset2 * p_MachineOffset2;
	td_oid_tool_offset * p_oid_tool_offset;
	td_DrillOffset1 * p_DrillOffset1;
	td_DrillOffset2 * p_DrillOffset2;
	td_OffsetSeal * p_OffsetSeal;
	td_OffsetFill * p_OffsetFill;
	td_oid_jog * p_oid_jog;
	td_JogSpeed * p_JogSpeed;
	td_JogAcc * p_JogAcc;
	td_JogDec * p_JogDec;
	td_JogFactor * p_JogFactor;
	td_HomePosnYPos * p_HomePosnYPos;
	td_PosnLimitYPos * p_PosnLimitYPos;
	td_HomePosnYNeg * p_HomePosnYNeg;
	td_PosnLimitYNeg * p_PosnLimitYNeg;
	td_LaserSensorOffset * p_LaserSensorOffset;
	td_CamOffset * p_CamOffset;
	//td_oid_vision_image * p_oid_vision_image;
	//td_VisionInspectResults * p_VisionInspectResults;
	td_LaserSensorAlgParam * p_LaserSensorAlgParam;
	td_CamAlgParam * p_CamAlgParam;
	td_PrimeDelay * p_PrimeDelay;
	td_FastenerFault * p_FastenerFault;
	td_DrillFault * p_DrillFault;
	td_HoleResultData * p_HoleResultData;
	//td_oid_movexy * p_oid_movexy;
	td_PosnDisplay * p_PosnDisplay;
	td_RFIDConfig * p_RFIDConfig;
	td_RFIDData * p_RFIDData;
	td_oid_tool_mgmt * p_oid_tool_mgmt;
	td_oid_tool_rec * p_LoadedTool;
	td_ForceSensorCalibration * p_ForceSensorCalibration;
	td_ForceSensor * p_ForceSensor;
	td_ForceLimits * p_ForceLimits;
	td_MOCal * p_MOCal;
	td_oid_rfid_tag_set * p_oid_rfid_tag_set;
	td_SystemComponents * p_SystemComponents;
	td_oid_rfid_tag_set2 * p_oid_rfid_tag_set2;
	//MakeOID-generated::END

	//td_SmartDrillFault * p_SmartDrillFault;
#ifdef FASTENER_STP
	td_FastenerXYOffset * p_FastenerXYOffset;
	td_FastenerSTATEANDSTATUS * p_Fastener;
#endif

#ifdef DRILL_DIRECT_READY
	td_SmartDrillHoleData * p_SmartDrillHoleData;
#endif
	td_RFID_F0 * p_rfid_f0;
	byte c_op;
	byte c_arg1;

	p_cSTPobjValBuff = g_STPtxMsg.p_cObjectValue; //for output value, reference the value location in STP tx

	uiOID = p_STPrxMsg->uiOID;

	if (p_STPSession->iSessionNum >= 0) //MiniFT's Client Sent this message
	{
		//p_cFIXME="FIXME";
#ifdef OUTPUT_RXSTP
		p_szSmartTool = "";
#ifndef SHOW_STP_HEARTBEAT
		if (uiOID!=OID_NULLOID)
#endif
		{
			if (uiOID!=OID_POSNMODE_CURXY && uiOID!=OID_POSNMODE_NEARPOSN && uiOID!=OID_POSNMODE_NEARXY &&
					uiOID!=OID_KHOLE_DISTANCE &&
					p_STPrxMsg->uiValueLength<0xFFF0)
			{
				//logf("RX STP: v%d %d-%s %d-%s %d\r\n", p_STPrxMsg->uiVersion, p_STPrxMsg->uiMsgType, DisplayMessageTypeName(p_STPrxMsg->uiMsgType), p_STPrxMsg->uiOID, DisplayOIDName(p_STPrxMsg->uiOID), p_STPrxMsg->uiValueLength);
				logf("RX STP: %d-%s %d-%s %d\r\n", p_STPrxMsg->uiMsgType, DisplayMessageTypeName(p_STPrxMsg->uiMsgType), uiOID, DisplayOIDName(uiOID), p_STPrxMsg->uiValueLength);
			}
		}
#endif

		switch (p_STPrxMsg->uiMsgType)
		{
		case STP_GET:
			switch (uiOID)
			{
			//MakeOID-generated:: STP GET (oid merge)
			case COMMON_OID_NULLOID:
				SmartToolMsgEmpty(p_STPSession, STP_GET_RESP, uiOID);
				break;
			case COMMON_OID_DEBUG:
				#ifdef USE_OUTPUT
				//logf("STP::GET::OID_DEBUG\r\n");
				#endif
				break;
			case COMMON_OID_SMARTTOOL_TYPE:
				SmartToolMsgStr(p_STPSession, STP_GET_RESP, uiOID, SMARTTOOL_TYPE_STRING);
				break;
			case COMMON_OID_SMARTTOOL_SUBTYPE:
				SmartToolMsgStr(p_STPSession, STP_GET_RESP, uiOID, SMARTTOOL_SUBTYPE_STRING);
				break;
			case COMMON_OID_SYSTEM_VERSION:
				//Let DynamicC append these.
				SmartToolMsgStr(p_STPSession, STP_GET_RESP, uiOID, SMARTTOOL_SUBTYPE_STRING " " SYSTEM_VERSION_STRING);
				break;
			case COMMON_OID_SERIAL_NUMBER:
				SmartToolMsgStr(p_STPSession, STP_GET_RESP, uiOID, g_szSerialNumber);
				break;
			case COMMON_OID_SCRIPT_TRANSFER:
				//FIXME minor  Implemented on Pendant, but not on Rabbit
				break;
			case COMMON_OID_GENERICMESSAGE:
				//Fall Through
			case COMMON_OID_DEBUGMESSAGE:
				SmartToolMsg(p_STPSession, STP_GET_RESP, uiOID, 10, "-");
				break;
			case COMMON_OID_STPSTATUS:
				p_StpStatus = (td_StpStatus *) p_cSTPobjValBuff;
				p_StpStatus->uiOID = htons(g_StpStatus.uiOID);
				p_StpStatus->uiStatus = htons(g_StpStatus.uiStatus);
				SmartToolMsg(p_STPSession, STP_GET_RESP, uiOID, sizeof(td_StpStatus), p_cSTPobjValBuff);
				break;
			case COMMON_OID_DIRECT_DOUT:
				p_oid_direct_dout = (td_oid_direct_dout *) p_cSTPobjValBuff;
				//FIXME00000000000000 FillStructWithData(p_oid_direct_dout);
				p_oid_direct_dout->cchannel = 0;
				p_oid_direct_dout->cvalue = 0;
				SmartToolMsg(p_STPSession, STP_GET_RESP, uiOID, sizeof(td_oid_direct_dout), p_cSTPobjValBuff);
				break;
			case COMMON_OID_DIRECT_DIN:
				p_oid_direct_din = (td_oid_direct_din *) p_cSTPobjValBuff;
				//FIXME FillStructWithData(p_oid_direct_din);
				p_oid_direct_din->cchannel = 1;
				p_oid_direct_din->cvalue = digIn(p_oid_direct_din->cchannel);
				SmartToolMsg(p_STPSession, STP_GET_RESP, uiOID, sizeof(td_oid_direct_din), p_cSTPobjValBuff);
				break;
			case COMMON_OID_COM_VERSION:
				SmartToolMsgUInt16(p_STPSession, STP_GET_RESP, uiOID, COM_VERSION);
				break;
				//case COMMON_OID_STREAM_SYNC: NO GET, but can alert
				//case COMMON_OID_STP_CLOSE: NO GET, but can alert
			case COMMON_OID_DATE_TIME:
				ServiceDateTime(); //update g_DateTime based on MS_TIMER
				p_DateTime = (td_DateTime *) p_cSTPobjValBuff;
				p_DateTime->ulticksMSW = htonul(g_DateTime.ulticksMSW);
				p_DateTime->ulticksLSW = htonul(g_DateTime.ulticksLSW);
				p_DateTime->uiyear = htons(g_DateTime.uiyear);
				p_DateTime->cmonth = g_DateTime.cmonth;
				p_DateTime->cdayOfMonth = g_DateTime.cdayOfMonth;
				p_DateTime->chour = g_DateTime.chour;
				p_DateTime->cminute = g_DateTime.cminute;
				p_DateTime->csecond = g_DateTime.csecond;
				p_DateTime->uimillisecond = htons(g_DateTime.uimillisecond);
				SmartToolMsg(p_STPSession, STP_GET_RESP, uiOID, sizeof(td_DateTime), p_cSTPobjValBuff);
				break;
				//end common OIDs
				//MiniFT OIDs
			case MINIFT_OID_MODE:
				SmartToolMsgChar(p_STPSession, STP_GET_RESP, uiOID, g_cModeState);
				break;
			case MINIFT_OID_ACTION:
				SmartToolMsgChar(p_STPSession, STP_GET_RESP, uiOID, g_cAction);
				break;
			case MINIFT_OID_ENCODER_RATIO:
				p_EncoderRatio = (td_EncoderRatio *) p_cSTPobjValBuff;
				p_EncoderRatio->fX = g_ConfigData.EncoderRatio.fX;
				p_EncoderRatio->fY = g_ConfigData.EncoderRatio.fY;
				SmartToolMsg(p_STPSession, STP_GET_RESP, uiOID, sizeof(td_EncoderRatio), p_cSTPobjValBuff);
				break;
			case MINIFT_OID_MC_CURRENT_LIMITS:
				p_MCCurrentLimit = (td_MCCurrentLimit *) p_cSTPobjValBuff;
				p_MCCurrentLimit->fX = g_ConfigData.MCCurrentLimit.fX;
				p_MCCurrentLimit->fY = g_ConfigData.MCCurrentLimit.fY;
				SmartToolMsg(p_STPSession, STP_GET_RESP, uiOID, sizeof(td_MCCurrentLimit), p_cSTPobjValBuff);
				break;
			case MINIFT_OID_MC_PEAK_CURRENT_LIMITS:
				p_MCPeakCurrentLimit = (td_MCPeakCurrentLimit *) p_cSTPobjValBuff;
				p_MCPeakCurrentLimit->fX = g_ConfigData.MCPeakCurrentLimit.fX;
				p_MCPeakCurrentLimit->fY = g_ConfigData.MCPeakCurrentLimit.fY;
				SmartToolMsg(p_STPSession, STP_GET_RESP, uiOID, sizeof(td_MCPeakCurrentLimit), p_cSTPobjValBuff);
				break;
			case MINIFT_OID_BRAKE_ON_TIMEOUT:
				SmartToolMsgUInt16(p_STPSession, STP_GET_RESP, uiOID, g_ConfigData.uiBrakeOnTimeout);
				break;
			case MINIFT_OID_MCERR:
				// MINIFT_OID_MCERR is used as an alert only, so the motion controller err msg is sent
				// when it occurs.  If a client explicitly tries to GET the mc error, just send "No Error"
				SmartToolMsg(p_STPSession, STP_GET_RESP, uiOID, 8, "NoErr");
				break;
			case MINIFT_OID_EEOPTION_DEFAULT:
				SmartToolMsgChar(p_STPSession, STP_GET_RESP, uiOID, g_ConfigData.cEEOptionDefault);
				break;
			case MINIFT_OID_EEOPTION:
				SmartToolMsgChar(p_STPSession, STP_GET_RESP, uiOID, g_cEEOption);
				break;
			case MINIFT_OID_BEEPER:
				SmartToolMsgChar(p_STPSession, STP_GET_RESP, uiOID, g_ConfigData.cbeeper);
				break;
			case MINIFT_OID_SYSTEM_MONITOR:
				p_oid_system_monitor = (td_oid_system_monitor *) p_cSTPobjValBuff;
				p_oid_system_monitor->fxtemp = 0;
				p_oid_system_monitor->fytemp = 0;
				p_oid_system_monitor->uiloop_avg_ms = 0;
				SmartToolMsg(p_STPSession, STP_GET_RESP, uiOID, sizeof(td_oid_system_monitor), p_cSTPobjValBuff);
				break;
			case MINIFT_OID_AIR_PRESSURE:
				#ifdef PRESSURESENSOR
				SmartToolMsgChar(p_STPSession, STP_GET_RESP, uiOID, g_cDigInAirPressure );
				#endif
				break;
			case MINIFT_OID_LIMITS_AND_OBSTRUCTIONS:
				lTemp = g_cObstructionCode;
				SmartToolMsgInt32(p_STPSession, STP_GET_RESP, uiOID, lTemp);
				break;
			case MINIFT_OID_TOOL_VERIFYENABLE:
				SmartToolMsgChar(p_STPSession, STP_GET_RESP, uiOID, g_ConfigData.cToolVerifyEnable);
				break;
			case MINIFT_OID_HOLE_PARAMETERS:
				SendHoleParameters(p_STPSession, STP_GET_RESP);
				break;
			case MINIFT_OID_TOOL_RESERVED:
				//Deprecated see MINIFT_OID_TOOL_REC
				break;
			case MINIFT_OID_PROCESS:
				SmartToolMsgChar(p_STPSession, STP_GET_RESP, uiOID, g_cLoadedProcess);
				break;
			case MINIFT_OID_RETURN_HEIGHT:
				SmartToolMsgFloat(p_STPSession, STP_GET_RESP, uiOID, g_ConfigData.fReturnHeight);
				break;
			case MINIFT_OID_GAUGE_LENGTH:
				SmartToolMsgFloat(p_STPSession, STP_GET_RESP, uiOID, g_fGaugeLength);
				logf("gl %f\r\n", g_fGaugeLength);
				break;
			case MINIFT_OID_SCALE_MODE:
				SmartToolMsgChar(p_STPSession, STP_GET_RESP, uiOID, g_ConfigData.cScaleMode);
				break;
			case MINIFT_OID_RESERVED_2:
				break;
			case MINIFT_OID_PARTPGM_REQUEST_FILE:
				SmartToolMsgStr(p_STPSession, STP_GET_RESP, uiOID, "");
				break;
			case MINIFT_OID_PARTPGM_DIR:
				//BSTRARRAY FORMAT
				//VDIR function is gone...
				break;
			case MINIFT_OID_PARTPGM_NAME:
				#ifdef OUTPUT_PP_SYS
				logf("tx PPnm %s\r\n",g_szPartPgmFilename); //debug
#endif
				SmartToolMsgStr(p_STPSession, STP_GET_RESP, uiOID, g_szPartPgmFilename);
				break;
			case MINIFT_OID_PARTPGM_DATA:
				//Sending only part of the data for the purpose pendant usage
				//The Data can be started at an index to avoid retransmit
				ui = p_STPrxMsg->uiValueLength;
				if (ui == 0)
				{
					//start at the front
					ul = 0;
				}
				else if (ui == 4)
				{
					ul = (unsigned long) ntohl(
							*((long*) (p_STPrxMsg->p_cObjectValue)));
				}
				StartSendPartProgramData(p_STPSession->iSessionNum, ul); //Session and start
				break;
			case MINIFT_OID_PARTPGM_CHECKSUM:
				//send all 16 bytes... no null
				SmartToolMsg(p_STPSession, STP_GET_RESP, uiOID, 16, (char *) g_PartPgmInfo.p_cChecksum16);
				break;
			case MINIFT_OID_PARTPGM_LINEARJOB:
				SmartToolMsgInt16(p_STPSession, STP_GET_RESP, uiOID, g_iPartPgmLinearHoles);
				break;
			case MINIFT_OID_PARTPGM_STATUS:
				SmartToolMsgChar(p_STPSession, STP_GET_RESP, uiOID, g_cPartPgmStatus);
				break;
			case MINIFT_OID_PARTPGM_LOCKED:
				SmartToolMsgChar(p_STPSession, STP_GET_RESP, uiOID, g_PartPgmInfo.cLocked);
				break;
			case MINIFT_OID_GRAVCOMP_STATUS:
				SmartToolMsgChar(p_STPSession, STP_GET_RESP, uiOID, g_cGravCompStatus);
				break;
			case MINIFT_OID_GRAVCOMP_AXES:
				SmartToolMsgChar(p_STPSession, STP_GET_RESP, uiOID, g_ConfigData.cGravCompAxes);
				break;
			case MINIFT_OID_GRAVCOMP_SPEED:
				p_GravCompSpeed = (td_GravCompSpeed *) p_cSTPobjValBuff;
				p_GravCompSpeed->fX = g_ConfigData.GravCompSpeed.fX;
				p_GravCompSpeed->fY = g_ConfigData.GravCompSpeed.fY;
				SmartToolMsg(p_STPSession, STP_GET_RESP, uiOID, sizeof(td_GravCompSpeed), p_cSTPobjValBuff);
				break;
			case MINIFT_OID_GRAVCOMP_ACCEL:
				p_GravCompAcc = (td_GravCompAcc *) p_cSTPobjValBuff;
				p_GravCompAcc->fX = g_ConfigData.GravCompAcc.fX;
				p_GravCompAcc->fY = g_ConfigData.GravCompAcc.fY;
				SmartToolMsg(p_STPSession, STP_GET_RESP, uiOID, sizeof(td_GravCompAcc), p_cSTPobjValBuff);
				break;
			case MINIFT_OID_GRAVCOMP_DECEL:
				p_GravCompDec = (td_GravCompDec *) p_cSTPobjValBuff;
				p_GravCompDec->fX = g_ConfigData.GravCompDec.fX;
				p_GravCompDec->fY = g_ConfigData.GravCompDec.fY;
				SmartToolMsg(p_STPSession, STP_GET_RESP, uiOID, sizeof(td_GravCompDec), p_cSTPobjValBuff);
				break;
			case MINIFT_OID_GRAVCOMP_MOVEDIST:
				p_GravCompMoveDist = (td_GravCompMoveDist *) p_cSTPobjValBuff;
				p_GravCompMoveDist->fX = g_ConfigData.GravCompMoveDist.fX;
				p_GravCompMoveDist->fY = g_ConfigData.GravCompMoveDist.fY;
				SmartToolMsg(p_STPSession, STP_GET_RESP, uiOID, sizeof(td_GravCompMoveDist), p_cSTPobjValBuff);
				break;
			case MINIFT_OID_GRAVCOMP_ALGORITHM:
				SmartToolMsgChar(p_STPSession, STP_GET_RESP, uiOID, g_ConfigData.cGravCompAlgorithm);
				break;
			case MINIFT_OID_GRAVCOMP_NOISE_LIMIT:
				p_GravCompNoiseLimit = (td_GravCompNoiseLimit *) p_cSTPobjValBuff;
				p_GravCompNoiseLimit->fX = g_ConfigData.GravCompNoiseLimit.fX;
				p_GravCompNoiseLimit->fY = g_ConfigData.GravCompNoiseLimit.fY;
				SmartToolMsg(p_STPSession, STP_GET_RESP, uiOID, sizeof(td_GravCompNoiseLimit), p_cSTPobjValBuff);
				break;
			case MINIFT_OID_GRAVCOMP_TRIGGERFACTOR:
				p_GravCompTriggerFactor = (td_GravCompTriggerFactor *) p_cSTPobjValBuff;
				p_GravCompTriggerFactor->fX = g_ConfigData.GravCompTriggerFactor.fX;
				p_GravCompTriggerFactor->fY = g_ConfigData.GravCompTriggerFactor.fY;
				SmartToolMsg(p_STPSession, STP_GET_RESP, uiOID, sizeof(td_GravCompTriggerFactor), p_cSTPobjValBuff);
				break;
			case MINIFT_OID_FLOAT_STATUS:
				SmartToolMsgChar(p_STPSession, STP_GET_RESP, uiOID, g_cFloatStatus);
				break;
			case MINIFT_OID_GRAVCOMP_RESULTS:
				p_GravCompResults = (td_GravCompResults *) p_cSTPobjValBuff;
				p_GravCompResults->fxp = g_GravCompResults.fxp;
				p_GravCompResults->fxn = g_GravCompResults.fxn;
				p_GravCompResults->fyp = g_GravCompResults.fyp;
				p_GravCompResults->fyn = g_GravCompResults.fyn;
				SmartToolMsg(p_STPSession, STP_GET_RESP, uiOID, sizeof(td_GravCompResults), p_cSTPobjValBuff);
				break;
			case MINIFT_OID_FLOAT_SPEEDLIMIT:
				SmartToolMsgFloat(p_STPSession, STP_GET_RESP, uiOID, g_ConfigData.fFloatSpeedLimit);
				break;
			case MINIFT_OID_JOG_SPEEDLIMIT:
				SmartToolMsgFloat(p_STPSession, STP_GET_RESP, uiOID, g_ConfigData.fJogSpeedLimit);
				break;
			case MINIFT_OID_MAX_SPEED_X:
				SmartToolMsgFloat(p_STPSession, STP_GET_RESP, uiOID, g_ConfigData.fMaxSpeedX);
				break;
			case MINIFT_OID_MAX_SPEED_Y:
				SmartToolMsgFloat(p_STPSession, STP_GET_RESP, uiOID, g_ConfigData.fMaxSpeedY);
				break;
			case MINIFT_OID_X_RAIL_SURFACE_OFFSET:
				SmartToolMsgFloat(p_STPSession, STP_GET_RESP, uiOID, g_fXRailSurfaceOffset);
				break;
			case MINIFT_OID_PROBE_METHOD:
				SmartToolMsgChar(p_STPSession, STP_GET_RESP, uiOID, g_cProbeMethod);
				break;
			case MINIFT_OID_PROBE_METHOD_DEFAULT:
				SmartToolMsgChar(p_STPSession, STP_GET_RESP, uiOID, g_ConfigData.cProbeMethodDefault);
				break;
			case MINIFT_OID_PROBE_OFFSET:
				p_ProbeOffset = (td_ProbeOffset *) p_cSTPobjValBuff;
				p_ProbeOffset->fX = g_ConfigData.ProbeOffset.fX;
				p_ProbeOffset->fY = g_ConfigData.ProbeOffset.fY;
				SmartToolMsg(p_STPSession, STP_GET_RESP, uiOID, sizeof(td_ProbeOffset), p_cSTPobjValBuff);
				break;
			case MINIFT_OID_PROBE_DIR:
				SmartToolMsgInt16(p_STPSession, STP_GET_RESP, uiOID, g_ConfigData.iProbeDir);
				break;
			case MINIFT_OID_DRILL_DIR:
				SmartToolMsgInt16(p_STPSession, STP_GET_RESP, uiOID, g_ConfigData.iDrillDir);
				break;
			case MINIFT_OID_KHOLE_MAX_DISTANCE_ERROR:
				SmartToolMsgFloat(p_STPSession, STP_GET_RESP, uiOID, g_ConfigData.fMaxKholeDistanceError);
				break;
			case MINIFT_OID_APPROX_LOCATION_ERROR:
				SmartToolMsgFloat(p_STPSession, STP_GET_RESP, uiOID, g_ConfigData.fApproxLocationError);
				break;
				//case MINIFT_OID_KHOLE_DISTANCE:
				//	Only Used for Alert
				//	break;
			case MINIFT_OID_PROBE_POSITION:
				p_oid_probe_position = (td_oid_probe_position *) p_cSTPobjValBuff;
				p_oid_probe_position->fX = g_PosnMode.fLastKnownPosnX;
				p_oid_probe_position->fY = g_PosnMode.fLastKnownPosnY;
				SmartToolMsg(p_STPSession, STP_GET_RESP, uiOID, sizeof(td_oid_probe_position), p_cSTPobjValBuff);
				break;
			case MINIFT_OID_PROBE_STATUS:
				//For now, this returns all the probe status in multiple responses
				SendAllProbeStatus(STP_GET_RESP);         //SPSALL
				AlertProbeStatusUpdate();
				break;
			case MINIFT_OID_PROBE_START:
				//For now, this returns all the probe start in multiple responses
				SendAllProbeStart(STP_GET_RESP);          //SPSALL
				AlertProbeStatusUpdate();
				break;
			case MINIFT_OID_PROBE_ACCEPT_REQUIRED:
				SmartToolMsgChar(p_STPSession, STP_GET_RESP, uiOID, g_ConfigData.cProbeAcceptRequired);
				break;
			case MINIFT_OID_HOME:
				#ifdef HOMESYSTEM
				SmartToolMsgChar(p_STPSession, STP_GET_RESP, uiOID, g_cHomed);
				ClearSentHomeStatus(); // Force Send of all the other Home Status...
									   //FIXME Not sure I like this pattern, but it's what I have now.
#else
				c=HOME_DONE;
				SmartToolMsgChar(p_STPSession, STP_GET_RESP, uiOID, c);
#endif
				break;
			case MINIFT_OID_HOME_SPEED:
				p_HomeSpeed = (td_HomeSpeed *) p_cSTPobjValBuff;
				p_HomeSpeed->fX = g_ConfigData.HomeSpeed.fX;
				p_HomeSpeed->fY = g_ConfigData.HomeSpeed.fY;
				SmartToolMsg(p_STPSession, STP_GET_RESP, uiOID, sizeof(td_HomeSpeed), p_cSTPobjValBuff);
				break;
			case MINIFT_OID_HOME_ACCEL:
				p_HomeAcc = (td_HomeAcc *) p_cSTPobjValBuff;
				p_HomeAcc->fX = g_ConfigData.HomeAcc.fX;
				p_HomeAcc->fY = g_ConfigData.HomeAcc.fY;
				SmartToolMsg(p_STPSession, STP_GET_RESP, uiOID, sizeof(td_HomeAcc), p_cSTPobjValBuff);
				break;
			case MINIFT_OID_HOME_DECEL:
				p_HomeDec = (td_HomeDec *) p_cSTPobjValBuff;
				p_HomeDec->fX = g_ConfigData.HomeDec.fX;
				p_HomeDec->fY = g_ConfigData.HomeDec.fY;
				SmartToolMsg(p_STPSession, STP_GET_RESP, uiOID, sizeof(td_HomeDec), p_cSTPobjValBuff);
				break;
			case MINIFT_OID_HOME_MOVEDIST:
				p_HomeMoveDist = (td_HomeMoveDist *) p_cSTPobjValBuff;
				p_HomeMoveDist->fX = g_ConfigData.HomeMoveDist.fX;
				p_HomeMoveDist->fY = g_ConfigData.HomeMoveDist.fY;
				SmartToolMsg(p_STPSession, STP_GET_RESP, uiOID, sizeof(td_HomeMoveDist), p_cSTPobjValBuff);
				break;
			case MINIFT_OID_PROBE_ADJUST_LIMIT:
				SmartToolMsgFloat(p_STPSession, STP_GET_RESP, uiOID, g_ConfigData.fProbeAdjustLimit);
				break;
			case MINIFT_OID_HOME_FINE_SPEED:
				p_HomeFineSpeed = (td_HomeFineSpeed *) p_cSTPobjValBuff;
				p_HomeFineSpeed->fX = g_ConfigData.HomeFineSpeed.fX;
				p_HomeFineSpeed->fY = g_ConfigData.HomeFineSpeed.fY;
				SmartToolMsg(p_STPSession, STP_GET_RESP, uiOID, sizeof(td_HomeFineSpeed), p_cSTPobjValBuff);
				break;
			case MINIFT_OID_PATTERN:
				SmartToolMsgChar(p_STPSession, STP_GET_RESP, uiOID, g_cPattern);
				break;
			case MINIFT_OID_POSNMODE_CURPOSN:
				SendCurPosnInd(p_STPSession, STP_GET_RESP);
				break;
			case MINIFT_OID_POSNMODE_NEARPOSN:
				//marker
				SendNearPosnInd(p_STPSession, STP_GET_RESP);
				break;
			case MINIFT_OID_POSNMODE_GOALPOSN:
				//marker
				SendGoalPosnInd(p_STPSession, STP_GET_RESP);
				break;
			case MINIFT_OID_POSNMODE_CURXY:
				MCGetPosition();
				g_PosnMode.fLastSentPosnX += 1; //force a send.
				break;
			case MINIFT_OID_POSNMODE_NEARXY:
				SendNearXY(p_STPSession, STP_GET_RESP);
				break;
			case MINIFT_OID_POSNMODE_POSNSUMMARY:
				//FIXME0000 not used yet
				break;
			case MINIFT_OID_POSNMODE_MOVETYPE:
				SmartToolMsgChar(p_STPSession, STP_GET_RESP, uiOID, g_ConfigData.cMoveType);
				break;
			case MINIFT_OID_POSNMODE_PREMOVEXY:
				p_PreMove = (td_PreMove *) p_cSTPobjValBuff;
				p_PreMove->fX = g_ConfigData.PreMove.fX;
				p_PreMove->fY = g_ConfigData.PreMove.fY;
				SmartToolMsg(p_STPSession, STP_GET_RESP, uiOID, sizeof(td_PreMove), p_cSTPobjValBuff);
				break;
			case MINIFT_OID_POSNMODE_SPEED:
				p_PosnSpeed = (td_PosnSpeed *) p_cSTPobjValBuff;
				p_PosnSpeed->fX = g_ConfigData.PosnSpeed.fX;
				p_PosnSpeed->fY = g_ConfigData.PosnSpeed.fY;
				SmartToolMsg(p_STPSession, STP_GET_RESP, uiOID, sizeof(td_PosnSpeed), p_cSTPobjValBuff);
				break;
			case MINIFT_OID_POSNMODE_ACCEL:
				p_PosnAcc = (td_PosnAcc *) p_cSTPobjValBuff;
				p_PosnAcc->fX = g_ConfigData.PosnAcc.fX;
				p_PosnAcc->fY = g_ConfigData.PosnAcc.fY;
				SmartToolMsg(p_STPSession, STP_GET_RESP, uiOID, sizeof(td_PosnAcc), p_cSTPobjValBuff);
				break;
			case MINIFT_OID_POSNMODE_DECEL:
				p_PosnDec = (td_PosnDec *) p_cSTPobjValBuff;
				p_PosnDec->fX = g_ConfigData.PosnDec.fX;
				p_PosnDec->fY = g_ConfigData.PosnDec.fY;
				SmartToolMsg(p_STPSession, STP_GET_RESP, uiOID, sizeof(td_PosnDec), p_cSTPobjValBuff);
				break;
			case MINIFT_OID_POSNMODE_FINALSPEED:
				p_PosnFinalSpeed = (td_PosnFinalSpeed *) p_cSTPobjValBuff;
				p_PosnFinalSpeed->fX = g_ConfigData.PosnFinalSpeed.fX;
				p_PosnFinalSpeed->fY = g_ConfigData.PosnFinalSpeed.fY;
				SmartToolMsg(p_STPSession, STP_GET_RESP, uiOID, sizeof(td_PosnFinalSpeed), p_cSTPobjValBuff);
				break;
			case MINIFT_OID_ORTHO_SLOPE:
				SmartToolMsgFloat(p_STPSession, STP_GET_RESP, uiOID, g_ConfigData.fOrthoSlope);
				break;
			case MINIFT_OID_POSNERR_LIMIT:
				p_PosnErrLimit = (td_PosnErrLimit *) p_cSTPobjValBuff;
				p_PosnErrLimit->fX = g_ConfigData.PosnErrLimit.fX;
				p_PosnErrLimit->fY = g_ConfigData.PosnErrLimit.fY;
				SmartToolMsg(p_STPSession, STP_GET_RESP, uiOID, sizeof(td_PosnErrLimit), p_cSTPobjValBuff);
				break;
			case MINIFT_OID_POSNMODE_TOLERANCE:
				SmartToolMsgFloat(p_STPSession, STP_GET_RESP, uiOID, g_ConfigData.fPosnTolerance);
				break;
			case MINIFT_OID_VELERR_LIMIT:
				p_VelErrLimit = (td_VelErrLimit *) p_cSTPobjValBuff;
				p_VelErrLimit->fVLimitMarginX = g_ConfigData.VelErrLimit.fVLimitMarginX;
				p_VelErrLimit->fVErrorX = g_ConfigData.VelErrLimit.fVErrorX;
				p_VelErrLimit->fVLimitMarginY = g_ConfigData.VelErrLimit.fVLimitMarginY;
				p_VelErrLimit->fVErrorY = g_ConfigData.VelErrLimit.fVErrorY;
				SmartToolMsg(p_STPSession, STP_GET_RESP, uiOID, sizeof(td_VelErrLimit), p_cSTPobjValBuff);
				break;
			case MINIFT_OID_LONG_DISTANCE:
				p_LongDistance = (td_LongDistance *) p_cSTPobjValBuff;
				p_LongDistance->fX = g_ConfigData.LongDistance.fX;
				p_LongDistance->fY = g_ConfigData.LongDistance.fY;
				SmartToolMsg(p_STPSession, STP_GET_RESP, uiOID, sizeof(td_LongDistance), p_cSTPobjValBuff);
				break;
			case MINIFT_OID_LONG_SPEED:
				p_LongSpeed = (td_LongSpeed *) p_cSTPobjValBuff;
				p_LongSpeed->fX = g_ConfigData.LongSpeed.fX;
				p_LongSpeed->fY = g_ConfigData.LongSpeed.fY;
				SmartToolMsg(p_STPSession, STP_GET_RESP, uiOID, sizeof(td_LongSpeed), p_cSTPobjValBuff);
				break;
			case MINIFT_OID_POSNMODE_MOVEDONE:
				SmartToolMsgChar(p_STPSession, STP_GET_RESP, uiOID, g_cMoveDone);
				break;
				//case MINIFT_OID_OP_STARTED:
				//	//Used For ALERT only
				//	break;
			case MINIFT_OID_OP_HISTORY:
				ui = p_STPrxMsg->uiValueLength;
				if (ui == 0)
				{
					//start as early as possible
					ul = 0;
				}
				else if (ui == 4)
				{
					//use this timestamp
					ul = (uint32) ntohl(*((uint32*) (p_STPrxMsg->p_cObjectValue)));
				}
				StartSendOpHistory(p_STPSession->iSessionNum, ul);
				break;
			case MINIFT_OID_DRILL_HOLE_ONE_TIME:
				SmartToolMsgChar(p_STPSession, STP_GET_RESP, uiOID, g_ConfigData.cDrillHoleOneTime);
				break;
			case MINIFT_OID_AUTOMOVE:
				SmartToolMsgChar(p_STPSession, STP_GET_RESP, uiOID, g_cAutoMove);
				break;
			case MINIFT_OID_AUTOMOVE_DELAY:
				SmartToolMsgUInt32(p_STPSession, STP_GET_RESP, uiOID, g_uiAutoTime);
				break;
			case MINIFT_OID_AUTOREPEAT:
				//logf("g_cAR=%d\r\n",g_cAutoRepeat);
				SmartToolMsgChar(p_STPSession, STP_GET_RESP, uiOID, g_cAutoRepeat);
				break;
			case MINIFT_OID_MACHINE_OFFSET:
				p_MachineOffset = (td_MachineOffset *) p_cSTPobjValBuff;
				p_MachineOffset->fX = g_MachineOffset.fX;
				p_MachineOffset->fY = g_MachineOffset.fY;
				SmartToolMsg(p_STPSession, STP_GET_RESP, uiOID, sizeof(td_MachineOffset), p_cSTPobjValBuff);
				break;
			case MINIFT_OID_MACHINE_OFFSET_CADJ:
				SmartToolMsgChar(p_STPSession, STP_GET_RESP, uiOID, g_cMachineOffsetCompensationAdjustment);
				break;
			case MINIFT_OID_MACHINE_OFFSET1:
				p_MachineOffset1 = (td_MachineOffset1 *) p_cSTPobjValBuff;
				p_MachineOffset1->fX = g_MachineOffset1.fX;
				p_MachineOffset1->fY = g_MachineOffset1.fY;
				p_MachineOffset1->fYExtension = g_MachineOffset1.fYExtension;
				SmartToolMsg(p_STPSession, STP_GET_RESP, uiOID, sizeof(td_MachineOffset1), p_cSTPobjValBuff);
				break;
			case MINIFT_OID_MACHINE_OFFSET2:
				p_MachineOffset2 = (td_MachineOffset2 *) p_cSTPobjValBuff;
				p_MachineOffset2->fX = g_MachineOffset2.fX;
				p_MachineOffset2->fY = g_MachineOffset2.fY;
				p_MachineOffset2->fYExtension = g_MachineOffset2.fYExtension;
				SmartToolMsg(p_STPSession, STP_GET_RESP, uiOID,
						sizeof(td_MachineOffset2), p_cSTPobjValBuff);
				break;
			case MINIFT_OID_STATION:
				SmartToolMsgChar(p_STPSession, STP_GET_RESP, uiOID, g_cStation);
				break;
			case MINIFT_OID_TOOL_OFFSET:
				p_oid_tool_offset = (td_oid_tool_offset *) p_cSTPobjValBuff;
				//flipping controls an extra 180 degree rotation.
				//if tool is in pos direction, flip only for Tool to Movement conversion (machine offset if oposite tool offset.)
				//if tool is in neg direction, flip for Tool to Movement conversion, and flip again since tool is oriented on oposite side 180.
				//	two flips cancel out (-1 * -1 = 1) so do nothing.
				if (g_ConfigData.cToolFlip != Y_NEG)
				{
					p_oid_tool_offset->fX = -g_MachineOffset.fX;
					p_oid_tool_offset->fY = -g_MachineOffset.fY;
				}
				else
				{
					p_oid_tool_offset->fX = g_MachineOffset.fX;
					p_oid_tool_offset->fY = g_MachineOffset.fY;
				}
				SmartToolMsg(p_STPSession, STP_GET_RESP, uiOID,
						sizeof(td_oid_tool_offset), p_cSTPobjValBuff);
				break;
			case MINIFT_OID_TOOL_FLIP:
				SmartToolMsgChar(p_STPSession, STP_GET_RESP, uiOID,
						g_ConfigData.cToolFlip);
#ifdef OUTPUT_TOOL_FLIP
				logf("Flp=%d\r\n",g_ConfigData.cToolFlip);
#endif
				break;
			case MINIFT_OID_DRIVE_THROUGH_BACKLASH:
				SmartToolMsgChar(p_STPSession, STP_GET_RESP, uiOID,
						g_ConfigData.cDriveThroughBacklash);
				break;
			case MINIFT_OID_DRILL_OFFSET1:
				p_DrillOffset1 = (td_DrillOffset1 *) p_cSTPobjValBuff;
				p_DrillOffset1->fX = g_ConfigData.DrillOffset1.fX;
				p_DrillOffset1->fY = g_ConfigData.DrillOffset1.fY;
				p_DrillOffset1->fYExtension = g_ConfigData.DrillOffset1.fYExtension;
				SmartToolMsg(p_STPSession, STP_GET_RESP, uiOID, sizeof(td_DrillOffset1), p_cSTPobjValBuff);
				break;
			case MINIFT_OID_DRILL_OFFSET2:
				p_DrillOffset2 = (td_DrillOffset2 *) p_cSTPobjValBuff;
				p_DrillOffset2->fX = g_ConfigData.DrillOffset2.fX;
				p_DrillOffset2->fY = g_ConfigData.DrillOffset2.fY;
				p_DrillOffset2->fYExtension = g_ConfigData.DrillOffset2.fYExtension;
				SmartToolMsg(p_STPSession, STP_GET_RESP, uiOID, sizeof(td_DrillOffset2), p_cSTPobjValBuff);
				break;
			case MINIFT_OID_OFFSET_SEAL:
				p_OffsetSeal = (td_OffsetSeal *) p_cSTPobjValBuff;
				p_OffsetSeal->fx = g_ConfigData.OffsetSeal.fx;
				p_OffsetSeal->fy = g_ConfigData.OffsetSeal.fy;
				SmartToolMsg(p_STPSession, STP_GET_RESP, uiOID, sizeof(td_OffsetSeal), p_cSTPobjValBuff);
				break;
			case MINIFT_OID_OFFSET_FILL:
				p_OffsetFill = (td_OffsetFill *) p_cSTPobjValBuff;
				p_OffsetFill->fx = g_ConfigData.OffsetFill.fx;
				p_OffsetFill->fy = g_ConfigData.OffsetFill.fy;
				SmartToolMsg(p_STPSession, STP_GET_RESP, uiOID,
						sizeof(td_OffsetFill), p_cSTPobjValBuff);
				break;
			case MINIFT_OID_JOG_SPEED:
				p_JogSpeed = (td_JogSpeed *) p_cSTPobjValBuff;
				p_JogSpeed->fX = g_ConfigData.JogSpeed.fX;
				p_JogSpeed->fY = g_ConfigData.JogSpeed.fY;
				SmartToolMsg(p_STPSession, STP_GET_RESP, uiOID,
						sizeof(td_JogSpeed), p_cSTPobjValBuff);
				break;
			case MINIFT_OID_JOG_ACCEL:
				p_JogAcc = (td_JogAcc *) p_cSTPobjValBuff;
				p_JogAcc->fX = g_ConfigData.JogAcc.fX;
				p_JogAcc->fY = g_ConfigData.JogAcc.fY;
				SmartToolMsg(p_STPSession, STP_GET_RESP, uiOID, sizeof(td_JogAcc), p_cSTPobjValBuff);
				break;
			case MINIFT_OID_JOG_DECEL:
				p_JogDec = (td_JogDec *) p_cSTPobjValBuff;
				p_JogDec->fX = g_ConfigData.JogDec.fX;
				p_JogDec->fY = g_ConfigData.JogDec.fY;
				SmartToolMsg(p_STPSession, STP_GET_RESP, uiOID, sizeof(td_JogDec), p_cSTPobjValBuff);
				break;
			case MINIFT_OID_JOG_FACTOR:
				p_JogFactor = (td_JogFactor *) p_cSTPobjValBuff;
				p_JogFactor->fX = g_JogFactor.fX;
				p_JogFactor->fY = g_JogFactor.fY;
				SmartToolMsg(p_STPSession, STP_GET_RESP, uiOID, sizeof(td_JogFactor), p_cSTPobjValBuff);
				break;
			case MINIFT_OID_HOME_POSITION_Y_POS:
				p_HomePosnYPos = (td_HomePosnYPos *) p_cSTPobjValBuff;
				p_HomePosnYPos->fX = g_ConfigData.HomePosnYPos.fX;
				p_HomePosnYPos->fY = g_ConfigData.HomePosnYPos.fY;
				SmartToolMsg(p_STPSession, STP_GET_RESP, uiOID, sizeof(td_HomePosnYPos), p_cSTPobjValBuff);
				break;
			case MINIFT_OID_POSITION_LIMIT_Y_POS:
				p_PosnLimitYPos = (td_PosnLimitYPos *) p_cSTPobjValBuff;
				p_PosnLimitYPos->fMinX = g_ConfigData.PosnLimitYPos.fMinX;
				p_PosnLimitYPos->fMaxX = g_ConfigData.PosnLimitYPos.fMaxX;
				p_PosnLimitYPos->fMinY = g_ConfigData.PosnLimitYPos.fMinY;
				p_PosnLimitYPos->fMaxY = g_ConfigData.PosnLimitYPos.fMaxY;
				SmartToolMsg(p_STPSession, STP_GET_RESP, uiOID,
						sizeof(td_PosnLimitYPos), p_cSTPobjValBuff);
				break;
			case MINIFT_OID_HOME_POSITION_Y_NEG:
				p_HomePosnYNeg = (td_HomePosnYNeg *) p_cSTPobjValBuff;
				p_HomePosnYNeg->fX = g_ConfigData.HomePosnYNeg.fX;
				p_HomePosnYNeg->fY = g_ConfigData.HomePosnYNeg.fY;
				SmartToolMsg(p_STPSession, STP_GET_RESP, uiOID, sizeof(td_HomePosnYNeg), p_cSTPobjValBuff);
				break;
			case MINIFT_OID_POSITION_LIMIT_Y_NEG:
				p_PosnLimitYNeg = (td_PosnLimitYNeg *) p_cSTPobjValBuff;
				p_PosnLimitYNeg->fMinX = g_ConfigData.PosnLimitYNeg.fMinX;
				p_PosnLimitYNeg->fMaxX = g_ConfigData.PosnLimitYNeg.fMaxX;
				p_PosnLimitYNeg->fMinY = g_ConfigData.PosnLimitYNeg.fMinY;
				p_PosnLimitYNeg->fMaxY = g_ConfigData.PosnLimitYNeg.fMaxY;
				SmartToolMsg(p_STPSession, STP_GET_RESP, uiOID, sizeof(td_PosnLimitYNeg), p_cSTPobjValBuff);
				break;
			case MINIFT_OID_PROBE_REGISTRATION:
				SmartToolMsgChar(p_STPSession, STP_GET_RESP, uiOID, g_Probe.cRegistration);
				break;
			case MINIFT_OID_OBSTRUCTION_CODE_MASK:
				SmartToolMsgChar(p_STPSession, STP_GET_RESP, uiOID, g_ConfigData.cObstructionCodeMask);
				break;
			case MINIFT_OID_MACHINE_LOCK_REQUIRED:
				SmartToolMsgChar(p_STPSession, STP_GET_RESP, uiOID, g_ConfigData.cMachineLockRequired);
				break;
			case MINIFT_OID_MACHINE_LOCK:
				//Use Summed up lock summary
				SmartToolMsgChar(p_STPSession, STP_GET_RESP, uiOID, g_cMachineLock);
				break;
			case MINIFT_OID_CLAMP:
				#ifdef CLAMP_SYSTEM
				SmartToolMsgChar(p_STPSession, STP_GET_RESP, uiOID, g_cClampState); //Return the Clamp State
#endif
				break;
			case MINIFT_OID_ALOCK:
				SmartToolMsgChar(p_STPSession, STP_GET_RESP, uiOID, g_cALockMode);
				break;
			case MINIFT_OID_ALOCKDELAY:
				SmartToolMsgUInt16(p_STPSession, STP_GET_RESP, uiOID, g_ConfigData.uiALockDelay);
				break;
			case MINIFT_OID_AUNLOCKDELAY:
				SmartToolMsgUInt16(p_STPSession, STP_GET_RESP, uiOID, g_ConfigData.uiAUnlockDelay);
				break;
			case MINIFT_OID_LEGLOCKDELAY:
				SmartToolMsgUInt16(p_STPSession, STP_GET_RESP, uiOID, g_ConfigData.uiLegsLockDelay);
				break;
			case MINIFT_OID_LEGUNLOCKDELAY:
				SmartToolMsgUInt16(p_STPSession, STP_GET_RESP, uiOID, g_ConfigData.uiLegsUnlockDelay);
				break;
			case MINIFT_OID_LEGSUPDELAY:
				SmartToolMsgUInt16(p_STPSession, STP_GET_RESP, uiOID, g_ConfigData.uiLegsUpDelay);
				break;
			case MINIFT_OID_LEGSDOWNDELAY:
				SmartToolMsgUInt16(p_STPSession, STP_GET_RESP, uiOID, g_ConfigData.uiLegsDownDelay);
				break;
			case MINIFT_OID_LOWPRESSUREDELAY:
				SmartToolMsgUInt16(p_STPSession, STP_GET_RESP, uiOID, g_ConfigData.uiLowPressureDelay);
				break;
			case MINIFT_OID_LOWPRESSUREDOWNDELAY:
				SmartToolMsgUInt16(p_STPSession, STP_GET_RESP, uiOID, g_ConfigData.uiLowPressureDownDelay);
				break;
			case MINIFT_OID_PRESSUREDELAY:
				SmartToolMsgUInt16(p_STPSession, STP_GET_RESP, uiOID, g_ConfigData.uiPressureDelay);
				break;
			case MINIFT_OID_PRESSUREDOWNDELAY:
				SmartToolMsgUInt16(p_STPSession, STP_GET_RESP, uiOID, g_ConfigData.uiPressureDownDelay);
				break;
			case MINIFT_OID_LOWPRESSURE:
				SmartToolMsgUInt16(p_STPSession, STP_GET_RESP, uiOID, g_ConfigData.uiLowPressure);
				break;
			case MINIFT_OID_PRESSURE:
				SmartToolMsgUInt16(p_STPSession, STP_GET_RESP, uiOID, g_ConfigData.uiPressure);
				break;
			case MINIFT_OID_AIR_CLEAR:
				SmartToolMsgUInt16(p_STPSession, STP_GET_RESP, uiOID, g_ConfigData.uiAirClear);
				break;
			case MINIFT_OID_LASER_SENSOR_OFFSET:
				p_LaserSensorOffset = (td_LaserSensorOffset *) p_cSTPobjValBuff;
				p_LaserSensorOffset->fX = g_ConfigData.LaserSensorOffset.fX;
				p_LaserSensorOffset->fY = g_ConfigData.LaserSensorOffset.fY;
				SmartToolMsg(p_STPSession, STP_GET_RESP, uiOID, sizeof(td_LaserSensorOffset), p_cSTPobjValBuff);
				break;
			case MINIFT_OID_CAM_OFFSET:
				p_CamOffset = (td_CamOffset *) p_cSTPobjValBuff;
				p_CamOffset->fX = g_ConfigData.CamOffset.fX;
				p_CamOffset->fY = g_ConfigData.CamOffset.fY;
				SmartToolMsg(p_STPSession, STP_GET_RESP, uiOID, sizeof(td_CamOffset), p_cSTPobjValBuff);
				break;
				//case MINIFT_OID_VISION_IMAGE:
				//For Alerting Images
				break;
				//Used For ALERTS
				//case MINIFT_OID_VISION_DATA:
				//case MINIFT_OID_VISION_INSPECT_RESULTS:

			case MINIFT_OID_LASER_SENSOR_ALG_PARAM:
				p_LaserSensorAlgParam = (td_LaserSensorAlgParam *) p_cSTPobjValBuff;
				p_LaserSensorAlgParam->fsearch_speed = g_ConfigData.LaserSensorAlgParam.fsearch_speed;
				p_LaserSensorAlgParam->fseek_speed = g_ConfigData.LaserSensorAlgParam.fseek_speed;
				p_LaserSensorAlgParam->frscan_speed = g_ConfigData.LaserSensorAlgParam.frscan_speed;
				p_LaserSensorAlgParam->frscan_speed_fast = g_ConfigData.LaserSensorAlgParam.frscan_speed_fast;
				p_LaserSensorAlgParam->fscan_speed = g_ConfigData.LaserSensorAlgParam.fscan_speed;
				p_LaserSensorAlgParam->fscan_speed_fast = g_ConfigData.LaserSensorAlgParam.fscan_speed_fast;
				p_LaserSensorAlgParam->fprobe_diameter = g_ConfigData.LaserSensorAlgParam.fprobe_diameter;
				p_LaserSensorAlgParam->funknown_diameter = g_ConfigData.LaserSensorAlgParam.funknown_diameter;
				p_LaserSensorAlgParam->cmode = g_ConfigData.LaserSensorAlgParam.cmode;
				p_LaserSensorAlgParam->cmode_fast = g_ConfigData.LaserSensorAlgParam.cmode_fast;
				p_LaserSensorAlgParam->cuse_avg = g_ConfigData.LaserSensorAlgParam.cuse_avg;
				p_LaserSensorAlgParam->cfull_scan = g_ConfigData.LaserSensorAlgParam.cfull_scan;
				p_LaserSensorAlgParam->cgdata_sel = g_ConfigData.LaserSensorAlgParam.cgdata_sel;
				p_LaserSensorAlgParam->cassume_posn = g_ConfigData.LaserSensorAlgParam.cassume_posn;
				p_LaserSensorAlgParam->cassume_posn_fast = g_ConfigData.LaserSensorAlgParam.cassume_posn_fast;
				p_LaserSensorAlgParam->crect_center = g_ConfigData.LaserSensorAlgParam.crect_center;
				p_LaserSensorAlgParam->cloops = g_ConfigData.LaserSensorAlgParam.cloops;
				p_LaserSensorAlgParam->cdelta_mode = g_ConfigData.LaserSensorAlgParam.cdelta_mode;
				p_LaserSensorAlgParam->idelta_flat = htons(g_ConfigData.LaserSensorAlgParam.idelta_flat);
				p_LaserSensorAlgParam->fdelta_basespan = g_ConfigData.LaserSensorAlgParam.fdelta_basespan;
				p_LaserSensorAlgParam->idelta_pos = htons(g_ConfigData.LaserSensorAlgParam.idelta_pos);
				p_LaserSensorAlgParam->idelta_neg = htons(g_ConfigData.LaserSensorAlgParam.idelta_neg);
				p_LaserSensorAlgParam->fdelta_span = g_ConfigData.LaserSensorAlgParam.fdelta_span;
				p_LaserSensorAlgParam->fdelta_edge = g_ConfigData.LaserSensorAlgParam.fdelta_edge;
				p_LaserSensorAlgParam->fpc_aspect_diff = g_ConfigData.LaserSensorAlgParam.fpc_aspect_diff;
				p_LaserSensorAlgParam->fmax_aspect_diff = g_ConfigData.LaserSensorAlgParam.fmax_aspect_diff;
				p_LaserSensorAlgParam->fmax_over_exp_diameter = g_ConfigData.LaserSensorAlgParam.fmax_over_exp_diameter;
				p_LaserSensorAlgParam->fmax_under_exp_diameter = g_ConfigData.LaserSensorAlgParam.fmax_under_exp_diameter;
				p_LaserSensorAlgParam->fmax_csnk_diff = g_ConfigData.LaserSensorAlgParam.fmax_csnk_diff;
				p_LaserSensorAlgParam->fmax_over_csnk = g_ConfigData.LaserSensorAlgParam.fmax_over_csnk;
				p_LaserSensorAlgParam->fmax_under_csnk = g_ConfigData.LaserSensorAlgParam.fmax_under_csnk;
				SmartToolMsg(p_STPSession, STP_GET_RESP, uiOID, sizeof(td_LaserSensorAlgParam), p_cSTPobjValBuff);
				break;
			case MINIFT_OID_CAM_ALG_PARAM:
				p_CamAlgParam = (td_CamAlgParam *) p_cSTPobjValBuff;
				p_CamAlgParam->fmove_speed = g_ConfigData.CamAlgParam.fmove_speed;
				p_CamAlgParam->cInfoMask = g_ConfigData.CamAlgParam.cInfoMask;
				p_CamAlgParam->cAMode = g_ConfigData.CamAlgParam.cAMode;
				p_CamAlgParam->cCMode = g_ConfigData.CamAlgParam.cCMode;
				p_CamAlgParam->cAux1 = g_ConfigData.CamAlgParam.cAux1;
				p_CamAlgParam->fmove_required = g_ConfigData.CamAlgParam.fmove_required;
				p_CamAlgParam->fmax_over_exp_diameter = g_ConfigData.CamAlgParam.fmax_over_exp_diameter;
				p_CamAlgParam->fmax_under_exp_diameter = g_ConfigData.CamAlgParam.fmax_under_exp_diameter;
				p_CamAlgParam->fmax_csnk_diff = g_ConfigData.CamAlgParam.fmax_csnk_diff;
				p_CamAlgParam->fmax_over_csnk = g_ConfigData.CamAlgParam.fmax_over_csnk;
				p_CamAlgParam->fmax_under_csnk = g_ConfigData.CamAlgParam.fmax_under_csnk;
				SmartToolMsg(p_STPSession, STP_GET_RESP, uiOID, sizeof(td_CamAlgParam), p_cSTPobjValBuff);
				break;
			case MINIFT_OID_VISION_AUTO_INSPECT:
				SmartToolMsgChar(p_STPSession, STP_GET_RESP, uiOID, g_ConfigData.cVisionAutoInspect);
				break;
			case MINIFT_OID_PROCESS_CONTINUE_MODE:
				SmartToolMsgChar(p_STPSession, STP_GET_RESP, uiOID, g_ConfigData.cProcessContinueMode);
				break;
			case MINIFT_OID_PROCESS_OPERATIONS:
				ui = g_ConfigData.uiProcessOperations;
#ifdef OUTPUT_OPERATIONS
				logf("spop=%u\r\n", ui);
#endif
				SmartToolMsgUInt16(p_STPSession, STP_GET_RESP, uiOID, ui);
				break;
			case MINIFT_OID_DRILL_STATE:
				SmartToolMsgChar(p_STPSession, STP_GET_RESP, uiOID, g_cDrillState);
				break;
			case MINIFT_OID_DRILL_EXPLANATION:
				SmartToolMsgChar(p_STPSession, STP_GET_RESP, uiOID, g_cDrillExplanation);
				break;
			case MINIFT_OID_SEAL_STATE:
				SmartToolMsgChar(p_STPSession, STP_GET_RESP, uiOID, g_cSealState);
				break;
			case MINIFT_OID_SEAL_CLAMP:
				SmartToolMsgChar(p_STPSession, STP_GET_RESP, uiOID, g_ConfigData.cSealClamp);
				break;
			case MINIFT_OID_SEAL_PRESSURE_DELAY:
				SmartToolMsgInt16(p_STPSession, STP_GET_RESP, uiOID, g_ConfigData.iSealPressureDelay);
				break;
			case MINIFT_OID_SEAL_PRESSURE_RELEASE_DELAY:
				SmartToolMsgInt16(p_STPSession, STP_GET_RESP, uiOID, g_ConfigData.iSealPressureReleaseDelay);
				break;
			case MINIFT_OID_SEAL_PRIME_DELAY:
				p_PrimeDelay = (td_PrimeDelay *) p_cSTPobjValBuff;
				p_PrimeDelay->fdiameter1 = g_ConfigData.PrimeDelay.fdiameter1;
				p_PrimeDelay->idelay1 = htons(g_ConfigData.PrimeDelay.idelay1);
				p_PrimeDelay->fdiameter2 = g_ConfigData.PrimeDelay.fdiameter2;
				p_PrimeDelay->idelay2 = htons(g_ConfigData.PrimeDelay.idelay2);
				p_PrimeDelay->fdiameter3 = g_ConfigData.PrimeDelay.fdiameter3;
				p_PrimeDelay->idelay3 = htons(g_ConfigData.PrimeDelay.idelay3);
				p_PrimeDelay->fdiameter4 = g_ConfigData.PrimeDelay.fdiameter4;
				p_PrimeDelay->idelay4 = htons(g_ConfigData.PrimeDelay.idelay4);
				p_PrimeDelay->fdiameter5 = g_ConfigData.PrimeDelay.fdiameter5;
				p_PrimeDelay->idelay5 = htons(g_ConfigData.PrimeDelay.idelay5);
				SmartToolMsg(p_STPSession, STP_GET_RESP, uiOID, sizeof(td_PrimeDelay), p_cSTPobjValBuff);
				break;
			case MINIFT_OID_SEAL_GLOB_DELAY:
				SmartToolMsgInt16(p_STPSession, STP_GET_RESP, uiOID, g_ConfigData.iSealGlobDelay);
				break;
			case MINIFT_OID_SEAL_APPLY_DELAY:
				SmartToolMsgInt16(p_STPSession, STP_GET_RESP, uiOID, g_ConfigData.iSealApplyDelay);
				break;
			case MINIFT_OID_FILL_STATE:
				SmartToolMsgChar(p_STPSession, STP_GET_RESP, uiOID, g_cFillState);
				break;
			case MINIFT_OID_FILL_CLAMP:
				SmartToolMsgChar(p_STPSession, STP_GET_RESP, uiOID, g_ConfigData.cFillClamp_);
				break;
			case MINIFT_OID_FILL_EXTEND_DELAY:
				SmartToolMsgInt16(p_STPSession, STP_GET_RESP, uiOID, g_ConfigData.iFillExtendDelay_);
				break;
			case MINIFT_OID_FILL_RAM_DELAY:
				SmartToolMsgInt16(p_STPSession, STP_GET_RESP, uiOID, g_ConfigData.iFillRamDelay_);
				break;
			case MINIFT_OID_FASTENER_REQUEST:
				//FIXME000000000000
				//SmartToolMsgStr(p_STPSession, STP_GET_RESP, uiOID, xvarname);
				break;
			case MINIFT_OID_FASTENER_ARRIVED:
				SmartToolMsgChar(p_STPSession, STP_GET_RESP, uiOID, g_cFastenerArrived);
				break;
			case MINIFT_OID_FASTENER_POST_DELAY:
				SmartToolMsgInt16(p_STPSession, STP_GET_RESP, uiOID, g_ConfigData.iFastenerPostDelay_);
				break;
			case MINIFT_OID_ACCEL_ARM:
				SmartToolMsgChar(p_STPSession, STP_GET_RESP, uiOID, g_cAccelArm_);
				break;
			case MINIFT_OID_ACCEL_TRIGGER:
				SmartToolMsgChar(p_STPSession, STP_GET_RESP, uiOID, g_cAccelTrigger_);
				break;
			case MINIFT_OID_TOOL_Z_BASE:
				SmartToolMsgFloat(p_STPSession, STP_GET_RESP, uiOID, g_ConfigData.fToolZBase);
				break;
			case MINIFT_OID_FASTENER_FAULT:
				p_FastenerFault = (td_FastenerFault *) p_cSTPobjValBuff;
				p_FastenerFault->cDevice = g_FastenerFault.cDevice;
				p_FastenerFault->cSeverity = g_FastenerFault.cSeverity;
				p_FastenerFault->lFaultCode = htonl(g_FastenerFault.lFaultCode);
				SmartToolMsg(p_STPSession, STP_GET_RESP, uiOID, sizeof(td_FastenerFault), p_cSTPobjValBuff);
				break;
			case MINIFT_OID_POSNMODE_ACTIVE_PREMOVEXY:
				SendActivePremove(p_STPSession, STP_GET_RESP);
				break;
			case MINIFT_OID_DRILL_FAULT:
				p_DrillFault = (td_DrillFault *) p_cSTPobjValBuff;
				p_DrillFault->cDevice = g_DrillFault.cDevice;
				p_DrillFault->cSeverity = g_DrillFault.cSeverity;
				p_DrillFault->lFaultCode = htonl(g_DrillFault.lFaultCode);
				SmartToolMsg(p_STPSession, STP_GET_RESP, uiOID, sizeof(td_DrillFault), p_cSTPobjValBuff);
				break;
			case MINIFT_OID_HOLE_RESULT_DATA:
				p_HoleResultData = (td_HoleResultData *) p_cSTPobjValBuff;
				p_HoleResultData->iHoleNumber = htons(g_HoleResultData.iHoleNumber);
				p_HoleResultData->iHoleResult = htons(g_HoleResultData.iHoleResult);
				SmartToolMsg(p_STPSession, STP_GET_RESP, uiOID, sizeof(td_HoleResultData), p_cSTPobjValBuff);
				break;
			case MINIFT_OID_MOVEXY:
				//FIXME Minor  No Get for this currently
				//	p_oid_movexy=(td_oid_movexy *)p_cSTPobjValBuff;
				//	//FIXME FillStructWithData(p_oid_movexy);
				//	p_oid_movexy->fMachineX=p_oid_movexy->fMachineX;
				//	p_oid_movexy->fMachineY=p_oid_movexy->fMachineY;
				//	SmartToolMsg(p_STPSession, STP_GET_RESP, uiOID, sizeof(td_oid_movexy),p_cSTPobjValBuff);
				break;
			case MINIFT_OID_POSN_DISPLAY:
				p_PosnDisplay = (td_PosnDisplay *) p_cSTPobjValBuff;
				p_PosnDisplay->cmode = g_ConfigData.PosnDisplay.cmode;
				p_PosnDisplay->corigin = g_ConfigData.PosnDisplay.corigin;
				p_PosnDisplay->ccontent = g_ConfigData.PosnDisplay.ccontent;
				SmartToolMsg(p_STPSession, STP_GET_RESP, uiOID, sizeof(td_PosnDisplay), p_cSTPobjValBuff);
				break;
			case MINIFT_OID_POSNMODE_XYDATA_ID:
				//Machine Data Set Code
				//Send MachineDataSetID now
				SendXYDataID(p_STPSession, STP_GET_RESP);
				break;
			case MINIFT_OID_POSNMODE_XYDATA:
				//Machine Data Set Code
				//The Data can be started at an index to avoid retransmit
				ui = p_STPrxMsg->uiValueLength;
				if (ui == 2)
				{
					//Start at a specific index
					ui = (unsigned int) ntohs(*((int*) (p_STPrxMsg->p_cObjectValue)));
				}
				else
				{
					//start at the front
					ui = 0;
				}
				StartSendXYData(p_STPSession->iSessionNum, ui);
				break;
			case MINIFT_OID_RFID_CONFIG:
				p_RFIDConfig = (td_RFIDConfig *) p_cSTPobjValBuff;
				p_RFIDConfig->cenabled = g_ConfigData.RFIDConfig.cenabled;
				p_RFIDConfig->cmethod = g_ConfigData.RFIDConfig.cmethod;
				p_RFIDConfig->uioptions = htons(g_ConfigData.RFIDConfig.uioptions);
				p_RFIDConfig->uicontinuousReadCycleTime = htons(g_ConfigData.RFIDConfig.uicontinuousReadCycleTime);
				p_RFIDConfig->uiseekReadCycleTime = htons(g_ConfigData.RFIDConfig.uiseekReadCycleTime);
				p_RFIDConfig->fseekMove1 = g_ConfigData.RFIDConfig.fseekMove1;
				p_RFIDConfig->fseekMove2 = g_ConfigData.RFIDConfig.fseekMove2;
				p_RFIDConfig->fseekFineMove = g_ConfigData.RFIDConfig.fseekFineMove;
				p_RFIDConfig->fseekSpeed = g_ConfigData.RFIDConfig.fseekSpeed;
				p_RFIDConfig->fseekFineSpeed = g_ConfigData.RFIDConfig.fseekFineSpeed;
				p_RFIDConfig->fRFIDOffset = g_ConfigData.RFIDConfig.fRFIDOffset;
				p_RFIDConfig->fseekPastBorder = g_ConfigData.RFIDConfig.fseekPastBorder;
				p_RFIDConfig->fminWindowSize = g_ConfigData.RFIDConfig.fminWindowSize;
				p_RFIDConfig->fmaxWindowSize = g_ConfigData.RFIDConfig.fmaxWindowSize;
				SmartToolMsg(p_STPSession, STP_GET_RESP, uiOID, sizeof(td_RFIDConfig), p_cSTPobjValBuff);
				break;
			case MINIFT_OID_RFID_DATA:
				SendRFIDData(p_STPSession, STP_GET_RESP);
				break;
			case MINIFT_OID_READ_RFID:
				SmartToolMsgChar(p_STPSession, STP_GET_RESP, uiOID, g_cReadRFID);
				break;
			case MINIFT_OID_ESTOP_CLEAR_DELAY:
				SmartToolMsgUInt16(p_STPSession, STP_GET_RESP, uiOID, g_ConfigData.uiEStopClearDelay);
				break;
			case MINIFT_OID_DRILL_BUTTON_DELAY:
				SmartToolMsgUInt16(p_STPSession, STP_GET_RESP, uiOID, g_ConfigData.uiDrillButtonDelay);
				break;
			case MINIFT_OID_USE_CUTTER_DETECT:
				SmartToolMsgChar(p_STPSession, STP_GET_RESP, uiOID, g_ConfigData.cUseCutterDetect);
				break;
			case MINIFT_OID_JOG_ENABLE_TIMEOUT:
				SmartToolMsgChar(p_STPSession, STP_GET_RESP, uiOID, g_ConfigData.cJogEnableTimeout);
				break;
			case MINIFT_OID_DRILL_CYCLE_DELAY:
				SmartToolMsgUInt16(p_STPSession, STP_GET_RESP, uiOID, g_ConfigData.uiDrillCycleDelay);
				break;
			case MINIFT_OID_INSPECT_METHOD:
				SmartToolMsgChar(p_STPSession, STP_GET_RESP, uiOID, g_ConfigData.cInspectMethod);
				break;
			case MINIFT_OID_COMMAND_INSPECT_METHOD:
				SmartToolMsgChar(p_STPSession, STP_GET_RESP, uiOID, g_ConfigData.cCommandInspectMethod);
				break;
			case MINIFT_OID_FORCE_SENSOR_CALIBRATION:
				p_ForceSensorCalibration = (td_ForceSensorCalibration *) p_cSTPobjValBuff;
				p_ForceSensorCalibration->iZeroX = htons(g_ConfigData.ForceSensorCalibration.iZeroX);
				p_ForceSensorCalibration->iZeroY = htons(g_ConfigData.ForceSensorCalibration.iZeroY);
				p_ForceSensorCalibration->iZeroZ = htons(g_ConfigData.ForceSensorCalibration.iZeroZ);
				p_ForceSensorCalibration->iCountsPerGX = htons(g_ConfigData.ForceSensorCalibration.iCountsPerGX);
				p_ForceSensorCalibration->iCountsPerGY = htons(g_ConfigData.ForceSensorCalibration.iCountsPerGY);
				p_ForceSensorCalibration->iCountsPerGZ = htons(g_ConfigData.ForceSensorCalibration.iCountsPerGZ);
				SmartToolMsg(p_STPSession, STP_GET_RESP, uiOID, sizeof(td_ForceSensorCalibration), p_cSTPobjValBuff);
				break;
			case MINIFT_OID_FORCE_SENSOR:
				p_ForceSensor = (td_ForceSensor *) p_cSTPobjValBuff;
				p_ForceSensor->fX = g_ForceSensor.fX;
				p_ForceSensor->fY = g_ForceSensor.fY;
				p_ForceSensor->fZ = g_ForceSensor.fZ;
				p_ForceSensor->cErrFlag = g_ForceSensor.cErrFlag;
				SmartToolMsg(p_STPSession, STP_GET_RESP, uiOID, sizeof(td_ForceSensor), p_cSTPobjValBuff);
				break;
			case MINIFT_OID_FORCE_LIMITS:
				p_ForceLimits = (td_ForceLimits *) p_cSTPobjValBuff;
				p_ForceLimits->uiSensorInterval = htons(g_ConfigData.ForceLimits.uiSensorInterval);
				p_ForceLimits->uiMinUpdateDelta = htons(g_ConfigData.ForceLimits.uiMinUpdateDelta);
				p_ForceLimits->cActive = g_ConfigData.ForceLimits.cActive;
				p_ForceLimits->cCurrentUnderMethod = g_ConfigData.ForceLimits.cCurrentUnderMethod;
				p_ForceLimits->uiCurrentOverX = htons(g_ConfigData.ForceLimits.uiCurrentOverX);
				p_ForceLimits->uiCurrentUnderX = htons(g_ConfigData.ForceLimits.uiCurrentUnderX);
				p_ForceLimits->uiCurrentOverY = htons(g_ConfigData.ForceLimits.uiCurrentOverY);
				p_ForceLimits->uiCurrentUnderY = htons(g_ConfigData.ForceLimits.uiCurrentUnderY);
				p_ForceLimits->uiFullGravX = htons(g_ConfigData.ForceLimits.uiFullGravX);
				p_ForceLimits->uiFullGravY = htons(g_ConfigData.ForceLimits.uiFullGravY);
				p_ForceLimits->uiFlatForceX = htons(g_ConfigData.ForceLimits.uiFlatForceX);
				p_ForceLimits->uiFlatForceY = htons(g_ConfigData.ForceLimits.uiFlatForceY);
				SmartToolMsg(p_STPSession, STP_GET_RESP, uiOID, sizeof(td_ForceLimits), p_cSTPobjValBuff);
				break;
			case MINIFT_OID_PROBE_FLAGS:
				SmartToolMsgChar(p_STPSession, STP_GET_RESP, uiOID, g_ConfigData.cProbeFlags);
				break;
			case MINIFT_OID_MO_CAL:
				p_MOCal = (td_MOCal *) p_cSTPobjValBuff;
#ifdef OBSTRUCTION_SYSTEM_MO_ANA
				p_MOCal->uim1=htons(g_ConfigData.MOCal.uim1);
				p_MOCal->uim2=htons(g_ConfigData.MOCal.uim2);
				p_MOCal->uim3=htons(g_ConfigData.MOCal.uim3);
				p_MOCal->uim4=htons(g_ConfigData.MOCal.uim4);
				p_MOCal->uim5=htons(g_ConfigData.MOCal.uim5);
				p_MOCal->uim6=htons(g_ConfigData.MOCal.uim6);
#else
				memset(p_cSTPobjValBuff, 0, sizeof(td_MOCal));
#endif
				SmartToolMsg(p_STPSession, STP_GET_RESP, uiOID, sizeof(td_MOCal), p_cSTPobjValBuff);
				break;
			case MINIFT_OID_RFID_TAG_SET:
				#ifdef USE_RFID_OMRON
				//When they do a get of this, return whatever the last read tag data was, but since this OID is designed to
				//use one format of OID only, ensure that this data will fit.
				memset(p_cSTPobjValBuff,0,sizeof(td_oid_rfid_tag_set));//fill with null, because it's better than spaces for the return value
				if (g_RfidStateAndData.iTagDataSize>2)
				{
					ui=g_RfidStateAndData.iTagDataSize - 2; //do not copy end code
					if (ui>sizeof(td_oid_rfid_tag_set))
					{
						ui=sizeof(td_oid_rfid_tag_set);
					}
					memcpy(p_cSTPobjValBuff,g_RfidStateAndData.szTagData+2,ui); //do not include 1st 2 chars which have endcode
				}
				SmartToolMsg(p_STPSession, STP_GET_RESP, uiOID, sizeof(td_oid_rfid_tag_set), p_cSTPobjValBuff);
				p_cSTPobjValBuff[ui]=0;
				logf("rtsfw \"%s\"\r\n",p_cSTPobjValBuff);
#endif
				break;
			case MINIFT_OID_KHOLE_MAX_DISTANCE_CHECK:
				SmartToolMsgFloat(p_STPSession, STP_GET_RESP, uiOID, g_ConfigData.fMaxKholeDistanceCheck);
				break;
			case MINIFT_OID_MAX_EDGE_SHIFT_PROBE_ACCEPT:
				SmartToolMsgFloat(p_STPSession, STP_GET_RESP, uiOID, g_ConfigData.fMaxEdgeShiftProbeAccept);
				break;
			case MINIFT_OID_ALLOW_DRILL_BEYOND_SHIFT_LIMITS:
				SmartToolMsgChar(p_STPSession, STP_GET_RESP, uiOID, g_ConfigData.cAllowDrillBeyondShiftLimits);
				break;
			case MINIFT_OID_NAC_SERIAL_NUMBER:
				//FIXME PORT MEDHIGH
//				SmartToolMsgStr(p_STPSession, STP_GET_RESP, uiOID, g_szRailSerialNumber);
				break;
			case MINIFT_OID_Y_RETRACT:
				//FIXME INCOMPLETE FEATURE
				SmartToolMsgChar(p_STPSession, STP_GET_RESP, uiOID, 0);
				break;
			case MINIFT_OID_SYSTEM_COMPONENTS:
				SendSystemComponents(p_STPSession, STP_GET_RESP);
				break;
			case MINIFT_OID_RFID_TAG_SET2:
				#ifdef USE_RFID_MIFARE
				p_rfid_f0 = (td_RFID_F0 *) g_bRFIDTagData;
				p_oid_rfid_tag_set2 = (td_oid_rfid_tag_set2 *) p_cSTPobjValBuff;
				if (p_rfid_f0->cFormat == 0)
				{
					p_oid_rfid_tag_set2->cFormat = p_rfid_f0->cFormat;
					p_oid_rfid_tag_set2->cRailType = p_rfid_f0->cRailType;
					p_oid_rfid_tag_set2->cGroup = p_rfid_f0->cGroup;
					p_oid_rfid_tag_set2->cSegment = p_rfid_f0->cSegmentAndRailSide & 0x7F;
					p_oid_rfid_tag_set2->cRailSide = 0;
					if ((p_rfid_f0->cSegmentAndRailSide & 128) > 0)
					{
						p_oid_rfid_tag_set2->cRailSide = 1;
					}
					p_oid_rfid_tag_set2->ulSerialNumber = p_rfid_f0->ulSerialNumber; //tag data is already in network order
					p_oid_rfid_tag_set2->ulPosition = p_rfid_f0->ulPosition;
					p_oid_rfid_tag_set2->ulSegmentPosition = p_rfid_f0->ulSegmentPosition;
					SmartToolMsg(p_STPSession, STP_GET_RESP, uiOID, sizeof(td_oid_rfid_tag_set2), p_cSTPobjValBuff);
				}
#endif
				break;
				//OIDs from Tool Module
			case TOOLMANAGEMENT_OID_TOOL_MGMT:
				p_oid_tool_mgmt = (td_oid_tool_mgmt *) p_STPrxMsg->p_cObjectValue;
				c_op = p_oid_tool_mgmt->coperation;
				if (c_op == load)
				{
					if (g_szToolIDlen == 0)
					{
						c_op = unload;
						SendToolMGMT(p_STPSession, STP_GET_RESP, c_op, 0, 0, "", 0);
						break;
					}
					SendTool(p_STPSession, STP_GET_RESP, c_op);
				}
				//handle everything else in a common way
				goto handle_tool_mgmt;
				break;
			case TOOLMANAGEMENT_OID_TOOL_REC:
				//FIXMENOWzxcv  see same issue with OID and labels and space...
				c_op = load; //MiniFTControl pays more attention to the ID for loaded or unloaded status,
							 //but Cell controller wants to see that op reflect the loaded status.
				if (g_szToolIDlen == 0)
				{
					c_op = unload;
				}
				SendTool(p_STPSession, STP_ALERT, c_op);
				break;
				//MakeOID-generated::END
			default:
				#ifdef OUTPUT_RXSTP
				p_szOIDName = DisplayOIDName(p_STPSession, uiOID);
				goto show_unsupported_stp_message;
#endif
				break;
			}
			break;
		case STP_SET:
			switch (uiOID)
			{
			//MakeOID-generated:: STP SET (oid merge)
			case COMMON_OID_NULLOID:
				//Do nothing
				break;
			case COMMON_OID_DEBUG:
				//This debug capabillity is disabled
				SocketConsoleClose(); //gives a way to reset this
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
				logf("OID_RESET_SYSTEM Not Implemented");
				break;
			case COMMON_OID_SCRIPT_TRANSFER:
				//FIXME minor  Implemented on Pendant, but not on Rabbit
				break;
			case COMMON_OID_GENERICMESSAGE:
				// fixme minor: do nothing with a SETting of a generic string message
				break;
			case COMMON_OID_DEBUGMESSAGE:
				// fixme minor: do nothing with a SETting of a debug string message
				break;
			case COMMON_OID_COMMON_MESSAGE_CODE:
				// return nothing... used entirely for alerts
				break;
			case COMMON_OID_CONTROL_DELAY:
				//code indicates UI delay
#ifdef OUTPUT_SESSION_EVENTS
				logf("Cntrl Dly\r\n");
#endif
				StopInterfaceTasks();
				break;
			case COMMON_OID_DIRECT_DOUT:
				p_oid_direct_dout =
						(td_oid_direct_dout *) p_STPrxMsg->p_cObjectValue;
				if (p_oid_direct_dout->cchannel < 16)
				{
					if (p_oid_direct_dout->cvalue != 0)
					{
						p_oid_direct_dout->cvalue = 1;
					}
					//logf("S%d=%d\r\n",p_oid_direct_dout->cchannel,p_oid_direct_dout->cvalue);
					digOut(p_oid_direct_dout->cchannel,
							p_oid_direct_dout->cvalue);
				}
				break;
			case COMMON_OID_DIRECT_DIN:
				p_oid_direct_din =
						(td_oid_direct_din *) p_STPrxMsg->p_cObjectValue;
				p_oid_direct_din->cvalue = digIn(p_oid_direct_din->cchannel);
				//logf("I%d=%d\r\n",p_oid_direct_din->cchannel,p_oid_direct_din->cvalue);
				break;
			case COMMON_OID_STREAM_SYNC:
				//Echo Back same value
				SmartToolMsg(p_STPSession, STP_ALERT, uiOID, 2,
						p_STPrxMsg->p_cObjectValue);
				//NTOHS value for display here
				//ui=(unsigned int)ntohs( *((int*)(p_STPrxMsg->p_cObjectValue)) );
				//logf("STREAM_SYNC %u\r\n", ui);
				break;
			case COMMON_OID_STP_CLOSE:
				i = p_STPrxMsg->p_cObjectValue[0];
				//FIXME MED should close this side now.
				//logf("STP_CLOSE %d\r\n",i);
				break;
//FIXME SEVERE
//
//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
// ... fix oid label bere
//
//				case fix_OID_fix_MESSAGE_CODE:
//                	// return nothing... used entirely for alerts
//					break;
			case COMMON_OID_STP_IDENTIFY:
				//The New Identify
				SmartToolMsgStr(p_STPSession, STP_ALERT, COMMON_OID_SMARTTOOL_TYPE, SMARTTOOL_TYPE_STRING);
				SmartToolMsgStr(p_STPSession, STP_ALERT, COMMON_OID_SMARTTOOL_SUBTYPE, SMARTTOOL_SUBTYPE_STRING);
				SmartToolMsgStr(p_STPSession, STP_ALERT, COMMON_OID_SYSTEM_VERSION, SMARTTOOL_SUBTYPE_STRING " " SYSTEM_VERSION_STRING);
				SmartToolMsgStr(p_STPSession, STP_ALERT, COMMON_OID_SERIAL_NUMBER, g_szSerialNumber);
				SmartToolMsgUInt16(p_STPSession, STP_ALERT, COMMON_OID_COM_VERSION, COM_VERSION);
				SmartToolMsgEmpty(p_STPSession, STP_ALERT, COMMON_OID_STP_IDENTIFY);
				break;
			case COMMON_OID_DATE_TIME:
				HandleCommonOIDDateTimeGet(p_STPSession);
#ifdef CLAMP_SYSTEM_NAC_STP
				NACSendDateTime();
#endif
				break;
				//end common OIDs
				//MiniFT OIDs
			case MINIFT_OID_MODE:
				g_cModeState = p_STPrxMsg->p_cObjectValue[0];
				break;
			case MINIFT_OID_RESET_MC:
				MCRunRestart();
				break;
			case MINIFT_OID_ENCODER_RATIO:
				p_EncoderRatio = (td_EncoderRatio *) p_STPrxMsg->p_cObjectValue;
				g_ConfigData.EncoderRatio.fX = p_EncoderRatio->fX;
				g_ConfigData.EncoderRatio.fY = p_EncoderRatio->fY;
				MCSetEncoderRatios();
				break;
			case MINIFT_OID_MC_CURRENT_LIMITS:
				p_MCCurrentLimit =
						(td_MCCurrentLimit *) p_STPrxMsg->p_cObjectValue;
				g_ConfigData.MCCurrentLimit.fX = p_MCCurrentLimit->fX;
				g_ConfigData.MCCurrentLimit.fY = p_MCCurrentLimit->fY;
				goto common_code_set_current_limits;
				break;
			case MINIFT_OID_MC_PEAK_CURRENT_LIMITS:
				p_MCPeakCurrentLimit =
						(td_MCPeakCurrentLimit *) p_STPrxMsg->p_cObjectValue;
				g_ConfigData.MCPeakCurrentLimit.fX = p_MCPeakCurrentLimit->fX;
				g_ConfigData.MCPeakCurrentLimit.fY = p_MCPeakCurrentLimit->fY;
				common_code_set_current_limits:
				MCSetCurrentLimits(
						g_ConfigData.MCCurrentLimit.fX,
						g_ConfigData.MCCurrentLimit.fY,
						g_ConfigData.MCPeakCurrentLimit.fX,
						g_ConfigData.MCPeakCurrentLimit.fY);
				break;
			case MINIFT_OID_BRAKE_ON_TIMEOUT:
				g_ConfigData.uiBrakeOnTimeout = (unsigned int) ntohs(
						*((int*) (p_STPrxMsg->p_cObjectValue)));
				break;
			case MINIFT_OID_STOREDEFAULT_CONFIG:
				SaveConfigFile();
				if (g_cConfigSaveSuccess)
				{
					uiMessageCode = MINIFTMC_STOREDEFAULT_SUCCESS;
				}
				else
				{
					uiMessageCode = MINIFTMC_STOREDEFAULT_FAILURE;
				}
				// provide user with a message to act as feedback that the config store worked (or didn't)
				SmartToolMsgMiniFtMessageCode(p_STPSession, MINIFT_OID_STOREDEFAULT_CONFIG, uiMessageCode);
				break;
			case MINIFT_OID_RECALL_CONFIG:
				iTemp = p_STPrxMsg->p_cObjectValue[0];
				//option 0 is no longer supported
				if (iTemp == 1)
				{
					//reload the config...
					LoadConfigFile();
					if (g_cConfigLoadSuccess)
					{
						uiMessageCode = MINIFTMC_LOADDEFAULT_SUCCESS;
					}
					else
					{
						uiMessageCode = MINIFTMC_LOADDEFAULT_FAILURE;
					}
				}
				else
				{
					uiMessageCode = MINIFTMC_INVALID_PARAMETER;
				}
				// provide user with a message to act as feedback that the config store worked (or didn't)
				SmartToolMsgMiniFtMessageCode(p_STPSession, MINIFT_OID_RECALL_CONFIG, uiMessageCode);
				MCClearMotionSet();
				//FIXME PORTHIGH2 after reload must set vars like remote script set would...
				//     harder issue... is it worth it ?
				break;
			case MINIFT_OID_EEOPTION_DEFAULT:
				#ifdef EEOPTION_PERMANENT
				//Never Allow EEOPTION CHANGES
#else
				g_ConfigData.cEEOptionDefault=p_STPrxMsg->p_cObjectValue[0];
				//logf("EEOpDef=%d\r\n",g_ConfigData.cEEOptionDefault);
#endif
				break;
			case MINIFT_OID_EEOPTION:
				//Currently we use off values generally, and we have no eeoption switches in default valve state.
#ifdef	EEOPTION_PERMANENT
				//Never Allow EEOPTION CHANGES
#else
				SetDefaultValveState();
				//now set new option
				i=p_STPrxMsg->p_cObjectValue[0];
				if (i==EEDEFAULT)
				{
					i=g_ConfigData.cEEOptionDefault;
					//logf("EEOp Use Def\r\n");
				}
				if (i != g_cEEOption)
				{
					g_cEEOption=i;
					//It's Changed
#ifdef HD_RAIL_STP
					InitRailSTP();
#endif
				}
				//And Do an Echo to ensure sync of value.
				SmartToolMsgChar(p_STPSession, STP_ALERT, uiOID, g_cEEOption);
				//logf("EEOp=%d\r\n",g_cEEOption);
#endif
				break;
//fIXME99999999
//place this
//                    	//special code will cause profiling to be sent back
//                        //Now Dump Profile points to output.
//						PrintProfilePoints();
//						ClearProfiling(); //erase buffer.. start again
//						//don't set the parameter

			case MINIFT_OID_BEEPER:
				c = p_STPrxMsg->p_cObjectValue[0];
				if (c <= 1)
				{
					//Active Or Deactivate the beeper
					g_ConfigData.cbeeper = c;
				}
				else
				{
					ui = c * 4;
					//make a beep of this delay
					//issue the beep for this duration
					BeepCountPrivate(BEEPONCE, 1, c, c);
				}
				break;
			case MINIFT_OID_TOOL_VERIFYENABLE:
				g_ConfigData.cToolVerifyEnable = p_STPrxMsg->p_cObjectValue[0];
				break;
			case MINIFT_OID_TOOL_RESERVED:
				break;
			case MINIFT_OID_PROCESS:
				//Read Only Currently:  MiniFT Loads the Processes as needed.
				//g_cLoadedProcess=p_STPrxMsg->p_cObjectValue[0];
				//#ifdef OUTPUT_PROCESS_AND_TOOL
				//logf("Rx Proc %d\r\n",g_cLoadedProcess);
				//#endif
				break;
			case MINIFT_OID_RETURN_HEIGHT:
				f = *(float *) p_STPrxMsg->p_cObjectValue;
				g_ConfigData.fReturnHeight = f;
				LoadToolHomeBackToDrill();
				break;
			case MINIFT_OID_GAUGE_LENGTH:
				//This can set g_fGaugeLength, but it's not the prefered way, which is for drill sync to do it.
				//This is more for special testing
				//Not Relation to DRILL_DIRECT_READY
				f = *(float *) p_STPrxMsg->p_cObjectValue;
				g_fGaugeLength = f;
#ifdef OUTPUT_PROCESS_AND_TOOL
				logf("Rx GL %f\r\n",f);
#endif
				LoadToolHomeBackToDrill();
				break;
			case MINIFT_OID_SCALE_MODE:
				#ifdef LINEAR_SCALE_OPTION
				g_ConfigData.cScaleMode = p_STPrxMsg->p_cObjectValue[0];
#else
				g_ConfigData.cScaleMode=0; //force to zero
#endif
				break;
			case MINIFT_OID_RESERVED_2:
				break;
			case MINIFT_OID_PARTPGM_CLEAR:
				g_szPartPgmFilename[0] = 0;
				ClearPartProgram();
				g_PartPgmInfo.cLocked = 0; //unlocked when cleared by OID
				SmartToolMsgChar(p_STPSession, STP_ALERT,
						MINIFT_OID_PARTPGM_STATUS, g_cPartPgmStatus);
				break;
			case MINIFT_OID_PARTPGM_NAME:
				ui = p_STPrxMsg->uiValueLength;	// If PP name is too long, abort.
				if (ui > MAX_FILENAME_LEN)
				{
					SmartToolMsgCommonMessageCode(p_STPSession, uiOID, COMMONMC_BADLENGTH);
					ui = 0;	//wipe out string
				}
				else
				{
					memcpy(g_szPartPgmFilename, p_STPrxMsg->p_cObjectValue, ui);
					g_szPartPgmFilename[ui] = 0;
#ifdef OUTPUT_PP_SYS
					logf("PPnm=%s\r\n",g_szPartPgmFilename); //debug
#endif
					g_iProgramLoadingSession = 0xFF; //Prepared for loading fresh
				}
				break;
			case MINIFT_OID_PARTPGM_DATA:
				ui = p_STPrxMsg->uiValueLength;
				//new style is not true streaming, but is superior because it doesn't
				//lock out other communication while the file is being parsed.
				if (g_iProgramLoadingSession == 0xFF)
				{
					if (g_cConfigLoadSuccess == 0)
					{
						//They Can't Load a file yet
						SmartToolMsgMiniFtMessageCode(p_STPSession, MINIFT_OID_PARTPGM_DATA, MINIFTMC_PROGRAM_WITHOUT_MEMORY_CONFIG);
						break;
					}
					//This packet is the 1st in the series
					if (g_cPartPgmStatus == PP_LOADING)
					{
						//it was already loading, and they are startin a new one
						//Should be an error???? a warning????
					}
					//We want to start loading, and it's clear so we'll go ahead.
					g_iProgramLoadingSession = p_STPSession->iSessionNum;
					//Start Parse....
					g_sPPDirect = (char *) p_STPrxMsg->p_cObjectValue;
					g_iPPDirectPos = 0;
					g_iPPDirectEnd = ui;
					ParsePartProgramStart();
#ifdef OUTPUT_PP_SYS
					logf("PPstrt\r\n");
#endif
				}
				else if (g_cPartPgmStatus != PP_LOADING)
				{
					if (ui == 0)
					{
						//This is a final packet marker...
						//The program must have just completed so just ignore this
						//FIXME FUTURE put method in packet itself that we can know if it's
						//continued. OR put timeout in OR both
					}
					else
					{
						//It's not currently loading, so it was either not setup correctly to start, or
						//it must have failed, but they are still sending....   Ignore it.
#ifdef OUTPUT_PP_SYS
						logf("Not Lding!\r\n");
#endif
					}
					break;
				}
				else if (g_iProgramLoadingSession != p_STPSession->iSessionNum)
				{
//FIXME0000000000
//better lifetime pattern for this variable
					//somebody else started loading it 1st.
					//IGNORE these packets
					//send back an error
					//FIXME0000000000000 error messag
					//Idea: Maybe a timeout would be good as a fail-safe reset????
#ifdef OUTPUT_PP_SYS
					logf("Diff s lding\r\n");
#endif
					//May need to have this cause a load failure for both
					//so that it will know it didn't succeed.
					break;
				}
				else
				{
					//This packet is a continuation
					if (g_iPPDirectPos < g_iPPDirectEnd)
					{
						//some data left previously
						//copy the new data after the old and process the entire buffer
						g_sPPDirect = (char *) temppploadbuffer; //use this location, which is empty until after file load.
						memcpy(g_sPPDirect + g_iPPDirectEnd, (char *) p_STPrxMsg->p_cObjectValue, ui);
						g_iPPDirectEnd += ui;
					}
					else
					{
						//no data left previously
						g_sPPDirect = (char *) p_STPrxMsg->p_cObjectValue;
						g_iPPDirectPos = 0;
						g_iPPDirectEnd = ui;
					}
				}
				//It is loading
				//Handle This message
				g_cPPDirectFinalCall = 0;
				if (ui == 0)
				{
					//zero length message signals the end of the file load.
					g_cPPDirectFinalCall = 1;
					//FIXME9999999 should be done file previously
					//Create a warning for this
#ifdef OUTPUT_PP_SYS
					logf("Final Pckt+ddnt cmp\r\n"); //Final Packet but didn't complete yet
#endif
				}
				ParsePartProgramContinue(0);
				if (g_iPPDirectPos < g_iPPDirectEnd)
				{
					if (g_iPPDirectPos > 0)
					{
						g_iPPDirectEnd = g_iPPDirectEnd - g_iPPDirectPos; //new length
						memcpy((char *) temppploadbuffer, g_sPPDirect + g_iPPDirectPos, g_iPPDirectEnd);
						g_iPPDirectPos = 0; //new start is back at zero..
					}
				}
				break;
			case MINIFT_OID_PARTPGM_LINEARJOB:
				g_iPartPgmLinearHoles = ntohs(*((int16*) (p_STPrxMsg->p_cObjectValue)));
				if (g_iPartPgmLinearHoles < 0)
				{
					g_iPartPgmLinearHoles = 0;
				}
				LoadLinearProgram();
				g_cScaleBasedOnProbe = 1; //And Set This
				break;
			case MINIFT_OID_STARTOVER:
				//See StartOver(); for notes
				StartOver();
				break;
			case MINIFT_OID_LOAD_YRAIL:
				ui = p_STPrxMsg->p_cObjectValue[0];
				//clear clamp but don't need to wait for it.
#ifdef CLAMP_SYSTEM
				if (g_cClampState == CLAMP_CLAMP)
				{
					g_cClampGoal = CLAMP_LOOSE_OR_UNCLAMP;
				}
#endif
				RunLoadYRail((byte) ui);
				break;
			case MINIFT_OID_GRAVCOMP_AXES:
				g_ConfigData.cGravCompAxes = p_STPrxMsg->p_cObjectValue[0];
				break;
			case MINIFT_OID_GRAVCOMP_CMD:
				if (g_cAction > ACTION_READY)
				{
					//logf("NOG\r\n");
					SmartToolMsgMiniFtMessageCode(p_STPSession, MINIFT_OID_GRAVCOMP_CMD, MINIFTMC_GRAVCOMP_PREVENTED);
					break;
				}
				if (g_cGravCompStatus == GRAVCOMP_RUNNING)
				{
					SmartToolMsgMiniFtMessageCode(p_STPSession, MINIFT_OID_GRAVCOMP_CMD, MINIFTMC_GRAVCOMP_PREVIOUSLY_RUNNING);
				}
				else if (g_cModeState == MODE_ESTOP)
				{
					//marker
					SmartToolMsgMiniFtMessageCode(p_STPSession, MINIFT_OID_GRAVCOMP_CMD, MINIFTMC_GRAVCOMP_PREVENTED_BY_ESTOP);
				}
				else if (g_cObstructionCode != 0)
				{
					AlertObstructionCode(p_STPSession);
				}
				else if (g_cMoveDone != MOVEDONE_TRUE)
				{ // disallow grav comp unless carriage is done with previous move
					SmartToolMsgMiniFtMessageCode(p_STPSession,
							MINIFT_OID_GRAVCOMP_CMD,
							MINIFTMC_WAIT_FOR_CARRIAGE_STOP);
				}
				else if (g_cDrillState != DRILLSTATE_IDLE)
				{
					break;
				}
				else if (g_cClear == 1) //FIXME000000000000 clear should be 0 even if goal is just to clamp...
				{
					//Clear, so allow grav comp
					//May want to disable other operations the way rivet did during this time
					//start grav comp
					g_cGravCompStatus = GRAVCOMP_RUNNING;
					g_cFloatStatus = FLOATSTAT_NOFLOAT;
					SmartToolMsgChar(p_STPSession, STP_ALERT, MINIFT_OID_GRAVCOMP_STATUS, g_cGravCompStatus);
					SmartToolMsgChar(p_STPSession, STP_ALERT, MINIFT_OID_FLOAT_STATUS, g_cFloatStatus);
					MCSetMoveParams(g_ConfigData.fPosnTolerance * 10, 0); //Grav Comp Shares Tolerance Test for Move Success.
					RunGravComp();
					g_uiGravCompTime = MS_TIMER;
				}
				break;
			case MINIFT_OID_GRAVCOMP_FLOAT:
				if (g_cAction > ACTION_READY)
				{
					//logf("NOF\r\n");
					SmartToolMsgMiniFtMessageCode(p_STPSession, MINIFT_OID_GRAVCOMP_FLOAT, MINIFTMC_FLOAT_PREVENTED);
					break;
				}
				ui = p_STPrxMsg->p_cObjectValue[0];
				DoFloat(ui); //usually FLOAT_TOGGLE
				break;
			case MINIFT_OID_GRAVCOMP_SPEED:
				p_GravCompSpeed = (td_GravCompSpeed *) p_STPrxMsg->p_cObjectValue;
				g_ConfigData.GravCompSpeed.fX = p_GravCompSpeed->fX;
				g_ConfigData.GravCompSpeed.fY = p_GravCompSpeed->fY;
				MCClearMotionSet();
				break;
			case MINIFT_OID_GRAVCOMP_ACCEL:
				p_GravCompAcc = (td_GravCompAcc *) p_STPrxMsg->p_cObjectValue;
				g_ConfigData.GravCompAcc.fX = p_GravCompAcc->fX;
				g_ConfigData.GravCompAcc.fY = p_GravCompAcc->fY;
				MCClearMotionSet();
				break;
			case MINIFT_OID_GRAVCOMP_DECEL:
				p_GravCompDec = (td_GravCompDec *) p_STPrxMsg->p_cObjectValue;
				g_ConfigData.GravCompDec.fX = p_GravCompDec->fX;
				g_ConfigData.GravCompDec.fY = p_GravCompDec->fY;
				MCClearMotionSet();
				break;
			case MINIFT_OID_GRAVCOMP_MOVEDIST:
				p_GravCompMoveDist = (td_GravCompMoveDist *) p_STPrxMsg->p_cObjectValue;
				g_ConfigData.GravCompMoveDist.fX = p_GravCompMoveDist->fX;
				g_ConfigData.GravCompMoveDist.fY = p_GravCompMoveDist->fY;
				break;
			case MINIFT_OID_GRAVCOMP_ALGORITHM:
				g_ConfigData.cGravCompAlgorithm = p_STPrxMsg->p_cObjectValue[0];
				break;
			case MINIFT_OID_GRAVCOMP_NOISE_LIMIT:
				p_GravCompNoiseLimit = (td_GravCompNoiseLimit *) p_STPrxMsg->p_cObjectValue;
				g_ConfigData.GravCompNoiseLimit.fX = p_GravCompNoiseLimit->fX;
				g_ConfigData.GravCompNoiseLimit.fY = p_GravCompNoiseLimit->fY;
				break;
			case MINIFT_OID_GRAVCOMP_TRIGGERFACTOR:
				p_GravCompTriggerFactor = (td_GravCompTriggerFactor *) p_STPrxMsg->p_cObjectValue;
				g_ConfigData.GravCompTriggerFactor.fX = p_GravCompTriggerFactor->fX;
				g_ConfigData.GravCompTriggerFactor.fY = p_GravCompTriggerFactor->fY;
				break;
			case MINIFT_OID_FLOAT_SPEEDLIMIT:
				f = *(float *) p_STPrxMsg->p_cObjectValue;
				g_ConfigData.fFloatSpeedLimit = f;
				break;
			case MINIFT_OID_JOG_SPEEDLIMIT:
				f = *(float *) p_STPrxMsg->p_cObjectValue;
				g_ConfigData.fJogSpeedLimit = f;
				break;
			case MINIFT_OID_MAX_SPEED_X:
				f = *(float *) p_STPrxMsg->p_cObjectValue;
				g_ConfigData.fMaxSpeedX = f;
				MCSetErrorLimits();
				break;
			case MINIFT_OID_MAX_SPEED_Y:
				f = *(float *) p_STPrxMsg->p_cObjectValue;
				g_ConfigData.fMaxSpeedY = f;
				MCSetErrorLimits();
				break;
			case MINIFT_OID_X_RAIL_SURFACE_OFFSET:
				f = *(float *) p_STPrxMsg->p_cObjectValue;
				g_fXRailSurfaceOffset = f;
				break;
			case MINIFT_OID_PROBE_METHOD:
				g_cProbeMethod = p_STPrxMsg->p_cObjectValue[0];
				break;
			case MINIFT_OID_PROBE_METHOD_DEFAULT:
				g_ConfigData.cProbeMethodDefault = p_STPrxMsg->p_cObjectValue[0];
				break;
			case MINIFT_OID_PROBE_OFFSET:
				p_ProbeOffset = (td_ProbeOffset *) p_STPrxMsg->p_cObjectValue;
				g_ConfigData.ProbeOffset.fX = p_ProbeOffset->fX;
				g_ConfigData.ProbeOffset.fY = p_ProbeOffset->fY;
				break;
			case MINIFT_OID_PROBE_DIR:
				g_ConfigData.iProbeDir = ntohs(*((int*) (p_STPrxMsg->p_cObjectValue)));
				break;
			case MINIFT_OID_DRILL_DIR:
				g_ConfigData.iDrillDir = ntohs(*((int*) (p_STPrxMsg->p_cObjectValue)));
				break;
			case MINIFT_OID_KHOLE_MAX_DISTANCE_ERROR:
				f = *(float *) p_STPrxMsg->p_cObjectValue;
				g_ConfigData.fMaxKholeDistanceError = f;
				break;
			case MINIFT_OID_APPROX_LOCATION_ERROR:
				f = *(float *) p_STPrxMsg->p_cObjectValue;
				g_ConfigData.fApproxLocationError = f;
				break;
			case MINIFT_OID_PROBE:
				p_oid_probe = (td_oid_probe *) p_STPrxMsg->p_cObjectValue;
				ccode = p_oid_probe->ccode;
				c = p_oid_probe->cKIndex;
				if (ccode == PC_CLEAR)
				{
					if (c > g_cKHoleCount)
					{
						//bad value
						SmartToolMsgMiniFtMessageCode(p_STPSession,
								MINIFT_OID_PROBE, MINIFTMC_INVALID_KI);
						break;
					}
					if (c == 0)
					{
						ResetProbeValues();
						RecalculateLocatingDirectives(0);
						SendProbeValues(); //SPSALL
						//ClearGolbalRotation(); //just clear two variables
						g_cRotationKP = 0;
						g_cRotationKS = 0;
						g_cRotationContext = 0;
					}
					else
					{
						//Not Currently Implemented:  Removed because of lack of use.
						//ResetProbeValue(c);
						////Adjust Current Rotation Displays based on probe status.
						//ProbeModeSelectAndSetGlobalRotation();
					}
					//FIXME2 should this alert the clear???? Perhaps
					break;
				}
				//Only Accept the Probe Commands while in Probe
				if (g_cModeState == MODE_PROBE)
				{
					//Store the command
					g_cKHolePrbeCommandInput = ccode;
					//Store the given probe KIndex
					g_cKHolePrbeIndexInput = c;
					//Assuming that it's waiting for a trigger
					g_cKHolePrbeTrigger = 1; //trigger
					if (ccode == PC_STOP)
					{
						//Since this can take effect while In a probe operation, go ahead and set the command now.
						g_cKHolePrbeCommand = g_cKHolePrbeCommandInput;
						//Clear the Trigger, since it doesn't require a trigger to take effect.
						g_cKHolePrbeTrigger = 0;
					}
				}
				else if (g_cModeState == MODE_TEACH)
				{
					//Use This OID to probe teach position...
					//Sender should send PC_ADD or PC_DELETE
					if (ccode == PC_ADD)
					{
						g_Probe.cTeachCaptureTrig = TRUE;
					}
					else if (ccode == PC_DELETE)
					{
						DeletePosition();
					}
					else if (ccode == PC_COMPLETE) //Code shared with normal probe system
					{
						CompleteTeachPosition();
					}
				}
				//FIXME55555555555
				//FIXME2 concerned that STOP will come, but it won't be in PROBE.. how will it stop...
				//FIXME2 bad place to bring this up, but how dose probe mode exit anyway?????
				//Can they just do posn?????
				//What about exit command?
				//If they just leave, what if XY not done calc????
				//Use Flag?????
				break;
			case MINIFT_OID_PROBE_POSITION:
				p_oid_probe_position = (td_oid_probe_position *) p_STPrxMsg->p_cObjectValue;
				//FIXME2 consider better record and relative adjust system, but for now... do it this way.
#ifdef OUTPUT_PROBE_SYS
				logf("SetPos %f,%f\r\n", p_oid_probe_position->fX, p_oid_probe_position->fY);
#endif
//FIXME PORTMED  Is this used??????  I don't really want to do this this way
//				SetPosition(p_oid_probe_position->fX, p_oid_probe_position->fY);
				logf("ERROR: SetPos not supported.\r\n");
				break;
			case MINIFT_OID_PROBE_STATUS:
				p_oid_probe_status =
						(td_oid_probe_status *) p_STPrxMsg->p_cObjectValue;
				c = p_oid_probe_status->cKIndex;
				if (c == 0 || c > g_cKHoleCount)
				{
					//bad value
#ifdef OUTPUT_PROBE_SYS
					logf("S PS c\r\n"); //S PS but no ks
#endif
					SmartToolMsgMiniFtMessageCode(p_STPSession,
							MINIFT_OID_PROBE_STATUS, MINIFTMC_INVALID_KI);
					break;
				}
				if (p_oid_probe_status->cStatus >= PS_PENDING_ACCEPTANCE)
				{
					//Only can set something to >=PS_PENDING_ACCEPTANCE if it was allready >=PS_PENDING_ACCEPTANCE
					if (g_cKHolePrbeStatus[c] < PS_PENDING_ACCEPTANCE)
					{
						//may not do this...
						break;
					}
				}
				if (p_oid_probe_status->fX >= -90000) //if less, then we should preserve the values.
				{
					//Update Probe Values
					g_fKHolePrbeX[c] = p_oid_probe_status->fX;
					g_fKHolePrbeY[c] = p_oid_probe_status->fY;
				}
				else
				{
					p_oid_probe_status->fX = g_fKHolePrbeX[c]; //instead restore them before ECHO
					p_oid_probe_status->fY = g_fKHolePrbeY[c];
				}
				g_cKHolePrbeStatus[c] = p_oid_probe_status->cStatus;
				if (p_oid_probe_status->cStatus == PS_APPROXIMATE)
				{
//ProbeStart Test
					//Update Probe Start Also
					g_cKHolePrbeStart[c] = p_oid_probe_status->cStatus;
					g_fKHolePrbeStartX[c] = p_oid_probe_status->fX; //These are always set for this kind of status
					g_fKHolePrbeStartY[c] = p_oid_probe_status->fY;
					//1st send start
					SendProbeStart(STP_ALERT, c); //SPS
				}
#ifdef OUTPUT_PROBE_SYS
				logf("S PS K%d %f,%f %d\r\n", c, p_oid_probe_status->fX, p_oid_probe_status->fY, p_oid_probe_status->cStatus);
#endif
				if (p_oid_probe_status->cStatus < PS_PROBED)
				{
					g_cProbeComplete = 0; //If a single K Hole is not complete, then probe is not complete
					g_cProbeCalculated = 0;
				}
				SmartToolMsg(0, STP_ALERT, MINIFT_OID_PROBE_STATUS,
						sizeof(td_oid_probe_status),
						(char *) p_oid_probe_status); //SPS
				//Adjust Current Rotation Displays based on probe status.
//marker no limit here
				ProbeModeSelectAndSetGlobalRotation();

				//This might be a status update which could complete probing.
				if (p_oid_probe_status->cStatus >= PS_PROBED)
				{
					//Recalculate Positions During Probe
					RecalculatePositionsDuringProbe(c);                    //SPS
//marker continue ext
					AlertProbeStatusUpdate();
					//see if Probe is complete by checking all status
					CheckProbeComplete();
					//If this completes a probe, and they want to complete probe as soon as possible, then, trigger a probe compelte here
					if (g_cModeState == MODE_PROBE)
					{
						if (g_cProbeComplete == 1)
						{
							if (g_ConfigData.cProbeFlags & AUTO_COMPLETE)
							{
								//Using the main system, complete the probe
								//I did this to make sure it would be in sync with any other probe operations
								//Store the command
								g_cKHolePrbeCommandInput = PC_COMPLETE;
								//Store the given probe KIndex
								g_cKHolePrbeIndexInput = 1;
								//Assuming that it's waiting for a trigger
								g_cKHolePrbeTrigger = 1; //trigger
							}
						}
						else
						{
							if (g_ConfigData.cProbeFlags & AUTO_MOVE_PROBE)
							{
								c++;
								if (c <= g_cKHoleCount)
									if (g_cKHolePrbeStatus[c] < PS_PROBED)
									{
										//next is not probed.
										g_cKHolePrbeCommandInput =
												PC_MOVE_PROBE;
										//Store the probe KIndex
										g_cKHolePrbeIndexInput = c;
										//Assuming that it's waiting for a trigger
										g_cKHolePrbeTrigger = 1; //trigger
									}
							}
						}
					}
				}
				else
				{
					if (g_cModeState == MODE_PROBE)
					{
						//anything depending on this positon should not be probed if this was updated to not be probed.
						ClearPositionsDuringProbe(c); //SPS
					}
					AlertProbeStatusUpdate();
				}
				//FIXME6666666666666666... disallow this when probe has been done, or before in probe
				//This item is really about the pattern of probe selection etc...

				//FIXME0000000000000000000000000 What is the rule about
				// setting this after probe is over?????
				//IF probe was complete, it may no longer be complete, but what do I do then.
				//Should it only be settable in probe mode????????????
				break;
			case MINIFT_OID_PROBE_ACCEPT_REQUIRED:
				g_ConfigData.cProbeAcceptRequired =
						p_STPrxMsg->p_cObjectValue[0];
				break;
			case MINIFT_OID_HOME:
				if (g_cAction > ACTION_READY && g_cAction != ACTION_HOME)
				{
					//logf("NOH\r\n");
					SmartToolMsgMiniFtMessageCode(p_STPSession, MINIFT_OID_HOME,
							MINIFTMC_HOME_PREVENTED);
					break;
				}
#ifndef HOMESYSTEM
				g_Probe.cHomeTrig = TRUE;
#else
				//FIXME SEVERE  must prevent this when not authorized to do it......
				//
//FIXME PORTHIGH in the old code, for a long time, this if was broken to ALAWAYS go... retest this area
				if (g_cHomed != HOME_RUNNING)
				{
					//First see if they are doing a special homing command
					ui = p_STPrxMsg->uiValueLength;
					c = p_STPrxMsg->p_cObjectValue[1]; //second character might be an axis specified
					if (ui < 2 || c == 0)
					{
						//Do the standard full homing
						//FIXME Action system improvement ideas
						g_cAction = ACTION_HOME;
						g_cHomed = HOME_RUNNING;
						//ALLOW ALL TO RESTART LIKE THIS
						SetAllHomeStatusPending();
					}
					else if (c == AXIS_X)
					{
						//FIXME FUTURE:  Implement this as way to trigger x
					}
					else if (c == AXIS_Y)
					{
						//special command
						g_cHomed = HOME_RUNNING;
						g_cHomedY = HOME_PENDING;
					}
					ClearSentHomeStatus(); //forces resend
				}
#endif
				break;
			case MINIFT_OID_HOME_SPEED:
				p_HomeSpeed = (td_HomeSpeed *) p_STPrxMsg->p_cObjectValue;
				g_ConfigData.HomeSpeed.fX = p_HomeSpeed->fX;
				g_ConfigData.HomeSpeed.fY = p_HomeSpeed->fY;
				MCClearMotionSet();
				break;
			case MINIFT_OID_HOME_ACCEL:
				p_HomeAcc = (td_HomeAcc *) p_STPrxMsg->p_cObjectValue;
				g_ConfigData.HomeAcc.fX = p_HomeAcc->fX;
				g_ConfigData.HomeAcc.fY = p_HomeAcc->fY;
				MCClearMotionSet();
				break;
			case MINIFT_OID_HOME_DECEL:
				p_HomeDec = (td_HomeDec *) p_STPrxMsg->p_cObjectValue;
				g_ConfigData.HomeDec.fX = p_HomeDec->fX;
				g_ConfigData.HomeDec.fY = p_HomeDec->fY;
				MCClearMotionSet();
				break;
			case MINIFT_OID_HOME_MOVEDIST:
				p_HomeMoveDist = (td_HomeMoveDist *) p_STPrxMsg->p_cObjectValue;
				g_ConfigData.HomeMoveDist.fX = p_HomeMoveDist->fX;
				g_ConfigData.HomeMoveDist.fY = p_HomeMoveDist->fY;
				break;
			case MINIFT_OID_PROBE_ADJUST:
				g_Probe.cProbeAdjustTrig = TRUE;
				break;
			case MINIFT_OID_PROBE_ADJUST_LIMIT:
				f = *(float *) p_STPrxMsg->p_cObjectValue;
				g_ConfigData.fProbeAdjustLimit = f;
				break;
			case MINIFT_OID_HOME_FINE_SPEED:
				p_HomeFineSpeed =
						(td_HomeFineSpeed *) p_STPrxMsg->p_cObjectValue;
				g_ConfigData.HomeFineSpeed.fX = p_HomeFineSpeed->fX;
				g_ConfigData.HomeFineSpeed.fY = p_HomeFineSpeed->fY;
				break;
			case MINIFT_OID_POSNMODE_MOVETONEXT:
				//logf("MN\r\n");
				if (MoveAllowed(uiOID) == 0)
				{
					//logf("MA=0\r\n");
					break;
				}
				SpecifyGotoPosn(GOTOPOSN_NEXT, 0);
				g_cAutoMove = 0; //turn off auto move if anything requested the move directly
				break;
			case MINIFT_OID_POSNMODE_MOVETOPREV:
				if (MoveAllowed(uiOID) == 0)
				{
					break;
				}
				SpecifyGotoPosn(GOTOPOSN_PREV, 0);
				g_cAutoMove = 0; //turn off auto move if anything requested the move directly
				break;
			case MINIFT_OID_POSNMODE_MOVETOIND:
				if (MoveAllowed(uiOID) == 0)
				{
					break;
				}
				iTemp = ntohs(*((int*) (p_STPrxMsg->p_cObjectValue)));
				SpecifyGotoPosn(GOTOPOSN_RANDOM_INDEX, iTemp);
				g_cAutoMove = 0; //turn off auto move if anything requested the move directly
				break;
			case MINIFT_OID_POSNMODE_MOVEAGAIN:
				if (MoveAllowed(uiOID) == 0)
				{
					break;
				}
				SpecifyGotoPosnAgain();
				g_cAutoMove = 0; //turn off auto move if anything requested the move directly
				break;
			case MINIFT_OID_POSNMODE_MOVETYPE:
				g_ConfigData.cMoveType = p_STPrxMsg->p_cObjectValue[0];
				SendActivePremove(p_STPSession, STP_ALERT);
				break;
			case MINIFT_OID_POSNMODE_PREMOVEXY:
				p_PreMove = (td_PreMove *) p_STPrxMsg->p_cObjectValue;
				g_ConfigData.PreMove.fX = p_PreMove->fX;
				g_ConfigData.PreMove.fY = p_PreMove->fY;
				SendActivePremove(p_STPSession, STP_ALERT);
				break;
			case MINIFT_OID_POSNMODE_SPEED:
				p_PosnSpeed = (td_PosnSpeed *) p_STPrxMsg->p_cObjectValue;
				g_ConfigData.PosnSpeed.fX = p_PosnSpeed->fX;
				g_ConfigData.PosnSpeed.fY = p_PosnSpeed->fY;
				MCClearMotionSet();
				break;
			case MINIFT_OID_POSNMODE_ACCEL:
				p_PosnAcc = (td_PosnAcc *) p_STPrxMsg->p_cObjectValue;
				g_ConfigData.PosnAcc.fX = p_PosnAcc->fX;
				g_ConfigData.PosnAcc.fY = p_PosnAcc->fY;
				MCClearMotionSet();
				break;
			case MINIFT_OID_POSNMODE_DECEL:
				p_PosnDec = (td_PosnDec *) p_STPrxMsg->p_cObjectValue;
				g_ConfigData.PosnDec.fX = p_PosnDec->fX;
				g_ConfigData.PosnDec.fY = p_PosnDec->fY;
				MCClearMotionSet();
				break;
			case MINIFT_OID_POSNMODE_FINALSPEED:
				p_PosnFinalSpeed =
						(td_PosnFinalSpeed *) p_STPrxMsg->p_cObjectValue;
				g_ConfigData.PosnFinalSpeed.fX = p_PosnFinalSpeed->fX;
				g_ConfigData.PosnFinalSpeed.fY = p_PosnFinalSpeed->fY;
				break;
			case MINIFT_OID_ORTHO_SLOPE:
				f = *(float *) p_STPrxMsg->p_cObjectValue;
				g_ConfigData.fOrthoSlope = f;
				break;
			case MINIFT_OID_POSNERR_LIMIT:
				p_PosnErrLimit = (td_PosnErrLimit *) p_STPrxMsg->p_cObjectValue;
				g_ConfigData.PosnErrLimit.fX = p_PosnErrLimit->fX;
				g_ConfigData.PosnErrLimit.fY = p_PosnErrLimit->fY;
				MCSetErrorLimits();
				break;
			case MINIFT_OID_POSNMODE_TOLERANCE:
				f = *(float *) p_STPrxMsg->p_cObjectValue;
				g_ConfigData.fPosnTolerance = f;
				break;
			case MINIFT_OID_VELERR_LIMIT:
				p_VelErrLimit = (td_VelErrLimit *) p_STPrxMsg->p_cObjectValue;
				g_ConfigData.VelErrLimit.fVLimitMarginX =
						p_VelErrLimit->fVLimitMarginX;
				g_ConfigData.VelErrLimit.fVErrorX = p_VelErrLimit->fVErrorX;
				g_ConfigData.VelErrLimit.fVLimitMarginY =
						p_VelErrLimit->fVLimitMarginY;
				g_ConfigData.VelErrLimit.fVErrorY = p_VelErrLimit->fVErrorY;
				MCSetErrorLimits();
				break;
			case MINIFT_OID_LONG_DISTANCE:
				p_LongDistance = (td_LongDistance *) p_STPrxMsg->p_cObjectValue;
				g_ConfigData.LongDistance.fX = p_LongDistance->fX;
				g_ConfigData.LongDistance.fY = p_LongDistance->fY;
				break;
			case MINIFT_OID_LONG_SPEED:
				p_LongSpeed = (td_LongSpeed *) p_STPrxMsg->p_cObjectValue;
				g_ConfigData.LongSpeed.fX = p_LongSpeed->fX;
				g_ConfigData.LongSpeed.fY = p_LongSpeed->fY;
				break;
			case MINIFT_OID_OP_STARTED:
				//FIXME0000 if old DF was replaced by new style, then much could could be removed..
				ui = (int16) ntohs(*((int16*) (p_STPrxMsg->p_cObjectValue)));
				logf("*oid\r\n");
				AddOpHistory(g_PosnMode.iCurPosnIndex, ui);
#ifdef OUTPUT_OPERATIONS
				logf("OP_S+=%u\r\n", ui);
#endif
				break;
			case MINIFT_OID_OP_HISTORY:
				ui = p_STPrxMsg->uiValueLength;
				if (ui == 2)
				{
					//Set for current position
					i = g_PosnMode.iCurPosnIndex;
				}
				else if (ui == 4)
				{
					//Set specified position
					i = (uint16) ntohs(
							*((int16*) (p_STPrxMsg->p_cObjectValue)));
					ui = (uint16) ntohs(
							*((uint16*) (p_STPrxMsg->p_cObjectValue + 2)));
				}
				SetOpHistory(i, ui);
				break;
				//	 FIXME Future Enhancement array gets
				//		FIXME: Can SmartToolMsg() handle an arbitrariy large point array?
				//case MINIFT_OID_OP_FULLHISTORY:
				//	//	Send to the pendant the operation history array for all holes in the part program.
				//    p_c = (char *) ( g_PartPgmInfo.xp_cOpHistory );
				//	SmartToolMsg( STP_ALERT, uiOID, g_PartPgmInfo.iNumDataPoints, p_c );
				//    break;
				//
			case MINIFT_OID_DRILL_HOLE_ONE_TIME:
				g_ConfigData.cDrillHoleOneTime = p_STPrxMsg->p_cObjectValue[0];
				if (g_ConfigData.cDrillHoleOneTime == 2)
				{
					g_ConfigData.cDrillHoleOneTime = 1;
					ClearOpHistory();
				}
				break;
			case MINIFT_OID_AUTOMOVE:
				g_cAutoMove = p_STPrxMsg->p_cObjectValue[0];
				g_uiStartAutoTime = MS_TIMER;
				break;
			case MINIFT_OID_AUTOMOVE_DELAY:
				g_uiAutoTime = ntohl(*((int32*) (p_STPrxMsg->p_cObjectValue)));
				break;
			case MINIFT_OID_AUTOREPEAT:
				g_cAutoRepeat = p_STPrxMsg->p_cObjectValue[0];
				break;
			case MINIFT_OID_MACHINE_OFFSET:
				p_MachineOffset =
						(td_MachineOffset *) p_STPrxMsg->p_cObjectValue;
				g_MachineOffset.fX = p_MachineOffset->fX;
				g_MachineOffset.fY = p_MachineOffset->fY;
				break;
			case MINIFT_OID_MACHINE_OFFSET_CADJ:
				g_cMachineOffsetCompensationAdjustment =
						p_STPrxMsg->p_cObjectValue[0];
				break;
			case MINIFT_OID_MACHINE_OFFSET1:
				p_MachineOffset1 =
						(td_MachineOffset1 *) p_STPrxMsg->p_cObjectValue;
				g_MachineOffset1.fX = p_MachineOffset1->fX;
				g_MachineOffset1.fY = p_MachineOffset1->fY;
				g_MachineOffset1.fYExtension = p_MachineOffset1->fYExtension;
				break;
			case MINIFT_OID_MACHINE_OFFSET2:
				p_MachineOffset2 =
						(td_MachineOffset2 *) p_STPrxMsg->p_cObjectValue;
				g_MachineOffset2.fX = p_MachineOffset2->fX;
				g_MachineOffset2.fY = p_MachineOffset2->fY;
				g_MachineOffset2.fYExtension = p_MachineOffset2->fYExtension;
				break;
			case MINIFT_OID_STATION:
				g_cStation = p_STPrxMsg->p_cObjectValue[0];
//TEMPORARY CODE TO MAKE THIS MOVEMENT:
				if (g_cStation == STATION_LASERPOINTER)
				{
					if (MoveAllowed(uiOID) == 0)
					{
						break;
					}

					if (g_cModeState == MODE_POSN)
					{
						//Trigger move
						SpecifyGotoPosnAgain();
						g_cStationGoal = STATION_LASERPOINTER;
					}
				}
				else if (g_cStation == STATION_UNSPEC)
				{
					g_cStationGoal = STATION_UNSPEC;
				}
				break;
			case MINIFT_OID_TOOL_OFFSET:
				p_oid_tool_offset =
						(td_oid_tool_offset *) p_STPrxMsg->p_cObjectValue;
				//logf("TOOL_OFFSET rxd\r\n");
				SetToolOffset(p_oid_tool_offset->fX, p_oid_tool_offset->fY);
				ResetNearestPosition();
				break;
			case MINIFT_OID_TOOL_FLIP:
				#ifndef ORIENTATION_PERMANENT
				g_ConfigData.cToolFlip = p_STPrxMsg->p_cObjectValue[0];
#ifdef OUTPUT_TOOL_FLIP
				logf("Flp=%d\r\n",g_ConfigData.cToolFlip);
#endif
#ifdef HOMESYSTEM_X_LINK
				g_cHomed = HOME_NOT_DONE;	//overall system home status
				g_cHomedX=0;
#endif
#ifdef HOMESYSTEM_Y_LINK
				g_cHomed = HOME_NOT_DONE;	//overall system home status
				g_cHomedY = 0;
#endif
#endif
				break;
			case MINIFT_OID_DRIVE_THROUGH_BACKLASH:
				ui = p_STPrxMsg->p_cObjectValue[0];
				if (ui != 1)
				{
					ui = 0;
				}
				g_ConfigData.cDriveThroughBacklash = ui;
#ifdef USE_HYSTERESIS_FROM_CENTERVISION
				SetDriveThroughBacklash(g_ConfigData.cDriveThroughBacklash,g_fCenterVisionHysX/2.0,g_fCenterVisionHysY/2.0);
#endif
				break;
			case MINIFT_OID_DRILL_OFFSET1:
				p_DrillOffset1 = (td_DrillOffset1 *) p_STPrxMsg->p_cObjectValue;
				g_ConfigData.DrillOffset1.fX = p_DrillOffset1->fX;
				g_ConfigData.DrillOffset1.fY = p_DrillOffset1->fY;
				g_ConfigData.DrillOffset1.fYExtension =
						p_DrillOffset1->fYExtension;
				break;
			case MINIFT_OID_DRILL_OFFSET2:
				p_DrillOffset2 = (td_DrillOffset2 *) p_STPrxMsg->p_cObjectValue;
				g_ConfigData.DrillOffset2.fX = p_DrillOffset2->fX;
				g_ConfigData.DrillOffset2.fY = p_DrillOffset2->fY;
				g_ConfigData.DrillOffset2.fYExtension =
						p_DrillOffset2->fYExtension;
				break;
			case MINIFT_OID_OFFSET_SEAL:
				p_OffsetSeal = (td_OffsetSeal *) p_STPrxMsg->p_cObjectValue;
				g_ConfigData.OffsetSeal.fx = p_OffsetSeal->fx;
				g_ConfigData.OffsetSeal.fy = p_OffsetSeal->fy;
				break;
			case MINIFT_OID_OFFSET_FILL:
				p_OffsetFill = (td_OffsetFill *) p_STPrxMsg->p_cObjectValue;
				g_ConfigData.OffsetFill.fx = p_OffsetFill->fx;
				g_ConfigData.OffsetFill.fy = p_OffsetFill->fy;
				break;
			case MINIFT_OID_JOG:
				ui = p_STPrxMsg->uiValueLength;
				if (ui == 0)
				{
					//OID is being used to continue current jog.
#ifdef JOG_ENABLE_TIME
					g_uiJogEnableTime = MS_TIMER;
#endif
					break;
				}
				p_oid_jog = (td_oid_jog *) p_STPrxMsg->p_cObjectValue;
//FIXMEJOG FIXMETODAY: maintain a global jog flag that in one check and tell if any motion at all
				if (g_cAction > ACTION_READY)
				{
					//logf("NOJ\r\n");
					SmartToolMsgMiniFtMessageCode(p_STPSession, MINIFT_OID_JOG,
							MINIFTMC_JOG_PREVENTED);
					break;
				}
				if (g_cMoveDone != MOVEDONE_TRUE)
				{	// disallow jog unless carriage is done with previous move
					SmartToolMsgMiniFtMessageCode(p_STPSession, MINIFT_OID_JOG,
							MINIFTMC_WAIT_FOR_CARRIAGE_STOP);
					break;
				}
				if (g_cFloatStatus == FLOATSTAT_FLOAT)
				{
					SmartToolMsgMiniFtMessageCode(p_STPSession, MINIFT_OID_JOG,
							MINIFTMC_JOG_PREVENTED_BY_FLOAT);
					break;
				}
				if (g_cModeState
						== MODE_POSN && g_cPosnModeState != POSNMODE_WAITNEXTPOSN)
				{
					//looks like it's doing something in posn (may be other than movement...)
					break;
				}
#ifdef CLAMP_SYSTEM_NAC_STP
				if (g_cNACClear==0)
				{
					logf("j:nac\r\n");
					break;
				}
#endif
				//Copy Goal
				g_fJogGoalX = p_oid_jog->fX;
				g_fJogGoalY = p_oid_jog->fY;
				//Invalid Jog Values should be treated as STOP
				if (g_fJogGoalX > 1.0)
				{
					g_fJogGoalX = 0;
				}
				else if (g_fJogGoalX < -1.0)
				{
					g_fJogGoalX = 0;
				}
				if (g_fJogGoalY > 1.0)
				{
					g_fJogGoalY = 0;
				}
				else if (g_fJogGoalY < -1.0)
				{
					g_fJogGoalY = 0;
				}
				//Now Set Char Flag Values
				g_cJogGoalX = JOGSTOP;
				g_cJogGoalY = JOGSTOP;
				if (g_fJogGoalX < 0)
				{
					g_cJogGoalX = JOGNEG;
				}
				if (g_fJogGoalX > 0)
				{
					g_cJogGoalX = JOGPOS;
				}
				if (g_fJogGoalY < 0)
				{
					g_cJogGoalY = JOGNEG;
				}
				if (g_fJogGoalY > 0)
				{
					g_cJogGoalY = JOGPOS;
				}
#ifdef OUTPUT_JOG
				logf("JG%d,%d\r\n",g_cJogGoalX,g_cJogGoalY);
#endif
#ifdef JOG_ENABLE_TIME
				g_uiJogEnableTime = MS_TIMER;
#endif
				//Check to see if move would be in the direction of obstruction
				if (g_cObstructionCode != 0)
				{
					//Check Individual Sensors
#ifdef Y_LIMIT_SENSORS
					if (g_cDigInYPosLimit==0)
					{
						if (g_cJogGoalY == JOGPOS)
						{	goto handle_jog_toward_obstruction;}
					}
					if (g_cDigInYNegLimit==0)
					{
						if (g_cJogGoalY == JOGNEG)
						{	goto handle_jog_toward_obstruction;}
					}
#endif
#ifdef OBSTRUCTION_SYSTEM_XP1
					if (g_cDigInObstructionXP1 == OBSTRUCTION)
					{
						//X+
						if (g_cJogGoalX == JOGPOS)
						{
							goto handle_jog_toward_obstruction;
						}
					}
#endif
#ifdef OBSTRUCTION_SYSTEM_XN1
					if (g_cDigInObstructionXN1 == OBSTRUCTION)
					{
						//X-
						if (g_cJogGoalX == JOGNEG)
						{
							goto handle_jog_toward_obstruction;
						}
					}
#endif
#ifdef OBSTRUCTION_SYSTEM_MOS
					if (g_cDigInObstructionMOS == MO_OBSTRUCTION)
					{
						goto handle_jog_toward_obstruction;
					}
#endif
#ifdef OBSTRUCTION_SYSTEM_MO_ANA
					if (g_cMOFlags > 0)
					{
						goto handle_jog_toward_obstruction;
					}
#endif
					if (0)
					{
						handle_jog_toward_obstruction:
						//Echo Obstruction Code Again
						AlertObstructionCode(p_STPSession);
						g_cJogGoalX = JOGSTOP;
						g_cJogGoalY = JOGSTOP;
#ifdef OUTPUT_JOG
						logf("JGobs\r\n");
#endif
					}
				}
//FIXMETODAY FIXMEJOG
//consider using one char just to indicate that we want to stop or  go
				g_PosnMode.cOnCurPosn = 0;	//moving actions always mean not on position
				if (g_cModeState == MODE_POSN)
				{
					LEDOff()
					;
				}
				break;
			case MINIFT_OID_JOG_SPEED:
				p_JogSpeed = (td_JogSpeed *) p_STPrxMsg->p_cObjectValue;
				g_ConfigData.JogSpeed.fX = p_JogSpeed->fX;
				g_ConfigData.JogSpeed.fY = p_JogSpeed->fY;
				MCClearMotionSet();
				break;
			case MINIFT_OID_JOG_ACCEL:
				p_JogAcc = (td_JogAcc *) p_STPrxMsg->p_cObjectValue;
				g_ConfigData.JogAcc.fX = p_JogAcc->fX;
				g_ConfigData.JogAcc.fY = p_JogAcc->fY;
				MCClearMotionSet();
				break;
			case MINIFT_OID_JOG_DECEL:
				p_JogDec = (td_JogDec *) p_STPrxMsg->p_cObjectValue;
				g_ConfigData.JogDec.fX = p_JogDec->fX;
				g_ConfigData.JogDec.fY = p_JogDec->fY;
				MCClearMotionSet();
				break;
			case MINIFT_OID_JOG_FACTOR:
				p_JogFactor = (td_JogFactor *) p_STPrxMsg->p_cObjectValue;
				g_JogFactor.fX = p_JogFactor->fX;
				g_JogFactor.fY = p_JogFactor->fY;
				break;
			case MINIFT_OID_HOME_POSITION_Y_POS:
				p_HomePosnYPos = (td_HomePosnYPos *) p_STPrxMsg->p_cObjectValue;
				g_ConfigData.HomePosnYPos.fX = p_HomePosnYPos->fX;
				g_ConfigData.HomePosnYPos.fY = p_HomePosnYPos->fY;
				break;
			case MINIFT_OID_POSITION_LIMIT_Y_POS:
				p_PosnLimitYPos =
						(td_PosnLimitYPos *) p_STPrxMsg->p_cObjectValue;
				g_ConfigData.PosnLimitYPos.fMinX = p_PosnLimitYPos->fMinX;
				g_ConfigData.PosnLimitYPos.fMaxX = p_PosnLimitYPos->fMaxX;
				g_ConfigData.PosnLimitYPos.fMinY = p_PosnLimitYPos->fMinY;
				g_ConfigData.PosnLimitYPos.fMaxY = p_PosnLimitYPos->fMaxY;
				//Setting this OID directly will adjust the limit right away,
				//but normally the global home and limits remain as-is until before homing
				SelectHomePositionAndPositionLimits();
				MCSetPositionLimits(g_cHomed != HOME_RUNNING);
				break;
			case MINIFT_OID_HOME_POSITION_Y_NEG:
				p_HomePosnYNeg = (td_HomePosnYNeg *) p_STPrxMsg->p_cObjectValue;
				g_ConfigData.HomePosnYNeg.fX = p_HomePosnYNeg->fX;
				g_ConfigData.HomePosnYNeg.fY = p_HomePosnYNeg->fY;
				break;
			case MINIFT_OID_POSITION_LIMIT_Y_NEG:
				p_PosnLimitYNeg =
						(td_PosnLimitYNeg *) p_STPrxMsg->p_cObjectValue;
				g_ConfigData.PosnLimitYNeg.fMinX = p_PosnLimitYNeg->fMinX;
				g_ConfigData.PosnLimitYNeg.fMaxX = p_PosnLimitYNeg->fMaxX;
				g_ConfigData.PosnLimitYNeg.fMinY = p_PosnLimitYNeg->fMinY;
				g_ConfigData.PosnLimitYNeg.fMaxY = p_PosnLimitYNeg->fMaxY;
				//Setting this OID directly will adjust the limit right away.
				//but normally the global home and limits remain as-is until before homing
				SelectHomePositionAndPositionLimits();
				MCSetPositionLimits(g_cHomed != HOME_RUNNING);
				break;
			case MINIFT_OID_OBSTRUCTION_CODE_MASK:
				g_ConfigData.cObstructionCodeMask =
						p_STPrxMsg->p_cObjectValue[0];
				break;
			case MINIFT_OID_MACHINE_LOCK_REQUIRED:
				g_ConfigData.cMachineLockRequired =
						p_STPrxMsg->p_cObjectValue[0];
				break;
			case MINIFT_OID_CLAMP:
				#ifdef CLAMP_SYSTEM
				ui = p_STPrxMsg->p_cObjectValue[0];
				if (ui >= 100 && ui <= 110)
				{
					//this is a special shortcut: DSI is telling us what the clamp state is now... this
					//is required because we don't have MINIFT_OID_CLAMP_GOAL etc.. on the outside, but this hack is only temporary
					//because MiniFT will eventually talk as a client to the sub tools in all cases
					ui = ui - 100;
					g_cClampState = ui;
					break;
				}
				//set clamp goal directly.
				g_cClampGoal = p_STPrxMsg->p_cObjectValue[0];
				//Any Time it comes from the Pendant or outside source, clear what was sent to be sure it gets sent again.
				g_cClampGoalSent = 0xFF; //force it to send goal again
#endif
				break;
			case MINIFT_OID_ALOCK:
				g_cALockMode = p_STPrxMsg->p_cObjectValue[0];
				//If 1, then always lock right now, and after, the clamp logic will not touch the a lock.
				//If 0, then always unlock right now, but the clamp logic will them control ALock.
#ifdef CLAMP_SYSTEM_HD_PISTON
				if (g_cALockMode==0)
				{
					g_cALock=ALOCK_OFF;
				}
				else
				{
					g_cALock=ALOCK_ON;
				}
				g_ulClampAUnlock=MS_TIMER;
#ifdef DIGOUT_CHNUM_ALOCK
				digOut(DIGOUT_CHNUM_ALOCK, g_cALock );
#endif
				//And Echo
				SmartToolMsgChar(p_STPSession, STP_ALERT, MINIFT_OID_ALOCK, g_cALockMode);
#endif
				break;
			case MINIFT_OID_ALOCKDELAY:
				g_ConfigData.uiALockDelay = (uint16) ntohs(
						*((int16*) (p_STPrxMsg->p_cObjectValue)));
				break;
			case MINIFT_OID_AUNLOCKDELAY:
				g_ConfigData.uiAUnlockDelay = (uint16) ntohs(
						*((int16*) (p_STPrxMsg->p_cObjectValue)));
				break;
			case MINIFT_OID_LEGLOCKDELAY:
				g_ConfigData.uiLegsLockDelay = (uint16) ntohs(
						*((int16*) (p_STPrxMsg->p_cObjectValue)));
				break;
			case MINIFT_OID_LEGUNLOCKDELAY:
				g_ConfigData.uiLegsUnlockDelay = (uint16) ntohs(
						*((int16*) (p_STPrxMsg->p_cObjectValue)));
				break;
			case MINIFT_OID_LEGSUPDELAY:
				g_ConfigData.uiLegsUpDelay = (uint16) ntohs(
						*((int16*) (p_STPrxMsg->p_cObjectValue)));
				break;
			case MINIFT_OID_LEGSDOWNDELAY:
				g_ConfigData.uiLegsDownDelay = (uint16) ntohs(
						*((int16*) (p_STPrxMsg->p_cObjectValue)));
				break;
			case MINIFT_OID_LOWPRESSUREDELAY:
				g_ConfigData.uiLowPressureDelay = (uint16) ntohs(
						*((int16*) (p_STPrxMsg->p_cObjectValue)));
				break;
			case MINIFT_OID_LOWPRESSUREDOWNDELAY:
				g_ConfigData.uiLowPressureDownDelay = (uint16) ntohs(
						*((int16*) (p_STPrxMsg->p_cObjectValue)));
				break;
			case MINIFT_OID_PRESSUREDELAY:
				g_ConfigData.uiPressureDelay = (uint16) ntohs(
						*((int16*) (p_STPrxMsg->p_cObjectValue)));
				break;
			case MINIFT_OID_PRESSUREDOWNDELAY:
				g_ConfigData.uiPressureDownDelay = (uint16) ntohs(
						*((int16*) (p_STPrxMsg->p_cObjectValue)));
				break;
			case MINIFT_OID_LOWPRESSURE:
				g_ConfigData.uiLowPressure = (uint16) ntohs(
						*((int16*) (p_STPrxMsg->p_cObjectValue)));
				break;
			case MINIFT_OID_PRESSURE:
				g_ConfigData.uiPressure = (uint16) ntohs(
						*((int16*) (p_STPrxMsg->p_cObjectValue)));
				//Also, when the default pressure is set and we are clamped, go to that pressure now.
#ifdef CLAMP_SYSTEM
				g_uiClampPressure = (int) g_ConfigData.uiPressure; //make current pressure
#ifdef CLAMP_SYSTEM_HD_PISTON
						if (g_cClampState==CLAMP_CLAMP)
						{
							g_uiClampPressureLBS = (uint16) g_uiClampPressure;
							SetClampPressureLBS();
						}
#endif
				g_cClampGoalSent = 0xFF; //force it to send goal again
#endif
				break;
			case MINIFT_OID_AIR_CLEAR:
				ui = (uint16) ntohs(*((int16*) (p_STPrxMsg->p_cObjectValue)));
				if (ui == 10) //this was to avoid a certain special signal at one time... for now it remains, but could be removed if investigated
				{
					break;
				}
#ifdef AIR_CLEAR
				g_ConfigData.uiAirClear=ui;
#else
				g_ConfigData.uiAirClear = 0;
#endif
				break;
			case MINIFT_OID_LASER_SENSOR_OFFSET:
				p_LaserSensorOffset =
						(td_LaserSensorOffset *) p_STPrxMsg->p_cObjectValue;
				g_ConfigData.LaserSensorOffset.fX = p_LaserSensorOffset->fX;
				g_ConfigData.LaserSensorOffset.fY = p_LaserSensorOffset->fY;
#ifdef OUTPUT_OFFSETS
				logf("LS OFFST x=%.5f y=%.5f\r\n",g_ConfigData.LaserSensorOffset.fX,g_ConfigData.LaserSensorOffset.fY);
#endif
				break;
			case MINIFT_OID_CAM_OFFSET:
				#ifdef HD_RAIL_STP
				if (g_cRailSTP==1)
				{
					SmartToolMsgMiniFtMessageCode(p_STPSession, MINIFT_OID_HOME_RFID, MINIFTMC_INVALID_PARAMETER); //FIXME Minor  More Specific Error Message
					break;
				}
#endif
				p_CamOffset = (td_CamOffset *) p_STPrxMsg->p_cObjectValue;
				g_ConfigData.CamOffset.fX = p_CamOffset->fX;
				g_ConfigData.CamOffset.fY = p_CamOffset->fY;
				break;
			case MINIFT_OID_VISION_INSPECT:
				if (MoveAllowed(uiOID) == 0)
				{
					break;
				}
#ifdef CENTERVISION
				g_cCenterVisionInspectType = p_STPrxMsg->p_cObjectValue[0];
#endif
				g_cInspectMethod = g_ConfigData.cCommandInspectMethod;
				g_cPositionInspection = 1;
				g_cCenterVisionRequiredResults = CENTERVISION_CENTER;
				if (g_cModeState == MODE_POSN)
				{
					//Trigger move
					SpecifyGotoPosnAgain();
					g_cStationGoal = STATION_INSPECT;
					g_cStationPlanInspect = 1;
				}
				else
				{
					//Do a stand alone inspect
					g_cModeState = MODE_INSPECT;
				}
				break;
			case MINIFT_OID_LASER_SENSOR_ALG_PARAM:
				p_LaserSensorAlgParam =
						(td_LaserSensorAlgParam *) p_STPrxMsg->p_cObjectValue;
				g_ConfigData.LaserSensorAlgParam.fsearch_speed =
						p_LaserSensorAlgParam->fsearch_speed;
				g_ConfigData.LaserSensorAlgParam.fseek_speed =
						p_LaserSensorAlgParam->fseek_speed;
				g_ConfigData.LaserSensorAlgParam.frscan_speed =
						p_LaserSensorAlgParam->frscan_speed;
				g_ConfigData.LaserSensorAlgParam.frscan_speed_fast =
						p_LaserSensorAlgParam->frscan_speed_fast;
				g_ConfigData.LaserSensorAlgParam.fscan_speed =
						p_LaserSensorAlgParam->fscan_speed;
				g_ConfigData.LaserSensorAlgParam.fscan_speed_fast =
						p_LaserSensorAlgParam->fscan_speed_fast;
				g_ConfigData.LaserSensorAlgParam.fprobe_diameter =
						p_LaserSensorAlgParam->fprobe_diameter;
				g_ConfigData.LaserSensorAlgParam.funknown_diameter =
						p_LaserSensorAlgParam->funknown_diameter;
				g_ConfigData.LaserSensorAlgParam.cmode =
						p_LaserSensorAlgParam->cmode;
				g_ConfigData.LaserSensorAlgParam.cmode_fast =
						p_LaserSensorAlgParam->cmode_fast;
				g_ConfigData.LaserSensorAlgParam.cuse_avg =
						p_LaserSensorAlgParam->cuse_avg;
				g_ConfigData.LaserSensorAlgParam.cfull_scan =
						p_LaserSensorAlgParam->cfull_scan;
				g_ConfigData.LaserSensorAlgParam.cgdata_sel =
						p_LaserSensorAlgParam->cgdata_sel;
				g_ConfigData.LaserSensorAlgParam.cassume_posn =
						p_LaserSensorAlgParam->cassume_posn;
				g_ConfigData.LaserSensorAlgParam.cassume_posn_fast =
						p_LaserSensorAlgParam->cassume_posn_fast;
				g_ConfigData.LaserSensorAlgParam.crect_center =
						p_LaserSensorAlgParam->crect_center;
				g_ConfigData.LaserSensorAlgParam.cloops =
						p_LaserSensorAlgParam->cloops;
				g_ConfigData.LaserSensorAlgParam.cdelta_mode =
						p_LaserSensorAlgParam->cdelta_mode;
				g_ConfigData.LaserSensorAlgParam.idelta_flat = ntohs(
						p_LaserSensorAlgParam->idelta_flat);
				g_ConfigData.LaserSensorAlgParam.fdelta_basespan =
						p_LaserSensorAlgParam->fdelta_basespan;
				g_ConfigData.LaserSensorAlgParam.idelta_pos = ntohs(
						p_LaserSensorAlgParam->idelta_pos);
				g_ConfigData.LaserSensorAlgParam.idelta_neg = ntohs(
						p_LaserSensorAlgParam->idelta_neg);
				g_ConfigData.LaserSensorAlgParam.fdelta_span =
						p_LaserSensorAlgParam->fdelta_span;
				g_ConfigData.LaserSensorAlgParam.fdelta_edge =
						p_LaserSensorAlgParam->fdelta_edge;
				g_ConfigData.LaserSensorAlgParam.fpc_aspect_diff =
						p_LaserSensorAlgParam->fpc_aspect_diff;
				g_ConfigData.LaserSensorAlgParam.fmax_aspect_diff =
						p_LaserSensorAlgParam->fmax_aspect_diff;
				g_ConfigData.LaserSensorAlgParam.fmax_over_exp_diameter =
						p_LaserSensorAlgParam->fmax_over_exp_diameter;
				g_ConfigData.LaserSensorAlgParam.fmax_under_exp_diameter =
						p_LaserSensorAlgParam->fmax_under_exp_diameter;
				g_ConfigData.LaserSensorAlgParam.fmax_csnk_diff =
						p_LaserSensorAlgParam->fmax_csnk_diff;
				g_ConfigData.LaserSensorAlgParam.fmax_over_csnk =
						p_LaserSensorAlgParam->fmax_over_csnk;
				g_ConfigData.LaserSensorAlgParam.fmax_under_csnk =
						p_LaserSensorAlgParam->fmax_under_csnk;
				break;
			case MINIFT_OID_CAM_ALG_PARAM:
				p_CamAlgParam = (td_CamAlgParam *) p_STPrxMsg->p_cObjectValue;
				g_ConfigData.CamAlgParam.fmove_speed =
						p_CamAlgParam->fmove_speed;
				g_ConfigData.CamAlgParam.cInfoMask = p_CamAlgParam->cInfoMask;
				g_ConfigData.CamAlgParam.cAMode = p_CamAlgParam->cAMode;
				g_ConfigData.CamAlgParam.cCMode = p_CamAlgParam->cCMode;
				g_ConfigData.CamAlgParam.cAux1 = p_CamAlgParam->cAux1;
				g_ConfigData.CamAlgParam.fmove_required =
						p_CamAlgParam->fmove_required;
				g_ConfigData.CamAlgParam.fmax_over_exp_diameter =
						p_CamAlgParam->fmax_over_exp_diameter;
				g_ConfigData.CamAlgParam.fmax_under_exp_diameter =
						p_CamAlgParam->fmax_under_exp_diameter;
				g_ConfigData.CamAlgParam.fmax_csnk_diff =
						p_CamAlgParam->fmax_csnk_diff;
				g_ConfigData.CamAlgParam.fmax_over_csnk =
						p_CamAlgParam->fmax_over_csnk;
				g_ConfigData.CamAlgParam.fmax_under_csnk =
						p_CamAlgParam->fmax_under_csnk;
				break;
			case MINIFT_OID_VISION_AUTO_INSPECT:
				g_ConfigData.cVisionAutoInspect = p_STPrxMsg->p_cObjectValue[0];
				break;
			case MINIFT_OID_STOP_INTERFACE_TASK:
				StopInterfaceTasks();
				break;
			case MINIFT_OID_PROCESS_START:
				//Only let them start if it's waiting and it's on position.
				if (g_cPosnModeState == POSNMODE_WAITNEXTPOSN)
				{
					if (g_PosnMode.cOnCurPosn == 1)
					{
						g_cStartProcess = 1;
					}
				}
				break;
			case MINIFT_OID_PROCESS_STOP:
				StopProcess();
				break;
			case MINIFT_OID_PROCESS_CONTINUE_MODE:
				g_ConfigData.cProcessContinueMode =
						p_STPrxMsg->p_cObjectValue[0];
//FIXME000 new for DFINT
//Maybe Echo...
				break;
			case MINIFT_OID_PROCESS_OPERATIONS:
				g_ConfigData.uiProcessOperations = (uint16) ntohs(*((int16*) (p_STPrxMsg->p_cObjectValue)));
				ui = g_ConfigData.uiProcessOperations;
				SmartToolMsgUInt16(0, STP_ALERT, uiOID, ui);
#ifdef OUTPUT_OPERATIONS
				logf("rpop=%u\r\n", ui);
#endif
				UpdateStationPlan();
				break;
			case MINIFT_OID_DRILL_STATE:
				c = g_cDrillState;
				g_cDrillState = p_STPrxMsg->p_cObjectValue[0]; //always just trust it
#ifdef HOMESYSTEM
#ifdef HOMESYSTEM_DRILL
				if (g_cDrillState == DRILLSTATE_IDLE)
				{
					//sign that drill home has been done
					g_cDrillHomed = HOME_DONE;
					if (g_cDrillStateGoal==DRILLSTATE_HOME)
					{
						g_cDrillStateGoal = DRILLSTATE_IDLE;
					}
					c='I';
				}
				else if (g_cDrillState == DRILLSTATE_ESTOP)
				{
					//Indicate Home has not been done
					g_cDrillHomed = HOME_NOT_DONE;
					c='E';
				}
				else if (g_cDrillState == DRILLSTATE_NOT_HOME)
				{
					if (g_cDrillHomed == HOME_RUNNING && g_cDrillStateGoalSent == DRILLSTATE_HOME && g_cDrillHomeWasStarted == 1)
					{
						g_cDrillHomed = HOME_FAILURE;
					}
					c='X';
				}
				else if (g_cDrillState == DRILLSTATE_HOME) //Means Homing Now
				{
					g_cDrillHomeWasStarted = 1;
					c='H';
				}
				else
				{
					logf("*DS %d\r\n",g_cDrillState);
					goto skip_ds_print_fmt;
				}
				logf("*DS %c\r\n",c);
				skip_ds_print_fmt:
#endif
#endif
				break;
			case MINIFT_OID_DRILL_EXPLANATION:
				g_cDrillExplanation = p_STPrxMsg->p_cObjectValue[0];
				break;
			case MINIFT_OID_SEAL_STATE:
				g_cSealState = p_STPrxMsg->p_cObjectValue[0];
				break;
			case MINIFT_OID_SEAL_CLAMP:
				g_ConfigData.cSealClamp = p_STPrxMsg->p_cObjectValue[0];
				break;
			case MINIFT_OID_SEAL_PRESSURE_DELAY:
				g_ConfigData.iSealPressureDelay = ntohs(
						*((int16*) (p_STPrxMsg->p_cObjectValue)));
				break;
			case MINIFT_OID_SEAL_PRESSURE_RELEASE_DELAY:
				g_ConfigData.iSealPressureReleaseDelay = ntohs(
						*((int16*) (p_STPrxMsg->p_cObjectValue)));
				break;
			case MINIFT_OID_SEAL_PRIME_DELAY:
				p_PrimeDelay = (td_PrimeDelay *) p_STPrxMsg->p_cObjectValue;
				g_ConfigData.PrimeDelay.fdiameter1 = p_PrimeDelay->fdiameter1;
				g_ConfigData.PrimeDelay.idelay1 = ntohs(p_PrimeDelay->idelay1);
				g_ConfigData.PrimeDelay.fdiameter2 = p_PrimeDelay->fdiameter2;
				g_ConfigData.PrimeDelay.idelay2 = ntohs(p_PrimeDelay->idelay2);
				g_ConfigData.PrimeDelay.fdiameter3 = p_PrimeDelay->fdiameter3;
				g_ConfigData.PrimeDelay.idelay3 = ntohs(p_PrimeDelay->idelay3);
				g_ConfigData.PrimeDelay.fdiameter4 = p_PrimeDelay->fdiameter4;
				g_ConfigData.PrimeDelay.idelay4 = ntohs(p_PrimeDelay->idelay4);
				g_ConfigData.PrimeDelay.fdiameter5 = p_PrimeDelay->fdiameter5;
				g_ConfigData.PrimeDelay.idelay5 = ntohs(p_PrimeDelay->idelay5);
				break;
			case MINIFT_OID_SEAL_GLOB_DELAY:
				g_ConfigData.iSealGlobDelay = ntohs(
						*((int16*) (p_STPrxMsg->p_cObjectValue)));
				break;
			case MINIFT_OID_SEAL_APPLY_DELAY:
				g_ConfigData.iSealApplyDelay = ntohs(
						*((int16*) (p_STPrxMsg->p_cObjectValue)));
				break;
			case MINIFT_OID_FILL_STATE:
				g_cFillState = p_STPrxMsg->p_cObjectValue[0];
				break;
			case MINIFT_OID_FILL_CLAMP:
				g_ConfigData.cFillClamp_ = p_STPrxMsg->p_cObjectValue[0];
				break;
			case MINIFT_OID_FILL_EXTEND_DELAY:
				g_ConfigData.iFillExtendDelay_ = ntohs(
						*((int16*) (p_STPrxMsg->p_cObjectValue)));
				break;
			case MINIFT_OID_FILL_RAM_DELAY:
				g_ConfigData.iFillRamDelay_ = ntohs(
						*((int16*) (p_STPrxMsg->p_cObjectValue)));
				break;
			case MINIFT_OID_FASTENER_REQUEST:
				//not setable
				break;
			case MINIFT_OID_FASTENER_ARRIVED:
				g_cFastenerArrived = p_STPrxMsg->p_cObjectValue[0];
				break;
			case MINIFT_OID_FASTENER_POST_DELAY:
				g_ConfigData.iFastenerPostDelay_ = ntohs(
						*((int16*) (p_STPrxMsg->p_cObjectValue)));
				break;
			case MINIFT_OID_ACCEL_ARM:
				g_cAccelArm_ = p_STPrxMsg->p_cObjectValue[0];
				break;
			case MINIFT_OID_ACCEL_TRIGGER:
				g_cAccelTrigger_ = p_STPrxMsg->p_cObjectValue[0];
				break;
			case MINIFT_OID_TOOL_Z_BASE:
				f = *(float *) p_STPrxMsg->p_cObjectValue;
				g_ConfigData.fToolZBase = f;
				break;
			case MINIFT_OID_MOVEXY:
				//FIXME Minor  Not Implemented Yet
				//	p_oid_movexy=(td_oid_movexy *)p_STPrxMsg->p_cObjectValue;
				//	p_oid_movexy->fMachineX=p_oid_movexy->fMachineX;
				//	p_oid_movexy->fMachineY=p_oid_movexy->fMachineY;
				break;
			case MINIFT_OID_POSN_DISPLAY:
				p_PosnDisplay = (td_PosnDisplay *) p_STPrxMsg->p_cObjectValue;
				g_ConfigData.PosnDisplay.cmode = p_PosnDisplay->cmode;
				g_ConfigData.PosnDisplay.corigin = p_PosnDisplay->corigin;
				g_ConfigData.PosnDisplay.ccontent = p_PosnDisplay->ccontent;
				break;
			case MINIFT_OID_RFID_CONFIG:
				p_RFIDConfig = (td_RFIDConfig *) p_STPrxMsg->p_cObjectValue;
				g_ConfigData.RFIDConfig.cenabled = p_RFIDConfig->cenabled;
				g_ConfigData.RFIDConfig.cmethod = p_RFIDConfig->cmethod;
				g_ConfigData.RFIDConfig.uioptions = ntohs(
						p_RFIDConfig->uioptions);
				g_ConfigData.RFIDConfig.uicontinuousReadCycleTime = ntohs(
						p_RFIDConfig->uicontinuousReadCycleTime);
				g_ConfigData.RFIDConfig.uiseekReadCycleTime = ntohs(
						p_RFIDConfig->uiseekReadCycleTime);
				g_ConfigData.RFIDConfig.fseekMove1 = p_RFIDConfig->fseekMove1;
				g_ConfigData.RFIDConfig.fseekMove2 = p_RFIDConfig->fseekMove2;
				g_ConfigData.RFIDConfig.fseekFineMove =
						p_RFIDConfig->fseekFineMove;
				g_ConfigData.RFIDConfig.fseekSpeed = p_RFIDConfig->fseekSpeed;
				g_ConfigData.RFIDConfig.fseekFineSpeed =
						p_RFIDConfig->fseekFineSpeed;
				g_ConfigData.RFIDConfig.fRFIDOffset = p_RFIDConfig->fRFIDOffset;
				g_ConfigData.RFIDConfig.fseekPastBorder =
						p_RFIDConfig->fseekPastBorder;
				g_ConfigData.RFIDConfig.fminWindowSize =
						p_RFIDConfig->fminWindowSize;
				g_ConfigData.RFIDConfig.fmaxWindowSize =
						p_RFIDConfig->fmaxWindowSize;
				break;
			case MINIFT_OID_RFID_DATA:
				p_RFIDData = (td_RFIDData *) p_STPrxMsg->p_cObjectValue;
				g_RFIDData.cstate = p_RFIDData->cstate;
				g_RFIDData.ccontext = p_RFIDData->ccontext;
				g_RFIDData.cseekstate = p_RFIDData->cseekstate;
				g_RFIDData.ultimestamp = ntohl(p_RFIDData->ultimestamp);
				g_RFIDData.ulrfidtimestamp = ntohl(p_RFIDData->ulrfidtimestamp);
				g_RFIDData.fposition = p_RFIDData->fposition;
				//barray field sztagdata
				g_RFIDData.uicrc16 = ntohs(p_RFIDData->uicrc16);
				g_RFIDData.uiendcode = ntohs(p_RFIDData->uiendcode);
				g_RFIDData.ulseektime = ntohl(p_RFIDData->ulseektime);
				g_RFIDData.fsstart = p_RFIDData->fsstart;
				g_RFIDData.fpstart = p_RFIDData->fpstart;
				g_RFIDData.fpend = p_RFIDData->fpend;
				g_RFIDData.fnstart = p_RFIDData->fnstart;
				g_RFIDData.fnend = p_RFIDData->fnend;
				g_RFIDData.fhs1 = p_RFIDData->fhs1;
				g_RFIDData.fhs2 = p_RFIDData->fhs2;
				g_RFIDData.fhsf = p_RFIDData->fhsf;
				//Read BStrings and/or BArrays after fixed size fields
				p_c = ((char *) p_RFIDData) + sizeof(td_RFIDData);
				ReadBArray(p_c, g_szTagDatalen, g_szTagData, 64)
				;
				//now alert this back out
				SendRFIDData(0, STP_ALERT);
				break;

#if 1
			case MINIFT_OID_HOME_RFID:
				#ifdef HOMESYSTEM_X
#ifdef HOMESYSTEM_X_RFID
				if (g_cAction == ACTION_HOME && g_cHomed == HOME_RUNNING)
				{
					//special case.... allow reqeust of RFID Home
				}
				else if (g_cAction > ACTION_READY)
				{
					//logf("NOG\r\n");
					SmartToolMsgMiniFtMessageCode(p_STPSession,
							MINIFT_OID_HOME_RFID,
							MINIFTMC_RFID_HOME_PREVENTED_ACTION);
					break;
				}
				if (g_cGravCompStatus == GRAVCOMP_RUNNING) //should be covered by action, but may not be
				{
					SmartToolMsgMiniFtMessageCode(p_STPSession,
							MINIFT_OID_HOME_RFID, MINIFTMC_RFID_HOME_PREVENTED);
					break;
				}
				if (g_cModeState != MODE_IDLE)
				{
					SmartToolMsgMiniFtMessageCode(p_STPSession,
							MINIFT_OID_HOME_RFID,
							MINIFTMC_RFID_HOME_PREVENTED_MODE);
					break;
				}
				if (g_cObstructionCode != 0)
				{
					AlertObstructionCode(p_STPSession);
					break;
				}
				if (g_cAction == ACTION_HOME && g_cHomed == HOME_RUNNING)
				{
					//special case.... allow reqeust of RFID Home... no need for these 3 checks here
				}
				else
				{
					if (g_cMoveDone != MOVEDONE_TRUE)
					{	// disallow home unless carriage is done with previous move
						SmartToolMsgMiniFtMessageCode(p_STPSession,
								MINIFT_OID_HOME_RFID,
								MINIFTMC_RFID_HOME_PREVENTED_MOVING);
						break;
					}
					if (g_cDrillState != DRILLSTATE_IDLE)
					{
						SmartToolMsgMiniFtMessageCode(p_STPSession,
								MINIFT_OID_HOME_RFID,
								MINIFTMC_RFID_HOME_PREVENTED_DRILL);
						logf("%d\r\n", g_cDrillState);
//try alert of drill state???
						break;
					}
					if (g_cClampState
							!= g_cClampGoal || g_cClampState != CLAMP_UNCLAMP)
					{
						SmartToolMsgMiniFtMessageCode(p_STPSession,
								MINIFT_OID_HOME_RFID,
								MINIFTMC_RFID_HOME_PREVENTED_CLAMP);
						//FIXME MEDHIGH switch to cause related messages
						break;
					}
				}
				//Main Home Code will operate and start any pending home components
				g_cHomed = HOME_RUNNING;
				g_cHomedX = HOME_PENDING;
#endif
#endif
				break;
			case MINIFT_OID_READ_RFID:
				g_cReadRFID = p_STPrxMsg->p_cObjectValue[0];
#ifdef USE_RFID_OMRON
				g_cTagReadState = RFID_TAG_CLEAR;
#endif
				break;
			case MINIFT_OID_HOME_STOP:
				logf("homestop\r\n");
#ifdef HOMESYSTEM_X_MC
				if (g_cHomedX == HOME_RUNNING || g_cHomedX == HOME_PENDING)
				{
					g_cHomedX = HOME_NOT_DONE;
					StopPosition();
				}
#endif
#ifdef HOMESYSTEM_X_RFID
				if (g_cHomedX == HOME_RUNNING || g_cHomedX == HOME_PENDING)
				{
					g_cHomedX = HOME_NOT_DONE;
					MCStopPosition();
					if (g_RFIDData.cseekstate < RFID_SEEK_TERMINAL_STATE)
					{
						g_RFIDData.cseekstate = RFID_SEEK_NULL;
					}
				}
#endif
#ifdef HOMESYSTEM_Y_MC
				if (g_cHomedY == HOME_RUNNING || g_cHomedY == HOME_PENDING)
				{
					g_cHomedY = HOME_NOT_DONE;
					MCStopPosition();
				}
#endif
#ifdef HOMESYSTEM_DRILL
				if (g_cDrillHomed == HOME_RUNNING || g_cDrillHomed == HOME_PENDING)
				{
					g_cDrillHomed = HOME_NOT_DONE;
					g_cDrillStateGoal = DRILLSTATE_IDLE;
					g_cDrillHomeWasStarted = 0; //clear this to prevent mode rx from turning this to failure
					logf("d s\r\n");
				}
				else
				{
					logf("d nr\r\n");
				}
#endif
				c = 0;
#ifdef HOMESYSTEM_CLAMP
				if (g_cNACClampHomed == HOME_RUNNING || g_cNACClampHomed == HOME_PENDING)
				{
					//Send Home Stop to NAC
					c=1;
					g_cNACClampHomed = HOME_NOT_DONE;
				}
#endif
#ifdef HOMESYSTEM_AAXIS
				if (g_cNACAAxisHomed == HOME_RUNNING || g_cNACAAxisHomed == HOME_PENDING)
				{
					//Send Home Stop to NAC
					c=1;
					g_cNACAAxisHomed = HOME_NOT_DONE;
				}
#endif
#ifdef HOMESYSTEM_AAXIS_OR_CLAMP
				if (c==1)
				{
					//Send Home Stop to NAC
					NACSendHomeStop();
					logf("n s\r\n");
				}
				else
				{
					logf("nac nr\r\n");
				}
#endif
#ifdef HOMESYSTEM_FASTENER
				if (g_cFastenerHomed == HOME_RUNNING || g_cFastenerHomed == HOME_PENDING)
				{
					logf("fa\r\n");
					c=FastenerSTPSet(FASTENER_OID_ABORT);
					if (c==1)
					{
						g_cFastenerHomed = HOME_NOT_DONE;
					}
				}
#endif
#ifdef HOMESYSTEM_FASTENERTRAY
				if (g_cFastenerTrayHomed == HOME_RUNNING || g_cFastenerTrayHomed == HOME_PENDING)
				{
					logf("fa\r\n");
#ifdef FASTENER_TRAY_STP
					c=FastenerTraySTPSet(FASTENERTRAY_OID_ABORT);
					if (c==1)
					{
						g_cFastenerTrayHomed = HOME_NOT_DONE;
					}
#endif
				}
#endif
				break;
			case MINIFT_OID_ESTOP_CLEAR_DELAY:
				g_ConfigData.uiEStopClearDelay = (uint16) ntohs(
						*((int16*) (p_STPrxMsg->p_cObjectValue)));
				break;
			case MINIFT_OID_DRILL_BUTTON_DELAY:
				g_ConfigData.uiDrillButtonDelay = (uint16) ntohs(
						*((int16*) (p_STPrxMsg->p_cObjectValue)));
				break;
			case MINIFT_OID_USE_CUTTER_DETECT:
				g_ConfigData.cUseCutterDetect = p_STPrxMsg->p_cObjectValue[0];
				break;
			case MINIFT_OID_JOG_ENABLE_TIMEOUT:
				g_ConfigData.cJogEnableTimeout = p_STPrxMsg->p_cObjectValue[0];
				break;
			case MINIFT_OID_DRILL_CYCLE_DELAY:
				g_ConfigData.uiDrillCycleDelay = (uint16) ntohs(
						*((int16*) (p_STPrxMsg->p_cObjectValue)));
				break;
			case MINIFT_OID_INSPECT_METHOD:
				g_ConfigData.cInspectMethod = p_STPrxMsg->p_cObjectValue[0];
				break;
			case MINIFT_OID_COMMAND_INSPECT_METHOD:
				g_ConfigData.cCommandInspectMethod =
						p_STPrxMsg->p_cObjectValue[0];
				break;
			case MINIFT_OID_FORCE_SENSOR_CALIBRATION:
				p_ForceSensorCalibration =
						(td_ForceSensorCalibration *) p_STPrxMsg->p_cObjectValue;
				g_ConfigData.ForceSensorCalibration.iZeroX = ntohs(
						p_ForceSensorCalibration->iZeroX);
				g_ConfigData.ForceSensorCalibration.iZeroY = ntohs(
						p_ForceSensorCalibration->iZeroY);
				g_ConfigData.ForceSensorCalibration.iZeroZ = ntohs(
						p_ForceSensorCalibration->iZeroZ);
				g_ConfigData.ForceSensorCalibration.iCountsPerGX = ntohs(
						p_ForceSensorCalibration->iCountsPerGX);
				g_ConfigData.ForceSensorCalibration.iCountsPerGY = ntohs(
						p_ForceSensorCalibration->iCountsPerGY);
				g_ConfigData.ForceSensorCalibration.iCountsPerGZ = ntohs(
						p_ForceSensorCalibration->iCountsPerGZ);
				break;
			case MINIFT_OID_FORCE_LIMITS:
				p_ForceLimits = (td_ForceLimits *) p_STPrxMsg->p_cObjectValue;
				g_ConfigData.ForceLimits.uiSensorInterval = ntohs(
						p_ForceLimits->uiSensorInterval);
				g_ConfigData.ForceLimits.uiMinUpdateDelta = ntohs(
						p_ForceLimits->uiMinUpdateDelta);
				g_ConfigData.ForceLimits.cActive = p_ForceLimits->cActive;
				g_ConfigData.ForceLimits.cCurrentUnderMethod =
						p_ForceLimits->cCurrentUnderMethod;
				g_ConfigData.ForceLimits.uiCurrentOverX = ntohs(
						p_ForceLimits->uiCurrentOverX);
				g_ConfigData.ForceLimits.uiCurrentUnderX = ntohs(
						p_ForceLimits->uiCurrentUnderX);
				g_ConfigData.ForceLimits.uiCurrentOverY = ntohs(
						p_ForceLimits->uiCurrentOverY);
				g_ConfigData.ForceLimits.uiCurrentUnderY = ntohs(
						p_ForceLimits->uiCurrentUnderY);
				g_ConfigData.ForceLimits.uiFullGravX = ntohs(
						p_ForceLimits->uiFullGravX);
				g_ConfigData.ForceLimits.uiFullGravY = ntohs(
						p_ForceLimits->uiFullGravY);
				g_ConfigData.ForceLimits.uiFlatForceX = ntohs(
						p_ForceLimits->uiFlatForceX);
				g_ConfigData.ForceLimits.uiFlatForceY = ntohs(
						p_ForceLimits->uiFlatForceY);
				break;
			case MINIFT_OID_PROBE_FLAGS:
				g_ConfigData.cProbeFlags = p_STPrxMsg->p_cObjectValue[0];
				break;
			case MINIFT_OID_MO_CAL:
				p_MOCal = (td_MOCal *) p_STPrxMsg->p_cObjectValue;
#ifdef OBSTRUCTION_SYSTEM_MO_ANA
				g_ConfigData.MOCal.uim1=ntohs(p_MOCal->uim1);
				g_ConfigData.MOCal.uim2=ntohs(p_MOCal->uim2);
				g_ConfigData.MOCal.uim3=ntohs(p_MOCal->uim3);
				g_ConfigData.MOCal.uim4=ntohs(p_MOCal->uim4);
				g_ConfigData.MOCal.uim5=ntohs(p_MOCal->uim5);
				g_ConfigData.MOCal.uim6=ntohs(p_MOCal->uim6);
#endif
				break;
			case MINIFT_OID_VISION_EXTERNAL_ANALYSIS:
				#ifdef CENTERVISION_ANALOG_POINT_LASER
				//Requires no NTOH ...pass buffer as object
				HandleVisionExternalAnalysis((td_oid_vision_external_analysis *) p_STPrxMsg->p_cObjectValue);
#endif
				break;
			case MINIFT_OID_RFID_TAG_SET:
				#ifdef USE_RFID_OMRON
				p_oid_rfid_tag_set=(td_oid_rfid_tag_set *)p_STPrxMsg->p_cObjectValue;
				ui=sizeof(td_oid_rfid_tag_set);
				p_STPrxMsg->p_cObjectValue[ui]=0;

				//replace nulls with spaces
				i=0;
				while(i<ui)
				{
					c=p_STPrxMsg->p_cObjectValue[i];
					if (c==0)
					{
						//contains nulls... convert to spaces
						c=' ';
						p_STPrxMsg->p_cObjectValue[i]=c;
					}
					logf("%d %d %c\r\n",i,c,c);
					i++;
				}
				//show this modified string
				logf("%d bytes %s\r\n",ui,p_STPrxMsg->p_cObjectValue);
				///Create The OMRON Command
				rfidWriteData((char*)p_oid_rfid_tag_set,ui);
#endif
				break;
			case MINIFT_OID_KHOLE_MAX_DISTANCE_CHECK:
				g_ConfigData.fMaxKholeDistanceCheck =
						*(float *) p_STPrxMsg->p_cObjectValue;
				break;
			case MINIFT_OID_MAX_EDGE_SHIFT_PROBE_ACCEPT:
				g_ConfigData.fMaxEdgeShiftProbeAccept =
						*(float *) p_STPrxMsg->p_cObjectValue;
				break;
			case MINIFT_OID_ALLOW_DRILL_BEYOND_SHIFT_LIMITS:
				g_ConfigData.cAllowDrillBeyondShiftLimits =
						p_STPrxMsg->p_cObjectValue[0];
				break;
			case MINIFT_OID_Y_RETRACT:
				// = p_STPrxMsg->p_cObjectValue[0];
//FIXME INCOMPLETE FEATURE
//FIXME MEDIUM MINIFT_OID_Y_RETRACT  #warns "Need MINIFT_OID_Y_RETRACT completed"
				break;
			case MINIFT_OID_SYSTEM_COMPONENTS:
				p_SystemComponents =
						(td_SystemComponents *) p_STPrxMsg->p_cObjectValue;
				g_ConfigData.SystemComponents.cDrill =
						p_SystemComponents->cDrill;
				g_ConfigData.SystemComponents.cFastener =
						p_SystemComponents->cFastener;
				g_ConfigData.SystemComponents.cFastenerTray =
						p_SystemComponents->cFastenerTray;
				g_ConfigData.SystemComponents.cAux1 = p_SystemComponents->cAux1;
				g_ConfigData.SystemComponents.cAux2 = p_SystemComponents->cAux2;
				g_ConfigData.SystemComponents.cAux3 = p_SystemComponents->cAux3;
				g_ConfigData.SystemComponents.cAux4 = p_SystemComponents->cAux4;
				g_ConfigData.SystemComponents.cAux5 = p_SystemComponents->cAux5;
				SendSystemComponents(p_STPSession, STP_ALERT); //echo back
//FIXME HIGH echos back to sender only.... issue is OID beyond mapping
//Add and test later.
				break;
			case MINIFT_OID_RFID_TAG_SET2:
				#ifdef USE_RFID_MIFARE
				p_oid_rfid_tag_set2 =
						(td_oid_rfid_tag_set2 *) p_STPrxMsg->p_cObjectValue;
				p_rfid_f0 = (td_RFID_F0 *) g_bRFIDTagData;
				p_rfid_f0->cFormat = p_oid_rfid_tag_set2->cFormat;
				if (p_rfid_f0->cFormat == 0)
				{
					p_rfid_f0->cRailType = p_oid_rfid_tag_set2->cRailType;
					p_rfid_f0->cGroup = p_oid_rfid_tag_set2->cGroup;
					p_rfid_f0->cSegmentAndRailSide =
							p_oid_rfid_tag_set2->cSegment & 0x7F;
					if (p_oid_rfid_tag_set2->cRailSide == 0)
					{
					}
					else if (p_oid_rfid_tag_set2->cRailSide == 1)
					{
						p_rfid_f0->cSegmentAndRailSide =
								p_rfid_f0->cSegmentAndRailSide & 128;
					}
					p_rfid_f0->ulSerialNumber =
							p_oid_rfid_tag_set2->ulSerialNumber; //tag data is already in network order
					p_rfid_f0->ulPosition = p_oid_rfid_tag_set2->ulPosition;
					p_rfid_f0->ulSegmentPosition =
							p_oid_rfid_tag_set2->ulSegmentPosition;
					g_szTagDatalen = RFID_F0_DATA_SIZE;
					SL031LoginSector();
					SL031WriteData((char*) g_bRFIDTagData, g_szTagDatalen);
				}
#endif
				break;

//FIXMENOWzxcv must determine if it's better to include this in MINIFT OID max, or allow it as another group beyond.
//I certainly have NONE of the labels set............ I have distinct label group, but NOT distinct handling...
//so what is the pattern for that?????????  can't store all strings from 2xx to 400.....
			case TOOLMANAGEMENT_OID_TOOL_MGMT:
				handle_tool_mgmt: p_oid_tool_mgmt =
						(td_oid_tool_mgmt *) p_STPrxMsg->p_cObjectValue;
				c_op = p_oid_tool_mgmt->coperation;
				c_arg1 = p_oid_tool_mgmt->carg1;
				logf("rxtm %d\r\n", c_op);
				if (c_op == register_tool_server)
				{
					//This is the current tool server
					//does not use any other arguments
					g_ToolServerSTPSession = p_STPSession;
					break;
				}
				if (c_op == register_tool_client)
				{
					//This connection is a tool client, such as MiniFControl
					//use carg1
//FIXMENOWzxcv
//Instead of retaining this information, we just treat all clients like they are
//signed up to show the required tool search results.
					break;
				}
				ui = ntohs(p_oid_tool_mgmt->uiarg2);
				s = ((char *) p_oid_tool_mgmt) + sizeof(td_oid_tool_mgmt); //save this location of the incoming bstring
				ilen = *s++; //read the length out of that bstring
				if (c_op == search || c_op == load_request)
				{
					//Message must be from client... handle by translating and forwarding to tool server
					//Is there a tool server register?
					if (g_ToolServerSTPSession == 0)
					{
						logf("nts\r\n");
						break;
					}
					//Write MGMT out
					SendToolMGMT(g_ToolServerSTPSession, p_STPrxMsg->uiMsgType, c_op, c_arg1, ui, s, ilen);
					break;
				}
				if (c_op == search_result || c_op == search_failure || c_op == load_failure)
				{
					//Message from server should be send back to interested clients.
					//FIXMENOWzxcv
					// For any tool clients interested in this result, forward the result back...
					SendToolMGMT(0, p_STPrxMsg->uiMsgType, c_op, c_arg1, ui, s, ilen);
					break;
				}
				unsupported_op_code:
				//unsupported op???
#ifdef OUTPUT_VERBOSE_TOOL_MGMT
				logf("TOOL_MGMT uop %d\r\n",c_op);
#endif
				break;
			case TOOLMANAGEMENT_OID_TOOL_REC:
				handle_tool_rec: p_LoadedTool =
						(td_oid_tool_rec *) p_STPrxMsg->p_cObjectValue;
				c_op = p_LoadedTool->cOperation;
				logf("rxtr %d\r\n", c_op);
				if (c_op == search_result_rec)
				{
					//forward to clients interested in this result...
//FIXMENOWzxcv
					//check the flags set by the MINIFT_OID_TOOL_MGMT search_result
//FIXMENOWzxcv
//TOOL_REC	search-result-rec	result-index	<tool record fields> (see below)
//FIXMENOWzxcv

//FIXMENOWzxcv  temporarilly going to  send this to all people....  EVEN...

					//Resend this entire message
					ui = p_STPrxMsg->uiValueLength;
					SmartToolMsg(0, p_STPrxMsg->uiMsgType, TOOLMANAGEMENT_OID_TOOL_REC, ui, ((char*) p_LoadedTool));
					break;
				}
				if (c_op == load_success)
				{
					//Returned from Tool Server, indicates we are allowed to load this tool
					goto proceed_with_load_tool;
				}
				if (c_op == load)
				{
					//loaded directly from pendant
					goto proceed_with_load_tool;
				}
				if (c_op == update)
				{
					//update from pendant
					goto proceed_with_load_tool;
				}
				if (c_op == unload)
				{
					//this is an unload.
					//First Send Alert Update of what was here to the Tool Server
					//FIXMENOWzxcv should only do this if it was unable to send previously.......
					if (g_ToolServerSTPSession != 0)
					{
#ifdef OUTPUT_VERBOSE_TOOL_MGMT
						logf("!pre un up\r\n",c_op);
#endif
						SendTool(g_ToolServerSTPSession, STP_ALERT, update);
					}
//FIXMENOWzxcv someday implement the abillity to resend the last used tools when the server reconnects.
					//first copy whatever they sent, which should be the proper empty values
					goto proceed_with_load_tool;
				}
				//unsupported op???
#ifdef OUTPUT_VERBOSE_TOOL_MGMT
				logf("TOOL_REC uop %d\r\n",c_op);
#endif
				break;
				proceed_with_load_tool: p_LoadedTool =
						(td_oid_tool_rec *) p_STPrxMsg->p_cObjectValue;
				g_LoadedTool.cOperation = p_LoadedTool->cOperation;
				g_LoadedTool.cToolStatus = p_LoadedTool->cToolStatus;
				g_LoadedTool.cHardstop = p_LoadedTool->cHardstop;
				g_LoadedTool.fDiameter = p_LoadedTool->fDiameter;
				g_LoadedTool.fLength = p_LoadedTool->fLength;
				g_LoadedTool.fMinBreak = p_LoadedTool->fMinBreak;
				g_LoadedTool.fLengthCountersink =
						p_LoadedTool->fLengthCountersink;
				g_LoadedTool.fCountersinkAdjust =
						p_LoadedTool->fCountersinkAdjust;
				g_LoadedTool.ulDTimeTicksMSW = ntohl(
						p_LoadedTool->ulDTimeTicksMSW);
				g_LoadedTool.ulDTimeTicksLSW = ntohl(
						p_LoadedTool->ulDTimeTicksLSW);
				g_LoadedTool.uiDCount = ntohs(p_LoadedTool->uiDCount);
				g_LoadedTool.uiDWarnCount = ntohs(p_LoadedTool->uiDWarnCount);
				g_LoadedTool.uiDLimitCount = ntohs(p_LoadedTool->uiDLimitCount);
				//bstring field szID
				//bstring field szToolTypeCode
				//Read BStrings after fixed size fields
				p_c = ((char *) p_LoadedTool) + sizeof(td_oid_tool_rec);
				ReadBString(p_c, g_szToolIDlen, g_szToolID, 256)
				;
				ReadBString(p_c, g_szToolTypeCodelen, g_szToolTypeCode, 64)
				;
				if (c_op == unload)
				{
					//ensure they did it right
					g_szToolID[0] = 0;
					g_szToolIDlen = 0;
					g_szToolTypeCode[0] = 0;
					g_szToolTypeCodelen = 0;
				}
				logf("-+ %d \"%s\" %d \"%s\"\r\n", g_szToolIDlen, g_szToolID,
						g_szToolTypeCodelen, g_szToolTypeCode);
				if (g_szToolIDlen == 0)
				{
					g_cLoadedTool = 0;
					c_op = unload; //this is an unload, no matter what op came in
				}
				else
				{
					g_cLoadedTool = LookupToolTypeCode(g_szToolTypeCode);
				}
				//And Alert this tool status out
				SendTool(0, STP_ALERT, c_op);
				logf("!eop %d\r\n", c_op);
#ifdef TOOL_IN_RAM
				SaveToolToRam();
#endif
				//Clear These
				g_cToolLoaded = 0;
				g_cLoadedProcess = 0; //Process is still good, but for sake of showing the entire data set going down in order do this here.
									  //And as it turns out, it might also be good to force the approximate depths used for process load to calc with this tool inorder to
									  //correct the possibility that the process was loaded at the time a bad tool was used.
				g_cProcessLoaded = 0;
				g_cOverrideCalculated = 0;
				g_cOverrideSent = 0;
#ifdef DRILL_DIRECT_READY
				//Load To Drill
				LoadToolToDrill();
				LoadToolHomeBackToDrill();
				//and reposition the drill after this tool load
				RepositionDrill();
#endif
				SmartToolMsgChar(p_STPSession, STP_ALERT, MINIFT_OID_PROCESS,
						g_cLoadedProcess); //New Purpose of Alert is to just show what is loaded...
				break;
				//MakeOID-generated::END
			default:
				SmartToolMsgCommonMessageCode(p_STPSession, uiOID,
						COMMONMC_NOSUCHOID);
#ifdef OUTPUT_RXSTP
				p_szOIDName = DisplayOIDName(p_STPSession, uiOID);
				goto show_unsupported_stp_message;
#endif
#endif
			}
			break;
		case STP_ALERT:
			switch (uiOID)
			{
			//MakeOID-generated:: STP ALERT RX (oid merge)
			//Not Really auto generated... generally the MiniFT does not deal with alearts, but the tool system requires this.
			case TOOLMANAGEMENT_OID_TOOL_MGMT:
				goto handle_tool_mgmt;
			case TOOLMANAGEMENT_OID_TOOL_REC:
				goto handle_tool_rec;
				//MakeOID-generated::END
			default:
				#ifdef OUTPUT_RXSTP
				p_szOIDName = "";
				goto show_unsupported_stp_message;
#endif
				break;
			}
			break;
			//case STP_GET_RESP:
			//    switch(uiOID)
			//    {
			//        case MINIFT_OID_NULLOID:
			//            //Do nothing
			//            break;
			//        default:
			//            #ifdef OUTPUT_RXSTP
			//            goto case_unsupported_stp_message;
			//	        #endif
			//	}
			//    break;
		default:
			#ifdef OUTPUT_RXSTP
			logf("bad STP mtyp:\r\n");
#endif
			break;
		}        //switch(uiMsgType)
		return 0;
	} //end MiniFT's Client Message
////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef CLAMP_SYSTEM_NAC_STP
	if ( p_STPSession->iSessionNum == -1) //NAC
	{
		//special NAC connection has handler for NAC OIDs
#ifdef OUTPUT_RXSTP
		p_szSmartTool = "nac ";
#ifdef OUTPUT_RXSTP_NAC
		if (uiOID!=0)
		{
			//logf("NAC RX STP: v%d %d-%s %d %d \r\n", p_STPrxMsg->uiVersion, p_STPrxMsg->uiMsgType, DisplayMessageTypeName(p_STPrxMsg->uiMsgType), p_STPrxMsg->uiOID, p_STPrxMsg->uiValueLength);
			logf("%s RX STP: %d-%s %d %d \r\n", p_szSmartTool, p_STPrxMsg->uiMsgType, DisplayMessageTypeName(p_STPrxMsg->uiMsgType), uiOID, p_STPrxMsg->uiValueLength);
		}
#endif
#endif
		switch(p_STPrxMsg->uiMsgType)
		{
			case STP_GET:
			switch(uiOID)
			{
				//MakeOID-generated:: STP GET (oid merge)
				case MINIFT_OID_NULLOID:
				SmartToolMsgEmpty(p_STPSession, STP_GET_RESP, uiOID);
				break;
			}
			break;
			case STP_ALERT:
			switch(uiOID)
			{
				case COMMON_OID_NULLOID:
				//the server is doing it's keep alive
				break;
				case COMMON_OID_SERIAL_NUMBER:
				ui=p_STPrxMsg->uiValueLength;
				if (ui>64 - 1)
				{
					ui=64 - 1;
				}
				//copy the value
				memcpy(g_szRailSerialNumber,p_STPrxMsg->p_cObjectValue,ui);
				g_szRailSerialNumber[ui]=0;
				logf("NAC is %s\r\n",g_szRailSerialNumber);
				SmartToolMsgStr(0, STP_ALERT, MINIFT_OID_NAC_SERIAL_NUMBER , g_szRailSerialNumber );
				break;
				case NAC_OID_CLAMP_STATE:
				g_cClampState = p_STPrxMsg->p_cObjectValue[0];
				logf("CS=%d\r\n",g_cClampState);
				break;
				case NAC_OID_AAXIS_HOMED:
#ifdef HOMESYSTEM_AAXIS
				c = p_STPrxMsg->p_cObjectValue[0];
				logf("rx nah=%d\r\n",c);
				if (c==HOME_NOT_DONE)
				{
					if (g_cNACAAxisHomed == HOME_RUNNING || g_cNACAAxisHomed == HOME_PENDING)
					{
						//this seems like a false alarm bug... correct this by ignoring this
						logf("*ignore\r\n");
						break;
					}
				}
				g_cNACAAxisHomed = c;
				if (g_cHomed!=HOME_RUNNING)
				{
					//Home Status has been adjusted outside of home running: Just update the home status
					if (g_cNACAAxisHomed == HOME_NOT_DONE || g_cNACAAxisHomed == HOME_FAILURE)
					{
						g_cHomed = g_cNACAAxisHomed;
					}
				}
				else
				{
					if (g_cNACAAxisHomed==HOME_NOT_DONE)
					{
					}
				}
#endif
				break;
				case NAC_OID_CLAMP_HOMED:
#ifdef HOMESYSTEM_CLAMP
				g_cNACClampHomed = p_STPrxMsg->p_cObjectValue[0];
				if (g_cHomed!=HOME_RUNNING)
				{
					//Home Status has been adjusted outside of home running: Just update the home status
					if (g_cNACClampHomed == HOME_NOT_DONE || g_cNACClampHomed == HOME_FAILURE)
					{
						g_cHomed = g_cNACClampHomed;
					}
				}
#endif
				break;
				case NAC_OID_NAC_CLEAR:
				g_cNACClear = p_STPrxMsg->p_cObjectValue[0];
				break;
				case NAC_OID_CAMERA_OFFSET:
				//Replace the MiniFT's Camera Offset
				//Since both hav the same struct, just use the MiniFt Struct
				p_CamOffset=(td_CamOffset *)p_STPrxMsg->p_cObjectValue;
				g_ConfigData.CamOffset.fX=p_CamOffset->fX;
				g_ConfigData.CamOffset.fY=p_CamOffset->fY;
				//And to be sure Pendant sees the update, in the case, alert it out.
				p_CamOffset=(td_CamOffset *)p_cSTPobjValBuff;
				p_CamOffset->fX=g_ConfigData.CamOffset.fX;
				p_CamOffset->fY=g_ConfigData.CamOffset.fY;
				SmartToolMsg(p_STPSession, STP_ALERT, MINIFT_OID_CAM_OFFSET, sizeof(td_CamOffset),p_cSTPobjValBuff);
				logf("camy\r\n");
				break;
//coming in via GET RESP NOW
//					case NAC_OID_CLAMP_Z_OFFSET:
//						//Record NACs Z offset for use in calculating clamp positions
//						g_fNACClampZOffset = *(float *)p_STPrxMsg->p_cObjectValue;
//logf("%s %f\r\n","z",f, g_fNACClampZOffset);
				default:
#ifdef OUTPUT_RXSTP
				p_szOIDName = "";
				goto nac_show_unsupported_stp_message;
#endif
			}
			break;
			case STP_GET_RESP:
			switch(uiOID)
			{
				case NAC_OID_CLAMP_Z_OFFSET:
				//Record NACs Z offset for use in calculating clamp positions
				g_fNACClampZOffset = *(float *)p_STPrxMsg->p_cObjectValue;
				logf("%s %f\r\n","z",f, g_fNACClampZOffset);
				break;
				default:
				logf("ungr %d\r\n",uiOID);
#ifdef OUTPUT_RXSTP
				p_szOIDName = "";
				goto nac_show_unsupported_stp_message;
#endif
			}
			break;
			default:
#ifdef OUTPUT_RXSTP
			logf("bad STP mtyp:\r\n");
#endif
		}						//switch(uiMsgType)
		return 0;
	} //end NAC Message
#endif
/////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef SMARTDRILL_STP
	if ( p_STPSession->iSessionNum == -2) //SmartDrill
	{
#ifdef OUTPUT_RXSTP
		p_szSmartTool = "sd ";
#ifdef OUTPUT_RXSTP_SD
		if (uiOID!=OID_NULLOID && uiOID!=OID_GENERICMESSAGE)
		{
			logf("%s RX STP: %d-%s %d %d \r\n", p_szSmartTool, p_STPrxMsg->uiMsgType, DisplayMessageTypeName(p_STPrxMsg->uiMsgType), uiOID, p_STPrxMsg->uiValueLength);
		}
#endif
#endif
		switch(p_STPrxMsg->uiMsgType)
		{
			case STP_GET:
			switch(uiOID)
			{
				//MakeOID-generated:: STP GET (oid merge)
				case MINIFT_OID_NULLOID:
				SmartToolMsgEmpty(p_STPSession, STP_GET_RESP, uiOID);
				break;
			}
			break;
			case STP_ALERT: //handle these together
			case STP_GET_RESP:
			switch(uiOID)
			{
				case COMMON_OID_NULLOID:
				//the server is doing it's keep alive
				break;
#ifdef DRILL_DIRECT_READY
				case COMMON_OID_SMARTTOOL_TYPE:
				case COMMON_OID_SYSTEM_VERSION:
				//ignore it...
				break;
				case COMMON_OID_COMMON_MESSAGE_CODE:
				goto nac_show_unsupported_stp_message;

				case COMMON_OID_GENERICMESSAGE:
				ui=p_STPrxMsg->uiValueLength;
				p_STPrxMsg->p_cObjectValue[ui]=0;
				logf("msg=%s\r\n",p_STPrxMsg->p_cObjectValue);
				if (ui < 13)
				{
					//don't bother looking for any messages, sicne they are all more than this
				}
				else if (memcmp(p_STPrxMsg->p_cObjectValue,"i:UnclampSafe",13)==0)
				{
					//DJWR FIXED 20120313-1
					//Only start unclamping, if the action code indicates it is running
					if (g_cAction == ACTION_EXECUTE && g_cStation==STATION_DRILL)
					{
						logf("ucae\r\n");
						g_ulUnclampStart = MS_TIMER;
						g_ulUnclampDone = g_ulUnclampStart;
						g_cSafeUnclamp = 1;
						g_cClampGoal=CLAMP_UNCLAMP;
					}
				}
				else if (memcmp(p_STPrxMsg->p_cObjectValue,"e:",2)==0)
				{
					p_c=p_STPrxMsg->p_cObjectValue+13;
					if (memcmp(p_c,"Drill start",11)==0)
					{
						g_ulDrillEchoStart = MS_TIMER;
						logf("@#%s %u\r\n","dse",0);
						//Increment Counter
						ServiceDateTime();
						g_LoadedTool.ulDTimeTicksMSW=g_DateTime.ulticksMSW;
						g_LoadedTool.ulDTimeTicksLSW=g_DateTime.ulticksLSW;
						g_LoadedTool.uiDCount++;
						SendTool(0, STP_ALERT,update);
						//Jump to below and share the code to alert this.
						//For Legacy Purposes, we track Drill Start with OP_DRILL_STARTED
						//This adds a OP_DRILL_STARTED to the history.
						ui = OP_DRILL_STARTED;
						goto addophist;//beware: do not move label to different function than taget below.
					}
					else if (memcmp(p_c,"Shift",5)==0)
					{
						logf("@#%s %u\r\n","dsh",MS_TIMER - g_ulDrillEchoStart);
					}
					else if (memcmp(p_c,"MaxLyr",6)==0)
					{
						logf("@#%s %u\r\n","drt",MS_TIMER - g_ulDrillEchoStart);
					}
				}
				else if (memcmp(p_STPrxMsg->p_cObjectValue,"i:BadProfile",12)==0)
				{
					//This always comes back when the profile and overrides are invalid.
					//This simple response will allow it to exit the process.
					g_cStartProcess = 0;
				}
				break;
				case SMARTDRILL_OID_MODE:
				c = g_cDrillState;
				g_cDrillState = p_STPrxMsg->p_cObjectValue[0]; //always just trust it
//FIXME Medium  check mode mapping.... consider renaming to correct... or plan renaming.....
#ifdef HOMESYSTEM
#ifdef HOMESYSTEM_DRILL
				if (g_cDrillState == DRILLSTATE_IDLE)
				{
					//sign that drill home has been done
					g_cDrillHomed = HOME_DONE;
//    _bDrillAtHome = true;  //and all that it contains: IE action limited on pendant etc....
					if (g_cDrillStateGoal==DRILLSTATE_HOME)
					{
						g_cDrillStateGoal = DRILLSTATE_IDLE;
					}
					c='I';
				}
				else if (g_cDrillState == DRILLSTATE_ESTOP)
				{
					//Indicate Home has not been done
					g_cDrillHomed = HOME_NOT_DONE;
//    _bDrillAtHome = false;  //and all that it contains: IE action limited
					c='E';
				}
				else if (g_cDrillState == DRILLSTATE_NOT_HOME)
				{
//    _bDrillAtHome = false;  //and all that it contains: IE action limited
					if (g_cDrillHomed == HOME_RUNNING && g_cDrillStateGoalSent == DRILLSTATE_HOME && g_cDrillHomeWasStarted == 1)
					{
						logf("FH!\r\n");
						g_cDrillHomed = HOME_FAILURE;
					}
					c='X';
				}
				else if (g_cDrillState == DRILLSTATE_HOME) //Means Homing Now
				{
//    _bDrillAtHome = false;  //and all that it contains: IE action limited
					g_cDrillHomeWasStarted = 1;
					c='H';
				}
				else if (g_cDrillState == DRILLSTATE_DRILL)
				{
					//sign that drill home has been done
					g_cDrillHomed = HOME_DONE;
					g_cSawDrillMode = 1;
#warnt "pendant code checked  if drillstategoal was not spinup or drill, then it would terminate using abort"
					c='D';
				}
				else if (g_cDrillState == DRILLSTATE_JOG)
				{
//    _bDrillAtHome = false;  //and all that it contains: IE action limited
					c='J';
				}
				else if (g_cDrillState == DRILLSTATE_FAULT)
				{
//    _bDrillAtHome = false;  //and all that it contains: IE action limited
					c='F';
				}
				else
				{
					logf("DS %d\r\n",g_cDrillState);
					goto skip_ds_print;
				}
				logf("DS %c\r\n",c);
				skip_ds_print:
#endif
#endif
				g_cDrillSync |= DRILL_SYNC_BIT1;
//FIXME Immediate  AtHome system is main thing I didn't port yet.....
				break;
				case SMARTDRILL_OID_GAGE_LINE:
				f = ntohfc(*(float *)p_STPrxMsg->p_cObjectValue);
				logf("Rx GL %f\r\n",f);
				g_cDrillSync |= DRILL_SYNC_BIT2;
				g_fGaugeLength=f;
				LoadToolHomeBackToDrill();
				break;
				case SMARTDRILL_OID_FAULT:
//Notice How Smart Drill has many more fields and we copy only the 3 we have in old structure td_DrillFault
//We don't care about the other fields at this time.
//typedef struct {
//	byte cDevice;
//	byte cSeverity;
//	long lFaultCode;
//} td_DrillFault;
//typedef struct {
//	byte cDevice;
//	byte cSeverity;
//	byte cSeverityPrev;
//	byte cReported;
//	long lFaultCode;
//	float fPosition;
//	int iCyclesOnCutter;
//	unsigned long ulDateTime;
//	unsigned int uiHoleNumber;
////Ignore these fields.
////	byte CutterType[32];
////	byte ProfileFilename[64];
//} td_SmartDrillFault;
				p_SmartDrillFault=(td_SmartDrillFault *)p_STPrxMsg->p_cObjectValue;
				g_DrillFault.cDevice=p_SmartDrillFault->cDevice;
				g_DrillFault.cSeverity=p_SmartDrillFault->cSeverity;
				g_DrillFault.lFaultCode=ntohl(p_SmartDrillFault->lFaultCode);
				logf("dfa %d %d %d\r\n",g_DrillFault.cDevice,g_DrillFault.cSeverity,g_DrillFault.lFaultCode);
				case SMARTDRILL_OID_HOLE_DATA:
				//we don't use all the fields that the drill has
				p_SmartDrillHoleData=(td_SmartDrillHoleData *)p_STPrxMsg->p_cObjectValue;
				g_HoleResultData.iHoleNumber=ntohs(p_SmartDrillHoleData->uiHoleNumber);
				g_HoleResultData.iHoleResult=ntohs(p_SmartDrillHoleData->iHoleResult);
				logf("HR %d %d\r\n",g_HoleResultData.iHoleNumber,g_HoleResultData.iHoleResult);
				if(g_ulDrillEchoStart!=0)
				{
					logf("@#%s %u\r\n","hr",MS_TIMER - g_ulDrillEchoStart);
				}
				//There is only one result for a drill cycle.
				//There could be a hole result even if it doesn't feed.
				//We store all the hole results in the structure, and these
				// values may be used in other places to help the cycle, but
				//for these 4 values, we also should update the op history for the hole.

				if (g_HoleResultData.iHoleNumber == g_iActionHoleIndex)
				{
					if (g_HoleResultData.iHoleResult == HR_STARTED)
					{
						ui = OP_DRILL_STARTED;
						goto addophist;
					}
					if (g_HoleResultData.iHoleResult == HR_FAULT)
					{
						ui = OP_DRILL_FAULT;
						goto addophist;
					}
					if (g_HoleResultData.iHoleResult == HR_ABORT)
					{
						ui = OP_DRILL_ABORT;
						goto addophist;
					}
					if (g_HoleResultData.iHoleResult == HR_SUCCESS)
					{
						ui = OP_DRILL_SUCCESS;
						logf("*dsa\r\n");
						addophist:
						g_uiPositionOpsCache |= ui; //since this is fir current position
						AddOpHistory(g_PosnMode.iCurPosnIndex,ui);
						logf("*aoh%u\r\n",ui);
						g_ulDrillEchoStart = 0;//clear this
					}
				}
				else
				{
					logf("*aohch%d i%d\r\n",g_iActionHoleIndex,g_HoleResultData.iHoleNumber);
				}
				break;
#endif
				default:
#ifdef OUTPUT_RXSTP
				p_szOIDName = "";
				goto show_unsupported_stp_message;
#endif
			}
			break;
			default:
#ifdef OUTPUT_RXSTP
			logf("bad STP mtyp:\r\n");
#endif
		}
		return 0;
	} //end SmartDrill Message
#endif
///////////////////////////////////////////////////////////////////////////////////////////////
#ifdef DRILLFILL_STP
	if ( p_STPSession->iSessionNum == -3) //DrillFill
	{
#ifdef OUTPUT_RXSTP
		p_szSmartTool = "df ";
#ifdef OUTPUT_RXSTP_DF
		if (uiOID!=OID_NULLOID)
		{
			logf("%s RX STP: %d-%s %d %d \r\n", p_szSmartTool, p_STPrxMsg->uiMsgType, DisplayMessageTypeName(p_STPrxMsg->uiMsgType), uiOID, p_STPrxMsg->uiValueLength);
		}
#endif
#endif
		switch(p_STPrxMsg->uiMsgType)
		{
			case STP_GET:
			switch(uiOID)
			{
				//MakeOID-generated:: STP GET (oid merge)
				case COMMON_OID_NULLOID:
				SmartToolMsgEmpty(p_STPSession, STP_GET_RESP, uiOID);
				break;
			}
			break;
			case STP_ALERT:
			switch(uiOID)
			{
				case COMMON_OID_NULLOID:
				//the server is doing it's keep alive
				break;
				case DRILLFILL_OID_CLAMP:
				g_cClampState = p_STPrxMsg->p_cObjectValue[0];
				break;
				case DRILLFILL_OID_DRILL_STATE:
				c=p_STPrxMsg->p_cObjectValue[0];
				logf("DF Drill State = %d\r\n",c);
				if (c==0)
				{
					if (g_cDrillHomed == HOME_RUNNING)
					{
						//looks like a failure...
						g_cDrillHomed = HOME_FAILURE;
					}
					if (g_HoleResultData.iHoleResult = HR_STARTED && g_cSafeUnclamp==1)
					{
						//must have made it all the way around the cycle
						g_HoleResultData.iHoleResult = HR_SUCCESS;
					}

					g_cDrillStateDFGoal = DRILLSTATE_IDLE;
					g_cDrillStateGoal = DRILLSTATE_IDLE;
					g_cDrillState = DRILLSTATE_IDLE;
					break;
				}
				if (c<=2)
				{
					//Jog Related
					break;
				}
				if (c<=9)
				{
					g_cDrillState = DRILLSTATE_NOT_HOME;
					break;
				}
				if (c<=18)
				{
					g_cDrillState = DRILLSTATE_DRILL;
					if (c<18)
					{
						g_cSafeUnclamp = 0; //Make sure that it's 0
					}
					else
					{
						g_cSafeUnclamp = 1; //a fix because it might appear from the MiniFT to unclamp before it's ready
					}
					break;
				}
				if (c==19)
				{
					g_cDrillState = DRILLSTATE_NOT_HOME;
					g_HoleResultData.iHoleResult = HR_ABORT;
					g_cSafeUnclamp = 0; //Make sure that it's 0
					break;
				}
				break;
				case DRILLFILL_OID_DRILL_HOMED:
				c=p_STPrxMsg->p_cObjectValue[0];
				if (c==1)
				{
					g_cDrillHomed = HOME_DONE;
				}
				logf("DF Homed = %d\r\n",c);
				break;
				default:
#ifdef OUTPUT_RXSTP
				p_szOIDName = "";
				goto show_unsupported_stp_message;
#endif
			}
			break;
			case STP_GET_RESP:
			switch(uiOID)
			{
				default:
#ifdef OUTPUT_RXSTP
				p_szOIDName = "";
				goto show_unsupported_stp_message;
#endif
			}
			break;
			default:
#ifdef OUTPUT_RXSTP
			logf("bad STP mtyp:\r\n");
#endif
		} //switch(uiMsgType)
		return 0;
	} //End DrillFill Message
#endif
/////////////////////////////////////////////////////////////////////////////////////////////
#ifdef FASTENER_STP
	if ( p_STPSession->iSessionNum == -4) //Fastener
	{
		//special Fastener connection has handler for Fastener OIDs
		//#ifdef OUTPUT_RXSTP
		p_szSmartTool = "fas ";
#ifdef OUTPUT_RXSTP_FAS
		if (uiOID!=0)
		{
			logf("%s RX STP: %d-%s %d %d \r\n", p_szSmartTool, p_STPrxMsg->uiMsgType, DisplayMessageTypeName(p_STPrxMsg->uiMsgType), uiOID, p_STPrxMsg->uiValueLength);
		}
#endif
		//#endif
		switch(p_STPrxMsg->uiMsgType)
		{
			case STP_GET:
			switch(uiOID)
			{
				//MakeOID-generated:: STP GET (oid merge)
				case MINIFT_OID_NULLOID:
				SmartToolMsgEmpty(p_STPSession, STP_GET_RESP, uiOID);
				break;
			}
			break;
			case STP_ALERT: //handle these together
			case STP_GET_RESP:
			switch(uiOID)
			{
				case COMMON_OID_NULLOID:
				//the server is doing it's keep alive
				break;
				case COMMON_OID_GENERICMESSAGE:
				ui=p_STPrxMsg->uiValueLength;
				p_STPrxMsg->p_cObjectValue[ui]=0;
				logf("msg=%s\r\n",p_STPrxMsg->p_cObjectValue);
				if (ui < 13)
				{
					//don't bother looking for any messages, sicne they are all more than this
				}
				else if (memcmp(p_STPrxMsg->p_cObjectValue,"i:UnclampSafe",13)==0)
				{
					//DJWR FIXED 20120313-1
					//Only start unclamping, if the action code indicates it is running
					if (g_cAction == ACTION_EXECUTE && g_cStation==STATION_FILL)
					{
						logf("ucae\r\n");
						g_ulUnclampStart = MS_TIMER;
						g_ulUnclampDone = g_ulUnclampStart;
						g_cSafeUnclamp = 1;
						g_cClampGoal=CLAMP_UNCLAMP;
					}
				}
				else if (memcmp(p_STPrxMsg->p_cObjectValue,"i:FastenDone",12)==0)
				{
					g_cFastDoneFlag = 1;
				}
				break;
				case FASTENER_OID_MODE:
				g_cFastenerMode = p_STPrxMsg->p_cObjectValue[0];
				logf("gfm%d\r\n", g_cFastenerMode);
				break;
				case FASTENER_OID_HOME_STATUS:
#ifdef HOMESYSTEM_FASTENER
				g_cFastenerHomed = p_STPrxMsg->p_cObjectValue[0];
				logf("gfh%d\r\n", g_cFastenerHomed);
				if (g_cHomed!=HOME_RUNNING)
				{
					//Home Status has been adjusted outside of home running: Just update the home status
					if (g_cFastenerHomed == HOME_NOT_DONE || g_cFastenerHomed == HOME_FAILURE)
					{
						g_cHomed = g_cFastenerHomed;
					}
				}
#endif
				break;
				case FASTENER_OID_FAULT:
				//Notice How Fastener has many more fields and we copy only the 3 we have
				p_FastenerFault=(td_FastenerFault *)p_STPrxMsg->p_cObjectValue;
				g_FastenerFault.cDevice=p_FastenerFault->cDevice;
				g_FastenerFault.cSeverity=p_FastenerFault->cSeverity;
				g_FastenerFault.lFaultCode=ntohl(p_FastenerFault->lFaultCode);
				logf("ffa %d %d %d\r\n",g_FastenerFault.cDevice,g_FastenerFault.cSeverity,g_FastenerFault.lFaultCode);
				if (g_cFastDoneFlag==1)
				{
					//ignore!!!
					g_FastenerFault.cSeverity=0;
					g_FastenerFault.lFaultCode=0;
				}
				break;
				case FASTENER_OID_FASTENER_XY_OFFSET:
				p_FastenerXYOffset = (td_FastenerXYOffset *) p_STPrxMsg->p_cObjectValue;
				g_ConfigData.OffsetFill.fx = p_FastenerXYOffset->fx;
				g_ConfigData.OffsetFill.fy = p_FastenerXYOffset->fy;
				logf("%s %f %f\r\n","xy",g_ConfigData.OffsetFill.fx,g_ConfigData.OffsetFill.fy);
				//Does not take effect right away, and changes to fastener are not queried right away.
				//When changing fastener, MiniFT Offset Fill should also be set if you want the effect to
				//take place right away.
#warns "Fastener FIXME Minor add setting change broadcast to fastener later."
				break;
				case FASTENER_OID_FASTENER_LOADED:
				g_cFastenerLoaded = p_STPrxMsg->p_cObjectValue[0];
				logf("%s %d\r\n","fl",g_cFastenerLoaded);
				break;
				case FASTENER_OID_FASTENER:
				p_Fastener = (td_FastenerSTATEANDSTATUS *) p_STPrxMsg->p_cObjectValue;
				//g_Fastener.uiType = ntohs(p_Fastener->uiType);
				g_cFastenerState = p_Fastener->cState;
				g_cPendingFastenerFaultUpdate = 0;
				logf("%s %d\r\n","*fs",g_cFastenerState);
				//g_Fastener.cSlot = p_Fastener->cSlot;
				//g_Fastener.fFastenedRevs = p_Fastener->fFastenedRevs;
				//g_Fastener.fFastenedTorqueA = p_Fastener->fFastenedTorqueA;
				//g_Fastener.fFastenedTorqueInLbs = p_Fastener->fFastenedTorqueInLbs;
				//g_Fastener.ulFastenTimeMs = ntohl(p_Fastener->ulFastenTimeMs);
				break;
				default:
#ifdef OUTPUT_RXSTP
				p_szOIDName = "";
				goto show_unsupported_stp_message;
#endif
			}
			break;
			default:
#ifdef OUTPUT_RXSTP
			logf("bad STP mtyp:\r\n");
#endif
		}						//switch(uiMsgType)
		return 0;
	} //end Fastener Message
#endif
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef FASTENER_TRAY_STP
	if ( p_STPSession->iSessionNum == -5) //Fastener Trey
	{
		//special Fastener connection has handler for Fastener OIDs
#ifdef OUTPUT_RXSTP
		p_szSmartTool = "ft ";
#ifdef OUTPUT_RXSTP_FT
		if (uiOID!=0)
		{
			logf("%s RX STP: %d-%s %d %d \r\n", p_szSmartTool, p_STPrxMsg->uiMsgType, DisplayMessageTypeName(p_STPrxMsg->uiMsgType), uiOID, p_STPrxMsg->uiValueLength);
		}
#endif
#endif
		switch(p_STPrxMsg->uiMsgType)
		{
			case STP_GET:
			switch(uiOID)
			{
				//MakeOID-generated:: STP GET (oid merge)
				case COMMON_OID_NULLOID:
				SmartToolMsgEmpty(p_STPSession, STP_GET_RESP, uiOID);
				break;
			}
			break;
			case STP_ALERT: //handle these together
			case STP_GET_RESP:
			switch(uiOID)
			{
				case COMMON_OID_NULLOID:
				//the server is doing it's keep alive
				break;
				case COMMON_OID_GENERICMESSAGE:
				ui=p_STPrxMsg->uiValueLength;
				p_STPrxMsg->p_cObjectValue[ui]=0;
				logf("msg=%s\r\n",p_STPrxMsg->p_cObjectValue);
#if 0
				if (ui < 13)
				{
					//don't bother looking for any messages, sicne they are all more than this
				}
#endif
				break;
				case FASTENERTRAY_OID_HOME_STATUS:
#ifdef HOMESYSTEM_FASTENERTRAY
				g_cFastenerTrayHomed = p_STPrxMsg->p_cObjectValue[0];
				logf("gfth%d\r\n", g_cFastenerTrayHomed);
				if (g_cHomed!=HOME_RUNNING)
				{
					//Home Status has been adjusted outside of home running: Just update the home status
					if (g_cFastenerTrayHomed == HOME_NOT_DONE || g_cFastenerTrayHomed == HOME_FAILURE)
					{
						g_cHomed = g_cFastenerTrayHomed;
					}
				}
#endif
				break;
				case FASTENERTRAY_OID_PICKUP_Y_OFFSET:
				f = *(float *)p_STPrxMsg->p_cObjectValue;
				g_fFastenerTrayPickupPosition = f;
				//Does not take effect right away, and changes to fastener tray are not queried right away.
				//Must get this prior to operation, so code will not allow a start if this is not available.
				logf("%s %f\r\n","py",f);
//FIXME MINOR #warns "Fastener FIXME Minor add setting change broadcast to fastener later."
				break;
				case FASTENERTRAY_OID_CLAMP_PICKUP_HEIGHT:
				f = *(float *)p_STPrxMsg->p_cObjectValue;
				g_fFastenerTrayClampPickupHeight = f;
				//Does not take effect right away, and changes to fastener tray are not queried right away.
				//Must get this prior to operation, so code will not allow a start if this is not available.
//FIXME MINOR #warns "Fastener FIXME Minor add setting change broadcast to fastener later."
				logf("%s %f\r\n","ph",f);
				break;

				default:
#ifdef OUTPUT_RXSTP
				p_szOIDName = "";
				goto show_unsupported_stp_message;
#endif
			}
			break;
			default:
#ifdef OUTPUT_RXSTP
			logf("bad STP mtyp:\r\n");
#endif
		}						//switch(uiMsgType)
		return 0;
	} //end Fastener Message
#endif
	//unknown session ???
	//fall through and use standard unsupported stp messge
#ifdef OUTPUT_RXSTP
#ifdef CLAMP_SYSTEM_NAC_STP
	nac_show_unsupported_stp_message:
#endif
	if (uiOID == 100 || uiOID == 103)
	{
		//Don't Bother showing... it appears on the interface due it it's direct NAC connection.
		//Also, we don't need to know this: NAC will report if clamp has failed.
		return 0;
	}
	show_unsupported_stp_message:
	logf(p_szSmartTool);
	logf("unspprtd STP msg : %d-%s %d-%s %d \r\n", p_STPrxMsg->uiMsgType, DisplayMessageTypeName(p_STPrxMsg->uiMsgType), uiOID, p_szOIDName, p_STPrxMsg->uiValueLength);
	return 0;
#else
#ifdef CLAMP_SYSTEM_NAC_STP
	nac_show_unsupported_stp_message:
#endif
	show_unsupported_stp_message:
	#endif
	return 0;
}

//FIXME PORTMED    ???  this is a prime candidate for file extract

////////////////////////////////////////////////////////////////////////////////
// Socket Console
////////////////////////////////////////////////////////////////////////////////
#ifdef SOCKETCONSOLE
void HandleSocketConsoleCommand(char * scmd, int icmdlen)
{
	byte c;
	byte lc;
	int i;
	char * sarg;
	char * sarg2;
	char temptestbuffer[16];
	td_RFID_F0 * p_rfid_f0;

	if (_cConsoleSockCommandMode >= 1)
	{
		if (_cConsoleSockCommandMode < 4)
		{
			//special direct
			// Console Command mode 1 goes to x whistle
			// Console Command mode 2 goes to y whistle
			if (icmdlen == 1 && *scmd == '~')
			{
				_cConsoleSockCommandMode = 0;
				return;
			}
			//anything else goes right to
			//logf("sending \"%s\"\r\n",scmd);
			scmd[icmdlen++] = '\r';			//OK to replace this character (the original end of command)
			c = scmd[icmdlen];			//record this character so we can restore this one.
			scmd[icmdlen] = 0;			//place temporary null
			if (_cConsoleSockCommandMode == 1)
			{
				MiniFTSendWhistleMsgXL(scmd, icmdlen);
			}
			else if (_cConsoleSockCommandMode == 2)
			{
				MiniFTSendWhistleMsgYL(scmd, icmdlen);
			}
#ifdef USE_RFID
			else if (_cConsoleSockCommandMode == 3)
			{
				//use for RFID
//FIXME PORTHIGH   disable this for MIFARE???

//				serRFIDrdFlush();
//				serRFIDwrFlush();
//				serRFIDputs(scmd);

				logf("\r\nsent.\r\n");
			}
#endif
			scmd[icmdlen] = c; //restore this character
			return;
		}
#ifdef CENTERVISION_CAM
		if (_cConsoleSockCommandMode == 4)
		{
			if (icmdlen == 1 && *scmd == '~')
			{
				_cConsoleSockCommandMode = 0;
				return;
			}
			//send it all to camera
			SendCVCamFromConsole(scmd, icmdlen);
			return;
		}
#endif
	}
	sarg = (char *) 0;
	sarg2 = (char *) 0;
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
	if (strcmp(scmd, "SToff") == 0)
	{
		//FIXME PORTHIGH make a shutdown
		logf("Smart Tool Shutdown Activated.\r\n");
		g_cSmartToolShutdown = 1;
	}
	else if (strcmp(scmd, "exit") == 0)
	{
		SocketConsoleClose();
	}
	else if (strcmp(scmd, "estop") == 0)
	{
		g_cEstopMCAsserted = 1;
		logf("estop asserted\r\n");
	}
	else if (strcmp(scmd, "brakeoff!") == 0) //Caution... Potentially dangerous to clear brake
	{
		BrakeOff();
		SetMotorActionDirectly(); //avoid brake timeout
	}
	else if (strcmp(scmd, "brake!") == 0) //Caution... Will Activate Brake without regard for motion underway.
	{
		BrakeOn();
		SetMotorActionDirectly(); //avoid brake timeout
	}
	else if (strcmp(scmd, "out") == 0)
	{
		if (sarg != 0)
		{
			i = atoi(sarg);
			c = 2;
			if (sarg2 != 0)
			{
				c = atoi(sarg2);
			}
			if (i < 128 && c < 2)
			{
				digOut(i, c);
				logf("set out %d %d\r\n", i, c);
			}
			else
			{
				logf("invalid args\r\n");
			}
		}
		//JLIME change for Angle sensors.
		else if( strcmp(scmd, "sa") == 0)
		{
			//activate showing the sensor angle details
			//mode 0 is quiet
			//mode 1 shows angles only
			//mode 2 shows more detail one time and then goes back to 0

			//if sarg is NULL, this will just end up out of range and therefore get set to 0
			if (*sarg==0)
			{
				g_cAngleSensorDisplay=2;
			}
			else
			{
				g_cAngleSensorDisplay = *sarg - '0';
				if (g_cAngleSensorDisplay>=4)
				{
					//no mode 4 or higher yet
					g_cAngleSensorDisplay=0;
				}
			}
		}
	}
#ifdef HOMESYSTEM_Y
	else if (strcmp(scmd, "yh") == 0) //make it think homing is done
	{
		g_cHomedY = HOME_DONE;
	}
#endif
#ifdef HOMESYSTEM_X
	else if (strcmp(scmd, "xh") == 0) //make it think homing is done
	{
		g_cHomedX = HOME_DONE;
	}
#endif
	else if (strcmp(scmd, "wx") == 0) //Whistle Clamp Axis Console Mode
	{
		//mc direct
		_cConsoleSockCommandMode = 1;
		i = 3; //default is rx and tx on
		if (sarg != 0)
		{
			i = atoi(sarg);
		}
		MCSetCommDisplayX(i);
	}
	else if (strcmp(scmd, "wy") == 0) //Whistle Clamp Axis Console Mode
	{
		//mc direct
		_cConsoleSockCommandMode = 2;
		i = 3; //default is rx and tx on
		if (sarg != 0)
		{
			i = atoi(sarg);
		}
		MCSetCommDisplayY(i);
	}
	else if (strcmp(scmd, "wq") == 0) //Whistle Quiet!
	{
		MCSetCommDisplayX(0);
		MCSetCommDisplayY(0);
	}
#ifdef USE_RFID
	else if (strcmp(scmd, "rfidcom") == 0) //Whistle Clamp Axis Console Mode
	{
		//rfid direct
		_cConsoleSockCommandMode = 3;
		logf("%s:\r\n", "rfidcom");
	}
#endif
	else if (strcmp(scmd, "cog") == 0) //Whistle Clamp Axis Console Mode
	{
		//mc direct
		_cConsoleSockCommandMode = 4;
	}
	else if (strcmp(scmd, "ocm") == 0)
	{
		if (sarg != 0)
		{
			g_ConfigData.cObstructionCodeMask = atoi(sarg);
		}
		logf("ocm=%x\r\n", g_ConfigData.cObstructionCodeMask);
	}
#ifdef FORCELIMITING
	else if (strcmp(scmd, "sf") == 0)
	{
		g_cShowForce = !g_cShowForce;
	}
#endif
	else if (strcmp(scmd, "test") == 0)
	{
		//g_cPrevOrSensors = 0xFF;
	}
#ifdef CLAMP_SYSTEM
	else if (strcmp(scmd, "t2") == 0)
	{
		//special testing code allows safe unclamp message testing
		if (sarg != 0)
		{
			g_cSafeUnclamp = atoi(sarg);
			if (g_cSafeUnclamp == 1)
			{
				g_cClampGoal = CLAMP_UNCLAMP;
			}
		}
	}
#endif
	else if (strcmp(scmd, "testopt") == 0)
	{
		if (sarg != 0)
		{
			g_cTestOpt = atoi(sarg);
		}
	}
	else if (strcmp(scmd, "cd") == 0)
	{
		if (sarg != 0)
		{
			g_ConfigData.cUseCutterDetect = (atoi(sarg) == 1);
		}
	}
#ifdef HOMESYSTEM
#ifdef HOMESYSTEM_X_RFID
//RFID TESTING CODE ///////////////////////////////////////////////////
	else if (strcmp(scmd, "rr") == 0)
	{
		//read RFID now
		g_cReadRFID = RFID_READ_NOW;
#ifdef USE_RFID_OMRON
		g_cTagReadState = RFID_TAG_CLEAR;
#endif
	}
#ifdef USE_RFID_MIFARE
	else if (strcmp(scmd, "rrs") == 0)
	{
		SL031SelectCard();
	}
	else if (strcmp(scmd, "rrl") == 0)
	{
		SL031LoginSector();
	}
	else if (strcmp(scmd, "rrlw") == 0)
	{
		SL031LoginSector();
		goto write_immediately_after;
	}
	else if (strcmp(scmd, "rrw") == 0)
	{
		write_immediately_after:
		//write the mifare tag with a test code
		memset(temptestbuffer, 0, 16);
		temptestbuffer[1] = 1;
		temptestbuffer[2] = 2;
		temptestbuffer[3] = 3;
		temptestbuffer[4] = 4;
		temptestbuffer[5] = 5;
		temptestbuffer[6] = 6;
		SL031WriteData(temptestbuffer, 16);
	}
#endif
//////////////////////////////////////////////////////////////////
	else if (strcmp(scmd, "rh") == 0)
	{
		//Start RFID HOME
		//Main Home Code will operate and start any pending home components
		g_cHomed = HOME_RUNNING;
		g_cHomedX = HOME_PENDING;
	}
	else if (strcmp(scmd, "rf") == 0)
	{
		//pending...
	}
#endif
#ifdef HOMESYSTEM_Y
	else if (strcmp(scmd, "hy") == 0)
	{
		//Start RFID HOME
		//Main Home Code will operate and start any pending home components
		g_cHomed = HOME_RUNNING;
		g_cHomedY = HOME_PENDING;
	}
#endif
#endif
#ifdef DRILL_DIRECT_READY
	else if (strcmp(scmd, "rd") == 0)
	{
		ClearDrillSync();
		SmartDrillResync();
		LoadToolToDrill();
	}
#endif
	else if (strcmp(scmd, "ee") == 0)
	{
		if (sarg != 0)
		{
			g_cEEOption = atoi(sarg);
		}
		logf("ee=%d\r\n", g_cEEOption);
		if (sarg != 0)
		{
			//It's Changed
#ifdef HD_RAIL_STP
			InitRailSTP();
#endif
		}
	}
//Hold: place the machine into a state where only socket console is serviced.
//Warning:  Hold should be run only when all implications of hold are understood and it is safe to do so
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
		g_cShowAna = !g_cShowAna;
	}
	else if (strcmp(scmd, "sta") == 0)
	{
		g_cJogGoalY = JOGSTOP;
		g_cJogGoalX = JOGSTOP;
	}
	else if (strcmp(scmd, "st") == 0)
	{
		MCStopPosition();
	}
	else if (strcmp(scmd, "nmt") == 0)
	{
//NEEDS A conversion for new sys
//		i=0;
//		while(i<=OID_MAX_NUMBER_MiniFT)
//		{
//			//FIXME PORTMED TEST
//			logf("%d %s\r\n",i,DisplayOIDName(0, i));
//			i++;
//		}
	}
	else if (strcmp(scmd, "vk") == 0)
	{
		g_cAllowKVisit = 1;
	}
	else if (strcmp(scmd, "tm") == 0)
	{
		ServiceDateTime();
		logf("date %d %d %d %d %d %d %d\r\n",
				g_DateTime.uiyear, g_DateTime.cmonth, g_DateTime.cdayOfMonth,
				g_DateTime.chour, g_DateTime.cminute, g_DateTime.csecond, g_DateTime.uimillisecond);
	}
#ifdef FASTENER_STP
	else if (strcmp(scmd, "rfr") == 0)
	{
		CloseFandFT();
	}
#endif
	else if (strcmp(scmd, "fh") == 0)
	{
#ifdef HOMESYSTEM_X_LINK
		g_cHomedX = HOME_NOT_DONE;
#endif
#ifdef HOMESYSTEM_Y_LINK
		g_cHomedY = HOME_NOT_DONE;
#endif
	}
	else if (strcmp(scmd, "hf") == 0)
	{
#ifdef HOMESYSTEM_FASTENER
		g_cFastenerHomed = HOME_DONE;
#endif
#ifdef HOMESYSTEM_FASTENERTRAY
		g_cFastenerTrayHomed = HOME_DONE;
#endif
	}
	else if (strcmp(scmd, "op") == 0)
	{
		if (sarg != 0)
		{
			g_ConfigData.uiProcessOperations = atoi(sarg);
		}
		logf("op %x %d\r\n", g_ConfigData.uiProcessOperations, g_ConfigData.uiProcessOperations);
		if (sarg != 0)
		{
			UpdateStationPlan();
		}
	}
	else if (strcmp(scmd, "nl") == 0)
	{
		if (sarg != 0)
		{
			g_cLubeBypass = atoi(sarg);
		}
		logf("%s=%d\r\n", "nl", g_cLubeBypass);
	}
	else if (strcmp(scmd, "rfid") == 0)
	{
		if (sarg != 0)
		{
			if (*sarg == '!')
			{
				g_cImmediateFakeRfid = 1;
				goto skip_rest_of_rfid_cmd;
			}
			else if (*sarg == '~')
			{
				sarg = "001T030000072000xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
				g_szTagDatalen = RFID_FMS00_DATA_SIZE; //just use this right size for this tag
			}
			else if (*sarg == 'b')
			{
				sarg = "001B090000072000xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
				g_szTagDatalen = RFID_FMS00_DATA_SIZE; //just use this right size for this tag
			}
			else if (*sarg == 'n')
			{
				p_rfid_f0 = (td_RFID_F0 *) g_szTagData;
				p_rfid_f0->cFormat = 0;
				p_rfid_f0->cRailType = 0;
				p_rfid_f0->cGroup = 0;
				p_rfid_f0->cSegmentAndRailSide = 128;
				p_rfid_f0->ulSerialNumber = htonul(1234321);
				p_rfid_f0->ulPosition = htonul((unsigned long)1000*50);
				p_rfid_f0->ulSegmentPosition = htonul((unsigned long)50000); //1000*50
				g_szTagDatalen = RFID_F0_DATA_SIZE;
				goto skip_rest_of_rfid_cmd;
			}
			else
			{
				g_szTagDatalen = strlen(sarg);
			}
			if (g_szTagDatalen > 64)
			{
				g_szTagDatalen = 64;
			}
			memcpy(g_szTagData, sarg, g_szTagDatalen);
			g_szTagData[g_szTagDatalen] = 0;
		}
		else
		{
			g_szTagData[64] = 0; //null the end just to be sure
		}
		logf("%s\r\n", "rfid");
		logf("%s\r\n", g_szTagData);
	}
#ifdef DIGOUT_CHNUM_AIRBLASTX
	else if (strcmp(scmd, "abx") == 0)
	{
		if (sarg!=0)
		{
			if (*sarg=='1')
			{
				i=1;
			}
			else
			{
				i=0;
			}
			digOut(DIGOUT_CHNUM_AIRBLASTX, i );
		}
	}
#endif
#ifdef DIGOUT_CHNUM_AIRBLASTY
	else if (strcmp(scmd, "aby") == 0)
	{
		if (sarg!=0)
		{
			if (*sarg=='1')
			{
				i=1;
			}
			else
			{
				i=0;
			}
			digOut(DIGOUT_CHNUM_AIRBLASTY, i );
		}
	}
#endif
#ifdef DIGOUT_CHNUM_LASER_POINTER
	else if (strcmp(scmd, "lp") == 0)
	{
		if (sarg!=0)
		{
			if (*sarg=='1')
			{
				i=1;
			}
			else
			{
				i=0;
			}
			digOut(DIGOUT_CHNUM_LASER_POINTER, i );
		}
	}
#endif
	else if (strcmp(scmd, "ftray") == 0)
	{
		if (sarg != 0)
		{
			if (*sarg == '1')
			{
				g_cUseFastenerTray = 1;
			}
			else
			{
				g_cUseFastenerTray = 0;
			}
		}
		logf("%s %d\r\n", "ftray", g_cUseFastenerTray);
	}
	else
	{
		unmatched_command:
		logf("echo:\"%s\"\r\n", scmd);
	}
	skip_rest_of_rfid_cmd:
	return;
}

void SocketConsoleShowIdentity()
{
	logf("%s %s sn %s\r\n", SMARTTOOL_SUBTYPE_STRING, SYSTEM_VERSION_STRING, g_szSerialNumber);
//logf("Welcome to the system.\r\n");
}
#endif

//for report back hack
void SendInspectResults();

////////////////////////////////////////////////////////////////////////////////
// Mode State Machine
////////////////////////////////////////////////////////////////////////////////
//FIXME0 nodebug
void ModeStateMachine(void)
{
	int i;
	unsigned int ui;
	byte cEntryState;
	byte cprobestatus;
	byte cresult;
	byte ck;
	//byte ck1, ck2;
	//byte cindex;
	//byte corient;
	static byte s_cEstopPrev = 127; //not true or false  // monitors prior estop state
	static uint32 s_uiEstopTime = 0;
	static byte s_cModeStatePrev = MODE_IDLE;
	static uint32 s_uiStartTime;
	static byte s_cProbeState;    // for MODE_PROBE
	//static byte s_cProbeHomeState;  // for MODE_PROBE_HOME
	//static byte s_cProbeAdjustState;  // for MODE_PROBE_ADJUST
	static byte s_cTeachState;  // for MODE_TEACH
	//static byte s_cInspectState;  // for MODE_INSPECT

	float fPreMoveX, fPreMoveY;  // for MODE_POSN
	static float s_fTargetX, s_fTargetY;  // for MODE_POSN
	static float s_fErrLev, s_fErrLevFinal;
	static byte s_cMoveType = MOVETYPE_FAST;
	static byte s_cOneMove, s_cMoveOpt, s_cMoveOptFinal;
	//static byte s_cProbeHomeDir = 3;
	float fDistance, fError;
	//float f;
	float fx, fy, fdx, fdy;
	float fx1, fy1, fx2, fy2;
	unsigned long ulx;
	//long lx;
	char * p_c;
	byte cdm;
	td_oid_probe_status * p_oid_probe_status;
	//int iypos;
	unsigned int uiop;
	char * pc1;
	char * pc2;
	float fddx, fddy;

	cEntryState = g_cModeState; // capture the entry state in case g_cModeState changes within this state machine

	g_cPrintAuth++; //Simple Trick to allow occasional printing for some debugging code
	if (g_cPrintAuth == 80)
	{
		g_cPrintAuth = 0;
	}

	//Check For Estop
	if (g_cDigInEstopSignal == ESTOP_SIGNALED || g_cEstopMCAsserted == 1) //estop signalled, or estop asserted by MC
	{
		//Estop is signaled
		s_uiEstopTime = MS_TIMER;
		// Estop is not clear
		// ALWAYS put us into ESTOP mode when any Estop button is engaged...
		// even if the Rabbit software is trying to command us into a different state somewhere else in this application
		// (for example, if a system error occurs while in estop mode, the ErrHandler will
		// try to force g_cModeState = MODE_IDLE...but since Estop is pressed, we want to stay in Estop mode)
		if (s_cEstopPrev != TRUE)
		{
			// do these actions only once when estop was pressed
			EstopEngageActions();
#ifdef USE_OUTPUT
			logf("ESTOP mode\r\n");
#endif
			s_cEstopPrev = TRUE;
			g_cEstopPrevMode = g_cModeState; //Save this mode to allow possible restore later.
			//clear the sign of the estop unibutton event... we'll need to see it to leave estop
		}
		s_cEstopPrev = TRUE;
		g_cModeState = MODE_ESTOP;
		goto label_past_estop;
	}
//Estop is not signalled
	if (s_cEstopPrev != FALSE)
	{
		//Estop was previously signalled: can we exit?
		if ((MS_TIMER - s_uiEstopTime) < g_ConfigData.uiEStopClearDelay)
		{
			//they have not been clear of one of the previous things long enough
			s_cEstopPrev = TRUE;
			g_cModeState = MODE_ESTOP;
			goto label_past_estop;
		}
		//they have been clear long enough to exit estop
		// estop cleared
		EstopDisengageActions();
#ifdef USE_OUTPUT
		logf("ESTOP clr\r\n");
#endif
		if (g_cEstopPrevMode == MODE_POSN
				|| g_cEstopPrevMode == MODE_PROBE_ADJUST)
		{
			//go back to this mode
			g_cModeState = g_cEstopPrevMode;
		}
		else if (g_cEstopPrevMode == MODE_PROBE)
		{
			//go back to probe
			g_cModeState = MODE_PROBE;
		}
		else
		{
			//go to IDLE
			g_cModeState = MODE_IDLE;
		}
		s_cEstopPrev = FALSE;
	}
	label_past_estop:

	if (g_cModeState != s_cModeStatePrev)
	{
		SmartToolMsgChar(0, STP_ALERT, MINIFT_OID_MODE, g_cModeState);

		//clear any tool offset, and let the new mode set if needed
		//FIXME check   is this really needed? is it wise?
		ClearToolOffset();
	}
	if (g_cAction != g_cActionSent)
	{
		SmartToolMsgChar(0, STP_ALERT, MINIFT_OID_ACTION, g_cAction);
		g_cActionSent = g_cAction;
	}

//Not Probe Home, or it is probe home, but there was a motion obstruction.

//In standard CircCirvetMounting:
//A is left positive X (down)
//B is left negative X (up)
//C is right negative X (up)
//D is right positive X (down)

//Obstruction System
//Every Time the set of obstructions changes... send an event
	if (g_cObstructionEvent != 0)
	{
		AlertObstructionCode(0);
		g_cObstructionEvent = 0;
		if (g_cObstructionCode != 0)
		{
			//This Event Signals at least one Obstruction Change, and there are still obstructions.
			//Note that this could actually be that one obstruction was cleared while another remains.
			if (g_cGravCompStatus == GRAVCOMP_RUNNING)
			{
				g_cGravCompStatus = GRAVCOMP_FAILY;
				BrakeOn();
			}
			else if (g_cFloatStatus == FLOATSTAT_FLOAT)
			{
				DoFloat(FLOAT_UNFLOAT_STOP);
				BrakeOn();
			}
			else if (g_cJogGoalY != JOGSTOP || g_cJogGoalX != JOGSTOP)
			{
				//If Jogging, other logic will stop movement if needed
			}
			else if (g_cModeState == MODE_PROBE_HOME)
			{
				//Probe Home must move around even if obstruction sensors are activated.
				//Probe Motions should have their own response to obstruction codes    //FIXME  This might not be wise.. might be better to let this do move error
			}
			else if (g_cModeState == MODE_POSN
					&& g_cMoveDone == MOVEDONE_FALSE)
			{
				//Moving Now
				if (g_cObstructionCodeNew > 0)
				{
					logf("oss\r\n");
					//new bits set since we last cleared it
					//Should Stop Position
					MCStopPosition();
					g_cMoveDone = MOVEDONE_ERROR;
				}
			}
			else
			{
				//Throw the Brake and set the flag
				BrakeOn();
				g_cMoveDone = MOVEDONE_ERROR;
			}
			g_cObstructionCodeNew = 0; //clear this here always
		}
		//FIXME minor Improvement : I don't like this pattern where each of these actions has to be checked.
		//							I just don't have a better pattern at this time to use.
	}
	if (g_cObstructionWarningEvent != 0)
	{
		AlertObstructionWarningCode(0);
		g_cObstructionWarningEvent = 0;
	}

	if (g_cGravCompStatus == GRAVCOMP_RUNNING)
	{
		uint32 uiElapsed = MS_TIMER - g_uiGravCompTime;
		if (uiElapsed > 8000)
		{
			//timeout
			SmartToolMsgMiniFtMessageCode(0, MINIFT_OID_MCERR, MINIFTMC_GRAVCOMP_FAILURE);
			g_cGravCompStatus = GRAVCOMP_FAILX;
			MCStopPosition(); //??? Is there something better to do?
			g_cFloatStatus = FLOATSTAT_NOFLOAT;
			SmartToolMsgChar(0, STP_ALERT, MINIFT_OID_GRAVCOMP_STATUS, g_cGravCompStatus);
			SmartToolMsgChar(0, STP_ALERT, MINIFT_OID_FLOAT_STATUS, g_cFloatStatus);
		}
	}

	switch (g_cModeState)
	{
	case MODE_IDLE:
		if (s_cModeStatePrev != g_cModeState)
		{
#ifdef USE_OUTPUT
			logf("IDLE M\r\n");
#endif
			LEDOff()
			;
#ifdef CLAMP_SYSTEM
			//DJWR FIXED 20120313-1
			//Shouldn't Unclamp here
			//g_cClampGoal=CLAMP_UNCLAMP;
			//g_cClampGoalSent = 0xFF; //force it to send goal again
#endif
#ifdef FASTENER_SYSTEM
			g_cFastenerArrived=0; //allow clearance, but hope it will redetect it
#endif
		}
#ifdef BEEPSYSTEM
		if (g_cBeepMode != BEEPOFF && g_cBeepMode < BEEPSIGNALS)
		{
			BeepOff()
			;
		}
#endif

#ifdef CENTERVISION
		g_cCenterVisionResult = CENTERVISION_OFF;
#endif
		//FIXME Must review MachineBaseExtension
#ifdef UNIBUTTON
		if (g_cUniButtonEvent == 1)
		{
			// capured a button press
			g_fMachineBaseExtension=g_PosnMode.fLastKnownPosnY;
			g_cUniButtonEvent=0;//clear event
		}
#endif
#ifdef HOMESYSTEM
		if (g_cHomed != HOME_RUNNING)
		{
			g_cAction = ACTION_IDLE;
		}
#else
		//So older Machines can operate
		g_cAction = ACTION_IDLE;
#endif
		break;
	case MODE_PROBE:
		if (s_cModeStatePrev != g_cModeState)
		{
			if (s_cModeStatePrev == MODE_TEACH)
			{
				//Special case
				//Only reason that teach comes here is for
				s_cProbeState = PROBE_WAIT_CALC;
				break;
			}
			s_cProbeState = PROBE_INIT;
			g_cKHolePrbeIndexInput = 0;
			g_cKHolePrbeIndex = 0;
			if (g_cPartPgmStatus != PP_LOADOK)
			{
				//shouldn't be here in probe
				SmartToolMsgMiniFtMessageCode(0, COMMON_OID_NULLOID,
						MINIFTMC_NO_PROGRAM);
				g_cModeState = MODE_IDLE;
				break;
			}
//FIXME asdfasdfasdfasdfasdfasdfasdfasdfasdfasdf
//current problem is going to be the looping here and the fact
//of what happens at init and then later.....
//Must refine flow and also allow repeat with proper intermediate states.....
//so start here with concept of PROBE mode.... IDLE
//etc....

//Manual Loop
//  like probe goes loose, but must get hole before can probe.
// Then button works to activate probe....

//Should I make them pick before I go loose?

//Laser Loop
// less to do at the state....

		}
#ifdef BEEPSYSTEM
		if (g_cBeepMode != BEEPPROBEK && g_cBeepMode < BEEPSIGNALS)
		{
			BeepProbeK()
			;
		}
#endif
		switch (s_cProbeState)
		{
		case PROBE_INIT:
			//Do the initial when probe is reset or when prove is reset after program load
			//Do not do it here everytime it comes into probe state.
			//RecalculateLocatingDirectives();
#ifdef OUTPUT_PROBE_SYS_VERBOSE
			logf("P_I\r\n");
#endif
			g_cAction = ACTION_PROBE;
			if (g_cProbeMethod == PROBE_MANUAL) //only manual should go to float
			{
				if (g_cFloatStatus != FLOATSTAT_FLOAT
						&& g_cGravCompStatus == GRAVCOMP_PASS)
				{
					DoFloat(FLOAT_FLOAT); //always makes it unclamp anyway.
				}
			}
			LEDProbeK()
			;
#ifdef CLAMP_SYSTEM
#ifdef CLAMP_SYSTEM_HD_PISTON
			if (g_cClampState==CLAMP_CLAMP || g_cClampGoal==CLAMP_CLAMP)
			{
				if (g_cALockMode==0)
				{
					//turn off A lock now to get loose A style unclamp
					g_cALock=ALOCK_OFF;
					g_ulClampAUnlock=MS_TIMER;
#ifdef DIGOUT_CHNUM_ALOCK
					digOut(DIGOUT_CHNUM_ALOCK, g_cALock );
#endif
				}
			}
#endif
			if (s_cModeStatePrev != MODE_ESTOP) //don't auto unclamp after estop, even in probe mode
			{
//zxcvzxcv111
				g_cClampGoal = CLAMP_UNCLAMP; //PREVIOUSLY LOOSE
				g_cClampGoalSent = 0xFF; //force it to send goal again
			}
#endif

			g_cKHolePrbeTrigger = 0; //clear trigger
			g_cProbeFlag = 0; //reset flag which indicates if we recevied coords
			g_Probe.cRegistration = REGISTRATION_UNKNOWN;
			SmartToolMsgChar(0, STP_ALERT, MINIFT_OID_PROBE_REGISTRATION, g_Probe.cRegistration);
#ifdef CENTERVISION
			g_cCenterVisionResult = CENTERVISION_OFF;
			g_cCenterVisionInspectType = INSPECT_PROBE;
			g_cCenterVisionRequiredResults = CENTERVISION_CENTER; //default
			//g_fCenterVisionExpectedDiameter set inside system
			g_VisionInspectResults.cContext = 1;					//Probe
			g_VisionInspectResults.lposn = -1;
			g_VisionInspectResults.fXPositionExpected = 0;
			g_VisionInspectResults.fYPositionExpected = 0;
#endif

			if (g_cProbeMethod == PROBE_LASER)
			{
				SetToolOffset(g_ConfigData.LaserSensorOffset.fX,
						g_ConfigData.LaserSensorOffset.fY);
				//FIXME0 ensure that probe after will work with the set tool offset....
				//FIXME0 ensure that the offset will remain set until that capture
			}
			else if (g_cProbeMethod == PROBE_CAM)
			{
				SetToolOffset(g_ConfigData.CamOffset.fX, g_ConfigData.CamOffset.fY);
			}
			else if (g_cProbeMethod == PROBE_INSTANT_OFFSET)
			{
				//g_ConfigData.cToolFlip=Y_POS; //FIXME0000000 this was needed, but for safety and space I removed it again
				SetToolOffset(g_ConfigData.ProbeOffset.fX, g_ConfigData.ProbeOffset.fY);
			}
			else // PROBE_MANUAL PROBE_INSTANT
			{
				ClearToolOffset();
			}
#ifdef USE_HYSTERESIS_FROM_CENTERVISION
			g_fCenterVisionHysX = 0; //clear it
			g_fCenterVisionHysY = 0;//clear it
			SetDriveThroughBacklash(0,0,0);
#endif
#ifdef CENTERVISION
			//Store these with Center Fision
			g_fCenterVisionOffsetX = g_MachineOffset.fX;
			g_fCenterVisionOffsetY = g_MachineOffset.fY;
#endif
			ResetNearestPosition();
			if (g_cProbeMethod == PROBE_ABSOLUTE)
			{
				//This Probe Mode Is Done Immediately
				s_cProbeState = PROBE_CREATE_ABSOLUTE_PROBE;
				break;
			}
			s_cProbeState = PROBE_WAIT;
#ifdef OUTPUT_PROBE_SYS_VERBOSE
			logf("P_W_0\r\n");
#endif
			break;
		case PROBE_WAIT:
			g_cAction = ACTION_IDLE;
			// wait for signal to probe a position
#ifdef UNIBUTTON
			if (g_cUniButtonEvent == 1) //FIXME MED  should button be taken out of some of this code????
			{
				// captured a button press
				g_cKHolePrbeCommandInput=PC_PROBE;
				if (g_cProbeMethod == PROBE_LASER || g_cProbeMethod == PROBE_CAM)
				{
					g_cKHolePrbeCommandInput=PC_MOVE_PROBE;
				}
//FIXME LOW  somehow make button work for old sytle MiniFT
//need to now what K Hole to probe???? IDeal of interface pre select?
//Next variable needed
				g_cKHolePrbeTrigger=1;//trigger
				g_cUniButtonEvent=0;//clear event
			}
#endif

			if (g_cKHolePrbeTrigger == 0)
			{
				//still waiting
#ifdef CENTERVISION
				g_VisionInspectResults.cContext = 0; //Preview
				PreviewCenterVision();
#endif
				break;
			}
			g_cKHolePrbeTrigger = 0; //clear trigger

#ifdef HOMESYSTEM
			if (g_cHomed == HOME_RUNNING)
			{
				SmartToolMsgMiniFtMessageCode(0, MINIFT_OID_PROBE,
						MINIFTMC_PROBE_PREVENTED_HOME);
				break;
			}
#endif
			if (g_cDrillState != DRILLSTATE_IDLE)
			{
				SmartToolMsgMiniFtMessageCode(0, MINIFT_OID_PROBE, MINIFTMC_PROBE_PREVENTED_DRILL);
				break;
			}
#ifdef CLAMP_SYSTEM
			if (g_cClampState != g_cClampGoal || g_cClampState != CLAMP_UNCLAMP)
			{
				SmartToolMsgMiniFtMessageCode(0, MINIFT_OID_PROBE, MINIFTMC_PROBE_PREVENTED_CLAMP);
				break;
			}
#endif

			g_cAction = ACTION_PROBE;

			//Set This Again....
			//FIXME9999999999999999 During testing with the cell controller,
			//there seemed to be a way that it could get here with the offset reset.
			//so as a fix we are going to set the
			//offset with each probe operation right here.
			//I still would like to know how it arrived at this position with the offset cleared,
			//and ensure that the mode and state flow is very stable

			if (g_cProbeMethod == PROBE_LASER)
			{
				SetToolOffset(g_ConfigData.LaserSensorOffset.fX, g_ConfigData.LaserSensorOffset.fY);
				//FIXME0 ensure that probe after will work with the set tool offset....
				//FIXME0 ensure that the offset will remain set until that capture
			}
			else if (g_cProbeMethod == PROBE_CAM)
			{
				SetToolOffset(g_ConfigData.CamOffset.fX, g_ConfigData.CamOffset.fY);
			}
			else if (g_cProbeMethod == PROBE_INSTANT_OFFSET)
			{
				//g_ConfigData.cToolFlip=Y_POS; //FIXME0000000 see note above
				SetToolOffset(g_ConfigData.ProbeOffset.fX, g_ConfigData.ProbeOffset.fY);
			}
			else // PROBE_MANUAL PROBE_INSTANT
			{
				ClearToolOffset();
			}
#ifdef CENTERVISION
			//Store these with Center Fision
			g_fCenterVisionOffsetX = g_MachineOffset.fX;
			g_fCenterVisionOffsetY = g_MachineOffset.fY;
#endif

			//Now Start
			//Copy Input Value
			g_cKHolePrbeCommand = g_cKHolePrbeCommandInput;
			g_cKHolePrbeIndex = g_cKHolePrbeIndexInput;
			//Check Value
			if (g_cKHolePrbeIndex == 0 || g_cKHolePrbeIndex > g_cKHoleCount)
			{
				//bad value
				//Do not allow probe
				SmartToolMsgMiniFtMessageCode(0, COMMON_OID_NULLOID, MINIFTMC_INVALID_KI);
				logf("Err Khole=%d Count=%d\r\n", g_cKHolePrbeIndex, g_cKHoleCount);
				break;
			}

			logf("Probe Trig cmd=%d KI=%d\r\n", g_cKHolePrbeCommand, g_cKHolePrbeIndex);

			//Load the details for this Locating feature
			LoadProbeCommand(g_cKHolePrbeIndex);
			if (g_cProbeCommand == PROBE_COMMAND_INVALID)
			{
				SmartToolMsgMiniFtMessageCode(0, MINIFT_OID_PROBE, g_cProbeCommandMessage);
				break;
			}
			g_cCenterVisionRequiredResults = CENTERVISION_CENTER; //default standard for centervision, and only supported option for some right now
			if (g_cProbeCommand == TKP_PROBE_EDGE_MVEC || g_cProbeCommand == TKP_PROBE_EDGE_VEC)
			{
				g_cCenterVisionRequiredResults = CENTERVISION_EDGE; //all that it does
				if (g_cProbeMethod != PROBE_CAM && g_cProbeMethod != PROBE_INSTANT)
				{
					logf("Only CAM for Edge.\r\n"); //Currently only the Camera Supports Edge Probes
					break;
				}
				logf("%s %d\r\n", "chk", g_cKHolePrbeStart[g_cKHolePrbeIndex]);
				if (g_cKHolePrbeStart[g_cKHolePrbeIndex] == PS_NO_PROBE)
				{
					//error
					logf("nos\r\n");
					s_cProbeState = PROBE_WAIT;
					break;
				}
			}

#ifdef CENTERVISION
			g_cCenterVisionResult = CENTERVISION_OFF;
			g_cCenterVisionInspectType = INSPECT_PROBE;
			//set above g_cCenterVisionRequiredResults=CENTERVISION_CENTER;
			//g_fCenterVisionExpectedDiameter set inside system
			g_VisionInspectResults.cContext = 1;					//Probe
			g_VisionInspectResults.lposn = g_cKHolePrbeIndex; //store this here for the Probe Result
			g_VisionInspectResults.fXPositionExpected = 0;
			g_VisionInspectResults.fYPositionExpected = 0;
#endif

			//Handle Command
			if (g_cKHolePrbeCommand == PC_PROBE)
			{
				if (g_cProbeCommand == 0)
				{
					//This does not actually require a probe, but is a defined location only....
					//Since this is now done, just break...
//FIXMENOW this path not done
					break;
				}
				//OID_PROBE PC_PROBE Kindex  Kick Off the selected probe algorithm to
				//probe for this Kindex at the current location
				//Set Status and Alert now
				g_cKHolePrbeStatus[g_cKHolePrbeIndex] = PS_PROBING;
				//Alert This Probe Status
				SendProbeStatus(STP_ALERT, g_cKHolePrbeIndex); //SPS
				//AlertProbeStatusUpdate(); //Don't bother: start_probe_probe_state will soon do this below
				s_cProbeState = PROBE_PROBE;
#ifdef OUTPUT_PROBE_SYS_VERBOSE
				logf("P_P_0\r\n");
#endif
				goto start_probe_probe_state;
				//to clear status and do other probe probe init
				break;
			}
			if (g_cKHolePrbeCommand == PC_MOVE)
			{
				//OID_PROBE PC_MOVE Kindex  Move To this K Hole Location, but do not probe.
				//Only K Holes with at least an extrapolated position can be be moved to.
				MCGetPositionCounts(); //ensure that move will start with good current position
				s_cProbeState = PROBE_MOVE;
#ifdef OUTPUT_PROBE_SYS_VERBOSE
				logf("P_M_0\r\n");
#endif
				break;
			}
			if (g_cKHolePrbeCommand == PC_MOVE_PROBE)
			{
				//OID_PROBE PC_MOVE_PROBE Kindex  Move To this K Hole Location,
				// and upon arrival do the probe.
				MCGetPositionCounts(); //ensure that move will start with good current position
				s_cProbeState = PROBE_MOVE;
#ifdef OUTPUT_PROBE_SYS_VERBOSE
				logf("P_M_1\r\n");
#endif
				break;
			}
			if (g_cKHolePrbeCommand == PC_MOVE_PROBE_ALL)
			{
				//OID_PROBE PC_MOVE_PROBE_ALL Kindex  Move To this K Hole Location,
				// and upon arrival do the probe.  After, continue PC_MOVE_PROBE_ALL with next Kindex.
				//Should be started at 1 in order to do all KHoles
				/*		This will skip any holes that can't be probed, but keep going.
				 Only K holes that have an approximate location can be probed, and if any K Hole is not found,
				 it will stop the probing process.
				 If It's allready probing, this will just stop probing.

				 This would only be able to probe positions
				 that would have an approximate location.  If holes are in the same flat XY,
				 and they do not have an approximate location, or they do have an approxiamte location,
				 but the source of that location is "Extrapolated", then an appoximate location
				 can be set based on the probe of any two positions in that flat XY.

				 FUTURE DETAIL NOT IMPLEMENTED:
				 Only K holes in the selected patterns, and K Holes of holes in the selected pattern
				 will need to be probed.

				 In the case of manual probe, is there any differece???
				 */

				MCGetPositionCounts(); //ensure that move will start with good current position
				s_cProbeState = PROBE_MOVE;
				break;
			}
			if (g_cKHolePrbeCommand == PC_COMPLETE)
			{
				s_cProbeState = PROBE_COMPLETE;
#ifdef OUTPUT_PROBE_SYS_VERBOSE
				logf("P_C_1\r\n");
#endif
				break;
			}
			if (g_cKHolePrbeCommand == PC_REAPPLY)
			{
				s_cProbeState = PROBE_REAPPLY;
				break;
			}
			//Other command implemented in other parts of the probe system:
			//g_cKHolePrbeCommand PC_STOP na
			//OID_PROBE PC_STOP na 	Stop Current Probe Move or Probe.
			break;
		case PROBE_MOVE:
			if (g_PosnMode.cFreshPosn != X_AND_Y)
			{
				//did not get a fresh position yet
				break;
			}
			//Move to this K Hole Location
			//Only K Holes with at least an extrapolated position can be be moved to.
			if (g_cKHolePrbeCommand == PC_MOVE_PROBE_ALL)
			{
				//allowed to search through
				if (g_cKHolePrbeIndex == 0)
				{
					g_cKHolePrbeIndex = 1; //start here, not at 0
				}
				if (g_cKHolePrbeIndex > g_cKHoleCount)
				{
					//nothing more to do...
					s_cProbeState = PROBE_WAIT;
//FIXME1 how will we know it's done this kind????
// how will we know it's done any kind?????
					break;
				}
				if (g_cKHolePrbeStart[g_cKHolePrbeIndex] == PS_NO_PROBE)
				{
					g_cKHolePrbeIndex++;
					break;
				}
			}
			else
			{
				if (g_cKHolePrbeIndex == 0 || g_cKHolePrbeIndex > g_cKHoleCount)
				{
					//error FIXME1
					//nothing more to do...
					s_cProbeState = PROBE_WAIT;
					break;
				}
//ProbeStart Test
				if (g_cKHolePrbeStart[g_cKHolePrbeIndex] == PS_NO_PROBE)
				{
					//error
					logf("nos\r\n");
					s_cProbeState = PROBE_WAIT;
					break;
				}
//FIXME2 since we don't assume orientation anymore, we need option for that feature to work with this.
//How should it work?
			}

			if (g_cObstructionCode != 0)
			{
				AlertObstructionCode(0);
				g_cKHolePrbeCommand = PC_STOP;
			}
			if (g_cKHolePrbeCommand == PC_STOP)
			{
				//OID_PROBE PC_STOP na 	Stop Current Probe Move or Probe.
				//Nothing to Stop Yet
				logf("*P_O_1\r\n");
				//Load status variable before jumping
				cprobestatus = g_cKHolePrbeStatus[g_cKHolePrbeIndex];
				goto probe_over;
			}
			//Setup Move

			if (g_cMoveDone != MOVEDONE_TRUE)
			{
				//Stop existing move
				MCStopPosition();
				logf("*P_sp\r\n");
			}
			//Check to make sure the move is allowed
			if (g_cClear == 0)
			{
				//not clear... should I unclamp etc...????
			}
//FIXME2 more checks?????s
//?do I need this first?

			//Probe Positions are free of offset values.
//ProbeStart Test
			fx = g_fKHolePrbeStartX[g_cKHolePrbeIndex];
			fy = g_fKHolePrbeStartY[g_cKHolePrbeIndex];

			if (g_cKHolePrbeCommand == PC_MOVE)
			{
				//just a move... do't set status
			}
			else
			{
				//move and probe, so set status now
				g_cKHolePrbeStatus[g_cKHolePrbeIndex] = PS_PROBING;
//FIXME SEVERE ... and if this fails due to ESTOP or something that changes mode, will it clear this?????????
				//Alert This Probe Status
				SendProbeStatus(STP_ALERT, g_cKHolePrbeIndex); //SPS
				AlertProbeStatusUpdate();
			}

			//Make sure it will stop float
			if (g_cFloatStatus == FLOATSTAT_FLOAT)
			{
				DoFloat(FLOAT_UNFLOAT_STOP);
				//FIXME does this need to wait longer??????
				//does this need more coordination?
			}
//FIXME consider if 2 move commands come in with one starting then the next....
//What happens in all cases
			g_cAction = ACTION_MOVE;

#ifdef CENTERVISION
			//Store this for probe results
			g_VisionInspectResults.fXPositionExpected = fx;
			g_VisionInspectResults.fYPositionExpected = fy;
#endif

			logf("probe mv K%d: x=%f,y=%f\r\n", g_cKHolePrbeIndex, fx, fy); //debug
//FIXME2 only true if doing laser ???
					//Need Offset to apply this because we want to move the laser or cam to this position...
			logf("At mv tm MaOffst=%f,%f\r\n", g_MachineOffset.fX, g_MachineOffset.fY);
			ApplyMachineOffset(&fx, &fy);
#ifdef CENTERVISION
			//Store these with Center Fision
			g_fCenterVisionOffsetX = g_MachineOffset.fX;
			g_fCenterVisionOffsetY = g_MachineOffset.fY;
#endif
			//Now apply any additional extra offset if Camera Probing
			if (g_cProbeMethod == PROBE_CAM)
			{
				//Add any specified probe extra offset for the purpose of avoiding clamps or moves to bad locations
				//The Camera might be able to see further, and the center result will be adjusted to counteract this.
				if (g_cProbeExtraOffsetGiven != 0)
				{
					fx1 = g_fProbeExtraOffsetX;
					fy1 = g_fProbeExtraOffsetY;
					logf("%s %f,%f\r\n", "eo", g_fProbeExtraOffsetX, g_fProbeExtraOffsetY);
					cresult = RotateVecDataSetToMachine(g_iKHoleHoleIndex[g_cKHolePrbeIndex], &fx1, &fy1);
					if (cresult == 0)
					{
						//Hole either does not have positioning primary and secondary K locations, or doesn't have approx values for them yet.
						//This is currently required to allow offset probing
						logf("neo\r\n");
						cprobestatus = PS_NO_PROBE;
						goto probe_over;
					}
					fx += fx1;
					fy += fy1;
					logf("%f,%f\r\n", fx1, fy1);
				}
				if (g_cProbeExtraMachineOffsetGiven != 0)
				{
//FIXMENOW Confirm direction of application makes sense
					fx += g_fProbeExtraMachineOffsetX;
					fy += g_fProbeExtraMachineOffsetY;
					logf("%s %f,%f\r\n", "emo", g_fProbeExtraMachineOffsetX, g_fProbeExtraMachineOffsetY);
				}
			}
			logf("probe mv K%d: x=%f,y=%f\r\n", g_cKHolePrbeIndex, fx, fy);	//debug

			//See if this position is allowable
			cresult = CheckObstructionsAndMotionLimits(fx, fy);
			if (cresult != 0)
			{
				//Found a reason not to allow this move
				cprobestatus = PS_NO_PROBE;
				goto probe_over;
				break;
			}

			fdx = fabs(fx - g_PosnMode.fLastKnownPosnX);
			fdy = fabs(fy - g_PosnMode.fLastKnownPosnY);
			logf("LastKnown=%f,%f\r\n", g_PosnMode.fLastKnownPosnX, g_PosnMode.fLastKnownPosnY);
			if (fdx < 2)
			{
				fdx = 2;
			} //Give it some speed since we have that issue...
			if (fdy < 2)
			{
				fdy = 2;
			}

			SetMoveSpeeds(1, 0, fdx, fdy);
			MCSetMoveParams(0.001, 1);

			MCRunPosition(fx, fy);
			s_cProbeState = PROBE_MOVE_WAIT;
#ifdef OUTPUT_PROBE_SYS_VERBOSE
			logf("P_MW_1\r\n");
#endif
			break;
		case PROBE_MOVE_WAIT:
			if (g_cObstructionCode != 0)
			{
				AlertObstructionCode(0);
				g_cKHolePrbeCommand = PC_STOP;
			}
			if (g_cKHolePrbeCommand == PC_STOP)
			{
				logf("*P_S_3\r\n");
				//OID_PROBE PC_STOP na 	Stop Current Probe Move or Probe.
				//Stop!!!!!
				//If Running, then stop...
				MCStopPosition();
//FIXME3 do I need more checking? test using delay to see if stop after stop is a prob
				logf("StopPos\r\n");
				//Load status variable before jumping
				cprobestatus = g_cKHolePrbeStatus[g_cKHolePrbeIndex];
				goto probe_over;
			}

			if (g_cMoveDone == MOVEDONE_FALSE)
			{
#ifdef OUTPUT_PROBE_SYS_VERBOSE
				logf("*P_S_4\r\n");
#endif
				//Not Yet Done
				if (g_cClear == 0)
				{
					MCStopPosition();
					//not clear... should I unclamp etc...????
					logf("StopPos NC\r\n");
					s_cProbeState = PROBE_WAIT;
				}
//FIXME3 more??? >>>>check for conditions that should stop
				break;
			}
			if (g_cMoveDone == MOVEDONE_STOP)
			{
				logf("MD STOP!\r\n");
//FIXME2 what to do???? test?????
				s_cProbeState = PROBE_WAIT;
				break;
			}
			if (g_cMoveDone == MOVEDONE_ERROR)
			{
				logf("MD ERR!\r\n");
//FIXME2 what to do???? test?????
				s_cProbeState = PROBE_WAIT;
				break;
			}
			//Move is done
			if (g_cKHolePrbeCommand == PC_MOVE)
			{
				//don't probe... just return to WAIT...
				if (g_cProbeMethod == PROBE_MANUAL)
				{
					//go to init because of float issue //FIXME000000000 what do I mean by that?
					s_cProbeState = PROBE_INIT;
					break;
				}
				s_cProbeState = PROBE_WAIT;
#ifdef OUTPUT_PROBE_SYS_VERBOSE
				logf("P_W_3\r\n");
#endif
			}
			//must be PC_MOVE_PROBE or PC_MOVE_PROBE_ALL
			start_probe_probe_state:
			//anything depending on this positon should not be probed if this was updated to not be probed.
			ClearPositionsDuringProbe(g_cKHolePrbeIndex);				//SPS
			AlertProbeStatusUpdate();
			//Begin Probe
			s_cProbeState = PROBE_PROBE;
#ifdef OUTPUT_PROBE_SYS_VERBOSE
			logf("P_P_3\r\n");
#endif
			break;
		case PROBE_PROBE:
			g_cAction = ACTION_PROBE;
			if (g_cProbeCommand == 0)
			{
				//This does not actually require a probe, but is a defined location only....
				//Since this is now done, just continue marking that this is done, and moving on.
//FIXMENOW FIXME PROBE....  must complete this case later.....
				break;
			}
#ifdef CENTERVISION
			if (g_cProbeMethod == PROBE_LASER || g_cProbeMethod == PROBE_CAM) //we never return since we don't clamp && g_cCenterVisionResult != CENTERVISION_SUCCESS)
			{
				if (g_cKHolePrbeCommand == PC_STOP)
				{
					//OID_PROBE PC_STOP na 	Stop Current Probe Move or Probe.
					if (g_cCenterVisionResult == CENTERVISION_OFF)
					{
						//hasn't really started yet... just cancel right now
						//Load status variable before jumping
						cprobestatus = g_cKHolePrbeStatus[g_cKHolePrbeIndex];
						goto probe_over;
					}
					//The Soft Stop of Center Vision:
					//1st call will put this into CV_RETURN type move as-if it failed.
					//2nd call
					CancelCenterVision();
					g_cKHolePrbeCommand = PC_PROBE; //don't call here again unless they hit stop again
				}
				ProgressCenterVision();
				if (g_cCenterVisionResult == CENTERVISION_OFF)
				{
					//Allow operation to continue to completion
					break;
				}
				if (g_cCenterVisionResult == CENTERVISION_WORKING)
				{
					//Allow operation to continue to completion
					break;
				}
				//Done Success OR Failure
				if (g_cCenterVisionResult == CENTERVISION_FAILURE)
				{
//ProbeStart Test
					//Go back and wait again
					//just set these to the start value for now...
					//Load Start Value into fx,fy
					fx = g_fKHolePrbeStartX[g_cKHolePrbeIndex];
					fy = g_fKHolePrbeStartY[g_cKHolePrbeIndex];
					g_fKHolePrbeX[g_cKHolePrbeIndex] = fx;
					g_fKHolePrbeY[g_cKHolePrbeIndex] = fy;
					cprobestatus = PS_PROBED_FAILED; //set fail code
					goto probe_failed_verify;
					//really just probe failed here
				}
				//Success
//FIXMENOW need to check actual prbe result types...
				g_cProbeFlag = X_AND_Y;
				if (g_cProbeCommand == KEYWORD_Probe)
				{
					//Standard Probe
					//Place position directly into probe setting
					g_fProbeX = g_VisionInspectResults.fXPosition;
					g_fProbeY = g_VisionInspectResults.fYPosition;

#ifdef OUTPUT_PROBE_SYS
					logf("%s got K: K%d x=%f,y=%f\r\n", "PRB", g_cKHolePrbeIndex, g_fProbeX, g_fProbeY);	//debug
#endif
				}
				else if (g_cProbeCommand == TKP_PROBE_EDGE_MVEC)
				{
					handle_like_machine:
					//This should be done from the prescribed vantage point, but as long as there
					//is an official fx,fy from KREFs, this should intersect edge with the line going through fx,fy and the vector.
					//It does the same thing from any vantage point and with the edge it sees.
					//The result will be checked for approx dist shift though.

#warning "FIXME HIGH COMPLETE CASE WHERE fx,fy are not accurate and it's just an edge vac probe."
					//If there is no official fx,fy for this probe, then it could use the camera center combined with the vac.
					//This is essentially the same thing as saying the to use the current position + the vac.
					//Offset should be ignore in this case
					//THIS CASE IS NOT COMPLETE

					g_VisionInspectResults.cInfo |=
							VisionInfoPositionProbeEdgeVec;

					//NOTE: CODE IMPROVEMENT:  The Start Position can be used for this since it's used to set that approx position, but
					//in order to get back an indicator, we do this now....
					//This also recalculates it, so it's sure to match this value...
					//It would be nice to use the position better, but with Probe being so heavily extended at this time,
					//it doesn't make sense to do more small scale redesign.

					//Large note here about the future!
					//Someday there should be a full scale refactoring into a better flowing pattern
					//where the adjustments and everything all happen in the same way, and there
					//isn't as much of a break for status updates, which have a heavy portion of logic in the OID receipt.
					//Biggest issues:  need for heavy logic in OID handler and some redundant logic bits.
					// single item status difficulties.  lack of clear issue summary.
					//too many returning layers of probe information and lack of clear issue summary.
					//No currently probing item bookmark to remind pendant what is next to do.
					//Probing offset issues... better than they used to be, but never did study current fixed concept
					//vs. late application of offset.
					//Calibration of offsets could be better.
					//No way to review older probes and/or accept older probes still relavant.
					//incomplete probe control system is very nice, but will be better with more implemented.
					//Future system will contain all the benefits.
					//There are many other things I could say about my desire to redesign the probing system with everything we learned.
					//There isn't time or need to do it right now and despite the complexity in some areas, the current pattern works well.

					cresult = CreateMachineDataSetForOneHole(
							g_iKHoleHoleIndex[g_cKHolePrbeIndex], &fx, &fy);

					logf("%f %f %f %f\r\n", fx, fy, g_fProbeMachineVectorX,
							g_fProbeMachineVectorY);
					if (cresult == 0)
					{
						//Send the report that was delayed earlier now.
						SendInspectResults();
						//This K Feature requires accurate location currently
						logf("***noap!uc\r\n");
						cprobestatus = PS_PROBED_FAILED; //set fail code
						goto probe_failed_verify;
						//really just probe failed here
					}

					g_VisionInspectResults.fXPositionExpected = fx;
					g_VisionInspectResults.fYPositionExpected = fy;

					logf("es %d\r\n", g_VisionInspectResults.cEdgeStatus);

					if (g_VisionInspectResults.cEdgeStatus == 0)
					{
						//Send the report that was delayed earlyer now.
						SendInspectResults();
						//problem finding the edge...
						//Go back and wait again
						cprobestatus = PS_PROBED_FAILED; //set fail code
						goto probe_failed_verify;
						//really just probe failed here
					}
					if ((g_VisionInspectResults.cEdgeStatus & 1) > 0)
					{
						//use primary edge
						fx1 = g_VisionInspectResults.fEdgeX1;
						fy1 = g_VisionInspectResults.fEdgeY1;
						fx2 = g_VisionInspectResults.fEdgeX2;
						fy2 = g_VisionInspectResults.fEdgeY2;
						g_VisionInspectResults.cEdgeNote = 1;
					}
					else //must use center edge...
					{
						fx1 = g_VisionInspectResults.fCEdgeX1;
						fy1 = g_VisionInspectResults.fCEdgeY1;
						fx2 = g_VisionInspectResults.fCEdgeX2;
						fy2 = g_VisionInspectResults.fCEdgeY2;
						g_VisionInspectResults.cEdgeNote = 2;
					}
					//These Values Have the offsets removed, and represent feature locations

					//In the future a better pattern could be established
					//Current pattern works, but could be clearer.  Future should be best pattern and very clearly done.
					//I should enumerate the patterns possible for doing offset probing.

					logf("%f %f %f %f\r\n", fx1, fy1, fx2, fy2);
					//Find the Intersection of the edge
					cresult = FindIntersection(fx, fy,
							fx + g_fProbeMachineVectorX,
							fy + g_fProbeMachineVectorY, fx1, fy1, fx2, fy2,
							&fx, &fy);
					logf("fiint%d %f %f\r\n", cresult, fx, fy);
					if (cresult != 0) //no valid intersection
					{
						g_VisionInspectResults.cEdgeNote = 0; //clear this again
						//Send the report that was delayed earlyer now.
						SendInspectResults();
						//Go back and wait again
						//?	//just set these to the start value for now...
						//    g_fKHolePrbeX[g_cKHolePrbeIndex]=fx;
						//    g_fKHolePrbeY[g_cKHolePrbeIndex]=fy;
						cprobestatus = PS_PROBED_FAILED; //set fail code
						goto probe_failed_verify;
						//really just probe failed here
					}
					//Add these values

					g_VisionInspectResults.fXPosition = fx;
					g_VisionInspectResults.fYPosition = fy;
					RecalculatePostionPixels();

					//Send the report that was delayed earlyer now.
					SendInspectResults();

#warning ">>>FIXME SEVERE  must check expected diameter?????"
					/* Do this first with START THEN TEST?????
					 OR DO BELOW???????  YES
					 float g_fProbeExpectedDiameter;
					 */

					g_fProbeX = fx;
					g_fProbeY = fy;
#ifdef OUTPUT_PROBE_SYS
					logf("%s got K: K%d x=%f,y=%f %f,%f,%f,%f,%u\r\n", "PEMV", g_cKHolePrbeIndex, fx, fy,
							g_VisionInspectResults.fXPositionExpected,
							g_VisionInspectResults.fYPositionExpected,
							fx - g_VisionInspectResults.fXPositionExpected,
							fy - g_VisionInspectResults.fYPositionExpected,
							MS_TIMER);
#endif
//zxcvbnm
				}
				else if (g_cProbeCommand == TKP_PROBE_EDGE_VEC)
				{
					//not done yet
//FIXMENOW NOT DONE
#warning "FIXME HIGH  didn't do DATA SET VEC edge"
					g_fProbeMachineVectorX = g_fProbeVectorX;
					g_fProbeMachineVectorY = g_fProbeVectorY;
					goto handle_like_machine;
					cprobestatus = PS_PROBED_FAILED; //set fail code
					goto probe_failed_verify;
					//really just probe failed here
				}
				else
				{
					//Unsupported Probe Style Is not compelte
					logf("probe???\r\n");
					cprobestatus = PS_PROBED_FAILED; //set fail code
					goto probe_failed_verify;
					//really just probe failed here
				}
#ifdef USE_DRIVE_THROUGH_BACKLASH_CODE
#ifdef USE_HYSTERESIS_FROM_CENTERVISION
				SetDriveThroughBacklash(g_ConfigData.cDriveThroughBacklash,g_fCenterVisionHysX/2.0,g_fCenterVisionHysY/2.0);
#else
				SetDriveThroughBacklash(0, 0, 0);
#endif
#endif
				goto done_with_direct_probe;
			}
#endif
			if (g_cKHolePrbeCommand == PC_STOP)
			{
				//OID_PROBE PC_STOP na 	Stop Current Probe Move or Probe.
				//Load status variable before jumping
				cprobestatus = g_cKHolePrbeStatus[g_cKHolePrbeIndex];
				goto probe_over;
			}
			if (g_cProbeMethod == PROBE_MANUAL) //Camera might have needed clamp, but it manages it's own clamping
			{
#ifdef CLAMP_SYSTEM
				if (g_cClampState != CLAMP_CLAMP)
				{
					//must get clamped
					g_uiClampPressure = (int) g_ConfigData.uiPressure;
					g_cClampGoal = CLAMP_CLAMP;
					break;
				}
#endif
			}
			//Probe K Now
			g_cProbeFlag = 0;

			//FIXME0 remove
#ifdef OUTPUT_PROBE_SYS
			logf("pr mo %f,%f\r\n",
					g_MachineOffset.fX, g_MachineOffset.fY);
#endif

			RunProbeK();
			done_with_direct_probe:
			#ifdef CENTERVISION
			g_cCenterVisionResult = CENTERVISION_OFF;
#endif
			s_cProbeState = PROBE_WAITCOORDS;
#ifdef OUTPUT_PROBE_SYS_VERBOSE
			logf("P_WC_4\r\n");
#endif
			s_uiStartTime = MS_TIMER;
			ResetNearestPosition();
			break;
		case PROBE_WAITCOORDS:
			if (g_cKHolePrbeCommand == PC_STOP)
			{
				//OID_PROBE PC_STOP na 	Stop Current Probe Move or Probe.
				//Load status variable before jumping
				cprobestatus = g_cKHolePrbeStatus[g_cKHolePrbeIndex];
				goto probe_over;
			}
			if (g_cProbeFlag != X_AND_Y)
			{
				//did not yet get coordinates
				if ((MS_TIMER - s_uiStartTime) >= (PROBE_TIMEOUT_SEC * 1000L))
				{
//ProbeStart Test
#ifdef OUTPUT_PROBE_SYS
					logf("timeout wt prb res\r\n");
#endif
					//Go back and wait again
					//g_fKHolePrbeX[g_cKHolePrbeIndex]; //Leave These
					//g_fKHolePrbeY[g_cKHolePrbeIndex]; //Leave These
					cprobestatus = PS_PROBED_FAILED; //set fail code
					goto probe_failed_verify;
					//really just probe failed here
					//s_cProbeState = PROBE_INIT;
					//break;
				}
				break;
			}

			g_cProbeFlag = 0; //reset flag
#ifdef OUTPUT_PROBE_SYS
			logf("PROBE coord ack\r\n"); //debug
#endif

			//now beep
			Beep();
			ResetNearestPosition();
			//Store

			//Got Coord Ack, Verify New Probe values.
			fx = g_fProbeX;
			fy = g_fProbeY;
			if (g_ConfigData.cProbeAcceptRequired == 0)
			{
				cprobestatus = PS_PROBED;
			}
			else if (g_cProbeMethod == PROBE_LASER) //we never return since we don't clamp && g_cCenterVisionResult != CENTERVISION_SUCCESS)
			{
				cprobestatus = PS_PENDING_ACCEPTANCE;
			}
			else if (g_cProbeMethod == PROBE_CAM)
			{
				cprobestatus = PS_PENDING_ACCEPTANCE;
			}
			else // if (g_cProbeMethod == PROBE_MANUAL, PROBE_INSTANT, PROBE_INSTANT_OFFSET ...
			{
				cprobestatus = PS_PROBED;
			}
			//FIXME MED LOW Scale Linear Program connects in here, but it's currently not implemented
			//ScaleLinearProgram could be used here

			//Store These Probed Values
			g_fKHolePrbeX[g_cKHolePrbeIndex] = fx;
			g_fKHolePrbeY[g_cKHolePrbeIndex] = fy;
			g_cKHolePrbeStatusWarnings[g_cKHolePrbeIndex] = 0; //clear warnings now
			g_cKHolePrbeStatusDistance[g_cKHolePrbeIndex] = 0; //clear distance warnings until rechecked in future

			//Verify Probe vs. Approximate value.
//ProbeStart Test
			if (g_cKHolePrbeStart[g_cKHolePrbeIndex] == PS_APPROXIMATE)
			{
				//verify with approximate value
				if (g_cCenterVisionRequiredResults != CENTERVISION_EDGE)
				{
					pc1 = "Point Check\r\n";
				}
				else
				{
					pc1 = "Edge Check\r\n";
				}
				logf(pc1);
				//Find Shift
				fdx = fx - g_fKHolePrbeStartX[g_cKHolePrbeIndex];
				fdy = fy - g_fKHolePrbeStartY[g_cKHolePrbeIndex];
				if (fdx == 0)
				{
					//use fast shortcut for common situation
					fError = fabs(fdy);
				}
				else if (fdy == 0)
				{
					//use fast shortcut for common situation
					fError = fabs(fdx);
				}
				else
				{
					fError = sqrt(fdx * fdx + fdy * fdy);
				}
				//Check Against Shift Limit... But all this does is prevent Drilling... it does NOT fail probing.
				if (g_cProbeShiftLimX == MACHINEXY)
				{
					if (fdx < g_fProbeShiftLimXMin
							|| fdx > g_fProbeShiftLimXMax)
					{
						logf("Shift Fail %s %f [%f,%f]\r\n", "mslx", fdx,
								g_fProbeShiftLimXMin, g_fProbeShiftLimXMax);
						g_cKHolePrbeStatusWarnings[g_cKHolePrbeIndex] = 1; //set warning
					}
				}
				if (g_cProbeShiftLimY == MACHINEXY)
				{
					if (fdy < g_fProbeShiftLimYMin
							|| fdy > g_fProbeShiftLimYMax)
					{
						logf("Shift Fail %s %f [%f,%f]\r\n", "msly", fdy,
								g_fProbeShiftLimYMin, g_fProbeShiftLimYMax);
						g_cKHolePrbeStatusWarnings[g_cKHolePrbeIndex] = 1; //set warning
					}
				}
				//see if this vector is needed in data set coordinates
				if (g_cProbeShiftLimX == DATAXY || g_cProbeShiftLimY == DATAXY)
				{
					//rotate error vector to dataset
					fddx = fdx; //first copy
					fddy = fdy;
					cresult = RotateVecMachineToDataSet(
							g_iKHoleHoleIndex[g_cKHolePrbeIndex], &fddx, &fddy);
					if (cresult == 0
							|| g_cRotationContext < PS_PENDING_ACCEPTANCE)
					{
						//Hole either does not have positioning primary and secondary K locations, or doesn't have then probed yet.
						//This is currently required for the checking of a shift limit
						logf("neo\r\n");
						cprobestatus = PS_NO_PROBE;
						goto probe_over;
					}
					if (g_cProbeShiftLimX == DATAXY)
					{
						if (fddx < g_fProbeShiftLimXMin
								|| fddx > g_fProbeShiftLimXMax)
						{
							logf("Shift Fail %s %f [%f,%f]\r\n", "slx", fddx,
									g_fProbeShiftLimXMin, g_fProbeShiftLimXMax);
							g_cKHolePrbeStatusWarnings[g_cKHolePrbeIndex] = 1; //set warning
						}
					}
					if (g_cProbeShiftLimY == DATAXY)
					{
						if (fddy < g_fProbeShiftLimYMin
								|| fddy > g_fProbeShiftLimYMax)
						{
							logf("Shift Fail %s %f [%f,%f]\r\n", "sly", fddy,
									g_fProbeShiftLimYMin, g_fProbeShiftLimYMax);
							g_cKHolePrbeStatusWarnings[g_cKHolePrbeIndex] = 1; //set warning
						}
					}
				}
				if (g_fProbeMaxDistShift > 0)
				{
					if (fError > g_fProbeMaxDistShift)
					{
						logf("Shift Fail %s %f>%f\r\n", "mds", fError, fdy,
								g_fProbeMaxDistShift);
						g_cKHolePrbeStatusWarnings[g_cKHolePrbeIndex] = 1; //set warning
					}
				}
				//Now Check Against Limits
				//Currently using ApproxLocationError to judge only non edge finding approx locations
				//    using MaxEdgeShiftProbeAccept to judge edges....
				//FIXME FUTURE Eventually want to move towards one number to check approx, and another to check derived location shifts.
				if (g_cCenterVisionRequiredResults != CENTERVISION_EDGE)
				{
					fDistance = g_ConfigData.fApproxLocationError;
					pc2 = "ale";
				}
				else
				{
					fDistance = g_ConfigData.fMaxEdgeShiftProbeAccept;
					pc2 = "mespa";
				}
				if (fError > fDistance)
				{
					logf("Shift Fail %s %f>%f\r\n", pc2, fError, fDistance);
					cprobestatus = PS_PROBED_APPROX_DIFF_FAILURE; //set fail code
					logf("Probe Fail\r\n");
					//FIXME Minor Add Print Control For these so that we can reclaim this space but turn on more testing when needed
					logf(" %f", g_fKHolePrbeStartX[g_cKHolePrbeIndex]);
					logf(" %f\r\n", g_fKHolePrbeStartY[g_cKHolePrbeIndex]);
					logf(" %f", fx);
					logf(" %f\r\n", fy);
					//too far from approximate value
					goto probe_failed_verify;
				}
			}
			//Verify Probe vs. Expected Distances.
			//FIXME0000000000000 notice same-XY assmption in all code here

			g_cDistanceErrorFlagSent = 0;
			g_cDistanceErrorShown = 0;

			//Check all position distances from this new probe, and for anything that was probed before and had failed.
			//This is all the checking that is needed:  If anything did not fail before, then only the new probe location
			//could cause it to fail, and if anything had failed before checking it again allows us to see if it passes now,
			//but we only need to check the other item

			//In the case when a distance limit is set, only other holes that are that close will be rechecked
			//but this make sense because only holes that close are required to pass a distance check in general.

			//While doing the 1st pass on ck, mark other holes that have seen improvement due to this new ck probe,
			//and give them another chance to check.

			//First Check this new probe.
			ck = g_cKHolePrbeIndex;
			CheckKLocationDistances(ck, 1);
			//Any other K Locations failing the distance check are now set to PS_PROBED_KHOLE_DIFF_FAILURE
			//Any other K Locations passing the distance check to this one location are now marked 0xFF to show that they need to be rechecked
			//completely to see if they are now valid.
			ck = 1;
			while (ck <= g_cKHoleCount)
			{
				if (g_cKHolePrbeStatusDistance[ck] == 0xFF) //this one was marked for RECHECK
				{
					//This didn't fail due to any of the previous checks, so recheck now.
					CheckKLocationDistances(ck, 0);
					//Any other K Locations failing the distance check are now set to PS_PROBED_KHOLE_DIFF_FAILURE
					//They would no longer marked 0xFF and can be skipped.
					//If this hole was found to be good, it should be set to the StatusAttempt value which
					//shows how it did without the distance error checking.
				}
				ck++;
			}
			//Count Distance Errors
			g_cProbeDistanceErrors = 0;
			ck = 1;
			while (ck <= g_cKHoleCount)
			{
				if (g_cKHolePrbeStatusDistance[ck] == 1)
				{
					g_cProbeDistanceErrors++;
				}
				ck++;
			}
			logf("kderr=%d\r\n", g_cProbeDistanceErrors);

			//If the Start is not approx, it may be updated by this probe also
			if (g_cKHolePrbeStart[g_cKHolePrbeIndex] != PS_APPROXIMATE)
			{
				//May Update ProbeStart
				if (g_cKHolePrbeStart[g_cKHolePrbeIndex] == PS_NO_PROBE || cprobestatus >= PS_PROBED)
				{
					g_fKHolePrbeStartX[g_cKHolePrbeIndex] = g_fKHolePrbeX[g_cKHolePrbeIndex];
					g_fKHolePrbeStartY[g_cKHolePrbeIndex] = g_fKHolePrbeY[g_cKHolePrbeIndex];
					g_cKHolePrbeStart[g_cKHolePrbeIndex] = cprobestatus;
					SendProbeStart(STP_ALERT, g_cKHolePrbeIndex); //SPS
				}
			}
			//Done Getting and Validating Probe
			probe_failed_verify:
			probe_over:
			//ProbeStart Test
			//Anything still marked PS_PROBING was a PS_PROBED_FAILED
			if (cprobestatus == PS_PROBING)
			{
				cprobestatus = PS_PROBED_FAILED;
			}
			g_cKHolePrbeStatus[g_cKHolePrbeIndex] = cprobestatus;
			logf("status=%d\r\n", cprobestatus);

			//Alert The Probe Status For This Probe
			SendProbeStatus(STP_ALERT, g_cKHolePrbeIndex);       //SPS
			AlertProbeStatusUpdate();

			g_cKHolePrbeTrigger = 0; //clear trigger again at this point in case it was triggered while we were already probing

//0a) attentd to all these functions and the need for O known.....

//FIXME make sure none of these (like the next one) will fail.... must have pattern set to 1 for old functions
			//send this after sending probe result...
			//can assume distance is cause of failure if probe is false when this is processed.

			/*
			 FIXME  build the remaining details of the new distance numbers
			 when no 1st probe is set...  distance = 0
			 when 1 probe is set only,  then distance expected =
			 distance to probe from last
			 vs distance to probe from target probe hole....
			 when 2 probes are set, then distance should be distance to probe target expected....
			 where probe target expected is set by prior probes.....

			 More Genericly:
			 When nothing is probed, distance = 0
			 When 1st probe is done for a flat xY set, store the values into 1st probe values.
			 when probing next time, distance will be from this double posn machine and dataset to
			 the target.....
			 When 2nd probe is done for a flat XY set, store the values into 2nd probe value.
			 Create approximate or calculated positions for all remaining in the set....
			 Now distance is distance from target position...

			 Most Genericly:
			 In the case that there is an approximate position, distance sent back is to that position.
			 In the case that there is not: distance sent back is distance from the last probed
			 position, end expected distance is calculated to go ith it.

			 PROGRESS REPORT:   this function does the correct job now, but only when approx is set for current probe hole....
			 */

			//Undo any probe specific motion and clamp state
			//Camera can operate clamp on it's own, but it doesn't hurt to say UNCLAMP here if arriving via another path
#ifdef CLAMP_SYSTEM
			if (g_cClampState == CLAMP_CLAMP || g_cClampGoal == CLAMP_CLAMP)
			{
#ifdef CLAMP_SYSTEM_HD_PISTON
				if (g_cALockMode==0)
				{
					//turn off A lock now to get loose A style unclamp
					g_cALock=ALOCK_OFF;
					g_ulClampAUnlock=MS_TIMER;
#ifdef DIGOUT_CHNUM_ALOCK
					digOut(DIGOUT_CHNUM_ALOCK, g_cALock );
#endif
				}
#endif
				if (g_cProbeMethod == PROBE_MANUAL)
				{
					g_cClampGoal = CLAMP_LOOSE; // Helps to come up off button without grabbing button
				}
				else
				{
					g_cClampGoal = CLAMP_UNCLAMP;
				}
			}
#endif

			ProbeModeSelectAndSetGlobalRotation();

			if (g_cKHolePrbeCommand == PC_STOP)
			{
				//return to init if probe was stopped
				s_cProbeState = PROBE_INIT;
#ifdef OUTPUT_PROBE_SYS_VERBOSE
				logf("P_I_xs\r\n");
#endif
				break;
			}

			//If this was not a success, or not yet accepted, then can't auto continue or complete
			if (cprobestatus < PS_PENDING_ACCEPTANCE)
			{
				//return to init if probe was not a success
				//loop back for next probe
				s_cProbeState = PROBE_INIT;
#ifdef OUTPUT_PROBE_SYS_VERBOSE
				logf("P_I_5\r\n");
#endif
				break;
			}

			if (cprobestatus == PS_PENDING_ACCEPTANCE)
			{
				//return to init and wait for person to accept the probe
				//This happens one at a time currently, but we could make it so that it can auto probe all,
				//with each image being sent back for approval.  We are not doing that now.
				s_cProbeState = PROBE_INIT;
#ifdef OUTPUT_PROBE_SYS_VERBOSE
				logf("P_I_7\r\n");
#endif
				break;
			}

			//Recalculate Positions During Probe
			RecalculatePositionsDuringProbe(g_cKHolePrbeIndex);			//SPS
			AlertProbeStatusUpdate();
			//If Probe is now complete, proceed to the PROBE_COMPLETE
			CheckProbeComplete();
			if (g_cProbeComplete == 1)
			{
				if (g_ConfigData.cProbeFlags & AUTO_COMPLETE)
				{
					//force complete for now
					s_cProbeState = PROBE_COMPLETE;
					break;
				}
				s_cProbeState = PROBE_INIT; //loop back for next
#ifdef OUTPUT_PROBE_SYS_VERBOSE
				logf("P_I_6\r\n");
#endif
				break;
			}

			//Special Command allows looping over k holes
			if (g_cKHolePrbeCommand == PC_MOVE_PROBE_ALL) //FIXME this path would need more testing to be used
			{
				//should move an probe next one too
				ck = g_cKHolePrbeIndex;
				ck++;
				if (ck <= g_cKHoleCount)
				{
					g_cKHolePrbeIndex = ck;
					s_cProbeState = PROBE_MOVE; //loop back for next
					//FIXME2 consider making this go to wait and making that do the jump in....
					//or even special state that also has final detect...
					break;
				}
				//at the end, but not complete... got back to init and wait further probes
			}

			if (g_ConfigData.cProbeFlags & AUTO_MOVE_PROBE)
			{
				//should move an probe next one too
				ck = g_cKHolePrbeIndex;
				ck++;
				if (ck <= g_cKHoleCount)
					if (g_cKHolePrbeStatus[ck] < PS_PROBED)
					{
						//next is not probed.
						g_cKHolePrbeCommandInput = PC_MOVE_PROBE;
						//Store the probe KIndex
						g_cKHolePrbeIndexInput = ck;
						//Assuming that it's waiting for a trigger
						g_cKHolePrbeTrigger = 1; //trigger
						//Go directly to wait
#ifdef OUTPUT_PROBE_SYS_VERBOSE
						logf("P_W_8\r\n");
#endif
						s_cProbeState = PROBE_WAIT; //loop back for next
						break;
					}
				//at the end, but not complete... got back to init and wait further probes
			}

#ifdef OUTPUT_PROBE_SYS_VERBOSE
			logf("P_I_8\r\n");
#endif
			s_cProbeState = PROBE_INIT; //loop back for next
			break;
		case PROBE_REAPPLY:
			//Use Last Probe Values
			//When a program is probed and completed, the status is backed up.
			//It can be restored and reapplied.
			//This is only allowed if the number of KHoles is the saame
			if (g_cKHoleCount == g_cKHoleLastCount)
			{
				ck = 1;
				while (ck <= g_cKHoleCount)
				{
//FIXME MINOR  REAPPLY NOT TESTED
					g_fKHolePrbeX[ck] = g_fKHoleLastPrbeX[ck];
					g_fKHolePrbeY[ck] = g_fKHoleLastPrbeY[ck];
					g_cKHolePrbeStatus[ck] = g_cKHoleLastPrbeStatus[ck];
					g_cKHolePrbeStart[ck] = g_cKHoleLastPrbeStatus[ck];
					g_fKHolePrbeStartX[ck] = g_fKHoleLastPrbeX[ck];
					g_fKHolePrbeStartY[ck] = g_fKHoleLastPrbeY[ck];
					//FIXME  g_cKHolePrbeStatusWarnings[g_cKHolePrbeIndex]=; //I'm leaving this out because I do't use REAPPLY yet
					ck++;
				}
			}
			//Alert All Probe Start and Status
			SendAllProbeStart(STP_ALERT);					//SPSALL
			SendAllProbeStatus(STP_ALERT);					//SPSALL
			AlertProbeStatusUpdate();
			//Now they have these values.
			//They may sone Done Probe and use them or they may use them, but camera probe again.
			s_cProbeState = PROBE_INIT; //loop back for next
			break;
		case PROBE_CREATE_ABSOLUTE_PROBE:
			g_cAction = ACTION_CALC;
			//Make Everything probed at it's absolute location
			ck = 1;
			while (ck <= g_cKHoleCount)
			{
				cprobestatus = PS_PROBED;
				ui = g_iKHoleHoleIndex[ck];
				fx = g_fRawDataX[ui];
				fy = g_fRawDataY[ui];
				g_cKHolePrbeStatus[ck] = cprobestatus;
				g_cKHolePrbeStatusWarnings[ck] = 0; //clear warning
				g_fKHolePrbeX[ck] = fx;
				g_fKHolePrbeY[ck] = fy;
				//Now Alert Result
				p_oid_probe_status = (td_oid_probe_status *) &g_STPtxMsg.p_cObjectValue;
				p_oid_probe_status->cKIndex = ck;
				p_oid_probe_status->fX = fx;
				p_oid_probe_status->fY = fy;
				p_oid_probe_status->cStatus = cprobestatus;
				SmartToolMsg(0, STP_GET_RESP, MINIFT_OID_PROBE_STATUS, sizeof(td_oid_probe_status), (char*) p_oid_probe_status);
				ck++;
			}
			//ClearGolbalRotation(); //just clear two variables
			g_cRotationKP = 0;
			g_cRotationKS = 0;
			g_cRotationContext = 0;
			//Adjust Current Rotation Displays based on probe status.
			ProbeModeSelectAndSetGlobalRotation();
			AlertProbeStatusUpdate();
			//FIXME  Improvement:
			//We could have skipped the true rotation and called a faster version
			//of create the dataset since we are one to one with the machine coords.
			//but currently still going the normal path.
			//I will have to research what to skip and what not to skip to do it.
			s_cProbeState = PROBE_COMPLETE;
			break;
		case PROBE_COMPLETE:
			g_cAction = ACTION_CALC;
			//Check that all are Probed
			CheckProbeComplete();
			if (g_cProbeComplete == 0)
			{
				//not all are probed
				s_cProbeState = PROBE_INIT; //loop back for next
				break;
			}
			//when all k holes done, must do posn cals
			StartCreateMachineDataSet();
			s_cProbeState = PROBE_WAIT_CALC;
			//fall through
		case PROBE_WAIT_CALC:
			g_cAction = ACTION_CALC;
			//Complete Position Calculation
			//MACHINEPOINTS
			if (g_uiCreateMachineDataSetIndex < g_iHoleCount)
			{
				//Still Need to Complete this
				CreateMachineDataSet();
				break;
			}
			if (g_uiCreateMachineDataSetHashIndex < g_iHoleCount)
			{
				//Still Need to Create Hash Lists
//FIXME1 true orient code
				CreatePosnHashList();
				break;
			}
			g_cProbeCalculated = 1;
			//copy over K Reprobe Hole Data
			g_cKHoleLastCount = 0;
			if (g_cKHoleCount <= MAXNUM_KHOLES_REPROBE)
			{
				ck = 1;
				while (ck <= g_cKHoleCount)
				{
					g_fKHoleLastPrbeX[ck] = g_fKHolePrbeX[ck];
					g_fKHoleLastPrbeY[ck] = g_fKHolePrbeY[ck];
					g_cKHoleLastPrbeStatus[ck] = g_cKHolePrbeStatus[ck];
					//FIXME 	=g_cKHolePrbeStatusWarnings[ck]; //not done this yet NOT USING REAPPLY YET
					//FIXME     =g_cKHolePrbeStatusDistance[ck]; //not done using reapply yet????
					//					if we continue to do this only after distance is good, then distance is always 0, but
					//					if we implement reapply for partial probe status, then we would need to restore all and recheck distance
					ck++;
				}
				g_cKHoleLastCount = g_cKHoleCount;
			}
			//First Position Mode
			//Clear These Variables : FIXME minor May be good to also do this when doing start over...
			g_PosnMode.iCurPosnIndex = 0;
			g_PosnMode.cFirstMove = 1; //Trigger 1st move in Posn Init and Indicates that Next must go to Current, and not add or subtract one.  Other paths will overwrite this as usual.

			//INSTEAD, GO FROM HERE TO POSITION MODE.
			//// go back and just wait to see if user wants to re-capture K2
			//s_cProbeState = PROBE_INIT;
			g_cModeState = MODE_POSN;

			break;

		}
		break;
	case MODE_PROBE_HOME:
		break;

	case MODE_PROBE_ADJUST:
		#ifdef SUPPORT_PROBE_ADJUST
		if (s_cModeStatePrev != g_cModeState)
		{
			s_cProbeAdjustState = PROBE_ADJUST_INIT;
		}
		if (g_cJogX!=JOGSTOP || g_cJogY!=JOGSTOP)
		{
			//if jogging, don't do anything
			break;
		}
#ifdef BEEPSYSTEM
		if (g_cBeepMode!=BEEPPROBEADJUST && g_cBeepMode<BEEPSIGNALS)
		{	BeepProbeAdjust();}
#endif
		switch (s_cProbeAdjustState)
		{
			case PROBE_ADJUST_INIT:

			LEDProbeK(); //Use same K Flash for Probe Adjust
			g_Probe.cProbeAdjustTrig = FALSE;
			g_Probe.cGotProbeAdjust = 0;//reset flag which indicates if we recevied coords

			s_cProbeAdjustState = PROBE_ADJUST_WAIT;
			break;

			case PROBE_ADJUST_WAIT:
			// wait for user to set MINIFT_OID_K2 or to press the red button
#ifdef UNIBUTTON
			if (g_cUniButtonEvent == 1 )
			{
				// capured a single button press
				g_Probe.cProbeAdjustTrig = TRUE;
				g_cUniButtonEvent=0;//clear event
			}
#endif
			if (g_Probe.cProbeAdjustTrig == TRUE )
			{
				g_Probe.cProbeAdjustTrig = FALSE;
				g_Probe.cGotProbeAdjust = 0; //reset flag which indicates if we recevied Probe Adjust coords
				RunProbeAdjust();
				logf("PA: sent req fr coords\r\n");//debug
				s_cProbeAdjustState = PROBE_ADJUST_WAITCOORDS;
			}
			break;
			case PROBE_ADJUST_WAITCOORDS:
			//	Wait here until we see the signal that the results have been processed.
			if (g_Probe.cGotProbeAdjust == X_AND_Y)
			{
				g_Probe.cGotProbeAdjust = 0; //reset flag
				g_iProbedIndex = CalcProbeAdjust();// Returns dataset point index at probe point, -1 if out of tolerance.
				//	Always beep, even if out of tolerance.
				Beep();
				ResetNearestPosition();
				if( g_iProbedIndex >= 0 )// If a dataset point is within tolerance, ...
				{
					//Once PROBE_ADJUST has been performed, registration is exact.
					g_Probe.cRegistration = REGISTRATION_EXACT;
					SmartToolMsgChar(p_STPSession, STP_ALERT, MINIFT_OID_PROBE_REGISTRATION, g_Probe.cRegistration);

					//Update OperationHistory for the probed hole.
					AddOpHistory( g_iProbedIndex, OP_PROBE);
					StartCreateMachineDataSet();
					s_cProbeAdjustState = PROBE_ADJUST_WAIT_CALC;
				}
				else
				{
					SmartToolMsgMiniFtMessageCode(0, MINIFT_OID_MCERR, MINIFTMC_PROBE_ADJUST_OUT_OF_TOLERANCE);
					logf("PA: Prbd pnt too far frm dataset loc.\r\n");
					s_cProbeAdjustState = PROBE_ADJUST_INIT; // go back and just wait to see if user wants to re-capture
				}
			}
			break;
			case PROBE_ADJUST_WAIT_CALC:
			//Complete Position Calculation
			//MACHINEPOINTS
			if (g_uiCreateMachineDataSetIndex < g_iHoleCount)
			{
				//Still Need to Complete this
				CreateMachineDataSet();
				break;
			}
			if (g_uiCreateMachineDataSetHashIndex < g_iHoleCount)
			{
				//Still Need to Create Hash Lists
				CreatePosnHashList();
				break;
			}
			//return probe adjust machine state to init
			s_cProbeAdjustState = PROBE_ADJUST_INIT;
			//Clear These Variables : FIXME minor May be good to also do this when doing start over...
			g_PosnMode.iCurPosnIndex=0;
			g_PosnMode.cFirstMove=1;//Trigger 1st move in Posn Init and Indicates that Next must go to Current, and not add or subtract one.  Other paths will overwrite this as usual.
			//Go to Posn Mode
			g_cModeState=MODE_POSN;// Return to position mode.
			break;
		}
		break;
#else
		//Probe Adjust Not Supported
		g_cModeState = MODE_IDLE;
		break;
#endif

	case MODE_POSN:
		//When Entering Mode Posn, Must Do Init Process.
		if (s_cModeStatePrev != g_cModeState)
		{
			//	On entry to POSNMODE, initiate the appropriate aaction based on EEOPTION:
			//		DRILLFILL:	Move to 1st hole
			//		default:	Do not initiate motion.
			g_cPosnModeState = POSNMODE_INIT;
#ifdef OUTPUT_POSITION_STATES
			logf("@PM_INIT\r\n");
#endif

			if (g_cProbeCalculated == 0)
			{
				//shouldn't be here in posn
				SmartToolMsgMiniFtMessageCode(0, COMMON_OID_NULLOID,
						MINIFTMC_PROBE_INCOMPLETE);
				g_cModeState = MODE_IDLE;
				break;
			}

			//if they are floating when they come into position mode, then stop floating
			if (g_cFloatStatus == FLOATSTAT_FLOAT)
			{
				logf("Unfloat stp\n");
				DoFloat(FLOAT_UNFLOAT_STOP);
			}
			g_cFloatExitModePosnState = 0; //clear this flag when starting position mode... we need to see only floats that stop during position mode.
		}

		if (g_cJogX != JOGSTOP || g_cJogY != JOGSTOP)
		{
			//if jogging, don't do anything
			break;
		}
		if (g_cGravCompStatus == GRAVCOMP_RUNNING)
		{
			//if grav comp, don't do anything
			break;
		}

		if (g_cFloatStatus == FLOATSTAT_FLOAT)
		{
			if (g_cPosnModeState == POSNMODE_INIT)
			{
				//still waiting for float to exit since entered mode.
				logf("Wait Flt Ex\n");
				break;
			}

			//Always do this  //FIXME dfnow fix indent later
			{
#ifdef CLAMP_SYSTEM
				if (g_cClampState == CLAMP_CLAMP)
				{
					//must be unclamped for float
					g_cClampGoal = CLAMP_UNCLAMP;
				}
#endif
			}

			//floating: Can't do position mode while floating, so avoid code below...
			// unibutton: exit float
#ifdef UNIBUTTON
			if (g_cUniButtonEvent == 1)
			{
				// capured a single button press
				// exit float mode
				DoFloat(FLOAT_UNFLOAT);
				g_cUniButtonEvent=0;//clear event
			}
#endif
			break;
		}
		if (g_cFloatExitModePosnState == POSNMODE_MOVE_NEAREST)
		{
			//when they stop floating, move to nearest
			g_cPosnModeState = POSNMODE_MOVE_NEAREST;
#ifdef OUTPUT_POSITION_STATES
			logf("@PM_MOVE_NEAREST\r\n");
#endif
			g_cFloatExitModePosnState = 0; //clear flag
		}

		switch (g_cPosnModeState)
		{
		case POSNMODE_INIT:
			if (g_iDrillDir == DRILLDIR_ATOB)
			{
				// so now we start from KA and traverse positive through the rotated data
				g_PosnMode.iStartPosnIndex = 0;  // start at start
			}
			else if (g_iDrillDir == DRILLDIR_BTOA)
			{
				// so now we start from KB and traverse negative through the rotated data
				g_PosnMode.iStartPosnIndex = g_iHoleCount - 1; // start at end
			}

			InitPosnMode();
			g_cMoveDone = MOVEDONE_TRUE; // intialize to true since carriage is not moving when we first enter posn mode
			g_cPosnModeState = POSNMODE_STARTWAITNEXTPOSN;

			LEDOff()
			;
#ifdef CLAMP_SYSTEM
#ifdef CLAMP_SYSTEM_HD_PISTON
			if (g_cClampState==CLAMP_CLAMP || g_cClampGoal==CLAMP_CLAMP)
			{
				if (g_cALockMode==0)
				{
					//turn off A lock now to get loose A style unclamp
					g_cALock=ALOCK_OFF;
					g_ulClampAUnlock=MS_TIMER;
#ifdef DIGOUT_CHNUM_ALOCK
					digOut(DIGOUT_CHNUM_ALOCK, g_cALock );
#endif
				}
			}
#endif
			if (s_cModeStatePrev != MODE_ESTOP) //don't auto unclamp after estop
			{
//zxcvzxcv111
				g_cClampGoal = CLAMP_UNCLAMP; //Previously LOOSE
				g_cClampGoalSent = 0xFF; //force it to send goal again
			}
#endif
			// Clear Inspection
			g_cPositionInspection = 0;

			g_PosnMode.cOnCurPosn = 0; //not on the position or unsure at this init time
			if (g_iProbedIndex >= 0)
			{
				//Move Probe Index
				//After successful probe index, auto move to the probed hole
				g_PosnMode.iCurPosnIndex = g_iProbedIndex;
				g_iProbedIndex = -1;
				//Move to this hole
				SpecifyGotoPosnAgain();
				break;
			}
#ifdef AUTO_MOVE_FIRST_POSITION
			if (g_PosnMode.cFirstMove == 1)
			{
				//Make First move to Start Posn
				g_PosnMode.iCurPosnIndex = g_PosnMode.iStartPosnIndex;
				//Auto move to 1st hole
				SpecifyGotoPosn(GOTOPOSN_NEXT, 0);
			}
#endif
			//FIXME Minor Might still be better improbed by setting specific next position and making that clearer from
			break;
		case POSNMODE_STARTWAITNEXTPOSN:
			#ifdef OUTPUT_POSITION_STATES
			logf("@PM_STARTWAITNEXTPOSN\r\n");
#endif
			//Ensure that StartProcess is clear
			g_cStartProcess = 0;
			g_cAction = ACTION_READY;
			//DRILL_DIRECT_PROCESS_AND_OVERRIDE
			//Clear this
			g_cDrillLoadProcessAndOverride = DRILL_LOAD_PROCESS_NOT_NEEDED;
			g_cOverrideSent = 0; //Don't need to recalculate again, but must send the override after each Start.
			g_ulMoveStart = 0; //clear move start time
			g_cFastDoneFlag = 0; //clear
			g_cPendingFastenerFaultUpdate = 0;

			logf("station %c g %c\r\n", g_cStationCode[g_cStation],
					g_cStationCode[g_cStationGoal]);

#ifdef OUTPUT_POSITION_STATES
			logf("@PM_WAITNEXTPOSN\r\n");
#endif
			g_cPosnModeState = POSNMODE_WAITNEXTPOSN;
			//Fall through
		case POSNMODE_WAITNEXTPOSN:
			//waiting for move signal

#ifdef BEEPSYSTEM
			if (g_cBeepMode != BEEPOFF && g_cBeepMode < BEEPSIGNALS)
			{
				BeepOff()
				;
			} //Turn Off Beeper
#endif

#ifdef OUTPUT_POSITION_INFO
			//This is like idle time, so it's a good time to output the position information
			//we do not print out during program load.
			ShowProgramRotationOutput();
#endif

#ifdef HOMESYSTEM
			if (g_cHomed != HOME_RUNNING)
			{
				g_cAction = ACTION_READY;
			}
#endif

			//Rot Sensor CAXIS Test Code
			//Normally we don't need to read this all the time but for testing it is helpful
			//ReadRailRotationSensor(); //FIXME debug code only

			// If waiting for move, then move must be done, so clear flags if needed
			if (g_cMoveDone != MOVEDONE_TRUE)
			{
				if (g_cMoveDone == MOVEDONE_ERROR)
				{
					g_PosnMode.cOnCurPosn = 0;			//moving actions always mean not on position
					LEDOff()
					;
					SendCurPosnInd(0, STP_ALERT); //Send Cur Posn Index, though it isn't on a hole
				}
				MCStopPosition(); //ensure it will stop motion
				g_cMoveDone = MOVEDONE_TRUE;
			}

#ifdef UNIBUTTON
			// unibutton: go to next hole
			if (g_cUniButtonEvent == 1)
			{
				// capured a button press
				// move to next posn in the PP
				ProfilePoint("BPRESS");
				SpecifyGotoPosn(GOTOPOSN_NEXT,0);
				ProfilePoint("Mv CMD");
				g_cUniButtonEvent=0;//clear event
				g_cAutoMove=0;//turn off auto move if anything requested the move directly
			}
#endif

			if (g_cStartProcess == 0 && g_PosnMode.cDoMoveTrig != TRUE
					&& g_cAutoMove != AUTOMOVE_MOVE)
			{
				//nothing is triggered, so do not continue
				break;
			}

			if (g_cDrillState != DRILLSTATE_IDLE)
			{
				//Shouldn't really be here.
				g_cStartProcess = 0;
				g_PosnMode.cDoMoveTrig = FALSE;
				logf("%d\r\n", -1001);
				break;
			}

#ifdef FASTENER_STP
			if (g_cFastenerMode==FASTENER_MODE_HOMING)
			{
				//Wait until this is complete
				break;
			}
			if (g_FastenerFault.cSeverity >= FAULT_SEVERITY_BLOCK_DRILL)
			{
				//Operator must clear this fault
				//Clear Start Process and break
				logf("%s %d\r\n","faf",g_FastenerFault.cSeverity);
				g_cStartProcess = 0;
				break;
			}
			if (g_FastenerFault.cSeverity!=0 || g_cFastenerState==FASTENERSTATE_FAULT)
			{
				logf("f %d\r\n",g_FastenerFault.cSeverity);
				FastenerSTPSet(FASTENER_OID_FAULT_CLEAR);
				g_FastenerFault.cSeverity = 0;
			}

			if (g_cPendingFastenerFaultUpdate==1)
			{
				//Wait for this, but don't clear startprocess...  continue after this is done
				break;
			}
			if (g_cFastenerMode!=FASTENER_MODE_NO_FASTENER && g_cFastenerMode!=FASTENER_MODE_READY)
			{
				//no auto home... must be home
				g_cStartProcess = 0;
//									//ask fastener to home
//								if (FastenerSTPSetUInt(FASTENER_OID_HOME,0)==1)
//								{
//									logf("FH\r\n");
//								}
				logf("%d\r\n",-1002);
				break;
			}
			logf("%s %d\r\n","F!",g_cFastenerState);

			if (g_cFastenerState==FASTENERSTATE_NONE)
			{
				//continue
			}
			else if (g_cFastenerState==FASTENERSTATE_LOADED)
			{
				//continue
			}
			else if (g_cFastenerState==FASTENERSTATE_FAULT)
			{
				//wait for this to
				g_cPendingFastenerFaultUpdate = 1;
				logf("%d\r\n",-1003);
				break;
			}
			else
			{
				//Fastener shouldn't be doing anything at this time
				//FIXME URGENT add message
				g_cStartProcess = 0;
				logf("%d\r\n",-1004);
				break;
			}
#endif

			if (g_cStartProcess == 1)
			{
				if (g_PosnMode.cOnCurPosn != 1)
				{
					g_cStartProcess = 0;
					break;
				}
				if (g_cStationGoal == STATION_LASERPOINTER)
				{
					g_cStationGoal = STATION_UNSPEC; //when hitting start, laser pointer is not a valid station this will later cause the start station to be used.
				}

#ifdef GENCIRCMFTX
				g_cStationPlanDrill = 1;
#ifdef FASTENER_STP
#warns "FASTENER_STP shouldn't be on for any CIRCMFT...."
#endif
#ifdef DRILL_DIRECT_READY
#warns "DRILL_DIRECT_READY shouldn't be on for any CIRCMFT...."
#endif
#else

				//On Position
				//STATION-SYSTEM
				//A Station Goal is currently set
				//When Starting, Always Update Plan
				UpdateStationPlan();
				//Advance the plan depending on operations selected and allowed
				AdvanceStations();

				//Check Drill Faults
				if (g_DrillFault.cSeverity >= FAULT_SEVERITY_BLOCK_DRILL)
				{
					//Operator must clear this fault
					//Clear Start Process and break
					logf("%s %d\r\n","drf",g_FastenerFault.cSeverity);
					g_cStartProcess = 0;
					break;
				}

#endif

				if (g_cStationPlanDrill == 1)
				{
					logf("uic=%u\r\n", g_uiPositionOpsCache);
					if (g_ConfigData.cDrillHoleOneTime != 0)
					{
						//Check History for this hole
						if ((g_uiPositionOpsCache
								& (OP_DRILL_STARTED | OP_DRILL_SUCCESS)) > 0)
						{
//logf("*pd\r\n");
							g_cStartProcess = 0;
							SmartToolMsgMiniFtMessageCode(0, COMMON_OID_NULLOID,
									MINIFTMC_POSITON_PREVIOUSLY_DRILLED);
							break;
						}
					}
					if (g_ConfigData.cAllowDrillBeyondShiftLimits != 1)
					{
						//Check for Probe Warnings
						if ((g_uiPositionOpsCache & OP_PROBE_WARNINGS) != 0)
						{
//logf("*pw\r\n");
							g_cStartProcess = 0;
							SmartToolMsgMiniFtMessageCode(0, COMMON_OID_NULLOID,
									MINIFTMC_PROBE_WARNING_DRILL_PREVENTED);
							break;
						}
					}
//FIXME this is where checking for ATHOME should be done?????????????????

					//Now Clear any fault.	No Need to Echo the Clear
#ifdef DRILL_DIRECT_READY
					SmartDrillSTPSet(SMARTDRILL_OID_FAULT_CLEAR);
#endif
					g_DrillFault.cSeverity = 0;
				}

#ifdef FASTENER_STP
				if (g_cStationPlanFill == 1 || g_cStationPlanRemove == 1)
				{
					if ( g_cStation == STATION_PICKUP || g_cStation == STATION_FILL)
					{
						//force long path for pickup and fill...
						//FIXME FASTENER this should be done a better way
						g_cStation = STATION_UNSPEC;
#warns "FAS  station repeat issue temp code"
					}
				}
#endif

//FIXME HIGH
//#warnt "SEVERE: proc check this path not good"
//Proc Check is OK, but main issue is when should I do tool and proc check... when selected to drill? when station drill?
//It's not consistent.....
//This is because a new process could have been loaded (or new tool)
//while we sat at this position and it will now spin up....

#ifdef GENCIRCMFTX
				goto direct_to_action_exit;
#endif
				logf("station %c g %c\r\n", g_cStationCode[g_cStation],
						g_cStationCode[g_cStationGoal]);
				if (g_cStation == g_cStationGoal)
				{
					//Send Hole Parameters again. Always must send before action.
					SendHoleParameters(0, STP_ALERT);
					//Go to Action state
					direct_to_action_exit:
#ifdef OUTPUT_POSITION_STATES
					logf("@PM_ACTION\r\n");
#endif
					g_cPosnModeState = POSNMODE_ACTION;
					g_cSawDrillMode = 0; //clear
					//And before going into action
					//Advance Station Prep if needed
					PrepareStations();
//FIXME HIGH  this is about PrepareStations Improvement Plan.... not done yet   commenting out warnt for now
//#warnt "Should be able to redo these next"
					break;
				}
				//going to have to start move, so start move logic
				SpecifyGotoPosnAgain();
				//jump all the way to the common handler which operates at the end of the cycle in waitcycle
				goto handle_process_gotoposn;
			}
			if (g_cAutoMove == AUTOMOVE_MOVE)
			{
				//start with motion
#ifdef OUTPUT_POSITION_STATES
				logf("@PM_WAITCYCLE\r\n");
#endif
				g_cPosnModeState = POSNMODE_WAITCYCLE;
				break;
			}

			if (g_PosnMode.cDoMoveTrig != TRUE)
			{
				//not triggered
				break;
			}
			//Move was triggered
			g_PosnMode.cDoMoveTrig = FALSE;
			logf("MT\r\n");

#ifdef MINIFT_TRIAL_STATION_CODE_A0
			//ALWAYS want to start motion with the start station
			//This code allows the first station to be used below.... unless sticking to laser pointer mode.
			if (g_cStationGoal != STATION_LASERPOINTER)
			{
				g_cStationGoal = STATION_UNSPEC;
			}
#endif
			logf("station %c g %c\r\n", g_cStationCode[g_cStation],
					g_cStationCode[g_cStationGoal]);
#ifdef GENCIRCMFTX
//Consider this for ALLPATTERN candidate
			if (g_cDrillState != DRILLSTATE_IDLE)
			{
				//no message because the pendant should be preventing the buttons
				return;
			}
#endif

			g_cPosnModeState = POSNMODE_TOOLCHECK;
#ifdef OUTPUT_POSITION_STATES
			logf("@PM_TOOLCHECK\r\n");
#endif
			ProfilePoint("PM_TLCHCK");
			//fall through for immediate tool check
		case POSNMODE_TOOLCHECK:
			//Set Action
			g_cAction = ACTION_RUNNING;
			//There is a race case where the goto posn could be set back to unknown...
			//To avoid that issue, if it happens, just abort the move, and no harm is done.
			if (g_PosnMode.iGotoPosnIndex < 0)
			{
				//Return to main POSNMODE State
				goto POSNMODE_CLEAR_AND_STARTWAITNEXTPOSN;
			}

			//CycleTime
			g_ulLastMoveStart = g_ulMoveStart;

			//start time for move
			g_ulMoveStart = MS_TIMER;

			if (g_ulLastMoveStart != 0)
			{
				logf("@@%s %u\r\n", "L", (g_ulMoveStart - g_ulLastMoveStart));
			}

			//load default move type;
			s_cMoveType = g_ConfigData.cMoveType;

			ProfilePoint("TLCHCK");

			//Clear the Drill Results (Will get new results after it's done.)
			g_HoleResultData.iHoleNumber = -1;
			g_HoleResultData.iHoleResult = HR_UNKNOWN;
			logf("khr\r\n");
			//announce the goto position
			SendGoalPosnInd(0, STP_ALERT);

			//lookup the previous ops history
			g_uiPositionOpsCache = GetOpHistory(g_PosnMode.iGotoPosnIndex);
			logf("*oh%d = %u\r\n", g_PosnMode.iGotoPosnIndex,
					g_uiPositionOpsCache);
			logf("station %c g %c\r\n", g_cStationCode[g_cStation],
					g_cStationCode[g_cStationGoal]);

			//Extract Data For Hole
			//Set g_HoleData g_HoleParam and several other reference variables
			//load the hole information for the gotoposn
			//struct copy //FIXME PORTMED later improve the storage
			g_HoleData = g_HoleDataArray[g_PosnMode.iGotoPosnIndex]; //struct copy

			// LOAD THIS INFORMATION:
			//typedef struct
			//{
			//  byte cKInd;
			//	byte cKPri;
			//    byte cKSec;
			//    byte cTool;
			//    byte cProcess;
			//    byte cFastener;
			//    byte cPattern;
			//	unsigned int uiOps; //array of bits used to indicate what operations are specified for this hole (optional for some input formats)
			//    unsigned int uiStack;
			//    int iCountersinkAdjust;
			//	  unsigned int d2uDiameter
			//} td_HoleData;

			LoadHoleParameters(); //this uses g_HoleData and loads more
			SendHoleParameters(0, STP_ALERT); //send the 1st time
			//DRILL_DIRECT_PROCESS_AND_OVERRIDE
			g_cDrillLoadProcessAndOverride = DRILL_LOAD_PROCESS_NOT_NEEDED; //until moving... don't start loading process and overrides
			//Clear Override Calculation Flag
			g_cOverrideCalculated = 0;
			//Clear drill mode flag
			g_cSawDrillMode = 0; //clear

			if (g_cLastSearchedRequiredTool != g_cRequiredTool)
			{
				SendReqToolSearch(STP_GET);
			}

			//Clear some IO that is usually off
#ifdef DIGOUT_CHNUM_LASER_POINTER
			if (g_cLaserPointer==1)
			{
				//turn off
				g_cLaserPointer = 0;
				digOut(DIGOUT_CHNUM_LASER_POINTER, g_cLaserPointer );
			}
#endif

			//STATION-SYSTEM

			//This code shouldn't really be part of the station system here, but we'll refactor this part later.
			//Never let them disable drill operation for running
#ifndef FASTENER_SYSTEM
#ifndef SEAL_SYSTEM
			g_ConfigData.uiProcessOperations |= OP_DRILL;
#endif
#endif
			g_ConfigData.uiProcessOperations |= OP_INSPECT; //allow this op, regardless of startup variable
			//

			UpdateStationPlan(); //Set Plan Flags
			if (g_cStationGoal == STATION_UNSPEC)
			{
				//select 1st possible station
				g_cStationGoal = g_cStartStation; //always set goal to start
			}
			if (g_cStartProcess == 1)
			{
				//Advance the plan depending on operations selected and allowed
				AdvanceStations();
			}
			logf("station %c g %c\r\n", g_cStationCode[g_cStation],
					g_cStationCode[g_cStationGoal]);

			if (g_cStationGoal != STATION_INSPECT
					&& g_cStationGoal != STATION_LASERPOINTER)
			{
				if (g_cStartProcess == 0)
				{
					//A Normal Move always just goes to the 1st selected station
					g_cStationGoal = g_cStartStation;
					logf("ss\r\n");
				}
			}
			if (g_cStationGoal == STATION_UNSPEC)
			{
				logf("w uns\r\n"); //warning unspec should not be showing up at this point...
				g_cStationGoal = g_cStartStation;
			}

			if (g_cStation != g_cStationGoal)
			{
				logf("%c\r\n", '!');
			}
			logf("station %c g %c\r\n", g_cStationCode[g_cStation],
					g_cStationCode[g_cStationGoal]);

			if (g_cStationGoal == STATION_DRILL)
			{
				//clear any offset
				ClearToolOffset();
#ifdef LS_USE_DUAL_PROBE_Y_COMP
				//FIXME0 only if laserprobing.... .. not mechanical probe  ... if mech probe, then must do diff
				SetToolOffsetWithYComp(
						g_ConfigData.DrillOffset1.fX,
						g_ConfigData.DrillOffset1.fY,
						g_ConfigData.DrillOffset1.fYExtension,
						g_ConfigData.DrillOffset2.fX,
						g_ConfigData.DrillOffset2.fY,
						g_ConfigData.DrillOffset2.fYExtension
				);
				FIXME000000 FUTURE TEST THIS OFFSET SYSTEM + also make work with all paths like mecho
#endif
				s_cMoveType = g_ConfigData.cMoveType;
			}
#ifdef SEAL_SYSTEM
			else if (g_cStationGoal == STATION_SEAL)
			{
				SetToolOffset(g_ConfigData.OffsetSeal.fx,g_ConfigData.OffsetSeal.fy);
				s_cMoveType = MOVETYPE_ROUGH;
			}
#endif
#ifdef FASTENER_SYSTEM
			else if (g_cStationGoal == STATION_PICKUP)
			{
				if (g_cStartProcess==1)
				{
					logf("fs\r\n");
					//must be doing a pickup from a fastener tray
					FastenerSTPSetUInt(FASTENER_OID_FASTENER_SELECT,1);
				}
//FIXME MED still need to add check for outputs
				SetToolOffset(g_ConfigData.OffsetFill.fx,g_ConfigData.OffsetFill.fy);
				s_cMoveType = MOVETYPE_ROUGH;
			}
			else if (g_cStationGoal == STATION_FILL)
			{
				//both fill and remove go to this location
				SetToolOffset(g_ConfigData.OffsetFill.fx,g_ConfigData.OffsetFill.fy);
				s_cMoveType = MOVETYPE_ROUGH;
			}
#endif
			else if (g_cStationGoal == STATION_INSPECT
					&& g_cStationPlanInspect == 1)
			{
				if (g_cStartProcess == 1)
				{
					//Use the standard auto inspect method specified when running the process
					//Other special position inspections run a position move inspection set to the CommandInspectMethod
					g_cInspectMethod = g_ConfigData.cInspectMethod;
				}
#ifdef CENTERVISION_ANALOG_POINT_LASER
				if (g_cInspectMethod == INSPECT_LASER)
				{
					SetToolOffset(g_ConfigData.LaserSensorOffset.fX,
							g_ConfigData.LaserSensorOffset.fY);
				}
#endif
#ifdef CENTERVISION_CAM
				if (g_cInspectMethod == INSPECT_CAMERA) //Inspection is with Camera
				{
					SetToolOffset(g_ConfigData.CamOffset.fX,
							g_ConfigData.CamOffset.fY);
				}
#endif
#ifdef CENTERVISION
				//Store these with Center Fision
				g_fCenterVisionOffsetX = g_MachineOffset.fX;
				g_fCenterVisionOffsetY = g_MachineOffset.fY;
#endif
				//if (g_cInspectMethod == INSPECT_MANUAL) //Don't bother clearing the offset or setting an offset
			}
			else if (g_cStationGoal == STATION_LASERPOINTER)
			{
				//recycle this value for this for now....
#warning "MUST ADD NEW OID FOR LASER SENSOR OFFSET TO Y???????????????????"
				SetToolOffset(g_ConfigData.LaserSensorOffset.fX,
						g_ConfigData.LaserSensorOffset.fY);
				s_cMoveType = g_ConfigData.cMoveType;
#ifdef DIGOUT_CHNUM_LASER_POINTER
				//And turn on Laser Pointer now
				g_cLaserPointer = 1;
				digOut(DIGOUT_CHNUM_LASER_POINTER, g_cLaserPointer );
#endif
			}

			g_cStation = g_cStationGoal;
			ResetNearestPosition();

			//logf("curpos=%d gotopos=%d\r\n",g_PosnMode.iCurPosnIndex,g_PosnMode.iGotoPosnIndex);

			//In the past this would skip tool check....
			// if (g_PartPgmInfo.cTeachModeData!=1 && g_cScaleBasedOnProbe!=1)
			//now the program allows any tool and process.....

			//Verify Tool
			// (If Teach Mode data, go right to moving since tool is not part of saved program currently)
			ProfilePoint("VerT");
			//Hole is g_PosnMode.iGotoPosnIndex
			//If moving to position the drill do tool check.
			if (g_cStationGoal == STATION_DRILL)
			{

				g_cShowPrevDrilledMessage = 0;
				if (g_cStartProcess == 1)
				{
					if (g_ConfigData.cDrillHoleOneTime != 0)
					{
						if ((g_uiPositionOpsCache
								& (OP_DRILL_STARTED | OP_DRILL_SUCCESS)) > 0)
						{
							//Do not do process
							logf("*pd\r\n");
							g_cStartProcess = 0;
							g_cShowPrevDrilledMessage = 1; //show after arrival
							//but do move
						}
					}
					if (g_ConfigData.cAllowDrillBeyondShiftLimits != 1)
					{
						//Check for Probe Warnings
						if ((g_uiPositionOpsCache & OP_PROBE_WARNINGS) != 0)
						{
							logf("*pw\r\n");
							g_cStartProcess = 0;
							g_cShowPrevDrilledMessage = 2; //show the other message MINIFTMC_PROBE_WARNING_DRILL_PREVENTED
						}
					}
				}

				//VerifyEnable == 1 : Must Pass Before Move
				if (g_ConfigData.cToolVerifyEnable == 1)
				{
					VerifyAndAlertTool();
					if (g_cToolLoaded == 0)
					{
						//proper tool was required and status was not OK.
						//Return to main POSNMODE State
						//Attempt to Clear any tool for movement
#ifdef CLAMP_SYSTEM
						g_cClampGoal = CLAMP_UNCLAMP;
						g_cClampGoalSent = 0xFF; //force it to send goal again
#endif
						goto POSNMODE_CLEAR_AND_STARTWAITNEXTPOSN;
						break;
					}
				}

			} //End GENHD only tool check when station is drill

#ifdef CLAMP_SYSTEM
			//Now setup for wait clamp
			//Goal is always unclamp now for moving
			g_cClampGoal = CLAMP_UNCLAMP;
#ifdef FASTENER_STP
			//							g_cClampGoalSent = 0xFF; //force it to send goal again

			if ((g_cStationGoal==STATION_PICKUP && g_cStationPlanFill==1) ||
					(g_cStationGoal==STATION_FILL && g_cStationPlanFill==1 && g_cClampState!=CLAMP_UNCLAMP))
			{
				//ask for special position instead of unclamp
				f=g_fFastenerTrayClampPickupHeight;
				//convert f to Clamp Axis Coordinates
				//C Pos = Rail Z Pos - MFT Tool Z Base - Clamp Zero Offset;
				f = f - g_ConfigData.fToolZBase - g_fNACClampZOffset;
				//Set the target position
				NACPositionTarget(f);
				logf("%f\r\n",f);
				g_cClampGoal = CLAMP_POSITION;
			}
#endif
#endif

			ProfilePoint("WClmp");
			g_cPosnModeState = POSNMODE_WAITCLAMP;

#ifdef OUTPUT_POSITION_STATES
			logf("@PM_WAITCLAMP\r\n");
#endif
			//fall through for immediate wait
		case POSNMODE_WAITCLAMP:
			#ifdef OUTPUT_POSITION_STATES
			//	logf("@PM_WAITCLAMP\r\n");
#endif

			//Advance Station Prep if needed
			//Many stations will start as they wait for unclamp and/or move to position
			PrepareStations();
//FIXME want to move this up to one time action before and after clamp

#ifndef GENCIRCMFTX
			//DRILL_DIRECT_PROCESS_AND_OVERRIDE
			//If moving to position, go ahead and start the process and override loading now
			if (g_cStationGoal == STATION_DRILL)
			{
				if (g_cDrillLoadProcessAndOverride==DRILL_LOAD_PROCESS_NOT_NEEDED)
				{
					g_cDrillLoadProcessAndOverride=DRILL_LOAD_PROCESS_NEEDED; //Now that there is a delay, authorize process loading to begin
				}
			}
#endif

#ifdef CLAMP_SYSTEM
			if (g_cClampGoal == CLAMP_POSITION)
			{
				if (g_cClampState != CLAMP_POSITION)
				{
					break;
				}
			}
			else if (g_cClampState != CLAMP_UNCLAMP)
			{
				//not ready to continue to the rest of this state.

				//NOTE: This would be where we would lock out clamp changes if we had a lockout feature
				//      For this reason there is no point to recheck clamp below during moving.
				//		If we need lockout, just add that feature
				g_cClampGoal = CLAMP_UNCLAMP; //ask to unclamp again
				break;
			}
#endif
			logf("%s %d\r\n", "aex", g_cClampState);

#ifdef FASTENER_STP
			if (g_cStartProcess==1)
			{
				if (g_cStationGoal==STATION_PICKUP && g_cStationPlanFill==1)
				{
					logf("pr\r\n");
					//once in clamp position, OK to signal MINIFT_OID_PRESENT_READY
					//c=FastenerSTPSet(FASTENER_OID_PRESENT_READY);
					FastenerSTPSet(FASTENER_OID_PRESENT_READY);
//FIXME MED check output like others. do it here and in other places not checking.
					//also must now order A Axis to normal
				}
			}
#endif

			ProfilePoint("PreM");
			g_cPosnModeState = POSNMODE_PREMOVE;
#ifdef OUTPUT_POSITION_STATES
			logf("@PM_PREMOVE\r\n");
#endif
			//fall through for immediate premove
		case POSNMODE_PREMOVE:
			#ifdef OUTPUT_POSITION_STATES
			logf("\r\n>>>>STARTING MOVE>>>>>>>\r\n");
			//	logf("@PM_PREMOVE\r\n");
#endif
			ProfilePoint("PM_PreM");
			//Set Action
			g_cAction = ACTION_MOVE;
			//MACHINEPOINTS
			//set fx,fy
			fx = g_fRawDataMX[g_PosnMode.iGotoPosnIndex];
			fy = g_fRawDataMY[g_PosnMode.iGotoPosnIndex];
			s_fTargetX = fx;
			s_fTargetY = fy;

			if (g_cStationGoal == STATION_PICKUP)
			{
				if (g_fFastenerTrayPickupPosition == 0)
				{
					//Did not get a tray pickup position
//FIXME MINOR  Fastener add message
#warning "Fas FIXME MINOR add message"
					logf("f %d\r\n", 0);
					//Return to main POSNMODE State
					goto POSNMODE_CLEAR_AND_STARTWAITNEXTPOSN;
					break;
				}
				s_fTargetY = g_fFastenerTrayPickupPosition;
				logf("%f\r\n", s_fTargetY);
			}

			ApplyMachineOffset(&s_fTargetX, &s_fTargetY);
			//FIXME: ORTHO AND OTHER ADJUSTMENTS MUST BE APPLIED LAST.....
			//I'm missing them here....
#ifdef OUTPUT_POSITION_INFO
			logf("MACHINE pos %d( %.4f, %.4f )\r\n",g_PosnMode.iGotoPosnIndex,s_fTargetX,s_fTargetY);
#endif

			ProfilePoint("DONEROT");

			//now find movement parameters
			switch (s_cMoveType)
			{
			case MOVETYPE_ORIGINAL:
				default:
				//Use MiniFt Tolerance Settings for Both moves
				s_cOneMove = 0;
				s_fErrLev = g_ConfigData.fPosnTolerance;
				s_cMoveOpt = 0;
				s_fErrLevFinal = g_ConfigData.fPosnTolerance;
				s_cMoveOptFinal = 0;
				break;
			case MOVETYPE_DIRECT:
				//Use Single Move with MiniFt Tolerance Settings
				s_cOneMove = 1;
				s_fErrLev = g_ConfigData.fPosnTolerance;
				s_cMoveOpt = 0;
				break;
			case MOVETYPE_FAST:
				//Let 1st move be as fast as possible,
				// but final move is with MiniFt Tolerance Settings
				s_cOneMove = 0;
				s_fErrLev = 0.01;
				s_cMoveOpt = 1;
				s_fErrLevFinal = g_ConfigData.fPosnTolerance;
				s_cMoveOptFinal = 0;
				break;
			case MOVETYPE_ROUGH:
				//Single Move with larger room for error
				s_cOneMove = 1;
				s_fErrLev = 0.01;
				s_cMoveOpt = 1;
				break;
			case MOVETYPE_VROUGH:
				//Single Move with large room for error
				s_cOneMove = 1;
				s_fErrLev = 0.1;
				s_cMoveOpt = 2;
				break;
			}
			if (g_cStationGoal == STATION_INSPECT
					&& g_cStationPlanInspect == 1)
			{
				s_cOneMove = 1;
				s_fErrLev = 0.01;
				s_cMoveOpt = 1;
#ifdef CENTERVISION
				g_cCenterVisionResult = CENTERVISION_OFF; //set it back to the start state.
				//g_cCenterVisionInspectType Set by OID when the inspection is activated
//FIXME SEVERE TEST POSITION INSPECTION
				g_cCenterVisionRequiredResults = CENTERVISION_CENTER; //default
				g_fCenterVisionExpectedDiameter =
						((float) g_HoleData.d2uDiameter) / 10000;
				g_VisionInspectResults.cContext = 2; //Posn
				g_VisionInspectResults.lposn = g_PosnMode.iGotoPosnIndex;
				g_VisionInspectResults.fXPositionExpected = fx; //copy position without machine offset
				g_VisionInspectResults.fYPositionExpected = fy;
#endif
			}

			//set premove if needed
			fPreMoveX = 0.0;
			fPreMoveY = 0.0;
			if (s_cOneMove == 0)
			{
				//calculate magnitude and sign of premove based entirely on current grav comp parameters.
				CalculatePreMove(&fPreMoveX, &fPreMoveY);
			}

			ProfilePoint("SND MV");

			// move the carriage
			g_PosnMode.cOnCurPosn = 0; //moving actions always mean not on position
			LEDOff()
			;
			//find change in x and y
			fdx = s_fTargetX + fPreMoveX - g_PosnMode.fLastKnownPosnX;
			fdy = s_fTargetY + fPreMoveY - g_PosnMode.fLastKnownPosnY;

			//Check to see if premove would be in the direction of obstruction, or out of limits
			cresult = CheckObstructionsAndMotionLimits(s_fTargetX + fPreMoveX,
					s_fTargetY + fPreMoveY);
			if (cresult != 0)
			{
				//Found a reason not to allow this move
				g_cPosnModeState = POSNMODE_STOP_MOVEMENT;
				break;
			}
			//Check to see if final move would be in the direction of obstruction, or out of limits
			cresult = CheckObstructionsAndMotionLimits(s_fTargetX, s_fTargetY);
			if (cresult != 0)
			{
				//Found a reason not to allow this move
				g_cPosnModeState = POSNMODE_STOP_MOVEMENT;
				break;
			}

			fdx = fabs(fdx);
			fdy = fabs(fdy);

			//set the speed to make approximately linear movement:
			ProfilePoint("SET SPDS");
			SetMoveSpeeds(1, 0, fdx, fdy);
			ProfilePoint("SET PRMS");
			MCSetMoveParams(s_fErrLev, s_cMoveOpt);

			//NOW MOVE
			ProfilePoint("MV");
			MCRunPosition(s_fTargetX + fPreMoveX, s_fTargetY + fPreMoveY);
#ifdef CLAMP_SYSTEM_NAC_STP
			if (g_cNAC==1)
			{
				//This Code is the Standard Clamp Code to clamp via NAC
				//Let Y Extension always be positive. NAC will have a table that accomodates the range it gets for YNEG and YPOS
				iypos=(int)(fabs(s_fTargetY)*1000);
				//Dont have values for surface posn or angle currently...
				//  send -16 which is out of range... NAC is not setup to use these values now
				//	send 1 ms which means as fast as possible... (0 means stop)
				cresult=NACClampPrepMove(iypos,-16,-16,1);
				logf("CPrepM y=%f->%d s=%d\r\n",s_fTargetY,iypos,cresult);
			}
#endif
			ProfilePoint("MV CMD");

#ifdef OUTPUT_POSITION_COMMAND_INFO
			logf("g_PosnMode.iCurPosnIndex=%d\r\n",g_PosnMode.iCurPosnIndex);
			logf("Premove command::PosnX=%.3f;PosnY=%.3f;XQ #POSN,0\r\n",
					s_fTargetX+fPreMoveX,
					s_fTargetY+fPreMoveY);						//debug
#endif

			BeepMove()
			; //Start Beeps

			s_uiStartTime = MS_TIMER;

			//If moving to position, go ahead and start the process and override loading now
			//DRILL_DIRECT_PROCESS_AND_OVERRIDE
			if (g_cStationGoal == STATION_DRILL)
			{
				if (g_cDrillLoadProcessAndOverride
						== DRILL_LOAD_PROCESS_NOT_NEEDED)
				{
					g_cDrillLoadProcessAndOverride = DRILL_LOAD_PROCESS_NEEDED; //Now that there is a delay, authorize process loading to begin
				}
			}

			if (s_cOneMove != 0)
			{
				//no premove-finalmove, just do one move
				g_cPosnModeState = POSNMODE_FINALMOVEWAIT; //Jump here and wait in the final move area
#ifdef OUTPUT_POSITION_STATES
				logf("@PM_FINALMOVEWAIT\r\n");
#endif
				ProfilePoint("PM_FNLMVWT");
			}
			else
			{
				//wait for premove to be done
				g_cPosnModeState = POSNMODE_PREMOVEWAIT;
#ifdef OUTPUT_POSITION_STATES
				logf("@PM_PREMOVEWAIT\r\n");
#endif
				ProfilePoint("PM_PRMVWT");
			}
			break;
		case POSNMODE_PREMOVEWAIT:
			#ifdef UNIBUTTON
			if (g_cUniButtonEvent == 1)
			{
				// capured a single button press while carriage was moving...user wants to halt
				g_cPosnModeState = POSNMODE_STOP_MOVEMENT;
				g_cUniButtonEvent=0;//clear event
			}
#endif

			if (g_cMoveDone == MOVEDONE_TRUE)
			{ // g_cMoveDone is set when rabbit receives "Done:Posn"
				ProfilePoint("PMV DN 1");

				g_cPosnModeState = POSNMODE_FINALMOVE;
				g_cMoveDone = MOVEDONE_FALSE; // reset flag that indicates when a move has completed now,
											  //so that nothing else can think it's done the combined move.

				//Advance Station Prep if needed
				//When posnmode reaches finalmove, spinup can happen
				PrepareStations();

#ifdef OUTPUT_POSITION_STATES
				logf("@PM_FINALMOVE\r\n");
#endif
			}
			else if (g_cMoveDone == MOVEDONE_STOP)
			{
#ifdef USE_OUTPUT
				logf("Mv Stop.\r\n");
#endif
				ProfilePoint("PMV DN S");
				g_cPosnModeState = POSNMODE_STOP_MOVEMENT;
			}
			else if (g_cMoveDone == MOVEDONE_ERROR)
			{
#ifdef USE_OUTPUT
				logf("Mv fail Err.\r\n");
#endif
				ProfilePoint("PMV DN E");
				g_cPosnModeState = POSNMODE_STOP_MOVEMENT;
			}
			else if ((MS_TIMER - s_uiStartTime) >= (POSN_TIMEOUT_SEC * 1000L))
			{
#ifdef USE_OUTPUT
				logf("Mv fail Timeout.\r\n");
#endif
				ProfilePoint("PMV DN T");
				g_cPosnModeState = POSNMODE_STOP_MOVEMENT;
			}
			break;
		case POSNMODE_FINALMOVE:
			// #ifdef OUTPUT_POSITION_STATES
			// logf("@PM_FINALMOVE\r\n");
			// #endif
			// in POSNMODE_PREMOVE, we already moved to the absolute posn specified by: (desired posn) +/- (premove),
			// so now we just move to the absolute posn specified by (desired posn)
			if (g_cMoveDone == MOVEDONE_STOP)
			{
#ifdef USE_OUTPUT
				logf("Mv Stop.\r\n");
#endif
				ProfilePoint("PMV DN S");
				g_cPosnModeState = POSNMODE_STOP_MOVEMENT;
				break;
			}
			if (g_cMoveDone == MOVEDONE_ERROR) //Catch Any Error that is sent between state change
			{
#ifdef USE_OUTPUT
				logf("Mv fail Err.\r\n");
#endif
				ProfilePoint("MV DN E");
				g_cPosnModeState = POSNMODE_STOP_MOVEMENT;
				break;
			}
			g_PosnMode.cOnCurPosn = 0; //moving actions always mean not on position
			LEDOff()
			;

			//MACHINEPOINTS

			//Check to see if final move would be in the direction of obstruction, or out of limits
			//If a premove is used, this might have been cheked above, but we should recheck before we move
			cresult = CheckObstructionsAndMotionLimits(s_fTargetX, s_fTargetY);
			if (cresult != 0)
			{
				//Found a reason not to allow this move
				g_cPosnModeState = POSNMODE_STOP_MOVEMENT;
				break;
			}

			//set proper values for final move. Since it does not need to be linear, don't bother setting fdx,fdy.
			SetMoveSpeeds(0, 1, 0, 0);
			MCSetMoveParams(s_fErrLevFinal, s_cMoveOptFinal);

			//NOW MOVE
			MCRunPosition(s_fTargetX, s_fTargetY);
			ProfilePoint("FMV CMD");

			s_uiStartTime = MS_TIMER;
			g_cPosnModeState = POSNMODE_FINALMOVEWAIT;
#ifdef OUTPUT_POSITION_STATES
			logf("@PM_FINALMOVEWAIT\r\n");
#endif
			break;
		case POSNMODE_FINALMOVEWAIT:
			#ifdef UNIBUTTON
			if (g_cUniButtonEvent == 1)
			{
				// capured a single button press while carriage was moving...user wants to halt
				g_cPosnModeState = POSNMODE_STOP_MOVEMENT;
				g_cUniButtonEvent=0;//clear event
			}
#endif
			if (g_PosnMode.cDoMoveTrig == TRUE)
			{
				//May have a new goto position put in right during the last part of move...
				//Stop and do not complete this move...  this works and lets next move go
				g_cPosnModeState = POSNMODE_STOP_MOVEMENT;
			}
			// g_cMoveDone is set when rabbit receives "Done:Posn", or finds error
			if (g_cMoveDone == MOVEDONE_FALSE)
			{
				if ((MS_TIMER - s_uiStartTime) < (POSN_TIMEOUT_SEC * 1000L))
				{
					//not done yet
					break;
				}
#ifdef USE_OUTPUT
				logf("Mv fail Timeout.\r\n");
#endif
				ProfilePoint("MV DN T");
				g_cMoveDone = MOVEDONE_ERROR;
			}
			if (g_cMoveDone == MOVEDONE_STOP)
			{
#ifdef USE_OUTPUT
				logf("Mv Stop.\r\n");
#endif
				ProfilePoint("PMV DN S");
				g_cPosnModeState = POSNMODE_STOP_MOVEMENT;
				break;
			}
			if (g_cMoveDone == MOVEDONE_ERROR)
			{
#ifdef USE_OUTPUT
				logf("Mv fail Err.\r\n");
#endif
				ProfilePoint("MV DN E");
				g_cPosnModeState = POSNMODE_STOP_MOVEMENT;
				break;
			}
			//g_cMoveDone == MOVEDONE_TRUE
			//Advance Station Prep if needed
			PrepareStations();

			ProfilePoint("MV DN 1");
			//Set this now even before final action because of special cases
			g_PosnMode.iCurPosnIndex = g_PosnMode.iGotoPosnIndex; // update current position
			g_cPosnModeState = POSNMODE_ARRIVE;
#ifdef OUTPUT_POSITION_STATES
			logf("@PM_ARRIVE\r\n");
#endif
			g_ulArrive = MS_TIMER;
			//fall through
		case POSNMODE_ARRIVE:
			//Have Result MOVEDONE_TRUE, now do final actions
			//but always look for input
#ifdef UNIBUTTON
			if (g_cUniButtonEvent == 1)
			{
				// capured a single button press while carriage was moving...user wants to halt
				g_cPosnModeState = POSNMODE_STOP_MOVEMENT;
				g_cUniButtonEvent=0;//clear event
				break;
			}
#endif
			if (g_PosnMode.cDoMoveTrig == TRUE)
			{
				//May have a new goto position put in right during the last part of move...
				//Stop and do not complete this move...  this works and lets next move go
				g_cPosnModeState = POSNMODE_STOP_MOVEMENT;
				break;
			}
			if (g_cStationGoal == STATION_INSPECT)
			{
				g_cStation = STATION_INSPECT;
				goto go_directly_to_inspection;
			}

			//If moving to position the drill do tool check.
			if (g_cStationGoal == STATION_DRILL)
			{
				//VerifyEnable == 2 : Must Pass After Move
				if (g_ConfigData.cToolVerifyEnable == 2) //g_PartPgmInfo.cTeachModeData!=1 &&
				{
					//Another Tool Check Here
					VerifyAndAlertTool();
					if (g_cToolLoaded == 0)
					{
						goto POSNMODE_CLEAR_AND_STARTWAITNEXTPOSN;
						break;
					}

					//If moving to position, go ahead and start the process and override loading now if it hasn't happended yet
					//DRILL_DIRECT_PROCESS_AND_OVERRIDE
					if (g_cDrillLoadProcessAndOverride
							!= DRILL_LOAD_PROCESS_DONE)
					{
						//Did not yet load process and override
						//waiting
						if ((MS_TIMER & 3) == 0)
						{
							logf("a"); //FIXME SEVERE
						}
						//waiting
						if (g_cPrintAuth == 0)
						{
							logf("dlp%d\r\n",
									((int) g_cDrillLoadProcessAndOverride));
						}
						break;
					}

					//Check Loaded Process now...
					if (g_cRequiredProcess > 0)
					{
						if (g_cLoadedProcess != g_cRequiredProcess)
						{
							logf("lp %d rp %d\r\n", g_cLoadedProcess,
									g_cRequiredProcess);
						}
					}

					VerifyAndAlertProcess();
					if (g_cProcessLoaded == 0)
					{
						//For Now, we actually stop the job in this case...
						//In the future, we may loop and wait for it to be done...
						//If it's ever going to be able to load the file, it should be able to load it
						//by this time.
						//Ensure that StartProcess is clear
						logf("pcf\r\n");
						goto POSNMODE_CLEAR_AND_STARTWAITNEXTPOSN;
						break;
					}
					logf("d\r\n");
				}   			        	        //end verify enable 2
				else if (g_ConfigData.cToolVerifyEnable == 3)
				{
					//Another Tool Check Here, but don't prevent them from arrival
					VerifyAndAlertTool();
					//just do verify and alert now, but then move on to allow arrival
					VerifyAndAlertProcess();
				}
			}
			//Advance Station Prep if needed
			PrepareStations();

			g_ulPastTPC = MS_TIMER;

			go_directly_to_inspection: g_PosnMode.iCurPosnIndex =
					g_PosnMode.iGotoPosnIndex; // update current position
			g_PosnMode.iGotoPosnIndex = GOTOPOSN_UNKNOWN; // we don't know next position until user tells us which hole is next
			g_PosnMode.cOnCurPosn = 1; //really on the position
			g_PosnMode.cFirstMove = 0; //clear the flag
#ifdef BRAKE_ON_WHEN_STOPPED
			BrakeOn();
#endif

			//logf("DEBUG  posn id = %d and on position...\r\n",g_PosnMode.iCurPosnIndex);
			//LED ON
			LEDOn()
			;
			//and beep one time
			Beep();
			if (g_cAutoMove == AUTOMOVE_MOVE)
			{
				g_uiStartAutoTime = MS_TIMER;
			}
			SendCurPosnInd(0, STP_ALERT); //Send Cur Posn Index.
			MCGetPosition(); //must ensure final position is set.
			g_PosnMode.fLastSentPosnX += 1; //force a send.
			// Flag this position as visited.
			AddOpHistory(g_PosnMode.iCurPosnIndex, OP_VISIT);

#ifdef OUTPUT_POSITION_STATES
			logf(">>ON POS\r\n\r\n");
#endif
			ProfilePoint("ON POS");
			g_ulMoveEnd = MS_TIMER;
			/*
			 logf("M %u M2A %u M2TPC %u  A2TPC delay %u\r\n",
			 g_ulMoveEnd - g_ulMoveStart,
			 g_ulArrive - g_ulMoveStart,
			 g_ulPastTPC - g_ulMoveStart,
			 g_ulPastTPC - g_ulArrive);
			 */
//For new time profile, I will track move done as ARRIVE
			logf("@@%s %u\r\n", "M", g_ulArrive - g_ulMoveStart);
//For new time profile, track this as profile ready
			logf("@@%s %u\r\n", "AP", g_ulMoveEnd - g_ulArrive);
			//Rivet has it's own implementation of AUTOMOVE_ACTION, so don't effect it for now.
			//It's possible that the systems could be combined so rivet uses the action state, but
			//it's not needed at all for now.

			if (g_cStartProcess == 1 || g_cPositionInspection == 1)
			{
				logf("inst %d %d\r\n", g_cStartProcess, g_cPositionInspection);
				//Only Do inspection if running process cycle, or if running an explicit inspection.
				if (g_cStation == STATION_INSPECT
						&& g_cStationPlanInspect == 1)
				{
					//not ready to officially go to action for this, but I want to share the code in that location
					goto inspection_code_handler;
				}
			}
			if (g_cStartProcess == 1)
			{
				//do NOT return to main wait state, but go to action state
#ifdef OUTPUT_POSITION_STATES
				logf("@PM_ACTION\r\n");
#endif
				g_cPosnModeState = POSNMODE_ACTION;
				//And before going into action
				//Advance Station Prep if needed
				PrepareStations();
				break;
			}

			if (g_cAutoMove == AUTOMOVE_MOVE)
			{
				//do NOT return to main wait state, but go to action state
				g_uiStartAutoTime = MS_TIMER;
#ifdef OUTPUT_POSITION_STATES
				logf("@PM_WAITCYCLE\r\n");
#endif
				g_cPosnModeState = POSNMODE_WAITCYCLE;
				break;
			}

			if (g_cShowPrevDrilledMessage == 1)
			{
				SmartToolMsgMiniFtMessageCode(0, COMMON_OID_NULLOID,
						MINIFTMC_POSITON_PREVIOUSLY_DRILLED);
			}
			else if (g_cShowPrevDrilledMessage == 2) //use this to show this other message
			{
				SmartToolMsgMiniFtMessageCode(0, COMMON_OID_NULLOID,
						MINIFTMC_PROBE_WARNING_DRILL_PREVENTED);
			}
			//Return and wait
			goto POSNMODE_CLEAR_AND_STARTWAITNEXTPOSN;
			break;
		case POSNMODE_ACTION:
			//starting action
			g_cAction = ACTION_EXECUTE; //until done
			g_iActionHoleIndex = g_PosnMode.iCurPosnIndex;
			g_ulProcStart = MS_TIMER;
			g_ulClampStart2 = 0;
			g_ulClampDone = 0;
			g_ulProcPassed = 0;
			g_ulDrillStart = 0;
			g_ulDrillDone = 0;
			g_ulFastenerStart = 0;
			g_ulFastenerDone = 0;

			if (g_PosnMode.cOnCurPosn == 0)
			{
				//can't do action
				//Ensure that StartProcess is clear
				g_cStartProcess = 0;
			}
#ifdef GENCIRCMFTX
			if (g_cStationPlanDrill != 1)
			{
				logf("Force st pln d\r\n");
				g_cStationPlanDrill = 1; //FIXME Test with and without this to see if it works properly instead of forced
			}
#endif
			logf("station %c g %c\r\n", g_cStationCode[g_cStation],
					g_cStationCode[g_cStationGoal]);
			//Tool Check

			if (g_cStation == STATION_INSPECT)
			{
				if (g_cStationPlanInspect == 1)
				{
					inspection_code_handler:
					#ifdef CENTERVISION
					g_cCenterVisionResult = CENTERVISION_OFF; //set it back to the start state.
#endif
#ifdef CENTERVISION_ANALOG_POINT_LASER
					if (g_cInspectMethod == INSPECT_LASER) //Inspection is Laser
#else
#ifdef CENTERVISION_CAM
					if (g_cInspectMethod == INSPECT_CAMERA) //Inspection is Camera
#else
					if (0)
#endif
#endif
					{
						//Move to inspection state...
						g_cPositionInspection = 1;
						g_cPosnModeState = POSNMODE_INSPECT;
#ifdef OUTPUT_POSITION_STATES
						logf("@PM_INSPECT\r\n");
#endif
						break;
					}
					else
					{
						//Do Manual Inspection
						g_cStartProcess = 0; //stop auto process
						SmartToolMsgMiniFtMessageCode(0, COMMON_OID_NULLOID,
								MINIFTMC_INSPECT_THIS_POSITION);
						//Clear this flag
						g_cPositionInspection = 0;
						//Return and wait
						goto POSNMODE_CLEAR_AND_STARTWAITNEXTPOSN;
						break;
					}
				}
				else
				{
					//Inspection Station, but it's not selected... Why would it even be on this station???
					logf("ins-no-sel\r\n");
					g_cStartProcess = 0; //stop auto process
					//Clear this flag
					g_cPositionInspection = 0;
					//Return and wait
					goto POSNMODE_CLEAR_AND_STARTWAITNEXTPOSN;
					break;
				}
			}
			//Action Tool Check
			//If moving to position the drill do tool check.
			if (g_cStation == STATION_DRILL && g_cStationPlanDrill == 1)
			{
#ifdef SMARTDRILL_STP
#warnt "DRILLSTP ensure inclusion of proper code for gen3 w/ dsi"
#endif
#warning "DRILLSTP to require plan drill or not for this????  Problem is that it will NOT Reload Override?"
//but is that even a problem at all? :::  if on position, then it must have been loaded previously + doesn't it infact get loaded... review this AGAIN
				//If moving to position, go ahead and start the process and override loading now
				//DRILL_DIRECT_PROCESS_AND_OVERRIDE
				VerifyAndAlertTool();
				if (g_cToolLoaded == 0)
				{
					//Return and wait
					logf("tc0\r\n");
					goto POSNMODE_CLEAR_AND_STARTWAITNEXTPOSN;
					break;
				}
				if (g_DrillFault.cSeverity >= FAULT_SEVERITY_ALARM)
				{
					//Do Not Start Action
					//Ensure that StartProcess is clear
					logf("drf %d\r\n", g_DrillFault.cSeverity);
					g_cStartProcess = 0;
				}
				//Advance Station Prep if needed //FIXME DRILLFILL this may be a prob when I go back to that rail
				PrepareStations();
			}
#ifdef FASTENER_SYSTEM
			else if (g_cStation==STATION_PICKUP && g_cStationPlanFill==1)
			{
				g_cPosnModeState = POSNMODE_ACTION_PREP_FASTENER_1;
#ifdef OUTPUT_POSITION_STATES
				logf("@PM_A%d\r\n",1);
#endif
				goto POSNMODE_ACTION_PREP_FASTENER_1_label;
			}
			else if (g_cStation==STATION_FILL && (g_cStationPlanFill==1 || g_cStationPlanRemove==1))
			{
				g_cPosnModeState = POSNMODE_ACTION_PREP_FASTENER_1;
#ifdef OUTPUT_POSITION_STATES
				logf("@PM_A%d\r\n",1);
#endif
				goto POSNMODE_ACTION_PREP_FASTENER_1_label;
			}
#endif
			if (g_cStartProcess == 0)
			{
				//Must have stopped the process
				//Return and wait
				goto POSNMODE_CLEAR_AND_STARTWAITNEXTPOSN;
				break;
			}
#ifdef OUTPUT_POSITION_STATES
			logf("@PM_ACTION_PREP\r\n");
#endif
			g_cPosnModeState = POSNMODE_ACTION_PREP;
			goto POSNMODE_ACTION_PREP_label;
#ifdef FASTENER_SYSTEM
			case POSNMODE_ACTION_PREP_FASTENER_1:
			POSNMODE_ACTION_PREP_FASTENER_1_label:

//SKIP THIS FOR THIS STATE, BECAUSE ERRORS OFTEN LINGER FROM THE END OF THE LAST OPERATION
//					if (g_FastenerFault.cSeverity >= FAULT_SEVERITY_ALARM)
//					{
//						//Do Not Start Action
//						//Ensure that StartProcess is clear
//logf("f %d\r\n",g_FastenerFault.cSeverity);
//	                    //Must have stopped the process
//	                	//Return and wait
//   		                goto POSNMODE_CLEAR_AND_STARTWAITNEXTPOSN;
//           	            break;
//
//					}
//Clear an error that has happened already
#ifdef FASTENER_STP
			logf("fc\r\n");
			FastenerSTPSet(FASTENER_OID_FAULT_CLEAR);
			g_FastenerFault.cSeverity = 0;
#endif

			if (g_cStartProcess==0)
			{
				//Must have stopped the process
				//Return and wait
				goto POSNMODE_CLEAR_AND_STARTWAITNEXTPOSN;
				break;
			}
			//First make sure that it's home
#ifdef FASTENER_STP
			logf("%s=%d\r\n","fm",g_cFastenerMode);
			if (g_cStation==STATION_PICKUP)
			{
				if (g_cClampState != CLAMP_POSITION)
				{
					logf("!!!\r\n");
				}
			}
			if (g_cStation==STATION_FILL)
			{
				if (g_cFastenerMode!=FASTENER_MODE_HOMING && g_cFastenerMode!=FASTENER_MODE_NO_FASTENER && g_cFastenerMode!=FASTENER_MODE_READY)
				{
					//ask fastener to home
					if (FastenerSTPSetUInt(FASTENER_OID_HOME,0)==1)
					{
						logf("FH\r\n");
						g_cFastenerMode = 0xFF; //clear until rx new value.
					}
					//but do continue so that the waiting for homing is done below
				}
			}
#endif
#ifdef OUTPUT_POSITION_STATES
			logf("@PM_A%d\r\n",2);
#endif
			g_cPosnModeState = POSNMODE_ACTION_PREP_FASTENER_2;
			//Fall Through
			case POSNMODE_ACTION_PREP_FASTENER_2:
			if (g_FastenerFault.cSeverity >= FAULT_SEVERITY_ALARM)
			{
				//Do Not Start Action
				//Ensure that StartProcess is clear
				logf("f %d\r\n",g_FastenerFault.cSeverity);
				//Must have stopped the process
				//Return and wait
				goto POSNMODE_CLEAR_AND_STARTWAITNEXTPOSN;
				break;
			}
			if (g_cStartProcess==0)
			{
				//Must have stopped the process
				//Return and wait
				goto POSNMODE_CLEAR_AND_STARTWAITNEXTPOSN;
				break;
			}
			if (g_cStation==STATION_PICKUP)
			{
//also see if it's OK that I don't checkif the operations are in plan
				//c=FastenerSTPSet(FASTENER_OID_PICKUP_READY);
				logf("pur\r\n");
				FastenerSTPSet(FASTENER_OID_PICKUP_READY);
				g_cFastenerLoaded = 16;
//FIXME MINOR check output like others that do it
			}
			else if (g_cStation==STATION_FILL)
			{
#ifdef FASTENER_STP
				if (g_cFastenerMode==FASTENER_MODE_HOMING || g_cFastenerMode==0xFF)
				{
					//wait
					break;
				}
#endif
				if (g_cFastenerMode!=FASTENER_MODE_NO_FASTENER && g_cFastenerMode!=FASTENER_MODE_READY)
				{
					//Bad Mode... not home
					logf("f %d\r\n",-123);
					goto POSNMODE_CLEAR_AND_STARTWAITNEXTPOSN;
					break;
				}
				if (g_cStationPlanFill==1)
				{
					if (g_cUseFastenerTray == 1)
					{
						//Using Fastener Tray ... must have one loaded from before
					}
					else
					{
						//Go ahead and ask for Fastener Now
						SmartToolMsgChar(p_STPSession, STP_ALERT, MINIFT_OID_FASTENER_ARRIVED, 0);
						g_cFastenerArrived = 0;//clear this value
					}
				}
				else if (g_cStationPlanRemove==1) //Can't run with both fill and remove ops
				{
					//nothing to ask for....
					logf("%c\r\n",'R');
				}
			}
			//Enter next state
#ifdef OUTPUT_POSITION_STATES
			logf("@PM_A%d\r\n",3);
#endif
			g_cPosnModeState = POSNMODE_ACTION_PREP_FASTENER_3;
			//Fall Through
			case POSNMODE_ACTION_PREP_FASTENER_3:
			if (g_FastenerFault.cSeverity >= FAULT_SEVERITY_ALARM)
			{
				//Do Not Start Action
				//Ensure that StartProcess is clear
				logf("f %d\r\n",g_FastenerFault.cSeverity);
				//Must have stopped the process
				//Return and wait
				goto POSNMODE_CLEAR_AND_STARTWAITNEXTPOSN;
				break;
			}
			if (g_cStartProcess==0)
			{
				//Must have stopped the process
				//Return and wait
				goto POSNMODE_CLEAR_AND_STARTWAITNEXTPOSN;
				break;
			}
			if (g_cStation==STATION_PICKUP)
			{
				//nothing to do
			}
			else if (g_cStation==STATION_FILL)
			{
				if (g_cStationPlanFill==1)
				{
					if (g_cUseFastenerTray == 1)
					{
						//Using Fastener Tray ... must have one loaded from before
						if (g_cFastenerLoaded == 0)
						{
							//need to wait for this
							logf("%c\r\n",'q');
							break;
						}
						if ( g_cFastenerMode != FASTENER_MODE_READY )
						{
							//Bad Mode... not home or does not have fastener
							logf("f %d\r\n",-124);
							goto POSNMODE_CLEAR_AND_STARTWAITNEXTPOSN;
							break;
						}
					}
					else
					{
						//wait for signal that fastener is loaded
						if (g_cFastenerArrived==0)
						{
							//Still waiting for signal that fastener is ready
							break;
						}
						//Tell Fastener That there is one loaded
						i = FastenerSTPSet(FASTENER_OID_FASTENER_LOADED);
						if (i==0)
						{
							logf("f\r\n");
							SmartToolMsgMiniFtMessageCode(0, COMMON_OID_NULLOID, MINIFTMC_FASTENER_COM_FAIL);
							g_cStartProcess = 0; //allow it to continue on, but trigger this so that it stops right away
						}
					}
				}
				else if (g_cStationPlanRemove==1)
				{
					if ( g_cFastenerMode != FASTENER_MODE_NO_FASTENER )
					{
						//Bad Mode... not home or does has fastener already
						logf("f %d\r\n",-125);
						goto POSNMODE_CLEAR_AND_STARTWAITNEXTPOSN;
						break;
					}
					logf("%c\r\n",'R');
				}
			}
			//Enter next state
#ifdef OUTPUT_POSITION_STATES
			logf("@PM_ACTION_PREP\r\n");
#endif
			g_cPosnModeState = POSNMODE_ACTION_PREP;
			//Fall Through
#endif
		case POSNMODE_ACTION_PREP:
			POSNMODE_ACTION_PREP_label: if (g_cStartProcess == 0)
			{
				//Must have stopped the process
#ifdef CLAMP_SYSTEM
				//FIXME000000000 none of the new clamp logic does any tests for EEOption
				if (g_cClampState != g_cClampGoal)
				{
					g_cClampGoal = CLAMP_HOLD;
				}
#endif
				g_cDrillStateGoal = DRILLSTATE_IDLE;
#ifdef SEAL_SYSTEM
				g_cSealState=SEALSTATE_OFF;
#endif
#ifdef FASTENER_SYSTEM
				g_cFillState=FILLSTATE_OFF;
				g_cFastenerArrived=0; //allow clearance, but hope it will redetect it
#endif
				//It's only in Spin up, so don't need to wait
				//if (g_cDrillState!=DRILLSTATE_IDLE)
				//{
				//	//Must Wait until it's back to IDLE
				//    break;
				//}
				//Return and wait
				goto POSNMODE_CLEAR_AND_STARTWAITNEXTPOSN;
				break;
			}
			if (g_cEEOption != EENONE)
			{
				if (g_cStation == STATION_DRILL && g_cStationPlanDrill == 1)
				{
#ifdef SMARTDRILL_STP
					if (g_DrillFault.cSeverity >= FAULT_SEVERITY_ALARM)
					{
						//Do Not Continue Action
						//Stop Any Spin Up
						g_cDrillStateGoal=DRILLSTATE_IDLE;
#ifdef CLAMP_SYSTEM
						//Reverse Clamp (unlike default start action)
						g_cClampGoal=CLAMP_UNCLAMP;
#endif
						//Instead of doing the same as stop, we just want to go to action complete and wait to unclamp
#ifdef OUTPUT_POSITION_STATES
						logf("@PM_ACTION_COMPLETE\r\n");
#endif
						g_cPosnModeState = POSNMODE_ACTION_COMPLETE;
#warnt "make sure this path does not grant credit for action"
						break;
					}
#endif
#ifdef CLAMP_SYSTEM
					//must be CLAMPED
					if (g_cClampGoal != CLAMP_CLAMP)
					{
						//ask for clamp
						SetPressureBasedOnProcess(g_cRequiredProcess);
						g_ulClampStart2 = MS_TIMER;
						g_cClampGoal = CLAMP_CLAMP;
						g_cClampGoalSent = 0xFF; //force it to send goal again
						logf("Ask Clamp.\r\n");
						g_cClampState = CLAMP_TRANSITION;
					}
#endif
#ifdef SMARTDRILL_STP
					//DRILL_DIRECT_PROCESS_AND_OVERRIDE
					if (g_cDrillLoadProcessAndOverride==DRILL_LOAD_PROCESS_NOT_NEEDED)
					{
						g_cDrillLoadProcessAndOverride=DRILL_LOAD_PROCESS_NEEDED; //Now that there is a delay (waiting for clamp up), authorize process loading to begin if it has not been done.
					}
#endif
#ifdef CLAMP_SYSTEM
					if (g_cClampState == CLAMP_HOLD
							|| g_cClampState == CLAMP_UNKNOWN) //problem clamping
					{
						//this is a sign that the clamp failed
						g_cStartProcess = 0;
						break;
					}
					if (g_cClampState != CLAMP_CLAMP)
					{
						break;
					}
#endif
					if (g_ulClampDone == 0)
					{
						logf("Hv Clamp\r\n"); //FIXME extra temp debug code
						ProfilePoint("CLMP DN");
						g_ulClampDone = MS_TIMER;
					}
#ifdef SMARTDRILL_STP
#ifdef DRILLFILL_STP
#fatal "Does having this here ruin DSI????"
#endif
//may need to use different checking for this
					//DRILL_DIRECT_PROCESS_AND_OVERRIDE
					if (g_cDrillLoadProcessAndOverride!=DRILL_LOAD_PROCESS_DONE)
					{
						//Did not yet load process and override
						//waiting
						if (g_cPrintAuth==0)
						{
							logf("dlp%d\r\n",((int)g_cDrillLoadProcessAndOverride));
						}
#warnt "redo drill wait timing"
						break;
					}
					VerifyAndAlertProcess();
					if (g_cToolLoaded==0 || g_cProcessLoaded==0)
					{
						//Ensure that StartProcess is clear
						logf("tcf %d %d\r\n",g_cToolLoaded,g_cProcessLoaded);
						g_cStartProcess = 0;
						break;
					}
#endif
					g_ulProcPassed = MS_TIMER;
				}
#ifdef FASTENER_SYSTEM
				else if (g_cStation==STATION_PICKUP && g_cStationPlanFill==1)
				{
#ifdef CLAMP_SYSTEM
					if (g_cClampState!=CLAMP_POSITION) //look for arrival at special position
					{
						logf("np\r\n");
						//this is a sign that the clamp failed: it should have been at position during the move
						g_cStartProcess = 0;
						break;
					}
					//go to action, which is just to wait for pickup
#endif
				}
				else if (g_cStation==STATION_FILL && (g_cStationPlanFill==1 || g_cStationPlanRemove==1))
				{
#ifdef CLAMP_SYSTEM
					//must be CLAMPED
					if (g_cClampGoal!=CLAMP_CLAMP)
					{
						//ask for clamp
						SetPressureBasedOnProcess(g_cRequiredProcess);
//FIXME HIGH  what is pressure set to for fill if there is no process still set?????
//will this load this for this hole?
//what about programs with only fastening????
						g_ulClampStart2 = MS_TIMER;
						g_cClampGoal=CLAMP_CLAMP;
						g_cClampGoalSent = 0xFF;//force it to send goal again
						logf("Ask Clamp.\r\n");
						g_cClampState=CLAMP_TRANSITION;
					}
					if (g_cClampState==CLAMP_HOLD || g_cClampState==CLAMP_UNKNOWN) //problem clamping
					{
						//this is a sign that the clamp failed
						g_cStartProcess = 0;
						break;
					}
					if (g_cClampState!=CLAMP_CLAMP)
					{
						break;
					}
#endif
					//fastener mode was already checked above, so I will not bother here, but I still will print
					logf("%d\r\n",g_cFastenerMode);
				}
#endif
			} //End not EENONE
			g_cSafeUnclamp = 0; //ensure this is 0
			g_cFastDoneFlag = 0;
#ifdef OUTPUT_POSITION_STATES
			logf("@PM_ACTION_EXECUTE\r\n");
#endif
			g_cPosnModeState = POSNMODE_ACTION_EXECUTE;
			//fall through
		case POSNMODE_ACTION_EXECUTE:
			//Don't bother checking if process was stopped or clamp is up:
			//This code only operatesvery quickly after PREP and before WATCH....
			//PREP just did all those checks a few lines above and nothing actually jumps to this case

			//Start the action
			if (g_cEEOption != EENONE)
			{
//FIXME dfnow...
				logf("stat=%c plan=%d\r\n", g_cStationCode[g_cStation],
						g_cStationPlanDrill);
				if (g_cStation == STATION_DRILL && g_cStationPlanDrill == 1)
				{
					//Start Drilling
					//Set Op
					AddOpHistory(g_PosnMode.iCurPosnIndex, OP_DRILL);
					//Clear the Drill Results (Will get new results after it's done.)
					g_HoleResultData.iHoleNumber = -1;
					g_HoleResultData.iHoleResult = HR_UNKNOWN;
//FIXMENOW
					logf("khrae\r\n");
					//Now Set Goal
					logf("Goal = Drill\r\n");
					g_ulDrillStart = MS_TIMER;
					g_cDrillStateGoal = DRILLSTATE_DRILL;
				}
#ifdef FASTENER_SYSTEM
				else if (g_cStation==STATION_PICKUP && g_cStationPlanFill==1)
				{
					//go to action, which is just to wait for pickup
				}
				else if (g_cStation==STATION_FILL && (g_cStationPlanFill==1 || g_cStationPlanRemove==1))
				{
					//For this version, the operator has already signaled that the fastener has arrived.
					//Now all that remains it to tell the fastener system to fasten.

					logf(">F\r\n");
					if (g_cStationPlanFill==1)
					{
						i = FastenerSTPSetUInt(FASTENER_OID_FASTEN,0);					//0 because it's not forced... should be ready
					}
					else if (g_cStationPlanRemove==1)
					{
						i = FastenerSTPSetUInt(FASTENER_OID_EXTRACT,0);	//0 because it's not forced... should be ready
					}
					if (i==0)
					{
						logf("f\r\n");
						SmartToolMsgMiniFtMessageCode(0, COMMON_OID_NULLOID, MINIFTMC_FASTENER_COM_FAIL);
						g_cStartProcess = 0; //allow it to continue on, but trigger this so that it stops right away
					}
					g_ulFastenerStart = MS_TIMER;
				}
#endif
			} //End Not EENONE

#ifdef OUTPUT_POSITION_STATES
			logf("@PM_ACTION_WATCH\r\n");
#endif
			g_cPosnModeState = POSNMODE_ACTION_WATCH;
			//fall through
		case POSNMODE_ACTION_WATCH:
			if (g_cStartProcess == 0)
			{
				stop_action_as_soon_as_possible:
				//Must have stopped the process
				//No longer in AUTO
				g_cDrillStateGoal = DRILLSTATE_IDLE;
#ifdef SEAL_SYSTEM
				g_cSealState=SEALSTATE_OFF;
#endif
#ifdef FASTENER_SYSTEM
				g_cFillState=FILLSTATE_OFF;
				g_cFastenerArrived=0; //allow clearance, but hope it will redetect it
#endif
#ifdef GENCIRCMFTX
				if (g_cDrillState == DRILLSTATE_DONE)
				{
					g_cDrillState = DRILLSTATE_IDLE;
					g_cDrillStateGoal = DRILLSTATE_IDLE;
				}
#endif
				if (g_cDrillState != DRILLSTATE_IDLE)
				{
					//Must Wait until it's back to IDLE
					break;
				}
				//Drill is IDLE...
				//Return to main POSNMODE State
				g_cPosnModeState = POSNMODE_STARTWAITNEXTPOSN;
#warning "ABORTS AND RESETS REVIEW"
////////////FIXME THIS CASE NO RESET ???????
//thereis a reason that I don't go to the place and it waits, but I'm not sure why....
//fasteenr does not get aborted here

//FIXME should have fault from tray and do branch on when to abort???
#ifdef FASTENER_SYSTEM
				if (g_FastenerFault.cSeverity==0)
				{
					if (g_cFastenerMode!=FASTENER_MODE_NO_FASTENER && g_cFastenerMode!=FASTENER_MODE_READY)
					{
						//Fastener Abort
						FastenerSTPSet(FASTENER_OID_ABORT);
#ifdef FASTENER_TRAY_STP
						FastenerTraySTPSet(FASTENERTRAY_OID_ABORT);
#endif
						logf("%d\r\n",-1166);
					}
				}
#endif
				break;
			}
#ifdef GENCIRCMFTX
			if (g_cDrillState != DRILLSTATE_DONE)
			{
				//Drill Machine is not done
				break;
			}
			//Set Drill Back to Idle
			g_cDrillState = DRILLSTATE_IDLE;
			g_cDrillStateGoal = DRILLSTATE_IDLE;
			g_cStationGoal = STATION_UNSPEC;
#else
			if (g_cEEOption!=EENONE)
			{
				if (g_cStation==STATION_DRILL)
				{
					if (g_cStationPlanDrill==1)
					{
#ifdef CLAMP_SYSTEM
						if (g_cSafeUnclamp == 0)
						{
							//must be CLAMPED
							if (g_cClampGoal!=CLAMP_CLAMP || g_cClampState!=CLAMP_CLAMP)
							{
								//This is an abnormal situation, so treat like STOP
								g_cStartProcess = 0;//Stop Process.
								logf("NOT CLAMPED!\r\n");
								goto stop_action_as_soon_as_possible;
								break;
							}
						}
						else
						{
							if (g_cClampState==CLAMP_UNCLAMP)
							{
								if (g_ulUnclampDone == g_ulUnclampStart) //need to record the end of clamp, unless it was previously recorded
								{
									g_ulUnclampDone = MS_TIMER;
								}
							}
						}
#endif

						//do not require idle.... just unclampsafe
						if (g_cSafeUnclamp == 0)
						{
							//did not yet see the standard mark of a complete cycle, but there are some other
							//ways that need  to exit here.
							if (g_cSawDrillMode==1 && g_cDrillState==DRILLSTATE_IDLE)
							{
								//it has returned home....
								//missed unclamp signal....
								logf("irt\r\n");
								goto skip_hole_result_wait;
							}
							else if (g_cDrillState==DRILLSTATE_NOT_HOME)
							{
								//couldn't return home
								logf("not home...\r\n");
								g_cStartProcess = 0;//Stop Process.
								goto stop_action_as_soon_as_possible;//drill is stopped, but this is best way to exit
							}
							//must wait until the terminating state...
							break;
						}

//FIXME HIGH Want a Timeout for drill start
//Also Want a Timeout for some operations that will stop auto?
//for example I could have timed out drill cycle
//This is not quite as important anymore because it's working so well, but I should consider it.

						//Fault Handling:  g_DrillFault.cSeverity >= FAULT_SEVERITY_ABORT_DRILL or FAULT_SEVERITY_ALARM
						//At this point, do nothing differently if there is a drill fault.

						//If there is a fault of ABORT or higher, then there is no point in waiting for
						//the hole to start or for hole results.
						if (g_DrillFault.cSeverity >= FAULT_SEVERITY_ABORT_DRILL)
						{
							//A Drill Fault was seen, so don't bother waiting for hole results
							goto skip_hole_result_wait;
						}

						//Check Result Structure
						//There will always be a hole number set, and a result coming in for this hole
						if (g_HoleResultData.iHoleNumber < 0)
						{
							//Still Waiting For Results....
							break;
						}
						if (g_HoleResultData.iHoleNumber != g_PosnMode.iCurPosnIndex)
						{
							//Looks like this for the wrong hole.  Must Stop here
							logf("*p %d r %d\r\n", g_PosnMode.iCurPosnIndex, g_HoleResultData.iHoleNumber);
							g_cStartProcess = 0;//Stop Process.
							goto stop_action_as_soon_as_possible;//drill is stopped, but this is best way to exit
							break;
						}
//FIXME000000000000000 potential issue with Drill Mode Race Condition... so leave this as started for now....
						if (g_HoleResultData.iHoleResult <= HR_STARTED)
						{
							//HR_UNKNOWN or HR_STARTED
							//Still not done yet
							break;
						}
						skip_hole_result_wait:
						logf("*ehr%d\r\n",g_HoleResultData.iHoleResult);
						//Now Completely done, so proceed.
						g_ulDrillDone = MS_TIMER;

//FIXME HIGH Must verify:
//is this code needed in combination with newer drill control loop in cycle?

//don't alter goal until it's done
						if (g_cDrillState==DRILLSTATE_IDLE)
						{
							g_cDrillStateGoal=DRILLSTATE_IDLE;
						}
						//Old code just forced IDLE here because it wasn't supposed to leave the action until drill was confirmed done.
						//g_cDrillStateGoal=DRILLSTATE_IDLE;

						//OK Success
						g_uiActionCycles++;
						if ((g_ConfigData.cVisionAutoInspect>0) &&
								((g_uiActionCycles % g_ConfigData.cVisionAutoInspect) == 0))
						{
							logf("vai\r\n");
							g_cInspectMethod = g_ConfigData.cInspectMethod;
							g_cPositionInspection = 1; //causes plan to allow inspection, and also used by inspection lib.
							g_cStationPlanInspect = 1;
						}
						if (g_DrillFault.cSeverity >= FAULT_SEVERITY_ABORT_DRILL)
						{
							//do not advance station this way...
							g_cStationGoal = STATION_UNSPEC;//causes skip to end
						}
					}
					else
					{
						//station is drill, but it was not selected
						logf("sgu\r\n");
					}

				}
#ifdef SEAL_SYSTEM
				else if (g_cStation==STATION_SEAL && g_cStationPlanSeal==1)
				{
					if (g_cSealState==SEALSTATE_OFF || g_cSealState==SEALSTATE_PRESSURE)
					{
						//didn't start autoprime so do it now
						g_cSealState=SEALSTATE_AUTOPRIME;
						break;
					}
					if (g_cSealState==SEALSTATE_WAIT)
					{
						logf("Seal Here\r\n");
						//ready to apply, so apply now
						g_cSealState=SEALSTATE_APPLY;
						break;
					}
					if (g_cSealState!=SEALSTATE_DONE)
					{
						//not yet done
						break;
					}
					g_cSealState=SEALSTATE_OFF; //There is no standby....
					//OK Success
				}
#endif
#ifdef FASTENER_SYSTEM
				else if (g_cStation==STATION_PICKUP && g_cStationPlanFill==1)
				{
					//Watch and wait for fastener to be done
					if (g_FastenerFault.cSeverity >= FAULT_SEVERITY_ABORT_DRILL)
					{
						g_cStartProcess = 0; //Stop Process.
						goto stop_action_as_soon_as_possible;
						//do not advance station this way...
						//g_cStationGoal = STATION_UNSPEC; //causes skip to end
					}

#ifdef CLAMP_SYSTEM
					if (g_cClampState!=CLAMP_POSITION)
					{
						//this is a sign that the clamp failed: it should have been unclamp during the move and still unclamped
						//FIXME when pickup is from ANOTHER POSITION, we'll need to modify all this
						g_cStartProcess = 0;
						break;
					}
					//go to action, which is just to wait for pickup
#endif
					//wait for mode ready....

					if (g_cFastenerLoaded == 0)
					{
						//like a failure.... it told us there was no fastener
						//CAN MOVE ANYWHERE...
						g_cStartProcess = 0;
						break;
					}

					//THIS MEANS I CAN MOVE HORIZONTALLY BUT NOT VERTICALLY
					//So After getting MODE READY from fastener, may continue, but don't allow ACTION until g_cFastenerLoaded == 1
					if (g_cFastenerMode!=FASTENER_MODE_READY)
					{
						/*
						 if (g_cFastenerMode!=FASTENER_MODE_NO_FASTENER)
						 {
						 //an error
						 g_cStartProcess = 0;
						 break;
						 }

						 */
						if (g_cPrintAuth==0)
						{
							logf("%c\r\n",'r');
						}
						break;
					}
				}
				else if (g_cStation==STATION_FILL && (g_cStationPlanFill==1 || g_cStationPlanRemove==1))
				{
					//Watch and wait for fastener to be done
					if (g_FastenerFault.cSeverity >= FAULT_SEVERITY_ABORT_DRILL)
					{
						g_cStartProcess = 0; //Stop Process.
						goto stop_action_as_soon_as_possible;
						//do not advance station this way...
						//g_cStationGoal = STATION_UNSPEC; //causes skip to end
					}

//FIXME
//should I be getting this for either??? both???
//						if (g_cSafeUnclamp==0)
//						{
//							//still waiting for this sign from the fastener
//							break;
//						}

//                        if (MS_TIMER - g_ulFastenerStart < (unsigned long)500)
//						{
//							//don't even bother looking at modes until 1/2 second in...  they might not have changed away
//							break;
//						}

					//Fill should always end in FASTENER_MODE_NO_FASTENER
					//Remove should end in FASTENER_MODE_READY
					if (g_cStationPlanFill==1)
					{
						if (g_cFastenerMode!=FASTENER_MODE_NO_FASTENER)
						{
							if (g_cPrintAuth==0)
							{
								logf("%c\r\n",'r');
							}
							break;
						}
					}
					else if (g_cStationPlanRemove==1)
					{
						if (g_cFastenerMode!=FASTENER_MODE_READY)
						{
							if (g_cPrintAuth==0)
							{
								logf("%c\r\n",'R');
							}
							break;
						}
					}
					//OK Success
				}
#endif
			} //END If not EENONE
#endif
			//Done Action
#ifdef OUTPUT_POSITION_STATES
			logf("@PM_ACTION_COMPLETE\r\n");
#endif
			g_cPosnModeState = POSNMODE_ACTION_COMPLETE;
			//fall through
		case POSNMODE_ACTION_COMPLETE:
			if (g_cStartProcess == 0)
			{
				//Must have stopped the process
#ifdef CLAMP_SYSTEM
				if (g_cClampState != g_cClampGoal)
				{
					g_cClampGoal = CLAMP_HOLD;
				}
#endif
				//Return and wait
				goto POSNMODE_CLEAR_AND_STARTWAITNEXTPOSN;
				break;
			}
#ifdef GENCIRCMFTX
//Want to see if this can be shared with main unclamping logic below..
			//FIXME000000000 for now do not wait until the wait code can be tested
			//ask for unclamp
			g_cClampGoal = CLAMP_UNCLAMP;
#endif
			if (g_cEEOption != EENONE)
			{
#ifdef CLAMP_SYSTEM
				//Wait for unclamp
#ifdef FASTENER_STP
				if (g_cStation==STATION_PICKUP)
				{
					//Wait for unclamp
					if (g_cClampGoal!=CLAMP_POSITION)
					{
						//ask for unclamp
						g_cClampGoal=CLAMP_POSITION;
						logf("Ask Unclamp.\r\n");
					}
					if (g_cClampState!=CLAMP_POSITION)
					{
						break;
					}
				}
				else
#endif
				{
					if (g_cClampGoal != CLAMP_UNCLAMP)
					{
						//ask for unclamp
						g_cClampGoal = CLAMP_UNCLAMP;
						logf("Ask Unclamp.\r\n");
					}
					if (g_cClampState != CLAMP_UNCLAMP)
					{
						break;
					}
				}
#endif
				if (g_ulUnclampDone == g_ulUnclampStart) //need to record the end of clamp, unless it was previously recorded
				{
					g_ulUnclampDone = MS_TIMER;
				}

				//DRILL_DIRECT_PROCESS_AND_OVERRIDE
				//Clear this
				g_cDrillLoadProcessAndOverride = DRILL_LOAD_PROCESS_NOT_NEEDED;

				//OK action was either skipped or was completed properly...
				//Determine next actions

				if (g_cStation == STATION_DRILL)
				{
					if (g_DrillFault.cSeverity >= FAULT_SEVERITY_ALARM)
					{
						//A Drill Fault was seen, so do not advance station, and do
						//not continue even if mode is continuous.
						g_cStationGoal = g_cStation; //return goal to current station
						goto POSNMODE_CLEAR_AND_STARTWAITNEXTPOSN;
					}
				}
				else if (g_cStation == STATION_FILL
						|| g_cStation == STATION_PICKUP) //pickup is "1st" but fill is more likely
				{
					if (g_FastenerFault.cSeverity >= FAULT_SEVERITY_ALARM)
					{
						//A Fastener Fault was seen, so do not advance station, and do
						//not continue even if mode is continuous.
						g_cStationGoal = g_cStation; //return goal to current station
						goto POSNMODE_CLEAR_AND_STARTWAITNEXTPOSN;
					}
				}

				g_ulFinalTime = MS_TIMER;

				if (g_ulClampStart2 != 0)
				{
					//ClampStart2 was set...
					logf("@@%s %u\r\n", "C", g_ulClampDone - g_ulClampStart2);
				}

				if (g_cStation == STATION_DRILL)
				{
					if (g_ulProcPassed != 0)
					{
						//Drill Path Shows Stats, Other Paths may need other output

						logf("@@%s %u\r\n", "SP", g_ulProcPassed - g_ulClampDone);
						if (g_ulDrillStart != 0 && g_ulDrillDone != 0)
						{
							logf("@@%s %u\r\n", "D", g_ulUnclampStart - g_ulDrillStart); //Drill Start to Unclamp Safe
							logf("@@%s %u\r\n", "U", g_ulUnclampDone - g_ulUnclampStart); //Unclamp Safe to Hole Results (might be near 0)
							int itd = (int32) (g_ulDrillDone - g_ulUnclampDone); //if positive, drill was done later than unclamp...
							if (itd < 0) //if negative, unclamp had time to spare... so there was 0 drill wait time
							{
								//Drill was done (done according to hole results) prior to unclamp done
								logf("@#%s %u\r\n", "tts", fabs(itd)); //show the time to spare
								itd = 0; //There was no drill wait time
							}
							logf("@@%s %u\r\n", "H", itd); //Unclamp Safe to Hole Results (might be near 0)
						}
					} //end ProcPassed

					if (g_ulMoveStart != 0)
					{
						ulx = g_ulMoveStart;
						logf("@@%s %d\r\n", "lr", g_ulLPR - ulx);
						logf("@@%s %d\r\n", "ld", g_ulLD - ulx);
						logf("@@%s %d\r\n", "lo", g_ulLO - ulx);
						logf("@@%s %d\r\n", "sp", g_ulSpinUp - ulx);
						logf("@@%s %d\r\n", "ds", g_ulDrillStart - ulx);
						logf("@@%s %u\r\n", "dT", (g_ulFinalTime - ulx));
					}
					ulx = g_ulProcStart;
					logf("@@%s %d\r\n", "lr", g_ulLPR - ulx);
					logf("@@%s %d\r\n", "ld", g_ulLD - ulx);
					logf("@@%s %d\r\n", "lo", g_ulLO - ulx);
					logf("@@%s %d\r\n", "sp", g_ulSpinUp - ulx);
					logf("@@%s %d\r\n", "ds", g_ulDrillStart - ulx);
					logf("@@%s %u\r\n", "dA", (g_ulFinalTime - ulx));
				} // end STATION_DRILL
				else if (g_cStation == STATION_FILL)
				{
					if (g_ulFastenerStart != 0 && g_ulFastenerDone != 0)
					{
						logf("@@%s %u\r\n", "F", g_ulFastenerDone - g_ulFastenerStart); //Drill Start to Unclamp Safe
					}
					logf("@@%s %u\r\n", "U", g_ulUnclampDone - g_ulUnclampStart); //Unclamp Safe to Hole Results (might be near 0)

					if (g_ulMoveStart != 0)
					{
						ulx = g_ulMoveStart;
						logf("@@%s %u\r\n", "fT", (g_ulFinalTime - ulx));
					}
					ulx = g_ulProcStart;
					logf("@@%s %u\r\n", "fA", (g_ulFinalTime - ulx));
				}

				if (g_cStationGoal == g_cStation) //Select the Next Station
				{

					logf("station %c g %c\r\n", g_cStationCode[g_cStation], g_cStationCode[g_cStationGoal]);
					//Select the next station goal
					NextStation();
					logf("station %c g %c\r\n", g_cStationCode[g_cStation], g_cStationCode[g_cStationGoal]);
					//Advance the plan depending on operations selected and allowed
					AdvanceStations();
				}

			} //end not EENONE

			//STATION-SYSTEM
			logf("station %c g %c\r\n", g_cStationCode[g_cStation], g_cStationCode[g_cStationGoal]);
			logf("wloop %c...\r\n", g_cStationCode[g_cStationGoal]);
			if (g_cStationGoal != STATION_UNSPEC)
			{
				//still more stations to complete this set
				//going to have to start move, so start move logic
				SpecifyGotoPosnAgain();
				//use the same handler as the main move next in waitcycle
				goto handle_process_gotoposn;
			}
			//Restore Station Goal Back To current Station
			g_cStationGoal = g_cStation;

			if (g_iActiveSessions == 0)
			{
				//do not continue
				logf("act0\r\n");
			}
			else if (g_ConfigData.cProcessContinueMode
					== PROCESS_CONTINUOUS)
			{
				//Should continue with next hole
				//do NOT return to main wait state, but go to cycle wait now
				g_uiStartAutoTime = MS_TIMER;
#ifdef OUTPUT_POSITION_STATES
				logf("@PM_WAITCYCLE\r\n");
#endif
				g_cPosnModeState = POSNMODE_WAITCYCLE;
				break;
			}
			//Done action so return to wait next position
			goto POSNMODE_CLEAR_AND_STARTWAITNEXTPOSN;
			break;
		case POSNMODE_WAITCYCLE:
			g_cAction = ACTION_RUNNING;
			if (g_cStartProcess == 0)
			{
				//No longer in AUTO
				//Return to wait next position
				goto POSNMODE_CLEAR_AND_STARTWAITNEXTPOSN;
				break;
			}

			//Ensure wait time is reached (if any)
			if ((MS_TIMER - g_uiStartAutoTime) < g_uiAutoTime)
			{
				//not yet
				break;
			}

			//Attempt next move
			i = SpecifyGotoPosn(GOTOPOSN_NEXT, 0);
			handle_process_gotoposn: if (g_PosnMode.cDoMoveTrig == TRUE)
			{
				//Success
				g_PosnMode.cDoMoveTrig = FALSE;
#ifdef OUTPUT_POSITION_STATES
				logf("@PM_TOOLCHECK\r\n");
				logf(" via 11577\r\n");
#endif
				g_cPosnModeState = POSNMODE_TOOLCHECK;
				break;
			}
			//Can't Move to Next:
			if (i == GOTOPOSN_OUTOFBOUNDS)
			{
				if (g_cAutoRepeat == 1)
				{
					//Start again
////this is the new way... forcing move to 1st index
					g_PosnMode.iGotoPosnIndex = g_PosnMode.iStartPosnIndex;
					SpecifyGotoPosn(GOTOPOSN_NEXT, 0);
					//Move was triggered
					//Clear Flag
					g_PosnMode.cDoMoveTrig = FALSE;
					//Go Right to Tool Check
					g_cPosnModeState = POSNMODE_TOOLCHECK;
					break;
				}
			}
			g_cAutoMove = 0;
			//Go Back to Main Wait State
			goto POSNMODE_CLEAR_AND_STARTWAITNEXTPOSN;
			break;
		case POSNMODE_STOP_MOVEMENT:
			#ifdef OUTPUT_POSITION_STATES
			logf("@PM_STOP_MV\r\n");
#endif
			MCStopPosition();
#ifdef CLAMP_SYSTEM_NAC_STP
			if (g_cNAC==1)
			{
				cresult=NACClampPrepMove(0,-16,-16,0); //itime=0 ... should stop A motion.
			}
#endif
			g_cMoveDone = MOVEDONE_TRUE;
			SendCurPosnInd(0, STP_ALERT); //Send Cur Posn Index, though it isn't on a hole
			//clear automove
			g_cAutoMove = 0;
			logf(">>>>MOVE DN\r\n\r\n");
			//Go Back to Main Wait State
			goto POSNMODE_CLEAR_AND_STARTWAITNEXTPOSN;
			break;
		case POSNMODE_MOVE_NEAREST:
			// request current machine posn
			MCGetPosition();
			s_uiStartTime = MS_TIMER;
			g_cPosnModeState = POSNMODE_MOVE_NEAREST_WAITCOORDS;
			//Ensure that StartProcess is clear
			g_cStartProcess = 0;
			g_cDrillStateGoal = DRILLSTATE_IDLE;
#ifdef SEAL_SYSTEM
			g_cSealState=SEALSTATE_OFF;
#endif
#ifdef FASTENER_SYSTEM
			g_cFillState=FILLSTATE_OFF;
			g_cFastenerArrived=0; //allow clearance, but hope it will redetect it
#endif
#ifdef OUTPUT_POSITION_STATES
			logf("@PM_MOVE_NR_WTCRDS\r\n");
#endif
			break;
		case POSNMODE_MOVE_NEAREST_WAITCOORDS:
			if (g_PosnMode.cFreshCurPosn == X_AND_Y)
			{
				//Ensure that StartProcess is clear so no path will start operations from here
				g_cStartProcess = 0;
				//Move to nearest now.
				i = SpecifyGotoPosn(GOTOPOSN_NEAREST, 0);
				if (i != GOTOPOSN_TOOFAR && i != GOTOPOSN_OUTOFBOUNDS)
				{
					//do not move to nearest
					g_cPosnModeState = POSNMODE_STARTWAITNEXTPOSN;
				}
				else
				{
					//Clear Trigger set by move nearest GotoPosition.
					//This is needed since code executes directly here, and does not need to hit the trigger above
					g_PosnMode.cDoMoveTrig = FALSE;
					//Go to Start that is start of move
					g_cPosnModeState = POSNMODE_TOOLCHECK;
#ifdef OUTPUT_POSITION_STATES
					logf("@PM_TOOLCHECK\r\n");
#endif
				}
			}
			else if ((MS_TIMER - s_uiStartTime)
					>= (POSN_TIMEOUT_SEC * 1000L))
			{
#ifdef USE_OUTPUT
				logf("timeout curposn mv nearest.\r\n");
#endif
				g_cPosnModeState = POSNMODE_STOP_MOVEMENT;
			}

			break;
		case POSNMODE_INSPECT:
			//In Position mode, the above system has placed the laser over the hole, and now is ready to inspect.
			//clear station goal back to drill
//after this... go backt o drill station
			g_cStationGoal = STATION_DRILL;

#ifdef CENTERVISION
			g_cAction = ACTION_INSPECT;
			PositionInspection();
			if (g_cPositionInspection == 1)
			{
				//still working on it
				break;
			}
#endif
			//Done
			SendCurPosnInd(0, STP_ALERT); //Send Cur Posn Index, though it isn't on a hole

			//Return to main POSNMODE State
			goto POSNMODE_CLEAR_AND_STARTWAITNEXTPOSN;
			POSNMODE_CLEAR_AND_STARTWAITNEXTPOSN:
			//Ensure that StartProcess is clear
			g_cStartProcess = 0;
			g_cDrillStateGoal = DRILLSTATE_IDLE;
#ifdef SEAL_SYSTEM
			g_cSealState=SEALSTATE_OFF;
#endif
#ifdef FASTENER_SYSTEM
			g_cFillState=FILLSTATE_OFF;
			g_cFastenerArrived=0; //allow clearance, but hope it will redetect it

//f-abort-code-loc-1
			if (g_FastenerFault.cSeverity==0)
			{
				if (g_cFastenerMode!=FASTENER_MODE_NO_FASTENER && g_cFastenerMode!=FASTENER_MODE_READY)
				{
					//Fastener Abort
					FastenerSTPSet(FASTENER_OID_ABORT);
#ifdef FASTENER_TRAY_STP
					FastenerTraySTPSet(FASTENERTRAY_OID_ABORT);
#endif
				}
			}
#endif
			g_cPosnModeState = POSNMODE_STARTWAITNEXTPOSN;
			break;
		}
		break;

	case MODE_TEACH:
		if (s_cModeStatePrev != g_cModeState)
		{
			s_cTeachState = TEACH_INIT;
		}

		switch (s_cTeachState)
		{
		case TEACH_INIT:
			g_Probe.cTeachCaptureTrig = FALSE;
			g_Probe.cGotTeachCoords = 0; //reset flag which indicates if we recevied coords
			g_iHoleCount = 0;
			//copy filename over
			//erxc Should this text be internationalized?
			strcpy(g_szPartPgmFilename, "Teach");
			g_PartPgmInfo.cLocked = 0;
			g_PartPgmInfo.cMiniFtFormat = 0;
			g_PartPgmInfo.cChecksum = 0;
			memset(g_PartPgmInfo.p_cChecksum16, 0, 16);
			g_PosnMode.iCurPosnIndex = 0;
			g_PosnMode.cFirstMove = 1; //Trigger 1st move in Posn Init and Indicates that Next must go to Current, and not add or subtract one.  Other paths will overwrite this as usual.
			g_cAllowKVisit = 1; //in teach mode, all holes can be visited.
			g_cScaleBasedOnProbe = 0; //And Turn Off Scaling
			g_PartPgmInfo.cTeachModeData = 1;
			s_cTeachState = TEACH_WAIT;
			LEDOff()
			;
			break;

		case TEACH_WAIT:
			//if parameters indicate that the current program is not a taught program, then
			//clear the program and start over
			if (g_PartPgmInfo.cTeachModeData == 0)
			{
				//If they missed init, or did something to bypass init... must init
				s_cTeachState = TEACH_INIT;
				break;
			}

			// wait for user to set MINIFT_OID_K2 or to press the red button
#ifdef UNIBUTTON
			if (g_cUniButtonEvent == 1)
			{
				// capured a single button press
				g_Probe.cTeachCaptureTrig = TRUE;
				g_cUniButtonEvent=0;//clear event
			}
#endif

			if (g_Probe.cTeachCaptureTrig == TRUE)
			{ // g_Probe.cTeachCaptureTrig can be set by UniButton (above) or by a SET of MINIFT_OID_POSN_ADD
				g_Probe.cGotTeachCoords = 0;
				ProbeTeachPosition();
				logf("sent p rqst\r\n");   //debug
				g_Probe.cTeachCaptureTrig = FALSE;  // reset flag
				s_cTeachState = TEACH_WAITCOORDS;
				s_uiStartTime = MS_TIMER;
			}

			break;
		case TEACH_WAITCOORDS:
			// wait here until we receive a msg that starts with "Done:K2Capture:"
			// once that msg is rxvd, GotK2Coords will be set for us
			if (g_Probe.cGotTeachCoords == X_AND_Y)
			{
				logf("got TEACH coord ack\r\n");                    //debug

				//Call the function to really create this position.
				CaptureTeachPosition();
				g_Probe.cGotTeachCoords = 0; //reset flag
				s_cTeachState = TEACH_WAIT; // go back and wait
			}
			else if ((MS_TIMER - s_uiStartTime) >= (PROBE_TIMEOUT_SEC * 1000L))
			{
#ifdef USE_OUTPUT
				logf("timeout prbres\r\n");
#endif
				s_cTeachState = TEACH_WAIT; // go back and wait
			}

			break;
		}
		break;
	case MODE_INSPECT:
		//The Stand alone inspection mode
		//Do the inspection right here
		if (s_cModeStatePrev != g_cModeState)
		{
			if (g_cFloatStatus == FLOATSTAT_FLOAT)
			{
				DoFloat(FLOAT_UNFLOAT_STOP); // Exit from float, and do not move to nearest hole.
			}
			LEDProbeK()
			; //use same flash pattern

#ifdef CENTERVISION
			g_cCenterVisionResult = CENTERVISION_OFF;
			//g_cCenterVisionInspectType Set by OID when the inspection is activated
			g_cCenterVisionRequiredResults = CENTERVISION_CENTER; //default
			g_fCenterVisionExpectedDiameter = 0.0; //unknown
			g_VisionInspectResults.cContext = 3; //Inspect
			g_VisionInspectResults.lposn = -1;
			g_VisionInspectResults.fXPositionExpected = 0;
			g_VisionInspectResults.fYPositionExpected = 0;
			g_cInspectMethod = g_ConfigData.cCommandInspectMethod;
#ifdef CENTERVISION_ANALOG_POINT_LASER
			if (g_cInspectMethod == INSPECT_LASER)
			{
				SetToolOffset(g_ConfigData.LaserSensorOffset.fX,
						g_ConfigData.LaserSensorOffset.fY);
			}
#endif
#ifdef CENTERVISION_CAM
			if (g_cInspectMethod == INSPECT_CAMERA) //Inspection is with Camera
			{
				SetToolOffset(g_ConfigData.CamOffset.fX,
						g_ConfigData.CamOffset.fY);
			}
#endif
#ifdef CENTERVISION
			//Store these with Center Fision
			g_fCenterVisionOffsetX = g_MachineOffset.fX;
			g_fCenterVisionOffsetY = g_MachineOffset.fY;
#endif
			//Use inspection flag
			g_cPositionInspection = 1;
#endif
			ResetNearestPosition();
		}
		if (g_cFloatStatus == FLOATSTAT_FLOAT)
		{
			break; //still in float...
		}
#ifdef CLAMP_SYSTEM
		if (g_cEEOption != EENONE)
		{
			if (g_cClampState != CLAMP_UNCLAMP)
			{
				//Should remove clamp
				g_cClampGoal = CLAMP_UNCLAMP;
				break;
			}
		}
#endif
#ifdef CENTERVISION
		PositionInspection();
		if (g_cPositionInspection == 1)
		{
			//still working on it
			break;
		}
#endif
		//Simply return to MODE_IDLE...
		g_cModeState = MODE_IDLE;
		break;
	case MODE_ESTOP:
		// what is performed when entering and exiting this Estop state is
		// determined by EstopEngageActions() and EstopDisengageActions() both of which are called above
		// even though the guts of this state do nothing, it is useful
		// to have it as a separate mode state so the user can query
		// and determine if the system is in an estop state
		g_cAction = ACTION_ESTOP;
		EstopContinueActions();
#ifdef USE_BOOTLOADER
		if (g_cEthernetDownloaderEnabled==1)
		{
			while (UDPDL_Tick()) // this should be called at least twice a second.
			{
				//boot loader is operating
			}
		}
#endif
		if (g_cGravCompStatus == GRAVCOMP_RUNNING)
		{
			//must conclude it failed
			g_cGravCompStatus = GRAVCOMP_NOTDONE;
			SmartToolMsgChar(0, STP_ALERT, MINIFT_OID_GRAVCOMP_STATUS, g_cGravCompStatus);
		}
#ifdef CLAMP_SYSTEM_HD_PISTON
		//Even Though Clamp goes loose, force Alock on
		g_cALock=ALOCK_ON;
#ifdef DIGOUT_CHNUM_ALOCK
		digOut(DIGOUT_CHNUM_ALOCK, g_cALock );
#endif
#endif

		//ensure process not set to run during this time
		g_cStartProcess = 0;
		//turn off any auto mode movements
		g_cAutoMove = 0;
		g_uiStartAutoTime = 0;
		g_cAutoRepeat = 0;
		//shut down move trigger
		g_PosnMode.cDoMoveTrig = FALSE;
		//set light off
		LEDOff()
		;
		break;
	default:
		break;
	}

// g_cModeState may have been changed in the state machine. Use the previous state
	s_cModeStatePrev = cEntryState;

//DRILL_DIRECT_PROCESS_AND_OVERRIDE
	if (g_cDrillSync == DRILL_SYNC)
	{
		if (g_cDrillLoadProcessAndOverride > DRILL_LOAD_PROCESS_NOT_NEEDED)
		{
//FIXME SEVERE GENCIRCMFTX
#ifndef GENCIRCMFTX
			if (g_cDrillLoadProcessAndOverride<DRILL_LOAD_PROCESS_READY)
			{
				//see if drill is finally ready to load the process
				if (g_cDrillStateGoal!=DRILLSTATE_SPINUP && g_cDrillState==DRILLSTATE_IDLE)
				{
					logf("Sdre\r\n");
					g_cDrillLoadProcessAndOverride=DRILL_LOAD_PROCESS_READY;
					g_ulLPR=MS_TIMER;
				}
			}
			if (g_cDrillLoadProcessAndOverride<DRILL_LOAD_PROCESS_DONE)
			{
				LoadProcessAndOverride();
			}
#endif
		}
	}
//    else
//	{
//if (g_cPrintAuth==0)
//{
//logf("nos\r\n");
//}
//	}
//Used to help determine if some operations are possible
	g_cClear = 1;

//FIXME use better EEOption Pattern Idea to improve the pattern below....

#ifdef HOMESYSTEM
//Homing Logic
	while (g_cHomed == HOME_RUNNING)
	{
		g_cClear = 0;
#ifdef HOMESYSTEM_DRILL
		if (g_cEEOption==EENONE)
		{
			g_cDrillHomed = HOME_DONE; //FIXME ??? is this good or should I make it know that it doesn't need drill home when EENONE?
			g_cDrillState = DRILLSTATE_IDLE;
		}
		if (g_cDrillHomed != HOME_DONE)
		{
			if (g_cDrillHomed == HOME_PENDING)
			{
				//Start Drill Home
				//HOW TO FIND THIS CASE WITH NEW CODE?????????????
				//FAILURE due to drill not being ready to home
				//g_cDrillHomed = HOME_FAILURE;
				g_cDrillStateGoal=DRILLSTATE_HOME;
				g_cDrillStateGoalSent=0xFF;//Clear this to force send.
				g_cDrillHomeWasStarted = 0;
				g_cDrillHomed = HOME_RUNNING;
#warnt "How to know HOME failed????"
			}
			if (g_cDrillHomed == HOME_RUNNING)
			{
				break;
			}
			if (g_cDrillHomed == HOME_FAILURE)
			{
				//Can't continue
				g_cHomed = HOME_FAILURE;
				break;
			}
			if (g_cDrillHomed == HOME_NOT_DONE)
			{
				//Not Triggered but not done either... must not be done
				//Can't continue on to next axis
				g_cHomed = HOME_NOT_DONE;
				break;
			}
		}
		if (g_cDrillState != DRILLSTATE_IDLE)
		{
			//It's no longer home!!!!!!!  Must fail
			//FIXME  not done dealing with this.. must find best way...
			g_cHomed = HOME_FAILURE;
			break;
		}
#endif
		//NAC Clamp Homing - Start Homing Portion
		if (g_cEEOption == EENONE || g_cEEOption == EEDRILLFILL)
		{
#ifdef HOMESYSTEM_CLAMP
			g_cNACClampHomed = HOME_DONE; //FIXME ??? is this good or should I make it know that it doesn't need drill home when EENONE?
#endif
#ifdef HOMESYSTEM_AAXIS
			g_cNACAAxisHomed = HOME_DONE;
#endif
#ifdef HOMESYSTEM_FASTENER
			g_cFastenerHomed = HOME_DONE; //FIXME ??? is this good or should I make it know that it doesn't need drill home when EENONE?
#endif
#ifdef HOMESYSTEM_FASTENERTRAY
			g_cFastenerTrayHomed = HOME_DONE; //FIXME ??? is this good or should I make it know that it doesn't need drill home when EENONE?
#endif
		}
#ifdef HOMESYSTEM_CLAMP
		if (g_cNACClampHomed == HOME_PENDING)
		{
			//Start NAC Clamp Home
			//FIXME Minor  Considing moving the failure detail down into the function...
			//FIXME Minor use the failure reasons to indicate why
			logf("NACHmClamp()\r\n");
			if (NACHomeClamp())
			{
				g_cNACClampHomed = HOME_RUNNING;
				g_cClampGoal = CLAMP_UNCLAMP; //need to set this goal also....
			}
			else
			{
				g_cNACClampHomed = HOME_FAILURE;
			}
		}
#endif
#ifdef HOMESYSTEM_FASTENER
		//Fastener Homing - Start Homing Portion
		if (g_cFastenerHomed == HOME_PENDING)
		{
			//Start Fastener Home
			//FIXME Minor  Considing moving the failure detail down into the function...
			//FIXME Minor use the failure reasons to indicate why
			if (FastenerSTPSetUInt(FASTENER_OID_HOME,1)==1)
			{
				logf("FH\r\n");
				g_cFastenerHomed = HOME_RUNNING;
			}
			else
			{
				logf("Fts\r\n");
				g_cFastenerHomed = HOME_FAILURE;
			}
		}
#endif
#ifdef HOMESYSTEM_CLAMP
		//NAC Clamp Homing - Homing Wait Portion
		if (g_cNACClampHomed == HOME_RUNNING)
		{
			break;
		}
#endif
#ifdef HOMESYSTEM_FASTENER
		//Fastener Homing - Homing Wait Portion
		if (g_cFastenerHomed == HOME_RUNNING)
		{
			break;
		}
#endif
#ifdef HOMESYSTEM_CLAMP
		//NAC Clamp Homing - Homing Check Portion
		if (g_cNACClampHomed == HOME_FAILURE)
		{
			//Can't continue
			g_cHomed = HOME_FAILURE;
			break;
		}
		if (g_cNACClampHomed != HOME_DONE)
		{
			//Not Triggered but not pending, not running, not failure, not done... must not be done
			//Can't continue on to next axis
			g_cHomed = HOME_NOT_DONE;
			break;
		}
		//Now g_cNACClampHomed == HOME_DONE
#endif
#ifdef HOMESYSTEM_FASTENER
		//Fastener Homing - Homing Check Portion
		if (g_cFastenerHomed == HOME_FAILURE)
		{
			//Can't continue
			g_cHomed = HOME_FAILURE;
			break;
		}
		if (g_cFastenerHomed != HOME_DONE)
		{
			//Not Triggered but not pending, not running, not failure, not done... must not be done
			//Can't continue on to next axis
			g_cHomed = HOME_NOT_DONE;
			break;
		}
#endif
#ifdef HOMESYSTEM_CLAMP
		if (g_cClampGoal == CLAMP_CLAMP)
		{
			//Can't continue on to next axis
			g_cHomed = HOME_NOT_DONE;
			break;
		}

		if (g_cNACClear==0)
		{
			//For X and Y Home, NAC must be clear
			logf("*nnc\r\n");
			g_cHomed = HOME_FAILURE;
			break;
		}

		//FIXME0000000000
		// FIXME SEVERE
		//Big problem is that I want to make sure it's clear, BUT AAxis Home will set this to TRANSITION....
		//So I can't use this to check...
		//Make Smarter In NAC
		//if (g_cClampState != CLAMP_UNCLAMP)
		//{
		//	//It's no longer unclamp!!!!!!!  Must fail
		//    //FIXME  not done dealing with this.. must find best way...
		//	g_cHomed = HOME_FAILURE;
		//    break;
		//}
#else
		//No Nac C Axis Home System
#ifdef HOMESYSTEM_CLAMP_UNCLAMP_ONLY
		if (g_cHomedX == HOME_PENDING || g_cHomedY == HOME_PENDING) //Temporary problemwith LFT2 calls for need to continue on if nothing is pending..
		{
			if (g_cClampState != CLAMP_UNCLAMP)
			{
				g_cClampGoal = CLAMP_UNCLAMP;
				break;
			}
		}
#endif
#endif
#ifdef HOMESYSTEM_X_LINK
		if (g_cHomedX == HOME_PENDING)
		{
			//StartX Home (or log failure)
			logf("RunHmX\r\n");
#ifdef HOMESYSTEM_X_MC
			SetDriveThroughBacklash(0,0,0); //FIXME HIGH  check on this call in relation to new system ... move to lib ????
			//Select the proper position limit variables
			SelectHomePositionAndPositionLimits();
			//Run Home
			RunHomeX();
#endif
			g_cHomedX = HOME_RUNNING;
		}
#endif
#ifdef HOMESYSTEM_Y_LINK
		if (g_cHomedY == HOME_PENDING)
		{
			//StartY Home (or log failure)
			logf("RunHmY\r\n");
#ifdef HOMESYSTEM_Y_MC
#ifdef BEEPSYSTEM
			if (g_cBeepMode != BEEPPROBEHOME && g_cBeepMode < BEEPSIGNALS)
			{
				BeepProbeHome()
				;
			}
#endif
#ifdef USE_DRIVE_THROUGH_BACKLASH_CODE
			SetDriveThroughBacklash(0, 0, 0);
#endif
			//FIXME SEVERE CIRCMFT1
			//Previous Implementations of Y Home have been integrated with Y Limit sensors for protection...
			//Confirm on one of those systems that limits will also stop the new Y Home

			//Select the proper position limit variables
			SelectHomePositionAndPositionLimits();
			//Run Home
			RunHomeY();
#endif
			g_cHomedY = HOME_RUNNING;
		}
#endif
#ifdef HOMESYSTEM_AAXIS
		if (g_cNACAAxisHomed == HOME_PENDING)
		{
			//Start A Home
			//FIXME Minor  Considing moving the failure detail down into the function...
			//FIXME Minor use the failure reasons to indicate why
			logf("NACHmAAxis()\r\n");
			if (NACHomeAAxis())
			{
				g_cNACAAxisHomed = HOME_RUNNING;
			}
			else
			{
				g_cNACAAxisHomed = HOME_FAILURE;
			}
		}
#endif
#ifdef HOMESYSTEM_FASTENERTRAY
		//FastenerTray Homing - Start Homing Portion
		if (g_cFastenerTrayHomed == HOME_PENDING)
		{
			//Start Fastener Tray Home
			//FIXME Minor  Considing moving the failure detail down into the function...
			//FIXME Minor use the failure reasons to indicate why
			if (FastenerTraySTPSetUInt(FASTENERTRAY_OID_HOME,1)==1)
			{
				logf("FH\r\n");
				g_cFastenerTrayHomed = HOME_RUNNING;
			}
			else
			{
				logf("Fts\r\n");
				g_cFastenerTrayHomed = HOME_FAILURE;
			}
		}
#endif
		//Now Unlinked X Home, Like RFID is also controlled by the home system inline,
		//but it currently must ensure that Y Home is done 1st
#ifdef HOMESYSTEM_X
#ifdef HOMESYSTEM_X_RFID
#ifdef HOMESYSTEM_Y_LINK
		if (g_cHomedY == HOME_RUNNING)
		{
			break;
		}
#endif
		if (g_cHomedX == HOME_PENDING)
		{
			g_cHomedX = HOME_RUNNING;
			//Make sure it will read RFID
			g_cReadRFID = RFID_READ_SEEK;
#ifdef USE_RFID_OMRON
			g_cTagReadState = RFID_TAG_CLEAR;
#endif
//					if (g_RFIDData.cseekstate>RFID_SEEK_TERMINAL_STATE)
//					{
//FIXME Med Timeout?????
			g_RFIDData.cseekstate = RFID_SEEK_NULL;
//					}
//FIXME SEVERE
// restart issues
// stop issues
// how to do it... I am puposely delaying those thins so I can work on the homing
		}
#endif
#endif
#ifdef HOMESYSTEM_X_LINK
		if (g_cHomedX == HOME_RUNNING)
		{
			break;
		}
#endif
#ifdef HOMESYSTEM_Y_LINK
		if (g_cHomedY == HOME_RUNNING)
		{
			break;
		}
#endif
#ifdef HOMESYSTEM_AAXIS
		if (g_cNACAAxisHomed == HOME_RUNNING)
		{
			break;
		}
#endif
#ifdef HOMESYSTEM_FASTENERTRAY
		//FastenerTray Homing - Homing Wait Portion
		if (g_cFastenerTrayHomed == HOME_RUNNING)
		{
			break;
		}
#endif
		//Any other Axis that is not linked into the main home group does not need to be homed
		//for the MiniFT to be homed, (example RFID Home on HD Gen 4)
		//but if any other Axis is HOME_RUNNING, then we should also say MiniFT is HOME_RUNNING
#ifdef HOMESYSTEM_X
#ifdef HOMESYSTEM_X_RFID
		if (g_cHomedX == HOME_RUNNING)
		{
			//This is not needed for MiniFT Home to be done, but
			//don't say MiniFT is done if this axis is still homing
			break;
		}
#endif
#endif
		//Everything Has Compelted
#ifdef HOMESYSTEM_X_LINK
		if (g_cHomedX == HOME_FAILURE)
		{
			logf("HomeFail-X\r\n");
			g_cHomed = HOME_FAILURE;
			break;
		}
#endif
#ifdef HOMESYSTEM_Y_LINK
		if (g_cHomedY == HOME_FAILURE)
		{
			logf("HomeFail-Y\r\n");
			g_cHomed = HOME_FAILURE;
			break;
		}
#endif
#ifdef HOMESYSTEM_AAXIS
		if (g_cNACAAxisHomed == HOME_FAILURE)
		{
			logf("HomeFail-A\r\n");
			g_cHomed = HOME_FAILURE;
			break;
		}
#endif
#ifdef HOMESYSTEM_FASTENERTRAY
		//FastenerTray Homing - Homing Check Portion
		if (g_cFastenerTrayHomed == HOME_FAILURE)
		{
			//Can't continue
			g_cHomed = HOME_FAILURE;
			break;
		}
#endif
#ifdef HOMESYSTEM_X_LINK
		if (g_cHomedX == HOME_NOT_DONE)
		{
			g_cHomed = HOME_NOT_DONE;
			break;
		}
#endif
#ifdef HOMESYSTEM_Y_LINK
		if (g_cHomedY == HOME_NOT_DONE)
		{
			g_cHomed = HOME_NOT_DONE;
			break;
		}
#endif
#ifdef HOMESYSTEM_AAXIS
		if (g_cNACAAxisHomed == HOME_NOT_DONE)
		{
			g_cHomed = HOME_NOT_DONE;
			break;
		}
#endif
#ifdef HOMESYSTEM_FASTENERTRAY
		if (g_cFastenerTrayHomed != HOME_DONE)
		{
			//Not Triggered but not pending, not running, not failure, not done... must not be done
			//Can't continue on to next axis
			g_cHomed = HOME_NOT_DONE;
			break;
		}
#endif

		logf("HomeDone.\r\n");
		g_cHomed = HOME_DONE;
		break;
	}

	if (g_cHomed != HOME_RUNNING)
	{
#ifdef HOMESYSTEM_DRILL
		if (g_cDrillHomed == HOME_PENDING)
		{
			g_cDrillHomed = HOME_NOT_DONE;
		}
#endif
#ifdef HOMESYSTEM_CLAMP
		if (g_cNACClampHomed == HOME_PENDING)
		{
			g_cNACClampHomed = HOME_NOT_DONE;
		}
#endif
#ifdef HOMESYSTEM_AAXIS
		if (g_cNACAAxisHomed == HOME_PENDING)
		{
			g_cNACAAxisHomed = HOME_NOT_DONE;
		}
#endif
#ifdef HOMESYSTEM_FASTENER
		if (g_cFastenerHomed == HOME_PENDING)
		{
			g_cFastenerHomed = HOME_NOT_DONE;
		}
#endif
#ifdef HOMESYSTEM_FASTENERTRAY
		if (g_cFastenerTrayHomed == HOME_PENDING)
		{
			g_cFastenerTrayHomed = HOME_NOT_DONE;
		}
#endif
#ifdef HOMESYSTEM_X
		if (g_cHomedX == HOME_PENDING)
		{
			g_cHomedX = HOME_NOT_DONE;
		}
#endif
#ifdef HOMESYSTEM_Y
		if (g_cHomedY == HOME_PENDING)
		{
			g_cHomedY = HOME_NOT_DONE;
		}
#endif

		//If Home is not running, then check all home status this way to determine if status has changed due to independant running and actions
		//If anything has failed, then homing has failed
		if (
		#ifdef HOMESYSTEM_DRILL
		g_cDrillHomed == HOME_FAILURE ||
#endif
#ifdef HOMESYSTEM_CLAMP
		g_cNACClampHomed == HOME_FAILURE ||
#endif
#ifdef HOMESYSTEM_AAXIS
		g_cNACAAxisHomed == HOME_FAILURE ||
#endif
#ifdef HOMESYSTEM_FASTENER
		g_cFastenerHomed == HOME_FAILURE ||
#endif
#ifdef HOMESYSTEM_FASTENERTRAY
		g_cFastenerTrayHomed == HOME_FAILURE ||
#endif
#ifdef HOMESYSTEM_X_LINK
		g_cHomedX == HOME_FAILURE ||
#endif
#ifdef HOMESYSTEM_Y_LINK
		g_cHomedY == HOME_FAILURE
				#else
				0
#endif
				)
		{
			g_cHomed = HOME_FAILURE;
		}
		//If anything is not done, then homing is not done
		else if (
		#ifdef HOMESYSTEM_DRILL
		g_cDrillHomed == HOME_NOT_DONE ||
#endif
#ifdef HOMESYSTEM_CLAMP
		g_cNACClampHomed == HOME_NOT_DONE ||
#endif
#ifdef HOMESYSTEM_AAXIS
		g_cNACAAxisHomed == HOME_NOT_DONE ||
#endif
#ifdef HOMESYSTEM_FASTENER
		g_cFastenerHomed == HOME_NOT_DONE ||
#endif
#ifdef HOMESYSTEM_FASTENERTRAY
		g_cFastenerTrayHomed == HOME_NOT_DONE ||
#endif
#ifdef HOMESYSTEM_X_LINK
		g_cHomedX == HOME_NOT_DONE ||
#endif
#ifdef HOMESYSTEM_Y_LINK
		g_cHomedY == HOME_NOT_DONE
				#else
				0
#endif
				)
		{
			g_cHomed = HOME_NOT_DONE;
		}
		//If anything is running, then home is not done.
		//Home is not set to running, unless they are running the overall full system home.
		else if (
		#ifdef HOMESYSTEM_DRILL
		g_cDrillHomed == HOME_RUNNING ||
#endif
#ifdef HOMESYSTEM_CLAMP
		g_cNACClampHomed == HOME_RUNNING ||
#endif
#ifdef HOMESYSTEM_AAXIS
		g_cNACAAxisHomed == HOME_RUNNING ||
#endif
#ifdef HOMESYSTEM_FASTENER
		g_cFastenerHomed == HOME_RUNNING ||
#endif
#ifdef HOMESYSTEM_FASTENERTRAY
		g_cFastenerTrayHomed == HOME_RUNNING ||
#endif
#ifdef HOMESYSTEM_X_LINK
		g_cHomedX == HOME_RUNNING ||
#endif
#ifdef HOMESYSTEM_Y_LINK
		g_cHomedY == HOME_RUNNING
				#else
				0
#endif
				)
		{
			//can't set g_cHomed=done yet, but experimentally trying setting it to not done
			g_cHomed = HOME_NOT_DONE;
		}
		else
		{
			//Every Axis was not failed, not_done, or running, so that means ever axis was HOME_DONE
			g_cHomed = HOME_DONE;
		}
	}

	if (g_cSentHomed != g_cHomed)
	{
		logf("Homed=%d\r\n", g_cHomed);
		AlertHomeStatus();
		g_cSentHomed = g_cHomed;
		//Change Action Back Here
		if (g_cHomed != HOME_RUNNING)
		{
			if (g_cAction == ACTION_HOME)
			{
				if (g_cModeState == MODE_IDLE)
				{
					g_cAction = ACTION_IDLE;
				}
				if (g_cModeState
						== MODE_POSN && g_cPosnModeState == POSNMODE_WAITNEXTPOSN) //FIXME Still annoyed by this pattern.
				{
					g_cAction = ACTION_READY;
				}
			}
		}
	}

	RFIDRun();

//FIXME000000000000 I don't use the status reason right now but I could...

#ifdef HOMESYSTEM_X
	if (g_cSentHomedX != g_cHomedX)
	{
		AlertHomeReport(AXIS_X, g_cHomedX, HOMESR_NULL);
		g_cSentHomedX = g_cHomedX;
	}
#endif
#ifdef HOMESYSTEM_Y
	if (g_cSentHomedY != g_cHomedY)
	{
		AlertHomeReport(AXIS_Y, g_cHomedY, HOMESR_NULL);
		g_cSentHomedY = g_cHomedY;
	}
#endif
#ifdef HOMESYSTEM_DRILL
	if (g_cSentDrillHomed != g_cDrillHomed)
	{
		logf("dhr %d\r\n",g_cDrillHomed);
		AlertHomeReport(AXIS_Z, g_cDrillHomed, HOMESR_NULL);
		g_cSentDrillHomed = g_cDrillHomed;
	}
#endif
#ifdef HOMESYSTEM_CLAMP
	if (g_cSentNACClampHomed != g_cNACClampHomed)
	{
		AlertHomeReport(AXIS_CLAMP, g_cNACClampHomed, HOMESR_NULL);
		g_cSentNACClampHomed = g_cNACClampHomed;
	}
#endif
#ifdef HOMESYSTEM_AAXIS
	if (g_cSentNACAAxisHomed != g_cNACAAxisHomed)
	{
		AlertHomeReport(AXIS_A, g_cNACAAxisHomed, HOMESR_NULL);
		g_cSentNACAAxisHomed = g_cNACAAxisHomed;
	}
#endif
#ifdef HOMESYSTEM_FASTENER
	if (g_cSentFastenerHomed != g_cFastenerHomed)
	{
		AlertHomeReport(AXIS_FF, g_cFastenerHomed, HOMESR_NULL);
		g_cSentFastenerHomed = g_cFastenerHomed;
	}
#endif
#ifdef HOMESYSTEM_FASTENERTRAY
	if (g_cSentFastenerTrayHomed != g_cFastenerTrayHomed)
	{
		AlertHomeReport(AXIS_FTA, g_cFastenerTrayHomed, HOMESR_NULL);
		g_cSentFastenerTrayHomed = g_cFastenerTrayHomed;
	}
#endif

#endif

//EEOPTION AND CLAMPING PATTERNS
//The system may from time to time set ClampGoals for tools
//which are not selected.  This a good space and time saving shortcut.
//Only the EEOPTION selected will act on it's goal, and only having
//an EEOPTION selected will cause code to wait for state changes on flags
//which belong to a specific EEOPTION.

#ifdef PRESSURESENSOR
	if (g_cAirPressureEvent != 2)
	{
		//Alert the lock out with OID....
		//OID_AIR_PRESSURE ALERT SEND
		SmartToolMsgChar(p_STPSession, STP_ALERT, MINIFT_OID_AIR_PRESSURE, g_cDigInAirPressure);
		g_cAirPressureEvent = 2;//clear event
	}
	if (g_cDigInAirPressure == 1)
	{
		//FIXME0 this call was causing trouble and
		//problem may be here, where it's CONTINUOUSLY CALLED....
		//not just on entery.....

		PressureLossActions();
		//	//must clear clamp
		//	g_cClampGoal = CLAMP_UNKNOWN;
	}
#endif

#ifdef ORIENTATION_SENSORS

#ifdef FORCE_NEG
	g_cDigInYOrientationA = 0;
	g_cDigInYOrientationB = 1;
#endif
	if (g_cDigInYOrientationA==0) //0 Means that it sees the magnet
	{
		g_ConfigData.cToolFlip = Y_NEG;
		corient = Y_NEG;
	}
	else if (g_cDigInYOrientationB==0)
	{
		g_ConfigData.cToolFlip = Y_POS;
		corient = Y_POS;
	}
	else
	{
		//If All rails had the system working, then this would be unknown
		//but don't change it from whatever it is in this case.
		//g_ConfigData.cToolFlip = Y_UNKNOWN;
		corient = Y_UNKNOWN;
	}
	if (corient != g_cPrevOrSensors)
	{
		SmartToolMsgChar(p_STPSession, STP_ALERT, MINIFT_OID_TOOL_FLIP, g_ConfigData.cToolFlip);
		g_cPrevOrSensors = corient;
		if (corient == Y_NEG)
		{	corient='n';}
		else
		if (corient == Y_POS)
		{	corient='p';}
		else
		{	corient='u';}
		logf("ors%c\r\n", corient);
		//Must Rehome!
#ifdef HOMESYSTEM_X_LINK
		g_cHomed = HOME_NOT_DONE;	//overall system home status
		g_cHomedX=0;
#endif
#ifdef HOMESYSTEM_Y_LINK
		g_cHomed = HOME_NOT_DONE;	//overall system home status
		g_cHomedY=0;
#endif
	}
#endif

	if (g_cLockEvent == 1)
	{
		//Alert the lock out with OID....
		//Use Summed up lock summary
		SmartToolMsgChar(0, STP_ALERT, MINIFT_OID_MACHINE_LOCK, g_cMachineLock);
		g_cLockEvent = 0; //clear event
	}

#ifdef CLAMP_SYSTEM
//Since the only side effect of not being locked is that you can't clamp (currently anyway) only check these when trying to clamp
	if (g_cClampGoal == CLAMP_CLAMP)
	{

		if ((g_cMachineLock & g_ConfigData.cMachineLockRequired) != g_ConfigData.cMachineLockRequired) //something that was need was not locked
		{
			g_cClampGoal = CLAMP_HOLD;
		}

	}
#endif

#ifdef CLAMP_SYSTEM

#ifdef CLAMP_SYSTEM_NAC_STP
#if 0
//This Code Shows how to bypass clamping
	g_cClampState=g_cClampGoal;
#warnt "WARNING: Direct Clamp Success Code is turned on"
#else
//Must Operate using Remote Clamp
	if (g_cClampGoalSent!=g_cClampGoal)
	{
		if (g_cNAC==1)
		{
			//This Code is the Standard Clamp Code to clamp via NAC
			//Let Y Extension always be positive. NAC will have a table that accomodates the range it gets for YNEG and YPOS
			i=(int)(fabs(g_PosnMode.fLastKnownPosnY)*1000);
			//Dont have values for surface posn or angle currently...
			//  send -16 which is out of range... NAC is not setup to use these values now
			cresult=NACClamp(g_cClampGoal,i,-16,-16,(int)g_uiClampPressure,(int)g_uiClampPressureWarning,(int)g_uiClampPressureAbort);
			if (cresult)
			{
				logf("Ask Clamp=%d y=%f->%d p=%d\r\n",g_cClampGoal,g_PosnMode.fLastKnownPosnY,i,g_uiClampPressure);
				g_cClampGoalSent=g_cClampGoal;
			}
		}
#ifdef DRILLFILL_STP
		else if (g_cEEOption==EEDRILLFILL)
		{
			cresult=DrillFillClamp(g_cClampGoal);
			if (cresult)
			{
				logf("Ask Clamp=%d p=%d\r\n",g_cClampGoal,-1);
				g_cClampGoalSent=g_cClampGoal;
			}
		}
#endif
	}
#endif

//Always Alert Clamp, no matter what EEOption is being used
	if (g_cClampState!=g_cClampStateSent)
	{
		//send alert of changed state
		SmartToolMsgChar(p_STPSession, STP_ALERT, MINIFT_OID_CLAMP, g_cClampState);
		g_cClampStateSent = g_cClampState;
		logf("Alrt Clamp=%d\r\n",g_cClampState);
	}

	if (g_cClampState!=CLAMP_UNCLAMP && g_cClampState!=CLAMP_HOLD)
	{
		g_cClear=0; //treat this like it's not clear
	}
	if (g_cClampGoal!=CLAMP_UNCLAMP && g_cClampGoal!=CLAMP_HOLD)
	{
		g_cClear=0; //treat this like it's not clear
	}
#endif
#ifdef CLAMP_SYSTEM_HD_PISTON
//HD RAIL

//HD Rail Clamp Machine

	if (g_cEEOption!=EENONE)//skip the entire clamp machine if the rail is not on.
	{

		if (g_cClampState!=g_cClampGoal)
		{
			g_cClampState=CLAMP_UNKNOWN;
			c=0;
			switch (g_cClampGoal)
			{
				case CLAMP_UNKNOWN:
				//This is not used as a goal, but only as the original state
				g_cClampGoal=CLAMP_LOOSE;
				g_cAirBlastX=0;
				g_cAirBlastY=0;
				c=1;
				break;
				case CLAMP_LOOSE:
				//need to become loose.
				if (g_cALock==ALOCK_ON && g_cModeState != MODE_ESTOP)
				{
					if (g_cALockMode==0)
					{
						g_cALock=ALOCK_OFF;
						g_ulClampAUnlock=MS_TIMER;
						c=1;
					}
				}

				if (g_uiClampPressureLBS>g_ConfigData.uiLowPressure)
				{
					//still at high pressure
					g_ulClampPressureHigh=MS_TIMER;//last time it was high
					g_uiClampPressureLBS=0;
					g_cClampExtend=VALVE_OFF;
					g_cClampRetract=VALVE_OFF;
					c=1;
				}
				if (g_uiClampPressureLBS>0)
				{
					//still at low pressure (or higher)
					g_ulClampPressureLow=MS_TIMER;//last time it was low
					g_uiClampPressureLBS=0;
					g_cClampExtend=VALVE_OFF;
					g_cClampRetract=VALVE_OFF;
					c=1;
				}
				if (g_cClampExtend==VALVE_ON)
				{
					//still extended, just retract
					g_cClampExtend=VALVE_OFF;
					c=1;
				}
				if (g_cClampRetract==VALVE_ON)
				{
					//still retracted, just retract
					g_cClampRetract=VALVE_OFF;
					c=1;
				}

				if ((MS_TIMER - g_ulClampPressureHigh) < g_ConfigData.uiPressureDownDelay) //HD PISTON Clamping
				{
					//can't be considered at zero pressure yet.
					break;
				}
				if ((MS_TIMER - g_ulClampPressureLow) < g_ConfigData.uiLowPressureDownDelay)
				{
					//can't be considered at zero pressure yet.
					break;
				}

				if (g_cLegsLock==LEGSLOCK_ON)
				{
					//now can unlock
					g_cLegsLock=LEGSLOCK_OFF;
					g_ulClampLegsUnlock=MS_TIMER;
					c=1;
				}

				if ((MS_TIMER - g_ulClampLegsUnlock) < g_ConfigData.uiLegsUnlockDelay) //HD Piston Clamping
				{
					//not done yet
					break;
				}

				if (g_cLegsUp==VALVE_OFF || g_cLegsDown==VALVE_ON)
				{
					g_cLegsDown=VALVE_OFF;
					g_cLegsUp=VALVE_ON;
					g_ulClampLegsUp=MS_TIMER;
					c=1;
				}

				if ((MS_TIMER - g_ulClampLegsUp) < g_ConfigData.uiLegsUpDelay)
				{
					//not done yet
					break;
				}

				//Can be considered loose
				if (g_cModeState != MODE_ESTOP)
				{
					if (g_cALockMode==0)
					{
						g_cALock=ALOCK_OFF;
					}
				}
				g_cLegsLock=LEGSLOCK_OFF;
				g_cLegsDown=VALVE_OFF;
				g_cLegsUp=VALVE_ON;
				g_cClampExtend=VALVE_OFF;
				g_cClampRetract=VALVE_OFF;
				g_uiClampPressureLBS=0;
				g_cAirBlastX=0;
				g_cAirBlastY=0;
				c=1;

				//Now Done Loose
				g_cClampState=CLAMP_LOOSE;
				break;

				case CLAMP_UNCLAMP:
				//Unclamp should unlock A-Lock, but relock it before all pressure is removed.
//#define TWO_STEP_WAY
#ifdef TWO_STEP_WAY
				if (g_uiClampPressureLBS>g_ConfigData.uiLowPressure)
				{
					//Pressure has not been released yet.
					if (g_cALock==ALOCK_ON)
					{
						if (g_ConfigData.uiALockDelay>0) //Lock Delay = 0 stops all ALock Off
						{
							if (g_cALockMode==0) //And now LockMode Does this even better.
							{
								g_cALock=ALOCK_OFF;
								g_ulClampAUnlock=MS_TIMER;
							}
						}
						c=1;
					}

					if (g_ConfigData.uiALockDelay>0) //Lock Delay = 0 stops all ALock Off
					{
						if (g_cALockMode==0) //And now LockMode Does this even better.
						{
							if ((MS_TIMER - g_ulClampAUnlock) < g_ConfigData.uiAUnlockDelay)
							{
								//not done yet
								break;
							}
						}
					}
					//Now unlocked

					//still at high pressure
					g_ulClampPressureHigh=MS_TIMER;//last time it was high
					g_uiClampPressureLBS=g_ConfigData.uiLowPressure;
					g_cClampExtend=VALVE_ON;
					g_cClampRetract=VALVE_OFF;
					c=1;
				}

				if ((MS_TIMER - g_ulClampPressureHigh) < g_ConfigData.uiPressureDownDelay) //HD PISTON Clamping
				{
					//can't be considered at low pressure yet.
					break;
				}
				//Now At Low Pressure

				//now lock again
				if (g_cALockMode==0)//Only needed if not prelocked locked
				{
					if (g_cALock==ALOCK_OFF)
					{
						g_cALock=ALOCK_ON;
						g_ulClampALock=MS_TIMER;
						c=1;
					}
					if ((MS_TIMER - g_ulClampALock) < g_ConfigData.uiALockDelay)
					{
						//not locked yet... must be locked
						break;
					}
				}

				//Now Ready to retract

				if (g_uiClampPressureLBS>0)
				{
					//still at low pressure (or higher)
					g_ulClampPressureLow=MS_TIMER;//last time it was low
					g_uiClampPressureLBS=0;
					g_cClampExtend=VALVE_OFF;
					g_cClampRetract=VALVE_ON;
					c=1;
				}

				if ((MS_TIMER - g_ulClampPressureLow) < g_ConfigData.uiLowPressureDownDelay)
				{
					//can't be considered at zero pressure yet.
					break;
				}
#else
//This way is the fast way
				//Never Unlock
				if (g_cALockMode==0)
				{
					if (g_cALock==ALOCK_OFF)
					{
						g_cALock=ALOCK_ON;
						g_ulClampALock=MS_TIMER;
						c=1;
					}
					if ((MS_TIMER - g_ulClampALock) < g_ConfigData.uiALockDelay)
					{
						//not locked yet... must be locked
						break;
					}
				}

				if (g_uiClampPressureLBS>0)
				{
					if (g_uiClampPressureLBS>g_ConfigData.uiLowPressure)
					{
						//still at high pressure
						g_ulClampPressureHigh=MS_TIMER;//last time it was high
					}
					//still at high or low pressure
					g_ulClampPressureLow=MS_TIMER;//last time it was low
					g_uiClampPressureLBS=0;
					g_cClampExtend=VALVE_OFF;
					g_cClampRetract=VALVE_ON;
					c=1;
				}

				if ((MS_TIMER - g_ulClampPressureHigh) < g_ConfigData.uiPressureDownDelay) //HD PISTON Clamping
				{
					//can't be considered at zero pressure yet.
					break;
				}
				if ((MS_TIMER - g_ulClampPressureLow) < g_ConfigData.uiLowPressureDownDelay)
				{
					//can't be considered at zero pressure yet.
					break;
				}
#endif
				//now front is done... lift legs

				if (g_cLegsLock==LEGSLOCK_ON)
				{
					//now can unlock
					g_cLegsLock=LEGSLOCK_OFF;
					g_cLegsDown=VALVE_OFF;//let down pressure dissipate also
					g_ulClampLegsUnlock=MS_TIMER;
					c=1;
				}

				if ((MS_TIMER - g_ulClampLegsUnlock) < g_ConfigData.uiLegsUnlockDelay) //HD Piston Clamping
				{
					//not done yet
					break;
				}

				if (g_cLegsUp==VALVE_OFF || g_cLegsDown==VALVE_ON)
				{
					g_cLegsDown=VALVE_OFF;
					g_cLegsUp=VALVE_ON;
					g_ulClampLegsUp=MS_TIMER;
					c=1;
				}

				if ((MS_TIMER - g_ulClampLegsUp) < g_ConfigData.uiLegsUpDelay)
				{
					//not done yet
					break;
				}

				//Can be considered unclamped
				g_cALock=ALOCK_ON;
				g_cLegsLock=LEGSLOCK_OFF;
				g_cLegsDown=VALVE_OFF;
				g_cLegsUp=VALVE_ON;
				g_cClampExtend=VALVE_OFF;
				g_cClampRetract=VALVE_ON;
				g_uiClampPressureLBS=0;
				g_cAirBlastX=0;
				g_cAirBlastY=0;
				c=1;

				//Now Done Unclamp
				g_cClampState=CLAMP_UNCLAMP;
				break;

				case CLAMP_CLAMP:
				//need to become clamped

				//ensure that main pressure is set higher than low pressure
				if (g_uiClampPressure < g_ConfigData.uiLowPressure)
				{
					g_uiClampPressure = g_ConfigData.uiLowPressure + 1;
				}

#define EXTEND_BEFORE_LEGS
#ifdef EXTEND_BEFORE_LEGS
				//Need to extend
				if (g_cClampExtend==VALVE_OFF)
				{
					//need to go to very low pressure while legs are going out
					g_uiClampPressureLBS=10;
					if (g_uiClampPressureLBS>g_ConfigData.uiLowPressure)
					{
						g_uiClampPressureLBS=g_ConfigData.uiLowPressure-1;
						if (g_uiClampPressureLBS<1)
						{
							g_uiClampPressureLBS=1;
						}
					}
					g_ulClampPressureZero=MS_TIMER; //must be less than balance pressure
					g_cClampExtend=VALVE_ON;
					g_cClampRetract=VALVE_OFF;
					c=1;
				}
#endif
				if (g_cLegsLock==LEGSLOCK_OFF)
				{
					//leg lock is not on

					//make sure legs are down
					if (g_cLegsDown==VALVE_OFF || g_cLegsUp==VALVE_ON)
					{
						g_cLegsDown=VALVE_ON;
						g_cLegsUp=VALVE_OFF;
						g_ulClampLegsDown=MS_TIMER;
						c=1;
					}

					if ((MS_TIMER - g_ulClampLegsDown) < g_ConfigData.uiLegsDownDelay)
					{
						//not done yet
						break;
					}

					g_cLegsLock=LEGSLOCK_ON;
					g_ulClampLegsLock=MS_TIMER;
					c=1;
				}

				if ((MS_TIMER - g_ulClampLegsLock) < g_ConfigData.uiLegsLockDelay)
				{
					//not done yet
					break;
				}
				//Legs are locked

				//Need to extend
				if (g_cClampExtend==VALVE_OFF)
				{
					g_cClampExtend=VALVE_ON;
					g_cClampRetract=VALVE_OFF;
					c=1;
				}

				//need to go to low pressure
				if (g_uiClampPressureLBS<g_ConfigData.uiLowPressure)
				{
					g_ulClampPressureZero=MS_TIMER; //must be less than balance pressure
					g_uiClampPressureLBS=g_ConfigData.uiLowPressure;
					g_cClampExtend=VALVE_ON;
					g_cClampRetract=VALVE_OFF;
					c=1;
				}
				//confirm for low pressure
				if ((MS_TIMER - g_ulClampPressureZero) < g_ConfigData.uiLowPressureDelay)
				{
					//can't be considered at low pressure yet.
					break;
				}

				if (g_uiClampPressureLBS!=g_uiClampPressure)
				{
					//Unlock A Lock.
					if (g_cALock==ALOCK_ON && g_ConfigData.uiALockDelay>0)//Lock Delay = 0 stops all ALock Off
					{
						if (g_cALockMode==0)
						{
							g_cALock=ALOCK_OFF;
							g_ulClampAUnlock=MS_TIMER;
							c=1;
						}
					}
					g_ulClampUnlockTime=MS_TIMER; //Used explicitly for Clamp To Drill Timing.

					//Ask for pressure
					g_ulClampPressureLow=MS_TIMER;//must be less than high pressure
					g_uiClampPressureLBS=g_uiClampPressure;
					g_cClampExtend=VALVE_ON;
					g_cClampRetract=VALVE_OFF;
					c=1;
				}

				//need to achieve high pressure
				if ((MS_TIMER - g_ulClampPressureZero) < g_ConfigData.uiPressureDelay)
				{
					//can't be considered at high pressure yet.
					break;
				}
				//need to achieve high pressure
				if ((MS_TIMER - g_ulClampPressureLow) < g_ConfigData.uiPressureDelay)
				{
					//can't be considered at high pressure yet.
					break;
				}
				if (g_ConfigData.uiALockDelay>0) //Lock Delay = 0 stops all ALock Off
				{
					if (g_cALockMode==0) //And now LockMode Does this even better.
					{
						//and ensure A was unlocked
						if ((MS_TIMER - g_ulClampAUnlock) < g_ConfigData.uiAUnlockDelay)
						{
							//not done yet
							break;
						}
					}
				}

				//Now that we are clamped, lock A again.
				if (g_cALock==ALOCK_OFF)
				{
					g_cALock=ALOCK_ON;
					g_ulClampALock=MS_TIMER;
					c=1;
					break;
				}

				if (g_cALockMode==0)
				{
					if ((MS_TIMER - g_ulClampALock) < g_ConfigData.uiALockDelay)
					{
						//not done yet
						break;
					}
				}

				//Reassert settings achieved
				g_cALock=ALOCK_ON;
				g_cLegsLock=LEGSLOCK_ON;
				g_cLegsDown=VALVE_ON;
				g_cLegsUp=VALVE_OFF;
				g_cClampExtend=VALVE_ON;
				g_cClampRetract=VALVE_OFF;
				g_uiClampPressureLBS=g_uiClampPressure;
				g_cAirBlastX=0;
				g_cAirBlastY=0;
				c=1;

				//now done Clamp
				g_cClampState=CLAMP_CLAMP;
				break;
			}
			if (c!=0)
			{
				digOut(DIGOUT_CHNUM_ALOCK, g_cALock );
				digOut(DIGOUT_CHNUM_LEGSLOCK, g_cLegsLock );
				digOut(DIGOUT_CHNUM_LEGSDOWN, g_cLegsDown );
				digOut(DIGOUT_CHNUM_LEGSUP, g_cLegsUp );
				digOut(DIGOUT_CHNUM_CLAMPEXTEND, g_cClampExtend );
				digOut(DIGOUT_CHNUM_CLAMPRETRACT, g_cClampRetract );
#ifdef OUTPUT_HDCLAMP_STATES
				logf( "HDCLAMP LEGSLOCK=%d, LEGSUP=%d, LEGSDOWN=%d, CLAMPEXTEND=%d, CLAMPRETRACT=%d\r\n", g_cLegsLock, g_cLegsUp, g_cLegsDown, g_cClampExtend, g_cClampRetract );
				logf( "HDCLAMP PRESSURELBS=%d\r\n", g_uiClampPressureLBS );
#endif
			}

			if (g_uiClampPressureLBSLastSet!=g_uiClampPressureLBS)
			{

				SetClampPressureLBS();
			}
			if (g_cClampState==g_cClampGoal)
			{
				//goal has been reached
				//send alert
				SmartToolMsgChar(p_STPSession, STP_ALERT, MINIFT_OID_CLAMP, g_cClampState);
			}
		}
		if (g_cClampState!=CLAMP_UNCLAMP)
		{
			g_cClear=0;
		}

	}
#endif
#endif

#ifdef GENCIRCMFTX
	p_c = (char *) 0;
//GENCIRCMFT2 Clamp Machine
//FIXME0 want, but not working
//	if (g_cDigInAirPressure == 1)
//	{
//		//no pressure... can't continue
//    	g_cClampState=CLAMP_UNKNOWN;
//	}
//	else

//FIXMEGENCIRCMFT2 test this to ensure it's not a problem
	byte c = 0;
	if (g_cDrillState != DRILLSTATE_IDLE) //only change clamping if drill is idle
	{
		if (g_cDrillState != DRILLSTATE_ESTOP) //dont print this warning out if it's just ESTOP...
		{
			if (g_cClampState != g_cClampGoal)
			{
				logf("cci\r\n"); //Clamp Chanage Impossible until Drill is IDLE");
			}
		}
	}
	else if (g_cClampState != g_cClampGoal)
	{
		g_cClampState = CLAMP_TRANSITION;
		c = 0;
		switch (g_cClampGoal)
		{
		case CLAMP_UNKNOWN:
			//This is not used as a goal, but only as the original state
			g_cClampGoal = CLAMP_UNCLAMP;
			c = 1;
			break;
		case CLAMP_LOOSE:
			#ifdef DIGOUT_CHNUM_DRILLUP
			//need to become loose.
			//start like unclamp
			p_c="l0";
			if (g_cColletClamp == COLLET_RELEASE || g_cDrillUp == 0)
			{
				p_c="l1";
				g_ulClampPressureHigh = MS_TIMER;
				g_cColletClamp = COLLET_RELEASE; //loose
#ifdef DIGOUT_CHNUM_DRILLUP
				g_cDrillUp = 1;
#endif
				g_cDrillDown = DRILL_DOWN_UP;
				c=1;
				break;
			}
			p_c="l2";
			//This is tricky: we are letting just enough air go into the up before
			//we turn off up again... This creates a cushion in case it drops on loose
			if ((MS_TIMER - g_ulClampPressureHigh) < 100)//30 ms for loose wait...
			{
				//not done collet clamp unclamp
				break;
			}
			p_c="l2";
			//then also turn off drill up
			g_cDrillUp = 0;
			g_cDrillDown = DRILL_DOWN_UP;
			if (g_cDigInDrillUp != DRILL_UP)//drill up is not ACTIVATED
			{
				g_cColletClamp = COLLET_RELEASE; //loose
			}
			else
			{
				g_cColletClamp = COLLET_RELEASE; //loose
			}
			c=1;
			//Now Done Loose
			g_cClampState=CLAMP_LOOSE;
			p_c="ld";
			break;
#endif
//FIXME0 what should the rest of the program do if it is supposed to be clampped and it sees the sensor???
//FIXME0 What if it's moving and suppoed to be up and the sensor drops?
//				I wanted to continual check..... so it would fail to be unclamped if
// the sensor goes, and this would kill motion.....

#ifndef DIGOUT_CHNUM_DRILLUP
			//this system supports one piston only.. make loose like unclamp, but end with no collect clamp (see end below)
			//handle with same code path
			//FALL THROUGH
#endif
		case CLAMP_UNCLAMP:
			//need to become unclamped
			p_c = "u0";
			if (g_cColletClamp == COLLET_LOCK )
			{
				g_ulClampPressureHigh = MS_TIMER;
				g_cColletClamp = COLLET_RELEASE;
				c = 1;
			}
			p_c = "u1";
			if ((MS_TIMER - g_ulClampPressureHigh)
					< g_ConfigData.uiAUnlockDelay)
			{
				//not done collet clamp unclamp
				break;
			}
			p_c = "u2";
#ifdef DIGOUT_CHNUM_DRILLUP
			if (g_cDrillUp == 0)
#else
			if (g_cDrillDown != DRILL_DOWN_UP)
			#endif
			{
				g_ulClampPressureLow = MS_TIMER;
#ifdef DIGOUT_CHNUM_DRILLUP
				g_cDrillUp = 1;
#endif
				g_cDrillDown = DRILL_DOWN_UP;
				c = 1;
				p_c = "u3";
			}
			//This delay is not really needed, but we don't want a false alarm on the up sensor.
			//I set this delay to 100.
			if ((MS_TIMER - g_ulClampPressureLow)
					< g_ConfigData.uiLowPressureDownDelay)
			{
				//not supposed to be done yet
				break;
			}
			//REMOVED//must not see bottom sensor to be unclamped
			//REMOVEDif (g_cDigInDrillDown == ACTIVATED) //drill down is ACTIVATED
			//REMOVED{
			//REMOVED	//can't be considered unclamped yet if still see this
			p_c = "u4b";      //REMOVED
							  //REMOVED	break;
							  //REMOVED}
							  //must see sensor to be unclamped
			if (g_cDigInDrillUp != DRILL_UP) //drill up is not ACTIVATED
			{
				//can't be considered unclamped yet
				p_c = "u5b";
				//should be done soon... could do timeout
				break;
			}
			g_ulClampPressureZero = MS_TIMER;
			//Can be considered unclamped
			g_cColletClamp = COLLET_RELEASE; //collect clamp is released.
#ifdef DIGOUT_CHNUM_DRILLUP
					g_cDrillUp = 1;
#endif
			g_cDrillDown = DRILL_DOWN_UP;
			c = 1;
			//Now Done Unclamp
			g_cClampState = CLAMP_UNCLAMP;
			p_c = "ud";
#ifndef DIGOUT_CHNUM_DRILLUP
			if (g_cClampGoal == CLAMP_LOOSE)
			{
				//trying to be loose instead...
				//just turn off collect clamp now
				g_cColletClamp = COLLET_LOCK; //collect clamp is NOT released, and button is accessible
				//Now Done Loose
				g_cClampState = CLAMP_LOOSE;
				p_c = "ld";
			}
#endif
//TRY THIS FOR ALL!!!!!!!
//
			g_cColletClamp = COLLET_LOCK; //collect clamp is NOT released, and button is accessible

			break;
		case CLAMP_CLAMP:
			//need to become clamped
			if (g_cDrillDown == DRILL_DOWN_UP)
			{
				//First Make sure collet is unclamped
				if (g_cColletClamp == COLLET_LOCK)
				{
					g_ulClampPressureHigh = MS_TIMER;
					g_cColletClamp = COLLET_RELEASE;
					c = 1;
				}
				if ((MS_TIMER - g_ulClampPressureHigh)
						< g_ConfigData.uiAUnlockDelay)
				{
					//not done collet clamp unclamp
					p_c = "c1";
					break;
				}
				//Collet is unclamped, so start down
				g_ulClampPressureZero = MS_TIMER;
#ifdef DIGOUT_CHNUM_DRILLUP
				g_cDrillUp = 0;
#endif
				g_cDrillDown = DRILL_DOWN;
				c = 1;
			}
			//sensor must be off to be down
			if (g_cDigInDrillUp == DRILL_UP)
			{
				//must not really be down yet.
				//reset timer, so that timer can be time from leaving sensor to collect lock
				g_ulClampPressureZero = MS_TIMER;
				p_c = "c2";
				break;
			}
			if ((MS_TIMER - g_ulClampPressureZero)
					< g_ConfigData.uiLowPressureDelay)
			{
				//not done yet down yet
				p_c = "c3";
				break;
			}
			//REMOVED//must see sensor to be clamped
			//REMOVEDif (g_cDigInDrillDown != ACTIVATED)
			//REMOVED{
			//REMOVED	//can't be considered clamped yet
			//REMOVED	p_c="c4";
			//REMOVED	break;
			//REMOVED}
			//now down by timer and sensor, so do clamp
			if (g_cColletClamp == COLLET_RELEASE )
			{
				g_ulClampPressureLow = MS_TIMER;
				g_cColletClamp = COLLET_LOCK;
				c = 1;
			}
			if ((MS_TIMER - g_ulClampPressureLow) < g_ConfigData.uiALockDelay)
			{
				//not done collet clamp unclamp
				p_c = "c5";
				break;
			}
			g_ulClampPressureHigh = MS_TIMER;
			//Can be considered unclamped
			g_cColletClamp = COLLET_LOCK; //clamp not released
#ifdef DIGOUT_CHNUM_DRILLUP
					g_cDrillUp = 0;
#endif
			g_cDrillDown = DRILL_DOWN;
			c = 1;
			//Now done Clamp
			g_cClampState = CLAMP_CLAMP;
			p_c = "cd";
			break;
		}
		if (c != 0)
		{
			digOut(DO_COLLET_BUTTON, g_cColletClamp);
#ifdef DIGOUT_CHNUM_DRILLUP
			//this system only has drill down doing both directions
			digOut(DIGOUT_CHNUM_DRILLUP, g_cDrillUp );
#endif
			digOut(DO_DRILL_DOWN, g_cDrillDown);
#ifdef OUTPUT_HDCLAMP_STATES
#ifdef DIGOUT_CHNUM_DRILLUP
			logf( "CLAMP collet=%d, UP=%d, DOWN=%d\r\n", g_cColletClamp, g_cDrillUp, g_cDrillDown );
#else
			logf("CLAMP collet=%d, DOWN=%d\r\n", g_cColletClamp, g_cDrillDown);
#endif
#endif
		}
	}
#ifndef DIGOUT_CHNUM_DRILLUP
//There is really no difference between unclamp and loose because there is only one value for UP or LOOSE
//If the up sensor is not seen, drop from CLAMP_UNCLAMP back down to CLAMP_LOOSE to protect the system from dragging the drill.
	else if (g_cClampState == CLAMP_UNCLAMP) //AND GOAL IS UNCLAMP
	{
		//Goal Is Unclamped, and the State is Unclamped...
		//If the up sensor is not seen then make the state LOOSE
		if (g_cDigInDrillUp != DRILL_UP) //drill up is not ACTIVATED
		{
			g_cClampState = CLAMP_LOOSE;
		}
	}
#endif
//Always Alert Clamp
	if (g_cClampState != g_cClampStateSent)
	{
		//send alert of changed state
		SmartToolMsgChar(0, STP_ALERT, MINIFT_OID_CLAMP, g_cClampState);
		g_cClampStateSent = g_cClampState;
		logf("Alrt Clamp=%d\r\n", g_cClampState);
	}

	if (g_cClampState != CLAMP_UNCLAMP)
	{
		g_cClear = 0;
	}
	if (p_c != 0)
	{
		if (g_szClampMessage != p_c)
		{
			g_szClampMessage = p_c;
			logf("%s\r\n", g_szClampMessage);
		}
	}

	if (g_ConfigData.cUseCutterDetect != 0)
	{
		int counts = anaIn(AIN_DRILL_NOISE);
		//FIXMENEXT drillnoise
//FIXME PORT FATAL use new drill noise sensor
#if 0
		//just use this for lone
		g_ulCutterAirTotal = g_ulCutterAirTotal - g_uiCutterAir
		+ anaIn(ADC_CHANNEL_DRILL_AIR_FLOW);
		g_uiCutterAir = g_ulCutterAirTotal / 64;

		if (g_uiCutterAir <= 1802)
		{
			if (g_cCutterDetected != CUTTER_DETECTED)
			{
				logf("CD!!!!!!!!!!!!!!\r\n");
				g_cCutterDetected = CUTTER_DETECTED;
			}
			logf("l %u\r\n", g_uiCutterAir);
		}
		else if (g_uiCutterAir > 1807)
		{
			if (g_cCutterDetected != CUTTER_NOT_DETECTED)
			{
				g_cCutterDetected = CUTTER_NOT_DETECTED;
				logf("CND-------------\r\n");
			}
			logf("h %u\r\n", g_uiCutterAir);
		}
		else
		{
			logf("m %u\r\n", g_uiCutterAir);
		}
#endif
	}

	/*
	 //This is the only place using ADC_CHANNEL_DRILL_AIR_FLOW, so read here
	 g_uiCutterAir = (g_uiCutterAir*7+anaIn(ADC_CHANNEL_DRILL_AIR_FLOW))/8;
	 logf("v %u \r\n",g_uiCutterAir);
	 //  <1800 is on
	 if (g_cCutterDetected == CUTTER_NOT_DETECTED)
	 {
	 if (g_uiCutterAir < 1800)
	 {
	 logf("v %u \r\n",g_uiCutterAir);
	 g_cCutterDetected = CUTTER_DETECTED;
	 logf("cd\r\n");
	 }
	 }
	 else
	 {
	 if (g_uiCutterAir > 1960) //use 100 points for hysteresis
	 {
	 logf("v %u \r\n",g_uiCutterAir);
	 g_cCutterDetected = CUTTER_NOT_DETECTED;
	 logf("cnd\r\n");
	 }
	 }
	 */

//GENCIRCMFT2 drill state machine
//cEntryState=g_cDrillState; //NOT DOING THIS... FIXME0 reconsider this and other enterystate style switch
	c = g_cDrillButton;
	switch (g_cDrillState)
	{
	case DRILLSTATE_INIT:
		cdm = '1';
		//Button should be off
		g_cDrillButton = START_DRILL_OFF;
//REMOVED // g_cCutterDetected not reliable when up..."
//REMOVED //			if (g_cCutterDetected == 1) //FIXME
//REMOVED //			{
//REMOVED //				//can't go to IDLE
//REMOVED //				break;
//REMOVED //			}
		g_cDrillState = DRILLSTATE_IDLE;
		break;
	case DRILLSTATE_IDLE:
		cdm = 'I';
		//Button should be off
		g_cDrillButton = START_DRILL_OFF;
		if (g_cDrillStateGoal == DRILLSTATE_DRILL)
		{
			if (g_cClampState != CLAMP_CLAMP)
			{
				break;
			}
			g_cDrillState = DRILLSTATE_DRILL;
			cdm = 'D';
			g_ulDrillStateStartTime = MS_TIMER;
			g_uiMessageCode = 0;
			g_cDrillButton = START_DRILL_ON;
			uiop = OP_DRILL_STARTED;
			g_uiPositionOpsCache |= uiop; //since this is fir current position
			AddOpHistory(g_PosnMode.iCurPosnIndex, uiop);
			logf("*aoh%u\r\n", uiop);
			c = 2; //force setting of output
			g_uiCutterAir = 1800;
		}
		break;
	case DRILLSTATE_DRILL:
		//This code has been written with the assumption that once the drill button is pressed, it will proceed to the limit and then retract
//and that there is nothing that can stop that cycle.   If the button is released early though, it might not have triggered.
//Anytime after the button delay time, if the drill is clear, assume it either did not fire, or has returned.
//The delay must be long enough to cover the time it takes to start the drill and feed to the point it has extended through the sensor.
		cdm = 'D';
		if (g_cClampState != CLAMP_CLAMP)
		{
			//clear button variable
			g_cDrillButton = START_DRILL_ON;
			c = 2; //force setting of output
		}

		//Calculate Elapsed Drill Time In One place for multiple uses below
		ui = (unsigned int) (MS_TIMER - g_ulDrillStateStartTime);

		//See if it has been held long enough.
		//This delay should be long enough that the bit would have extended through the sensor.
		if (ui < g_ConfigData.uiDrillButtonDelay)
		{
			//not done holding button
			break;
		}
		if (g_cDrillButton == START_DRILL_ON)
		{
			g_cDrillButton = START_DRILL_OFF;
			c = 2;			//force setting of output
		}
		if (g_ConfigData.cUseCutterDetect != 0)
		{
			if (ui < g_ConfigData.uiPressureDelay)
			{
				//Time is shorter than the shortest acceptable drill time.
				//Air Must Should be going
				if (g_cCutterDetected == CUTTER_NOT_DETECTED) //JLIME... how the cutter detected system was used in the past.
				{
					//Air is not going
					//issue warning, stop cycle, and do not unclamp
					if (g_uiMessageCode == 0)
					{
						g_uiMessageCode = MINIFTMC_DRILL_AIR_OFF_EARLY;
						SmartToolMsgMiniFtMessageCode(0, COMMON_OID_NULLOID, MINIFTMC_DRILL_AIR_OFF_EARLY);
					}
					g_cStartProcess = 0; //Turns off the cycle and makes ACTION_WATCH do nothing except wait for drill to be done.
				}
				break;
			}
			//Anytime the air is done, the cycle can be done
			if (g_cCutterDetected == CUTTER_DETECTED)
			{
				//Air is flowing
				if (ui < g_ConfigData.uiDrillCycleDelay)
				{
					//still waiting for cycle time
					break;
				}
				//Cycle is running long
				//Issue Warning, take it out of continuous, but do not stop this cycle... let it terminate and unclamp
				if (g_uiMessageCode != MINIFTMC_DRILL_AIR_ON_LONG)
				{
					g_uiMessageCode = MINIFTMC_DRILL_AIR_ON_LONG;
					SmartToolMsgMiniFtMessageCode(0, COMMON_OID_NULLOID, MINIFTMC_DRILL_AIR_ON_LONG);
				}
//FIXME PORTHIGH NEED TEST BECAUSE OF DOUBLE ASSIGNMENT IN PAST and also need test to see that error is gone....(warning error in eclipse was happening)

				g_ConfigData.cProcessContinueMode = (byte) PROCESS_SINGLE;
				//no need to alert this out...
				//Pendant Will Still say CONTINUOUS, and when they hot Start again it will send CONTINOUS again first.
				break;
			}
			//Air is done
		}
		else
		{
			//timer only
			//This delay should be long enough for a cycle.
			if (ui < g_ConfigData.uiDrillCycleDelay)
			{
				//not done cycle
				break;
			}
		}
		//It's Done
		uiop = OP_DRILL_SUCCESS;
		g_uiPositionOpsCache |= uiop; //since this is fir current position
		AddOpHistory(g_PosnMode.iCurPosnIndex, uiop);
		logf("*aoh%u\r\n", uiop);

//FIXMENOWzxcvzxcv>>>>
		if (g_ConfigData.uiPressure == 0) //FIXME Minor  Add OID for this, or get rid of feature... right now I'm not sure we will use this
		{
			g_cDrillState = DRILLSTATE_WAIT_CONFIRM;
			cdm = 'W';
		}
		else
		{
			g_cDrillState = DRILLSTATE_DONE;
			cdm = 'X';
		}
		break;
	case DRILLSTATE_ESTOP:
		cdm = 'E';
		break;
	case DRILLSTATE_WAIT_CONFIRM:
		cdm = 'W';
		//Button should be off
		g_cDrillButton = START_DRILL_OFF;
		//stay until OID message moves to done...
		break;
	case DRILLSTATE_DONE:
		cdm = 'X';
		if (g_cModeState != MODE_POSN || g_cPosnModeState != POSNMODE_ACTION)
		{
			//can go from done to IDLE
			g_cDrillState = DRILLSTATE_IDLE;
			g_cDrillStateGoal = DRILLSTATE_IDLE; //clear this too
		}
		//Button should be off
		g_cDrillButton = START_DRILL_OFF;
		break;
	default:
		cdm = '?';
		break;
	}
	if (g_cDrillButton != c)
	{
		digOut(DO_START_DRILL, g_cDrillButton);
	}
	if (g_cDrillState != g_cDrillStatePrev)
	{
		logf("DS_%c\r\n", cdm);
		//send out drill state
		//FIXME0 should I have drill state.....
		//		should I have this work pendant....
		//		Or genernal action state??????
		SmartToolMsgChar(0, STP_ALERT, MINIFT_OID_DRILL_STATE, g_cDrillState);
	}

//capture the correct previous state
	g_cDrillStatePrev = g_cDrillState;

	if (g_cDrillState != DRILLSTATE_IDLE) //Also do not let it consider clear if drilling.
	{
		g_cClear = 0;
	}

//Let CIRCMFT operate as EENONE
	if (g_cEEOption == EENONE)
	{
		g_cClear = 1;
		g_cClampGoal = CLAMP_UNCLAMP;
		g_cClampState = CLAMP_UNCLAMP;
	}
#endif

	if (g_cJogGoalY != JOGSTOP || g_cJogGoalX != JOGSTOP) // Check Reason that Jog Might have to be stopped
	{

		//Obstruction System - Stop Jog
		if (g_cObstructionCode != 0)
		{
#ifdef Y_LIMIT_SENSORS
			if (g_cDigInYPosLimit==0)
			{
				if (g_cJogGoalY == JOGPOS)
				{	goto handle_stop_jog;}
			}
			if (g_cDigInYNegLimit==0)
			{
				if (g_cJogGoalY == JOGNEG)
				{	goto handle_stop_jog;}
			}
#endif
#ifdef OBSTRUCTION_SYSTEM_XP1
			if (g_cDigInObstructionXP1 == OBSTRUCTION)
			{
				if (g_cJogGoalX == JOGPOS)
				{
					goto handle_stop_jog;
				}
			}
#endif
#ifdef OBSTRUCTION_SYSTEM_XN1
			if (g_cDigInObstructionXN1 == OBSTRUCTION)
			{
				if (g_cJogGoalX == JOGNEG)
				{
					goto handle_stop_jog;
				}
			}
#endif
#ifdef OBSTRUCTION_SYSTEM_MOS
			if (g_cDigInObstructionMOS == MO_OBSTRUCTION)
			{
				goto handle_stop_jog;
			}
#endif
#ifdef OBSTRUCTION_SYSTEM_MO_ANA
			if (g_cMOFlags > 0)
			{
				goto handle_stop_jog;
			}
#endif
		}

		if (g_cClear == 0)
		{
			logf("j:nc\r\n"); //special debug
			if (g_cTestOpt == 5)
			{
				//allow jog anyway
			}
			else
			{
				//do not allow jog
				goto handle_stop_jog_no_break;
				//must stop jog goal
			}
		}
		if (g_cAction > ACTION_READY)
		{
			logf("j:oa\r\n");
			goto handle_stop_jog_no_break;
			//must stop jog goal
		}
#ifdef JOG_ENABLE_TIME
		if (g_ConfigData.cJogEnableTimeout > 0)
		{
			uint32 uiElapsed = MS_TIMER - g_uiJogEnableTime;
			if (uiElapsed > g_ConfigData.cJogEnableTimeout)
			{
				logf("j:et\r\n");
				//Jog must be enabled at a rate of more than 100 ms,
				//If jog is not continually enabled, then any jog move will be terminated.
				goto handle_stop_jog_no_break;
				//must stop jog goal
			}
		}
#endif
#ifdef CLAMP_SYSTEM_NAC_STP
		if (g_cNACClear==0)
		{
			if (g_cTestOpt == 5)
			{
				//allow jog anyway
			}
			else
			{
				logf("j:nac\r\n"); //special debug
				goto handle_stop_jog_no_break;//must stop jog goal
			}
		}
#endif
		if (0)
		{
handle_stop_jog:
			BrakeOn();
handle_stop_jog_no_break:
			g_cJogGoalX = JOGSTOP;
			g_cJogGoalY = JOGSTOP;
		}

	} //END Check Reasons that Jog Might have to be stopped

	if (g_cJogX != g_cJogGoalX)
	{
		//Check For Jogs
		if (g_cJogGoalX != JOGSTOP)
		{
			//RunJogX
			RunJogX(g_cJogGoalX, g_fJogGoalX);
			if (g_cJogGoalX != 0)
			{
				g_cJogX = g_cJogGoalX;
				g_fJogX = g_fJogGoalX;
				g_uiJogStopX = MS_TIMER - 50;
			}
			//#ifdef BEEPSYSTEM
			//if (g_cBeepMode!=BEEPJOG) { BeepJog(); }
			//#endif
		}
		else //IT's a STOP
		{
			//X is waiting to see acknowledgements of stop
			if ((MS_TIMER - g_uiJogStopX) >= 30)
			{
logf("Stop X\r\n");
				StopJogX(); //must contiune until confirmed stop
				g_uiJogStopX = MS_TIMER;
			}
		}
	}
	if (g_cJogY != g_cJogGoalY)
	{
		if (g_cJogGoalY != JOGSTOP)
		{
			//RunJogY
			RunJogY(g_cJogGoalY, g_fJogGoalY);
			if (g_cJogGoalY != 0)
			{
				g_cJogY = g_cJogGoalY;
				g_fJogY = g_fJogGoalY;
				g_uiJogStopY = MS_TIMER - 50;
			}
			//I don't think this is needed FIXME minor  test on machine that has beep
			//#ifdef BEEPSYSTEM
			//if (g_cBeepMode!=BEEPJOG) { BeepJog(); }
			//#endif
		}
		else //IT's a STOP
		{
			//Y is waiting to see acknowledgements of stop
			if ((MS_TIMER - g_uiJogStopY) >= 30)
			{
logf("Stop Y\r\n");
				StopJogY(); //must contiune until confirmed stop
				g_uiJogStopY = MS_TIMER;
			}
		}
	}
#ifdef BEEPSYSTEM
	if (g_cJogX == JOGSTOP && g_cJogY == JOGSTOP)
	{
		//not Jogging
		if (g_cBeepMode == BEEPJOG)
		{
			BeepOff()
			;
		} //will cause mode to be BEEPOFF, which will let other permanent modal modes activate....
	}
	else
	{
		if (g_cBeepMode != BEEPJOG && g_cBeepMode < BEEPSIGNALS)
		{
			BeepJog()
			;
		}
	}
	if (g_cFloatStatus != FLOATSTAT_FLOAT)
	{
		if (g_cBeepMode == BEEPFLOAT)
		{
			BeepOff()
			;
		}
	}
	else
	{
		if (g_cBeepMode != BEEPFLOAT && g_cBeepMode < BEEPSIGNALS)
		{
			BeepFloat()
			;
		}
	}
#endif

//MOVE ERROR and MOVE STOP RESET CODE
	if (g_cMoveDone == MOVEDONE_ERROR || g_cMoveDone == MOVEDONE_STOP)
	{
		//If nothing above caught this flag, then now catch it:
		g_PosnMode.cOnCurPosn = 0;	//moving actions always mean not on position
		LEDOff()
		;
		g_cMoveDone = MOVEDONE_TRUE;
	}

//Periodic STP Output : Mode based
// Because output depends on mode and changes during modes, this is the current location for this code
#ifdef CENTERVISION
	if (g_cCenterVisionResult == CENTERVISION_OFF)
	{
		g_uiPositionUpdateThrottle = POSITION_UPDATE_THROTTLE_DEFAULT;
		g_uiPositionSendThrottle = POSITION_SEND_THROTTLE_DEFAULT;
	}
#endif

	if ((MS_TIMER - g_uiPositionUpdateTime) >= g_uiPositionUpdateThrottle)
	{
		//time for position update
		g_uiPositionUpdateTime = MS_TIMER;
		MCGetPosition();
	}

	if (g_PosnMode.cFreshCurPosnSend == 1)
	{
		if ((MS_TIMER - g_uiPositionSendTime) >= g_uiPositionSendThrottle)
		{
			if (fabs(g_PosnMode.fLastSentPosnX - g_PosnMode.fLastKnownPosnX)
					> g_fPosnLSCD
					|| fabs(
							g_PosnMode.fLastSentPosnY
									- g_PosnMode.fLastKnownPosnY)
							> g_fPosnLSCD)
			{
				g_PosnMode.fLastSentPosnX = g_PosnMode.fLastKnownPosnX;
				g_PosnMode.fLastSentPosnY = g_PosnMode.fLastKnownPosnY;

				//FIXME0000000 currently only sending nearest after everything is probed and probe is complete
				//I could send nearest as soon as partial information was ready, but it's much more complicated.
				if (g_cProbeCalculated == 1)
				{
					if (g_PosnMode.iLastSentNearestPosn
							!= FindNearestPosition()) //FindNearest will cache for this X and Y
					{
						SendNearXY(0, STP_ALERT); //only uses cached value
						g_PosnMode.iLastSentNearestPosn =
								g_PosnMode.iNearestPosn; //even faster chache of last sent....
					}
				}

				//Send XY Last : Pendant will only update the screen for the above information
				//when it gets the XY coordinates.
				SendCurXY(0, STP_ALERT);
			}
			g_uiPositionSendTime = MS_TIMER;
			g_PosnMode.cFreshCurPosnSend = 0;
		}
	}

	if (g_cSendPPDataSessions != 0)
	{
		//Continue SendPartProgramData in background
		ContinueSendPartProgramData();
	}

	if (g_cSendXYDataSessions != 0)
	{
		//Continue Send XYData in background
		ContinueSendXYData();
	}

//Continue Send Op History
	if (g_cSendOpHistory != 0)
	{
		ContinueSendOpHistory();
	}

	if (g_cLastAutoMove != g_cAutoMove)
	{
		SmartToolMsgChar(0, STP_ALERT, MINIFT_OID_AUTOMOVE, g_cAutoMove);			//echo for sync
		g_cLastAutoMove = g_cAutoMove;
	}

	if (g_cSentStationGoal != g_cStationGoal)
	{
		//Since there is OID for goal, use MINIFT_OID_STATION over 100
		SmartToolMsgChar(0, STP_ALERT, MINIFT_OID_STATION, 100 + g_cStation);
		g_cSentStationGoal = g_cStationGoal;
	}
	if (g_cSentStation != g_cStation)
	{
		SmartToolMsgChar(0, STP_ALERT, MINIFT_OID_STATION, g_cStation);
		g_cSentStation = g_cStation;
	}

} //ModeStateMachine

#ifdef USE_RFID_OMRON
#error "OMRON CODE WAS NOT YET ADJUSTED for new system"
void RFIDRun()
{
//This subsystem could be extended to support more than just RFID, but it currently does not.

	if ( g_cReadRFID != RFID_READ_OFF )
	{
		if ( g_cReadRFID == RFID_READ_STOP )
		{
			g_cReadRFID = RFID_READ_OFF;
			g_cTagReadState = RFID_TAG_CLEAR;
			goto skip_rfid_read;
		}
		if ( g_cReadRFID == RFID_READ_SEEK )
		{
			if (g_cHomedX != HOME_RUNNING)
			{
				g_cReadRFID = RFID_READ_OFF;
				g_cTagReadState = RFID_TAG_CLEAR;
				logf("r sk bt hmnot\r\n");
				goto skip_rfid_read;
			}
		}
//FIXME alter RFID rate and any other changes

		//RFID READING
		rfidWatcher();
		//Always check to see what results comeback.
		rfidCheckResponses();

		//FIXME Move this into Function.???
		if (g_cLastTagReadState != g_cTagReadState)
		{
			switch(g_cTagReadState)
			{
				default:
				case RFID_TAG_PRESENCE_UNKNOWN:
				logf("rfid: presence unknown.\r\n");
				break;
				case RFID_TAG_IS_PRESENT:
				logf("rfid: e=%2x d=\"%s\"\r\n", g_RfidStateAndData.iOmronResponseEndCode, g_RfidStateAndData.szTagData);
				break;
				case RFID_TAG_NOT_PRESENT:
				logf("rfid: not present. e=%2x\r\n", g_RfidStateAndData.iOmronResponseEndCode);
				break;
				case RFID_TAG_CLEAR:
				logf("rfid: clear.\r\n");
				break;
			}
			if (g_cReadRFID!=RFID_READ_SEEK)
			{
				//then use OID to alert this now
				g_RFIDData.cstate = RFID_INIT;
				g_RFIDData.ccontext = RFID_CONTEXT_READ;
				g_RFIDData.cseekstate = RFID_SEEK_NULL;
				g_RFIDData.fposition = 0;
				g_RFIDData.fsstart = 0;
				g_RFIDData.fpstart = 0;
				g_RFIDData.fpend = 0;
				g_RFIDData.fnstart = 0;
				g_RFIDData.fnend = 0;
				g_RFIDData.fhs1 = 0;
				g_RFIDData.fhs2 = 0;
				g_RFIDData.fhsf = 0;
				SendRFIDData(0, STP_ALERT);
			}
			if ( g_cReadRFID == RFID_READ_NOW )
			{
				if (g_cTagReadState!=RFID_TAG_CLEAR)
				{
					//got an answer
					g_cReadRFID = RFID_READ_OFF;
					g_cTagReadState = RFID_TAG_CLEAR;
					goto skip_rfid_read;
				}
			}
		}
	}
	skip_rfid_read:
	if ( g_cReadRFID == RFID_READ_OFF )
	{
		//even when off:
		//Always check to see what results comeback.
		rfidCheckResponses();
	}

//continue omron code
#ifdef HOMESYSTEM_X_RFID
	while (g_cHomedX == HOME_RUNNING)
	{
		//Advance X RFID Homing
//FIXME SEVERE  How to start or restart....
//How and when to clear the stored data....
//store home across reads of continuous????
//
//exit state and how to manange rfid
		if (g_ConfigData.RFIDConfig.cenabled==0)
		{
			g_cHomedX = HOME_FAILURE;
			break;
		}
		if (g_RFIDData.cseekstate < RFID_SEEK_INIT)
		{
			logf("RFpi\r\n");
			if (g_cMoveDone == MOVEDONE_FALSE)
			{
				//Stop Move
				if (g_cMCRunningMsgGoal != PositionStop)
				{	StopPositionAdvanced();}
				break;
			}
			g_cMoveDone = MOVEDONE_TRUE;
			GetPositionCounts();
			g_RFIDData.ccontext = RFID_CONTEXT_SEEK;
			g_RFIDData.cseekstate = RFID_SEEK_INIT;
		}
		if (g_RFIDData.cseekstate == RFID_SEEK_INIT)
		{
			if (g_PosnMode.cFreshPosn != X_AND_Y)
			{
				//did not get a fresh position yet
				break;
			}
			logf("RFi\r\n");
			//Start RFID Home
			g_fStartX = g_PosnMode.fLastKnownPosnX;
			g_fStartY = g_PosnMode.fLastKnownPosnY;
#ifdef OUTPUT_RFID_HOME
			logf("Start %f %f\r\n",g_fStartX,g_fStartY);
#endif
			g_RFIDData.ccontext = RFID_CONTEXT_SEEK;
			g_RFIDData.cseekstate = RFID_SEEK_INIT;
			g_RFIDData.fposition = 0;
			g_RFIDData.fsstart = 0;
			g_RFIDData.fpstart = 0;
			g_RFIDData.fpend = 0;
			g_RFIDData.fnstart = 0;
			g_RFIDData.fnend = 0;
			g_RFIDData.fhs1 = 0;
			g_RFIDData.fhs2 = 0;
			g_RFIDData.fhsf = 0;
			g_cTagP = TAG_NONE;
			g_cTagN = TAG_NONE;
			SendRFIDData(0, STP_ALERT);

			g_RfidStateAndData.cFullReading = 0;
			if (g_ConfigData.RFIDConfig.cmethod==FIRST_DETECTION)
			{
				g_RfidStateAndData.cFullReading = 1;
			}
//>>>>Activate continuous reading at the right frequency

			g_ulPositionUpdateThrottle=POSITION_UPDATE_THROTTLE_RFID_SCAN;
			//DJWR FIXED 20120314-1
			//Should use action code here
			g_cAction = ACTION_HOME;
		}
		if (g_cObstructionCode!=0)
		{
			logf("o s rf\r\n");
			AlertObstructionCode(0);
			StopPosition();
			goto handle_as_rfid_seek_error;
		}
		if (g_RFIDData.cseekstate==RFID_SEEK_INIT)
		{
			//Init RFID Home ... Continued
			//Do not bother checking current move status
			//Setup Move
			//Clear tag state
			g_cTagState=TAG_NONE;
			goto setup_seek_move1;
		}
		if (g_cMoveDone != MOVEDONE_TRUE)
		{
			if (g_cTagState==TAG_NONE)
			{
				if (g_cTagReadState==RFID_TAG_IS_PRESENT)
				{
					//Start of tag...
//>>>>record
//This is only the most basic way to record... it does not suite the fine scan, but I need to get done....
					g_RFIDData.ultimestamp = MS_TIMER;
					g_RFIDData.ulrfidtimestamp = g_RFIDData.ultimestamp;
					if (g_RFIDData.cseekstate<RFID_SEEK_FINE1)
					{
						g_RFIDData.fposition = g_PosnMode.fLastKnownPosnX;
					}
					if (g_RfidStateAndData.cFullReading==1)	//FIXME minor... see rfid notes.
					{
						g_szTagDatalen = RFID_FMS00_DATA_SIZE - 1; //reads only the last part and then sets byte 47 to ' '
						memcpy(g_szTagData,g_RfidStateAndData.szTagData+2,g_szTagDatalen);//64 + 1
						g_RFIDData.uicrc16 = *(unsigned int *)(g_RfidStateAndData.szTagData+2 + g_szTagDatalen);
						g_szTagData[g_szTagDatalen++]=' ';
						g_szTagData[g_szTagDatalen]=0;
					}
					else
					{
						g_szTagDatalen = RFID_FMS00_DATA_SIZE;
						memset(g_szTagData,0,RFID_FMS00_DATA_SIZE);
					}
					g_RFIDData.uiendcode = (unsigned int)g_RfidStateAndData.iOmronResponseEndCode;

					g_fTagStart = g_PosnMode.fLastKnownPosnX;
					g_fTagEnd = g_fTagStart;
#ifdef OUTPUT_RFID_HOME
					logf("[%f,\r\n", g_fTagStart);
#endif

//FIXME SEVERE
					g_cTagState=TAG_STARTED;
					if (g_ConfigData.RFIDConfig.cmethod==FIRST_DETECTION)
					{
						//This is good enough to be "found" for FIRST_DETECTION
						g_cTagState=TAG_FOUND;
						//Stop Move
						if (g_cMCRunningMsgGoal != PositionStop)
						{	StopPositionAdvanced();}
					}
				}
			}
			else //if (g_cTagState==TAG_STARTED)  || TAG_FOUND
			{
//FIXME Dual Tag problem presents a number of issues..... MUST IGNORE THIS FOR NOW for sake of time....
				if (g_cTagReadState==RFID_TAG_IS_PRESENT)//FIXMENOW FIXME SEVERE  || TagNotSameTag)
				{
					//Tag Continues
					g_fTagEnd = g_PosnMode.fLastKnownPosnX;
				}
				if (g_cTagReadState==RFID_TAG_NOT_PRESENT) //FIXMENOW FIXME SEVERE  || TagNotSameTag)
				{
					//Tag ended... but don't do any stops unless move1 and move2... it could be a premature false alarm

					if (g_RFIDData.cseekstate <= RFID_SEEK_FINE2)
					{
						if (g_cMoveDir == JOGPOS)
						{
							if (g_fTagEnd + g_ConfigData.RFIDConfig.fseekPastBorder < g_PosnMode.fLastKnownPosnX)
							{
								if (g_cMCRunningMsgGoal != PositionStop)
								{	StopPositionAdvanced(); logf("se\r\n");}

							}
						}
						else
						{
							if (g_fTagEnd - g_ConfigData.RFIDConfig.fseekPastBorder > g_PosnMode.fLastKnownPosnX)
							{
								if (g_cMCRunningMsgGoal != PositionStop)
								{	StopPositionAdvanced(); logf("se\r\n");}
							}
						}
					}

//>>>>record

					//
					g_cTagState=TAG_FOUND;

					if (g_cLastTagReadState!=RFID_TAG_NOT_PRESENT)
					{
#ifdef OUTPUT_RFID_HOME
						logf(",%f]\r\n", g_fTagEnd);
#endif
					}
				}
			}
			if (g_cMoveDone == MOVEDONE_ERROR)
			{
//FIXME.......
				logf("mde\r\n");
				goto handle_as_rfid_seek_error;
			}
			if (g_cMoveDone == MOVEDONE_FALSE)
			{
				//continue waiting....
				break;
			}
			if (g_cMoveDone == MOVEDONE_STOP)
			{
				//Got Our Stop Confirmation... clear the state
				logf("mdsc\r\n");
				g_cMoveDone = MOVEDONE_TRUE;
			}
		}
#ifdef OUTPUT_RFID_HOME
		if (g_RFIDData.cseekstate!=RFID_SEEK_CENTER) //because this state loops too many times while checking for final read...
		{ //but for all other states, show md
			logf("md\r\n");
		}
#endif
		//Done Moving
//>>>>>>>>>>>>FIXME CASE WHERE IT IS ON DIFFERENT TAG WHEN DONE.... just fix with compares...

//FIXME position this better after we unerstand how RFID reading will be turned on...
		if (g_ConfigData.RFIDConfig.cmethod==FIRST_DETECTION)
		{
			if (g_cTagState==TAG_FOUND)
			{
//FIXME Found it
				logf("Found Tag\r\n");
				g_RFIDData.cstate = RFID_INIT;
				g_RFIDData.ccontext = RFID_CONTEXT_SEEK;
				g_RFIDData.cseekstate = RFID_SEEK_SUCCESS;
				g_RFIDData.fposition = (g_fTagStart + g_fTagEnd) / 2;
				g_RFIDData.ulseektime = 0;//FIXME SEVERE
				g_RFIDData.fsstart = 0;
				g_RFIDData.fpstart = 0;
				g_RFIDData.fpend = 0;
				g_RFIDData.fnstart = 0;
				g_RFIDData.fnend = 0;
				g_RFIDData.fhs1 = 0;
				g_RFIDData.fhs2 = 0;
				g_RFIDData.fhsf = 0;
				SendRFIDData(0, STP_ALERT);
				g_cHomedX = HOME_DONE;
				g_ulPositionUpdateThrottle=POSITION_UPDATE_THROTTLE_DEFAULT;
				break;
			}
		}
		if (g_RFIDData.cseekstate <= RFID_SEEK_FINE2)
		{
			if (g_cMoveDir == JOGPOS)
			{
				g_RFIDData.fpstart = g_fTagStart;
				g_RFIDData.fpend = g_fTagEnd;
				g_cTagP = g_cTagState;
			}
			else
			{
				g_RFIDData.fnstart = g_fTagStart;
				g_RFIDData.fnend = g_fTagEnd;
				g_cTagN = g_cTagState;
			}
#ifdef OUTPUT_RFID_HOME
			logf("tag [%f,%f] l=%f\r\n", g_fTagStart, g_fTagEnd, g_PosnMode.fLastKnownPosnX);
#endif
		}
		if (g_RFIDData.cseekstate==RFID_SEEK_MOVE1 || g_RFIDData.cseekstate==RFID_SEEK_MOVE2 || g_RFIDData.cseekstate==RFID_SEEK_MOVEOFF)
		{
			//Move1 or Move2 is complete
			if (g_cTagState==TAG_STARTED)
			{
				//Saw part of a tag, but it must complete after the range
				//For these moves, this is acceptable because it's the fine move that really measures.
//FIXME record??? or record abobe???
				logf("nte\r\n");
			}
			if (g_cTagState==TAG_FOUND)
			{
				//Great....
				if (g_ConfigData.RFIDConfig.cmethod==CENTER_FAST)
				{
//FIXME
				}
				if (g_cTagReadState!=RFID_TAG_NOT_PRESENT)
				{
					if (g_RFIDData.cseekstate==RFID_SEEK_MOVEOFF)
					{
						//Already did a move to get off tag and it failed
						//Setup Move
						g_RFIDData.cseekstate=RFID_SEEK_RETURN;
//FIXME
						//Setup return move
						goto setup_return_move;
					}
					//setup move OFF tag before fine move 1
					goto setup_seek_moveoff;
				}
				//Setup fine move 1
				g_RFIDData.cseekstate=RFID_SEEK_FINE1;
				//Clear tag state
				g_cTagState=TAG_NONE;
				goto setup_seek_fine_move;
			}
			logf("lnp %f\r\n", g_PosnMode.fLastKnownPosnX);
			if (g_RFIDData.cseekstate==RFID_SEEK_MOVE1)
			{
				//Setup Move 2
				//Clear tag state
				g_cTagState=TAG_NONE;
				goto setup_seek_move2;
			}
			//Setup Move
			g_RFIDData.cseekstate=RFID_SEEK_RETURN;
//FIXME
			//Setup return move
			goto setup_return_move;
		}
		if (g_RFIDData.cseekstate==RFID_SEEK_FINE1)
		{
			if (g_cTagState==TAG_FOUND)
			{
				//Good
				if (g_ConfigData.RFIDConfig.cmethod==CENTER_1PASS)
				{
//FIXME
				}
				//Setup fine move 2
				g_RFIDData.cseekstate=RFID_SEEK_FINE2;
				//Clear tag state
				g_cTagState=TAG_NONE;
				goto setup_seek_fine_move;
			}
			//Did not find both parts
			//Setup Return Move
			goto setup_return_move;
		}
		if (g_RFIDData.cseekstate==RFID_SEEK_FINE2)
		{
			if (g_cTagState==TAG_FOUND)
			{
				//Good
				//CENTER_2PASS is valid now
				//g_ConfigData.RFIDConfig.cmethod==CENTER_2PASS

				if (g_cTagN != TAG_FOUND ||
						g_cTagP != TAG_FOUND ||
						(fabs(g_RFIDData.fpstart - g_RFIDData.fpend) > 2.0) ||
						(fabs(g_RFIDData.fnstart - g_RFIDData.fnend) > 2.0))
				{
					//Failure case one
					logf("fc\r\n");
					//Setup Move
					g_RFIDData.cseekstate=RFID_SEEK_RETURN;
					//Setup return move
					goto setup_return_move;
				}
				logf("Found Tag\r\n");
				g_RFIDData.cstate = RFID_INIT;
				g_RFIDData.ccontext = RFID_CONTEXT_SEEK;
				g_RFIDData.cseekstate = RFID_SEEK_CENTER;

				g_RFIDData.fhs1 = (g_RFIDData.fpstart + g_RFIDData.fpend) / 2.0;
				g_RFIDData.fhs2 = (g_RFIDData.fnstart + g_RFIDData.fnend) / 2.0;
				g_RFIDData.fhsf = (g_RFIDData.fhs1 + g_RFIDData.fhs2) / 2.0;
				g_RFIDData.fposition = g_RFIDData.fhsf;
				logf("Center %f\r\n",g_RFIDData.fposition);
				//Turn on full reading
				g_RfidStateAndData.cFullReading = 1;
				rfidReadNow();//clear the throttle time so that it will read ASAP
				//Clear tag state
				g_cTagState=TAG_NONE;
				g_ulCenterStart = MS_TIMER;
				goto setup_center_move;
				//Done!
//FIXME
			}
			//Did not find both parts
			//Setup return move
			goto setup_return_move;
		}
		if (g_RFIDData.cseekstate==RFID_SEEK_RETURN)
		{
			//Move Back is done...  This is a failure to find a tag
			handle_as_rfid_seek_error:
			g_RFIDData.cstate = RFID_INIT;
			g_RFIDData.ccontext = RFID_CONTEXT_SEEK;
			g_RFIDData.cseekstate = RFID_SEEK_FAIL_NOT_FOUND;

			g_RFIDData.ulseektime = 0;//FIXME SEVERE
			g_RFIDData.fsstart = 0;
			g_RFIDData.fpstart = 0;
			g_RFIDData.fpend = 0;
			g_RFIDData.fnstart = 0;
			g_RFIDData.fnend = 0;
			g_RFIDData.fhs1 = 0;
			g_RFIDData.fhs2 = 0;
			g_RFIDData.fhsf = 0;
			SendRFIDData(0, STP_ALERT);
			g_cHomedX = HOME_FAILURE;
			g_cReadRFID = RFID_READ_OFF;
			//Clear this as well. Issue with library may have a residual tag presence
			g_cTagReadState = RFID_TAG_CLEAR;
			g_ulPositionUpdateThrottle=POSITION_UPDATE_THROTTLE_DEFAULT;
			break;
		}
		if (g_RFIDData.cseekstate==RFID_SEEK_CENTER)
		{
			logf("Center %f\r\n",g_RFIDData.fposition);
			if (g_cTagReadState!=RFID_TAG_IS_PRESENT && g_cTagState==TAG_NONE)
			{
				if (((unsigned long)(MS_TIMER - g_ulCenterStart)) < 10000)
				{
					//wait more
					break;
				}
				//Somehow, the tag could not be read here....
				logf("Final Read Fail\r\n");
				goto handle_as_rfid_seek_error;
			}
			//Copy Tag Data
			g_szTagDatalen = RFID_FMS00_DATA_SIZE - 1;//using 47 not 48... it sets last to ' ' below
			memcpy(g_szTagData,g_RfidStateAndData.szTagData+2,g_szTagDatalen);//64 + 1
			g_RFIDData.uicrc16 = *(unsigned int *)(g_RfidStateAndData.szTagData+2 + g_szTagDatalen);
			g_szTagData[g_szTagDatalen++]=' ';
			g_szTagData[g_szTagDatalen]=0;
			g_RFIDData.uiendcode = (unsigned int)g_RfidStateAndData.iOmronResponseEndCode;
			logf("rfid: e=%2x d=\"%s\"\r\n", g_RfidStateAndData.iOmronResponseEndCode, g_szTagData);
			//Verify Tag is valid format
			p_c=g_szTagData;
			if (p_c[0]!='0' || p_c[1]!='0')
			{	goto handle_as_bad_rfid_tag_error;}
			//valid tag format we know
			p_c+=3;
			if (*p_c!='T' && *p_c!='B')
			{	goto handle_as_bad_rfid_tag_error;}
			p_c++;
			cindex=0;
			while(cindex<12)
			{
				ck=p_c[cindex++];
				if (ck<'0'||ck>'9')
				{	goto handle_as_bad_rfid_tag_error;}
			}
			//Done checking tag format
			logf("Done ");
			g_RFIDData.cseekstate = RFID_SEEK_SUCCESS;
			SendRFIDData(0, STP_ALERT);
			g_cHomedX = HOME_DONE;
			g_cReadRFID = RFID_READ_OFF;
			g_cTagReadState = RFID_TAG_CLEAR;
			g_ulPositionUpdateThrottle=POSITION_UPDATE_THROTTLE_DEFAULT;
			//Done!
			break;
			handle_as_bad_rfid_tag_error:
			logf("Tag ???:\r\n");
			logf("len=%d\r\n",strlen(g_RfidStateAndData.szTagData));
			memdump("tag",g_RfidStateAndData.szTagData,60);
			g_RFIDData.cstate = RFID_INIT;
			g_RFIDData.ccontext = RFID_CONTEXT_SEEK;
			g_RFIDData.cseekstate = RFID_SEEK_FAIL_BAD_TAG_DATA;

			SendRFIDData(0, STP_ALERT);
			g_cHomedX = HOME_FAILURE;
			g_cReadRFID = RFID_READ_OFF;
			//Clear this as well. Issue with library may have a residual tag presence
			g_cTagReadState = RFID_TAG_CLEAR;
			g_ulPositionUpdateThrottle=POSITION_UPDATE_THROTTLE_DEFAULT;
			break;
		}
		break;
		setup_seek_move1:
		g_RFIDData.cseekstate=RFID_SEEK_MOVE1;
		g_fTargetX = g_fStartX + g_ConfigData.RFIDConfig.fseekMove1;
		g_cMoveDir = JOGPOS;
		if (g_ConfigData.RFIDConfig.fseekMove1 < 0)
		{	g_cMoveDir = JOGNEG;}
		if (g_ConfigData.RFIDConfig.fseekMove1!=0)
		{
			//Setup move 1
			SetMoveSpeedParamsEven(g_ConfigData.RFIDConfig.fseekSpeed,10);
#ifdef OUTPUT_RFID_HOME
			logf("M X %s %.4f\r\n","sm1",g_fTargetX);
#endif
			MCRunPosition( g_fTargetX, g_fStartY );
		}
		break;
		setup_seek_move2:
		g_RFIDData.cseekstate=RFID_SEEK_MOVE2;
		g_fTargetX = g_fStartX + g_ConfigData.RFIDConfig.fseekMove2;
		g_cMoveDir = JOGPOS;
		if (g_fTargetX < g_PosnMode.fLastKnownPosnX)
		{	g_cMoveDir = JOGNEG;}
		if (g_ConfigData.RFIDConfig.fseekMove2!=0)
		{
			//Setup move 2
			SetMoveSpeedParamsEven(g_ConfigData.RFIDConfig.fseekSpeed,10);
#ifdef OUTPUT_RFID_HOME
			logf("M X %s %.4f\r\n","sm2",g_fTargetX);
#endif
			MCRunPosition( g_fTargetX, g_fStartY );
		}
		break;
		setup_seek_moveoff:
		g_RFIDData.cseekstate=RFID_SEEK_MOVEOFF;
		//New Target ... continue same direction
		if (g_cMoveDir == JOGNEG)
		{
			g_fTargetX = g_PosnMode.fLastKnownPosnX - g_ConfigData.RFIDConfig.fseekFineMove; //was going negative continue going negative
		}
		else
		{
			g_fTargetX = g_PosnMode.fLastKnownPosnX + g_ConfigData.RFIDConfig.fseekFineMove; //was going positive continue going positive
		}
		SetMoveSpeedParamsEven(g_ConfigData.RFIDConfig.fseekSpeed,10);
#ifdef OUTPUT_RFID_HOME
		logf("M X %s %.4f\r\n","smo",g_fTargetX);
#endif
		MCRunPosition( g_fTargetX, g_fStartY );
		break;
		setup_seek_fine_move:
		//Setup fine move 1 or 2
		//New Target
		if (g_cMoveDir == JOGNEG)
		{
			g_fTargetX = g_PosnMode.fLastKnownPosnX + g_ConfigData.RFIDConfig.fseekFineMove; //was going negative move back positive
			g_cMoveDir = JOGPOS;
		}
		else
		{
			g_fTargetX = g_PosnMode.fLastKnownPosnX - g_ConfigData.RFIDConfig.fseekFineMove; //was going positive move back negative
			g_cMoveDir = JOGNEG;
		}
		SetMoveSpeedParamsEven(g_ConfigData.RFIDConfig.fseekFineSpeed,10);
#ifdef OUTPUT_RFID_HOME
		logf("M X %s %.4f\r\n","sf",g_fTargetX);
#endif
		MCRunPosition( g_fTargetX, g_fStartY );
		break;
		setup_return_move:
		g_RFIDData.cseekstate=RFID_SEEK_RETURN;
		//Setup return move
		g_fTargetX = g_fStartX;
		SetMoveSpeedParamsEven(g_ConfigData.RFIDConfig.fseekSpeed,10);
#ifdef OUTPUT_RFID_HOME
		logf("M X %s %.4f\r\n","r",g_fTargetX);
#endif
		MCRunPosition( g_fTargetX, g_fStartY );
		break;
		setup_center_move:
		g_RFIDData.cseekstate=RFID_SEEK_CENTER;
		//Setup return move
		g_fTargetX = g_RFIDData.fposition;
		SetMoveSpeedParamsEven(g_ConfigData.RFIDConfig.fseekSpeed,10);
#ifdef OUTPUT_RFID_HOME
		logf("M X %s %.4f\r\n","c",g_fTargetX);
#endif
		MCRunPosition( g_fTargetX, g_fStartY );
		break;
	}
#ifdef USE_RFID
	g_cLastTagReadState = g_cTagReadState;
#endif
#warnt "FIXME MED RFID Features are not fully compelted"
//1) resolve partial read system and final reading issues
//3) test and confirm needed fast exits w/ min read cycles as well
//4) put in approx positioning and measure it's offset....

//FIXME Improve code by adding better exit state from this so I can do the exit code just once
	if (g_cHomedX != HOME_RUNNING)
	{
//FIXME alter RFID rate and any other changes
		//RFID Home is still running,??? Stop????
		//FIXME Severe...not sure what to do about the states, how to start, how to run etc....
		if (g_ulPositionUpdateThrottle==POSITION_UPDATE_THROTTLE_RFID_SCAN)
		{
			g_ulPositionUpdateThrottle=POSITION_UPDATE_THROTTLE_DEFAULT;
		}
	}
#endif //HOMESYSTEM_X_RFID
}
#endif //USE_RFID_OMRON
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef USE_RFID_MIFARE
void RFIDRun()
{
	unsigned long ulx;
	td_RFID_F0 * p_rfid_f0;
	td_RFID_F0 RFID_F0Data;
	int i;
	char sprintfbuf[16];
	char * p_cSerialNumber;
//support immediate and continuous tag reading here...
//This subsystem could be extended to support more than just RFID, but it currently does not. (a suggestion only)
	if (g_cReadRFID != RFID_READ_OFF)
	{
		if (g_cReadRFID == RFID_READ_STOP)
		{
			g_cReadRFID = RFID_READ_OFF;
			goto skip_rfid_read;
		}
		if (g_cReadRFID == RFID_READ_SEEK)
		{
			if (g_cHomedX != HOME_RUNNING)
			{
				g_cReadRFID = RFID_READ_OFF;
				goto skip_rfid_read;
			}
		}

		if (g_cReadRFID == RFID_READ_NOW)
		{
			//read right now.. and turn reading off
			SL031LoginSector();
			SL031ReadData();
			g_cReadRFID = RFID_READ_OFF;
		}
		else if (g_cReadRFID == RFID_READ_CONTINUOUS)
		{
			//for continuous, read when state changes and show.
			logf("cni\r\n"); //Continuous Not Implemented
			//Since it's not implemented
			g_cReadRFID = RFID_READ_OFF;
		}
		else if (g_cReadRFID == RFID_READ_SEEK)
		{
			//just let the RFID search routine read when it's time
		}

//since I don't monitor for change this way I also don't send here.
//			if (g_cReadRFID!=RFID_READ_SEEK)
//			{
//				//then use OID to alert this now
//				g_RFIDData.cstate = RFID_INIT;
//				g_RFIDData.ccontext = RFID_CONTEXT_READ;
//				g_RFIDData.cseekstate = RFID_SEEK_NULL;
//				g_RFIDData.fposition = 0;
//				g_RFIDData.fsstart = 0;
//				g_RFIDData.fpstart = 0;
//				g_RFIDData.fpend = 0;
//				g_RFIDData.fnstart = 0;
//				g_RFIDData.fnend = 0;
//				g_RFIDData.fhs1 = 0;
//				g_RFIDData.fhs2 = 0;
//				g_RFIDData.fhsf = 0;
//				SendRFIDData(0, STP_ALERT);
//			}
	}
	skip_rfid_read:
	//Always check to see what results comeback.
//I don't think it takes long to see if data is waiting, so this isn't really a problem.
//The library handles the responses differently depending on if it is expecting a response. ( see SL031CommRcv() );
	SL031CommRcv();

//NEW RFID LIBRARY CODE IN THE NEXT SECTION, INCLUDING FIND WINDOW CODE
#ifdef HOMESYSTEM_X_RFID
//This code works with a special console command to simulate an immediate RFID Homing Success
//FIXME LOW TEMP this might be considered temp code... or temp quallity anyway... might want to consider organizing it differently in the future.
	if (g_cImmediateFakeRfid == 1)
	{
		g_cImmediateFakeRfid = 0;
		memcpy(g_bRFIDTagData, g_szTagData, g_szTagDatalen);
		goto immediate_fake_rfid;
	}

	while (g_cHomedX == HOME_RUNNING)
	{
		//Advance X RFID Homing
//FIXME SEVERE  How to start or restart....
//How and when to clear the stored data....
//store home across reads of continuous????
//
//exit state and how to manange rfid
		if (g_ConfigData.RFIDConfig.cenabled == 0)
		{
			g_cHomedX = HOME_FAILURE;
			break;
		}
		if (g_RFIDData.cseekstate < RFID_SEEK_INIT)
		{
			logf("RFpi\r\n");
			if (g_cMoveDone == MOVEDONE_FALSE)
			{
				//Stop Move
//testpath
				if (g_cMCRunningMsgGoal != PositionStop)
				{
					MCStopPositionAdvanced();
				}
				break;
			}
			g_cMoveDone = MOVEDONE_TRUE;
			MCGetPositionCounts();
			g_RFIDData.ccontext = RFID_CONTEXT_SEEK;
			g_RFIDData.cseekstate = RFID_SEEK_INIT;
		}
		if (g_RFIDData.cseekstate == RFID_SEEK_INIT)
		{
			if (g_PosnMode.cFreshPosn != X_AND_Y)
			{
				//did not get a fresh position yet
				break;
			}
			logf("RFi\r\n");
			//Start RFID Home
			g_fStartX = g_PosnMode.fLastKnownPosnX;
			g_fStartY = g_PosnMode.fLastKnownPosnY;
#ifdef OUTPUT_RFID_HOME
			logf("Start %f %f\r\n", g_fStartX, g_fStartY);
#endif
			g_RFIDData.ccontext = RFID_CONTEXT_SEEK;
			g_RFIDData.cseekstate = RFID_SEEK_INIT;
			g_RFIDData.fposition = 0;
			g_RFIDData.fsstart = 0;
			g_RFIDData.fpstart = 0;
			g_RFIDData.fpend = 0;
			g_RFIDData.fnstart = 0;
			g_RFIDData.fnend = 0;
			g_RFIDData.fhs1 = 0;
			g_RFIDData.fhs2 = 0;
			g_RFIDData.fhsf = 0;
			SendRFIDData(0, STP_ALERT);

			//This is to ensure that this will be set to 1, only after a FindWinOK
			g_cMCWindowFound = 0;

			//DJWR FIXED 20120314-1
			//Should use action code here
			g_cAction = ACTION_HOME;
		}
		if (g_cObstructionCode != 0)
		{
			logf("o s rf\r\n");
			AlertObstructionCode(0);
			MCStopPosition();
#warning "TEST THIS PATH and determine if any additional action or alternate code is needed other than letting move fail below"
			g_RFIDData.cseekstate = RFID_SEEK_FAIL_HARDSTOP;
		}
		if (g_RFIDData.cseekstate == RFID_SEEK_INIT)
		{
			//Init RFID Home ... Continued
			//Do not bother checking current move status
			//Setup Move
			//Setup Seek Move 1
			g_RFIDData.cseekstate = RFID_SEEK_MOVE1;
			SetupRFIDSeekMove(g_ConfigData.RFIDConfig.fseekMove1, "sm1");
			break;
		}
		if (g_cMoveDone != MOVEDONE_TRUE)
		{
			//Clear this on every loop through here
			//This is to ensure that this will be set to 1, only after a FindWinOK
			g_cMCWindowFound = 0;
			if (g_cMoveDone == MOVEDONE_FALSE)
			{
				//continue waiting....
				break;
			}
			if (g_cMoveDone == MOVEDONE_ERROR)
			{
				logf("mde\r\n");
				if (g_RFIDData.cseekstate == RFID_SEEK_RETURN)
				{
					//A move error on the last move... Can't return to start position
					g_RFIDData.cseekstate = RFID_SEEK_FAIL_HARDSTOP;
				}
				else if ((g_ConfigData.RFIDConfig.uioptions
						& RFID_OPTION_REVERSE_ON_HARDSTOP) == 0)
				{
					//Hard Stop should be treated as an end case
					g_RFIDData.cseekstate = RFID_SEEK_FAIL_HARDSTOP;
				}
				else
				{
					//Just move on to do next move as normal... This is the effect of the RFID_OPTION_REVERSE_ON_HARDSTOP Option
					g_cMoveDone = MOVEDONE_TRUE;
				}
			}
			else if (g_cMoveDone == MOVEDONE_STOP)
			{
				//Got Our Stop Confirmation... clear the state
//FIXME HIGH
#warning "//FIXME HIGH should see if this is invovled in RFID Home Stop or if that bypasses the entire thing.. and test STOP"
//FIXME MED .  Don't call a user STOP a hardstop"
				logf("mdsc\r\n");
				g_RFIDData.cseekstate = RFID_SEEK_FAIL_HARDSTOP;
				g_cMoveDone = MOVEDONE_TRUE;
			}
		}
		//Move is done
#ifdef OUTPUT_RFID_HOME
		if (g_RFIDData.cseekstate != RFID_SEEK_CENTER) //because this state loops too many times while checking for final read...
		{ //but for all other states, show md
			logf("md\r\n");
		}
#endif

		if (g_cMCWindowFound == 1 && g_RFIDData.cseekstate != RFID_SEEK_CENTER)
		{
			logf("ftl\r\n");
			//Found Tag Location
			//Enter State SEEK CENTER and do READ
			g_RFIDData.cseekstate = RFID_SEEK_CENTER; //Using Center, but not for move
			//found tag location ... set state to RFID_SEEK_CENTER,
			//Read the tag now

//#define RFID_FAKED
#ifdef RFID_FAKED
#warns "RFID FAKED"
			//RFID FAKE//////////////////////
			g_cRFIDResult = RFID_RESULT_GOOD_DATA;
			memcpy(g_bRFIDTagData,g_szTagData,g_szTagDatalen);
#else
			//RFID READ
			SL031LoginSector();
			SL031ReadData();
#endif
			break;
		}

		//did not find tag location yet
		if (g_RFIDData.cseekstate == RFID_SEEK_MOVE1)
		{
			//Setup Seek Move 2
			g_RFIDData.cseekstate = RFID_SEEK_MOVE2;
			SetupRFIDSeekMove(g_ConfigData.RFIDConfig.fseekMove2, "sm2");
			break;
		}
		if (g_RFIDData.cseekstate == RFID_SEEK_MOVE2)
		{
			//Setup Seek and Return Move
			g_RFIDData.cseekstate = RFID_SEEK_RETURN;
			SetupRFIDSeekMove(0, "r");
			break;
		}
		if (g_RFIDData.cseekstate == RFID_SEEK_RETURN)
		{
			//Move Back is done...  This is a failure to find a tag
			g_RFIDData.cseekstate = RFID_SEEK_FAIL_NOT_FOUND;
			//Allow code to continue on to final state handler
		}
		if (g_RFIDData.cseekstate == RFID_SEEK_CENTER)
		{
			//See if RFID Tag Had Login or Was Read Yet
			if (g_bRFIDResult == RFID_RESULT_PENDING)
			{
				//wait longer for tag data
				break;
			}
			if (g_PosnMode.cFreshPosn != X_AND_Y)
			{
				//did not get a fresh position yet
				//Position is requested at the same time as the window flag is set.
				break;
			}

			//tag read result is ready
			if (g_bRFIDResult == RFID_RESULT_READ_SUCCESS)
			{
				g_szTagDatalen = RFID_F0_DATA_SIZE;
				immediate_fake_rfid: //in this case, use the set len

				//Read Result was RFID_RESULT_GOOD_DATA
				logf("Found Tag\r\n");
				g_RFIDData.cstate = RFID_INIT;
				g_RFIDData.ccontext = RFID_CONTEXT_SEEK;
				g_RFIDData.cseekstate = RFID_SEEK_CENTER;

				g_RFIDData.fhs1 = 0;
				g_RFIDData.fhs2 = 0;
				g_RFIDData.fhsf = 0;
				g_RFIDData.fposition = g_PosnMode.fPosnX;

				//Copy Tag Data

				memcpy(g_szTagData, g_bRFIDTagData, g_szTagDatalen);
				g_szTagData[g_szTagDatalen] = 0; //even though we don't show it, null this so if it does get printed, it does not fail
				g_RFIDData.uicrc16 = 0; //NA
				g_RFIDData.uiendcode = 0; //NA
				p_rfid_f0 = (td_RFID_F0 *) g_szTagData;
				logf("rfid:\r\n");
				if (p_rfid_f0->cFormat == '0')
				{
					//old omron format means that user has supplied a tag data for rfid sim... so allow it....
					logf("%s\r\n", g_szTagData);
				}
				else
				{
					//newer binary format
					logf("%d\r\n", p_rfid_f0->cFormat);
					logf("%d\r\n", p_rfid_f0->cRailType);
					logf("%d\r\n", p_rfid_f0->cGroup);
					logf("%d\r\n", p_rfid_f0->cSegmentAndRailSide);
					logf("%u\r\n", ntohul(p_rfid_f0->ulSerialNumber) );
					logf("%u\r\n", ntohul(p_rfid_f0->ulPosition) );
					logf("%u\r\n", ntohul(p_rfid_f0->ulSegmentPosition) );
					//Verify Tag is valid format
					if (p_rfid_f0->cFormat != 0)
					{
						goto handle_bad_rfid_format;
					} //It's NOT format 0
					if (p_rfid_f0->cRailType > 16)
					{
						//assume rail type and revisions 0 to 16 are valid at this time for this tag... so this one is not valid
						goto handle_bad_rfid_format;
					}
//logf("%d\r\n",1);
					if (p_rfid_f0->cGroup > 16)
					{
						//assume groups 0 to 16 are valid... so this one is not valid
						goto handle_bad_rfid_format;
					}
					if ((p_rfid_f0->cSegmentAndRailSide & 127) > 32)
					{
						//assume segements 0 to 32 are valid... so this one is not valid
						goto handle_bad_rfid_format;
					}
					ulx = ntohul(p_rfid_f0->ulPosition);
					if (ulx > 2000000)
					{
						//assume less than 2000 inches is valid... so this one is not valid
						goto handle_bad_rfid_format;
					}
//logf("%d\r\n",4);
					ulx = ntohul(p_rfid_f0->ulSegmentPosition);
					if (ulx > 100000)
					{
						//assume less than 100 inches is valid... so this one is not valid
						goto handle_bad_rfid_format;
					}
					if ((p_rfid_f0->cSegmentAndRailSide & 128) > 0)
					{
						g_iRFIDRailOrientation = 1;
					}
					else
					{
						g_iRFIDRailOrientation = -1;
					}
					//Save the positions that relate RFID Rail Coordinates to the machine.
					//This assumes that RFID offset is given relative to machine primary coords. See RFIDOrient doc.
					g_fRFIDRailX = ((float) p_rfid_f0->ulPosition) / 1000.0; //Record the Tag's Rail Coordinate
					g_fRFIDMachineX = g_PosnMode.fPosnX
							+ g_iRFIDRailOrientation
									* g_ConfigData.RFIDConfig.fRFIDOffset;
//logf("%f %f %f\r\n",g_PosnMode.fPosnX,g_fRFIDRailX,g_fRFIDMachineX);
#define SIMULATE_OLDER_FORMAT_TAG
#ifdef SIMULATE_OLDER_FORMAT_TAG
					RFID_F0Data = *p_rfid_f0; //copy all struct to this location
					if ((RFID_F0Data.cSegmentAndRailSide & 127) > 9)
					{
						// too many digits for conversion
						goto handle_bad_rfid_format;
					}
					//now build older format data
					p_cSerialNumber = g_szTagData + 16;
					memset(g_szTagData, '0', 16); //zeros at front
					memset(p_cSerialNumber, ' ', 48); //spaces behind serial
					g_szTagData[2] = '0' + (RFID_F0Data.cSegmentAndRailSide & 127);
					if ((RFID_F0Data.cSegmentAndRailSide & 128) > 0)
					{
						g_szTagData[3] = 'T';
					}
					else
					{
						g_szTagData[3] = 'B';
					}
//logf("%d\r\n",12);
					sprintf(sprintfbuf, "%u", ntohul(RFID_F0Data.ulPosition) );
					i = strlen(sprintfbuf);
					if (i > 6)
					{
						goto handle_bad_rfid_format;
					}
					memcpy(g_szTagData + 4 + 6 - i, sprintfbuf, i);
					memcpy(g_szTagData + 4 + 6, "072000", 6);
					sprintf(p_cSerialNumber, "serial%u", ntohul(RFID_F0Data.ulSerialNumber) );

					g_szTagDatalen = RFID_FMS00_DATA_SIZE;
					g_szTagData[g_szTagDatalen] = 0; //null this just to be clear.
					//show the constructed tag
					logf("%s\r\n", g_szTagData);
#endif
				}
				//Done checking tag format
				logf("Done ");
				g_RFIDData.cseekstate = RFID_SEEK_SUCCESS;
				SendRFIDData(0, STP_ALERT);
				g_cHomedX = HOME_DONE;
				g_cReadRFID = RFID_READ_OFF;
				//Done!
				break;
				handle_bad_rfid_format: memdump("tag", g_bRFIDTagData, 60);
				g_RFIDData.cseekstate = RFID_SEEK_FAIL_BAD_TAG_DATA;
			}
			else if (g_bRFIDResult == RFID_RESULT_ERROR)
			{
				g_RFIDData.cseekstate = RFID_SEEK_FAIL_RFID_ERROR;
			}
			else if (g_bRFIDResult == RFID_RESULT_NO_TAG)
			{
				g_RFIDData.cseekstate = RFID_SEEK_FAIL_NOT_FOUND;
			}
			else if (g_bRFIDResult == RFID_RESULT_READ_FAIL) //really just a bad read in this case
			{
				g_RFIDData.cseekstate = RFID_SEEK_FAIL_BAD_TAG_DATA;
			}
			else
			{
				//any other result is not expected, but callit an RFID ERROR
				g_RFIDData.cseekstate = RFID_SEEK_FAIL_RFID_ERROR;
			}
		}
		if (g_RFIDData.cseekstate <= RFID_SEEK_TERMINAL_STATE)
		{
			logf("is\r\n");
			//Any other states than the ones above and the terminal states are not used by the new RFID
			g_RFIDData.cseekstate = RFID_SEEK_FAIL_RFID_ERROR;
		}
		//Handle Final RFID State
		g_RFIDData.cstate = RFID_INIT;
		g_RFIDData.ccontext = RFID_CONTEXT_SEEK;

		g_RFIDData.ulseektime = 0; //FIXME SEVERE
		g_RFIDData.fsstart = 0;
		g_RFIDData.fpstart = 0;
		g_RFIDData.fpend = 0;
		g_RFIDData.fnstart = 0;
		g_RFIDData.fnend = 0;
		g_RFIDData.fhs1 = 0;
		g_RFIDData.fhs2 = 0;
		g_RFIDData.fhsf = 0;
		SendRFIDData(0, STP_ALERT);
		g_cHomedX = HOME_FAILURE;
		g_cReadRFID = RFID_READ_OFF;
		logf("rfid done fail %d\r\n", g_RFIDData.cseekstate);
		break;
	}
#endif //HOMESYSTEM_X_RFID
}
#endif //USE_RFID_MIFARE
//RFID notes:
//FIXME MINOR  Could do more	p_RFIDConfig->uioptions=htons(g_ConfigData.RFIDConfig.uioptions);
//FIXME MINOR use these ?? :
//					p_RFIDConfig->uicontinuousReadCycleTime=htons(g_ConfigData.RFIDConfig.uicontinuousReadCycleTime);
//					p_RFIDConfig->uiseekReadCycleTime=htons(g_ConfigData.RFIDConfig.uiseekReadCycleTime);
//FIXME MINOR use this?	p_RFIDConfig->fRFIDOffset=g_ConfigData.RFIDConfig.fRFIDOffset;
//FIXME Medium Use Rail Posn from tag????
#warning "FIXME MED RFID Features are not fully compelted"

#define OUTPUT_DFINT_STATES
#define OUTPUT_PRIMEDELAY
//FIXME dfint.... dfnow
void DFModeStateMachine(void)
{
	int i;
//unsigned int uiSTPmsgSize;
	//byte cEntryState;
	byte c;
	//char * p_c;
	char * p_sStateName;
	float fDiameter;
	static byte s_cDrillStatePrev;
	static byte s_cSealStatePrev;
	static byte s_cFillStatePrev;

	static uint32 s_uiStartTime;
	static uint32 s_uiSealStart;
	static uint32 s_uiSealPressureOn;
	static uint32 s_uiFillStart;
	static uint32 s_uiClampStart;
	static uint32 s_uiFillFastenerArrive;
	static uint32 s_uiAccelArmTime;
	static uint32 s_uiSamples;
	char defaultbuf[12];

//Alert Out the Goal State
	if (g_cDrillStateGoalSent != g_cDrillStateGoal)
	{
		logf("Send DSG %d\r\n", g_cDrillStateGoal);
		//send out drill state goal
		SmartToolMsgChar(0, STP_ALERT, MINIFT_OID_DRILL_STATE,
				g_cDrillStateGoal);
		g_cDrillStateGoalSent = g_cDrillStateGoal;
	}

#ifdef DRILL_DIRECT_READY
//DRILL DIRECT CONTROL CODE
	if (g_cSmartDrill==1)//FIXME Optional Component System concept needs improvement
	{
		//Send any new commands to the drill
		if (g_cDrillStateGoalCommanded != g_cDrillStateGoal)
		{
			if (g_DrillSTP.wLastState >= tcp_StateESTAB)
			{
				logf("DSG = %d\r\n",g_cDrillStateGoal);

				if (g_cDrillStateGoal == DRILLSTATE_IDLE)
				{
					logf("sa\r\n");
					SmartDrillSTPSet(SMARTDRILL_OID_ABORT);
				}
				else if (g_cDrillStateGoal == DRILLSTATE_SPINUP)
				{
					SmartDrillSTPSet(SMARTDRILL_OID_SPINUP);
					logf("su\r\n");
				}
				else if (g_cDrillStateGoal == DRILLSTATE_DRILL)
				{
					SmartDrillSTPSet(SMARTDRILL_OID_DRILL);
					logf("sd\r\n");
				}
				else if (g_cDrillStateGoal == DRILLSTATE_HOME)
				{
					logf("sh\r\n");
					SmartDrillSTPSet(SMARTDRILL_OID_HOME);
					//This is temporary until Tom returns mode all the time after homing.
					//Currently Tom may skip this in the case that he is already home.
					if (g_cDrillStateGoal==DRILLSTATE_HOME)
					{
						//drill was home???...  just get mode to confirm and trigger end
						logf("skipped\r\n");
						//SmartDrillGetMode();
					}
					else
					{
						logf("wnh\r\n");
					}
				}
//FIXME Severe  despite the check... we should also check for stp sending failures AFTER the send
				g_cDrillStateGoalCommanded = g_cDrillStateGoal;

			}
			else
			{
//Not Connected.....!!!!!!!
//FIXME
//work on this pattern

//FIXME Severe
//current idea is that it will detect this and fail operation goal.
//but without code here, it will just continue and wait
			}
		}
	}
#endif

#ifdef DRILLFILL_STP
	if (g_cEEOption==EEDRILLFILL) //if drill fill, then act on state machine
	{
		if (g_cDrillStateDFGoal != g_cDrillStateGoal)
		{
			if (g_cDrillStateGoal == DRILLSTATE_HOME)
			{
				logf("DG %c\r\n",'h');
				DrillFillHome();
				if (DrillFillHome()==0)
				{
					//It couldn't even send the signal
					g_cDrillHomed = HOME_FAILURE;
				}
			}
			else if (g_cDrillStateGoal == DRILLSTATE_SPINUP)
			{
				logf("DG %c\r\n",'s');
				DrillFillSpinUp();
			}
			else if (g_cDrillStateGoal == DRILLSTATE_DRILL)
			{
				logf("DG %c\r\n",'d');
				g_HoleResultData.iHoleNumber = g_PosnMode.iCurPosnIndex;
				g_HoleResultData.iHoleResult = HR_STARTED;
				ServiceDateTime();
				g_LoadedTool.ulDTimeTicksMSW=g_DateTime.ulticksMSW;
				g_LoadedTool.ulDTimeTicksLSW=g_DateTime.ulticksLSW;
				g_LoadedTool.uiDCount++; //might increment two times, but this is only DSI code
				SendTool(0, STP_ALERT,update);
				if (DrillFillDrillStart()==0)
				{
					g_HoleResultData.iHoleResult = HR_FAULT;
					logf("fault??\r\n");
				}
			}
			else if (g_cDrillStateGoal == DRILLSTATE_IDLE)
			{
				DrillFillDrillAbort();
			}
			else
			{
				logf("??DSG=%d\r\n");
			}
			g_cDrillStateDFGoal = g_cDrillStateGoal;
		}
	}
#endif

#ifdef SEAL_SYSTEM
//sealant state machine
	cEntryState=g_cSealState;
	switch (g_cSealState)
	{
		case SEALSTATE_OFF:
		//standard start up status
		g_cSealantPressure=SEALANT_PRESSURE_OFF;//pressure off
		g_cSealantPinch=SEALANT_PINCH_ON;//pinch
		g_cSealantApply=SEALANT_APPLY_OFF;//tip retracted
		//Set it to AUTOPRIME TO BEGIN AUTO PATH
		break;
		case SEALSTATE_START_LOAD:
		g_cSealantPressure=SEALANT_PRESSURE_OFF;//pressure off
		g_cSealantPinch=SEALANT_PINCH_ON;//pinch
		g_cSealantApply=SEALANT_APPLY_OFF;//tip retracted
		s_ulSealStart=MS_TIMER;//start waiting.
		g_cSealState=SEALSTATE_WAIT_LOAD;
		break;
		case SEALSTATE_WAIT_LOAD:
		if ((MS_TIMER - s_ulSealStart) > g_ConfigData.iSealPressureReleaseDelay)
		{
			g_cSealState=SEALSTATE_LOAD;
		}
		break;
		case SEALSTATE_LOAD:
		g_cSealantPressure=SEALANT_PRESSURE_OFF; //pressure off
		g_cSealantPinch=SEALANT_PINCH_OFF;//release pinch
		g_cSealantApply=SEALANT_APPLY_OFF;//tip retracted
		break;
		case SEALSTATE_PRIME:
		if (g_cSealantPressure==SEALANT_PRESSURE_OFF)
		{
			g_cSealantPressure=SEALANT_PRESSURE_ON; //pressure on
			s_ulSealPressureOn = MS_TIMER;
		}
		g_cSealantPinch=SEALANT_PINCH_OFF; //release pinch
		g_cSealantApply=SEALANT_APPLY_OFF;//tip retracted
		break;
		case SEALSTATE_PRESSURE:
		if (g_cSealantPressure==SEALANT_PRESSURE_OFF)
		{
			g_cSealantPressure=SEALANT_PRESSURE_ON; //pressure on
			s_ulSealPressureOn = MS_TIMER;
		}
		g_cSealantPinch=SEALANT_PINCH_ON; //pinch
		g_cSealantApply=SEALANT_APPLY_OFF;//tip retracted
		break;
		case SEALSTATE_AUTOPRIME:
#ifdef OUTPUT_DFINT_STATES
		logf("@S_AUTOPRIME\r\n");
#endif
		if (g_cSealantPressure==SEALANT_PRESSURE_OFF)
		{
			g_cSealantPressure=SEALANT_PRESSURE_ON; //pressure on
			s_ulSealPressureOn = MS_TIMER;
		}
		g_cSealantPinch=SEALANT_PINCH_ON; //pinch on
		g_cSealantApply=SEALANT_APPLY_OFF;//tip retracted
#ifdef OUTPUT_DFINT_STATES
		logf("@S_WAIT_PRESSURE\r\n");
#endif
		g_cSealState=SEALSTATE_WAIT_PRESSURE;
		break;
		case SEALSTATE_WAIT_PRESSURE:
		//FIXME00000 I know I must examine this s_ulSealPressureOn variable...
		//I also shouldn't clear it so quickly during auto.... perhaps
		//it could be delayed off....  I'll have to think about this later...
		if ((MS_TIMER - s_ulSealPressureOn) > g_ConfigData.iSealPressureDelay)
		{
			g_cSealantPinch=SEALANT_PINCH_OFF; //release pinch
			g_cSealantApply=SEALANT_APPLY_OFF;//tip retracted
			s_ulSealStart=MS_TIMER;//start waiting.
			// Use diameter table mechanism, based on diameter for this hole
//FIXME SEVERE : Will this always be set here????
			fDiameter = ((float)g_HoleData.d2uDiameter)/10000;
			g_iPrimeDelay=LookupDiameterPrimeDelay( fDiameter );
#ifdef OUTPUT_PRIMEDELAY
			logf("PrimeDelay: Diameter=%f Seal Prime Delay = %d\r\n",fDiameter,g_iPrimeDelay);
#endif
#ifdef OUTPUT_DFINT_STATES
			logf("@S_WAIT_AUTOPRIME\r\n");
#endif
			g_cSealState=SEALSTATE_WAIT_AUTOPRIME;
		}
		break;
		case SEALSTATE_WAIT_AUTOPRIME:
		if ((MS_TIMER - s_ulSealStart) > g_iPrimeDelay)
		{
			g_cSealState=SEALSTATE_WAIT_GLOB;
			g_cSealantPinch=SEALANT_PINCH_ON; //pinch
			s_ulSealStart=MS_TIMER;//start waiting for glob now
#ifdef OUTPUT_DFINT_STATES
			logf("@S_WAIT_GLOB\r\n");
#endif
		}
		break;
		case SEALSTATE_WAIT_GLOB:
		if ((MS_TIMER - s_ulSealStart) > g_ConfigData.iSealGlobDelay)
		{
			g_cSealState=SEALSTATE_WAIT;
#ifdef OUTPUT_DFINT_STATES
			logf("@S_WAIT\r\n");
#endif
		}
		break;
		case SEALSTATE_WAIT:
		//Remain Here until external code tells us to apply
		//by setting the state to SEALSTATE_APPLY
		//Used to allow clamp options... currently not doing this
		//	if (g_ConfigData.cSealClamp==0)
		//    {
		//    	//make sure still unclamped
		//    	g_cClampGoal=CLAMP_UNCLAMP;
		//		if (g_cClampState==CLAMP_UNCLAMP)
		//        {
		//        	//now apply
		//			g_cSealState=SEALSTATE_APPLY;
		//        }
		//    }
		//    else
		//    {
		//       	//use clamp
		//    	g_cClampGoal=CLAMP_CLAMP;
		//		if (g_cClampState==CLAMP_CLAMP)
		//        {
		//        	//now apply
		//			g_cSealState=SEALSTATE_APPLY;
		//		}
		//    }
		break;
		case SEALSTATE_APPLY:
#ifdef OUTPUT_DFINT_STATES
		logf("@S_APPLY\r\n");
#endif
		// Report Seal
		AddOpHistory(g_PosnMode.iCurPosnIndex, OP_SEAL);
		// Now Wait for seal to be done
		g_cSealantPressure=SEALANT_PRESSURE_ON;//pressure on
		g_cSealantApply=SEALANT_APPLY_ON;//tip out
		s_ulSealStart=MS_TIMER;//start waiting.
		g_cSealState=SEALSTATE_WAIT_APPLY;
#ifdef OUTPUT_DFINT_STATES
		logf("@S_WAIT_APPLY\r\n");
#endif
		break;
		case SEALSTATE_WAIT_APPLY:
		if ((MS_TIMER - s_ulSealStart) > g_ConfigData.iSealApplyDelay)
		{
			g_cSealState=SEALSTATE_DONE;
			g_cSealantApply=SEALANT_APPLY_OFF; //tip retracted
#ifdef OUTPUT_DFINT_STATES
			logf("@S_DONE\r\n");
#endif
		}
		break;
		case SEALSTATE_DONE:
		//wait here until somebody changes us back to off
		g_cSealantPressure=SEALANT_PRESSURE_OFF;//pressure off
		g_cSealantPinch=SEALANT_PINCH_ON;//pinch
		g_cSealantApply=SEALANT_APPLY_OFF;//tip retracted
		break;
	}
	if (g_cSealState!=s_cSealStatePrev) //The only time output changes above is when the state changes.
	{
		digOut(DIGOUT_CHNUM_SEALANT_PRESSURE, g_cSealantPressure);
		digOut(DIGOUT_CHNUM_SEALANT_PINCH, g_cSealantPinch);
		digOut(DIGOUT_CHNUM_SEALANT_APPLY, g_cSealantApply);
	}
	s_cSealStatePrev=cEntryState;
#endif
#ifdef FASTENER_SYSTEM

#endif
	return;

//FIXMENOW got rid of clamp but may need to review DF clamp vs OUR clamp....
}

void SetupRFIDSeekMove(float fmovedelta, char * label)
{
	g_fTargetX = g_fStartX + fmovedelta;
//Setup Move to target
	MCSetMoveSpeedParamsX(g_ConfigData.RFIDConfig.fseekSpeed, 16, 16);
#ifdef OUTPUT_RFID_HOME
	logf("M X %s %.4f\r\n", label, g_fTargetX);
#endif
	RunFindWindowX(g_fTargetX);
}

//FIXME0000000000 relocate

#ifdef HOMESYSTEM
void ClearAllHomeStatus()
{
//For this function clear all home status... only used for sys init
#ifdef HOMESYSTEM_X
	g_cHomedX = HOME_NOT_DONE;	//X Axis
#endif
#ifdef HOMESYSTEM_Y
	g_cHomedY = HOME_NOT_DONE;	//Y Axis
#endif
#ifdef HOMESYSTEM_DRILL
	g_cDrillHomed = HOME_NOT_DONE;	//an external system
#endif
#ifdef HOMESYSTEM_CLAMP
	g_cNACClampHomed = HOME_NOT_DONE;	//an external system
#endif
#ifdef HOMESYSTEM_AAXIS
	g_cNACAAxisHomed = HOME_NOT_DONE;	//an external system
#endif
#ifdef HOMESYSTEM_FASTENER
	g_cFastenerHomed = HOME_NOT_DONE;	//an external system
#endif
#ifdef HOMESYSTEM_FASTENERTRAY
	g_cFastenerTrayHomed = HOME_NOT_DONE;	//an external system
#endif
}

void ClearSentHomeStatus()
{
//Clear all sent home status so that it will resend status
#ifdef HOMESYSTEM_X
	g_cSentHomedX = 0xFF;
#endif
#ifdef HOMESYSTEM_Y
	g_cSentHomedY = 0xFF;
#endif
#ifdef HOMESYSTEM_DRILL
	g_cSentDrillHomed = 0xFF;
#endif
#ifdef HOMESYSTEM_CLAMP
	g_cSentNACClampHomed = 0xFF;
#endif
#ifdef HOMESYSTEM_AAXIS
	g_cSentNACAAxisHomed = 0xFF;
#endif
#ifdef HOMESYSTEM_FASTENER
	g_cSentFastenerHomed = 0xFF;	//an external system
#endif
#ifdef HOMESYSTEM_FASTENERTRAY
	g_cSentFastenerTrayHomed = 0xFF;	//an external system
#endif
}

void SetAllHomeStatusPending()
{
//Set status to pending, but only for an Axis that is part of the main homing sequence X_LINK and Y_LINK, but not _X and _Y
#ifdef HOMESYSTEM_X_LINK
	g_cHomedX = HOME_PENDING;	//X Axis
#endif
#ifdef HOMESYSTEM_Y_LINK
	g_cHomedY = HOME_PENDING;	//Y Axis
#endif
#ifdef HOMESYSTEM_DRILL
	g_cDrillHomed = HOME_PENDING;	//an external system
#endif
#ifdef HOMESYSTEM_CLAMP
	g_cNACClampHomed = HOME_PENDING;	//an external system
#endif
#ifdef HOMESYSTEM_AAXIS
	g_cNACAAxisHomed = HOME_PENDING;	//an external system
#endif
#ifdef HOMESYSTEM_FASTENER
	g_cFastenerHomed = HOME_PENDING;	//an external system
#endif
#ifdef HOMESYSTEM_FASTENERTRAY
	g_cFastenerTrayHomed = HOME_PENDING;	//an external system
#endif
}

void AlertHomeStatus()
{
//Send back only Main Home Status
	SmartToolMsgChar(0, STP_ALERT, MINIFT_OID_HOME, g_cHomed);
	return;
}

void AlertHomeReport(byte axis_code, byte status, byte status_reason)
{
	//Send back Axis Specific Home Report.
	td_oid_home_report oid_home_report;
	oid_home_report.caxis_code = axis_code;
	oid_home_report.cstatus = status;
	oid_home_report.cstatus_reason = status_reason;
	SmartToolMsg(0, STP_ALERT, MINIFT_OID_HOME_REPORT, sizeof(td_oid_home_report), (char*) &oid_home_report);
	return;
}
#endif

void SelectHomePositionAndPositionLimits()
{
	if (g_ConfigData.cToolFlip != Y_NEG)
	{
		g_fHomePositionX = g_ConfigData.HomePosnYPos.fX;
		g_fHomePositionY = g_ConfigData.HomePosnYPos.fY;
		g_fPositionMinX = g_ConfigData.PosnLimitYPos.fMinX;
		g_fPositionMaxX = g_ConfigData.PosnLimitYPos.fMaxX;
		g_fPositionMinY = g_ConfigData.PosnLimitYPos.fMinY;
		g_fPositionMaxY = g_ConfigData.PosnLimitYPos.fMaxY;
	}
	else
	{
		g_fHomePositionX = g_ConfigData.HomePosnYNeg.fX;
		g_fHomePositionY = g_ConfigData.HomePosnYNeg.fY;
		g_fPositionMinX = g_ConfigData.PosnLimitYNeg.fMinX;
		g_fPositionMaxX = g_ConfigData.PosnLimitYNeg.fMaxX;
		g_fPositionMinY = g_ConfigData.PosnLimitYNeg.fMinY;
		g_fPositionMaxY = g_ConfigData.PosnLimitYNeg.fMaxY;
	}
}

#ifdef SEAL_SYSTEM
int LookupDiameterPrimeDelay(float fdiameter)
{
	float fdiameters[5];
	int idelays[5];
	int idelay;
	int i;

//Config must be in order, but ending rows not needed can be zero.
//Starting rows should not be zero.
//When the first row with idelay=0 is seen it will stop and not consider past it.

	fdiameters[0]=g_ConfigData.PrimeDelay.fdiameter1;
	fdiameters[1]=g_ConfigData.PrimeDelay.fdiameter2;
	fdiameters[2]=g_ConfigData.PrimeDelay.fdiameter3;
	fdiameters[3]=g_ConfigData.PrimeDelay.fdiameter4;
	fdiameters[4]=g_ConfigData.PrimeDelay.fdiameter5;
	idelays[0]=g_ConfigData.PrimeDelay.idelay1;
	idelays[1]=g_ConfigData.PrimeDelay.idelay2;
	idelays[2]=g_ConfigData.PrimeDelay.idelay3;
	idelays[3]=g_ConfigData.PrimeDelay.idelay4;
	idelays[4]=g_ConfigData.PrimeDelay.idelay5;

	idelay=0;
	i=0;
	while(i<5)
	{
		idelay = idelays[i];
		if (idelay==0)
		{
			//reach the end, so break
			break;
		}
#ifdef OUTPUT_PRIMEDELAY
		logf("idelay = %d\r\n",idelay);
#endif
		if (fdiameter < fdiameters[i])
		{
#ifdef OUTPUT_PRIMEDELAY
			logf("break on fdiameters[i]=%f\r\n",fdiameters[i]);
#endif
			break;
		}
		if (i==4)
		{
			//this is the last one possible
#ifdef OUTPUT_PRIMEDELAY
			logf("break on i==4\r\n");
#endif
			break;
		}
		if (fdiameter < ((fdiameters[i]+fdiameters[i+1])/2))
		{
#ifdef OUTPUT_PRIMEDELAY
			logf("break on avg(fdiameters[i],fdiameters[i+1])=%f\r\n",((fdiameters[i]+fdiameters[i+1])/2));
#endif
			break;
		}
		i++;
	}
	return idelay;
}

#endif

////////////////////////////////////////////////////////////////////////////////
// Main
////////////////////////////////////////////////////////////////////////////////
int main(void)
{
	//int i;
	uint32 uiTime;
	//char * p;

	MiniFTIOInit();

	//JLIME change
	ClearShowAna();

#ifdef SOCKETCONSOLE
	InitSocketConsole();
	InitListenSocketConsole();

//If Debugging, wait 15 seconds or until we have a SocketConsole
//logf("Waiting up to 15 seconds for SocketConsole\r\n");
//SocketConsoleWaitConnect(15000);
#endif
	logf(SMARTTOOL_SUBTYPE_STRING " " SYSTEM_VERSION_STRING);
	logf("\r\n");

	//FIXME PORTMED move these it init
//light on indicates booting
#ifdef LEDSYSTEM
	g_cLED = 1;
	digOut(DO_LIGHT_GREEN, 1);
#endif

//beeper is off
#ifdef BEEPSYSTEM
	digOut(DO_BEEPER_1, 0);
#endif
	InitVars();
	MiniFTInitConfig();

#ifdef FORCELIMITING
	ReadForceInitMem();
#endif

	initCopyHexToByte();

	InitADC();

	LoadConfigFile();

	//Now that Config is loaded setup some boot time defaults
	g_cEEOption = g_ConfigData.cEEOptionDefault;
	g_cProbeMethod = g_ConfigData.cProbeMethodDefault;
#ifndef LINEAR_SCALE_OPTION
	g_ConfigData.cScaleMode=0; //force to zero so pulling config will be correct.  on a machine that doesn't support scalemode>0
#endif

//FIXME MED    find more ways to protect boot flip... rename to boot???
#ifdef ORIENTATION_SENSORS
//do not let this come from config... start unknown
	g_ConfigData.cToolFlip = Y_UNKNOWN;
#endif
//Select the proper position limits
	SelectHomePositionAndPositionLimits();

#ifdef TOOL_IN_RAM
	LoadToolFromRam();
#endif

	//Init Motion Control System : Main Init Call for connection and control.
	MiniFTWhistleInit();

	STPInit((char *) SmartToolTypeMiniFT, &g_MiniFTSmartTool, &g_ToolManagementSmartTool);

#ifdef HD_RAIL_STP
	InitRailSTP();
#endif

//now turn off light... soon the main loop will take control of the light
#ifdef LEDSYSTEM
	g_cLED = 0;
	digOut(DO_LIGHT_GREEN, 0);
#endif

//Do Boot Beep after this point (config may have beep off)
	BeepBoot()
	;

//TestFloatResP10(100.0);
//TestFloatResP10(-100.0);

#ifdef SHOW_MEM
	show_memory();
#endif

#ifdef USE_RFID_OMRON
//RFID Open and configure the serial port.
	rfidOpenAndConfigPort();
	rfidInitStateAndData();
#endif
#ifdef USE_RFID_MIFARE
	SL031Init();
#endif

	/////TEMPORARY CODE
	STATSInit();

	//JLIME Angle sensor
	PrecalcAngleSensor();

	//MAIN SYSTEM LOOP
	uiTime = MS_TIMER;
	while (g_cSmartToolShutdown == 0)
	{
		/////TEMPORARY CODE
		STATSCount();
		STATSStart(); ///TEMPCODE

#ifdef SOCKETCONSOLE
		CheckSocketConsole();
		RxSocketConsole();
		if (g_cHold)
		{
			goto show_io;
		}
#endif
		//RUN_DIO
		ReadDigitalInputs(); //	Duration: 100 usec average, 120 usec worst case.

		DigOutService(); //Duration New version not timed... est < 75 usec average

		//RUN_ADC
		ReadADC();			//	Duration typical 45 usec, 150 usec worst case.
#ifdef FORCELIMITING
		ReadForceSensor();
#endif
		STATSEnd(1);			///TEMPCODE
		STATSStart();			///TEMPCODE

		//MOTION CONTROL SERVICE
		//Syscheck, Communication, and Monitor all in one location.
		MiniFTWhistleService();

		//Second call to Read DIO
		ReadDigitalInputs();							//	Duration: 100 usec average, 120 usec worst case.

		STATSEnd(2);							///TEMPCODE
		STATSStart();							///TEMPCODE
		//STP CHECK AND RX
		uiTime = MS_TIMER - uiTime;
		if (uiTime > 500)
		{
			logf("Non Eth=%u\r\n", uiTime);
		}

		ServiceSTP();

		STATSEnd(3);		///TEMPCODE
		STATSStart();		///TEMPCODE
		uiTime = MS_TIMER;

#ifdef HD_RAIL_STP
		ServiceRailSTP();
#endif
		STATSEnd(4);		///TEMPCODE
		STATSStart();		///TEMPCODE

#ifdef DIRECT_LASER_SENSOR_CONNECT
		ServiceLaserSensor();
#endif
#ifdef CENTERVISION_CAM
//FIXME PORTFATAL  Add Cam connection back in....
//		ServiceCam();
#endif
		STATSEnd(5);		///TEMPCODE
		STATSStart();		///TEMPCODE
		//Handle Mode Specific Ops, Control State Machines, and other inlines services
		ModeStateMachine();  //	Duration: Minimum 220 usec; 420 usec in E-STOP
							 //	500 usec if active; Worst case ~ 1.5 msec.

		DFModeStateMachine();

		show_io:
		if (g_cShowIO == 1)
		{
			ShowIO();
		}
		if (g_cShowAna == 1)
		{
			ShowAna();
		}
		STATSEnd(6);							 ///TEMPCODE
	} //continue loop

	//Shutdown was Activated

	MCStopAndFailMove();

	STPShutdown();
	SocketConsoleShutdown();
	exit(0);
}

void ServiceNow()
{
	int i;
//call to debounce
	ReadDigitalInputs();	//	Duration: 100 usec average, 120 usec worst case.
	MiniFTWhistleService();
	ServiceSTP();

#ifdef SOCKETCONSOLE
	CheckSocketConsole();
	RxSocketConsole();
#endif
}

////////////////////////////////////////////////////////////////////////////
// Part Program Loading / Parsing functions
////////////////////////////////////////////////////////////////////////////

//ClearPartProgram
//Load the Part Program structures with empty values
void ClearPartProgram()
{
	int i;
	g_iHoleCount = 0;
	g_PartPgmInfo.iNumVisited = 0;
	g_PartPgmInfo.cLocked = 1; //leave it locked until they clear from the teach screen
	g_PartPgmInfo.cTeachModeData = 0;
	g_PartPgmInfo.cMiniFtFormat = 0;
	g_PartPgmInfo.cChecksum = 0;
	memset(g_PartPgmInfo.p_cChecksum16, 0, 16);
	g_cPartPgmStatus = PP_NOTLOADED;
	g_PartPgmInfo.cErrorCount = 0; //reset error count

//make sure that storage is present

	g_cToolCount = 0; //clear
	g_cProcessCount = 0;
	g_cFastenerCount = 0;
	g_cPatternCount = 0;
	g_iHoleCount = 0;
	g_cKHoleCount = 0;

	g_cRequireParameterRecognition = 0; //clear options

	g_cKHoleHoleIndexSet = 0;

//erase the reference to the data
	g_pcToolNames = 0;
	g_uiToolNamesLen = 0;

//erase the reference to the data
	g_pcProcessLayerDataBase = 0;
	g_pcProbeCommandDataBase = 0;

//Program Options
	g_fAssumeApproxDataSetOrientation = -1; //-1 means it's not in use
	g_cAssumeApproxPositionsFromDataset = 0;
	g_cRequireParameterRecognition = 0;

//ClearGolbalRotation(); //just clear two variables
	g_cRotationKP = 0;
	g_cRotationKS = 0;
	g_cRotationContext = 0;

//Clear Probe Completely
	InitProbeValues(); //does not send probe status back. this happens later.

//also reset any job parameters
	StartOver(); //SPSALL

//And reset any which are sticky across jobs, but not programs
//FIXME medium : should expand on this concept making it more fool proof;

//	For rivet programs only - just to make sure values are not crazy if the data is missing from the part program:
	g_PartPgmInfo.K1OriginDistX = 0; // in inches, X offset of K1 from track origin bumper
	g_PartPgmInfo.K1EndDistX = 10.0; // in inches, X offset of K1 from track end bumper
	g_PartPgmInfo.K1OriginDistY = 0; // in inches, Y offset of K1 from center of X track

//Clear Scaling
	g_cScaleBasedOnProbe = 0; //And Turn Off Scaling

//Program does not have default tool flip anymore...
//A program either has tool flip settings or it does not adjust what is currently on the tool
//g_ConfigData.cToolFlip = Y_POS;

//Clear Options
	g_fAssumeApproxDataSetOrientation = -1; //-1 means it's not in use  0>= x < 360 means it's in use
	g_cAssumeApproxPositionsFromDataset = 0;
	g_cRequireParameterRecognition = 0;

//Clear Lube Bypass
	g_cLubeBypass = 0;

//Stop SendPPData
	g_cSendPPDataSessions = 0;
//Clear PP Data
	g_uiPartProgramDataLength = 0;

	g_cBlockCount = 0;
}

void ParsePartProgramStart()
{
	byte cSuccess;
	char *s;
	g_cPartPgmStatus = PP_LOADING;

//init stats
// COUNTING ASSUMPTION: Input file enums have enum id in order starting at 0, and will use a range of numbers,
//						such that nothing will be skipped from 0 to N-1, where there are N enums.
//						This is the only possible output from the PPparser.pl

//first clear
	ClearPartProgram();

	if (g_PartPgmInfo.cErrorCount > 0)
	{
		//If the clear didn't even have zero errors, then return here.
		g_cPartPgmStatus = PP_NOTLOADED;
		goto return_partpgmstatus;
	}

	cSuccess = 1;
	if (g_sPPDirect == 0)
	{
		//we only support direct data now, but keep this error as-is for now
		g_cPartPgmStatus = PP_NOTLOADED;
		LogParseError(MINIFTMC_PPERR_FAILURE_TO_OPEN_FILE);
		goto return_partpgmstatus;
	}
	logf("file %s\r\n", g_szPartPgmFilename);    //debug

//Set Parse Section
	g_cBeingParsed = PARSE_SECTION;

//Now Ready To Read The File In
//The main loop will see g_cPartPgmStatus=PP_LOADING, and will repeat calls the the next function
	g_cPartPgmStatus = PP_LOADING;
	logf("Loading\r\n");
	return_partpgmstatus:
	SmartToolMsgChar(0, STP_ALERT, MINIFT_OID_PARTPGM_STATUS, g_cPartPgmStatus);
}

//ParsePartProgramContinue
// cMaxLinesPerContinue
//   X  the number of lines to read each time the continue is called... as a byte type max is 255
//   0  allow all the lines to be parsed right now.... same as blocking version
void ParsePartProgramContinue(byte cMaxLinesPerContinue)
{
//local vars
	char *s;
	float fx, fy, fd;
	byte cKIndex;
	byte cPKIndex;
	byte cSKIndex;
	byte cPattern;
	byte cToolFlip;
	byte cToolType;
	byte cProcess;
	byte cFastener;
	uint16 d2uDiameter;
	uint16 uiOps;
	uint16 uiStack;
	int icsnkadj;
	int i;
	uint16 ui;
	uint16 uiRemain;
	uint32 ul;
	uint32 ulRemain;
	char *p_c;
	char tempproc[64];
	byte tempproclen;
	byte c;
	byte cl;

	int iStartPos;
	byte cLoopNow;

	static uint16 s_uiSBN;
	static uint16 s_uiSB;
	static uint32 s_ulSBN;
	static uint32 s_ulSB;

	static byte s_cKHoleIndexRowSize;
	//static byte s_cToolRowSize;
	static byte s_cProcessRowSize;
	static byte s_cHoleRowSize;
	static byte s_cKHoleRowSize;
	static byte s_cMaterialStackRowSize;
	static byte s_cMaxMaterialLayers;
	static byte s_cBytesOpField;
	static uint32 s_uiP3DIV;

	static byte s_cCount;
	static uint16 s_uiCount;

	//td_tool_data_fileformat * p_tool_data;
	td_process_data * p_process_data;
	td_hole_data_fileformat * p_hole_data_f;

	td_HoleData xHoleData;
	int ihole;

	static byte s_cKHoleIndex;
	byte pbChecksumbuffer[16];

	if (g_cPartPgmStatus != PP_LOADING)
	{
		//should not be here and may not be prepared to parse, so return
		return;
	}

//   	szCurLine=szCurLineBuffer;

	if (g_sPPDirect == 0)
	{
		logf("ERROR:g_sPPDirect=0\r\n");
	}

	loop_now: iStartPos = g_iPPDirectPos;
	cLoopNow = 0;
	while (1)
	{
		//reading new format

		//continue to parse the file
		s = g_sPPDirect + g_iPPDirectPos; //The current buffer
		i = g_iPPDirectEnd - g_iPPDirectPos;
		switch (g_cBeingParsed)
		{
		case PARSE_SECTION:
			//looking for a section header.
			//FIXME0000 consider requirement of section order but for now: parsing this way
			if (i < 2)
			{
				break; //come back when more data to process.
			}
			g_iPPDirectPos += 2;
			s_uiSBN = 0;
			s_uiSB = 0;
			s_cCount = 0;
			s_uiCount = 0;
			s_ulSBN = 0;
			s_ulSB = 0;
			logf("ps: %d %d\r\n", s[0], s[1]); //FIXME00000000 test
			c = s[0];
			if (c != 0)
			{
				logf("Warning: Section Header MSB is not 0\r\n");
			}
			c = s[1];
			g_cBeingParsed = c; //state is the same as the section code
			if (g_PartPgmInfo.cMiniFtFormat == 0)
			{
				if (c != SECTION_Header)
				{
					//Header Must be 1st...
					//This could also be the wrong format
					//Assume it's the old format
					g_PartPgmInfo.cMiniFtFormat = 1;
					LogParseError(MINIFTMC_PPERR_NOT_MINIFT_FORMAT);
					g_cBeingParsed = PARSE_FAIL;
					continue; //will not return to any states other than FAIL
				}
				//Follows Binary Format Rules of having header 1st
				g_PartPgmInfo.cMiniFtFormat = 2;
				//Clear these static variables
				s_cKHoleIndexRowSize = 0;
				//s_cToolRowSize = 0;
				s_cProcessRowSize = 0;
				s_cHoleRowSize = 0;
				s_cKHoleRowSize = 0;
				//preserve the file so far in checksum in case it's needed
				g_PartPgmInfo.p_cChecksum16[0] = 0; //Header section
				g_PartPgmInfo.p_cChecksum16[1] = 1; //Header section
			}
			if (c == SECTION_Invalid)
			{
				//FIXME MED HIGH  easy - check what should do with unknown section forward compatible ideas...
				LogParseError(MINIFTMC_PPERR_UNKNOWN_SECTION);
				g_cBeingParsed = PARSE_FAIL;
				continue; //will not return to any states other than FAIL
			}
			continue;
//FIXME MED LOW Consider that this funtion never breaks until it reads it's full buffer.....
//  Should it break more to allow faster main loop??????
// This will depends on how long the code takes for normal programs.
		case SECTION_Header:
			if (s_uiSBN == 0)
			{
				if (i < 2)
				{
					break; //come back when more data to process.
				}
				g_iPPDirectPos += 2;
				s_uiSBN = *((int16 *) s);
				//And For Header, copy the 2 size bytes into the checksum buffer for init
				g_PartPgmInfo.p_cChecksum16[2] = s[0]; //
				g_PartPgmInfo.p_cChecksum16[3] = s[1]; //
				//SPECIAL PATTERN
				//Ensure that this data is processed now before content starts.
				//Since Checksum is not on yet it will ignore the 1st 4 characters, but we saved them
				//above so we can use them later.
				//The data will be writen out to the PPData storage buffer.
				cLoopNow = 1;
				break;
			}
			if (i < s_uiSBN)
			{
				break; //come back when more data to process.
			}
			logf("%s Hv %ub\r\n", "H", s_uiSBN);
			g_iPPDirectPos += s_uiSBN;

			c = s[0]; //major version
			if (c != 0x01)
			{
				goto minift_format_version_fail;
			}
			c = s[1]; //minor version
			g_cFormatMinorVersion = c;
			if (c < 0x02)
			{
				minift_format_version_fail:
				logf("Ver Fail:%d\r\n", c);
				LogParseError(MINIFTMC_PPERR_NOT_MINIFT_FORMAT);
				g_cBeingParsed = PARSE_FAIL;
				continue; //will not return to any states other than FAIL
			}
			c = s[2];
			g_PartPgmInfo.cChecksum = c;
			if (c == 0)
			{
				//ignore it: This is not recommended, but we support this.
			}
#ifdef MD5HASH
			else if (c == 1)
			{
				//MD5
				MD5_Init(&g_md5_context);

				//We stored the 1st 4 bytes into this buffer.
				//before processing the section content.
				//Add them to the checksum now.
				MD5_Update(&g_md5_context, g_PartPgmInfo.p_cChecksum16, 4);
				//The rest of the data will be added the normal way when it leaves
				//the function and processes all the data waiting
			}
#else
#warning "MD5HASH is not defined: There can be no checking of MD5"
#endif
#ifdef MDARC4
			else if (c==2)
			{
				//FIXME LOW build MDARC4
				LogParseError(MINIFTMC_PPERR_UNSUPPORTED_CHECKSUM_TYPE);
			}
#endif
			else
			{
#ifndef BYPASS_MD5_MUST_MATCH
				//a Kind of checksum that is not currently supported
				LogParseError(MINIFTMC_PPERR_UNSUPPORTED_CHECKSUM_TYPE);
#endif
			}

			//Parse more...

			c = s[3]; //Char Encoding
			if (c == 1) //ASCII
			{
			}
			else if (c == 2) //UTF-8
			{
				logf("!UTF-8 not tested");
			}
			else
			{
				LogParseError(MINIFTMC_PPERR_UNSUPPORTED_CHAR_ENCODING);
			}

			//just print
			c = s[s_uiSBN];
			s[s_uiSBN] = 0;
			logf("text = \"%s\"\r\n", s + 4);
			s[s_uiSBN] = c; //restore buffer...

			g_cBeingParsed = PARSE_SECTION;
			continue;

		case SECTION_Options:
			if (s_uiSBN == 0)
			{
				if (i < 2)
				{
					break; //come back when more data to process.
				}
				g_iPPDirectPos += 2;
				s_uiSBN = *((int16 *) s);
				if (s_uiSBN == 0)
				{
					g_cBeingParsed = PARSE_SECTION;
				}
				continue;
			}
			if (i < s_uiSBN)
			{
				break; //come back
			}

			logf("%s Hv %ub\r\n", "op", s_uiSBN);
			g_iPPDirectPos += s_uiSBN;

			if (g_cFormatMinorVersion >= 3) //for version 1.3 and up using binary option format
			{
				//Parse more...
				ui = 0;
				while (ui < s_uiSBN)
				{
					fx = 0;
					fy = 0;
					c = s[ui++];
					cl = s[ui++];
					if (c == 0)
					{
						break;
					}
					if (c == 1)
					{
						//AssumeApproxDataSetOrientation fd
						fd = *((float *) (s + ui));
						ui += 4;
						if (fd < 0)
						{
							fd += 360;
						}
						if (fd < 0 || fd >= 360)
						{
							fd = 0;
						}
						g_fAssumeApproxDataSetOrientation = fd;
						fx = fd;
						goto show_sbo;
						continue;
					}
					if (c == 2)
					{
						//KHoleApproxPosition cindex,fx,fy
						cl = s[ui++]; //use this byte for cindex
						fx = *((float *) (s + ui));
						ui += 4;
						fy = *((float *) (s + ui));
						ui += 4;
						g_cKHolePrbeStart[cl] = PS_APPROXIMATE;
						g_cKHolePrbeStatus[cl] = PS_APPROXIMATE;
						g_cKHolePrbeStatusWarnings[cl] = 0; //clear warning
						g_fKHolePrbeStartX[cl] = fx;
						g_fKHolePrbeStartY[cl] = fy;
						g_fKHolePrbeX[cl] = fx;
						g_fKHolePrbeY[cl] = fy;
//FIXMENOW test
						goto show_sbo;
						continue;
					}
					if (c == 3)
					{
						//AssumeApproxPositionsFromDataset
						g_cAssumeApproxPositionsFromDataset = (s[ui] == '1');
						ui++;
//FIXMENOW test
						goto show_sbo;
						continue;
					}
					if (c == 4)
					{
						//DefaultSelectedOperations uint16
						uiOps = *((uint16 *) (s + ui));
						ui += 2;
						fx = uiOps; //show this variable
						g_ConfigData.uiProcessOperations = uiOps; //set this
						SmartToolMsgUInt16(0, STP_ALERT, MINIFT_OID_PROCESS_OPERATIONS, uiOps);
						goto show_sbo;
						continue;
					}
					if (c == 5)
					{
						//ProgramInspectionInterval uint16
						uiOps = *((uint16 *) (s + ui));
						ui += 2;
						if (uiOps > 250)
						{
							uiOps = 250;
						}
						g_ConfigData.cVisionAutoInspect = uiOps;
//FIXME
//Integrate into existing OID and test that OID....
						fx = uiOps; //show this variable
						goto show_sbo;
						continue;
					}
					if (c == 6)
					{
						//RequireParameterRecognition
						g_cRequireParameterRecognition = (s[ui] == '1');
						ui++;
						goto show_sbo;
						continue;
					}
					if (c == 7)
					{
						//KHoleApproxRailPosition cindex,fx,fy
						cl = s[ui++]; //use this byte for cindex
						fx = *((float *) (s + ui));
						ui += 4;
						fy = *((float *) (s + ui));
						ui += 4;

						if (g_cHomedX != HOME_DONE || g_cHomedY != HOME_DONE)
						{
							//just log this error
							LogParseError(MINIFTMC_PPERR_XY_NOT_HOMED);
						}
						else
						{
#ifdef HOMESYSTEM_X_RFID
							//Convert fx from Rail to machine coordinates
							fx = g_fRFIDMachineX + g_iRFIDRailOrientation * (fx - g_fRFIDRailX);
#else
							//just allow Home Coordinates to be Equal to rail coordinates
#endif

							g_cKHolePrbeStart[cl] = PS_APPROXIMATE;
							g_cKHolePrbeStatus[cl] = PS_APPROXIMATE;
							g_cKHolePrbeStatusWarnings[cl] = 0; //clear warning
							g_fKHolePrbeStartX[cl] = fx; //use RFID
							g_fKHolePrbeStartY[cl] = fy; //after Y Home, Y machine position is the rail position
							g_fKHolePrbeX[cl] = fx;
							g_fKHolePrbeY[cl] = fy;
						}
//FIXMENOW test
						goto show_sbo;
						continue;
					}
					//unrecognized option...
					ui += cl; //move forward the length of the data
					show_sbo:
					logf("SBO %d %d %f %f\r\n", c, cl, fx, fy);
					if (g_cRequireParameterRecognition)
					{
						//just log this error
						LogParseError(MINIFTMC_PPERR_UNKNOWN_PARAMETER);
					}
					continue;
				}
			}

			//OLD VERSION WAS TEXT
			////just print
			//c=s[s_uiSBN];
			//s[s_uiSBN]=0;
			//logf("text = \"%s\"\r\n",s);
			//s[s_uiSBN]=c; //restore buffer...

			g_cBeingParsed = PARSE_SECTION;
			continue;
		case SECTION_Counts:
			if (s_uiSBN == 0)
			{
				if (i < 2)
				{
					break; //come back when more data to process.
				}
				g_iPPDirectPos += 2;
				s_uiSBN = *((int16 *) s);
				if (s_uiSBN == 0)
				{
					g_cBeingParsed = PARSE_SECTION;
				}
				continue;
			}
			if (i < s_uiSBN)
			{
				break; //come back when more data to process.
			}
			logf("%s Hv %ub\r\n", "co", s_uiSBN);
			g_iPPDirectPos += s_uiSBN;
			//use this data
			g_cToolCount = s[0]; //number of tools
			g_cProcessCount = s[1]; //number of processes
			g_cFastenerCount = s[2]; //number of fasteners
			g_cPatternCount = s[3]; //number of patterns
			g_iHoleCount = *((int16 *) (s + 4)); //number of holes
			g_cKHoleCount = s[6]; //number of KHoles
			s_cKHoleIndexRowSize = s[7];
			//s_cToolRowSize=s[8]; //bytes per tooltype data row //v1.1 only... cut out from v1.2+
			s_cProcessRowSize = s[8]; //bytes per process data row
			s_cHoleRowSize = s[9]; //bytes per hole data row
			s_cKHoleRowSize = s[10]; //bytes per khole data row
			s_cMaterialStackRowSize = s[11]; //FIXME Incomplete  Material Stack Not Being Used
			s_cMaxMaterialLayers = s[12];
			s_cBytesOpField = s[13]; //bytes per op field
			s_uiP3DIV = *((uint32*) (s + 14));
			logf("p3div = %u\r\n", s_uiP3DIV);

			if (g_cKHoleCount > MAXNUM_KHOLES)
			{
				//FIXME Minor report diff error....
				LogParseError(MINIFTMC_PPERR_SECTION_SIZE_DIFF);
				g_cBeingParsed = PARSE_FAIL;
				continue; //will not return to any states other than FAIL
			}

			g_cBeingParsed = PARSE_SECTION;
			//	IF we ever need to enforce lower limits than 255 enforce like this
			//               LogParseError(MINIFTMC_PPERR_OVER_FASTENER_LIMIT);
			continue;
		case SECTION_ToolTypeNames:
			if (s_uiSBN == 0)
			{
				if (i < 2)
				{
					break; //come back when more data to process.
				}
				s_uiSBN = *((int16 *) s);
				g_iPPDirectPos += 2;
				if (s_uiSBN == 0)
				{
					g_cBeingParsed = PARSE_SECTION;
				}
				//The Position In the file is the data we have saved, plus how far DirectPos has now moved since iStartPos
				//Anything Prior to iStartPos is included in g_uiPartProgramDataLength because it has been saved.
				g_pcToolNames = g_cPartProgramData + g_uiPartProgramDataLength + g_iPPDirectPos - iStartPos;
				g_uiToolNamesLen = s_uiSBN;
				continue;
			}
			if (i == 0)
			{
				break; //come back when more data to process.
			}
			//copy all of this into a special location up to what we need.
			//copy happens below where the entire input stream is preserved
			uiRemain = s_uiSBN - s_uiSB;
			if (i > uiRemain)
			{
				i = uiRemain;
			}
			//move

			//add them
			s_uiSB += i;
			g_iPPDirectPos += i;

			if (s_uiSB == s_uiSBN)
			{
				//have enough
				logf("%s Hv %ub\r\n", "t", s_uiSBN);
				g_cBeingParsed = PARSE_SECTION;
			}
			continue;
		case SECTION_ProcessNames:
			if (s_uiSBN == 0)
			{
				if (i < 2)
				{
					break; //come back when more data to process.
				}
				s_uiSBN = *((int16 *) s);
				g_iPPDirectPos += 2;
				if (s_uiSBN == 0)
				{
					g_cBeingParsed = PARSE_SECTION;
				}
				continue;
			}
			if (i == 0)
			{
				break; //come back when more data to process.
			}

			//copy all of this into a special location up to what we need.

			uiRemain = s_uiSBN - s_uiSB;
			if (i > uiRemain)
			{
				i = uiRemain;
			}

			//move

			//add them
			s_uiSB += i;
			g_iPPDirectPos += i;

			if (s_uiSB == s_uiSBN)
			{
				//have enough
				logf("%s Hv %ub\r\n", "p", s_uiSBN);
				g_cBeingParsed = PARSE_SECTION;
			}
			continue;
		case SECTION_FastenerTypeNames:
			if (s_uiSBN == 0)
			{
				if (i < 2)
				{
					break; //come back when more data to process.
				}
				s_uiSBN = *((int16 *) s);
				g_iPPDirectPos += 2;
				if (s_uiSBN == 0)
				{
					g_cBeingParsed = PARSE_SECTION;
				}
				continue;
			}
			if (i == 0)
			{
				break; //come back when more data to process.
			}
			//copy all of this into a special location up to what we need.
			//copy happens below where the entire input stream is preserved
			uiRemain = s_uiSBN - s_uiSB;
			if (i > uiRemain)
			{
				i = uiRemain;
			}
			//move

			//add them
			s_uiSB += i;
			g_iPPDirectPos += i;

			if (s_uiSB == s_uiSBN)
			{
				//have enough
				logf("%s Hv %ub\r\n", "fa", s_uiSBN);
				g_cBeingParsed = PARSE_SECTION;
			}
			continue;
		case SECTION_PatternNames:
			if (s_uiSBN == 0)
			{
				if (i < 2)
				{
					break; //come back when more data to process.
				}
				s_uiSBN = *((int16 *) s);
				g_iPPDirectPos += 2;
				if (s_uiSBN == 0)
				{
					g_cBeingParsed = PARSE_SECTION;
				}
				continue;
			}
			if (i == 0)
			{
				break; //come back when more data to process.
			}
			//copy all of this into a special location up to what we need.
			//copy happens below where the entire input stream is preserved
			uiRemain = s_uiSBN - s_uiSB;
			if (i > uiRemain)
			{
				i = uiRemain;
			}
			//move

			//add them
			s_uiSB += i;
			g_iPPDirectPos += i;

			if (s_uiSB == s_uiSBN)
			{
				//have enough
				logf("%s Hv %ub\r\n", "pat", s_uiSBN);
				g_cBeingParsed = PARSE_SECTION;
			}
			continue;
		case SECTION_HoleNames:
			if (s_ulSBN == 0)
			{
				if (i < 4)
				{
					break; //come back when more data to process.
				}
				s_ulSBN = *((uint32 *) s);
				g_iPPDirectPos += 4;
				if (s_ulSBN == 0)
				{
					g_cBeingParsed = PARSE_SECTION;
				}
				continue;
			}
			if (i == 0)
			{
				break; //come back when more data to process.
			}
			//copy all of this into a special location up to what we need.
			//copy happens below where the entire input stream is preserved
			ulRemain = s_ulSBN - s_ulSB;
			if (i > ulRemain)
			{
				i = (int) ulRemain;
			}
			//move

			//add them
			s_ulSB += i;
			g_iPPDirectPos += i;

			if (s_ulSB == s_ulSBN)
			{
				//have enough
				logf("%s Have all %u bytes.\r\n", "h", s_ulSBN);
				g_cBeingParsed = PARSE_SECTION;
			}
			continue;
		case SECTION_KHoleIndex:
			if (s_uiSBN == 0)
			{
				if (i < 2)
				{
					break; //come back when more data to process.
				}
				s_uiSBN = *((int16 *) s);
				s_uiSB = g_cKHoleCount * s_cKHoleIndexRowSize;
				g_iPPDirectPos += 2;
				if (s_uiSB != s_uiSBN)
				{
					logf("Bad len %s", "khi");
					LogParseError(MINIFTMC_PPERR_SECTION_SIZE_DIFF);
					g_cBeingParsed = PARSE_FAIL;
					continue; //will not return to any states other than FAIL
				}
				//s_uiSB = 0; //no need
				g_cKHoleHoleIndexSet = 1;
				g_iKHoleHoleIndex[0] = 0; //clear that first unused entery
				if (g_cKHoleCount == 0)
				{
					g_cBeingParsed = PARSE_SECTION;
				}
				continue;
			}
			if (i < 2)
			{
				break; //come back
			}
			//Ignore this data... just let it write into the stored data section
			//add bytes
			//s_uiSB += s_cKHoleRowSize; no need
			g_iPPDirectPos += 2;

			//record index
			c = s_cCount + 1;
			ihole = *((int16 *) s);
			logf("k %d %d\r\n", c, ihole);
			//save this to the index
			g_iKHoleHoleIndex[c] = ihole;

			//Count
			s_cCount++;
			if (s_cCount == g_cKHoleCount)
			{
				logf("All %s\r\n", "khi");
				g_cBeingParsed = PARSE_SECTION;
			}
			continue;
		case SECTION_ProcessData:
			if (s_uiSBN == 0)
			{
				if (i < 2)
				{
					break; //come back when more data to process.
				}
				s_uiSBN = *((int16 *) s);
				s_uiSB = g_cProcessCount * s_cProcessRowSize;
				g_iPPDirectPos += 2;
				if (s_uiSB != s_uiSBN)
				{
					logf("Bad len %s", "pd");
					LogParseError(MINIFTMC_PPERR_SECTION_SIZE_DIFF);
					g_cBeingParsed = PARSE_FAIL;
					continue; //will not return to any states other than FAIL
				}
				//s_uiSB = 0; no need
				if (g_cProcessCount == 0)
				{
					g_cBeingParsed = PARSE_SECTION;
				}
				continue;
			}
			if (i < s_cProcessRowSize)
			{
				break; //come back
			}
			//read process
			p_process_data = (td_process_data *) s;

			c = s_cCount + 1;

			logf("proc%d l=%d m=%d lbs=%d ops=%d depth=%d\r\n", c,
					p_process_data->layers, p_process_data->material,
					p_process_data->clamplbs, p_process_data->ops,
					p_process_data->countersinkDepth);

			logf(" pl=%d w=%d a=%d hsa=%d\r\n", p_process_data->proclayers,
					p_process_data->clampwarnlbs, p_process_data->clampalarmlbs,
					p_process_data->hardstopamps); //Times 100 for 2 digits of decimal
			logf(" ps %d \r\n", p_process_data->procstyle);

			if (c < MAXNUM_PROCESSES)
			{
				g_cProcessLayerCount[c] = p_process_data->layers;
				g_cProcessMaterials[c] = p_process_data->material;
				g_uiProcessPounds[c] = p_process_data->clamplbs;
				g_cProcessOps[c] = p_process_data->ops;
				g_uiProcessCountersink[c] = p_process_data->countersinkDepth;
				g_cProcessProcLayerCount[c] = p_process_data->proclayers;
				g_uiProcessPoundsWarning[c] = p_process_data->clampwarnlbs;
				g_uiProcessPoundsAbort[c] = p_process_data->clampalarmlbs;
				g_uiProcessHardstopAmps[c] = p_process_data->hardstopamps;
				g_cProcessStyle[c] = p_process_data->procstyle;
			}
//FIXME00000000000000000000
			//Should just check to see that sections are all present

			//add bytes
			//s_uiSB += s_cProcessRowSize; no need
			g_iPPDirectPos += s_cProcessRowSize;
			//Count
			s_cCount++;
			if (s_cCount == g_cProcessCount)
			{
				logf("All %s\r\n", "pd");
				g_cBeingParsed = PARSE_SECTION;
			}
			continue;
		case SECTION_ProcessLayerData:
			case SECTION_ProcessLayerData_Alt:
			if (s_uiSBN == 0)
			{
				if (i < 2)
				{
					break; //come back when more data to process.
				}
				s_uiSBN = *((int16 *) s);
				logf("pld SBN %u\r\n", s_uiSBN);
				g_iPPDirectPos += 2;
				if (s_uiSBN == 0)
				{
					g_cBeingParsed = PARSE_SECTION;
				}
				continue;
			}
			if (i == 0)
			{
				break; //come back when more data to process.
			}
			if (s_uiSB == 0)
			{
				//still working on 1st part
				if (i < g_cProcessCount * 2 + 4)
				{
					//don't have enough for map yet
					break;
				}
				i = g_cProcessCount * 2 + 4;
				//logf("copy i=%d\r\n",i);
				//copy entire map
				memcpy(((char*) g_uiProcessLayerDataIndex), (uint16*) s, i);
				//count these bytes as used
				s_uiSB += i;
				g_iPPDirectPos += i;
#define SHOW_PLD
#ifdef SHOW_PLD
				i = 0;
				while (i <= g_cProcessCount + 1) //p0 is just a place holder and the last entery is just so the length of the last processes data can be known.
				{
					logf("p %d ind %d\r\n", i, g_uiProcessLayerDataIndex[i]);
					i++;
				}
#endif
				//The Position In the file is the data we have saved, plus how far DirectPos has now moved since iStartPos
				//Anything Prior to iStartPos is included in g_uiPartProgramDataLength because it has been saved.
				g_pcProcessLayerDataBase = g_cPartProgramData + g_uiPartProgramDataLength + g_iPPDirectPos - iStartPos;
#ifdef SHOW_PLD
				logf("xp_pld=%p\r\n", g_pcProcessLayerDataBase);
#endif
				continue;
			}

			//copy all of this into a special location up to what we need.

			uiRemain = s_uiSBN - s_uiSB;
			logf("pld SBN %u SB %u  rem %u\r\n", s_uiSBN, s_uiSB, uiRemain);
			if (i > uiRemain)
			{
				i = uiRemain;
			}
			//move

			//add them
			s_uiSB += i;
			g_iPPDirectPos += i;

			if (s_uiSB == s_uiSBN)
			{
				//have enough
				logf("%s Hv %ub\r\n", "pld", s_uiSBN);
				g_cBeingParsed = PARSE_SECTION;
			}
			continue;
		case SECTION_HoleData:
			if (s_ulSBN == 0)
			{
				if (i < 4)
				{
					break; //come back when more data to process.
				}
				g_iPPDirectPos += 4;
				s_ulSBN = *((uint32 *) s);
				s_ulSB = ((uint32) g_iHoleCount) * s_cHoleRowSize;
//FIXME Minor clean this temp code size limit of s_cHoleRowSize"
#define USE_TEMP_WARNING_FOR_s_cHoleRowSize
#ifdef USE_TEMP_WARNING_FOR_s_cHoleRowSize
				if (s_cHoleRowSize < 19)
				{
					LogParseError(MINIFTMC_PPERR_SECTION_SIZE_DIFF);
					g_cBeingParsed = PARSE_FAIL;
					continue;
				}
#endif
				if (s_ulSB != s_ulSBN)
				{
					logf("Bad len %s", "hd");
					LogParseError(MINIFTMC_PPERR_SECTION_SIZE_DIFF);
					g_cBeingParsed = PARSE_FAIL;
					continue; //will not return to any states other than FAIL
				}
				//s_ulSB = 0; no need
				if (g_iHoleCount == 0)
				{
					g_cBeingParsed = PARSE_SECTION;
				}
				//make sure index was set
				if (g_cKHoleCount > 0)
				{
					if (g_cKHoleHoleIndexSet == 0)
					{
						LogParseError(MINIFTMC_PPERR_SECTION_SIZE_DIFF); //not a specific error for this
						g_cBeingParsed = PARSE_FAIL;
						continue; //will not return to any states other than FAIL
					}
				}
				//set this counter
				s_cKHoleIndex = 1;
				continue;
			}
			if (i < s_cHoleRowSize)
			{
				break; //come back
			}
			//read hole data

			p_hole_data_f = (td_hole_data_fileformat *) s;

			p_c = (char *) &ul;

			//FlatX
			ul = *((uint32 *) &p_hole_data_f->flatx);
			if ((p_c[2] & 128) == 0)
			{
				p_c[3] = 0; //clear MSB....
			}
			else
			{
				p_c[3] = 0xFF; //Clear MSG or sign extended MSB
			}
			fx = (int32) ul; //it is signed....
			fx = fx / s_uiP3DIV;

			//FlatY
			ul = *((uint32 *) &p_hole_data_f->flaty);
			if ((p_c[2] & 128) == 0)
			{
				p_c[3] = 0; //clear MSB....
			}
			else
			{
				p_c[3] = 0xFF; //sign extended
			}
			fy = (int32) ul; //it is signed....
			fy = fy / s_uiP3DIV;

			//cKIndex
			cKIndex = 0;

			//Primary KIndex
			cPKIndex = p_hole_data_f->ki_primary;
			//Secondary KIndex
			cSKIndex = p_hole_data_f->ki_secondary;
			//Diameter
			d2uDiameter = p_hole_data_f->diameter;
			//Tool
			cToolType = p_hole_data_f->tooltype;
			//Process
			cProcess = p_hole_data_f->process;
			//Fastener
			cFastener = p_hole_data_f->fastener;
			//Pattern
			cPattern = p_hole_data_f->pattern;
			//Operations
			uiOps = p_hole_data_f->ops; //FIXME same old width issue
			uiStack = p_hole_data_f->depthstack;
			icsnkadj = p_hole_data_f->countersink_adj;

			if (g_cFormatMinorVersion <= 2) //for version 1.3 and up, they must set cPattern+=128 for K Holes that are not inline
			{
				if (cPKIndex == 0)
				{
					cPattern += 128;
				}
				uiOps |= 1; //always allow access by default.. In 1.3 this bit can prevent hole access.
			}
			//store
			ihole = s_uiCount;

			//FIXME Test test perormance without logfs in this section to see load time effect.
			//Leave Off if not needed
			logf("i%d p%d s%d d%d t%d p%d fas%d pat%d o%d\r\n", ihole, cPKIndex,
					cSKIndex, d2uDiameter, cToolType, cProcess, cFastener,
					cPattern, uiOps);

			//Check If this is a KHole Using the new approach
			if (s_cKHoleIndex <= g_cKHoleCount)
			{
				logf("lk %d ni %d i %d\r\n", s_cKHoleIndex, g_iKHoleHoleIndex[s_cKHoleIndex], ihole);
				if (ihole == g_iKHoleHoleIndex[s_cKHoleIndex])
				{
					logf("fik %d ni %d i %d\r\n", s_cKHoleIndex, g_iKHoleHoleIndex[s_cKHoleIndex], ihole);
					cKIndex = s_cKHoleIndex++;
				}
			}

			//Set special X mem arrays to hold this data.
			if (ihole < MAXNUM_POSNS)
			{
				g_fRawDataX[ihole] = fx;
				g_fRawDataY[ihole] = fy;
				g_fRawDataR[ihole] = 0; //start it at zero.
				xHoleData.cKInd = cKIndex;
				xHoleData.cKPri = cPKIndex;
				xHoleData.cKSec = cSKIndex;
				xHoleData.cTool = cToolType; //FIXME00000 check low end range....
				xHoleData.cProcess = cProcess;
				xHoleData.cFastener = cFastener;
				xHoleData.cPattern = cPattern;
				xHoleData.uiOps = uiOps;
				xHoleData.uiStack = uiStack; //d2u format number... or pointer to stack in stack storage system
				xHoleData.iCountersinkAdjust = icsnkadj; //d2s format number
				xHoleData.d2uDiameter = d2uDiameter;
				//struct copy //FIXME PORTMED later improve the storage
				g_HoleDataArray[ihole] = xHoleData; //struct copy

				//xp_uiOpHistory cleared after loop
			}
			else
			{
				LogParseError(MINIFTMC_PPERR_OVER_POSN_LIMIT);
			}

			//add bytes
			//s_ulSB += s_cHoleRowSize; no need
			g_iPPDirectPos += s_cHoleRowSize;
			//Count
			s_uiCount++;
			if (s_uiCount == g_iHoleCount)
			{
				logf("All %s\r\n", "hd");
				if (s_cKHoleIndex - 1 != g_cKHoleCount)
				{
					logf("FAILURE found %d k but kc = %d\r\n", s_cKHoleIndex - 1, g_cKHoleCount);
					//FIXME MED HIGH better error
					LogParseError(MINIFTMC_PPERR_SECTION_SIZE_DIFF);
					g_cBeingParsed = PARSE_FAIL;
					continue; //will not return to any states other than FAIL
				}
				g_cBeingParsed = PARSE_SECTION;
			}
			continue;
		case SECTION_KHoleData:
			if (s_uiSBN == 0)
			{
				if (i < 2)
				{
					break; //come back when more data to process.
				}
				s_uiSBN = *((int16 *) s);
				s_uiSB = g_cKHoleCount * s_cKHoleRowSize;
				g_iPPDirectPos += 2;
				if (s_uiSB != s_uiSBN)
				{
					logf("Bad len %s", "khd");
					LogParseError(MINIFTMC_PPERR_SECTION_SIZE_DIFF);
					g_cBeingParsed = PARSE_FAIL;
					continue; //will not return to any states other than FAIL
				}
				//s_uiSB = 0; no need
				if (g_cKHoleCount == 0)
				{
					g_cBeingParsed = PARSE_SECTION;
				}
				continue;
			}
			if (i < s_cKHoleRowSize)
			{
				break; //come back
			}

			//read tool
//FIXME9999999999999999999 Add after the rest works... low priority data
//FIXME PASS 2
			//add bytes
			//s_uiSB += s_cKHoleRowSize; no need
			g_iPPDirectPos += s_cKHoleRowSize;
			//Count
			s_cCount++;
			if (s_cCount == g_cKHoleCount)
			{
				logf("All %s\r\n", "khd");
				g_cBeingParsed = PARSE_SECTION;
			}
			continue;
			//probe control field
			//This new section contains mainly the information in the program control field
		case SECTION_ProbeControl:
			if (s_uiSBN == 0)
			{
				if (i < 2)
				{
					break; //come back when more data to process.
				}
				s_uiSBN = *((int16 *) s);
				//logf("khi SBN %u\r\n",s_uiSBN);
				g_iPPDirectPos += 2;
				if (s_uiSBN == 0)
				{
					logf("nd\r\n");
					g_cBeingParsed = PARSE_SECTION;
				}
				continue;
			}
			if (i == 0)
			{
				//logf(".\r\n");
				break;//come back when more data to process.
			}
			//logf("i=%d\r\n",i);

			if (s_uiSB == 0)
			{
				logf("buf = %d %d\r\n", s[0], s[1]);
				//The Position In the file is the data we have saved, plus how far DirectPos has now moved since iStartPos
				//Anything Prior to iStartPos is included in g_uiPartProgramDataLength because it has been saved.
				g_pcProbeCommandDataBase = g_cPartProgramData + g_uiPartProgramDataLength + g_iPPDirectPos - iStartPos;
			}

			uiRemain = s_uiSBN - s_uiSB;
			//logf("pld SBN %u SB %u  rem %u\r\n",s_uiSBN,s_uiSB,uiRemain);
			if (i > uiRemain)
			{
				i = uiRemain;
			}
			//move

			//add them
			s_uiSB += i;
			g_iPPDirectPos += i;

			if (s_uiSB == s_uiSBN)
			{
				//have enough
				logf("%s Hv %ub\r\n", "kpc", s_uiSBN);
				g_cBeingParsed = PARSE_SECTION;
			}
			continue;
		case SECTION_MaterialStack:
			if (s_uiSBN == 0)
			{
				if (i < 2)
				{
					break; //come back when more data to process.
				}
				s_uiSBN = *((int16 *) s);
				g_iPPDirectPos += 2;
				//FIXME MED Potentially I could do a check to see if the max used index is covered
				//Check to see if there is enough memory
				if (s_uiSBN > MAXSIZE_MATERIALS)
				{
					LogParseError(MINIFTMC_PPERR_OVER_RESERVED_PPMEM);
					g_cBeingParsed = PARSE_FAIL;
				}
				s_uiSB = 0;
				if (s_uiSBN == 0)
				{
					logf("nd\r\n");
					g_cBeingParsed = PARSE_SECTION;
				}
				continue;
			}
			if (i == 0)
			{
				break; //come back when more data to process.
			}
//FIXME MED Potentially I could do a check to see if the max used index is covered
			//copy all of this into a special location up to what we need.
//FIXMEasdf
			uiRemain = s_uiSBN - s_uiSB;
			if (i > uiRemain)
			{
				i = uiRemain;
			}
			//move
			memcpy(g_cMaterialData + s_uiSB, s, i);
			//add them
			s_uiSB += i;
			g_iPPDirectPos += i;

			if (s_uiSB == s_uiSBN)
			{
				//have enough
				logf("%s Hv %ub\r\n", "mat", s_uiSBN);
				g_cBeingParsed = PARSE_SECTION;
			}
			continue;
		case SECTION_StackData:
			if (s_uiSBN == 0)
			{
				if (i < 2)
				{
					break; //come back when more data to process.
				}
				s_uiSBN = *((int16 *) s);
				g_iPPDirectPos += 2;
				//Check to see if there is enough memory
				if (s_uiSBN > MAXSIZE_STACKDATA)
				{
					LogParseError(MINIFTMC_PPERR_OVER_RESERVED_PPMEM);
					g_cBeingParsed = PARSE_FAIL;
				}
				s_uiSB = 0;
				if (s_uiSBN == 0)
				{
					logf("nd\r\n");
					g_cBeingParsed = PARSE_SECTION;
				}
				continue;
			}
			logf("hv %d\r\n", i);
			if (i == 0)
			{
				break; //come back when more data to process.
			}
			//copy all of this into a special location up to what we need.
			uiRemain = s_uiSBN - s_uiSB;
			if (i > uiRemain)
			{
				i = uiRemain;
			}
			//move
			memcpy(g_cStackData + s_uiSB, s, i);
			//add them
			s_uiSB += i;
			g_iPPDirectPos += i;

			if (s_uiSB == s_uiSBN)
			{
				//have enough
				logf("%s Hv %ub\r\n", "sta", s_uiSBN);
				g_cBeingParsed = PARSE_SECTION;
			}
			continue;
		case SECTION_SynonymousHole:
			if (s_uiSBN == 0)
			{
				if (i < 2)
				{
					break; //come back when more data to process.
				}
				s_uiSBN = *((int16 *) s);
				g_iPPDirectPos += 2;
				s_uiSB = 0;
				if (s_uiSBN == 0)
				{
					g_cBeingParsed = PARSE_SECTION;
				}
				continue;
			}
			if (i == 0)
			{
				break; //come back when more data to process.
			}
			//copy all of this into a special location up to what we need.
//FIXMEasdf
			uiRemain = s_uiSBN - s_uiSB;
			if (i > uiRemain)
			{
				i = uiRemain;
			}
			//move
//FIXME9999999999999999999 Add after the rest works... low priority data
//FIXME0000000000000000000000000000 WAVE2
			//add them
			s_uiSB += i;
			g_iPPDirectPos += i;

			if (s_uiSB == s_uiSBN)
			{
				//have enough
				logf("%s Hv %ub\r\n", "syn", s_uiSBN);
				g_cBeingParsed = PARSE_SECTION;
			}
			continue;
		case SECTION_Checksum:
			if (s_uiSBN == 0)
			{
				if (i < 2)
				{
					break; //come back when more data to process.
				}
				g_iPPDirectPos += 2;
				s_uiSBN = *((int16 *) s);
				logf("Checksum\r\n");
				if (s_uiSBN != 16)
				{
					logf("Bad len %s", "check");
					LogParseError(MINIFTMC_PPERR_SECTION_SIZE_DIFF);
					g_cBeingParsed = PARSE_FAIL;
					continue; //will not return to any states other than FAIL
				}
				s_uiSB = 0;
				//SPECIAL PATTERN:
				// Do not continue now: Rather do a break so that everything up
				// to this point will be added into the checksum right now
				// and then loop back to the top again
				cLoopNow = 1;
				break;
			}
			if (i < 16)
			{
				//SPECIAL PATTERN:
				//Do not let this data be put into the checksum
				//Advance this number so that noting from this data will be put in.
				iStartPos = g_iPPDirectPos;
				break;
			}
			if (g_PartPgmInfo.cChecksum > 0)
			{
				//Both MD5 and MDARC4 use 16 byte checksum
				memcpy(g_PartPgmInfo.p_cChecksum16, s, 16);
#ifdef MD5HASH
				if (g_PartPgmInfo.cChecksum == 1)
				{
					MD5_Final(pbChecksumbuffer, &g_md5_context); //use line buffer to store 16 byte hash (can't use direct data buffer because this stores data after the end of the position.)
					if (memcmp(pbChecksumbuffer, g_PartPgmInfo.p_cChecksum16, 16) != 0)
					{
						//different
						LogParseError(MINIFTMC_PPERR_CHECKSUM_FALURE);
					}
					else
					{
						logf("Checksum Pass\r\n");
					}
				}
#ifdef MDARC4
				else
#endif
#endif
#ifdef MDARC4
				if (g_PartPgmInfo.cChecksum==2)
				{
					//FIXME MED LOW  complete ARC4 path and test speed improvement
				}
#endif
			}
			g_iPPDirectPos += 16;
			i = g_iPPDirectEnd - g_iPPDirectPos;
			if (i > 0)
			{
				//bytes left over in the format?????
				LogParseError(MINIFTMC_PPERR_BYTES_AFTER_CHECKSUM);
			}

			//Now the file must be done
			//Do NOT continue.
			//Jump to code that will complete the reading

			goto complete_file_load;
			break;
//FIXMEasdf remove store string ????? etc??????

		case PARSE_FAIL:
			//Any Path that comes here, simply exit the loop and stop load mode
			goto complete_file_load;
			break;
		default:
			logf("Unrec Section=%d\r\n", g_cBeingParsed);
			LogParseError(MINIFTMC_PPERR_UNKNOWN_SECTION);
			g_cBeingParsed = PARSE_FAIL;
			//FIXME:::: technically I should be going by this, and I should NOT be breaking....
			//I'm changing my mind on this issue back and forth, but for now I want it to reject programs that are beyond it.
			goto complete_file_load;
			break;
		}
		//break
		break;
	}
//end while(1)
//I use break above to leave and come back later.
//I use continue to go on right at the current time and process more of the input.

//Before we leave the function, add all the used data to the checksum
	s = g_sPPDirect + iStartPos; //The start of the buffer sent to us
	i = g_iPPDirectPos - iStartPos; //The bytes from the start to where we parsed
//logf("rev %d = %d - %d\r\n",i,g_iPPDirectPos,iStartPos);
	if (i > 0)
	{
#ifdef MD5HASH
		if (g_PartPgmInfo.cChecksum == 1)
		{
			MD5_Update(&g_md5_context, s, i);
		}
#ifdef MDARC4
		else
#endif
#endif
#ifdef MDARC4
		if (g_PartPgmInfo.cChecksum==2)
		{
		}
#endif
	}

//Now store data into our storage system
//This is for data that we want to be able to send back out directly
//Not Every section above is stored here

//Currently saving the entire program
	if (g_uiPartProgramDataLength + i > MAX_PARTPGM_STORAGE)
	{
		LogParseError(MINIFTMC_PPERR_OVER_RESERVED_PPMEM);
		g_cBeingParsed = PARSE_FAIL;
	}
	else
	{
		logf("@@@wr %u %d\r\n", g_uiPartProgramDataLength, i);
		memcpy(g_cPartProgramData + g_uiPartProgramDataLength, s, i);
		g_uiPartProgramDataLength += i;
	}

	if (cLoopNow == 1)
	{
		goto loop_now;
	}
//logf("ret\r\n");
	return; //come back later

	complete_file_load:

//Clear Op History
	ClearOpHistory();
	g_iActionHoleIndex = -1;

	if (g_pcProcessLayerDataBase == 0)
	{
		logf("Process Layer Database was not found.\r\n");
		//FIXME MED make true error code
		LogParseError(MINIFTMC_PPERR_OVER_RESERVED_PPMEM);
	}

#ifdef SHOW_PLD
	i = 0;
	while (i <= g_cProcessCount + 1) //p0 is just a place holder and the last entery is just so the length of the last processes data can be known.
	{
		logf("p %d ind %d\r\n", i, g_uiProcessLayerDataIndex[i]);
		i++;
	}
#endif
	logf(".\r\n");
//Since Tool Map was reloaded, correct the loaded tool id
	if (g_szToolIDlen == 0)
	{
		g_cLoadedTool = 0;
	}
	else
	{
		g_cLoadedTool = LookupToolTypeCode(g_szToolTypeCode);
	}
//and any tool searched previously may differ now
	g_cLastSearchedRequiredTool = 0;

	logf("pdpcd\r\n");
	PreviewDisplayProbeCommandDataBase();

	if (g_cAssumeApproxPositionsFromDataset == 1)
	{
		c = 1;
		while (c <= g_cKHoleCount)
		{
			if (g_cKHolePrbeStart[c] != PS_APPROXIMATE) //If they used another method to have these set during the options section, leave those values
			{
				g_cKHolePrbeStart[c] = PS_APPROXIMATE;
				g_cKHolePrbeStatus[c] = PS_APPROXIMATE;
				g_cKHolePrbeStatusWarnings[c] = 0; //clear warning
				i = g_iKHoleHoleIndex[c];
				fx = g_fRawDataX[i];
				fy = g_fRawDataY[i];
				g_fKHolePrbeStartX[c] = fx;
				g_fKHolePrbeStartY[c] = fy;
				g_fKHolePrbeX[c] = fx;
				g_fKHolePrbeY[c] = fy;
			}
			c++;
		}
		SendAllProbeStart(STP_ALERT); //SPSALL
		SendAllProbeStatus(STP_ALERT); //SPSALL
		AlertProbeStatusUpdate();
//zxcvzxcv

	}
	logf("cpeo\r\n");
	CreateProbeEvaluationOrder();

//I'm not sure what this applies to anymore:
//FIXME9999999999999  we never check the length of this and the memory size we have for it.

	logf("che\r\n");
	if (g_PartPgmInfo.cErrorCount == 0)
	{
		//before alerting program is loaded, send out the name and checksum
		SmartToolMsgStr(0, STP_ALERT, MINIFT_OID_PARTPGM_NAME, g_szPartPgmFilename);
		SmartToolMsg(0, STP_ALERT, MINIFT_OID_PARTPGM_CHECKSUM, 16, (char *) g_PartPgmInfo.p_cChecksum16);

		g_cPartPgmStatus = PP_LOADOK;
	}
	else
	{
		g_cPartPgmStatus = PP_PARSEFAIL;
	}
	SmartToolMsgChar(0, STP_ALERT, MINIFT_OID_PARTPGM_STATUS, g_cPartPgmStatus);

//Now, in the case of failure, and after the status has been sent echo the 1st error
	if (g_PartPgmInfo.cErrorCount == 0)
	{
		PartProgramShowStatus();
	}
	else
	{
		SmartToolMsgMiniFtMessageCode(0, MINIFT_OID_PARTPGM_STATUS, g_PartPgmInfo.p_uiErrorMessages[0]);
		PartProgramShowStatusPart2();
	}

#ifdef OUTPUT_PROGRAM_MEM_USAGE
//This does not show every kind of memory used, but displays the usage of
//certain areas that could potentially by exceeded.
	logf("Prog Mem:\r\n");
	logf(" %u data bytes used / %u available\r\n", g_uiPartProgramDataLength, MAX_PARTPGM_STORAGE);
#endif

	logf("  K1OD X=%.3f Y=%.3f EDX=%.3f\r\n", g_PartPgmInfo.K1OriginDistX,
			g_PartPgmInfo.K1OriginDistY, g_PartPgmInfo.K1EndDistX);

	return;
}

//Part Program Abstracted Section Function
void PartProgramShowStatus()
{
	logf("PP OK %s:\r\n", g_szPartPgmFilename);
	logf("T=%d P=%d F=%d Pa=%d H=%d\r\n", g_cToolCount, g_cProcessCount,
			g_cFastenerCount, g_cPatternCount, g_iHoleCount);
//for (iTemp = 0 ; iTemp < g_iHoleCount; iTemp++)
//{
//		logf("%.5f %.5f \r\n",xgetfloat(xp_fRawDataX+4*iTemp) , xgetfloat(xp_fRawDataY+4*iTemp));
//}
}

void PartProgramShowStatusPart2()
{
	int i;
	logf("PP FAIL status=%d errs=%d\r\n", g_cPartPgmStatus, g_PartPgmInfo.cErrorCount);	//debug
	i = 0;
	while (i < g_PartPgmInfo.cErrorCount)
	{
		logf(" #%d:%d\r\n", i, g_PartPgmInfo.p_uiErrorMessages[i]);
		i++;
	}
}

//Part Program Supporting Functions

//Add an error message to the part program structures error list
void LogParseError(unsigned int uiMessageCode)
{
	g_PartPgmInfo.p_uiErrorMessages[g_PartPgmInfo.cErrorCount] = uiMessageCode;
	g_PartPgmInfo.cErrorCount++;
}

//FIXME minor  move to lib

void CreateProbeEvaluationOrder()
{
	byte c;
//assume that they are in order...
//assume everything requires a probe

	c = 1;
	while (c <= g_cKHoleCount)
	{
		g_cKHolePrbeEvalDCount[c] = 1;
		g_cKHolePrbeEvalOrder[c] = c;
		c++;
	}

//FIXMENOW00 FUTURE: Must implement the real code to do this right as outlined
//Currently we only use rows that do have probe, and we only
	/*
	 --establish a calculation order as follows
	 set all order to 255
	 go through the list.
	 if an item requires no probes and no other items, then set it to 0
	 if an item requires a probe, but no dependents set it to 1
	 otherwise if an item has dependents, see what the highest number is, set this number to that +1.
	 but if any of the dependents are 255, queue this item into a list and move on.
	 When done the 1st pass through do the next pass through the queue, but this time
	 some of the 255 should have their dependents done... requeue what is needed
	 Do this until all have an order number.
	 Now write all of the 0s into an order array, then the 1s, etc.

	 So now there is a perfect order for evaluation.
	 If any thing is recalculated, only items appearing later in the list
	 depend on it..
	 */

	return;
}

void RecalculateLocatingDirectives(byte ki)
{
	if (ki == 0)
	{
		//The initial Recalculation....
		//FIXMENOW00 FUTURE: Don't have any types which are not probe supported as of this moment so there is never any progress.
		return;
	}

	return;
}

//// TEACH MODE

void CaptureTeachPosition()
{
	//int i;
	int index;
	//int floatindex;
	//int sr;
	//char * s;
	//char * nullafter;
	//byte c;
	float fx;
	float fy;
	td_HoleData xHoleData;

	if (g_PartPgmInfo.cTeachModeData == 0)
	{
		//not a teach mode program;
		return;//just return without adding //FIXME add alert
	}

	index = g_iHoleCount;

	fx = g_TeachMachinePosn.fX;
	fy = g_TeachMachinePosn.fY;
//There is no name for these positions.
//There is no part program

	if (index == 0)
	{
		//First Hole
		g_fKHolePrbeX[1] = fx;
		g_fKHolePrbeY[1] = fy;
		g_cKHolePrbeStatus[1] = PS_PROBED;
		g_cKHolePrbeStatusWarnings[1] = 0; //clear warning
	}
	else
	{
		g_fKHolePrbeX[2] = fx;
		g_fKHolePrbeY[2] = fy;
		g_cKHolePrbeStatus[2] = PS_PROBED;
		g_cKHolePrbeStatusWarnings[2] = 0; //clear warning
	}

//save the position data
#ifdef OUTPUT_TEACH
	logf("TEACH X=%.4f Y=%.4f into Hole %d\r\n",fx,fy,index); //debug code
#endif

//X DATA
	g_fRawDataX[index] = fx;

//Y DATA
	g_fRawDataY[index] = fy;

//MX DATA
	g_fRawDataMX[index] = fx;

//MY DATA
	g_fRawDataMY[index] = fy;

//radius to half way to nearest point (stored square of true value)
	g_fRawDataR[index] = 0; //start at 0

//Make EVERY Hole Look like a K Hole
	xHoleData.cKInd = 0;
	xHoleData.cKPri = 0;
	xHoleData.cKSec = 0;
//And Set all these to default values
	xHoleData.cTool = 0;
	xHoleData.cProcess = 0;
	xHoleData.cFastener = 0;
	xHoleData.cPattern = 0;
	xHoleData.uiOps = OP_DRILL | OP_FILL | OP_SEAL | OP_REMOVE;
	xHoleData.uiStack = 0;
	xHoleData.iCountersinkAdjust = 0;
	xHoleData.d2uDiameter = 10000;

//struct copy //FIXME PORTMED later improve the storage
	g_HoleDataArray[index] = xHoleData; //struct copy

//Init Operation History for this point to Zero
	g_uiOpHistory[index] = 0;

//add to data point count
	g_iHoleCount++;

//blink for capture
	LEDCount(3, 200, 400);
//and beep one time
	Beep();

	g_uiPartProgramDataLength = 0; //Don't set this until we complete at the end
//Set the Position Data Here
	memcpy(g_cPartProgramData + 6 + 8 * index, (char *) &fx, 4); //no compression + no conversion... send as float
	memcpy(g_cPartProgramData + 10 + 8 * index, (char *) &fy, 4); //no compression + no conversion... send as float

	ResetNearestPosition();
	g_uiCreateMachineDataSetHashIndex = 0; //Ensure that it won't be used the entire time during capture
	return;
}

void DeletePosition()
{
	if (g_iHoleCount == 0)
	{
		return;
	}
//decrement data points
	g_iHoleCount--;
	return;
}

void CompleteTeachPosition()
{
	if (g_iHoleCount == 0)
	{
		return;
	}

//Calculate the used length of the data
	g_uiPartProgramDataLength = 6 + 8 * g_iHoleCount;

//Set the Header, Length, and Count
	*(uint16*) (g_cPartProgramData) = (uint16) 0x6000; //special header
	*(uint16*) (g_cPartProgramData + 2) = (uint16) (g_uiPartProgramDataLength
			- 4);
	*(int16*) (g_cPartProgramData + 4) = g_iHoleCount;
//The rest is all data that was set above

//modify the checksum
	*(uint32 *) (g_PartPgmInfo.p_cChecksum16) = MS_TIMER;
	*(int16 *) (g_PartPgmInfo.p_cChecksum16 + 4) = g_iHoleCount;

//before alerting program is loaded, send out the name and checksum
	SmartToolMsgStr(0, STP_ALERT, MINIFT_OID_PARTPGM_NAME, g_szPartPgmFilename);
	SmartToolMsg(0, STP_ALERT, MINIFT_OID_PARTPGM_CHECKSUM, 16, (char *) g_PartPgmInfo.p_cChecksum16);

	g_cPartPgmStatus = PP_LOADOK;
	SmartToolMsg(0, STP_ALERT, MINIFT_OID_PARTPGM_STATUS, 1, (char *) &g_cPartPgmStatus);

	ResetNearestPosition();

//Set DrillDir based on DrillDir default settings and ProbeDir
	SetDrillDir(g_ConfigData.iProbeDir);
//FIXME
//I think there are still some arrays that need setting...
//ensure any program things are reset in regard to probe
//   used to have all kinds of probe=true stuff here

//Now Begin Calculation, and move to the standard PROBE WAIT CALC
//All we do here is go to MODE_PROBE, but that will move us directly to PROBE_WAIT_CALC
	g_cModeState = MODE_PROBE;
	StartCreateMachineDataSet();
	return;
}

void LoadLinearProgram()
{
//FIXME MED LOW  Load Linear Program
//This code removed to  old_linear_program.txt
	return;
}

void ClearOpHistory()
{
	byte c;
//Clear OpHistory Array for this part program.
	memset(g_uiOpHistory, 0, 2 * g_iHoleCount);
	g_cBlockCount = (g_iHoleCount + 127) >> 7; //must get full block for any part of the last 128, so add 127
	logf("BC=%d\r\n", g_cBlockCount);
	c = 0;
	while (c < OPHISTORYBLOCKCOUNT) //since there are only OPHISTORYBLOCKCOUNT (40 when using 5000 holes arrays), clear all spots
	{
		g_cOpHistoryBlockOrder[c] = c;
		g_ulOpHistoryBlockTime[c] = 0;
		c++;
	}
}

void AddOpHistory(int index, unsigned int uiOperation)
{
	unsigned int ops;
	unsigned int newops;
	byte cBlock;
	byte c;
	byte clen;
	byte cMRUPosition;
	char *p_cdst;
	char *p_csrc;
	if (index < 0)
	{
		return;
	}
	ops = (unsigned int) g_uiOpHistory[index];
	newops = ops | uiOperation;
#ifdef OUTPUT_OPHIST
	logf("SetOp i=%d ops %u => %u\r\n",index,ops,newops);
#endif
	ops = newops;
	g_uiOpHistory[index] = ops;
//Now Alert this operation via the OP_STARTED (we do this always)
	if (uiOperation != OP_PROBE_WARNINGS) //don't bother sending OP_PROBE_WARNINGS back one at a time Pendant will request update after probe.
	{
		SmartToolMsgUInt16(0, STP_ALERT, MINIFT_OID_OP_STARTED, uiOperation);
	}
//I currently am NOT sending OP_HISTORY back after an update,
//and I am not putting a timestamp on MINIFT_OID_OP_STARTED right now.
//I am keeping it simple, but when it reconnect it will have a low time for the last update
//and it will end up getting more blocks.... This is OK.  The system is efficient.
//I should at some point add the timestamp to OP_STARTED....

	if (g_cBlockCount == 0)
	{
		//must be no program loaded
		return;
	}

	cBlock = index >> 7;
	if (cBlock >= g_cBlockCount)
	{
		//too high
		logf("bcth\r\n");
		return;
	}
	g_ulOpHistoryBlockTime[cBlock] = SEC_TIMER; //set the time

#ifdef OUTPUT_OPHIST_VERBOSE
//debug code
	c=0;
	while(c<g_cBlockCount)
	{
		logf("%d]=b %d\r\n",c,g_cOpHistoryBlockOrder[c]); c++;
	}
#endif

	cMRUPosition = g_cBlockCount - 1;
	c = cMRUPosition;
	while (cBlock != g_cOpHistoryBlockOrder[c])
	{
		if (c == 0)
		{
			logf("never found\r\n");
			return;
		}
		c--;
	}
//found it here
	if (c == cMRUPosition)
	{
		//it's allready at the MRU position, so we don't need to move anything
		return;
	}
//Block was at this position
#ifdef OUTPUT_OPHIST_VERBOSE
	logf("bp=%d\r\n",c);
#endif
//Move everything else down one, and set the MRU position to cBlock;
	p_cdst = (char *) g_cOpHistoryBlockOrder + c;
	p_csrc = p_cdst + 1;
	clen = cMRUPosition - c;
	memcpy(p_cdst, p_csrc, clen); //move everything else down
	g_cOpHistoryBlockOrder[cMRUPosition] = cBlock; //move this block to the MRU position
//Note: there is and O(1) speed algorithym to do this using double linked lists thata requires at least 2 40 byte arrays,
//but the current algorithm works just fine.   I choose to implement the simpler algorithm, but if I had many more blocks,
//I could have selected the O(1) algorithm.

#ifdef OUTPUT_OPHIST_VERBOSE
//debug code
	c=0;
	while(c<g_cBlockCount)
	{
		logf("%d]=b %d\r\n",c,g_cOpHistoryBlockOrder[c]); c++;
	}
#endif

	return;
//test needed
}

void SetOpHistory(int index, unsigned int uiOperations)
{
//Set is less common than Add, therefore, share code and make Set Use Add.
//To Set we just set the memory to the operations, but then call add anyway to take care of the rest.
//That Add ends up doing an extra OR operation, but we hardly use SetOpHistory
	g_uiOpHistory[index] = uiOperations;
	AddOpHistory(index, uiOperations);
}

unsigned int GetOpHistory(int index)
{
	return g_uiOpHistory[index];
}

void SetDrillDir(int iProbeDir)
{
//Set DrillDir based on DrillDir default settings and ProbeDir

	g_iDrillDir = g_ConfigData.iDrillDir; //use default here
	if (g_iDrillDir == DRILLDIR_NONE) //we never accept none as a setting
	{
		g_iDrillDir = DRILLDIR_ATOB;
	}
	if (g_iDrillDir == DRILLDIR_SAME)
	{
		if (iProbeDir == PROBEDIR_ATOB)
		{
			g_iDrillDir = DRILLDIR_ATOB;
		}
		else
		{
			g_iDrillDir = DRILLDIR_BTOA;
		}
	}
	else if (g_iDrillDir == DRILLDIR_REVERSE)
	{
		if (iProbeDir == PROBEDIR_ATOB)
		{
			g_iDrillDir = DRILLDIR_BTOA;
		}
		else
		{
			g_iDrillDir = DRILLDIR_ATOB;
		}
	}
}

//FIXME000000 Could put this in a different place....
//move this stuff later
void InitProbeValues()
{
	byte c;
	logf("IPV\r\n");
	c = 1;
	while (c <= MAXNUM_KHOLES) //Must Clear them all
	{
		g_fKHolePrbeX[c] = 0;
		g_fKHolePrbeY[c] = 0;
		g_cKHolePrbeStatus[c] = PS_NO_PROBE;
		g_cKHolePrbeStatusWarnings[c] = 0; //clear warning
		g_cKHolePrbeStatusDistance[c] = 0;
		g_fKHolePrbeStartX[c] = 0;
		g_fKHolePrbeStartY[c] = 0;
		g_cKHolePrbeStart[c] = PS_NO_PROBE;
		c++;
	}
	g_cProbeDistanceErrors = 0;
	g_cProbeComplete = 0;
	g_cProbeCalculated = 0;
//Do not send them back, but depend on the caller to know when to do this.
//The Only Place this is used Immediately calls another functon which does a reset at the start
}

void ResetProbeValues()
{
	byte c;
	logf("RPV\r\n");
	c = 1;
	while (c <= MAXNUM_KHOLES)
	{
		g_fKHolePrbeX[c] = 0;
		g_fKHolePrbeY[c] = 0;
		g_cKHolePrbeStatus[c] = PS_NO_PROBE;
		g_cKHolePrbeStatusWarnings[c] = 0; //clear warning
		g_cKHolePrbeStatusDistance[c] = 0;
		if (g_cKHolePrbeStart[c] == PS_APPROXIMATE)
		{
			//leave it
		}
		else
		{
			//reset it
			g_fKHolePrbeStartX[c] = 0;
			g_fKHolePrbeStartY[c] = 0;
			g_cKHolePrbeStart[c] = PS_NO_PROBE;
		}
		c++;
	}
	g_cProbeDistanceErrors = 0;
	g_cProbeComplete = 0;
	g_cProbeCalculated = 0;
//Do not send them back, but depend on the caller to know when to do this.
//The Places this is used will call another function which will finally send the updated status back.
}

void SendProbeValues()
{
//Alert All Probe Start and Status
	SendAllProbeStart(STP_ALERT);	//SPSALL
	SendAllProbeStatus(STP_ALERT);	//SPSALL
	AlertProbeStatusUpdate();
}

void CheckProbeComplete()
{
	byte ck;
	ck = 1;
	while (ck <= g_cKHoleCount)
	{
		if (g_cKHolePrbeStatus[ck] < PS_PROBED)
		{
			//not all are probed
			g_cProbeComplete = 0;
			return;
		}
		if (g_cKHolePrbeStatusDistance[ck] > 0)
		{
			g_cProbeComplete = 0;
			return;
		}
		ck++;
	}
	g_cProbeComplete = 1;
	return;
}

//////////////////////////////////////////////////////////////////////
// Job Selection
//////////////////////////////////////////////////////////////////////
void StartOver()
{
//Does not clear all job parameters, but requires reprobe at least.
//This clears whatever is needed when they select "Start Over".
//If it does not clear until they load a new program, then it should go into
// ClearPartProgram() instead.
	logf("so\r\n");

	ResetProbeValues();
	RecalculateLocatingDirectives(0);
	SendProbeValues();    //SPSALL

//ClearGolbalRotation(); //just clear two variables
	g_cRotationKP = 0;
	g_cRotationKS = 0;
	g_cRotationContext = 0;

//Clear DataSetIDTimes
	g_ulMachineDataSetIDTimeSec = 0;
	g_ulMachineDataSetIDTime = 0;
//Alert new MachineDataSetID now
	SendXYDataID(0, STP_ALERT);
//Stop SendXYData
	g_cSendXYDataSessions = 0;
//Stop Sending Op History
	g_cSendOpHistory = 0;

	memset(g_cSendXYDataSession, 0, MAXNUM_STP_SESSIONS);
	memset(g_cSendOpHistoryBlockOrderIndex, 0xFF, MAXNUM_STP_SESSIONS);
	ClearOpHistory();
	g_iActionHoleIndex = -1;

//FIXME MED Pattern
//g_iChosenPattern=0;

	g_uiActionCycles = 0;

	g_cAllowKVisit = 0;
	g_cAutoMove = 0;
	g_uiStartAutoTime = 0;
	g_cAutoRepeat = 0;

	ResetNearestPosition();
	g_uiCreateMachineDataSetHashIndex = 0;

	g_cToolLoaded = 0;
	g_cOverrideCalculated = 0;
	g_cProcessLoaded = 0;
	g_cLoadedProcess = 0;
	g_cDrillLoadProcessAndOverride = DRILL_LOAD_PROCESS_NOT_NEEDED;

	g_MachineOffset.fX = 0;
	g_MachineOffset.fY = 0;

	g_Probe.cRegistration = REGISTRATION_UNKNOWN;
	SmartToolMsgChar(0, STP_ALERT, MINIFT_OID_PROBE_REGISTRATION, g_Probe.cRegistration);

#ifdef DRILL_DIRECT_READY
	SmartDrillSTPSet(SMARTDRILL_OID_FAULT_CLEAR);
#endif
	g_DrillFault.cSeverity = 0;

#ifdef FASTENER_STP
	FastenerSTPSet(FASTENER_OID_FAULT_CLEAR);
	g_FastenerFault.cSeverity = 0;

	FastenerSTPSetUInt(FASTENER_OID_FASTENER_LOADED,0); //Clear this when starting over...
#endif
	g_ConfigData.uiProcessOperations = OP_DRILL | OP_SEAL | OP_FILL | OP_REMOVE | OP_INSPECT; //set this to the main set...
	SmartToolMsgUInt16(0, STP_ALERT, MINIFT_OID_PROCESS_OPERATIONS, g_ConfigData.uiProcessOperations);

}

//Send Part Program Data
//
// PHASE 1
// This phase simply stores the file data from start to the hole section
// and then sends this part..
//
// PHASE 2
// Keep track of section and be able to send the 1st part, then reconstruct the
// Tool Data, Process Data, and Hole Data from memory in order to send those back as well
// Potentially we might need another OID that sends back data in an more digested format
// for the pendant to use.  Don't worry about this for now.
// FIXME MED HIGH PPData Phase 2 not sure about priority ?????????

//FIXME May need to move this to better location
void StartSendPartProgramData(int iSession, uint32 uiStart)
{
	byte c;
//Must have the file loaded.  Can't be loading the file at the same time
	if (g_cPartPgmStatus != PP_LOADOK)
	{
		return;
	}

	if (uiStart > g_uiPartProgramDataLength)
	{
		uiStart = 0;
	}

//Get ready to send the list.
	g_cSendPPDataSession[iSession] = 1;
	g_uiSendPPDataIndex[iSession] = uiStart;

//Find the count of sessions....
	c = 0;
	g_cSendPPDataSessions = 0;
	while (c < MAXNUM_STP_SESSIONS)
	{
		if (g_cSendPPDataSession[c] == 1)
		{
			g_cSendPPDataSessions++;
		}
		c++;
	}
#ifdef OUTPUT_SENDPPDATA
	logf("PPDATA %u\r\n",uiStart); //FIXME0 can remove soon
#endif
//Now there is a section in the main mode machine which will
//continue calling the function below to send the file back
	return;
}

void ContinueSendPartProgramData()
{
	uint32 uix;
	uint16 ui;
	char * p_buffer;
	byte c;
	byte cfound;
	byte cfinal;
	byte csendcount;
	uint32 uiStart;
	td_STPsessions * p_STPSession = 0;

//Do not check here, but check before call
//if (g_cSendPPDataSessions == 0)
//{
// 	//nothing is being sent
//    return;
//}

	p_buffer = g_STPtxMsg.p_cObjectValue;

	c = 0;
	cfound = 0;
	while (c < MAXNUM_STP_SESSIONS)
	{
		if (g_cSendPPDataSession[c] == 1)
		{
			uiStart = g_uiSendPPDataIndex[c];
			if (uiStart >= g_uiPartProgramDataLength)
			{
				//seems like we sent it all
				g_cSendPPDataSession[c] = 0;
#ifdef OUTPUT_SENDPPDATA
				logf("sa %u %u \r\n",ulStart,g_uiPartProgramDataLength);
#endif
			}
			else
			{
				//count how many we are sending to
				cfound++;
				csendcount = 0;
				while (csendcount < 2)
				{
					//have something to send
					//send the next part now
					//Create This Packet and Send Some Data
					//Calculate How Much to send
					cfinal = 1;
					uix = g_uiPartProgramDataLength - uiStart;
					if (uix > 480) // 512-32 = 480
					{
						uix = 480;
						cfinal = 0;
					}
					ui = (uint16) uix;

					//The Start of the Data
					writeHInt32ToNBuffer(p_buffer, uiStart);
					//Flag if this is the last segment
					p_buffer[4] = cfinal;

					//The Length of the Data is calculated because it knows that it's the payload - 5

					//The Data Itself
					memcpy(p_buffer + 5, g_cPartProgramData + uiStart, ui);
					p_STPSession = GetSessionByNumber((unsigned int) c);
					SmartToolMsg(p_STPSession, STP_GET_RESP, MINIFT_OID_PARTPGM_DATA, ui + 5, p_buffer);
#ifdef OUTPUT_SENDPPDATA
					logf("s %u %u %d\r\n",uiStart,ui,cfinal);
#endif
					uiStart += ui;  //add the send value to the index
					g_uiSendPPDataIndex[c] = uiStart;

					//See if the end was reached...
					if (uiStart == g_uiPartProgramDataLength)
					{
						//NOTE: Could send back more data here if needed, but I decided to not do this currently.
						//  see /archive/send_write_file_sections.txt

						//Now Clear The Flag to indicate that this is complete
						g_cSendPPDataSession[c] = 0;
						//Don't Count this one, since it's actually done now.
						cfound--;
						//break from the loop
						break;
					}
					csendcount++;
				}
			}
		}
		c++;
	}
	if (cfound == 0)
	{
		//has gone back to zero so clear the main variable
		g_cSendPPDataSessions = 0;
#ifdef OUTPUT_SENDPPDATA
		logf("s cf0\r\n");
#endif
	}
}

//marker
void SendProbeStatus(unsigned int uiMsgType, byte cKIndex)
{
	td_oid_probe_status * p_oid_probe_status;
	byte c;
//logf("sps\r\n");
	p_oid_probe_status = (td_oid_probe_status *) g_STPtxMsg.p_cObjectValue;
	c = cKIndex;
	p_oid_probe_status->cKIndex = c;
	p_oid_probe_status->fX = g_fKHolePrbeX[c];
	p_oid_probe_status->fY = g_fKHolePrbeY[c];
	p_oid_probe_status->cStatus = g_cKHolePrbeStatus[c];
	p_oid_probe_status->cMethod = g_cKHolePrbeStatusWarnings[c] << 4; //use to send warning in high half
	if (g_cKHolePrbeStatusDistance[c] != 0)
	{
		p_oid_probe_status->cMethod |= 8; //set bit 4 to indicate distance issue
	}

#ifdef OUTPUT_PROBE_SYS
	logf("%s K%d %f,%f %d\r\n", "G PS", c, p_oid_probe_status->fX, p_oid_probe_status->fY, p_oid_probe_status->cStatus);
#endif
	SmartToolMsg(0, uiMsgType, MINIFT_OID_PROBE_STATUS,
			sizeof(td_oid_probe_status), g_STPtxMsg.p_cObjectValue);
}

void SendAllProbeStatus(unsigned int uiMsgType)
{
	td_oid_probe_status * p_oid_probe_status;
	byte c;
	p_oid_probe_status = (td_oid_probe_status *) g_STPtxMsg.p_cObjectValue;
	c = 1;
//logf("saps\r\n");
	while (c <= g_cKHoleCount)
	{
		p_oid_probe_status->cKIndex = c;
		p_oid_probe_status->cStatus = g_cKHolePrbeStatus[c];
		p_oid_probe_status->cMethod = g_cKHolePrbeStatusWarnings[c] << 4; //use to send warning in high half
		if (g_cKHolePrbeStatusDistance[c] != 0)
		{
			p_oid_probe_status->cMethod |= 8; //set bit 4 to indicate distance issue
		}
		p_oid_probe_status->fX = g_fKHolePrbeX[c];
		p_oid_probe_status->fY = g_fKHolePrbeY[c];
#ifdef OUTPUT_PROBE_SYS
		logf("%s K%d %f,%f %d\r\n", "G PS", c, p_oid_probe_status->fX, p_oid_probe_status->fY, p_oid_probe_status->cStatus);
#endif
		SmartToolMsg(0, uiMsgType, MINIFT_OID_PROBE_STATUS,
				sizeof(td_oid_probe_status), g_STPtxMsg.p_cObjectValue);
		memdump("dump1", g_STPtxMsg.p_cObjectValue, sizeof(td_oid_probe_status));
		p_oid_probe_status->fY = 0;
		memdump("dump2", g_STPtxMsg.p_cObjectValue, sizeof(td_oid_probe_status));
		p_oid_probe_status->fX = 0;
		memdump("dump3", g_STPtxMsg.p_cObjectValue, sizeof(td_oid_probe_status));
		c++;
	}
}

void SendProbeStart(unsigned int uiMsgType, byte cKIndex)
{
	td_oid_probe_start * p_oid_probe_start;
	byte c;
	p_oid_probe_start = (td_oid_probe_start *) g_STPtxMsg.p_cObjectValue;
	c = cKIndex;
	p_oid_probe_start->cKIndex = c;
	p_oid_probe_start->cStatus = g_cKHolePrbeStart[c];
	p_oid_probe_start->cMethod = 0;
//#warnt "FIXME Minor  Not using saving Probe Method with probe start or status"
//see here and in this area
	p_oid_probe_start->fX = g_fKHolePrbeStartX[c];
	p_oid_probe_start->fY = g_fKHolePrbeStartY[c];
#ifdef OUTPUT_PROBE_SYS
	logf("%s K%d %f,%f %d\r\n", "G Pstart", c, p_oid_probe_start->fX, p_oid_probe_start->fY, p_oid_probe_start->cStatus);
#endif
	SmartToolMsg(0, uiMsgType, MINIFT_OID_PROBE_START,
			sizeof(td_oid_probe_start), g_STPtxMsg.p_cObjectValue);
}

void SendAllProbeStart(unsigned int uiMsgType)
{
	td_oid_probe_start * p_oid_probe_start;
	byte c;
	p_oid_probe_start = (td_oid_probe_start *) g_STPtxMsg.p_cObjectValue;
	c = 1;
	while (c <= g_cKHoleCount)
	{
		p_oid_probe_start->cKIndex = c;
		p_oid_probe_start->cStatus = g_cKHolePrbeStart[c];
		p_oid_probe_start->cMethod = 0;
		p_oid_probe_start->fX = g_fKHolePrbeStartX[c];
		p_oid_probe_start->fY = g_fKHolePrbeStartY[c];
#ifdef OUTPUT_PROBE_SYS
		logf("%s K%d %f,%f %d\r\n", "G Pstart", c, p_oid_probe_start->fX, p_oid_probe_start->fY, p_oid_probe_start->cStatus);
#endif
		SmartToolMsg(0, uiMsgType, MINIFT_OID_PROBE_START,
				sizeof(td_oid_probe_start), g_STPtxMsg.p_cObjectValue);
		memdump("dump1", g_STPtxMsg.p_cObjectValue, sizeof(td_oid_probe_start));
		p_oid_probe_start->fY = 0;
		memdump("dump2", g_STPtxMsg.p_cObjectValue, sizeof(td_oid_probe_start));
		p_oid_probe_start->fX = 0;
		memdump("dump3", g_STPtxMsg.p_cObjectValue, sizeof(td_oid_probe_start));
		c++;
	}
}

void AlertProbeStatusUpdate()
{
//Send this single empty OID that tells pendant that it should update the k hole information display now
//This is preferable to doing an update for each status update that returns.

//    SmartToolMsgEmpty(p_STPSession, STP_ALERT, MINIFT_OID_PROBE_UPDATE_NOW);
//
	SmartToolMsgMiniFtMessageCode(0, MINIFT_OID_PROBE_STATUS, 0); //use message code zero... this is just a hack to allow this to work
}

//void SendKHoleDistance(unsigned int uiMsgType,float fExpected,float fFound)
//{
//	td_oid_khole_distance * p_oid_khole_distance;
//	p_oid_khole_distance=(td_oid_khole_distance *)g_STPtxMsg.p_cObjectValue;
//	p_oid_khole_distance->fexpected=fExpected;
//	p_oid_khole_distance->ffound=fFound;
//	SmartToolMsg(uiMsgType, MINIFT_OID_KHOLE_DISTANCE, sizeof(td_oid_khole_distance),g_STPtxMsg.p_cObjectValue);
//	return;
//}

//FIXME PORTMED REVIEW

//Machine Data Set Code
//SendXYData System

//Machine Data Set Code
void SendXYDataID(td_STPsessions * p_STPSession, unsigned int uiMsgType)
{
	uint32 * p_ui32;
	byte g_cXYDataID[8];
	p_ui32 = (uint32 *) g_cXYDataID;
	p_ui32[0] = g_ulMachineDataSetIDTimeSec;
	p_ui32[1] = g_ulMachineDataSetIDTime;
//FIXME PORTHIGH  sendxydata not defined?????
#ifdef OUTPUT_SENDXYDATA
	logf("XYDataID sent %d %d %d %d %d %d %d %d\r\n",
			g_cXYDataID[0], g_cXYDataID[1],
			g_cXYDataID[2], g_cXYDataID[3],
			g_cXYDataID[4], g_cXYDataID[5],
			g_cXYDataID[6], g_cXYDataID[7]);
#endif
	SmartToolMsg(p_STPSession, STP_ALERT, MINIFT_OID_POSNMODE_XYDATA_ID, 8, (char *) g_cXYDataID);
}

void StartSendXYData(int iSession, unsigned int uiStart)
{
//Must have the file loaded.  Can't be loading the file at the same time
	if (g_cPartPgmStatus != PP_LOADOK)
	{
		return;
	}

	if (uiStart > g_iHoleCount)
	{
		uiStart = 0;
	}

//Get ready to send the list.
	g_cSendXYDataSession[iSession] = 1;
	g_uiSendXYDataIndex[iSession] = uiStart;
	g_cSendXYDataSessions = 1; //Because there is at least one session now
#ifdef OUTPUT_SENDXYDATA
			logf("XYDATA %u\r\n",uiStart);
#endif
//Now there is a section in the main mode machine which will
//continue calling the function below to send the file back
	return;
}

void ContinueSendXYData()
{
	unsigned int uix;
	unsigned int ui;
	char * p_buffer;
	byte c;
	byte csendcount;
	byte cNotDone;
	unsigned int uiStart;
	float fX, fY;
	td_STPsessions * p_STPSession = 0;

//do not check here, check before call
//if (g_cSendXYDataSessions == 0)
//{
//	//nothing is being sent
//    return;
//}

	c = 0;
	cNotDone = 0;
	while (c < MAXNUM_STP_SESSIONS)
	{
		if (g_cSendXYDataSession[c] == 1)
		{
			uiStart = g_uiSendXYDataIndex[c];
			//count how many we are sending to
			csendcount = 0;
			while (1)
			{
				if (uiStart >= g_iHoleCount)
				{
					//seems like we sent it all
					g_cSendXYDataSession[c] = 0;
					break;
				}
				//have something to send
				//send the next part now
				//Create This Packet and Send Some Data
				//Calculate How Much to send
				uix = g_uiCreateMachineDataSetIndex - uiStart;
				if (uix > 64) // 512-32 = 480
				{
					uix = 64;
				}
				if (uix > 0)
				{
					p_buffer = g_STPtxMsg.p_cObjectValue;
					//The Start of the Data
					writeHInt16ToNBuffer(p_buffer, uiStart);
					p_buffer += 2;
					writeHInt16ToNBuffer(p_buffer, g_iHoleCount); //to make sure they know count when they get this.
					p_buffer += 2;
					writeHInt16ToNBuffer(p_buffer, uix);
					p_buffer += 2;
#ifdef OUTPUT_SENDXYDATA
					logf("XYData Seg %u %d %u\r\n",uiStart, g_iHoleCount, uix);
#endif
					ui = uiStart;
					while (uix > 0)
					{
						fX = g_fRawDataMX[ui];
						fY = g_fRawDataMY[ui];
						((float *) p_buffer)[0] = fX;
						p_buffer += 4;
						((float *) p_buffer)[0] = fY;
						p_buffer += 4;
						ui++;
						uix--;
					}
#ifdef OUTPUT_SENDXYDATA
					logf("XYDATA %u to %u\r\n",uiStart,ui-1);
#endif
					p_STPSession = GetSessionByNumber((unsigned int) c);
					SmartToolMsg(p_STPSession, STP_ALERT, MINIFT_OID_POSNMODE_XYDATA, p_buffer - g_STPtxMsg.p_cObjectValue, g_STPtxMsg.p_cObjectValue);

					uiStart = ui;  //Update the start
					g_uiSendXYDataIndex[c] = uiStart;
				}
				//See if the end was reached...
				if (uiStart >= g_iHoleCount)
				{
					//seems like we sent it all
					g_cSendXYDataSession[c] = 0;
					//break from the loop
					break;
				}
				if (uiStart >= g_uiCreateMachineDataSetIndex)
				{
					//need more calculated but we are not yet done
					cNotDone = 1;
					//break from the loop
					break;
				}
				csendcount++;
				if (csendcount == 2)
				{
					//no more loops now
					cNotDone = 1;
					break;
				}
			}
		}
		c++;
	}
//Retain Flag if Anything is still needed
	g_cSendXYDataSessions = cNotDone;
}

void StartSendOpHistory(int iSession, unsigned long ulAfterTime)
{
//Must have the file loaded.  Can't be loading the file at the same time
	if (g_cPartPgmStatus != PP_LOADOK)
	{
		return;
	}
//More?

//Get ready to send the list.
	g_cSendOpHistoryBlockOrderIndex[iSession] = 0;
	g_ulSendOpHistoryAfterTime[iSession] = ulAfterTime;
	g_cSendOpHistory = 1; //there is at least one that needs sending now

#ifdef OUTPUT_OPHIST
	logf("OHS\r\n");
#endif
//Now there is a section in the main mode machine which will
//continue calling the function below to send the data back
	return;
}

void ContinueSendOpHistory()
{
	unsigned int uix;
	unsigned int ui;
	unsigned long ulAfterTime;
	unsigned long ulBlockTime;
	char * p_buffer;
	byte c;
	//byte csendcount;
	byte cNotDone;
	byte cBlockOrderIndex;
	byte cBlock;
	//unsigned int uiStart;
	int iPos;
	int len;
	td_STPsessions * p_STPSession = 0;

//Move through all the blocks and detect if the block has been modified since ulAfterTime
//If it has, then send back the block.
//Note that it's possible that history blocks may update while you are sending back the history,
//but all history updates are also alerted directly and they will be writen over top of this data.
//The data always makes it to the client and each message contains the last modified time according to the MiniFT.

//Don't check this flag here. Check before call.
//if (g_cSendOpHistory == 0)
//{
//	//nothing is being sent
//    return;
//}

	c = 0;
	cNotDone = 0;
	while (c < MAXNUM_STP_SESSIONS)
	{
		cBlockOrderIndex = g_cSendOpHistoryBlockOrderIndex[c];
#ifdef OUTPUT_OPHIST_VERBOSE
		logf("OH cBOI=%d:\r\n",cBlockOrderIndex);    //FIXME0 can remove soon
#endif
		if (cBlockOrderIndex < 0xFF)
		{
			//Load the time we need to check with
			//All blocks changed after this time need to be sent.
			ulAfterTime = g_ulSendOpHistoryAfterTime[c];
			//If index goes above block count it's done
			p_STPSession = GetSessionByNumber(c);

			while (cBlockOrderIndex < g_cBlockCount)
			{
				cBlock = g_cOpHistoryBlockOrder[cBlockOrderIndex];
#ifdef OUTPUT_OPHIST_VERBOSE
				logf("cB = %d\r\n",cBlock);
#endif

				//see if this block has been touched
				ulBlockTime = g_ulOpHistoryBlockTime[cBlock];
				if (ulBlockTime == 0)
				{
					//never touched.
					//There is no point in sending it.  The default value of the Pendant is 0 and is the same as the default value that is present here.
					//try another block right now
					//Performance of this is fine because it's a small set of blocks to check
					cBlockOrderIndex++;
					continue;
				}
				if (ulAfterTime > ulBlockTime)
				{
					//try another block right now
					//Performance of this is fine because it's a small set of blocks to check
					cBlockOrderIndex++;
					continue;
				}
				//Send back this block
				//Send block
				//Send Count (for case that block is not complete)
				//Send time
				//Send Data as copy of uint16 op data...
				//  This is not put into network order 1st
				//  Therefore, the target must take care to not treat them like an array of network ordered uint16 upon arrival.
				p_buffer = (char *) g_STPtxMsg.p_cObjectValue;
				*p_buffer++ = cBlock; //send block
				//Find starting position for this block.
				iPos = cBlock * 128;
#ifdef OUTPUT_OPHIST_VERBOSE
				logf("OH sb %d i=%d\r\n",cBlock,iPos);
#endif
				//Find len.
				len = g_iHoleCount - iPos;
				if (len > 128) //block is 128 holes
				{
					len = 128;
				}
				*p_buffer++ = len; //send length
				*((uint32 *) (p_buffer)) = ulBlockTime; //write time
				p_buffer += 4;
				len = len * 2; //each hole has 2 bytes of data.
				memcpy(p_buffer, ((char *) (g_uiOpHistory)) + (2 * iPos), len); //
				SmartToolMsg(p_STPSession, STP_ALERT, MINIFT_OID_OP_HISTORY, 6 + len, g_STPtxMsg.p_cObjectValue);

				//Advance to next block index
				cBlockOrderIndex++;
				//but break now, because we only send one block per call
				break;
			}
			if (cBlockOrderIndex < g_cBlockCount)
			{
				//this session is not done, so system is not done
				cNotDone = 1;
			}
			else
			{
				//must now be done
				//We can send back the CURRENT time now because if anything was updated during the sending, it would have been reflected by the block.
				//If a position was updated after a block was sent, then the position update would overwrite the block entery.
				//Send special packet Block = 0xFF Count = 0  Time = current time
				p_buffer = (char *) g_STPtxMsg.p_cObjectValue;
				*p_buffer++ = 0xFF; //send block
				*p_buffer++ = 0; //send length
				*((unsigned long *) (p_buffer)) = SEC_TIMER; //write time
				SmartToolMsg(p_STPSession, STP_ALERT, MINIFT_OID_OP_HISTORY, 6, g_STPtxMsg.p_cObjectValue);

				//set index to 0xFF
				cBlockOrderIndex = 0xFF;
#ifdef OUTPUT_OPHIST
				logf("OHD\r\n");
#endif
			}
			g_cSendOpHistoryBlockOrderIndex[c] = cBlockOrderIndex;
		}
		c++;
	}
//Retain Flag if Anything is still needed
	g_cSendOpHistory = cNotDone;
}

void SendRFIDData(td_STPsessions * p_STPSession, unsigned int uiMsgType)
{
	char * p_c;
	td_RFIDData * p_RFIDData;
	p_RFIDData = (td_RFIDData *) g_STPtxMsg.p_cObjectValue;
	p_RFIDData->cstate = g_RFIDData.cstate;
	p_RFIDData->ccontext = g_RFIDData.ccontext;
	p_RFIDData->cseekstate = g_RFIDData.cseekstate;
	p_RFIDData->ultimestamp = htonul(g_RFIDData.ultimestamp);
	p_RFIDData->ulrfidtimestamp = htonul(g_RFIDData.ulrfidtimestamp);
	p_RFIDData->fposition = g_RFIDData.fposition;
//bstring field sztagdata
	p_RFIDData->uicrc16 = htons(g_RFIDData.uicrc16);
	p_RFIDData->uiendcode = htons(g_RFIDData.uiendcode);
	p_RFIDData->ulseektime = htonul(g_RFIDData.ulseektime);
	p_RFIDData->fsstart = g_RFIDData.fsstart;
	p_RFIDData->fpstart = g_RFIDData.fpstart;
	p_RFIDData->fpend = g_RFIDData.fpend;
	p_RFIDData->fnstart = g_RFIDData.fnstart;
	p_RFIDData->fnend = g_RFIDData.fnend;
	p_RFIDData->fhs1 = g_RFIDData.fhs1;
	p_RFIDData->fhs2 = g_RFIDData.fhs2;
	p_RFIDData->fhsf = g_RFIDData.fhsf;
//Send BStrings after fixed size fields
	p_c = g_STPtxMsg.p_cObjectValue + sizeof(td_RFIDData);
	WriteBArray(p_c, g_szTagDatalen, g_szTagData);
	SmartToolMsg(p_STPSession, uiMsgType, MINIFT_OID_RFID_DATA,
			p_c - g_STPtxMsg.p_cObjectValue, g_STPtxMsg.p_cObjectValue);
}

void SendCurPosnInd(td_STPsessions * p_STPSession, unsigned int uiMsgType)
{
	int i;
	i = -1;
	if (g_PosnMode.iCurPosnIndex >= 0 && g_PosnMode.cOnCurPosn == 1)
	{
		i = g_PosnMode.iCurPosnIndex;
	}
	SmartToolMsgInt16(p_STPSession, uiMsgType, MINIFT_OID_POSNMODE_CURPOSN, i);
}

void SendNearPosnInd(td_STPsessions * p_STPSession, unsigned int uiMsgType)
{
	int i;
	i = -1;
	if (g_cProbeCalculated == 1)
	{
		i = FindNearestPosition();
	}
	SmartToolMsgInt16(p_STPSession, uiMsgType, MINIFT_OID_POSNMODE_NEARPOSN, i);
}

void SendGoalPosnInd(td_STPsessions * p_STPSession, unsigned int uiMsgType)
{
	if (g_PosnMode.iGotoPosnIndex >= 0)
	{
		SmartToolMsgInt16(p_STPSession, uiMsgType, MINIFT_OID_POSNMODE_GOALPOSN,
				g_PosnMode.iGotoPosnIndex);
	}
}

void SendCurXY(td_STPsessions * p_STPSession, unsigned int uiMsgType)
{
	float fx, fy, frx, fry;
	byte cm;
	byte co;
	td_oid_posnmode_curxy * p_oid_posnmode_curxy;

	cm = g_ConfigData.PosnDisplay.cmode;
// PD_MACHINE Use Machine Coordinates
// PD_DATASET Use Dataset Coordinates, possibly including scaling
// PD_DATASET_NS Use Dataset Coordinates, for orientation and zero, but do not scale them based on PK SK distance
// PD_NOTHING Don't show anything (returns Machine... and does not show them on UI)

	co = g_ConfigData.PosnDisplay.corigin;
// PD_ZERO   Use absolute XY in this mode (distance from 0,0 of this mode)
// PD_PK	 Use PK as the zero point (showing distance FROM pk)
// PD_SK	 Use SK as the zero point (showing distance FROM sk)
// PD_NEAREST USe Nearest as the zero point (showing distance FROM nearest)

//g_ConfigData.PosnDisplay.ccontent is not used here
//	PD_XY 0  Show XY on Pendant
//  PD_DIST 1 Show Dist using XY on Pendant (no work here is different)
//  PD_IDS 2  show the posn labels cooresponding with the Current, PK, SK or NEAREST
// All the work for these is on the pendant.  We send back XY all the time.

	fx = g_PosnMode.fLastKnownPosnX;
	fy = g_PosnMode.fLastKnownPosnY;

//FIXME0000000000 TEST AND then removed this if because I doubt this could ever happen
	if (fx > 10000 || fy > 10000)
	{
		//skip sending the position (which is not valid)
		//logf("INVALID Last Known Posn!!!!!!!!!!!!!!!!!\r\n");
		return;
	}

	if (cm == PD_MACHINE)
	{
		frx = fx;
		fry = fy;
		if (co != PD_ZERO)
		{
			if (co == PD_PK)
			{
//FIXME INCORRECT Not Implemented Yet
			}
			if (co == PD_SK)
			{
//FIXME INCORRECT Not Implemented Yet
			}
			if (co == PD_NEAREST)
			{
//FIXME INCORRECT Not Implemented Yet
			}
		}
	}
	else if (cm == PD_DATASET)
	{
		//FIXME0000000 New Multi Probe would require special rotate back to dataset.
//FIXME INCORRECT Not Implemented 100% correctly Yet
		RotateMachineToDataset(fx, fy, &frx, &fry);
		if (co != PD_ZERO)
		{
			if (co == PD_PK)
			{
//FIXME INCORRECT Not Implemented Yet
			}
			if (co == PD_SK)
			{
//FIXME INCORRECT Not Implemented Yet
			}
			if (co == PD_NEAREST)
			{
//FIXME INCORRECT Not Implemented Yet
			}
		}
	}
	else if (cm == PD_DATASET_NS)
	{
//FIXME INCORRECT Not Implemented 100% correctly Yet
		RotateMachineToDataset(fx, fy, &frx, &fry);
		if (co != PD_ZERO)
		{
			if (co == PD_PK)
			{
//FIXME INCORRECT Not Implemented Yet
			}
			if (co == PD_SK)
			{
//FIXME INCORRECT Not Implemented Yet
			}
			if (co == PD_NEAREST)
			{
//FIXME INCORRECT Not Implemented Yet
			}
		}
	}
	else if (cm == PD_NOTHING)
	{
		frx = fx;
		fry = fy;
	}

//initialize pointer to structure
	p_oid_posnmode_curxy = (td_oid_posnmode_curxy *) g_STPtxMsg.p_cObjectValue;
//set fields
//logf(" CUR %.3f,%.3f   M %.3f,%.3f\r\n",frx,fry,fx,fy);
	p_oid_posnmode_curxy->fDataSetX = frx;
	p_oid_posnmode_curxy->fDataSetY = fry;
	p_oid_posnmode_curxy->fMachineX = fx;
	p_oid_posnmode_curxy->fMachineY = fy;
//Send back to all sessions: This is a noted exception to GET_RESP currently.
//See notes about pending system removal
	SmartToolMsg(p_STPSession, uiMsgType, MINIFT_OID_POSNMODE_CURXY,
			sizeof(td_oid_posnmode_curxy), g_STPtxMsg.p_cObjectValue);
}

void SendNearXY(td_STPsessions * p_STPSession, unsigned int uiMsgType)
{
	td_oid_posnmode_nearxy * p_oid_posnmode_nearxy;
//these are upated to reflect the last nearposn as calculated in FindNearestPosition();
	if (g_PosnMode.iNearestPosn >= 0)
	{
		p_oid_posnmode_nearxy =
				(td_oid_posnmode_nearxy *) g_STPtxMsg.p_cObjectValue;
		p_oid_posnmode_nearxy->fDataSetX = g_PosnMode.fNearestPosnDX;
		p_oid_posnmode_nearxy->fDataSetY = g_PosnMode.fNearestPosnDY;
		p_oid_posnmode_nearxy->fMachineX = g_PosnMode.fNearestPosnMX;
		p_oid_posnmode_nearxy->fMachineY = g_PosnMode.fNearestPosnMY;
		p_oid_posnmode_nearxy->iNearPosn = htons(g_PosnMode.iNearestPosn); //set right before this is used
		SmartToolMsg(p_STPSession, uiMsgType, MINIFT_OID_POSNMODE_NEARXY,
				sizeof(td_oid_posnmode_nearxy), g_STPtxMsg.p_cObjectValue);
	}
}

void SendActivePremove(td_STPsessions * p_STPSession, unsigned int uiMsgType)
{
	float fx;
	float fy;
	byte c;
	td_oid_posnmode_active_premovexy * p_oid_posnmode_active_premovexy;
	p_oid_posnmode_active_premovexy =
			(td_oid_posnmode_active_premovexy *) g_STPtxMsg.p_cObjectValue;
	c = g_ConfigData.cMoveType;
	fx = 0;
	fy = 0;
	if (c == MOVETYPE_ORIGINAL || c == MOVETYPE_FAST)
	{
		CalculatePreMove(&fx, &fy);
	}
	p_oid_posnmode_active_premovexy->fX = fx;
	p_oid_posnmode_active_premovexy->fY = fy;
	SmartToolMsg(p_STPSession, uiMsgType, MINIFT_OID_POSNMODE_ACTIVE_PREMOVEXY,
			sizeof(td_oid_posnmode_active_premovexy),
			g_STPtxMsg.p_cObjectValue);
}

void SendSystemComponents(td_STPsessions * p_STPSession, unsigned int uiMsgType)
{
	td_SystemComponents * p_SystemComponents;
	p_SystemComponents = (td_SystemComponents *) g_STPtxMsg.p_cObjectValue;
	p_SystemComponents->cDrill = g_ConfigData.SystemComponents.cDrill;
	p_SystemComponents->cFastener = g_ConfigData.SystemComponents.cFastener;
	p_SystemComponents->cFastenerTray =
			g_ConfigData.SystemComponents.cFastenerTray;
	p_SystemComponents->cAux1 = g_ConfigData.SystemComponents.cAux1;
	p_SystemComponents->cAux2 = g_ConfigData.SystemComponents.cAux2;
	p_SystemComponents->cAux3 = g_ConfigData.SystemComponents.cAux3;
	p_SystemComponents->cAux4 = g_ConfigData.SystemComponents.cAux4;
	p_SystemComponents->cAux5 = g_ConfigData.SystemComponents.cAux5;
	SmartToolMsg(p_STPSession, uiMsgType, MINIFT_OID_SYSTEM_COMPONENTS,
			sizeof(td_SystemComponents), g_STPtxMsg.p_cObjectValue);
}

byte g_cProcStyle;

void LoadHoleParameters()
{
	//char buffer[16];
	byte cLayers;
	byte cProcLayers;
	byte cCountersink;
	byte cw;
	unsigned int ui;
	unsigned int uilayer;
	float f;
	float * fdata;

	//uses the global g_HoleData and loads many more thigns into g_HoleParam

	//First Set these globals
	g_cRequiredTool = g_HoleData.cTool;
	g_cRequiredProcess = g_HoleData.cProcess;

	//mask hole ops with all the ops the process allows... and allow inspection, seal, and fill if the hole wants it. ( The process does not care about those operations. )
	g_uiHoleOps = g_HoleData.uiOps & (g_cProcessOps[g_cRequiredProcess] | OP_INSPECT | OP_SEAL | OP_FILL | OP_REMOVE);

	//FIXME Operations-Limits  Tool used to be part of operations limit but it was removed.
	//       					do we need better operation limits?

	cLayers = g_cProcessLayerCount[g_cRequiredProcess];
	cProcLayers = g_cProcessProcLayerCount[g_cRequiredProcess];
	cCountersink = 0;
	if (g_uiHoleOps & OP_COUNTERSINK)
	{
		cCountersink = 1;
	}

	g_HoleParam.cProcess = g_cRequiredProcess;
	g_HoleParam.cToolType = g_cRequiredTool;
	g_HoleParam.cFastenerType = g_HoleData.cFastener;
	g_HoleParam.cLayers = cLayers;
	g_HoleParam.cCountersink = cCountersink;
	g_HoleParam.uiOperations = g_uiHoleOps;
	g_HoleParam.fDiameter = ((float) g_HoleData.d2uDiameter) / 10000;
	g_HoleParam.fProcessCountersink = ((float) g_uiProcessCountersink[g_cRequiredProcess]) / 10000;
	g_HoleParam.fHoleCountersinkAdjust = ((float) g_HoleData.iCountersinkAdjust) / 10000;
//FIXME666666666666 need print out for testing or breakpoints......
	logf("Layer Count = %d\r\n", cLayers);
	logf("plc %d csnk %d\r\n", cProcLayers, cCountersink);
	fdata = &g_HoleParam.flayer1;
	if (cLayers > 8)
	{
		logf("Bad Layer Count");
		cLayers = 1;
	}
	if (cLayers == 1)
	{
		g_HoleParam.flayer1 = ((float) g_HoleData.uiStack) / 10000;
		cw = 1;
	}
	else
	{
		ui = g_HoleData.uiStack * 2;
		cw = 0;
		while (cw < cLayers)
		{
			uilayer = *(uint16 *) (g_cStackData + ui);
			f = ((float) uilayer) / 10000;
			fdata[cw] = f;
			logf("%d %u\r\n", cw, f);
			ui += 2;
			cw++;
		}
	}
	while (cw < 8)
	{
		fdata[cw] = 0;
		cw++;
	}
//FIXME medium don't look these up yet
	g_HoleParam.cmat1 = 0; //FIXME medium
	g_HoleParam.cmat2 = 0;
	g_HoleParam.cmat3 = 0;
	g_HoleParam.cmat4 = 0;
	g_HoleParam.cmat5 = 0;
	g_HoleParam.cmat6 = 0;
	g_HoleParam.cmat7 = 0;
	g_HoleParam.cmat8 = 0;

	g_cProcStyle = g_cProcessStyle[g_cRequiredProcess];
}

void SendHoleParameters(td_STPsessions * p_STPSession, unsigned int uiMsgType)
{
	td_HoleParam xHoleParam;
	td_HoleParam * p_HoleParam;

	p_HoleParam = &xHoleParam;
	p_HoleParam->cProcess = g_HoleParam.cProcess;
	p_HoleParam->cToolType = g_HoleParam.cToolType;
	p_HoleParam->cFastenerType = g_HoleParam.cFastenerType;
	p_HoleParam->cLayers = g_HoleParam.cLayers;
	p_HoleParam->cCountersink = g_HoleParam.cCountersink;
	p_HoleParam->uiOperations = htons(g_HoleParam.uiOperations);
	p_HoleParam->fDiameter = g_HoleParam.fDiameter;
	p_HoleParam->fProcessCountersink = g_HoleParam.fProcessCountersink;
	p_HoleParam->fHoleCountersinkAdjust = g_HoleParam.fHoleCountersinkAdjust;
	p_HoleParam->flayer1 = g_HoleParam.flayer1;
	p_HoleParam->flayer2 = g_HoleParam.flayer2;
	p_HoleParam->flayer3 = g_HoleParam.flayer3;
	p_HoleParam->flayer4 = g_HoleParam.flayer4;
	p_HoleParam->flayer5 = g_HoleParam.flayer5;
	p_HoleParam->flayer6 = g_HoleParam.flayer6;
	p_HoleParam->flayer7 = g_HoleParam.flayer7;
	p_HoleParam->flayer8 = g_HoleParam.flayer8;
	p_HoleParam->cmat1 = g_HoleParam.cmat1;
	p_HoleParam->cmat2 = g_HoleParam.cmat2;
	p_HoleParam->cmat3 = g_HoleParam.cmat3;
	p_HoleParam->cmat4 = g_HoleParam.cmat4;
	p_HoleParam->cmat5 = g_HoleParam.cmat5;
	p_HoleParam->cmat6 = g_HoleParam.cmat6;
	p_HoleParam->cmat7 = g_HoleParam.cmat7;
	p_HoleParam->cmat8 = g_HoleParam.cmat8;

	SmartToolMsg(p_STPSession, uiMsgType, MINIFT_OID_HOLE_PARAMETERS, sizeof(td_HoleParam), (char*) p_HoleParam);
}

#ifdef SMARTDRILL_STP

//FIXME codeloc  where should this go if not here?
//This code calculates the override to send to the drill and is the start of a replacement
//of doing this on the pendant, however it operates in parallel to the pendant currently,
//and it doesn't send it to the drill.  This is for display only right now and perfecting the system
//not a true send yet...
//To perfect this, we must be able to calculate overrides and send them at the right times.
//this must be done when needed and compable to what the pendant does now
//I'm not sure why the pedant sends it some of the times that it does...
//It seems to even resend hole param after a tool load fail......
//FIXME severe
//This would need to be redone if certain tool parameters are changed...

//Another concern partially mentioned in other places is this:
//Override confirmation system....
//How does MiniFT know that this send has made it?
//One way is tht Drill will not drill unless getting it...
//but we are talking about improved handshaking in the future

//FIXME SEVERE move function locations for the 3 of these to the best combo for all of them
//currently LoadPRocess is near VerifyAndAlert but SendLayerOverride and CalculateLayerOverride are here

void CalculateLayerOverride()
{
	byte bl;
	byte bmateriallayercount;
	byte blayercount;
	byte bl_out;

	float fGaugeLength;
	float fStart;
	float fCountersinkStart;
	float fCountersinkEnd;
	float fTotalThickness;
	float fThick;
	float fLayer;
	float *p_fLayerThickness;
	td_LayerOverride * p_layer;

//assumptions:
//  GaugeLength is positive
//  Length is positive
//  CountersinkLength is positive
//  CountersinkAdjust is signed with - being deeper
//  Hole CountersinkAdjust is signed with - being deeper
//  ReturnLength is positive
//
//Gauge Length
//
//Default Gauge Length in The Pendant is 0, but we need the real value to operate.
//When getting connected to the drill, ask it for gauge length.
//When the gauge length is returned, remember it.
//FUTURE: when the value is set, also echo it to all
//When the connection is lost, wipe it to 0.
//When Setting Hole parameters, if it's 0, open message and do not mark parameters as set.
//
//RETURN_HEIGHT
//
//Default Return height in the pendant is 0, but we need the real value to operate.
//When getting connected to the MiniFt, ask it for the return height.
//When the value is returned, remember it.
//FUTURE: when the value is set, also echo it to all FIXME FUTURE
//When the connection is lost, wipe it to 0.
//When they hit home, if both values are known,
//Set the Home Back OID based on gauge length and return_height.

//Overview of sections of data toad
//Code related to setting drill parameters:
//1) Set HomeBack and Based on Return_Height
//2) Make sure gaugelength is loaded
//3) set minbreak right after loading a tool, as this is controlled by tool
//4) Set override on a per hole basis
//
//absolute positions:
//Start = - ( GaugeLength -  Length );    // Length is Tool Length
//
//CountersinkStart = - (GaugeLength - CountersinkLength) + CountersinkAdjust;
//
//CountersinkEnd = CountersinkStart - Countersink + Hole-CoutersinkAdjust;
//
//Layers start at 0
//Layers all active
//Layers Use all Y
//HArdstop = 0
//AbsEnd for 0 = Start
//Thick = ReturnLength
//All material layers Here
//AbsEnd for Transite =  CountersinkStart
//AbsEnd for csnk = CountersinkEnd
//
//No countersink: don't send last 2 layers like normal. Just send 1 marked Layers Use = N

	if (g_cToolLoaded == 0)
	{
		g_cOverrideCalculated=0;
		logf("tp not ready\r\n");
		return;
	}

	fGaugeLength = g_fGaugeLength;

	if (fGaugeLength == 0)
	{
		g_cOverrideCalculated=0;
		logf("FATAL:GL=0\r\n");
		return;
	}

//Do We have Hole Posn Info? Yes.
//This is always called when either on posn, or as moving to a posn.
//In all cases, we know the hole parameters by this time.

	logf("RH=%f\r\n",g_ConfigData.fReturnHeight);
	logf("GL=%f", fGaugeLength);
	logf(" T Len=%f", g_LoadedTool.fLength);
	logf(" T CLen =%f", g_LoadedTool.fLengthCountersink);

	fStart = - (fGaugeLength - g_LoadedTool.fLength);

	fCountersinkStart = 0;
	fCountersinkEnd = 0;
	if (g_LoadedTool.fLengthCountersink > 0)
	{
		fCountersinkStart = - (fGaugeLength - g_LoadedTool.fLengthCountersink) + g_LoadedTool.fCountersinkAdjust;
		if (g_HoleParam.cCountersink > 0)
		{
			fCountersinkEnd = fCountersinkStart - g_HoleParam.fProcessCountersink + g_HoleParam.fHoleCountersinkAdjust;

			if (g_LoadedTool.cHardstop == 0 && g_LoadedTool.fLengthCountersink == 0)
			{
				logf("TRH\r\n");
				return;
			}
		}
	}
//FIXME FUTURE consider using the ops to alow no countersink even when hole is set for it.
//if (_usCurrentParametersOperations & OP_COUNTERSINK)

	bmateriallayercount = g_HoleParam.cLayers;
	blayercount = bmateriallayercount;

//Find Total Thickness of layers here (used in multiple places below)
	if(bmateriallayercount>8)
	{	bmateriallayercount=8;} //FIXME000000 is this check needed?
	p_fLayerThickness = &g_HoleParam.flayer1;//in memory these are all inline like an array
	fThick = 0;
	fTotalThickness = 0;
	bl = 1;
	while(bl<=bmateriallayercount)
	{
		fThick = *p_fLayerThickness;
		logf("L=%d Th=%f\r\n",((int)bl), fThick);
		fTotalThickness += fThick;
		p_fLayerThickness++; //actually adds 4 because it knows it's a float pointer
		bl++;
	}

//Check the position we should reach during the material drilling
	fLayer = fStart - fTotalThickness - g_LoadedTool.fMinBreak;
	if (fCountersinkStart!=0)
	{
		if (fLayer < fCountersinkStart)
		{
			//Warning: Material Layers Deeper than Can be drilled without the Countersink Start reaching material
			SmartToolMsgMiniFtMessageCode(0, COMMON_OID_NULLOID, MINIFTMC_TOOL_SHORT_WARNING_COUNTERSINK_WILL_START);
			logf("*w %f < %f\r\n",fLayer,fCountersinkStart);
		}
		if (fCountersinkEnd!=0)
		{
			//should have valid Start And End
			if (fLayer < fCountersinkEnd)
			{
				//Failure: Material Layers Deeper than Can be drilled without maaking a countersink too deep
				logf("*f %f < %f\r\n",fLayer,fCountersinkEnd);
				SmartToolMsgMiniFtMessageCode(0, COMMON_OID_NULLOID, MINIFTMC_TOOL_SHORT_FAILURE);
				if (g_cTestOpt==4)
				{
					goto bypass_failure;
				}
				//turn off process to force stop
				g_cStartProcess = 0;//FIXME should be checked.
				return;
				bypass_failure:
			}
		}
	}

	if (g_cProcStyle == 0)
	{
		//Standard Layer Patterns

//FIXME: does this include the hole ops?????
		if (g_HoleParam.cCountersink > 0)
		{
			blayercount+=3; //add one for air, transit, and csnk
		}
		else
		{
			blayercount+=2; //add one for air and one for terminator
		}

//AIR Layer
		bl = 0;
		p_layer = g_LayerOverrides;
		p_layer->cLayerNumber = bl;
		p_layer->cOverrideActive = 1;
		p_layer->cUseHardStop = 0;
		p_layer->cUseThisLayer = 1;
		p_layer->fAbsoluteEnd = fStart;//use ABSOLUTE motion.
		p_layer->fThickness = g_ConfigData.fReturnHeight;//not used, but for reference. Drill May want this to be set
		p_layer++;//increase one layer override
		bl++;
		logf("S LO: Air Abs:%f", fStart);

		//Go through and create material layers using thicknesses
		p_fLayerThickness = &g_HoleParam.flayer1;//in memory these are all inline like an array
		while(bl<=bmateriallayercount)
		{
			p_layer->cLayerNumber = bl;
			p_layer->cOverrideActive = 1;
			p_layer->cUseHardStop = 0;
			p_layer->cUseThisLayer = 1;
			p_layer->fAbsoluteEnd = 0;
			fThick = *p_fLayerThickness;
			p_layer->fThickness = fThick;
			p_layer++; //increase one layer override

			logf("L=%d Th=%f\r\n",((int)bl), fThick);

			p_fLayerThickness++;//actually adds 4 because it knows it's a float pointer
			bl++;
		}

		if (g_HoleParam.cCountersink > 0)
		{
			//transit
			//Find Thickness
			//The total thickness from start to csnk start minus
			//the part we drilled above to find the transit thickness
			logf("Tot Th=%f\r\n", fTotalThickness);
			fThick = (-(fCountersinkStart - fStart)) - fTotalThickness;
			logf("Tran Th=%f\r\n", fThick);
			//Set Fields
			p_layer->cLayerNumber = bl;
			p_layer->cOverrideActive = 1;
			p_layer->cUseHardStop = 0;
			p_layer->cUseThisLayer = 1;
			p_layer->fAbsoluteEnd = fCountersinkStart;//use ABSOLUTE motion.
			p_layer->fThickness = fThick;//not used, but for reference. Drill May want this to be set
			p_layer++;//increase to next td_LayerOverride
			logf("L=%d cs=%f\r\n",((int)bl), fCountersinkStart);
			bl++;

			//csnk
			//Find Thickness
			fThick = -(fCountersinkEnd - fCountersinkStart);//Thickness of countersink
			//Set Fields
			p_layer->cLayerNumber = bl;
			p_layer->cOverrideActive = 1;
			p_layer->cUseHardStop = 0;
			if (g_LoadedTool.cHardstop > 0)
			{
				p_layer->cUseHardStop = 1;
			}
			p_layer->cUseThisLayer = 1;
			p_layer->fAbsoluteEnd = fCountersinkEnd; //use ABSOLUTE motion.
			p_layer->fThickness = fThick;//not used, but for reference. Drill May want this to be set
			logf("L=%d ce=%f\r\n",((int)bl), fCountersinkEnd);
			bl++;
		}
		else
		{
			//Set Fields to special value that will stop drilling at this layer
			//but to support the fact that process loading might use the override data to load a countersink process
			//that is going to be used for straight drilling now, but for countersink later, also set the memory with absolute ends.
			//In that case, the fake absolute ends here would be replaced by the code above using the tool information
			//if the process was later used for a real countersink hole.
			fLayer = fLayer - g_LoadedTool.fMinBreak;//set this end another min break down.
			p_layer->cLayerNumber = bl;
			p_layer->cOverrideActive = 1;
			p_layer->cUseHardStop = 0;
			p_layer->cUseThisLayer = 0;//SPECIAL TERMINATOR
			p_layer->fAbsoluteEnd = fLayer;
			p_layer->fThickness = 0;
			p_layer++;//increase to next td_LayerOverride
			bl++;

			//This layer will not be counted or sent.
			//It only exists to store AbsoluteEnd for the case mentioned in the comment above.

			fLayer = fLayer - g_LoadedTool.fMinBreak;//set this end another min break down.
			p_layer->cLayerNumber = bl;
			p_layer->cOverrideActive = 1;
			p_layer->cUseHardStop = 0;
			p_layer->cUseThisLayer = 0;//SPECIAL TERMINATOR ...
			p_layer->fAbsoluteEnd = fLayer;
			p_layer->fThickness = 0;
		}

	}
	else if (g_cProcStyle==1) //Using special 2 layer process mode.
	{

		//add teo for csnk+terms OR terminator
		if (g_HoleParam.cCountersink > 0)
		{
			blayercount = 3; //a two layer process always with terminator
			//If doing countersink, we want to do 1st layer all the way down to countersink start
			fThick = (-(fCountersinkStart - fStart));
		}
		else
		{
			blayercount = 2; //a single layer with terminator
			//If not doing coutnersink, we want the thickness to be all the material layers
			fThick = fTotalThickness;
		}
		logf("AMT-1 Th=%f\r\n",fThick);
		//try special 2 layer process
		//Layer 1 Air+Material+Transit
		//Layer 2 Countersink
		//AIR+Material

		bl=0;
		p_layer = g_LayerOverrides;
		p_layer->cLayerNumber = bl;
		p_layer->cOverrideActive = 1;
		p_layer->cUseHardStop = 0;
		p_layer->cUseThisLayer = 1;
		//go all the way from fStart down
		p_layer->fAbsoluteEnd = fStart - fThick;//use ABSOLUTE motion.
		p_layer->fThickness = g_ConfigData.fReturnHeight + fThick;//not used, but for reference. Drill May want this to be set
		p_layer++;//increase to next td_LayerOverride
		bl++;

		if (g_HoleParam.cCountersink > 0)
		{
			//csnk
			//Find Thickness
			fThick = -(fCountersinkEnd - fCountersinkStart);//Thickness of countersink
			//Set Fields
			p_layer->cLayerNumber = bl;
			p_layer->cOverrideActive = 1;
			p_layer->cUseHardStop = 0;
			if (g_LoadedTool.cHardstop > 0)
			{
				p_layer->cUseHardStop = 1;
			}
			p_layer->cUseThisLayer = 1;

			p_layer->fAbsoluteEnd = fCountersinkEnd; //use ABSOLUTE motion.
			p_layer->fThickness = fThick;//not used, but for reference. Drill May want this to be set
			p_layer++;//increase to next td_LayerOverride
			logf("L=%d ce=%f\r\n",((int)bl), fCountersinkEnd);
			bl++;
		}

		//Set Fields to special value that will stop drilling at this layer
		p_layer->cLayerNumber = bl;
		p_layer->cOverrideActive = 1;
		p_layer->cUseHardStop = 0;
		p_layer->cUseThisLayer = 0;//SPECIAL TERMINATOR
		p_layer->fAbsoluteEnd = 0;
		p_layer->fThickness = 0;
		bl++;
	}
	else if (g_cProcStyle==2) //Using Drill One Layer
	{

		//add teo for csnk+terms OR terminator
		if (g_HoleParam.cCountersink > 0)
		{
			blayercount = 2; //a single layer process
			//If doing countersink, we want to do 1st layer all the way down to countersink end
			fThick = (-(fCountersinkEnd - fStart));
		}
		else
		{
			blayercount = 2; //a single layer proces
			if(bmateriallayercount>8)
			{	bmateriallayercount=8;} //FIXME000000 is this check needed?
			p_fLayerThickness = &g_HoleParam.flayer1;//in memory these are all inline like an array
			//If not doing coutnersink, we want the thickness to be all the material layers
			fThick = 0;
			fTotalThickness = 0;
			bl = 1;
			while(bl<=bmateriallayercount)
			{
				fThick = *p_fLayerThickness;
				logf("L=%d",((int)bl));
				logf(" Thick:%f\r\n", fThick);

				fTotalThickness += fThick;
				p_fLayerThickness++; //actually adds 4 because it knows it's a float pointer
				bl++;
			}
			fThick = fTotalThickness;
		}
		logf("ONE-1 Thick=%f\r\n",fThick);
		//try special 1 layer process
		//Layer 1 Air+Material+Transit+Countersink

		bl=0;
		p_layer = g_LayerOverrides;
		p_layer->cLayerNumber = bl;
		p_layer->cOverrideActive = 1;
		p_layer->cUseHardStop = 0;
		if (g_LoadedTool.cHardstop > 0)
		{
			p_layer->cUseHardStop = 1;
		}
		p_layer->cUseThisLayer = 1;
		//go all the way from fStart down
		p_layer->fAbsoluteEnd = fStart - fThick;//use ABSOLUTE motion.
		p_layer->fThickness = g_ConfigData.fReturnHeight + fThick;//not used, but for reference. Drill May want this to be set
		bl++;

		logf("L=%d",((int)bl));
		logf(" CSNKEnd %f\r\n", fCountersinkEnd);
	}
	else
	{
		//not implemented....
		g_cOverrideCalculated = 0;
		return;
	}

	g_cLayerOverrides = bl;
	ShowLayerOverride();

	g_cOverrideCalculated = 1;
}

void SendLayerOverride()
{
	byte c;
	int i;
	td_LayerOverride * p_layer;

//DRILL_DIRECT_PROCESS_AND_OVERRIDE
//First Update the Hole Number
//Use the goal Posn Index Unless it's not set
	i = g_PosnMode.iGotoPosnIndex;
	if (i<0)
	{
		i=g_PosnMode.iCurPosnIndex;
	}

	if (g_DrillSTP.wLastState >= tcp_StateESTAB)
	{
		logf("hrs %d %d\r\n",g_PosnMode.iGotoPosnIndex,g_PosnMode.iCurPosnIndex);
		if (i>=0)
		{
			i=htons(i);
			SmartDrillSTPSetData(SMARTDRILL_OID_HOLE_NUMBER,2,(char *)&i);
		}
	}

//Copy Override Data to output buffer
	p_layer = (td_LayerOverride *)g_STPtxMsg.p_cObjectValue;

	c=0;
	while(c<g_cLayerOverrides)
	{
		p_layer->cLayerNumber = g_LayerOverrides[c].cLayerNumber;
		p_layer->cOverrideActive = g_LayerOverrides[c].cOverrideActive;
		p_layer->cUseHardStop = g_LayerOverrides[c].cUseHardStop;
		p_layer->cUseThisLayer = g_LayerOverrides[c].cUseThisLayer;
		p_layer->fAbsoluteEnd = htonfc(g_LayerOverrides[c].fAbsoluteEnd);
		p_layer->fThickness = htonfc(g_LayerOverrides[c].fThickness);
		p_layer++;
		c++;
	}

	if (g_DrillSTP.wLastState >= tcp_StateESTAB)
	{
		SmartDrillSTPSetData(SMARTDRILL_OID_LAYER_OVERRIDE,sizeof(td_LayerOverride)*g_cLayerOverrides,(char *)g_STPtxMsg.p_cObjectValue);
		logf("LO Sent \r\n");
		g_ulLO = MS_TIMER;
	}
	else
	{
		logf("LO Send Fail\r\n");
	}

	ShowLayerOverride();
}

void ShowLayerOverride()
{
	byte c;
	c=0;
	while(c<g_cLayerOverrides)
	{
		logf("%d %d %d %d %f %f\r\n",
				g_LayerOverrides[c].cLayerNumber,
				g_LayerOverrides[c].cOverrideActive,
				g_LayerOverrides[c].cUseHardStop,
				g_LayerOverrides[c].cUseThisLayer,
				g_LayerOverrides[c].fAbsoluteEnd,
				g_LayerOverrides[c].fThickness
		);
		c++;
	}
}
#endif

void SendReqToolSearch(unsigned int uiMsgType)
{
	byte c_op;
	byte c_arg1;
	if (g_ToolServerSTPSession == 0)
	{
		return;
	}
	c_op = search;
	c_arg1 = required_tool_type;
//Write MGMT out
//limit to tool server
	SendToolMGMT(g_ToolServerSTPSession, uiMsgType, c_op, c_arg1, 0, "", 0);
	return;
}

void SendToolMGMT(td_STPsessions * p_STPSession, unsigned int uiMsgType, byte c_op, byte carg1, unsigned int ui, char *s, int ilen)
{
	td_oid_tool_mgmt * p_oid_tool_mgmt;
	char * p_c;

	p_oid_tool_mgmt = (td_oid_tool_mgmt *) g_STPtxMsg.p_cObjectValue;
	p_oid_tool_mgmt->coperation = c_op;
	p_oid_tool_mgmt->carg1 = carg1;
	p_oid_tool_mgmt->uiarg2 = htons(ui);
//bstring field szsarg3
//Send BStrings or BArrays after fixed size fields
	p_c = ((char *) p_oid_tool_mgmt) + sizeof(td_oid_tool_mgmt);
//SPECIAL: If required tool type search, then lookup the string and write it here instead of copy input
	if (carg1 == required_tool_type)
	{
		//special path: copy the required tool right to p_c
		s = p_c + 1;
		LookupToolTypeString(s, g_cRequiredTool);
		g_cLastSearchedRequiredTool = g_cRequiredTool;
		if (*s == 0)
		{
			//nothing was there
			p_oid_tool_mgmt->carg1 = complete;
			logf("irt scequired tool\r\n");
			g_cLastSearchedRequiredTool = 0;
			ilen = 0;
		}
		else
		{
			ilen = strlen(s);
		}
	}
	WriteBString(p_c, ilen, s);
//Write out to tool server
	logf("tm %d\r\n", c_op);
	SmartToolMsg(p_STPSession, uiMsgType, TOOLMANAGEMENT_OID_TOOL_MGMT, p_c - ((char*) p_oid_tool_mgmt), ((char*) p_oid_tool_mgmt));
}

void SendTool(td_STPsessions * p_STPSession, unsigned int uiMsgType,
		byte cOperation)
{
	td_oid_tool_rec * p_LoadedTool;
	char * p_c;
	p_LoadedTool = (td_oid_tool_rec *) g_STPtxMsg.p_cObjectValue;
	p_LoadedTool->cOperation = cOperation;
	p_LoadedTool->cToolStatus = g_LoadedTool.cToolStatus;
	p_LoadedTool->cHardstop = g_LoadedTool.cHardstop;
	p_LoadedTool->fDiameter = g_LoadedTool.fDiameter;
	p_LoadedTool->fLength = g_LoadedTool.fLength;
	p_LoadedTool->fMinBreak = g_LoadedTool.fMinBreak;
	p_LoadedTool->fLengthCountersink = g_LoadedTool.fLengthCountersink;
	p_LoadedTool->fCountersinkAdjust = g_LoadedTool.fCountersinkAdjust;
	p_LoadedTool->ulDTimeTicksMSW = htonul(g_LoadedTool.ulDTimeTicksMSW);
	p_LoadedTool->ulDTimeTicksLSW = htonul(g_LoadedTool.ulDTimeTicksLSW);
	p_LoadedTool->uiDCount = htons(g_LoadedTool.uiDCount);
	p_LoadedTool->uiDWarnCount = htons(g_LoadedTool.uiDWarnCount);
	p_LoadedTool->uiDLimitCount = htons(g_LoadedTool.uiDLimitCount);
//bstring field szID
//bstring field szToolTypeCode
//Send BStrings after fixed size fields
	p_c = ((char *) p_LoadedTool) + sizeof(td_oid_tool_rec);
	WriteBString(p_c, g_szToolIDlen, g_szToolID);
	WriteBString(p_c, g_szToolTypeCodelen, g_szToolTypeCode);
	logf("st\r\n");	//asdfggggggg
	logf("-+ %d \"%s\" %d \"%s\"\r\n", g_szToolIDlen, g_szToolID, g_szToolTypeCodelen, g_szToolTypeCode);
	SmartToolMsg(p_STPSession, uiMsgType, TOOLMANAGEMENT_OID_TOOL_REC, p_c - ((char *) p_LoadedTool), ((char *) p_LoadedTool));
}

#ifdef TOOL_IN_RAM
void SaveToolToRam()
{
//FIXME IMMEDIATE
	logf("Saved Tool\r\n");
}

void LoadToolFromRam()
{
	byte cvalid;
	cvalid = 0;
//FIXME IMMEDIATE
	if (g_LoadedTool.cToolStatus == FIXME)
	{
		//It was the null tool, so just clear the memory again
		logf("tool type was 0");
	}
	else if (g_LoadedTool.fLength < 0 || g_LoadedTool.fLength > 20)
	{
		logf("tool failed length test");
	}
	else
	{
		cvalid = 1;
		logf("Loaded Tool\r\n");
	}
	if (cvalid == 0)
	{
		//clear the memory copy
		g_LoadedTool.cOperation = 0;
		g_LoadedTool.cToolStatus = 0;
		g_LoadedTool.cHardstop = 0;
		g_LoadedTool.fDiameter = 0;
		g_LoadedTool.fLength = 0;
		g_LoadedTool.fMinBreak = 0;
		g_LoadedTool.fLengthCountersink = 0;
		g_LoadedTool.fCountersinkAdjust = 0;
		g_cLoadedTool = 0;
		g_szToolIDlen = 0;
	}
	logf("Tool: t %d h %d dia %f len %f minb %f lencsnk %f csnkadj %f\r\n",
			g_LoadedTool.cToolStatus,
			g_LoadedTool.cHardstop,
			g_LoadedTool.fDiameter,
			g_LoadedTool.fLength,
			g_LoadedTool.fMinBreak,
			g_LoadedTool.fLengthCountersink,
			g_LoadedTool.fCountersinkAdjust);
//FIXME this would not work as currently done because we never added the strings to the memory image for battery backed ram...
//Currently there is no batter, so this will not work and is not included in the build

}
#endif

typedef struct
{
	char Type[32];
	char PendantFilename[64];
	float fMinBreak;
	float fDiam;
	int iStallCountLimit;
	float fStallDensityLimit;
	float fStallZoneLimit;
	float fThrustLimitTi;
	float fThrustLimitCf;
} td_CutterType;

typedef struct
{
	char SN[32];
	unsigned int uiCyclesOnCutter;
	int iHighestStallCount;
	float fHighestStallDensity;
	float fHighestStallZone;
	float fAvgThrustTi;
	float fAvgThrustCf;
} td_Cutter;
//asdfggggg<<<<<<<<<<<<<<Does it ever set cutter?

void LoadToolToDrill()
{
#ifdef DRILL_DIRECT_READY
	byte c;
	td_CutterType * p_CutterType;

	if (g_szToolIDlen == 0)
	{
		//Clear tool and use default home back
#ifdef DRILL_DIRECT_CUTTER_OIDS
		logf("cr\r\n");
		SmartDrillSTPSet(SMARTDRILL_OID_CUTTER_RESET);
#endif
	}
	else
	{
		logf("cl mb%f\r\n",g_LoadedTool.fMinBreak);
		//Load Cutter Type
		memset(g_STPtxMsg.p_cObjectValue,0,sizeof(td_CutterType));
		p_CutterType=(td_CutterType *)g_STPtxMsg.p_cObjectValue;
		c = g_szToolIDlen;
		if (c>31)
		{	c=31;}
		memcpy(p_CutterType->Type,g_szToolID,c);
		p_CutterType->fMinBreak = ntohfc(g_LoadedTool.fMinBreak);
		p_CutterType->fDiam = ntohfc(g_LoadedTool.fDiameter);
		//leave this at 0 ... no stall recovery tries.
		//p_CutterType->iStallCountLimit=0x7F7F;
		//leave other fields at 0
#ifdef DRILL_DIRECT_CUTTER_OIDS
		SmartDrillSTPSetData(SMARTDRILL_OID_CUTTER_TYPE,sizeof(td_CutterType),(char *)g_STPtxMsg.p_cObjectValue);
#endif
	}
#endif
}

void LoadToolHomeBackToDrill()
{
#ifdef DRILL_DIRECT_READY
	float fHomeBack;

	logf("hb chk\r\n");
	if (g_cDrillSync == DRILL_SYNC)
	{
		fHomeBack = 0.040;
		if (g_szToolIDlen > 0)
		{
			//Set Home Backup
			fHomeBack = (g_fGaugeLength - g_LoadedTool.fLength) - g_ConfigData.fReturnHeight;
		}
		g_fHomeBack = fHomeBack;

		logf("hb %f\r\n",fHomeBack);
		writeFloatToBufferNTOHFC(g_STPtxMsg.p_cObjectValue,fHomeBack);
		SmartDrillSTPSetData(SMARTDRILL_OID_HOME_BACKUP,4,(char *)g_STPtxMsg.p_cObjectValue);
		g_fLastSentHomeBack = fHomeBack;
	}
#endif
}

void RepositionDrill()
{
#ifdef DRILL_DIRECT_READY
	byte c;
	float fHomeBack;

	logf("hbch\r\n");
	if (g_cDrillSync == DRILL_SYNC)
	{
		fHomeBack = g_fHomeBack;
#ifdef DRILL_DIRECT_HOME_AFTER_HOME_BACK_SET
		if (g_cDrillHomed==HOME_NOT_DONE)
		{
			//Do not home the drill because it was not previously homed.
			//In should be homed from the Home Button.
		}
		else if (g_cDrillHomed==HOME_RUNNING || g_cDrillState==DRILLSTATE_HOME)
		{
			//Abort the current home and issue a new one
			SmartDrillSTPSet(SMARTDRILL_OID_ABORT);
			logf("bha\r\n");
			goto issue_home_now;
		}
		else if (g_cDrillState == DRILLSTATE_IDLE || g_cDrillState == DRILLSTATE_NOT_HOME || g_cDrillState == DRILLSTATE_JOG)
		{
			issue_home_now:
			//Issue a Home Now....
			g_cDrillStateGoal = DRILLSTATE_HOME;//request a home
			g_cDrillStateGoalCommanded = DRILLSTATE_IDLE;//will cause it to resend
			logf("dsgh%d %f\r\n",g_cDrillState,g_fLastSentHomeBack);
		}
#endif
	}
#endif
}

byte CheckObstructionsAndMotionLimits(float fX, float fY)
{
	if (g_cObstructionCode != 0)
	{
		//Check Individual Sensors
#ifdef Y_LIMIT_SENSORS
		if (g_cDigInYPosLimit==0)
		{
			if (fY>g_PosnMode.fLastKnownPosnY)
			{	goto would_move_toward_obstruction;}
		}
		if (g_cDigInYNegLimit==0)
		{
			if (fY<g_PosnMode.fLastKnownPosnY)
			{	goto would_move_toward_obstruction;}
		}
#endif
#ifdef OBSTRUCTION_SYSTEM_XP1
		if (g_cDigInObstructionXP1 == OBSTRUCTION)
		{
			//X+
			if (fX > g_PosnMode.fLastKnownPosnX)
			{
				goto would_move_toward_obstruction;
			}
		}
#endif
#ifdef OBSTRUCTION_SYSTEM_XN1
		if (g_cDigInObstructionXN1 == OBSTRUCTION)
		{
			//X-
			if (fX < g_PosnMode.fLastKnownPosnX)
			{
				goto would_move_toward_obstruction;
			}
		}
#endif
#ifdef OBSTRUCTION_SYSTEM_MOS
		if (g_cDigInObstructionMOS == MO_OBSTRUCTION)
		{
			goto would_move_toward_obstruction;
		}
#endif
#ifdef OBSTRUCTION_SYSTEM_MO_ANA
		if (g_cMOFlags > 0)
		{
			goto would_move_toward_obstruction;
		}
#endif
	}
//No Obstruction currently detected...
//Check if movement would be out of range
#ifdef CHECK_X_MOVE_OUT_OF_RANGE
	if (fX > g_fPositionMaxX)
	{
		goto would_move_out_of_range;
	}
	if (fX < g_fPositionMinX)
	{
		goto would_move_out_of_range;
	}
#endif
#ifdef CHECK_Y_MOVE_OUT_OF_RANGE
	if (fY > g_fPositionMaxY)
	{
		goto would_move_out_of_range;
	}
	if (fY < g_fPositionMinY)
	{
		goto would_move_out_of_range;
	}
#endif
	return 0;
	would_move_out_of_range:
//can't do this move
#ifdef USE_OUTPUT
	logf("Mvbd");	//bad dir
#endif
	SmartToolMsgMiniFtMessageCode(0, COMMON_OID_NULLOID, MINIFTMC_MOVE_WOULD_GO_OUT_OF_RANGE);
	return 1;
	would_move_toward_obstruction:
//can't do this move
#ifdef USE_OUTPUT
	logf("Mvto");	//mv toward obstruction
#endif
	AlertObstructionCode(0);
	return 1;
}

void AlertObstructionCode(td_STPsessions * p_STPSession)
{
	unsigned long ul;
#ifdef OUTPUT_OBSTRUCTION_CODE
	logf("Aoc=%x\r\n",g_cObstructionCode);
#endif
	ul = g_cObstructionCode;
	SmartToolMsgUInt32(p_STPSession, STP_ALERT, MINIFT_OID_LIMITS_AND_OBSTRUCTIONS, ul);
}

//marker
void AlertObstructionWarningCode(td_STPsessions * p_STPSession)
{
	unsigned long ul;
#ifdef OUTPUT_OBSTRUCTION_CODE
	logf("Aowc=%x\r\n",g_cObstructionWarningCode);
#endif
	ul = g_cObstructionWarningCode;
	SmartToolMsgUInt32(p_STPSession, STP_ALERT,
			MINIFT_OID_LIMITS_AND_OBSTRUCTION_WARNINGS, ul);
}

void UpdateStationPlan()
{
#ifdef MINIFT_TRIAL_STATION_CODE_USE_FIRST_HOLE_OP
//USE NEW CODE THAT USES FIRST SELECTED STATION THAT IS ALSO ALLOWED ON THIS HOLE
//First set start, which is only based on selection right now
	if ( (g_ConfigData.uiProcessOperations & OP_DRILL) && (g_uiHoleOps & OP_DRILL) )
	{
		g_cStartStation = STATION_DRILL;
	}
//INSPECT IS NEVER THE START STATION  AT THIS POINT BECAUSE IT'S ALWAYS AUTHORIZED...
//	else if ( (g_ConfigData.uiProcessOperations & OP_INSPECT) && (g_uiHoleOps & OP_INSPECT) )
//	{
//		g_cStartStation = STATION_INSPECT;
//	}
#ifdef SEAL_SYSTEM
	else if ( (g_ConfigData.uiProcessOperations & OP_SEAL) && (g_uiHoleOps & OP_SEAL) )
	{
		g_cStartStation = STATION_SEAL;
	}
#endif
#ifdef FASTENER_SYSTEM
	else if ( (g_ConfigData.uiProcessOperations & OP_FILL) && (g_uiHoleOps & OP_FILL) )
	{
		if (g_cUseFastenerTray==1)
		{
			g_cStartStation = STATION_PICKUP;
		}
		else
		{
			g_cStartStation = STATION_FILL;
		}
#warns "DON'T FORGET TO ALSO TAKE CARE OF FULL CYCLE MODE FOR INSERT<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<"
	}
	else if ( (g_ConfigData.uiProcessOperations & OP_REMOVE) && (g_uiHoleOps & OP_REMOVE) )
	{
		g_cStartStation = STATION_FILL;
	}
#endif
#else
//USE ORIGINAL CODE THAT USES FIRST SELECTED AND IGNORES HOLE OPS
//First set start, which is only based on selection right now
	if (g_ConfigData.uiProcessOperations & OP_DRILL)
	{
		g_cStartStation = STATION_DRILL;
	}
//Inspect is never a valid start station in this version... operations for inspect don't work this way... station will be directly set to insepect if needed
//	else if (g_ConfigData.uiProcessOperations & OP_INSPECT)
//	{
//		g_cStartStation = STATION_INSPECT;
//	}
#ifdef SEAL_SYSTEM
	else if (g_ConfigData.uiProcessOperations & OP_SEAL)
	{
		g_cStartStation = STATION_SEAL;
	}
#endif
#ifdef FASTENER_SYSTEM
	else if (g_ConfigData.uiProcessOperations & OP_FILL)
	{
		if (g_cUseFastenerTray==1)
		{
			g_cStartStation = STATION_PICKUP;
		}
		else
		{
			g_cStartStation = STATION_FILL;
		}
#warns "DON'T FORGET TO ALSO TAKE CARE OF FULL CYCLE MODE FOR INSERT<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<"
	}
	else if (g_ConfigData.uiProcessOperations & OP_REMOVE)
	{
		g_cStartStation = STATION_FILL;
	}
#endif
#endif
	else
	{
		//default to going here
		g_cStartStation = STATION_DRILL;
	}

//Now set plan flags
//Drill
	g_cStationPlanDrill = 0;
	if (g_ConfigData.uiProcessOperations & OP_DRILL)
	{
		if (g_uiHoleOps & OP_DRILL)
		{
			g_cStationPlanDrill = 1;
		}
	}
//Inspect

	g_cStationPlanInspect = 0;
	if (g_cPositionInspection == 1) //allow inspection
	{
		g_cStationPlanInspect = 1;
		goto past_inspect_plan;
	}
	if (g_ConfigData.uiProcessOperations & OP_INSPECT)
	{
		if (g_uiHoleOps & OP_INSPECT)
		{
			g_cStationPlanInspect = 1;
			goto past_inspect_plan;
		}
	}
	past_inspect_plan:
#ifdef SEAL_SYSTEM
//Seal
	g_cStationPlanSeal = 0;
	if (g_ConfigData.uiProcessOperations & OP_SEAL)
	{
		if (g_uiHoleOps & OP_SEAL)
		{
			g_cStationPlanSeal = 1;
		}
	}
#endif
#ifdef FASTENER_SYSTEM
//Fill
	logf(" OP_FILL %u  pop %u hop %u \r\n",OP_FILL,g_ConfigData.uiProcessOperations, g_uiHoleOps);

	g_cStationPlanFill = 0;
	if (g_ConfigData.uiProcessOperations & OP_FILL)
	{
		if (g_uiHoleOps & OP_FILL)
		{
			g_cStationPlanFill = 1;
			logf("%c\r\n",'f');
		}
	}
	g_cStationPlanRemove = 0;
	if (g_ConfigData.uiProcessOperations & OP_REMOVE)
	{
		if (g_uiHoleOps & OP_REMOVE)
		{
			g_cStationPlanRemove = 1;
			logf("%c\r\n",'R');
		}
	}
	if (g_cStationPlanFill==1) //do not allow remove to be in plan if fill is //FIXME URGENT  Better in future to allow stage so it can have both in sequence only
	{
		g_cStationPlanRemove=0;
	}
#endif
	logf("%c\r\n", g_cStationCode[g_cStartStation]);
	logf("u=%u\r\n", g_ConfigData.uiProcessOperations);
}

//Just take the next step in stations, and then let advancestations work this out.
void NextStation()
{
	logf("ns\r\n");
	logf("%c\r\n", g_cStationCode[g_cStationGoal]);
	if (g_cStationGoal == STATION_DRILL)
	{
		g_cStationGoal = STATION_INSPECT;
	}
	else if (g_cStationGoal == STATION_INSPECT)
	{
#ifdef SEAL_SYSTEM
		g_cStationGoal = STATION_SEAL;
#else
#ifdef FASTENER_SYSTEM
		g_cStationGoal = STATION_PICKUP;
#else
		g_cStationGoal = STATION_UNSPEC;
#endif
#endif
	}
#ifdef SEAL_SYSTEM
	else if (g_cStationGoal==STATION_SEAL)
	{
		g_cStationGoal = STATION_PICKUP;
	}
#endif
#ifdef FASTENER_SYSTEM
	else if (g_cStationGoal==STATION_PICKUP)
	{
		g_cStationGoal = STATION_FILL;
	}
	else if (g_cStationGoal==STATION_FILL)
	{
		g_cStationGoal = STATION_UNSPEC;
	}
#endif
	logf("%c\r\n", g_cStationCode[g_cStationGoal]);
}

void AdvanceStations()
{
	logf("as\r\n");
	logf("%c\r\n", g_cStationCode[g_cStationGoal]);
	if (g_cStationGoal == STATION_DRILL)
	{
		if (g_cStationPlanDrill == 0)
		{
			g_cStationGoal = STATION_INSPECT;
		}
		logf("  %c\r\n", g_cStationCode[g_cStationGoal]);
	}
	if (g_cStationGoal == STATION_INSPECT)
	{
#ifdef SEAL_SYSTEM
		if (g_cStationPlanInspect==0)
		{	g_cStationGoal = STATION_SEAL;}
#else
#ifdef FASTENER_SYSTEM
		if (g_cStationPlanInspect==0)
		{	g_cStationGoal = STATION_PICKUP;}
#else
		if (g_cStationPlanInspect == 0)
		{
			g_cStationGoal = STATION_UNSPEC;
		}
#endif
#endif
		logf("  %c\r\n", g_cStationCode[g_cStationGoal]);
	}
#ifdef SEAL_SYSTEM
	if (g_cStationGoal==STATION_SEAL)
	{
		if (g_cStationPlanSeal==0)
		{	g_cStationGoal = STATION_PICKUP;}
		logf("  %c\r\n",g_cStationCode[g_cStationGoal]);
	}
#endif
#ifdef FASTENER_SYSTEM
	if (g_cStationGoal==STATION_PICKUP)
	{
		if (g_cFastenerState==FASTENERSTATE_LOADED)
		{
			g_cStationGoal = STATION_FILL;
			logf("  %c\r\n",'>');
		}

		if (g_cStationPlanFill==0 || g_cUseFastenerTray==0)
		{
			g_cStationGoal = STATION_FILL;
		}
	}
	if (g_cStationGoal==STATION_FILL)
	{
		if (g_cFastenerState==FASTENERSTATE_NONE)
		{
			if (g_cStationPlanFill==1 && g_cUseFastenerTray==1)
			{
				g_cStationGoal = STATION_PICKUP;
				logf("  %c\r\n",'<');
			}
		}
		if (g_cStationPlanFill==0 && g_cStationPlanRemove==0)
		{	g_cStationGoal = STATION_UNSPEC;}
		logf("  %c\r\n",g_cStationCode[g_cStationGoal]);
	}
#endif
	logf("%c\r\n", g_cStationCode[g_cStationGoal]);
}

void PrepareStations()
{
	if (g_cStartProcess == 1)
	{
		//running the process
		//FIXME0000 note: in the future it will be more about what station is set than who you are.....
		//    So there will need to be less identity checking
		if (g_cEEOption != EENONE)
		{
			//For GENHD, only tool check when process is running //FIXME00000 confirm this and consider move comment
			if (g_cStation == STATION_DRILL)
			{
				if (g_cStationPlanDrill == 1)
				{
					if (g_DrillFault.cSeverity >= FAULT_SEVERITY_ALARM)
					{
						//Do Not Continue Action
						g_cDrillStateGoal = DRILLSTATE_IDLE;
					}
#warning "This must cause abort"
//FIXME SEVERE  these could be stale from last load???????
//unless I adopt the pattern that they are cleared at the start of each move to position
					else if (g_cToolLoaded != 1)
					{
						//not ready
					}
					//DRILL_DIRECT_PROCESS_AND_OVERRIDE
					else if (g_cDrillLoadProcessAndOverride
							== DRILL_LOAD_PROCESS_DONE)
					{
						//It's Ready
						if (g_cPosnModeState >= POSNMODE_FINALMOVE)
						{
							//ALWAYS DO SPIN UP HERE
							g_cDrillStateGoal = DRILLSTATE_SPINUP;
							g_ulSpinUp = MS_TIMER;
						}
						if (g_cPosnModeState >= POSNMODE_ACTION)
						{
#ifdef SEAL_SYSTEM
							if (g_cStationPlanSeal==1)
							{
								if (g_cSealState==SEALSTATE_OFF)
								{
									//just start pressure
									g_cSealState=SEALSTATE_PRESSURE;
								}
							}
#endif
						}
					}
				}
				else
				{
					//Not planning to drill now
					logf("st:nptd\r\n");
				}
			}
#ifdef SEAL_SYSTEM
			if (g_cStation==STATION_SEAL && g_cStationPlanSeal==1)
			{
				if (g_cSealState==SEALSTATE_OFF || g_cSealState==SEALSTATE_PRESSURE)
				{
					//start autoprime
					g_cSealState=SEALSTATE_AUTOPRIME;
				}
			}
#endif
#ifdef FASTENER_SYSTEM
			if (g_cStation>=STATION_SEAL && g_cStation<=STATION_FILL && g_cStationPlanFill==1)
			{
//FIXME unsure of what to do when advance here....
				//Nothing to do for this yet...
				//if (g_cFillState==FILLSTATE_OFF)
				//{
				//	//start fill
				//	g_cFillState=FILLSTATE_FASTENER_REQUEST;
				//}
			}
#endif
		}
	}
}

void StopProcess(void)
{
	td_STPsessions * p_STPSession;

	g_cStartProcess = 0;
	g_cAutoMove = 0;
//Clear Any Pending Move Trigger
	g_PosnMode.cDoMoveTrig = FALSE;

	if (g_cPosnModeState < POSNMODE_ACTION
			|| g_cPosnModeState > POSNMODE_ACTION_COMPLETE) //do I need g_cPosnModeState != POSNMODE_WAITNEXTPOSN too???
	{
		g_cPosnModeState = POSNMODE_STOP_MOVEMENT;
	}
	g_cDrillStateGoal = DRILLSTATE_IDLE;
#ifdef SEAL_SYSTEM
	g_cSealState=SEALSTATE_OFF;
#endif
#ifdef FASTENER_SYSTEM
	g_cFillState=FILLSTATE_OFF;
	g_cFastenerArrived=0; //allow clearance, but hope it will redetect it
#endif
//Echo the Stop Back to the clients
//FIXME dfnow  may not be needed when everything is run here.
	SmartToolMsgEmpty(0, STP_ALERT, MINIFT_OID_PROCESS_STOP); //FIXME:share:tom
}

//////////////////////////////////////////////////////////////////////
// Tool Lookup
//////////////////////////////////////////////////////////////////////
#define SHOW_ENTIRE_LIST
byte LookupToolTypeCode(char * tooltype)
{
	char buffer[256];
	int ibase;
	int i;
	int iw;
	int b;
	byte ctool;
	byte ctoolmatch;
	char * p_c;

	if (g_pcToolNames == 0 || g_uiToolNamesLen == 0)
	{
		return 0;
	}
//Comment this out to show the entire list even when no tool has yet been loaded
//	if (*tooltype == 0)
//	{
//		return 0;
//	}

	ctoolmatch = 0;
	ctool = 0;
	ibase = 0;
	logf("c %d %d\r\n", g_cToolCount, g_uiToolNamesLen);
	while (ibase < g_uiToolNamesLen)
	{
		b = g_uiToolNamesLen - ibase;
		if (b > 64)
		{
			b = 64;
		}
		logf("c %d %d\r\n", ibase, b);
		memcpy(buffer, g_pcToolNames + ibase, b);
		i = 0;
		iw = 0;
		p_c = buffer;
		while (i < b)
		{
			if (buffer[i] == '\r')
			{
				buffer[i] = 0;
				ctool++;
				logf("%d s=%s\r\n", ctool, p_c);
				if (strcmp(tooltype, p_c) == 0)
				{
					//found the tool
					ctoolmatch = ctool;
					logf("@@@\r\n");
#ifndef SHOW_ENTIRE_LIST
					goto found_tool_type_code;
#endif
				}
				if (ctool >= g_cToolCount)
				{
					goto no_tool_type_code;
				}
				i += 2;
				p_c = buffer + i;
				iw = i;
				continue;
			}
			i++;
		}
		ibase += iw; //move the base up
	}
	no_tool_type_code:
	#ifndef SHOW_ENTIRE_LIST
	found_tool_type_code:
#endif
	return ctoolmatch;
}

void LookupToolTypeString(char * tooltypeout, byte ctoolsearch)
{
	char buffer[256];
	int ibase;
	int i;
	int iw;
	int b;
	byte ctool;
	byte ctoolmatch;
	char * p_c;

	if (g_pcToolNames == 0 || g_uiToolNamesLen == 0)
	{
		goto no_tool_type_code;
	}
	if (ctoolsearch == 0)
	{
		goto no_tool_type_code;
	}
	ctoolmatch = 0;
	ctool = 0;
	ibase = 0;
	while (ibase < g_uiToolNamesLen)
	{
		b = g_uiToolNamesLen - ibase;
		if (b > 64)
		{
			b = 64;
		}
		memcpy(buffer, g_pcToolNames + ibase, b);
		i = 0;
		iw = 0;
		p_c = buffer;
		while (i < b)
		{
			if (buffer[i] == '\r')
			{
				buffer[i] = 0;
				ctool++;
				if (ctool == ctoolsearch)
				{
					logf("%d s=%s\r\n", ctool, p_c);
					//found the tool
					strcpy(tooltypeout, p_c);
					return;
				}
				if (ctool >= g_cToolCount)
				{
					goto no_tool_type_code;
				}
				i += 2;
				p_c = buffer + i;
				iw = i;
				continue;
			}
			i++;
		}
		ibase += iw; //move the base up
	}
	no_tool_type_code: *tooltypeout = 0;
}

//////////////////////////////////////////////////////////////////////
// Tool Sync
//////////////////////////////////////////////////////////////////////
void VerifyAndAlertTool()
{
	g_cToolLoaded = 1;
	if (g_cRequiredTool > 0)
	{
		if (g_cLoadedTool != g_cRequiredTool)
		{
			//Tool Failed
			g_cToolLoaded = 0;
			//Alert HoleParam
			SendHoleParameters(0, STP_ALERT);
			//..... the alert of parameters in this case is more to just make sure the pendant knows...
			//Alert Tool
			SendTool(0, STP_ALERT, improper_type);
		}
		else if (g_LoadedTool.uiDCount >= g_LoadedTool.uiDLimitCount
				&& g_LoadedTool.uiDLimitCount > 0)
		{
			//Tool Failed Because of Limit
			g_cToolLoaded = 0;
			//Alert Tool
			SendTool(0, STP_ALERT, improper_type); //FIXME FUTURE Create another type for this message
//marker
		}
	}
}

void VerifyAndAlertProcess()
{
	g_cProcessLoaded = 1;
	if (g_cRequiredProcess > 0)
	{
		if (g_cLoadedProcess != g_cRequiredProcess)
		{
			//Process Failed
			g_cProcessLoaded = 0;
			//Alert HoleParam
			SendHoleParameters(0, STP_ALERT);
			//Alert Process
			SmartToolMsgChar(0, STP_ALERT, MINIFT_OID_PROCESS,
					g_cLoadedProcess);
		}
	}
}

void LoadProcessAndOverride()
{
//DRILL_DIRECT_PROCESS_AND_OVERRIDE
	if (g_cToolLoaded != 0)
	{
		//proper tool was loaded

#ifdef DRILLFILL_STP
		if (g_cEEOption==EEDRILLFILL)
		{
			//There is no such thing as a layer override for DSI...  Instead of trying to fit new system, simply use this basic system
			if (g_cDrillLoadProcessAndOverride<=DRILL_LOAD_PROCESS_READY)
			{
				logf("dsipl\r\n");
				//Send Parameters
				DrillFillSendFastenerParameters();
				//Indicate that process is loaded
				g_cLoadedProcess = g_cRequiredProcess;
				SmartToolMsgChar(p_STPSession, STP_ALERT, MINIFT_OID_PROCESS, g_cLoadedProcess);//New Purpose of Alert is to just show what is loaded...
				//Mark this as done
				g_cDrillLoadProcessAndOverride=DRILL_LOAD_PROCESS_DONE;
				//Advance Station Prep if needed //FIXME DRILLFILL this may be a prob when I go back to that rail
				PrepareStations();
			}
			return;
		}
#endif
#ifdef SMARTDRILL_STP

		//Must Have Correct Process Loaded, and send layer overrides
		if (g_cOverrideCalculated==0)
		{
#warnt "SEVERE Must ensure that any operations effecting this will cause recalc!!!!!!!!!!!!!!!"
//Every Hole Must RECALC this... using new hole parameters
			CalculateLayerOverride();
			if (g_cOverrideCalculated==0)
			{
				//Problem.... Not Ready to Calculate Overrides...
				//FIXME SEVERE  This is only a very basic protection... pattern needs improvement
				return;
			}

			g_cOverrideSent = 0;
		}
		if (g_cDrillLoadProcessAndOverride<DRILL_LOAD_PROCESS_READY)
		{
			if (g_cDrillLoadProcessAndOverride==DRILL_LOAD_PROCESS_NEEDED)
			{
				//Not Yet Ready... change status to waiting
				g_cDrillLoadProcessAndOverride=DRILL_LOAD_PROCESS_WAIT;
//FIXME Immediate  track this time
				return;
			}
			//Still waiting....
			return;
		}
		if (g_cLoadedProcess!=g_cRequiredProcess)
		{
			logf("L%d R%d\r\n",g_cLoadedProcess,g_cRequiredProcess);
			LoadProcess();
			g_cLoadedProcess = g_cRequiredProcess;
			SmartToolMsgChar(p_STPSession, STP_ALERT, MINIFT_OID_PROCESS, g_cLoadedProcess); //New Purpose of Alert is to just show what is loaded...
//FIXME... request vs. confirm issue
			g_cOverrideSent = 0;
		}
		if (g_cOverrideSent == 0)
		{
			//Send Layer Override
			SendLayerOverride();
			g_cOverrideSent = 1;
		}
		//And when this is called again, if it reaches this point, it is still done.
		g_cDrillLoadProcessAndOverride=DRILL_LOAD_PROCESS_DONE;
		//Advance Station Prep if needed //FIXME DRILLFILL this may be a prob when I go back to that rail
		PrepareStations();
#else
		g_cDrillLoadProcessAndOverride = DRILL_LOAD_PROCESS_DONE;
#endif
	}
	return;
}

#ifdef SMARTDRILL_STP
typedef struct SmartDrillStackStruct
{
	float MinBreak;
	char PendantFilename[64];
	char Desc[64];
	char RabbitFilename[9];
	byte UseLayerOverrideData;
	int CutterCycleLimit;
}td_SmartDrillStackStruct;

void LoadProcess()
{
	unsigned int index;
	int len;
	//char * sbuf;
	char * buf;
	byte layer;
	byte proclayers;
	byte flags;
	byte flagsb;
	td_Layers * p_Layer;
	byte layer_micropeck;
	byte layer_lcdeltathrust;
	byte layer_setback;
	byte layer_earlyshiftdistance;
	byte layer_harder;
	float fThrust;
	td_LayerOverride * p_LayerOverride;
	float fhardstopamps;
	td_SmartDrillStackStruct * p_SmartDrillStackStruct;
	byte clube;

	logf("LoadProcess\r\n");

	if (g_pcProcessLayerDataBase == 0)
	{
		//no loaded process layer data
		logf("Process Layer Database was not found.\r\n");
		return;
	}
	clube=0;

//DRILL_DIRECT_PROCESS_AND_OVERRIDE
//Send this data to the drill
	if (g_DrillSTP.wLastState >= tcp_StateESTAB)
	{
		fhardstopamps=((float)g_uiProcessHardstopAmps[g_cRequiredProcess])/100;
		writeFloatToBufferNTOHFC(g_STPtxMsg.p_cObjectValue,fhardstopamps);
		SmartDrillSTPSetData(SMARTDRILL_OID_HARDSTOPCURRENT,4,(char *)g_STPtxMsg.p_cObjectValue);
		logf("ha=%f\r\n",fhardstopamps);

		memset(g_STPtxMsg.p_cObjectValue,0,sizeof(td_SmartDrillStackStruct));
		p_SmartDrillStackStruct=(td_SmartDrillStackStruct *)g_STPtxMsg.p_cObjectValue;
		p_SmartDrillStackStruct->MinBreak = ntohfc(g_LoadedTool.fMinBreak); //use min break of loaded tool.
		p_SmartDrillStackStruct->UseLayerOverrideData=1;
		p_SmartDrillStackStruct->CutterCycleLimit=0;
		SmartDrillSTPSetData(SMARTDRILL_OID_STACK,sizeof(td_SmartDrillStackStruct),(char *)g_STPtxMsg.p_cObjectValue);
	}

	proclayers = g_cProcessProcLayerCount[g_cRequiredProcess];

	index = g_uiProcessLayerDataIndex[g_cRequiredProcess];
	len = (int)(g_uiProcessLayerDataIndex[g_cRequiredProcess + 1] - (unsigned int)index);

	logf("p %d ind %d\r\n", g_cRequiredProcess, index);
	logf("l=%d pl=%d\r\n", len, proclayers);

	buf = g_pcProcessLayerDataBase + index;

	p_LayerOverride = (td_LayerOverride *)g_LayerOverrides;

	p_Layer = (td_Layers *)g_STPtxMsg.p_cObjectValue;

	layer=1;
	while(layer<=proclayers)
	{
		if ( ((char *)p_Layer) + sizeof(td_Layers) > sizeof(g_STPtxMsg.p_cObjectValue))
		{
			logf("FAILURE!!!!!! Need more memory to pack layers\r\n");
//FIXME Immediate   must show up as a failure to load proc.
			break;
		}
		//read out this compressed layer data
		flags = *buf++;
		flagsb = *buf++;
		memset((char *)p_Layer,0,sizeof(td_Layers));
		layer_micropeck = 0;
		layer_lcdeltathrust = 0;
		layer_setback = 0;
		layer_earlyshiftdistance = 0;
		layer_harder = 0;

		p_Layer->Name[0]=('0'+layer);
		p_Layer->Name[1]=0;
		p_Layer->cLayerNumber = layer;
		if (flags & 1)
		{	p_Layer->cCoolantType=1;}
		if (flags & 2)
		{	p_Layer->cPeckType=1;}
		if (flags & 4)
		{	p_Layer->cShiftRetract=1;}
		if (flags & 8)
		{	p_Layer->cLcUseSlope=1;}
		if (flags & 16)
		{	p_Layer->cH2SPeck=1;}
		if (flags & 32)
		{	p_Layer->cCountersinkLayer=1;}
		if (flags & 64)
		{	p_Layer->cUseHardStop=1;}
		//if (flags & 128) { p_Layer->cBurstInterval=1; } //FIXMENOW
		if (flagsb & 1)
		{	layer_micropeck=1;}
		if (flagsb & 2)
		{	layer_lcdeltathrust=1;}
		if (flagsb & 4)
		{	layer_setback=1;}
		if (flagsb & 8)
		{	layer_earlyshiftdistance=1;}
		if (flagsb & 16)
		{	layer_harder=1;}

		p_Layer->uiRpm = *(unsigned int *)buf; buf+=2;
		p_Layer->fIpr = ((float)(*(int *)buf))/10000; buf+=2;
		if (layer_setback)
		{	p_Layer->fSetback = ((float)(*(int *)buf))/10000; buf+=2;}
		if (layer_earlyshiftdistance)
		{	p_Layer->fEarlyShiftDistance = ((float)(*(int *)buf))/10000; buf+=2;}

#ifdef GEN4LUBE
#define LUBE_AUTO 2
#define LUBE_SPRITZ 3
		if (p_Layer->cCoolantType) //CoolantOn
		{
			p_Layer->iLubeDurationMs = *(unsigned int *)buf; buf+=2;
			if (clube<SPRITZ) //not yet spritz
			{
				if (p_Layer->iLubeDurationMs == 1)
				{
					clube=AUTO; //auto
				}
				else
				{
					clube=SPRITZ; //spritz
				}
				if (layer==1)
				{
					if (clube>0)
					{
						//using lube on the 1st layer.
						if (g_ConfigData.fReturnHeight < g_ConfigData.LaserSensorAlgParam.fdelta_basespan)
						{
							logf("lrh %f %f\r\n", g_ConfigData.fReturnHeight, g_ConfigData.LaserSensorAlgParam.fdelta_basespan);
							//Warn about return height.
							SmartToolMsgMiniFtMessageCode(0, COMMON_OID_NULLOID, MINIFTMC_LUBE_POSITION_WARNING);
						}
					}
				}
			}
		}
#else
		//The New Way
#define LUBEMODE_AUTO 2
#define LUBELAYER_OFF 0
#define LUBELAYER_ON 1
#define LUBELAYER_SPRITZ 4
		if (p_Layer->cCoolantType>0)
		{
			p_Layer->iLubeDurationMs = *(unsigned int *)buf; buf+=2;
			logf("rld %d\r\n",p_Layer->iLubeDurationMs);
			if (clube<LUBEMODE_AUTO) //not yet set
			{
				clube=LUBEMODE_AUTO; //auto
				if (layer==1)
				{
					//using lube on the 1st layer.
					if (g_ConfigData.fReturnHeight < g_ConfigData.LaserSensorAlgParam.fdelta_basespan)
					{
						logf("lrh %f %f\r\n", g_ConfigData.fReturnHeight, g_ConfigData.LaserSensorAlgParam.fdelta_basespan);
						//Warn about return height.
						SmartToolMsgMiniFtMessageCode(0, COMMON_OID_NULLOID, MINIFTMC_LUBE_POSITION_WARNING);
					}
				}
			}
			if (p_Layer->iLubeDurationMs > 1)
			{
				p_Layer->cCoolantType = LUBELAYER_SPRITZ;
			}
			else if (p_Layer->iLubeDurationMs < 1)
			{
				p_Layer->cCoolantType = LUBELAYER_OFF;
			}
			else
			{
				p_Layer->cCoolantType = LUBELAYER_ON;
			}
		}
		else
		{
			p_Layer->cCoolantType = LUBELAYER_OFF;
			p_Layer->iLubeDurationMs=0;
		}
#endif
		if (p_Layer->cPeckType>0)
		{
			p_Layer->uiPeckDwell = *(unsigned int *)buf; buf+=2;
			p_Layer->fPeckIncrement = ((float)(*(int *)buf))/10000; buf+=2;
		}
		if (layer_micropeck)
		{
			p_Layer->fMicroPeckRot = ((float)(*(int *)buf))/100; buf+=2;
			p_Layer->fMicroPeckSetback = ((float)(*(int *)buf))/10000; buf+=2;
		}
		if (layer_lcdeltathrust)
		{
			p_Layer->fThrustBaselineDistance = ((float)(*(int *)buf))/10000; buf+=2;
			p_Layer->fThrustMin = *(unsigned int *)buf; buf+=2;
			p_Layer->fThrustMax = *(unsigned int *)buf; buf+=2;
			p_Layer->fLcDeltaThrust = *(int *)buf; buf+=2;
		}
		else
		{
			if (layer==1)
			{
				fThrust = 0;
			}
			else if (layer_harder)
			{
				fThrust += 10;
			}
			else
			{
				fThrust -= 10;
			}
			p_Layer->fThrustMin = fThrust;
			p_Layer->fThrustMax = fThrust + 40;
			if (layer==1)
			{
				//Start Next at 20
				fThrust = 20;
			}
		}
		p_Layer->fAbsoluteEnd = p_LayerOverride->fAbsoluteEnd; //Copy From Overrides
		p_Layer->fThicknessMax = p_LayerOverride->fThickness;//Copy From Overrides

//FIXME SEVERE
		//Testing Display
//		logfdebuglogf("%d",123);
//		logfdebuglogf("%d",456);

		logf(" L ");
		logf(" %d ",layer);
		logf(" %u ",p_Layer->uiRpm);
		logf(" %f ",p_Layer->fIpr);
		if (flagsb & 4)
		{	logf(" sb"); logf(" %f ", p_Layer->fSetback);}
		if (flagsb & 8)
		{	logf(" es"); logf(" %f ", p_Layer->fEarlyShiftDistance);}
		if (flags & 1)
		{	logf(" c"); logf(" %d ", p_Layer->iLubeDurationMs);}
		if (flags & 2)
		{	logf(" p"); logf(" %u ", p_Layer->uiPeckDwell); logf(" %f ", p_Layer->fPeckIncrement);}
		if (flagsb & 1)
		{	logf(" mp"); logf(" %f ", p_Layer->fMicroPeckRot); logf(" %f ", p_Layer->fMicroPeckSetback);}
		if (flagsb & 16)
		{	logf(" ha");}
		if (flagsb & 2)
		{
			logf(" lc");
			logf(" %f ", p_Layer->fThrustBaselineDistance);
			logf(" %f ", p_Layer->fThrustMin);
			logf(" %f ", p_Layer->fThrustMax);
			logf(" %f ", p_Layer->fLcDeltaThrust);
		}
		else
		{
			logf(" tmm");
			logf(" %f ", p_Layer->fThrustMin);
			logf(" %f ", p_Layer->fThrustMax);
		}
		logf(" ab");logf(" %f ", p_Layer->fAbsoluteEnd);
		logf(" th");logf(" %f ", p_Layer->fThicknessMax);
		if (flags & 4)
		{	logf(" sr");}
		if (flags & 8)
		{	logf(" l");}
		if (flags & 16)
		{	logf(" h2s");}
		if (flags & 32)
		{	logf(" csl");}
		if (flags & 64)
		{	logf(" hs");}
		if (flags & 128)
		{	logf(" ss");}
		logf(" \r\n");

		//Prepare For Network Transmission
		p_Layer->uiRpm=htons(p_Layer->uiRpm);
		p_Layer->uiPeckDwell=htons(p_Layer->uiPeckDwell);//45
		p_Layer->iLubeDurationMs=htons(p_Layer->iLubeDurationMs);//47
		p_Layer->fMicroPeckRot=htonfc(p_Layer->fMicroPeckRot);
		p_Layer->fMicroPeckSetback=htonfc(p_Layer->fMicroPeckSetback);
		p_Layer->fAbsoluteEnd=htonfc(p_Layer->fAbsoluteEnd);
		p_Layer->fThicknessMax=htonfc(p_Layer->fThicknessMax);
		p_Layer->fIpr=htonfc(p_Layer->fIpr);//67
		p_Layer->fPeckIncrement=htonfc(p_Layer->fPeckIncrement);
		p_Layer->fSetback=htonfc(p_Layer->fSetback);
		p_Layer->fEarlyShiftDistance=htonfc(p_Layer->fEarlyShiftDistance);
		p_Layer->fThrustBaselineDistance=htonfc(p_Layer->fThrustBaselineDistance);
		p_Layer->fThrustMin=htonfc(p_Layer->fThrustMin);
		p_Layer->fThrustMax=htonfc(p_Layer->fThrustMax);
		p_Layer->fLcDeltaThrust=htonfc(p_Layer->fLcDeltaThrust);//95

		p_Layer++;//Increments the entire size of the struct
		p_LayerOverride++;//increase one layer override
		layer++;

		//must resolve mising fields ??????

	}
	len=buf-sbuf;
	logf("csb=%d\r\n",len); //verify consumed characters.....

//DRILL_DIRECT_PROCESS_AND_OVERRIDE
//Send this data to the drill
	if (g_DrillSTP.wLastState >= tcp_StateESTAB)
	{
		SmartDrillSTPSetData(SMARTDRILL_OID_LAYERDEFNS,sizeof(td_Layers)*(layer-1),(char *)g_STPtxMsg.p_cObjectValue);
		logf("LD Sent \r\n");
		g_ulLD = MS_TIMER;
	}
	else
	{
		logf("LD Send Fail\r\n");
	}

	if (g_cLubeBypass==1)
	{
		clube=0;
		goto set_lube;
	}
	if (clube>0)
	{
		set_lube:
		SmartDrillSTPSetData(SMARTDRILL_OID_COOLANT_MODE,1,(char *)&clube);
	}
	return;
}
#endif

//probe control field

void PreviewDisplayProbeCommandDataBase()
{
	char * pc;
	int i;
	int p;
	byte ck;

	pc = g_pcProbeCommandDataBase;
	if (pc == 0)
	{
		logf("npcf\r\n");
		return;
	}

	ck = 1;
	while (ck <= g_cKHoleCount)
	{
		i = *(int16 *) pc;
		pc += 2;
		logf("kc %d  %d  \r\n", ck, i);
		if (i < 512)
		{
			p = 0;
			while (p < i)
			{
				logf(" %d %d\r\n", p, pc[p]);
				p++;
			}
			logf("dl %d %d\r\n\r\n", p, i); //end check for issue with data kill???
		}
		else
		{
			//fatal????
			//FIXME0000 FIXMENOW zxcvzxcvzxcvzxcvzxcvzxcvzxcv
			logf("no show\r\n");
		}
		ck++;
		pc += i;
	}
}

void LoadProbeCommand(byte ckprobe)
{
//go down the list and find this record....
//This could be more efficient, but it's fast enough and can be done in a simpler way after port to new tech platform in 8 months from now...
	char * pc;
	int i;
	int p;
	byte ck;
	char * pccmd;
	char sbuf[2];

	pc = g_pcProbeCommandDataBase;
	if (pc == 0)
	{
		logf("npcf\r\n");
		//Clear all probe command details
		ClearProbeCommand();
		//treat like probe
		g_cProbeCommand = KEYWORD_Probe;
		return;
	}

	ck = 1;
	while (ck <= g_cKHoleCount)
	{
		i = *(int16 *) pc;
		pc += 2;
		logf("kc %d  %d  \r\n", ck, i);
		if (ck == ckprobe)
		{
			if (i > 512)
			{
				//fatal????
				//FIXME0000 FIXMENOW zxcvzxcvzxcvzxcvzxcvzxcvzxcv
				logf("no show\r\n");
				i = 0;
			}
			if (i == 0)
			{
				//Do like "Probe" only
				i = 1;
				sbuf[0] = KEYWORD_Probe;
				pccmd = sbuf;
			}
			else
			{
				pccmd = pc;
			}
			ParseProbeCommand(ckprobe, pccmd, i);
			break;
		}
		ck++;
		pc += i;
	}
}

//If Blank length will be zero, Treat like "PROBE" with no options
//	move to end
//
//group 1 required
//
//PROBE			process and move to group 2
//PROBE_EDGE_VEC X Y	process and move to group 2
//PROBE_EDGE_MVEC X Y	process and move to group 2
//no match? must be a syntax error: raise error, and move to end
//
//group 2 optional
//
//OFFSET X Y	process and continue group 2 (but this may appear only once)
//MOFFSET X Y	process and continue group 2 (but this may appear only once)
//nothing left?  OK move to end
//no match in group? move to group 3
//
//group 3 optional
//
//REQUIRE		process and move to group 4
//nothing left?  OK move to end
//no match? must be a syntax error: raise error, and move to end
//
//group 4 optional
//
//EXPECTED_DIAMETER D 	process and continue group 4
//nothing left?  OK move to end
//no match? must be a syntax error: raise error, and move to end

void ClearProbeCommand()
{
	g_cProbeCommand = 0; //clear probe command
	g_cProbeCommandMessage = 0;
	g_fProbeVectorX = 0;
	g_fProbeVectorY = 0;
	g_fProbeMachineVectorX = 0;
	g_fProbeMachineVectorY = 0;
	g_cProbeExtraOffsetGiven = 0;
	g_cProbeExtraMachineOffsetGiven = 0;
	g_fProbeExtraOffsetX = 0;
	g_fProbeExtraOffsetY = 0;
	g_fProbeExtraMachineOffsetX = 0;
	g_fProbeExtraMachineOffsetY = 0;
	g_fProbeMaxDistShift = 0; //also means no custom limit
	g_cProbeShiftLimX = 0;
	g_fProbeShiftLimXMin = 0;
	g_fProbeShiftLimXMax = 0;
	g_cProbeShiftLimY = 0;
	g_fProbeShiftLimYMin = 0;
	g_fProbeShiftLimYMax = 0;
	g_fProbeExpectedDiameter = 0;
}

void ParseProbeCommand(byte ckprobe, char * sbuf, int i)
{
	int p;
	byte c;
	float f;
	float f2;
	float fswap;

	ClearProbeCommand();
	p = 0;
	if (i == 0)
	{
		//treat like probe
		g_cProbeCommand = KEYWORD_Probe;
		goto parse_done;
	}
//group 1 required
	c = sbuf[p++];
	if (c == KEYWORD_Probe)
	{
		g_cProbeCommand = c;
		//continue to group 2
	}
	else if (c == TKP_PROBE_EDGE_VEC)
	{
		g_cProbeCommand = c;
		if (p + 8 > i)
		{
			//missing parameter arguments
			g_cProbeCommandMessage = MINIFTMC_PROBE_COMMAND_PARAMETERS;
			goto parse_fail;
		}
		g_fProbeVectorX = *(float *) (sbuf + p);
		p += 4;
		g_fProbeVectorY = *(float *) (sbuf + p);
		p += 4;
		logf("%s %f,%f\r\n", "pev", g_fProbeVectorX, g_fProbeVectorY);
		//continue to group 2
	}
	else if (c == TKP_PROBE_EDGE_MVEC)
	{
		g_cProbeCommand = c;
		if (p + 8 > i)
		{
			//missing parameter arguments
			g_cProbeCommandMessage = MINIFTMC_PROBE_COMMAND_PARAMETERS;
			goto parse_fail;
		}
		g_fProbeMachineVectorX = *(float *) (sbuf + p);
		p += 4;
		g_fProbeMachineVectorY = *(float *) (sbuf + p);
		p += 4;
		logf("%s %f,%f\r\n", "pemv", g_fProbeMachineVectorX, g_fProbeMachineVectorY);
		//continue to group 2
	}
	else
	{
		//failure to match group 1
		g_cProbeCommandMessage = MINIFTMC_PROBE_COMMAND_INVALID;
		goto parse_fail;
	}
//group 2 optional and group 3 required
	while (p < i) //more to parse
	{
		c = sbuf[p++];
		if (c == KEYWORD_Offset)
		{
			if (g_cProbeExtraOffsetGiven == 1)
			{
				//already specified
				g_cProbeCommandMessage = MINIFTMC_PROBE_COMMAND_REPEAT;
				goto parse_fail;
			}
			g_cProbeExtraOffsetGiven = 1;
			if (p + 8 > i)
			{
				//missing parameter arguments
				g_cProbeCommandMessage = MINIFTMC_PROBE_COMMAND_PARAMETERS;
				goto parse_fail;
			}
			g_fProbeExtraOffsetX = *(float *) (sbuf + p);
			p += 4;
			g_fProbeExtraOffsetY = *(float *) (sbuf + p);
			p += 4;
			logf("%s %f,%f\r\n", "eo", g_fProbeExtraOffsetX, g_fProbeExtraOffsetY);
			//continue with group 2
			continue;
		}
		if (c == TKP_MACHINEOFFSET)
		{
			if (g_cProbeExtraMachineOffsetGiven == 1)
			{
				//already specified
				g_cProbeCommandMessage = MINIFTMC_PROBE_COMMAND_REPEAT;
				goto parse_fail;
			}
			g_cProbeExtraMachineOffsetGiven = 1;
			if (p + 8 > i)
			{
				//missing parameter arguments
				g_cProbeCommandMessage = MINIFTMC_PROBE_COMMAND_PARAMETERS;
				goto parse_fail;
			}
			g_fProbeExtraMachineOffsetX = *(float *) (sbuf + p);
			p += 4;
			g_fProbeExtraMachineOffsetY = *(float *) (sbuf + p);
			p += 4;
			logf("%s %f,%f\r\n", "emo", g_fProbeExtraMachineOffsetX, g_fProbeExtraMachineOffsetY);
			//continue with group 2
			continue;
		}
		//group 3 required
		if (c == KEYWORD_Require)
		{
			//continue to group 4
			break;
		}
		//did not match anything in group 2 or required group 3
		//Expected Require
		g_cProbeCommandMessage = MINIFTMC_PROBE_COMMAND_OPTIONS_INVALID;
		goto parse_fail;
	}
//group 4 optional
	while (p < i) //more to parse
	{
		c = sbuf[p++];
		if (c == KEYWORD_MaxDistShift || c == TKP_MachMaxDistShift)
		{
			if (p + 4 > i)
			{
				//missing parameter arguments
				g_cProbeCommandMessage = MINIFTMC_PROBE_COMMAND_PARAMETERS;
				goto parse_fail;
			}
			g_fProbeMaxDistShift = *(float *) (sbuf + p);
			p += 4;
			logf("%s %f\r\n", "mds", g_fProbeMaxDistShift);

			//continue with group 4
			continue;
		}
		if (c == KEYWORD_ShiftLimX || c == TKP_MachShiftLimX)
		{
			if (p + 4 > i)
			{
				//missing parameter arguments
				g_cProbeCommandMessage = MINIFTMC_PROBE_COMMAND_PARAMETERS;
				goto parse_fail;
			}
			f = *(float *) (sbuf + p);
			p += 4;
			f2 = *(float *) (sbuf + p);
			p += 4;
			logf("%s %f %f\r\n", "slx", f, f2);
			if (f > f2)
			{
				fswap = f;
				f = f2;
				f2 = fswap;
			}
			g_cProbeShiftLimX = MACHINEXY;
			if (c == KEYWORD_ShiftLimX)
			{
				g_cProbeShiftLimX = DATAXY;
			}
			g_fProbeShiftLimXMin = f;
			g_fProbeShiftLimXMax = f2;
			//continue with group 4
			continue;
		}
		if (c == KEYWORD_ShiftLimY || c == TKP_MachShiftLimY)
		{
			if (p + 4 > i)
			{
				//missing parameter arguments
				g_cProbeCommandMessage = MINIFTMC_PROBE_COMMAND_PARAMETERS;
				goto parse_fail;
			}
			f = *(float *) (sbuf + p);
			p += 4;
			f2 = *(float *) (sbuf + p);
			p += 4;
			logf("%s %f %f\r\n", "sly", f, f2);
			if (f > f2)
			{
				fswap = f;
				f = f2;
				f2 = fswap;
			}
			g_cProbeShiftLimY = MACHINEXY;
			if (c == KEYWORD_ShiftLimY)
			{
				g_cProbeShiftLimY = DATAXY;
			}
			g_fProbeShiftLimYMin = f;
			g_fProbeShiftLimYMax = f2;
			//continue with group 4
			continue;
		}
		if (c == KEYWORD_AuxA || c == TKP_MachAuxA)
		{
			if (p + 4 > i)
			{
				//missing parameter arguments
				g_cProbeCommandMessage = MINIFTMC_PROBE_COMMAND_PARAMETERS;
				goto parse_fail;
			}
			//Incomplete because Auxa is reserved
			f = *(float *) (sbuf + p);
			p += 4;
			f2 = *(float *) (sbuf + p);
			p += 4;
			logf("%s %f %f\r\n", "auxa", f, f2);
			//continue with group 4
			continue;
		}
		if (c == TKP_EXPECTED_DIAMETER) //FIXME incomplete must implement this some day
		{
			if (p + 12 > i)
			{
				//missing parameter arguments
				g_cProbeCommandMessage = MINIFTMC_PROBE_COMMAND_PARAMETERS;
				goto parse_fail;
			}
			g_fProbeExpectedDiameter = *(float *) (sbuf + p);
			p += 4;
			f = *(float *) (sbuf + p);
			p += 4;
			f2 = *(float *) (sbuf + p);
			p += 4;
			logf("%s %f,%f,%f\r\n", "exd", g_fProbeExpectedDiameter, f, f2);

			//continue with group 4
			continue;
		}
		//did not match anything in group 4
		g_cProbeCommandMessage = MINIFTMC_PROBE_COMMAND_REQUIRE_INVALID;
		goto parse_fail;
	}
	parse_done:
	//done
	return;
	parse_fail:
	//set the command byte to invalid, and also return with the ProbeCommandMessage Set
	g_cProbeCommand = PROBE_COMMAND_INVALID;
	if (g_cProbeCommandMessage == 0)
	{
		//This message is used for specific error above, but also used here
		//	in the case that something was not otherwise set... (shouldn't happen)
		g_cProbeCommandMessage = MINIFTMC_PROBE_COMMAND_INVALID;
	}
	return;

}
/*

 #define KEYWORD_Probe	1
 #define KEYWORD_Point	2
 #define KEYWORD_Edge	3
 #define KEYWORD_Machine 4
 #define KEYWORD_Vector	5
 #define KEYWORD_Feature 6
 #define KEYWORD_Offset	7
 #define KEYWORD_Require	8
 #define KEYWORD_Intersect	9
 #define KEYWORD_Distance	10
 #define KEYWORD_Dist		10
 #define KEYWORD_Center		11
 #define KEYWORD_Expected	12
 #define KEYWORD_DistShift	13
 #define KEYWORD_Diameter	14

 //Other Token Values
 #define TK_Double		64
 #define TK_Numeric		65
 #define TK_QuotedString	66
 #define TK_String		67

 //Pattern Tokens
 #define TKP_PROBE_EDGE	128
 #define TKP_PROBE_EDGE_VEC	129
 #define TKP_PROBE_EDGE_MVEC 130
 #define TKP_PROBE_POINT_FEATURE 131
 #define TKP_PROBE_EDGE_FEATURE 132
 #define TKP_MACHINEOFFSET 134
 #define TKP_POINT_LITERAL 135
 #define TKP_MPOINT_LITERAL 136
 #define TKP_MVEC		137
 #define TKP_NAMED_EDGE	138
 #define TKP_NAMED_POINT	139
 #define TKP_EXPECTED_VECTOR 140
 #define TKP_EXPECTED_MACHINE_VECTOR 141
 */

void ClearPositionsDuringProbe(byte ki)
{
//Any point that depends on this point should be set to no probe and recalculated later.
	int i;
	byte c;
	byte ck;
	byte cki;
	byte cpk;
	byte csk;

	ck = 1;
	while (ck <= g_cKHoleCount)
	{
		i = g_iKHoleHoleIndex[ck];
		cki = g_HoleDataArray[i].cKInd;
		cpk = g_HoleDataArray[i].cKPri;
		csk = g_HoleDataArray[i].cKSec;
		if (cki != ck)
		{
			logf("cyclical check fail\r\n");
			g_cKHolePrbeStatus[ck] = PS_PROBED_FAILED;
			return;
		}
		if (cpk == ki || csk == ki)
		{
			//this khole depends on ki
			if (g_cKHolePrbeStatus[ck] != PS_NO_PROBE)
			{
				g_cKHolePrbeStatus[ck] = PS_NO_PROBE;
				SendProbeStatus(STP_ALERT, ck); //SPS
			}
			if (g_cKHolePrbeStart[ck] != PS_NO_PROBE)
			{
				g_cKHolePrbeStart[ck] = PS_NO_PROBE;
				SendProbeStart(STP_ALERT, ck); //SPS
			}
		}
		ck++;
	}
//FIXME Probe System Point Dependance.. if point dependance can be multi tiered, then this check must be done for all the derivative points....
	return;
}

void RecalculatePositionsDuringProbe(byte ki)
{
//currently just determine if any K Holes use ths K Holes for reference and update their approximate positions
//this needs to be refactored, however it will properly implement the required functionallity in a logical manner and can
//be consdered acceptable source for the reorg of probing that is likely to be needed after all the new features are filled in.
	int i;
	byte c;
	byte ck;
	byte cki;
	byte cpk;
	byte csk;
	float fx;
	float fy;
	logf("rpdp %d\r\n", ki);
	ck = 1;
	while (ck <= g_cKHoleCount)
	{
		i = g_iKHoleHoleIndex[ck];
		cki = g_HoleDataArray[i].cKInd;
		cpk = g_HoleDataArray[i].cKPri;
		csk = g_HoleDataArray[i].cKSec;
//logf(" %d %d %d  %d %d\r\n",cki,cpk,csk, g_cKHolePrbeStatus[cpk], g_cKHolePrbeStatus[csk]);
		if (cki != ck)
		{
			logf("cyclical check fail\r\n");
			g_cKHolePrbeStatus[ck] = PS_PROBED_FAILED;
			return;
		}
		if (cpk == ki || csk == ki)
		{
			//this khole depends on ki
			if (g_cKHolePrbeStatus[cpk] >= PS_PROBED)
			{
				if (g_cKHolePrbeStatus[csk] >= PS_PROBED)
				{
					//both are probed, so calculation could happen with this
					c = CreateMachineDataSetForOneHole(i, &fx, &fy);
					if (c == 0)
					{
						g_cKHolePrbeStatus[ck] = PS_PROBED_FAILED; //set fail code (This should never happen if program is valid and the parents are probed.)
						logf("fail\r\n");
					}
					else
					{
//FIXME HIGHEST SEVERE shouldn't a redo of the position FORCE a reprobe of this one????? Isn't this the point here????
//Under what case would this not need to be done?
						//make this location the APPROXIMATE LOCATION defined by it's k holes
						g_cKHolePrbeStart[ck] = PS_APPROXIMATE;
						g_fKHolePrbeStartX[ck] = fx;
						g_fKHolePrbeStartY[ck] = fy;
						logf("saprx %d\r\n", ck);
					}
					//1st send start
					SendProbeStart(STP_ALERT, ck); //SPS
				}
			}
		}
		ck++;
	}
	return;
}

void CheckKLocationDistances(byte ck, byte mark)
{
	byte ck2;
	unsigned int ui;
	unsigned int ui2;
	float fx, fy, fdx, fdy, fddx, fddy, fdatax, fdatay;
	float fDistanceData, fDistance;
	float fmd;
	float dlim;

	fmd = g_ConfigData.fMaxKholeDistanceError;

	dlim = g_ConfigData.fMaxKholeDistanceCheck; // limit for checking distance
	dlim = dlim * dlim; //compare with the squared value

	g_cKHolePrbeStatusDistance[ck] = 0; //clear the distance error on this... it will get set back later in this function if there is an error

	ui = g_iKHoleHoleIndex[ck];
//Get FlatXY data value for this hole....
	fdatax = g_fRawDataX[ui];
	fdatay = g_fRawDataY[ui];
//load fx and fy with this probe position
	fx = g_fKHolePrbeX[ck];
	fy = g_fKHolePrbeY[ck];
	logf("%c %d %f %f %f %f\r\n", 'a', ck, fdatax, fdatay, fx, fy);
	ck2 = 1;
	while (ck2 <= g_cKHoleCount)
	{
		if (ck2 != ck && g_cKHolePrbeStatus[ck2] >= PS_PENDING_ACCEPTANCE)
		{
			//another Probed KHole
			ui2 = g_iKHoleHoleIndex[ck2];
			//Get FlatXY data diff values for this hole....
			fddx = fdatax - g_fRawDataX[ui2];
			fddy = fdatay - g_fRawDataY[ui2];
			//Find expected distance
			fDistanceData = fddx * fddx + fddy * fddy;
			if (fDistanceData < dlim || ck2 == 1 || ck2 == g_cKHoleCount) //only need to pass test if distance is less than KHoleDistanceCheck, ck2==1 or ck2=g_cKHoleCount
			{

				fDistanceData = sqrt(fDistanceData);

				//Get Probe diff values
				fdx = fx - g_fKHolePrbeX[ck2];
				fdy = fy - g_fKHolePrbeY[ck2];
				//Find Probed distance
				fDistance = sqrt(fdx * fdx + fdy * fdy);
				if (fabs(fDistanceData - fDistance) > fmd)
				{
					//Error is too large.
					//SendKHoleDistance(p_STPSession, STP_ALERT,fDistanceData,fDistance);
					if (g_cDistanceErrorFlagSent == 0)
					{
						//send this on the 1st one only...
						SmartToolMsgFloat(0, STP_ALERT, MINIFT_OID_KHOLE_MAX_DISTANCE_ERROR, g_ConfigData.fMaxKholeDistanceError);
						g_cDistanceErrorFlagSent = 1;
					}
					//Set Distance Error Flags
					g_cKHolePrbeStatusDistance[ck] = 1;
					g_cKHolePrbeStatusDistance[ck2] = 1;
					//Show 1st 4 distance errors
					if (g_cDistanceErrorShown < 4)
					{
						logf("%c %d %f %f %f %f\r\n", 'f', ck2, fdatax - fddx, fdatay - fddy, g_fKHolePrbeX[ck2], g_fKHolePrbeY[ck2]);
						g_cDistanceErrorShown++;
					}
				}
				else
				{
					//Acceptable
					//ck does not fail, and ck2 might be better now
					if (mark)
					{
						if (g_cKHolePrbeStatusDistance[ck2] == 1)
						{
							//give ck2 a chance to be checked later when this returns
							g_cKHolePrbeStatusDistance[ck2] = 0xFF;
						}
					}
				}
			}
		}
		ck2++;
	}
}

//////////////////////////////////////////////////////////////////////
//  Position Mode functions
//////////////////////////////////////////////////////////////////////
void ResetNearestPosition()
{
	g_PosnMode.iNearestPosn = -1; //make sure this is cleared
	g_PosnMode.fLastSentPosnX = -0.123456; //random so that it will send back whatever new position it gets.
}

//MACHINEPOINTS
int FindNearestPosition()
{
//	Search for the nearest program position ID to current location in g_PosnMode.fLastKnownPosnX,Y.
	int i, imax, iClosestIndex;
	float fSearchX, fSearchY;

	float fX, fY, fCurDistSquared, fClosestSquared, fR;
	float fdx, fdy, fd, fbucket;
	float fdx2, fdy2;
	byte cSkipKholes;
	byte cSearchMode;
	t_MPOSNBUCKETTYPE bucket, bucketa, bucketb;
	float fax, fbx;

//Last known position is stored in Machine coords.
	fSearchX = g_PosnMode.fLastKnownPosnX;
	fSearchY = g_PosnMode.fLastKnownPosnY;

//For Searching the machine positions, we need to REMOVE machine offset
//So for example, if you have a stapler at -1,0
//And you have it set to staple, then the machine will move to 1, 0 in order to have
//The stapler be over 0,0 .
//When searching points, you need to remove the tool offset to go from 1,0 to 0,0 to check for points around 0,0

	RemoveMachineOffset(&fSearchX, &fSearchY); //Because we want to search for points near our selected station

	if (g_PosnMode.iNearestPosn >= 0)
	{
		//Test to see if the previous search can be recycled
		//If this position is in a box 0.001 around the last calculated... just use that.
		if (fabs(g_PosnMode.fNearestCalculatedForMachineX - fSearchX)
				< g_fPosnLSCD
				&& fabs(g_PosnMode.fNearestCalculatedForMachineY - fSearchY)
						< g_fPosnLSCD)
		{
			//The last Nearest Posn was calculated for this machine position... reuse the result
			//logf("R %d\r\n",g_PosnMode.iNearestPosn);
			return g_PosnMode.iNearestPosn;
		}
	}
	if (g_uiCreateMachineDataSetHashIndex < g_iHoleCount)
	{
		//Never completed Building the hash, so don't return the nearest
		return 0;
	}

//must do nearest search....
//Save the position we are searching from so that we can compare it (above) on the next pass.
	g_PosnMode.fNearestCalculatedForMachineX = fSearchX;
	g_PosnMode.fNearestCalculatedForMachineY = fSearchY;

	iClosestIndex = -1;
	fClosestSquared = 10000000000.0; // initialize to an impossibly large inch value

	i = 0;
	imax = g_iHoleCount;

	if (g_PosnMode.iNearestPosn >= 0)
	{
		//before we search the whole list, see how far we are from the last best one.
		if (i <= g_PosnMode.iNearestPosn && g_PosnMode.iNearestPosn <= imax)
		{
			//still valid... try this one 1st
			//set it to the initial first place
			fX = g_PosnMode.fNearestPosnMX;
			fY = g_PosnMode.fNearestPosnMY;
			iClosestIndex = g_PosnMode.iNearestPosn;
			fdx = fX - fSearchX;
			fdy = fY - fSearchY;
			fClosestSquared = fdx * fdx + fdy * fdy;
			g_PosnMode.fNearestDistanceX = fdx;
			g_PosnMode.fNearestDistanceY = fdy;
			g_PosnMode.fNearestDistanceSquared = fClosestSquared; //save new value
			if (fClosestSquared <= g_PosnMode.fNearestRadiusExclusive) //Not truely the radius, but exclusive radius squared.
			{
				//the distance to this hole is inside it's exclusive radius
				//logf("S %f %f\r\n",fClosestSquared , g_PosnMode.fNearestRadiusExclusive);
				return g_PosnMode.iNearestPosn;
			}
		}
	}

//For Drill / Drillfill, don't check kholes.
	if (g_PartPgmInfo.cTeachModeData != 1)
	{
		cSkipKholes = 1;
	}

	g_PosnMode.iNearestPosn = -1; //Clear default

	cSearchMode = 0;

	search_for_nearest:
	//Reset closest Squared info
	fClosestSquared = 10000000000.0; // initialize to an impossibly large inch value
	iClosestIndex = -1;

//Use new hash system

//FIXME00000000000 there are some issues with the bucket list logic lookup at runtime....
// I have though about using the bucket list to relist a full posn index.,
// and then using the hash for inches to list index....
//Less radical:::
//Just use simpler bucket calc to adjust min max bucket....
//when we find a closer item....
//Should confirm current math for bucket borders....
//Further ideas:
//If each bucket knew it's min and max X and Y then we could
//do even better.
//Instead of all this we could compare to the bucket area....
//Still would have to hash into 1st bucket, but from there it would be
//measuring distance...
//At the very least: Could set a list of bucket borders.... no runtime add...????

//logf("SX=%f ",fSearchX);
	fbucket = fSearchX - g_fMPosnHashMinBucket;
	fbucket = fbucket / g_fMPosnHashBucketsize;
	if (fbucket < 0)
	{
		fbucket = 0;
	}
	else if (fbucket > (MPOSNHASHSIZE - 1))
	{
		fbucket = MPOSNHASHSIZE - 1;
	}

	bucket = (t_MPOSNBUCKETTYPE) fbucket;
#ifdef OUTPUT_NEAREST_INFO
	logf("f=%f ibucket=%d\r\n", fbucket, bucket);
#endif

	fd = bucket * g_fMPosnHashBucketsize + g_fMPosnHashMinBucket;
	fax = fSearchX - fd; //distance from X to next bucket....
	fd += g_fMPosnHashBucketsize;
	fbx = fd - fSearchX; //distance from X to next bucket....

	bucketa = bucket;
	bucketb = bucket;

//start this bucket
	try_this_bucket:
	i = g_iMPosnBucket[bucket];
	while (1)
	{
		if (i == -1)
		{
			//This is Empty... find next bucket to check
			if (bucket == bucketa)
			{
				//was doing a so do b now
				if (bucketb < MPOSNHASHSIZE - 1)
				{
					return_to_b: if (fbx * fbx < fClosestSquared)
					{
						bucketb++;
						bucket = bucketb;
						fbx += g_fMPosnHashBucketsize;
						goto try_this_bucket;
					}
					//advance to limit
					bucketb = MPOSNHASHSIZE - 1;
				}
			}
			//try a
			if (bucketa > 0)
			{
				if (fax * fax < fClosestSquared)
				{
					bucketa--;
					bucket = bucketa;
					fax += g_fMPosnHashBucketsize;
					goto try_this_bucket;
				}
				//advance to limit
				bucketa = 0;
			}
			//try b
			if (bucketb < MPOSNHASHSIZE - 1)
			{
				goto return_to_b;
			}
			//reached
			//reached the limits
			break;
		}
		//this is another position
		if (cSkipKholes == 1)
		{
			if (g_HoleDataArray[i].cKInd > 0) // Is KHole
			{
				//move to next in list
				goto next_in_bucket_list;
			}
		}

		fX = g_fRawDataMX[i];

		//Find X Distance from us
		fdx = fX - fSearchX;
		fdx2 = fdx * fdx;
		if (fdx2 > fClosestSquared)
		{
			//delta X alone is farther than our closest
			goto next_in_bucket_list;
		}

		fY = g_fRawDataMY[i];

		//Find Distance
		fdy = fY - fSearchY;
		fdy2 = fdy * fdy;
		fCurDistSquared = fdx2 + fdy2;
		if (cSearchMode == 1)
		{
			if (fCurDistSquared < 0.0001)
			{
				//ignore this position for what we are doing here
				goto next_in_bucket_list;
			}
		}
		if (fClosestSquared > fCurDistSquared)
		{
			//Found a new closer one
			fClosestSquared = fCurDistSquared;
			iClosestIndex = i;
			//And while we are here, check exclusive radius

			fR = g_fRawDataR[i];
			if (fR > 0 && fR > fClosestSquared)
			{
				//We are inside this positions exclusive radius!
				//No more searching needed.
				if (cSearchMode == 0)
				{
					goto found_by_exclusiveradius;
				}
				goto found_er_by_exclusiveradius;
			}
		}
		next_in_bucket_list:
		i = g_iMPosnHashList[i];
	}
	if (iClosestIndex == -1)
	{
#ifdef OUTPUT_NEAREST_INFO
		logf("nearest was not found\r\n");
#endif
	}

	if (iClosestIndex > 0)
	{
		if (cSearchMode == 0)
		{
			fX = g_fRawDataMX[iClosestIndex];
			fY = g_fRawDataMY[iClosestIndex];
			fR = g_fRawDataR[iClosestIndex];
			fdx = fSearchX - fX;
			fdy = fSearchY - fY;
			found_by_exclusiveradius: g_PosnMode.iNearestPosn = iClosestIndex;
			g_PosnMode.fNearestPosnMX = fX;
			g_PosnMode.fNearestPosnMY = fY;
			g_PosnMode.fNearestDistanceX = fdx;	//These are in machine coords, but the only place they are used,
			g_PosnMode.fNearestDistanceY = fdy;	//is in the determination of what a long distance move is.
			g_PosnMode.fNearestDistanceSquared = fClosestSquared;
#ifdef OUTPUT_NEAREST_INFO
			logf("p=%f,%f n=%d  %f,%f\r\n",fSearchX,fSearchY,g_PosnMode.iNearestPosn,fX,fY);
#endif
			//but have to lookup the dataset value.
			g_PosnMode.fNearestPosnDX = g_fRawDataX[iClosestIndex];
			g_PosnMode.fNearestPosnDY = g_fRawDataY[iClosestIndex];
			g_PosnMode.fNearestRadiusExclusive = fR;
			if (fR == 0)
			{
				//GO BACK and find the nearest radius exclusive
				//Do this by searching with this point as the center
				cSearchMode = 1;
				cSkipKholes = 0;
				fSearchX = fX;
				fSearchY = fY;
				goto search_for_nearest;
			}
		}
		else
		{
			//was doing a search for the next nearest to the nearest
			//did find another close hole
			//restore the answer to the main search
			found_er_by_exclusiveradius: iClosestIndex =
					g_PosnMode.iNearestPosn;
			//Set the exclusive radius for the hole (we store the value^2 though)
			//ClosestDistance ^ 2 = (ExclusiveRadius * 2) ^ 2
			//Therefore    ClosestDistance ^ 2 = 4* (ExclusiveRadius) ^ 2
			//Therefore    ClosestDistance ^ 2 / 4.0 = (ExclusiveRadius) ^ 2
			fR = fClosestSquared / 4.0;
			g_PosnMode.fNearestRadiusExclusive = fR;
			if (fR == 0)
			{
				fR = 0.0000001; //so it will not trigger another search in the future.
			}
			g_fRawDataR[iClosestIndex] = fR;
			//logf("Found exclusive radius for %d = %f\r\n",iClosestIndex,fR);
		}
	}
	return iClosestIndex;
}

int MoveAllowed(unsigned int uiSourceOid)
{
	if (g_cAction > ACTION_READY) //ACTION_IDLE==0 and ACTION_READY==1
	{
		//MiniFT is working and not ready to move
		SmartToolMsgMiniFtMessageCode(0, uiSourceOid, MINIFTMC_MOVE_PREVENTED);
		g_cActionSent = 0xFF; //Force Resend //If MiniFT is Alerted Ready, Buttons can be enabled again
		return 0;
	}
	if (g_cMoveDone != MOVEDONE_TRUE)
	{ // disallow move commands unless carriage is done with previous move
		SmartToolMsgMiniFtMessageCode(0, uiSourceOid, MINIFTMC_WAIT_FOR_CARRIAGE_STOP);
		g_cActionSent = 0xFF; //Force Resend
		return 0;
	}
	if (g_cFloatStatus == FLOATSTAT_FLOAT)
	{
		SmartToolMsgMiniFtMessageCode(0, uiSourceOid, MINIFTMC_MOVE_PREVENTED_BY_FLOAT);
		g_cActionSent = 0xFF; //Force Resend
		return 0;
	}
#ifdef GENCIRCMFTX
//Consider this for ALLPATTERN candidate
	if (g_cDrillState != DRILLSTATE_IDLE)
	{
		//no message because the pendant should be preventing the buttons, but
		//just in case, it won't allow movement
		g_cActionSent = 0xFF; //Force Resend
		return 0;
	}
#endif
	return 1;
}

int SpecifyGotoPosn(int iGotoPosn, int iIndex)
{
// "GotoPosn" is the next position as specified by the user...not necessarily the next sequential position in the
// list.  For exmple, the "GotoPosn" could be the previous position, the next sequential position, or some randomly specified position
// iIndex is ignored if iGotoPosn is not GOTOPOSN_RANDOM_INDEX (pass a 0 to the function)
	int iRetVal;
	int i, sr;
	int iPosnIndex;
	int d;
	unsigned int uiMessageCode;
	byte cGotoHolePattern;
	uint16 uiGotoHoleOPS;

	iRetVal = 0;
	iPosnIndex = g_PosnMode.iCurPosnIndex;

	uiMessageCode = 0;
	logf("station %c g %c\r\n", g_cStationCode[g_cStation],
			g_cStationCode[g_cStationGoal]);

	switch (iGotoPosn)
	{
	case GOTOPOSN_NEXT:
		case GOTOPOSN_PREV:
		// DEFAULT: probed BTOA, which means we're walking through it oppositely (A to B)...PosnA is the first posn, PosnB is the final position
		d = 0;
		if (g_iDrillDir == PROBEDIR_ATOB)
		{
			d = 1;
		}
		if (g_iDrillDir == PROBEDIR_BTOA)
		{
			//we're walking through it oppositely (B to A)
			d = -1;
		}
		if (iGotoPosn == GOTOPOSN_PREV)
		{
			d = -d; //reverses dir in this case
		}
		//If Init Move, Must go to 1st position since they never moved at all.
		//This is allready set into the current position variable
		//Otherwise, must move in the direction specified.
		if (g_PosnMode.cFirstMove != 1)
		{
			iPosnIndex += d;
		}
		// keep iRetVal unchanged
		//skip K Holes unless we are allowed to visit them

		if (g_PartPgmInfo.cTeachModeData != 1) //KHoles are never visitable this way in normal mode g_cAllowKVisit==0
		{
			while (iPosnIndex >= 0 && iPosnIndex < g_iHoleCount)
			{
				cGotoHolePattern = g_HoleDataArray[iPosnIndex].cPattern;
				uiGotoHoleOPS = g_HoleDataArray[iPosnIndex].uiOps;

				if (cGotoHolePattern >= 128) //This is restricted From Next and Prev Currently
				{
					//continue move past this khole
					iPosnIndex += d;
					continue;
				}
				if ((uiGotoHoleOPS & 1) == 0)
				{
					logf("nvo%d\r\n", iPosnIndex);
					//can't visit this hole
					iPosnIndex += d;
					continue;
				}
				break;
			}
		}

		if (iPosnIndex < 0 || iPosnIndex >= g_iHoleCount)
		{
			iRetVal = GOTOPOSN_OUTOFBOUNDS;
			if (iGotoPosn == GOTOPOSN_NEXT)
			{
				uiMessageCode = MINIFTMC_FINAL_POSN_KHOLE_VISIT;
				//if they hit next on the last hole, we'll allow Khole visitation
				//also can activate repeat
				//Allow K Hole Visitation
				g_cAllowKVisit = 1;
			}
			else
			{
				uiMessageCode = MINIFTMC_FINAL_POSN;
			}
		}
		//Set goal to UNSPEC always, which causes it to attempt to go to the first operation
		if (g_cStationGoal != STATION_LASERPOINTER)
		{
			g_cStationGoal = STATION_UNSPEC;
		}
		break;
	case GOTOPOSN_RANDOM_INDEX:
		//Use the provided index
		iPosnIndex = iIndex;
		//must be valid hole
		if (iPosnIndex < 0 || iPosnIndex >= g_iHoleCount)
		{
			iRetVal = GOTOPOSN_OUTOFBOUNDS;
			uiMessageCode = MINIFTMC_GOTOPOSN_OUT_OF_BOUNDS;
		}
		//skip K Holes unless we are allowed to visit them

		cGotoHolePattern = g_HoleDataArray[iPosnIndex].cPattern;
		uiGotoHoleOPS = g_HoleDataArray[iPosnIndex].uiOps;

		if (g_cAllowKVisit == 0)
		{
			if (cGotoHolePattern >= 128) //This is restricted From Next and Prev Currently
			{
				//continue move past this khole
				iRetVal = GOTOPOSN_KHOLEPREVENTED;
				uiMessageCode = MINIFTMC_KHOLE_MOVE_PREVENTED;
			}
		}
		if ((uiGotoHoleOPS & 1) == 0)
		{
			logf("nvo%d\r\n", iPosnIndex);
			iRetVal = GOTOPOSN_KHOLEPREVENTED;
			uiMessageCode = MINIFTMC_HOLE_MOVE_PREVENTED;
		}
		//Set goal to UNSPEC always, which causes it to attempt to go to the first operation
		if (g_cStationGoal != STATION_LASERPOINTER)
		{
			g_cStationGoal = STATION_UNSPEC;
		}
		break;
	case GOTOPOSN_NEAREST:
		// search for the nearest posn ID...
		i = FindNearestPosition();
		if (i >= 0) //only set the new position if one was found.
		{
			iPosnIndex = i;
			if (fabs(g_PosnMode.fNearestDistanceX)
					>= g_ConfigData.LongDistance.fX
					|| fabs(g_PosnMode.fNearestDistanceY)
							>= g_ConfigData.LongDistance.fY)
			{
				//This is too far for GOTOPOSN_NEAREST TO ALLOW
				iRetVal = GOTOPOSN_TOOFAR;
				uiMessageCode = 0; //no message... just don't move to nearest
			}
		}
		//Leave whatever Station was set
		break;
	}
	if (iRetVal == 0)
	{
		//can do this move
		g_PosnMode.iGotoPosnIndex = iPosnIndex;
		g_PosnMode.cDoMoveTrig = TRUE;
	}
	if (uiMessageCode != 0)
	{
		//show the message
		SmartToolMsgMiniFtMessageCode(0, COMMON_OID_NULLOID, uiMessageCode);
		g_cActionSent = 0xFF; //Force Resend //If the move was prevented somehow, sending READY again will allow buttons to be enabled
	}
	logf("t=%d\r\n", g_PosnMode.cDoMoveTrig);
	logf("station %c g %c\r\n", g_cStationCode[g_cStation],
			g_cStationCode[g_cStationGoal]);
	return iRetVal;
}

void SpecifyGotoPosnAgain()
{
	int iPosnIndex;
	iPosnIndex = g_PosnMode.iCurPosnIndex;
	g_PosnMode.iGotoPosnIndex = iPosnIndex;
	g_PosnMode.cDoMoveTrig = TRUE;
}

void SetPressureBasedOnProcess(byte cProcess)
{
	if (cProcess == 0)
	{
		g_uiClampPressure = g_ConfigData.uiPressure;
		g_uiClampPressureWarning = 0;
		g_uiClampPressureAbort = 0;
		logf("Pdef=%d\r\n", g_uiClampPressure, cProcess);
	}
	else
	{
		g_uiClampPressure = g_uiProcessPounds[cProcess];
		g_uiClampPressureWarning = g_uiProcessPoundsWarning[cProcess];
		g_uiClampPressureAbort = g_uiProcessPoundsAbort[cProcess];
		logf("Pproc=%d p=%d\r\n", cProcess, g_uiClampPressure);
	}
#warning "what if NAC is disconnected? "
//danger is how it can just want and the when connected suddently clmp and operate
// or it could unclamp and move.

//FIXME9999999999999999
// I don't use this here
// g_PosnMode.iGotoPosnIndex,
// but old version DID... and I found recently that it's set to -1 in final action states.....
//so the point is that I must check to see if it;s used in other placess
}

//FIXME99999999999999999999 Going to need a retest of HD rails....

#ifdef PRESSURE_BY_E2P
void SetClampPressureLBS()
{
	float f;
//Measured Data
//V    LBS  distance up 1 volts
//0 ....    0
//1  = 47   +43
//2  = 94   +50
//3  =144   +62
//4  =206   +64
//5  =270   +64
//6  =336   +98
//7  =434
//Currently using experimental values...
// The values have been modified slightly in addition

	if (g_uiClampPressureLBS < 15)
	{
		f=g_uiClampPressureLBS;
		f=f/75.0;
	}
	else if (g_uiClampPressureLBS <=141)
	{
		f=g_uiClampPressureLBS-5;
		f=f/50.0;
	}
	else if (g_uiClampPressureLBS <=276)
	{
		f=g_uiClampPressureLBS-141;
		f=3 + f/64.0;
	}
	else
	{
		f=g_uiClampPressureLBS-276;
		f=5 + f/98.0;
	}
	if (f>10.05)
	{	f=10.05;}

	logf("PRESSURE SET : pounds %d   volts %.3f \r\n",g_uiClampPressureLBS,f);
	anaOutVolts(DAC_CHANNEL_00, f);
	g_uiClampPressureLBSLastSet=g_uiClampPressureLBS;
}
#endif

void CalculatePreMove(float* p_fPreMoveX, float* p_fPreMoveY)
{
	if (g_GravComp.iDirX > 0)
	{ // gravcomp vector is +X (which means the physical gravity vector is -X)
	  // Always move below the target
		*p_fPreMoveX = -g_ConfigData.PreMove.fX; // make the pre-move negative so that we go below the target
	}
	else
	{ // gravcomp vector is -X (which means the physical gravity vector is +X)
// Always move below the target
		*p_fPreMoveX = g_ConfigData.PreMove.fX; // keep the pre-move positive so that we go below the target
	}

	if (g_GravComp.iDirY > 0)
	{ // gravcomp vector is +Y (which means the physical gravity vector is -Y)
	  // Always move below the target
		*p_fPreMoveY = -g_ConfigData.PreMove.fY; // make the pre-move negative so that we go below the target
	}
	else
	{ // gravcomp vector is -Y (which means the physical gravity vector is +Y)
// Always move below the target
		*p_fPreMoveY = g_ConfigData.PreMove.fY; // keep the pre-move positive so that we go below the target
	}
	logf("PreMove: X=%.3f Y=%.3f\r\n", *p_fPreMoveX, *p_fPreMoveY);   //debug
}

void CalcProbeHome(void)
{
//Not Actually Using Any Versions With ProbeHome Rightnow
#if 0
// This function calculates machine values for K1 and K2 from dataset locations and offset of K1 in the from part program to rail origin.
//	NOTE: This function is only called on success of the Probe Home motion sequence in RunProbeHome().
//	The bumpers we sense determine Y axis tool flip status.
	float RailOriginX, RailOriginY;
	int ikp,iks;

// Using K1 in rivet dataset, calculate K1 reference location in machine coordinates.
//NEW RIVET SYSTEM WILL DOESN'T ADJUST AXIS... SHOULD ID  FIXMEZ-RILEY-ERIC
//if( g_ConfigData.cToolFlip == Y_POS )
//{
//See Below... we ALWAYS  use this code now.
//}
//else if(g_ConfigData.cToolFlip == Y_NEG )
//{
//	//	Flipped Y axis orientation.  We are at Ymax in machine coordinates.
//    RailOriginY = g_Probe.fProbeHomeY + g_ConfigData.PosnLimit.fMaxY;	// Y location of rail cordinate orgin - center of X rail.
//	g_K1MachinePosn.fY = RailOriginY - g_PartPgmInfo.K1OriginDistY; // K1.Y in machine coordinates
//}

// NORMAL Y axis orientation.
	RailOriginY = g_Probe.fHomeY - g_ConfigData.PosnLimit.fMinY;// Y location of rail coordinate origin

	g_fKHolePrbeY[1]=RailOriginY + g_PartPgmInfo.K1OriginDistY;// K1.Y in machine coordinates.

	if(g_Probe.cXBumperDirection == X_BUMP_NEG)
	{
		// We are at lower end of X travel, at the rail Cartesian origin.
		RailOriginX = g_Probe.fHomeX - g_ConfigData.PosnLimit.fMinX;// X location of right end of rail (not signed in Rivet)(so positive makes more to side)
		g_fKHolePrbeX[1] = RailOriginX + g_PartPgmInfo.K1OriginDistX;// K1.X in machine coordinates.
	}
	else if(g_Probe.cXBumperDirection == X_BUMP_POS)
	{
		// We are at far end of X travel (Xmax).
		RailOriginX = g_Probe.fHomeX + g_ConfigData.PosnLimit.fMaxX;// X location of end of rail
		g_fKHolePrbeX[1] = RailOriginX - g_PartPgmInfo.K1EndDistX;// K1.X in machine coordinates.
	}

	g_cKHolePrbeStatus[1]=PS_APPROXIMATE;

//Create K2 data based on difference
	ikp = g_iKHoleHoleIndex[1];
	iks = g_iKHoleHoleIndex[2];
	g_fPKX = xgetfloat((xp_fRawDataX)+(4*ikp));
	g_fPKY = xgetfloat((xp_fRawDataY)+(4*ikp));
	g_fSKX = xgetfloat((xp_fRawDataX)+(4*iks));
	g_fSKY = xgetfloat((xp_fRawDataY)+(4*iks));

//	Now derive K2 Machine locations from Dataset values differences
	g_fKHolePrbeX[2] = g_fKHolePrbeX[1] + (g_fSKX - g_fPKX);
	g_fKHolePrbeY[2] = g_fKHolePrbeY[1] + (g_fSKY - g_fPKY);
	g_cKHolePrbeStatus[2]=PS_APPROXIMATE;

//force a recalculate inside SetGlobalRotation
	g_cRotationKP=0;
	g_cRotationKS=0;
	SetGlobalRotation(1, 2);

	logf( "ProbeHome:\r\n");
	logf( " Rail Coordinate Origin = %.3f, %.3f\r\n", RailOriginX, RailOriginY);
	logf( " KPMachinePosn= %.3f, %.3f\r\n", g_fPKMX, g_fPKMY);
	logf( " KSMachinePosn= %.3f, %.3f\r\n", g_fSKMX, g_fSKMY);
	logf( "\tDatasetKS=%.3f,%.3f; DatasetKS=%.3f,%.3f.\r\n",g_fPKX,g_fPKY, g_fSKX,g_fSKY);
#endif
}

//	RIVET TOOL ONLY:  Calculate corrections to machine <-> dataset conversion offsets after PROBE_ADJUST.
//	NOTE: Corrections are calculated based on last value saved for K1,K2 machine coordinates.
//			This means consecutive Probe Adjust operations could introduce cumulative error.
//			Since rivet operations can span 40 feet, incremental adjustments are preferable to using ProbeHome location as a gold standard.
// MACHINEPOINTS version

#ifdef SUPPORT_PROBE_ADJUST
int CalcProbeAdjust()
{
	float fdX, fdY;		// Temporary to hold result of difference calculation.
	float fProbedMachineX, fProbedMachineY;
	float fExpMachineX, fExpMachineY;
	int i;// Index in pattern of nearest point to curent location

//	PROBE_ADJUST was done with the laser head, so account for the laser head offset from hammer head.
	fProbedMachineX = g_Probe.fProbeAdjustX;
	fProbedMachineY = g_Probe.fProbeAdjustY;

//	Ask FindNearestPosition() to look for the nearest dataset location to the rivet laser location
	ResetNearestPosition();// Do not try to match to the previous nearest position.
//Search with the offset, since we set the laser offset as a tool offset
	g_PosnMode.fLastKnownPosnX = fProbedMachineX;
	g_PosnMode.fLastKnownPosnY = fProbedMachineY;
	logf("ProbeAdjust: Probed machine location = %.3f,%.3f.\r\n", fProbedMachineX, fProbedMachineY);
//But Remove the offset from these to find the value we would have if the tool was up there
	RemoveMachineOffset(&fProbedMachineX,&fProbedMachineY);
	logf("ProbeAdjust: When offset was removed = %.3f,%.3f.\r\n", fProbedMachineX, fProbedMachineY);

	i = FindNearestPosition();
	if (i<0)//	If returned index into dataset is less than zero, no nearest point was found.
	{
		logf("-->ERROR: Probe Adjust couldn't find a nearest dataset location!\r\n");
		return(-1);
	}

// Get the dataset location for the nearest program point from the result of FindNearestPosition.
	fExpMachineX = g_PosnMode.fNearestPosnMX;
	fExpMachineY = g_PosnMode.fNearestPosnMY;
//But this machine position

//	Calculate the offset of probed location from expected location in machine coordinates.
	fdX = fProbedMachineX - fExpMachineX;
	fdY = fProbedMachineY - fExpMachineY;

//	But are we within tolerance?
// Use X and Y box, not true distance
	logf("ProbeAdjust: Nearest point = %d: %.3f, %.3f at distance = %.3f, %.3f.\r\n", i, fExpMachineX, fExpMachineY, -fdX, -fdY );

	if ((fabs(fdX) > g_ConfigData.fProbeAdjustLimit) ||
			(fabs(fdY) > g_ConfigData.fProbeAdjustLimit) )
	{
		// 	Nearest program point is farther than max allowable distance from probed point.
		logf("Nearest rivet location is outside max distance from ProbeAdjust location");
		return(-2);
	}

//	Probed point is acceptable.  Its true machine value is at fdX,fdY returned by RunProbeAdjust().
//	This is a translation, NOT a rotation,
	logf("ProbeAdjust: Adjust %.3f, %.3f.\r\n", fdX, fdY );

//Shift all of the positions
//FIXME LOW  Probe Adjust must move all of the K Holes and other data
//			by the shift value
//	g_K1MachinePosn.fX += fdX;
//    g_K1MachinePosn.fY += fdY;
//	g_K2MachinePosn.fX += fdX;
//    g_K2MachinePosn.fY += fdY;

//	Return index of the matched dataset point to the caller.
	return( i );
}
#else
int CalcProbeAdjust()
{
	return -2;
}
#endif

//SetGlobalRotation(byte cpk, byte csk)

//Global Machine Rotation System
//Set and Cache Globals needed for rotation
//Used both for creating of the Machine points, and for tranformation
// of points from machine data back into data set coordinates.

//FIXME66666666666666666666666 when will it get cleared and stop taking effect??????
//FIXME66666666666666666666666
//Tie in better with probe clearing etc....
//FIXME66666666666666666666666
//Perhaps change setglobal argument system....

void SetGlobalRotation(byte cpk, byte csk)
{
	int ipk, isk;
	float fmd, fmdx, fmdy;
	float fd, fdx, fdy;

	if (cpk != g_cRotationKP || csk != g_cRotationKS)
	{
		//Update the Global Setting Indicators
		g_cRotationKP = cpk;
		g_cRotationKS = csk;

		if (cpk == 0)
		{
			//no k holes yet.
			g_cRotationContext = 0;
			return;
		}

		g_cRotationContext = PS_PROBED;

		if (g_cKHolePrbeStatus[cpk] >= PS_PROBED)
		{
			g_fPKMX = g_fKHolePrbeX[cpk];
			g_fPKMY = g_fKHolePrbeY[cpk];
		}
		else if (g_cKHolePrbeStart[cpk] == PS_APPROXIMATE)
		{
			g_fPKMX = g_fKHolePrbeStartX[cpk];
			g_fPKMY = g_fKHolePrbeStartY[cpk];
			g_cRotationContext = PS_APPROXIMATE;
		}
		else
		{
			//no position for this yet.
			g_cRotationContext = 0;
			return;
		}
		ipk = g_iKHoleHoleIndex[cpk];
		g_fPKX = g_fRawDataX[ipk];
		g_fPKY = g_fRawDataY[ipk];

		logf("cpk=%d ipk=%d x,y= %f %f\r\n", cpk, ipk, g_fPKX, g_fPKY);

		if (csk == 0)
		{
			//just one K Hole... Only use the K position we just loaded
			g_cRotationContext = 0;
			return;
		}

		if (g_cKHolePrbeStatus[csk] >= PS_PROBED)
		{
			g_fSKMX = g_fKHolePrbeX[csk];
			g_fSKMY = g_fKHolePrbeY[csk];
		}
		else if (g_cKHolePrbeStart[cpk] == PS_APPROXIMATE)
		{
			g_fSKMX = g_fKHolePrbeStartX[csk];
			g_fSKMY = g_fKHolePrbeStartY[csk];
			g_cRotationContext = PS_APPROXIMATE;
		}
		else
		{
			//no position for this yet.
			g_cRotationContext = 0;
			return;
		}

		isk = g_iKHoleHoleIndex[csk];
		g_fSKX = g_fRawDataX[isk];
		g_fSKY = g_fRawDataY[isk];

		logf("csk=%d isk=%d x,y= %f %f\r\n", csk, isk, g_fSKX, g_fSKY);

		//Find Cos and Sin Theta for this pair
		//Find Machine delta from the probed positions
		fmdx = g_fSKMX - g_fPKMX;
		fmdy = g_fSKMY - g_fPKMY;
		//Find the distance in machine coordinates
		fmd = sqrt(fmdx * fmdx + fmdy * fmdy);
		if (fmd < 0.001)
		{
			//Has to be a failure...
			//just set KS to nothing
			g_cRotationKS = 0;

			//Clear Probe and reset.
			SmartToolMsgMiniFtMessageCode(0, COMMON_OID_NULLOID, MINIFTMC_ROTATION_POINTS_TOO_CLOSE);
			logf("Pnts close!\r\n");

			if (g_cAction == ACTION_CALC)
			{
				logf("to IDLE\r\n");
				StartOver();            //SPSALL
				//go to IDLE
				g_cModeState = MODE_IDLE;
			}
			return;
		}
		//Find unit vector pointing from PK to SK in machine coords
		fmdx = fmdx / fmd;
		fmdy = fmdy / fmd;

		//Find vector from DataSet PK to DataSet SK
		fdx = g_fSKX - g_fPKX;
		fdy = g_fSKY - g_fPKY;
		//Find the distance in dataset coordinates
		fd = sqrt(fdx * fdx + fdy * fdy);
		if (fd < 0.001)
		{
			//Has to be a failure
			//Clear Probe and reset.
			SmartToolMsgMiniFtMessageCode(0, COMMON_OID_NULLOID, MINIFTMC_ROTATION_POINTS_TOO_CLOSE);
			StartOver();            //SPSALL
			//go to IDLE
			logf("IDLE low d\r\n");
			g_cModeState = MODE_IDLE;
			return;
		}
		//Find unit vector from DataSet PK to DataSet SK
		fdx = fdx / fd;
		fdy = fdy / fd;

		// moving one unit of measurment along machine vector moves one unit vector along data set vector.

#ifdef LINEAR_SCALE_OPTION
		if (g_ConfigData.cScaleMode == 0)
		{
#endif

			//cosine of unit vectors:
			// cosine theta = ax * bx + ay * by
			g_fCosTheta = fmdx * fdx + fmdy * fdy;

			//sine of unit vectors:
			// sine theta = ax * by - ay * bx
			g_fSinTheta = fmdx * fdy - fmdy * fdx;

			logf(" theta c s = %f %f\n", g_fCosTheta, g_fSinTheta);

#ifdef LINEAR_SCALE_OPTION
		}
		else
		{
			g_fScale = fmd / fd;
			g_fUnscale = fd / fmd;

			//The Cos and Sin for rotation from X,0 to fmdx,fmdy
			g_fCosScale = fmdx;
			g_fSinScale = fmdy;
			logf(" scale c s = %f %f\r\n", g_fCosScale, g_fSinScale);

			//The Cos and Sin for rotation from X,0 to fdx,fdy
			g_fCosTheta = fdx;
			g_fSinTheta = fdy;
			logf(" theta c s = %f %f\n", g_fCosTheta, g_fSinTheta);
		}
#endif
		//Done setting variables needed for rotation
	}
}

//FIXME HIGH  OrthoSlope is ignored
// I'm not even sure where to apply this currently
//#ifdef POSN_BASIS_K1
//    frX -= g_ConfigData.fOrthoSlope * (frY - g_K1MachinePosn.fY);
//#else
//    frX -= g_ConfigData.fOrthoSlope * (frY - g_K2MachinePosn.fY);
//#endif
//logf("ORTHOUT\t%.3f\t\t%.3f\r\n",frX,frY);

//Trasformation from Dataset coordinates to Machine coordinates

//MACHINEPOINTS
//StartCreateMachineDataSet()
//CreateMachineDataSet()
//CreatePosnHashList()

//When completing probe mode, we must calculate a machine position for each hole.
//This allows search for the nearest hole in machine positions, and allows us to
//go to the position without transforming the point again.

//Testing found that CreateMachineDataSet takes about 2ms per program point.
//For this reason, we must do this in parts

void StartCreateMachineDataSet()
{
	int i;
	g_ulMachineDataSetIDTimeSec = SEC_TIMER;
	g_ulMachineDataSetIDTime = MS_TIMER;
	g_ulCreateMachineDataSetStartTime = MS_TIMER;
	g_uiCreateMachineDataSetIndex = 0;
	g_uiCreateMachineDataSetHashIndex = 0;

//Stop SendXYData (because they will need new data)
	g_cSendXYDataSessions = 0;
	i = 0;
	while (i < MAXNUM_STP_SESSIONS)
	{
		g_cSendXYDataSession[i] = 0;
		i++;
	}
//Alert new MachineDataSetID now
	SendXYDataID(0, STP_ALERT);

//FIXME000000000000 make sure that these can't be used for lookup during this time....
// NO lookup during this time....

	g_fMPosnHashMinBucket = g_fKHolePrbeX[1]; //use first khole as a start for min and max
	g_fMPosnHashMaxBucket = g_fKHolePrbeY[1];

//Clear the Hash List
	i = 0;
	while (i < MPOSNHASHSIZE)
	{
		g_iMPosnBucket[i] = -1;
		i++;
	}

//Clear this so that it will recalculate them all here
	g_cRotationKP = 0;
	g_cRotationKS = 0;
}

void CreateMachineDataSet()
{
	unsigned int i, ki, istart, imax;
	byte ck;
	byte cki;
	byte cpk;
	byte csk;
	int ipk;
	int isk;
	//char *p_c;
	byte cwarn;
	uint32 ui;

// DataSet to Machine
	float fX, fY, frX, frY, fsrX, fsrY; //  temporary variables for rotated location.
	float f;
	unsigned long ul;

	istart = g_uiCreateMachineDataSetIndex;
	imax = istart + 64;
	if (imax >= g_iHoleCount)
	{
		imax = g_iHoleCount;
	}

	if (g_PartPgmInfo.cTeachModeData == 1)
	{
		i = 0;
		imax = g_iHoleCount;
		while (i < imax)
		{
			//MX DATA
			frX = g_fRawDataMX[i];
			//MY DATA
			frY = g_fRawDataMY[i];
			if (frX < g_fMPosnHashMinBucket)
			{
				g_fMPosnHashMinBucket = frX;
			}
			else if (frX > g_fMPosnHashMaxBucket)
			{
				g_fMPosnHashMaxBucket = frX;
			}
			i++;
		}
		g_uiCreateMachineDataSetIndex = g_iHoleCount;
		goto skip_standard_create_machine_dataset;
	}
//Now For all points in the pattern, set the rotated value

//Use Rotate but not Tool Offset....
//These are machine coords for the 0,0 position ...
	i = istart;
	while (i < imax)
	{
		cki = g_HoleDataArray[i].cKInd;
		cpk = g_HoleDataArray[i].cKPri;
		csk = g_HoleDataArray[i].cKSec;
		if (cki > 0)
		{
			//It's a K Hole or other Locating feature... use the value established by the probe system
			frX = g_fKHolePrbeX[cki]; //use first khole as a start for min and max
			frY = g_fKHolePrbeY[cki];
		}
		else
		{

			if (cpk != g_cRotationKP || csk != g_cRotationKS) //do check here to avoid the call even though the function also checks
			{
				SetGlobalRotation(cpk, csk);
			}

			//Load the Point we are going to translate into machine coordinates

			fX = g_fRawDataX[i];
			fY = g_fRawDataY[i];
#ifdef OUTPUT_POSITION_INFO_CREATION
			logf("Extracted %f,%f\r\n",fX,fY);
#endif
			//Translate
			//	Drill & DrillFill programs arbitrarily set the dataset basis location to machine location 0,0.
			//	Translate the input dataset position to a position relative to dataset position 0,0

			fX -= g_fPKX;
			fY -= g_fPKY;
#ifdef OUTPUT_POSITION_INFO_CREATION
			logf("Translated to %f,%f\r\n",fX,fY);
#endif

			//Rotate Dataset to Machine
#ifdef LINEAR_SCALE_OPTION
			if (g_ConfigData.cScaleMode == 0)
			{
#endif

				//Use the reverse values since the values are meant for Machine to Dataset, and we are going back.
				//cos(-x) = cos(x) //no change needed
				//sin(-x) = -sin(x) //reverse sign on second term to reverse rotation.
				frX = fX * g_fCosTheta + fY * g_fSinTheta;
				frY = fY * g_fCosTheta - fX * g_fSinTheta;
#ifdef OUTPUT_POSITION_INFO_CREATION
				logf("Rotation to %f,%f\r\n",frX,frY);
#endif

#ifdef LINEAR_SCALE_OPTION
			}
			else
			{
				//The CosTheta and SinTheta for rotation from X,0 to fdx,fdy
				//Use Reverse Rotation to orient the dataset fdx,fdy along X
				//logf("Dataset Prior %f %f\r\n",fX,fY);
				fsrX = fX * g_fCosTheta + fY * g_fSinTheta;
				fsrY = fY * g_fCosTheta - fX * g_fSinTheta;
				//Scale Just along the direction from  khole to khole
				//logf("scale %f %f s=%f\r\n",fsrX,fsrY,g_fScale);
				fsrX *= g_fScale;
				//Now rotate from these scale coords to machine cordinates
				//The CosScale and SinScale for rotation from X,0 to fmdx,fmdy
				//Use standard rotation to orient the scale x,y to the machine probe vector fmdx,fmdy
				frX = fsrX * g_fCosScale - fsrY * g_fSinScale;
				frY = fsrY * g_fCosScale + fsrX * g_fSinScale;
				//logf("Machine Post %f %f\r\n",frX,frY);
			}
#endif

			//Translate 0,0 to machine position matching the dataset position used above
			frX += g_fPKMX;
			frY += g_fPKMY;
#ifdef OUTPUT_POSITION_INFO_CREATION
			logf("Machine Translation to %f,%f\r\n",frX,frY);
#endif

		}
		//MX DATA
		g_fRawDataMX[i] = frX;
		//MY DATA
		g_fRawDataMY[i] = frY;

		//and while doing this, set warnings...
		cwarn = 0;
		if (cki > 0)
		{
			//It's a K Hole or other Locating feature... use the value established by the probe system
			cwarn = g_cKHolePrbeStatusWarnings[cki]; //just copy
		}
		if (cpk > 0 && cwarn == 0)
		{
			//It has a primary locating hole
			cwarn = g_cKHolePrbeStatusWarnings[cpk];
		}
		if (csk > 0 && cwarn == 0)
		{
			//It has a secondary locating hole
			cwarn = g_cKHolePrbeStatusWarnings[csk];
		}
		if (cwarn > 0)
		{
			//it has a warning
			//Use Operations to record this
			ui = OP_PROBE_WARNINGS;
			AddOpHistory(i, ui);
			logf("h%d opw\r\n");
		}

		if (frX < g_fMPosnHashMinBucket)
		{
			g_fMPosnHashMinBucket = frX;
		}
		else if (frX > g_fMPosnHashMaxBucket)
		{
			g_fMPosnHashMaxBucket = frX;
		}
		i++;
	}
	g_uiCreateMachineDataSetIndex = i; //save the value we got to

	if (g_uiCreateMachineDataSetIndex >= g_iHoleCount)
	{
		skip_standard_create_machine_dataset:
		//must have just completed this loop
		//now also have the low and high.
		g_fMPosnHashBucketsize = g_fMPosnHashMaxBucket - g_fMPosnHashMinBucket;
		g_fMPosnHashBucketsize = g_fMPosnHashBucketsize + 0.001;
		g_fMPosnHashBucketsize = g_fMPosnHashBucketsize / 256;

		i = g_iHoleCount;
		ui = MS_TIMER - g_ulCreateMachineDataSetStartTime;
		f = ((float) ui / (float) i);
		logf("%d items %u ms  %f per item\r\n", i, ui, f);
	}
	return;
}

void CreatePosnHashList()
{
	unsigned int i, istart, imax;

	float f;
	float finverted;
	t_MPOSNBUCKETTYPE bucket;
	uint32 ui;

	istart = g_uiCreateMachineDataSetHashIndex;
	imax = istart + 128;
	if (imax >= g_iHoleCount)
	{
		imax = g_iHoleCount;
	}
	if (istart == 0)
	{
		//first one of these, so reset time for time test
		g_ulCreateMachineDataSetStartTime = MS_TIMER; //get diff time for 2nd part.
	}

	finverted = 1.0 / g_fMPosnHashBucketsize;

	i = istart;
	while (i < imax)
	{
		f = g_fRawDataMX[i];
		//logf("X=%f ",f);
		f -= g_fMPosnHashMinBucket;

		//f = f / g_fMPosnHashBucketsize;
		f = f * finverted; //multiplication is faster

		bucket = (t_MPOSNBUCKETTYPE) f;
		//logf("f=%f ibucket=%d\r\n", f, bucket);

		//link the current bucket contents
		g_iMPosnHashList[i] = g_iMPosnBucket[bucket];
		//replace the bucket's top item
		g_iMPosnBucket[bucket] = i;

		i++;
	}
	g_uiCreateMachineDataSetHashIndex = i;

	if (g_uiCreateMachineDataSetHashIndex >= g_iHoleCount)
	{
		//Must have just completed this loop
#ifdef OUTPUT_POSITION_INFO
		g_uiProgramRotationOutputIndex=0;
#endif

		i = g_iHoleCount;
		ui = MS_TIMER - g_ulCreateMachineDataSetStartTime;
		f = ((float) ui / (float) i);
		logf("%d items %u ms  %f per item\r\n", i, ui, f);
	}
	return;
}

#ifdef OUTPUT_POSITION_INFO
void ShowProgramRotationOutput()
{
	float fX,fY,fMX,fMY;
	if (g_uiProgramRotationOutputIndex < g_iHoleCount)
	{
		fX = xgetfloat((xp_fRawDataX)+(4*g_uiProgramRotationOutputIndex));
		fY = xgetfloat((xp_fRawDataY)+(4*g_uiProgramRotationOutputIndex));
		fMX = xgetfloat((xp_fRawDataMX)+(4*g_uiProgramRotationOutputIndex));
		fMY = xgetfloat((xp_fRawDataMY)+(4*g_uiProgramRotationOutputIndex));
		logf("%d",g_uiProgramRotationOutputIndex);
		logf(" D= %.4f, %.4f",fX,fY);
		logf(" M= %.4f, %.4f\r\n",fMX,fMY);
		g_uiProgramRotationOutputIndex++;
	}
}
#endif

byte CreateMachineDataSetForOneHole(unsigned int i, float * fpX, float * fpY)
{
	//unsigned int ki, istart, imax;
	byte ck;
	byte cki;
	byte cpk;
	byte csk;
	//int ipk;
	//int isk;
	//char *p_c;

// DataSet to Machine
	float fX, fY, frX, frY, fsrX, fsrY; //  temporary variables for rotated location.
	//float f;
	//unsigned long ul;
	byte result;

	result = 0; //invalid

	if (i >= g_iHoleCount)
	{
		return 0;
	}

	if (g_PartPgmInfo.cTeachModeData == 1)
	{
		//MX DATA
		frX = g_fRawDataMX[i];
		//MY DATA
		frY = g_fRawDataMY[i];
		result = 1;
		goto skip_strait_to_output;
	}
//Use Rotate but not Tool Offset....
//These are machine coords for the 0,0 position ...

	cki = g_HoleDataArray[i].cKInd;
	cpk = g_HoleDataArray[i].cKPri;
	csk = g_HoleDataArray[i].cKSec;
	if (cpk == 0)
	{
		//It's a K Hole or other Locating feature that has No References
		frX = g_fKHolePrbeX[cki]; //use first khole as a start for min and max
		frY = g_fKHolePrbeY[cki];
		//this is not a derived hole that is placed accurately based on other features, so return 0 for the purposes of this function
		goto skip_strait_to_output;
	}
	else
	{

		if (cpk != g_cRotationKP || csk != g_cRotationKS) //do check here to avoid the call even though the function also checks
		{
			SetGlobalRotation(cpk, csk);
		}
		if (g_cRotationContext < PS_PENDING_ACCEPTANCE)
		{
			//can't find exact machine data set for this hole
			logf("sta\r\n");
			return 0;
		}

		//Load the Point we are going to translate into machine coordinates

		fX = g_fRawDataX[i];
		fY = g_fRawDataY[i];
#ifdef OUTPUT_POSITION_INFO_CREATION
		logf("Extracted %f,%f\r\n",fX,fY);
#endif
		//Translate
		//	Drill & DrillFill programs arbitrarily set the dataset basis location to machine location 0,0.
		//	Translate the input dataset position to a position relative to dataset position 0,0

		fX -= g_fPKX;
		fY -= g_fPKY;
#ifdef OUTPUT_POSITION_INFO_CREATION
		logf("Translated to %f,%f\r\n",fX,fY);
#endif

		//Rotate Dataset to Machine
#ifdef LINEAR_SCALE_OPTION
		if (g_ConfigData.cScaleMode == 0)
		{
#endif
			//Use the reverse values since the values are meant for Machine to Dataset, and we are going back.
			//cos(-x) = cos(x) //no change needed
			//sin(-x) = -sin(x) //reverse sign on second term to reverse rotation.
			frX = fX * g_fCosTheta + fY * g_fSinTheta;
			frY = fY * g_fCosTheta - fX * g_fSinTheta;
#ifdef OUTPUT_POSITION_INFO_CREATION
			logf("Rotation to %f,%f\r\n",frX,frY);
#endif

#ifdef LINEAR_SCALE_OPTION
		}
		else
		{
			//The CosTheta and SinTheta for rotation from X,0 to fdx,fdy
			//Use Reverse Rotation to orient the dataset fdx,fdy along X
			//logf("Dataset Prior %f %f\r\n",fX,fY);
			fsrX = fX * g_fCosTheta + fY * g_fSinTheta;
			fsrY = fY * g_fCosTheta - fX * g_fSinTheta;
			//Scale Just along the direction from  khole to khole
			//logf("scale %f %f s=%f\r\n",fsrX,fsrY,g_fScale);
			fsrX *= g_fScale;
			//Now rotate from these scale coords to machine cordinates
			//The CosScale and SinScale for rotation from X,0 to fmdx,fmdy
			//Use standard rotation to orient the scale x,y to the machine probe vector fmdx,fmdy
			frX = fsrX * g_fCosScale - fsrY * g_fSinScale;
			frY = fsrY * g_fCosScale + fsrX * g_fSinScale;
			//logf("Machine Post %f %f\r\n",frX,frY);
		}
#endif

		//Translate 0,0 to machine position matching the dataset position used above
		frX += g_fPKMX;
		frY += g_fPKMY;
#ifdef OUTPUT_POSITION_INFO_CREATION
		logf("Machine Translation to %f,%f\r\n",frX,frY);
#endif
		result = 1; //valid

	}
	skip_strait_to_output:
	*fpX = frX;
	*fpY = frY;
	return result;
}

//Rotate this data vec to machine vec in the same orientation of this hole's dataset.
byte RotateVecDataSetToMachine(unsigned int i, float * fpX, float * fpY)
{
	//unsigned istart;
	//unsigned int ki;
	//unsigned int imax;
	//byte ck;
	byte cki;
	byte cpk;
	byte csk;
	//int ipk;
	//int isk;
	//char *p_c;

// DataSet to Machine
	float fX, fY, frX, frY, fsrX, fsrY; //  temporary variables for rotated location.
	//float f;
	//unsigned long ul;
	byte result;

	result = 0; //invalid

	if (i >= g_iHoleCount)
	{
		return result;
	}

	if (g_PartPgmInfo.cTeachModeData == 1)
	{
		//no rotation
		result = 1;
		return result;
	}
//Use Rotate but not Tool Offset....
//These are machine coords for the 0,0 position ...

	cki = g_HoleDataArray[i].cKInd;
	cpk = g_HoleDataArray[i].cKPri;
	csk = g_HoleDataArray[i].cKSec;
	if (cpk == 0)
	{
		//this is not a derived hole that is placed accurately based on other features, so return 0 for the purposes of this function
		return result;
	}

	if (cpk != g_cRotationKP || csk != g_cRotationKS) //do check here to avoid the call even though the function also checks
	{
		SetGlobalRotation(cpk, csk);
	}
	if (g_cRotationContext < PS_APPROXIMATE)
	{
		//can't find exact vector OR even approximate vector for this
		logf("sta\r\n");
		return result;
	}

	fX = *fpX;
	fY = *fpY;
	logf("Vec %f,%f\r\n", fX, fY);

//Rotate Dataset to Machine
#ifdef LINEAR_SCALE_OPTION
	if (g_ConfigData.cScaleMode == 0)
	{
#endif
		//Use the reverse values since the values are meant for Machine to Dataset, and we are going back.
		//cos(-x) = cos(x) //no change needed
		//sin(-x) = -sin(x) //reverse sign on second term to reverse rotation.
		frX = fX * g_fCosTheta + fY * g_fSinTheta;
		frY = fY * g_fCosTheta - fX * g_fSinTheta;

#ifdef LINEAR_SCALE_OPTION
	}
	else
	{
		//The CosTheta and SinTheta for rotation from X,0 to fdx,fdy
		//Use Reverse Rotation to orient the dataset fdx,fdy along X
		//logf("Dataset Prior %f %f\r\n",fX,fY);
		fsrX = fX * g_fCosTheta + fY * g_fSinTheta;
		fsrY = fY * g_fCosTheta - fX * g_fSinTheta;
		//Scale Just along the direction from  khole to khole
		//logf("scale %f %f s=%f\r\n",fsrX,fsrY,g_fScale);
		fsrX *= g_fScale;
		//Now rotate from these scale coords to machine cordinates
		//The CosScale and SinScale for rotation from X,0 to fmdx,fmdy
		//Use standard rotation to orient the scale x,y to the machine probe vector fmdx,fmdy
		frX = fsrX * g_fCosScale - fsrY * g_fSinScale;
		frY = fsrY * g_fCosScale + fsrX * g_fSinScale;
		//logf("Machine Post %f %f\r\n",frX,frY);
	}
#endif

#ifdef OUTPUT_POSITION_INFO_CREATION
	logf("Rot to %f,%f\r\n",frX,frY);
#endif
	*fpX = frX;
	*fpY = frY;
	result = 1; //valid
	return result;
}

//Rotate this vec from machine orientation into data set orientation
byte RotateVecMachineToDataSet(unsigned int i, float * fpX, float * fpY)
{
	//unsigned int ki;
	//unsigned int istart;
	//unsigned int imax;
	//byte ck;
	byte cki;
	byte cpk;
	byte csk;
	//int ipk;
	//int isk;
	//char *p_c;

// DataSet to Machine
	float fX, fY, frX, frY, fsrX, fsrY; //  temporary variables for rotated location.
	//float f;
	//unsigned long ul;
	byte result;

	result = 0; //invalid

	if (i >= g_iHoleCount)
	{
		return result;
	}

	if (g_PartPgmInfo.cTeachModeData == 1)
	{
		//no rotation
		result = 1;
		return result;
	}
//Use Rotate but not Tool Offset....
//These are machine coords for the 0,0 position ...

	cki = g_HoleDataArray[i].cKInd;
	cpk = g_HoleDataArray[i].cKPri;
	csk = g_HoleDataArray[i].cKSec;
	if (cpk == 0)
	{
		//this is not a derived hole that is placed accurately based on other features, so return 0 for the purposes of this function
		return result;
	}

	if (cpk != g_cRotationKP || csk != g_cRotationKS) //do check here to avoid the call even though the function also checks
	{
		SetGlobalRotation(cpk, csk);
	}
	if (g_cRotationContext < PS_APPROXIMATE)
	{
		//can't find exact vector OR even approximate vector for this
		logf("sta\r\n");
		return result;
	}

	fX = *fpX;
	fY = *fpY;
	logf("Vec %f,%f\r\n", fX, fY);

//Rotate Machine to Dataset
#ifdef LINEAR_SCALE_OPTION
	if (g_ConfigData.cScaleMode == 0)
	{
#endif

		//Rotate
		frX = fX * g_fCosTheta - fY * g_fSinTheta;
		frY = fY * g_fCosTheta + fX * g_fSinTheta;

#ifdef LINEAR_SCALE_OPTION
	}
	else
	{
		//Rotate from these machine cordinates to scale coords
		//The CosScale and SinScale for rotation from X,0 to fmdx,fmdy
		//Use Reverse Rotation to orient the machine probe vector fmdx,fmdy along X ( scale x,y coords )
		//logf("Machine Prior %f %f\r\n",fX,fY);
		fsrX = fX * g_fCosScale + fY * g_fSinScale;
		fsrY = fY * g_fCosScale - fX * g_fSinScale;
		//Unscale Just along the direction from  khole to khole
		//logf("uscale %f %f us=%f\r\n",fsrX,fsrY,g_fUnscale);
		fsrX *= g_fUnscale;
		//Rotate from these scale coords to data set coords
		//The CosTheta and SinTheta for rotation from X,0 to fdx,fdy
		//Use standard rotation to orient the scale x,y to the data set vector fdx,fdy
		frX = fsrX * g_fCosTheta - fsrY * g_fSinTheta;
		frY = fsrY * g_fCosTheta + fsrX * g_fSinTheta;
		//logf("Dataset Post %f %f\r\n",frX,frY);
	}
#endif

	logf("Rot to %f,%f\r\n", frX, frY);
	*fpX = frX;
	*fpY = frY;
	result = 1; //valid
	return result;
}

//Trasformation from Machine coordinates To Dataset coordinates

//ProbeModeSelectAndSetGlobalRotation()

//While still in probe mode, we wish to be able to translate machine position into meaningful
//postion information for the user.
//This function looks at the probed, approximately known, and extrapolated position information
//and selects a pair of K Holes to use for the Global Rotation System.
//This function is called after each time the probe positions are updated.

//Method: Search the probed K Holes to find the closest 2 K Holes to the current machine position.
//If nothing is found for K1 and K2 or for just K2, search again including any aproximately known holes.

void ProbeModeSelectAndSetGlobalRotation()
{
	byte cStatus;
	byte ck1;
	byte ck2;
	byte ck1a;
	byte ck2a;
	byte ck;

//While Probing we need to maintain the rotation system so that the displayed coordinates
//are useful to the operator.

//Find the nearest probed K Holes, and nearest approx K Holes as an alternate.
	ck1 = 0;
	ck2 = 0;
	ck1a = 0;
	ck2a = 0;
	ck = 1;
//FIXME6666666666666666 find the nearest not just the 1st......
	while (ck <= g_cKHoleCount)
	{
		//FIXME0000000000000 notice same-XY assmption in all code here
		cStatus = g_cKHolePrbeStatus[ck];
		if (cStatus >= PS_PENDING_ACCEPTANCE)
		{
			if (ck1 == 0)
			{
				ck1 = ck; //save this as k1 in this flatXY
			}
			else if (ck2 == 0)
			{
				ck2 = ck; //save this as k2 in this flatXY
			}
		}
		else if (cStatus >= PS_EXTRAPOLATED && cStatus != PS_PROBING)
		{
			if (ck1a == 0)
			{
				ck1a = ck;
			}
			else if (ck2a == 0)
			{
				ck2a = ck;
			}
		}
		ck++;
	}
	if (ck1 == 0)
	{
		//didn't find any probed... use best approx
		ck1 = ck1a;
		ck2 = ck2a;
	}
	else if (ck2 == 0)
	{
		//found one probed... use approx for number 2
		ck2 = ck1a;
	}
//Now whatever we found, use this as the current global rotation.
	if (g_cRotationKP == ck1 && g_cRotationKS == ck2)
	{
		//force a recalculate inside SetGlobalRotation
		g_cRotationKP = 0;
		g_cRotationKS = 0;
	}
	SetGlobalRotation(ck1, ck2);
}

//RotateMachineToDataset()
//
//This function provides coordinate translation from the machine position to data set positions.
//The rotation used is the rotation established by the Global Rotation System.
//During Probe Mode, the global rotation system is set by ProbeModeSelectAndSetGlobalRotation()

void RotateMachineToDataset(float fX, float fY, float* p_fRotatedX,
		float* p_fRotatedY)
{
	float frX, frY, fsrX, fsrY;	// Temporary rotated dataset location.
// Machine to DataSet

#ifdef OUTPUT_POSITION_INFO_MACHINE_TO_DATASET
	logf(">>>RM2D: M= %.4f, %.4f",fX,fY);
#endif

//Always use the machine offset value given
	RemoveMachineOffset(&fX, &fY);

//FIXME MED TEACH MODE
// Fixme : I want to get rid of that teach mode flag if possible and use the g_cKHoleP here.....
	if (g_PartPgmInfo.cTeachModeData == 1 || g_cRotationKP == 0)
	{
		//Teach mode data is in machine coordinates.  Also return no rotation when no K1 probed...
		*p_fRotatedX = fX;
		*p_fRotatedY = fY;
		return;
	}

	if (g_cRotationKS == 0)
	{
		//Found a Primary K Hole, but no secondary....
		//Just Translate this
		fX = fX - g_fPKMX + g_fPKX;
		fY = fY - g_fPKMY + g_fPKY;
		*p_fRotatedX = fX;
		*p_fRotatedY = fY;
		return;
	}

// compensate for x-y orthogonality
// reverse the above
//FIXME HIGH  OrthoSlope is ignored
// I'm not even sure where to apply this currently
//#ifdef POSN_BASIS_K1
//    fX += g_ConfigData.fOrthoSlope * (fY - g_K1MachinePosn.fY);;
//#else
//    fX += g_ConfigData.fOrthoSlope * (fY - g_K2MachinePosn.fY);
//#endif

//Translate Rotation Center to 0,0 from Machine Center of Rotation
	fX -= g_fPKMX;
	fY -= g_fPKMY;
#ifdef OUTPUT_POSITION_INFO_MACHINE_TO_DATASET_VERBOSE
	logf("   R Machine Translation to %f,%f\r\n",fX,fY);
#endif

//Rotate Machine to Dataset
#ifdef LINEAR_SCALE_OPTION
	if (g_ConfigData.cScaleMode == 0)
	{
#endif

		//Rotate
		frX = fX * g_fCosTheta - fY * g_fSinTheta;
		frY = fY * g_fCosTheta + fX * g_fSinTheta;
#ifdef OUTPUT_POSITION_INFO_MACHINE_TO_DATASET_VERBOSE
		logf("   R Rot to %f,%f\r\n",frX,frY);
#endif

#ifdef LINEAR_SCALE_OPTION
	}
	else
	{
		//Rotate from these machine cordinates to scale coords
		//The CosScale and SinScale for rotation from X,0 to fmdx,fmdy
		//Use Reverse Rotation to orient the machine probe vector fmdx,fmdy along X ( scale x,y coords )
		//logf("Machine Prior %f %f\r\n",fX,fY);
		fsrX = fX * g_fCosScale + fY * g_fSinScale;
		fsrY = fY * g_fCosScale - fX * g_fSinScale;
		//Unscale Just along the direction from  khole to khole
		//logf("uscale %f %f us=%f\r\n",fsrX,fsrY,g_fUnscale);
		fsrX *= g_fUnscale;
		//Rotate from these scale coords to data set coords
		//The CosTheta and SinTheta for rotation from X,0 to fdx,fdy
		//Use standard rotation to orient the scale x,y to the data set vector fdx,fdy
		frX = fsrX * g_fCosTheta - fsrY * g_fSinTheta;
		frY = fsrY * g_fCosTheta + fsrX * g_fSinTheta;
		//logf("Dataset Post %f %f\r\n",frX,frY);
	}
#endif

//Translate 0,0 to dataset position matching the machine position used above
	frX += g_fPKX;
	frY += g_fPKY;

#ifdef OUTPUT_POSITION_INFO_MACHINE_TO_DATASET
	logf("  D= %.4f, %.4f\r\n",frX,frY);
#endif

	*p_fRotatedX = frX;
	*p_fRotatedY = frY;
	return;
}

void ApplyMachineOffset(float * p_fX, float * p_fY)
{
	float fx, fy, fy1;
	float fyadj;
	fx = *p_fX;
	fy = *p_fY;

	if (g_cMachineOffsetCompensationAdjustment == 0)
	{
		//just add normal offsets
		fx += g_MachineOffset.fX;
		fy += g_MachineOffset.fY;
	}
	else
	{
		//must apply special offset
		//find offset per inch
		fyadj = (g_MachineOffset2.fY - g_MachineOffset1.fY)
				/ (g_MachineOffset2.fYExtension - g_MachineOffset1.fYExtension);

		//current extension = (fy - base)
		//extension passed offset one extension = (fy - base) - offset one extension

		fy1 = fy;
		fx += g_MachineOffset1.fX;
		fy = fy + g_MachineOffset1.fY
				+ fyadj
						* ((fy - g_fMachineBaseExtension)
								- g_MachineOffset1.fYExtension);
		logf("\r\n adj %.4f bo %.4f y %.4f %.4f \r\n", fyadj,
				(fy1 - g_fMachineBaseExtension), fy1, fy);
	}
	*p_fX = fx;
	*p_fY = fy;
}

void RemoveMachineOffset(float * p_fX, float * p_fY)
{
	float fx, fy;
	float fyadj;
	fx = *p_fX;
	fy = *p_fY;

	if (g_cMachineOffsetCompensationAdjustment == 0)
	{
		//just normal offsets
		fx -= g_MachineOffset.fX;
		fy -= g_MachineOffset.fY;
	}
	else
	{
		//special offset
		//find offset per inch
		fyadj = (g_MachineOffset2.fY - g_MachineOffset1.fY)
				/ (g_MachineOffset2.fYExtension - g_MachineOffset1.fYExtension);

		//Backwards version of above (solve for rvalue fy in above to get this equation)
		fx -= g_MachineOffset1.fX;
		fy = (fy - g_MachineOffset1.fY
				+ fyadj
						* (g_fMachineBaseExtension
								+ g_MachineOffset1.fYExtension)) / (1 + fyadj);

	}
	*p_fX = fx;
	*p_fY = fy;
}

void SetToolOffset(float fToolOffsetX, float fToolOffsetY)
{
	td_MachineOffset * p_MachineOffset;
//flipping controls an extra 180 degree rotation.
//if tool is in pos direction, flip only for Tool to Movement conversion (machine offset if oposite tool offset.)
//if tool is in neg direction, flip for Tool to Movement conversion, and flip again since tool is oriented on oposite side 180.
//	two flips cancel out (-1 * -1 = 1) so do nothing.
	g_cMachineOffsetCompensationAdjustment = 0;
	if (g_ConfigData.cToolFlip == Y_POS)
	{
		g_MachineOffset.fX = -fToolOffsetX;
		g_MachineOffset.fY = -fToolOffsetY;
	}
	else if (g_ConfigData.cToolFlip == Y_NEG)
	{
		g_MachineOffset.fX = fToolOffsetX;
		g_MachineOffset.fY = fToolOffsetY;
	}
	else
	{
		//ToolFlip is UNKNOWN : Must be set
		logf("ERROR:stou\r\n"); //SetToolOffset used, but flip is UNKNOWN\r\n");
		//MAKE THIS KILL POSNMODE FOR SAFETY
		g_cModeState = MODE_IDLE;
	}
#ifdef OUTPUT_TOOL_OFFSET
	logf("STARTING Tool Offset : MO= ( %.3f , %.3f ) flip=%d\r\n",g_MachineOffset.fX,g_MachineOffset.fY,g_ConfigData.cToolFlip);
#endif
	p_MachineOffset = (td_MachineOffset *) g_STPtxMsg.p_cObjectValue;
	p_MachineOffset->fX = g_MachineOffset.fX;
	p_MachineOffset->fY = g_MachineOffset.fY;
	SmartToolMsg(0, STP_ALERT, MINIFT_OID_MACHINE_OFFSET, sizeof(td_MachineOffset), (char *) p_MachineOffset);
}

//FIXME000000000 how would I alert if I'm using this....  relates to future of HD
void SetToolOffsetWithYComp(float fToolOffset1X, float fToolOffset1Y,
		float fToolOffset1YExtension, float fToolOffset2X, float fToolOffset2Y,
		float fToolOffset2YExtension)
{
	if (g_ConfigData.cToolFlip == Y_POS)
	{
		g_cMachineOffsetCompensationAdjustment = 1;
		g_MachineOffset1.fX = -fToolOffset1X;
		g_MachineOffset1.fY = -fToolOffset1Y;
		g_MachineOffset1.fYExtension = fToolOffset1YExtension;
		g_MachineOffset2.fX = -fToolOffset2X;
		g_MachineOffset2.fY = -fToolOffset2Y;
		g_MachineOffset2.fYExtension = fToolOffset2YExtension;
	}
	else if (g_ConfigData.cToolFlip == Y_NEG)
	{
		g_cMachineOffsetCompensationAdjustment = 1;
		g_MachineOffset1.fX = fToolOffset1X;
		g_MachineOffset1.fY = fToolOffset1Y;
		g_MachineOffset1.fYExtension = fToolOffset1YExtension;
		g_MachineOffset2.fX = fToolOffset2X;
		g_MachineOffset2.fY = fToolOffset2Y;
		g_MachineOffset2.fYExtension = fToolOffset2YExtension;
	}
	else
	{
		//ToolFlip is UNKNOWN : Must be set
		logf("ERROR:stou\r\n"); //SetToolOffset used, but flip is UNKNOWN\r\n");
		//MAKE THIS KILL POSNMODE FOR SAFETY
		g_cModeState = MODE_IDLE;
	}
#ifdef OUTPUT_TOOL_OFFSET
	logf("STARTING Tool Offset : MO1=( %.3f , %.3f ) MO2=( %.3f , %.3f )\r\n",
			g_MachineOffset1.fX, g_MachineOffset1.fY,
			g_MachineOffset2.fX, g_MachineOffset2.fY
	);
#endif
}

void ClearToolOffset()
{
	td_MachineOffset * p_MachineOffset;
	if (g_MachineOffset.fX != 0 || g_MachineOffset.fY != 0)
	{
		g_MachineOffset.fX = 0;
		g_MachineOffset.fY = 0;
#ifdef OUTPUT_TOOL_OFFSET
		logf("STOP Tool Offset\r\n");
#endif
		ResetNearestPosition();
	}
	if (g_cMachineOffsetCompensationAdjustment == 1)
	{
		g_cMachineOffsetCompensationAdjustment = 0; //turn off special machine offset logic.
#ifdef OUTPUT_TOOL_OFFSET
				logf("STOP Tool Offset\r\n");
#endif
		ResetNearestPosition();
	}
#ifdef OUTPUT_TOOL_OFFSET
	logf("STOP Tool Offset\r\n");
#endif
	p_MachineOffset = (td_MachineOffset *) g_STPtxMsg.p_cObjectValue;
	p_MachineOffset->fX = g_MachineOffset.fX;
	p_MachineOffset->fY = g_MachineOffset.fY;
	SmartToolMsg(0, STP_ALERT, MINIFT_OID_MACHINE_OFFSET, sizeof(td_MachineOffset), (char *) p_MachineOffset);
}

void InitPosnMode(void)
{
	//send down position mode parameters
	//Currently all set just-in-time, however use this to set them as defaults
	SetMoveSpeeds(0, 0, 1.0, 1.0);
	return;
}

void SetMoveSpeeds(byte bMakeLinear, byte bFinalMove, float fdx, float fdy)
{
	float fsx, fsy, facx, facy, fdcx, fdcy;
	float fr, fc;

	if (bFinalMove == 0)
	{
		if (fdx < g_ConfigData.LongDistance.fX
				&& fdy < g_ConfigData.LongDistance.fY)
		{
			fsx = g_ConfigData.PosnSpeed.fX;
			fsy = g_ConfigData.PosnSpeed.fY;
		}
		else
		{
			fsx = g_ConfigData.LongSpeed.fX;
			fsy = g_ConfigData.LongSpeed.fY;
		}
	}
	else
	{
		fsx = g_ConfigData.PosnFinalSpeed.fX;
		fsy = g_ConfigData.PosnFinalSpeed.fY;
	}

	facx = g_ConfigData.PosnAcc.fX;
	facy = g_ConfigData.PosnAcc.fY;
	fdcx = g_ConfigData.PosnDec.fX;
	fdcy = g_ConfigData.PosnDec.fY;

	if (bMakeLinear != 0)
	{
		//One change is so small, don't bother doing extra calculation... it can move as fast as it wants in that line.
		//Avoid 0 ratio, and 0 speeds which could cause issues.
		//due to math problems, we had this set to 0.0002, but due to a different problem,
		//it's now set to 0.1
		fr = 0.1;
		if (fdx < fdy)
		{
			if (fdx < fr)
			{
				return; //skip_make_linear;
			}

			//x is shorter and will need to be slower to arrive at the same time.
			fr = fdx / fdy;

			fc = fr * fsy; //set speed to this proportion based on y
			if (fsx >= fc)
			{
				fsx = fc;
			}
			else
			{
				//x is set to move even slower than this... base y speed off x at desired ratio
				fsy = fsx / fr;
			}
			//only scale ac dc if moving fast enough in for it to matter
			if (fr >= 0.01)
			{
				fc = fr * facy; //set ac. to this proportion based on y
				if (facx >= fc)
				{
					facx = fc;
				}
				else
				{
					//x is set to ac. even slower than this... base y ac. off x at desired ratio
					facy = facx / fr;
				}
				fc = fr * fdcy; //set dc. to this proportion based on y
				if (fdcx >= fc)
				{
					fdcx = fc;
				}
				else
				{
					//x is set to dc. even slower than this... base y dc. off x at desired ratio
					fdcy = fdcx / fr;
				}
			}
		}
		else
		{
			if (fdy < fr)
			{
				return; //skip_make_linear;
			}

			//y is shorter or the same and will need to be slower or the same to arrive at the same time.
			fr = fdy / fdx;

			fc = fr * fsx; //set speed to this proportion based on x
			if (fsy >= fc)
			{
				fsy = fc;
			}
			else
			{
				//y is set to move even slower than this... base x speed off y at desired ratio
				fsx = fsy / fr;
			}
			//only scale ac dc if moving fast enough in for it to matter
			if (fr >= 0.01)
			{
				fc = fr * facx; //set ac. to this proportion based on x
				if (facy >= fc)
				{
					facy = fc;
				}
				else
				{
					//y is set to ac. even slower than this... base x ac. off y at desired ratio
					facx = facy / fr;
				}
				fc = fr * fdcx; //set dc. to this proportion based on x
				if (fdcy >= fc)
				{
					fdcy = fc;
				}
				else
				{
					//y is set to dc. even slower than this... base x dc. off y at desired ratio
					fdcx = fdcy / fr;
				}
			}
		}
		return;
	}

	MCSetMoveSpeedParams(fsx, fsy, facx, facy, fdcx, fdcy);
	return;
}

void DoFloat(byte cAction)
{
#ifdef OUTPUT_FLOAT_STATES
	logf("DoFloat %d\r\n",(int)cAction);
#endif
	if (g_cGravCompStatus != GRAVCOMP_PASS && cAction != FLOAT_UNFLOAT
			&& cAction != FLOAT_UNFLOAT_STOP)
	{
		SmartToolMsgMiniFtMessageCode(0, COMMON_OID_NULLOID, MINIFTMC_FLOAT_NEEDS_GRAVCOMP);
		return;
	}
	if (g_cMoveDone != MOVEDONE_TRUE)
	{ // disallow float unless carriage is done with previous move
		SmartToolMsgMiniFtMessageCode(0, COMMON_OID_NULLOID, MINIFTMC_WAIT_FOR_CARRIAGE_STOP);
		return;
	}
	if (g_cModeState == MODE_ESTOP)
	{
// cannot float in estop
		SmartToolMsgMiniFtMessageCode(0, COMMON_OID_NULLOID, MINIFTMC_FLOAT_PREVENTED_BY_ESTOP);
		return;
	}
	if (cAction != FLOAT_UNFLOAT && cAction != FLOAT_UNFLOAT_STOP)
	{
		if (g_cObstructionCode != 0)
		{
			// can't float : too close to pos or neg limit, or obstruction
			AlertObstructionCode(0);
			return;
		}
	}
	g_PosnMode.cOnCurPosn = 0;	//floating must clear the on position flag
	if (g_cModeState == MODE_POSN)
	{
		LEDOff()
		;
	}

	if (cAction == FLOAT_TOGGLE)
	{
		if (g_cFloatStatus == FLOATSTAT_FLOAT)
		{
			cAction = FLOAT_UNFLOAT;
		}
		else
		{
			cAction = FLOAT_FLOAT;
		}
	}

	g_cFloatExitModePosnState = 0; //just clear the flag... only time it gets set is when unfloating normal way
	if (cAction == FLOAT_UNFLOAT)
	{
		g_cFloatExitModePosnState = POSNMODE_MOVE_NEAREST; //set the flag
	}

	if (cAction == FLOAT_FLOAT)
	{
//unclamp now, but don't need to wait for it.
#ifdef CLAMP_SYSTEM
		g_cClampGoal = CLAMP_UNCLAMP;
#endif
		RunFloat();
#ifdef OUTPUT_FLOAT_STATES
		logf("RunFloat()\r\n");
#endif
		g_cFloatGoal = FLOATSTAT_FLOAT;
		BeepFloat()
		;
	}
	else if (cAction == FLOAT_UNFLOAT || cAction == FLOAT_UNFLOAT_STOP)
	{
		RunUnfloat();
#ifdef OUTPUT_FLOAT_STATES
		logf("RunUnfloat()\r\n");
#endif
		g_cFloatGoal = FLOATSTAT_NOFLOAT;
#ifdef BEEPSYSTEM
		if (g_cBeepMode == BEEPFLOAT)
		{
			BeepOff();
		}
#endif
	}
}

//EOF

