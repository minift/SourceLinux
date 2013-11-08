// MiniFTIO.h

#ifndef MINIFTIO_H_
#define MINIFTIO_H_

#include "MiniFTDefs.h"
#include "hwio.h"

//See hwio.h for hardware layer io

//LOGICAL MAPPING

//GENHD3 IO
#ifdef GENHD3
//#define DO_LIGHT_GREEN 0
//#define DO_BEEPER_1 1
#define DO_BRAKE_RELEASE 2

// physical digital input channels (IN00 = 0 through IN23 = 23 for the BL2100)
#define DIGIN_CHNUM_ESTOP 0         // physical channel IN00...monitors motor power supply
#define DIGIN_CHNUM_UNIBUTTON 1         // physical channel IN01
#define DI_ESTOP 2        // physical channel IN02...monitors physical ESTOP switch on lid of MiniFT

//DIGIN_CHNUM_YHOME NOT USED currently
#define DIGIN_CHNUM_YHOME 8
#define DIGIN_CHNUM_YORIENTATION_A 6
#define DIGIN_CHNUM_YORIENTATION_B 7

#define DI_COM_FLAG_X 9        // physical channel IN09 Detect X Whistle Signal
#define DI_COM_FLAG_Y 10         // physical channel IN10 Detect Y Whistle Signal

//Obstruction Sensors
#define OBSTRUCTION_SYSTEM_XP1
#define OBSTRUCTION_SYSTEM_XN1
#define DI_OBSTRUCTION_XP1 17
#define DI_OBSTRUCTION_XN1 16

//analog
#define ADC_CHANNEL_ANALOG_POINT_LASER ADC_CHANNEL_00

#define IO_MAPPED
#endif

//GENHD4 IO
#ifdef GENHD4
//#define DO_LIGHT_GREEN 0
//#define DO_BEEPER_1 1
#define DO_BRAKE_RELEASE 2

// physical digital input channels (IN00 = 0 through IN23 = 23 for the BL2100)
//#define DIGIN_CHNUM_ESTOP 0
//#define DIGIN_CHNUM_UNIBUTTON 1
#define DI_ESTOP 2    //monitors logical ESTOP signal.

#define DIGIN_CHNUM_YLOCK 5
//#define DIGIN_CHNUM_YHOME 6 //goes to whistle
#define DIGIN_CHNUM_YORIENTATION_A 7
#define DIGIN_CHNUM_YORIENTATION_B 8

#define DI_COM_FLAG_X 9        // physical channel IN09 Detect X Whistle Signal
#define DI_COM_FLAG_Y 10         // physical channel IN10 Detect Y Whistle Signal

//NOT UNTIL TESTED
//#define DIGIN_SOFTSTOP 11

//Actually not mapped
#define DIGIN_FASTENERSENSOR 15

//Obstruction Sensors
#define OBSTRUCTION_SYSTEM_XP1
#define OBSTRUCTION_SYSTEM_XN1
#define DI_OBSTRUCTION_XP1 16
#define DI_OBSTRUCTION_XN1 17

//analog
//#define ADC_CHANNEL_ANALOG_POINT_LASER ADC_CHANNEL_00

#ifdef FORCELIMITING
#define ADC_FORCE_SENSOR_X ADC_CHANNEL_01
#define ADC_FORCE_SENSOR_Y ADC_CHANNEL_02
#define ADC_FORCE_SENSOR_Z ADC_CHANNEL_03
#endif

#define IO_MAPPED
#endif

//GENHD5 IO
#ifdef GENHD5
//#define DO_LIGHT_GREEN 0
//#define DO_BEEPER_1 1
#define DO_BRAKE_RELEASE 2
#define DIGOUT_CHNUM_BRAKE_ALTERNATE 3

