// MiniFTDefs.h

#ifndef MINIFTDEFS_H_
#define MINIFTDEFS_H_

#include "SmartTool.h"

//Configuration Options

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
//#define GENHD5
//#define GENFD
//#define GENFLOORBEAM
//#define GENCIRCMFT2
#define GENCIRCMFTL26

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
#ifdef GENCIRCMFTL26
#define SMARTTOOL_SUBTYPE_STRING "MiniFT-Circ2.6"
#endif

#ifndef SMARTTOOL_SUBTYPE_STRING
#error "ERROR: Build generations or SMARTTOOL_SUBTYPE_STRING not defined"
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


//For Building Laser Vision and Bypassing CAM vision on HDGEN3
#define GENHD3WITHLASER


//Define Common Include Codes
//Defaults defined prior to Complication Options controlled by machine generation

//All Machines Except those undefining and defining an alternate use ELMO WHISTLE MC
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
#define DELAY_AFTER_ESTOP_DEFAULT 1500
#define USE_FINDWINDOW
#endif

#ifdef GENCIRCMFT1
#define GENCIRCMFTX
#define CENTERVISION
#define CENTERVISION_ANALOG_POINT_LASER
//#define USE_HYSTERESIS_FROM_CENTERVISION
#define DELAY_AFTER_ESTOP_DEFAULT 500
#define LEDSYSTEM
#define BEEPSYSTEM
#endif

#ifdef GENCIRCMFT2
#define GENCIRCMFTX
#define CLAMP_SYSTEM
#define CENTERVISION
#define CENTERVISION_ANALOG_POINT_LASER
//#define USE_HYSTERESIS_FROM_CENTERVISION
#define USE_RFID
#define USE_RFID_OMRON
#define DELAY_AFTER_ESTOP_DEFAULT 500
#define LEDSYSTEM
#define BEEPSYSTEM
//#define GENCIRCMFTX_RUN_XCM
#endif

#ifdef GENCIRCMFTL26
#define GENCIRCMFTX
#define CLAMP_SYSTEM
#define CENTERVISION
#define CENTERVISION_CAM
#define CAM_ORIENATION_CXMXN
#define USE_RFID
#define USE_RFID_MIFARE
#define USE_FINDWINDOW
#define DELAY_AFTER_ESTOP_DEFAULT 500
#define LEDSYSTEM
#define BEEPSYSTEM
#define OBSTRUCTION_SYSTEM_XP1
#define OBSTRUCTION_SYSTEM_XN1
//FIXME PORTFATAL need to add this back in when ready
//#define FORCELIMITING
#endif


//More Build Options
//The Estop Signal Variable Signal that matches ESTOP
#define ESTOP_SIGNALED	0

//Turn this on for Everything
#define AUTO_MOVE_FIRST_POSITION

#ifdef GENCIRCMFTX
#define CLAMP_LOOSE_OR_UNCLAMP CLAMP_UNCLAMP
#endif

#ifdef CLAMP_SYSTEM
#ifndef CLAMP_LOOSE_OR_UNCLAMP
#define CLAMP_LOOSE_OR_UNCLAMP CLAMP_LOOSE
#endif
#endif

#ifdef GENCIRCMFTX
#undef DRILL_DIRECT_READY
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
#ifdef GENCIRCMFTL26
//FIXME PORTFATAL  Add EEOPTION FOR LFT26
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

//Don't leave estop until power supply to drive is stable
#define DELAY_AFTER_ESTOP_DEFAULT 500

//MC Asserted Estop
#define MC_ASSERTED_ESTOP

//Jog Enable System
#define JOG_ENABLE_TIME

//The rotation system is often marked with the
// MACHINEPOINTS heading

// for development mode (using debugger and using logfs), set DEVMODE to 1
// for stand-alone release, set DEVMODE to 0
#define DEVMODE 1

#if DEVMODE == 1
	#define USE_OUTPUT
	//Set thee following to control various groups of output.
	#define OUTPUT_ENET
	//#define OUTPUT_SESSION_EVENTS

	//#define OUTPUT_JOG
	#define OUTPUT_BRAKE
	#define OUTPUT_OBSTRUCTION_CODE
	#define OUTPUT_ESTOP
	//#define OUTPUT_PP_SYS
//    #define OUTPUT_PROGRAM_MEM_USAGE
	#define OUTPUT_PROBE_SYS
//Hoping to solve a specific issue with this.......
	#define OUTPUT_PROBE_SYS_VERBOSE
	//#define OUTPUT_TOOL_FLIP
	//#define OUTPUT_OFFSETS
	//#define OUTPUT_TOOL_OFFSET
    //#define OUTPUT_POSITION_INFO_CREATION
//	#define OUTPUT_POSITION_INFO 1
    //#define OUTPUT_POSITION_COMMAND_INFO 1
   	//#define OUTPUT_POSITION_INFO_MACHINE_TO_DATASET 1
   	//#define OUTPUT_POSITION_INFO_MACHINE_TO_DATASET_VERBOSE 1
    //#define OUTPUT_NEAREST_INFO
    #define OUTPUT_POSITION_STATES 1
//	#define OUTPUT_PROCESS_AND_TOOL
	#define OUTPUT_OPERATIONS
    #define OUTPUT_HDCLAMP_STATES 1
//	#define OUTPUT_DIGIN_CHNUM_XLOCK
	#define OUTPUT_DIGIN_CHNUM_YLOCK
	//#define OUTPUT_AIRPRESSURE_DIO 1
	//#define OUTPUT_FLOAT_STATES 1
	//#define OUTPUT_PROBE_STATES 1
    //#define OUTPUT_FASTENER_PARAMETER_SEARCH
    //STP Lib
