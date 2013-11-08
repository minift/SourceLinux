////////////////////////////////////////////////////////////////////////////////
//
//	MiniFtLube.c - COOLANT / LUBRICANT MANAGEMENT
//
//	This module is responsible managing lube delivery for MiniFt systems
//	that do not have a SmartDrill to manage lube/coolant delivery when drilling. 
//  If no coolant / lube delivery controls are installed, or if a SmartDrill has
//  responsibility for controlling coolant / lube, set CoolantState to NOT_INSTALLED
//  and BurstOnDuration to zero.
//	
//	CoolantModes are not implemented because there is no knowledge of
//	material layers in a drill cycle on which to base changes to lube delivery.
//	So when drilling, ON and OFF are used to control CoolantState, the state of the coolant supply valve.
//  To prime the lube plumbing, turn CoolantState ON then OFF when priming is done.
//
//	By defining a non-zero SpritzDuration, coolant can be run in Spritz mode.
//  Spritz mode turns coolant on for a timed duration in ms at the start of a drill cycle.
//	Coolant is turned off when the timer expires.
//	If the spritz duration is zero, lube/coolant is delivered during the entire drill cycle.
//
//	ConfigData parameters that control lube modulation ("Burster" valve) control:
//	o	BurstIntervalMs is elapsed ms from start of one burst cycle to the start of the next burst. 
//	o	BurstDutyCycle is the percentage (0-100) of burst interval during which the burst valve is open.
//  A BurstOnDuration of zero indicates that the burster valve is NOT installed.
//	If the burst valve is installed, burst logic MUST ALWAYS used when the coolant supply valve is OPEN.
//  If burst duration and interval settings are invalid, typical valid values will be forced.
//
//	Revision history:
//		2013-10-28	TC		Original coding for LFT 2.6, which runs Angstrom Linux on a Beagle Bone platform.
//
////////////////////////////////////////////////////////////////////////////////

//#define LUBE_STANDALONETEST 1

//#ifdef LUBE_STANDALONETEST
//#define FALSE   0
//#define TRUE    1

//#define logf(x) SttpDrillLog( STTP_MSGTYPE_ALERT, 'p', x );

//#endif

//#define logf printf

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <math.h>
#include <errno.h>

#include "SmartTool.h"
#include "MiniFTDefs.h"
#include "hwio.h"
#include "CommonSmartTool.h"
#include "MiniFTSmartTool.h"
#include "MiniFTIO.h"
#include "CrawlerConfigScript.h"
#include "STP.h"
#include "SmartToolUtil.h"
#include "SocketConsole.h"

#include "MiniFtLube.h"


///////////////////////////////////////////////////////////////////////////////
//	Local (private) global variables
///////////////////////////////////////////////////////////////////////////////

byte 	g_bSpritzState;		//	See enums define above.
bool    g_bBursterVerbose;      //  Set TRUE to get detailed log messages on BursterState.
int     g_iBursterState;        //  COOLANT_ON or COOLANT_OFF.

uint16  g_uiSpritzStartTimeMs;  //  Time tag when spritz mode was started. 0 if not started.
uint16	g_uiSpritzTimeLeftMs;	//	Countdown timer for spritz mode.

//	Local copies of ConfigData values:
//	IMPORTANT: These may be set to override ConfigData values if configuration is invalid.
int16   g_iCoolantState;        //  Boolean lube supply valve state. TRUE -> valve ON, FALSE -> valve OFF.
uint16	g_uiBurstIntervalMs;	//	Interval in ms between start of one burst cycle and start of the next burst cycle. If zero, no burster is installed.
uint16	g_uiBurstOnDurationMs;	//	This must be less than BurstIntervalMs.

static char g_szTemp[256];      //  Scratch string buffer for event logging.

//	Prototypes of private functions.
int CoolantSpritzActive( void );
int CoolantSpritzCheck( void );
int BursterActive( void );
void BursterStateSet( int iState );
int BursterDutyCycleUpdate( void );


///////////////////////////////////////////////////////////////////////////////
//	Public functions for Coolant / Lube control.
///////////////////////////////////////////////////////////////////////////////