// physical digital input channels (IN00 = 0 through IN23 = 23 for the BL2100)
//#define DIGIN_CHNUM_ESTOP 0
//#define DIGIN_CHNUM_UNIBUTTON 1
#define DI_ESTOP 2    //monitors logical ESTOP signal.

#define DIGIN_CHNUM_XLOCK 4
#define DIGIN_CHNUM_YLOCK 5

#define DIGIN_CHNUM_YORIENTATION_A 7
#define DIGIN_CHNUM_YORIENTATION_B 8

#define DI_COM_FLAG_X 9        // physical channel IN09 Detect X Whistle Signal
#define DI_COM_FLAG_Y 10         // physical channel IN10 Detect Y Whistle Signal

#define DIGIN_RFID_PRESENT 11

//Actually not mapped
//#define DIGIN_FASTENERSENSOR 15

//Obstruction Sensors
#define OBSTRUCTION_SYSTEM_XP1
#define OBSTRUCTION_SYSTEM_XN1
#define DI_OBSTRUCTION_XP1 16
#define DI_OBSTRUCTION_XN1 17

//analog
//#define ADC_CHANNEL_ANALOG_POINT_LASER ADC_CHANNEL_00

#ifdef FORCELIMITING
#error "HD5 does not support this at this time"
#define ADC_FORCE_SENSOR_X ADC_CHANNEL_01
#define ADC_FORCE_SENSOR_Y ADC_CHANNEL_02
#define ADC_FORCE_SENSOR_Z ADC_CHANNEL_03
#endif

#define IO_MAPPED
#endif

//GENFD IO
#ifdef GENFD
//#define DO_LIGHT_GREEN 0
//#define DO_BEEPER_1 1
#define DO_BRAKE_RELEASE 2
// physical digital input channels (IN00 = 0 through IN23 = 23 for the BL2100)
#define DI_ESTOP 2        // physical channel IN02...monitors physical ESTOP switch on lid of MiniFT

#define DI_COM_FLAG_X 9        // physical channel IN09 Detect X Whistle Signal
#define DI_COM_FLAG_Y 10         // physical channel IN10 Detect Y Whistle Signal

#endif

//GENFLOORBEAM IO
#ifdef GENFLOORBEAM
//#define DO_LIGHT_GREEN 0
//#define DO_BEEPER_1 1
#define DO_BRAKE_RELEASE 2

//New in Floorbeam
//Also known as ChipClearXY
#define DIGOUT_CHNUM_AIRBLASTX 6
#define DIGOUT_CHNUM_AIRBLASTY 7
//Laser on 10
#define DIGOUT_CHNUM_LASER_POINTER 10

// physical digital input channels (IN00 = 0 through IN23 = 23 for the BL2100)
//#define DIGIN_CHNUM_ESTOP 0
//#define DIGIN_CHNUM_UNIBUTTON 1
#define DI_ESTOP 2    //monitors logical ESTOP signal.

//#define DIGIN_CHNUM_YLOCK 5
//#define DIGIN_CHNUM_YHOME 6 //goes to whistle

#define DI_COM_FLAG_X 9        // physical channel IN09 Detect X Whistle Signal
#define DI_COM_FLAG_Y 10         // physical channel IN10 Detect Y Whistle Signal

#define DIGIN_RFID_TAG_DETECT 11

//Obstruction Sensors
#define OBSTRUCTION_SYSTEM_XP1
#define OBSTRUCTION_SYSTEM_XN1
#define DI_OBSTRUCTION_XP1 16
#define DI_OBSTRUCTION_XN1 17

//analog
//#define ADC_CHANNEL_ANALOG_POINT_LASER ADC_CHANNEL_00

#ifdef FORCELIMITING
#define ADC_FORCE_SENSOR_X ADC_CHANNEL_01
#define ADC_FORCE_SENSOR_Y ADC_CHANNEL_02
#define ADC_FORCE_SENSOR_Z ADC_CHANNEL_03
#endif

#define IO_MAPPED
#endif