//ex    #define STP_OUTPUT
	//#define STP_OUTPUT_PARSED
	//#define STP_OUTPUT_PARSED_NULLOID
    //#define STP_OUTPUT_TX
	#define OUTPUT_MiniFtMC
//	#define OUTPUT_RXSTP 1
//	#define OUTPUT_RXSTP_FAS 1
//	#define OUTPUT_RXSTP_FT 1
	//#define OUTPUT_RXSTP_NAC 1
//	#define OUTPUT_RXSTP_SD 1
//	#define OUTPUT_RXSTP_DF 1
	//#define OUTPUT_SENDPPDATA
	//#define OUTPUT_SENDXYDATA
//	#define OUTPUT_OPHIST
	//#define OUTPUT_OPHIST_VERBOSE
#define OUTPUT_RFID_HOME
    //SocketConsole
	//#define SOCKETCONSOLE_OUTPUT

#else
	#undef USE_OUTPUT
	#undef OUTPUT_POSITION_INFO
	#undef OUTPUT_POSITION_STATES
#endif

#define SOCKETCONSOLE

//Without USE_LOGF defined, logf is really just printf
//you can use logf to write debug code out to the Socket Console
//With USE_LOGF defiend, you must implement logf.
#define USE_LOGF
//There is an advanced logf which uses even mroe custom printf code...
//#define USE_LOGF_ADVANCED

#ifndef USE_LOGF
//If not doing logf, then make it printf
#define logf printf
#endif


//More Timing Defines
#define POSITION_UPDATE_THROTTLE_DEFAULT 180
#define POSITION_SEND_THROTTLE_DEFAULT 150
#define POSITION_UPDATE_THROTTLE_RFID_SCAN 20

#define X_AXIS 1
#define Y_AXIS 2
#define X_AND_Y 3

// state defs for Probe Modes
#define PROBE_INIT 0
#define PROBE_WAIT 1
#define PROBE_MOVE 2
#define PROBE_MOVE_WAIT 3
#define PROBE_PROBE 4
#define PROBE_WAITCOORDS 5
#define PROBE_REAPPLY 6
#define PROBE_CREATE_ABSOLUTE_PROBE 7
#define PROBE_COMPLETE 8
#define PROBE_WAIT_CALC 9

//Homing And Limits
#ifdef GENFD
#define HOMESYSTEM
#define HOMESYSTEM_X
#define HOMESYSTEM_Y
#define HOMESYSTEM_X_MC
#define HOMESYSTEM_Y_MC
#define HOMESYSTEM_X_HPRM -20
#define HOMESYSTEM_Y_HPRM -20
#define HOMESYSTEM_X_HIB "hib=1;\r"
#define HOMESYSTEM_X_HIC "hic=11;\r"
#define HOMESYSTEM_Y_HIB "hib=1;\r"
#define HOMESYSTEM_Y_HIC "hic=11;\r"
#define HOMESYSTEM_X_LINK	//X is part of overall home
#define HOMESYSTEM_Y_LINK	//Y is part of overall home
#define HOMESYSTEM_DRILL
#define CHECK_X_MOVE_OUT_OF_RANGE
#define CHECK_Y_MOVE_OUT_OF_RANGE
//#define HOMESYSTEM_AAXIS
//#define HOMESYSTEM_CLAMP
#endif
#ifdef GENHD3
#define HOMESYSTEM
#define HOMESYSTEM_DRILL
//#define HOMESYSTEM_AAXIS
//#define HOMESYSTEM_CLAMP
//No Longer Using Any NAC with Gen 3...
#endif

#ifdef GENHD4
#define HOMESYSTEM
#define HOMESYSTEM_DRILL
#define HOMESYSTEM_AAXIS
#define HOMESYSTEM_CLAMP
#define HOMESYSTEM_X
//#define HOMESYSTEM_X_MC //NO... NOT USING MC for Home
#define HOMESYSTEM_X_RFID //Use X_RFID system
//#define HOMESYSTEM_X_LINK	//X is not part of overall home
#define HOMESYSTEM_Y
#define HOMESYSTEM_Y_MC
#define HOMESYSTEM_Y_HPRM -20
#define HOMESYSTEM_Y_LINK	//Y is part of overall home
#define CHECK_X_MOVE_OUT_OF_RANGE
#define CHECK_Y_MOVE_OUT_OF_RANGE
#endif

#ifdef GENHD5
#define HOMESYSTEM
#define HOMESYSTEM_DRILL
#define HOMESYSTEM_AAXIS
#define HOMESYSTEM_CLAMP
#ifdef FASTENER_SYSTEM
#define HOMESYSTEM_FASTENER
#ifdef FASTENER_TRAY_STP
#define HOMESYSTEM_FASTENERTRAY
#endif
#endif
#define HOMESYSTEM_X
//#define HOMESYSTEM_X_MC //NO... NOT USING MC for Home
#define HOMESYSTEM_X_RFID //Use X_RFID system
//#define HOMESYSTEM_X_LINK	//X is not part of overall home
#define HOMESYSTEM_Y
#define HOMESYSTEM_Y_MC
#define HOMESYSTEM_Y_HPRM 20
#define HOMESYSTEM_Y_LINK	//Y is part of overall home
#define CHECK_X_MOVE_OUT_OF_RANGE
#define CHECK_Y_MOVE_OUT_OF_RANGE
//FIXME CHECK ON CHECK_Y_MOVE_OUT_OF_RANGE ......
#endif

