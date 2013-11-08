// MiniFTIO.c
#include "MiniFTDefs.h"
#include "MiniFTIO.h"
#include "SmartTool.h"
#include "CommonSmartTool.h"
#include "MiniFTSmartTool.h"
#include "SmartToolUtil.h"
#include "SocketConsole.h"
#include "AngleSensor.h"
#include "LoadCell.h"

//MiniFt Core DIO
byte g_cLED;
#ifdef BEEPSYSTEM
byte g_cBeep = 0;
#endif

byte g_cLEDCount;
uint16 g_uiLEDOn;
uint16 g_uiLEDWhole;
uint32 g_ulLEDStart;
#ifdef BEEPSYSTEM
byte g_cBeepMode;
byte g_cBeepCount;
uint16 g_uiBeepOn;
uint16 g_uiBeepWhole;
uint32 g_ulBeepStart;
#endif

//DIO

//HD
// Assume initial HDRAIL settings are clamped to force transition to CLAMP_LOOSE at start-up.
#ifdef CLAMP_SYSTEM_HD_PISTON
byte g_cALock = ALOCK_ON;
byte g_cLegsLock = LEGSLOCK_ON;
byte g_cLegsDown = VALVE_OFF;
byte g_cLegsUp = VALVE_ON;
byte g_cClampExtend = VALVE_ON;
byte g_cClampRetract = VALVE_ON;
#endif
byte g_cAirBlastX = 0;
byte g_cAirBlastY = 0;


#ifdef SEAL_SYSTEM
byte g_cSealantApply;
byte g_cSealantPressure;
byte g_cSealantPinch;
#endif

#ifdef GENCIRCMFTX
//CIRCMFT
#ifdef DIGOUT_CHNUM_DRILLUP
byte g_cDrillUp = 0;
#endif
byte g_cDrillDown = DRILL_DOWN_UP;
byte g_cDrillButton = 0;
byte g_cColletClamp = 0;
#endif

byte g_cLaserPointer;

#ifdef UNIBUTTON
byte g_cDigInUniButtonXCount=1;
byte g_cDigInUniButton=0;
#endif
byte g_cDigInEstopSignalXCount=1;
byte g_cDigInEstopSignal=1;

byte g_cMachineLock=0;
#ifdef DIGIN_CHNUM_XLOCK
byte g_cDigInXLock=1;
byte g_cDigInXLockXCount=1;
#endif
#ifdef DIGIN_CHNUM_YLOCK
byte g_cDigInYLock=1;
byte g_cDigInYLockXCount=1;
#endif

#ifdef GENCIRCMFTX
byte g_cDigInDrillUp=0; //no count
//REMOVED byte g_cDigInDrillDown=0; //no count
#endif

#ifdef Y_LIMIT_SENSORS
byte g_cDigInYPosLimit=1; //no count
byte g_cDigInYNegLimit=1;//no count
#endif

#ifdef PRESSURESENSOR
byte g_cDigInAirPressure=0;
byte g_cDigInAirPressureXCount=1;
#endif

#ifdef ORIENTATION_SENSORS
byte g_cDigInYOrientationA=1; //no count
byte g_cDigInYOrientationB=1;//no count
#endif
byte g_cPrevOrSensors=0xFF;

//Obstruction System
byte g_cObstructionCode = 0;
byte g_cLastObstructionCode = 0;
byte g_cObstructionCodeNew = 0;

byte g_cObstructionWarningCode = 0;
byte g_cLastObstructionWarningCode = 0;

#ifdef OBSTRUCTION_SYSTEM_XP1
byte g_cDigInObstructionXP1 = NO_OBSTRUCTION;
#endif
#ifdef OBSTRUCTION_SYSTEM_XN1
byte g_cDigInObstructionXN1 = NO_OBSTRUCTION;
#endif
#ifdef OBSTRUCTION_SYSTEM_MOW
byte g_cDigInObstructionMOW = MO_NO_OBSTRUCTION;
#endif
#ifdef OBSTRUCTION_SYSTEM_MOS
byte g_cDigInObstructionMOS = MO_NO_OBSTRUCTION;
#endif
#ifdef OBSTRUCTION_SYSTEM_MO_ANA
byte g_cMOFlags = 0;
#endif
#ifdef DIGIN_SOFTSTOP
byte g_cDigInSoftStop = 1;
#endif

