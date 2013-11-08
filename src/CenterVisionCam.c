///////////////////////////////////////////////////////////////////////////////
//
//	MiniFt - CenterVisionCam.c
//
//	This module is responsible for managing the camera subsystem used to locate K-holes and edges.
//
//	Revision history:
//		2013-10-24	TC		Original coding for port of MiniFt executive functions to Angstrom Linux on a Beagle Bone platform.
//
///////////////////////////////////////////////////////////////////////////////

//	FIXME: These should be defined in a global application H file.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <math.h>
//	#include <termios.h>	//	Needed for devices that have a serial interface.
#include <sys/socket.h>
#include <sys/ioctl.h>

//#include <linux/i2c.h>
//#include <linux/i2c-dev.h>

#include <arpa/inet.h>     // To define `struct in_addr'.


//	Application specific include files.
//	Common resources for the SmartTool family of products.
#include "SmartToolUtil.h"
#include "SmartTool.h"
#include "CommonSmartTool.h"

//	MiniFt system resources.
#include "MiniFTDefs.h"
#include "MiniFTSmartTool.h"
//#include "NacClamp.h"	//	FIXME: Isolate clamp API in a separate H file?
#include "MiniFTWXY.h"

//	Ethernet protocol support.
#include "SocketConsole.h"
#include "STP.h"

//	Public resources for this subsystem.
#include "CenterVisionCam.h"

#ifdef CENTERVISION_CAM

//vars are needed....

//	FIXME PORTFATAL

///////////////////////////////////////////////////////////////////////////////
//	Private symbols and definitions.
///////////////////////////////////////////////////////////////////////////////

//	Enable the following for stand-alone testing of the vision subsystem.
//#define VISION_STANDALONETEST	1

#ifndef USE_OUTPUT
#define USE_OUTPUT
#endif 

//	Internal System States
#define CENTERVISION_INIT 0
#define CENTERVISION_CAM_SAMPLE 1
#define CENTERVISION_CAM_MOVE 2
#define CENTERVISION_CAM_MOVE_WAIT 3
#define CENTERVISION_CLOSURE 4
#define CENTERVISION_RETURN 5
#define CENTERVISION_RETURN_WAIT 6
#define CENTERVISION_STOP 7

//	FIXME: port this to a generic TCP support header file.
#define sock_established(x) ((x==0)?0:1)
#define sock_write(x,y,z) write(x,y,z)

///////////////////////////////////////////////////////////////////////////////
//	Public Globals for this subsystem
///////////////////////////////////////////////////////////////////////////////

char g_cCenterVisionResult; //	The current state and result state.
int g_iCenterVisionResultMessage; //	A message code that explains a failure state.
char g_cCenterVisionCamData; //	A flag used to tell when samples are received

//	Globals that configure behavior of the camera subsystem.
char g_cCenterVisionInspectType; //	Select what type of inspection to use
char g_cCenterVisionRequiredResults;
float g_fCenterVisionExpectedDiameter; //	In inches. Or pass 0 if unknown
float g_fCenterVisionMoveSpeed; //	Specify carriage (x,y) speed for locating edges or K-holes.

//	Physical (x,y) offset of the camera center from the center of the drill head.
float g_fCenterVisionOffsetX; //	Input Vision Offset is removed at the end for communicating object positions
float g_fCenterVisionOffsetY;

///////////////////////////////////////////////////////////////////////////////
//	Public Functions
///////////////////////////////////////////////////////////////////////////////

void
PreviewCenterVision();
void
ProgressCenterVision();
void
PositionInspection();
void
CancelCenterVision();

void
ServiceCam(void);
void
EchoCVCamData();

void
CVCamSample();
void
CVCamClose();
void
SendCVCamRaw(char *buffer, int len);
void
SendCVCamFromConsole(char *buffer, int len);

void
RecalculatePostionPixels();

//	Enable the following for testing without true probing.
//#define PROVIDE_IMMEDIATE_FAKE_SUCCESS_ON_CURRENT_POSITION

//	Enable the following to test with probe by connection to cam sim
//#define USE_CAM_SIMULATOR

//	Output Options
#define CV_DETAIL_OUT
//#define CV_VERBOSE_DETAIL_OUT
#define OUTPUT_CV_MOVE_INFO

///////////////////////////////////////////////////////////////////////////////
//	Private Local functions
///////////////////////////////////////////////////////////////////////////////

//void CenterVisionMonitorMotionInit( void );
//void CenterVisionMonitorMotion( void );
void
InspectionResultsClear(void);
//void RecordCenterVisionSample( void );
//void ProcessCenterVisionSamples( void );
//void ClearCVData( void );
//void ShowCVData( void );
//void CenterVisionExamineRange( void );
//void CenterVisionNarrowRange( void );
//void CenterVisionAnalysis( void );
//void CenterVisionCombineAnalysis( void );
void
SendInspectResults(void);
void
SendImageInformation(void);
//void SendDataGroup( char ci );
void
ParseCamMsg(char *s);

///////////////////////////////////////////////////////////////////////////////
//	Global variables from other subsystems that are accessed in this module.
///////////////////////////////////////////////////////////////////////////////

//	while this is not proper header use it can work.
//	FIXME PORTHIGH

extern byte g_cFloatGoal;
extern byte g_cPositionInspection;
extern byte g_cProbeCommand;

///////////////////////////////////////////////////////////////////////////////
//	Private variables
///////////////////////////////////////////////////////////////////////////////

uint32 g_ulCenterVisionStartTime;
uint32 g_ulCenterVisionCamDataTime;
uint32 g_uiCenterVisionCamDataCount;

char g_cCenterVisionLoopCount;
uint32 g_uiCenterVisionCamDataCount;
float g_fCenterVisionCurX;
float g_fCenterVisionCurY;
float g_fCenterVisionStartX;
float g_fCenterVisionStartY;
long g_lCenterVisionCurX;
long g_lCenterVisionCurY;

float g_fCenterVisionX;
float g_fCenterVisionY;
float g_fCenterVisionDiameter;

char g_cVisionScanState;
float g_fSeekSpeed;

///////////////////////////////////////////////////////////////////////////////
//	Private Variables that identify the current operating state.
///////////////////////////////////////////////////////////////////////////////
char g_cCenterVisionMethodMode;
char g_cgdatamode;
char g_cInfoMask;
char g_cAMode;
char g_cCMode;
char g_cAux1;
float g_fMoveRequired;
float g_fMaxOverExpDiameter;
float g_fMaxUnderExpDiameter;

//	Support for the Ethernet connection to the camera.
int g_iCVCamSock_fd; //	Ethernet socket to camera.
struct sockaddr_in CVSockAddr;
//struct sockaddr CVSockAddr;

///////////////////////////////////////////////////////////////////////////////
//	Private Timer Variables
///////////////////////////////////////////////////////////////////////////////

uint32 g_ulPosnRequestTime; //	Time tag when position was requested.
uint32 g_ulCamRequestTime; //
uint32 g_ulCamFirstRequestTime; //

char g_cEchoCVCamData;

//	Camera parse states.
#define CAMPARSE_NOTHING 0
#define CAMPARSE_SAMPLE 1

char g_cCamParseCode;
char g_cCamParseVariable;
char g_cCamHoleParseErrs;
char g_cCamEdgeParseErrs;

//	Cam Sample data
char g_cCamSampleStatus;
float g_fCamHoleX;
float g_fCamHoleY;

float g_fCamX;
float g_fCamY;

//	CameraVision Image Object
td_oid_vision_image g_CVCamVisionImage;

uint32 g_ulProbeClampGoalTime;
char g_cProbeClampGoal;
char g_cProbeClampState;
uint32 g_ulProbeDwellTime;

//	fixme minor  relocate
char g_cEdgeStatus1;
char g_cEdgeStatus2;

float g_fPixelsPerInchX;
float g_fPixelsPerInchY;

#ifdef CLAMP_SYSTEM
#define CAM_CLAMP_SYSTEM
#endif 

void
PreviewCenterVision()
{
    //	FIXME000000000000000000 no preview at this time
    //	could return status and have a way to know if it sees a hole
#if 0
    //Now get latest data
    if( g_cCenterVisionCamData == 0 )
    {
        //Request Sensor Data, but throttle
        if( MS_TIMER - g_ulCamRequestTime > 1000 )
        {
            g_cEchoCVCamData = 1;
            CVCamSample();
            g_ulCamRequestTime = MS_TIMER;
        }return;
    }
    if( g_cCenterVisionCamData == 1 )
    {
        //previously requested
        if( MS_TIMER - g_ulCamRequestTime > 3000 )
        {
            //3 seconds have passed so allow a second attempt at the data query.
            g_cCenterVisionCamData = 0;
        }
        return;
    }
    //Must have data now.
    //just put out vision requests...
    //Pendant alone can deal with the answers as they travel back through pendant,
    //or rx code in Rabbit can echo the answers back to the pendant if using direct connection.
    g_cCenterVisionCamData = 0;//allow next return to get new sensor data
#endif 
}

//	Initialize inspection results for a new inspection pass.
void
InspectionResultsClear(void)
{
    g_VisionInspectResults.cMethod = g_cCenterVisionInspectType;
    g_VisionInspectResults.fDiameterExpected = g_fCenterVisionExpectedDiameter;
    g_VisionInspectResults.fDiameter = 0; //clear
    g_VisionInspectResults.fAlgDiameter = 0; //clear

    g_VisionInspectResults.cEdgeStatus = 0;
    g_VisionInspectResults.cEdgeNote = 0;
    g_VisionInspectResults.fEdgeX1 = 0;
    g_VisionInspectResults.fEdgeY1 = 0;
    g_VisionInspectResults.fEdgeX2 = 0;
    g_VisionInspectResults.fEdgeY2 = 0;
    g_VisionInspectResults.fCEdgeX1 = 0;
    g_VisionInspectResults.fCEdgeY1 = 0;
    g_VisionInspectResults.fCEdgeX2 = 0;
    g_VisionInspectResults.fCEdgeY2 = 0;
    g_VisionInspectResults.fXPositionPixels = 0;
    g_VisionInspectResults.fYPositionPixels = 0;
    g_VisionInspectResults.fXPositionExpectedPixels = 0;
    g_VisionInspectResults.fYPositionExpectedPixels = 0;
    g_VisionInspectResults.fEdgeX1Pixels = 0;
    g_VisionInspectResults.fEdgeY1Pixels = 0;
    g_VisionInspectResults.fEdgeX2Pixels = 0;
    g_VisionInspectResults.fEdgeY2Pixels = 0;
    g_VisionInspectResults.fCEdgeX1Pixels = 0;
    g_VisionInspectResults.fCEdgeY1Pixels = 0;
    g_VisionInspectResults.fCEdgeX2Pixels = 0;
    g_VisionInspectResults.fCEdgeY2Pixels = 0;

}