#ifdef GENFLOORBEAM
#define HOMESYSTEM
#define HOMESYSTEM_DRILL
//#define HOMESYSTEM_AAXIS
#define HOMESYSTEM_CLAMP
#define HOMESYSTEM_X
//#define HOMESYSTEM_X_MC //NO... NOT USING MC for Home
#define HOMESYSTEM_X_RFID //Use X_RFID system
//#define HOMESYSTEM_X_LINK	//X is not part of overall home
#define HOMESYSTEM_Y
#define HOMESYSTEM_Y_MC
#define HOMESYSTEM_Y_HPRM -20
#define HOMESYSTEM_Y_LINK	//Y is part of overall home
#define CHECK_X_MOVE_OUT_OF_RANGE
#define CHECK_Y_MOVE_OUT_OF_RANGE
//FIXME
#endif
#ifdef GENCIRCMFT1
#define HOMESYSTEM
#define HOMESYSTEM_Y
#define HOMESYSTEM_Y_MC
#define HOMESYSTEM_Y_LINK	//Y is part of overall home
#error "Home System has been updated, but this model has not been compelted"
//Needs integration with new home system
#endif
#ifdef GENCIRCMFT1
//FIXME  notice that this old system is the only system using Y Limit code in X Carriage...  I should preserve what I want of this for the new systems
#define Y_LIMIT_SENSORS 1
#endif
#ifdef GENCIRCMFT2
#define HOMESYSTEM
#define HOMESYSTEM_CLAMP_UNCLAMP_ONLY
#define HOMESYSTEM_X
//#define HOMESYSTEM_X_MC //NO... NOT USING MC for Home
#define HOMESYSTEM_X_RFID //Use X_RFID system
//#define HOMESYSTEM_X_LINK	//X is not part of overall home

#define HOMESYSTEM_Y
#define HOMESYSTEM_Y_MC
#define HOMESYSTEM_Y_HPRM -30
//FIXME PORTFATAL test whistle homing????
#define HOMESYSTEM_Y_HIB "hib=2;\r"
#define HOMESYSTEM_Y_HIC "hic=13;\r"
#define HOMESYSTEM_Y_POS_PROC_SENSOR "ppi=3;\r"
//There is no Y NEG Sensor... Only Home
//#define HOMESYSTEM_Y_NEG_PROC_SENSOR "pni=4;\r"
#define HOMESYSTEM_Y_LINK	//Y is part of overall home
#define CHECK_X_MOVE_OUT_OF_RANGE
#define CHECK_Y_MOVE_OUT_OF_RANGE
#endif
#ifdef GENCIRCMFTL26
#define HOMESYSTEM
#define HOMESYSTEM_CLAMP_UNCLAMP_ONLY
#define HOMESYSTEM_X
//#define HOMESYSTEM_X_MC //NO... NOT USING MC for Home
#define HOMESYSTEM_X_RFID //Use X_RFID system
//#define HOMESYSTEM_X_LINK	//X is not part of overall home

#define HOMESYSTEM_Y
#define HOMESYSTEM_Y_MC
#define HOMESYSTEM_Y_HPRM -30
#define HOMESYSTEM_Y_HIB "hib=2;\r"
#define HOMESYSTEM_Y_HIC "hic=13;\r"
#define HOMESYSTEM_Y_POS_PROC_SENSOR "ppi=3;\r"
//There is no Y NEG Sensor... Only Home
//#define HOMESYSTEM_Y_NEG_PROC_SENSOR "pni=4;\r"
#define HOMESYSTEM_Y_LINK	//Y is part of overall home
#define CHECK_X_MOVE_OUT_OF_RANGE
#define CHECK_Y_MOVE_OUT_OF_RANGE
#endif

//Turn on Default whistle inputs define
#ifndef HOMESYSTEM_X_HIB
#define HOMESYSTEM_X_HIB "hib=2;\r"
#endif
#ifndef HOMESYSTEM_X_HIC
#define HOMESYSTEM_X_HIC "hic=13;\r"
#endif
#ifndef HOMESYSTEM_Y_HIB
#define HOMESYSTEM_Y_HIB "hib=2;\r"
#endif
#ifndef HOMESYSTEM_Y_HIC
#define HOMESYSTEM_Y_HIC "hic=13;\r"
#endif

#define PROBE_ADJUST_INIT 0
#define PROBE_ADJUST_WAIT 1
#define PROBE_ADJUST_WAITCOORDS 2
#define PROBE_ADJUST_WAIT_CALC 3

// defs for Probe Mode
#define PROBE_TIMEOUT_SEC 10

// state defs for Position Mode
#define POSNMODE_INIT 0
#define POSNMODE_STARTWAITNEXTPOSN 1
#define POSNMODE_WAITNEXTPOSN 2
#define POSNMODE_TOOLCHECK 3
#define POSNMODE_WAITCLAMP 4
#define POSNMODE_PREMOVE 5
#define POSNMODE_PREMOVEWAIT 6
#define POSNMODE_FINALMOVE 7
#define POSNMODE_FINALMOVEWAIT 8
#define POSNMODE_ARRIVE 9
#define POSNMODE_ACTION 10
#ifdef FASTENER_SYSTEM
#define POSNMODE_ACTION_PREP_FASTENER_1	11
#define POSNMODE_ACTION_PREP_FASTENER_2	12
#define POSNMODE_ACTION_PREP_FASTENER_3	13
#endif
#define POSNMODE_ACTION_PREP 14
#define POSNMODE_ACTION_EXECUTE 16
#define POSNMODE_ACTION_WATCH 17
#define POSNMODE_ACTION_COMPLETE 18
#define POSNMODE_WAITCYCLE 19
#define POSNMODE_STOP_MOVEMENT 20
#define POSNMODE_MOVE_NEAREST 21
#define POSNMODE_MOVE_NEAREST_WAITCOORDS 22
#define POSNMODE_INSPECT 23