#ifdef UNIBUTTON
byte g_cUniButtonEvent = 0;
#endif

//Use one lock event for all lock changes
byte g_cLockEvent = 0;
#ifdef PRESSURESENSOR
byte g_cAirPressureEvent = 2; //because it can watch for both directions
#endif
//Obstruction System
byte g_cObstructionEvent = 0;
//Obstruction Flag Event
byte g_cMCObstructionEvent = 0;
//Obstruction Warning Event
byte g_cObstructionWarningEvent = 0;

byte g_cBrakeReleased = 0;


void MiniFTIOInit()
{
	HWIOInit();
	InitDIO();
}

////////////////////////////////////////////////////////////////////////////////
// InitDIO
////////////////////////////////////////////////////////////////////////////////
void InitDIO(void)
{
	//set managed channels
	LEDOff();
	BeepOff();

	//set brake on at boot
	BrakeOn();

	//This assumes the outputs are off already because they default to off.

	SetDefaultValveState();
}

void SetDefaultValveState()
{
	//Turn all valves and tool related output to defaults
	//Always set Tool Machine states to defaults on estop, even if the eeoption is not active.

	//HD

	//All valves other than brake are part of the clamp machine.
	//Since Loose is exactly the same as the desired boot state, setting the default
	//is simply ensuring that it goes to the Loose State
#ifdef CLAMP_SYSTEM
#ifdef CLAMP_SYSTEM_NAC_STP
	//do not use SetDefaultValveState for this clamp system
#else
	g_cClampState = CLAMP_UNKNOWN; //true init state
	g_cClampGoal = CLAMP_LOOSE_OR_UNCLAMP; //and make sure it goes to LOOSE
#endif
#endif

	//Drill Fill Integrated
#ifdef SEAL_SYSTEM
	//FIXME0 ensure that Clamp will clear fastener ram, or will not operate in that case (because of conflict)
	g_cSealantApply = SEALANT_APPLY_OFF;
	g_cSealantPressure = SEALANT_PRESSURE_OFF;
	g_cSealantPinch = SEALANT_PINCH_ON;
	digOut(DIGOUT_CHNUM_SEALANT_APPLY, g_cSealantApply);
	digOut(DIGOUT_CHNUM_SEALANT_PRESSURE, g_cSealantPressure);
	digOut(DIGOUT_CHNUM_SEALANT_PINCH, g_cSealantPinch);
#endif
}

////////////////////////////////////////////////////////////////////////////////
// InitADC
////////////////////////////////////////////////////////////////////////////////
void InitADC(void)
{
	// analog inputs configured by calling brdInit() which is done in main()
	// keep this state, though, in case we want to do more
	// advanced initialization of the ADC
	// default setup for ADC on BL2100 is -10V to 10V, 12bits
}

