

#ifndef CENTERVISIONCAM_H_
#define CENTERVISIONCAM_H_

#include "MiniFTDefs.h"

///////////////////////////////////////////////////////////////////////////////
//	Public symbols and definitions.
///////////////////////////////////////////////////////////////////////////////

//	Enable the following in MiniFTDefs.h to compile and link the camera subsystem
//	Comment it out to NOT include the camera subsystem in the MiniFt build.
//#define CENTERVISION_CAM 1

#ifdef CENTERVISION_CAM


///////////////////////////////////////////////////////////////////////////////
//Public Globals
///////////////////////////////////////////////////////////////////////////////

extern char g_cCenterVisionResult;			//	The current state and result state.
extern int g_iCenterVisionResultMessage;	//	A message code that explains a failure state.
extern char g_cCenterVisionCamData;			//	A flag used to tell when samples are recieved

//	Configuration pParameter Globals for the center vision system.
extern char g_cCenterVisionInspectType;		//	Select what type of inspection to use
extern char g_cCenterVisionRequiredResults;
extern float g_fCenterVisionExpectedDiameter;	//	In inches. Or pass 0 if unknown.
extern float g_fCenterVisionMoveSpeed;		//	Specify carriage (x,y) motion speed in inches / second.

//	Physical (x,y) offset of the camera center from the center of the drill head.
extern float g_fCenterVisionOffsetX;		//	Input Vision Offset is removed at the end for communicating object positions.
extern float g_fCenterVisionOffsetY;

///////////////////////////////////////////////////////////////////////////////
//Public Functions
///////////////////////////////////////////////////////////////////////////////
void PreviewCenterVision();
void ProgressCenterVision();
void PositionInspection();
void CancelCenterVision();

void ServiceCam(void);
void EchoCVCamData();

void CVCamSample();
void CVCamClose();
void SendCVCamRaw(char * buffer, int len);
void SendCVCamFromConsole(char * buffer, int len);

void RecalculatePostionPixels();

#endif	//	CENTERVISION_CAM

#endif	//	CENTERVISIONCAM_H_