//GENCIRCMFT1 IO
#ifdef GENCIRCMFT1
//On a RivetMotherboard1
#define DO_LIGHT_GREEN 0
#define DO_BEEPER_1 1
#define DO_BRAKE_RELEASE 2
#define DIGOUT_CHNUM_DRILLUP 3
#define DO_DRILL_DOWN 4
#define DO_START_DRILL 5
#define DO_COLLET_EXPAND 6
// physical digital input channels (IN00 = 0 through IN23 = 23 for the BL2100)
#define DIGIN_CHNUM_ESTOP 0         // physical channel IN00...monitors motor power supply
#define DIGIN_CHNUM_UNIBUTTON 1         // physical channel IN01
#define DI_ESTOP 2        // physical channel IN02...monitors physical ESTOP switch on lid of MiniFT
#define DI_DRILL_UP	5
//#define DIGIN_CHNUM_Y_HOME 6 //should be rewired to whistle
#warnt "SEVERE  Rewire to whistle"
#define DIGIN_CHNUM_Y_POS_LIMIT 7
#define DIGIN_CHNUM_Y_NEG_LIMIT 8

#define DI_COM_FLAG_X 9         // Detect X Whistle Signal
#define DI_COM_FLAG_Y 10         // Detect Y Whistle Signal

#define DIGIN_CHNUM_AIR_PRESSURE 11         // physical channel IN11
#define IO_MAPPED
#endif

//GENCIRCMFT2 IO
#ifdef GENCIRCMFT2
//BL2100 Gen4 Style Design
#define DO_BEEPER_1 0
#define DO_LIGHT_GREEN 1
#define DO_BRAKE_RELEASE 2
//DIGOUT_CHNUM_DRILLUP DIGOUT_CHNUM_DRILLUP
//remove UP  and make this IO Control Logic...
//#define DIGOUT_CHNUM_DRILLUP 3
#define DO_DRILL_DOWN 4
#define DO_START_DRILL 5
#define DO_COLLET_EXPAND 6
// physical digital input channels (IN00 = 0 through IN23 = 23 for the BL2100)
//#define DIGIN_CHNUM_ESTOP 0
//#define DIGIN_CHNUM_UNIBUTTON 1
#define DI_ESTOP 2    //monitors logical ESTOP signal.
//REMOVED #define DIGIN_CHNUM_CUTTER_DETECTED 4
#define DI_DRILL_UP	6
//REMOVED #define DIGIN_CHNUM_DRILLDOWN 5

#define DI_COM_FLAG_X 9         // Detect X Whistle Signal
#define DI_COM_FLAG_Y 10         // Detect Y Whistle Signal

//#define DIGIN_CHNUM_AIR_PRESSURE 11         // physical channel IN11

#define OBSTRUCTION_SYSTEM_XP1
#define OBSTRUCTION_SYSTEM_XN1
#define DI_OBSTRUCTION_XP1 16
#define DI_OBSTRUCTION_XN1 17

#define OBSTRUCTION_SYSTEM_MOW
#define OBSTRUCTION_SYSTEM_MOS
#ifdef OBSTRUCTION_SYSTEM_MOW
#define DIGIN_OBSTRUCTION_MOW 18
#endif
#ifdef OBSTRUCTION_SYSTEM_MOS
#define DIGIN_OBSTRUCTION_MOS 19
#endif

//FOR FUTURE
//#define DIGIN_OBSTRUCTION_XP2 20
//#define DIGIN_OBSTRUCTION_XN2 21

//analog
#define ADC_CHANNEL_ANALOG_POINT_LASER ADC_CHANNEL_00

#define FORCELIMITING
#ifdef FORCELIMITING
#define ADC_FORCE_SENSOR_X ADC_CHANNEL_01
#define ADC_FORCE_SENSOR_Y ADC_CHANNEL_02
#define ADC_FORCE_SENSOR_Z ADC_CHANNEL_03
#endif