////////////////////////////////////////////////////////////////////////////////
// DigOutService
////////////////////////////////////////////////////////////////////////////////
void DigOutService(void)
{
	unsigned long ulNow;
	unsigned long ul;
	unsigned int ui;
	static byte s_cLEDSet = 2; //ensure 1st value will be obeyed even if it is 1 or 0
	static byte s_cBeepSet = 2;

#ifdef LEDSYSTEM
	if (g_cLEDCount > 0)
	{
		ulNow = MS_TIMER;
		ul = ulNow - g_ulLEDStart;
		while (ul >= g_uiLEDWhole)
		{
			g_ulLEDStart += g_uiLEDWhole;
			ul -= g_uiLEDWhole;
			g_cLEDCount--;
			if (g_cLEDCount == 0)
			{
				goto completed_LED_count;
			}
		}
		if (ul < g_uiLEDOn)
		{
			g_cLED = 1;
		}
		else
		{
			completed_LED_count: g_cLED = 0;
		}
	}
	if (s_cLEDSet != g_cLED)
	{
		digOut(DO_LIGHT_GREEN, g_cLED);
		s_cLEDSet = g_cLED;
	}
#endif

#ifdef BEEPSYSTEM
	if (g_ConfigData.cbeeper == 0)
	{
		g_cBeepCount = 0;
		g_cBeepMode = BEEPOFF;
		g_cBeep = 0;
	}
	else if (g_cBeepCount > 0)
	{
		ulNow = MS_TIMER;
		ul = ulNow - g_ulBeepStart;
		while (ul >= g_uiBeepWhole)
		{
			g_ulBeepStart += g_uiBeepWhole;
			ul -= g_uiBeepWhole;
			if (g_cBeepCount < 255)
			{
				g_cBeepCount--;
				if (g_cBeepCount == 0)
				{
					g_cBeepMode = BEEPOFF;
					goto completed_Beep_count;
				}
			}
		}
		if (ul < g_uiBeepOn)
		{
			g_cBeep = 1;
		}
		else
		{
			completed_Beep_count: g_cBeep = 0;
		}
	}
	if (s_cBeepSet != g_cBeep)
	{
		digOut(DO_BEEPER_1, g_cBeep);
		s_cBeepSet = g_cBeep;
		//I don't like this being here, but it's the only single mode that needs this feature
		//This location makes it operate only once a cycle so there is no concern about CPU time
		if (g_cBeepMode == BEEPPROBEK && g_cBeep == 1)
		{
			//Special pattern 2 sections 120ms and 880ms. Beep at start of each
			//	For the Setting specified by BeepProbeK2 macro.
			//Setup a special pattern for every other time.
			//Change the duration of the cycle each time
			if (g_uiBeepWhole > 120)
			{
				g_uiBeepWhole = 120;
			}
			else
			{
				g_uiBeepWhole = 880;
			}
		}
	}
#endif
}

#ifdef BEEPSYSTEM
void Beep()
{
	BeepCountPrivate(BEEPONCE, 1, 500, 800);
}
#endif

void BrakeOn()
{
	//Brake control may be by one or both of Rabbit and MC
#ifdef RABBITBRAKES
	g_cBrakeReleased = 0;
	digOut(DO_BRAKE_RELEASE, BRAKE_RELEASE_OFF);
#ifdef DIGOUT_CHNUM_BRAKE_ALTERNATE
	digOut(DIGOUT_CHNUM_BRAKE_ALTERNATE, 0);
#endif
#ifdef OUTPUT_BRAKE
	logf("BrakeOn()\r\n");
#endif
#endif
#ifdef MCBRAKES
	MCBrakeOn();
#endif
}
void BrakeOff()
{
	//Brake control may be by one or both of Rabbit and MC
//FIXME PORTLOW correct this pattern
#ifdef RABBITBRAKES
	g_cBrakeReleased = 1;
	digOut(DO_BRAKE_RELEASE, BRAKE_RELEASE_ON);
#ifdef DIGOUT_CHNUM_BRAKE_ALTERNATE
	digOut(DIGOUT_CHNUM_BRAKE_ALTERNATE, 1);
#endif
#ifdef OUTPUT_BRAKE
	logf("BrakeOff()\r\n");
#endif
#endif
#ifdef MCBRAKES
	MCBrakeOff();
#endif
}

////////////////////////////////////////////////////////////////////////////////
// ReadDigitalInputs Debounce / DIO / ADC
////////////////////////////////////////////////////////////////////////////////
//FIXME0
byte g_cFAKETEST;

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

	byte cv;
#ifdef UNIBUTTON
	cv=digIn(DIGIN_CHNUM_UNIBUTTON);
	if (cv!=g_cDigInUniButton)
	{
		//disagreement ... less common path
		g_cDigInUniButtonXCount++;
		if (g_cDigInUniButtonXCount>=DIGIN_DEBOUNCE_HISTORY_COUNT)
		{
			//now reach the count... flip
			g_cDigInUniButton=cv;
			g_cDigInUniButtonXCount=0;
			//Set Event Flag also
			// If high user can handle and set to low.
			// If signal goes low, turn off event flag even if they did not handle it.
			//	(a matter of choice for this style.)
			g_cUniButtonEvent=cv;
		}
	}
	else
	{
		//agreement
		g_cDigInUniButtonXCount=0;
	}