// defs for Position Mode
#define GOTOPOSN_TOOFAR -4
#define GOTOPOSN_KHOLEPREVENTED -3  //Can't jump to K Hole currently
#define GOTOPOSN_OUTOFBOUNDS -2  // the specified destination posn is beyond the first or last position
#define GOTOPOSN_UNKNOWN -1  // the next destination position is unknown
#define GOTOPOSN_NEXT 0  // go to next adjacent posn in PP
#define GOTOPOSN_PREV 1  // go to previous adjacent posn in PP
//FIXME00000000 when all done move these defines up
#define GOTOPOSN_RANDOM_INDEX 3  // go to user-specified posn in PP
#define GOTOPOSN_NEAREST 4

#define POSN_TIMEOUT_SEC 120L

#ifdef GENCIRCMFTX
#define LINEAR_SCALE_OPTION
#endif


//Probe Dir + Drill Dir:
// A to B is from first-listed Khole (KA) in PP to second-listed Khole (KB) in PP
// B to A is from second-listed Khole (KB) in PP to first-listed Khole (KA) in PP
//values defined in the OID DEFINES section above
//The defaults are stored in configuration, however specific scan of holes
//will change the order.
//Drill can be set to a DrillDir value which is specific, or relative to ProbeDir
//see g_ConfigData.iProbeDir


// defs for teach mode
#define TEACH_INIT 0
#define TEACH_WAIT 1
#define TEACH_WAITCOORDS 2


// The following are returned from Motion Control after PROBE_HOME motion complete.
// They indicate direction of motion when boundaries were encountered.
#define X_BUMP_UNKNOWN 0
#define X_BUMP_POS 1
#define X_BUMP_NEG 2
#define X_BUMP_OBSTRUCTION 3
#define Y_BUMP_UNKNOWN 0
#define Y_BUMP_POS 1
#define Y_BUMP_NEG 2
#define Y_BUMP_OBSTRUCTION 3

// Values assigned to g_Probe.cXOrientation and g_Probe.cYOrientation.
// These are track axis orientation states derived from bump values above
#define X_NORMAL 1
#define X_FLIPPED 2
#define Y_NORMAL 1
#define Y_FLIPPED 2

// System mode flags, use with set() built-in function
#define MODEFLAG_DONEGRAVCOMPX   0 //bit 0
#define MODEFLAG_DONEGRAVCOMPY   1 //bit 1



// defines for other state machines
#define DRILLSTATE_INIT 0
#define DRILLSTATE_IDLE 1
#define DRILLSTATE_JOG 2
#define DRILLSTATE_HOME 3
#define DRILLSTATE_DRILL 4
#define DRILLSTATE_FAULT 5
#define DRILLSTATE_ESTOP 6
#define DRILLSTATE_NOT_HOME 7
//Special States for Use With Goal Alert Only
#define DRILLSTATE_SPINUP 8
//Special states used with CIRCMFTX
#define DRILLSTATE_DONE	9
#define DRILLSTATE_WAIT_CONFIRM 10

#if 0
//copied here for ref only
#define SMARTDRILL_MODE_DEBUG 0;
#define SMARTDRILL_MODE_IDLE 1;
#define SMARTDRILL_MODE_JOG 2;
#define SMARTDRILL_MODE_HOME 3;
#define SMARTDRILL_MODE_DRILL 4;
#define SMARTDRILL_MODE_FAULT 5;
#define SMARTDRILL_MODE_ESTOP 6;
#define SMARTDRILL_MODE_NOT_HOME 7;
//Special Code For Initial State
#define SMARTDRILL_MODE_INIT 8;
//
#endif

#ifdef SEAL_SYSTEM
//FIXME000000 shrink down and combine with other....
//FIXME0 DFINT these are not aligned.....
#define SEALSTATE_OFF 0
#define SEALSTATE_START_LOAD 1
#define SEALSTATE_WAIT_LOAD 2
#define SEALSTATE_LOAD 3
#define SEALSTATE_PRIME 4
#define SEALSTATE_PRESSURE 5
#define SEALSTATE_AUTOPRIME 6
#define SEALSTATE_WAIT_PRESSURE 7
#define SEALSTATE_WAIT_AUTOPRIME 8
#define SEALSTATE_WAIT_GLOB 9
#define SEALSTATE_WAIT 10
#define SEALSTATE_APPLY 11
#define SEALSTATE_WAIT_APPLY 12
#define SEALSTATE_DONE 13
#endif

#ifdef FASTENER_SYSTEM
#define FILLSTATE_OFF 0
#endif

//DFINT defs...
//IO defs
#ifdef SEAL_SYSTEM
#define SEALANT_APPLY_OFF 0
#define SEALANT_APPLY_ON 1
#define SEALANT_PRESSURE_OFF 0
#define SEALANT_PRESSURE_ON 1
#define SEALANT_PINCH_ON 0
#define SEALANT_PINCH_OFF 1
#endif

#define FAULT_SEVERITY_ALARM 2
#define FAULT_SEVERITY_ABORT_DRILL 3
#define FAULT_SEVERITY_BLOCK_DRILL 4

#define HR_UNKNOWN 0
#define HR_SPINRQST 1
#define HR_DRILLRQST 2
#define HR_STARTED 3
#define HR_FAULT 4
#define HR_ABORT 5
#define HR_SUCCESS 6

//define this for some special strings...applied by hand below..
//FIXME0 Move to OID defs after next wave, and ensure that global OID def is not tampered with
#define MAX_DRILLSETTINGS_LEN 256


//FIXME PORT LOW  the items in this file might be able to be moved to the proper file fairly easily.. take a pass through again....