//  MiniFt turns COOLANT_ON at the start of a drill cycle or to prime coolant lines.
//  MiniFt must set COOLANT_OFF to shut off the coolant supply valve at the end of a drill cycle.
//	This is a low level function to set the controlled state of the coolant/lube supply valve.
//	It maintains coolant supply valve state to match the requested setting.
void CoolantStateSet( int iState )
{
	//int iPrevState;       //  Enable for debug print (below).

	//iPrevState = g_iCoolantState;

	switch( iState )
	{
	case COOLANT_NOT_INSTALLED:	//	No coolant subsystem control is installed in this MiniFt
		//	OR coolant is controlled by the SmartDrill.
		g_iCoolantState = iState;
		break;
	case COOLANT_ON:	//	Flowing
		//	Open coolant flow immediately.
	    digOut( DIGOUT_CHANENTRY_COOLANT, DIGOUT_ON );
		g_iCoolantState = iState;
		break;
	case COOLANT_OFF:	//	Not flowing
		//	Kill coolant flow immediately.
	    digOut( DIGOUT_CHANENTRY_COOLANT, DIGOUT_OFF );
		g_iCoolantState = iState;
		if( CoolantSpritzActive() )
		    CoolantSpritzStateSet( SPRITZ_OFF );
		break;
	default:
	    logf( "Invalid CoolantState requested. Coolant system disabled.\r\n" );
	    g_iCoolantState = COOLANT_NOT_INSTALLED;
	    break;
	}

	/*	Debug print of lube/coolant valve position. 
	if( iPrevState != g_iCoolantState )
	{
		sprintf( g_szTemp, "Coolant: %d", g_iCoolantState );
		logf( g_szTemp );
	}
	*/
}

//  MiniFt has to turn SPRITZ_ON at the start of a drill sequence that uses spritz.
//  The ConfigData parameter SpritzDurationMs controls timing of spritz shut-off.
int CoolantSpritzStateSet( int iSpritzCmd )
{
	switch( iSpritzCmd )
	{
	case SPRITZ_DISABLED: 
		g_ConfigData.uiSpritzDurationMs = 0;	
		g_uiSpritzTimeLeftMs = 0;
        g_uiSpritzStartTimeMs = 0;
        g_bSpritzState = iSpritzCmd;
		break;
	case SPRITZ_OFF:
		CoolantStateSet( COOLANT_OFF );
		g_uiSpritzTimeLeftMs = 0;
		g_uiSpritzStartTimeMs = 0;
        g_bSpritzState = iSpritzCmd;
		break;
	case SPRITZ_ON:
		//	Start only if the feature has been enabled. 
		if( g_ConfigData.uiSpritzDurationMs > 0 )
		{	//	Set the countdown timer, and open the coolant valve. 
			g_uiSpritzStartTimeMs = MS_TIMER;
			CoolantStateSet( COOLANT_ON );
			g_bSpritzState = iSpritzCmd;
		}
		else
		    logf( "Spritz requested but SpritzDuration is zero.\r\n" );
		break;
    default:
        logf( "Invalid SpritzState requested. Spritz disabled\r\n" );
        g_bSpritzState = SPRITZ_DISABLED;
        break;
	}
	return((int) g_bSpritzState );
}

//  This is the coolant / lube service function that must be called continually from the MiniFt main loop.
void CoolantUpdate( void )
{
    if( g_iCoolantState == COOLANT_ON )
    {
        if( g_cModeState == MODE_ESTOP )
        {   //  Coolant flow is prohibited in E-STOP.
            CoolantStateSet( COOLANT_OFF );
        }
    }
	CoolantSpritzCheck();
	BursterDutyCycleUpdate();
	
}


///////////////////////////////////////////////////////////////////////////////
//	Private functions.
///////////////////////////////////////////////////////////////////////////////

//	Return Boolean TRUE if Spritz is currently active; FALSE if not.
int CoolantSpritzActive( void )
{
	int iActive;

	//	If a start time was recorded, and timer has not expired, Spritz is active.
	iActive = ( g_ConfigData.uiSpritzDurationMs > 0 ) && ( g_uiSpritzStartTimeMs != 0 ) && ( g_uiSpritzTimeLeftMs != 0 );

	return( iActive );
}