#endif

	cv = digIn(DI_ESTOP);
	if (cv != g_cDigInEstopSignal)
	{
		//disagreement ... less common path
		g_cDigInEstopSignalXCount++;
		if (g_cDigInEstopSignalXCount >= DIGIN_DEBOUNCE_HISTORY_COUNT)
		{
			//now reach the count... flip
			g_cDigInEstopSignal = cv;
			g_cDigInEstopSignalXCount = 0;
		}
	}
	else
	{
		//agreement
		g_cDigInEstopSignalXCount = 0;
	}

#ifdef DIGIN_CHNUM_XLOCK
	cv=digIn(DIGIN_CHNUM_XLOCK);
	if (cv!=g_cDigInXLock)
	{
		//disagreement ... less common path
		g_cDigInXLockXCount++;
		if (g_cDigInXLockXCount>=DIGIN_XLOCK_DEBOUNCE_HISTORY_COUNT)
		{
			//now reach the count... flip
			g_cDigInXLock=cv;
			g_cDigInXLockXCount=0;
			//Set Event Flag also (use one lock event)
			g_cLockEvent=1;
#ifdef OUTPUT_DIGIN_CHNUM_XLOCK
			logf("X Lock Sensor Now %d\r\n",(int)cv);
#endif
		}
	}
	else
	{
		//agreement
		g_cDigInXLockXCount=0;
	}
#endif

#ifdef DIGIN_CHNUM_YLOCK
	cv=digIn(DIGIN_CHNUM_YLOCK);
	if (cv!=g_cDigInYLock)
	{
		//disagreement ... less common path
		g_cDigInYLockXCount++;
		if (g_cDigInYLockXCount>=DIGIN_DEBOUNCE_HISTORY_COUNT)
		{
			//now reach the count... flip
			g_cDigInYLock=cv;
			g_cDigInYLockXCount=0;
			//Set Event Flag also (use one lock event)
			g_cLockEvent=1;
#ifdef OUTPUT_DIGIN_CHNUM_YLOCK
			logf("Y Lock = %d\r\n",(int)cv);
#endif
		}
	}
	else
	{
		//agreement
		g_cDigInYLockXCount=0;
	}
#endif

	//Sum up the lock status when it changes
	if (g_cLockEvent == 1) //happens only when changed or very recently changed on loop  before event is cleared
	{
		g_cMachineLock = 0;
#ifdef DIGIN_CHNUM_XLOCK
		if (g_cDigInXLock == 0) //locked
		{
			g_cMachineLock |= XLock;
		}
#endif
#ifdef DIGIN_CHNUM_YLOCK
		if (g_cDigInYLock == 0) //locked
		{
			g_cMachineLock |= YLock;
		}
#endif
	}

	//No Debounce : Signals pre amplified and/or controled.
#ifdef GENCIRCMFTX
	g_cDigInDrillUp = digIn(DI_DRILL_UP);
	//REMOVED g_cDigInDrillDown = digIn(DIGIN_CHNUM_DRILLDOWN);
#endif
#ifdef Y_LIMIT_SENSORS
	//Y Limit
	g_cDigInYPosLimit=digIn(DIGIN_CHNUM_Y_POS_LIMIT);
	g_cDigInYNegLimit=digIn(DIGIN_CHNUM_Y_NEG_LIMIT);
#endif

#ifdef OBSTRUCTION_SYSTEM_XP1
	g_cDigInObstructionXP1 = digIn(DI_OBSTRUCTION_XP1);
#endif
#ifdef OBSTRUCTION_SYSTEM_XN1
	g_cDigInObstructionXN1 = digIn(DI_OBSTRUCTION_XN1);
#endif
#ifdef OBSTRUCTION_SYSTEM_MOW
	g_cDigInObstructionMOW = digIn(DI_OBSTRUCTION_MOW);
#endif
#ifdef OBSTRUCTION_SYSTEM_MOS
	g_cDigInObstructionMOS = digIn(DI_OBSTRUCTION_MOS);