#define ADC_CHANNEL_DRILL_AIR_FLOW ADC_CHANNEL_04

//#define OBSTRUCTION_SYSTEM_MO_ANA
#ifdef OBSTRUCTION_SYSTEM_MO_ANA
#define ADC_MO1 ADC_CHANNEL_04
#define ADC_MO2 ADC_CHANNEL_05
#define ADC_MO3 ADC_CHANNEL_06
#define ADC_MO4 ADC_CHANNEL_07
#define ADC_MO5 ADC_CHANNEL_08
#define ADC_MO6 ADC_CHANNEL_09
#endif

#define IO_MAPPED
#endif

//GENCIRCMFTL26 IO
#ifdef GENCIRCMFTL26

//FIXME PORTLOW   for now GENCIRCMFTL26 IO is defined in MiniFTIO.h
// but none of the other tools have been moved.
//Clean this up and find the best pattern for the new system.

//Digital INPUTS	Shows as 1 - 7
//JLIME
#define DI_V_GROOVE_S1		DI_BBB_1
#define DI_DRILL_UP			DI_BBB_2
#define DI_ESTOP			DI_BBB_3   // Monitors logical ESTOP signal.
#define DI_COM_FLAG_X		DI_BBB_4   // Detect X Whistle Signal
#define DI_COM_FLAG_Y		DI_BBB_5   // Detect Y Whistle Signal
#define DI_OBSTRUCTION_XP1	DI_BBB_6
#define DI_OBSTRUCTION_XN1	DI_BBB_7

//Digital OUTPUTS	Shows as 1 - 8

//#define DO_COLLET_BUTTON	DO_BBB_1
//#define DO_START_DRILL	DO_BBB_2
#define DO_BRAKE_RELEASE	DO_BBB_3
#define DO_LIGHT_GREEN		DO_BBB_4
#define DO_BEEPER_1			DO_BBB_5
#define DO_LIGHT_RED		DO_BBB_6
#define DO_BEEPER_2			DO_BBB_7
#define DO_LASER_POINTER	DO_BBB_8

//	Shows as 64 - 69
//FIXME PORTFATAL needs test
#define DO_DRILL_AIR		DO_EXT1_1
#define DO_V_GROOVE_ROLLERS	DO_EXT1_2
#define DO_DRILL_DOWN		DO_EXT1_3
//NOTE: 1st one LEFT TO RIGHT
#define DO_LUBE				DO_EXT1_4
//NOTE: 1st one LEFT TO RIGHT
#define DO_LUBE_MOD			DO_EXT1_5
#define DO_Y_CLUTCH			DO_EXT1_6
#define DO_COLLET_BUTTON	DO_EXT1_7
#define DO_START_DRILL		DO_EXT1_8



//Input and output value defines:
//DI_DRILL_UP
//FIXME PORTFATAL check polarity.... (code done)
#define DRILL_UP 	1
#define DRILL_NOT_UP	0

//DI_V_GROOVE_S1
//FIXME PORTFATAL meaning of sensor...  write code... etc...
#define V_GROOVE_S1_LOCK	0
#define V_GROOVE_S1_UNLOCK	1

//DO_COLLET_BUTTON
//Pressing the button moves the collet down and released the collet.
//Letting go of the button brings it up, but locks it in the bushing.
//Because of this, we renamed the states COLLET_LOCK and COLLET_RELEASE
//The locking also has a function of pulling the bushing down and creating force, so the LOCK is
//a clamping function the drill does.   This happens then the button is NOT pressed.
//The button, does the collet release,
#define COLLET_RELEASE 	1
#define COLLET_LOCK 	0

//DO_START_DRILL
#define START_DRILL_ON 		1
#define START_DRILL_OFF 	0

//DO_BRAKE_RELEASE
#define BRAKE_RELEASE_ON	1
#define BRAKE_RELEASE_OFF	0