//	Manages coolant ON -> OFF transition driven by the spritz timer.
//	This function is called ONLY if CoolantSpritzActive.
//	This function is responsible for maintaining SpritzTimeLeftMs after a spritz has been started.
int CoolantSpritzCheck( void )
{
	//int	iCoolantState;

	//	If spritz countdown timer is active. , ...
	if( CoolantSpritzActive() )
	{	//	Check the state of the countdown timer.
		g_uiSpritzTimeLeftMs = g_ConfigData.uiSpritzDurationMs - GetDiff( MS_TIMER, g_uiSpritzStartTimeMs );
		if( g_uiSpritzTimeLeftMs <= 0 )
		{	//	Timer has expired. Turn off coolant supply valve. 
			CoolantSpritzStateSet( SPRITZ_OFF );
		}
        if( g_cModeState == MODE_ESTOP )
        {   //  Coolant flow is prohibited in E-STOP.
            CoolantSpritzStateSet( SPRITZ_OFF );
        }
	}	//	Endif spritz active.

	//  Spritz mode is NOT currently active.
	//	If spritz mode is programmed (i.e. a duration has been specified), but timer has expired, ...
	else if( g_ConfigData.uiSpritzDurationMs > 0 )
	{	//	Safety net. Make sure coolant supply valve is OFF.
		if( g_iCoolantState != COOLANT_OFF )
			CoolantStateSet( COOLANT_OFF );
	}

	//	Return CoolantState based on spritz timer.
	return( (int) g_iCoolantState );
}


//	Return Boolean TRUE if Burst Mode SHOULD be active.
//	Burst mode is active when:
//		COOLANT_ON and BurstOnDurationMs in ConfigData is > 0.
int BursterActive( void )
{
	int iActive;
	static int s_iActivePrev = FALSE;

	iActive = FALSE;	//	Assume burst mode is not active.
	//	BurstOnDuration MUST ONLY be set > 0 if a burster valve is present.
	//	If this installation is configured to use a burster valve, ...
	//	FIXME: How do MiniFt substems tell if ESTOP is engaged?
	if( g_ConfigData.uiBurstOnDurationMs > 0L )
	{	//	If in MODE_ESTOP, ...
		if( g_cModeState == MODE_ESTOP )
		{	//	Coolant flow is prohibited.
			iActive = FALSE;
		}

		//	Otherwise, enable burster any time coolant flow is enabled.
		else if( g_iCoolantState & COOLANT_ON )
		{
			iActive = TRUE;
		}
	}	//	Endif a coolant burster is configured on this unit.
	else        //  No burster valve installed. Burster is never active.
		iActive = FALSE;

	//	Debug print of change in burster active status.
	if( s_iActivePrev != iActive )
	{
		s_iActivePrev = iActive;
		sprintf( g_szTemp, "Burster %sACTIVE.\r\n", ( iActive ? "" : "NOT " ));
		logf( g_szTemp );
	}
	return( iActive );
}

//	Set up Burster duty cycle parameters.
//	Return 0 on success, or non-zero error code on failure.
int BursterInit( void )
{
	int	iCase;		//	Identify reason to override with a safe value.

	iCase = 0;		//	Assume configured values will be used.
	g_uiBurstOnDurationMs = g_ConfigData.uiBurstOnDurationMs;
	g_uiBurstIntervalMs =  g_ConfigData.uiBurstIntervalMs;

	//	Check values from ConfigData.
	//	First see if burster is enabled.
	if( g_ConfigData.uiBurstOnDurationMs <= 0 )
	{	//	Burster not installed.
		iCase = 0;
		logf( "BursterInit: Burst valve not installed.\r\n" );
	}

	//	If burst duration is unreasonably long, ....
	else if( g_ConfigData.uiBurstOnDurationMs > 5000 )
	{	//	Shorten ON duration to a typical value of 1 second.
		iCase = 1;
		g_uiBurstOnDurationMs = 1000;
	}
	
	//	Check OnDuration vs. burst interval.
	else if( g_uiBurstOnDurationMs > g_ConfigData.uiBurstIntervalMs * 0.8 )
	{	//	On duration exceeds burst interval.
		iCase = 2;
		g_uiBurstIntervalMs = g_uiBurstOnDurationMs * 2.0;
		logf( "BursterInit: Burst duty cycle of 1:2 imposed. " );
	}

	//	Log results of the burster ON and OFF duration calculations.
	sprintf( g_szTemp, "BursterInit: OnDur = %d ms; OffDur = %d ms.\r\n", g_uiBurstOnDurationMs, g_uiBurstIntervalMs - g_uiBurstOnDurationMs );
	logf( g_szTemp );

	//	Return an indication if IF and WHY an override value was used.
	return( iCase );
}