//Clamp States and Rivet States
#define CLAMP_OFF 0
#define CLAMP_LOOSE 1
#define CLAMP_UNCLAMP 2
#define CLAMP_CLAMP 3
#define CLAMP_HOLD 4
#define CLAMP_TRANSITION 5
#define CLAMP_UNKNOWN 6
#define CLAMP_POSITION 7
//Only partially used... may be temporary
#define CLAMP_ERROR 8

#define LEG_RETRACT 0
#define LEG_EXTEND 1
#define LEG_LOCK 0
#define LEG_UNLOCK 1
#define VALVE_RETRACT 0
#define VALVE_EXTEND 1

#define VALVE_OFF 0
#define VALVE_ON 1
#define ALOCK_ON 0
#define ALOCK_OFF 1

//	Leg lock used by HD.
#define LEGSLOCK_ON 0
#define LEGSLOCK_OFF 1

#ifdef GENCIRCMFTX
#define ACTIVATED 0
#define CUTTER_NOT_DETECTED 0
#define CUTTER_DETECTED 1
#endif
#define CUTTER_UNKNOWN 2

//Obstruction Sensor
#define OBSTRUCTION 0
#define NO_OBSTRUCTION 1

//MOW MOS OBSTRUCTIONS
#define MO_OBSTRUCTION 1
#define MO_NO_OBSTRUCTION 0


//OTHER DEFINES FOR SPECIAL ARGUMENT VALUES:
// When passed as length to build function, function will calculate the length based on a string interpretation of the buffer.
//#define FINDLENGTH_LEGACY_DO_NOT_USE 0x7FFF

//PD1 bits
#define PD1_MASK 7
#define PD1_MACHINE 0
#define PD1_DATASET 1
#define PD1_DATASET_NS 2
#define PD1_POSN 3
#define PD1_NOTHING 7

//PD2 bits 00111000 = 56
#define PD2_MASK 56
#define PD2_FROMZERO 0
#define PD2_PK 8
#define PD2_SK 16
#define PD2_NEAREST 24

//PD3 bit 01000000 = 64
#define PD3_MASK 64
#define PD3_XY 0
#define PD3_DIST 64


//Part Program Related Defines
#define PARTPGM_MAXLINELENGTH 256

//FIXME6666666666666666666
#define FOUND_NOMATCH -2  // at end of narrowing, no common pattern was found
#define FOUND_MORETHANONE -3  // at end of narrowing, more than one pattern is possible


#define MAXNUM_KHOLES	128	//Allocates 2 additional array indexes... so for KHoles, max index is 64 at a[64] and allocation is a[66]
#define MAXNUM_KHOLES_REPROBE 8 //To save memory, a smaller size is uzed for reprobe

#define MAXNUM_PATTERNS 64 //The new system patterns

#define MAXLEN_PATTERNNAME 16   // pattern names can be no longer than this number  FIXME: Not yet used.

#define MAXNUM_TOOLS    64 //64  // this is max number of tools *in the PP*

#define MAXNUM_PROCESSES 64

#define MAXNUM_POSNS 5000  // this is maxnum of x,y positions for the entire PP

#define MAXSIZE_MATERIALS 256 //Can't Address more than this size memory in file format
#define MAXSIZE_STACKDATA 20000 //10000 d2u values

#define PARTPGM_MAX_ERROR_MESSAGES 50

#define MAX_PARTPGM_STORAGE 140000

//These match the parse state flags in the binary format
#define PARSE_SECTION	0x00
#define SECTION_Invalid 0x00
#define SECTION_Header 0x01
#define SECTION_Options 0x02
#define SECTION_Counts 0x03
#define SECTION_ToolTypeNames 0x10
#define SECTION_ProcessNames 0x11
#define SECTION_FastenerTypeNames 0x12
#define SECTION_PatternNames 0x13
#define SECTION_HoleNames 0x14
#define SECTION_KHoleIndex 0x20
//#define SECTION_ToolTypeData 0x21
#define SECTION_ProcessData 0x22
#define SECTION_ProcessLayerData_Alt 0x36 //out of order due to later addition
#define SECTION_ProcessLayerData 0x33 //out of order due to later addition
#define SECTION_HoleData 0x23
#define SECTION_KHoleData 0x24
#define SECTION_ProbeControl 0x25
#define SECTION_MaterialStack 0x30
#define SECTION_StackData 0x31
#define SECTION_SynonymousHole 0x32
#define SECTION_Checksum 0xFF
//HEX	DEC
//30	48
//31	49
//32	50


//for parsing, but not for file format
#define PARSE_FAIL 0xFE

//probe control field
//keywords
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
#define KEYWORD_MaxDistShift	13
#define KEYWORD_Diameter	14
#define KEYWORD_ShiftLimX    15
#define KEYWORD_ShiftLimY    16
#define KEYWORD_AuxA    17

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
#define TKP_EXPECTED_DIAMETER 142

#define TKP_MachShiftLimX 143
#define TKP_MachShiftLimY 144
#define TKP_MachAuxA 145
#define TKP_MachMaxDistShift 146

#define PROBE_COMMAND_INVALID 255

#define MACHINEXY 1
#define DATAXY 2

//Fastener Database
#define MAXNUM_FASTENERPARAMETERS 256
//Used to be 64

#ifdef CENTERVISION
	#define LASERSENSOR_OFF 0
	#define LASERSENSOR_ONLINE 1
	#define LASERSENSOR_SEE_HOLE 2
	#define LASERSENSOR_SEE_CENTER 4
	#define LASERSENSOR_OVER_HOLE 8
	#define LASERSENSOR_SEE_EDGE 16

	//Result
	#define CENTERVISION_OFF	0
	#define CENTERVISION_WORKING	1
	#define CENTERVISION_FAILURE	2
	#define CENTERVISION_SUCCESS	3

	//Requirements for success
	#define CENTERVISION_CENTER 1
	#define CENTERVISION_EDGE 2