#endif

	g_cObstructionCode = 0;
#ifdef OBSTRUCTION_SYSTEM_XP1
	if (g_cDigInObstructionXP1 == OBSTRUCTION)
	{
		g_cObstructionCode |= 1;
	}
#endif
#ifdef OBSTRUCTION_SYSTEM_XN1
	if (g_cDigInObstructionXN1 == OBSTRUCTION)
	{
		g_cObstructionCode |= 2;
	}
#endif
#ifdef OBSTRUCTION_SYSTEM_MOS
	if (g_cDigInObstructionMOS == MO_OBSTRUCTION)
	{
		g_cObstructionCode |= 4;
	}
#endif
#ifdef OBSTRUCTION_SYSTEM_MO_ANA
	if (g_cMOFlags>0)
	{	g_cObstructionCode |= 4;}
#endif

#ifdef Y_LIMIT_SENSORS
	if (g_cDigInYPosLimit==0)
	{	g_cObstructionCode |= 16;}
	if (g_cDigInYNegLimit==0)
	{	g_cObstructionCode |= 32;}
#endif
	if ((g_cMCObstructionEvent & X_AXIS) > 0)
	{
		g_cObstructionCode |= 64;
	}
	if ((g_cMCObstructionEvent & Y_AXIS) > 0)
	{
		g_cObstructionCode |= 128;
	}
	g_cMCObstructionEvent = 0; //clear MC obstruction events now that they have been recieved and merged here

	g_cObstructionCode &= g_ConfigData.cObstructionCodeMask;

	if (g_cObstructionCode != g_cLastObstructionCode)
	{
		cv = g_cObstructionCode ^ g_cLastObstructionCode; //cv has a 1 for any flag that changed
#ifdef OUTPUT_OBSTRUCTION_CODE
				logf("oc=%x\r\n", g_cObstructionCode);
#endif
		cv = cv & g_cObstructionCode; //cv now has a 1 for any flag that is changed, and is now set
		//next set g_cObstructionCodeNew with 1 for any new flags but retain previously new flags not handled.
		g_cObstructionCodeNew = g_cObstructionCodeNew | cv;

		g_cLastObstructionCode = g_cObstructionCode; //update the last check
		g_cObstructionEvent = 1; //mark an event to process
	}

	//With only one source of warnings so far, the warning code is much simpler
	g_cObstructionWarningCode = 0;
#ifdef OBSTRUCTION_SYSTEM_MOW
	if (g_cDigInObstructionMOW == MO_OBSTRUCTION)
	{
		g_cObstructionWarningCode = 4;
	}
#endif
	if (g_cObstructionWarningCode != g_cLastObstructionWarningCode)
	{
#ifdef OUTPUT_OBSTRUCTION_CODE
		logf("owc=%x\r\n", g_cObstructionWarningCode);
#endif
		g_cLastObstructionWarningCode = g_cObstructionWarningCode;
		g_cObstructionWarningEvent = 1;
	}

#ifdef PRESSURESENSOR
	cv=digIn(DIGIN_CHNUM_AIR_PRESSURE);
	if (cv!=g_cDigInAirPressure)
	{
		//disagreement ... less common path
		g_cDigInAirPressureXCount++;
		if (g_cDigInAirPressureXCount>=DIGIN_DEBOUNCE_HISTORY_COUNT)
		{
			//now reach the count... flip
			g_cDigInAirPressure=cv;
			g_cDigInAirPressureXCount=0;
			//Set Event Flag also
			g_cAirPressureEvent=cv;
#ifdef OUTPUT_AIRPRESSURE_DIO
			logf("Air Pressure Sensor Now %d\r\n",(int)cv);
#endif
		}
	}
	else
	{
		//agreement
		g_cDigInAirPressureXCount=0;
	}
#endif

#ifdef ORIENTATION_SENSORS
	g_cDigInYOrientationA = digIn(DIGIN_CHNUM_YORIENTATION_A); //no count
	g_cDigInYOrientationB = digIn(DIGIN_CHNUM_YORIENTATION_B);//no count
