// MiniFTWXY.h
// MiniFT Whistle Gold Motion Library
//
// The Library is based on new methods rather than a direct port of the entire old MiniFT Whistle Library
// For more information about the features of the new library, and changes from the old library, see The main library source file.

#ifndef MINIFTWXY_H_
#define MINIFTWXY_H_

#include "SmartTool.h"
#include "CommonSmartTool.h"
#include "MiniFTSmartTool.h"

#ifdef GENCIRCMFT2
#define FORCE_LIMIT_WHISTLE_CODE
#endif

//FIXME PORTHIGH  PUB AND PRIVATE... consider reorg of some of these defs...
//i values for the Whistle
#define NoMessage 0
#define GravCompRun 1
#define GravCompOK 2
#define GravCompFail 3
#define PositionRun 4
#define PositionOK 5
#define PositionFail 6
#define PositionStop 7

#define JogRun 9
#define PositionStart 10
#define HomeRun 12
#define HomeOK	13
#define HomeFail 14
#define AutoPERR 16
#define AutoER 17
#define ForceLimitCurrent 24
#define FindWinRun 25
#define FindWinOK 26
#define FindWinFail 27
#define FindWinNone 28

#define ServoHereNow 32
#define MotorOffNow 33

//local motionsets
#define UnknownMotionSet 0
#define GravMotionSet 1
#define JogMotionSet 2
#define PositionMotionSet 3
#define HomeMotionSet 4

//Init And Service
void MiniFTWhistleInit(void);

void MiniFTWhistleService(void);

void MCSetCommDisplayX(byte c);
void MCSetCommDisplayY(byte c);
void MCSetCommEnableX(byte cEnable);
void MCSetCommEnableY(byte cEnable);

//Settings and Operations
void MCSetConfigParams(void);
void MCSetEncoderRatios();
void MCSetErrorLimits();
void MCSetCurrentLimits(float fx, float fy, float fpx, float fpy); //units in amps
void MCSetPositionLimits(byte bAllowLimitSetNow);
void MCSetVelmonAndBoostParameters(void);
void MCSetSD();



///////////////ALL OF THEMHERE
void MCClearMotionSet();
void MCRunRestart();
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




//Create External Calls which can be used to send directly to whistle
//These are similar to the internal macros, however the point is to
//carefully limit places which external code is passing in messages.
void MiniFTSendWhistleMsgX(char * s);
void MiniFTSendWhistleMsgXL(char * s, int l);
void MiniFTSendWhistleMsgY(char * s);
void MiniFTSendWhistleMsgYL(char * s, int l);

void MiniFTSendWhistleMsgXY(char * s);
void MiniFTSendWhistleMsgXYL(char * s, int l);


//FIXME PORTMED are these also needed global access?
void RunMotorActionBrakeTimeoutCheck();
void RunMotorActionAirClear();
//This is, so rename...
void SetMotorActionDirectly();



//	Global defines


#define GETPOSN_CLEAR 0
#define GETPOSN_NEEDED 1
#define GETPOSN_REQUESTED 2
#define GETPOSN_FRESH 3

//Global Variables
extern byte g_cMCRunningMsgGoal;
extern byte g_cMCWindowFound;



#endif /* MINIFTWXY_H_ */