#endif

//MESSAGE CODE DEFINES
#define COMMONMC_GET_RESP		1
#define COMMONMC_BADLENGTH		2
#define COMMONMC_NOSUCHOID		3

#define MINIFTMC_GET_RESP		100
#define MINIFTMC_INVALID_PARAMETER	101
#define MINIFTMC_UNABLE_TO_MOVE		102
#define MINIFTMC_STOREDEFAULT_SUCCESS	103
#define MINIFTMC_STOREDEFAULT_FAILURE	104
#define MINIFTMC_LOADFACTORYSETTINGS_SUCCESS	105
#define MINIFTMC_LOADDEFAULT_SUCCESS	106
#define MINIFTMC_LOADDEFAULT_FAILURE	107
#define MINIFTMC_GRAVCOMP_PREVIOUSLY_RUNNING	108
#define MINIFTMC_GRAVCOMP_PREVENTED_BY_ESTOP	109
#define MINIFTMC_WAIT_FOR_CARRIAGE_STOP	110
#define MINIFTMC_JOG_PREVENTED_BY_FLOAT	111
#define MINIFTMC_MOVE_PREVENTED_BY_FLOAT	112
#define MINIFTMC_FLOAT_NEEDS_GRAVCOMP	113
#define MINIFTMC_FLOAT_PREVENTED_BY_ESTOP	114
#define MINIFTMC_POSN_NO_PATTERN	115
#define MINIFTMC_FINAL_POSN_KHOLE_VISIT	116
#define MINIFTMC_FINAL_POSN		117
#define MINIFTMC_KHOLE_MOVE_PREVENTED	118
#define MINIFTMC_GOTOPOSN_OUT_OF_BOUNDS	119
#define MINIFTMC_PROGRAM_LOCKED		120
#define MINIFTMC_ZRAIL_FAIL			121
#define MINIFTMC_VM_EMBA	122
#define MINIFTMC_VM_EMBV	123
#define MINIFTMC_BEYOND_TOLERANCE_X	124
#define MINIFTMC_BEYOND_TOLERANCE_Y	125
#define MINIFTMC_NO_BUMPER_SENSED	126
#define MINIFTMC_PROBE_ADJUST_OUT_OF_TOLERANCE	127
#define MINIFTMC_GRAVCOMP_FAILURE	128
#define MINIFTMC_FAILED_UNCLAMP	129
#define MINIFTMC_FAILED_CLAMP	130
#define MINIFTMC_CV_LASER_SENSOR_CONNECTION 131
#define MINIFTMC_CV_LASER_START_OVER_HOLE 132
#define MINIFTMC_CV_FAIL_CENTER 133
#define MINIFTMC_CV_FAIL_EDGE 134
#define MINIFTMC_CV_FAIL_EDGE_SENSITIVE 137
#define MINIFTMC_CV_FAIL_DIA_DIFFERENCE 138
#define	MINIFTMC_CV_FAIL_DIA_SIZE 139
#define	MINIFTMC_CV_FAIL_RESOURCES 140
#define MINIFTMC_CV_FAIL_EXT_ANALYSIS 141
#define MINIFTMC_CV_FAIL_NOT_IMPLEMENTED 142
#define MINIFTMC_MOVE_WOULD_GO_OUT_OF_RANGE	144
#define MINIFTMC_PROBE_INCOMPLETE 145
#define MINIFTMC_NO_PROGRAM 146
#define MINIFTMC_INVALID_KI 147
#define MINIFTMC_ROTATION_POINTS_TOO_CLOSE 148
#define MINIFTMC_PROGRAM_WITHOUT_MEMORY_CONFIG 149
#define MINIFTMC_GRAVCOMP_PREVENTED 150
#define MINIFTMC_JOG_PREVENTED	151
#define MINIFTMC_FLOAT_PREVENTED	152
#define MINIFTMC_HOME_PREVENTED 153
#define MINIFTMC_MOVE_PREVENTED 154
#define MINIFTMC_CV_CAM_CONNECTION 155
#define MINIFTMC_CV_CAM_NO_HOLE 156
#define MINIFTMC_CV_CAM_MOVE_LIMIT 157
#define MINIFTMC_CV_CAM_MOVE_ERROR 158
#define MINIFTMC_CV_CAM_CLAMP_ERROR 159
#define MINIFTMC_CV_CAM_NO_EDGE 160
//reserved block
#define MINIFTMC_PROBE_PREVENTED 167
#define MINIFTMC_PROBE_PREVENTED_HOME 168
#define MINIFTMC_PROBE_PREVENTED_DRILL 169
#define MINIFTMC_PROBE_PREVENTED_CLAMP 170
#define MINIFTMC_RFID_HOME_PREVENTED 171
#define MINIFTMC_RFID_HOME_PREVENTED_ACTION 172
#define MINIFTMC_RFID_HOME_PREVENTED_MODE 173
#define MINIFTMC_RFID_HOME_PREVENTED_MOVING 174
#define MINIFTMC_RFID_HOME_PREVENTED_DRILL 175
#define MINIFTMC_RFID_HOME_PREVENTED_CLAMP 176
#define MINIFTMC_POSITON_PREVIOUSLY_DRILLED 177
#define MINIFTMC_LUBE_POSITION_WARNING 178
#define MINIFTMC_INSPECT_THIS_POSITION 179
#define MINIFTMC_DRILLFILL_LOW_RPM_FAILURE 180
#define MINIFTMC_DRILL_AIR_OFF_EARLY 181
#define MINIFTMC_DRILL_AIR_ON_LONG 182
#define MINIFTMC_PROBE_COMMAND_INVALID 183
#define MINIFTMC_PROBE_COMMAND_REPEAT 184
#define MINIFTMC_PROBE_COMMAND_OPTIONS_INVALID 185
#define MINIFTMC_PROBE_COMMAND_REQUIRE_INVALID 186
#define MINIFTMC_PROBE_COMMAND_PARAMETERS 187