#endif
#ifdef DIGIN_SOFTSTOP
	cv=digIn(DIGIN_SOFTSTOP);
	if (cv!=g_cDigInSoftStop)
	{
		g_cDigInSoftStop=cv;
		if (cv==0)
		{
			//change to 0 is a stop event
			StopProcess();
		}
	}
#endif
}


void ReadADC(void)
{
	static uint32 s_uiLastSampleTime = 0;
	uint32 uiTime;
	uint32 uiSampleInterval;
	unsigned int ui;
	byte c;
	byte cmo;
	//float f;
#ifdef GENHDACCELEROMETERS
	static float fmin=100;
	static float fmax=-100;
	float fa;
	float fb;
	float fc;
	static float famin=100;
	static float famax=-100;
	static float fbmin=100;
	static float fbmax=-100;
	static float fcmin=100;
	static float fcmax=-100;
#endif
#ifdef OBSTRUCTION_SYSTEM_MO_ANA
	static byte s_cLastMoCheck;
	static byte s_cLastMoPrint;
#endif

#ifdef GENCIRCMFTL26
	uiTime = MS_TIMER;
	uiSampleInterval = uiTime - s_uiLastSampleTime;
	if (uiSampleInterval >= 3)
	{
		//There read all the time at an interval, but they
		//could be modified to read only when clamping and using them.
		//FIXME PORTLOW consider that as a time saving improvement
		ReadLoadCell(2);
		CalcAngleSensor();
		s_uiLastSampleTime = uiTime;
	}
#endif

#ifdef GENHDACCELEROMETERS
	//READ ACCELEROMETERS
	fa=anaInVolts(ADC_ACC_A)-2.5;
	fb=anaInVolts(ADC_ACC_B)-2.5;
	fc=anaInVolts(ADC_ACC_C)-2.5;
	g_fAccelValue=fa;
	if (fabs(g_fAccelValue)<fabs(fb))
	{
		g_fAccelValue=fb;
	}
	if (fabs(g_fAccelValue)<fabs(fc))
	{
		g_fAccelValue=fc;
	}

	if (g_cAccelArm==0)
	{
		//not armed
		if (fmin<100)
		{
			//got data to show
			logf("Armed Win fmin=%f fmax=%f abc\r\n",fmin,fmax);
			logf("  %f to %f\r\n",famin,famax);
			logf("  %f to %f\r\n",fbmin,fbmax);
			logf("  %f to %f\r\n",fcmin,fcmax);
			//reset window vals
			fmin=100;
			fmax=-100;
			famin=100;
			famax=-100;
			fbmin=100;
			fbmax=-100;
			fcmin=100;
			fcmax=-100;
		}
	}
	else
	{
		//allow values to be stored for min and max
		if (g_fAccelValue>fmax)
		{	fmax=g_fAccelValue;}
		if (g_fAccelValue<fmin)
		{	fmin=g_fAccelValue;}
		if (fa>famax)
		{	famax=fa;}
		if (fa<famin)
		{	famin=fa;}
		if (fb>fbmax)
		{	fbmax=fb;}
		if (fb<fbmin)
		{	fbmin=fb;}
		if (fc>fcmax)
		{	fcmax=fc;}
		if (fc<fcmin)
		{	fcmin=fc;}
	}
#endif

	//FORCE Limits are read in another function

#ifdef OBSTRUCTION_SYSTEM_MO_ANA
	c = (byte)(MS_TIMER >> 2); //check every 4 ms
	if (c!=s_cLastMoCheck)
	{
		s_cLastMoCheck = c;
		//Check MoCap
		cmo=0;
#ifdef ADC_MO1
		ui=anaIn(ADC_MO1); if (ui<g_ConfigData.MOCal.uim1)
		{	cmo|=1;}
#endif
#ifdef ADC_MO2
		ui=anaIn(ADC_MO2); if (ui<g_ConfigData.MOCal.uim2)
		{	cmo|=2;}
#endif
#ifdef ADC_MO3
		ui=anaIn(ADC_MO3); if (ui<g_ConfigData.MOCal.uim3)
		{	cmo|=4;}
#endif
#ifdef ADC_MO4
		ui=anaIn(ADC_MO4); if (ui<g_ConfigData.MOCal.uim4)
		{	cmo|=8;}
#endif
#ifdef ADC_MO5
		ui=anaIn(ADC_MO5); if (ui<g_ConfigData.MOCal.uim5)
		{	cmo|=16;}
#endif
#ifdef ADC_MO6
		ui=anaIn(ADC_MO6); if (ui<g_ConfigData.MOCal.uim6)
		{	cmo|=32;}
#endif
		g_cMOFlags = cmo;

		//Check Mo Print
		c = (byte)(MS_TIMER >> 10);//print every second approx
		if (c!=s_cLastMoPrint)
		{
			s_cLastMoPrint = c;
			//Check MoCap
			logf("Checking MO %u\r\n",MS_TIMER);
			logf("%.3f %u  \t%.3f %u  \t%.3f %u\r\n",anaInVolts(ADC_MO1), anaIn(ADC_MO1), anaInVolts(ADC_MO2), anaIn(ADC_MO2), anaInVolts(ADC_MO3), anaIn(ADC_MO3) );
			logf("%.3f %u  \t%.3f %u  \t%.3f %u\r\n",anaInVolts(ADC_MO4), anaIn(ADC_MO4), anaInVolts(ADC_MO5), anaIn(ADC_MO5), anaInVolts(ADC_MO6), anaIn(ADC_MO6) );
			if (cmo>0)
			{
				logf("cmo=%d\r\n",cmo);
			}
		}

	}

#endif
}