//	Set low level state of coolant/lube Burster valve. Maintain valve state to match.
void BursterStateSet( int iState )
{
	//int iPrevState;

	//	Remember the state on entry to this function to support debug prints (below).
	//iPrevState = g_iBursterState;

	//	If current state if OFF and desired state is ON, ...
	if(( g_iBursterState == COOLANT_OFF ) && ( iState = COOLANT_ON ))
	{
	    digOut( DIGOUT_CHANENTRY_BURSTER, DIGOUT_ON );
		g_iBursterState = COOLANT_ON;
	}
	if(( g_iBursterState == COOLANT_ON ) && ( iState == COOLANT_OFF ))
	{
	    digOut( DIGOUT_CHANENTRY_BURSTER, DIGOUT_OFF );
		g_iBursterState = COOLANT_OFF;
	}

	//	Debug print of changes in burster state.
	//if( iPrevState != g_iBursterState )
	//{
	//	sprintf( g_szTemp, "Burster %s", GetDigoutStateString( g_iBursterState ) );
	//	SttpDrillLog( STTP_MSGTYPE_ALERT, 'p', g_szTemp );
	//}
}

//	Update Burster valve state based on Burster duty cycle settings.
//	When burster is changed from inactive to active, BursterInit is called to set up the duty cycle.
int BursterDutyCycleUpdate( void )
{
	static int s_iActive = FALSE;
	static uint16 s_uiBurstOnTimeMs;
	static uint16 s_uiBurstOffTimeMs;
	static uint16 s_uiBurstOnDurationMs;
	static uint16 s_uiBurstOffDurationMs;
	int iResult;

	//	If Burster should be active, ...
	iResult = 0;
	if( BursterActive())
	{	//	But is not currently active, ...
		if( s_iActive == FALSE )
		{	//	Calculate duration of ON and OFF phases of the burst duty cycle.
			iResult = BursterInit();    //  iLayer, &s_uiBurstOnDurationMs, &s_uiBurstOffDurationMs );
			//	BursterInit ALWAYS sets usable ON and OFF durations.
			s_iActive = TRUE;
			//	Start the ON portion of the duty cycle.
			s_uiBurstOnTimeMs = MS_TIMER;
			s_uiBurstOffTimeMs = MS_TIMER;	//	Garbage trap.
			BursterStateSet( COOLANT_ON );
			logf( "Burster: Starting.\r\n" );
		}	//	Endif Burster needs to be activated.
		else	//	Burster has been started. Maintain the duty cycle.
		{	//	Branch to current burster valve state.
			switch( g_iBursterState )
			{
			case COOLANT_ON:
				if( GetDiff( MS_TIMER, s_uiBurstOnTimeMs ) > s_uiBurstOnDurationMs )
				{
					s_uiBurstOffTimeMs = MS_TIMER;
					BursterStateSet( COOLANT_OFF );
					if( g_bBursterVerbose )
						logf("Burster OFF.\r\n" );
				}
				break;
			case COOLANT_OFF:
				if( GetDiff( MS_TIMER, s_uiBurstOffTimeMs ) > s_uiBurstOffDurationMs )
				{
					s_uiBurstOnTimeMs = MS_TIMER;
					BursterStateSet( COOLANT_ON );
					if( g_bBursterVerbose )
						logf( "Burster ON.\r\n" );
				}
				break;
			default:	//	Garbage trap. Treat any other value as OFF state.
				if( g_iBursterState == COOLANT_ON  )
				{
					BursterStateSet( COOLANT_OFF );
					if( g_bBursterVerbose )
						logf( "Burster OFF.\r\n" );
				}
				break;
			}	//	End switch on current BursterState.
		}	//	Endif Burster is active.
	}	//	Endif Burster should be active.

	//	Else Burster should be inactive. Is it?
	else
	{
		if( s_iActive != FALSE )
		{	//	Deactivate the Burster.
			BursterStateSet( COOLANT_OFF );
			s_iActive = FALSE;
			logf( "Burster: Stopping.\r\n" );
		}
		else	//	Should be inactive already.
		{	//	If we have not turned off the burster valve, ...
			if( g_iBursterState == COOLANT_ON  )
			{	//	Do so now.
				BursterStateSet( COOLANT_OFF );
				if( g_bBursterVerbose )
					logf( "Burster OFF.\r\n" );
			}
		}
	}	//	Endif commanded and actual states are both INACTIVE>
	return( iResult );
}