#define MINIFTMC_HOLE_MOVE_PREVENTED	195
#define MINIFTMC_PROBE_WARNING_DRILL_PREVENTED 196

#define MINIFTMC_FASTENER_COM_FAIL 199

// Messages unique to PP Parser:
#define MINIFTMC_PPERR_XMEM_NOT_ALLOCATED	200
#define MINIFTMC_PPERR_FAILURE_TO_OPEN_FILE	201
#define MINIFTMC_PPERR_NOT_MINIFT_FORMAT	202
//skip 203
#define MINIFTMC_PPERR_UNKNOWN_SECTION		204
#define MINIFTMC_PPERR_UNSUPPORTED_CHECKSUM_TYPE 205
#define MINIFTMC_PPERR_UNSUPPORTED_CHAR_ENCODING 206
#define MINIFTMC_PPERR_CHECKSUM_FALURE	207
#define MINIFTMC_PPERR_BYTES_AFTER_CHECKSUM 208
#define MINIFTMC_PPERR_TOOL_FLIP_MISMATCH 209
#define MINIFTMC_PPERR_OVER_POSN_LIMIT		210
#define MINIFTMC_PPERR_SECTION_SIZE_DIFF     219
#define MINIFTMC_PPERR_OVER_RESERVED_PPMEM	220
#define MINIFTMC_PPERR_OVER_STRING_MEM		221
#define MINIFTMC_PPERR_OVER_POSN_NAME_STRING_MEM	222
#define MINIFTMC_PPERR_XY_NOT_HOMED	223
#define MINIFTMC_PPERR_UNKNOWN_PARAMETER	230

#define MINIFTMC_TOOL_SHORT_WARNING_COUNTERSINK_WILL_START 235
#define MINIFTMC_TOOL_SHORT_FAILURE 236



//edit
typedef struct
{
	float fX;
	float fY;
} td_XYDistance;

typedef struct
{
	byte cDevice;
	byte cSeverity;
	byte cSeverityPrev;
	byte cReported;
	long lFaultCode;
	float fPosition;
	int iCyclesOnCutter;
	unsigned long ulDateTime;
	unsigned int uiHoleNumber;
//Ignore these fields.
//	byte CutterType[32];
//	byte ProfileFilename[64];
} td_SmartDrillFault;

//FIXME PORTLOW  look into this switch but do away
//
//#ifdef OBSTRUCTION_SYSTEM_MO_ANA
//	td_MOCal MOCal;
//#endif

typedef struct
{
	int iDirX;
	int iDirY;
} td_GravComp;

typedef struct
{
	byte cCaptureTrig;
	byte cHomeTrig;
	float fHomeX; //machine coordinates for Home
	float fHomeY; //machine coordinates for Home
	byte cGotHome;
	byte cXBumperDirection;
	byte cYBumperDirection;
	byte cRegistration;
	byte cProbeAdjustTrig;
	float fProbeAdjustX; //machine coordinates for PROBE_ADJUST
	float fProbeAdjustY; //machine coordinates for PROBE_ADJUST
	byte cGotProbeAdjust;
//Save mem since nothing is using this now
//	char szTeachPosnName[MAX_STRING_LEN];
	byte cTeachCaptureTrig;
	byte cGotTeachCoords;
} td_ProbeMode;

typedef struct
{
	int iStartPosnIndex;
	int iCurPosnIndex;
	byte cFirstMove;
	byte cOnCurPosn;
	int iGotoPosnIndex;
	byte cDoMoveTrig;
	float fLastKnownPosnX;
	float fLastKnownPosnY;
	byte cFreshCurPosn;
	byte cFreshCurPosnSend;
	float fPosnX; //similar to LastKnownPosn, but only set by newer GetPosition* functions
	float fPosnY;
	byte cFreshPosn; //For use only with new GetPosition* functions to distinctly detect their return
	unsigned long ulFreshPosnTime; //the rxtime
	int iNearestPosn;
	float fNearestPosnDX;
	float fNearestPosnDY;
	float fNearestPosnMX;
	float fNearestPosnMY;
	float fNearestDistanceX; //X distance to this point... signed
	float fNearestDistanceY; //Y distance to this point... signed
	float fNearestDistanceSquared; //distance of this point at time of discovery
	float fNearestCalculatedForMachineX;
	float fNearestCalculatedForMachineY;
	float fNearestRadiusExclusive;
	float fLastSentPosnX;
	float fLastSentPosnY;
	int iLastSentNearestPosn;
} td_PosnMode;

//Teach

typedef struct
{
	float fX;
	float fY;
} td_TeachMachinePosn;

extern byte g_cModeState;
//FIXME PORTMED + EstopMCAsserted???????
extern byte g_cEstopMCAsserted;


//HOMESYSTEM
#ifdef HOMESYSTEM
extern byte g_cHomed;	//overall system home status
extern byte g_cSentHomed;
#ifdef HOMESYSTEM_X
extern byte g_cHomedX;	//X Axis
#endif
#ifdef HOMESYSTEM_Y
extern byte g_cHomedY;	//Y Axis
#endif
#endif


extern byte g_cEstopMCAsserted;
extern byte g_cEstopFullyEngaged;

extern uint16 g_uiClampPressureLBS;