//DO_LIGHT_GREEN
//FIXME PORTFATAL no light system using current lights with right logic
#define LIGHT_GREEN_ON	1
#define LIGHT_GREEN_OFF	0

//DO_BEEPER_1
//FIXME PORTFATAL no beeper system using current beepers with right logic
#define BEEPER_1_ON		1
#define BEEPER_1_OFF	0

//DO_LIGHT_RED
//FIXME PORTFATAL no light system using current lights with right logic
#define LIGHT_RED_ON	1
#define LIGHT_RED_OFF	0

//DO_BEEPER_2
//FIXME PORTFATAL no beeper system using current beepers with right logic
#define BEEPER_ON		1
#define BEEPER_OFF		0

//DO_LASER_POINTER - completed no code to modify
//FIXME PORTFATAL  link this to existing laser pointer code...
#define LASER_POINTER_ON	1
#define LASER_POINTER_OFF	0

//DO_DRILL_AIR -completed no code to modify
//FIXME PORTFATAL polarity check and logic needed in drill cycle...
#define DRILL_AIR_ON	0
#define DRILL_AIR_OFF	1


//DO_V_GROOVE_ROLLERS - not ready
//FIXME PORTFATAL logic needed.
#define V_GROOVE_ROLLERS_ON 	1
#define V_GROOVE_ROLLERS_OFF	0

//DO_DRILL_DOWN
#define DRILL_DOWN_UP	1
#define DRILL_DOWN	0

//JLIME didn't DO_LUBE or DO_LUBE_MOD
//FIXME PORTFATAL  check polarity   check Tom's code  add to drill cycle
#define DO_LUBE_ON
#define DO_LUBE_OFF

//FIXME PORTFATAL  check polarity   check Tom's code  add to drill cycle
#define DO_LUBE_MOD_ON
#define DO_LUBE_MOD_OFF

//DO_Y_CLUTCH -not ready
//FIXME PORTFATAL  add code to do A axis adjust
#define Y_CLUTCH_ON 1
#define Y_CLUTCH_OFF 0












//Analog Inputs

#define AIN_DRILL_NOISE	AIN_BBB_1
#define AIN_RESERVED1	AIN_BBB_2
#define AIN_DVRT_A		AIN_BBB_3
#define AIN_DVRT_B		AIN_BBB_4
#define AIN_LOAD_CELL	AIN_BBB_5

#ifdef FORCELIMITING
//FIXME PORTHIGH  when this gets added back in , must set this to the new analog values and check code
#define ADC_FORCE_SENSOR_X ADC_CHANNEL_01
#define ADC_FORCE_SENSOR_Y ADC_CHANNEL_02
#define ADC_FORCE_SENSOR_Z ADC_CHANNEL_03
#endif

#define IO_MAPPED
#endif

//A Listing for each model must appear above and define this after it's last IO defs
#ifndef IO_MAPPED
#error "IO_MAPPED was not defined"
#endif


#define DIGIN_DEBOUNCE_HISTORY_COUNT 8
#define DIGIN_XLOCK_DEBOUNCE_HISTORY_COUNT 32
#define DIGIN_IMMEDIATE_COUNT 1


// Analog Inputs

#ifdef CENTERVISION_ANALOG_POINT_LASER
#ifndef ADC_CHANNEL_ANALOG_POINT_LASER
#define ADC_CHANNEL_ANALOG_POINT_LASER ADC_CHANNEL_02
#ifdef ADC_CHANNEL_02_USED
#error "ERROR: You have specified incompatible uses for ADC CHANNEL 2."
#endif
#define ADC_CHANNEL_02_USED
#endif
#endif


//For DrillFill
#ifdef GENHDACCELEROMETERS
#define ADC_ACC_A ADC_CHANNEL_03
#define ADC_ACC_B ADC_CHANNEL_04
#define ADC_ACC_C ADC_CHANNEL_05
#endif