// CENTERVISION_ALGORITHM_CAM_A0
void
ProgressCenterVision()
{
    char cStatus;
    char cClampGoal;
    //float f;
    //float fXCenter;
    uint32 ulElapsedMs;
    uint32 ultc;
    //long ltd;
    td_CamAlgParam *p_CamAlgParam;
    float fdx, fdy;

    //	If carriage axes are floating, stop float mode before starting the camera process.
    if( g_cFloatStatus == FLOATSTAT_FLOAT )
    {
        if( g_cFloatGoal != FLOATSTAT_NOFLOAT )
        {
            logf("Ask for Unfloat stop.\n");
            DoFloat(FLOAT_UNFLOAT_STOP);
        }
        //	Wait for unfloat to be done
        return;
    }

    if( g_cCenterVisionResult == CENTERVISION_OFF )
    {
        logf("CV-PRE\r\n");
        //	Record Start Time
        g_cCenterVisionLoopCount = 0;
        g_ulCenterVisionStartTime = MS_TIMER;
        //	Clear ResultMessage
        g_iCenterVisionResultMessage = 0;
        //	tell it carriage moving is done...
        //	This is not to clear the screen but to enable new motion to start
        if( g_cMoveDone != MOVEDONE_TRUE )
        {
            //	This should never happen, but it's not that bad
            logf("Warning: Had to set MOVEDONE_TRUE\r\n");
            MCStopPosition();
            g_cMoveDone = MOVEDONE_TRUE;
        }

        ////FIXME9999999999999 how to set to feature size???? how to test???
        ////g_fCenterVisionExpectedDiameter = g_ConfigData.LaserSensorAlgParam.fprobe_diameter; //use size from OID for first scan
        InspectionResultsClear();

        //Load More Parameters
        p_CamAlgParam = &g_ConfigData.CamAlgParam;

        //FIXME9999999999999999
        //Fields waiting to be used......
        //used	float fmove_speed;
        //	char cInfoMask;
        //	char cAMode;
        //	char cCMode;
        //	char cAux1;
        //	float fmove_required;
        //used	float fmax_over_exp_diameter;
        //used	float fmax_under_exp_diameter;
        //	float fmax_csnk_diff;
        //	float fmax_over_csnk;
        //	float fmax_under_csnk;

        g_fCenterVisionMoveSpeed = p_CamAlgParam->fmove_speed;
        g_fSeekSpeed = g_fCenterVisionMoveSpeed;

        /****** FIXME9999999999999999
         g_cCenterVisionMethodMode = p_CamAlgParam->cmode;//FIXME0 implement better
         */

        //FIXME9999999999999999  I'm reading these now, but I'm not using them all yet
        g_cInfoMask = p_CamAlgParam->cInfoMask;
        g_cAMode = p_CamAlgParam->cAMode;
        g_cCMode = p_CamAlgParam->cCMode;
        g_cAux1 = p_CamAlgParam->cAux1;
        g_fMoveRequired = p_CamAlgParam->fmove_required;

        g_fMaxOverExpDiameter = p_CamAlgParam->fmax_over_exp_diameter;
        g_fMaxUnderExpDiameter = p_CamAlgParam->fmax_under_exp_diameter;
        //Fields waiting to be used......
        //	float fmax_csnk_diff;
        //	float fmax_over_csnk;
        //	float fmax_under_csnk;

        //Clear Flag to allow it to ask again
        g_cCenterVisionCamData = 0;
        g_uiCenterVisionCamDataCount = 0;

        //	Clear Samples
        //FIXME0000		g_cCenterVisionSampleCount = 0;

        //	Now Save additional input parameters that echo to report context.
        //g_VisionInspectResults.cContext set by input
        g_VisionInspectResults.cStatus = 0; //	Set Below to Success or Fail
        g_VisionInspectResults.cMethod = PROBE_CAM;
        g_VisionInspectResults.cInfo = VisionInfoPosition
                | VisionInfoPositionExpected | VisionInfoDiameter;

        g_VisionInspectResults.fDiameterExpected =
                g_fCenterVisionExpectedDiameter;
        g_VisionInspectResults.fDiameter = 0; //clear
        g_VisionInspectResults.fAlgDiameter = 0; //clear

        g_VisionInspectResults.uiImageSequence = 0; //clear

        //	Set move parameters for precision movement.
        MCSetMoveParams(0.0005, 1);

        //	Turn Down Normal frequency of GetPosn polling.
        g_uiPositionUpdateThrottle = 120000L; // 2 minutes
        g_uiPositionSendThrottle = 2000; //  2 seconds

        //	And go to the search start state
        g_cVisionScanState = 0;
        //	To avoid having a problem with the initial pass by clamp checking logic below, set initial goal to whatever the state is.
        g_cProbeClampGoal = g_cClampState;
        g_cProbeClampState = g_cClampState;
        g_ulProbeClampGoalTime = MS_TIMER;
        g_ulProbeDwellTime = 0;
        //	Now Move to first goal state
        g_cVisionScanState = CENTERVISION_CAM_SAMPLE;
    }

    //	Now starting the vision process.
    g_cCenterVisionResult = CENTERVISION_WORKING;

    //	If Moving ... Wait ...
    if( g_cMoveDone == MOVEDONE_FALSE )
    {
        //Moving, so wait until done for samples and next move plan
        return;
    }
    if( g_cMoveDone == MOVEDONE_ERROR )
    {
        //	Cam scan can't continue. Motion error.
        logf("CV-Move Error\r\n");
        g_cMoveDone = MOVEDONE_TRUE; //Clear the Error Here.
        g_iCenterVisionResultMessage = MINIFTMC_CV_CAM_MOVE_ERROR;
        g_cCenterVisionResult = CENTERVISION_FAILURE;
        goto final_report;
    }

    //	If we reach here, X/Y carriage motion is complete.
    //	Before camera can probe, verify clamping is completed too.
    g_cProbeClampState = g_cClampState;
    if( g_cProbeClampState != g_cProbeClampGoal )
    {
        //	Waiting for Clamp to finish clamping.
        ulElapsedMs = MS_TIMER - g_ulProbeClampGoalTime;
        if( g_cProbeClampState != CLAMP_TRANSITION
                && g_cProbeClampState != CLAMP_UNKNOWN )
        {
            if( ulElapsedMs > 1000 )
            {
                //	One Second in a non-transitional state that is not our target is not good!
                //	This is a clamp failure....
                g_cProbeClampState = CLAMP_ERROR;
            }
        }
        else
        {
            if( ulElapsedMs > 15000 )
            {
                //	This is a clamp failure....
                //	15 Seconds of clamp time is enough.
                g_cProbeClampState = CLAMP_ERROR;
            }
        }
    }

    //	The CenterVision camera probe state machine starts here:
    switch (g_cVisionScanState)
    {
    case CENTERVISION_CAM_SAMPLE:
        //	Beginning the first sample or move is Complete.
        //	Get the latest camera data.
        if( g_cCenterVisionCamData == 0 )
        {
#ifdef CAM_CLAMP_SYSTEM
            //	Check if a clamp is needed for camera probing.
            if( g_cCMode == 1 )
            {
                //	Must be clamped. Set the clamp goal.  The pressure argument is always set, though only some systems will use this pressure.
                g_uiClampPressure = (int) g_ConfigData.uiPressure;
                cClampGoal = CLAMP_CLAMP;
            }
            else
            {
                //	Must be unclamped.
                cClampGoal = CLAMP_UNCLAMP;
            }
            //	Set the clamp goal, which is read and acted upon by the clamp subsystem.
            g_cProbeClampGoal = cClampGoal;

            //	Has the clamp adopted the clamp goal needed to do a CV camera probe?
            if( g_cClampGoal != g_cProbeClampGoal )
            {
                //	Not yet. Return to try again later.
                g_cClampGoal = g_cProbeClampGoal;
                g_ulProbeClampGoalTime = MS_TIMER;
                logf("ProbeCG=%d\r\n", g_cProbeClampGoal);
                break;
            }

            //	Has the clamp state reached the clamp goal needed to probe?
            if( g_cProbeClampState != g_cProbeClampGoal )
            {
                if( g_cProbeClampState == CLAMP_ERROR )
                {
                    logf("ClampError\r\n");
                    g_iCenterVisionResultMessage = MINIFTMC_CV_CAM_CLAMP_ERROR;
                    //#warning "do better fix"
                    if( g_cCenterVisionLoopCount == 0 )
                    {
                        //If Loop Zero, never got a start position yet
                        //FIXME temp  do a better job at this
                        g_fCenterVisionCurX = g_PosnMode.fLastKnownPosnX;
                        g_fCenterVisionCurY = g_PosnMode.fLastKnownPosnY;
                        logf("Cur Posn %f , %f\r\n", g_fCenterVisionCurX,
                                g_fCenterVisionCurY);
                        g_fCenterVisionStartX = g_fCenterVisionCurX; //retain the start position
                        g_fCenterVisionStartY = g_fCenterVisionCurY;
                    }
                    g_cVisionScanState = CENTERVISION_RETURN;
                    break;
                }
                //	Wait some more.
                break;
            }

            //FIXME HIGH  Should do these clamp and probe issues. I commented out two warnings related to this
            //#warning "check second check.
            // to see if really needed... if all locations immediately set state to transition it would not be needed.... but I don't do that.... is there a potention
            //I'm concerned about that pattern
#endif 

            //	Now Dwell in the clamped (or unclamped) position before taking a camera image.
            if( g_ulProbeDwellTime == 0 )
            {
                g_ulProbeDwellTime = MS_TIMER;
            }
            ulElapsedMs = MS_TIMER - g_ulProbeDwellTime;
            if( ulElapsedMs < 500 )
            //	FIXME OID could add oid for dwell or use additional CAM parameter???
            {
                //	Do not acquire image yet.
                break;
            }

            //	Request camera Sensor Data
            //g_cDir = 7; //AlgA   FIXME00000000000 update data reporting system
            CVCamSample();
            g_ulCamRequestTime = MS_TIMER;
            //	Also Get current camera X and Y position.
            g_ulPosnRequestTime = MS_TIMER;
            MCGetPositionCounts(); //	Slight potential for race-case FIXME00000000 posn stable exit position reporting....
#warning "FIXME Check on GetPositionCounts an if anything for probe could have stale posn..."
            return;
        }

        //	If camera data was previously requested, ...
        if( g_cCenterVisionCamData == 1 )
        {
            //	Check for timeout.
            ultc = MS_TIMER - g_ulCamRequestTime;
            if( ultc > 10000 )
            {
                //a 10 seconds have passed so allow a second attempt at the data query.
                logf("CVC\r\n");
                g_cCenterVisionCamData = 0;
            }
            return;
        }

        //	If Camera Sample Data has been received, ...
        if( g_cCenterVisionCamData == 2 )
        {
            if( g_VisionInspectResults.uiImageSequence > 0 )
            {
                //	Transmit camera image data.
                SendImageInformation();
            }
            g_cCenterVisionCamData = 3; //	indicate inspection results were sent
        }

        //	Check for position here.
        //	See if position data for both X and Y has come in.
        if( g_PosnMode.cFreshPosn != X_AND_Y )
        {
            //	We did not get a fresh position report yet. Check for timeout.
            ultc = MS_TIMER - g_ulPosnRequestTime;
            if( ultc > 1000 )
            {
                logf("RPA\r\n"); //	Request Posn Again
                g_ulPosnRequestTime = MS_TIMER;
                MCGetPositionCounts(); //	Slight potential for race-case FIXME00000000 posn stable exit position reporting....
            }
            return;
        }

        //	Record position.
        g_fCenterVisionCurX = g_PosnMode.fLastKnownPosnX;
        g_fCenterVisionCurY = g_PosnMode.fLastKnownPosnY;

        cStatus = g_cCamSampleStatus;
        logf("cs=%d\r\n", cStatus);

        //	Now ready to use sensor data and position.
        logf("\r\nLoop %d : %lu\r\n", g_cCenterVisionLoopCount,
                MS_TIMER - g_ulCenterVisionStartTime);

        //	Record camera position.
        g_fCenterVisionCurX = g_PosnMode.fLastKnownPosnX;
        g_fCenterVisionCurY = g_PosnMode.fLastKnownPosnY;
        logf("Cur Posn %f , %f\r\n", g_fCenterVisionCurX, g_fCenterVisionCurY);

        if( g_cCenterVisionLoopCount == 0 )
        {
            //	First picture before moving.
            g_fCenterVisionStartX = g_fCenterVisionCurX; //	Retain the start position
            g_fCenterVisionStartY = g_fCenterVisionCurY;
            g_fCenterVisionX = g_fCenterVisionCurX - g_fCenterVisionOffsetX; //	Give it a default target, but remove offset
            g_fCenterVisionY = g_fCenterVisionCurY - g_fCenterVisionOffsetY;
            logf("StartPos\r\n");
        }

        //	Allow next query to get new sensor data (if we do return)
        g_cCenterVisionCamData = 0;

        if( (cStatus & LASERSENSOR_ONLINE) == 0 )
        {
            //	Sensor not event getting connected or responding
            //	FIXMENOWNOW High message to use with information
            logf("CV no con.\r\n");
            g_iCenterVisionResultMessage = MINIFTMC_CV_CAM_CONNECTION;
            g_cVisionScanState = CENTERVISION_RETURN;
            return;
        }
        //	Calculate Translation that aligns Camera coordinates to the real world
        fdx = g_fCenterVisionCurX - g_fCenterVisionOffsetX;
        fdy = g_fCenterVisionCurY - g_fCenterVisionOffsetY;
        if( (cStatus & LASERSENSOR_SEE_HOLE) != 0 )
        {
            logf("CamXY %f,%f\r\n", g_fCamHoleX, g_fCamHoleY);

            //	CamXY is the position from the center of the camera to the hole.
            g_fCenterVisionX = fdx + g_fCamHoleX;
            g_fCenterVisionY = fdy + g_fCamHoleY;
#ifdef OUTPUT_CV_MOVE_INFO
            logf("Center   %.4f,%.4f\r\n", g_fCenterVisionX, g_fCenterVisionY);
#endif 
        }
        if( (cStatus & LASERSENSOR_SEE_EDGE) != 0 )
        {
            g_VisionInspectResults.fEdgeX1 += fdx;
            g_VisionInspectResults.fEdgeY1 += fdy;
            g_VisionInspectResults.fEdgeX2 += fdx;
            g_VisionInspectResults.fEdgeY2 += fdy;
            g_VisionInspectResults.fCEdgeX1 += fdx;
            g_VisionInspectResults.fCEdgeY1 += fdy;
            g_VisionInspectResults.fCEdgeX2 += fdx;
            g_VisionInspectResults.fCEdgeY2 += fdy;
        }

        if( (g_cCenterVisionRequiredResults & CENTERVISION_CENTER) != 0 )
        {
            if( (cStatus & LASERSENSOR_SEE_HOLE) == 0 )
            {
                //	Can't See the Hole
                //	FIXMENOWNOW High message to use with information
                logf("Can't See Hole status=%d.\r\n", cStatus);
                g_iCenterVisionResultMessage = MINIFTMC_CV_CAM_NO_HOLE;
                g_cVisionScanState = CENTERVISION_RETURN;
                return;
            }
        }
        if( (g_cCenterVisionRequiredResults & CENTERVISION_EDGE) != 0 )
        {
            if( (cStatus & LASERSENSOR_SEE_EDGE) == 0 )
            {
                //	Can't See the Edge
                //	FIXMENOWNOW High message to use with information
                logf("Can't See Hole status=%d.\r\n", cStatus);
                g_iCenterVisionResultMessage = MINIFTMC_CV_CAM_NO_EDGE;
                g_cVisionScanState = CENTERVISION_RETURN;
                return;
            }
        }
        //	This had whatever data was needed
        if( (g_cCenterVisionRequiredResults & CENTERVISION_CENTER) != 0 )
        {
            //	Check if a move is required.
            logf("g_fMoveRequired=%f\r\n", g_fMoveRequired);
            if( fabs(g_fCamHoleX) > g_fMoveRequired
                    || fabs(g_fCamHoleY) > g_fMoveRequired )
            {
                //	Need to be closer for success
                if( g_cProbeExtraOffsetGiven != 0 || g_cProbeExtraMachineOffsetGiven != 0 )
                {
                    logf("namc\r\n"); //not allowed move closer
                    g_iCenterVisionResultMessage = MINIFTMC_CV_CAM_MOVE_LIMIT;
                    g_cVisionScanState = CENTERVISION_RETURN;
                    return;
                }
                if( g_cCenterVisionLoopCount >= 2 )
                {
                    //past loop 0, past loop 1
                    //Should have been able to do this...  a failure....
                    //FIXME9999999999 need better error messages for CAM
                    logf("After 2 pictures, still too far.\r\n", cStatus);
                    g_iCenterVisionResultMessage = MINIFTMC_CV_CAM_MOVE_LIMIT;
                    g_cVisionScanState = CENTERVISION_RETURN;
                    return;
                }
                g_cVisionScanState = CENTERVISION_CAM_MOVE;
                return;
            }
            //	Close enough to use this value without new picture
        }

        //	Complete.
        g_cVisionScanState = CENTERVISION_CLOSURE;
        return;

    case CENTERVISION_CAM_MOVE:
#ifdef CAM_CLAMP_SYSTEM
        //	Must be unclamped
        if( g_cProbeClampState != CLAMP_UNCLAMP )
        {
            //	must unclamp
            if( g_cProbeClampGoal != CLAMP_UNCLAMP )
            {
                g_cClampGoal = CLAMP_UNCLAMP;
                g_cProbeClampGoal = CLAMP_UNCLAMP;
                g_ulProbeClampGoalTime = MS_TIMER;
                logf("ProbeCG=%d\r\n", g_cClampGoal);
            }
            if( g_cProbeClampState == CLAMP_ERROR )
            {
                logf("ce2\r\n");
                g_iCenterVisionResultMessage = MINIFTMC_CV_CAM_CLAMP_ERROR;
                g_cVisionScanState = CENTERVISION_RETURN;
            }
            //wait more
            break;
        }
#endif 

        //	Try move
        MCSetMoveSpeedParamsEven(g_fSeekSpeed, 10);

#ifdef OUTPUT_CV_MOVE_INFO
        logf("Move Cam to %.4f,%.4f\r\n", g_fCenterVisionX, g_fCenterVisionY);
#endif 
        MCRunPosition(g_fCenterVisionX + g_fCenterVisionOffsetX, g_fCenterVisionY + g_fCenterVisionOffsetY);
        g_cCenterVisionLoopCount++;
        g_cVisionScanState = CENTERVISION_CAM_MOVE_WAIT;
        break;

    case CENTERVISION_CAM_MOVE_WAIT:
        //	Nothing to do
        //	It will not come back to the statemachine until the move is complete
        //		(see move checking done outside of state machine)
        //	Now ready for next sample.
        //	Clear Dwell Time
        g_ulProbeDwellTime = 0;
        g_cVisionScanState = CENTERVISION_CAM_SAMPLE;
        break;

    case CENTERVISION_CLOSURE:
        //	Move must be complete now
        //	Accept this Position
        logf("Center   %.4f,%.4f\r\n", g_fCenterVisionX, g_fCenterVisionY);

        //	Save Position For Report
        g_VisionInspectResults.fXPosition = g_fCenterVisionX;
        g_VisionInspectResults.fYPosition = g_fCenterVisionY;
        //	Both Diameter values use the same result in this library
        g_VisionInspectResults.fDiameter = g_fCenterVisionDiameter;
        g_VisionInspectResults.fAlgDiameter = g_fCenterVisionDiameter;

        //	Diameter
        //	This is set by the diameter check section of CenterVisionCombineAnalysis

        //	Countersink : This would only work if using depth, rather than intensity

        //	Clear countersink details
        g_VisionInspectResults.fNEdgeHeight = 0;
        g_VisionInspectResults.fPEdgeHeight = 0;
        g_VisionInspectResults.fNCsnkEdgeHeight = 0;
        g_VisionInspectResults.fPCsnkEdgeHeight = 0;

        //	Clear countersink depth and slope
        g_VisionInspectResults.fCountersinkDepth = 0;
        g_VisionInspectResults.cCountersinkAccepted = 1;

        //	Set Image details

        //	Don't Set these here
        //g_CVCamVisionImage.uiSequence = 0;
        //g_CVCamVisionImage.uiFlags = 0;
        //g_CVCamVisionImage.uiWidth = 0;
        //g_CVCamVisionImage.uiHeight = 0;
        //g_CVCamVisionImage.fX = 0;		//From Cam 0,0 from last read
        //g_CVCamVisionImage.fY = 0;
        //g_CVCamVisionImage.fDiameter = 0; //FROM last read.....
        //g_CVCamVisionImage.fXPixels = 0;	//IN PIXELS FROM last read
        //g_CVCamVisionImage.fYPixels = 0;
        //g_CVCamVisionImage.fDiameterPixels = 0; //FROM last read.....
        //g_CVCamVisionImage.fPixelsPerInch = 0;
        //g_CVCamVisionImage.fX1 = 0;
        //g_CVCamVisionImage.fY1 = 0;
        //g_CVCamVisionImage.fX2 = 0;
        //g_CVCamVisionImage.fY2 = 0;

        //	Final Sanity Check on the resulting center
        //            if (g_fCenterVisionX < g_fMinX || g_fCenterVisionX > g_fMaxX ||
        //            	g_fCenterVisionY < g_fMinY || g_fCenterVisionY > g_fMaxY)
        //            {
        //            	//FIXME0000000000 make this a better error
        //            	g_iCenterVisionResultMessage = MINIFTMC_CV_FAIL_EDGE;
        //            }

        //			//	But if Analyis returned an error do not center
        //			if (g_iCenterVisionResultMessage!=0)
        //            {
        //            	g_cVisionScanState=CENTERVISION_RETURN;
        //            	break;
        //            }

        //And Return Image Info
        //

        logf("Center Vision Success\r\n");
        g_cCenterVisionResult = CENTERVISION_SUCCESS;
        goto final_report;
        break;
    case CENTERVISION_RETURN:
#ifdef CAM_CLAMP_SYSTEM
        //Must be unclamped
        if( g_cProbeClampState != CLAMP_UNCLAMP )
        {
            //must unclamp
            if( g_cProbeClampGoal != CLAMP_UNCLAMP )
            {
                g_cClampGoal = CLAMP_UNCLAMP;
                g_cProbeClampGoal = CLAMP_UNCLAMP;
                g_ulProbeClampGoalTime = MS_TIMER;
                logf("ProbeCG=%d\r\n", g_cClampGoal);
                //will need to come back
                break;
            }
            if( g_cProbeClampState == CLAMP_ERROR )
            {
                //complete anyway... don't wait for unclamp anylonger.
                logf("ce3\r\n");
            }
            else
            {
                //still waiting
                break;
            }
        }
#endif 

        //	Now setup Return
        logf("CV-RETURN\r\n");
        logf("Cur Posn %f , %f\r\n", g_fCenterVisionStartX,
                g_fCenterVisionStartY);
        MCSetMoveSpeedParamsEven(g_fSeekSpeed, 10);
        //	GO BACK TO THE EXACT START POSITION
        MCRunPosition(g_fCenterVisionStartX, g_fCenterVisionStartY);
        //	This code was to restart scan loop
        g_cVisionScanState = CENTERVISION_RETURN_WAIT;
        break;

    case CENTERVISION_RETURN_WAIT:
        //	Post message
        //#warning "skipping msg in favor of new"
        //            SmartToolMsgMiniFtMessageCode( OID_NULLOID, g_iCenterVisionResultMessage);
        //	Now Return with failure
        logf("Center Vision Failure\r\n");
        g_cCenterVisionResult = CENTERVISION_FAILURE;
        goto final_report;
        break;

    case CENTERVISION_STOP:
        //	Post message
        //#warning "skipping msg in favor of new"
        //            SmartToolMsgMiniFtMessageCode( OID_NULLOID, g_iCenterVisionResultMessage );
        MCStopPosition();
        //	Now Return with failure
        logf("Center Vision Failure\r\n");
        g_cCenterVisionResult = CENTERVISION_FAILURE;
        goto final_report;
        break;

    default:
        logf("BadState\r\n");
        g_cVisionScanState = CENTERVISION_RETURN;
        break;
    }
    return;

    //	Reached Success or Failure...
    final_report: if( g_cCenterVisionResult == CENTERVISION_FAILURE )
    {
        // Failure
        g_VisionInspectResults.cStatus = 0;
        g_VisionInspectResults.cResultMessage = g_iCenterVisionResultMessage; //return Result Message
    }
    else
    {
        //	Success
        g_VisionInspectResults.cStatus = 1;
    }
    //	Inspect Time
    g_VisionInspectResults.finspecttime = ((float) (MS_TIMER
            - g_ulCenterVisionStartTime)) / 1000.0;
    logf("T=%f s\r\n", g_VisionInspectResults.finspecttime);

    if( g_cCenterVisionResult != CENTERVISION_FAILURE
            && g_cProbeCommand == TKP_PROBE_EDGE_MVEC )
    {
        //	Do not yet send report... let main system add analysis for now...
#warning "VHIGH REORG SendInspectResults HACK"
        //	not even sure how to reorg right now...
    }
    else
    {
        //	Send final report
        SendInspectResults();
    }

    if( g_cMoveDone != MOVEDONE_TRUE )
    {
        logf( "FinalReportStop.\r\n" ); //	Final Report Stop
        MCStopPosition(); //	Ensure it will stop motion.
        g_cMoveDone = MOVEDONE_TRUE;
    }
    SmartToolMsg((td_STPsessions*) 0, STP_ALERT, MINIFT_OID_POSNMODE_MOVEDONE, 1, (char *)&g_cMoveDone);

    g_uiPositionUpdateThrottle = POSITION_UPDATE_THROTTLE_DEFAULT;
    g_uiPositionSendThrottle = POSITION_SEND_THROTTLE_DEFAULT;
    return;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
//	PositionInspection()
//	Second Public function in the library helps with repeated calling and final alerting of result,
//		when using center vision for inspection.
//////////////////////////////////////////////////////////////////////////////////////////////////

void
PositionInspection()
{
    //	Start where we want to inspect and have code CENTERVISION_OFF
    //	Let the following complete and then notify the pendant with the results.
    ProgressCenterVision();
    if( g_cCenterVisionResult == CENTERVISION_OFF )
    {
        //	Allow operation to continue to completion
        return;
    }
    if( g_cCenterVisionResult == CENTERVISION_WORKING )
    {
        //	Allow operation to continue to completion
        return;
    }
    g_cPositionInspection = 0; // Clear Inspection
    return;
}

void
CancelCenterVision()
{
    if( g_cCenterVisionResult == CENTERVISION_WORKING )
    {
        //	Just go to the stop state
        g_cVisionScanState = CENTERVISION_STOP;
    }
}

void
SendInspectResults()
{
    //	Prepare Output
    //	Prepare For Network Order
    td_VisionInspectResults *p_Results;
    p_Results = (td_VisionInspectResults*) g_STPtxMsg.p_cObjectValue;
    p_Results->cContext = g_VisionInspectResults.cContext;
    p_Results->cStatus = g_VisionInspectResults.cStatus;
    p_Results->cMethod = g_VisionInspectResults.cMethod;
    p_Results->cInfo = g_VisionInspectResults.cInfo;
    p_Results->lposn = htonl(g_VisionInspectResults.lposn);
    p_Results->fXPosition = g_VisionInspectResults.fXPosition;
    p_Results->fYPosition = g_VisionInspectResults.fYPosition;
    p_Results->fXPositionExpected = g_VisionInspectResults.fXPositionExpected;
    p_Results->fYPositionExpected = g_VisionInspectResults.fYPositionExpected;
    p_Results->fDiameterExpected = g_VisionInspectResults.fDiameterExpected;
    p_Results->fDiameter = g_VisionInspectResults.fDiameter;
    p_Results->fAlgDiameter = g_VisionInspectResults.fAlgDiameter;
    p_Results->cResultMessage = g_VisionInspectResults.cResultMessage;
    p_Results->fCountersinkExpected =
            g_VisionInspectResults.fCountersinkExpected;
    p_Results->fCountersinkDepth = g_VisionInspectResults.fCountersinkDepth;
    p_Results->cCountersinkAccepted =
            g_VisionInspectResults.cCountersinkAccepted;
    p_Results->fNEdgeHeight = g_VisionInspectResults.fNEdgeHeight;
    p_Results->fPEdgeHeight = g_VisionInspectResults.fPEdgeHeight;
    p_Results->fNCsnkEdgeHeight = g_VisionInspectResults.fNCsnkEdgeHeight;
    p_Results->fPCsnkEdgeHeight = g_VisionInspectResults.fPCsnkEdgeHeight;
    p_Results->uiImageSequence = htons(g_VisionInspectResults.uiImageSequence);
    p_Results->finspecttime = g_VisionInspectResults.finspecttime;
    p_Results->cEdgeStatus = g_VisionInspectResults.cEdgeStatus;
    p_Results->cEdgeNote = g_VisionInspectResults.cEdgeNote;
    p_Results->fEdgeX1 = g_VisionInspectResults.fEdgeX1;
    p_Results->fEdgeY1 = g_VisionInspectResults.fEdgeY1;
    p_Results->fEdgeX2 = g_VisionInspectResults.fEdgeX2;
    p_Results->fEdgeY2 = g_VisionInspectResults.fEdgeY2;
    p_Results->fCEdgeX1 = g_VisionInspectResults.fCEdgeX1;
    p_Results->fCEdgeY1 = g_VisionInspectResults.fCEdgeY1;
    p_Results->fCEdgeX2 = g_VisionInspectResults.fCEdgeX2;
    p_Results->fCEdgeY2 = g_VisionInspectResults.fCEdgeY2;
    p_Results->fXPositionPixels = g_VisionInspectResults.fXPositionPixels;
    p_Results->fYPositionPixels = g_VisionInspectResults.fYPositionPixels;
    p_Results->fXPositionExpectedPixels =
            g_VisionInspectResults.fXPositionExpectedPixels;
    p_Results->fYPositionExpectedPixels =
            g_VisionInspectResults.fYPositionExpectedPixels;
    p_Results->fEdgeX1Pixels = g_VisionInspectResults.fEdgeX1Pixels;
    p_Results->fEdgeY1Pixels = g_VisionInspectResults.fEdgeY1Pixels;
    p_Results->fEdgeX2Pixels = g_VisionInspectResults.fEdgeX2Pixels;
    p_Results->fEdgeY2Pixels = g_VisionInspectResults.fEdgeY2Pixels;
    p_Results->fCEdgeX1Pixels = g_VisionInspectResults.fCEdgeX1Pixels;
    p_Results->fCEdgeY1Pixels = g_VisionInspectResults.fCEdgeY1Pixels;
    p_Results->fCEdgeX2Pixels = g_VisionInspectResults.fCEdgeX2Pixels;
    p_Results->fCEdgeY2Pixels = g_VisionInspectResults.fCEdgeY2Pixels;

    //	OID_VISION_INSPECT_RESULTS ALERT SEND
    SmartToolMsg((td_STPsessions*) 0, STP_ALERT,
            MINIFT_OID_VISION_INSPECT_RESULTS, sizeof(td_VisionInspectResults),
            (void*) g_STPtxMsg.p_cObjectValue);
}

void
SendImageInformation()
{
    //	OID_VISION_IMAGE ALERT SEND
    td_oid_vision_image *p_oid_vision_image;
    p_oid_vision_image = (td_oid_vision_image*) g_STPtxMsg.p_cObjectValue;
    p_oid_vision_image->uiSequence = htons(g_CVCamVisionImage.uiSequence);
    p_oid_vision_image->uiFlags = htons(g_CVCamVisionImage.uiFlags);
    p_oid_vision_image->uiWidth = htons(g_CVCamVisionImage.uiWidth);
    p_oid_vision_image->uiHeight = htons(g_CVCamVisionImage.uiHeight);
    p_oid_vision_image->fX = g_CVCamVisionImage.fX;
    p_oid_vision_image->fY = g_CVCamVisionImage.fY;
    p_oid_vision_image->fDiameter = g_CVCamVisionImage.fDiameter;
    p_oid_vision_image->fXPixels = g_CVCamVisionImage.fXPixels;
    p_oid_vision_image->fYPixels = g_CVCamVisionImage.fYPixels;
    p_oid_vision_image->fDiameterPixels = g_CVCamVisionImage.fDiameterPixels;
    p_oid_vision_image->fPixelsPerInch = g_CVCamVisionImage.fPixelsPerInch;
    p_oid_vision_image->cEdgeStatus = g_CVCamVisionImage.cEdgeStatus;
    p_oid_vision_image->cEdgeNote = g_CVCamVisionImage.cEdgeNote;
    p_oid_vision_image->fX1 = g_CVCamVisionImage.fX1;
    p_oid_vision_image->fY1 = g_CVCamVisionImage.fY1;
    p_oid_vision_image->fX2 = g_CVCamVisionImage.fX2;
    p_oid_vision_image->fY2 = g_CVCamVisionImage.fY2;
    p_oid_vision_image->fCX1 = g_CVCamVisionImage.fCX1;
    p_oid_vision_image->fCY1 = g_CVCamVisionImage.fCY1;
    p_oid_vision_image->fCX2 = g_CVCamVisionImage.fCX2;
    p_oid_vision_image->fCY2 = g_CVCamVisionImage.fCY2;
    p_oid_vision_image->fX1Pixels = g_CVCamVisionImage.fX1Pixels;
    p_oid_vision_image->fY1Pixels = g_CVCamVisionImage.fY1Pixels;
    p_oid_vision_image->fX2Pixels = g_CVCamVisionImage.fX2Pixels;
    p_oid_vision_image->fY2Pixels = g_CVCamVisionImage.fY2Pixels;
    p_oid_vision_image->fCX1Pixels = g_CVCamVisionImage.fCX1Pixels;
    p_oid_vision_image->fCY1Pixels = g_CVCamVisionImage.fCY1Pixels;
    p_oid_vision_image->fCX2Pixels = g_CVCamVisionImage.fCX2Pixels;
    p_oid_vision_image->fCY2Pixels = g_CVCamVisionImage.fCY2Pixels;
    //OID_VISION_IMAGE ALERT SEND
    SmartToolMsg((td_STPsessions*) 0, STP_ALERT, MINIFT_OID_VISION_IMAGE,
            sizeof(td_oid_vision_image), (void*) g_STPtxMsg.p_cObjectValue);
    logf("%s %d\r\n", "OID_VISION_IMAGE", g_CVCamVisionImage.uiSequence);
}

/*FIXME000000000000 if we bring back data sending, copy from point system
 void SendDataGroup(char ci)
 {
 float f;
 float fy1;
 float fy2;
 //	When searching transitions between an old and new width, send the points

 td_oid_laser_sensor_gdata * p_oid_laser_sensor_gdata;
 p_oid_laser_sensor_gdata=(td_oid_laser_sensor_gdata *)g_STPtxMsg.p_cObjectValue;

 f=g_fCVDataW[ci];
 f=f/2;
 fy1=((float)g_lCVDataYa[ci]/g_ConfigData.EncoderRatio.fY);
 fy2=((float)g_lCVDataYb[ci]/g_ConfigData.EncoderRatio.fY);

 p_oid_laser_sensor_gdata->igroup= 0;
 p_oid_laser_sensor_gdata->fx1= -f;
 p_oid_laser_sensor_gdata->fy1= fy1;
 p_oid_laser_sensor_gdata->fx2= f;
 p_oid_laser_sensor_gdata->fy2= fy1;
 p_oid_laser_sensor_gdata->fx3= -f;
 p_oid_laser_sensor_gdata->fy3= fy2;
 p_oid_laser_sensor_gdata->fx4= f;
 p_oid_laser_sensor_gdata->fy4= fy2;

 p_oid_laser_sensor_gdata->igroup=htons(p_oid_laser_sensor_gdata->igroup);
 p_oid_laser_sensor_gdata->fx1=htonf(p_oid_laser_sensor_gdata->fx1);
 p_oid_laser_sensor_gdata->fy1=htonf(p_oid_laser_sensor_gdata->fy1);
 p_oid_laser_sensor_gdata->fx2=htonf(p_oid_laser_sensor_gdata->fx2);
 p_oid_laser_sensor_gdata->fy2=htonf(p_oid_laser_sensor_gdata->fy2);
 p_oid_laser_sensor_gdata->fx3=htonf(p_oid_laser_sensor_gdata->fx3);
 p_oid_laser_sensor_gdata->fy3=htonf(p_oid_laser_sensor_gdata->fy3);
 p_oid_laser_sensor_gdata->fx4=htonf(p_oid_laser_sensor_gdata->fx4);
 p_oid_laser_sensor_gdata->fy4=htonf(p_oid_laser_sensor_gdata->fy4);
 SmartToolMsg( (td_STPsessions *) 0, STP_ALERT, old_OID_LASER_SENSOR_GDATA, sizeof(td_oid_laser_sensor_gdata),(void *)g_STPtxMsg.p_cObjectValue );
 }
 */

//	CVCamSample
//	Galil manual indicates 1000 is their default port, so let's use that
#define CVCAM_CONNECTPORT 23
#define CVCAM_MAXSIZE 288  //256+32 for buffers, reading, and space for null
#define CVCAM_READSIZE 284

#ifndef IPADDR_CVCAM
#define IPADDR_CVCAM "192.168.0.74"
#endif 

//	State definitions for Ethernet connection to the camera.
#define INITCONN_INIT 0
#define INITCONN_SOCKOPEN 1
#define INITCONN_WAIT 2
#define INITCONN_DONE 3
#define INITCONN_MONITOR 4

//Private
void
CamConnection(void);
void
RxCVCamMsg();
//void EchoCamData();

//	FIXME PORTFATAL Continue testing this
//Ethernet socket interface.
int g_iCVCamSock_fd;
char g_cLastCVCamSocketState;
char g_cCVCamReady;
unsigned int g_uiCVCamConnects;
char g_cCVCamBuff[CVCAM_MAXSIZE];
char g_cCVCamBuffInb;
uint32 g_ulCVCamConnectTime; //and when down, contains connect start time...

void
ServiceCam(void)
{
    CamConnection(); //Get Connected, Monitor Connection.  Nonblocking.

    RxCVCamMsg(); //Read Packets
}

////////////////////////////////////////////////////////////////////////////////
//	MCConnection
////////////////////////////////////////////////////////////////////////////////
void
CamConnection(void)
{
    static int s_iState;
    static bool s_bInitialized = FALSE;
    int iResult, iEstablished;  //  , iAlive;
    uint32 ulElapsedMs;
    static int s_iEstabPrev;    //  , s_iAlivePrev;

    if( !s_bInitialized )
    {
        //	This is a port of how Dynamic C initializes statics.
        s_iState = INITCONN_INIT;
        //	Just clear variables that need to be maintained
        g_cCVCamReady = 0;
        g_uiCVCamConnects = 0;
        g_ulCVCamConnectTime = 0;
        s_bInitialized = TRUE;
    }

    switch (s_iState)
    {
    case INITCONN_INIT:
        //	If the socket is already open, ...
        //	if( tcp_tick(&g_iCVCamSock_fd ) != 0 )		//	Rabbit version
        //		sock_close( &g_iCVCamSock_fd );		//	Close it if it's already open
        //	Linux version.
        if( g_iCVCamSock_fd == 0 )
        {
            g_cCVCamReady = 0;
        }
        s_iState = INITCONN_SOCKOPEN;
        break;
    case INITCONN_SOCKOPEN:
        //	Open a socket to the camera.
        //	(Rabbit version).
        //if (tcp_open(&g_iCVCamSock_fd, 0, inet_addr(IPADDR_CVCAM), CVCAM_CONNECTPORT, NULL) == 0)
        //	Open a socket to the camera (Linux version).
        g_iCVCamSock_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if( g_iCVCamSock_fd < 0 )
        {
            //	Can't allocate socket...try to re-init
#ifdef USE_OUTPUT
            logf( "CVCam: sock fail %d\r\n", g_iCVCamSock_fd);
#endif 
            s_iState = INITCONN_INIT;
        }
        else
        //	Sock was created successfully.  __CONST_SOCKADDR_ARG  socklen_t
        {
            /*  The following is for a server connection.
            //	Bind the socket to an IP address and port.
            memset((char*) &CVSockAddr, 0, sizeof(CVSockAddr)); //	Clear the socket structure.
            CVSockAddr.sin_family = AF_INET;
            CVSockAddr.sin_port = htons(CVCAM_CONNECTPORT);
            CVSockAddr.sin_addr.s_addr = htonl(inet_addr(IPADDR_CVCAM));
            logf( "CVCam: sock = %d; port = %d; IPAddr = 0x%8x (%s:%d).\r\n", g_iCVCamSock_fd, CVSockAddr.sin_port, CVSockAddr.sin_addr.s_addr, IPADDR_CVCAM, CVCAM_CONNECTPORT );

            iResult = bind( g_iCVCamSock_fd, (struct sockaddr_in*) &CVSockAddr, sizeof(&CVSockAddr) );
            if( iResult < 0 )
            {
                perror("bind error");
                //logf( "CVCam: bind error %d.\r\n", iResult );
                close(g_iCVCamSock_fd);
                g_iCVCamSock_fd = 0; //	Flag the socket as not in use.
            }
            else
            //	bind was successful.
            {
                logf( "CVCam: bind success %d.\r\n ", iResult );
                //	Fetch option flags for this socket.
                int flags = fcntl( g_iCVCamSock_fd, F_GETFL, 0 );
                if( flags < 0 )
                {
                    logf("CVCam: fcntl get error", flags);
                    close(g_iCVCamSock_fd);
                    g_iCVCamSock_fd = 0; //	Flag the socket as not in use.
                }
                //	Set to NON blocking.
                iResult = fcntl(g_iCVCamSock_fd, F_SETFL, flags | O_NONBLOCK);
                if( iResult < 0 )
                {
                    logf( "CVCam: fcntl set error %d", iResult);
                    close(g_iCVCamSock_fd);
                    g_iCVCamSock_fd = 0; //	Flag the socket as not in use.
                }
                //	Now connect to the camera.
                iResult = connect( g_iCVCamSock_fd,  (struct sockaddr *) &CVSockAddr, sizeof(CVSockAddr));
                if( iResult < 0 )
                {
                    logf( "CVCam: connect error %d", iResult);
                    close(g_iCVCamSock_fd);
                    g_iCVCamSock_fd = 0; //	Flag the socket as not in use.
                }
                else
                {
#ifdef STP_OUTPUT
                    logf( "CVCam: connecting to %s:%d.\r\n", IPADDR_CVCAM, CVCAM_CONNECTPORT);
#endif 
                    s_iState = INITCONN_WAIT;
                    g_ulCVCamConnectTime = MS_TIMER;
                    g_cLastCVCamSocketState = 0xFF;
                }
            }
             */
            //  Open a client connection to the Cognex camera.
            CVSockAddr.sin_family = AF_INET;
            CVSockAddr.sin_port = htons(CVCAM_CONNECTPORT);
            iResult = inet_pton( AF_INET, IPADDR_CVCAM, &CVSockAddr.sin_addr );

            if( 0 > iResult )
            {
                perror( "CVCam: first parameter is not a valid address family." );
                close( g_iCVCamSock_fd );
                g_iCVCamSock_fd = 0;
                break;
            }
            if( 0 == iResult )
            {
                perror( "CVCam: char string (second parameter does not contain valid ipaddress)" );
                close( g_iCVCamSock_fd );
                g_iCVCamSock_fd = 0;
                break;
            }
            int flags = fcntl(g_iCVCamSock_fd, F_GETFL, 0 );
            if( flags < 0 )
            {
                perror( "CVCam: fcntl get failed" );
                close( g_iCVCamSock_fd );
                g_iCVCamSock_fd = 0;
                break;
            }
            if( fcntl( g_iCVCamSock_fd, F_SETFL, flags | O_NONBLOCK) < 0 )
            {
                perror( "error fcntl set failed" );
                close( g_iCVCamSock_fd );
                g_iCVCamSock_fd = 0;
                break;
            }
            iResult = connect( g_iCVCamSock_fd, (struct sockaddr *) &CVSockAddr,  sizeof(CVSockAddr) );
            g_ulCVCamConnectTime = MS_TIMER;
            s_iState = INITCONN_WAIT;

//FIXME PORTFATAL  must ensure nonblocking connect is capable, or shouldn't attempt to do this unless at cordinated time.
            /*  Allwo time for the connnection to work.
            if( iResult < 0 )
            {
                //FIXME VHDJWRRECHECK PORT RECHECK

    //FIXME PORTFATAL  wouldblock
                perror( "CVCam: connect failed. " );
                logf( "CVCam: Connect result = %d.\r\n", iResult );
                close( g_iCVCamSock_fd );
                g_iCVCamSock_fd = 0;
                break;
            }
            */
            //  Success so far, we think...
        }
        break;
    case INITCONN_WAIT:
        //	Verify the connection was a success.
        //	IMPORTANT! No equivalent exists in linux, so this state is not used.
        //  s_iState = INITCONN_DONE;

        //tcp_tick(NULL);  // call tcp_tick since this fcn may be called during initialization outside the main loop
         //	Print state of connection...notice no newline character
        iEstablished = sock_established( g_iCVCamSock_fd );
        //iAlive = sock_alive( &g_iCVCamSock_fd );

         ulElapsedMs = MS_TIMER - g_ulCVCamConnectTime;
         #ifdef USE_OUTPUT
        //if( g_cLastCVCamSocketState != CVSockAddr )
        //if(( s_iEstabPrev != iEstablished ) || ( s_iAlivePrev != iAlive ))
        if( s_iEstabPrev != iEstablished )
        {
            logf( "CVCam: ElapsedMs = %lu; socket = %u, established = %d.\r\n", /* alive = %d.\r",*/
                                ulElapsedMs, g_iCVCamSock_fd, iEstablished /*, iAlive */ );
            //g_cLastCVCamSocketState = CVSockAddr;
         }
         #endif
        if( iEstablished == 1 )     //  && ( iAlive != 0 ))
         {
            // sock is connected.
            logf( "CVCam: Connected. \r\n");
         s_iState = INITCONN_DONE;
         }
         else if( ulElapsedMs >= 6000 )
         {
            logf( "CVCam: Connect timeout. \r\n");
         s_iState = INITCONN_INIT;
         }
        else if( iEstablished == 0 )   //  && ( iAlive == 0 ))
         {
            //  Sock_alive went to zero - means we must re-open the socket.
            logf( "CVCam: Not connected. \r\n" );
         s_iState = INITCONN_INIT;
         }
        //  Remember states on this pass to compare on next pass.
        s_iEstabPrev = iEstablished;
        //s_iAlivePrev = iAlive;
        break;
    case INITCONN_DONE:
#ifdef USE_OUTPUT
        logf( "CVCam: Connection established.\r\n");
#endif 
        //	s_iState = INITCONN_MONITOR;	//	can't do this in linux.
        //	Init Ready
        g_cCVCamReady = 1;
        g_uiCVCamConnects++;
        g_cCamParseCode = CAMPARSE_NOTHING;
        g_cCamParseVariable = 0;
        g_cCVCamBuffInb = 0;
        //	Init Flag and stats
        g_cCenterVisionCamData = 0; //	No data
        g_ulCenterVisionCamDataTime = 0;
        g_uiCenterVisionCamDataCount = 0;
        break;
    case INITCONN_MONITOR:
         g_cCVCamReady = 1;
        iEstablished = sock_established( g_iCVCamSock_fd );
        if( iEstablished != 1 )
         {
            //  Connection is not up.
            logf( "CVCam: Connection down (t=%lu).\r\n", MS_TIMER );
            //	Not good: should init again.
         s_iState = INITCONN_INIT;
         g_cCVCamReady = 0;
         }

        break;
    }

}

void
CVCamSample()
{
    //future
    //PROPOSED PARAMETER SETTING
    //	char minibuffer[48];

    if( sock_established( g_iCVCamSock_fd ) != 1 )
    {
        //	Return the indicator which shows we are not connected
        g_cCamSampleStatus = 0;
        /* FIXME999999999999999 repair the return later
         g_LaserSensorData.fXNDistance=0.0;
         g_LaserSensorData.fXPDistance=0.0;
         g_LaserSensorData.fNEdgeHeight=0.0;
         g_LaserSensorData.fPEdgeHeight=0.0;
         g_LaserSensorData.fNCsnkEdgeHeight=0.0;
         g_LaserSensorData.fPCsnkEdgeHeight=0.0;
         */
        g_cCenterVisionCamData = 2; //indicate that data is present.
        g_ulCenterVisionCamDataTime = MS_TIMER;
        g_uiCenterVisionCamDataCount++;
        //	If the option is on, then echo this to the pendant
        /*
         if (g_cEchoCVCamData==1)
         {
         EchoCVCamData();
         g_cEchoCVCamData=0;
         }
         */
        //	And Clear this
        g_CVCamVisionImage.uiSequence = 0;
        return;
    }
    //	Set Data Flag
    g_cCenterVisionCamData = 1; //requested
    //	Prepare Parsing State
    g_cCamParseCode = CAMPARSE_SAMPLE;
    g_cCamParseVariable = 0;
    //future
    //PROPOSED PARAMETER SETTING
    //	//	Set any variable cells
    //	sprintf(minibuffer,"sfL4 %f\r\nsfL5 %f\r\n",g_VisionInspectResults.fXPositionExpected,g_VisionInspectResults.fYPositionExpected);
    //	SendCVCamRaw(minibuffer,strlen(minibuffer));
    //	Trigger Image
    SendCVCamRaw("SW8\r\n", 5);
}

void
CVCamSamplePart2()
{
    SendCVCamRaw("gvL021\r\n", 8); //seq  (new location)
}

void
CVCamSamplePart3()
{
    SendCVCamRaw("gvL022\r\n", 8); //Cam X (in cam coords... see below)
    SendCVCamRaw("gvL023\r\n", 8); //Cam Y
    SendCVCamRaw("gvL024\r\n", 8); //Center X in pixels  (in cam pixels... see below)
    SendCVCamRaw("gvL025\r\n", 8); //Center Y in pixels
    SendCVCamRaw("gvL026\r\n", 8); //horizontal res (width)
    SendCVCamRaw("gvL027\r\n", 8); //vertical res (height)
    SendCVCamRaw("gvL031\r\n", 8); //Diameter
    SendCVCamRaw("gvL032\r\n", 8); //Radius in pixels
    SendCVCamRaw("gvL033\r\n", 8); //Edge Status
    SendCVCamRaw("gvL034\r\n", 8); //Edge Status2
}

void
CVCamSamplePart4()
{
    SendCVCamRaw("gvL035\r\ngvL036\r\n", 16); //X1//Y1
    SendCVCamRaw("gvL037\r\ngvL038\r\n", 16); //X2//Y2
    SendCVCamRaw("gvL039\r\ngvL040\r\n", 16); //CX1//CY1
    SendCVCamRaw("gvL041\r\ngvL042\r\n", 16); //CX2//CY2
    SendCVCamRaw("gvL043\r\n", 8); //X1Pixels
    SendCVCamRaw("gvL044\r\n", 8); //Y1Pixels
    SendCVCamRaw("gvL045\r\n", 8); //X2Pixels
    SendCVCamRaw("gvL046\r\n", 8); //Y2Pixels
    SendCVCamRaw("gvL047\r\n", 8); //CX1Pixels
    SendCVCamRaw("gvL048\r\n", 8); //CY1Pixels
    SendCVCamRaw("gvL049\r\n", 8); //CX2Pixels
    SendCVCamRaw("gvL050\r\n", 8); //CY2Pixels
    SendCVCamRaw("gvL051\r\n", 8); //for now this is pixels per inch for image x (data y)
    SendCVCamRaw("gvL052\r\n", 8); //for now this is pixels per inch for image y (data x)
    //FUTURE
    //	input p1x converted to Pixels using camera cal grid
    //	input p1y converted to Pixels using camera cal grid
}

////////////////////////////////////////////////////////////////////////////////
// void RxCVCamMsg()
////////////////////////////////////////////////////////////////////////////////
void
RxCVCamMsg()
{
    //int inb;
    int irx;
    //char * p_c;
    //char * p_cEnd;
    //char c;
    //long * p_l;
    //long lx;
    //long ld;
    char *s;
    char *sval;
    char *sstop;

    if( sock_established( g_iCVCamSock_fd ) != 1 )
    {
        g_cCamParseCode = CAMPARSE_NOTHING;
        g_cCamParseVariable = 0;
        g_cCVCamBuffInb = 0;
        return;
    }

    irx = CVCAM_READSIZE - g_cCVCamBuffInb;
    if( irx <= 0 )
    {
        //	Full input buffer... shouldn't happen ... start again reading
        logf("CVCam: Full input buffer; dump and continue...\r\n");
        g_cCamParseCode = CAMPARSE_NOTHING;
        g_cCamParseVariable = 0;
        g_cCVCamBuffInb = 0;
        irx = CVCAM_READSIZE;
    }

    //	Sock_fastread returns # of bytes read
    //	irx = sock_fastread(&g_iCVCamSock_fd, g_cCVCamBuff+g_cCVCamBuffInb, irx);	//	Rabbit API.
    irx = read(g_iCVCamSock_fd, g_cCVCamBuff + g_cCVCamBuffInb, irx);
    if( irx == 0 )
    {
        //	Just nothing there
        return;
    }
    if( irx < 0 )
    //FIXME Medium : just see if there is any meaning in a negative code...
    // or if it never returns error this way, don't catch it
    {
        logf( "CVCam: read error.\r\n");
        return;
    }
    g_cCVCamBuffInb += irx;

    s = g_cCVCamBuff;
    if( *s == '\n' )
    {
        s++;
    } //skip n //LAST MESSAGE buffer ended in "\r", but this one started with the "\n"
    sval = s;
    sstop = s + g_cCVCamBuffInb;
    while( s < sstop )
    {
        if( *s == '\r' )
        //most messages end in \r (see below)
        {
            *s = 0;
            s++;
            if( *s == '\n' )
            {
                s++;
            } //skip n
            ParseCamMsg(sval);
            sval = s;
            continue;
        }
        s++;
    }
    if( sval < sstop )
    {
        //	Move the remaining down
        g_cCVCamBuffInb = sstop - sval;
        memcpy(g_cCVCamBuff, sval, g_cCVCamBuffInb);
    }
    else
    {
        //	Mark it clear
        g_cCVCamBuffInb = 0;
    }
}

void
ParseCamMsg(char *sval)
{
    int inb;
    char c;
    int iseq;
    //uint32 ulPrev;
    float f, fx, fy;

    fx = 0; //  Elminiate warnings about uninitialized stack variables.
    fy = 0;

    if( g_cCamParseCode == CAMPARSE_NOTHING )
    //FIXME0000000000 make login group
    {
        //	Also reinit these here
        g_fPixelsPerInchX = 786; //this is approximate and only true for one camera model
        g_fPixelsPerInchY = 786; //this is approximate and only true for one camera model

        //	Most messages end in \r, but the login does not.
        //	We make up for this by sending the login and password right when it says welcome.
        g_cCamParseVariable = 0;
        g_cCVCamBuffInb = 0;
        if( memcmp(g_cCVCamBuff, "Welcome", 7) == 0 )
        {
            //	Write Login.... And Password together
            SendCVCamRaw("admin\r\n\r\n", 9);
            return; //FIXME0000000000000000000 once works, fix pattern
        }
        //	It sees "Password" after it actually has logged in because password line is not parsed until
        //		the carriage return is sent.... we can deal with this efficiently just by matching it now...
        if( memcmp(g_cCVCamBuff, "Password", 7) == 0 )
        {
            //	We do have one more command to send
            SendCVCamRaw("SO1\r\n", 5);
            return;
        }
        logf("N:Unhandled!\r\n");
    }
    if( g_cCamParseCode == CAMPARSE_SAMPLE )
    {
        g_cCamParseVariable++;
        logf( "CVCam %d \"%s\"\r\n", g_cCamParseVariable, sval);
        if( g_cCamParseVariable == 17 )
        {
            logf("<dia\r\n"); //FIXME HIGH  remove debug
        }
        if( (g_cCamParseVariable & 1) == 0 )
        {
            goto quick_jump_even;
        }
        if( g_cCamParseVariable >= 25 )
        {
            goto quick_jump_25;
        }

        if( g_cCamParseVariable == 1 )
        {
            //	Did the query....
            //	First Variable: Clear error
            g_cCamHoleParseErrs = 0;
            g_cCamEdgeParseErrs = 0;
            if( sval[0] == '1' && sval[1] == 0 )
            {
                //	Success
                goto do_cam_sample_part_2;
            }
            if( sval[1] == '1' && sval[2] == 0 )
            {
                //	Success... but looks like error with junk character in fron... try to continue
                goto do_cam_sample_part_2;
            }
            //	Fail???
            g_cCamHoleParseErrs++;
            g_cCamEdgeParseErrs++;
            goto complete_sample_failure;
            do_cam_sample_part_2: CVCamSamplePart2();
            return;
        }
        if( g_cCamParseVariable == 3 )
        {
            if( g_cCamHoleParseErrs > 0 )
            {
                goto complete_sample_failure;
            }
            logf("seq=%s\r\n", sval);

            //	Set The Sequence and default other values

            iseq = atoi(sval);
            memset(&g_CVCamVisionImage, 0, sizeof(td_oid_vision_image)); //clear all values 1st
            g_VisionInspectResults.uiImageSequence = iseq;
            g_CVCamVisionImage.uiSequence = iseq;
            if( iseq == 0 )
            {
                g_cCamHoleParseErrs++;
                g_cCamEdgeParseErrs++;
                logf("s0\r\n");
            }
            //g_CVCamVisionImage.uiFlags = 0; //Flags 0 means nothing found Entire structure is zeroed above
            CVCamSamplePart3();
            return;
        }
#ifdef CAM_ORIENATION_STANDARD
#ifdef CAM_ORIENATION_CXMXN
#fatal "Can't define CAM_ORIENATION_CXMXN when CAM_ORIENATION_STANDARD is defined"
#endif 
#endif 
#ifndef CAM_ORIENATION_STANDARD
#ifndef CAM_ORIENATION_CXMXN
//  FIXME FATAL "Cam Orientation undefined"
#endif 
#endif 

#ifdef CAM_ORIENATION_STANDARD
        //	The Camera has an odd orientation that will require use to convert the coordinates
        //	Camera Orientation....
        //	Cam Y+ is X-
        //	Cam X+ is Y-
#endif 
#ifdef CAM_ORIENATION_CXMXN
        //	Cam X is X-
        //	Cam Y is Y
#endif 
        if( g_cCamParseVariable == 5 )
        //	Tool Y From Center to object
        {
            //	Value in this position is CAM X
        	g_fCamX = atofSafe(sval,0);
            if( atofSafeFailure != 0 )
            {
                g_cCamHoleParseErrs++;
            }
            //FIXME FUTURE HIGH  "using fixed Z height scale factor for this. (using 1.0)" ...
            //	if this were to go back to variable height system this would need to be restored and redesigned with z height in mind
            return;
        }
        if( g_cCamParseVariable == 7 )
        //Tool X From Center to object
        {
            //	Value in this position is CAM Y
            g_fCamY = atofSafe(sval,0);
            if( atofSafeFailure != 0 )
            {
                g_cCamHoleParseErrs++;
            }

#ifdef CAM_ORIENATION_STANDARD
            //	Convert Camera Position X mm  into minift Y delta to camera inches
            g_fCamHoleY = g_fCamX / - 25.4;
            //	Convert Camera Position Y mm  into minift X delta to camera inches
            g_fCamHoleX = g_fCamY / - 25.4;
#endif 
#ifdef CAM_ORIENATION_CXMXN
            //Convert Camera Position X mm  into minift X delta to camera inches
            g_fCamHoleX = g_fCamX / - 25.4;
            //Convert Camera Position Y mm  into minift Y delta to camera inches
            g_fCamHoleY = g_fCamY / 25.4;
#endif 
            //FIXME FUTURE HIGH  see z height comment above
            if( g_ConfigData.cToolFlip != Y_POS )
            {
                //The direction the camera is talking about is backwards
                g_fCamHoleX = -g_fCamHoleX;
                g_fCamHoleY = -g_fCamHoleY;
#ifdef OUTPUT_FLIP
                logf( "Flipped because of ToolFlip\r\n" );
#endif 
            }
            g_CVCamVisionImage.fX = g_fCamHoleX;
            g_CVCamVisionImage.fY = g_fCamHoleY;
            g_CVCamVisionImage.uiFlags = 3; //means found circle to draw + it's accepted as a pass
            return;
        }
        if( g_cCamParseVariable == 9 )
        //	Center X in pixels  (in cam pixels... see below)
        {
            //	(X pixels in Camera orientation from edge)
            g_CVCamVisionImage.fXPixels = atofSafe(sval,0);
            if( atofSafeFailure != 0 )
            {
                g_CVCamVisionImage.uiFlags = 0;
                g_CVCamVisionImage.fXPixels = 0;
            }
            return;
        }
        if( g_cCamParseVariable == 11 )
        //	Center Y in pixels
        {
            //	(Y pixels in Camera orientation from edge)
            g_CVCamVisionImage.fYPixels = atofSafe(sval,0);
            if( atofSafeFailure != 0 )
            {
                g_CVCamVisionImage.uiFlags = 0;
                g_CVCamVisionImage.fYPixels = 0;
            }
            return;
        }
        if( g_cCamParseVariable == 13 )
        {
            //	Width In Pixels
            g_CVCamVisionImage.uiWidth = (int) atofSafe(sval,0);
            if( atofSafeFailure != 0 )
            {
                g_CVCamVisionImage.uiFlags = 0;
                g_CVCamVisionImage.uiWidth = 0;
            }
            return;
        }
        if( g_cCamParseVariable == 15 )
        {
            //	Height In Pixels
            g_CVCamVisionImage.uiHeight = (int) atofSafe(sval,0);
            if( atofSafeFailure != 0 )
            {
                g_CVCamVisionImage.uiFlags = 0;
                g_CVCamVisionImage.uiHeight = 0;
            }
            return;
        }
        if( g_cCamParseVariable == 17 )
        {
            //	Diameter in mm
            g_fCenterVisionDiameter = atofSafe(sval,0) / 25.4;
            g_CVCamVisionImage.fDiameter = g_fCenterVisionDiameter;
            if( atofSafeFailure != 0 )
            {
                g_fCenterVisionDiameter = 0;
                g_CVCamVisionImage.uiFlags = 0;
                g_CVCamVisionImage.fDiameter = 0;
            }
            return;
        }
        if( g_cCamParseVariable == 19 )
        {
            //	Radius in pixels ... ignore any prospect that pixel may not be square... cam is not square, but pixels of cam are.
            g_CVCamVisionImage.fDiameterPixels = 2 * atofSafe(sval,0);
            if( atofSafeFailure != 0 )
            {
                g_CVCamVisionImage.uiFlags = 0;
                g_CVCamVisionImage.fDiameterPixels = 0;
            }
            //	No longer calculate this.... set below
            //g_CVCamVisionImage.fPixelsPerInch = 0;
            //if (g_CVCamVisionImage.fDiameter > 0)
            //{
            //	//	Just get this approx number from diameter ratio.
            //	g_CVCamVisionImage.fPixelsPerInch = g_CVCamVisionImage.fDiameterPixels / g_CVCamVisionImage.fDiameter;
            //}
            CVCamSamplePart4();
            logf("edges next\r\n");
            return;
        }
        if( g_cCamParseVariable == 21 )
        // Edge status 1
        {
            g_cEdgeStatus1 = 0;
            if( *sval == '1' )
            {
                g_cEdgeStatus1 = 1;
            }
            return;
        }
        if( g_cCamParseVariable == 23 )
        //	Edge status 2
        {
            g_cEdgeStatus2 = 0;
            if( *sval == '1' )
            {
                g_cEdgeStatus2 = 1;
            }
            return;
        }
        quick_jump_25:
        //see orientation notes above about coordinate transform
        if( g_cCamParseVariable == 25 )
        //	x1 cam
        {
            g_fCamX = atofSafe(sval,0);
            if( atofSafeFailure != 0 )
            {
                g_cEdgeStatus1 = 0;
            }
            return;
        }
        if( g_cCamParseVariable == 27 )
        //	y1 cam
        {
            g_fCamY = atofSafe(sval,0);
            if( atofSafeFailure != 0 )
            {
                g_cEdgeStatus1 = 0;
            }

#ifdef CAM_ORIENATION_STANDARD
            //x1 cam to minift -y1
            //y1 cam to minift -x1
            //Convert Camera Position X mm  into minift Y delta to camera inches
            fy = g_fCamX / - 25.4;
            //Convert Camera Position Y mm  into minift X delta to camera inches
            fx = g_fCamY / - 25.4;
#endif 
#ifdef CAM_ORIENATION_CXMXN
            //	Convert Camera Position X mm  into minift X delta to camera inches
            fx = g_fCamX / - 25.4;
            //	Convert Camera Position Y mm  into minift Y delta to camera inches
            fy = g_fCamY / 25.4;
#endif 
            if( g_ConfigData.cToolFlip != Y_POS )
            {
                //The direction the camera is talking about is backwards
                fx = -fx;
                fy = -fy;
            }
            g_VisionInspectResults.fEdgeX1 = fx;
            g_CVCamVisionImage.fX1 = fx;
            g_VisionInspectResults.fEdgeY1 = fy;
            g_CVCamVisionImage.fY1 = fy;
            return;
        }
        if( g_cCamParseVariable == 29 )
        //x2 cam
        {
            g_fCamX = atofSafe(sval,0);
            if( atofSafeFailure != 0 )
            {
                g_cEdgeStatus1 = 0;
            }
            return;
        }
        if( g_cCamParseVariable == 31 )
        //y2 cam
        {
            g_fCamY = atofSafe(sval,0);
            if( atofSafeFailure != 0 )
            {
                g_cEdgeStatus1 = 0;
            }

#ifdef CAM_ORIENATION_STANDARD
            //	x2 cam to minift -y2
            //	y2 cam to minift -x2
            //	Convert Camera Position X mm  into minift Y delta to camera inches
            fy = g_fCamX / - 25.4;
            //	Convert Camera Position Y mm  into minift X delta to camera inches
            fx = g_fCamY / - 25.4;
#endif 
#ifdef CAM_ORIENATION_CXMXN
            //	Convert Camera Position X mm  into minift X delta to camera inches
            fx = g_fCamX / - 25.4;
            //	Convert Camera Position Y mm  into minift Y delta to camera inches
            fy = g_fCamY / 25.4;
#endif 
            if( g_ConfigData.cToolFlip != Y_POS )
            {
                //	The direction the camera is talking about is backwards
                fx = -fx;
                fy = -fy;
            }
            g_VisionInspectResults.fEdgeX2 = fx;
            g_CVCamVisionImage.fX2 = fx;
            g_VisionInspectResults.fEdgeY2 = fy;
            g_CVCamVisionImage.fY2 = fy;
            return;
        }
        if( g_cCamParseVariable == 33 )
        //	cx1 cam
        {
            g_fCamX = atofSafe(sval,0);
            if( atofSafeFailure != 0 )
            {
                g_cEdgeStatus2 = 0;
            }
            return;
        }
        if( g_cCamParseVariable == 35 )
        //	cy1 cam
        {
            g_fCamY = atofSafe(sval,0);
            if( atofSafeFailure != 0 )
            {
                g_cEdgeStatus2 = 0;
            }

#ifdef CAM_ORIENATION_STANDARD
            //	cx1 cam to minift -cy1
            //	cy1 cam to minift -cx1
            //	Convert Camera Position X mm  into minift Y delta to camera inches
            fy = g_fCamX / - 25.4;
            //	Convert Camera Position Y mm  into minift X delta to camera inches
            fx = g_fCamY / - 25.4;
#endif 
#ifdef CAM_ORIENATION_CXMXN
            //	Convert Camera Position X mm  into minift X delta to camera inches
            fx = g_fCamX / - 25.4;
            //	Convert Camera Position Y mm  into minift Y delta to camera inches
            fy = g_fCamY / 25.4;
#endif 
            if( g_ConfigData.cToolFlip != Y_POS )
            {
                //	The direction the camera is talking about is backwards
                fx = -fx;
                fy = -fy;
            }
            g_VisionInspectResults.fCEdgeX1 = fx;
            g_CVCamVisionImage.fCX1 = fx;
            g_VisionInspectResults.fCEdgeY1 = fy;
            g_CVCamVisionImage.fCY1 = fy;
            return;
        }
        if( g_cCamParseVariable == 37 )
        //	cx2 cam
        {
            g_fCamX = atofSafe(sval,0);
            if( atofSafeFailure != 0 )
            {
                g_cEdgeStatus2 = 0;
            }
            return;
        }
        if( g_cCamParseVariable == 39 )
        //	cy2 cam
        {
            g_fCamY = atofSafe(sval,0);
            if( atofSafeFailure != 0 )
            {
                g_cEdgeStatus2 = 0;
            }

#ifdef CAM_ORIENATION_STANDARD
            //	cx2 cam to minift -cy2
            //	cy2 cam to minift -cx2
            //	Convert Camera Position X mm  into minift Y delta to camera inches
            fy = g_fCamX / - 25.4;
            //	Convert Camera Position Y mm  into minift X delta to camera inches
            fx = g_fCamY / - 25.4;
#endif 
#ifdef CAM_ORIENATION_CXMXN
            //	Convert Camera Position X mm  into minift X delta to camera inches
            fx = g_fCamX / - 25.4;
            //	Convert Camera Position Y mm  into minift Y delta to camera inches
            fy = g_fCamY / 25.4;
#endif 
            if( g_ConfigData.cToolFlip != Y_POS )
            {
                //The direction the camera is talking about is backwards
                fx = -fx;
                fy = -fy;
            }
            g_VisionInspectResults.fCEdgeX2 = fx;
            g_CVCamVisionImage.fCX2 = fx;
            g_VisionInspectResults.fCEdgeY2 = fy;
            g_CVCamVisionImage.fCY2 = fy;
            return;
        }
        if( g_cCamParseVariable == 41 )
        //x1pixels
        {
            f = atofSafe(sval,0);
            g_VisionInspectResults.fEdgeX1Pixels = f;
            g_CVCamVisionImage.fX1Pixels = f;
            if( atofSafeFailure != 0 )
            {
                g_cEdgeStatus1 = 0;
            }
            return;
        }
        if( g_cCamParseVariable == 43 )
        //y1pixels
        {
            f = atofSafe(sval,0);
            g_VisionInspectResults.fEdgeY1Pixels = f;
            g_CVCamVisionImage.fY1Pixels = f;
            if( atofSafeFailure != 0 )
            {
                g_cEdgeStatus1 = 0;
            }
            return;
        }
        if( g_cCamParseVariable == 45 )
        //	x2pixels
        {
            f = atofSafe(sval,0);
            g_VisionInspectResults.fEdgeX2Pixels = f;
            g_CVCamVisionImage.fX2Pixels = f;
            if( atofSafeFailure != 0 )
            {
                g_cEdgeStatus1 = 0;
            }
            return;
        }
        if( g_cCamParseVariable == 47 )
        //	y2pixels
        {
            f = atofSafe(sval,0);
            g_VisionInspectResults.fEdgeY2Pixels = f;
            g_CVCamVisionImage.fY2Pixels = f;
            if( atofSafeFailure != 0 )
            {
                g_cEdgeStatus1 = 0;
            }
            return;
        }
        if( g_cCamParseVariable == 49 )
        //	cx1pixels
        {
            f = atofSafe(sval,0);
            g_VisionInspectResults.fCEdgeX1Pixels = f;
            g_CVCamVisionImage.fCX1Pixels = f;
            if( atofSafeFailure != 0 )
            {
                g_cEdgeStatus2 = 0;
            }
            return;
        }
        if( g_cCamParseVariable == 51 )
        //	cy1pixels
        {
            f = atofSafe(sval,0);
            g_VisionInspectResults.fCEdgeY1Pixels = f;
            g_CVCamVisionImage.fCY1Pixels = f;
            if( atofSafeFailure != 0 )
            {
                g_cEdgeStatus2 = 0;
            }
            return;
        }
        if( g_cCamParseVariable == 53 )
        //	cx2pixels
        {
            f = atofSafe(sval,0);
            g_VisionInspectResults.fCEdgeX2Pixels = f;
            g_CVCamVisionImage.fCX2Pixels = f;
            if( atofSafeFailure != 0 )
            {
                g_cEdgeStatus2 = 0;
            }
            return;
        }
        if( g_cCamParseVariable == 55 )
        //	cy2pixels
        {
            f = atofSafe(sval,0);
            g_VisionInspectResults.fCEdgeY2Pixels = f;
            g_CVCamVisionImage.fCY2Pixels = f;
            if( atofSafeFailure != 0 )
            {
                g_cEdgeStatus2 = 0;
            }
            return;
        }
        //	NOTE: Cam pixels per inch x and y are only approximate,
        //	Since it actually converts with a grid, however this is the best we have
        //	for the additional rotations below
        if( g_cCamParseVariable == 57 )
        {
            f = atofSafe(sval,0);
            g_fPixelsPerInchX = f;
            g_CVCamVisionImage.fPixelsPerInch = f;
            if( atofSafeFailure != 0 )
            {
                g_CVCamVisionImage.fPixelsPerInch = 0;
            }
            return;
        }
        if( g_cCamParseVariable == 59 )
        {
            f = atofSafe(sval,0);
            g_fPixelsPerInchY = f;
            g_CVCamVisionImage.fPixelsPerInch = f;
            if( atofSafeFailure != 0 )
            {
                g_CVCamVisionImage.fPixelsPerInch = 0;
            }
            //	Drop down and complete sample
        }
        else
        {
            quick_jump_even:
            //	must be one of the command response lines...
            //	has to be 1
            //logf("cmdrline : %d : %s\r\n", g_cCamParseVariable, sval);
            if( sval[0] == '1' && sval[1] == 0 )
            {
                //	Success
            }
            else if( g_cCamParseVariable >= 54 )
            {
                //	Should be 1, but I have seen many times it returns "" at 54 and both numbers are fine.
                logf("ignore\r\n");
            }
            else
            {
                logf("fail0\r\n");
                if( g_cCamParseVariable >= 21 )
                {
                    //	Applies to edges part
                    g_cEdgeStatus1 = 0;
                    g_cEdgeStatus2 = 0;
                }
                else if( g_cCamParseVariable >= 15 )
                {
                    //	Applies only to radius of circle on image... so just fail that part
                    g_CVCamVisionImage.uiFlags = 0;
                }
                else
                {
                    //	Got to record this as an error
                    //	Fail???
                    g_cCamHoleParseErrs++;
                }
            }
            return;
        }

        logf("Rx Done\r\n"); //	Done Reading Sample
        if( g_cCamEdgeParseErrs > 0 )
        {
            //	Clear these if there were any parse errors effecting edge path
            g_cEdgeStatus1 = 0;
            g_cEdgeStatus2 = 0;
            logf("epe\r\n");
        }
        //	Complete Edge Status
        g_VisionInspectResults.cEdgeStatus = 0;
        if( g_cEdgeStatus1 == 1 )
        {
            g_VisionInspectResults.cEdgeStatus |= 1;
        }
        if( g_cEdgeStatus2 == 1 )
        {
            g_VisionInspectResults.cEdgeStatus |= 2;
        }

        g_VisionInspectResults.cEdgeNote = 0;

        g_CVCamVisionImage.cEdgeStatus = g_VisionInspectResults.cEdgeStatus;
        g_CVCamVisionImage.cEdgeNote = 0;
        //	Done Edge Status

        g_cCamParseCode = CAMPARSE_NOTHING;
        g_cCamParseVariable = 0;

        g_cCamSampleStatus = LASERSENSOR_ONLINE;
        if( g_cCamHoleParseErrs == 0 )
        {
            g_cCamSampleStatus |= LASERSENSOR_SEE_HOLE | LASERSENSOR_SEE_CENTER;
        }
        if( g_VisionInspectResults.cEdgeStatus != 0 )
        {
            g_cCamSampleStatus |= LASERSENSOR_SEE_EDGE;
        }
        if( g_cCamSampleStatus == LASERSENSOR_ONLINE )
        {
            complete_sample_failure: logf("Rx Fail\r\n"); //	Sample Failure....
            g_cCamSampleStatus = LASERSENSOR_ONLINE;
        }

        g_cCenterVisionCamData = 2; //	Indicate that data is present.
        g_ulCenterVisionCamDataTime = MS_TIMER;
        g_uiCenterVisionCamDataCount++;

        //	If the option is on, then echo this to the pendant
        if( g_cEchoCVCamData == 1 )
        {
            EchoCVCamData();
            g_cEchoCVCamData = 0;
        }
        return;
    }
    logf("Cam \"%s\"\r\n", sval);

    inb = 0;
    while( 1 )
    {
        c = g_cCVCamBuff[inb++];
        if( c == 0 )
        {
            break;
        }
        if( c > 20 )
        {
            logf("%c", c);
        }
        else
        {
            logf("(%d)", c);
        }
    }
    logf("\r\n");

}

void
CVCamClose(void)
{
    close(g_iCVCamSock_fd);
    g_iCVCamSock_fd = 0;
}

void
EchoCVCamData(void)
{
    /* FIXME999999999999999999999999
     td_LaserSensorData * p_LaserSensorData;
     p_LaserSensorData=(td_LaserSensorData *)g_STPtxMsg.p_cObjectValue;
     p_LaserSensorData->cStatus=g_LaserSensorData.cStatus;
     p_LaserSensorData->fXNDistance=g_LaserSensorData.fXNDistance;
     p_LaserSensorData->fXPDistance=g_LaserSensorData.fXPDistance;
     p_LaserSensorData->fNEdgeHeight=g_LaserSensorData.fNEdgeHeight;
     p_LaserSensorData->fPEdgeHeight=g_LaserSensorData.fPEdgeHeight;
     p_LaserSensorData->fNCsnkEdgeHeight=g_LaserSensorData.fNCsnkEdgeHeight;
     p_LaserSensorData->fPCsnkEdgeHeight=g_LaserSensorData.fPCsnkEdgeHeight;
     SmartToolMsg( (td_STPsessions *) 0, STP_ALERT, OID_LASER_SENSOR_DATA, sizeof(td_LaserSensorData), g_STPtxMsg.p_cObjectValue);
     */
}

void
SendCVCamRaw(char *buffer, int len)
{
    char buffertemp[256];

    memcpy(buffertemp, buffer, len);
    buffertemp[len] = 0;
    logf("txC:%s", buffer);
    //	sock_flushnext(&g_iCVCamSock_fd);		//	No equivalent in Linux.
    sock_write( g_iCVCamSock_fd, buffer, len);
    //	sock_flush(&g_iCVCamSock_fd);			//	No equivalent in Linux.
}

void
SendCVCamFromConsole(char *buffer, int len)
{
    char buffertemp[256];
    memcpy(buffertemp, buffer, len);
    buffertemp[len++] = '\r';
    buffertemp[len++] = '\n';
    buffertemp[len++] = 0;
    logf("txC:%s", buffer);
    //	sock_flushnext(&g_iCVCamSock_fd);		//	No equivalent in Linux.
    sock_write( g_iCVCamSock_fd, buffer, len);
    //	sock_flush(&g_iCVCamSock_fd);			//	No equivalent in Linux.
}

//Call After Library is done, but if Position and Postion Expected Are Calculated different outside (as with Edge Vec currently)
// then this can find the best pixel values for those in this.
void
RecalculatePostionPixels()
{
    float fx1, fy1, fx2, fy2;
    float fx1Pixels, fy1Pixels, fx2Pixels, fy2Pixels;
    float fx, fy, fpx, fpy;
    //float fzx,fzy,fdx,fdy;
    float fdpx, fdpy, f;
    //Camera Rotation is fixed and the camera is assumed to be in a certain orientation.
    //Find the pixel values for position and expectedposition for the cases where these are updated

    logf("rpp\r\n");

    if( g_cEdgeStatus1 == 1 )
    {
        //	Use primary edge
        fx1 = g_VisionInspectResults.fEdgeX1;
        fy1 = g_VisionInspectResults.fEdgeY1;
        fx2 = g_VisionInspectResults.fEdgeX2;
        fy2 = g_VisionInspectResults.fEdgeY2;
        fx1Pixels = g_VisionInspectResults.fEdgeX1Pixels;
        fy1Pixels = g_VisionInspectResults.fEdgeY1Pixels;
        fx2Pixels = g_VisionInspectResults.fEdgeX2Pixels;
        fy2Pixels = g_VisionInspectResults.fEdgeY2Pixels;
    }
    else if( g_cEdgeStatus2 == 1 )
    //	Must use center edge...
    {
        fx1 = g_VisionInspectResults.fCEdgeX1;
        fy1 = g_VisionInspectResults.fCEdgeY1;
        fx2 = g_VisionInspectResults.fCEdgeX2;
        fy2 = g_VisionInspectResults.fCEdgeY2;
        fx1Pixels = g_VisionInspectResults.fCEdgeX1Pixels;
        fy1Pixels = g_VisionInspectResults.fCEdgeY1Pixels;
        fx2Pixels = g_VisionInspectResults.fCEdgeX2Pixels;
        fy2Pixels = g_VisionInspectResults.fCEdgeY2Pixels;
    }
    else
    {
        //	Allow rough conversion ??? //Currently ONLY using this for the cases that there are edges
        logf("nrc\r\n");
        return;
    }

    //	Now convert
    //logf("p\r\n");
    //logf("%f %f\r\n",fx1,fy1);
    //logf("%f %f\r\n",fx1Pixels,fy1Pixels);
    logf("c\r\n");
    fx = g_VisionInspectResults.fXPosition;
    fy = g_VisionInspectResults.fYPosition;
    logf("%f %f\r\n", fx, fy);
    //to convert the position, use the fact that it's got to be on the pixel line!
    //NOTE:  this ONLY works if the cognex calibration has zero, or very minimal rotation...
    //The ultimate solution is to ask the Cognex to convert the pixels.
    fdpx = fx2Pixels - fx1Pixels;
    fdpy = fy2Pixels - fy1Pixels;
    if( fabs(fdpx) > fabs(fdpy) )
    {
        //do conversion based on x pixels percentage...
        //Y position goes along with X pixels.
        fy -= fy1; //translate relative to one end
        fy2 -= fy1;
        if( fy2 == 0 )
        {
            //these are like the same point!...  There shouldn't even be a line
            goto fail_pixel_convert;
        }
        f = fy / fy2; //the ratio of how far along this line the intersection is...  could be greater than 1 or negative
    }
    else
    {
        //do conversion based on y pixels percentage...
        //X position goes along with Y pixels.
        fx -= fx1; //translate relative to one end
        fx2 -= fx1;
        if( fx2 == 0 )
        {
            //these are like the same point!...  There shouldn't even be a line
            goto fail_pixel_convert;
        }
        f = fx / fx2; //the ratio of how far along this line the intersection is... could be greater than 1 or negative
    }
    //now apply the ratio
    fpx = fx1Pixels + f * fdpx;
    fpy = fy1Pixels + f * fdpy;
    logf("%f %f\r\n", fpx, fpy);
    g_VisionInspectResults.fXPositionPixels = fpx;
    g_VisionInspectResults.fYPositionPixels = fpy;

    //Now convert expected position to pixels
    //For the second position, placing it on the line will not work.
    //A full rotation would be best, but the very best would be to ask the cognex to
    //map the coordinates back to pixels.

    //Currently, this applies the pixels per inch from the camera, which
    //only works well if there is little or no rotation (outside of the fact that Y and X are flipped.
    //(There isn't supposed to be any rotation, but the image calibration may be doing this a tiny bit)

    //To help reduce the error of this method, the conversion can be done from the intersection pixel
    fdpx = g_fPixelsPerInchX;
    fdpy = g_fPixelsPerInchY;
    fx = g_VisionInspectResults.fXPositionExpected;
    fy = g_VisionInspectResults.fYPositionExpected;
    logf("%f %f\r\n", fx, fy);
    fx -= g_VisionInspectResults.fXPosition; //translate relative to conversion point...
    fy -= g_VisionInspectResults.fYPosition;
#warning "FIXME SEVERE test pixel rotation of expected position"
    //my concern is that there is a reversal here which hides an earlier reversal.
#ifdef CAM_ORIENATION_STANDARD
    fpx = ( fy *fdpx ) + g_VisionInspectResults.fXPositionPixels; //image x is based on position -y
    fpy = ( fx *fdpy ) + g_VisionInspectResults.fYPositionPixels;//image y is based on position -x
#endif 
#ifdef CAM_ORIENATION_CXMXN
    fpx = ( fx *fdpx ) + g_VisionInspectResults.fXPositionPixels; //image x is based on position -x
    fpy = ( - fy * fdpy ) + g_VisionInspectResults.fYPositionPixels;//image y is based on position y
#endif 
    g_VisionInspectResults.fXPositionExpectedPixels = fpx;
    g_VisionInspectResults.fYPositionExpectedPixels = fpy;
    logf("%f %f\r\n", fpx, fpy);
    return;

    fail_pixel_convert: logf("b\r\n");
    g_VisionInspectResults.fXPositionPixels = 0;
    g_VisionInspectResults.fYPositionPixels = 0;
    g_VisionInspectResults.fXPositionExpectedPixels = 0;
    g_VisionInspectResults.fYPositionExpectedPixels = 0;
    return;
}

//#endif	//	#IF0 that temporarily excludes this library from the MiniFt build.

#ifdef VISION_STANDALONETEST
int
main(void)
{

    return 0;
}

#endif //	VISION_STANDALONETEST
#endif //END #ifdef CENTERVISION_CAM