//state variables for specific modes
byte g_cCurrentJogDir;

// struct for holding gravcomp results and handshakes
extern td_GravComp g_GravComp;
// grav comp timer
extern uint32 g_uiGravCompTime;

// struct for holding probe results and handshakes
extern td_ProbeMode g_Probe;

// struct for holding Position Mode results and handshakes
extern td_PosnMode g_PosnMode;

//Jog
extern byte g_cJogX; //The status
extern byte g_cJogY; //The status
extern byte g_cJogGoalX; //The goal
extern byte g_cJogGoalY; //The goal
extern float g_fJogX; //The status speed factor
extern float g_fJogY; //The status speed factor
extern float g_fJogGoalX; //The goal speed factor
extern float g_fJogGoalY; //The goal speed factor
extern uint32 g_uiJogStopX;
extern uint32 g_uiJogStopY;
#ifdef JOG_ENABLE_TIME
extern uint32 g_uiJogEnableTime;
#endif

//HD Clamp Status
#ifdef CLAMP_SYSTEM
extern byte g_cClampState;
extern byte g_cClampGoal;
extern byte g_cClampStateSent;
extern byte g_cClampGoalSent;
#endif
extern uint32 g_ulClampStart;
extern uint32 g_ulClampPressureZero;
extern uint32 g_ulClampPressureLow;
extern uint32 g_ulClampPressureHigh;
extern uint32 g_ulClampLegsLock;
extern uint32 g_ulClampLegsUnlock;
extern uint32 g_ulClampLegsUp;
extern uint32 g_ulClampLegsDown;
extern uint32 g_ulClampALock;
extern uint32 g_ulClampAUnlock;
extern uint16 g_uiClampPressure;
extern uint16 g_uiClampPressureWarning;
extern uint16 g_uiClampPressureAbort;
extern char * g_szClampMessage;

//Position Update
extern uint32 g_uiPositionUpdateTime;
extern uint32 g_uiPositionUpdateThrottle;
extern uint32 g_uiPositionSendTime;
extern uint32 g_uiPositionSendThrottle;
extern float g_fPosnLSCD; //Least significant change distance: Changes smaller than this should not be signalled back

//Position Limit
//Loaded from Config at the time needed
extern float g_fHomePositionX;
extern float g_fHomePositionY;
extern float g_fPositionMinX;
extern float g_fPositionMaxX;
extern float g_fPositionMinY;
extern float g_fPositionMaxY;

td_TeachMachinePosn g_TeachMachinePosn;

//probe
extern byte g_cProbeFlag;
extern float g_fProbeX; //Hold The Probe Position Until it is placed into the arrays
extern float g_fProbeY;

//Probe Command Field Probe Parameters for Probe Control
extern byte g_cProbeCommand;
extern byte g_cProbeCommandMessage;
extern float g_fProbeVectorX;
extern float g_fProbeVectorY;
extern float g_fProbeMachineVectorX;
extern float g_fProbeMachineVectorY;
extern byte g_cProbeExtraOffsetGiven;
extern byte g_cProbeExtraMachineOffsetGiven;
extern float g_fProbeExtraOffsetX;
extern float g_fProbeExtraOffsetY;
extern float g_fProbeExtraMachineOffsetX;
extern float g_fProbeExtraMachineOffsetY;
extern float g_fProbeMaxDistShift;
extern byte g_cProbeShiftLimX;
extern float g_fProbeShiftLimXMin;
extern float g_fProbeShiftLimXMax;
extern byte g_cProbeShiftLimY;
extern float g_fProbeShiftLimYMin;
extern float g_fProbeShiftLimYMax;
extern float g_fProbeExpectedDiameter;


//Some Additional functions still in Main which are needed elsewhere
void DoFloat(byte cAction);

//I Placed these in an extended file to help break down the main file
//FIXME PORTMED  reorg the extended file

//Data, string, and debug functions
int SignedCharValue(char c);
char * scanToNull(char * s);
char * scanToChar(char * s, char c);
char * scanPastWhiteSpace(char * s);
char * scanToWhiteSpace(char * s);
char * scanToWhiteSpaceOrChar(char * s, char cDelimiter);
int prefix(char *prefix, char *str);
int equal(char *str1, char *str2);
char *strcpy_returnpost(char *dst, char *src);
char split(char cDelimiter, char * s, char ** p_szFields, char cMaxFields);
void rchr(char *s, char c, char n);
//define two special split return values
#define SPLIT_PARSE_ERROR_QUOTE_MISMATCH 255
#define SPLIT_PARSE_ERROR_CHARS_AFTER_VALUE 254

void initCopyHexToByte();
byte copyHexToByte(byte * bytes, byte *szhex, byte cCount, byte cRequireEnd);

#ifdef PROFILEPOINTS
void ClearProfiling();
void ProfilePoint(char * p_cLocation);
void PrintProfilePoints();
#else
#define ProfilePoint(s)
#endif

// DEBUG DISPLAY TOOLS ////////////////////////////////////////////
void memchardump(char * label, char * data, int iLen);
void memdump(char * label, byte * data, int iLen);

#define strnull(x) *((char *)x)=0

void ClearShowIO();
void ShowIO();
void ClearShowAna();
void ShowAna();

//  Utillity Section
//Math
//returns 0 on success
byte FindIntersection(float ax1, float ay1, float ax2, float ay2, float bx1, float by1, float bx2, float by2, float *fx, float *fy);

extern byte g_cConfigLoadSuccess;
extern byte g_cConfigSaveSuccess;

void LoadConfigFile();
void SaveConfigFile();



void InitConfigHARDCODED(void);



#endif /* MINIFTDEFS_H_ */