// Analog Outputs
#define DAC_CHANNEL_00			0


//DIO

//MiniFt Core DIO
extern byte g_cLED;
#ifdef BEEPSYSTEM
extern byte g_cBeep;
#endif

extern byte g_cLEDCount;
extern uint16 g_uiLEDOn;
extern uint16 g_uiLEDWhole;
extern uint32 g_ulLEDStart;
#ifdef BEEPSYSTEM
extern byte g_cBeepMode;
extern byte g_cBeepCount;
extern uint16 g_uiBeepOn;
extern uint16 g_uiBeepWhole;
extern uint32 g_ulBeepStart;
#endif

//HD
#ifdef CLAMP_SYSTEM_HD_PISTON
extern byte g_cALock;
extern byte g_cLegsLock;
extern byte g_cLegsDown;
extern byte g_cLegsUp;
extern byte g_cClampExtend;
extern byte g_cClampRetract;
#endif
byte g_cAirBlastX;
byte g_cAirBlastY;

#ifdef SEAL_SYSTEM
extern byte g_cSealantApply;
extern byte g_cSealantPressure;
extern byte g_cSealantPinch;
#endif

#ifdef GENCIRCMFTX
//CIRCMFT
#ifdef DO_DRILL_UP
extern byte g_cDrillUp;
#endif
extern byte g_cDrillDown;
extern byte g_cDrillButton;
extern byte g_cColletClamp;
#endif

extern byte g_cLaserPointer;

#ifdef UNIBUTTON
extern byte g_cDigInUniButtonXCount;
extern byte g_cDigInUniButton;
#endif
extern byte g_cDigInEstopSignalXCount;
extern byte g_cDigInEstopSignal;

extern byte g_cMachineLock;
#ifdef DIGIN_CHNUM_XLOCK
extern byte g_cDigInXLock;
extern byte g_cDigInXLockXCount;
#endif
#ifdef DIGIN_CHNUM_YLOCK
extern byte g_cDigInYLock;
extern byte g_cDigInYLockXCount;
#endif

#ifdef GENCIRCMFTX
extern byte g_cDigInDrillUp; //no count
extern byte g_cDigInDrillDown; //no count
#endif

#ifdef Y_LIMIT_SENSORS
extern byte g_cDigInYPosLimit; //no count
extern byte g_cDigInYNegLimit;//no count
#endif

#ifdef PRESSURESENSOR
extern byte g_cDigInAirPressure;
extern byte g_cDigInAirPressureXCount;
#endif

#ifdef ORIENTATION_SENSORS
extern byte g_cDigInYOrientationA; //no count
extern byte g_cDigInYOrientationB;//no count
#endif
byte g_cPrevOrSensors;

//Obstruction System
extern byte g_cObstructionCode;
extern byte g_cLastObstructionCode;
extern byte g_cObstructionCodeNew;

extern byte g_cObstructionWarningCode;
extern byte g_cLastObstructionWarningCode;

#ifdef OBSTRUCTION_SYSTEM_XP1
extern byte g_cDigInObstructionXP1;
#endif
#ifdef OBSTRUCTION_SYSTEM_XN1
extern byte g_cDigInObstructionXN1;
#endif
#ifdef OBSTRUCTION_SYSTEM_MOW
extern byte g_cDigInObstructionMOW;
#endif
#ifdef OBSTRUCTION_SYSTEM_MOS
extern byte g_cDigInObstructionMOS;
#endif
#ifdef OBSTRUCTION_SYSTEM_MO_ANA
extern byte g_cMOFlags;
#endif
#ifdef DIGIN_SOFTSTOP
extern byte g_cDigInSoftStop;
#endif

#ifdef UNIBUTTON
extern byte g_cUniButtonEvent;
#endif