#ifdef FORCELIMITING

unsigned int g_uiReadForceSensorTime; //use int since exact granularity doesn't matter
byte g_cShowForce;

void ReadForceInitMem()
{
	g_ForceSensor.cErrFlag = 1; //don't use it before it has been read...
	g_uiReadForceSensorTime = 0;
	g_cShowForce = 0;
}

//Read Force Sensor if it's time, and update force limits if executing a force limited move
void ReadForceSensor()
{
	unsigned int ui;
	ui = (unsigned int) MS_TIMER; //truncate
	if ((ui - g_uiReadForceSensorTime)
			> g_ConfigData.ForceLimits.uiSensorInterval)
	{
		//Time to Read Force Sensors
		ReadForceSensorNow();

		UpdateForceLimitingCurrents(); //only will do it if needed.
	}
}

void ReadForceSensorNow()
{
	unsigned int ui;
	int ix;
	int iy;
	int iz;
	float f;

	ui = (unsigned int) MS_TIMER; //truncate
	g_uiReadForceSensorTime = ui;

	ix = anaIn(ADC_FORCE_SENSOR_X);
	iy = anaIn(ADC_FORCE_SENSOR_Y);
	iz = anaIn(ADC_FORCE_SENSOR_Z);

	g_ForceSensor.fX =
			((float) (ix - g_ConfigData.ForceSensorCalibration.iZeroX))
					/ g_ConfigData.ForceSensorCalibration.iCountsPerGX;
	g_ForceSensor.fY =
			((float) (iy - g_ConfigData.ForceSensorCalibration.iZeroY))
					/ g_ConfigData.ForceSensorCalibration.iCountsPerGY;
	g_ForceSensor.fZ =
			((float) (iz - g_ConfigData.ForceSensorCalibration.iZeroZ))
					/ g_ConfigData.ForceSensorCalibration.iCountsPerGZ;
	g_ForceSensor.cErrFlag = 0;

	f = g_ForceSensor.fX * g_ForceSensor.fX
			+ g_ForceSensor.fY * g_ForceSensor.fY
			+ g_ForceSensor.fZ * g_ForceSensor.fZ;
	// not needed just compare to squared vales
	if (f < 0.7 || f > 1.3)
	{
		//likely invalid
		g_ForceSensor.cErrFlag = 1;
	}
	if (g_cShowForce == 1)
	{
		logf("fs %d %d %d %d\r\n", ix, iy, iz, g_ForceSensor.cErrFlag);
		logf("fs %f %f %f\r\n", g_ForceSensor.fX, g_ForceSensor.fY,
				g_ForceSensor.fZ);
	}
}

#endif