//Use one lock event for all lock changes
extern byte g_cLockEvent;
#ifdef PRESSURESENSOR
extern byte g_cAirPressureEvent;
#endif
//Obstruction System
extern byte g_cObstructionEvent;
//Obstruction Flag Event
extern byte g_cMCObstructionEvent;
//Obstruction Warning Event
extern byte g_cObstructionWarningEvent;

extern byte g_cBrakeReleased;

void MiniFTIOInit();

void InitDIO(void);
void SetDefaultValveState();
void InitADC(void);

void BrakeOn();
void BrakeOff();
void DigOutService(void);

void ReadDigitalInputs(void);

void ReadADC(void);
#ifdef FORCELIMITING
void ReadForceInitMem();
void ReadForceSensor();
void ReadForceSensorNow();
#endif


//LED Output Macros
#ifdef LEDSYSTEM
#define LEDOn() g_cLED = 1; g_cLEDCount = 0;
#define LEDOff() g_cLED = 0; g_cLEDCount = 0;
#define LEDCount(c,p,w) g_cLEDCount=(c); g_uiLEDOn=(p); g_uiLEDWhole=(w); g_ulLEDStart=MS_TIMER;
#define LEDProbeK() LEDCount(255,200,500)
#else
#define LEDOn()
#define LEDOff()
#define LEDCount(c,p,w)
#define LEDProbeK()
#endif

#ifdef BEEPSYSTEM
void Beep();
#endif

//beep system
#ifdef BEEPSYSTEM
//BEEP Output Macros
//#define BeepOn() g_cBeep = 1; g_cBeepCount = 0; //We Never really just turn the beeper on
#define BeepOff() g_cBeepMode=BEEPOFF; g_cBeep = 0; g_cBeepCount = 0;
#define BeepCountPrivate(m,c,p,w) g_cBeepMode=(m); g_cBeepCount=(c); g_uiBeepOn=(p); g_uiBeepWhole=(w); g_ulBeepStart=MS_TIMER;

//#define BeepPattern(

//Ongoing Modes
#define BEEPOFF 0
#define BEEPPROBEK 3
#define BEEPPROBEHOME 5
#define BEEPPROBEADJUST 6
#define BEEPMOVE 7
//Short Modes - not interupted by modal beep patterns
#define BEEPSIGNALS 10
#define BEEPFLOAT 11
#define BEEPJOG 12
#define BEEPBOOT 13
#define BEEPONCE 14
#define BEEPMCERR 15
#define BEEPMCERR2 16

#define BeepBoot() BeepCountPrivate(BEEPBOOT, 3, 250, 400)
#define BeepMCERR() BeepCountPrivate(BEEPMCERR,20,40,100)
#define BeepMCERR2() BeepCountPrivate(BEEPMCERR2,15,50,100)
#define BeepFloat() BeepCountPrivate(BEEPFLOAT,255,50,2000)
#define BeepJog() BeepCountPrivate(BEEPJOG,255,50,600)
#define BeepProbeK() BeepCountPrivate(BEEPPROBEK,255,20,500)
#define BeepProbeHome() BeepCountPrivate(BEEPPROBEHOME,255,20,1000)
#define BeepProbeAdjust() BeepCountPrivate(BEEPPROBEADJUST,255,20,500)
#define BeepMove() BeepCountPrivate(BEEPMOVE,255,10,300)
#define BeepAlternate() BeepCountPrivate(BEEPONCE,3, 300, 600)
#define BeepBootFatal() BeepCountPrivate(BEEPBOOT, 255, 30, 80)
#else
//No Beep System
#define Beep()
#define BeepOff()
#define BeepCountPrivate(m,c,p,w)

#define BeepBoot()
#define BeepMCERR()
#define BeepMCERR2()
#define BeepFloat()
#define BeepJog()
#define BeepProbeK()
#define BeepProbeHome()
#define BeepProbeAdjust()
#define BeepMove()
#define BeepAlternate()
#endif

#endif /* MINIFTIO_H_ */
