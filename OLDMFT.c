////////////////////////////////////////////////////////////////////////////
// MiniFt And MiniFt HD
//
// 20131018-1-0843	David Whalen-Robinson	First Linux Smart Tool
//	Code should be considered a prototype.  All libraries are local project files.  All libraries and code locations
//  should be considered in temporary locations and are likely to experience subsequent revisions
#if 0

////////////////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <math.h>
#include <errno.h>

#include "CrawlerDefs.h"
#include "hwio.h"
#include "SmartToolUtil.h"
#include "SmartTool.h"
#include "CommonSmartTool.h"
#include "CrawlerSmartTool.h"
#include "CrawlerConfigScript.h"
#include "STP.h"

//HEREDOWN

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

//MC Asserted Estop
#define MC_ASSERTED_ESTOP

//Control Reading of motor voltage
//#define READMOTORVOLTAGE

//Use Motor Voltage for Estop rather than digital in
//#define ANALOG_MOTORVOLTAGE_ESTOP_DETECT
#ifdef ANALOG_MOTORVOLTAGE_ESTOP_DETECT
#ifndef READMOTORVOLTAGE
#fatal "ERROR:ANALOG_MOTORVOLTAGE_ESTOP_DETECT Requires READMOTORVOLTAGE"
#endif
#endif

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
	//#define OUTPUT_BRAKE
//	#define OUTPUT_OBSTRUCTION_CODE
//	#define OUTPUT_ESTOP
	//#define OUTPUT_PP_SYS
//    #define OUTPUT_PROGRAM_MEM_USAGE
//ex	#define OUTPUT_PROBE_SYS
//Hoping to solve a specific issue with this.......
	//#define OUTPUT_PROBE_SYS_VERBOSE
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
//ex	#define OUTPUT_OPERATIONS
//ex    #define OUTPUT_HDCLAMP_STATES 1
//	#define OUTPUT_DIGIN_CHNUM_XLOCK
//ex	#define OUTPUT_DIGIN_CHNUM_YLOCK
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
	#nodebug
	#undef USE_OUTPUT
	#undef OUTPUT_POSITION_INFO
	#undef OUTPUT_POSITION_STATES
#endif

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

//additional core macros
//#define atoul(x) ((unsigned long)atol(x))
//#define atoui(x) ((unsigned int)atol(x))
#define setholedata(i,d)	root2xmem(xp_tdHoleData+(((unsigned long)i)<<4),(void *)d,sizeof(td_HoleData))
#define loadholedata(i,d)	xmem2root((void *)d,xp_tdHoleData+(((unsigned long)i)<<4),sizeof(td_HoleData))
//take advantage of the fact that we don't need to always load all the holddata.
//return kind
#define getkind(i)  (xgetint(xp_tdHoleData+(((unsigned long)i)<<4)) & 0xFF)
#define GETKIKPKS(cki,ckp,cks,i)  g_ultemp = xgetlong(xp_tdHoleData+(((unsigned long)i)<<4)); cki = p_cultemp[0]; cpk = p_cultemp[1]; csk = p_cultemp[2];

// definitions for ethernet bootloader
#define UDP_SOCKETS 1 // allow enough for downloader and DHCP
#define MAX_UDP_SOCKET_BUFFERS 1

//#define SHOW_STP_HEARTBEAT

#define TCPCONFIG 1     // configure TCP/IP, see tcp_config.lib for configuration table
#define _PRIMARY_STATIC_IP	"192.168.0.10"
#define _PRIMARY_NETMASK	"255.255.255.0"
#define MY_GATEWAY			"192.168.0.1"
#define MY_NAMESERVER		"192.168.0.1"

#define MAXNUM_STP_SESSIONS 2

//MAX_TCP_SOCKET_BUFFERS should be all STP sessions,
// any motor control, any socket debug sessions, etc..
//NOTE: We get an extra one because errors were happening otherwise.
//FIXME MEDIUM : Dynamic C bug?
// (fastread gets the number of bytes rxd correct, but the actual message is truncated)
//Normal Sessions + SocketConsole + NAC + Cam + Drill + Fastener + Fastener Tray
#define MAX_TCP_SOCKET_BUFFERS   MAXNUM_STP_SESSIONS + 6

#define TCP_CONNECTION_TIMEOUT_SEC 5
//#define DCRTCP_DEBUG
//#define TCP_VERBOSE
//#define ARP_VERBOSE

#define IF_ETH0_TIMEOUT_SEC 5

//uncomment to monitor packet flood conditions
#define PACKET_FLOOD_MONITOR

#define PARSE_NONBLOCKING

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
#define HOMESYSTEM_X_HIBHIC "hib=1;hic=11;\r"
#define HOMESYSTEM_Y_HIBHIC "hib=1;hic=11;\r"
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
#fatal "Home System has been updated, but this model has not been compelted"
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
#define HOMESYSTEM_Y_HIBHIC "hib=2;hic=13;\r"
#define HOMESYSTEM_Y_POS_PROC_SENSOR "ppi=3;\r"
//There is no Y NEG Sensor... Only Home
//#define HOMESYSTEM_Y_NEG_PROC_SENSOR "pni=4;\r"
#define HOMESYSTEM_Y_LINK	//Y is part of overall home
#define CHECK_X_MOVE_OUT_OF_RANGE
#define CHECK_Y_MOVE_OUT_OF_RANGE
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

// STP OID definitions
//MakeOID-generated:: OID DEFINES (complete)
#define OID_NULLOID		0
#define OID_DEBUG		1
#define OID_SMARTTOOL_TYPE		2
#define OID_SMARTTOOL_SUBTYPE		3
#define OID_SYSTEM_VERSION		4
#define OID_SERIAL_NUMBER		5
#define OID_RESET_SYSTEM		6
#define OID_SCRIPT_TRANSFER		7
#define OID_GENERICMESSAGE		8
#define OID_DEBUGMESSAGE		9
#define OID_STPSTATUS		10
#define OID_COMMON_MESSAGE_CODE		11
#define OID_CONTROL_DELAY		12
#define OID_DIRECT_DOUT		13
#define OID_DIRECT_DIN		14
#define OID_COM_VERSION		15
#define OID_STREAM_SYNC		16
#define OID_STP_CLOSE		17
#define OID_STP_IDENTIFY		18
#define OID_DATE_TIME		19
#define OID_MAX_NUMBER_Common		20

#define OID_MINIFT_MESSAGE_CODE		100
#define OID_MODE		101
#define OID_ACTION		102
#define OID_RESET_MC		103
#define OID_ENCODER_RATIO		104
#define OID_MC_CURRENT_LIMITS		105
#define OID_MC_PEAK_CURRENT_LIMITS		106
#define OID_BRAKE_ON_TIMEOUT		107
#define OID_MCERR		108
#define OID_STOREDEFAULT_CONFIG		109
#define OID_RECALL_CONFIG		110
#define OID_EEOPTION_DEFAULT		111
#define OID_EEOPTION		112
#define OID_BEEPER		113
#define OID_SYSTEM_MONITOR		114
#define OID_AIR_PRESSURE		115
#define OID_LIMITS_AND_OBSTRUCTIONS		116
#define OID_TOOL_VERIFYENABLE		117
#define OID_HOLE_PARAMETERS		118
#define OID_TOOL_RESERVED		119
#define OID_PROCESS		120
#define OID_RETURN_HEIGHT		121
#define OID_GAUGE_LENGTH		122
#define OID_SCALE_MODE		123
#define OID_RESERVED_2		124
#define OID_PARTPGM_CLEAR		125
#define OID_PARTPGM_REQUEST_FILE		126
#define OID_PARTPGM_DIR		127
#define OID_PARTPGM_NAME		128
#define OID_PARTPGM_DATA		129
#define OID_PARTPGM_CHECKSUM		130
#define OID_PARTPGM_LINEARJOB		131
#define OID_PARTPGM_STATUS		132
#define OID_PARTPGM_LOCKED		133
#define OID_STARTOVER		134
#define OID_LOAD_YRAIL		135
#define OID_GRAVCOMP_STATUS		136
#define OID_GRAVCOMP_AXES		137
#define OID_GRAVCOMP_CMD		138
#define OID_GRAVCOMP_FLOAT		139
#define OID_GRAVCOMP_SPEED		140
#define OID_GRAVCOMP_ACCEL		141
#define OID_GRAVCOMP_DECEL		142
#define OID_GRAVCOMP_MOVEDIST		143
#define OID_GRAVCOMP_ALGORITHM		144
#define OID_GRAVCOMP_NOISE_LIMIT		145
#define OID_GRAVCOMP_TRIGGERFACTOR		146
#define OID_FLOAT_STATUS		147
#define OID_GRAVCOMP_RESULTS		148
#define OID_FLOAT_SPEEDLIMIT		149
#define OID_JOG_SPEEDLIMIT		150
#define OID_MAX_SPEED_X		151
#define OID_MAX_SPEED_Y		152
#define OID_X_RAIL_SURFACE_OFFSET		153
#define OID_PROBE_METHOD		154
#define OID_PROBE_METHOD_DEFAULT		155
#define OID_PROBE_OFFSET		156
#define OID_PROBE_DIR		157
#define OID_DRILL_DIR		158
#define OID_KHOLE_MAX_DISTANCE_ERROR		159
#define OID_APPROX_LOCATION_ERROR		160
#define OID_KHOLE_DISTANCE		161
#define OID_PROBE		162
#define OID_PROBE_POSITION		163
#define OID_PROBE_STATUS		164
#define OID_PROBE_START		165
#define OID_PROBE_ACCEPT_REQUIRED		166
#define OID_HOME		167
#define OID_HOME_SPEED		168
#define OID_HOME_ACCEL		169
#define OID_HOME_DECEL		170
#define OID_HOME_MOVEDIST		171
#define OID_PROBE_ADJUST		172
#define OID_PROBE_ADJUST_LIMIT		173
#define OID_HOME_FINE_SPEED		174
#define OID_HOME_REPORT		175
#define OID_PATTERN		176
#define OID_POSNMODE_CURPOSN		177
#define OID_POSNMODE_NEARPOSN		178
#define OID_POSNMODE_GOALPOSN		179
#define OID_POSNMODE_CURXY		180
#define OID_POSNMODE_NEARXY		181
#define OID_POSNMODE_POSNSUMMARY		182
#define OID_POSNMODE_MOVETONEXT		183
#define OID_POSNMODE_MOVETOPREV		184
#define OID_POSNMODE_MOVETOIND		185
#define OID_POSNMODE_MOVEAGAIN		186
#define OID_POSNMODE_MOVETYPE		187
#define OID_POSNMODE_PREMOVEXY		188
#define OID_POSNMODE_SPEED		189
#define OID_POSNMODE_ACCEL		190
#define OID_POSNMODE_DECEL		191
#define OID_POSNMODE_FINALSPEED		192
#define OID_ORTHO_SLOPE		193
#define OID_POSNERR_LIMIT		194
#define OID_POSNMODE_TOLERANCE		195
#define OID_VELERR_LIMIT		196
#define OID_LONG_DISTANCE		197
#define OID_LONG_SPEED		198
#define OID_POSNMODE_MOVEDONE		199
#define OID_OP_STARTED		200
#define OID_OP_HISTORY		201
#define OID_DRILL_HOLE_ONE_TIME		202
#define OID_AUTOMOVE		203
#define OID_AUTOMOVE_DELAY		204
#define OID_AUTOREPEAT		205
#define OID_MACHINE_OFFSET		206
#define OID_MACHINE_OFFSET_CADJ		207
#define OID_MACHINE_OFFSET1		208
#define OID_MACHINE_OFFSET2		209
#define OID_STATION		210
#define OID_TOOL_OFFSET		211
#define OID_TOOL_FLIP		212
#define OID_DRIVE_THROUGH_BACKLASH		213
#define OID_DRILL_OFFSET1		214
#define OID_DRILL_OFFSET2		215
#define OID_OFFSET_SEAL		216
#define OID_OFFSET_FILL		217
#define OID_JOG		218
#define OID_JOG_SPEED		219
#define OID_JOG_ACCEL		220
#define OID_JOG_DECEL		221
#define OID_JOG_FACTOR		222
#define OID_HOME_POSITION_Y_POS		223
#define OID_POSITION_LIMIT_Y_POS		224
#define OID_HOME_POSITION_Y_NEG		225
#define OID_POSITION_LIMIT_Y_NEG		226
#define OID_PROBE_REGISTRATION		227
#define OID_OBSTRUCTION_CODE_MASK		228
#define OID_MACHINE_LOCK_REQUIRED		229
#define OID_MACHINE_LOCK		230
#define OID_CLAMP		231
#define OID_ALOCK		232
#define OID_ALOCKDELAY		233
#define OID_AUNLOCKDELAY		234
#define OID_LEGLOCKDELAY		235
#define OID_LEGUNLOCKDELAY		236
#define OID_LEGSUPDELAY		237
#define OID_LEGSDOWNDELAY		238
#define OID_LOWPRESSUREDELAY		239
#define OID_LOWPRESSUREDOWNDELAY		240
#define OID_PRESSUREDELAY		241
#define OID_PRESSUREDOWNDELAY		242
#define OID_LOWPRESSURE		243
#define OID_PRESSURE		244
#define OID_AIR_CLEAR		245
#define OID_LASER_SENSOR_OFFSET		246
#define OID_CAM_OFFSET		247
#define OID_VISION_INSPECT		248
#define OID_VISION_IMAGE		249
#define OID_VISION_DATA		250
#define OID_VISION_INSPECT_RESULTS		251
#define OID_LASER_SENSOR_ALG_PARAM		252
#define OID_CAM_ALG_PARAM		253
#define OID_VISION_AUTO_INSPECT		254
#define OID_STOP_INTERFACE_TASK		255
#define OID_PROCESS_START		256
#define OID_PROCESS_STOP		257
#define OID_PROCESS_CONTINUE_MODE		258
#define OID_PROCESS_OPERATIONS		259
#define OID_DRILL_STATE		260
#define OID_DRILL_EXPLANATION		261
#define OID_DRILL_EXPLANATION_DATA		262
#define OID_SEAL_STATE		263
#define OID_SEAL_CLAMP		264
#define OID_SEAL_PRESSURE_DELAY		265
#define OID_SEAL_PRESSURE_RELEASE_DELAY		266
#define OID_SEAL_PRIME_DELAY		267
#define OID_SEAL_GLOB_DELAY		268
#define OID_SEAL_APPLY_DELAY		269
#define OID_FILL_STATE		270
#define OID_FILL_CLAMP		271
#define OID_FILL_EXTEND_DELAY		272
#define OID_FILL_RAM_DELAY		273
#define OID_FASTENER_REQUEST		274
#define OID_FASTENER_ARRIVED		275
#define OID_FASTENER_POST_DELAY		276
#define OID_ACCEL_ARM		277
#define OID_ACCEL_TRIGGER		278
#define OID_TOOL_Z_BASE		279
#define OID_FASTENER_FAULT		280
#define OID_POSNMODE_ACTIVE_PREMOVEXY		281
#define OID_DRILL_FAULT		282
#define OID_HOLE_RESULT_DATA		283
#define OID_MOVEXY		284
#define OID_POSN_DISPLAY		285
#define OID_POSNMODE_XYDATA_ID		286
#define OID_POSNMODE_XYDATA		287
#define OID_RFID_CONFIG		288
#define OID_RFID_DATA		289
#define OID_HOME_RFID		290
#define OID_READ_RFID		291
#define OID_HOME_STOP		292
#define OID_ESTOP_CLEAR_DELAY		293
#define OID_DRILL_BUTTON_DELAY		294
#define OID_USE_CUTTER_DETECT		295
#define OID_JOG_ENABLE_TIMEOUT		296
#define OID_DRILL_CYCLE_DELAY		297
#define OID_INSPECT_METHOD		298
#define OID_COMMAND_INSPECT_METHOD		299
#define OID_FORCE_SENSOR_CALIBRATION		300
#define OID_FORCE_SENSOR		301
#define OID_FORCE_LIMITS		302
#define OID_PROBE_FLAGS		303
#define OID_MO_CAL		304
#define OID_VISION_EXTERNAL_ANALYSIS		305
#define OID_LIMITS_AND_OBSTRUCTION_WARNINGS		306
#define OID_RFID_TAG_SET		307
#define OID_PROBE_UPDATE_NOW		308
#define OID_KHOLE_MAX_DISTANCE_CHECK		309
#define OID_MAX_EDGE_SHIFT_PROBE_ACCEPT		310
#define OID_ALLOW_DRILL_BEYOND_SHIFT_LIMITS		311
#define OID_NAC_SERIAL_NUMBER		312
#define OID_Y_RETRACT		313
#define OID_SYSTEM_COMPONENTS		314
#define OID_RFID_TAG_SET2		315

#define OID_MAX_NUMBER_MiniFT		316

#define OID_TOOL_MGMT		400
#define OID_TOOL_REC		401

#define OID_MAX_NUMBER_ToolManagement		402
//#endif

//MakeOID-generated::END

//MakeOID-generated:: VALUE DEFINES (complete)

//MakeOID-generated:: STPSTAT value association
#define STPSTAT_OK 0
#define STPSTAT_NOSUCHOID 1
#define STPSTAT_VALOVERMAX 2
#define STPSTAT_VALUNDERMIN 3
#define STPSTAT_BADLENGTH 4

//MakeOID-generated:: STPCLOSEREASON value association
#define CLOSE_COMPLETE 0
#define CLOSE_REQUESTED 1
#define CLOSE_TIMEOUT 2
#define CLOSE_SYNC 3
#define CLOSE_ERROR 4

//MakeOID-generated:: MiniFTMode value association
#define MODE_IDLE 0
#define MODE_PROBE 1
#define MODE_PROBEK1 2
#define MODE_PROBEK2 3
#define MODE_PROBE_HOME 4
#define MODE_PROBE_ADJUST 5
#define MODE_POSN 6
#define MODE_TEACH 7
#define MODE_CALIBRATE 8
#define MODE_INSPECT 9
#define MODE_ESTOP 10

//MakeOID-generated:: ACTION_CODES value association
#define ACTION_IDLE 0
#define ACTION_READY 1
#define ACTION_HOME 2
#define ACTION_PROBE 3
#define ACTION_CALC 4
#define ACTION_RUNNING 5
#define ACTION_MOVE 6
#define ACTION_EXECUTE 7
#define ACTION_INSPECT 8
#define ACTION_ESTOP 9

//MakeOID-generated:: GRAVCOMP_AXES value association
#define GRAVCOMP_ALLAXES 0
#define GRAVCOMP_XAXIS 1
#define GRAVCOMP_YAXIS 2

//MakeOID-generated:: JOGVALUE value association
#define JOGSTOP 0
#define JOGPOS 1
#define JOGNEG 2

//MakeOID-generated:: ProbeMethods value association
#define PROBE_NONE 0
#define PROBE_INSTANT 1
#define PROBE_INSTANT_OFFSET 2
#define PROBE_MANUAL 3
#define PROBE_LASER 4
#define PROBE_CAM 5
#define PROBE_HARDSTOP 6
#define PROBE_ABSOLUTE 7

//MakeOID-generated:: InspectMethods value association
#define INSPECT_MANUAL 3
#define INSPECT_LASER 4
#define INSPECT_CAMERA 5

//MakeOID-generated:: ProbeDir value association
//Deprecated ??
#define PROBEDIR_NONE 0
#define PROBEDIR_ATOB 1
#define PROBEDIR_BTOA 2

//MakeOID-generated:: DrillDir value association
//Deprecated ??
#define DRILLDIR_NONE 0
#define DRILLDIR_ATOB 1
#define DRILLDIR_BTOA 2
#define DRILLDIR_SAME 3
#define DRILLDIR_REVERSE 4

//MakeOID-generated:: ProbeCodes value association
#define PC_CLEAR 0
#define PC_PROBE 1
#define PC_MOVE 2
#define PC_MOVE_PROBE 3
#define PC_MOVE_PROBE_ALL 4
#define PC_STOP 5
#define PC_COMPLETE 6
#define PC_ADD 7
#define PC_DELETE 8
#define PC_REAPPLY 9

//MakeOID-generated:: ProbeStatus value association
#define PS_NO_PROBE 0
#define PS_PROBED_APPROX_DIFF_FAILURE 1
#define PS_PROBED_KHOLE_DIFF_FAILURE 2
#define PS_PROBED_FAILED 3
#define PS_EXTRAPOLATED 4
#define PS_APPROXIMATE 5
#define PS_PROBING 6
#define PS_PENDING_ACCEPTANCE 7
#define PS_PROBED 8
#define PS_PROBED_ACCEPTED 9

//MakeOID-generated:: ProbeRegistration value association
//Deprecated ???
#define REGISTRATION_UNKNOWN 0
#define REGISTRATION_APPROX 1
#define REGISTRATION_EXACT 2

//MakeOID-generated:: EEOptions value association
#define EENONE 0
#define EEKEYPRESSER_retired 1
#define EEHDRAIL_retired 2
#define EERIVET_retired 3
#define EECIRCRIVET_retired 4
#define EEDRILLFILL 5
#define EEGSPINDLE 6
#define EEHDDRILLFILL_retired 7
#define EEHDGEN3RAIL 8
#define EECIRCMFT1 9
#define EEFD1 10
#define EEFD2 11
#define EERESERVED 12
#define EEDRILLFILLADV 13
#define EEHDGEN4RAIL 14
#define EEHDGEN4CDRAIL 15
#define EECIRCMFT2 16
#define EEHDGEN5RAIL 17
#define EEFLOORBEAM 18
#define EEDEFAULT 64

//MakeOID-generated:: AxisCode value association
#define AXIS_NULL 0
#define AXIS_X 1
#define AXIS_Y 2
#define AXIS_CLAMP 3
#define AXIS_A 4
#define AXIS_Z 5
#define AXIS_FF 6
#define AXIS_FS 7
#define AXIS_FTA 8

//MakeOID-generated:: HomeStatus value association
#define HOME_NOT_DONE 0
#define HOME_RUNNING 1
#define HOME_FAILURE 2
#define HOME_DONE 3
#define HOME_PENDING 4

//MakeOID-generated:: HomeStatusReason value association
#define HOMESR_NULL 0
#define HOMESR_COM 1
#define HOMESR_SENSOR 2
#define HOMESR_MOTOR 3
#define HOMESR_DISABLED 4
#define HOMESR_UNKNOWN 5

//MakeOID-generated:: ProcessContinueMode value association
#define PROCESS_SINGLE 0
#define PROCESS_CONTINUOUS 1

//MakeOID-generated:: YOrientation value association
#define Y_POS 0
#define Y_NEG 1
#define Y_UNKNOWN 2

//MakeOID-generated:: INSPECTTYPE value association
#define INSPECT_PROBE 0
#define INSPECT_FULL 1
#define INSPECT_FAST 2

//MakeOID-generated:: LSMETHODS value association
#define LSMETHOD_INTENSITY 0
#define LSMETHOD_DELTA 1
#define LSMETHOD_SYMETRIC 2
#define LSMETHOD_GATE 3
#define LSMETHOD_INSTANT 4
#define LSMETHOD_EXTERNAL_BASE 16
#define LSMETHOD_EXTERNAL_SYMETRIC 17
#define LSMETHOD_EXTERNAL_STRATA_MIDS 18
#define LSMETHOD_EXTERNAL_LRMIN 19
#define LSMETHOD_EXTERNAL_LRFIRSTDELTA 20
#define LSMETHOD_EXTERNAL_VALUELR 21

//MakeOID-generated:: DELTAMODES value association
#define DELTA_BASE 1
#define DELTA_BASETOP 2

//MakeOID-generated:: OPERATION_BIT_FIELD_VA value association
#define OP_BASE 0
#define OP_VISIT 1
#define OP_PROBE 2
#define OP_DRILL 4
#define OP_COUNTERSINK 8
#define OP_SEAL 16
#define OP_FILL 32
//#define OP_RIVET 64
#define OP_REMOVE 64
#define OP_INSPECT 128
#define OP_DRILL_ABORT 512
#define OP_DRILL_STARTED 1024
#define OP_DRILL_FAULT 2048
#define OP_DRILL_SUCCESS 4096
#define OP_PROBE_WARNINGS 32768
#define OP_MAXVAL 65535

//MakeOID-generated:: MachineLocks value association
#define NoLock 0
#define XLock 1
#define YLock 2
#define XAndYLock 3

//MakeOID-generated:: POSNDISPLAYMODES value association
#define PD_MACHINE 0
#define PD_DATASET 1
#define PD_DATASET_NS 2
#define PD_NOTHING 4

//MakeOID-generated:: POSNDISPLAYORIGIN value association
#define PD_ZERO 0
#define PD_PK 1
#define PD_SK 2
#define PD_NEAREST 3

//MakeOID-generated:: POSNDISPLAYCONTENT value association
#define PD_XY 0
#define PD_DIST 1
#define PD_IDS 2

//MakeOID-generated:: ToolVerifyEnable value association
#define TOOLVERIFY_OFF 0
#define TOOLVERIFY_PRE 1
#define TOOLVERIFY_ARRIVE 2
#define TOOLVERIFY_ACTION 3

//MakeOID-generated:: PartPgmStatus value association
#define PP_NOTLOADED 0
#define PP_LOADOK 1
#define PP_NOTFOUND 2
#define PP_PARSEFAIL 3
#define PP_LOADING 4

//MakeOID-generated:: PartPgmLocked value association
#define PARTPGM_NOT_LOCKED 0
#define PARTPGM_LOCKED 1

//MakeOID-generated:: GravcompStatus value association
#define GRAVCOMP_NOTDONE 0
#define GRAVCOMP_RUNNING 1
#define GRAVCOMP_PASS 2
#define GRAVCOMP_FAILX 3
#define GRAVCOMP_FAILY 4

//MakeOID-generated:: GravCompFloat value association
#define FLOAT_TOGGLE 0
#define FLOAT_FLOAT 1
#define FLOAT_UNFLOAT 2
#define FLOAT_UNFLOAT_STOP 3

//MakeOID-generated:: GravCompAlgorithm value association
#define GC_ORIGINAL 1
#define GC_FILTERED 2
#define GC_SHORT 3
#define GC_IMMEDIATE 4
#define GC_DRIFT 5

//MakeOID-generated:: FloatStatus value association
#define FLOATSTAT_NOFLOAT 0
#define FLOATSTAT_FLOAT 1

//MakeOID-generated:: MoveType value association
#define MOVETYPE_ORIGINAL 1
#define MOVETYPE_DIRECT 2
#define MOVETYPE_FAST 3
#define MOVETYPE_ROUGH 4
#define MOVETYPE_VROUGH 5

//MakeOID-generated:: MoveDone value association
#define MOVEDONE_FALSE 0
#define MOVEDONE_TRUE 1
#define MOVEDONE_ERROR 2
#define MOVEDONE_STOP 3


//MakeOID-generated:: AutoMove value association
#define AUTOMOVE_OFF 0
#define AUTOMOVE_MOVE 1
#define AUTOMOVE_ACTION 2

//MakeOID-generated:: AutoRepeat value association
#define NO_AUTOREPEAT 0
#define AUTOREPEAT 1

//MakeOID-generated:: Station value association
#define STATION_UNSPEC 0
#define STATION_DRILL 1
#define STATION_SEAL 2
#define STATION_PICKUP 3
#define STATION_FILL 4
#define STATION_MOVING 5
#define STATION_INSPECT 6
#define STATION_LASERPOINTER 7
#define STATION_RESERVED8 8
#define STATION_RESERVED9 9


//MakeOID-generated:: VisionInfo value association
#define VisionInfoPosition 1
#define VisionInfoPositionExpected 2
#define VisionInfoDiameter 4
#define VisionInfoCountersink 8
#define VisionInfoPositionProbeEdgeVec 16

//MakeOID-generated:: RFIDSEEKMETHODS value association
#define FIRST_DETECTION 1		// (complete at first sight.)
#define CENTER_FAST 2		// (complete at first start and first finish.)
#define CENTER_1PASS 3		// (complete after 1 st fine speed start and finish.)
#define CENTER_2PASS 4		// (complete after 2 fine passes going oposite directions.)

//MakeOID-generated:: RFIDOPTIONS value association
#define RFID_OPTION_BASE 0
#define RFID_OPTION_REVERSE_ON_HARDSTOP 1
#define RFID_OPTION_ALERT_PROGRESS_DATA 2
#define RFID_OPTION_READS_UPPER_TAG 4
#define RFID_OPTION_MAX 7

//MakeOID-generated:: RFIDSTATE value association
#define RFID_INIT 0
#define RFID_NOT_SUPPORTED 1		// MiniFT System Build Does not support RFID
#define RFID_NOT_ENABLED 2
#define RFID_NO_CONNECTION 3
#define RFID_NOT_PRESENT 4		// (no tag found and no specific error reported... RFID can't see anything, but is working properly)
#define RFID_PRESENT 5		// (tag is present.  In the case of seeking, this means it's complete also)
#define RFID_ERROR 6		// (read attempt failed for some reason.)

//MakeOID-generated:: RFIDCONTEXT value association
#define RFID_CONTEXT_READ 0
#define RFID_CONTEXT_SEEK 1

//MakeOID-generated:: RFIDSEEKSTATE value association
#define RFID_SEEK_NULL 0		// OID was created to show the result of a READ, and there is no SEEK status.
#define RFID_SEEK_INIT 1		// (seek commanded, but nothing else done yet)
#define RFID_SEEK_MOVE1 2		// (move 1 pending)
#define RFID_SEEK_MOVE2 3		// (move 2 pending)
#define RFID_SEEK_MOVEOFF 4		// (move off pending)
#define RFID_SEEK_FINE1 5		// (fine move 1 pending)
#define RFID_SEEK_FINE2 6		// (fine move 2 pending)
#define RFID_SEEK_RETURN 7		// (return to start move)
#define RFID_SEEK_CENTER 8		// (center on tag center)
#define RFID_SEEK_TERMINAL_STATE 9		// (used for checking if seekstate is terminal: all states from here down are terminal states)
#define RFID_SEEK_SUCCESS 10		// (ended seek operation)
#define RFID_SEEK_FAIL_INIT 11		// (either not supported, not enabled, or no connection. See state field)
#define RFID_SEEK_FAIL_NOT_FOUND 12		// (was able to follow all the specified motions, but never found a tag)
#define RFID_SEEK_FAIL_NOT_LOCATED 13		// (was able to follow all the specified motions, found a tag, but couldn'tisolate the location according to the specified motion requirements.)
#define RFID_SEEK_FAIL_HARDSTOP 14		// (ended after following specified motion options with the last eventbeing a hardstop.)
#define RFID_SEEK_FAIL_RFID_ERROR 15		// (encountered errors from the RFID reader while looking for the tag.It might continue looking for a short while, but if it never finds a validtag then it will result in this state
#define RFID_SEEK_FAIL_BAD_TAG_DATA 16		// (found tag, but tag did not conform to format or rules.)

//MakeOID-generated:: RFIDREAD value association
#define RFID_READ_OFF 0
#define RFID_READ_NOW 1
#define RFID_READ_CONTINUOUS 2
#define RFID_READ_SEEK 3
#define RFID_READ_STOP 4

//MakeOID-generated:: ProbeFlags value association
#define AUTO_MOVE_PROBE 1
#define AUTO_COMPLETE 2

#define MAX_FILENAME_LEN 128
//#define MAX_STRING_LEN 32
//MakeOID-generated::END

//Now From Tool Module
//MakeOID-generated:: VALUE DEFINES (complete)

//MakeOID-generated:: TOOL_SYSTEM_OPERATIONS value association
#define operation_null 0
#define register_tool_server 1
#define register_tool_client 2
#define search 3
#define search_result 4
#define search_result_rec 5
#define search_failure 6
#define update 7
#define update_failure 8
#define load_request 9
#define load_success 10
#define load_failure 11
#define load 12
#define unload 13
#define improper_type 14
#define error_report 15
#define operation_num_values 16

//MakeOID-generated:: TOOL_SYSTEM_SEARCH_CODES value association
#define search_not_set 0
#define required_tool_type 1
#define tool_type 2
#define tool_id 3
#define complete 4
#define list_tool_types 5
#define search_num_values 6

//MakeOID-generated:: TOOL_SYSTEM_RESULT_CODES value association
#define result_not_set 0
#define no_tool_server 1
#define tool_server_error 2
#define tool_not_found 3
#define bad_arg_1 4
#define bad_arg_2 5
#define bad_arg_3 6
#define result_num_values 7

//MakeOID-generated:: TOOL_STATUS value association
#define TSTAT_NOT_SET 0
#define TSTAT_ACTIVE 1
#define TSTAT_END_OF_LIFE 2
#define TSTAT_BROKEN 3
#define TSTAT_MISSING 4
#define TSTAT_NEEDS_INSPECTION 5
#define TSTAT_LOCAL 6
#define TSTAT_NUM_VALUES 7

//MakeOID-generated::END

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

//individual category flags
#define OID_CATEGORYBIT_CONFIG   0x01
#define OID_CATEGORYBIT_READ 0x02
#define OID_CATEGORYBIT_WRITE 0x04

//quick flag format for setting the values
#define OID_CATEGORY_CRW   0x07
#define OID_CATEGORY_RW    0x06
#define OID_CATEGORY_R     0x02

//Since many features of GEN20 and GEN25 are common define a common define
#ifdef GEN20
#define GEN20OR25
#endif
#ifdef GEN25
#define GEN20OR25
#endif

// MiniFt And HD Rabbit DIO and AIO and code labels

//
// Outputs and Inputs
// keywords: DIO DIGIN DIGOUT ANALOG ANAIN ANAOUT
//

// Always Sourcing
#define DIGOUT_CONFIG 0xFFFF  // dig out #0 - #15...if bit=1 it is sourcing...if bit=0 it is sinking

//Note About Banks:
//Outputs OUT00 to OUT07 are driven by bank1 +K_1
//Outputs OUT08 to OUT15 are driven by bank1 +K_2
//On some older units K_1 or K_2 is +5V, and the other is +24V
//On RivetMotherboard1, bank1 is 24v, but bank2 is 5v to drive relays.
//Those relays in turn may drive 24v.

//GEN20 IO
//GEN25 IO
#ifdef GEN20OR25
//bank1 +5V K_1
#define DIGOUT_CHNUM_LED 0  // physical channel #0
#define DIGOUT_CHNUM_BEEPER 1  // physical channel #1
//NOT USED					2
#define DIGOUT_CHNUM_LASER 4  // physical channel #4
#define DIGOUT_CHNUM_TIME_SIGNAL 5
//bank2 +24V K2
#define DIGOUT_CHNUM_BRAKE 8 // Brake when under minift rabbit control
//Special GEN20 Rivet Prototype
#define DIGOUT_CHNUM_LEG_EXTEND 10
#define DIGOUT_CHNUM_RIVET_CLAMP 11
#define DIGOUT_CHNUM_LEG_LOCK 12
#define DIGOUT_CHNUM_PRELOAD_CLAMP 13
#define DIGOUT_CHNUM_ACTIVE_LED 14
#define DIGOUT_CHNUM_RIVET_HAMMER 15
//For EEOption1 use this output that goes to the right pin.
#define DIGOUT_CHNUM_KEYPRESSER 10

// physical digital input channels (IN00 = 0 through IN23 = 23 for the BL2100)
#define DIGIN_CHNUM_ESTOP 0         // physical channel IN00...monitors motor power supply
#define DIGIN_CHNUM_UNIBUTTON 1         // physical channel IN01
#define DIGIN_CHNUM_ESTOP_SWITCH 2        // physical channel IN02...monitors physical ESTOP switch on lid of MiniFT

#ifdef WHISTLEMC
#define DIGIN_CHNUM_WHISTLEX_MSG 3         // physical channel IN03 Detect X Whistle Signal
#define DIGIN_CHNUM_WHISTLEY_MSG 4         // physical channel IN04 Detect Y Whistle Signal
#endif

#define IO_MAPPED
#endif

#if 0
//FIXME0 DFINT move the above outs once that's fixed
#ifdef SEAL_SYSTEM
#define DIGOUT_CHNUM_SEALANT_APPLY 9
#endif
#ifdef SEAL_SYSTEM
#define DIGOUT_CHNUM_SEALANT_PRESSURE 12
#define DIGOUT_CHNUM_SEALANT_PINCH 13
#endif
//FIXME0 bring over GENHD2 code for air blast
#define DIGOUT_CHNUM_AIR_BLAST 14
//#define DIGOUT_CHNUM_LUBE 15
//If used, need assignment
//	#define DIGOUT_CHNUM_TIME_SIGNAL 15
//	#define DIGOUT_CHNUM_KEYPRESSER 15

#endif

//GENHD3 IO
#ifdef GENHD3
//#define DIGOUT_CHNUM_LED 0
//#define DIGOUT_CHNUM_BEEPER 1
#define DIGOUT_CHNUM_BRAKE 2

// physical digital input channels (IN00 = 0 through IN23 = 23 for the BL2100)
#define DIGIN_CHNUM_ESTOP 0         // physical channel IN00...monitors motor power supply
#define DIGIN_CHNUM_UNIBUTTON 1         // physical channel IN01
#define DIGIN_CHNUM_ESTOP_SWITCH 2        // physical channel IN02...monitors physical ESTOP switch on lid of MiniFT

//DIGIN_CHNUM_YHOME NOT USED currently
#define DIGIN_CHNUM_YHOME 8
#define DIGIN_CHNUM_YORIENTATION_A 6
#define DIGIN_CHNUM_YORIENTATION_B 7

#ifdef WHISTLEMC
#define DIGIN_CHNUM_WHISTLEX_MSG 9        // physical channel IN09 Detect X Whistle Signal
#define DIGIN_CHNUM_WHISTLEY_MSG 10         // physical channel IN10 Detect Y Whistle Signal
#endif

//Obstruction Sensors
#define OBSTRUCTION_SYSTEM_XP1
#define OBSTRUCTION_SYSTEM_XN1
#define DIGIN_OBSTRUCTION_XP1 17
#define DIGIN_OBSTRUCTION_XN1 16

//analog
#define ADC_CHANNEL_ANALOG_POINT_LASER ADC_CHANNEL_00

#define IO_MAPPED
#endif

//GENHD4 IO
#ifdef GENHD4
//#define DIGOUT_CHNUM_LED 0
//#define DIGOUT_CHNUM_BEEPER 1
#define DIGOUT_CHNUM_BRAKE 2

// physical digital input channels (IN00 = 0 through IN23 = 23 for the BL2100)
//#define DIGIN_CHNUM_ESTOP 0
//#define DIGIN_CHNUM_UNIBUTTON 1
#define DIGIN_CHNUM_ESTOP_SWITCH 2    //monitors logical ESTOP signal.

#define DIGIN_CHNUM_YLOCK 5
//#define DIGIN_CHNUM_YHOME 6 //goes to whistle
#define DIGIN_CHNUM_YORIENTATION_A 7
#define DIGIN_CHNUM_YORIENTATION_B 8

#ifdef WHISTLEMC
#define DIGIN_CHNUM_WHISTLEX_MSG 9        // physical channel IN09 Detect X Whistle Signal
#define DIGIN_CHNUM_WHISTLEY_MSG 10         // physical channel IN10 Detect Y Whistle Signal
#endif

//NOT UNTIL TESTED
//#define DIGIN_SOFTSTOP 11

//Actually not mapped
#define DIGIN_FASTENERSENSOR 15

//Obstruction Sensors
#define OBSTRUCTION_SYSTEM_XP1
#define OBSTRUCTION_SYSTEM_XN1
#define DIGIN_OBSTRUCTION_XP1 16
#define DIGIN_OBSTRUCTION_XN1 17

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
//#define DIGOUT_CHNUM_LED 0
//#define DIGOUT_CHNUM_BEEPER 1
#define DIGOUT_CHNUM_BRAKE 2
#define DIGOUT_CHNUM_BRAKE_ALTERNATE 3

// physical digital input channels (IN00 = 0 through IN23 = 23 for the BL2100)
//#define DIGIN_CHNUM_ESTOP 0
//#define DIGIN_CHNUM_UNIBUTTON 1
#define DIGIN_CHNUM_ESTOP_SWITCH 2    //monitors logical ESTOP signal.

#define DIGIN_CHNUM_XLOCK 4
#define DIGIN_CHNUM_YLOCK 5

#define DIGIN_CHNUM_YORIENTATION_A 7
#define DIGIN_CHNUM_YORIENTATION_B 8

#ifdef WHISTLEMC
#define DIGIN_CHNUM_WHISTLEX_MSG 9        // physical channel IN09 Detect X Whistle Signal
#define DIGIN_CHNUM_WHISTLEY_MSG 10         // physical channel IN10 Detect Y Whistle Signal
#endif

#define DIGIN_RFID_PRESENT 11

//Actually not mapped
//#define DIGIN_FASTENERSENSOR 15

//Obstruction Sensors
#define OBSTRUCTION_SYSTEM_XP1
#define OBSTRUCTION_SYSTEM_XN1
#define DIGIN_OBSTRUCTION_XP1 16
#define DIGIN_OBSTRUCTION_XN1 17

//analog
//#define ADC_CHANNEL_ANALOG_POINT_LASER ADC_CHANNEL_00

#ifdef FORCELIMITING
#fatal "HD5 does not support this at this time"
#define ADC_FORCE_SENSOR_X ADC_CHANNEL_01
#define ADC_FORCE_SENSOR_Y ADC_CHANNEL_02
#define ADC_FORCE_SENSOR_Z ADC_CHANNEL_03
#endif

#define IO_MAPPED
#endif

//GENFD IO
#ifdef GENFD
//#define DIGOUT_CHNUM_LED 0
//#define DIGOUT_CHNUM_BEEPER 1
#define DIGOUT_CHNUM_BRAKE 2
// physical digital input channels (IN00 = 0 through IN23 = 23 for the BL2100)
#define DIGIN_CHNUM_ESTOP_SWITCH 2        // physical channel IN02...monitors physical ESTOP switch on lid of MiniFT

#ifdef WHISTLEMC
#define DIGIN_CHNUM_WHISTLEX_MSG 9        // physical channel IN09 Detect X Whistle Signal
#define DIGIN_CHNUM_WHISTLEY_MSG 10         // physical channel IN10 Detect Y Whistle Signal
#endif

#endif

//GENFLOORBEAM IO
#ifdef GENFLOORBEAM
//#define DIGOUT_CHNUM_LED 0
//#define DIGOUT_CHNUM_BEEPER 1
#define DIGOUT_CHNUM_BRAKE 2

//New in Floorbeam
//Also known as ChipClearXY
#define DIGOUT_CHNUM_AIRBLASTX 6
#define DIGOUT_CHNUM_AIRBLASTY 7
//Laser on 10
#define DIGOUT_CHNUM_LASER_POINTER 10

// physical digital input channels (IN00 = 0 through IN23 = 23 for the BL2100)
//#define DIGIN_CHNUM_ESTOP 0
//#define DIGIN_CHNUM_UNIBUTTON 1
#define DIGIN_CHNUM_ESTOP_SWITCH 2    //monitors logical ESTOP signal.

//#define DIGIN_CHNUM_YLOCK 5
//#define DIGIN_CHNUM_YHOME 6 //goes to whistle

#ifdef WHISTLEMC
#define DIGIN_CHNUM_WHISTLEX_MSG 9        // physical channel IN09 Detect X Whistle Signal
#define DIGIN_CHNUM_WHISTLEY_MSG 10         // physical channel IN10 Detect Y Whistle Signal
#endif

#define DIGIN_RFID_TAG_DETECT 11

//Obstruction Sensors
#define OBSTRUCTION_SYSTEM_XP1
#define OBSTRUCTION_SYSTEM_XN1
#define DIGIN_OBSTRUCTION_XP1 16
#define DIGIN_OBSTRUCTION_XN1 17

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
#define DIGOUT_CHNUM_LED 0
#define DIGOUT_CHNUM_BEEPER 1
#define DIGOUT_CHNUM_BRAKE 2
#define DIGOUT_CHNUM_DRILLUP 3
#define DIGOUT_CHNUM_DRILLDOWN 4
#define DIGOUT_CHNUM_DRILLBUTTON 5
#define DIGOUT_CHNUM_COLLETCLAMP 6
// physical digital input channels (IN00 = 0 through IN23 = 23 for the BL2100)
#define DIGIN_CHNUM_ESTOP 0         // physical channel IN00...monitors motor power supply
#define DIGIN_CHNUM_UNIBUTTON 1         // physical channel IN01
#define DIGIN_CHNUM_ESTOP_SWITCH 2        // physical channel IN02...monitors physical ESTOP switch on lid of MiniFT
#define DIGIN_CHNUM_DRILLUP	5
//#define DIGIN_CHNUM_Y_HOME 6 //should be rewired to whistle
#warnt "SEVERE  Rewire to whistle"
#define DIGIN_CHNUM_Y_POS_LIMIT 7
#define DIGIN_CHNUM_Y_NEG_LIMIT 8
#ifdef WHISTLEMC
#define DIGIN_CHNUM_WHISTLEX_MSG 9         // Detect X Whistle Signal
#define DIGIN_CHNUM_WHISTLEY_MSG 10         // Detect Y Whistle Signal
#endif
#define DIGIN_CHNUM_AIR_PRESSURE 11         // physical channel IN11
#define IO_MAPPED
#endif

//GENCIRCMFT2 IO
#ifdef GENCIRCMFT2
//BL2100 Gen4 Style Design
#define DIGOUT_CHNUM_BEEPER 0
#define DIGOUT_CHNUM_LED 1
#define DIGOUT_CHNUM_BRAKE 2
//DIGOUT_CHNUM_DRILLUP DIGOUT_CHNUM_DRILLUP
//remove UP  and make this IO Control Logic...
//#define DIGOUT_CHNUM_DRILLUP 3
#define DIGOUT_CHNUM_DRILLDOWN 4
#define DIGOUT_CHNUM_DRILLBUTTON 5
#define DIGOUT_CHNUM_COLLETCLAMP 6
// physical digital input channels (IN00 = 0 through IN23 = 23 for the BL2100)
//#define DIGIN_CHNUM_ESTOP 0
//#define DIGIN_CHNUM_UNIBUTTON 1
#define DIGIN_CHNUM_ESTOP_SWITCH 2    //monitors logical ESTOP signal.
//REMOVED #define DIGIN_CHNUM_CUTTER_DETECTED 4
#define DIGIN_CHNUM_DRILLUP	6
//REMOVED #define DIGIN_CHNUM_DRILLDOWN 5
#ifdef WHISTLEMC
#define DIGIN_CHNUM_WHISTLEX_MSG 9         // Detect X Whistle Signal
#define DIGIN_CHNUM_WHISTLEY_MSG 10         // Detect Y Whistle Signal
#endif
//#define DIGIN_CHNUM_AIR_PRESSURE 11         // physical channel IN11

#define OBSTRUCTION_SYSTEM_XP1
#define OBSTRUCTION_SYSTEM_XN1
#define DIGIN_OBSTRUCTION_XP1 16
#define DIGIN_OBSTRUCTION_XN1 17

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

///Done Machine IO Mapping

///Some Older Code Extracted From specific machine setup
///could be brought back in if neded
//OLD SEAL AND INSERT IO
//#warnt "Much of the old dfint notes are outdated.<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<"
//FIXME0 DFINT move the above outs once that's fixed
//#ifdef SEAL_SYSTEM
//#define DIGOUT_CHNUM_SEALANT_APPLY 9
//#endif
//#ifdef SEAL_SYSTEM
//#define DIGOUT_CHNUM_SEALANT_PRESSURE 12
//#define DIGOUT_CHNUM_SEALANT_PINCH 13
//#endif

//#define DIGOUT_CHNUM_AIR_BLAST 14
//#define DIGOUT_CHNUM_LUBE 15

//If used, need assignment
//	#define DIGOUT_CHNUM_TIME_SIGNAL 15
//	#define DIGOUT_CHNUM_KEYPRESSER 15
//End older code for outputs that could be brought back if needed
//Actually no mapped
//#define DIGIN_FASTENERSENSOR 5


#define DIGIN_DEBOUNCE_HISTORY_COUNT 8
#define DIGIN_XLOCK_DEBOUNCE_HISTORY_COUNT 32
#define DIGIN_IMMEDIATE_COUNT 1

#ifndef IO_MAPPED
#fatal "IO_MAPPED was not defined"
#endif

//
// Analog Inputs
//
// ADCIN00	XTHERM
// ADCIN01	YTHERM
//

#define ADC_CHANNEL_00	0  // physical ADC channel # (ADC00)
#define ADC_CHANNEL_01	1  // physical ADC channel # (ADC01)
#define ADC_CHANNEL_02	2  // physical ADC channel # (ADC02)
#define ADC_CHANNEL_03	3  // physical ADC channel # (ADC03)
#define ADC_CHANNEL_04	4  // physical ADC channel # (ADC04)
#define ADC_CHANNEL_05	5  // physical ADC channel # (ADC05)
#define ADC_CHANNEL_06	6  // physical ADC channel # (ADC05)
#define ADC_CHANNEL_07	7  // physical ADC channel # (ADC05)
#define ADC_CHANNEL_08	8  // physical ADC channel # (ADC05)
#define ADC_CHANNEL_09	9  // physical ADC channel # (ADC05)

#ifdef READ_TEMP
#define ADC_CHANNEL_TEMPX	ADC_CHANNEL_00
#define ADC_CHANNEL_TEMPY	ADC_CHANNEL_01
#warnt "FATAL"
#endif

#ifdef READMOTORVOLTAGE
#define ADC_CHANNEL_MOTORVOLTAGE ADC_CHANNEL_02
#define ADC_CHANNEL_02_USED
#endif

#ifdef CENTERVISION_ANALOG_POINT_LASER
#ifndef ADC_CHANNEL_ANALOG_POINT_LASER
#define ADC_CHANNEL_ANALOG_POINT_LASER ADC_CHANNEL_02
#ifdef ADC_CHANNEL_02_USED
#fatal "ERROR: You have specified incompatible uses for ADC CHANNEL 2."
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

//	TIMERB support
//#define USE_TIMERB
#ifdef USE_TIMERB
#define TIMERB_TICK_USEC	67
#define TIMERB_MAX_MSEC 	2000
#endif

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





//LIBRARY INCLUDES

//SocketConsole
#include "SocketConsole.h"




#ifdef WHISTLEMC
#use "MiniFtWhistle.lib"
#endif

//STP LIB
//Use STP LIB Here now that all needed libs are included
#define STP_TXMSG_TIMEOUT_MS 1000
#define STP_RXMSG_DELAY_TIMEOUT_MS 1500
#warnt "4 sec timeout being used"
#define STP_RXMSG_CLOSE_TIMEOUT_MS 4000
#define STP_STREAMING_OID_MODE
#define STP_RX_REPEAT_READ
#define STP_RX_MAX_REPEAT_READS 20
#define STP_RX_MAX_REPEAT_READ_MS 50
#define DO_NOT_USE_NTOHF
#define EXCLUDE_OID_POSNMODE_CURXY
#define EXCLUDE_OID_VISION_DATA
#use "STP.lib"

//STP Rail LIB
#ifdef HD_RAIL_STP
#use "MiniFtRailSTP.lib"
#endif

#define USE_BOOTLOADER

#ifdef USE_BOOTLOADER
#define UDPDL_LOADER "../../STutils/EthernetLoader/PDL-Generic.bin"  //for ethernet bootloader
#use udpdownl.lib   // for ethernet bootloader
#endif

//MD5 for PP checking
#define MD5HASH 1
#ifdef MD5HASH
#use "MD5.LIB"
#endif

//RFID Lib
#ifdef USE_RFID_OMRON
#define RFID_OMRON_USE_LOGF
#define RFID_SERVICE_PERIOD_MS		35
#define OMRON_USE_PORTD
//#use "OMRON_RFID_SYSTEM_FOR_DC9XX.LIB"
#use "OMRON_RFID_SYSTEM_FOR_MFTDC9.LIB"
#endif

//RFID Lib
#ifdef USE_RFID_MIFARE
//#define RFID_SERVICE_PERIOD_MS		35
#define SL031_USE_LOGF
#define SL031_USE_PORTD
#use "STRONGLINKSL031SERIAL.LIB"
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
	char cFormat;
	char cRailType;
	char cGroup;
	char cSegmentAndRailSide;
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

//When Updating Structures frmo MakeOID
//remove Common OID Def structures and place them into the STP lib.

//MakeOID-generated:: TYPEDEFS (oid complete except for common)

typedef struct {
	unsigned int uiOID;
	unsigned int uiCode;
} td_oid_minift_message_code;

typedef struct {
	float fX;
	float fY;
} td_EncoderRatio;

typedef struct {
	float fX;
	float fY;
} td_MCCurrentLimit;

typedef struct {
	float fX;
	float fY;
} td_MCPeakCurrentLimit;

typedef struct {
	float fxtemp;
	float fytemp;
	unsigned int uiloop_avg_ms;
} td_oid_system_monitor;

typedef struct {
	char cProcess;
	char cToolType;
	char cFastenerType;
	char cLayers;
	char cCountersink;
	unsigned int uiOperations;
	float fDiameter;
	float fProcessCountersink;
	float fHoleCountersinkAdjust;
	float flayer1;
	float flayer2;
	float flayer3;
	float flayer4;
	float flayer5;
	float flayer6;
	float flayer7;
	float flayer8;
	char cmat1;
	char cmat2;
	char cmat3;
	char cmat4;
	char cmat5;
	char cmat6;
	char cmat7;
	char cmat8;
} td_HoleParam;

typedef struct {
	float fX;
	float fY;
} td_GravCompSpeed;

typedef struct {
	float fX;
	float fY;
} td_GravCompAcc;

typedef struct {
	float fX;
	float fY;
} td_GravCompDec;

typedef struct {
	float fX;
	float fY;
} td_GravCompMoveDist;

typedef struct {
	float fX;
	float fY;
} td_GravCompNoiseLimit;

typedef struct {
	float fX;
	float fY;
} td_GravCompTriggerFactor;

typedef struct {
	float fxp;
	float fxn;
	float fyp;
	float fyn;
} td_GravCompResults;

typedef struct {
	float fX;
	float fY;
} td_ProbeOffset;

typedef struct {
	float fexpected;
	float ffound;
} td_oid_khole_distance;

typedef struct {
	char ccode;
	char cKIndex;
} td_oid_probe;

typedef struct {
	float fX;
	float fY;
} td_oid_probe_position;

typedef struct {
	char cKIndex;
	char cStatus;
	char cMethod;
	float fX;
	float fY;
} td_oid_probe_status;

typedef struct {
	char cKIndex;
	char cStatus;
	char cMethod;
	float fX;
	float fY;
} td_oid_probe_start;

typedef struct {
	float fX;
	float fY;
} td_HomeSpeed;

typedef struct {
	float fX;
	float fY;
} td_HomeAcc;

typedef struct {
	float fX;
	float fY;
} td_HomeDec;

typedef struct {
	float fX;
	float fY;
} td_HomeMoveDist;

typedef struct {
	float fX;
	float fY;
} td_HomeFineSpeed;

typedef struct {
	char caxis_code;
	char cstatus;
	char cstatus_reason;
} td_oid_home_report;

typedef struct {
	float fDataSetX;
	float fDataSetY;
	float fMachineX;
	float fMachineY;
} td_oid_posnmode_curxy;

typedef struct {
	float fDataSetX;
	float fDataSetY;
	float fMachineX;
	float fMachineY;
	int iNearPosn;
} td_oid_posnmode_nearxy;

typedef struct {
	float fX;
	float fY;
} td_PreMove;

typedef struct {
	float fX;
	float fY;
} td_PosnSpeed;

typedef struct {
	float fX;
	float fY;
} td_PosnAcc;

typedef struct {
	float fX;
	float fY;
} td_PosnDec;

typedef struct {
	float fX;
	float fY;
} td_PosnFinalSpeed;

typedef struct {
	float fX;
	float fY;
} td_PosnErrLimit;

typedef struct {
	float fVLimitMarginX;
	float fVErrorX;
	float fVLimitMarginY;
	float fVErrorY;
} td_VelErrLimit;

typedef struct {
	float fX;
	float fY;
} td_LongDistance;

typedef struct {
	float fX;
	float fY;
} td_LongSpeed;

typedef struct {
	float fX;
	float fY;
} td_MachineOffset;

typedef struct {
	float fX;
	float fY;
	float fYExtension;
} td_MachineOffset1;

typedef struct {
	float fX;
	float fY;
	float fYExtension;
} td_MachineOffset2;

typedef struct {
	float fX;
	float fY;
} td_oid_tool_offset;

typedef struct {
	float fX;
	float fY;
	float fYExtension;
} td_DrillOffset1;

typedef struct {
	float fX;
	float fY;
	float fYExtension;
} td_DrillOffset2;

typedef struct {
	float fx;
	float fy;
} td_OffsetSeal;

typedef struct {
	float fx;
	float fy;
} td_OffsetFill;

typedef struct {
	float fX;
	float fY;
} td_oid_jog;

typedef struct {
	float fX;
	float fY;
} td_JogSpeed;

typedef struct {
	float fX;
	float fY;
} td_JogAcc;

typedef struct {
	float fX;
	float fY;
} td_JogDec;

typedef struct {
	float fX;
	float fY;
} td_JogFactor;

typedef struct {
	float fX;
	float fY;
} td_HomePosnYPos;

typedef struct {
	float fMinX;
	float fMaxX;
	float fMinY;
	float fMaxY;
} td_PosnLimitYPos;

typedef struct {
	float fX;
	float fY;
} td_HomePosnYNeg;

typedef struct {
	float fMinX;
	float fMaxX;
	float fMinY;
	float fMaxY;
} td_PosnLimitYNeg;

typedef struct {
	float fX;
	float fY;
} td_LaserSensorOffset;

typedef struct {
	float fX;
	float fY;
} td_CamOffset;

typedef struct {
	unsigned int uiSequence;
	unsigned int uiFlags;
	unsigned int uiWidth;
	unsigned int uiHeight;
	float fX;
	float fY;
	float fDiameter;
	float fXPixels;
	float fYPixels;
	float fDiameterPixels;
	float fPixelsPerInch;
	char cEdgeStatus;
	char cEdgeNote;
	float fX1;
	float fY1;
	float fX2;
	float fY2;
	float fCX1;
	float fCY1;
	float fCX2;
	float fCY2;
	float fX1Pixels;
	float fY1Pixels;
	float fX2Pixels;
	float fY2Pixels;
	float fCX1Pixels;
	float fCY1Pixels;
	float fCX2Pixels;
	float fCY2Pixels;
} td_oid_vision_image;

typedef struct {
	char cContext;
	char cStatus;
	char cMethod;
	char cInfo;
	long lposn;
	float fXPosition;
	float fYPosition;
	float fXPositionExpected;
	float fYPositionExpected;
	float fDiameterExpected;
	float fDiameter;
	float fAlgDiameter;
	char cResultMessage;
	float fCountersinkExpected;
	float fCountersinkDepth;
	char cCountersinkAccepted;
	float fNEdgeHeight;
	float fPEdgeHeight;
	float fNCsnkEdgeHeight;
	float fPCsnkEdgeHeight;
	unsigned int uiImageSequence;
	float finspecttime;
	char cEdgeStatus;
	char cEdgeNote;
	float fEdgeX1;
	float fEdgeY1;
	float fEdgeX2;
	float fEdgeY2;
	float fCEdgeX1;
	float fCEdgeY1;
	float fCEdgeX2;
	float fCEdgeY2;
	float fXPositionPixels;
	float fYPositionPixels;
	float fXPositionExpectedPixels;
	float fYPositionExpectedPixels;
	float fEdgeX1Pixels;
	float fEdgeY1Pixels;
	float fEdgeX2Pixels;
	float fEdgeY2Pixels;
	float fCEdgeX1Pixels;
	float fCEdgeY1Pixels;
	float fCEdgeX2Pixels;
	float fCEdgeY2Pixels;
} td_VisionInspectResults;

typedef struct {
	float fsearch_speed;
	float fseek_speed;
	float frscan_speed;
	float frscan_speed_fast;
	float fscan_speed;
	float fscan_speed_fast;
	float fprobe_diameter;
	float funknown_diameter;
	char cmode;
	char cmode_fast;
	char cuse_avg;
	char cfull_scan;
	char cgdata_sel;
	char cassume_posn;
	char cassume_posn_fast;
	char crect_center;
	char cloops;
	char cdelta_mode;
	int idelta_flat;
	float fdelta_basespan;
	int idelta_pos;
	int idelta_neg;
	float fdelta_span;
	float fdelta_edge;
	float fpc_aspect_diff;
	float fmax_aspect_diff;
	float fmax_over_exp_diameter;
	float fmax_under_exp_diameter;
	float fmax_csnk_diff;
	float fmax_over_csnk;
	float fmax_under_csnk;
} td_LaserSensorAlgParam;

typedef struct {
	float fmove_speed;
	char cInfoMask;
	char cAMode;
	char cCMode;
	char cAux1;
	float fmove_required;
	float fmax_over_exp_diameter;
	float fmax_under_exp_diameter;
	float fmax_csnk_diff;
	float fmax_over_csnk;
	float fmax_under_csnk;
} td_CamAlgParam;

typedef struct {
	float fdiameter1;
	int idelay1;
	float fdiameter2;
	int idelay2;
	float fdiameter3;
	int idelay3;
	float fdiameter4;
	int idelay4;
	float fdiameter5;
	int idelay5;
} td_PrimeDelay;

typedef struct {
	char cDevice;
	char cSeverity;
	long lFaultCode;
} td_FastenerFault;

typedef struct {
	float fX;
	float fY;
} td_oid_posnmode_active_premovexy;

typedef struct {
	char cDevice;
	char cSeverity;
	long lFaultCode;
} td_DrillFault;

typedef struct {
	int iHoleNumber;
	int iHoleResult;
} td_HoleResultData;

typedef struct {
	float fMachineX;
	float fMachineY;
} td_oid_movexy;

typedef struct {
	char cmode;
	char corigin;
	char ccontent;
} td_PosnDisplay;

typedef struct {
	char cenabled;
	char cmethod;
	unsigned int uioptions;
	unsigned int uicontinuousReadCycleTime;
	unsigned int uiseekReadCycleTime;
	float fseekMove1;
	float fseekMove2;
	float fseekFineMove;
	float fseekSpeed;
	float fseekFineSpeed;
	float fRFIDOffset;
	float fseekPastBorder;
	float fminWindowSize;
	float fmaxWindowSize;
} td_RFIDConfig;

typedef struct {
	char cstate;
	char ccontext;
	char cseekstate;
	unsigned long ultimestamp;
	unsigned long ulrfidtimestamp;
	float fposition;
	//char * sztagdata; //not stored in struct
	unsigned int uicrc16;
	unsigned int uiendcode;
	unsigned long ulseektime;
	float fsstart;
	float fpstart;
	float fpend;
	float fnstart;
	float fnend;
	float fhs1;
	float fhs2;
	float fhsf;
} td_RFIDData;

typedef struct {
	int iZeroX;
	int iZeroY;
	int iZeroZ;
	int iCountsPerGX;
	int iCountsPerGY;
	int iCountsPerGZ;
} td_ForceSensorCalibration;

typedef struct {
	float fX;
	float fY;
	float fZ;
	char cErrFlag;
} td_ForceSensor;

typedef struct {
	unsigned int uiSensorInterval;
	unsigned int uiMinUpdateDelta;
	char cActive;
	char cCurrentUnderMethod;
	unsigned int uiCurrentOverX;
	unsigned int uiCurrentUnderX;
	unsigned int uiCurrentOverY;
	unsigned int uiCurrentUnderY;
	unsigned int uiFullGravX;
	unsigned int uiFullGravY;
	unsigned int uiFlatForceX;
	unsigned int uiFlatForceY;
} td_ForceLimits;

typedef struct {
	unsigned int uim1;
	unsigned int uim2;
	unsigned int uim3;
	unsigned int uim4;
	unsigned int uim5;
	unsigned int uim6;
} td_MOCal;

typedef struct {
	char cscan;
	char cgroup;
	char cdir;
	char cfoundCenter;
	char cfoundEdges;
	char cresult;
	float frangeN;
	float frangeP;
	float fedgeN;
	float fedgeP;
	float ffeatureN;
	float ffeatureP;
	float fcenter;
	float foffsetNP;
	float fdiffNP;
} td_oid_vision_external_analysis;

typedef struct {
	char format[2];
	char segment[1];
	char tb[1];
	char posnTag[6];
	char lenRail[6];
	char SerialNumber[32];
} td_oid_rfid_tag_set;

typedef struct {
	char cDrill;
	char cFastener;
	char cFastenerTray;
	char cAux1;
	char cAux2;
	char cAux3;
	char cAux4;
	char cAux5;
} td_SystemComponents;

typedef struct {
	char cFormat;
	char cRailType;
	char cGroup;
	char cSegment;
	char cRailSide;
	unsigned long ulSerialNumber;
	unsigned long ulPosition;
	unsigned long ulSegmentPosition;
} td_oid_rfid_tag_set2;

//MakeOID-generated::END

//Now From Tool Module
//MakeOID-generated:: TYPEDEFS (oid complete)

typedef struct {
	char coperation;
	char carg1;
	unsigned int uiarg2;
	//char * szsarg3; //not stored in struct
} td_oid_tool_mgmt;

typedef struct {
	char cOperation;
	char cToolStatus;
	char cHardstop;
	float fDiameter;
	float fLength;
	float fMinBreak;
	float fLengthCountersink;
	float fCountersinkAdjust;
	unsigned long ulDTimeTicksMSW;
	unsigned long ulDTimeTicksLSW;
	unsigned int uiDCount;
	unsigned int uiDWarnCount;
	unsigned int uiDLimitCount;
	//char * szID; //not stored in struct
	//char * szToolTypeCode; //not stored in struct
} td_oid_tool_rec;

//MakeOID-generated::END


typedef struct {
	float fX;
	float fY;
} td_XYDistance;

typedef struct {
	char cDevice;
	char cSeverity;
	char cSeverityPrev;
	char cReported;
	long lFaultCode;
	float fPosition;
	int iCyclesOnCutter;
	unsigned long ulDateTime;
	unsigned int uiHoleNumber;
//Ignore these fields.
//	byte CutterType[32];
//	byte ProfileFilename[64];
} td_SmartDrillFault;


// structure for ConfigData
typedef struct
{
//MakeOID-generated:: CONFIG DECLARATIONS (oid complete)
	td_EncoderRatio EncoderRatio;
	td_MCCurrentLimit MCCurrentLimit;
	td_MCPeakCurrentLimit MCPeakCurrentLimit;
	unsigned int uiBrakeOnTimeout;
	char cEEOptionDefault;
	char cbeeper;
	char cToolVerifyEnable;
	float fReturnHeight;
	char cScaleMode;
	char cGravCompAxes;
	td_GravCompSpeed GravCompSpeed;
	td_GravCompAcc GravCompAcc;
	td_GravCompDec GravCompDec;
	td_GravCompMoveDist GravCompMoveDist;
	char cGravCompAlgorithm;
	td_GravCompNoiseLimit GravCompNoiseLimit;
	td_GravCompTriggerFactor GravCompTriggerFactor;
	float fFloatSpeedLimit;
	float fJogSpeedLimit;
	float fMaxSpeedX;
	float fMaxSpeedY;
	char cProbeMethodDefault;
	td_ProbeOffset ProbeOffset;
	int iProbeDir;
	int iDrillDir;
	float fMaxKholeDistanceError;
	float fApproxLocationError;
	char cProbeAcceptRequired;
	td_HomeSpeed HomeSpeed;
	td_HomeAcc HomeAcc;
	td_HomeDec HomeDec;
	td_HomeMoveDist HomeMoveDist;
	float fProbeAdjustLimit;
	td_HomeFineSpeed HomeFineSpeed;
	char cMoveType;
	td_PreMove PreMove;
	td_PosnSpeed PosnSpeed;
	td_PosnAcc PosnAcc;
	td_PosnDec PosnDec;
	td_PosnFinalSpeed PosnFinalSpeed;
	float fOrthoSlope;
	td_PosnErrLimit PosnErrLimit;
	float fPosnTolerance;
	td_VelErrLimit VelErrLimit;
	td_LongDistance LongDistance;
	td_LongSpeed LongSpeed;
	char cDrillHoleOneTime;
	char cToolFlip;
	char cDriveThroughBacklash;
	td_DrillOffset1 DrillOffset1;
	td_DrillOffset2 DrillOffset2;
	td_OffsetSeal OffsetSeal;
	td_OffsetFill OffsetFill;
	td_JogSpeed JogSpeed;
	td_JogAcc JogAcc;
	td_JogDec JogDec;
	td_HomePosnYPos HomePosnYPos;
	td_PosnLimitYPos PosnLimitYPos;
	td_HomePosnYNeg HomePosnYNeg;
	td_PosnLimitYNeg PosnLimitYNeg;
	char cObstructionCodeMask;
	char cMachineLockRequired;
	unsigned int uiALockDelay;
	unsigned int uiAUnlockDelay;
	unsigned int uiLegsLockDelay;
	unsigned int uiLegsUnlockDelay;
	unsigned int uiLegsUpDelay;
	unsigned int uiLegsDownDelay;
	unsigned int uiLowPressureDelay;
	unsigned int uiLowPressureDownDelay;
	unsigned int uiPressureDelay;
	unsigned int uiPressureDownDelay;
	unsigned int uiLowPressure;
	unsigned int uiPressure;
	unsigned int uiAirClear;
	td_LaserSensorOffset LaserSensorOffset;
	td_CamOffset CamOffset;
	td_LaserSensorAlgParam LaserSensorAlgParam;
	td_CamAlgParam CamAlgParam;
	char cVisionAutoInspect;
	char cProcessContinueMode;
	unsigned int uiProcessOperations;
	char cSealClamp;
	int iSealPressureDelay;
	int iSealPressureReleaseDelay;
	td_PrimeDelay PrimeDelay;
	int iSealGlobDelay;
	int iSealApplyDelay;
	//These few are no longer used for fill, so they are just place holders for OIDs... new OIDs could replace these if needed for fill or anything
	char cFillClamp_;
	int iFillExtendDelay_;
	int iFillRamDelay_;
	int iFastenerPostDelay_;
	float fToolZBase;
	td_PosnDisplay PosnDisplay;
	td_RFIDConfig RFIDConfig;
	unsigned int uiEStopClearDelay;
	unsigned int uiDrillButtonDelay;
	char cUseCutterDetect;
	char cJogEnableTimeout;
	unsigned int uiDrillCycleDelay;
	char cInspectMethod;
	char cCommandInspectMethod;
	td_ForceSensorCalibration ForceSensorCalibration;
	td_ForceLimits ForceLimits;
	char cProbeFlags;
//Don't add this on HD4 yet.
#ifdef OBSTRUCTION_SYSTEM_MO_ANA
	td_MOCal MOCal;
#endif
	float fMaxKholeDistanceCheck;
	float fMaxEdgeShiftProbeAccept;
	char cAllowDrillBeyondShiftLimits;
	td_SystemComponents SystemComponents;
//MakeOID-generated::END
} td_ConfigData;


typedef struct
{
    int iDirX;
    int iDirY;
} td_GravComp;

typedef struct
{
	char cCaptureTrig;
    char cHomeTrig;
	float fHomeX; //machine coordinates for Home
	float fHomeY; //machine coordinates for Home
	char cGotHome;
    char cXBumperDirection;
    char cYBumperDirection;
    char cRegistration;
    char cProbeAdjustTrig;
	float fProbeAdjustX; //machine coordinates for PROBE_ADJUST
	float fProbeAdjustY; //machine coordinates for PROBE_ADJUST
	char cGotProbeAdjust;
//Save mem since nothing is using this now
//	char szTeachPosnName[MAX_STRING_LEN];
    char cTeachCaptureTrig;
    char cGotTeachCoords;
} td_ProbeMode;

typedef struct
{
	int iStartPosnIndex;
	int iCurPosnIndex;
    char cFirstMove;
    char cOnCurPosn;
	int iGotoPosnIndex;
	char cDoMoveTrig;
	float fLastKnownPosnX;
	float fLastKnownPosnY;
	char cFreshCurPosn;
	char cFreshCurPosnSend;
	float fPosnX; //similar to LastKnownPosn, but only set by newer GetPosition* functions
	float fPosnY;
	long lPosnX; //in counts, for use only with GetPositionCounts*
	long lPosnY;
	long lPosnTime; //Set by GetPositionTime or GetPositionCountsTime
	char cFreshPosn; //For use only with new GetPosition* functions to distinctly detect their return
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

//Types for Part Program system

typedef struct
{
//FIXME HIGH
//continue cleanup of these variable locations
	char cLocked;
    char cTeachModeData;
    char cMiniFtFormat; //indicate that new format is being used.
    char cChecksum;
	char p_cChecksum16[16];
    int iNumVisited; //total visited since load

	//special program parameters
    float K1OriginDistX; 	// in inches, X offset of K1 from track origin bumper
    float K1EndDistX;		// in inches, X offset of K1 from track end bumper
    float K1OriginDistY; 	// in inches, Y offset of K1 from center of X track

    char cErrorCount;
    unsigned int p_uiErrorMessages[PARTPGM_MAX_ERROR_MESSAGES];

} td_PartPgmInfo;

//New Program
//FIXME000000000000000 consolidate all the new as I clean

typedef struct
{
	unsigned int diameter;
    char ops; //FIXME000000000000 Ops variable width issues...
} td_tool_data_fileformat;

typedef struct
{
	char layers;
    char material;
	unsigned int clamplbs;
    char ops; //FIXME000000000000 Ops variable width issues...
    unsigned int countersinkDepth;
	//the following fields are only set when the larger format row is sent
	char proclayers;
	unsigned int clampwarnlbs;
	unsigned int clampalarmlbs;
	unsigned int procminbreak_not_used; //d2u format //no longer used, but needed for proper record spacing
	unsigned int hardstopamps; //Times 100 for 2 digits of decimal
	char procstyle;
} td_process_data;

typedef struct
{
	char flatx;
    char _flatxb2;
    char _flatxb3;
	char flaty;
    char _flatyb2;
    char _flatyb3;
    char ki_primary;
    char ki_secondary;
    unsigned int diameter;
    char tooltype;
    char process;
    char fastener;
    char pattern;
    char ops;
    unsigned int depthstack;
    int countersink_adj;
} td_hole_data_fileformat;

//FIXME00000000000000000000000000
typedef struct
{
	char cKInd;
	char cKPri;
    char cKSec;
    char cTool;
    char cProcess;
    char cFastener;
    char cPattern;
	unsigned int uiOps; //array of bits used to indicate what operations are specified for this hole (optional for some input formats)
    unsigned int uiStack;
    int iCountersinkAdjust;
    unsigned int d2uDiameter;
} td_HoleData;

//Teach

typedef struct {
	float fX;
	float fY;
} td_TeachMachinePosn;


//Smart Drill Struct Used for Layer Defs
typedef struct {
	char Name[32];
	char cLayerNumber;
	char cCoolantType;
	char cPeckType;
	char cShiftRetract;//36
	char cLcUseSlope;
	char cH2SPeck;
	char cCountersinkLayer;
	char cUseHardStop;//40
	char cBurstInterval; //10ms units  0=0ms 1=10ms//cSpeedShift;
	unsigned int uiRpm;
	unsigned int uiPeckDwell;//45
	int iLubeDurationMs;//47
	float fMicroPeckRot;
	float fMicroPeckSetback;
	float fAbsoluteEnd;
	float fThicknessMax;
	float fIpr;//67
	float fPeckIncrement;
	float fSetback;
	float fEarlyShiftDistance;
	float fThrustBaselineDistance;
	float fThrustMin;//87
	float fThrustMax;
	float fLcDeltaThrust;//95
} td_Layers;

//Smart Drill Struct Used for OVERRIDE

typedef struct {
	char cLayerNumber;
	char cOverrideActive;
	char cUseThisLayer;
	char cUseHardStop;
	float fAbsoluteEnd;
	float fThickness;
} td_LayerOverride;


//Function Predeclarations

void InitVars(void);
void InitConfig(void);
void SockInit(void);
void InitEthernet(void);
//Referenced in STP lib, but defined here
//void SessionDelay();
//void SessionClosing();
void StopInterfaceTasks();
#ifdef DRILL_DIRECT_READY
void ClearDrillSync();
#endif

void InitDIO(void);
void SetDefaultValveState();
void InitADC(void);

#ifdef BEEPSYSTEM
void Beep();
#endif
void BrakeOn();
void BrakeOff();
void DigOutService(void);

void ReadDigitalInputs(void);
void EstopEngageActions(void);
void EstopContinueActions(void);
void EstopDisengageActions(void);
void ReadADC(void);
#ifdef READ_TEMP
void ReadTempX(void);
void ReadTempY(void);
#endif
#ifdef READMOTORVOLTAGE
void ReadMotorVoltage();
#endif
#ifdef FORCELIMITING
void ReadForceInitMem();
void ReadForceSensor();
void ReadForceSensorNow();
#endif


//FIXME0 consider new position
void SmartToolMsgMiniFtMessageCode(unsigned int uiOID, unsigned int uiMessageCode);

// user block functions...config defaults are stored in User Block of flash
int UBwriteConfig(void);
int UBreadConfig(void);

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
nodebug void ClearAllHomeStatus();
nodebug void ClearSentHomeStatus();
nodebug void SetAllHomeStatusPending();
nodebug void AlertHomeStatus();
nodebug void AlertHomeReport(char axis_code, char status, char status_reason);
#endif
void SelectHomePositionAndPositionLimits(void);
#ifdef SEAL_SYSTEM
int LookupDiameterPrimeDelay(float fdiameter);
#endif

void main(void);

void ServiceNow();

// Probe Distance Check
void CheckKLocationDistances(char ck, char mark);

// functions associated with Modes
void ResetNearestPosition();
int FindNearestPosition();
int MoveAllowed(unsigned int uiSource);
int SpecifyGotoPosn(int iGotoPosn, int iIndex);
void SpecifyGotoPosnAgain();
void SetPressureBasedOnProcess(char cProcess);
void SetClampPressureLBS();
float CalcDistanceSquared( float fX1, float fY1, float fX2, float fY2 );
//MACHINEPOINTS
void CalcProbeHome();
int CalcProbeAdjust();
//ClearGolbalRotation(); //just clear two variables
void SetGlobalRotation(char cpk, char csk);
void StartCreateMachineDataSet();
void CreateMachineDataSet();
void CreatePosnHashList();
#ifdef OUTPUT_POSITION_INFO
//instead of listing the program detail all at once,
//this can put it out over time after the program is done loading
void ShowProgramRotationOutput();
#endif
char CreateMachineDataSetForOneHole(unsigned int i, float * fpX, float * fpY);
char RotateVecDataSetToMachine(unsigned int i, float * fpX, float * fpY);
char RotateVecMachineToDataSet(unsigned int i, float * fpX, float * fpY);
void ProbeModeSelectAndSetGlobalRotation();
void RotateMachineToDataset(float fX, float fY, float* p_fRotatedX, float* p_fRotatedY);
void ApplyMachineOffset(float * p_fX,float * p_fY);
void RemoveMachineOffset(float * p_fX,float * p_fY);
void SetToolOffset(float fToolOffsetX,float fToolOffsetY);
void SetToolOffsetWithYComp(float fToolOffset1X,float fToolOffset1Y,float fToolOffset1YExtension,
							float fToolOffset2X,float fToolOffset2Y,float fToolOffset2YExtension);
void ClearToolOffset();
void CalculatePreMove(float* p_fPreMoveX, float* p_fPreMoveY);
void SetMoveSpeeds(char bMakeLinear, char bFinalMove, float fdx, float fdy);
void DoFloat(char cAction);

// Part Program Loading / Parsing functions
void ClearPartProgram();
void ParsePartProgramStart();
void ParsePartProgramContinue(char cMaxLinesPerContinue);
void PartProgramShowStatus();
void PartProgramShowStatusPart2();
void LogParseError(unsigned int uiMessageCode);
void CreateProbeEvaluationOrder(); //probing move to probe lib???
void RecalculateLocatingDirectives(char ki);
void PreviewDisplayProbeCommandDataBase();
void LoadProbeCommand(char chprobe);
void ClearProbeCommand();
void ParseProbeCommand(char ckprobe, char * sbuf, int i);
void ClearPositionsDuringProbe(char ki);
void RecalculatePositionsDuringProbe(char ki);

void CaptureTeachPosition();
void DeletePosition();
void CompleteTeachPosition();
void LoadLinearProgram(); //an alternate
void InitXmemStorage();
void ClearOpHistory(void);
void AddOpHistory(int index,unsigned int uiOperation);
void SetOpHistory(int index,unsigned int uiOperations);
unsigned int GetOpHistory(int index);
void SetDrillDir(int iProbeDir);

// Job Selection
void StartOver();
void StartSendPartProgramData(int iSession,unsigned long ulStart);
void ContinueSendPartProgramData();
void SendProbeStatus(unsigned int uiMsgType,char cKIndex);
void SendAllProbeStatus(unsigned int uiMsgType);
void SendProbeStart(unsigned int uiMsgType,char cKIndex);
void SendAllProbeStart(unsigned int uiMsgType);
void AlertProbeStatusUpdate();

//void SendKHoleDistance(unsigned int uiMsgType,float fExpected,float fFound);
void SendXYDataID(unsigned int uiMsgType);
void StartSendXYData(int iSession, unsigned int uiStart );
void ContinueSendXYData();
void StartSendOpHistory(int iSession, unsigned long ulAfterTime);
void ContinueSendOpHistory();
void SendRFIDData(unsigned int uiMsgType);
void SendCurPosnInd(unsigned int uiMsgType);
void SendNearPosnInd(unsigned int uiMsgType);
void SendGoalPosnInd(unsigned int uiMsgType);
void SendCurXY(unsigned int uiMsgType);
void SendNearXY(unsigned int uiMsgType);
void SendActivePremove(unsigned int uiMsgType);
void SendSystemComponents(unsigned int uiMsgType);
void LoadHoleParameters();
void SendHoleParameters(unsigned int uiMsgType);
#ifdef SMARTDRILL_STP
void CalculateLayerOverride();
void SendLayerOverride();
void ShowLayerOverride();
#endif
void SendReqToolSearch(unsigned int uiMsgType);
void SendToolMGMT(unsigned int uiMsgType,char c_op,char carg1,unsigned int ui,char *s, int ilen);
void SendTool(unsigned int utMsgType,char cOperation);
char CheckObstructionsAndMotionLimits(float fX,float fY);
void AlertObstructionCode();
void AlertObstructionWarningCode();

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
char LookupToolTypeCode(char * tooltype);
void LookupToolTypeString(char * tooltypeout,char ctool_search);

//Tool Sync
void VerifyAndAlertTool();
void VerifyAndAlertProcess();
void LoadProcessAndOverride();
void LoadProcess();

// STP UTIL SECTION
char * DisplayOIDName(unsigned int ui);

//CenterVision
#ifdef CENTERVISION
#ifdef CENTERVISION_LINE_SCANNER
#use "MiniFtVision.LIB"
#endif
#ifdef CENTERVISION_ANALOG_POINT_LASER
//#define EXCLUDE250
#use "MiniFtVisionPoint.LIB"
#endif
#ifdef CENTERVISION_CAM
#use "MiniFtVisionCam.lib"
#endif
#endif

//#define PROFILEPOINTS
#ifdef PROFILEPOINTS
//  Profiling Tool - A performance profiling tool
nodebug void InitProfiling();
nodebug void ClearProfiling();
nodebug void ProfilePoint(char * p_cLocation);
nodebug void PrintProfilePoints();
#else
#define ProfilePoint(x)
#endif

//Maintain g_DateTime
void ServiceDateTime();

//  Utillity Section
//Math
//returns 0 on success
char FindIntersection(float ax1, float ay1, float ax2, float ay2,
						float bx1, float by1, float bx2, float by2,
						float *fx, float *fy);

//System Function
nodebug void RebootRabbit();
//long xallocsafe(long lAllocBytes);
long xallocsafedetail(char * name,long lAllocBytes);
long xallocsaferamtypedetail(char * name,long lAllocBytes,word typeram);
//Data, string, and debug functions
nodebug int SignedCharValue(char c);
nodebug speed char * scanToNull(char * s);
nodebug speed char * scanToChar(char * s,char c);
nodebug speed char * scanPastWhiteSpace(char * s);
nodebug char * scanToWhiteSpace(char * s);
nodebug char * scanToWhiteSpaceOrChar(char * s,char cDelimiter);
nodebug useix int prefix(char *prefix, char *str);
nodebug useix int equal(char *str1, char *str2);
nodebug useix char *strcpy_returnpost(char *dst, char *src);
nodebug char split(char cDelimiter,char * s,char ** p_szFields,char cMaxFields);
nodebug void rchr(char *s,char c,char n);
//define two special split return values
#define SPLIT_PARSE_ERROR_QUOTE_MISMATCH 255
#define SPLIT_PARSE_ERROR_CHARS_AFTER_VALUE 254

void initCopyHexToByte();
char copyHexToByte(char * bytes,char *szhex,char cCount,char cRequireEnd);

// TimerB functions
#ifdef USE_TIMERB
void TimerBInit(void );
void TimerBStart(void);
int	TimerBRead(void);
int	TimerBReset(void);
#endif

// DEBUG DISPLAY TOOLS ////////////////////////////////////////////
void memchardump(char * label,char * data,int iLen);
void memdump(char * label,unsigned char * data,int iLen);

#define strnull(x) *((char *)x)=0

//#define SHOW_MEM
#ifdef SHOW_MEM
//void TestFloatResP10(float f);
void show_memory();
#endif

void ClearShowIO();
void ShowIO();
void ClearShowAna();
void ShowAna();

//Global Storage

char g_cConfigLoadSuccess;

char g_szSerialNumber[64];

td_ConfigData  g_ConfigData;

//MakeOID-generated:: GLOBAL DECLARATIONS (oid complete)
td_StpStatus g_StpStatus;
td_DateTime g_DateTime;
char g_cModeState;
char g_cAction;
char g_cEEOption;
td_HoleParam g_HoleParam;
char g_cLoadedProcess;
float g_fGaugeLength;
char g_szPartPgmFilename[MAX_FILENAME_LEN+1];
char g_cPartPgmStatus;
char g_cGravCompStatus;
char g_cFloatStatus;
td_GravCompResults g_GravCompResults;
float g_fXRailSurfaceOffset;
char g_cProbeMethod;
char g_cPattern;
char g_cMoveDone;
td_MachineOffset g_MachineOffset;
char g_cMachineOffsetCompensationAdjustment;
td_MachineOffset1 g_MachineOffset1;
td_MachineOffset2 g_MachineOffset2;
char g_cStation;
td_JogFactor g_JogFactor;
char g_cALockMode;
td_VisionInspectResults g_VisionInspectResults;
char g_cDrillState;
char g_cDrillExplanation;
char g_cSealState;
char g_cFillState;
char g_cFastenerArrived;
//These 3 are place holder. The oids could be recycled for fill or anything
char g_cAccelArm_;
char g_cAccelTrigger_;
td_FastenerFault g_FastenerFault;
td_DrillFault g_DrillFault;
td_HoleResultData g_HoleResultData;
td_RFIDData g_RFIDData;
char g_szTagDatalen;
char g_szTagData[65]; //64 + 1
char g_cReadRFID;
td_ForceSensor g_ForceSensor;
//Tool Management OID globals
char g_szToolIDlen;
char g_szToolID[257]; //256 + 1
char g_szToolTypeCodelen;
char g_szToolTypeCode[65]; //64 + 1
//MakeOID-generated::END

//Time of Last g_DateTime update
unsigned long g_ulDateTimeMS;

//Action Sent
char g_cActionSent;

//Hold
char g_cHold;

#ifdef HOMESYSTEM_X_RFID
float g_fRFIDMachineX;
float g_fRFIDRailX;
int g_iRFIDRailOrientation;
#endif

//assisting variables to globals
#ifdef READ_TEMP
float g_fXTemp;
float g_fYTemp;
#endif

char g_cEstopPrevMode;
char g_cPosnModeState;
char g_cStartProcess;
char g_cStationGoal;
char g_cStartStation;
char g_cStationPlanDrill;
char g_cStationPlanInspect;
#ifdef SEAL_SYSTEM
char g_cStationPlanSeal;
#endif
#ifdef FASTENER_SYSTEM
char g_cStationPlanFill;
char g_cStationPlanRemove;
#endif
char g_cSentStation;
char g_cSentStationGoal;
char *g_cStationCode;

unsigned int g_uiActionCycles;
int g_iActionHoleIndex;

//FIXME0000: are these 2 used really????
char g_cDrillStatePrev;
unsigned long g_ulDrillStateStartTime;

//Temp Quick Implementation for this
char g_cSafeUnclamp; //Signal used by Drill and Fastener
char g_cSawDrillMode;
char g_cTestOpt;
char g_cLubeBypass;

char g_cFastDoneFlag;
char g_cPendingFastenerFaultUpdate;

int g_iPrimeDelay;

char g_cCutterDetected;
char g_cLastCutterDetected;
unsigned int g_uiMessageCode;

//Tool
td_oid_tool_rec g_LoadedTool;

td_STPsessions * g_ToolServerSTPSession;

//Part Program Data
td_PartPgmInfo g_PartPgmInfo;  // supporting only one PP at a time, so this is not an array

//additional parse state, start by start and used in continue
char g_cBeingParsed;
//used for direct data
char * g_sPPDirect;
int g_iPPDirectPos;
int g_iPPDirectEnd;
char g_cPPDirectFinalCall;
int g_iProgramLoadingSession;

//Part Program
char g_cFormatMinorVersion;
char g_cToolCount;
char g_cProcessCount;
char g_cFastenerCount;
char g_cPatternCount;
int g_iHoleCount;
char g_cKHoleCount;

//NEW_MINIFT_FORMAT
//PROCESS DATA
char g_cProcessLayerCount[MAXNUM_PROCESSES];
char g_cProcessMaterials[MAXNUM_PROCESSES];
char g_cProcessProcLayerCount[MAXNUM_PROCESSES];
unsigned int g_uiProcessPounds[MAXNUM_PROCESSES];
unsigned int g_uiProcessPoundsWarning[MAXNUM_PROCESSES];
unsigned int g_uiProcessPoundsAbort[MAXNUM_PROCESSES];
char g_cProcessOps[MAXNUM_PROCESSES];
unsigned int g_uiProcessCountersink[MAXNUM_PROCESSES];
unsigned int g_uiProcessHardstopAmps[MAXNUM_PROCESSES];
char g_cProcessStyle[MAXNUM_PROCESSES];
unsigned int g_uiProcessLayerDataIndex[MAXNUM_PROCESSES];
//(7*2+4)= 18 bytes per proc * 64 procs = 1152 bytes

//HOLE DATA
int g_xallocerrors;
char g_cMemoryAllocated;
long xp_fRawDataX;
long xp_fRawDataY;
long xp_fRawDataMX;
long xp_fRawDataMY;
long xp_fRawDataR; //radius to halfway mark to nearest hole.
long xp_uiOpHistory; //array of bits used to indicate hole operation history.

long xp_tdHoleData;	//buffer for hole data structures....

long xp_MaterialData;
long xp_StackData;

long xp_PartProgramData; //buffer for PP Data that we need to be able to send out later

unsigned long g_ulPartProgramDataLength;

long xp_ToolNames; //not allocated but a pointer into the part program data
int g_iToolNamesLen;

long xp_ProcessLayerDataBase; //not allocated but a pointer into the part program data
long xp_ProbeCommandDataBase; //probe control field

//Program Options
float g_fAssumeApproxDataSetOrientation;
char g_cAssumeApproxPositionsFromDataset;
char g_cRequireParameterRecognition;

//Prbe - NOT A TYPO : New system is called Prbe until the old is 100% removed
char g_cProbeFlag;
float g_fProbeX; //Hold The Probe Position Until it is placed into the arrays
float g_fProbeY;
int g_iKHoleHoleIndex[MAXNUM_KHOLES + 2];
float g_fKHolePrbeX[MAXNUM_KHOLES + 2];
float g_fKHolePrbeY[MAXNUM_KHOLES + 2]; //MAXNUM_KHOLES + 2];
char g_cKHolePrbeStatus[MAXNUM_KHOLES + 2];
char g_cKHolePrbeStatusWarnings[MAXNUM_KHOLES + 2];//goes out in HIGH HALF of method
char g_cKHolePrbeStatusDistance[MAXNUM_KHOLES + 2];//used only for distance check loop
//ProbeStart
char g_cKHolePrbeStart[MAXNUM_KHOLES + 2];
float g_fKHolePrbeStartX[MAXNUM_KHOLES + 2];
float g_fKHolePrbeStartY[MAXNUM_KHOLES + 2]; //MAXNUM_KHOLES + 2];
char g_cKHoleHoleIndexSet;
char g_cDistanceErrorFlagSent;
char g_cDistanceErrorShown;
char g_cProbeDistanceErrors;

char g_cKHolePrbeEvalDCount[MAXNUM_KHOLES + 2];
char g_cKHolePrbeEvalOrder[MAXNUM_KHOLES + 2];
//Probe Command Field Probe Parameters for Probe Control
char g_cProbeCommand;
char g_cProbeCommandMessage;
float g_fProbeVectorX;
float g_fProbeVectorY;
float g_fProbeMachineVectorX;
float g_fProbeMachineVectorY;
char g_cProbeExtraOffsetGiven;
char g_cProbeExtraMachineOffsetGiven;
float g_fProbeExtraOffsetX;
float g_fProbeExtraOffsetY;
float g_fProbeExtraMachineOffsetX;
float g_fProbeExtraMachineOffsetY;
float g_fProbeMaxDistShift;
char g_cProbeShiftLimX;
float g_fProbeShiftLimXMin;
float g_fProbeShiftLimXMax;
char g_cProbeShiftLimY;
float g_fProbeShiftLimYMin;
float g_fProbeShiftLimYMax;
float g_fProbeExpectedDiameter;

//Probe Command for new runtime probe system
char g_cKHolePrbeCommand;
char g_cKHolePrbeCommandInput;
//Hole Being Probed by new probe system
char g_cKHolePrbeIndex;
char g_cKHolePrbeIndexInput; //input value used with trigger
//New Trigger
char g_cKHolePrbeTrigger;
//Complete and Calculated
char g_cProbeComplete;
char g_cProbeCalculated;
//Reapply Feature
float g_fKHoleLastPrbeX[MAXNUM_KHOLES_REPROBE + 2];
float g_fKHoleLastPrbeY[MAXNUM_KHOLES_REPROBE + 2];
char g_cKHoleLastPrbeStatus[MAXNUM_KHOLES_REPROBE + 2];
char g_cKHoleLastCount;

//RFID HOME
float g_fStartX;
float g_fStartY;
float g_fTargetX;
char g_cTagState;
char g_cMoveDir;
unsigned long g_ulCenterStart;
float g_fTagStart;
float g_fTagEnd;
char g_cTagP;
char g_cTagN;
#define TAG_NONE 0
#define TAG_STARTED 1
#define TAG_FOUND 2

char g_cImmediateFakeRfid;

//XYDATA_ID
unsigned long g_ulMachineDataSetIDTimeSec;
unsigned long g_ulMachineDataSetIDTime;

//OpHistory
#define OPHISTORYBLOCKCOUNT 40
char g_cBlockCount;
char g_cOpHistoryBlockOrder[OPHISTORYBLOCKCOUNT]; // 40 > 5000/128
unsigned long g_ulOpHistoryBlockTime[OPHISTORYBLOCKCOUNT];

int	g_iProbedIndex;

//HD Clamp Status
#ifdef CLAMP_SYSTEM
char g_cClampState;
char g_cClampGoal;
char g_cClampStateSent;
char g_cClampGoalSent;
#endif
unsigned long g_ulClampStart;
unsigned long g_ulClampPressureZero;
unsigned long g_ulClampPressureLow;
unsigned long g_ulClampPressureHigh;
unsigned long g_ulClampLegsLock;
unsigned long g_ulClampLegsUnlock;
unsigned long g_ulClampLegsUp;
unsigned long g_ulClampLegsDown;
unsigned long g_ulClampALock;
unsigned long g_ulClampAUnlock;
unsigned int g_uiClampPressure;
unsigned int g_uiClampPressureWarning;
unsigned int g_uiClampPressureAbort;
char * g_szClampMessage;

//Pending Ops
char g_cClear;
#ifdef CLAMP_SYSTEM_NAC_STP
char g_cNACClear;
float g_fNACClampZOffset;
#endif
//Jog
char g_cJogX; //The status
char g_cJogY; //The status
char g_cJogGoalX; //The goal
char g_cJogGoalY; //The goal
float g_fJogX; //The status speed factor
float g_fJogY; //The status speed factor
float g_fJogGoalX; //The goal speed factor
float g_fJogGoalY; //The goal speed factor
unsigned long g_ulJogStopX;
unsigned long g_ulJogStopY;
#ifdef JOG_ENABLE_TIME
unsigned long g_ulJogEnableTime;
#endif

//Position Update
unsigned long g_ulPositionUpdateTime;
unsigned long g_ulPositionUpdateThrottle;
unsigned long g_ulPositionSendTime;
unsigned long g_ulPositionSendThrottle;
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

#ifdef MD5HASH
md5_state_t g_md5_state;
#endif

//FIXME MED Pattern
// bring this back some day when I use the new patterh feature
// this is really a reminder to build the new pattern feature
//int g_iChosenPattern;

//Tool and Process Check
char g_cRequiredTool;
char g_cRequiredProcess;
char g_cLoadedTool; //the code of the loaded tool (or 0 if it has none in the map)
char g_cToolLoaded; //bool
char g_cProcessLoaded; //bool
char g_cOverrideCalculated;
char g_cOverrideSent;
char g_cLayerOverrides;
td_LayerOverride g_LayerOverrides[8];

char g_cLastSearchedRequiredTool;

//DRILL_DIRECT_PROCESS_AND_OVERRIDE
char g_cDrillLoadProcessAndOverride;
#define DRILL_LOAD_PROCESS_NOT_NEEDED 0
#define DRILL_LOAD_PROCESS_NEEDED 1
#define DRILL_LOAD_PROCESS_WAIT 2
#define DRILL_LOAD_PROCESS_READY 3
#define DRILL_LOAD_PROCESS_DONE 4

#define DRILL_SYNC 3
#define DRILL_SYNC_BIT1 1
#define DRILL_SYNC_BIT2 2
char g_cDrillSync;
float g_fHomeBack;
float g_fLastSentHomeBack;

char g_iDrillDir; //hold the effective drill direction ( may not be the same as the default )

char g_cAllowKVisit;
char g_cAutoMove;
unsigned long g_ulAutoTime;
unsigned long g_ulStartAutoTime;
char g_cAutoRepeat;
char g_cLastAutoMove;

//Float
char g_cFloatGoal; //used only by DoFloat to indicate what the last request was
char g_cFloatExitModePosnState;

unsigned int   g_uiModeFlags;

//HOMESYSTEM
#ifdef HOMESYSTEM
char g_cHomed;	//overall system home status
char g_cSentHomed;
#ifdef HOMESYSTEM_X
char g_cHomedX;	//X Axis
char g_cSentHomedX;
#endif
#ifdef HOMESYSTEM_Y
char g_cHomedY;	//Y Axis
char g_cSentHomedY;
#endif
#ifdef HOMESYSTEM_DRILL
char g_cDrillHomed;	//an external system
char g_cSentDrillHomed;
#endif
#ifdef HOMESYSTEM_AAXIS
char g_cNACAAxisHomed;	//an external system
char g_cSentNACAAxisHomed;
#endif
#ifdef HOMESYSTEM_CLAMP
char g_cNACClampHomed;	//an external system
char g_cSentNACClampHomed;
#endif
#ifdef HOMESYSTEM_FASTENER
char g_cFastenerHomed; //an external system
char g_cSentFastenerHomed;
#endif
#ifdef HOMESYSTEM_FASTENERTRAY
char g_cFastenerTrayHomed; //an external system
char g_cSentFastenerTrayHomed;
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

//DIO

//MiniFt Core DIO
char g_cLED;
#ifdef BEEPSYSTEM
char g_cBeep;
#endif

char g_cLEDCount;
unsigned int g_uiLEDOn;
unsigned int g_uiLEDWhole;
unsigned long g_ulLEDStart;
#ifdef BEEPSYSTEM
char g_cBeepMode;
char g_cBeepCount;
unsigned int g_uiBeepOn;
unsigned int g_uiBeepWhole;
unsigned long g_ulBeepStart;
#endif

//HD
#ifdef CLAMP_SYSTEM_HD_PISTON
char g_cALock;
char g_cLegsLock;
char g_cLegsDown;
char g_cLegsUp;
char g_cClampExtend;
char g_cClampRetract;
#endif
char g_cAirBlastX;
char g_cAirBlastY;

//DFINT
char g_cDrillStateGoal;
char g_cDrillStateGoalSent;
char g_cDrillHomeWasStarted;
char g_cDrillStateGoalCommanded;
char g_cDrillStateDFGoal;

#ifdef SEAL_SYSTEM
char g_cSealantApply;
char g_cSealantPressure;
char g_cSealantPinch;
#endif

#ifdef GENCIRCMFTX
//CIRCMFT
#ifdef DIGOUT_CHNUM_DRILLUP
char g_cDrillUp;
#endif
char g_cDrillDown;
char g_cDrillButton;
char g_cColletClamp;
#endif

char g_cLaserPointer;

#ifdef ESTOPV1B
char        g_cDigInEstopXCount;
char        g_cDigInEstop;
#endif
#ifdef UNIBUTTON
char        g_cDigInUniButtonXCount;
char        g_cDigInUniButton;
#endif
char        g_cDigInEstopSwitchXCount;
char        g_cDigInEstopSwitch;

char		g_cMachineLock;
#ifdef DIGIN_CHNUM_XLOCK
char		g_cDigInXLock;
char		g_cDigInXLockXCount;
#endif
#ifdef DIGIN_CHNUM_YLOCK
char		g_cDigInYLock;
char		g_cDigInYLockXCount;
#endif

#ifdef GENCIRCMFTX
char		g_cDigInDrillUp; //no count
char		g_cDigInDrillDown; //no count
#endif

#ifdef Y_LIMIT_SENSORS
char		g_cDigInYPosLimit; //no count
char		g_cDigInYNegLimit; //no count
#endif

#ifdef PRESSURESENSOR
char		g_cDigInAirPressure;
char		g_cDigInAirPressureXCount;
#endif

#ifdef ORIENTATION_SENSORS
char		g_cDigInYOrientationA; //no count
char		g_cDigInYOrientationB; //no count
#endif
char g_cPrevOrSensors;

//Obstruction System
char	g_cObstructionCode;
char	g_cLastObstructionCode;
char	g_cObstructionCodeNew;

char g_cObstructionWarningCode;
char g_cLastObstructionWarningCode;

#ifdef OBSTRUCTION_SYSTEM_XP1
char	g_cDigInObstructionXP1;
#endif
#ifdef OBSTRUCTION_SYSTEM_XN1
char	g_cDigInObstructionXN1;
#endif
#ifdef OBSTRUCTION_SYSTEM_MOW
char g_cDigInObstructionMOW;
#endif
#ifdef OBSTRUCTION_SYSTEM_MOS
char g_cDigInObstructionMOS;
#endif
#ifdef OBSTRUCTION_SYSTEM_MO_ANA
char	g_cMOFlags;
#endif
#ifdef DIGIN_SOFTSTOP
char g_cDigInSoftStop;
#endif

#ifdef UNIBUTTON
char g_cUniButtonEvent;
#endif

//Use one lock event for all lock changes
char g_cLockEvent;
#ifdef PRESSURESENSOR
char g_cAirPressureEvent;
#endif
//Obstruction System
char g_cObstructionEvent;
//Obstruction Flag Event
char g_cMCObstructionEvent;
//Obstruction Warning Event
char g_cObstructionWarningEvent;

char g_cBrakeReleased;

char g_cEstopMCAsserted;
char g_cEstopFullyEngaged;

//ADC DAC
unsigned int g_uiClampPressureLBS;
unsigned int g_uiClampPressureLBSLastSet;

#ifdef READMOTORVOLTAGE
//reading motor voltage
float g_fMotorVoltage;
#endif

//state variables for specific modes
char            g_cCurrentJogDir;

// struct for holding gravcomp results and handshakes
td_GravComp g_GravComp;
// grav comp timer
unsigned long g_ulGravCompTime;

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
char	g_cRotationKP;
char	g_cRotationKS;
char	g_cRotationContext;
float g_fPKMX;
float g_fPKMY;
float g_fSKMX;
float g_fSKMY;
float g_fPKX;
float g_fPKY;
float g_fSKX;
float g_fSKY;
float	g_fCosTheta;
float	g_fSinTheta;
#ifdef LINEAR_SCALE_OPTION
float	g_fCosScale;
float   g_fSinScale;
float	g_fScale;
float	g_fUnscale;
#endif

//SendPPData System
char g_cSendPPDataSessions;
char g_cSendPPDataSession[MAXNUM_STP_SESSIONS]; //indicate sending to this one
unsigned long g_ulSendPPDataIndex[MAXNUM_STP_SESSIONS];
unsigned long g_ulSendPPDataMax[MAXNUM_STP_SESSIONS];

//SendXYData System
char g_cSendXYDataSessions; //not a count but single flag indicating something is set
char g_cSendXYDataSession[MAXNUM_STP_SESSIONS]; //indicate sending to this one
unsigned int g_uiSendXYDataIndex[MAXNUM_STP_SESSIONS];

//OpHistory
char g_cSendOpHistory;
char g_cSendOpHistoryBlockOrderIndex[MAXNUM_STP_SESSIONS];
unsigned long g_ulSendOpHistoryAfterTime[MAXNUM_STP_SESSIONS];

//Scaling Flag
char g_cScaleBasedOnProbe;
//Flag For Linear Program Mode number
int g_iPartPgmLinearHoles;

//Inspection Flag
char g_cInspectMethod;
//Inspection Progress Flag
char g_cPositionInspection;

unsigned int g_uiPositionOpsCache;
char g_cShowPrevDrilledMessage;

char g_cFastenerMode;
char g_cFastenerLoaded;
char g_cFastenerState;

//FIXME FAS
#warns "FAS  DEMO FASTENER SHORTCUTS"
//There are a number of demo fastener shortcuts to reeval
//  Some has NO #ifdef control....  Some are temporary code
char g_cUseFastenerTray;
float g_fFastenerTrayPickupPosition;
float g_fFastenerTrayClampPickupHeight;

//	TimerB support
#ifdef USE_TIMERB
int	g_iTBcount;
int g_iTBelapsedcounts;
long int g_lTBelapsed_usec;
#endif

//Additional Time Measure
unsigned long g_ulClampUnlockTime;

float g_fMachineBaseExtension; //make this an OID????

//MACHINEPOINTS
//CLOSEST_HOLE_SYSTEM
#define MPOSNHASHSIZE 256
typedef char t_MPOSNBUCKETTYPE;
long xp_uiMPosnHashList;
float g_fMPosnHashMinBucket;
float g_fMPosnHashMaxBucket;
float g_fMPosnHashBucketsize;
unsigned int g_uiMPosnHashList[MPOSNHASHSIZE];

unsigned long g_ulCreateMachineDataSetStartTime;
unsigned int g_uiCreateMachineDataSetIndex;
unsigned int g_uiCreateMachineDataSetHashIndex;
#ifdef OUTPUT_POSITION_INFO
unsigned int g_uiProgramRotationOutputIndex;
#endif

unsigned long g_ulArrive;
unsigned long g_ulPastTPC;
unsigned long g_ulLastMoveStart;
unsigned long g_ulMoveStart;
unsigned long g_ulMoveEnd;
unsigned long g_ulProcStart;
unsigned long g_ulClampStart2;
unsigned long g_ulClampDone;
unsigned long g_ulProcPassed;
unsigned long g_ulDrillStart;
unsigned long g_ulFastenerStart;
unsigned long g_ulUnclampStart;
unsigned long g_ulUnclampDone;
unsigned long g_ulDrillDone;
unsigned long g_ulFastenerDone;
unsigned long g_ulFinalTime;
unsigned long g_ulDrillEchoStart;
//Start of newer cycle based system... not ready to replace old system but adding for LD, LO, Spinup etc...
unsigned long g_ulLPR;
unsigned long g_ulLD;
unsigned long g_ulLO;
unsigned long g_ulSpinUp;
unsigned int g_uiCutterAir;
char g_cPrintAuth;

#warnt "Check this buffer use and need"
char temppploadbuffer[2024];

//For use by macro
unsigned long g_ultemp;
char * p_cultemp;

//For use by EthernetDownloaderEnabled
char g_cEthernetDownloaderEnabled;
//For Use with ShowIO
char g_cShowIO;
char g_cShowAna;

//MakeOID-generated:: NAMES ALT (complete)
xstring xsOIDNAMESCommon {
	"NULLOID",
	"DEBUG",
	"SMARTTOOL_TYPE",
	"SMARTTOOL_SUBTYPE",
	"SYSTEM_VERSION",
	"SERIAL_NUMBER",
	"RESET_SYSTEM",
	"SCRIPT_TRANSFER",
	"GENERICMESSAGE",
	"DEBUGMESSAGE",
	"STPSTATUS",
	"COMMON_MESSAGE_CODE",
	"CONTROL_DELAY",
	"DIRECT_DOUT",
	"DIRECT_DIN",
	"COM_VERSION",
	"STREAM_SYNC",
	"STP_CLOSE",
	"STP_IDENTIFY",
	"DATE_TIME",
	"OID_MAX_NUMBER_Common"
};
//MakeOID-generated::END

//MakeOID-generated:: NAMES ALT (complete)
xstring xsOIDNAMESMiniFT {
	"MINIFT_MESSAGE_CODE",
	"MODE",
	"ACTION",
	"RESET_MC",
	"ENCODER_RATIO",
	"MC_CURRENT_LIMITS",
	"MC_PEAK_CURRENT_LIMITS",
	"BRAKE_ON_TIMEOUT",
	"MCERR",
	"STOREDEFAULT_CONFIG",
	"RECALL_CONFIG",
	"EEOPTION_DEFAULT",
	"EEOPTION",
	"BEEPER",
	"SYSTEM_MONITOR",
	"AIR_PRESSURE",
	"LIM_A_OBS",
	"TOOL_VERIFYENABLE",
	"HOLE_PARAMETERS",
	"TOOL",
	"PROCESS",
	"RETURN_HEIGHT",
	"GAUGE_LENGTH",
	"SCALE_MODE",
	"R2",
	"PPGM_CLEAR",
	"PPGM_REQ_FILE",
	"PPGM_DIR",
	"PPGM_NAME",
	"PPGM_DATA",
	"PPGM_CHECKSUM",
	"PPGM_LINEARJOB",
	"PPGM_STATUS",
	"PPGM_LOCKED",
	"STARTOVER",
	"LOAD_YRAIL",
	"GC_STATUS",
	"GC_AXES",
	"GC_CMD",
	"GC_FLOAT",
	"GC_SPEED",
	"GC_ACCEL",
	"GC_DECEL",
	"GC_MOVEDIST",
	"GC_ALGORITHM",
	"GC_NOISE_LIMIT",
	"GC_TRIGGERFACTOR",
	"FLOAT_STATUS",
	"GRAVCOMP_RESULTS",
	"FLOAT_SPEEDLIMIT",
	"JOG_SPEEDLIMIT",
	"MAX_SPEED_X",
	"MAX_SPEED_Y",
	"X_RAIL_SURFACE_OFFSET",
	"PROBE_METHOD",
	"PROBE_METHOD_DEFAULT",
	"PROBE_OFFSET",
	"PROBEDIR",
	"DRILLDIR",
	"KHOLE_MAX_DISTANCE_ERROR",
	"APPROX_LOCATION_ERROR",
	"KHOLE_DISTANCE",
	"PROBE",
	"PROBE_POSITION",
	"PROBE_STATUS",
	"PROBE_START",
	"PROBE_ACCEPT_REQUIRED",
	"HOME",
	"HOME_SPEED",
	"HOME_ACCEL",
	"HOME_DECEL",
	"HOME_MOVEDIST",
	"PROBE_ADJUST",
	"PROBE_ADJUST_LIMIT",
	"HOME_FINE_SPEED",
	"HOME_REPORT",
	"PATTERN",
	"PM_CURPOSN",
	"PM_NEARPOSN",
	"PM_GOALPOSN",
	"PM_CURXY",
	"PM_NEARXY",
	"PM_POSNSUMMARY",
	"PM_MOVETONEXT",
	"PM_MOVETOPREV",
	"PM_MOVETOIND",
	"PM_MOVEAGAIN",
	"PM_MOVETYPE",
	"PM_PREMOVEXY",
	"PM_SPEED",
	"PM_ACCEL",
	"PM_DECEL",
	"PM_FINALSPEED",
	"ORTHO_SLOPE",
	"POSNERR_LIMIT",
	"PM_TOLERANCE",
	"VELERR_LIMIT",
	"LONG_DISTANCE",
	"LONG_SPEED",
	"PM_MOVEDONE",
	"OP_STARTED",
	"OP_HISTORY",
	"DRILL_HOLE_ONE_TIME",
	"AUTOMOVE",
	"AUTOMOVE_DELAY",
	"AUTOREPEAT",
	"MACHINE_OFFSET",
	"MACHINE_OFFSET_CADJ",
	"MACHINE_OFFSET1",
	"MACHINE_OFFSET2",
	"STATION",
	"TOOL_OFFSET",
	"TOOL_FLIP",
	"DRIVE_THR_BACKLASH",
	"DRILL_OFFSET1",
	"DRILL_OFFSET2",
	"OFFSET_SEAL",
	"OFFSET_FILL",
	"JOG",
	"JOG_SPEED",
	"JOG_ACCEL",
	"JOG_DECEL",
	"JOG_FACTOR",
	"HOME_POS_Y_POS",
	"POS_LIMIT_Y_POS",
	"HOME_POS_Y_NEG",
	"POS_LIMIT_Y_NEG",
	"PROBE_REGISTRATION",
	"OBSTRUCTION_CODE_MASK",
	"MACHINE_LOCK_REQUIRED",
	"MACHINE_LOCK",
	"CLAMP",
	"ALOCK",
	"ALOCKDELAY",
	"AUNLOCKDELAY",
	"LEGSLOCKDELAY",
	"LEGSUNLOCKDELAY",
	"LEGSUPDELAY",
	"LEGSDOWNDELAY",
	"LOWPRESSUREDELAY",
	"LOWPRESSUREDOWNDELAY",
	"PRESSUREDELAY",
	"PRESSUREDOWNDELAY",
	"LOWPRESSURE",
	"PRESSURE",
	"AIR_CLEAR",
	"LASER_SENSOR_OFFSET",
	"CAM_OFFSET",
	"VISION_INSPECT",
	"VISION_IMAGE",
	"VISION_DATA",
	"VISION_INS_RES",
	"LS_ALG_PARAM",
	"CAM_ALG_PARAM",
	"LS_AUTO_INSPECT",
	"STOP_INT_TASK",
	"PROC_START",
	"PROC_STOP",
	"PROC_CONT_MODE",
	"PROC_OPS",
	"DRILL_STATE",
	"DRILL_EXPL",
	"DRILL_EXPL_DATA",
	"SEAL_STATE",
	"SEAL_CLAMP",
	"SEAL_PRESSURE_DELAY",
	"SEAL_PRE_REL_DELAY",
	"SEAL_PRIME_DELAY",
	"SEAL_GLOB_DELAY",
	"SEAL_APPLY_DELAY",
	"FILL_STATE",
	"FILL_CLAMP",
	"FILL_EXTEND_DELAY",
	"FILL_RAM_DELAY",
	"FASTENER_REQUEST",
	"FASTENER_ARRIVED",
	"FASTENER_POST_DELAY",
	"ACCEL_ARM",
	"ACCEL_TRIGGER",
	"TOOL_Z_BASE",
	"FASTENER_FAULT",
	"PM_AC_PREMOVEXY",
	"DRILL_FAULT",
	"HOLE_RESULT_DATA",
	"MOVEXY",
	"POSN_DISPLAY",
	"PM_XYDATA_ID",
	"PM_XYDATA",
	"RFID_CONFIG",
	"RFID_DATA",
	"HOME_RFID",
	"READ_RFID",
	"HOME_STOP",
	"ESTOP_CLEAR_DELAY",
	"DRILL_BUTTON_DELAY",
	"USE_CUTTER_DETECT",
	"JOG_ENABLE_TIMEOUT",
	"DRILL_CYCLE_DELAY",
	"INSPECT_METHOD",
	"COMMAND_INSPECT_METHOD",
	"FORCE_SENSOR_CALIBRATION",
	"FORCE_SENSOR",
	"FORCE_LIMITS",
	"PROBE_FLAGS",
	"MO_CAL",
	"VISION_EXTERNAL_ANALYSIS",
	"LAO_WARNINGS",
	"RFID_TAG_SET",
	"PROBE_UPDATE_NOW",
	"KHOLE_MAX_DISTANCE_CHECK",
	"MAX_EDGE_SHIFT_PROBE_ACCEPT",
	"ALLOW_DRILL_BEYOND_SHIFT_LIMITS",
	"NAC_SERIAL_NUMBER",
	"Y_RETRACT",
	"SYSTEM_COMPONENTS",
	"RFID_TAG_SET2",
	"OID_MAX_NUMBER_MiniFT"
};
//MakeOID-generated::END

//Now From Tool Module
//MakeOID-generated:: NAMES ALT (complete)
xstring xsOIDNAMESToolManagement {
	"TOOL_MGMT",
	"TOOL_REC",
	"OID_MN_ToolManagement"
};
//MakeOID-generated::END


unsigned long g_ulCutterAirTotal;

////////////////////////////////////////////////////////////////////////////////
// InitVars
// Description : Initialize All Global variables
////////////////////////////////////////////////////////////////////////////////
nodebug void InitVars(void)
{
    int i;
	long lBytes;

g_ulCutterAirTotal=1880*64;

	// gravity compensation mode, float, and jog
	g_cFloatStatus = FLOATSTAT_NOFLOAT;
	g_cGravCompStatus = GRAVCOMP_NOTDONE;
    g_GravComp.iDirX = 0;
    g_GravComp.iDirY = 0;

	//Probe mode
	g_Probe.cCaptureTrig=FALSE;

	// Probe home and Probe Adjust modes
    g_Probe.cHomeTrig=FALSE;
	g_Probe.fHomeX = 0.0;
	g_Probe.fHomeY = 0.0; //FIXME Minor  May not need these older home system values
	g_Probe.cGotHome = 0;
    g_Probe.cXBumperDirection=X_BUMP_UNKNOWN;
    g_Probe.cYBumperDirection=Y_BUMP_UNKNOWN;
    g_Probe.cRegistration=REGISTRATION_UNKNOWN;
    g_Probe.cProbeAdjustTrig=FALSE;
	g_Probe.fProbeAdjustX = 0.0;
	g_Probe.fProbeAdjustY = 0.0;
	g_Probe.cGotProbeAdjust = 0;

	// Probe Teach
//Save mem since nothing is using this now
//	strnull(g_Probe.szTeachPosnName);
    g_Probe.cTeachCaptureTrig=FALSE;
    g_Probe.cGotTeachCoords=0;

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
    g_PosnMode.iStartPosnIndex=0;
    g_PosnMode.iCurPosnIndex=0;
    g_PosnMode.cOnCurPosn=0;
	g_PosnMode.iGotoPosnIndex=-1;  // "next" position to go to...could be next, previous, or random
	g_PosnMode.cDoMoveTrig=FALSE;
	g_cMoveDone=MOVEDONE_TRUE; // initialize to true because carriage is not moving (it is "done moving") at power-up
	g_PosnMode.cFreshCurPosn = 0;
	g_PosnMode.cFreshCurPosnSend = 0;
	g_PosnMode.fLastKnownPosnX = 0.0;
	g_PosnMode.fLastKnownPosnY = 0.0;
	g_PosnMode.fPosnX=0.0;
	g_PosnMode.fPosnY=0.0;
	g_PosnMode.lPosnX=0;
	g_PosnMode.lPosnY=0;
	g_PosnMode.lPosnTime=0;
	g_PosnMode.cFreshPosn=0;
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
	memset(&g_VisionInspectResults,0,sizeof(td_VisionInspectResults));
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
	g_szTagDatalen=0;
	g_szTagData[0]=0;
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
	g_szToolIDlen=0;
	g_szToolID[0]=0;
	g_szToolTypeCodelen=0;
	g_szToolTypeCode[0]=0;
//MakeOID-generated::END

	g_ulDateTimeMS = 0;

	g_cActionSent = 0xFF;
	g_cHold = 0;

	#ifdef HOMESYSTEM_X_RFID
	g_fRFIDMachineX=0;
	g_fRFIDRailX=0;
	g_iRFIDRailOrientation=1;
	#endif

	#ifdef READ_TEMP
	g_fXTemp = 0;
	g_fYTemp = 0;
	#endif
	g_cPosnModeState = POSNMODE_INIT;
	g_cEstopPrevMode = MODE_IDLE;

	g_uiActionCycles = 0;
	g_iActionHoleIndex = -1;
	g_cSafeUnclamp=0;
	g_cSawDrillMode=0;
	g_cTestOpt=0;
	g_cLubeBypass=0;
	g_cFastDoneFlag = 0;
	g_cCutterDetected=CUTTER_UNKNOWN;
	g_cLastCutterDetected=CUTTER_UNKNOWN;
	g_uiMessageCode=0;

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
	g_ToolServerSTPSession=(td_STPsessions *)0;

	g_cToolCount = 0; //clear
    g_cProcessCount = 0;
    g_cFastenerCount = 0;
    g_cPatternCount = 0;
    g_iHoleCount = 0;
    g_cKHoleCount = 0;

	ClearProbeCommand();

	g_cKHolePrbeCommand = 0;
    g_cKHolePrbeCommandInput = 0;
	g_cKHolePrbeTrigger=0;
    g_cProbeComplete = 0;
	g_cProbeCalculated = 0;
	g_cKHoleLastCount = 0;
	//ClearGolbalRotation(); //just clear two variables
	g_cRotationKP = 0;
	g_cRotationKS = 0;
	g_cRotationContext = 0;

	g_cLoadedTool = 0; //FIXME NOW if tool ram, then exlucde this
	g_cLastSearchedRequiredTool = 0;

    g_cProcessLayerCount[0]=1; //Default Process
    g_cProcessMaterials[0]=0; //Should Really not supprt this for drilling, but
	g_uiProcessPounds[0]=50; //on simple machines there is no point in requiring process at all.
	g_uiProcessPoundsWarning[0]=0;
	g_uiProcessPoundsAbort[0]=0;
	g_cProcessOps[0]=255;
    g_uiProcessCountersink[0]=500;

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
	g_cStationCode="udspfmil##"; //For showing current station

	g_cDrillStatePrev = 255;
	g_ulDrillStateStartTime = 0;

	g_TeachMachinePosn.fX = 0.0;
	g_TeachMachinePosn.fY = 0.0;

    g_sPPDirect=(char*)0;
	g_iPPDirectPos=0;
	g_iPPDirectEnd=0;
	g_cPPDirectFinalCall=0;

	//HD + All Clamping Systems
#ifdef CLAMP_SYSTEM
	#ifdef CLAMP_SYSTEM_NAC_STP
	//only set state unknown
	g_cClampState=CLAMP_UNKNOWN;
	g_cClampGoal=CLAMP_UNKNOWN;
	#else
	g_cClampState=CLAMP_UNKNOWN;
	g_cClampGoal=CLAMP_LOOSE_OR_UNCLAMP;
	#endif
    g_cClampStateSent = 0xFF;
    g_cClampGoalSent = 0xFF;
#endif

	//HD
    // Assume initial HDRAIL settings are clamped to force transition to CLAMP_LOOSE at start-up.
	#ifdef CLAMP_SYSTEM_HD_PISTON
    g_cALock = ALOCK_ON;
	g_cClampExtend = VALVE_ON;
	g_cClampRetract = VALVE_ON;
	g_cLegsLock = LEGSLOCK_ON;
	g_cLegsDown = VALVE_OFF; //new default
    g_cLegsUp = VALVE_ON; //new default compatible with DFINT
	#endif
    g_cAirBlastX = 0;
    g_cAirBlastY = 0;
	g_uiClampPressure=100;
	g_uiClampPressureWarning=0;
	g_uiClampPressureAbort=0;

#ifdef CENTERVISION
	g_cCenterVisionResult = CENTERVISION_OFF;
#endif

	//CIRCMFT Drill Machine
	#ifdef GENCIRCMFTX
	#ifdef DIGOUT_CHNUM_DRILLUP
	g_cDrillUp = 0;
	#endif
	g_cDrillDown = 0;
	g_cDrillButton = 0;
	g_cColletClamp = 0;
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

	//ADC DAC information
	#ifdef READMOTORVOLTAGE
	g_fMotorVoltage=0;
	#endif

	//HOMESYSTEM  //FIXME0000000 fix common-order hierarchy
	#ifdef HOMESYSTEM
	g_cHomed = HOME_NOT_DONE;	//overall system home status
    g_cSentHomed = 0xFF;
    ClearAllHomeStatus();
	ClearSentHomeStatus();
	#endif


g_cImmediateFakeRfid=0; //temp feature may be added in nicer way later

	//Pending Ops
    g_cClear = 0;
	#ifdef CLAMP_SYSTEM_NAC_STP
	g_cNACClear = 0;
	#endif
//	g_cAction = 0;

	g_cJogX=0;
	g_cJogY=0;
	g_cJogGoalX=0;
	g_cJogGoalY=0;
	g_fJogX=0; //The status speed factor
	g_fJogY=0; //The status speed factor
	g_fJogGoalX=0; //The goal speed factor
	g_fJogGoalY=0; //The goal speed factor
	g_ulJogStopX=0;
	g_ulJogStopY=0;

	//Position Update
	g_ulPositionUpdateTime=0;
	g_ulPositionSendTime=0;
	g_ulPositionUpdateThrottle=POSITION_UPDATE_THROTTLE_DEFAULT;
	g_ulPositionSendThrottle=POSITION_SEND_THROTTLE_DEFAULT;
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

	g_iDrillDir=DRILLDIR_ATOB;

	ResetNearestPosition();

    g_cAllowKVisit=0;
    g_cAutoMove=0;
	g_ulAutoTime=0;
	g_ulStartAutoTime=0;
	g_cAutoRepeat=0;
    g_cLastAutoMove=3;
	g_cFloatGoal=255;
    g_cFloatExitModePosnState=0;
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

	g_cEthernetDownloaderEnabled = 0;

	g_cShowIO = 0;
	g_cShowAna = 0;

	//Call Function to init Part Program Vars
	ClearPartProgram();

	g_fMachineBaseExtension=0;

	p_cultemp = (char *)&g_ultemp;
    return;
}

////////////////////////////////////////////////////////////////////////////////
// InitConfig
////////////////////////////////////////////////////////////////////////////////
nodebug void InitConfig(void)
{
	g_cConfigLoadSuccess = 0; //indicate that this was not the config loaded from the user block
    //Config initialization
//MakeOID-generated:: CONFIG DEFAULTS (oid complete - except gen20 and gen25 options)
#ifdef GEN20
    g_ConfigData.EncoderRatio.fX = 252688.3;
    g_ConfigData.EncoderRatio.fY = 232200.1;
#endif
#ifdef GEN25
    g_ConfigData.EncoderRatio.fX = 245858.9;
    g_ConfigData.EncoderRatio.fY = 245858.9;
#endif
//#ifdef GENHD1
//	g_ConfigData.EncoderRatio.fX = 469365.9; //currently...
//	g_ConfigData.EncoderRatio.fY = 245858.9;
//#endif
//#ifdef GENHD2
//	//corrective numbers
//	//Need X to move 0.0005 more....
//	//469365.9*1.0005 = 469600
//	//Need Y to move 0.004 less
//	//469365.9*0.996 = 467488
//	g_ConfigData.EncoderRatio.fX = 469600; //Previously 469365.9 Previously 469367
//	g_ConfigData.EncoderRatio.fY = 467488; //Previously 469365.9 Previously 469367
//#endif
#ifdef GENHD3
	g_ConfigData.EncoderRatio.fX = 737577; //From Ratio conversion GEN25X * 3
	g_ConfigData.EncoderRatio.fY = 245859;
#endif
#ifdef GENHD4
	g_ConfigData.EncoderRatio.fX = 78013.145; //New Motor System
	g_ConfigData.EncoderRatio.fY = 78013.145;
#endif
#ifdef GENHD5
	g_ConfigData.EncoderRatio.fX = 78021.11289; //Slightly better calculation 1.0001021352234934253708141108784
	g_ConfigData.EncoderRatio.fY = 78021.11289;
#endif
#ifdef GENFD
#ifdef PRIMARY_ENCODER_PX
	g_ConfigData.EncoderRatio.fX = 208076.8; //Primary Encoder
    g_ConfigData.EncoderRatio.fY = 208076.8; //Primary Encoder
#endif
#ifdef PRIMARY_ENCODER_PY
	g_ConfigData.EncoderRatio.fX = 25400; //AUX Encoder
    g_ConfigData.EncoderRatio.fY = 25400; //AUX Encoder
#endif
#endif
#ifdef GENFLOORBEAM
	g_ConfigData.EncoderRatio.fX = 136169.6; //Primary Encoder From calc sheet
    g_ConfigData.EncoderRatio.fY = 254647.9; //Primary Encoder
#endif
#ifdef GENCIRCMFT1
	g_ConfigData.EncoderRatio.fX = 464672.241; //Previously 469365.9; //FIXME0
    g_ConfigData.EncoderRatio.fY = 537408.304; //based on scan   538971.3207; //theoretical value
#endif
#ifdef GENCIRCMFT2
	g_ConfigData.EncoderRatio.fX = 78013.145; //New Motor System
#ifdef PRIMARY_ENCODER_PX
	g_ConfigData.EncoderRatio.fY = 78013.145;
#endif
#ifdef PRIMARY_ENCODER_PY
	g_ConfigData.EncoderRatio.fY = 80000;
#endif
#endif
#ifdef GENFLOORBEAM
	g_ConfigData.EncoderRatio.fX = 126667;//These are good approximations from experimental values.
	g_ConfigData.EncoderRatio.fY = 256000;
#endif

	g_ConfigData.MCCurrentLimit.fX = 6.7;
	g_ConfigData.MCCurrentLimit.fY = 6.7;
#ifdef GENFD
	g_ConfigData.MCCurrentLimit.fX = 2.0; //low low current limits for small machine
	g_ConfigData.MCCurrentLimit.fY = 2.0;
#endif
#ifdef GENFLOORBEAM
	g_ConfigData.MCCurrentLimit.fX = 2.8;
	g_ConfigData.MCCurrentLimit.fY = 2.8;
#endif

	g_ConfigData.MCPeakCurrentLimit.fX = 10.0;
	g_ConfigData.MCPeakCurrentLimit.fY = 10.0;
#ifdef GENFD
	g_ConfigData.MCPeakCurrentLimit.fX = 2.4;  //low low current limits for small machine
	g_ConfigData.MCPeakCurrentLimit.fY = 2.4;
#endif
#ifdef GENFLOORBEAM
	g_ConfigData.MCPeakCurrentLimit.fX = 4;
	g_ConfigData.MCPeakCurrentLimit.fY = 4;
#endif
	g_ConfigData.uiBrakeOnTimeout = 4000;
	g_ConfigData.cEEOptionDefault = EEOPTION_DEFAULT;
	g_ConfigData.cbeeper = 1;
	g_ConfigData.cToolVerifyEnable = DEFAULT_TOOLVERIFY;
	g_ConfigData.fReturnHeight = 0;
	g_ConfigData.cScaleMode = 0;
	g_ConfigData.cGravCompAxes = 0;
	g_ConfigData.GravCompSpeed.fX = 0.45;
	g_ConfigData.GravCompSpeed.fY = 0.45;
	g_ConfigData.GravCompAcc.fX = 8.0;
	g_ConfigData.GravCompAcc.fY = 8.0;
	g_ConfigData.GravCompDec.fX = 8.0;
	g_ConfigData.GravCompDec.fY = 8.0;
	g_ConfigData.GravCompMoveDist.fX = 0.25;
	g_ConfigData.GravCompMoveDist.fY = 0.25;
	g_ConfigData.cGravCompAlgorithm = GC_ORIGINAL;
	g_ConfigData.GravCompNoiseLimit.fX = 4.0;
	g_ConfigData.GravCompNoiseLimit.fY = 4.0;
	g_ConfigData.GravCompTriggerFactor.fX = 0.1;
	g_ConfigData.GravCompTriggerFactor.fY = 0.1;
	g_ConfigData.fFloatSpeedLimit = 40;
	g_ConfigData.fJogSpeedLimit = 40;
	g_ConfigData.fMaxSpeedX = 1;
	g_ConfigData.fMaxSpeedY = 1;
#ifdef CENTERVISION_CAM
	g_ConfigData.cProbeMethodDefault = PROBE_CAM;
#else
	g_ConfigData.cProbeMethodDefault = PROBE_MANUAL;
#endif
	g_ConfigData.ProbeOffset.fX = 0;
	g_ConfigData.ProbeOffset.fY = 0;
	g_ConfigData.iProbeDir = PROBEDIR_ATOB;
	g_ConfigData.iDrillDir = DRILLDIR_SAME;
	g_ConfigData.fMaxKholeDistanceError = 0.1;
	g_ConfigData.fApproxLocationError = 0.6;
	g_ConfigData.cProbeAcceptRequired = 0;
#ifdef GENCIRCMFTX
	g_ConfigData.HomeSpeed.fX = 2.0;
	g_ConfigData.HomeSpeed.fY = 2.0;
#else
	g_ConfigData.HomeSpeed.fX = 0.5;
	g_ConfigData.HomeSpeed.fY = 0.5;
#endif
	g_ConfigData.HomeAcc.fX = 8.0;
	g_ConfigData.HomeAcc.fY = 8.0;
	g_ConfigData.HomeDec.fX = 8.0;
	g_ConfigData.HomeDec.fY = 8.0;
	g_ConfigData.HomeMoveDist.fX = 0.5;
	g_ConfigData.HomeMoveDist.fY = 0.5;
	g_ConfigData.fProbeAdjustLimit = 0.1;
	g_ConfigData.HomeFineSpeed.fX = 0.05;
	g_ConfigData.HomeFineSpeed.fY = 0.05;
	g_ConfigData.cMoveType = 3;
	g_ConfigData.PreMove.fX = 0.15;
	g_ConfigData.PreMove.fY = 0.15;
#ifdef OLD_SYSTEM_SPEED_DEFAULTS
	g_ConfigData.PosnSpeed.fX = 14.0;
	g_ConfigData.PosnSpeed.fY = 14.0;
	g_ConfigData.PosnAcc.fX = 16.0;
	g_ConfigData.PosnAcc.fY = 14.0;
	g_ConfigData.PosnDec.fX = 16.0;
	g_ConfigData.PosnDec.fY = 14.0;
	g_ConfigData.PosnFinalSpeed.fX = 3.0;
	g_ConfigData.PosnFinalSpeed.fY = 3.0;
#endif
#ifdef GENHDSPEEDS
	g_ConfigData.PosnSpeed.fX = 7.0; //FIXME00000000000 better defaults.
	g_ConfigData.PosnSpeed.fY = 7.0;
	g_ConfigData.PosnAcc.fX = 20.0;
	g_ConfigData.PosnAcc.fY = 20.0;
	g_ConfigData.PosnDec.fX = 20.0;
	g_ConfigData.PosnDec.fY = 20.0;
	g_ConfigData.PosnFinalSpeed.fX = 4.0;
	g_ConfigData.PosnFinalSpeed.fY = 4.0;
#endif
#ifdef GENFD
	g_ConfigData.PosnSpeed.fX = 1.0; //FIXME00000000000 better defaults.
	g_ConfigData.PosnSpeed.fY = 1.0;
	g_ConfigData.PosnAcc.fX = 10.0;
	g_ConfigData.PosnAcc.fY = 10.0;
	g_ConfigData.PosnDec.fX = 10.0;
	g_ConfigData.PosnDec.fY = 10.0;
	g_ConfigData.PosnFinalSpeed.fX = 1.0;
	g_ConfigData.PosnFinalSpeed.fY = 1.0;
#endif
#ifdef GENCIRCMFTX
	g_ConfigData.PosnSpeed.fX = 6.0;
	g_ConfigData.PosnSpeed.fY = 6.0;
	g_ConfigData.PosnAcc.fX = 8.0;
	g_ConfigData.PosnAcc.fY = 8.0;
	g_ConfigData.PosnDec.fX = 8.0;
	g_ConfigData.PosnDec.fY = 8.0;
	g_ConfigData.PosnFinalSpeed.fX = 2.0;
	g_ConfigData.PosnFinalSpeed.fY = 2.0;
#endif
	g_ConfigData.fOrthoSlope = 0.0;
	g_ConfigData.PosnErrLimit.fX = 0.1;
	g_ConfigData.PosnErrLimit.fY = 0.1;
	g_ConfigData.fPosnTolerance = 0.0025;
	g_ConfigData.VelErrLimit.fVLimitMarginX = 0.1;
	g_ConfigData.VelErrLimit.fVErrorX = 1;
	g_ConfigData.VelErrLimit.fVLimitMarginY = 0.1;
	g_ConfigData.VelErrLimit.fVErrorY = 1;
	g_ConfigData.LongDistance.fX = 12.0;
	g_ConfigData.LongDistance.fY = 12.0;
	g_ConfigData.LongSpeed.fX = 6.0;
	g_ConfigData.LongSpeed.fY = 6.0;
	g_ConfigData.cDrillHoleOneTime = 0;
	g_ConfigData.cToolFlip = Y_POS; //do not use UNKNOWN in RABBIT unless all code is protected
	g_ConfigData.cDriveThroughBacklash = 1;
	g_ConfigData.DrillOffset1.fX = 0;
	g_ConfigData.DrillOffset1.fY = 0;
	g_ConfigData.DrillOffset1.fYExtension = 0;
	g_ConfigData.DrillOffset2.fX = 0;
	g_ConfigData.DrillOffset2.fY = 0;
	g_ConfigData.DrillOffset2.fYExtension = 0;
	g_ConfigData.OffsetSeal.fx = 2.95;
	g_ConfigData.OffsetSeal.fy = 0.0;
	g_ConfigData.OffsetFill.fx = 6.37;
	g_ConfigData.OffsetFill.fy = 0.1;
#ifdef OLD_SYSTEM_SPEED_DEFAULTS
	g_ConfigData.JogSpeed.fX = 14.0;
	g_ConfigData.JogSpeed.fY = 14.0;
	g_ConfigData.JogAcc.fX = 6.0;
	g_ConfigData.JogAcc.fY = 6.0;
	g_ConfigData.JogDec.fX = 64.0;
	g_ConfigData.JogDec.fY = 64.0;
#endif
#ifdef GENHDSPEEDS
	g_ConfigData.JogSpeed.fX = 8;
	g_ConfigData.JogSpeed.fY = 4;
	g_ConfigData.JogAcc.fX = 8.0;
	g_ConfigData.JogAcc.fY = 8.0;
	g_ConfigData.JogDec.fX = 30.0;
	g_ConfigData.JogDec.fY = 30.0;
#endif
#ifdef GENFD
	g_ConfigData.JogSpeed.fX = 2.0;
	g_ConfigData.JogSpeed.fY = 2.0;
	g_ConfigData.JogAcc.fX = 2.0;
	g_ConfigData.JogAcc.fY = 2.0;
	g_ConfigData.JogDec.fX = 24.0;
	g_ConfigData.JogDec.fY = 24.0;
#endif
#ifdef GENCIRCMFT1
	g_ConfigData.JogSpeed.fX = 4.0;
	g_ConfigData.JogSpeed.fY = 4.0;
	g_ConfigData.JogAcc.fX = 4.0;
	g_ConfigData.JogAcc.fY = 4.0;
	g_ConfigData.JogDec.fX = 64.0;
	g_ConfigData.JogDec.fY = 10.0;
#endif
	//Default everthing else like this for now.
	g_ConfigData.HomePosnYPos.fX = 0.0;
	g_ConfigData.HomePosnYPos.fY = 0.0;
	g_ConfigData.PosnLimitYPos.fMinX = -1000.0;
	g_ConfigData.PosnLimitYPos.fMaxX = +1000.0;
	g_ConfigData.PosnLimitYPos.fMinY = -1000.0;
	g_ConfigData.PosnLimitYPos.fMaxY = +1000.0;
	g_ConfigData.HomePosnYNeg.fX = 0.0;
	g_ConfigData.HomePosnYNeg.fY = 0.0;
	g_ConfigData.PosnLimitYNeg.fMinX = 0.0;
	g_ConfigData.PosnLimitYNeg.fMaxX = 0.0;
	g_ConfigData.PosnLimitYNeg.fMinY = 0.0;
	g_ConfigData.PosnLimitYNeg.fMaxY = 0.0;
	g_ConfigData.cObstructionCodeMask = 255;
	g_ConfigData.cMachineLockRequired = (
		0
	#ifdef DIGIN_CHNUM_XLOCK
		| XLock
	#endif
	#ifdef DIGIN_CHNUM_YLOCK
		| YLock
	#endif
		);
	g_ConfigData.uiALockDelay = 50;
	g_ConfigData.uiAUnlockDelay = 80;
	g_ConfigData.uiLegsLockDelay = 50;		//	350;	// Attempt to speed up clamp cycle
	g_ConfigData.uiLegsUnlockDelay = 100;
	g_ConfigData.uiLegsUpDelay = 200;
	g_ConfigData.uiLegsDownDelay = 200;
	g_ConfigData.uiLowPressureDelay = 1200;
	g_ConfigData.uiLowPressureDownDelay = 500;
	g_ConfigData.uiPressureDelay = 1600;
	g_ConfigData.uiPressureDownDelay = 1;
	g_ConfigData.uiLowPressure = 300;
	g_ConfigData.uiPressure = 300;
#ifdef AIR_CLEAR
	g_ConfigData.uiAirClear = 1;
#else
	g_ConfigData.uiAirClear = 0;
#endif
#ifdef GENCIRCMFT1
	g_ConfigData.LaserSensorOffset.fX = -4.0;
	g_ConfigData.LaserSensorOffset.fY = 0.0;
#else
#ifdef GENCIRCMFT2
	g_ConfigData.LaserSensorOffset.fX = 1.644; //approximate
	g_ConfigData.LaserSensorOffset.fY = 0.167;
#else
	g_ConfigData.LaserSensorOffset.fX = -4.331284;
	g_ConfigData.LaserSensorOffset.fY = -0.108;
#endif
#endif
	g_ConfigData.CamOffset.fX = 5;
	g_ConfigData.CamOffset.fY = 0;
	g_ConfigData.LaserSensorAlgParam.fsearch_speed = 1;
	g_ConfigData.LaserSensorAlgParam.fseek_speed = 0.6;
	g_ConfigData.LaserSensorAlgParam.frscan_speed = 0.025;
	g_ConfigData.LaserSensorAlgParam.frscan_speed_fast = 0.025;
	g_ConfigData.LaserSensorAlgParam.fscan_speed = 0.025;
	g_ConfigData.LaserSensorAlgParam.fscan_speed_fast = 0.025;
	g_ConfigData.LaserSensorAlgParam.fprobe_diameter = 0.7;
	g_ConfigData.LaserSensorAlgParam.funknown_diameter = 0.7;
	g_ConfigData.LaserSensorAlgParam.cmode = LSMETHOD_DELTA;
	g_ConfigData.LaserSensorAlgParam.cmode_fast = LSMETHOD_DELTA;
	g_ConfigData.LaserSensorAlgParam.cuse_avg = 1;
	g_ConfigData.LaserSensorAlgParam.cfull_scan = 0;
	g_ConfigData.LaserSensorAlgParam.cgdata_sel = 1;
	g_ConfigData.LaserSensorAlgParam.cassume_posn = 0;
	g_ConfigData.LaserSensorAlgParam.cassume_posn_fast = 0;
	g_ConfigData.LaserSensorAlgParam.crect_center = 0;
	g_ConfigData.LaserSensorAlgParam.cloops = 22;
	g_ConfigData.LaserSensorAlgParam.cdelta_mode = DELTA_BASE;
	g_ConfigData.LaserSensorAlgParam.idelta_flat = 4;
	g_ConfigData.LaserSensorAlgParam.fdelta_basespan = 0.08;
	g_ConfigData.LaserSensorAlgParam.idelta_pos = 100;
	g_ConfigData.LaserSensorAlgParam.idelta_neg = -50;
	g_ConfigData.LaserSensorAlgParam.fdelta_span = 0.035;
	g_ConfigData.LaserSensorAlgParam.fdelta_edge = 0.0;
	g_ConfigData.LaserSensorAlgParam.fpc_aspect_diff = 0.004;
	g_ConfigData.LaserSensorAlgParam.fmax_aspect_diff = 0.004;
	g_ConfigData.LaserSensorAlgParam.fmax_over_exp_diameter = 0.5;
	g_ConfigData.LaserSensorAlgParam.fmax_under_exp_diameter = 0.5;
	g_ConfigData.LaserSensorAlgParam.fmax_csnk_diff = 0.001;
	g_ConfigData.LaserSensorAlgParam.fmax_over_csnk = 0.001;
	g_ConfigData.LaserSensorAlgParam.fmax_under_csnk = 0.001;
	g_ConfigData.CamAlgParam.fmove_speed = 1;
	g_ConfigData.CamAlgParam.cInfoMask = 1;
	g_ConfigData.CamAlgParam.cAMode = 0;
	g_ConfigData.CamAlgParam.cCMode = 0;
	g_ConfigData.CamAlgParam.cAux1 = 0;
	g_ConfigData.CamAlgParam.fmove_required = 0.0005;
	g_ConfigData.CamAlgParam.fmax_over_exp_diameter = 0.5;
	g_ConfigData.CamAlgParam.fmax_under_exp_diameter = 0.5;
	g_ConfigData.CamAlgParam.fmax_csnk_diff = 0.001;
	g_ConfigData.CamAlgParam.fmax_over_csnk = 0.001;
	g_ConfigData.CamAlgParam.fmax_under_csnk = 0.001;
	g_ConfigData.cVisionAutoInspect = 0;
	g_ConfigData.cProcessContinueMode = PROCESS_SINGLE;
	g_ConfigData.uiProcessOperations = 0;
	g_ConfigData.cSealClamp = 0;
	g_ConfigData.iSealPressureDelay = 800;
	g_ConfigData.iSealPressureReleaseDelay = 100;
	g_ConfigData.PrimeDelay.fdiameter1 = 0.1875;
	g_ConfigData.PrimeDelay.idelay1 = 1875;
	g_ConfigData.PrimeDelay.fdiameter2 = 0.25;
	g_ConfigData.PrimeDelay.idelay2 = 2500;
	g_ConfigData.PrimeDelay.fdiameter3 = 0;
	g_ConfigData.PrimeDelay.idelay3 = 0;
	g_ConfigData.PrimeDelay.fdiameter4 = 0;
	g_ConfigData.PrimeDelay.idelay4 = 0;
	g_ConfigData.PrimeDelay.fdiameter5 = 0;
	g_ConfigData.PrimeDelay.idelay5 = 0;
	g_ConfigData.iSealGlobDelay = 3000;
	g_ConfigData.iSealApplyDelay = 1000;
	//These few are no longer used for fill, so they are just place holders for OIDs... new OIDs could replace these if needed for fill or anything
	g_ConfigData.cFillClamp_ = 0;
	g_ConfigData.iFillExtendDelay_ = 100;
	g_ConfigData.iFillRamDelay_ = 200;
	g_ConfigData.iFastenerPostDelay_ = 200;
	g_ConfigData.fToolZBase = 2.0;
	g_ConfigData.PosnDisplay.cmode = PD_DATASET;
	g_ConfigData.PosnDisplay.corigin = PD_ZERO;
	g_ConfigData.PosnDisplay.ccontent = PD_XY;
	g_ConfigData.RFIDConfig.cenabled = 1;
	g_ConfigData.RFIDConfig.cmethod = FIRST_DETECTION;
	g_ConfigData.RFIDConfig.uioptions = 0;
	g_ConfigData.RFIDConfig.uicontinuousReadCycleTime = 250;
	g_ConfigData.RFIDConfig.uiseekReadCycleTime = 50;
	g_ConfigData.RFIDConfig.fseekMove1 = -4;
	g_ConfigData.RFIDConfig.fseekMove2 = 4;
	g_ConfigData.RFIDConfig.fseekFineMove = 1;
	g_ConfigData.RFIDConfig.fseekSpeed = 1;
	g_ConfigData.RFIDConfig.fseekFineSpeed = 0.1;
	g_ConfigData.RFIDConfig.fRFIDOffset = 7;
	g_ConfigData.RFIDConfig.fseekPastBorder = 0.25;
	g_ConfigData.RFIDConfig.fminWindowSize = 0.1;
	g_ConfigData.RFIDConfig.fmaxWindowSize = 0.15;
	g_ConfigData.uiEStopClearDelay=DELAY_AFTER_ESTOP_DEFAULT;
	g_ConfigData.uiDrillButtonDelay = 300;
	g_ConfigData.cUseCutterDetect = 1;
	g_ConfigData.cJogEnableTimeout = 200;
	g_ConfigData.uiDrillCycleDelay = 6000;
	g_ConfigData.cInspectMethod = INSPECT_MANUAL;
	g_ConfigData.cCommandInspectMethod = INSPECT_MANUAL;
	g_ConfigData.ForceSensorCalibration.iZeroX = 2000;
	g_ConfigData.ForceSensorCalibration.iZeroY = 2000;
	g_ConfigData.ForceSensorCalibration.iZeroZ = 2000;
	g_ConfigData.ForceSensorCalibration.iCountsPerGX = 2000;
	g_ConfigData.ForceSensorCalibration.iCountsPerGY = 2000;
	g_ConfigData.ForceSensorCalibration.iCountsPerGZ = 2000;
	g_ConfigData.ForceLimits.uiSensorInterval = 100;
	g_ConfigData.ForceLimits.uiMinUpdateDelta = 200;
	g_ConfigData.ForceLimits.cActive = 0;
	g_ConfigData.ForceLimits.cCurrentUnderMethod = 0;
	g_ConfigData.ForceLimits.uiCurrentOverX = 2000;
	g_ConfigData.ForceLimits.uiCurrentUnderX = 2000;
	g_ConfigData.ForceLimits.uiCurrentOverY = 2000;
	g_ConfigData.ForceLimits.uiCurrentUnderY = 2000;
	g_ConfigData.ForceLimits.uiFullGravX = 2000;
	g_ConfigData.ForceLimits.uiFullGravY = 2000;
	g_ConfigData.ForceLimits.uiFlatForceX = 300;
	g_ConfigData.ForceLimits.uiFlatForceY = 300;
	g_ConfigData.cProbeFlags = AUTO_COMPLETE | AUTO_MOVE_PROBE;
#ifdef OBSTRUCTION_SYSTEM_MO_ANA
	g_ConfigData.MOCal.uim1 = 512;
	g_ConfigData.MOCal.uim2 = 512;
	g_ConfigData.MOCal.uim3 = 512;
	g_ConfigData.MOCal.uim4 = 512;
	g_ConfigData.MOCal.uim5 = 512;
	g_ConfigData.MOCal.uim6 = 512;
#endif
	g_ConfigData.fMaxKholeDistanceCheck = 48;
	g_ConfigData.fMaxEdgeShiftProbeAccept = 0.5;
	g_ConfigData.cAllowDrillBeyondShiftLimits = 0;
	g_ConfigData.SystemComponents.cDrill = 1;
	g_ConfigData.SystemComponents.cFastener = 0;
	g_ConfigData.SystemComponents.cFastenerTray = 0;
	g_ConfigData.SystemComponents.cAux1 = 0;
	g_ConfigData.SystemComponents.cAux2 = 0;
	g_ConfigData.SystemComponents.cAux3 = 0;
	g_ConfigData.SystemComponents.cAux4 = 0;
	g_ConfigData.SystemComponents.cAux5 = 0;
//MakeOID-generated::END

	//Right after InitConfig always copy the default into certain variables
	g_cProbeMethod = g_ConfigData.cProbeMethodDefault;
}

////////////////////////////////////////////////////////////////////////////////
// SockInit
////////////////////////////////////////////////////////////////////////////////
nodebug void SockInit(void)
{
	//Call this only one time, or networking will develop an error
    //in it's TCPIP Config that will effect some kinds of communication.
	if (sock_init() != 0)
	{
		#ifdef OUTPUT_ENET
	    logf("sock_init() fail\r\n");
	    #endif
	}
}

////////////////////////////////////////////////////////////////////////////////
// InitEthernet
////////////////////////////////////////////////////////////////////////////////
nodebug void InitEthernet(void)
{
	int iReturnVal;
	static int s_iState;
	unsigned long s_ulStartTime;

	#ifdef OUTPUT_ENET
	logf("enet\r\n");
	#endif
    s_ulStartTime = SEC_TIMER;  //capture time for timeout
	while (ifconfig(IF_ETH0, IFS_UP, IFS_END) != 0)
    {
		#ifdef OUTPUT_ENET
        logf("fail\r\n");
        #endif
    }
    while(ifpending(IF_ETH0) != IF_UP)
    {
		tcp_tick(NULL); // call tcp_tick
		if ((SEC_TIMER - s_ulStartTime) > IF_ETH0_TIMEOUT_SEC )
        {
			#ifdef OUTPUT_ENET
            logf("Timeout\r\n");
            #endif
            s_ulStartTime = SEC_TIMER;  //capture time for timeout
        }
    }
	#ifdef OUTPUT_ENET
	logf( "Up. ifp=%d\r\n",ifpending(IF_ETH0));
	#endif
    return;
}

////////////////////////////////////////////////////////////////////////////////
// SessionStarting
////////////////////////////////////////////////////////////////////////////////

nodebug void SessionStarting( td_STPsessions * p_STPSession )
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
	if ( iSession == -3)
	{
		//df
		g_cDrillSync = DRILL_SYNC; //no sync requirements for df
		return;
	}
	else if ( iSession == -4)
	{
		FastenerResync();
	}
	else if ( iSession == -5)
	{
		FastenerTrayResync();
	}
}

////////////////////////////////////////////////////////////////////////////////
// SessionDelay
////////////////////////////////////////////////////////////////////////////////

nodebug void SessionDelay( td_STPsessions * p_STPSession )
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

nodebug void SessionClosing( td_STPsessions * p_STPSession )
{
	int iSession;
	#ifdef OUTPUT_SESSION_EVENTS
	logf("S Cls\r\n");
	#endif

	iSession = p_STPSession->iSessionNum;
    if ( iSession < 0)
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
	if ( iSession == -3)
	{
		//df
		g_cDrillSync = 0;
		//Must Load Process Again
		if (g_cDrillLoadProcessAndOverride>DRILL_LOAD_PROCESS_NEEDED)
		{
			g_cDrillLoadProcessAndOverride = DRILL_LOAD_PROCESS_NEEDED;
		}
		return;
	}
	}
	//Stop Interface Tasks 1st.
	StopInterfaceTasks();

	//Stop Any Auto Move
  	g_cAutoMove=0;
	g_ulStartAutoTime=0;
	g_cAutoRepeat=0;

	//FIXME DISABLE  Rivet had a nice disable system that could disable the entire thing by a call to one function
	//	MiniFT might need a disable/enable like that some day, but it would be different in some ways.
	//If I do it I should review RivetDisable from the archives

	g_cSendPPDataSessions = 0; //FIXME SEVERE  really need to stop ONLY this session
   	g_cSendXYDataSession[iSession] = 0;
	g_cSendOpHistoryBlockOrderIndex[iSession] = 0xFF;

	if (p_STPSession==g_ToolServerSTPSession)
	{
		g_ToolServerSTPSession=(td_STPsessions *)0; //clear this
	}
}

////////////////////////////////////////////////////////////////////////////////
// Stop Interface High Speed Communication Dependant Tasks
////////////////////////////////////////////////////////////////////////////////

nodebug void StopInterfaceTasks()
{
	//Stop Jog
logf("si\r\n");
    g_cJogGoalY = JOGSTOP;
    g_cJogGoalX = JOGSTOP;
}

#ifdef DRILL_DIRECT_READY
//Resent Drill Sync Variables
nodebug void ClearDrillSync()
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

////////////////////////////////////////////////////////////////////////////////
// InitDIO
////////////////////////////////////////////////////////////////////////////////
nodebug void InitDIO(void)
{
    digOutConfig(DIGOUT_CONFIG);

    // if a digital input is by default pulled up to 5V (high) and then
    // grounded (low) when a switch is pressed, then the initial
    // state of these switches should be "1".  if not, then the default should be 0.

#ifdef ESTOPV1B
	g_cDigInEstopXCount=1;
	g_cDigInEstop=0; //init to low.
#endif
#ifdef UNIBUTTON
	g_cDigInUniButtonXCount=1;
	g_cDigInUniButton=0;// normally low, pulled high, initialize to all low
#endif
	g_cDigInEstopSwitchXCount=1;
	g_cDigInEstopSwitch=1; // normally high, pulled low.

	g_cMachineLock=0;
	#ifdef DIGIN_CHNUM_XLOCK
	g_cDigInXLockXCount=1;
	g_cDigInXLock=1; //normally pulled high, on goes low
	#endif
	#ifdef DIGIN_CHNUM_YLOCK
	g_cDigInYLockXCount=1;
	g_cDigInYLock=1; //normally pulled high, on goes low
	#endif
    #ifdef GENCIRCMFTX
	g_cDigInDrillUp=0; //no count
	g_cDigInDrillDown=0; //no count
    #endif
	#ifdef Y_LIMIT_SENSORS
	g_cDigInYPosLimit=1; //no count
	g_cDigInYNegLimit=1; //no count
	#endif

	#ifdef PRESSURESENSOR
	g_cDigInAirPressureXCount=1;
	g_cDigInAirPressure=0; //start out in OK state
	#endif

	#ifdef ORIENTATION_SENSORS
	g_cDigInYOrientationA = 1; //no count
	g_cDigInYOrientationB = 1; //no count
	#endif
	g_cPrevOrSensors = 0xFF;

	//Obstruction System
	g_cObstructionCode=0;
	g_cLastObstructionCode=0;
	g_cObstructionCodeNew=0;
	g_cObstructionWarningCode=0;
	g_cLastObstructionWarningCode=0;
	#ifdef OBSTRUCTION_SYSTEM_XP1
	g_cDigInObstructionXP1=NO_OBSTRUCTION;
	#endif
	#ifdef OBSTRUCTION_SYSTEM_XN1
	g_cDigInObstructionXN1=NO_OBSTRUCTION;
	#endif
	#ifdef OBSTRUCTION_SYSTEM_MOW
	g_cDigInObstructionMOW=MO_NO_OBSTRUCTION;
	#endif
	#ifdef OBSTRUCTION_SYSTEM_MOS
	g_cDigInObstructionMOS=MO_NO_OBSTRUCTION;
	#endif
	#ifdef OBSTRUCTION_SYSTEM_MO_ANA
	g_cMOFlags = 0;
	#endif
	#ifdef DIGIN_SOFTSTOP
	g_cDigInSoftStop = 1;
	#endif

	#ifdef UNIBUTTON
	g_cUniButtonEvent=0; //used to trap button press
    #endif

	g_cLockEvent = 0;

	#ifdef PRESSURESENSOR
	g_cAirPressureEvent = 2;  //because it can watch for both directions
	#endif

	//Obstruction System
	g_cObstructionEvent = 0;
	g_cMCObstructionEvent = 0;
	g_cObstructionWarningEvent = 0;

	g_cBrakeReleased = 0;

	g_cEstopMCAsserted = 0;
	g_cEstopFullyEngaged = 1;

	//set managed channels
    LEDOff();
    BeepOff();

    //set brake on at boot
    BrakeOn();

	//This assumes the outputs are off already because they default to off.

    SetDefaultValveState();
}

nodebug void SetDefaultValveState()
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
		g_cClampState=CLAMP_UNKNOWN; //true init state
		g_cClampGoal=CLAMP_LOOSE_OR_UNCLAMP; //and make sure it goes to LOOSE
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
nodebug void InitADC(void)
{
    // analog inputs configured by calling brdInit() which is done in main()
    // keep this state, though, in case we want to do more
    // advanced initialization of the ADC
    // default setup for ADC on BL2100 is -10V to 10V, 12bits
}

////////////////////////////////////////////////////////////////////////////////
// DigOutService
////////////////////////////////////////////////////////////////////////////////
nodebug void DigOutService(void)
{
   	unsigned long ulNow;
    unsigned long ul;
	unsigned int ui;
    static char s_cLEDSet;
    static char s_cBeepSet;

    #GLOBAL_INIT
    {
    	s_cLEDSet=2; //ensure 1st value will be obeyed even if it is 1 or 0
    	s_cBeepSet=2;
    }

#ifdef LEDSYSTEM
    if (g_cLEDCount>0)
    {
    	ulNow=MS_TIMER;
        ul = ulNow - g_ulLEDStart;
        while (ul>=g_uiLEDWhole)
        {
			g_ulLEDStart += g_uiLEDWhole;
            ul -= g_uiLEDWhole;
            g_cLEDCount--;
            if (g_cLEDCount==0) { goto completed_LED_count; }
        }
		if (ul < g_uiLEDOn)
        {
        	g_cLED = 1;
        }
        else
        {
completed_LED_count:
        	g_cLED = 0;
        }
    }
    if (s_cLEDSet!=g_cLED)
    {
		digOut(DIGOUT_CHNUM_LED, g_cLED);
    	s_cLEDSet=g_cLED;
    }
#endif

#ifdef BEEPSYSTEM
   	if (g_ConfigData.cbeeper==0)
    {
       	g_cBeepCount=0;
       	g_cBeepMode=BEEPOFF;
        g_cBeep=0;
    }
    else if (g_cBeepCount>0)
    {
    	ulNow=MS_TIMER;
        ul = ulNow - g_ulBeepStart;
        while (ul>=g_uiBeepWhole)
        {
			g_ulBeepStart += g_uiBeepWhole;
            ul -= g_uiBeepWhole;
            if (g_cBeepCount<255)
            {
            	g_cBeepCount--;
    	        if (g_cBeepCount==0)
	            {
        	    	g_cBeepMode=BEEPOFF;
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
completed_Beep_count:
			g_cBeep = 0;
        }
    }
    if (s_cBeepSet!=g_cBeep)
    {
		digOut(DIGOUT_CHNUM_BEEPER, g_cBeep);
    	s_cBeepSet=g_cBeep;
        //I don't like this being here, but it's the only single mode that needs this feature
        //This location makes it operate only once a cycle so there is no concern about CPU time
		if (g_cBeepMode==BEEPPROBEK && g_cBeep==1)
   	    {
           	//Special pattern 2 sections 120ms and 880ms. Beep at start of each
            //	For the Setting specified by BeepProbeK2 macro.
          	//Setup a special pattern for every other time.
            //Change the duration of the cycle each time
            if (g_uiBeepWhole>120) { g_uiBeepWhole=120; } else { g_uiBeepWhole=880; }
        }
    }
#endif
}

#ifdef BEEPSYSTEM
nodebug void Beep()
{
    BeepCountPrivate(BEEPONCE,1, 500, 800);
}
#endif

nodebug void BrakeOn()
{
	//Brake control may be by one or both of Rabbit and MC
	#ifdef RABBITBRAKES
    g_cBrakeReleased = 0;
	digOut(DIGOUT_CHNUM_BRAKE, 0);
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
nodebug void BrakeOff()
{
	//Brake control may be by one or both of Rabbit and MC
	#ifdef RABBITBRAKES
    g_cBrakeReleased = 1;
	digOut(DIGOUT_CHNUM_BRAKE, 1);
	#ifdef DIGOUT_CHNUM_BRAKE_ALTERNATE
	digOut(DIGOUT_CHNUM_BRAKE_ALTERNATE, 1);
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
char g_cFAKETEST;

nodebug void ReadDigitalInputs(void)
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
#ifdef ESTOPV1B
	#ifdef ANALOG_MOTORVOLTAGE_ESTOP_DETECT
    //do NOT check the estop motor power this way
    #else
    cv=digIn(DIGIN_CHNUM_ESTOP);
    if (cv!=g_cDigInEstop)
	{
    	//disagreement ... less common path
    	g_cDigInEstopXCount++;
    	if (g_cDigInEstopXCount>=DIGIN_DEBOUNCE_HISTORY_COUNT)
    	{
    		//now reach the count... flip
    		g_cDigInEstop=cv;
    		logf("Estop=%d\r\n",(int)cv);
	    	g_cDigInEstopXCount=0;
    	}
    }
    else
    {
    	//agreement
    	g_cDigInEstopXCount=0;
    }
    #endif
#endif
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

    cv=digIn(DIGIN_CHNUM_ESTOP_SWITCH);
    if (cv!=g_cDigInEstopSwitch)
	{
    	//disagreement ... less common path
    	g_cDigInEstopSwitchXCount++;
    	if (g_cDigInEstopSwitchXCount>=DIGIN_DEBOUNCE_HISTORY_COUNT)
    	{
    		//now reach the count... flip
    		g_cDigInEstopSwitch=cv;
	    	g_cDigInEstopSwitchXCount=0;
    	}
	}
	else
	{
    	//agreement
    	g_cDigInEstopSwitchXCount=0;
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
	if (g_cLockEvent==1) //happens only when changed or very recently changed on loop  before event is cleared
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
	g_cDigInDrillUp = digIn(DIGIN_CHNUM_DRILLUP);
	//REMOVED g_cDigInDrillDown = digIn(DIGIN_CHNUM_DRILLDOWN);
	#endif
	#ifdef Y_LIMIT_SENSORS
	//Y Limit
	g_cDigInYPosLimit=digIn(DIGIN_CHNUM_Y_POS_LIMIT);
	g_cDigInYNegLimit=digIn(DIGIN_CHNUM_Y_NEG_LIMIT);
	#endif

	#ifdef OBSTRUCTION_SYSTEM_XP1
	g_cDigInObstructionXP1=digIn(DIGIN_OBSTRUCTION_XP1);
	#endif
	#ifdef OBSTRUCTION_SYSTEM_XN1
	g_cDigInObstructionXN1=digIn(DIGIN_OBSTRUCTION_XN1);
	#endif
	#ifdef OBSTRUCTION_SYSTEM_MOW
	g_cDigInObstructionMOW=digIn(DIGIN_OBSTRUCTION_MOW);
	#endif
	#ifdef OBSTRUCTION_SYSTEM_MOS
	g_cDigInObstructionMOS=digIn(DIGIN_OBSTRUCTION_MOS);
	#endif

	g_cObstructionCode=0;
	#ifdef OBSTRUCTION_SYSTEM_XP1
	if (g_cDigInObstructionXP1==OBSTRUCTION) { g_cObstructionCode |= 1; }
	#endif
	#ifdef OBSTRUCTION_SYSTEM_XN1
	if (g_cDigInObstructionXN1==OBSTRUCTION) { g_cObstructionCode |= 2; }
	#endif
	#ifdef OBSTRUCTION_SYSTEM_MOS
	if (g_cDigInObstructionMOS==MO_OBSTRUCTION) { g_cObstructionCode |= 4; }
	#endif
	#ifdef OBSTRUCTION_SYSTEM_MO_ANA
	if (g_cMOFlags>0) { g_cObstructionCode |= 4; }
	#endif

	#ifdef Y_LIMIT_SENSORS
    if (g_cDigInYPosLimit==0) { g_cObstructionCode |= 16; }
    if (g_cDigInYNegLimit==0) { g_cObstructionCode |= 32; }
    #endif
    if ((g_cMCObstructionEvent & X_AXIS) > 0) { g_cObstructionCode |= 64; }
    if ((g_cMCObstructionEvent & Y_AXIS) > 0) { g_cObstructionCode |= 128; }
	g_cMCObstructionEvent = 0; //clear MC obstruction events now that they have been recieved and merged here

   	g_cObstructionCode &= g_ConfigData.cObstructionCodeMask;

	if (g_cObstructionCode != g_cLastObstructionCode)
	{
		cv=g_cObstructionCode ^ g_cLastObstructionCode; //cv has a 1 for any flag that changed
		#ifdef OUTPUT_OBSTRUCTION_CODE
		logf("oc=%x\r\n", g_cObstructionCode);
		#endif
		cv = cv & g_cObstructionCode; //cv now has a 1 for any flag that is changed, and is now set
		//next set g_cObstructionCodeNew with 1 for any new flags but retain previously new flags not handled.
		g_cObstructionCodeNew = g_cObstructionCodeNew | cv;

		g_cLastObstructionCode=g_cObstructionCode; //update the last check
		g_cObstructionEvent=1; //mark an event to process
	}

	//With only one source of warnings so far, the warning code is much simpler
	g_cObstructionWarningCode=0;
	#ifdef OBSTRUCTION_SYSTEM_MOW
	if (g_cDigInObstructionMOW==MO_OBSTRUCTION) { g_cObstructionWarningCode = 4; }
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
	g_cDigInYOrientationA = digIn(DIGIN_CHNUM_YORIENTATION_A);//no count
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

nodebug void EstopEngageActions(void)
{
	BrakeOn();
#ifdef ESTOPV1B
   	if (g_cDigInEstop == 0 || g_cEstopMCAsserted == 1)
#endif
#ifdef ESTOPV2
   	if (g_cEstopMCAsserted == 1)
#endif
    {
    	//No Motor Power, so run EstopEngage Right Now
		RunEstopEngage();
       	g_cEstopFullyEngaged = 1;
		#ifdef OUTPUT_ESTOP
        logf("Estop Full\r\n");
		#endif
    }
    else
    {
    	//Still Have Motor Power, so Run Servo Here
#ifdef GALIL
		RunEstopEngage();
#else
		RunEstopWithPower();
#endif
       	g_cEstopFullyEngaged = 0;
		#ifdef OUTPUT_ESTOP
        logf("Estop W Pow\r\n");
		#endif
    }

	g_cAction = ACTION_ESTOP;

	ClearToolOffset();//FIXME00000 is this a problem for returning to position mode???
	SetDefaultValveState();

	#ifdef CLAMP_SYSTEM_NAC_STP
	//Do almost nothing in the case of Estop, but do cancel any goal. NAC will set state to hold in most cases also
	if (g_cClampGoal!=g_cClampState)
	{
		g_cClampGoal=CLAMP_HOLD;
	}
	#endif

	g_PosnMode.cOnCurPosn=0;	//moving actions always mean not on position
	SendCurPosnInd(STP_ALERT); //Send Cleared Cur Posn Index

	g_cMoveDone = MOVEDONE_TRUE;  // not moving now.
    SmartToolMsgChar(STP_ALERT, OID_POSNMODE_MOVEDONE, g_cMoveDone);
    g_cFloatStatus = FLOATSTAT_NOFLOAT;
    SmartToolMsgChar(STP_ALERT, OID_FLOAT_STATUS, g_cFloatStatus);
#ifdef BEEPSYSTEM
	if (g_cBeepMode < BEEPSIGNALS)
	{
    	BeepOff();
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
    SmartToolMsgChar(STP_ALERT, OID_GRAVCOMP_STATUS, g_cGravCompStatus);

	#ifdef CENTERVISION
	g_cCenterVisionResult = CENTERVISION_OFF;
	#endif

	#ifdef GENCIRCMFTX
	//DO NOT RESET DRILL STATE	g_cDrillState = DRILLSTATE_IDLE;
	g_cDrillButton = 0;
	digOut(DIGOUT_CHNUM_DRILLBUTTON, g_cDrillButton);
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

nodebug void EstopContinueActions(void)
{
	if (g_cEstopFullyEngaged != 1)
    {
    	//Still using Power
#ifdef ESTOPV1B
   		if (g_cDigInEstop == 0 || g_cEstopMCAsserted == 1)
#endif
#ifdef ESTOPV2
   		if (g_cEstopMCAsserted == 1)
#endif
		{
			//No Motor Power, so run EstopEngage Right Now
			RunEstopEngage();
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
#ifdef ESTOPV1B
       	if (g_cDigInEstopSwitch == 0)
        {
			//switch is in
        	g_cEstopMCAsserted = 0;
        }
#endif
#ifdef ESTOPV2
       	if (g_cDigInEstopSwitch == 1)
        {
        	//Estop is signaled
        	g_cEstopMCAsserted = 0;
        }
#endif
	}

	g_cAction = ACTION_ESTOP;

    return;
}

nodebug void EstopDisengageActions(void)
{
	RunEstopDisengage();
	#ifdef GENCIRCMFTX
	g_cDrillState = DRILLSTATE_INIT;
	#endif
   	g_cEstopFullyEngaged = 0;
}

nodebug void PressureLossActions(void)
{
return; //FIXME0 Want to test later.......
//FIXME00000000000 because when it was on there was a major issue......
//  BUT I NEED SOME OF THIS TO BE SAFE.... TEST LATER....

	BrakeOn();

	g_PosnMode.cOnCurPosn=0;	//moving actions always mean not on position
	SendCurPosnInd(STP_ALERT); //Send Cleared Cur Posn Index

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
    	BeepOff();
	}
    #endif
    //stop float
   	if (g_cFloatStatus == FLOATSTAT_FLOAT)
    {
		   DoFloat(FLOAT_UNFLOAT_STOP);
    }
	g_cFloatExitModePosnState=0;
    //stop jog
	g_cJogGoalX = JOGSTOP;
	g_cJogGoalY = JOGSTOP;

    //force them to grav comp again by deciding it has not been done yet.
    g_cGravCompStatus = GRAVCOMP_NOTDONE;
    SmartToolMsgChar(STP_ALERT, OID_GRAVCOMP_STATUS, g_cGravCompStatus);

	#ifdef CENTERVISION
	g_cCenterVisionResult = CENTERVISION_OFF;
	#endif

	#ifdef GENCIRCMFTX
	g_cDrillButton = 0;
	digOut(DIGOUT_CHNUM_DRILLBUTTON, g_cDrillButton);
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

nodebug void ReadADC(void)
{
    static unsigned long s_ulLastMotorPowerSampleTime;
    static unsigned long s_ulLastSampleTime;
    unsigned long ulTime;
    unsigned long ulSampleInterval;
	unsigned int ui;
	char c;
	char cmo;
	float f;
	#ifdef GENHDACCELEROMETERS
    static float fmin;
    static float fmax;
    float fa;
    float fb;
    float fc;
    static float famin;
    static float famax;
    static float fbmin;
    static float fbmax;
    static float fcmin;
    static float fcmax;
    #endif
	#ifdef OBSTRUCTION_SYSTEM_MO_ANA
	static char s_cLastMoCheck;
	static char s_cLastMoPrint;
	#endif

    #GLOBAL_INIT
    {
    	s_ulLastMotorPowerSampleTime = 0;
        s_ulLastSampleTime = 0;
		#ifdef GENHDACCELEROMETERS
        fmin=100;
        fmax=-100;
		famin=100;
		famax=-100;
		fbmin=100;
		fbmax=-100;
		fcmin=100;
		fcmax=-100;
	    #endif
    }

	#ifdef READ_TEMP
    ulTime = MS_TIMER;
    ulSampleInterval = ulTime - s_ulLastSampleTime;
    if (ulSampleInterval > 60000L)
    {
    	//Only Read These When Needed
    	ReadTempX();
    	ReadTempY();
        s_ulLastSampleTime = ulTime;
    }
	#endif

	#ifdef READMOTORVOLTAGE
    ulTime = MS_TIMER;
    ulSampleInterval = ulTime - s_ulLastMotorPowerSampleTime;
    if (ulSampleInterval > 200)
    {
    	//Check Motor Voltage
    	ReadMotorVoltage();
        s_ulLastMotorPowerSampleTime = ulTime;
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
		if (g_fAccelValue>fmax) { fmax=g_fAccelValue; }
		if (g_fAccelValue<fmin) { fmin=g_fAccelValue; }
		if (fa>famax) { famax=fa; }
		if (fa<famin) { famin=fa; }
		if (fb>fbmax) { fbmax=fb; }
		if (fb<fbmin) { fbmin=fb; }
		if (fc>fcmax) { fcmax=fc; }
		if (fc<fcmin) { fcmin=fc; }
	}
	#endif

	//FORCE Limits are read in another function

#ifdef OBSTRUCTION_SYSTEM_MO_ANA
	c = (char)(MS_TIMER >> 2); //check every 4 ms
	if (c!=s_cLastMoCheck)
	{
		s_cLastMoCheck = c;
        //Check MoCap
		cmo=0;
		#ifdef ADC_MO1
		ui=anaIn(ADC_MO1); if (ui<g_ConfigData.MOCal.uim1) { cmo|=1; }
		#endif
		#ifdef ADC_MO2
		ui=anaIn(ADC_MO2); if (ui<g_ConfigData.MOCal.uim2) { cmo|=2; }
		#endif
		#ifdef ADC_MO3
		ui=anaIn(ADC_MO3); if (ui<g_ConfigData.MOCal.uim3) { cmo|=4; }
		#endif
		#ifdef ADC_MO4
		ui=anaIn(ADC_MO4); if (ui<g_ConfigData.MOCal.uim4) { cmo|=8; }
		#endif
		#ifdef ADC_MO5
		ui=anaIn(ADC_MO5); if (ui<g_ConfigData.MOCal.uim5) { cmo|=16; }
		#endif
		#ifdef ADC_MO6
		ui=anaIn(ADC_MO6); if (ui<g_ConfigData.MOCal.uim6) { cmo|=32; }
		#endif
		g_cMOFlags = cmo;


	//Check Mo Print
	c = (char)(MS_TIMER >> 10); //print every second approx
	if (c!=s_cLastMoPrint)
	{
		s_cLastMoPrint = c;
        //Check MoCap
		logf("Checking MO %lu\r\n",MS_TIMER);
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


#ifdef READ_TEMP
nodebug float CalculateTemperatureFromVolts(int i)
{
	//FIXME Enhancement  I doubt this formula we were given : Retest the volts and temp...
	float fVolts,fA,fb,fC,f;
    fVolts = anaInVolts(i);
	fA = 112.9122;
    fb = -1.13066;
    fC = 34.36272;
    f = fA * exp(fb * fVolts) + fC;
    #ifdef USE_OUTPUT
	//logf("%.3f V to %.3f C\r\n",fVolts,f);
    #endif
    return f;
}

//****************************************************************
//  ReadTempX() - Read Temperature
//
//****************************************************************

nodebug void ReadTempX(void)
{
	g_fXTemp=CalculateTemperatureFromVolts(ADC_CHANNEL_TEMPX);
	if (g_fXTemp > 38)
	{
	    #ifdef USE_OUTPUT
	    logf("XTemp:%f\r\n",g_fXTemp);
    	#endif
	}
}
nodebug void ReadTempY(void)
{
	g_fYTemp=CalculateTemperatureFromVolts(ADC_CHANNEL_TEMPY);
	if (g_fYTemp > 38)
	{
	    #ifdef USE_OUTPUT
	    logf("YTemp:%f\r\n",g_fYTemp);
    	#endif
	}
}
#endif

#ifdef READMOTORVOLTAGE
nodebug void ReadMotorVoltage()
{
	int iSample;
	float fVolts;

	fVolts = anaInVolts(ADC_CHANNEL_MOTORVOLTAGE);
	//convert this voltage back to approximate motor volts
	g_fMotorVoltage = fVolts*11;
    #ifdef USE_OUTPUT
    if (g_fMotorVoltage>26.5)
    {
    	logf("g_fMotorVoltage:%f\r\n", g_fMotorVoltage);
    }
   	#endif
	#ifdef ANALOG_MOTORVOLTAGE_ESTOP_DETECT
	//Simulate the normal g_cDigInEstop signal
   	if (g_fMotorVoltage<16.0)
    {
		g_cDigInEstop = 0; //Set this to 0
    }
    else
    {
  		g_cDigInEstop = 1; //Set this to 0
    }
   	#endif
}
#endif

#ifdef FORCELIMITING

unsigned int g_uiReadForceSensorTime; //use int since exact granularity doesn't matter
char g_cShowForce;

nodebug void ReadForceInitMem()
{
	g_ForceSensor.cErrFlag=1; //don't use it before it has been read...
	g_uiReadForceSensorTime=0;
	g_cShowForce = 0;
}

//Read Force Sensor if it's time, and update force limits if executing a force limited move
nodebug void ReadForceSensor()
{
	unsigned int ui;
	ui=(unsigned int)MS_TIMER; //truncate
	if ((ui-g_uiReadForceSensorTime)>g_ConfigData.ForceLimits.uiSensorInterval)
	{
		//Time to Read Force Sensors
		ReadForceSensorNow();

		UpdateForceLimitingCurrents(); //only will do it if needed.
	}
}

nodebug void ReadForceSensorNow()
{
	unsigned int ui;
	int ix;
	int iy;
	int iz;
	float f;

	ui=(unsigned int)MS_TIMER; //truncate
	g_uiReadForceSensorTime=ui;

   	ix=anaIn(ADC_FORCE_SENSOR_X);
   	iy=anaIn(ADC_FORCE_SENSOR_Y);
	iz=anaIn(ADC_FORCE_SENSOR_Z);

	g_ForceSensor.fX = ((float)(ix-g_ConfigData.ForceSensorCalibration.iZeroX))/g_ConfigData.ForceSensorCalibration.iCountsPerGX;
	g_ForceSensor.fY = ((float)(iy-g_ConfigData.ForceSensorCalibration.iZeroY))/g_ConfigData.ForceSensorCalibration.iCountsPerGY;
	g_ForceSensor.fZ = ((float)(iz-g_ConfigData.ForceSensorCalibration.iZeroZ))/g_ConfigData.ForceSensorCalibration.iCountsPerGZ;
	g_ForceSensor.cErrFlag = 0;

	f=g_ForceSensor.fX*g_ForceSensor.fX+g_ForceSensor.fY*g_ForceSensor.fY+g_ForceSensor.fZ*g_ForceSensor.fZ;
	// not needed just compare to squared vales
	if (f<0.7 || f>1.3)
	{
		//likely invalid
		g_ForceSensor.cErrFlag=1;
	}
	if (g_cShowForce==1)
	{
		logf("fs %d %d %d %d\r\n",ix,iy,iz,g_ForceSensor.cErrFlag);
		logf("fs %f %f %f\r\n", g_ForceSensor.fX, g_ForceSensor.fY, g_ForceSensor.fZ );
	}
}

#endif
//STP MiniFt Definitions

////////////////////////////////////////////////////////////////////////////////
// STP: BuildMiniFtMessageCode
// builds an STP_ALERT using OID_MINIFT_MESSAGE_CODE
////////////////////////////////////////////////////////////////////////////////
//FIXME0 headers
nodebug void SmartToolMsgMiniFtMessageCode(unsigned int uiOID, unsigned int uiMessageCode)
{
	unsigned int ui;
	td_oid_minift_message_code * p_oid_minift_message_code;

    #ifdef OUTPUT_MiniFtMC
    logf("STP MiniFtMC:s=%s mc=%d\r\n", DisplayOIDName(uiOID), uiMessageCode);
    #endif
	#ifdef PACKET_FLOOD_MONITOR
	CheckPacketFlood(OID_MINIFT_MESSAGE_CODE);
    #endif

    g_STPtxMsg.uiVersion = htons(STP_VERSION);
    g_STPtxMsg.uiMsgType = htons(STP_ALERT);
    g_STPtxMsg.uiOID = htons(OID_MINIFT_MESSAGE_CODE);
    g_STPtxMsg.uiValueLength = htons(sizeof(td_oid_minift_message_code));

    p_oid_minift_message_code=(td_oid_minift_message_code *)&g_STPtxMsg.p_cObjectValue;
	p_oid_minift_message_code->uiOID=htons(uiOID);
	p_oid_minift_message_code->uiCode=htons(uiMessageCode);

    ui = (STP_HEADERSIZE  + sizeof(td_oid_minift_message_code));
    SendSTPmsg(&g_STPtxMsg, ui);
}




////////////////////////////////////////////////////////////////////////////////
// STP: HandleSmartToolMsg
////////////////////////////////////////////////////////////////////////////////
nodebug int HandleSmartToolMsg(td_STPmsg* p_STPrxMsg, td_STPsessions * p_STPSession)
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
    char* p_c_end;
    char* s;
    long* p_l;
    float* p_f;
    int iTemp;
    int iPos;
    int sr;
    unsigned int ui;
    long lTemp;
	int ilen;
    char c,ccode;
    int iNextPosn, iPrevPosn;
	char * p_szSmartTool; //because a few things can be turned on without main OUTPUT_RXSTP
	#ifdef OUTPUT_RXSTP
	char * p_szOIDName;
	#endif
    //char * p_cFIXME;   //this helps test compiling when using new makeoid code, but must commented out after.

	//MakeOID-generated:: GET AND SET DECLARATIONS (oid complete)

	td_StpStatus * p_StpStatus;
	td_oid_common_message_code * p_oid_common_message_code;
	td_oid_direct_dout * p_oid_direct_dout;
	td_oid_direct_din * p_oid_direct_din;
	td_DateTime * p_DateTime;
	td_oid_minift_message_code * p_oid_minift_message_code;
	td_EncoderRatio * p_EncoderRatio;
	td_MCCurrentLimit * p_MCCurrentLimit;
	td_MCPeakCurrentLimit * p_MCPeakCurrentLimit;
	td_oid_system_monitor * p_oid_system_monitor;
	td_HoleParam * p_HoleParam;
	td_GravCompSpeed * p_GravCompSpeed;
	td_GravCompAcc * p_GravCompAcc;
	td_GravCompDec * p_GravCompDec;
	td_GravCompMoveDist * p_GravCompMoveDist;
	td_GravCompNoiseLimit * p_GravCompNoiseLimit;
	td_GravCompTriggerFactor * p_GravCompTriggerFactor;
	td_GravCompResults * p_GravCompResults;
	td_ProbeOffset * p_ProbeOffset;
	td_oid_khole_distance * p_oid_khole_distance;
	td_oid_probe * p_oid_probe;
	td_oid_probe_position * p_oid_probe_position;
	td_oid_probe_status * p_oid_probe_status;
	td_oid_probe_start * p_oid_probe_start;
	td_HomeSpeed * p_HomeSpeed;
	td_HomeAcc * p_HomeAcc;
	td_HomeDec * p_HomeDec;
	td_HomeMoveDist * p_HomeMoveDist;
	td_HomeFineSpeed * p_HomeFineSpeed;
	td_oid_home_report * p_oid_home_report;
	td_oid_posnmode_curxy * p_oid_posnmode_curxy;
	td_oid_posnmode_nearxy * p_oid_posnmode_nearxy;
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
	td_oid_vision_image * p_oid_vision_image;
	td_VisionInspectResults * p_VisionInspectResults;
	td_LaserSensorAlgParam * p_LaserSensorAlgParam;
	td_CamAlgParam * p_CamAlgParam;
	td_PrimeDelay * p_PrimeDelay;
	td_FastenerFault * p_FastenerFault;
	td_DrillFault * p_DrillFault;
	td_HoleResultData * p_HoleResultData;
	td_oid_movexy * p_oid_movexy;
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

	td_SmartDrillFault * p_SmartDrillFault;
	td_FastenerXYOffset * p_FastenerXYOffset;
	td_FastenerSTATEANDSTATUS * p_Fastener;

#ifdef DRILL_DIRECT_READY
	td_SmartDrillHoleData * p_SmartDrillHoleData;
#endif
	td_RFID_F0 * p_rfid_f0;
	char c_op;
	char c_arg1;

    p_cSTPobjValBuff=g_STPtxMsg.p_cObjectValue; //for output value, reference the value location in STP tx

    uiOID=p_STPrxMsg->uiOID;

    if ( p_STPSession->iSessionNum >= 0) //MiniFT's Client Sent this message
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

    switch(p_STPrxMsg->uiMsgType)
    {
        case STP_GET:
            switch(uiOID)
            {
				//MakeOID-generated:: STP GET (oid merge)
                case OID_NULLOID:
					SmartToolMsgEmpty(STP_GET_RESP, uiOID);
                    break;
                case OID_DEBUG:
                    #ifdef USE_OUTPUT
                    //logf("STP::GET::OID_DEBUG\r\n");
                    #endif
					MCStatusOutput();
                    break;
				case OID_SMARTTOOL_TYPE:
					SmartToolMsgStr(STP_GET_RESP, uiOID, SMARTTOOL_TYPE_STRING);
					break;
				case OID_SMARTTOOL_SUBTYPE:
					SmartToolMsgStr(STP_GET_RESP, uiOID, SMARTTOOL_SUBTYPE_STRING);
					break;
                case OID_SYSTEM_VERSION:
                	//Let DynamicC append these.
                    SmartToolMsgStr(STP_GET_RESP, uiOID, SMARTTOOL_SUBTYPE_STRING " " SYSTEM_VERSION_STRING);
                    break;
				case OID_SERIAL_NUMBER:
					SmartToolMsgStr(STP_GET_RESP, uiOID, g_szSerialNumber);
					break;
				case OID_SCRIPT_TRANSFER:
                	//FIXME minor  Implemented on Pendant, but not on Rabbit
					break;
				case OID_GENERICMESSAGE:
                	//Fall Through
				case OID_DEBUGMESSAGE:
					SmartToolMsg(STP_GET_RESP, uiOID, 10, "-");
					break;
				case OID_STPSTATUS:
					p_StpStatus=(td_StpStatus *)p_cSTPobjValBuff;
					p_StpStatus->uiOID=htons(g_StpStatus.uiOID);
					p_StpStatus->uiStatus=htons(g_StpStatus.uiStatus);
					SmartToolMsg(STP_GET_RESP, uiOID, sizeof(td_StpStatus),p_cSTPobjValBuff);
					break;
				case OID_DIRECT_DOUT:
					p_oid_direct_dout=(td_oid_direct_dout *)p_cSTPobjValBuff;
					//FIXME00000000000000 FillStructWithData(p_oid_direct_dout);
					p_oid_direct_dout->cchannel=0;
					p_oid_direct_dout->cvalue=0;
					SmartToolMsg(STP_GET_RESP, uiOID, sizeof(td_oid_direct_dout),p_cSTPobjValBuff);
					break;
				case OID_DIRECT_DIN:
					p_oid_direct_din=(td_oid_direct_din *)p_cSTPobjValBuff;
					//FIXME FillStructWithData(p_oid_direct_din);
					p_oid_direct_din->cchannel=1;
					p_oid_direct_din->cvalue=digIn(p_oid_direct_din->cchannel);
					SmartToolMsg(STP_GET_RESP, uiOID, sizeof(td_oid_direct_din),p_cSTPobjValBuff);
					break;
				case OID_COM_VERSION:
					SmartToolMsgUInt(STP_GET_RESP, uiOID, COM_VERSION);
					break;
				//case OID_STREAM_SYNC: NO GET, but can alert
				//case OID_STP_CLOSE: NO GET, but can alert
				case OID_DATE_TIME:
					ServiceDateTime(); //update g_DateTime based on MS_TIMER
					p_DateTime=(td_DateTime *)p_cSTPobjValBuff;
					p_DateTime->ulticksMSW=htonul(g_DateTime.ulticksMSW);
					p_DateTime->ulticksLSW=htonul(g_DateTime.ulticksLSW);
					p_DateTime->uiyear=htons(g_DateTime.uiyear);
					p_DateTime->cmonth=g_DateTime.cmonth;
					p_DateTime->cdayOfMonth=g_DateTime.cdayOfMonth;
					p_DateTime->chour=g_DateTime.chour;
					p_DateTime->cminute=g_DateTime.cminute;
					p_DateTime->csecond=g_DateTime.csecond;
					p_DateTime->uimillisecond=htons(g_DateTime.uimillisecond);
					SmartToolMsg(STP_GET_RESP, uiOID, sizeof(td_DateTime), p_cSTPobjValBuff);
					break;
				//end common OIDs
				//MiniFT OIDs
				case OID_MODE:
					SmartToolMsgChar(STP_GET_RESP, uiOID, g_cModeState);
					break;
				case OID_ACTION:
					SmartToolMsgChar(STP_GET_RESP, uiOID, g_cAction);
					break;
				case OID_ENCODER_RATIO:
					p_EncoderRatio=(td_EncoderRatio *)p_cSTPobjValBuff;
					p_EncoderRatio->fX=g_ConfigData.EncoderRatio.fX;
					p_EncoderRatio->fY=g_ConfigData.EncoderRatio.fY;
					SmartToolMsg(STP_GET_RESP, uiOID, sizeof(td_EncoderRatio),p_cSTPobjValBuff);
					break;
				case OID_MC_CURRENT_LIMITS:
					p_MCCurrentLimit=(td_MCCurrentLimit *)p_cSTPobjValBuff;
					p_MCCurrentLimit->fX=g_ConfigData.MCCurrentLimit.fX;
					p_MCCurrentLimit->fY=g_ConfigData.MCCurrentLimit.fY;
					SmartToolMsg(STP_GET_RESP, uiOID, sizeof(td_MCCurrentLimit),p_cSTPobjValBuff);
					break;
				case OID_MC_PEAK_CURRENT_LIMITS:
					p_MCPeakCurrentLimit=(td_MCPeakCurrentLimit *)p_cSTPobjValBuff;
					p_MCPeakCurrentLimit->fX=g_ConfigData.MCPeakCurrentLimit.fX;
					p_MCPeakCurrentLimit->fY=g_ConfigData.MCPeakCurrentLimit.fY;
					SmartToolMsg(STP_GET_RESP, uiOID, sizeof(td_MCPeakCurrentLimit),p_cSTPobjValBuff);
					break;
				case OID_BRAKE_ON_TIMEOUT:
					SmartToolMsgUInt(STP_GET_RESP, uiOID, g_ConfigData.uiBrakeOnTimeout);
					break;
				case OID_MCERR:
					// OID_MCERR is used as an alert only, so the motion controller err msg is sent
					// when it occurs.  If a client explicitly tries to GET the mc error, just send "No Error"
                    SmartToolMsg(STP_GET_RESP, uiOID, 8, "NoErr");
					break;
				case OID_EEOPTION_DEFAULT:
					SmartToolMsgChar(STP_GET_RESP, uiOID, g_ConfigData.cEEOptionDefault);
					break;
				case OID_EEOPTION:
					SmartToolMsgChar(STP_GET_RESP, uiOID, g_cEEOption);
					break;
				case OID_BEEPER:
					SmartToolMsgChar(STP_GET_RESP, uiOID, g_ConfigData.cbeeper);
					break;
				case OID_SYSTEM_MONITOR:
					p_oid_system_monitor=(td_oid_system_monitor *)p_cSTPobjValBuff;
					#ifdef READ_TEMP
                	ReadTempX();
                	ReadTempY();
					p_oid_system_monitor->fxtemp=g_fXTemp;
					p_oid_system_monitor->fytemp=g_fYTemp;
					#else
					p_oid_system_monitor->fxtemp=0;
					p_oid_system_monitor->fytemp=0;
					#endif
					p_oid_system_monitor->uiloop_avg_ms=0;
					SmartToolMsg(STP_GET_RESP, uiOID, sizeof(td_oid_system_monitor),p_cSTPobjValBuff);
					break;
				case OID_AIR_PRESSURE:
					#ifdef PRESSURESENSOR
					SmartToolMsgChar(STP_GET_RESP, uiOID, g_cDigInAirPressure );
					#endif
					break;
				case OID_LIMITS_AND_OBSTRUCTIONS:
					lTemp=g_cObstructionCode;
					SmartToolMsgLong(STP_GET_RESP, uiOID, lTemp);
					break;
				case OID_TOOL_VERIFYENABLE:
					SmartToolMsgChar(STP_GET_RESP, uiOID, g_ConfigData.cToolVerifyEnable);
					break;
				case OID_HOLE_PARAMETERS:
                	SendHoleParameters(STP_GET_RESP);
					break;
				case OID_TOOL_RESERVED:
					//Deprecated see OID_TOOL_REC
					break;
				case OID_PROCESS:
					SmartToolMsgChar(STP_GET_RESP, uiOID, g_cLoadedProcess);
					break;
				case OID_RETURN_HEIGHT:
					SmartToolMsgFloat(STP_GET_RESP, uiOID, g_ConfigData.fReturnHeight);
					break;
				case OID_GAUGE_LENGTH:
					SmartToolMsgFloat(STP_GET_RESP, uiOID, g_fGaugeLength);
logf("gl %f\r\n", g_fGaugeLength);
					break;
				case OID_SCALE_MODE:
					SmartToolMsgChar(STP_GET_RESP, uiOID, g_ConfigData.cScaleMode);
					break;
				case OID_RESERVED_2:
					break;
				case OID_PARTPGM_REQUEST_FILE:
					SmartToolMsgStr(STP_GET_RESP, uiOID, "");
					break;
				case OID_PARTPGM_DIR:
	               	//BSTRARRAY FORMAT
                    //VDIR function is gone...
					break;
				case OID_PARTPGM_NAME:
					#ifdef OUTPUT_PP_SYS
   					logf("tx PPnm %s\r\n",g_szPartPgmFilename); //debug
					#endif
					SmartToolMsgStr(STP_GET_RESP, uiOID, g_szPartPgmFilename);
					break;
				case OID_PARTPGM_DATA:
                	//Sending only part of the data for the purpose pendant usage
                    //The Data can be started at an index to avoid retransmit
                    ui=p_STPrxMsg->uiValueLength;
                    if (ui==0)
                    {
                    	//start at the front
                    	ul=0;
                    }
                    else if (ui==4)
                    {
                    	ul=(unsigned long)ntohl( *((long*)(p_STPrxMsg->p_cObjectValue)) );
                    }
                    StartSendPartProgramData(p_STPSession->iSessionNum, ul); //Session and start
					break;
				case OID_PARTPGM_CHECKSUM:
                	//send all 16 bytes... no null
					SmartToolMsg(STP_GET_RESP, uiOID, 16, g_PartPgmInfo.p_cChecksum16);
					break;
				case OID_PARTPGM_LINEARJOB:
					SmartToolMsgInt(STP_GET_RESP, uiOID, g_iPartPgmLinearHoles);
					break;
				case OID_PARTPGM_STATUS:
					SmartToolMsgChar(STP_GET_RESP, uiOID, g_cPartPgmStatus);
					break;
				case OID_PARTPGM_LOCKED:
					SmartToolMsgChar(STP_GET_RESP, uiOID, g_PartPgmInfo.cLocked);
					break;
				case OID_GRAVCOMP_STATUS:
					SmartToolMsgChar(STP_GET_RESP, uiOID, g_cGravCompStatus);
					break;
				case OID_GRAVCOMP_AXES:
					SmartToolMsgChar(STP_GET_RESP, uiOID, g_ConfigData.cGravCompAxes);
					break;
				case OID_GRAVCOMP_SPEED:
					p_GravCompSpeed=(td_GravCompSpeed *)p_cSTPobjValBuff;
					p_GravCompSpeed->fX=g_ConfigData.GravCompSpeed.fX;
					p_GravCompSpeed->fY=g_ConfigData.GravCompSpeed.fY;
					SmartToolMsg(STP_GET_RESP, uiOID, sizeof(td_GravCompSpeed),p_cSTPobjValBuff);
					break;
				case OID_GRAVCOMP_ACCEL:
					p_GravCompAcc=(td_GravCompAcc *)p_cSTPobjValBuff;
					p_GravCompAcc->fX=g_ConfigData.GravCompAcc.fX;
					p_GravCompAcc->fY=g_ConfigData.GravCompAcc.fY;
					SmartToolMsg(STP_GET_RESP, uiOID, sizeof(td_GravCompAcc),p_cSTPobjValBuff);
					break;
				case OID_GRAVCOMP_DECEL:
					p_GravCompDec=(td_GravCompDec *)p_cSTPobjValBuff;
					p_GravCompDec->fX=g_ConfigData.GravCompDec.fX;
					p_GravCompDec->fY=g_ConfigData.GravCompDec.fY;
					SmartToolMsg(STP_GET_RESP, uiOID, sizeof(td_GravCompDec),p_cSTPobjValBuff);
					break;
				case OID_GRAVCOMP_MOVEDIST:
					p_GravCompMoveDist=(td_GravCompMoveDist *)p_cSTPobjValBuff;
					p_GravCompMoveDist->fX=g_ConfigData.GravCompMoveDist.fX;
					p_GravCompMoveDist->fY=g_ConfigData.GravCompMoveDist.fY;
					SmartToolMsg(STP_GET_RESP, uiOID, sizeof(td_GravCompMoveDist),p_cSTPobjValBuff);
					break;
				case OID_GRAVCOMP_ALGORITHM:
					SmartToolMsgChar(STP_GET_RESP, uiOID, g_ConfigData.cGravCompAlgorithm);
					break;
				case OID_GRAVCOMP_NOISE_LIMIT:
					p_GravCompNoiseLimit=(td_GravCompNoiseLimit *)p_cSTPobjValBuff;
					p_GravCompNoiseLimit->fX=g_ConfigData.GravCompNoiseLimit.fX;
					p_GravCompNoiseLimit->fY=g_ConfigData.GravCompNoiseLimit.fY;
					SmartToolMsg(STP_GET_RESP, uiOID, sizeof(td_GravCompNoiseLimit),p_cSTPobjValBuff);
					break;
				case OID_GRAVCOMP_TRIGGERFACTOR:
					p_GravCompTriggerFactor=(td_GravCompTriggerFactor *)p_cSTPobjValBuff;
					p_GravCompTriggerFactor->fX=g_ConfigData.GravCompTriggerFactor.fX;
					p_GravCompTriggerFactor->fY=g_ConfigData.GravCompTriggerFactor.fY;
					SmartToolMsg(STP_GET_RESP, uiOID, sizeof(td_GravCompTriggerFactor),p_cSTPobjValBuff);
					break;
				case OID_FLOAT_STATUS:
					SmartToolMsgChar(STP_GET_RESP, uiOID, g_cFloatStatus);
					break;
				case OID_GRAVCOMP_RESULTS:
					p_GravCompResults=(td_GravCompResults *)p_cSTPobjValBuff;
					p_GravCompResults->fxp=g_GravCompResults.fxp;
					p_GravCompResults->fxn=g_GravCompResults.fxn;
					p_GravCompResults->fyp=g_GravCompResults.fyp;
					p_GravCompResults->fyn=g_GravCompResults.fyn;
					SmartToolMsg(STP_GET_RESP, uiOID, sizeof(td_GravCompResults),p_cSTPobjValBuff);
					break;
				case OID_FLOAT_SPEEDLIMIT:
					SmartToolMsgFloat(STP_GET_RESP, uiOID, g_ConfigData.fFloatSpeedLimit);
					break;
				case OID_JOG_SPEEDLIMIT:
					SmartToolMsgFloat(STP_GET_RESP, uiOID, g_ConfigData.fJogSpeedLimit);
					break;
				case OID_MAX_SPEED_X:
					SmartToolMsgFloat(STP_GET_RESP, uiOID, g_ConfigData.fMaxSpeedX);
					break;
				case OID_MAX_SPEED_Y:
					SmartToolMsgFloat(STP_GET_RESP, uiOID, g_ConfigData.fMaxSpeedY);
					break;
				case OID_X_RAIL_SURFACE_OFFSET:
					SmartToolMsgFloat( STP_GET_RESP, uiOID, g_fXRailSurfaceOffset );
					break;
				case OID_PROBE_METHOD:
					SmartToolMsgChar(STP_GET_RESP, uiOID, g_cProbeMethod);
					break;
				case OID_PROBE_METHOD_DEFAULT:
					SmartToolMsgChar(STP_GET_RESP, uiOID, g_ConfigData.cProbeMethodDefault);
					break;
				case OID_PROBE_OFFSET:
					p_ProbeOffset=(td_ProbeOffset *)p_cSTPobjValBuff;
					p_ProbeOffset->fX=g_ConfigData.ProbeOffset.fX;
					p_ProbeOffset->fY=g_ConfigData.ProbeOffset.fY;
					SmartToolMsg(STP_GET_RESP, uiOID, sizeof(td_ProbeOffset),p_cSTPobjValBuff);
					break;
				case OID_PROBE_DIR:
					SmartToolMsgInt(STP_GET_RESP, uiOID, g_ConfigData.iProbeDir);
					break;
				case OID_DRILL_DIR:
					SmartToolMsgInt(STP_GET_RESP, uiOID, g_ConfigData.iDrillDir);
					break;
				case OID_KHOLE_MAX_DISTANCE_ERROR:
					SmartToolMsgFloat(STP_GET_RESP, uiOID, g_ConfigData.fMaxKholeDistanceError);
					break;
				case OID_APPROX_LOCATION_ERROR:
					SmartToolMsgFloat(STP_GET_RESP, uiOID, g_ConfigData.fApproxLocationError);
					break;
                //case OID_KHOLE_DISTANCE:
                //	Only Used for Alert
                //	break;
				case OID_PROBE_POSITION:
					p_oid_probe_position=(td_oid_probe_position *)p_cSTPobjValBuff;
					p_oid_probe_position->fX=g_PosnMode.fLastKnownPosnX;
					p_oid_probe_position->fY=g_PosnMode.fLastKnownPosnY;
					SmartToolMsg(STP_GET_RESP, uiOID, sizeof(td_oid_probe_position),p_cSTPobjValBuff);
					break;
				case OID_PROBE_STATUS:
                    //For now, this returns all the probe status in multiple responses
                    g_pSTPSession = (void *)0; //no limit
					SendAllProbeStatus(STP_GET_RESP);//SPSALL
					AlertProbeStatusUpdate();
                    g_pSTPSession = p_STPSession; //restore limit
					break;
				case OID_PROBE_START:
                    //For now, this returns all the probe start in multiple responses
                    g_pSTPSession = (void *)0; //no limit
					SendAllProbeStart(STP_GET_RESP);//SPSALL
					AlertProbeStatusUpdate();
                    g_pSTPSession = p_STPSession; //restore limit
					break;
				case OID_PROBE_ACCEPT_REQUIRED:
					SmartToolMsgChar(STP_GET_RESP, uiOID, g_ConfigData.cProbeAcceptRequired);
					break;
				case OID_HOME:
                	#ifdef HOMESYSTEM
					SmartToolMsgChar(STP_GET_RESP, uiOID, g_cHomed);
                   	ClearSentHomeStatus(); // Force Send of all the other Home Status...
                    					   //FIXME Not sure I like this pattern, but it's what I have now.
                    #else
                   	c=HOME_DONE;
					SmartToolMsgChar(STP_GET_RESP, uiOID, c);
                    #endif
					break;
				case OID_HOME_SPEED:
					p_HomeSpeed=(td_HomeSpeed *)p_cSTPobjValBuff;
					p_HomeSpeed->fX=g_ConfigData.HomeSpeed.fX;
					p_HomeSpeed->fY=g_ConfigData.HomeSpeed.fY;
					SmartToolMsg(STP_GET_RESP, uiOID, sizeof(td_HomeSpeed),p_cSTPobjValBuff);
					break;
				case OID_HOME_ACCEL:
					p_HomeAcc=(td_HomeAcc *)p_cSTPobjValBuff;
					p_HomeAcc->fX=g_ConfigData.HomeAcc.fX;
					p_HomeAcc->fY=g_ConfigData.HomeAcc.fY;
					SmartToolMsg(STP_GET_RESP, uiOID, sizeof(td_HomeAcc),p_cSTPobjValBuff);
					break;
				case OID_HOME_DECEL:
					p_HomeDec=(td_HomeDec *)p_cSTPobjValBuff;
					p_HomeDec->fX=g_ConfigData.HomeDec.fX;
					p_HomeDec->fY=g_ConfigData.HomeDec.fY;
					SmartToolMsg(STP_GET_RESP, uiOID, sizeof(td_HomeDec),p_cSTPobjValBuff);
					break;
				case OID_HOME_MOVEDIST:
					p_HomeMoveDist=(td_HomeMoveDist *)p_cSTPobjValBuff;
					p_HomeMoveDist->fX=g_ConfigData.HomeMoveDist.fX;
					p_HomeMoveDist->fY=g_ConfigData.HomeMoveDist.fY;
					SmartToolMsg(STP_GET_RESP, uiOID, sizeof(td_HomeMoveDist),p_cSTPobjValBuff);
					break;
				case OID_PROBE_ADJUST_LIMIT:
					SmartToolMsgFloat(STP_GET_RESP, uiOID, g_ConfigData.fProbeAdjustLimit);
					break;
				case OID_HOME_FINE_SPEED:
					p_HomeFineSpeed=(td_HomeFineSpeed *)p_cSTPobjValBuff;
					p_HomeFineSpeed->fX=g_ConfigData.HomeFineSpeed.fX;
					p_HomeFineSpeed->fY=g_ConfigData.HomeFineSpeed.fY;
					SmartToolMsg(STP_GET_RESP, uiOID, sizeof(td_HomeFineSpeed),p_cSTPobjValBuff);
					break;
				case OID_PATTERN:
					SmartToolMsgChar(STP_GET_RESP, uiOID, g_cPattern);
					break;
				case OID_POSNMODE_CURPOSN:
					SendCurPosnInd(STP_GET_RESP);
					break;
				case OID_POSNMODE_NEARPOSN:
					SendNearPosnInd(STP_GET_RESP);
					break;
				case OID_POSNMODE_GOALPOSN:
                	SendGoalPosnInd(STP_GET_RESP);
					break;
				case OID_POSNMODE_CURXY:
					GetPosition();
					g_PosnMode.fLastSentPosnX+=1; //force a send.
					break;
				case OID_POSNMODE_NEARXY:
					SendNearXY(STP_GET_RESP);
					break;
				case OID_POSNMODE_POSNSUMMARY:
                	//FIXME0000 not used yet
					break;
				case OID_POSNMODE_MOVETYPE:
					SmartToolMsgChar(STP_GET_RESP, uiOID, g_ConfigData.cMoveType);
					break;
				case OID_POSNMODE_PREMOVEXY:
					p_PreMove=(td_PreMove *)p_cSTPobjValBuff;
					p_PreMove->fX=g_ConfigData.PreMove.fX;
					p_PreMove->fY=g_ConfigData.PreMove.fY;
					SmartToolMsg(STP_GET_RESP, uiOID, sizeof(td_PreMove),p_cSTPobjValBuff);
					break;
				case OID_POSNMODE_SPEED:
					p_PosnSpeed=(td_PosnSpeed *)p_cSTPobjValBuff;
					p_PosnSpeed->fX=g_ConfigData.PosnSpeed.fX;
					p_PosnSpeed->fY=g_ConfigData.PosnSpeed.fY;
					SmartToolMsg(STP_GET_RESP, uiOID, sizeof(td_PosnSpeed),p_cSTPobjValBuff);
					break;
				case OID_POSNMODE_ACCEL:
					p_PosnAcc=(td_PosnAcc *)p_cSTPobjValBuff;
					p_PosnAcc->fX=g_ConfigData.PosnAcc.fX;
					p_PosnAcc->fY=g_ConfigData.PosnAcc.fY;
					SmartToolMsg(STP_GET_RESP, uiOID, sizeof(td_PosnAcc),p_cSTPobjValBuff);
					break;
				case OID_POSNMODE_DECEL:
					p_PosnDec=(td_PosnDec *)p_cSTPobjValBuff;
					p_PosnDec->fX=g_ConfigData.PosnDec.fX;
					p_PosnDec->fY=g_ConfigData.PosnDec.fY;
					SmartToolMsg(STP_GET_RESP, uiOID, sizeof(td_PosnDec),p_cSTPobjValBuff);
					break;
				case OID_POSNMODE_FINALSPEED:
					p_PosnFinalSpeed=(td_PosnFinalSpeed *)p_cSTPobjValBuff;
					p_PosnFinalSpeed->fX=g_ConfigData.PosnFinalSpeed.fX;
					p_PosnFinalSpeed->fY=g_ConfigData.PosnFinalSpeed.fY;
					SmartToolMsg(STP_GET_RESP, uiOID, sizeof(td_PosnFinalSpeed),p_cSTPobjValBuff);
					break;
				case OID_ORTHO_SLOPE:
					SmartToolMsgFloat(STP_GET_RESP, uiOID, g_ConfigData.fOrthoSlope);
					break;
				case OID_POSNERR_LIMIT:
					p_PosnErrLimit=(td_PosnErrLimit *)p_cSTPobjValBuff;
					p_PosnErrLimit->fX=g_ConfigData.PosnErrLimit.fX;
					p_PosnErrLimit->fY=g_ConfigData.PosnErrLimit.fY;
					SmartToolMsg(STP_GET_RESP, uiOID, sizeof(td_PosnErrLimit),p_cSTPobjValBuff);
					break;
				case OID_POSNMODE_TOLERANCE:
					SmartToolMsgFloat(STP_GET_RESP, uiOID, g_ConfigData.fPosnTolerance);
					break;
				case OID_VELERR_LIMIT:
					p_VelErrLimit=(td_VelErrLimit *)p_cSTPobjValBuff;
					p_VelErrLimit->fVLimitMarginX=g_ConfigData.VelErrLimit.fVLimitMarginX;
					p_VelErrLimit->fVErrorX=g_ConfigData.VelErrLimit.fVErrorX;
					p_VelErrLimit->fVLimitMarginY=g_ConfigData.VelErrLimit.fVLimitMarginY;
					p_VelErrLimit->fVErrorY=g_ConfigData.VelErrLimit.fVErrorY;
					SmartToolMsg(STP_GET_RESP, uiOID, sizeof(td_VelErrLimit),p_cSTPobjValBuff);
					break;
				case OID_LONG_DISTANCE:
					p_LongDistance=(td_LongDistance *)p_cSTPobjValBuff;
					p_LongDistance->fX=g_ConfigData.LongDistance.fX;
					p_LongDistance->fY=g_ConfigData.LongDistance.fY;
					SmartToolMsg(STP_GET_RESP, uiOID, sizeof(td_LongDistance),p_cSTPobjValBuff);
					break;
				case OID_LONG_SPEED:
					p_LongSpeed=(td_LongSpeed *)p_cSTPobjValBuff;
					p_LongSpeed->fX=g_ConfigData.LongSpeed.fX;
					p_LongSpeed->fY=g_ConfigData.LongSpeed.fY;
					SmartToolMsg(STP_GET_RESP, uiOID, sizeof(td_LongSpeed),p_cSTPobjValBuff);
					break;
				case OID_POSNMODE_MOVEDONE:
					SmartToolMsgChar(STP_GET_RESP, uiOID, g_cMoveDone);
					break;
				//case OID_OP_STARTED:
                //	//Used For ALERT only
                //	break;
				case OID_OP_HISTORY:
                    ui=p_STPrxMsg->uiValueLength;
                    if (ui==0)
                    {
                    	//start as early as possible
                    	ul=0;
                    }
                    else if (ui==4)
                    {
						//use this timestamp
                    	ul=(unsigned long)ntohl( *((long*)(p_STPrxMsg->p_cObjectValue)) );
                    }
					StartSendOpHistory(p_STPSession->iSessionNum, ul);
					break;
				case OID_DRILL_HOLE_ONE_TIME:
					SmartToolMsgChar(STP_GET_RESP, uiOID, g_ConfigData.cDrillHoleOneTime);
					break;
				case OID_AUTOMOVE:
					SmartToolMsgChar(STP_GET_RESP, uiOID, g_cAutoMove);
					break;
				case OID_AUTOMOVE_DELAY:
					SmartToolMsgULong(STP_GET_RESP, uiOID, g_ulAutoTime);
					break;
				case OID_AUTOREPEAT:
					//logf("g_cAR=%d\r\n",g_cAutoRepeat);
					SmartToolMsgChar(STP_GET_RESP, uiOID, g_cAutoRepeat);
					break;
				case OID_MACHINE_OFFSET:
					p_MachineOffset=(td_MachineOffset *)p_cSTPobjValBuff;
					p_MachineOffset->fX=g_MachineOffset.fX;
					p_MachineOffset->fY=g_MachineOffset.fY;
					SmartToolMsg(STP_GET_RESP, uiOID, sizeof(td_MachineOffset),p_cSTPobjValBuff);
					break;
				case OID_MACHINE_OFFSET_CADJ:
					SmartToolMsgChar(STP_GET_RESP, uiOID, g_cMachineOffsetCompensationAdjustment);
					break;
				case OID_MACHINE_OFFSET1:
					p_MachineOffset1=(td_MachineOffset1 *)p_cSTPobjValBuff;
					p_MachineOffset1->fX=g_MachineOffset1.fX;
					p_MachineOffset1->fY=g_MachineOffset1.fY;
					p_MachineOffset1->fYExtension=g_MachineOffset1.fYExtension;
					SmartToolMsg(STP_GET_RESP, uiOID, sizeof(td_MachineOffset1),p_cSTPobjValBuff);
					break;
				case OID_MACHINE_OFFSET2:
					p_MachineOffset2=(td_MachineOffset2 *)p_cSTPobjValBuff;
					p_MachineOffset2->fX=g_MachineOffset2.fX;
					p_MachineOffset2->fY=g_MachineOffset2.fY;
					p_MachineOffset2->fYExtension=g_MachineOffset2.fYExtension;
					SmartToolMsg(STP_GET_RESP, uiOID, sizeof(td_MachineOffset2),p_cSTPobjValBuff);
					break;
				case OID_STATION:
					SmartToolMsgChar(STP_GET_RESP, uiOID, g_cStation);
					break;
				case OID_TOOL_OFFSET:
					p_oid_tool_offset=(td_oid_tool_offset *)p_cSTPobjValBuff;
					//flipping controls an extra 180 degree rotation.
					//if tool is in pos direction, flip only for Tool to Movement conversion (machine offset if oposite tool offset.)
					//if tool is in neg direction, flip for Tool to Movement conversion, and flip again since tool is oriented on oposite side 180.
					//	two flips cancel out (-1 * -1 = 1) so do nothing.
					if (g_ConfigData.cToolFlip!=Y_NEG)
					{
						p_oid_tool_offset->fX = -g_MachineOffset.fX;
						p_oid_tool_offset->fY = -g_MachineOffset.fY;
					}
					else
					{
						p_oid_tool_offset->fX = g_MachineOffset.fX;
						p_oid_tool_offset->fY = g_MachineOffset.fY;
					}
					SmartToolMsg(STP_GET_RESP, uiOID, sizeof(td_oid_tool_offset),p_cSTPobjValBuff);
					break;
				case OID_TOOL_FLIP:
					SmartToolMsgChar(STP_GET_RESP, uiOID, g_ConfigData.cToolFlip);
					#ifdef OUTPUT_TOOL_FLIP
                	logf("Flp=%d\r\n",g_ConfigData.cToolFlip);
					#endif
					break;
				case OID_DRIVE_THROUGH_BACKLASH:
					SmartToolMsgChar(STP_GET_RESP, uiOID, g_ConfigData.cDriveThroughBacklash);
					break;
				case OID_DRILL_OFFSET1:
					p_DrillOffset1=(td_DrillOffset1 *)p_cSTPobjValBuff;
					p_DrillOffset1->fX=g_ConfigData.DrillOffset1.fX;
					p_DrillOffset1->fY=g_ConfigData.DrillOffset1.fY;
					p_DrillOffset1->fYExtension=g_ConfigData.DrillOffset1.fYExtension;
					SmartToolMsg(STP_GET_RESP, uiOID, sizeof(td_DrillOffset1),p_cSTPobjValBuff);
					break;
				case OID_DRILL_OFFSET2:
					p_DrillOffset2=(td_DrillOffset2 *)p_cSTPobjValBuff;
					p_DrillOffset2->fX=g_ConfigData.DrillOffset2.fX;
					p_DrillOffset2->fY=g_ConfigData.DrillOffset2.fY;
					p_DrillOffset2->fYExtension=g_ConfigData.DrillOffset2.fYExtension;
					SmartToolMsg(STP_GET_RESP, uiOID, sizeof(td_DrillOffset2),p_cSTPobjValBuff);
					break;
				case OID_OFFSET_SEAL:
					p_OffsetSeal=(td_OffsetSeal *)p_cSTPobjValBuff;
					p_OffsetSeal->fx=g_ConfigData.OffsetSeal.fx;
					p_OffsetSeal->fy=g_ConfigData.OffsetSeal.fy;
					SmartToolMsg(STP_GET_RESP, uiOID, sizeof(td_OffsetSeal),p_cSTPobjValBuff);
					break;
				case OID_OFFSET_FILL:
					p_OffsetFill=(td_OffsetFill *)p_cSTPobjValBuff;
					p_OffsetFill->fx=g_ConfigData.OffsetFill.fx;
					p_OffsetFill->fy=g_ConfigData.OffsetFill.fy;
					SmartToolMsg(STP_GET_RESP, uiOID, sizeof(td_OffsetFill),p_cSTPobjValBuff);
					break;
				case OID_JOG_SPEED:
					p_JogSpeed=(td_JogSpeed *)p_cSTPobjValBuff;
					p_JogSpeed->fX=g_ConfigData.JogSpeed.fX;
					p_JogSpeed->fY=g_ConfigData.JogSpeed.fY;
					SmartToolMsg(STP_GET_RESP, uiOID, sizeof(td_JogSpeed),p_cSTPobjValBuff);
					break;
				case OID_JOG_ACCEL:
					p_JogAcc=(td_JogAcc *)p_cSTPobjValBuff;
					p_JogAcc->fX=g_ConfigData.JogAcc.fX;
					p_JogAcc->fY=g_ConfigData.JogAcc.fY;
					SmartToolMsg(STP_GET_RESP, uiOID, sizeof(td_JogAcc),p_cSTPobjValBuff);
					break;
				case OID_JOG_DECEL:
					p_JogDec=(td_JogDec *)p_cSTPobjValBuff;
					p_JogDec->fX=g_ConfigData.JogDec.fX;
					p_JogDec->fY=g_ConfigData.JogDec.fY;
					SmartToolMsg(STP_GET_RESP, uiOID, sizeof(td_JogDec),p_cSTPobjValBuff);
					break;
				case OID_JOG_FACTOR:
					p_JogFactor=(td_JogFactor *)p_cSTPobjValBuff;
					p_JogFactor->fX=g_JogFactor.fX;
					p_JogFactor->fY=g_JogFactor.fY;
					SmartToolMsg(STP_GET_RESP, uiOID, sizeof(td_JogFactor),p_cSTPobjValBuff);
					break;
				case OID_HOME_POSITION_Y_POS:
					p_HomePosnYPos=(td_HomePosnYPos *)p_cSTPobjValBuff;
					p_HomePosnYPos->fX=g_ConfigData.HomePosnYPos.fX;
					p_HomePosnYPos->fY=g_ConfigData.HomePosnYPos.fY;
					SmartToolMsg(STP_GET_RESP, uiOID, sizeof(td_HomePosnYPos),p_cSTPobjValBuff);
					break;
				case OID_POSITION_LIMIT_Y_POS:
					p_PosnLimitYPos=(td_PosnLimitYPos *)p_cSTPobjValBuff;
					p_PosnLimitYPos->fMinX=g_ConfigData.PosnLimitYPos.fMinX;
					p_PosnLimitYPos->fMaxX=g_ConfigData.PosnLimitYPos.fMaxX;
					p_PosnLimitYPos->fMinY=g_ConfigData.PosnLimitYPos.fMinY;
					p_PosnLimitYPos->fMaxY=g_ConfigData.PosnLimitYPos.fMaxY;
					SmartToolMsg(STP_GET_RESP, uiOID, sizeof(td_PosnLimitYPos),p_cSTPobjValBuff);
					break;
				case OID_HOME_POSITION_Y_NEG:
					p_HomePosnYNeg=(td_HomePosnYNeg *)p_cSTPobjValBuff;
					p_HomePosnYNeg->fX=g_ConfigData.HomePosnYNeg.fX;
					p_HomePosnYNeg->fY=g_ConfigData.HomePosnYNeg.fY;
					SmartToolMsg(STP_GET_RESP, uiOID, sizeof(td_HomePosnYNeg), p_cSTPobjValBuff);
					break;
				case OID_POSITION_LIMIT_Y_NEG:
					p_PosnLimitYNeg=(td_PosnLimitYNeg *)p_cSTPobjValBuff;
					p_PosnLimitYNeg->fMinX=g_ConfigData.PosnLimitYNeg.fMinX;
					p_PosnLimitYNeg->fMaxX=g_ConfigData.PosnLimitYNeg.fMaxX;
					p_PosnLimitYNeg->fMinY=g_ConfigData.PosnLimitYNeg.fMinY;
					p_PosnLimitYNeg->fMaxY=g_ConfigData.PosnLimitYNeg.fMaxY;
					SmartToolMsg(STP_GET_RESP, uiOID, sizeof(td_PosnLimitYNeg), p_cSTPobjValBuff);
					break;
				case OID_PROBE_REGISTRATION:
					SmartToolMsgChar(STP_GET_RESP, uiOID, g_Probe.cRegistration);
					break;
				case OID_OBSTRUCTION_CODE_MASK:
					SmartToolMsgChar(STP_GET_RESP, uiOID, g_ConfigData.cObstructionCodeMask);
					break;
				case OID_MACHINE_LOCK_REQUIRED:
					SmartToolMsgChar( STP_GET_RESP, uiOID, g_ConfigData.cMachineLockRequired );
					break;
				case OID_MACHINE_LOCK:
					//Use Summed up lock summary
					SmartToolMsgChar( STP_GET_RESP, uiOID, g_cMachineLock );
					break;
				case OID_CLAMP:
					#ifdef CLAMP_SYSTEM
					SmartToolMsgChar(STP_GET_RESP, uiOID, g_cClampState); //Return the Clamp State
					#endif
					break;
				case OID_ALOCK:
					SmartToolMsgChar(STP_GET_RESP, uiOID, g_cALockMode);
					break;
				case OID_ALOCKDELAY:
					SmartToolMsgUInt(STP_GET_RESP, uiOID, g_ConfigData.uiALockDelay);
					break;
				case OID_AUNLOCKDELAY:
					SmartToolMsgUInt(STP_GET_RESP, uiOID, g_ConfigData.uiAUnlockDelay);
					break;
				case OID_LEGLOCKDELAY:
					SmartToolMsgUInt(STP_GET_RESP, uiOID, g_ConfigData.uiLegsLockDelay);
					break;
				case OID_LEGUNLOCKDELAY:
					SmartToolMsgUInt(STP_GET_RESP, uiOID, g_ConfigData.uiLegsUnlockDelay);
					break;
				case OID_LEGSUPDELAY:
					SmartToolMsgUInt(STP_GET_RESP, uiOID, g_ConfigData.uiLegsUpDelay);
					break;
				case OID_LEGSDOWNDELAY:
					SmartToolMsgUInt(STP_GET_RESP, uiOID, g_ConfigData.uiLegsDownDelay);
					break;
				case OID_LOWPRESSUREDELAY:
					SmartToolMsgUInt(STP_GET_RESP, uiOID, g_ConfigData.uiLowPressureDelay);
					break;
				case OID_LOWPRESSUREDOWNDELAY:
					SmartToolMsgUInt(STP_GET_RESP, uiOID, g_ConfigData.uiLowPressureDownDelay);
					break;
				case OID_PRESSUREDELAY:
					SmartToolMsgUInt(STP_GET_RESP, uiOID, g_ConfigData.uiPressureDelay);
					break;
				case OID_PRESSUREDOWNDELAY:
					SmartToolMsgUInt(STP_GET_RESP, uiOID, g_ConfigData.uiPressureDownDelay);
					break;
				case OID_LOWPRESSURE:
					SmartToolMsgUInt(STP_GET_RESP, uiOID, g_ConfigData.uiLowPressure);
					break;
				case OID_PRESSURE:
					SmartToolMsgUInt(STP_GET_RESP, uiOID, g_ConfigData.uiPressure);
					break;
				case OID_AIR_CLEAR:
					SmartToolMsgUInt(STP_GET_RESP, uiOID, g_ConfigData.uiAirClear);
					break;
				case OID_LASER_SENSOR_OFFSET:
					p_LaserSensorOffset=(td_LaserSensorOffset *)p_cSTPobjValBuff;
					p_LaserSensorOffset->fX=g_ConfigData.LaserSensorOffset.fX;
					p_LaserSensorOffset->fY=g_ConfigData.LaserSensorOffset.fY;
					SmartToolMsg(STP_GET_RESP, uiOID, sizeof(td_LaserSensorOffset),p_cSTPobjValBuff);
					break;
				case OID_CAM_OFFSET:
					p_CamOffset=(td_CamOffset *)p_cSTPobjValBuff;
					p_CamOffset->fX=g_ConfigData.CamOffset.fX;
					p_CamOffset->fY=g_ConfigData.CamOffset.fY;
					SmartToolMsg(STP_GET_RESP, uiOID, sizeof(td_CamOffset),p_cSTPobjValBuff);
					break;
				//case OID_VISION_IMAGE:
                	//For Alerting Images
					break;
                //Used For ALERTS
				//case OID_VISION_DATA:
				//case OID_VISION_INSPECT_RESULTS:

				case OID_LASER_SENSOR_ALG_PARAM:
					p_LaserSensorAlgParam=(td_LaserSensorAlgParam *)p_cSTPobjValBuff;
					p_LaserSensorAlgParam->fsearch_speed=g_ConfigData.LaserSensorAlgParam.fsearch_speed;
					p_LaserSensorAlgParam->fseek_speed=g_ConfigData.LaserSensorAlgParam.fseek_speed;
					p_LaserSensorAlgParam->frscan_speed=g_ConfigData.LaserSensorAlgParam.frscan_speed;
					p_LaserSensorAlgParam->frscan_speed_fast=g_ConfigData.LaserSensorAlgParam.frscan_speed_fast;
					p_LaserSensorAlgParam->fscan_speed=g_ConfigData.LaserSensorAlgParam.fscan_speed;
					p_LaserSensorAlgParam->fscan_speed_fast=g_ConfigData.LaserSensorAlgParam.fscan_speed_fast;
					p_LaserSensorAlgParam->fprobe_diameter=g_ConfigData.LaserSensorAlgParam.fprobe_diameter;
					p_LaserSensorAlgParam->funknown_diameter=g_ConfigData.LaserSensorAlgParam.funknown_diameter;
					p_LaserSensorAlgParam->cmode=g_ConfigData.LaserSensorAlgParam.cmode;
					p_LaserSensorAlgParam->cmode_fast=g_ConfigData.LaserSensorAlgParam.cmode_fast;
					p_LaserSensorAlgParam->cuse_avg=g_ConfigData.LaserSensorAlgParam.cuse_avg;
					p_LaserSensorAlgParam->cfull_scan=g_ConfigData.LaserSensorAlgParam.cfull_scan;
					p_LaserSensorAlgParam->cgdata_sel=g_ConfigData.LaserSensorAlgParam.cgdata_sel;
					p_LaserSensorAlgParam->cassume_posn=g_ConfigData.LaserSensorAlgParam.cassume_posn;
					p_LaserSensorAlgParam->cassume_posn_fast=g_ConfigData.LaserSensorAlgParam.cassume_posn_fast;
					p_LaserSensorAlgParam->crect_center=g_ConfigData.LaserSensorAlgParam.crect_center;
					p_LaserSensorAlgParam->cloops=g_ConfigData.LaserSensorAlgParam.cloops;
					p_LaserSensorAlgParam->cdelta_mode=g_ConfigData.LaserSensorAlgParam.cdelta_mode;
					p_LaserSensorAlgParam->idelta_flat=htons(g_ConfigData.LaserSensorAlgParam.idelta_flat);
					p_LaserSensorAlgParam->fdelta_basespan=g_ConfigData.LaserSensorAlgParam.fdelta_basespan;
					p_LaserSensorAlgParam->idelta_pos=htons(g_ConfigData.LaserSensorAlgParam.idelta_pos);
					p_LaserSensorAlgParam->idelta_neg=htons(g_ConfigData.LaserSensorAlgParam.idelta_neg);
					p_LaserSensorAlgParam->fdelta_span=g_ConfigData.LaserSensorAlgParam.fdelta_span;
					p_LaserSensorAlgParam->fdelta_edge=g_ConfigData.LaserSensorAlgParam.fdelta_edge;
					p_LaserSensorAlgParam->fpc_aspect_diff=g_ConfigData.LaserSensorAlgParam.fpc_aspect_diff;
					p_LaserSensorAlgParam->fmax_aspect_diff=g_ConfigData.LaserSensorAlgParam.fmax_aspect_diff;
					p_LaserSensorAlgParam->fmax_over_exp_diameter=g_ConfigData.LaserSensorAlgParam.fmax_over_exp_diameter;
					p_LaserSensorAlgParam->fmax_under_exp_diameter=g_ConfigData.LaserSensorAlgParam.fmax_under_exp_diameter;
					p_LaserSensorAlgParam->fmax_csnk_diff=g_ConfigData.LaserSensorAlgParam.fmax_csnk_diff;
					p_LaserSensorAlgParam->fmax_over_csnk=g_ConfigData.LaserSensorAlgParam.fmax_over_csnk;
					p_LaserSensorAlgParam->fmax_under_csnk=g_ConfigData.LaserSensorAlgParam.fmax_under_csnk;
					SmartToolMsg(STP_GET_RESP, uiOID, sizeof(td_LaserSensorAlgParam),p_cSTPobjValBuff);
					break;
				case OID_CAM_ALG_PARAM:
					p_CamAlgParam=(td_CamAlgParam *)p_cSTPobjValBuff;
					p_CamAlgParam->fmove_speed=g_ConfigData.CamAlgParam.fmove_speed;
					p_CamAlgParam->cInfoMask=g_ConfigData.CamAlgParam.cInfoMask;
					p_CamAlgParam->cAMode=g_ConfigData.CamAlgParam.cAMode;
					p_CamAlgParam->cCMode=g_ConfigData.CamAlgParam.cCMode;
					p_CamAlgParam->cAux1=g_ConfigData.CamAlgParam.cAux1;
					p_CamAlgParam->fmove_required=g_ConfigData.CamAlgParam.fmove_required;
					p_CamAlgParam->fmax_over_exp_diameter=g_ConfigData.CamAlgParam.fmax_over_exp_diameter;
					p_CamAlgParam->fmax_under_exp_diameter=g_ConfigData.CamAlgParam.fmax_under_exp_diameter;
					p_CamAlgParam->fmax_csnk_diff=g_ConfigData.CamAlgParam.fmax_csnk_diff;
					p_CamAlgParam->fmax_over_csnk=g_ConfigData.CamAlgParam.fmax_over_csnk;
					p_CamAlgParam->fmax_under_csnk=g_ConfigData.CamAlgParam.fmax_under_csnk;
					SmartToolMsg(STP_GET_RESP, uiOID, sizeof(td_CamAlgParam),p_cSTPobjValBuff);
					break;
				case OID_VISION_AUTO_INSPECT:
					SmartToolMsgChar(STP_GET_RESP, uiOID, g_ConfigData.cVisionAutoInspect);
					break;
				case OID_PROCESS_CONTINUE_MODE:
					SmartToolMsgChar(STP_GET_RESP, uiOID, g_ConfigData.cProcessContinueMode);
					break;
				case OID_PROCESS_OPERATIONS:
                    ui=g_ConfigData.uiProcessOperations;
					#ifdef OUTPUT_OPERATIONS
                    logf("spop=%u\r\n",ui);
					#endif
					SmartToolMsgUInt(STP_GET_RESP, uiOID, ui);
					break;
				case OID_DRILL_STATE:
					SmartToolMsgChar(STP_GET_RESP, uiOID, g_cDrillState);
					break;
				case OID_DRILL_EXPLANATION:
					SmartToolMsgChar(STP_GET_RESP, uiOID, g_cDrillExplanation);
					break;
				case OID_SEAL_STATE:
					SmartToolMsgChar(STP_GET_RESP, uiOID, g_cSealState);
					break;
				case OID_SEAL_CLAMP:
					SmartToolMsgChar(STP_GET_RESP, uiOID, g_ConfigData.cSealClamp);
					break;
				case OID_SEAL_PRESSURE_DELAY:
					SmartToolMsgInt(STP_GET_RESP, uiOID, g_ConfigData.iSealPressureDelay);
					break;
				case OID_SEAL_PRESSURE_RELEASE_DELAY:
					SmartToolMsgInt(STP_GET_RESP, uiOID, g_ConfigData.iSealPressureReleaseDelay);
					break;
				case OID_SEAL_PRIME_DELAY:
					p_PrimeDelay=(td_PrimeDelay *)p_cSTPobjValBuff;
					p_PrimeDelay->fdiameter1=g_ConfigData.PrimeDelay.fdiameter1;
					p_PrimeDelay->idelay1=htons(g_ConfigData.PrimeDelay.idelay1);
					p_PrimeDelay->fdiameter2=g_ConfigData.PrimeDelay.fdiameter2;
					p_PrimeDelay->idelay2=htons(g_ConfigData.PrimeDelay.idelay2);
					p_PrimeDelay->fdiameter3=g_ConfigData.PrimeDelay.fdiameter3;
					p_PrimeDelay->idelay3=htons(g_ConfigData.PrimeDelay.idelay3);
					p_PrimeDelay->fdiameter4=g_ConfigData.PrimeDelay.fdiameter4;
					p_PrimeDelay->idelay4=htons(g_ConfigData.PrimeDelay.idelay4);
					p_PrimeDelay->fdiameter5=g_ConfigData.PrimeDelay.fdiameter5;
					p_PrimeDelay->idelay5=htons(g_ConfigData.PrimeDelay.idelay5);
					SmartToolMsg(STP_GET_RESP, uiOID, sizeof(td_PrimeDelay),p_cSTPobjValBuff);
					break;
				case OID_SEAL_GLOB_DELAY:
					SmartToolMsgInt(STP_GET_RESP, uiOID, g_ConfigData.iSealGlobDelay);
					break;
				case OID_SEAL_APPLY_DELAY:
					SmartToolMsgInt(STP_GET_RESP, uiOID, g_ConfigData.iSealApplyDelay);
					break;
				case OID_FILL_STATE:
					SmartToolMsgChar(STP_GET_RESP, uiOID, g_cFillState);
					break;
				case OID_FILL_CLAMP:
					SmartToolMsgChar(STP_GET_RESP, uiOID, g_ConfigData.cFillClamp_);
					break;
				case OID_FILL_EXTEND_DELAY:
					SmartToolMsgInt(STP_GET_RESP, uiOID, g_ConfigData.iFillExtendDelay_);
					break;
				case OID_FILL_RAM_DELAY:
					SmartToolMsgInt(STP_GET_RESP, uiOID, g_ConfigData.iFillRamDelay_);
					break;
				case OID_FASTENER_REQUEST:
                	//FIXME000000000000
					//SmartToolMsgStr(STP_GET_RESP, uiOID, xvarname);
					break;
				case OID_FASTENER_ARRIVED:
					SmartToolMsgChar(STP_GET_RESP, uiOID, g_cFastenerArrived);
					break;
				case OID_FASTENER_POST_DELAY:
					SmartToolMsgInt(STP_GET_RESP, uiOID, g_ConfigData.iFastenerPostDelay_);
					break;
				case OID_ACCEL_ARM:
					SmartToolMsgChar(STP_GET_RESP, uiOID, g_cAccelArm_);
					break;
				case OID_ACCEL_TRIGGER:
					SmartToolMsgChar(STP_GET_RESP, uiOID, g_cAccelTrigger_);
					break;
				case OID_TOOL_Z_BASE:
					SmartToolMsgFloat( STP_GET_RESP, uiOID, g_ConfigData.fToolZBase );
					break;
				case OID_FASTENER_FAULT:
					p_FastenerFault = (td_FastenerFault *) p_cSTPobjValBuff;
					p_FastenerFault->cDevice = g_FastenerFault.cDevice;
					p_FastenerFault->cSeverity = g_FastenerFault.cSeverity;
					p_FastenerFault->lFaultCode = htonl( g_FastenerFault.lFaultCode );
					SmartToolMsg( STP_GET_RESP, uiOID, sizeof(td_FastenerFault), p_cSTPobjValBuff );
					break;
				case OID_POSNMODE_ACTIVE_PREMOVEXY:
                	SendActivePremove(STP_GET_RESP);
					break;
				case OID_DRILL_FAULT:
					p_DrillFault=(td_DrillFault *)p_cSTPobjValBuff;
					p_DrillFault->cDevice=g_DrillFault.cDevice;
					p_DrillFault->cSeverity=g_DrillFault.cSeverity;
					p_DrillFault->lFaultCode=htonl(g_DrillFault.lFaultCode);
					SmartToolMsg(STP_GET_RESP, uiOID, sizeof(td_DrillFault),p_cSTPobjValBuff);
					break;
				case OID_HOLE_RESULT_DATA:
					p_HoleResultData=(td_HoleResultData *)p_cSTPobjValBuff;
					p_HoleResultData->iHoleNumber=htons(g_HoleResultData.iHoleNumber);
					p_HoleResultData->iHoleResult=htons(g_HoleResultData.iHoleResult);
					SmartToolMsg(STP_GET_RESP, uiOID, sizeof(td_HoleResultData),p_cSTPobjValBuff);
					break;
				case OID_MOVEXY:
                //FIXME Minor  No Get for this currently
				//	p_oid_movexy=(td_oid_movexy *)p_cSTPobjValBuff;
				//	//FIXME FillStructWithData(p_oid_movexy);
				//	p_oid_movexy->fMachineX=p_oid_movexy->fMachineX;
				//	p_oid_movexy->fMachineY=p_oid_movexy->fMachineY;
				//	SmartToolMsg(STP_GET_RESP, uiOID, sizeof(td_oid_movexy),p_cSTPobjValBuff);
					break;
				case OID_POSN_DISPLAY:
					p_PosnDisplay=(td_PosnDisplay *)p_cSTPobjValBuff;
					p_PosnDisplay->cmode=g_ConfigData.PosnDisplay.cmode;
					p_PosnDisplay->corigin=g_ConfigData.PosnDisplay.corigin;
					p_PosnDisplay->ccontent=g_ConfigData.PosnDisplay.ccontent;
					SmartToolMsg(STP_GET_RESP, uiOID, sizeof(td_PosnDisplay),p_cSTPobjValBuff);
					break;
				case OID_POSNMODE_XYDATA_ID:
					//Machine Data Set Code
					//Send MachineDataSetID now
					SendXYDataID(STP_GET_RESP);
					break;
				case OID_POSNMODE_XYDATA:
					//Machine Data Set Code
                    //The Data can be started at an index to avoid retransmit
                    ui=p_STPrxMsg->uiValueLength;
                    if (ui==2)
                    {
						//Start at a specific index
                    	ui=(unsigned int)ntohs( *((int*)(p_STPrxMsg->p_cObjectValue)) );
                    }
					else
					{
                    	//start at the front
                    	ui=0;
					}
					StartSendXYData(p_STPSession->iSessionNum, ui);
					break;
				case OID_RFID_CONFIG:
					p_RFIDConfig=(td_RFIDConfig *)p_cSTPobjValBuff;
					p_RFIDConfig->cenabled=g_ConfigData.RFIDConfig.cenabled;
					p_RFIDConfig->cmethod=g_ConfigData.RFIDConfig.cmethod;
					p_RFIDConfig->uioptions=htons(g_ConfigData.RFIDConfig.uioptions);
					p_RFIDConfig->uicontinuousReadCycleTime=htons(g_ConfigData.RFIDConfig.uicontinuousReadCycleTime);
					p_RFIDConfig->uiseekReadCycleTime=htons(g_ConfigData.RFIDConfig.uiseekReadCycleTime);
					p_RFIDConfig->fseekMove1=g_ConfigData.RFIDConfig.fseekMove1;
					p_RFIDConfig->fseekMove2=g_ConfigData.RFIDConfig.fseekMove2;
					p_RFIDConfig->fseekFineMove=g_ConfigData.RFIDConfig.fseekFineMove;
					p_RFIDConfig->fseekSpeed=g_ConfigData.RFIDConfig.fseekSpeed;
					p_RFIDConfig->fseekFineSpeed=g_ConfigData.RFIDConfig.fseekFineSpeed;
					p_RFIDConfig->fRFIDOffset=g_ConfigData.RFIDConfig.fRFIDOffset;
					p_RFIDConfig->fseekPastBorder=g_ConfigData.RFIDConfig.fseekPastBorder;
					p_RFIDConfig->fminWindowSize = g_ConfigData.RFIDConfig.fminWindowSize;
					p_RFIDConfig->fmaxWindowSize = g_ConfigData.RFIDConfig.fmaxWindowSize;
					SmartToolMsg(STP_GET_RESP, uiOID, sizeof(td_RFIDConfig), p_cSTPobjValBuff);
					break;
				case OID_RFID_DATA:
					SendRFIDData(STP_GET_RESP);
					break;
				case OID_READ_RFID:
					SmartToolMsgChar(STP_GET_RESP, uiOID, g_cReadRFID);
					break;
				case OID_ESTOP_CLEAR_DELAY:
					SmartToolMsgUInt(STP_GET_RESP, uiOID, g_ConfigData.uiEStopClearDelay);
					break;
				case OID_DRILL_BUTTON_DELAY:
					SmartToolMsgUInt(STP_GET_RESP, uiOID, g_ConfigData.uiDrillButtonDelay);
					break;
				case OID_USE_CUTTER_DETECT:
					SmartToolMsgChar(STP_GET_RESP, uiOID, g_ConfigData.cUseCutterDetect);
					break;
				case OID_JOG_ENABLE_TIMEOUT:
					SmartToolMsgChar(STP_GET_RESP, uiOID, g_ConfigData.cJogEnableTimeout);
					break;
				case OID_DRILL_CYCLE_DELAY:
					SmartToolMsgUInt(STP_GET_RESP, uiOID, g_ConfigData.uiDrillCycleDelay);
					break;
				case OID_INSPECT_METHOD:
					SmartToolMsgChar(STP_GET_RESP, uiOID, g_ConfigData.cInspectMethod);
					break;
				case OID_COMMAND_INSPECT_METHOD:
					SmartToolMsgChar(STP_GET_RESP, uiOID, g_ConfigData.cCommandInspectMethod);
					break;
				case OID_FORCE_SENSOR_CALIBRATION:
					p_ForceSensorCalibration=(td_ForceSensorCalibration *)p_cSTPobjValBuff;
					p_ForceSensorCalibration->iZeroX=htons(g_ConfigData.ForceSensorCalibration.iZeroX);
					p_ForceSensorCalibration->iZeroY=htons(g_ConfigData.ForceSensorCalibration.iZeroY);
					p_ForceSensorCalibration->iZeroZ=htons(g_ConfigData.ForceSensorCalibration.iZeroZ);
					p_ForceSensorCalibration->iCountsPerGX=htons(g_ConfigData.ForceSensorCalibration.iCountsPerGX);
					p_ForceSensorCalibration->iCountsPerGY=htons(g_ConfigData.ForceSensorCalibration.iCountsPerGY);
					p_ForceSensorCalibration->iCountsPerGZ=htons(g_ConfigData.ForceSensorCalibration.iCountsPerGZ);
					SmartToolMsg(STP_GET_RESP, uiOID, sizeof(td_ForceSensorCalibration), p_cSTPobjValBuff);
					break;
				case OID_FORCE_SENSOR:
					p_ForceSensor=(td_ForceSensor *)p_cSTPobjValBuff;
					p_ForceSensor->fX=g_ForceSensor.fX;
					p_ForceSensor->fY=g_ForceSensor.fY;
					p_ForceSensor->fZ=g_ForceSensor.fZ;
					p_ForceSensor->cErrFlag=g_ForceSensor.cErrFlag;
					SmartToolMsg(STP_GET_RESP, uiOID, sizeof(td_ForceSensor), p_cSTPobjValBuff);
					break;
				case OID_FORCE_LIMITS:
					p_ForceLimits=(td_ForceLimits *)p_cSTPobjValBuff;
					p_ForceLimits->uiSensorInterval=htons(g_ConfigData.ForceLimits.uiSensorInterval);
					p_ForceLimits->uiMinUpdateDelta=htons(g_ConfigData.ForceLimits.uiMinUpdateDelta);
					p_ForceLimits->cActive=g_ConfigData.ForceLimits.cActive;
					p_ForceLimits->cCurrentUnderMethod=g_ConfigData.ForceLimits.cCurrentUnderMethod;
					p_ForceLimits->uiCurrentOverX=htons(g_ConfigData.ForceLimits.uiCurrentOverX);
					p_ForceLimits->uiCurrentUnderX=htons(g_ConfigData.ForceLimits.uiCurrentUnderX);
					p_ForceLimits->uiCurrentOverY=htons(g_ConfigData.ForceLimits.uiCurrentOverY);
					p_ForceLimits->uiCurrentUnderY=htons(g_ConfigData.ForceLimits.uiCurrentUnderY);
					p_ForceLimits->uiFullGravX=htons(g_ConfigData.ForceLimits.uiFullGravX);
					p_ForceLimits->uiFullGravY=htons(g_ConfigData.ForceLimits.uiFullGravY);
					p_ForceLimits->uiFlatForceX=htons(g_ConfigData.ForceLimits.uiFlatForceX);
					p_ForceLimits->uiFlatForceY=htons(g_ConfigData.ForceLimits.uiFlatForceY);
					SmartToolMsg(STP_GET_RESP, uiOID, sizeof(td_ForceLimits), p_cSTPobjValBuff);
					break;
				case OID_PROBE_FLAGS:
					SmartToolMsgChar(STP_GET_RESP, uiOID, g_ConfigData.cProbeFlags);
					break;
				case OID_MO_CAL:
					p_MOCal=(td_MOCal *)p_cSTPobjValBuff;
#ifdef OBSTRUCTION_SYSTEM_MO_ANA
					p_MOCal->uim1=htons(g_ConfigData.MOCal.uim1);
					p_MOCal->uim2=htons(g_ConfigData.MOCal.uim2);
					p_MOCal->uim3=htons(g_ConfigData.MOCal.uim3);
					p_MOCal->uim4=htons(g_ConfigData.MOCal.uim4);
					p_MOCal->uim5=htons(g_ConfigData.MOCal.uim5);
					p_MOCal->uim6=htons(g_ConfigData.MOCal.uim6);
#else
					memset(p_cSTPobjValBuff,0,sizeof(td_MOCal));
#endif
					SmartToolMsg(STP_GET_RESP, uiOID, sizeof(td_MOCal), p_cSTPobjValBuff);
					break;
				case OID_RFID_TAG_SET:
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
						memcpy(p_cSTPobjValBuff,g_RfidStateAndData.szTagData+2,ui);//do not include 1st 2 chars which have endcode
					}
					SmartToolMsg(STP_GET_RESP, uiOID, sizeof(td_oid_rfid_tag_set), p_cSTPobjValBuff);
p_cSTPobjValBuff[ui]=0;
logf("rtsfw \"%s\"\r\n",p_cSTPobjValBuff);
#endif
					break;
				case OID_KHOLE_MAX_DISTANCE_CHECK:
					SmartToolMsgFloat(STP_GET_RESP, uiOID, g_ConfigData.fMaxKholeDistanceCheck);
					break;
				case OID_MAX_EDGE_SHIFT_PROBE_ACCEPT:
					SmartToolMsgFloat(STP_GET_RESP, uiOID, g_ConfigData.fMaxEdgeShiftProbeAccept);
					break;
				case OID_ALLOW_DRILL_BEYOND_SHIFT_LIMITS:
					SmartToolMsgChar(STP_GET_RESP, uiOID, g_ConfigData.cAllowDrillBeyondShiftLimits);
					break;
				case OID_NAC_SERIAL_NUMBER:
					SmartToolMsgStr( STP_GET_RESP, uiOID, g_szRailSerialNumber );
					break;
				case OID_Y_RETRACT:
//FIXME INCOMPLETE FEATURE
					SmartToolMsgChar( STP_GET_RESP, uiOID, 0 );
					break;
				case OID_SYSTEM_COMPONENTS:
					SendSystemComponents( STP_GET_RESP );
					break;
				case OID_RFID_TAG_SET2:
#ifdef USE_RFID_MIFARE
					p_rfid_f0 = (td_RFID_F0 *)g_bRFIDTagData;
					p_oid_rfid_tag_set2 = (td_oid_rfid_tag_set2 *)p_cSTPobjValBuff;
					if (p_rfid_f0->cFormat==0)
					{
						p_oid_rfid_tag_set2->cFormat = p_rfid_f0->cFormat;
						p_oid_rfid_tag_set2->cRailType = p_rfid_f0->cRailType;
						p_oid_rfid_tag_set2->cGroup = p_rfid_f0->cGroup;
						p_oid_rfid_tag_set2->cSegment = p_rfid_f0->cSegmentAndRailSide & 0x7F;
						p_oid_rfid_tag_set2->cRailSide = 0;
						if ((p_rfid_f0->cSegmentAndRailSide & 128) > 0) { p_oid_rfid_tag_set2->cRailSide = 1; }
						p_oid_rfid_tag_set2->ulSerialNumber = p_rfid_f0->ulSerialNumber; //tag data is already in network order
						p_oid_rfid_tag_set2->ulPosition = p_rfid_f0->ulPosition;
						p_oid_rfid_tag_set2->ulSegmentPosition = p_rfid_f0->ulSegmentPosition;
						SmartToolMsg( STP_GET_RESP, uiOID, sizeof(td_oid_rfid_tag_set2), p_cSTPobjValBuff );
					}
#endif
					break;
				//OIDs from Tool Module
				case OID_TOOL_MGMT:
					p_oid_tool_mgmt=(td_oid_tool_mgmt *)p_STPrxMsg->p_cObjectValue;
					c_op = p_oid_tool_mgmt->coperation;
					if (c_op == load)
					{
						if(g_szToolIDlen==0)
						{
        	            	c_op = unload;
							SendToolMGMT(STP_GET_RESP,c_op,0,0,"",0);
							break;
						}
	                	SendTool(STP_GET_RESP,c_op);
					}
					//handle everything else in a common way
					goto handle_tool_mgmt;
					break;
				case OID_TOOL_REC:
//FIXMENOWzxcv  see same issue with OID and labels and space...
					c_op = load; //MiniFTControl pays more attention to the ID for loaded or unloaded status,
								 //but Cell controller wants to see that op reflect the loaded status.
					if(g_szToolIDlen==0)
					{
                    	c_op = unload;
					}
                	SendTool(STP_ALERT,c_op);
					break;
                //MakeOID-generated::END
                default:
                    #ifdef OUTPUT_RXSTP
					p_szOIDName = DisplayOIDName(uiOID);
                    goto show_unsupported_stp_message;
                    #endif
                    break;
            }
            break;
        case STP_SET:
            switch(uiOID)
            {
				//MakeOID-generated:: STP SET (oid merge)
				case OID_NULLOID:
                    //Do nothing
					break;
				case OID_DEBUG:
					//This debug capabillity is disabled
					SocketConsoleClose(); //gives a way to reset this
					break;
				case OID_SERIAL_NUMBER:
					ui=p_STPrxMsg->uiValueLength;
					if (ui>64 - 1)
					{
						SmartToolMsgCommonMessageCode(uiOID,COMMONMC_BADLENGTH);
                        break;
					}
                    if (ui==1 && p_STPrxMsg->p_cObjectValue[0]=='c')
                    {
                    	//special code means to clear the serial number
                        g_szSerialNumber[0]=0;
                    }
                    else if (g_szSerialNumber[0]==0)
                    {
                    	//if the serial number is clear, we can write anything into it
						memcpy(g_szSerialNumber,p_STPrxMsg->p_cObjectValue,ui);
						g_szSerialNumber[ui]=0;
                    }
                    g_pSTPSession = (void *)0; //no limit
					SmartToolMsgStr(STP_ALERT, OID_SERIAL_NUMBER, g_szSerialNumber);
                    g_pSTPSession = p_STPSession; //restore limit
					break;
				case OID_RESET_SYSTEM:
                    RebootRabbit();
					break;
				case OID_SCRIPT_TRANSFER:
                	//FIXME minor  Implemented on Pendant, but not on Rabbit
					break;
				case OID_GENERICMESSAGE:
                    // fixme minor: do nothing with a SETting of a generic string message
					break;
				case OID_DEBUGMESSAGE:
                    // fixme minor: do nothing with a SETting of a debug string message
					break;
				case OID_COMMON_MESSAGE_CODE:
                	// return nothing... used entirely for alerts
					break;
				case OID_CONTROL_DELAY:
					//code indicates UI delay
					#ifdef OUTPUT_SESSION_EVENTS
					logf("Cntrl Dly\r\n");
					#endif
					StopInterfaceTasks();
					break;
				case OID_DIRECT_DOUT:
					p_oid_direct_dout=(td_oid_direct_dout *)p_STPrxMsg->p_cObjectValue;
                    if (p_oid_direct_dout->cchannel < 16)
                    {
                    	if (p_oid_direct_dout->cvalue!=0)
                        {
                        	p_oid_direct_dout->cvalue=1;
                        }
                       	//logf("S%d=%d\r\n",p_oid_direct_dout->cchannel,p_oid_direct_dout->cvalue);
						digOut(p_oid_direct_dout->cchannel,p_oid_direct_dout->cvalue);
                    }
					break;
				case OID_DIRECT_DIN:
					p_oid_direct_din=(td_oid_direct_din *)p_STPrxMsg->p_cObjectValue;
					p_oid_direct_din->cvalue = digIn(p_oid_direct_din->cchannel);
                   	//logf("I%d=%d\r\n",p_oid_direct_din->cchannel,p_oid_direct_din->cvalue);
					break;
				case OID_STREAM_SYNC:
                	//Echo Back same value
					SmartToolMsg(STP_ALERT, uiOID, 2, p_STPrxMsg->p_cObjectValue);
                    //NTOHS value for display here
					//ui=(unsigned int)ntohs( *((int*)(p_STPrxMsg->p_cObjectValue)) );
                    //logf("STREAM_SYNC %u\r\n", ui);
					break;
				case OID_STP_CLOSE:
					i=p_STPrxMsg->p_cObjectValue[0];
                    //FIXME MED should close this side now.
                    //logf("STP_CLOSE %d\r\n",i);
					break;
				case OID_MINIFT_MESSAGE_CODE:
                	// return nothing... used entirely for alerts
					break;
				case OID_STP_IDENTIFY:
                	//The New Identify
					SmartToolMsgStr(STP_ALERT, OID_SMARTTOOL_TYPE, SMARTTOOL_TYPE_STRING);
					SmartToolMsgStr(STP_ALERT, OID_SMARTTOOL_SUBTYPE, SMARTTOOL_SUBTYPE_STRING);
                    SmartToolMsgStr(STP_ALERT, OID_SYSTEM_VERSION, SMARTTOOL_SUBTYPE_STRING " " SYSTEM_VERSION_STRING);
					SmartToolMsgStr(STP_ALERT, OID_SERIAL_NUMBER, g_szSerialNumber);
					SmartToolMsgUInt(STP_ALERT, OID_COM_VERSION, COM_VERSION);
					SmartToolMsgEmpty(STP_ALERT, OID_STP_IDENTIFY);
					break;
				case OID_DATE_TIME:
					p_DateTime=(td_DateTime *)p_STPrxMsg->p_cObjectValue;
					g_DateTime.ulticksMSW=ntohl(p_DateTime->ulticksMSW);
					g_DateTime.ulticksLSW=ntohl(p_DateTime->ulticksLSW);
					g_DateTime.uiyear=ntohs(p_DateTime->uiyear);
					g_DateTime.cmonth=p_DateTime->cmonth;
					g_DateTime.cdayOfMonth=p_DateTime->cdayOfMonth;
					g_DateTime.chour=p_DateTime->chour;
					g_DateTime.cminute=p_DateTime->cminute;
					g_DateTime.csecond=p_DateTime->csecond;
					g_DateTime.uimillisecond=ntohs(p_DateTime->uimillisecond);
					g_ulDateTimeMS = MS_TIMER;
					#ifdef CLAMP_SYSTEM_NAC_STP
					NACSendDateTime();
					#endif
					break;
				case OID_MODE:
					g_cModeState=p_STPrxMsg->p_cObjectValue[0];
					break;
				case OID_RESET_MC:
					RunRestart();
					break;
				case OID_ENCODER_RATIO:
					p_EncoderRatio=(td_EncoderRatio *)p_STPrxMsg->p_cObjectValue;
					g_ConfigData.EncoderRatio.fX=p_EncoderRatio->fX;
					g_ConfigData.EncoderRatio.fY=p_EncoderRatio->fY;
					SetEncoderRatios();
					break;
				case OID_MC_CURRENT_LIMITS:
					p_MCCurrentLimit=(td_MCCurrentLimit *)p_STPrxMsg->p_cObjectValue;
					g_ConfigData.MCCurrentLimit.fX=p_MCCurrentLimit->fX;
					g_ConfigData.MCCurrentLimit.fY=p_MCCurrentLimit->fY;
#ifdef GALIL
					SetMCCurrentLimits(g_ConfigData.MCCurrentLimit.fX,g_ConfigData.MCCurrentLimit.fY);
#else
goto common_code_set_current_limits;
#endif
					break;
				case OID_MC_PEAK_CURRENT_LIMITS:
					p_MCPeakCurrentLimit=(td_MCPeakCurrentLimit *)p_STPrxMsg->p_cObjectValue;
					g_ConfigData.MCPeakCurrentLimit.fX=p_MCPeakCurrentLimit->fX;
					g_ConfigData.MCPeakCurrentLimit.fY=p_MCPeakCurrentLimit->fY;
#ifdef GALIL
					//Galil version has no support for this.
#else
common_code_set_current_limits:
					SetMCCurrentLimits(g_ConfigData.MCCurrentLimit.fX,g_ConfigData.MCCurrentLimit.fY,
                    					g_ConfigData.MCPeakCurrentLimit.fX,g_ConfigData.MCPeakCurrentLimit.fY);
#endif
					break;
				case OID_BRAKE_ON_TIMEOUT:
					g_ConfigData.uiBrakeOnTimeout=(unsigned int)ntohs( *((int*)(p_STPrxMsg->p_cObjectValue)) );
					break;
				case OID_STOREDEFAULT_CONFIG:
					if (UBwriteConfig() == 0)
					{
                        uiMessageCode=MINIFTMC_STOREDEFAULT_SUCCESS;
					}
					else
					{
                        uiMessageCode=MINIFTMC_STOREDEFAULT_FAILURE;
					}
					// provide user with a message to act as feedback that the config store worked (or didn't)
					SmartToolMsgMiniFtMessageCode(OID_STOREDEFAULT_CONFIG, uiMessageCode);
					break;
   				case OID_RECALL_CONFIG:
					iTemp=p_STPrxMsg->p_cObjectValue[0];
                    if (iTemp==0)
                    {
                    	//factor defaults : reload parameters
						InitConfig();
						uiMessageCode=MINIFTMC_LOADFACTORYSETTINGS_SUCCESS;
                    }
                    else if (iTemp==1)
                    {
                    	//user block recall
						if (UBreadConfig() == 0)
                        {
							uiMessageCode=MINIFTMC_LOADDEFAULT_SUCCESS;
                        }
                        else
                        {
							uiMessageCode=MINIFTMC_LOADDEFAULT_FAILURE;
                        }
                    }
                    else
                    {
						uiMessageCode=MINIFTMC_INVALID_PARAMETER;
                    }
					// provide user with a message to act as feedback that the config store worked (or didn't)
					SmartToolMsgMiniFtMessageCode(OID_RECALL_CONFIG, uiMessageCode);
					ClearMotionSet();
					break;
				case OID_EEOPTION_DEFAULT:
					#ifdef EEOPTION_PERMANENT
                    //Never Allow EEOPTION CHANGES
					#else
					g_ConfigData.cEEOptionDefault=p_STPrxMsg->p_cObjectValue[0];
                    //logf("EEOpDef=%d\r\n",g_ConfigData.cEEOptionDefault);
                    #endif
					break;
				case OID_EEOPTION:
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
					SmartToolMsgChar(STP_ALERT, uiOID, g_cEEOption);
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

				case OID_BEEPER:
					c=p_STPrxMsg->p_cObjectValue[0];
                    if (c<=1)
                    {
                    	//Active Or Deactivate the beeper
                        g_ConfigData.cbeeper = c;
                    }
                    else
                    {
                       	ui=c*4;
						//make a beep of this delay
   	                	//issue the beep for this duration
   	                	BeepCountPrivate(BEEPONCE,1,c,c);
                    }
					break;
				case OID_TOOL_VERIFYENABLE:
					g_ConfigData.cToolVerifyEnable=p_STPrxMsg->p_cObjectValue[0];
					break;
				case OID_TOOL_RESERVED:
					break;
				case OID_PROCESS:
					//Read Only Currently:  MiniFT Loads the Processes as needed.
					//g_cLoadedProcess=p_STPrxMsg->p_cObjectValue[0];
					//#ifdef OUTPUT_PROCESS_AND_TOOL
                    //logf("Rx Proc %d\r\n",g_cLoadedProcess);
					//#endif
					break;
				case OID_RETURN_HEIGHT:
					f = *(float *)p_STPrxMsg->p_cObjectValue;
					g_ConfigData.fReturnHeight=f;
					LoadToolHomeBackToDrill();
					break;
				case OID_GAUGE_LENGTH:
					//This can set g_fGaugeLength, but it's not the prefered way, which is for drill sync to do it.
					//This is more for special testing
					//Not Relation to DRILL_DIRECT_READY
					f = *(float *)p_STPrxMsg->p_cObjectValue;
					g_fGaugeLength=f;
					#ifdef OUTPUT_PROCESS_AND_TOOL
                    logf("Rx GL %f\r\n",f);
					#endif
					LoadToolHomeBackToDrill();
					break;
				case OID_SCALE_MODE:
					#ifdef LINEAR_SCALE_OPTION
					g_ConfigData.cScaleMode=p_STPrxMsg->p_cObjectValue[0];
					#else
					g_ConfigData.cScaleMode=0; //force to zero
					#endif
					break;
				case OID_RESERVED_2:
					break;
				case OID_PARTPGM_CLEAR:
                   	g_szPartPgmFilename[0]=0;
	                ClearPartProgram();
                    g_PartPgmInfo.cLocked=0; //unlocked when cleared by OID
    				SmartToolMsgChar(STP_ALERT, OID_PARTPGM_STATUS, g_cPartPgmStatus);
					break;
				case OID_PARTPGM_NAME:
                    ui=p_STPrxMsg->uiValueLength;	// If PP name is too long, abort.
					if (ui> MAX_FILENAME_LEN)
					{
						SmartToolMsgCommonMessageCode(uiOID,COMMONMC_BADLENGTH);
						ui=0;//wipe out string
					}
					else
					{
    					memcpy(g_szPartPgmFilename,p_STPrxMsg->p_cObjectValue,ui);
    					g_szPartPgmFilename[ui]=0;
						#ifdef OUTPUT_PP_SYS
    					logf("PPnm=%s\r\n",g_szPartPgmFilename); //debug
						#endif
                        g_iProgramLoadingSession = 0xFF; //Prepared for loading fresh
					}
					break;
				case OID_PARTPGM_DATA:
					ui=p_STPrxMsg->uiValueLength;
                   	//new style is not true streaming, but is superior because it doesn't
                   	//lock out other communication while the file is being parsed.
					if (g_iProgramLoadingSession == 0xFF)
                    {
                    	if (g_cConfigLoadSuccess == 0)
                        {
                        	//They Can't Load a file yet
							SmartToolMsgMiniFtMessageCode(OID_PARTPGM_DATA, MINIFTMC_PROGRAM_WITHOUT_MEMORY_CONFIG);
                            break;
                        }
                    	//This packet is the 1st in the series
                    	if (g_cPartPgmStatus==PP_LOADING)
                        {
                        	//it was already loading, and they are startin a new one
                            //Should be an error???? a warning????
                        }
	               		//We want to start loading, and it's clear so we'll go ahead.
						g_iProgramLoadingSession = p_STPSession->iSessionNum;
						//Start Parse....
						g_sPPDirect = (char *)p_STPrxMsg->p_cObjectValue;
    	                g_iPPDirectPos = 0;
						g_iPPDirectEnd = ui;
    					ParsePartProgramStart();
						#ifdef OUTPUT_PP_SYS
                        logf("PPstrt\r\n");
						#endif
					}
					else if (g_cPartPgmStatus!=PP_LOADING)
                    {
						if (ui==0)
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
                            g_sPPDirect = (char *)temppploadbuffer; //use this location, which is empty until after file load.
							memcpy(g_sPPDirect+g_iPPDirectEnd,(char *)p_STPrxMsg->p_cObjectValue,ui);
                            g_iPPDirectEnd += ui;
                        }
                        else
                        {
                	       	//no data left previously
							g_sPPDirect = (char *)p_STPrxMsg->p_cObjectValue;
    		                g_iPPDirectPos = 0;
							g_iPPDirectEnd = ui;
                        }
                    }
					//It is loading
					//Handle This message
					g_cPPDirectFinalCall = 0;
					if (ui==0)
					{
						//zero length message signals the end of the file load.
						g_cPPDirectFinalCall = 1;
                        //FIXME9999999 should be done file previously
                        //Create a warning for this
						#ifdef OUTPUT_PP_SYS
                        logf("Final Pckt+ddnt cmp\r\n"); //Final Packet but didn't complete yet
						#endif
					}
                    g_pSTPSession = (void *)0; //no limit
                  	ParsePartProgramContinue(0);
                    g_pSTPSession = p_STPSession; //restore limit
                    if (g_iPPDirectPos < g_iPPDirectEnd)
                    {
                    	if (g_iPPDirectPos > 0)
                        {
                        	g_iPPDirectEnd = g_iPPDirectEnd - g_iPPDirectPos; //new length
                        	memcpy((char *)temppploadbuffer,g_sPPDirect+g_iPPDirectPos,g_iPPDirectEnd);
                            g_iPPDirectPos = 0; //new start is back at zero..
                        }
                    }
					break;
				case OID_PARTPGM_LINEARJOB:
					g_iPartPgmLinearHoles=ntohs( *((int*)(p_STPrxMsg->p_cObjectValue)) );
					if (g_iPartPgmLinearHoles < 0 ) { g_iPartPgmLinearHoles=0; }
					LoadLinearProgram();
					g_cScaleBasedOnProbe = 1; //And Set This
					break;
				case OID_STARTOVER:
					//See StartOver(); for notes
                    StartOver();
                    break;
				case OID_LOAD_YRAIL:
					ui=p_STPrxMsg->p_cObjectValue[0];
					//clear clamp but don't need to wait for it.
					#ifdef CLAMP_SYSTEM
					if (g_cClampState==CLAMP_CLAMP)
					{
						g_cClampGoal=CLAMP_LOOSE_OR_UNCLAMP;
    				}
                    #endif
					RunLoadYRail((char)ui);
					break;
				case OID_GRAVCOMP_AXES:
					g_ConfigData.cGravCompAxes=p_STPrxMsg->p_cObjectValue[0];
					break;
				case OID_GRAVCOMP_CMD:
					if (g_cAction>ACTION_READY)
					{
						//logf("NOG\r\n");
						SmartToolMsgMiniFtMessageCode(OID_GRAVCOMP_CMD, MINIFTMC_GRAVCOMP_PREVENTED);
                        break;
					}
					if (g_cGravCompStatus == GRAVCOMP_RUNNING)
				    {
						SmartToolMsgMiniFtMessageCode(OID_GRAVCOMP_CMD, MINIFTMC_GRAVCOMP_PREVIOUSLY_RUNNING);
				    }
				    else if (g_cModeState == MODE_ESTOP)
				    {
						SmartToolMsgMiniFtMessageCode(OID_GRAVCOMP_CMD, MINIFTMC_GRAVCOMP_PREVENTED_BY_ESTOP);
					}
					else if (g_cObstructionCode!=0)
					{
						AlertObstructionCode();
					}
					else if (g_cMoveDone != MOVEDONE_TRUE)
					{	// disallow grav comp unless carriage is done with previous move
						SmartToolMsgMiniFtMessageCode(OID_GRAVCOMP_CMD, MINIFTMC_WAIT_FOR_CARRIAGE_STOP);
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
			            SmartToolMsgChar(STP_ALERT, OID_GRAVCOMP_STATUS, g_cGravCompStatus);
        			    SmartToolMsgChar(STP_ALERT, OID_FLOAT_STATUS, g_cFloatStatus);
            		    SetMoveParams(g_ConfigData.fPosnTolerance*10,0); //Grav Comp Shares Tolerance Test for Move Success.
						RunGravComp();
                        g_ulGravCompTime = MS_TIMER;
                    }
					break;
				case OID_GRAVCOMP_FLOAT:
					if (g_cAction>ACTION_READY)
					{
						//logf("NOF\r\n");
						SmartToolMsgMiniFtMessageCode(OID_GRAVCOMP_FLOAT, MINIFTMC_FLOAT_PREVENTED);
                        break;
					}
					ui=p_STPrxMsg->p_cObjectValue[0];
                    DoFloat(ui); //usually FLOAT_TOGGLE
					break;
				case OID_GRAVCOMP_SPEED:
					p_GravCompSpeed=(td_GravCompSpeed *)p_STPrxMsg->p_cObjectValue;
					g_ConfigData.GravCompSpeed.fX=p_GravCompSpeed->fX;
					g_ConfigData.GravCompSpeed.fY=p_GravCompSpeed->fY;
					ClearMotionSet();
					break;
				case OID_GRAVCOMP_ACCEL:
					p_GravCompAcc=(td_GravCompAcc *)p_STPrxMsg->p_cObjectValue;
					g_ConfigData.GravCompAcc.fX=p_GravCompAcc->fX;
					g_ConfigData.GravCompAcc.fY=p_GravCompAcc->fY;
					ClearMotionSet();
					break;
				case OID_GRAVCOMP_DECEL:
					p_GravCompDec=(td_GravCompDec *)p_STPrxMsg->p_cObjectValue;
					g_ConfigData.GravCompDec.fX=p_GravCompDec->fX;
					g_ConfigData.GravCompDec.fY=p_GravCompDec->fY;
					ClearMotionSet();
					break;
				case OID_GRAVCOMP_MOVEDIST:
					p_GravCompMoveDist=(td_GravCompMoveDist *)p_STPrxMsg->p_cObjectValue;
					g_ConfigData.GravCompMoveDist.fX=p_GravCompMoveDist->fX;
					g_ConfigData.GravCompMoveDist.fY=p_GravCompMoveDist->fY;
                    break;
				case OID_GRAVCOMP_ALGORITHM:
					g_ConfigData.cGravCompAlgorithm=p_STPrxMsg->p_cObjectValue[0];
					break;
				case OID_GRAVCOMP_NOISE_LIMIT:
					p_GravCompNoiseLimit=(td_GravCompNoiseLimit *)p_STPrxMsg->p_cObjectValue;
					g_ConfigData.GravCompNoiseLimit.fX=p_GravCompNoiseLimit->fX;
					g_ConfigData.GravCompNoiseLimit.fY=p_GravCompNoiseLimit->fY;
					break;
				case OID_GRAVCOMP_TRIGGERFACTOR:
					p_GravCompTriggerFactor=(td_GravCompTriggerFactor *)p_STPrxMsg->p_cObjectValue;
					g_ConfigData.GravCompTriggerFactor.fX=p_GravCompTriggerFactor->fX;
					g_ConfigData.GravCompTriggerFactor.fY=p_GravCompTriggerFactor->fY;
					break;
				case OID_FLOAT_SPEEDLIMIT:
					f = *(float *)p_STPrxMsg->p_cObjectValue;
					g_ConfigData.fFloatSpeedLimit=f;
					break;
				case OID_JOG_SPEEDLIMIT:
					f = *(float *)p_STPrxMsg->p_cObjectValue;
					g_ConfigData.fJogSpeedLimit=f;
					break;
				case OID_MAX_SPEED_X:
					f = *(float *)p_STPrxMsg->p_cObjectValue;
					g_ConfigData.fMaxSpeedX=f;
					SetMCErrorLimits();
					break;
				case OID_MAX_SPEED_Y:
					f = *(float *)p_STPrxMsg->p_cObjectValue;
					g_ConfigData.fMaxSpeedY=f;
					SetMCErrorLimits();
					break;
				case OID_X_RAIL_SURFACE_OFFSET:
					f = *(float *)p_STPrxMsg->p_cObjectValue;
					g_fXRailSurfaceOffset = f;
					break;
				case OID_PROBE_METHOD:
					g_cProbeMethod=p_STPrxMsg->p_cObjectValue[0];
					break;
				case OID_PROBE_METHOD_DEFAULT:
					g_ConfigData.cProbeMethodDefault=p_STPrxMsg->p_cObjectValue[0];
					break;
				case OID_PROBE_OFFSET:
					p_ProbeOffset=(td_ProbeOffset *)p_STPrxMsg->p_cObjectValue;
					g_ConfigData.ProbeOffset.fX=p_ProbeOffset->fX;
					g_ConfigData.ProbeOffset.fY=p_ProbeOffset->fY;
					break;
				case OID_PROBE_DIR:
					g_ConfigData.iProbeDir=ntohs( *((int*)(p_STPrxMsg->p_cObjectValue)) );
					break;
				case OID_DRILL_DIR:
					g_ConfigData.iDrillDir=ntohs( *((int*)(p_STPrxMsg->p_cObjectValue)) );
					break;
				case OID_KHOLE_MAX_DISTANCE_ERROR:
					f = *(float *)p_STPrxMsg->p_cObjectValue;
					g_ConfigData.fMaxKholeDistanceError=f;
					break;
				case OID_APPROX_LOCATION_ERROR:
					f = *(float *)p_STPrxMsg->p_cObjectValue;
					g_ConfigData.fApproxLocationError=f;
					break;
				case OID_PROBE:
					p_oid_probe=(td_oid_probe *)p_STPrxMsg->p_cObjectValue;
					ccode=p_oid_probe->ccode;
					c=p_oid_probe->cKIndex;
                    if (ccode==PC_CLEAR)
                    {
	                    if (c>g_cKHoleCount)
    	                {
        	            	//bad value
							SmartToolMsgMiniFtMessageCode(OID_PROBE, MINIFTMC_INVALID_KI);
                	        break;
                    	}
                        if (c==0)
                        {
							ResetProbeValues();
							RecalculateLocatingDirectives(0);
							SendProbeValues();//SPSALL
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
                    if (g_cModeState==MODE_PROBE)
                    {
	                    //Store the command
    	                g_cKHolePrbeCommandInput=ccode;
        	            //Store the given probe KIndex
            	        g_cKHolePrbeIndexInput=c;
                	    //Assuming that it's waiting for a trigger
	                	g_cKHolePrbeTrigger=1; //trigger
						if (ccode==PC_STOP)
						{
							//Since this can take effect while In a probe operation, go ahead and set the command now.
							g_cKHolePrbeCommand = g_cKHolePrbeCommandInput;
							//Clear the Trigger, since it doesn't require a trigger to take effect.
							g_cKHolePrbeTrigger=0;
						}
					}
                    else if (g_cModeState==MODE_TEACH)
					{
						//Use This OID to probe teach position...
						//Sender should send PC_ADD or PC_DELETE
						if (ccode==PC_ADD)
						{
							g_Probe.cTeachCaptureTrig = TRUE;
						}
						else if (ccode==PC_DELETE)
						{
							DeletePosition();
						}
						else if (ccode==PC_COMPLETE) //Code shared with normal probe system
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
				case OID_PROBE_POSITION:
					p_oid_probe_position=(td_oid_probe_position *)p_STPrxMsg->p_cObjectValue;
                    //FIXME2 consider better record and relative adjust system, but for now... do it this way.
					#ifdef OUTPUT_PROBE_SYS
                    logf("SetPos %f,%f\r\n",p_oid_probe_position->fX,p_oid_probe_position->fY);
					#endif
                    SetPosition(p_oid_probe_position->fX,p_oid_probe_position->fY);
					break;
				case OID_PROBE_STATUS:
					p_oid_probe_status=(td_oid_probe_status *)p_STPrxMsg->p_cObjectValue;
					c=p_oid_probe_status->cKIndex;
                    if (c==0 || c>g_cKHoleCount)
                    {
                    	//bad value
						#ifdef OUTPUT_PROBE_SYS
                        logf("S PS c\r\n"); //S PS but no ks
						#endif
						SmartToolMsgMiniFtMessageCode(OID_PROBE_STATUS, MINIFTMC_INVALID_KI);
                        break;
                    }
					if (p_oid_probe_status->cStatus>=PS_PENDING_ACCEPTANCE)
					{
						//Only can set something to >=PS_PENDING_ACCEPTANCE if it was allready >=PS_PENDING_ACCEPTANCE
						if (g_cKHolePrbeStatus[c]<PS_PENDING_ACCEPTANCE)
						{
							//may not do this...
                        	break;
						}
					}
					if (p_oid_probe_status->fX >= -90000) //if less, then we should preserve the values.
					{
						//Update Probe Values
    	                g_fKHolePrbeX[c]=p_oid_probe_status->fX;
   	    		        g_fKHolePrbeY[c]=p_oid_probe_status->fY;
					}
					else
					{
    	                p_oid_probe_status->fX=g_fKHolePrbeX[c]; //instead restore them before ECHO
   	    		        p_oid_probe_status->fY=g_fKHolePrbeY[c];
					}
                    g_cKHolePrbeStatus[c]=p_oid_probe_status->cStatus;
					if (p_oid_probe_status->cStatus == PS_APPROXIMATE)
					{
//ProbeStart Test
						//Update Probe Start Also
						g_cKHolePrbeStart[c]=p_oid_probe_status->cStatus;
	                    g_fKHolePrbeStartX[c]=p_oid_probe_status->fX; //These are always set for this kind of status
    	                g_fKHolePrbeStartY[c]=p_oid_probe_status->fY;
						//1st send start
    	                g_pSTPSession = (void *)0; //no limit
						SendProbeStart(STP_ALERT,c); //SPS
						//restore session below
					}
					#ifdef OUTPUT_PROBE_SYS
					logf("S PS K%d %f,%f %d\r\n",c,p_oid_probe_status->fX,p_oid_probe_status->fY,p_oid_probe_status->cStatus);
					#endif
					if (p_oid_probe_status->cStatus<PS_PROBED)
                    {
						g_cProbeComplete = 0; //If a single K Hole is not complete, then probe is not complete
						g_cProbeCalculated = 0;
                    }
   	                g_pSTPSession = (void *)0; //no limit
					SmartToolMsg(STP_ALERT, OID_PROBE_STATUS, sizeof(td_oid_probe_status),(char *)p_oid_probe_status);//SPS
                    //g_pSTPSession = p_STPSession; //restore limit below
                    //Adjust Current Rotation Displays based on probe status.
                    ProbeModeSelectAndSetGlobalRotation();

					//This might be a status update which could complete probing.
					if (p_oid_probe_status->cStatus>=PS_PROBED)
                    {
						//Recalculate Positions During Probe
						RecalculatePositionsDuringProbe(c);//SPS
						AlertProbeStatusUpdate();
						//see if Probe is complete by checking all status
						CheckProbeComplete();
						//If this completes a probe, and they want to complete probe as soon as possible, then, trigger a probe compelte here
    	                if (g_cModeState==MODE_PROBE)
        	            {
							if (g_cProbeComplete==1)
							{
								if (g_ConfigData.cProbeFlags & AUTO_COMPLETE)
								{
									//Using the main system, complete the probe
									//I did this to make sure it would be in sync with any other probe operations
			                    	//Store the command
	    			                g_cKHolePrbeCommandInput=PC_COMPLETE;
    	    			            //Store the given probe KIndex
        	    			        g_cKHolePrbeIndexInput=1;
            	    			    //Assuming that it's waiting for a trigger
	            	    			g_cKHolePrbeTrigger=1; //trigger
								}
							}
							else
							{
								if (g_ConfigData.cProbeFlags & AUTO_MOVE_PROBE)
								{
									c++;
									if (c<=g_cKHoleCount)
									if (g_cKHolePrbeStatus[c]<PS_PROBED)
									{
										//next is not probed.
	    				                g_cKHolePrbeCommandInput=PC_MOVE_PROBE;
    	    				            //Store the probe KIndex
        		    			        g_cKHolePrbeIndexInput=c;
    	        	    			    //Assuming that it's waiting for a trigger
		            	    			g_cKHolePrbeTrigger=1; //trigger
									}
								}
							}
						}
    				}
					else
					{
    	                if (g_cModeState==MODE_PROBE)
        	            {
							//anything depending on this positon should not be probed if this was updated to not be probed.
							ClearPositionsDuringProbe(c);//SPS
						}
						AlertProbeStatusUpdate();
					}
   	                g_pSTPSession = p_STPSession; //restore limit
                    //FIXME6666666666666666... disallow this when probe has been done, or before in probe
                    //This item is really about the pattern of probe selection etc...

                    //FIXME0000000000000000000000000 What is the rule about
                    // setting this after probe is over?????
                    //IF probe was complete, it may no longer be complete, but what do I do then.
                    //Should it only be settable in probe mode????????????
					break;
				case OID_PROBE_ACCEPT_REQUIRED:
					g_ConfigData.cProbeAcceptRequired=p_STPrxMsg->p_cObjectValue[0];
					break;
				case OID_HOME:
					if (g_cAction>ACTION_READY && g_cAction!=ACTION_HOME)
					{
						//logf("NOH\r\n");
						SmartToolMsgMiniFtMessageCode(OID_HOME, MINIFTMC_HOME_PREVENTED);
                        break;
					}
                	#ifndef HOMESYSTEM
                    g_Probe.cHomeTrig = TRUE;
                    #else
                    //FIXME SEVERE  must prevent this when not authorized to do it......
                    //
					if (g_cHomed!=HOME_RUNNING);
					{
						//First see if they are doing a special homing command
    	                ui=p_STPrxMsg->uiValueLength;
						c=p_STPrxMsg->p_cObjectValue[1]; //second character might be an axis specified
						if (ui < 2 || c==0)
						{
							//Do the standard full homing
							//FIXME Action system improvement ideas
							g_cAction = ACTION_HOME;
							g_cHomed=HOME_RUNNING;
							//ALLOW ALL TO RESTART LIKE THIS
							SetAllHomeStatusPending();
						}
                        else if (c==AXIS_X)
						{
							//FIXME FUTURE:  Implement this as way to trigger x
						}
						else if (c==AXIS_Y)
						{
							//special command
							g_cHomed = HOME_RUNNING;
							g_cHomedY = HOME_PENDING;
						}
						ClearSentHomeStatus(); //forces resend
					}
                    #endif
					break;
				case OID_HOME_SPEED:
					p_HomeSpeed=(td_HomeSpeed *)p_STPrxMsg->p_cObjectValue;
					g_ConfigData.HomeSpeed.fX=p_HomeSpeed->fX;
					g_ConfigData.HomeSpeed.fY=p_HomeSpeed->fY;
					ClearMotionSet();
					break;
				case OID_HOME_ACCEL:
					p_HomeAcc=(td_HomeAcc *)p_STPrxMsg->p_cObjectValue;
					g_ConfigData.HomeAcc.fX=p_HomeAcc->fX;
					g_ConfigData.HomeAcc.fY=p_HomeAcc->fY;
					ClearMotionSet();
					break;
				case OID_HOME_DECEL:
					p_HomeDec=(td_HomeDec *)p_STPrxMsg->p_cObjectValue;
					g_ConfigData.HomeDec.fX=p_HomeDec->fX;
					g_ConfigData.HomeDec.fY=p_HomeDec->fY;
					ClearMotionSet();
					break;
				case OID_HOME_MOVEDIST:
					p_HomeMoveDist=(td_HomeMoveDist *)p_STPrxMsg->p_cObjectValue;
					g_ConfigData.HomeMoveDist.fX=p_HomeMoveDist->fX;
					g_ConfigData.HomeMoveDist.fY=p_HomeMoveDist->fY;
					break;
				case OID_PROBE_ADJUST:
                    g_Probe.cProbeAdjustTrig = TRUE;
					break;
				case OID_PROBE_ADJUST_LIMIT:
					f =  *(float *)p_STPrxMsg->p_cObjectValue;
					g_ConfigData.fProbeAdjustLimit=f;
					break;
				case OID_HOME_FINE_SPEED:
					p_HomeFineSpeed=(td_HomeFineSpeed *)p_STPrxMsg->p_cObjectValue;
					g_ConfigData.HomeFineSpeed.fX=p_HomeFineSpeed->fX;
					g_ConfigData.HomeFineSpeed.fY=p_HomeFineSpeed->fY;
					break;
				case OID_POSNMODE_MOVETONEXT:
	                //logf("MN\r\n");
                    if (MoveAllowed(uiOID)==0)
					{
						//logf("MA=0\r\n");
						break;
					}
   					SpecifyGotoPosn(GOTOPOSN_NEXT,0);
                    g_cAutoMove=0; //turn off auto move if anything requested the move directly
					break;
				case OID_POSNMODE_MOVETOPREV:
                    if (MoveAllowed(uiOID)==0) { break; }
   					SpecifyGotoPosn(GOTOPOSN_PREV,0);
                    g_cAutoMove=0; //turn off auto move if anything requested the move directly
                    break;
				case OID_POSNMODE_MOVETOIND:
                    if (MoveAllowed(uiOID)==0) { break; }
					iTemp=ntohs( *((int*)(p_STPrxMsg->p_cObjectValue)) );
                    SpecifyGotoPosn(GOTOPOSN_RANDOM_INDEX,iTemp);
                    g_cAutoMove=0; //turn off auto move if anything requested the move directly
                    break;
				case OID_POSNMODE_MOVEAGAIN:
                    if (MoveAllowed(uiOID)==0) { break; }
   					SpecifyGotoPosnAgain();
                    g_cAutoMove=0; //turn off auto move if anything requested the move directly
					break;
				case OID_POSNMODE_MOVETYPE:
					g_ConfigData.cMoveType=p_STPrxMsg->p_cObjectValue[0];
                    SendActivePremove(STP_ALERT);
					break;
				case OID_POSNMODE_PREMOVEXY:
					p_PreMove=(td_PreMove *)p_STPrxMsg->p_cObjectValue;
					g_ConfigData.PreMove.fX=p_PreMove->fX;
					g_ConfigData.PreMove.fY=p_PreMove->fY;
                    SendActivePremove(STP_ALERT);
					break;
				case OID_POSNMODE_SPEED:
					p_PosnSpeed=(td_PosnSpeed *)p_STPrxMsg->p_cObjectValue;
					g_ConfigData.PosnSpeed.fX=p_PosnSpeed->fX;
					g_ConfigData.PosnSpeed.fY=p_PosnSpeed->fY;
					ClearMotionSet();
					break;
				case OID_POSNMODE_ACCEL:
					p_PosnAcc=(td_PosnAcc *)p_STPrxMsg->p_cObjectValue;
					g_ConfigData.PosnAcc.fX=p_PosnAcc->fX;
					g_ConfigData.PosnAcc.fY=p_PosnAcc->fY;
					ClearMotionSet();
					break;
				case OID_POSNMODE_DECEL:
					p_PosnDec=(td_PosnDec *)p_STPrxMsg->p_cObjectValue;
					g_ConfigData.PosnDec.fX=p_PosnDec->fX;
					g_ConfigData.PosnDec.fY=p_PosnDec->fY;
					ClearMotionSet();
					break;
				case OID_POSNMODE_FINALSPEED:
					p_PosnFinalSpeed=(td_PosnFinalSpeed *)p_STPrxMsg->p_cObjectValue;
					g_ConfigData.PosnFinalSpeed.fX=p_PosnFinalSpeed->fX;
					g_ConfigData.PosnFinalSpeed.fY=p_PosnFinalSpeed->fY;
					break;
                case OID_ORTHO_SLOPE:
					f =  *(float *)p_STPrxMsg->p_cObjectValue;
					g_ConfigData.fOrthoSlope=f;
					break;
				case OID_POSNERR_LIMIT:
					p_PosnErrLimit=(td_PosnErrLimit *)p_STPrxMsg->p_cObjectValue;
					g_ConfigData.PosnErrLimit.fX=p_PosnErrLimit->fX;
					g_ConfigData.PosnErrLimit.fY=p_PosnErrLimit->fY;
					SetMCErrorLimits();
                    break;
				case OID_POSNMODE_TOLERANCE:
					f =  *(float *)p_STPrxMsg->p_cObjectValue;
					g_ConfigData.fPosnTolerance=f;
					break;
				case OID_VELERR_LIMIT:
					p_VelErrLimit=(td_VelErrLimit *)p_STPrxMsg->p_cObjectValue;
					g_ConfigData.VelErrLimit.fVLimitMarginX=p_VelErrLimit->fVLimitMarginX;
					g_ConfigData.VelErrLimit.fVErrorX=p_VelErrLimit->fVErrorX;
					g_ConfigData.VelErrLimit.fVLimitMarginY=p_VelErrLimit->fVLimitMarginY;
					g_ConfigData.VelErrLimit.fVErrorY=p_VelErrLimit->fVErrorY;
					SetMCErrorLimits();
					break;
				case OID_LONG_DISTANCE:
					p_LongDistance=(td_LongDistance *)p_STPrxMsg->p_cObjectValue;
					g_ConfigData.LongDistance.fX=p_LongDistance->fX;
					g_ConfigData.LongDistance.fY=p_LongDistance->fY;
					break;
				case OID_LONG_SPEED:
					p_LongSpeed=(td_LongSpeed *)p_STPrxMsg->p_cObjectValue;
					g_ConfigData.LongSpeed.fX=p_LongSpeed->fX;
					g_ConfigData.LongSpeed.fY=p_LongSpeed->fY;
					break;
				case OID_OP_STARTED:
                	//FIXME0000 if old DF was replaced by new style, then much could could be removed..
					ui=(unsigned int)ntohs( *((int*)(p_STPrxMsg->p_cObjectValue)) );
                    g_pSTPSession = (void *)0; //no limit
					logf("*oid\r\n");
					AddOpHistory(g_PosnMode.iCurPosnIndex,ui);
                    g_pSTPSession = p_STPSession; //restore limit
					#ifdef OUTPUT_OPERATIONS
					logf("OP_S+=%u\r\n",ui);
					#endif
					break;
				case OID_OP_HISTORY:
                    ui=p_STPrxMsg->uiValueLength;
                    if (ui==2)
                    {
						//Set for current position
                    	i=g_PosnMode.iCurPosnIndex;
                    }
                    else if (ui==4)
                    {
						//Set specified position
						i=(unsigned int)ntohs( *((int*)(p_STPrxMsg->p_cObjectValue)) );
						ui=(unsigned int)ntohs( *((int*)(p_STPrxMsg->p_cObjectValue+2)) );
                    }
					SetOpHistory(i,ui);
					break;
                //	 FIXME Future Enhancement array gets
                //		FIXME: Can SmartToolMsg() handle an arbitrariy large point array?
                //case OID_OP_FULLHISTORY:
               	//	//	Send to the pendant the operation history array for all holes in the part program.
                //    p_c = (char *) ( g_PartPgmInfo.xp_cOpHistory );
				//	SmartToolMsg( STP_ALERT, uiOID, g_PartPgmInfo.iNumDataPoints, p_c );
                //    break;
                //
				case OID_DRILL_HOLE_ONE_TIME:
					g_ConfigData.cDrillHoleOneTime=p_STPrxMsg->p_cObjectValue[0];
					if (g_ConfigData.cDrillHoleOneTime==2)
					{
						g_ConfigData.cDrillHoleOneTime=1;
						ClearOpHistory();
					}
					break;
				case OID_AUTOMOVE:
					g_cAutoMove=p_STPrxMsg->p_cObjectValue[0];
                    g_ulStartAutoTime=MS_TIMER;
					break;
				case OID_AUTOMOVE_DELAY:
					g_ulAutoTime=ntohl( *((long*)(p_STPrxMsg->p_cObjectValue)) );
					break;
				case OID_AUTOREPEAT:
					g_cAutoRepeat=p_STPrxMsg->p_cObjectValue[0];
					break;
				case OID_MACHINE_OFFSET:
					p_MachineOffset=(td_MachineOffset *)p_STPrxMsg->p_cObjectValue;
					g_MachineOffset.fX=p_MachineOffset->fX;
					g_MachineOffset.fY=p_MachineOffset->fY;
					break;
				case OID_MACHINE_OFFSET_CADJ:
					g_cMachineOffsetCompensationAdjustment=p_STPrxMsg->p_cObjectValue[0];
					break;
				case OID_MACHINE_OFFSET1:
					p_MachineOffset1=(td_MachineOffset1 *)p_STPrxMsg->p_cObjectValue;
					g_MachineOffset1.fX=p_MachineOffset1->fX;
					g_MachineOffset1.fY=p_MachineOffset1->fY;
					g_MachineOffset1.fYExtension=p_MachineOffset1->fYExtension;
					break;
				case OID_MACHINE_OFFSET2:
					p_MachineOffset2=(td_MachineOffset2 *)p_STPrxMsg->p_cObjectValue;
					g_MachineOffset2.fX=p_MachineOffset2->fX;
					g_MachineOffset2.fY=p_MachineOffset2->fY;
					g_MachineOffset2.fYExtension=p_MachineOffset2->fYExtension;
					break;
				case OID_STATION:
					g_cStation=p_STPrxMsg->p_cObjectValue[0];
//TEMPORARY CODE TO MAKE THIS MOVEMENT:
					if (g_cStation==STATION_LASERPOINTER)
					{
	                    if (MoveAllowed(uiOID)==0) { break; }

						if (g_cModeState == MODE_POSN)
						{
							//Trigger move
	   						SpecifyGotoPosnAgain();
							g_cStationGoal = STATION_LASERPOINTER;
						}
					}
					else if (g_cStation==STATION_UNSPEC)
					{
						g_cStationGoal=STATION_UNSPEC;
					}
					break;
				case OID_TOOL_OFFSET:
					p_oid_tool_offset=(td_oid_tool_offset *)p_STPrxMsg->p_cObjectValue;
                    //logf("TOOL_OFFSET rxd\r\n");
					SetToolOffset(p_oid_tool_offset->fX,p_oid_tool_offset->fY);
					ResetNearestPosition();
					break;
				case OID_TOOL_FLIP:
					#ifndef ORIENTATION_PERMANENT
					g_ConfigData.cToolFlip=p_STPrxMsg->p_cObjectValue[0];
					#ifdef OUTPUT_TOOL_FLIP
                	logf("Flp=%d\r\n",g_ConfigData.cToolFlip);
					#endif
					#ifdef HOMESYSTEM_X_LINK
					g_cHomed = HOME_NOT_DONE;	//overall system home status
					g_cHomedX=0;
					#endif
					#ifdef HOMESYSTEM_Y_LINK
					g_cHomed = HOME_NOT_DONE;	//overall system home status
					g_cHomedY=0;
					#endif
					#endif
					break;
				case OID_DRIVE_THROUGH_BACKLASH:
					ui=p_STPrxMsg->p_cObjectValue[0];
                    if (ui!=1)
                    {
                    	ui=0;
                    }
                    g_ConfigData.cDriveThroughBacklash = ui;
                    #ifdef USE_HYSTERESIS_FROM_CENTERVISION
                    SetDriveThroughBacklash(g_ConfigData.cDriveThroughBacklash,g_fCenterVisionHysX/2.0,g_fCenterVisionHysY/2.0);
                    #endif
					break;
				case OID_DRILL_OFFSET1:
					p_DrillOffset1=(td_DrillOffset1 *)p_STPrxMsg->p_cObjectValue;
					g_ConfigData.DrillOffset1.fX=p_DrillOffset1->fX;
					g_ConfigData.DrillOffset1.fY=p_DrillOffset1->fY;
					g_ConfigData.DrillOffset1.fYExtension=p_DrillOffset1->fYExtension;
					break;
				case OID_DRILL_OFFSET2:
					p_DrillOffset2=(td_DrillOffset2 *)p_STPrxMsg->p_cObjectValue;
					g_ConfigData.DrillOffset2.fX=p_DrillOffset2->fX;
					g_ConfigData.DrillOffset2.fY=p_DrillOffset2->fY;
					g_ConfigData.DrillOffset2.fYExtension=p_DrillOffset2->fYExtension;
					break;
				case OID_OFFSET_SEAL:
					p_OffsetSeal=(td_OffsetSeal *)p_STPrxMsg->p_cObjectValue;
					g_ConfigData.OffsetSeal.fx=p_OffsetSeal->fx;
					g_ConfigData.OffsetSeal.fy=p_OffsetSeal->fy;
					break;
				case OID_OFFSET_FILL:
					p_OffsetFill=(td_OffsetFill *)p_STPrxMsg->p_cObjectValue;
					g_ConfigData.OffsetFill.fx=p_OffsetFill->fx;
					g_ConfigData.OffsetFill.fy=p_OffsetFill->fy;
					break;
				case OID_JOG:
                    ui=p_STPrxMsg->uiValueLength;
					if (ui==0)
					{
						//OID is being used to continue current jog.
						#ifdef JOG_ENABLE_TIME
						g_ulJogEnableTime = MS_TIMER;
						#endif
						break;
					}
					p_oid_jog=(td_oid_jog *)p_STPrxMsg->p_cObjectValue;
//FIXMEJOG FIXMETODAY: maintain a global jog flag that in one check and tell if any motion at all
					if (g_cAction>ACTION_READY)
					{
						//logf("NOJ\r\n");
						SmartToolMsgMiniFtMessageCode(OID_JOG, MINIFTMC_JOG_PREVENTED);
                        break;
					}
					if (g_cMoveDone != MOVEDONE_TRUE)
					{	// disallow jog unless carriage is done with previous move
						SmartToolMsgMiniFtMessageCode(OID_JOG, MINIFTMC_WAIT_FOR_CARRIAGE_STOP);
                        break;
					}
                    if (g_cFloatStatus == FLOATSTAT_FLOAT)
					{
						SmartToolMsgMiniFtMessageCode(OID_JOG, MINIFTMC_JOG_PREVENTED_BY_FLOAT);
                        break;
                    }
                    if (g_cModeState==MODE_POSN && g_cPosnModeState != POSNMODE_WAITNEXTPOSN)
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
                	g_fJogGoalX=p_oid_jog->fX;
					g_fJogGoalY=p_oid_jog->fY;
                    //Invalid Jog Values should be treated as STOP
                    if (g_fJogGoalX > 1.0) { g_fJogGoalX=0; }
                    else if (g_fJogGoalX < -1.0) { g_fJogGoalX=0; }
                    if (g_fJogGoalY > 1.0) { g_fJogGoalY=0; }
                    else if (g_fJogGoalY < -1.0) { g_fJogGoalY=0; }
                    //Now Set Char Flag Values
                    g_cJogGoalX = JOGSTOP;
                    g_cJogGoalY = JOGSTOP;
                    if (g_fJogGoalX < 0) { g_cJogGoalX = JOGNEG; }
                    if (g_fJogGoalX > 0) { g_cJogGoalX = JOGPOS; }
                    if (g_fJogGoalY < 0) { g_cJogGoalY = JOGNEG; }
                    if (g_fJogGoalY > 0) { g_cJogGoalY = JOGPOS; }
					#ifdef OUTPUT_JOG
                    logf("JG%d,%d\r\n",g_cJogGoalX,g_cJogGoalY);
					#endif
					#ifdef JOG_ENABLE_TIME
					g_ulJogEnableTime = MS_TIMER;
					#endif
					//Check to see if move would be in the direction of obstruction
					if (g_cObstructionCode!=0)
					{
						//Check Individual Sensors
						#ifdef Y_LIMIT_SENSORS
    	                if (g_cDigInYPosLimit==0)
        	            {
            	        	if (g_cJogGoalY == JOGPOS) { goto handle_jog_toward_obstruction; }
						}
						if (g_cDigInYNegLimit==0)
						{
							if (g_cJogGoalY == JOGNEG) { goto handle_jog_toward_obstruction; }
						}
                        #endif
						#ifdef OBSTRUCTION_SYSTEM_XP1
						if (g_cDigInObstructionXP1==OBSTRUCTION)
						{
							//X+
							if (g_cJogGoalX == JOGPOS) { goto handle_jog_toward_obstruction; }
						}
						#endif
						#ifdef OBSTRUCTION_SYSTEM_XN1
						if (g_cDigInObstructionXN1==OBSTRUCTION)
						{
							//X-
                    		if (g_cJogGoalX == JOGNEG) { goto handle_jog_toward_obstruction; }
						}
						#endif
						#ifdef OBSTRUCTION_SYSTEM_MOS
						if (g_cDigInObstructionMOS==MO_OBSTRUCTION)
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
							AlertObstructionCode();
    	    	            g_cJogGoalX = JOGSTOP;
	    	                g_cJogGoalY = JOGSTOP;
							#ifdef OUTPUT_JOG
		                    logf("JGobs\r\n");
							#endif
						}
					}
//FIXMETODAY FIXMEJOG
//consider using one char just to indicate that we want to stop or  go
                    g_PosnMode.cOnCurPosn=0;	//moving actions always mean not on position
                    if (g_cModeState == MODE_POSN)
                    {
                    	LEDOff();
                    }
					break;
				case OID_JOG_SPEED:
					p_JogSpeed=(td_JogSpeed *)p_STPrxMsg->p_cObjectValue;
					g_ConfigData.JogSpeed.fX=p_JogSpeed->fX;
					g_ConfigData.JogSpeed.fY=p_JogSpeed->fY;
					ClearMotionSet();
					break;
				case OID_JOG_ACCEL:
					p_JogAcc=(td_JogAcc *)p_STPrxMsg->p_cObjectValue;
					g_ConfigData.JogAcc.fX=p_JogAcc->fX;
					g_ConfigData.JogAcc.fY=p_JogAcc->fY;
					ClearMotionSet();
					break;
				case OID_JOG_DECEL:
					p_JogDec=(td_JogDec *)p_STPrxMsg->p_cObjectValue;
					g_ConfigData.JogDec.fX=p_JogDec->fX;
					g_ConfigData.JogDec.fY=p_JogDec->fY;
					ClearMotionSet();
					break;
				case OID_JOG_FACTOR:
					p_JogFactor=(td_JogFactor *)p_STPrxMsg->p_cObjectValue;
					g_JogFactor.fX=p_JogFactor->fX;
					g_JogFactor.fY=p_JogFactor->fY;
					break;
				case OID_HOME_POSITION_Y_POS:
					p_HomePosnYPos=(td_HomePosnYPos *)p_STPrxMsg->p_cObjectValue;
					g_ConfigData.HomePosnYPos.fX=p_HomePosnYPos->fX;
					g_ConfigData.HomePosnYPos.fY=p_HomePosnYPos->fY;
					break;
				case OID_POSITION_LIMIT_Y_POS:
					p_PosnLimitYPos=(td_PosnLimitYPos *)p_STPrxMsg->p_cObjectValue;
					g_ConfigData.PosnLimitYPos.fMinX=p_PosnLimitYPos->fMinX;
					g_ConfigData.PosnLimitYPos.fMaxX=p_PosnLimitYPos->fMaxX;
					g_ConfigData.PosnLimitYPos.fMinY=p_PosnLimitYPos->fMinY;
					g_ConfigData.PosnLimitYPos.fMaxY=p_PosnLimitYPos->fMaxY;
					//Setting this OID directly will adjust the limit right away,
					//but normally the global home and limits remain as-is until before homing
					SelectHomePositionAndPositionLimits();
#ifdef GALIL
					//No Support on Galil version
#else
					SetPositionLimits(g_cHomed != HOME_RUNNING);
#endif
                    break;
				case OID_HOME_POSITION_Y_NEG:
					p_HomePosnYNeg=(td_HomePosnYNeg *)p_STPrxMsg->p_cObjectValue;
					g_ConfigData.HomePosnYNeg.fX=p_HomePosnYNeg->fX;
					g_ConfigData.HomePosnYNeg.fY=p_HomePosnYNeg->fY;
					break;
				case OID_POSITION_LIMIT_Y_NEG:
					p_PosnLimitYNeg=(td_PosnLimitYNeg *)p_STPrxMsg->p_cObjectValue;
					g_ConfigData.PosnLimitYNeg.fMinX=p_PosnLimitYNeg->fMinX;
					g_ConfigData.PosnLimitYNeg.fMaxX=p_PosnLimitYNeg->fMaxX;
					g_ConfigData.PosnLimitYNeg.fMinY=p_PosnLimitYNeg->fMinY;
					g_ConfigData.PosnLimitYNeg.fMaxY=p_PosnLimitYNeg->fMaxY;
					//Setting this OID directly will adjust the limit right away.
					//but normally the global home and limits remain as-is until before homing
					SelectHomePositionAndPositionLimits();
#ifdef GALIL
					//No Support on Galil version
#else
					SetPositionLimits(g_cHomed != HOME_RUNNING);
#endif
					break;
				case OID_OBSTRUCTION_CODE_MASK:
					g_ConfigData.cObstructionCodeMask=p_STPrxMsg->p_cObjectValue[0];
					break;
				case OID_MACHINE_LOCK_REQUIRED:
					g_ConfigData.cMachineLockRequired = p_STPrxMsg->p_cObjectValue[0];
					break;
				case OID_CLAMP:
					#ifdef CLAMP_SYSTEM
                    ui=p_STPrxMsg->p_cObjectValue[0];
					if (ui>=100 && ui<=110)
					{
						//this is a special shortcut: DSI is telling us what the clamp state is now... this
						//is required because we don't have OID_CLAMP_GOAL etc.. on the outside, but this hack is only temporary
						//because MiniFT will eventually talk as a client to the sub tools in all cases
						ui=ui-100;
						g_cClampState = ui;
						break;
					}
					//set clamp goal directly.
					g_cClampGoal=p_STPrxMsg->p_cObjectValue[0];
                    //Any Time it comes from the Pendant or outside source, clear what was sent to be sure it gets sent again.
					g_cClampGoalSent = 0xFF; //force it to send goal again
                    #endif
					break;
				case OID_ALOCK:
					g_cALockMode=p_STPrxMsg->p_cObjectValue[0];
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
					SmartToolMsgChar(STP_ALERT, OID_ALOCK, g_cALockMode);
					#endif
					break;
				case OID_ALOCKDELAY:
					g_ConfigData.uiALockDelay=(unsigned int)ntohs( *((int*)(p_STPrxMsg->p_cObjectValue)) );
					break;
				case OID_AUNLOCKDELAY:
					g_ConfigData.uiAUnlockDelay=(unsigned int)ntohs( *((int*)(p_STPrxMsg->p_cObjectValue)) );
					break;
				case OID_LEGLOCKDELAY:
					g_ConfigData.uiLegsLockDelay=(unsigned int)ntohs( *((int*)(p_STPrxMsg->p_cObjectValue)) );
					break;
				case OID_LEGUNLOCKDELAY:
					g_ConfigData.uiLegsUnlockDelay=(unsigned int)ntohs( *((int*)(p_STPrxMsg->p_cObjectValue)) );
					break;
				case OID_LEGSUPDELAY:
					g_ConfigData.uiLegsUpDelay=(unsigned int)ntohs( *((int*)(p_STPrxMsg->p_cObjectValue)) );
					break;
				case OID_LEGSDOWNDELAY:
					g_ConfigData.uiLegsDownDelay=(unsigned int)ntohs( *((int*)(p_STPrxMsg->p_cObjectValue)) );
					break;
				case OID_LOWPRESSUREDELAY:
					g_ConfigData.uiLowPressureDelay=(unsigned int)ntohs( *((int*)(p_STPrxMsg->p_cObjectValue)) );
					break;
				case OID_LOWPRESSUREDOWNDELAY:
					g_ConfigData.uiLowPressureDownDelay=(unsigned int)ntohs( *((int*)(p_STPrxMsg->p_cObjectValue)) );
					break;
				case OID_PRESSUREDELAY:
					g_ConfigData.uiPressureDelay=(unsigned int)ntohs( *((int*)(p_STPrxMsg->p_cObjectValue)) );
					break;
				case OID_PRESSUREDOWNDELAY:
					g_ConfigData.uiPressureDownDelay=(unsigned int)ntohs( *((int*)(p_STPrxMsg->p_cObjectValue)) );
					break;
				case OID_LOWPRESSURE:
					g_ConfigData.uiLowPressure=(unsigned int)ntohs( *((int*)(p_STPrxMsg->p_cObjectValue)) );
					break;
				case OID_PRESSURE:
					g_ConfigData.uiPressure=(unsigned int)ntohs( *((int*)(p_STPrxMsg->p_cObjectValue)) );
					//Also, when the default pressure is set and we are clamped, go to that pressure now.
					#ifdef CLAMP_SYSTEM
					g_uiClampPressure = (int)g_ConfigData.uiPressure; //make current pressure
					#ifdef CLAMP_SYSTEM_HD_PISTON
					if (g_cClampState==CLAMP_CLAMP)
					{
						g_uiClampPressureLBS = (unsigned int) g_uiClampPressure;
						SetClampPressureLBS();
					}
					#endif
					g_cClampGoalSent = 0xFF; //force it to send goal again
					#endif
					break;
				case OID_AIR_CLEAR:
					ui=(unsigned int)ntohs( *((int*)(p_STPrxMsg->p_cObjectValue)) );
					if (ui==10) //this was to avoid a certain special signal at one time... for now it remains, but could be removed if investigated
					{
						break;
					}
					#ifdef AIR_CLEAR
					g_ConfigData.uiAirClear=ui;
					#else
					g_ConfigData.uiAirClear=0;
					#endif
					break;
				case OID_LASER_SENSOR_OFFSET:
					p_LaserSensorOffset=(td_LaserSensorOffset *)p_STPrxMsg->p_cObjectValue;
					g_ConfigData.LaserSensorOffset.fX=p_LaserSensorOffset->fX;
					g_ConfigData.LaserSensorOffset.fY=p_LaserSensorOffset->fY;
					#ifdef OUTPUT_OFFSETS
					logf("LS OFFST x=%.5f y=%.5f\r\n",g_ConfigData.LaserSensorOffset.fX,g_ConfigData.LaserSensorOffset.fY);
					#endif
					break;
				case OID_CAM_OFFSET:
#ifdef HD_RAIL_STP
					if (g_cRailSTP==1)
					{
						SmartToolMsgMiniFtMessageCode(OID_HOME_RFID, MINIFTMC_INVALID_PARAMETER); //FIXME Minor  More Specific Error Message
						break;
					}
#endif
					p_CamOffset=(td_CamOffset *)p_STPrxMsg->p_cObjectValue;
					g_ConfigData.CamOffset.fX=p_CamOffset->fX;
					g_ConfigData.CamOffset.fY=p_CamOffset->fY;
					break;
				case OID_VISION_INSPECT:
                    if (MoveAllowed(uiOID)==0) { break; }
					#ifdef CENTERVISION
					g_cCenterVisionInspectType=p_STPrxMsg->p_cObjectValue[0];
					#endif
					g_cInspectMethod = g_ConfigData.cCommandInspectMethod;
					g_cPositionInspection=1;
					g_cCenterVisionRequiredResults=CENTERVISION_CENTER;
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
				case OID_LASER_SENSOR_ALG_PARAM:
					p_LaserSensorAlgParam=(td_LaserSensorAlgParam *)p_STPrxMsg->p_cObjectValue;
					g_ConfigData.LaserSensorAlgParam.fsearch_speed=p_LaserSensorAlgParam->fsearch_speed;
					g_ConfigData.LaserSensorAlgParam.fseek_speed=p_LaserSensorAlgParam->fseek_speed;
					g_ConfigData.LaserSensorAlgParam.frscan_speed=p_LaserSensorAlgParam->frscan_speed;
					g_ConfigData.LaserSensorAlgParam.frscan_speed_fast=p_LaserSensorAlgParam->frscan_speed_fast;
					g_ConfigData.LaserSensorAlgParam.fscan_speed=p_LaserSensorAlgParam->fscan_speed;
					g_ConfigData.LaserSensorAlgParam.fscan_speed_fast=p_LaserSensorAlgParam->fscan_speed_fast;
					g_ConfigData.LaserSensorAlgParam.fprobe_diameter=p_LaserSensorAlgParam->fprobe_diameter;
					g_ConfigData.LaserSensorAlgParam.funknown_diameter=p_LaserSensorAlgParam->funknown_diameter;
					g_ConfigData.LaserSensorAlgParam.cmode=p_LaserSensorAlgParam->cmode;
					g_ConfigData.LaserSensorAlgParam.cmode_fast=p_LaserSensorAlgParam->cmode_fast;
					g_ConfigData.LaserSensorAlgParam.cuse_avg=p_LaserSensorAlgParam->cuse_avg;
					g_ConfigData.LaserSensorAlgParam.cfull_scan=p_LaserSensorAlgParam->cfull_scan;
					g_ConfigData.LaserSensorAlgParam.cgdata_sel=p_LaserSensorAlgParam->cgdata_sel;
					g_ConfigData.LaserSensorAlgParam.cassume_posn=p_LaserSensorAlgParam->cassume_posn;
					g_ConfigData.LaserSensorAlgParam.cassume_posn_fast=p_LaserSensorAlgParam->cassume_posn_fast;
					g_ConfigData.LaserSensorAlgParam.crect_center=p_LaserSensorAlgParam->crect_center;
					g_ConfigData.LaserSensorAlgParam.cloops=p_LaserSensorAlgParam->cloops;
					g_ConfigData.LaserSensorAlgParam.cdelta_mode=p_LaserSensorAlgParam->cdelta_mode;
					g_ConfigData.LaserSensorAlgParam.idelta_flat=ntohs(p_LaserSensorAlgParam->idelta_flat);
					g_ConfigData.LaserSensorAlgParam.fdelta_basespan=p_LaserSensorAlgParam->fdelta_basespan;
					g_ConfigData.LaserSensorAlgParam.idelta_pos=ntohs(p_LaserSensorAlgParam->idelta_pos);
					g_ConfigData.LaserSensorAlgParam.idelta_neg=ntohs(p_LaserSensorAlgParam->idelta_neg);
					g_ConfigData.LaserSensorAlgParam.fdelta_span=p_LaserSensorAlgParam->fdelta_span;
					g_ConfigData.LaserSensorAlgParam.fdelta_edge=p_LaserSensorAlgParam->fdelta_edge;
					g_ConfigData.LaserSensorAlgParam.fpc_aspect_diff=p_LaserSensorAlgParam->fpc_aspect_diff;
					g_ConfigData.LaserSensorAlgParam.fmax_aspect_diff=p_LaserSensorAlgParam->fmax_aspect_diff;
					g_ConfigData.LaserSensorAlgParam.fmax_over_exp_diameter=p_LaserSensorAlgParam->fmax_over_exp_diameter;
					g_ConfigData.LaserSensorAlgParam.fmax_under_exp_diameter=p_LaserSensorAlgParam->fmax_under_exp_diameter;
					g_ConfigData.LaserSensorAlgParam.fmax_csnk_diff=p_LaserSensorAlgParam->fmax_csnk_diff;
					g_ConfigData.LaserSensorAlgParam.fmax_over_csnk=p_LaserSensorAlgParam->fmax_over_csnk;
					g_ConfigData.LaserSensorAlgParam.fmax_under_csnk=p_LaserSensorAlgParam->fmax_under_csnk;
					break;
				case OID_CAM_ALG_PARAM:
					p_CamAlgParam=(td_CamAlgParam *)p_STPrxMsg->p_cObjectValue;
					g_ConfigData.CamAlgParam.fmove_speed=p_CamAlgParam->fmove_speed;
					g_ConfigData.CamAlgParam.cInfoMask=p_CamAlgParam->cInfoMask;
					g_ConfigData.CamAlgParam.cAMode=p_CamAlgParam->cAMode;
					g_ConfigData.CamAlgParam.cCMode=p_CamAlgParam->cCMode;
					g_ConfigData.CamAlgParam.cAux1=p_CamAlgParam->cAux1;
					g_ConfigData.CamAlgParam.fmove_required=p_CamAlgParam->fmove_required;
					g_ConfigData.CamAlgParam.fmax_over_exp_diameter=p_CamAlgParam->fmax_over_exp_diameter;
					g_ConfigData.CamAlgParam.fmax_under_exp_diameter=p_CamAlgParam->fmax_under_exp_diameter;
					g_ConfigData.CamAlgParam.fmax_csnk_diff=p_CamAlgParam->fmax_csnk_diff;
					g_ConfigData.CamAlgParam.fmax_over_csnk=p_CamAlgParam->fmax_over_csnk;
					g_ConfigData.CamAlgParam.fmax_under_csnk=p_CamAlgParam->fmax_under_csnk;
					break;
				case OID_VISION_AUTO_INSPECT:
					g_ConfigData.cVisionAutoInspect=p_STPrxMsg->p_cObjectValue[0];
					break;
				case OID_STOP_INTERFACE_TASK:
					StopInterfaceTasks();
					break;
				case OID_PROCESS_START:
                	//Only let them start if it's waiting and it's on position.
                	if (g_cPosnModeState == POSNMODE_WAITNEXTPOSN)
   	                {
       	            	if (g_PosnMode.cOnCurPosn==1)
           	            {
            	    		g_cStartProcess=1;
                   	    }
                    }
					break;
				case OID_PROCESS_STOP:
					StopProcess();
					break;
				case OID_PROCESS_CONTINUE_MODE:
					g_ConfigData.cProcessContinueMode=p_STPrxMsg->p_cObjectValue[0];
//FIXME000 new for DFINT
//Maybe Echo...
					break;
				case OID_PROCESS_OPERATIONS:
					g_ConfigData.uiProcessOperations=(unsigned int)ntohs( *((int*)(p_STPrxMsg->p_cObjectValue)) );
                    ui=g_ConfigData.uiProcessOperations;
					g_pSTPSession = (void *)0; // no limit
					SmartToolMsgUInt(STP_ALERT, uiOID, ui);
					g_pSTPSession = p_STPSession; //restore limit
					#ifdef OUTPUT_OPERATIONS
                    logf("rpop=%u\r\n",ui);
					#endif
					UpdateStationPlan();
					break;
				case OID_DRILL_STATE:
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
				case OID_DRILL_EXPLANATION:
					g_cDrillExplanation=p_STPrxMsg->p_cObjectValue[0];
					break;
				case OID_SEAL_STATE:
					g_cSealState=p_STPrxMsg->p_cObjectValue[0];
					break;
				case OID_SEAL_CLAMP:
					g_ConfigData.cSealClamp=p_STPrxMsg->p_cObjectValue[0];
					break;
				case OID_SEAL_PRESSURE_DELAY:
					g_ConfigData.iSealPressureDelay=ntohs( *((int*)(p_STPrxMsg->p_cObjectValue)) );
					break;
				case OID_SEAL_PRESSURE_RELEASE_DELAY:
					g_ConfigData.iSealPressureReleaseDelay=ntohs( *((int*)(p_STPrxMsg->p_cObjectValue)) );
					break;
				case OID_SEAL_PRIME_DELAY:
					p_PrimeDelay=(td_PrimeDelay *)p_STPrxMsg->p_cObjectValue;
					g_ConfigData.PrimeDelay.fdiameter1=p_PrimeDelay->fdiameter1;
					g_ConfigData.PrimeDelay.idelay1=ntohs(p_PrimeDelay->idelay1);
					g_ConfigData.PrimeDelay.fdiameter2=p_PrimeDelay->fdiameter2;
					g_ConfigData.PrimeDelay.idelay2=ntohs(p_PrimeDelay->idelay2);
					g_ConfigData.PrimeDelay.fdiameter3=p_PrimeDelay->fdiameter3;
					g_ConfigData.PrimeDelay.idelay3=ntohs(p_PrimeDelay->idelay3);
					g_ConfigData.PrimeDelay.fdiameter4=p_PrimeDelay->fdiameter4;
					g_ConfigData.PrimeDelay.idelay4=ntohs(p_PrimeDelay->idelay4);
					g_ConfigData.PrimeDelay.fdiameter5=p_PrimeDelay->fdiameter5;
					g_ConfigData.PrimeDelay.idelay5=ntohs(p_PrimeDelay->idelay5);
					break;
				case OID_SEAL_GLOB_DELAY:
					g_ConfigData.iSealGlobDelay=ntohs( *((int*)(p_STPrxMsg->p_cObjectValue)) );
					break;
				case OID_SEAL_APPLY_DELAY:
					g_ConfigData.iSealApplyDelay=ntohs( *((int*)(p_STPrxMsg->p_cObjectValue)) );
					break;
				case OID_FILL_STATE:
					g_cFillState=p_STPrxMsg->p_cObjectValue[0];
					break;
				case OID_FILL_CLAMP:
					g_ConfigData.cFillClamp_=p_STPrxMsg->p_cObjectValue[0];
					break;
				case OID_FILL_EXTEND_DELAY:
					g_ConfigData.iFillExtendDelay_=ntohs( *((int*)(p_STPrxMsg->p_cObjectValue)) );
					break;
				case OID_FILL_RAM_DELAY:
					g_ConfigData.iFillRamDelay_=ntohs( *((int*)(p_STPrxMsg->p_cObjectValue)) );
					break;
				case OID_FASTENER_REQUEST:
                	//not setable
					break;
				case OID_FASTENER_ARRIVED:
					g_cFastenerArrived=p_STPrxMsg->p_cObjectValue[0];
					break;
				case OID_FASTENER_POST_DELAY:
					g_ConfigData.iFastenerPostDelay_=ntohs( *((int*)(p_STPrxMsg->p_cObjectValue)) );
					break;
				case OID_ACCEL_ARM:
					g_cAccelArm_=p_STPrxMsg->p_cObjectValue[0];
					break;
				case OID_ACCEL_TRIGGER:
					g_cAccelTrigger_=p_STPrxMsg->p_cObjectValue[0];
					break;
				case OID_TOOL_Z_BASE:
					f = *(float *)p_STPrxMsg->p_cObjectValue;
					g_ConfigData.fToolZBase = f;
					break;
				case OID_MOVEXY:
                //FIXME Minor  Not Implemented Yet
				//	p_oid_movexy=(td_oid_movexy *)p_STPrxMsg->p_cObjectValue;
				//	p_oid_movexy->fMachineX=p_oid_movexy->fMachineX;
				//	p_oid_movexy->fMachineY=p_oid_movexy->fMachineY;
					break;
				case OID_POSN_DISPLAY:
					p_PosnDisplay=(td_PosnDisplay *)p_STPrxMsg->p_cObjectValue;
					g_ConfigData.PosnDisplay.cmode=p_PosnDisplay->cmode;
					g_ConfigData.PosnDisplay.corigin=p_PosnDisplay->corigin;
					g_ConfigData.PosnDisplay.ccontent=p_PosnDisplay->ccontent;
					break;
				case OID_RFID_CONFIG:
					p_RFIDConfig=(td_RFIDConfig *)p_STPrxMsg->p_cObjectValue;
					g_ConfigData.RFIDConfig.cenabled=p_RFIDConfig->cenabled;
					g_ConfigData.RFIDConfig.cmethod=p_RFIDConfig->cmethod;
					g_ConfigData.RFIDConfig.uioptions=ntohs(p_RFIDConfig->uioptions);
					g_ConfigData.RFIDConfig.uicontinuousReadCycleTime=ntohs(p_RFIDConfig->uicontinuousReadCycleTime);
					g_ConfigData.RFIDConfig.uiseekReadCycleTime=ntohs(p_RFIDConfig->uiseekReadCycleTime);
					g_ConfigData.RFIDConfig.fseekMove1=p_RFIDConfig->fseekMove1;
					g_ConfigData.RFIDConfig.fseekMove2=p_RFIDConfig->fseekMove2;
					g_ConfigData.RFIDConfig.fseekFineMove=p_RFIDConfig->fseekFineMove;
					g_ConfigData.RFIDConfig.fseekSpeed=p_RFIDConfig->fseekSpeed;
					g_ConfigData.RFIDConfig.fseekFineSpeed=p_RFIDConfig->fseekFineSpeed;
					g_ConfigData.RFIDConfig.fRFIDOffset=p_RFIDConfig->fRFIDOffset;
					g_ConfigData.RFIDConfig.fseekPastBorder=p_RFIDConfig->fseekPastBorder;
					g_ConfigData.RFIDConfig.fminWindowSize = p_RFIDConfig->fminWindowSize;
					g_ConfigData.RFIDConfig.fmaxWindowSize = p_RFIDConfig->fmaxWindowSize;
					break;
				case OID_RFID_DATA:
					p_RFIDData=(td_RFIDData *)p_STPrxMsg->p_cObjectValue;
					g_RFIDData.cstate=p_RFIDData->cstate;
					g_RFIDData.ccontext=p_RFIDData->ccontext;
					g_RFIDData.cseekstate=p_RFIDData->cseekstate;
					g_RFIDData.ultimestamp=ntohl(p_RFIDData->ultimestamp);
					g_RFIDData.ulrfidtimestamp=ntohl(p_RFIDData->ulrfidtimestamp);
					g_RFIDData.fposition=p_RFIDData->fposition;
					//barray field sztagdata
					g_RFIDData.uicrc16=ntohs(p_RFIDData->uicrc16);
					g_RFIDData.uiendcode=ntohs(p_RFIDData->uiendcode);
					g_RFIDData.ulseektime=ntohl(p_RFIDData->ulseektime);
					g_RFIDData.fsstart=p_RFIDData->fsstart;
					g_RFIDData.fpstart=p_RFIDData->fpstart;
					g_RFIDData.fpend=p_RFIDData->fpend;
					g_RFIDData.fnstart=p_RFIDData->fnstart;
					g_RFIDData.fnend=p_RFIDData->fnend;
					g_RFIDData.fhs1=p_RFIDData->fhs1;
					g_RFIDData.fhs2=p_RFIDData->fhs2;
					g_RFIDData.fhsf=p_RFIDData->fhsf;
					//Read BStrings and/or BArrays after fixed size fields
					p_c = ((char *)p_RFIDData) + sizeof(td_RFIDData);
					ReadBArray(p_c,g_szTagDatalen,g_szTagData,64);
					//now alert this back out
					g_pSTPSession = (void *)0; // no limit
					SendRFIDData(STP_ALERT);
					g_pSTPSession = p_STPSession; //restore limit
					break;

#ifdef WHISTLEMC
				case OID_HOME_RFID:
#ifdef HOMESYSTEM_X
#ifdef HOMESYSTEM_X_RFID
					if (g_cAction==ACTION_HOME && g_cHomed==HOME_RUNNING)
					{
						//special case.... allow reqeust of RFID Home
					}
					else if (g_cAction>ACTION_READY)
					{
						//logf("NOG\r\n");
						SmartToolMsgMiniFtMessageCode(OID_HOME_RFID, MINIFTMC_RFID_HOME_PREVENTED_ACTION);
                        break;
					}
					if (g_cGravCompStatus == GRAVCOMP_RUNNING) //should be covered by action, but may not be
				    {
						SmartToolMsgMiniFtMessageCode(OID_HOME_RFID, MINIFTMC_RFID_HOME_PREVENTED);
						break;
				    }
				    if (g_cModeState != MODE_IDLE)
				    {
						SmartToolMsgMiniFtMessageCode(OID_HOME_RFID, MINIFTMC_RFID_HOME_PREVENTED_MODE);
						break;
					}
					if (g_cObstructionCode!=0)
					{
						AlertObstructionCode();
						break;
					}
					if (g_cAction==ACTION_HOME && g_cHomed==HOME_RUNNING)
					{
						//special case.... allow reqeust of RFID Home... no need for these 3 checks here
					}
					else
					{
					if (g_cMoveDone != MOVEDONE_TRUE)
					{	// disallow home unless carriage is done with previous move
						SmartToolMsgMiniFtMessageCode(OID_HOME_RFID, MINIFTMC_RFID_HOME_PREVENTED_MOVING);
						break;
					}
					if (g_cDrillState != DRILLSTATE_IDLE)
					{
						SmartToolMsgMiniFtMessageCode(OID_HOME_RFID, MINIFTMC_RFID_HOME_PREVENTED_DRILL);
logf("%d\r\n",g_cDrillState);
//try alert of drill state???
						break;
					}
					if (g_cClampState!=g_cClampGoal || g_cClampState!=CLAMP_UNCLAMP)
					{
						SmartToolMsgMiniFtMessageCode(OID_HOME_RFID, MINIFTMC_RFID_HOME_PREVENTED_CLAMP); //FIXME MEDHIGH switch to cause related messages
						break;
					}
					}
					//Main Home Code will operate and start any pending home components
					g_cHomed = HOME_RUNNING;
					g_cHomedX = HOME_PENDING;
#endif
#endif
					break;
				case OID_READ_RFID:
					g_cReadRFID=p_STPrxMsg->p_cObjectValue[0];
					#ifdef USE_RFID_OMRON
					g_cTagReadState = RFID_TAG_CLEAR;
					#endif
					break;
				case OID_HOME_STOP:
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
						StopPosition();
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
						StopPosition();
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
					c=0;
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
				case OID_ESTOP_CLEAR_DELAY:
					g_ConfigData.uiEStopClearDelay=(unsigned int)ntohs( *((int*)(p_STPrxMsg->p_cObjectValue)) );
					break;
				case OID_DRILL_BUTTON_DELAY:
					g_ConfigData.uiDrillButtonDelay=(unsigned int)ntohs( *((int*)(p_STPrxMsg->p_cObjectValue)) );
					break;
				case OID_USE_CUTTER_DETECT:
					g_ConfigData.cUseCutterDetect=p_STPrxMsg->p_cObjectValue[0];
					break;
				case OID_JOG_ENABLE_TIMEOUT:
					g_ConfigData.cJogEnableTimeout=p_STPrxMsg->p_cObjectValue[0];
					break;
				case OID_DRILL_CYCLE_DELAY:
					g_ConfigData.uiDrillCycleDelay=(unsigned int)ntohs( *((int*)(p_STPrxMsg->p_cObjectValue)) );
					break;
				case OID_INSPECT_METHOD:
					g_ConfigData.cInspectMethod=p_STPrxMsg->p_cObjectValue[0];
					break;
				case OID_COMMAND_INSPECT_METHOD:
					g_ConfigData.cCommandInspectMethod=p_STPrxMsg->p_cObjectValue[0];
					break;
				case OID_FORCE_SENSOR_CALIBRATION:
					p_ForceSensorCalibration=(td_ForceSensorCalibration *)p_STPrxMsg->p_cObjectValue;
					g_ConfigData.ForceSensorCalibration.iZeroX=ntohs(p_ForceSensorCalibration->iZeroX);
					g_ConfigData.ForceSensorCalibration.iZeroY=ntohs(p_ForceSensorCalibration->iZeroY);
					g_ConfigData.ForceSensorCalibration.iZeroZ=ntohs(p_ForceSensorCalibration->iZeroZ);
					g_ConfigData.ForceSensorCalibration.iCountsPerGX=ntohs(p_ForceSensorCalibration->iCountsPerGX);
					g_ConfigData.ForceSensorCalibration.iCountsPerGY=ntohs(p_ForceSensorCalibration->iCountsPerGY);
					g_ConfigData.ForceSensorCalibration.iCountsPerGZ=ntohs(p_ForceSensorCalibration->iCountsPerGZ);
					break;
				case OID_FORCE_LIMITS:
					p_ForceLimits=(td_ForceLimits *)p_STPrxMsg->p_cObjectValue;
					g_ConfigData.ForceLimits.uiSensorInterval=ntohs(p_ForceLimits->uiSensorInterval);
					g_ConfigData.ForceLimits.uiMinUpdateDelta=ntohs(p_ForceLimits->uiMinUpdateDelta);
					g_ConfigData.ForceLimits.cActive=p_ForceLimits->cActive;
					g_ConfigData.ForceLimits.cCurrentUnderMethod=p_ForceLimits->cCurrentUnderMethod;
					g_ConfigData.ForceLimits.uiCurrentOverX=ntohs(p_ForceLimits->uiCurrentOverX);
					g_ConfigData.ForceLimits.uiCurrentUnderX=ntohs(p_ForceLimits->uiCurrentUnderX);
					g_ConfigData.ForceLimits.uiCurrentOverY=ntohs(p_ForceLimits->uiCurrentOverY);
					g_ConfigData.ForceLimits.uiCurrentUnderY=ntohs(p_ForceLimits->uiCurrentUnderY);
					g_ConfigData.ForceLimits.uiFullGravX=ntohs(p_ForceLimits->uiFullGravX);
					g_ConfigData.ForceLimits.uiFullGravY=ntohs(p_ForceLimits->uiFullGravY);
					g_ConfigData.ForceLimits.uiFlatForceX=ntohs(p_ForceLimits->uiFlatForceX);
					g_ConfigData.ForceLimits.uiFlatForceY=ntohs(p_ForceLimits->uiFlatForceY);
					break;
				case OID_PROBE_FLAGS:
					g_ConfigData.cProbeFlags=p_STPrxMsg->p_cObjectValue[0];
					break;
				case OID_MO_CAL:
					p_MOCal=(td_MOCal *)p_STPrxMsg->p_cObjectValue;
#ifdef OBSTRUCTION_SYSTEM_MO_ANA
					g_ConfigData.MOCal.uim1=ntohs(p_MOCal->uim1);
					g_ConfigData.MOCal.uim2=ntohs(p_MOCal->uim2);
					g_ConfigData.MOCal.uim3=ntohs(p_MOCal->uim3);
					g_ConfigData.MOCal.uim4=ntohs(p_MOCal->uim4);
					g_ConfigData.MOCal.uim5=ntohs(p_MOCal->uim5);
					g_ConfigData.MOCal.uim6=ntohs(p_MOCal->uim6);
#endif
					break;
				case OID_VISION_EXTERNAL_ANALYSIS:
					#ifdef CENTERVISION_ANALOG_POINT_LASER
					//Requires no NTOH ...pass buffer as object
					HandleVisionExternalAnalysis((td_oid_vision_external_analysis *)p_STPrxMsg->p_cObjectValue);
					#endif
					break;
				case OID_RFID_TAG_SET:
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
				case OID_KHOLE_MAX_DISTANCE_CHECK:
					g_ConfigData.fMaxKholeDistanceCheck = *(float *)p_STPrxMsg->p_cObjectValue;
					break;
				case OID_MAX_EDGE_SHIFT_PROBE_ACCEPT:
					g_ConfigData.fMaxEdgeShiftProbeAccept = *(float *)p_STPrxMsg->p_cObjectValue;
					break;
				case OID_ALLOW_DRILL_BEYOND_SHIFT_LIMITS:
					g_ConfigData.cAllowDrillBeyondShiftLimits=p_STPrxMsg->p_cObjectValue[0];
					break;
				case OID_Y_RETRACT:
					// = p_STPrxMsg->p_cObjectValue[0];
//FIXME INCOMPLETE FEATURE
//FIXME MEDIUM OID_Y_RETRACT  #warns "Need OID_Y_RETRACT completed"
					break;
				case OID_SYSTEM_COMPONENTS:
					p_SystemComponents = (td_SystemComponents *) p_STPrxMsg->p_cObjectValue;
					g_ConfigData.SystemComponents.cDrill = p_SystemComponents->cDrill;
					g_ConfigData.SystemComponents.cFastener = p_SystemComponents->cFastener;
					g_ConfigData.SystemComponents.cFastenerTray = p_SystemComponents->cFastenerTray;
					g_ConfigData.SystemComponents.cAux1 = p_SystemComponents->cAux1;
					g_ConfigData.SystemComponents.cAux2 = p_SystemComponents->cAux2;
					g_ConfigData.SystemComponents.cAux3 = p_SystemComponents->cAux3;
					g_ConfigData.SystemComponents.cAux4 = p_SystemComponents->cAux4;
					g_ConfigData.SystemComponents.cAux5 = p_SystemComponents->cAux5;
					SendSystemComponents( STP_ALERT ); //echo back
//FIXME HIGH echos back to sender only.... issue is OID beyond mapping
//Add and test later.
					break;
				case OID_RFID_TAG_SET2:
#ifdef USE_RFID_MIFARE
					p_oid_rfid_tag_set2 = (td_oid_rfid_tag_set2 *) p_STPrxMsg->p_cObjectValue;
					p_rfid_f0 = (td_RFID_F0 *)g_bRFIDTagData;
					p_rfid_f0->cFormat = p_oid_rfid_tag_set2->cFormat;
					if (p_rfid_f0->cFormat==0)
					{
						p_rfid_f0->cRailType = p_oid_rfid_tag_set2->cRailType;
						p_rfid_f0->cGroup = p_oid_rfid_tag_set2->cGroup;
						p_rfid_f0->cSegmentAndRailSide = p_oid_rfid_tag_set2->cSegment & 0x7F;
						if (p_oid_rfid_tag_set2->cRailSide == 0) { } else
						if (p_oid_rfid_tag_set2->cRailSide == 1) { p_rfid_f0->cSegmentAndRailSide = p_rfid_f0->cSegmentAndRailSide & 128; }
						p_rfid_f0->ulSerialNumber = p_oid_rfid_tag_set2->ulSerialNumber; //tag data is already in network order
						p_rfid_f0->ulPosition = p_oid_rfid_tag_set2->ulPosition;
						p_rfid_f0->ulSegmentPosition = p_oid_rfid_tag_set2->ulSegmentPosition;
						g_szTagDatalen=RFID_F0_DATA_SIZE;
						SL031LoginSector();
						SL031WriteData((char*)g_bRFIDTagData,g_szTagDatalen);
					}
#endif
					break;

//FIXMENOWzxcv must determine if it's better to include this in MINIFT OID max, or allow it as another group beyond.
//I certainly have NONE of the labels set............ I have distinct label group, but NOT distinct handling...
//so what is the pattern for that?????????  can't store all strings from 2xx to 400.....
				case OID_TOOL_MGMT:
handle_tool_mgmt:
					p_oid_tool_mgmt=(td_oid_tool_mgmt *)p_STPrxMsg->p_cObjectValue;
					c_op = p_oid_tool_mgmt->coperation;
					c_arg1 = p_oid_tool_mgmt->carg1;
logf("rxtm %d\r\n",c_op);
					if (c_op == register_tool_server)
					{
						//This is the current tool server
						//does not use any other arguments
						g_ToolServerSTPSession=p_STPSession;
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
					ui=ntohs(p_oid_tool_mgmt->uiarg2);
					s = ((char *)p_oid_tool_mgmt) + sizeof(td_oid_tool_mgmt); //save this location of the incoming bstring
					ilen=*s++; //read the length out of that bstring
					if (c_op == search || c_op == load_request)
					{
						//Message must be from client... handle by translating and forwarding to tool server
						//Is there a tool server register?
						if (g_ToolServerSTPSession==0)
						{
							logf("nts\r\n");
							break;
						}
						//Write MGMT out
						g_pSTPSession = (void *)g_ToolServerSTPSession; //limit to tool server
						goto send_tool_mgmt;
						break;
					}
					if (c_op == search_result || c_op == search_failure || c_op == load_failure)
					{
						//Message from server should be send back to interested clients.
	//FIXMENOWzxcv
	// For any tool clients interested in this result, forward the result back...
						g_pSTPSession = (void *)0; // no limit
send_tool_mgmt:
						SendToolMGMT(p_STPrxMsg->uiMsgType,c_op,c_arg1,ui,s,ilen);
						g_pSTPSession = p_STPSession; //restore limit
						break;
					}
unsupported_op_code:
					//unsupported op???
#ifdef OUTPUT_VERBOSE_TOOL_MGMT
					logf("TOOL_MGMT uop %d\r\n",c_op);
#endif
					break;
				case OID_TOOL_REC:
handle_tool_rec:
					p_LoadedTool=(td_oid_tool_rec *)p_STPrxMsg->p_cObjectValue;
					c_op=p_LoadedTool->cOperation;
logf("rxtr %d\r\n",c_op);
					if (c_op == search_result_rec)
					{
	//forward to clients interested in this result...
//FIXMENOWzxcv
	//check the flags set by the OID_TOOL_MGMT search_result
//FIXMENOWzxcv
//TOOL_REC	search-result-rec	result-index	<tool record fields> (see below)
//FIXMENOWzxcv

//FIXMENOWzxcv  temporarilly going to  send this to all people....  EVEN...

                        //Resend this entire message
						g_pSTPSession = (void *)0; // no limit
	                    ui=p_STPrxMsg->uiValueLength;
						SmartToolMsg(p_STPrxMsg->uiMsgType, OID_TOOL_REC, ui, ((char*)p_LoadedTool));
						g_pSTPSession = p_STPSession; //restore limit
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
						if (g_ToolServerSTPSession!=0)
						{
#ifdef OUTPUT_VERBOSE_TOOL_MGMT
logf("!pre un up\r\n",c_op);
#endif
							g_pSTPSession = g_ToolServerSTPSession;
							SendTool(STP_ALERT,update);
							g_pSTPSession = p_STPSession; //restore limit
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
proceed_with_load_tool:
					p_LoadedTool=(td_oid_tool_rec *)p_STPrxMsg->p_cObjectValue;
					g_LoadedTool.cOperation=p_LoadedTool->cOperation;
					g_LoadedTool.cToolStatus=p_LoadedTool->cToolStatus;
					g_LoadedTool.cHardstop=p_LoadedTool->cHardstop;
					g_LoadedTool.fDiameter=p_LoadedTool->fDiameter;
					g_LoadedTool.fLength=p_LoadedTool->fLength;
					g_LoadedTool.fMinBreak=p_LoadedTool->fMinBreak;
					g_LoadedTool.fLengthCountersink=p_LoadedTool->fLengthCountersink;
					g_LoadedTool.fCountersinkAdjust=p_LoadedTool->fCountersinkAdjust;
					g_LoadedTool.ulDTimeTicksMSW=ntohl(p_LoadedTool->ulDTimeTicksMSW);
					g_LoadedTool.ulDTimeTicksLSW=ntohl(p_LoadedTool->ulDTimeTicksLSW);
					g_LoadedTool.uiDCount=ntohs(p_LoadedTool->uiDCount);
					g_LoadedTool.uiDWarnCount=ntohs(p_LoadedTool->uiDWarnCount);
					g_LoadedTool.uiDLimitCount=ntohs(p_LoadedTool->uiDLimitCount);
					//bstring field szID
					//bstring field szToolTypeCode
					//Read BStrings after fixed size fields
					p_c = ((char *)p_LoadedTool) + sizeof(td_oid_tool_rec);
					ReadBString(p_c,g_szToolIDlen,g_szToolID,256);
					ReadBString(p_c,g_szToolTypeCodelen,g_szToolTypeCode,64);
					if (c_op == unload)
					{
						//ensure they did it right
						g_szToolID[0]=0;
						g_szToolIDlen=0;
						g_szToolTypeCode[0]=0;
						g_szToolTypeCodelen=0;
					}
logf("-+ %d \"%s\" %d \"%s\"\r\n", g_szToolIDlen, g_szToolID, g_szToolTypeCodelen, g_szToolTypeCode );
					if(g_szToolIDlen==0)
					{
						g_cLoadedTool=0;
                    	c_op = unload; //this is an unload, no matter what op came in
					}
					else
					{
						g_cLoadedTool = LookupToolTypeCode(g_szToolTypeCode);
					}
					//And Alert this tool status out
                    g_pSTPSession = (void *)0; //no limit
                	SendTool(STP_ALERT,c_op);
logf("!eop %d\r\n",c_op);
                    g_pSTPSession = p_STPSession; //restore limit
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
           			SmartToolMsgChar(STP_ALERT, OID_PROCESS, g_cLoadedProcess); //New Purpose of Alert is to just show what is loaded...
					break;
				//MakeOID-generated::END
				default:
					SmartToolMsgCommonMessageCode(uiOID,COMMONMC_NOSUCHOID);
                    #ifdef OUTPUT_RXSTP
					p_szOIDName = DisplayOIDName(uiOID);
                    goto show_unsupported_stp_message;
                    #endif
#endif
            }
            break;
        case STP_ALERT:
        	switch(uiOID)
            {
				//MakeOID-generated:: STP ALERT RX (oid merge)
				//Not Really auto generated... generally the MiniFT does not deal with alearts, but the tool system requires this.
				case OID_TOOL_MGMT:
					goto handle_tool_mgmt;
				case OID_TOOL_REC:
					goto handle_tool_rec;
				//MakeOID-generated::END
                default:
		            #ifdef OUTPUT_RXSTP
					p_szOIDName = "";
                    goto show_unsupported_stp_message;
		            #endif
            }
            break;
        //case STP_GET_RESP:
        //    switch(uiOID)
        //    {
        //        case OID_NULLOID:
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
    }//switch(uiMsgType)
    return 0;
	} //end MiniFT's Client Message
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
            	    case OID_NULLOID:
						SmartToolMsgEmpty(STP_GET_RESP, uiOID);
                    	break;
                }
                break;
	        case STP_ALERT:
    			switch(uiOID)
	            {
            	    case OID_NULLOID:
                    	//the server is doing it's keep alive
                    	break;
					case OID_SERIAL_NUMBER:
						ui=p_STPrxMsg->uiValueLength;
						if (ui>64 - 1)
						{
							ui=64 - 1;
						}
                    	//copy the value
						memcpy(g_szRailSerialNumber,p_STPrxMsg->p_cObjectValue,ui);
						g_szRailSerialNumber[ui]=0;
						logf("NAC is %s\r\n",g_szRailSerialNumber);
						g_pSTPSession = (void *)0; //no limit
						SmartToolMsgStr( STP_ALERT, OID_NAC_SERIAL_NUMBER , g_szRailSerialNumber );
						g_pSTPSession = p_STPSession; //restore limit
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
						SmartToolMsg(STP_ALERT, OID_CAM_OFFSET, sizeof(td_CamOffset),p_cSTPobjValBuff);
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
	    }//switch(uiMsgType)
		return 0;
	} //end NAC Message
#endif
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
            	    case OID_NULLOID:
						SmartToolMsgEmpty(STP_GET_RESP, uiOID);
                    	break;
                }
                break;
	        case STP_ALERT: //handle these together
        	case STP_GET_RESP:
    			switch(uiOID)
	            {
            	    case OID_NULLOID:
                    	//the server is doing it's keep alive
                    	break;
#ifdef DRILL_DIRECT_READY
					case OID_SMARTTOOL_TYPE:
					case OID_SYSTEM_VERSION:
						//ignore it...
						break;
					case OID_COMMON_MESSAGE_CODE:
	                    goto nac_show_unsupported_stp_message;

					case OID_GENERICMESSAGE:
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
logf("@#%s %lu\r\n","dse",(unsigned long)0);
								//Increment Counter
								ServiceDateTime();
								g_LoadedTool.ulDTimeTicksMSW=g_DateTime.ulticksMSW;
								g_LoadedTool.ulDTimeTicksLSW=g_DateTime.ulticksLSW;
								g_LoadedTool.uiDCount++;
								g_pSTPSession = (void *)0; // no limit
								SendTool(STP_ALERT,update);
								g_pSTPSession = p_STPSession; //restore limit
								//Jump to below and share the code to alert this.
								//For Legacy Purposes, we track Drill Start with OP_DRILL_STARTED
								//This adds a OP_DRILL_STARTED to the history.
								ui = OP_DRILL_STARTED;
								goto addophist; //beware: do not move label to different function than taget below.
							}
							else if (memcmp(p_c,"Shift",5)==0)
							{
logf("@#%s %lu\r\n","dsh",MS_TIMER - g_ulDrillEchoStart);
							}
							else if (memcmp(p_c,"MaxLyr",6)==0)
							{
logf("@#%s %lu\r\n","drt",MS_TIMER - g_ulDrillEchoStart);
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
					f =  ntohfc(*(float *)p_STPrxMsg->p_cObjectValue);
                    logf("Rx GL %f\r\n",f);
					g_cDrillSync |= DRILL_SYNC_BIT2;
					g_fGaugeLength=f;
					LoadToolHomeBackToDrill();
					break;
					case SMARTDRILL_OID_FAULT:
//Notice How Smart Drill has many more fields and we copy only the 3 we have in old structure td_DrillFault
//We don't care about the other fields at this time.
//typedef struct {
//	char cDevice;
//	char cSeverity;
//	long lFaultCode;
//} td_DrillFault;
//typedef struct {
//	char cDevice;
//	char cSeverity;
//	char cSeverityPrev;
//	char cReported;
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
					logf("dfa %d %d %ld\r\n",g_DrillFault.cDevice,g_DrillFault.cSeverity,g_DrillFault.lFaultCode);
					case SMARTDRILL_OID_HOLE_DATA:
					//we don't use all the fields that the drill has
					p_SmartDrillHoleData=(td_SmartDrillHoleData *)p_STPrxMsg->p_cObjectValue;
					g_HoleResultData.iHoleNumber=ntohs(p_SmartDrillHoleData->uiHoleNumber);
					g_HoleResultData.iHoleResult=ntohs(p_SmartDrillHoleData->iHoleResult);
                    logf("HR %d %d\r\n",g_HoleResultData.iHoleNumber,g_HoleResultData.iHoleResult);
					if(g_ulDrillEchoStart!=0)
					{
						logf("@#%s %lu\r\n","hr",MS_TIMER - g_ulDrillEchoStart);
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
							g_pSTPSession = (void *)0; //no limit
							AddOpHistory(g_PosnMode.iCurPosnIndex,ui);
							g_pSTPSession = p_STPSession; //restore limit
							logf("*aoh%u\r\n",ui);
							g_ulDrillEchoStart = 0; //clear this
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
            	    case OID_NULLOID:
						SmartToolMsgEmpty(STP_GET_RESP, uiOID);
                    	break;
                }
                break;
	        case STP_ALERT:
    			switch(uiOID)
	            {
            	    case OID_NULLOID:
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
	    }//switch(uiMsgType)
		return 0;
	} //End DrillFill Message
#endif
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
            	    case OID_NULLOID:
						SmartToolMsgEmpty(STP_GET_RESP, uiOID);
                    	break;
                }
                break;
	        case STP_ALERT: //handle these together
        	case STP_GET_RESP:
    			switch(uiOID)
	            {
            	    case OID_NULLOID:
                    	//the server is doing it's keep alive
                    	break;
					case OID_GENERICMESSAGE:
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
						logf("ffa %d %d %ld\r\n",g_FastenerFault.cDevice,g_FastenerFault.cSeverity,g_FastenerFault.lFaultCode);
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
	    }//switch(uiMsgType)
		return 0;
	} //end Fastener Message
#endif
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
            	    case OID_NULLOID:
						SmartToolMsgEmpty(STP_GET_RESP, uiOID);
                    	break;
                }
                break;
	        case STP_ALERT: //handle these together
        	case STP_GET_RESP:
    			switch(uiOID)
	            {
            	    case OID_NULLOID:
                    	//the server is doing it's keep alive
                    	break;
					case OID_GENERICMESSAGE:
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
	    }//switch(uiMsgType)
		return 0;
	} //end Fastener Message
#endif
	//unknown session ???
	//fall through and use standard unsupported stp messge
#ifdef OUTPUT_RXSTP
nac_show_unsupported_stp_message:
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
nac_show_unsupported_stp_message:
show_unsupported_stp_message:
#endif
}



////////////////////////////////////////////////////////////////////////////////
// User Block functions
////////////////////////////////////////////////////////////////////////////////
nodebug int UBwriteConfig(void)
{
	md5_state_t  md5state;
    char md5result[16];
	unsigned long ulHeader;
	int iRetVal;

	iRetVal = 0;

	// write a unique header (0x55AA55AB) to signify that actual data exists
	ulHeader = 0x55AA55AB;
    // write size of current

	// compute md5 against ONLY configdata (do not include header in md5 calc)
	md5_init(&md5state);
	md5_append(&md5state, (char*)&g_ConfigData, sizeof(td_ConfigData));
	md5_finish(&md5state, md5result);

	iRetVal += writeUserBlock(0,&ulHeader,4);
	iRetVal += writeUserBlock(4,g_szSerialNumber,64);

	iRetVal += writeUserBlock(68,&g_ConfigData,sizeof(td_ConfigData));
	iRetVal += writeUserBlock(68+sizeof(td_ConfigData),md5result,16);

	if (iRetVal != 0)
	{
	    return -1;
	}
	return 0;
}


nodebug int UBreadConfig(void)
{
	// read config data from user block on flash and store it in g_ConfigData
	// if the read fails, this function will NOT destroy what was previously in g_ConfigData
	md5_state_t  md5state;
    char md5storedInFlash[16];
    char md5ofData[16];
	unsigned long ulHeader;
	int iRetVal;
	td_ConfigData ConfigDataTemp;
	int i;

	// RetVal keeps track of errors...all fcns that produce an error will return <0...
	// just add them all up and if it's less than zero at the end, simply return -1
	iRetVal = 0;

	// check for the unique header (0x55AA55AB) to signify that actual data exists
	iRetVal += readUserBlock(&ulHeader,0,4);
    // load serial number
	iRetVal += readUserBlock(g_szSerialNumber,4,64);
	g_szSerialNumber[63]=0;

	if (ulHeader != 0x55AA55AB)
	{
    	g_szSerialNumber[0]=0;
		iRetVal--;
	}

	// read it into a temp area so that we can check the md5 before stomping the global g_ConfigData
	iRetVal += readUserBlock(&ConfigDataTemp,68,sizeof(td_ConfigData));
	iRetVal += readUserBlock(md5storedInFlash,68+sizeof(td_ConfigData),16);

	md5_init(&md5state);
	md5_append(&md5state, (char*)&ConfigDataTemp, sizeof(td_ConfigData));
	md5_finish(&md5state, md5ofData);

	for (i = 0 ; i < 16 ; i++)
	{
	    if (md5ofData[i] != md5storedInFlash[i])
	    {
	        iRetVal--;
	    }
	}

	//now make some model specific corrections
	#ifndef LINEAR_SCALE_OPTION
	g_ConfigData.cScaleMode=0; //force to zero so pulling config will be correct.  on a machine that doesn't support scalemode>0
	#endif

	if (iRetVal != 0)
	{
		#ifdef USE_OUTPUT
		logf("UB:Fail\r\n");
		#endif
        g_cConfigLoadSuccess = 0;
	    return -1;
	}
	else
	{
		#ifdef USE_OUTPUT
		logf("UB:Ldd\r\n");
		#endif
		g_ConfigData = ConfigDataTemp; //copy entire structure
        g_cConfigLoadSuccess = 1;
		return 0;
	}
}

////////////////////////////////////////////////////////////////////////////////
// Socket Console
////////////////////////////////////////////////////////////////////////////////
#ifdef SOCKETCONSOLE
nodebug void HandleSocketConsoleCommand(char * scmd, int icmdlen)
{
	char c;
    char lc;
	int i;
    char * sarg;
    char * sarg2;
    tcp_Socket * sock;
	char temptestbuffer[16];
	td_RFID_F0 * p_rfid_f0;

    if (_cConsoleSockCommandMode >= 1)
    {
#ifdef WHISTLEMC
    if (_cConsoleSockCommandMode < 4)
    {
       	//special direct
		// Console Command mode 1 goes to x whistle
		// Console Command mode 2 goes to y whistle
        if (icmdlen==1 && *scmd=='~')
		{
			_cConsoleSockCommandMode = 0;
			return;
        }
        //anything else goes right to
		//logf("sending \"%s\"\r\n",scmd);
		scmd[icmdlen++]='\r'; //OK to replace this character (the original end of command)
		c=scmd[icmdlen]; //record this character so we can restore this one.
		scmd[icmdlen]=0; //place temporary null
	    if (_cConsoleSockCommandMode == 1)
    	{
			sendWhistleXMsg(scmd,icmdlen);
		}
		else if (_cConsoleSockCommandMode == 2)
		{
			sendWhistleYMsg(scmd,icmdlen);
		}
#ifdef USE_RFID
		else if (_cConsoleSockCommandMode == 3)
		{
			//use for RFID
			serRFIDrdFlush();
			serRFIDwrFlush();
			serRFIDputs(scmd);
			logf("\r\nsent.\r\n");
		}
#endif
		scmd[icmdlen]=c; //restore this character
        return;
    }
#endif
#ifdef CENTERVISION_CAM
    if (_cConsoleSockCommandMode == 4)
    {
        if (icmdlen==1 && *scmd=='~')
        {
          	_cConsoleSockCommandMode=0;
            return;
        }
       	//send it all to camera
        SendCVCamFromConsole( scmd, icmdlen);
        return;
    }
#endif
	}
    sarg=(char *)0;
    sarg2=(char *)0;
    i=0;
    lc = 0;
    while(i<icmdlen)
    {
    	c=scmd[i];
    	if (c==' ')
        {
        	scmd[i]=0;
        }
        else if (lc==' ')
        {
        	//last was space, but this is not
            if (sarg==0)
            {
	            sarg=scmd+i;
            }
            else if (sarg2==0)
            {
            	sarg2=scmd+i;
            }
        }
        lc=c;
        i++;
    }
    if (strcmp(scmd, "update") == 0)
    {
       	g_cEthernetDownloaderEnabled = 1;
    	logf("update enabled\r\n");
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
#ifdef WHISTLEMC
		SetMotorActionDirectly(); //avoid brake timeout
#endif
	}
	else if (strcmp(scmd, "brake!") == 0) //Caution... Will Activate Brake without regard for motion underway.
	{
		BrakeOn();
#ifdef WHISTLEMC
		SetMotorActionDirectly(); //avoid brake timeout
#endif
	}
	else if (strcmp(scmd, "out") == 0)
	{
		if (sarg!=0)
		{
			i = atoi(sarg);
			c = 2;
			if (sarg2!=0)
			{
				c = atoi(sarg2);
			}
			if (i<16 & c<2)
			{
				digOut(i, c);
				logf("out%d %d\r\n",i,c);
			}
			else
			{
				logf("invalid args\r\n");
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
#ifdef WHISTLEMC
    else if (strcmp(scmd, "wx") == 0) //Whistle Clamp Axis Console Mode
  	{
  		//mc direct
		_cConsoleSockCommandMode = 1;
		i=3; //default is rx and tx on
		if (sarg!=0) { i = atoi(sarg); }
		g_cWhistleXCommDisplay = i;
  	}
    else if (strcmp(scmd, "wy") == 0) //Whistle Clamp Axis Console Mode
  	{
  		//mc direct
		_cConsoleSockCommandMode = 2;
		i=3; //default is rx and tx on
		if (sarg!=0) { i = atoi(sarg); }
		g_cWhistleYCommDisplay = i;
  	}
    else if (strcmp(scmd, "wq") == 0) //Whistle Quiet!
  	{
		g_cWhistleXCommDisplay = 0;
		g_cWhistleYCommDisplay = 0;
  	}
#endif
#ifdef USE_RFID
    else if (strcmp(scmd, "rfidcom") == 0) //Whistle Clamp Axis Console Mode
  	{
  		//rfid direct
		_cConsoleSockCommandMode = 3;
		logf("%s:\r\n","rfidcom");
	}
#endif
    else if (strcmp(scmd, "cog") == 0) //Whistle Clamp Axis Console Mode
  	{
  		//mc direct
		_cConsoleSockCommandMode = 4;
	}
#ifdef GALIL
	//no support
#else
	else if (strcmp(scmd, "can") == 0)
	{
	    ShowCAN();
    }
#endif
    else if (strcmp(scmd, "ocm") == 0)
    {
		if (sarg!=0)
		{
			g_ConfigData.cObstructionCodeMask=atoi(sarg);
        }
        logf("ocm=%x\r\n",g_ConfigData.cObstructionCodeMask);
    }
#ifdef FORCELIMITING
	else if (strcmp(scmd, "sf") == 0)
	{
		g_cShowForce=!g_cShowForce;
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
		if (sarg!=0)
		{
			g_cSafeUnclamp=atoi(sarg);
			if (g_cSafeUnclamp == 1)
			{
				g_cClampGoal=CLAMP_UNCLAMP;
			}
		}
    }
#endif
    else if (strcmp(scmd, "testopt") == 0)
    {
		if (sarg!=0)
		{
			g_cTestOpt = atoi(sarg);
		}
	}
#ifdef HOMESYSTEM
#ifdef HOMESYSTEM_X_RFID
//RFID TESTING CODE ///////////////////////////////////////////////////
	else if (strcmp(scmd, "rr") == 0)
	{
		//read RFID now
		g_cReadRFID=RFID_READ_NOW;
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
		memset(temptestbuffer,0,16);
		temptestbuffer[1]=1;
		temptestbuffer[2]=2;
		temptestbuffer[3]=3;
		temptestbuffer[4]=4;
		temptestbuffer[5]=5;
		temptestbuffer[6]=6;
		SL031WriteData(temptestbuffer,16);
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
		if (sarg!=0)
		{
			g_cEEOption=atoi(sarg);
		}
		logf("ee=%d\r\n",g_cEEOption);
		if (sarg!=0)
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
		if (g_cHold==1)
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
		StopPosition();
	}
	else if (strcmp(scmd, "nmt") == 0)
	{
		i=0;
		while(i<=OID_MAX_NUMBER_MiniFT)
		{
			logf("%d %s\r\n",i,DisplayOIDName(i));
			i++;
		}
	}
	else if (strcmp(scmd, "vk") == 0)
	{
		g_cAllowKVisit=1;
	}
	else if (strcmp(scmd, "tm") == 0)
	{
		ServiceDateTime();
logf("date %d %d %d %d %d %d %d\r\n",
	g_DateTime.uiyear,g_DateTime.cmonth,g_DateTime.cdayOfMonth,
	g_DateTime.chour,g_DateTime.cminute,g_DateTime.csecond,g_DateTime.uimillisecond);
	}
	else if (strcmp(scmd, "rfr") == 0)
	{
		CloseFandFT();
	}
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
		if (sarg!=0)
		{
			g_ConfigData.uiProcessOperations = atoi(sarg);
		}
		logf("op %x %d\r\n",g_ConfigData.uiProcessOperations,g_ConfigData.uiProcessOperations);
		if (sarg!=0)
		{
			UpdateStationPlan();
		}
	}
    else if (strcmp(scmd, "nl") == 0)
    {
		if (sarg!=0)
		{
			g_cLubeBypass = atoi(sarg);
		}
		logf("%s=%d\r\n","nl",g_cLubeBypass);
	}
    else if (strcmp(scmd, "jrl") == 0)
    {
		if (sarg!=0)
		{
			g_uiJogRepeatLimit = atoi(sarg);
		}
		logf("%s=%d\r\n","jrl",g_uiJogRepeatLimit);
	}
	else if (strcmp(scmd, "rfid") == 0)
	{
		if (sarg!=0)
		{
			if (*sarg=='!')
			{
				g_cImmediateFakeRfid=1;
				goto skip_rest_of_rfid_cmd;
			}
			else if (*sarg=='~')
			{
				sarg="001T030000072000xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
				g_szTagDatalen=RFID_FMS00_DATA_SIZE; //just use this right size for this tag
			}
			else if (*sarg=='b')
			{
				sarg="001B090000072000xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
				g_szTagDatalen=RFID_FMS00_DATA_SIZE; //just use this right size for this tag
			}
			else if (*sarg=='n')
			{
				p_rfid_f0 = (td_RFID_F0 *)g_szTagData;
				p_rfid_f0->cFormat=0;
				p_rfid_f0->cRailType=0;
				p_rfid_f0->cGroup=0;
				p_rfid_f0->cSegmentAndRailSide=128;
				p_rfid_f0->ulSerialNumber=htonul(1234321);
				p_rfid_f0->ulPosition=htonul((unsigned long)1000*50);
				p_rfid_f0->ulSegmentPosition=htonul((unsigned long)50000); //1000*50
				g_szTagDatalen=RFID_F0_DATA_SIZE;
				goto skip_rest_of_rfid_cmd;
			}
			else
			{
				g_szTagDatalen = strlen(sarg);
			}
			if (g_szTagDatalen>64) { g_szTagDatalen=64; }
			memcpy(g_szTagData,sarg,g_szTagDatalen);
			g_szTagData[g_szTagDatalen]=0;
		}
		else
		{
			g_szTagData[64]=0; //null the end just to be sure
		}
		logf("%s\r\n","rfid");
		logf("%s\r\n",g_szTagData);
skip_rest_of_rfid_cmd:
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
		if (sarg!=0)
		{
			if (*sarg=='1')
			{
                g_cUseFastenerTray = 1;
			}
			else
			{
				g_cUseFastenerTray = 0;
			}
		}
		logf("%s %d\r\n","ftray",g_cUseFastenerTray);
	}
	else
	{
unmatched_command:
	   	logf("echo:\"%s\"\r\n",scmd);
	}
	return;
}

nodebug void SocketConsoleShowIdentity()
{
	logf("%s %s sn %s\r\n", SMARTTOOL_SUBTYPE_STRING, SYSTEM_VERSION_STRING, g_szSerialNumber );
	//logf("Welcome to the system.\r\n");
}
#endif

//for report back hack
nodebug void SendInspectResults();

////////////////////////////////////////////////////////////////////////////////
// Mode State Machine
////////////////////////////////////////////////////////////////////////////////
//FIXME0 nodebug
nodebug void ModeStateMachine(void)
{
    int i;
    unsigned int ui;
    char cEntryState;
	char cprobestatus;
	char cresult;
    char ck,ck1,ck2;
	char cindex;
	char corient;
    static char s_cEstopPrev;  // monitors prior estop state
    static unsigned long s_ulEstopTime;
#ifdef ESTOPV1B
    static char s_cEstopUnibuttonEvent;
#endif
    static char s_cModeStatePrev;
    static unsigned long s_ulStartTime;
    static char s_cProbeState;    // for MODE_PROBE
    static char s_cProbeHomeState;  // for MODE_PROBE_HOME
    static char s_cProbeAdjustState;  // for MODE_PROBE_ADJUST
    static char s_cTeachState;  // for MODE_TEACH
    static char s_cInspectState;  // for MODE_INSPECT

    float fPreMoveX, fPreMoveY;  // for MODE_POSN
	static float s_fTargetX, s_fTargetY;  // for MODE_POSN
	static float s_fErrLev,s_fErrLevFinal;
	static char s_cMoveType,s_cOneMove,s_cMoveOpt,s_cMoveOptFinal;
	static char s_cProbeHomeDir;
    float fDistance,fError;
    float f,fx,fy,fdx,fdy;
	float fx1,fy1,fx2,fy2;
    unsigned long ulx;
	long lx;
	char * p_c;
	char cdm;
	td_oid_probe_status * p_oid_probe_status;
	int iypos;
	unsigned int uiop;
	char * pc1;
	char * pc2;
	float fddx,fddy;
    // GLOBAL_INIT: this is how Dynamic C initializes function statics... must be after declarations and before code
    #GLOBAL_INIT
    {
        s_cEstopPrev = 127; //not true or false
        s_ulEstopTime = 0;
#ifdef ESTOPV1B
        s_cEstopUnibuttonEvent = 1;
#endif
        s_cModeStatePrev = MODE_IDLE;
        s_cProbeHomeDir=3;

		s_cMoveType = MOVETYPE_FAST;
    }

	cEntryState = g_cModeState;  // capture the entry state in case g_cModeState changes within this state machine

	g_cPrintAuth++; //Simple Trick to allow occasional printing for some debugging code
	if (g_cPrintAuth==80) { g_cPrintAuth=0; }

   	//Check For Estop
#ifdef ESTOPV1B
   	if (g_cDigInEstopSwitch == 0)
	{
		//switch is still in
       	g_cUniButtonEvent=0; // clear event
		s_cEstopUnibuttonEvent=0; // clear event
		goto label_common_estop_code; //using this goto eliminates 1 variable, 1 variable init, and 1 if statement
	}
	if (s_cEstopUnibuttonEvent == 0)
	{
		//red button has not been pressed since the switch was cleared
        if (g_cUniButtonEvent == 1)
        {
        	//Saw the red button go in, so allow above logic to clear estop.
        	s_cEstopUnibuttonEvent=1;
        	g_cUniButtonEvent=0;
        }
		goto label_common_estop_code;
	}
   	if (g_cDigInEstop == 0 || g_cEstopMCAsserted == 1) //no power, or estop asserted by MC
#else
	if (g_cDigInEstopSwitch == 1 || g_cEstopMCAsserted == 1) //estop signalled, or estop asserted by MC
#endif
   	{
		//Estop is signalled
label_common_estop_code:
		s_ulEstopTime=MS_TIMER;
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
			#ifdef READMOTORVOLTAGE
			if (g_fMotorVoltage<16.0)
			{
	        	logf(" last motor voltage = %f\r\n",g_fMotorVoltage);
	        }
	        #endif
	        #endif
			s_cEstopPrev = TRUE;
            g_cEstopPrevMode = g_cModeState; //Save this mode to allow possible restore later.
    	    //clear the sign of the estop unibutton event... we'll need to see it to leave estop
#ifdef ESTOPV1B
	       	g_cUniButtonEvent=0;
	        s_cEstopUnibuttonEvent=0;
#endif
        }
		s_cEstopPrev = TRUE;
        g_cModeState = MODE_ESTOP;
        goto label_past_estop;
   	}
    //Estop is not signalled
    if (s_cEstopPrev != FALSE)
    {
    	//Estop was previously signalled: can we exit?
		if ((MS_TIMER - s_ulEstopTime)<g_ConfigData.uiEStopClearDelay)
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
		if (g_cEstopPrevMode == MODE_POSN ||
			g_cEstopPrevMode == MODE_PROBE_ADJUST)
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
        SmartToolMsgChar(STP_ALERT, OID_MODE, g_cModeState);

		//clear any tool offset, and let the new mode set if needed
		//FIXME check   is this really needed? is it wise?
		ClearToolOffset();
	}
	if (g_cAction != g_cActionSent)
	{
		SmartToolMsgChar(STP_ALERT, OID_ACTION, g_cAction);
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
    if (g_cObstructionEvent!=0)
    {
		AlertObstructionCode();
    	g_cObstructionEvent=0;
    	if (g_cObstructionCode!=0)
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
			else if (g_cModeState ==  MODE_PROBE_HOME)
			{
				//Probe Home must move around even if obstruction sensors are activated.
				//Probe Motions should have their own response to obstruction codes    //FIXME  This might not be wise.. might be better to let this do move error
			}
			else if (g_cModeState == MODE_POSN && g_cMoveDone == MOVEDONE_FALSE)
			{
				//Moving Now
				if (g_cObstructionCodeNew>0)
				{
					logf("oss\r\n");
					//new bits set since we last cleared it
					//Should Stop Position
                    StopPosition();
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
	if (g_cObstructionWarningEvent!=0)
	{
		AlertObstructionWarningCode();
    	g_cObstructionWarningEvent=0;
	}


	if (g_cGravCompStatus == GRAVCOMP_RUNNING)
	{
    	ulx = MS_TIMER - g_ulGravCompTime;
        if (ulx > 8000)
        {
        	//timeout
			SmartToolMsgMiniFtMessageCode(OID_MCERR, MINIFTMC_GRAVCOMP_FAILURE );
            g_cGravCompStatus = GRAVCOMP_FAILX;
            StopPosition(); //??? Is there something better to do?
			g_cFloatStatus = FLOATSTAT_NOFLOAT;
        	SmartToolMsgChar(STP_ALERT, OID_GRAVCOMP_STATUS, g_cGravCompStatus);
			SmartToolMsgChar(STP_ALERT, OID_FLOAT_STATUS, g_cFloatStatus);
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
				LEDOff();
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
			if (g_cBeepMode!=BEEPOFF && g_cBeepMode<BEEPSIGNALS) { BeepOff(); }
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
				g_cUniButtonEvent=0; //clear event
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
				if (g_cPartPgmStatus!=PP_LOADOK)
                {
                	//shouldn't be here in probe
					SmartToolMsgMiniFtMessageCode(0, MINIFTMC_NO_PROGRAM);
                    g_cModeState=MODE_IDLE;
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
			if (g_cBeepMode!=BEEPPROBEK && g_cBeepMode<BEEPSIGNALS) { BeepProbeK(); }
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
	              	if (g_cFloatStatus != FLOATSTAT_FLOAT && g_cGravCompStatus == GRAVCOMP_PASS)
 		            {
						DoFloat(FLOAT_FLOAT); //always makes it unclamp anyway.
					}
                    }
            		LEDProbeK();
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
					if (s_cModeStatePrev!=MODE_ESTOP)//don't auto unclamp after estop, even in probe mode
					{
//zxcvzxcv111
           	        	g_cClampGoal=CLAMP_UNCLAMP; //PREVIOUSLY LOOSE
						g_cClampGoalSent = 0xFF; //force it to send goal again
					}
                    #endif

                    g_cKHolePrbeTrigger=0; //clear trigger
                    g_cProbeFlag = 0; //reset flag which indicates if we recevied coords
					g_Probe.cRegistration = REGISTRATION_UNKNOWN;
                    SmartToolMsgChar(STP_ALERT, OID_PROBE_REGISTRATION, g_Probe.cRegistration);
					#ifdef CENTERVISION
                    g_cCenterVisionResult = CENTERVISION_OFF;
					g_cCenterVisionInspectType=INSPECT_PROBE;
					g_cCenterVisionRequiredResults=CENTERVISION_CENTER; //default
					//g_fCenterVisionExpectedDiameter set inside system
					g_VisionInspectResults.cContext=1;//Probe
					g_VisionInspectResults.lposn=-1;
					g_VisionInspectResults.fXPositionExpected=0;
					g_VisionInspectResults.fYPositionExpected=0;
                    #endif

					if (g_cProbeMethod == PROBE_LASER)
					{
						SetToolOffset(g_ConfigData.LaserSensorOffset.fX,g_ConfigData.LaserSensorOffset.fY);
                        //FIXME0 ensure that probe after will work with the set tool offset....
                        //FIXME0 ensure that the offset will remain set until that capture
					}
                    else if (g_cProbeMethod == PROBE_CAM)
                    {
						SetToolOffset(g_ConfigData.CamOffset.fX,g_ConfigData.CamOffset.fY);
                    }
					else if (g_cProbeMethod == PROBE_INSTANT_OFFSET)
					{
                        //g_ConfigData.cToolFlip=Y_POS; //FIXME0000000 this was needed, but for safety and space I removed it again
						SetToolOffset(g_ConfigData.ProbeOffset.fX,g_ConfigData.ProbeOffset.fY);
					}
                    else // PROBE_MANUAL PROBE_INSTANT
                    {
                    	ClearToolOffset();
                    }
                    #ifdef USE_HYSTERESIS_FROM_CENTERVISION
                    g_fCenterVisionHysX = 0; //clear it
                    g_fCenterVisionHysY = 0; //clear it
                    SetDriveThroughBacklash(0,0,0);
					#endif
					#ifdef CENTERVISION
					//Store these with Center Fision
					g_fCenterVisionOffsetX=g_MachineOffset.fX;
					g_fCenterVisionOffsetY=g_MachineOffset.fY;
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
	                    g_cKHolePrbeTrigger=1; //trigger
						g_cUniButtonEvent=0; //clear event
    	            }
					#endif

                    if (g_cKHolePrbeTrigger==0)
                    {
                    	//still waiting
						#ifdef CENTERVISION
						g_VisionInspectResults.cContext=0;//Preview
						PreviewCenterVision();
						#endif
						break;
                    }
                    g_cKHolePrbeTrigger=0; //clear trigger

					#ifdef HOMESYSTEM
					if (g_cHomed == HOME_RUNNING)
					{
						SmartToolMsgMiniFtMessageCode(OID_PROBE, MINIFTMC_PROBE_PREVENTED_HOME);
						break;
		            }
					#endif
					if (g_cDrillState != DRILLSTATE_IDLE)
					{
						SmartToolMsgMiniFtMessageCode(OID_PROBE, MINIFTMC_PROBE_PREVENTED_DRILL);
						break;
					}
					#ifdef CLAMP_SYSTEM
					if (g_cClampState!=g_cClampGoal || g_cClampState!=CLAMP_UNCLAMP)
					{
						SmartToolMsgMiniFtMessageCode(OID_PROBE, MINIFTMC_PROBE_PREVENTED_CLAMP);
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
						SetToolOffset(g_ConfigData.LaserSensorOffset.fX,g_ConfigData.LaserSensorOffset.fY);
                        //FIXME0 ensure that probe after will work with the set tool offset....
                        //FIXME0 ensure that the offset will remain set until that capture
					}
                    else if (g_cProbeMethod == PROBE_CAM)
                    {
						SetToolOffset(g_ConfigData.CamOffset.fX,g_ConfigData.CamOffset.fY);
                    }
					else if (g_cProbeMethod == PROBE_INSTANT_OFFSET)
					{
                        //g_ConfigData.cToolFlip=Y_POS; //FIXME0000000 see note above
						SetToolOffset(g_ConfigData.ProbeOffset.fX,g_ConfigData.ProbeOffset.fY);
					}
                    else // PROBE_MANUAL PROBE_INSTANT
                    {
                    	ClearToolOffset();
                    }
					#ifdef CENTERVISION
					//Store these with Center Fision
					g_fCenterVisionOffsetX=g_MachineOffset.fX;
					g_fCenterVisionOffsetY=g_MachineOffset.fY;
					#endif

                    //Now Start
                    //Copy Input Value
                    g_cKHolePrbeCommand = g_cKHolePrbeCommandInput;
                    g_cKHolePrbeIndex = g_cKHolePrbeIndexInput;
                    //Check Value
                    if (g_cKHolePrbeIndex==0 || g_cKHolePrbeIndex>g_cKHoleCount)
                    {
                    	//bad value
                        //Do not allow probe
						SmartToolMsgMiniFtMessageCode(0, MINIFTMC_INVALID_KI);
                       	logf("Err Khole=%d Count=%d\r\n",g_cKHolePrbeIndex,g_cKHoleCount);
                        break;
                    }

					logf("Probe Trig cmd=%d KI=%d\r\n",g_cKHolePrbeCommand,g_cKHolePrbeIndex);

					//Load the details for this Locating feature
					LoadProbeCommand(g_cKHolePrbeIndex);
					if (g_cProbeCommand==PROBE_COMMAND_INVALID)
					{
						SmartToolMsgMiniFtMessageCode(OID_PROBE, g_cProbeCommandMessage);
						break;
					}
					g_cCenterVisionRequiredResults=CENTERVISION_CENTER; //default standard for centervision, and only supported option for some right now
					if (g_cProbeCommand==TKP_PROBE_EDGE_MVEC || g_cProbeCommand==TKP_PROBE_EDGE_VEC)
					{
						g_cCenterVisionRequiredResults=CENTERVISION_EDGE; //all that it does
						if (g_cProbeMethod != PROBE_CAM && g_cProbeMethod != PROBE_INSTANT)
						{
							logf("Only CAM for Edge."); //Currently only the Camera Supports Edge Probes
							break;
						}
logf("%s %d\r\n","chk",g_cKHolePrbeStart[g_cKHolePrbeIndex]);
	                    if (g_cKHolePrbeStart[g_cKHolePrbeIndex]==PS_NO_PROBE)
                        {
                        	//error
logf("nos\r\n");
							s_cProbeState = PROBE_WAIT;
                            break;
                        }
					}

					#ifdef CENTERVISION
                    g_cCenterVisionResult = CENTERVISION_OFF;
					g_cCenterVisionInspectType=INSPECT_PROBE;
					//set above g_cCenterVisionRequiredResults=CENTERVISION_CENTER;
					//g_fCenterVisionExpectedDiameter set inside system
					g_VisionInspectResults.cContext=1;//Probe
					g_VisionInspectResults.lposn=g_cKHolePrbeIndex; //store this here for the Probe Result
					g_VisionInspectResults.fXPositionExpected=0;
					g_VisionInspectResults.fYPositionExpected=0;
                    #endif

                    //Handle Command
                	if (g_cKHolePrbeCommand==PC_PROBE)
                    {
						if (g_cProbeCommand==0)
						{
							//This does not actually require a probe, but is a defined location only....
							//Since this is now done, just break...
//FIXMENOW this path not done
							break;
						}
						//OID_PROBE PC_PROBE Kindex  Kick Off the selected probe algorithm to
						//probe for this Kindex at the current location
						//Set Status and Alert now
						g_cKHolePrbeStatus[g_cKHolePrbeIndex]=PS_PROBING;
						//Alert This Probe Status
						SendProbeStatus(STP_ALERT, g_cKHolePrbeIndex); //SPS
						//AlertProbeStatusUpdate(); //Don't bother: start_probe_probe_state will soon do this below
						s_cProbeState = PROBE_PROBE;
#ifdef OUTPUT_PROBE_SYS_VERBOSE
logf("P_P_0\r\n");
#endif
						goto start_probe_probe_state; //to clear status and do other probe probe init
						break;
                    }
                	if (g_cKHolePrbeCommand==PC_MOVE)
                    {
						//OID_PROBE PC_MOVE Kindex  Move To this K Hole Location, but do not probe.
						//Only K Holes with at least an extrapolated position can be be moved to.
						GetPositionCounts(); //ensure that move will start with good current position
						s_cProbeState = PROBE_MOVE;
#ifdef OUTPUT_PROBE_SYS_VERBOSE
logf("P_M_0\r\n");
#endif
                        break;
                    }
                	if (g_cKHolePrbeCommand==PC_MOVE_PROBE)
                    {
						//OID_PROBE PC_MOVE_PROBE Kindex  Move To this K Hole Location,
                        // and upon arrival do the probe.
						GetPositionCounts(); //ensure that move will start with good current position
						s_cProbeState = PROBE_MOVE;
#ifdef OUTPUT_PROBE_SYS_VERBOSE
logf("P_M_1\r\n");
#endif
                        break;
                    }
                	if (g_cKHolePrbeCommand==PC_MOVE_PROBE_ALL)
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

						GetPositionCounts(); //ensure that move will start with good current position
						s_cProbeState = PROBE_MOVE;
                        break;
                    }
                    if (g_cKHolePrbeCommand==PC_COMPLETE)
                    {
						s_cProbeState = PROBE_COMPLETE;
#ifdef OUTPUT_PROBE_SYS_VERBOSE
logf("P_C_1\r\n");
#endif
                        break;
                    }
					if (g_cKHolePrbeCommand==PC_REAPPLY)
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
                	if (g_cKHolePrbeCommand==PC_MOVE_PROBE_ALL)
                    {
                    	//allowed to search through
                        if (g_cKHolePrbeIndex==0)
                        {
                        	g_cKHolePrbeIndex=1; //start here, not at 0
                        }
                        if (g_cKHolePrbeIndex>g_cKHoleCount)
                        {
                        	//nothing more to do...
							s_cProbeState = PROBE_WAIT;
//FIXME1 how will we know it's done this kind????
// how will we know it's done any kind?????
							break;
                        }
	                    if (g_cKHolePrbeStart[g_cKHolePrbeIndex]==PS_NO_PROBE)
                        {
							g_cKHolePrbeIndex++;
                            break;
                        }
                    }
                    else
                    {
    	                if (g_cKHolePrbeIndex==0 || g_cKHolePrbeIndex>g_cKHoleCount)
	                    {
                        	//error FIXME1
                        	//nothing more to do...
							s_cProbeState = PROBE_WAIT;
                            break;
                        }
//ProbeStart Test
	                    if (g_cKHolePrbeStart[g_cKHolePrbeIndex]==PS_NO_PROBE)
                        {
                        	//error
logf("nos\r\n");
							s_cProbeState = PROBE_WAIT;
                            break;
                        }
//FIXME2 since we don't assume orientation anymore, we need option for that feature to work with this.
//How should it work?
                    }

					if (g_cObstructionCode!=0)
					{
						AlertObstructionCode();
						g_cKHolePrbeCommand=PC_STOP;
					}
                	if (g_cKHolePrbeCommand==PC_STOP)
                    {
						//OID_PROBE PC_STOP na 	Stop Current Probe Move or Probe.
                        //Nothing to Stop Yet
logf("*P_O_1\r\n");
						//Load status variable before jumping
						cprobestatus=g_cKHolePrbeStatus[g_cKHolePrbeIndex];
						goto probe_over;
                    }
                    //Setup Move

					if (g_cMoveDone != MOVEDONE_TRUE)
					{
						//Stop existing move
						StopPosition();
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
					fx=g_fKHolePrbeStartX[g_cKHolePrbeIndex];
					fy=g_fKHolePrbeStartY[g_cKHolePrbeIndex];

                	if (g_cKHolePrbeCommand==PC_MOVE)
                    {
                    	//just a move... do't set status
                    }
                   	else
                    {
                    	//move and probe, so set status now
						g_cKHolePrbeStatus[g_cKHolePrbeIndex]=PS_PROBING;
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
					g_VisionInspectResults.fXPositionExpected=fx;
					g_VisionInspectResults.fYPositionExpected=fy;
                    #endif

					logf("probe mv K%d: x=%f,y=%f\r\n",g_cKHolePrbeIndex,fx,fy);//debug
//FIXME2 only true if doing laser ???
					//Need Offset to apply this because we want to move the laser or cam to this position...
					logf("At mv tm MaOffst=%f,%f\r\n",
	                    g_MachineOffset.fX, g_MachineOffset.fY);
					ApplyMachineOffset(&fx,&fy);
					#ifdef CENTERVISION
					//Store these with Center Fision
					g_fCenterVisionOffsetX=g_MachineOffset.fX;
					g_fCenterVisionOffsetY=g_MachineOffset.fY;
					#endif
					//Now apply any additional extra offset if Camera Probing
					if (g_cProbeMethod == PROBE_CAM)
					{
						//Add any specified probe extra offset for the purpose of avoiding clamps or moves to bad locations
						//The Camera might be able to see further, and the center result will be adjusted to counteract this.
						if (g_cProbeExtraOffsetGiven!=0)
						{
							fx1 = g_fProbeExtraOffsetX;
							fy1 = g_fProbeExtraOffsetY;
							logf("%s %f,%f\r\n","eo",g_fProbeExtraOffsetX,g_fProbeExtraOffsetY);
							cresult=RotateVecDataSetToMachine(g_iKHoleHoleIndex[g_cKHolePrbeIndex], &fx1, &fy1);
							if (cresult==0)
							{
								//Hole either does not have positioning primary and secondary K locations, or doesn't have approx values for them yet.
								//This is currently required to allow offset probing
								logf("neo\r\n");
								cprobestatus=PS_NO_PROBE;
								goto probe_over;
							}
							fx += fx1;
							fy += fy1;
							logf("%f,%f\r\n",fx1,fy1);
						}
						if (g_cProbeExtraMachineOffsetGiven!=0)
						{
//FIXMENOW Confirm direction of application makes sense
							fx += g_fProbeExtraMachineOffsetX;
							fy += g_fProbeExtraMachineOffsetY;
							logf("%s %f,%f\r\n","emo",g_fProbeExtraMachineOffsetX,g_fProbeExtraMachineOffsetY);
						}
                    }
					logf("probe mv K%d: x=%f,y=%f\r\n",g_cKHolePrbeIndex,fx,fy);//debug

					//See if this position is allowable
                    cresult=CheckObstructionsAndMotionLimits(fx,fy);
                    if (cresult!=0)
                    {
                    	//Found a reason not to allow this move
						cprobestatus=PS_NO_PROBE;
						goto probe_over;
                    	break;
                    }

                    fdx=fabs(fx - g_PosnMode.fLastKnownPosnX);
                    fdy=fabs(fy - g_PosnMode.fLastKnownPosnY);
logf("LastKnown=%f,%f\r\n",g_PosnMode.fLastKnownPosnX,g_PosnMode.fLastKnownPosnY);
if (fdx<2) { fdx=2; } //Give it some speed since we have that issue...
if (fdy<2) { fdy=2; }

               		SetMoveSpeeds(1,0,fdx,fdy);
					SetMoveParams(0.001,1);

    				RunPosition(fx, fy);
                    s_cProbeState = PROBE_MOVE_WAIT;
#ifdef OUTPUT_PROBE_SYS_VERBOSE
logf("P_MW_1\r\n");
#endif
                	break;
                case PROBE_MOVE_WAIT:
					if (g_cObstructionCode!=0)
					{
						AlertObstructionCode();
						g_cKHolePrbeCommand=PC_STOP;
					}
                	if (g_cKHolePrbeCommand==PC_STOP)
                    {
logf("*P_S_3\r\n");
						//OID_PROBE PC_STOP na 	Stop Current Probe Move or Probe.
                        //Stop!!!!!
						//If Running, then stop...
						StopPosition();
//FIXME3 do I need more checking? test using delay to see if stop after stop is a prob
						logf("StopPos\r\n");
						//Load status variable before jumping
						cprobestatus=g_cKHolePrbeStatus[g_cKHolePrbeIndex];
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
							StopPosition();
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
                	if (g_cKHolePrbeCommand==PC_MOVE)
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
					ClearPositionsDuringProbe(g_cKHolePrbeIndex);//SPS
					AlertProbeStatusUpdate();
					//Begin Probe
					s_cProbeState = PROBE_PROBE;
#ifdef OUTPUT_PROBE_SYS_VERBOSE
logf("P_P_3\r\n");
#endif
                	break;
                case PROBE_PROBE:
					g_cAction = ACTION_PROBE;
					if (g_cProbeCommand==0)
					{
						//This does not actually require a probe, but is a defined location only....
						//Since this is now done, just continue marking that this is done, and moving on.
//FIXMENOW FIXME PROBE....  must complete this case later.....
						break;
					}
					#ifdef CENTERVISION
					if (g_cProbeMethod == PROBE_LASER || g_cProbeMethod == PROBE_CAM) //we never return since we don't clamp && g_cCenterVisionResult != CENTERVISION_SUCCESS)
					{
	                	if (g_cKHolePrbeCommand==PC_STOP)
    	                {
							//OID_PROBE PC_STOP na 	Stop Current Probe Move or Probe.
							if (g_cCenterVisionResult == CENTERVISION_OFF)
							{
								//hasn't really started yet... just cancel right now
								//Load status variable before jumping
								cprobestatus=g_cKHolePrbeStatus[g_cKHolePrbeIndex];
								goto probe_over;
							}
							//The Soft Stop of Center Vision:
							//1st call will put this into CV_RETURN type move as-if it failed.
							//2nd call
							CancelCenterVision();
							g_cKHolePrbeCommand=PC_PROBE; //don't call here again unless they hit stop again
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
							fx=g_fKHolePrbeStartX[g_cKHolePrbeIndex];
							fy=g_fKHolePrbeStartY[g_cKHolePrbeIndex];
                            g_fKHolePrbeX[g_cKHolePrbeIndex]=fx;
                            g_fKHolePrbeY[g_cKHolePrbeIndex]=fy;
							cprobestatus=PS_PROBED_FAILED; //set fail code
							goto probe_failed_verify; //really just probe failed here
						}
						//Success
//FIXMENOW need to check actual prbe result types...
						g_cProbeFlag = X_AND_Y;
						if (g_cProbeCommand==KEYWORD_Probe)
						{
							//Standard Probe
	                        //Place position directly into probe setting
    	                    g_fProbeX = g_VisionInspectResults.fXPosition;
							g_fProbeY = g_VisionInspectResults.fYPosition;

							#ifdef OUTPUT_PROBE_SYS
							logf("%s got K: K%d x=%f,y=%f\r\n","PRB",g_cKHolePrbeIndex,g_fProbeX,g_fProbeY);//debug
							#endif
						}
						else if (g_cProbeCommand==TKP_PROBE_EDGE_MVEC)
						{
handle_like_machine:
							//This should be done from the prescribed vantage point, but as long as there
							//is an official fx,fy from KREFs, this should intersect edge with the line going through fx,fy and the vector.
							//It does the same thing from any vantage point and with the edge it sees.
							//The result will be checked for approx dist shift though.

#warns "FIXEM HIGH COMPLETE CASE WHERE fx,fy are not accurate and it's just an edge vac probe."
							//If there is no official fx,fy for this probe, then it could use the camera center combined with the vac.
							//This is essentially the same thing as saying the to use the current position + the vac.
							//Offset should be ignore in this case
							//THIS CASE IS NOT COMPLETE

							g_VisionInspectResults.cInfo |= VisionInfoPositionProbeEdgeVec;


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

							cresult=CreateMachineDataSetForOneHole(g_iKHoleHoleIndex[g_cKHolePrbeIndex], &fx,&fy);

							logf("%f %f %f %f\r\n",fx,fy,g_fProbeMachineVectorX,g_fProbeMachineVectorY);
							if (cresult==0)
							{
								//Send the report that was delayed earlier now.
								SendInspectResults();
								//This K Feature requires accurate location currently
								logf("***noap!uc\r\n");
								cprobestatus=PS_PROBED_FAILED; //set fail code
								goto probe_failed_verify; //really just probe failed here
							}

    	                    g_VisionInspectResults.fXPositionExpected = fx;
							g_VisionInspectResults.fYPositionExpected = fy;

							logf("es %d\r\n",g_VisionInspectResults.cEdgeStatus);

							if (g_VisionInspectResults.cEdgeStatus == 0)
							{
								//Send the report that was delayed earlyer now.
								SendInspectResults();
								//problem finding the edge...
								//Go back and wait again
								cprobestatus=PS_PROBED_FAILED; //set fail code
								goto probe_failed_verify; //really just probe failed here
							}
							if ((g_VisionInspectResults.cEdgeStatus & 1) > 0)
							{
								//use primary edge
								fx1=g_VisionInspectResults.fEdgeX1;
								fy1=g_VisionInspectResults.fEdgeY1;
								fx2=g_VisionInspectResults.fEdgeX2;
								fy2=g_VisionInspectResults.fEdgeY2;
								g_VisionInspectResults.cEdgeNote = 1;
							}
							else //must use center edge...
							{
								fx1=g_VisionInspectResults.fCEdgeX1;
								fy1=g_VisionInspectResults.fCEdgeY1;
								fx2=g_VisionInspectResults.fCEdgeX2;
								fy2=g_VisionInspectResults.fCEdgeY2;
								g_VisionInspectResults.cEdgeNote = 2;
							}
							//These Values Have the offsets removed, and represent feature locations

							//In the future a better pattern could be established
							//Current pattern works, but could be clearer.  Future should be best pattern and very clearly done.
							//I should enumerate the patterns possible for doing offset probing.

							logf("%f %f %f %f\r\n",fx1,fy1,fx2,fy2);
							//Find the Intersection of the edge
							cresult=FindIntersection(	fx,
												fy,
												fx+g_fProbeMachineVectorX,
												fy+g_fProbeMachineVectorY,
												fx1,
												fy1,
												fx2,
												fy2,
                                                &fx,
                                                &fy
												);
							logf("fiint%d %f %f\r\n",cresult,fx,fy);
							if (cresult!=0) //no valid intersection
							{
								g_VisionInspectResults.cEdgeNote = 0; //clear this again
								//Send the report that was delayed earlyer now.
								SendInspectResults();
								//Go back and wait again
							//?	//just set these to the start value for now...
            	            //    g_fKHolePrbeX[g_cKHolePrbeIndex]=fx;
                	        //    g_fKHolePrbeY[g_cKHolePrbeIndex]=fy;
								cprobestatus=PS_PROBED_FAILED; //set fail code
								goto probe_failed_verify; //really just probe failed here
							}
							//Add these values


    	                    g_VisionInspectResults.fXPosition = fx;
							g_VisionInspectResults.fYPosition = fy;
							RecalculatePostionPixels();

							//Send the report that was delayed earlyer now.
							SendInspectResults();

#warnt ">>>FIXME SEVERE  must check expected diameter?????"
/* Do this first with START THEN TEST?????
OR DO BELOW???????  YES
float g_fProbeExpectedDiameter;
*/

							g_fProbeX = fx;
							g_fProbeY = fy;
							#ifdef OUTPUT_PROBE_SYS
							logf("%s got K: K%d x=%f,y=%f %f,%f,%f,%f,%lu\r\n","PEMV",g_cKHolePrbeIndex,fx,fy,
										g_VisionInspectResults.fXPositionExpected,
										g_VisionInspectResults.fYPositionExpected,
										fx-g_VisionInspectResults.fXPositionExpected,
										fy-g_VisionInspectResults.fYPositionExpected,
										MS_TIMER);
							#endif
//zxcvbnm
						}
						else if (g_cProbeCommand==TKP_PROBE_EDGE_VEC)
						{
							//not done yet
//FIXMENOW NOT DONE
#warnt "FIXME HIGH  didn't do DATA SET VEC edge"
g_fProbeMachineVectorX=g_fProbeVectorX;
g_fProbeMachineVectorY=g_fProbeVectorY;
goto handle_like_machine;
							cprobestatus=PS_PROBED_FAILED; //set fail code
							goto probe_failed_verify; //really just probe failed here
						}
						else
						{
							//Unsupported Probe Style Is not compelte
							logf("probe???\r\n");
							cprobestatus=PS_PROBED_FAILED; //set fail code
							goto probe_failed_verify; //really just probe failed here
						}
                        #ifdef USE_HYSTERESIS_FROM_CENTERVISION
                        SetDriveThroughBacklash(g_ConfigData.cDriveThroughBacklash,g_fCenterVisionHysX/2.0,g_fCenterVisionHysY/2.0);
                        #else
                        SetDriveThroughBacklash(0,0,0);
						#endif
                        goto done_with_direct_probe;
    	            }
					#endif
                	if (g_cKHolePrbeCommand==PC_STOP)
   	                {
						//OID_PROBE PC_STOP na 	Stop Current Probe Move or Probe.
						//Load status variable before jumping
						cprobestatus=g_cKHolePrbeStatus[g_cKHolePrbeIndex];
						goto probe_over;
                    }
					if (g_cProbeMethod == PROBE_MANUAL) //Camera might have needed clamp, but it manages it's own clamping
					{
						#ifdef CLAMP_SYSTEM
        	           	if (g_cClampState!=CLAMP_CLAMP)
            	       	{
                	   		//must get clamped
							g_uiClampPressure = (int)g_ConfigData.uiPressure;
                    		g_cClampGoal=CLAMP_CLAMP;
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
					s_ulStartTime = MS_TIMER;
					ResetNearestPosition();
                    break;
				case PROBE_WAITCOORDS:
                	if (g_cKHolePrbeCommand==PC_STOP)
                    {
						//OID_PROBE PC_STOP na 	Stop Current Probe Move or Probe.
						//Load status variable before jumping
						cprobestatus=g_cKHolePrbeStatus[g_cKHolePrbeIndex];
						goto probe_over;
                    }
                    if (g_cProbeFlag != X_AND_Y)
                    {
                    	//did not yet get coordinates
						if ( ( MS_TIMER - s_ulStartTime ) >=  (PROBE_TIMEOUT_SEC * 1000L) )
						{
//ProbeStart Test
							#ifdef OUTPUT_PROBE_SYS
							logf("timeout wt prb res\r\n");
        					#endif
							//Go back and wait again
                            //g_fKHolePrbeX[g_cKHolePrbeIndex]; //Leave These
                            //g_fKHolePrbeY[g_cKHolePrbeIndex]; //Leave These
							cprobestatus=PS_PROBED_FAILED; //set fail code
							goto probe_failed_verify; //really just probe failed here
							//s_cProbeState = PROBE_INIT;
							//break;
						}
                    	break;
                    }

                    g_cProbeFlag = 0; //reset flag
					#ifdef OUTPUT_PROBE_SYS
                   	logf("PROBE coord ack\r\n");//debug
					#endif

                    //now beep
                    Beep();
					ResetNearestPosition();
                    //Store

                    //Got Coord Ack, Verify New Probe values.
					fx=g_fProbeX;
					fy=g_fProbeY;
					if (g_ConfigData.cProbeAcceptRequired==0)
					{
						cprobestatus=PS_PROBED;
					}
					else if (g_cProbeMethod == PROBE_LASER) //we never return since we don't clamp && g_cCenterVisionResult != CENTERVISION_SUCCESS)
					{
	        	        cprobestatus=PS_PENDING_ACCEPTANCE;
					}
                    else if (g_cProbeMethod == PROBE_CAM)
                    {
	        	        cprobestatus=PS_PENDING_ACCEPTANCE;
                    }
                    else // if (g_cProbeMethod == PROBE_MANUAL, PROBE_INSTANT, PROBE_INSTANT_OFFSET ...
                    {
	        	        cprobestatus=PS_PROBED;
                    }
                    //FIXME MED LOW Scale Linear Program connects in here, but it's currently not implemented
                    //ScaleLinearProgram could be used here

					//Store These Probed Values
                    g_fKHolePrbeX[g_cKHolePrbeIndex]=fx;
					g_fKHolePrbeY[g_cKHolePrbeIndex]=fy;
					g_cKHolePrbeStatusWarnings[g_cKHolePrbeIndex]=0; //clear warnings now
					g_cKHolePrbeStatusDistance[g_cKHolePrbeIndex]=0; //clear distance warnings until rechecked in future

                    //Verify Probe vs. Approximate value.
//ProbeStart Test
                    if (g_cKHolePrbeStart[g_cKHolePrbeIndex]==PS_APPROXIMATE)
                    {
                    	//verify with approximate value
						if (g_cCenterVisionRequiredResults!=CENTERVISION_EDGE)
						{
							pc1 = "Point Check\r\n";
						}
						else
						{
							pc1 = "Edge Check\r\n";
						}
						logf(pc1);
						//Find Shift
                        fdx=fx-g_fKHolePrbeStartX[g_cKHolePrbeIndex];
                        fdy=fy-g_fKHolePrbeStartY[g_cKHolePrbeIndex];
						if (fdx==0)
						{
							//use fast shortcut for common situation
							fError=fabs(fdy);
						}
						else if (fdy==0)
						{
							//use fast shortcut for common situation
							fError=fabs(fdx);
						}
						else
						{
                        	fError=sqrt(fdx*fdx+fdy*fdy);
						}
						//Check Against Shift Limit... But all this does is prevent Drilling... it does NOT fail probing.
						if (g_cProbeShiftLimX==MACHINEXY)
						{
							if (fdx<g_fProbeShiftLimXMin || fdx>g_fProbeShiftLimXMax)
							{
								logf("Shift Fail %s %f [%f,%f]\r\n","mslx",fdx,g_fProbeShiftLimXMin,g_fProbeShiftLimXMax);
								g_cKHolePrbeStatusWarnings[g_cKHolePrbeIndex]=1; //set warning
							}
						}
						if (g_cProbeShiftLimY==MACHINEXY)
						{
							if (fdy<g_fProbeShiftLimYMin || fdy>g_fProbeShiftLimYMax)
							{
								logf("Shift Fail %s %f [%f,%f]\r\n","msly",fdy,g_fProbeShiftLimYMin,g_fProbeShiftLimYMax);
								g_cKHolePrbeStatusWarnings[g_cKHolePrbeIndex]=1; //set warning
							}
						}
						//see if this vector is needed in data set coordinates
						if (g_cProbeShiftLimX==DATAXY || g_cProbeShiftLimY==DATAXY)
						{
							//rotate error vector to dataset
							fddx=fdx; //first copy
							fddy=fdy;
							cresult=RotateVecMachineToDataSet(g_iKHoleHoleIndex[g_cKHolePrbeIndex], &fddx, &fddy);
							if (cresult==0 || g_cRotationContext < PS_PENDING_ACCEPTANCE )
							{
								//Hole either does not have positioning primary and secondary K locations, or doesn't have then probed yet.
								//This is currently required for the checking of a shift limit
								logf("neo\r\n");
								cprobestatus=PS_NO_PROBE;
								goto probe_over;
							}
							if (g_cProbeShiftLimX==DATAXY)
							{
								if (fddx<g_fProbeShiftLimXMin || fddx>g_fProbeShiftLimXMax)
								{
									logf("Shift Fail %s %f [%f,%f]\r\n","slx",fddx,g_fProbeShiftLimXMin,g_fProbeShiftLimXMax);
									g_cKHolePrbeStatusWarnings[g_cKHolePrbeIndex]=1; //set warning
								}
							}
							if (g_cProbeShiftLimY==DATAXY)
							{
								if (fddy<g_fProbeShiftLimYMin || fddy>g_fProbeShiftLimYMax)
								{
									logf("Shift Fail %s %f [%f,%f]\r\n","sly",fddy,g_fProbeShiftLimYMin,g_fProbeShiftLimYMax);
									g_cKHolePrbeStatusWarnings[g_cKHolePrbeIndex]=1; //set warning
								}
							}
						}
						if (g_fProbeMaxDistShift>0)
						{
							if (fError>g_fProbeMaxDistShift)
							{
								logf("Shift Fail %s %f>%f\r\n","mds",fError,fdy,g_fProbeMaxDistShift);
								g_cKHolePrbeStatusWarnings[g_cKHolePrbeIndex]=1; //set warning
							}
						}
						//Now Check Against Limits
						//Currently using ApproxLocationError to judge only non edge finding approx locations
						//    using MaxEdgeShiftProbeAccept to judge edges....
						//FIXME FUTURE Eventually want to move towards one number to check approx, and another to check derived location shifts.
						if (g_cCenterVisionRequiredResults!=CENTERVISION_EDGE)
						{
							fDistance=g_ConfigData.fApproxLocationError;
							pc2 = "ale";
						}
						else
						{
							fDistance=g_ConfigData.fMaxEdgeShiftProbeAccept;
							pc2 = "mespa";
						}
                        if (fError>fDistance)
                        {
							logf("Shift Fail %s %f>%f\r\n",pc2,fError,fDistance);
							cprobestatus=PS_PROBED_APPROX_DIFF_FAILURE; //set fail code
							logf("Probe Fail\r\n");
							//FIXME Minor Add Print Control For these so that we can reclaim this space but turn on more testing when needed
							logf(" %f",g_fKHolePrbeStartX[g_cKHolePrbeIndex]);
							logf(" %f\r\n",g_fKHolePrbeStartY[g_cKHolePrbeIndex]);
							logf(" %f",fx);
							logf(" %f\r\n",fy);
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
					ck=g_cKHolePrbeIndex;
					CheckKLocationDistances(ck,1);
					//Any other K Locations failing the distance check are now set to PS_PROBED_KHOLE_DIFF_FAILURE
					//Any other K Locations passing the distance check to this one location are now marked 0xFF to show that they need to be rechecked
					//completely to see if they are now valid.
					ck=1;
                    while(ck<=g_cKHoleCount)
                    {
	                   	if (g_cKHolePrbeStatusDistance[ck]==0xFF) //this one was marked for RECHECK
						{
							//This didn't fail due to any of the previous checks, so recheck now.
							CheckKLocationDistances(ck,0);
							//Any other K Locations failing the distance check are now set to PS_PROBED_KHOLE_DIFF_FAILURE
							//They would no longer marked 0xFF and can be skipped.
							//If this hole was found to be good, it should be set to the StatusAttempt value which
							//shows how it did without the distance error checking.
						}
						ck++;
                    }
					//Count Distance Errors
					g_cProbeDistanceErrors=0;
					ck=1;
                    while(ck<=g_cKHoleCount)
                    {
						if (g_cKHolePrbeStatusDistance[ck]==1)
						{
							g_cProbeDistanceErrors++;
						}
						ck++;
                    }
					logf("kderr=%d\r\n",g_cProbeDistanceErrors);

 					//If the Start is not approx, it may be updated by this probe also
					if (g_cKHolePrbeStart[g_cKHolePrbeIndex]!=PS_APPROXIMATE)
					{
						//May Update ProbeStart
						if (g_cKHolePrbeStart[g_cKHolePrbeIndex]==PS_NO_PROBE || cprobestatus>=PS_PROBED)
						{
							g_fKHolePrbeStartX[g_cKHolePrbeIndex]=g_fKHolePrbeX[g_cKHolePrbeIndex];
							g_fKHolePrbeStartY[g_cKHolePrbeIndex]=g_fKHolePrbeY[g_cKHolePrbeIndex];
							g_cKHolePrbeStart[g_cKHolePrbeIndex]=cprobestatus;
							SendProbeStart(STP_ALERT,g_cKHolePrbeIndex); //SPS
						}
					}
                   	//Done Getting and Validating Probe
probe_failed_verify:
probe_over:
//ProbeStart Test
                    //Anything still marked PS_PROBING was a PS_PROBED_FAILED
                   	if (cprobestatus==PS_PROBING)
					{
						cprobestatus=PS_PROBED_FAILED;
					}
					g_cKHolePrbeStatus[g_cKHolePrbeIndex]=cprobestatus;
					logf("status=%d\r\n",cprobestatus);

					//Alert The Probe Status For This Probe
					SendProbeStatus(STP_ALERT,g_cKHolePrbeIndex);//SPS
					AlertProbeStatusUpdate();

                    g_cKHolePrbeTrigger=0; //clear trigger again at this point in case it was triggered while we were already probing

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
					if (g_cClampState==CLAMP_CLAMP || g_cClampGoal==CLAMP_CLAMP)
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
							g_cClampGoal=CLAMP_LOOSE; // Helps to come up off button without grabbing button
						}
						else
						{
							g_cClampGoal=CLAMP_UNCLAMP;
						}
					}
                    #endif

                    ProbeModeSelectAndSetGlobalRotation();

                	if (g_cKHolePrbeCommand==PC_STOP)
					{
    	                //return to init if probe was stopped
   		                s_cProbeState = PROBE_INIT;
#ifdef OUTPUT_PROBE_SYS_VERBOSE
logf("P_I_xs\r\n");
#endif
						break;
					}

					//If this was not a success, or not yet accepted, then can't auto continue or complete
               	    if (cprobestatus<PS_PENDING_ACCEPTANCE)
           	        {
    	                //return to init if probe was not a success
						//loop back for next probe
   		                s_cProbeState = PROBE_INIT;
#ifdef OUTPUT_PROBE_SYS_VERBOSE
logf("P_I_5\r\n");
#endif
	                    break;
                    }

               	    if (cprobestatus==PS_PENDING_ACCEPTANCE)
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
					RecalculatePositionsDuringProbe(g_cKHolePrbeIndex);//SPS
					AlertProbeStatusUpdate();
					//If Probe is now complete, proceed to the PROBE_COMPLETE
					CheckProbeComplete();
					if (g_cProbeComplete==1)
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
                	if (g_cKHolePrbeCommand==PC_MOVE_PROBE_ALL) //FIXME this path would need more testing to be used
                    {
                    	//should move an probe next one too
						ck=g_cKHolePrbeIndex;
						ck++;
                        if (ck<=g_cKHoleCount)
                        {
	                        g_cKHolePrbeIndex=ck;
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
						ck=g_cKHolePrbeIndex;
						ck++;
                        if (ck<=g_cKHoleCount)
						if (g_cKHolePrbeStatus[ck]<PS_PROBED)
                        {
							//next is not probed.
							g_cKHolePrbeCommandInput=PC_MOVE_PROBE;
							//Store the probe KIndex
							g_cKHolePrbeIndexInput=ck;
							//Assuming that it's waiting for a trigger
							g_cKHolePrbeTrigger=1; //trigger
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
					SendAllProbeStart(STP_ALERT);//SPSALL
					SendAllProbeStatus(STP_ALERT);//SPSALL
					AlertProbeStatusUpdate();
					//Now they have these values.
					//They may sone Done Probe and use them or they may use them, but camera probe again.
                    s_cProbeState = PROBE_INIT; //loop back for next
					break;
        		case PROBE_CREATE_ABSOLUTE_PROBE:
					g_cAction = ACTION_CALC;
                	//Make Everything probed at it's absolute location
                    ck=1;
                    while(ck<=g_cKHoleCount)
                    {
                        cprobestatus=PS_PROBED;
                        ui = g_iKHoleHoleIndex[ck];
                        fx=xgetfloat(xp_fRawDataX+4*ui);
                        fy=xgetfloat(xp_fRawDataY+4*ui);
                      	g_cKHolePrbeStatus[ck]=cprobestatus;
						g_cKHolePrbeStatusWarnings[ck]=0; //clear warning
                        g_fKHolePrbeX[ck] = fx;
                        g_fKHolePrbeY[ck] = fy;
	                    //Now Alert Result
						p_oid_probe_status=(td_oid_probe_status *)&g_STPtxMsg.p_cObjectValue;
    					p_oid_probe_status->cKIndex=ck;
						p_oid_probe_status->fX=fx;
						p_oid_probe_status->fY=fy;
						p_oid_probe_status->cStatus=cprobestatus;
						SmartToolMsg(STP_GET_RESP, OID_PROBE_STATUS, sizeof(td_oid_probe_status),(char*)p_oid_probe_status);
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
					if (g_cProbeComplete==0)
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
			        g_PosnMode.iCurPosnIndex=0;
					g_PosnMode.cFirstMove=1; //Trigger 1st move in Posn Init and Indicates that Next must go to Current, and not add or subtract one.  Other paths will overwrite this as usual.

                    //INSTEAD, GO FROM HERE TO POSITION MODE.
                    //// go back and just wait to see if user wants to re-capture K2
                    //s_cProbeState = PROBE_INIT;
					g_cModeState=MODE_POSN;

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
			if (g_cBeepMode!=BEEPPROBEADJUST && g_cBeepMode<BEEPSIGNALS) { BeepProbeAdjust(); }
            #endif
            switch (s_cProbeAdjustState)
            {
                case PROBE_ADJUST_INIT:

                    LEDProbeK(); //Use same K Flash for Probe Adjust
                    g_Probe.cProbeAdjustTrig = FALSE;
                    g_Probe.cGotProbeAdjust = 0; //reset flag which indicates if we recevied coords

                    s_cProbeAdjustState = PROBE_ADJUST_WAIT;
                    break;

                case PROBE_ADJUST_WAIT:
                    // wait for user to set OID_K2 or to press the red button
					#ifdef UNIBUTTON
   	                if (g_cUniButtonEvent == 1 )
       	            {
           	        	// capured a single button press
               	        g_Probe.cProbeAdjustTrig = TRUE;
						g_cUniButtonEvent=0; //clear event
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
	                	g_iProbedIndex = CalcProbeAdjust();  	// Returns dataset point index at probe point, -1 if out of tolerance.
       	                //	Always beep, even if out of tolerance.
           	            Beep();
						ResetNearestPosition();
						if( g_iProbedIndex >= 0 )		// If a dataset point is within tolerance, ...
                        {
							//Once PROBE_ADJUST has been performed, registration is exact.
							g_Probe.cRegistration = REGISTRATION_EXACT;
		                    SmartToolMsgChar(STP_ALERT, OID_PROBE_REGISTRATION, g_Probe.cRegistration);

							//Update OperationHistory for the probed hole.
							AddOpHistory( g_iProbedIndex, OP_PROBE);
	                        StartCreateMachineDataSet();
	           	            s_cProbeAdjustState = PROBE_ADJUST_WAIT_CALC;
						}
                        else
                        {
   							SmartToolMsgMiniFtMessageCode(OID_MCERR, MINIFTMC_PROBE_ADJUST_OUT_OF_TOLERANCE);
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
					g_PosnMode.cFirstMove=1; //Trigger 1st move in Posn Init and Indicates that Next must go to Current, and not add or subtract one.  Other paths will overwrite this as usual.
					//Go to Posn Mode
					g_cModeState=MODE_POSN;		// Return to position mode.
                    break;
            }
            break;
#else
			//Probe Adjust Not Supported
			g_cModeState=MODE_IDLE;
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

				if (g_cProbeCalculated==0)
   	            {
       	        	//shouldn't be here in posn
					SmartToolMsgMiniFtMessageCode(0, MINIFTMC_PROBE_INCOMPLETE);
                   	g_cModeState=MODE_IDLE;
					break;
   	            }

                //if they are floating when they come into position mode, then stop floating
              	if (g_cFloatStatus == FLOATSTAT_FLOAT)
 	            {
                        logf("Unfloat stp\n");
                        DoFloat(FLOAT_UNFLOAT_STOP);
                }
       			g_cFloatExitModePosnState=0; //clear this flag when starting position mode... we need to see only floats that stop during position mode.
	        }

        	if (g_cJogX!=JOGSTOP || g_cJogY!=JOGSTOP)
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
					if (g_cClampState==CLAMP_CLAMP)
    		        {
						//must be unclamped for float
   	        			g_cClampGoal=CLAMP_UNCLAMP;
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
					g_cUniButtonEvent=0; //clear event
    			}
                #endif
                break;
            }
			if (g_cFloatExitModePosnState==POSNMODE_MOVE_NEAREST)
            {
				//when they stop floating, move to nearest
  	   	       	g_cPosnModeState = POSNMODE_MOVE_NEAREST;
                #ifdef OUTPUT_POSITION_STATES
                logf("@PM_MOVE_NEAREST\r\n");
                #endif
                g_cFloatExitModePosnState=0; //clear flag
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
					g_cMoveDone = MOVEDONE_TRUE;  // intialize to true since carriage is not moving when we first enter posn mode
                    g_cPosnModeState = POSNMODE_STARTWAITNEXTPOSN;

	           		LEDOff();
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
					if (s_cModeStatePrev!=MODE_ESTOP)//don't auto unclamp after estop
					{
//zxcvzxcv111
	               	    g_cClampGoal=CLAMP_UNCLAMP; //Previously LOOSE
						g_cClampGoalSent = 0xFF; //force it to send goal again
					}
                    #endif
                   	// Clear Inspection
					g_cPositionInspection=0;

                    g_PosnMode.cOnCurPosn=0; //not on the position or unsure at this init time
					if( g_iProbedIndex >= 0 )
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
    	                g_PosnMode.iCurPosnIndex=g_PosnMode.iStartPosnIndex;
        	   	       	//Auto move to 1st hole
            	    	SpecifyGotoPosn(GOTOPOSN_NEXT,0);
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
					g_cDrillLoadProcessAndOverride=DRILL_LOAD_PROCESS_NOT_NEEDED;
					g_cOverrideSent = 0; //Don't need to recalculate again, but must send the override after each Start.
					g_ulMoveStart = 0; //clear move start time
					g_cFastDoneFlag = 0; //clear
					g_cPendingFastenerFaultUpdate = 0;

logf("station %c g %c\r\n",g_cStationCode[g_cStation],g_cStationCode[g_cStationGoal]);

	            	#ifdef OUTPUT_POSITION_STATES
            		logf("@PM_WAITNEXTPOSN\r\n");
            		#endif
                    g_cPosnModeState = POSNMODE_WAITNEXTPOSN;
                    //Fall through
	            case POSNMODE_WAITNEXTPOSN:
                	//waiting for move signal

					#ifdef BEEPSYSTEM
					if (g_cBeepMode!=BEEPOFF && g_cBeepMode<BEEPSIGNALS) { BeepOff(); } //Turn Off Beeper
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
		                	g_PosnMode.cOnCurPosn=0;		//moving actions always mean not on position
        	    		   	LEDOff();
							SendCurPosnInd(STP_ALERT); //Send Cur Posn Index, though it isn't on a hole
                    	}
						StopPosition(); //ensure it will stop motion
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
						g_cUniButtonEvent=0; //clear event
                        g_cAutoMove=0; //turn off auto move if anything requested the move directly
	                }
					#endif

                    if (g_cStartProcess==0 && g_PosnMode.cDoMoveTrig != TRUE && g_cAutoMove!=AUTOMOVE_MOVE)
                    {
						//nothing is triggered, so do not continue
						break;
					}

                    if (g_cDrillState != DRILLSTATE_IDLE)
                    {
                    	//Shouldn't really be here.
                        g_cStartProcess = 0;
						g_PosnMode.cDoMoveTrig = FALSE;
						logf("%d\r\n",-1001);
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

                    if (g_cStartProcess==1)
                    {
                    	if (g_PosnMode.cOnCurPosn!=1)
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
logf("uic=%u\r\n",g_uiPositionOpsCache);
							if (g_ConfigData.cDrillHoleOneTime!=0)
							{
								//Check History for this hole
								if ((g_uiPositionOpsCache & (OP_DRILL_STARTED | OP_DRILL_SUCCESS)) > 0)
								{
//logf("*pd\r\n");
               	                    g_cStartProcess = 0;
									SmartToolMsgMiniFtMessageCode(OID_NULLOID, MINIFTMC_POSITON_PREVIOUSLY_DRILLED);
									break;
								}
							}
							if (g_ConfigData.cAllowDrillBeyondShiftLimits!=1)
							{
								//Check for Probe Warnings
								if ((g_uiPositionOpsCache & OP_PROBE_WARNINGS) != 0)
								{
//logf("*pw\r\n");
       		   	                    g_cStartProcess = 0;
									SmartToolMsgMiniFtMessageCode(OID_NULLOID, MINIFTMC_PROBE_WARNING_DRILL_PREVENTED);
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
logf("station %c g %c\r\n",g_cStationCode[g_cStation],g_cStationCode[g_cStationGoal]);
						if ( g_cStation == g_cStationGoal )
                        {
		                    //Send Hole Parameters again. Always must send before action.
	    	                SendHoleParameters(STP_ALERT);
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
                    if (g_cAutoMove==AUTOMOVE_MOVE)
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
					if (g_cStationGoal!=STATION_LASERPOINTER)
					{
						g_cStationGoal = STATION_UNSPEC;
					}
#endif
logf("station %c g %c\r\n",g_cStationCode[g_cStation],g_cStationCode[g_cStationGoal]);
					#ifdef GENCIRCMFTX
//Consider this for ALLPATTERN candidate
 					if (g_cDrillState!=DRILLSTATE_IDLE)
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
                    if (g_PosnMode.iGotoPosnIndex<0)
                    {
						//Return to main POSNMODE State
	                    goto POSNMODE_CLEAR_AND_STARTWAITNEXTPOSN;
                    }

					//CycleTime
					g_ulLastMoveStart = g_ulMoveStart;

					//start time for move
					g_ulMoveStart = MS_TIMER;

					if (g_ulLastMoveStart!=0)
					{
logf("@@%s %lu\r\n","L",(g_ulMoveStart - g_ulLastMoveStart));
					}

                    //load default move type;
					s_cMoveType = g_ConfigData.cMoveType;

					ProfilePoint("TLCHCK");

                    //Clear the Drill Results (Will get new results after it's done.)
					g_HoleResultData.iHoleNumber = -1;
					g_HoleResultData.iHoleResult = HR_UNKNOWN;
logf("khr\r\n");
                	//announce the goto position
                    SendGoalPosnInd(STP_ALERT);

					//lookup the previous ops history
					g_uiPositionOpsCache = GetOpHistory(g_PosnMode.iGotoPosnIndex);
                    logf("*oh%d = %u\r\n",g_PosnMode.iGotoPosnIndex,g_uiPositionOpsCache);
logf("station %c g %c\r\n",g_cStationCode[g_cStation],g_cStationCode[g_cStationGoal]);

                    //Extract Data For Hole
                    //Set g_HoleData g_HoleParam and several other reference variables
                    //load the hole information for the gotoposn
					loadholedata(g_PosnMode.iGotoPosnIndex,&g_HoleData);
                    // LOAD THIS INFORMATION:
					//typedef struct
					//{
					//  char cKInd;
					//	char cKPri;
					//    char cKSec;
					//    char cTool;
					//    char cProcess;
					//    char cFastener;
					//    char cPattern;
					//	unsigned int uiOps; //array of bits used to indicate what operations are specified for this hole (optional for some input formats)
					//    unsigned int uiStack;
					//    int iCountersinkAdjust;
                    //	  unsigned int d2uDiameter
					//} td_HoleData;

                    LoadHoleParameters(); //this uses g_HoleData and loads more
                    SendHoleParameters(STP_ALERT); //send the 1st time
					//DRILL_DIRECT_PROCESS_AND_OVERRIDE
					g_cDrillLoadProcessAndOverride=DRILL_LOAD_PROCESS_NOT_NEEDED; //until moving... don't start loading process and overrides
					//Clear Override Calculation Flag
					g_cOverrideCalculated = 0;
					//Clear drill mode flag
					g_cSawDrillMode = 0; //clear

					if (g_cLastSearchedRequiredTool!=g_cRequiredTool)
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
                    if (g_cStartProcess==1)
					{
						//Advance the plan depending on operations selected and allowed
                    	AdvanceStations();
					}
logf("station %c g %c\r\n",g_cStationCode[g_cStation],g_cStationCode[g_cStationGoal]);

					if (g_cStationGoal != STATION_INSPECT && g_cStationGoal != STATION_LASERPOINTER)
					{
	           	        if (g_cStartProcess==0)
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
						logf("%c\r\n",'!');
					}
logf("station %c g %c\r\n",g_cStationCode[g_cStation],g_cStationCode[g_cStationGoal]);

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
					else if (g_cStationGoal==STATION_INSPECT && g_cStationPlanInspect==1)
					{
    	                if (g_cStartProcess==1)
						{
							//Use the standard auto inspect method specified when running the process
							//Other special position inspections run a position move inspection set to the CommandInspectMethod
							g_cInspectMethod = g_ConfigData.cInspectMethod;
						}
						#ifdef CENTERVISION_ANALOG_POINT_LASER
                        if (g_cInspectMethod == INSPECT_LASER)
                        {
							SetToolOffset(g_ConfigData.LaserSensorOffset.fX,g_ConfigData.LaserSensorOffset.fY);
                        }
						#endif
						#ifdef CENTERVISION_CAM
						if (g_cInspectMethod == INSPECT_CAMERA) //Inspection is with Camera
    	                {
							SetToolOffset(g_ConfigData.CamOffset.fX,g_ConfigData.CamOffset.fY);
						}
						#endif
						#ifdef CENTERVISION
						//Store these with Center Fision
						g_fCenterVisionOffsetX=g_MachineOffset.fX;
						g_fCenterVisionOffsetY=g_MachineOffset.fY;
						#endif
						//if (g_cInspectMethod == INSPECT_MANUAL) //Don't bother clearing the offset or setting an offset
                    }
					else if (g_cStationGoal==STATION_LASERPOINTER)
					{
						//recycle this value for this for now....
#warns "MUST ADD NEW OID FOR LASER SENSOR OFFSET TO Y???????????????????"
						SetToolOffset(g_ConfigData.LaserSensorOffset.fX,g_ConfigData.LaserSensorOffset.fY);
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
						if (g_ConfigData.cDrillHoleOneTime!=0)
						{
							if ((g_uiPositionOpsCache & (OP_DRILL_STARTED | OP_DRILL_SUCCESS)) > 0)
							{
								//Do not do process
logf("*pd\r\n");
	        		        	g_cStartProcess = 0;
								g_cShowPrevDrilledMessage = 1; //show after arrival
								//but do move
							}
						}
						if (g_ConfigData.cAllowDrillBeyondShiftLimits!=1)
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
					if (g_ConfigData.cToolVerifyEnable==1)
					{
						VerifyAndAlertTool();
						if (g_cToolLoaded==0)
   						{
            	            //proper tool was required and status was not OK.
							//Return to main POSNMODE State
		        	    	//Attempt to Clear any tool for movement
							#ifdef CLAMP_SYSTEM
							g_cClampGoal=CLAMP_UNCLAMP;
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
					//TimerBStart();		//	Begin software elapsed timer.

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
					if (g_cClampGoal==CLAMP_POSITION)
                    {
						if (g_cClampState!=CLAMP_POSITION)
						{
							break;
        				}
					}
					else if (g_cClampState!=CLAMP_UNCLAMP)
					{
						//not ready to continue to the rest of this state.


						//NOTE: This would be where we would lock out clamp changes if we had a lockout feature
						//      For this reason there is no point to recheck clamp below during moving.
						//		If we need lockout, just add that feature
						g_cClampGoal=CLAMP_UNCLAMP; //ask to unclamp again
						break;
					}
                    #endif
logf("%s %d\r\n","aex",g_cClampState);

					#ifdef FASTENER_STP
					if (g_cStartProcess==1)
                    {
					if (g_cStationGoal==STATION_PICKUP && g_cStationPlanFill==1)
                    {
logf("pr\r\n");
						//once in clamp position, OK to signal OID_PRESENT_READY
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
		            fx = xgetfloat((xp_fRawDataMX)+(4*(g_PosnMode.iGotoPosnIndex)));
					fy = xgetfloat((xp_fRawDataMY)+(4*(g_PosnMode.iGotoPosnIndex)));
					s_fTargetX = fx;
					s_fTargetY = fy;

					if (g_cStationGoal == STATION_PICKUP)
					{
						if (g_fFastenerTrayPickupPosition == 0)
						{
							//Did not get a tray pickup position
//FIXME MINOR  Fastener add message
#warns "Fas FIXME MINOR add message"
logf("f %d\r\n",0);
							//Return to main POSNMODE State
	                	    goto POSNMODE_CLEAR_AND_STARTWAITNEXTPOSN;
							break;
						}
						s_fTargetY = g_fFastenerTrayPickupPosition;
logf("%f\r\n",s_fTargetY);
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
			                s_cOneMove=0;
                            s_fErrLev=g_ConfigData.fPosnTolerance;
							s_cMoveOpt=0;
                            s_fErrLevFinal=g_ConfigData.fPosnTolerance;
							s_cMoveOptFinal=0;
                			break;
	                	case MOVETYPE_DIRECT:
                        	//Use Single Move with MiniFt Tolerance Settings
		                	s_cOneMove=1;
                            s_fErrLev=g_ConfigData.fPosnTolerance;
							s_cMoveOpt=0;
	            	    	break;
	                	case MOVETYPE_FAST:
                        	//Let 1st move be as fast as possible,
                        	// but final move is with MiniFt Tolerance Settings
		                	s_cOneMove=0;
							s_fErrLev=0.01;
                            s_cMoveOpt=1;
                            s_fErrLevFinal=g_ConfigData.fPosnTolerance;
							s_cMoveOptFinal=0;
	            	    	break;
	                	case MOVETYPE_ROUGH:
                        	//Single Move with larger room for error
	                		s_cOneMove=1;
							s_fErrLev=0.01;
							s_cMoveOpt=1;
	    	            	break;
	        	        case MOVETYPE_VROUGH:
                        	//Single Move with large room for error
	            	    	s_cOneMove=1;
							s_fErrLev=0.1;
							s_cMoveOpt=2;
		                	break;
    	            }
					if (g_cStationGoal==STATION_INSPECT && g_cStationPlanInspect==1)
					{
                		s_cOneMove=1;
						s_fErrLev=0.01;
						s_cMoveOpt=1;
						#ifdef CENTERVISION
						g_cCenterVisionResult = CENTERVISION_OFF; //set it back to the start state.
						//g_cCenterVisionInspectType Set by OID when the inspection is activated
//FIXME SEVERE TEST POSITION INSPECTION
						g_cCenterVisionRequiredResults=CENTERVISION_CENTER; //default
						g_fCenterVisionExpectedDiameter = ((float)g_HoleData.d2uDiameter)/10000;
						g_VisionInspectResults.cContext=2;//Posn
						g_VisionInspectResults.lposn=g_PosnMode.iGotoPosnIndex;
						g_VisionInspectResults.fXPositionExpected=fx; //copy position without machine offset
						g_VisionInspectResults.fYPositionExpected=fy;
						#endif
					}

					//set premove if needed
            		fPreMoveX=0.0;
	            	fPreMoveY=0.0;
					if (s_cOneMove==0)
					{
						//calculate magnitude and sign of premove based entirely on current grav comp parameters.
	            	    CalculatePreMove(&fPreMoveX, &fPreMoveY);
		            }

   					ProfilePoint("SND MV");

        	        // move the carriage
                	g_PosnMode.cOnCurPosn=0;		//moving actions always mean not on position
        	       	LEDOff();
                	//find change in x and y
					fdx = s_fTargetX+fPreMoveX - g_PosnMode.fLastKnownPosnX;
    	    		fdy = s_fTargetY+fPreMoveY - g_PosnMode.fLastKnownPosnY;

					//Check to see if premove would be in the direction of obstruction, or out of limits
                    cresult=CheckObstructionsAndMotionLimits(s_fTargetX+fPreMoveX,s_fTargetY+fPreMoveY);
                    if (cresult!=0)
                    {
                    	//Found a reason not to allow this move
						g_cPosnModeState = POSNMODE_STOP_MOVEMENT;
                    	break;
                    }
					//Check to see if final move would be in the direction of obstruction, or out of limits
                    cresult=CheckObstructionsAndMotionLimits(s_fTargetX,s_fTargetY);
                    if (cresult!=0)
                    {
                    	//Found a reason not to allow this move
						g_cPosnModeState = POSNMODE_STOP_MOVEMENT;
                    	break;
                    }

                    fdx=fabs(fdx);
                    fdy=fabs(fdy);

					//set the speed to make approximately linear movement:
   					ProfilePoint("SET SPDS");
               		SetMoveSpeeds(1,0,fdx,fdy);
   					ProfilePoint("SET PRMS");
					SetMoveParams(s_fErrLev,s_cMoveOpt);

					//NOW MOVE
   					ProfilePoint("MV");
					RunPosition(s_fTargetX+fPreMoveX, s_fTargetY+fPreMoveY);
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
        	        						s_fTargetY+fPreMoveY);//debug
        	        #endif

					BeepMove(); //Start Beeps

	                s_ulStartTime = MS_TIMER;

					//If moving to position, go ahead and start the process and override loading now
					//DRILL_DIRECT_PROCESS_AND_OVERRIDE
					if (g_cStationGoal == STATION_DRILL)
					{
						if (g_cDrillLoadProcessAndOverride==DRILL_LOAD_PROCESS_NOT_NEEDED)
						{
							g_cDrillLoadProcessAndOverride=DRILL_LOAD_PROCESS_NEEDED; //Now that there is a delay, authorize process loading to begin
						}
					}

	               	if (s_cOneMove!=0)
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
						g_cUniButtonEvent=0; //clear event
        	        }
					#endif

    	            if (g_cMoveDone == MOVEDONE_TRUE)
        	        {   // g_cMoveDone is set when rabbit receives "Done:Posn"
						ProfilePoint("PMV DN 1");

            	        g_cPosnModeState = POSNMODE_FINALMOVE;
                   	    g_cMoveDone = MOVEDONE_FALSE;  // reset flag that indicates when a move has completed now,
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
                    else if ( ( MS_TIMER - s_ulStartTime ) >=  (POSN_TIMEOUT_SEC * 1000L) )
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
                	g_PosnMode.cOnCurPosn=0;		//moving actions always mean not on position
	               	LEDOff();

					//MACHINEPOINTS

					//Check to see if final move would be in the direction of obstruction, or out of limits
                    //If a premove is used, this might have been cheked above, but we should recheck before we move
                    cresult=CheckObstructionsAndMotionLimits(s_fTargetX,s_fTargetY);
                    if (cresult!=0)
                    {
                    	//Found a reason not to allow this move
						g_cPosnModeState = POSNMODE_STOP_MOVEMENT;
                    	break;
                    }

        	        //set proper values for final move. Since it does not need to be linear, don't bother setting fdx,fdy.
            	    SetMoveSpeeds(0,1,0,0);
					SetMoveParams(s_fErrLevFinal,s_cMoveOptFinal);

	                //NOW MOVE
					RunPosition(s_fTargetX, s_fTargetY);
					ProfilePoint("FMV CMD");

                	s_ulStartTime = MS_TIMER;
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
						g_cUniButtonEvent=0; //clear event
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
						if ( ( MS_TIMER - s_ulStartTime ) <  (POSN_TIMEOUT_SEC * 1000L) )
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
						g_cUniButtonEvent=0; //clear event
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
						if (g_ConfigData.cToolVerifyEnable == 2 ) //g_PartPgmInfo.cTeachModeData!=1 &&
        	            {
            	        	//Another Tool Check Here
							VerifyAndAlertTool();
	                	    if (g_cToolLoaded==0)
	                        {
								goto POSNMODE_CLEAR_AND_STARTWAITNEXTPOSN;
								break;
							}

							//If moving to position, go ahead and start the process and override loading now if it hasn't happended yet
							//DRILL_DIRECT_PROCESS_AND_OVERRIDE
							if (g_cDrillLoadProcessAndOverride!=DRILL_LOAD_PROCESS_DONE)
							{
								//Did not yet load process and override
			//waiting
			if ((MS_TIMER & 3) == 0)
			{
				logf("a"); //FIXME SEVERE
			}
			//waiting
			if (g_cPrintAuth==0)
			{
				logf("dlp%d\r\n",((int)g_cDrillLoadProcessAndOverride));
			}
								break;
							}


							//Check Loaded Process now...
							if (g_cRequiredProcess>0)
							{
								if (g_cLoadedProcess != g_cRequiredProcess)
								{
									logf("lp %d rp %d\r\n", g_cLoadedProcess, g_cRequiredProcess );
								}
							}

							VerifyAndAlertProcess();
		                    if (g_cProcessLoaded==0)
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
						}//end verify enable 2
                        else if (g_ConfigData.cToolVerifyEnable == 3 )
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

    	go_directly_to_inspection:
        	        g_PosnMode.iCurPosnIndex = g_PosnMode.iGotoPosnIndex; // update current position
            	    g_PosnMode.iGotoPosnIndex = GOTOPOSN_UNKNOWN;  // we don't know next position until user tells us which hole is next
                	g_PosnMode.cOnCurPosn=1; //really on the position
                    g_PosnMode.cFirstMove=0; //clear the flag
					#ifdef BRAKE_ON_WHEN_STOPPED
    				BrakeOn();
                    #endif

       				//logf("DEBUG  posn id = %d and on position...\r\n",g_PosnMode.iCurPosnIndex);
					//LED ON
                   	LEDOn();
					//and beep one time
    	            Beep();
                	if (g_cAutoMove==AUTOMOVE_MOVE)
		            {
			            g_ulStartAutoTime=MS_TIMER;
    	            }
					SendCurPosnInd(STP_ALERT); //Send Cur Posn Index.
					GetPosition(); //must ensure final position is set.
					g_PosnMode.fLastSentPosnX+=1; //force a send.
					// Flag this position as visited.
                    AddOpHistory(g_PosnMode.iCurPosnIndex, OP_VISIT);

		            #ifdef OUTPUT_POSITION_STATES
	            	logf(">>ON POS\r\n\r\n");
	            	#endif
					ProfilePoint("ON POS");
					g_ulMoveEnd = MS_TIMER;
/*
					logf("M %lu M2A %lu M2TPC %lu  A2TPC delay %lu\r\n",
						g_ulMoveEnd - g_ulMoveStart,
						g_ulArrive - g_ulMoveStart,
						g_ulPastTPC - g_ulMoveStart,
						g_ulPastTPC - g_ulArrive);
*/
//For new time profile, I will track move done as ARRIVE
logf("@@%s %lu\r\n","M",g_ulArrive - g_ulMoveStart);
//For new time profile, track this as profile ready
logf("@@%s %lu\r\n","AP",g_ulMoveEnd - g_ulArrive);
                    //Rivet has it's own implementation of AUTOMOVE_ACTION, so don't effect it for now.
                    //It's possible that the systems could be combined so rivet uses the action state, but
                    //it's not needed at all for now.

					if (g_cStartProcess==1 || g_cPositionInspection==1)
					{
logf("inst %d %d\r\n",g_cStartProcess, g_cPositionInspection);
						//Only Do inspection if running process cycle, or if running an explicit inspection.
						if (g_cStation==STATION_INSPECT && g_cStationPlanInspect==1)
    	                {
							//not ready to officially go to action for this, but I want to share the code in that location
							goto inspection_code_handler;
						}
					}
                    if (g_cStartProcess==1)
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

                	if (g_cAutoMove==AUTOMOVE_MOVE)
		            {
                    	//do NOT return to main wait state, but go to action state
			            g_ulStartAutoTime = MS_TIMER;
	            		#ifdef OUTPUT_POSITION_STATES
            			logf("@PM_WAITCYCLE\r\n");
			       		#endif
	                	g_cPosnModeState = POSNMODE_WAITCYCLE;
                        break;
    	            }

					if (g_cShowPrevDrilledMessage == 1)
					{
						SmartToolMsgMiniFtMessageCode(OID_NULLOID, MINIFTMC_POSITON_PREVIOUSLY_DRILLED);
					}
					else if (g_cShowPrevDrilledMessage == 2) //use this to show this other message
					{
						SmartToolMsgMiniFtMessageCode(OID_NULLOID, MINIFTMC_PROBE_WARNING_DRILL_PREVENTED);
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

					if (g_PosnMode.cOnCurPosn==0)
                    {
                    	//can't do action
    	                //Ensure that StartProcess is clear
	                    g_cStartProcess = 0;
                    }
					#ifdef GENCIRCMFTX
					if (g_cStationPlanDrill!=1)
					{
logf("Force st pln d\r\n");
					g_cStationPlanDrill = 1; //FIXME Test with and without this to see if it works properly instead of forced
					}
					#endif
logf("station %c g %c\r\n",g_cStationCode[g_cStation],g_cStationCode[g_cStationGoal]);
                    //Tool Check

					if (g_cStation==STATION_INSPECT)
					{
					if (g_cStationPlanInspect==1)
                    {
inspection_code_handler:
						#ifdef CENTERVISION
						g_cCenterVisionResult = CENTERVISION_OFF; //set it back to the start state.
						#endif
						#ifdef CENTERVISION_ANALOG_POINT_LASER
						if (g_cInspectMethod==INSPECT_LASER) //Inspection is Laser
						#else
						#ifdef CENTERVISION_CAM
						if (g_cInspectMethod==INSPECT_CAMERA) //Inspection is Camera
						#else
						if (0)
						#endif
						#endif
	                    {
		                	//Move to inspection state...
							g_cPositionInspection=1;
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
							SmartToolMsgMiniFtMessageCode(OID_NULLOID, MINIFTMC_INSPECT_THIS_POSITION);
							//Clear this flag
							g_cPositionInspection=0;
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
						g_cPositionInspection=0;
        		        //Return and wait
		                goto POSNMODE_CLEAR_AND_STARTWAITNEXTPOSN;
						break;
					}
					}
                   	//Action Tool Check
					//If moving to position the drill do tool check.
					if (g_cStation==STATION_DRILL && g_cStationPlanDrill==1)
					{
#ifdef SMARTDRILL_STP
#warnt "DRILLSTP ensure inclusion of proper code for gen3 w/ dsi"
#endif
#warnt "DRILLSTP to require plan drill or not for this????  Problem is that it will NOT Reload Override?"
//but is that even a problem at all? :::  if on position, then it must have been loaded previously + doesn't it infact get loaded... review this AGAIN
						//If moving to position, go ahead and start the process and override loading now
						//DRILL_DIRECT_PROCESS_AND_OVERRIDE
						VerifyAndAlertTool();
						if (g_cToolLoaded==0)
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
logf("drf %d\r\n",g_DrillFault.cSeverity);
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
                    if (g_cStartProcess==0)
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
    							SmartToolMsgChar(STP_ALERT, OID_FASTENER_ARRIVED, 0);
							g_cFastenerArrived = 0; //clear this value
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
									SmartToolMsgMiniFtMessageCode(0, MINIFTMC_FASTENER_COM_FAIL);
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
POSNMODE_ACTION_PREP_label:
                    if (g_cStartProcess==0)
                    {
	                    //Must have stopped the process
						#ifdef CLAMP_SYSTEM
                        //FIXME000000000 none of the new clamp logic does any tests for EEOption
						if (g_cClampState!=g_cClampGoal)
                        {
                        	g_cClampGoal = CLAMP_HOLD;
                        }
                        #endif
       					g_cDrillStateGoal=DRILLSTATE_IDLE;
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
                    if (g_cEEOption!=EENONE)
					{
					if (g_cStation==STATION_DRILL && g_cStationPlanDrill==1)
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
	    	            if (g_cClampGoal!=CLAMP_CLAMP)
   		    	        {
   							//ask for clamp
   							SetPressureBasedOnProcess(g_cRequiredProcess);
							g_ulClampStart2 = MS_TIMER;
							g_cClampGoal=CLAMP_CLAMP;
							g_cClampGoalSent = 0xFF; //force it to send goal again
   							logf("Ask Clamp.\r\n");
                            g_cClampState=CLAMP_TRANSITION;
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
						if (g_ulClampDone==0)
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
							g_cClampGoalSent = 0xFF; //force it to send goal again
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
                    }//End not EENONE
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
                    if (g_cEEOption!=EENONE)
					{
//FIXME dfnow...
logf("stat=%c plan=%d\r\n",g_cStationCode[g_cStation],g_cStationPlanDrill);
					if (g_cStation==STATION_DRILL && g_cStationPlanDrill==1)
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
							i = FastenerSTPSetUInt(FASTENER_OID_FASTEN,0);//0 because it's not forced... should be ready
						}
						else if (g_cStationPlanRemove==1)
						{
							i = FastenerSTPSetUInt(FASTENER_OID_EXTRACT,0);//0 because it's not forced... should be ready
						}
						if (i==0)
						{
							logf("f\r\n");
							SmartToolMsgMiniFtMessageCode(0, MINIFTMC_FASTENER_COM_FAIL);
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
                    if (g_cStartProcess==0)
                    {
stop_action_as_soon_as_possible:
	                    //Must have stopped the process
						//No longer in AUTO
       					g_cDrillStateGoal=DRILLSTATE_IDLE;
						#ifdef SEAL_SYSTEM
						g_cSealState=SEALSTATE_OFF;
                        #endif
						#ifdef FASTENER_SYSTEM
						g_cFillState=FILLSTATE_OFF;
						g_cFastenerArrived=0; //allow clearance, but hope it will redetect it
                        #endif
						#ifdef GENCIRCMFTX
    	                if (g_cDrillState==DRILLSTATE_DONE)
	                    {
							g_cDrillState = DRILLSTATE_IDLE;
							g_cDrillStateGoal = DRILLSTATE_IDLE;
						}
						#endif
						if (g_cDrillState!=DRILLSTATE_IDLE)
                        {
                        	//Must Wait until it's back to IDLE
                            break;
                        }
                        //Drill is IDLE...
						//Return to main POSNMODE State
    	            	g_cPosnModeState = POSNMODE_STARTWAITNEXTPOSN;
#warns "ABORTS AND RESETS REVIEW"
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
                    if (g_cDrillState!=DRILLSTATE_DONE)
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
                	           	g_cStartProcess = 0; //Stop Process.
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
   	    	                   	g_cStartProcess = 0; //Stop Process.
								goto stop_action_as_soon_as_possible; //drill is stopped, but this is best way to exit
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
                           	g_cStartProcess = 0; //Stop Process.
							goto stop_action_as_soon_as_possible; //drill is stopped, but this is best way to exit
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
							g_cStationGoal = STATION_UNSPEC; //causes skip to end
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
                    if (g_cStartProcess==0)
                    {
	                    //Must have stopped the process
						#ifdef CLAMP_SYSTEM
						if (g_cClampState!=g_cClampGoal)
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
					g_cClampGoal=CLAMP_UNCLAMP;
                    #endif
                    if (g_cEEOption!=EENONE)
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
	  		            if (g_cClampGoal!=CLAMP_UNCLAMP)
    			        {
							//ask for unclamp
							g_cClampGoal=CLAMP_UNCLAMP;
							logf("Ask Unclamp.\r\n");
	   					}
						if (g_cClampState!=CLAMP_UNCLAMP)
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
					g_cDrillLoadProcessAndOverride=DRILL_LOAD_PROCESS_NOT_NEEDED;

                    //OK action was either skipped or was completed properly...
                    //Determine next actions

					if (g_cStation==STATION_DRILL)
					{
                    	if (g_DrillFault.cSeverity >= FAULT_SEVERITY_ALARM)
	                    {
    	                	//A Drill Fault was seen, so do not advance station, and do
        	                //not continue even if mode is continuous.
							g_cStationGoal=g_cStation; //return goal to current station
	        	            goto POSNMODE_CLEAR_AND_STARTWAITNEXTPOSN;
                	    }
                    }
					else if (g_cStation==STATION_FILL || g_cStation==STATION_PICKUP) //pickup is "1st" but fill is more likely
					{
                    	if (g_FastenerFault.cSeverity >= FAULT_SEVERITY_ALARM)
	                    {
    	                	//A Fastener Fault was seen, so do not advance station, and do
        	                //not continue even if mode is continuous.
							g_cStationGoal=g_cStation; //return goal to current station
	        	            goto POSNMODE_CLEAR_AND_STARTWAITNEXTPOSN;
                	    }
                    }


					g_ulFinalTime = MS_TIMER;

					if (g_ulClampStart2 != 0)
					{
						//ClampStart2 was set...
						logf("@@%s %lu\r\n","C",g_ulClampDone - g_ulClampStart2);
					}

					if (g_cStation==STATION_DRILL)
					{
					if (g_ulProcPassed != 0)
					{
						//Drill Path Shows Stats, Other Paths may need other output

						logf("@@%s %lu\r\n","SP",g_ulProcPassed - g_ulClampDone);
						if (g_ulDrillStart!=0 && g_ulDrillDone!=0)
						{
							logf("@@%s %lu\r\n","D",g_ulUnclampStart - g_ulDrillStart); //Drill Start to Unclamp Safe
							logf("@@%s %lu\r\n","U",g_ulUnclampDone - g_ulUnclampStart); //Unclamp Safe to Hole Results (might be near 0)
							lx = (long)(g_ulDrillDone - g_ulUnclampDone); //if positive, drill was done later than unclamp...
							if (lx < 0)                                   //if negative, unclamp had time to spare... so there was 0 drill wait time
							{
								//Drill was done (done according to hole results) prior to unclamp done
								logf("@#%s %lu\r\n","tts",fabs(lx)); //show the time to spare
								lx = 0; //There was no drill wait time
							}
							logf("@@%s %lu\r\n","H",lx); //Unclamp Safe to Hole Results (might be near 0)
						}
					} //end ProcPassed

					if (g_ulMoveStart!=0)
					{
						ulx=g_ulMoveStart;
						logf("@@%s %ld\r\n","lr",g_ulLPR - ulx);
						logf("@@%s %ld\r\n","ld",g_ulLD - ulx);
						logf("@@%s %ld\r\n","lo",g_ulLO - ulx);
						logf("@@%s %ld\r\n","sp",g_ulSpinUp - ulx);
						logf("@@%s %ld\r\n","ds",g_ulDrillStart - ulx);
						logf("@@%s %lu\r\n","dT",(g_ulFinalTime - ulx));
					}
					ulx=g_ulProcStart;
					logf("@@%s %ld\r\n","lr",g_ulLPR - ulx);
					logf("@@%s %ld\r\n","ld",g_ulLD - ulx);
					logf("@@%s %ld\r\n","lo",g_ulLO - ulx);
					logf("@@%s %ld\r\n","sp",g_ulSpinUp - ulx);
					logf("@@%s %ld\r\n","ds",g_ulDrillStart - ulx);
					logf("@@%s %lu\r\n","dA",(g_ulFinalTime - ulx));
					} // end STATION_DRILL
					else if (g_cStation==STATION_FILL)
					{
						if (g_ulFastenerStart!=0 && g_ulFastenerDone!=0)
						{
							logf("@@%s %lu\r\n","F",g_ulFastenerDone - g_ulFastenerStart); //Drill Start to Unclamp Safe
						}
						logf("@@%s %lu\r\n","U",g_ulUnclampDone - g_ulUnclampStart); //Unclamp Safe to Hole Results (might be near 0)

						if (g_ulMoveStart!=0)
						{
							ulx=g_ulMoveStart;
							logf("@@%s %lu\r\n","fT",(g_ulFinalTime - ulx));
						}
						ulx=g_ulProcStart;
						logf("@@%s %lu\r\n","fA",(g_ulFinalTime - ulx));
					}

					if (g_cStationGoal==g_cStation) //Select the Next Station
					{

logf("station %c g %c\r\n",g_cStationCode[g_cStation],g_cStationCode[g_cStationGoal]);
						//Select the next station goal
						NextStation();
logf("station %c g %c\r\n",g_cStationCode[g_cStation],g_cStationCode[g_cStationGoal]);
	                    //Advance the plan depending on operations selected and allowed
	                    AdvanceStations();
					}

                    } //end not EENONE

					//STATION-SYSTEM
logf("station %c g %c\r\n",g_cStationCode[g_cStation],g_cStationCode[g_cStationGoal]);
	logf("wloop %c...\r\n",g_cStationCode[g_cStationGoal]);
					if (g_cStationGoal!=STATION_UNSPEC)
                    {
                    	//still more stations to complete this set
                    	//going to have to start move, so start move logic
						SpecifyGotoPosnAgain();
                        //use the same handler as the main move next in waitcycle
                        goto handle_process_gotoposn;
                    }
					//Restore Station Goal Back To current Station
					g_cStationGoal=g_cStation;

					if (g_cActiveSessions==0)
					{
						//do not continue
logf("act0\r\n");
					}
					else if (g_ConfigData.cProcessContinueMode == PROCESS_CONTINUOUS)
		            {
                    	//Should continue with next hole
                    	//do NOT return to main wait state, but go to cycle wait now
			            g_ulStartAutoTime = MS_TIMER;
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
                    if (g_cStartProcess==0)
                    {
						//No longer in AUTO
    	            	//Return to wait next position
	                    goto POSNMODE_CLEAR_AND_STARTWAITNEXTPOSN;
                        break;
                    }

                    //Ensure wait time is reached (if any)
                	if ((MS_TIMER - g_ulStartAutoTime) < g_ulAutoTime)
                	{
                       	//not yet
                       	break;
					}

                    //Attempt next move
	                i=SpecifyGotoPosn(GOTOPOSN_NEXT,0);
				handle_process_gotoposn:
	                if (g_PosnMode.cDoMoveTrig == TRUE)
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
						if (g_cAutoRepeat==1)
                        {
		    	    		//Start again
////this is the new way... forcing move to 1st index
							g_PosnMode.iGotoPosnIndex = g_PosnMode.iStartPosnIndex;
	                    	SpecifyGotoPosn(GOTOPOSN_NEXT,0);
			                //Move was triggered
                            //Clear Flag
							g_PosnMode.cDoMoveTrig = FALSE;
							//Go Right to Tool Check
		                    g_cPosnModeState = POSNMODE_TOOLCHECK;
                            break;
                        }
                    }
                  	g_cAutoMove=0;
                    //Go Back to Main Wait State
                    goto POSNMODE_CLEAR_AND_STARTWAITNEXTPOSN;
					break;
        	    case POSNMODE_STOP_MOVEMENT:
	            	#ifdef OUTPUT_POSITION_STATES
            		logf("@PM_STOP_MV\r\n");
            		#endif
					StopPosition();
					#ifdef CLAMP_SYSTEM_NAC_STP
					if (g_cNAC==1)
					{
						cresult=NACClampPrepMove(0,-16,-16,0); //itime=0 ... should stop A motion.
					}
					#endif
    	            g_cMoveDone = MOVEDONE_TRUE;
					SendCurPosnInd(STP_ALERT); //Send Cur Posn Index, though it isn't on a hole
                    //clear automove
					g_cAutoMove=0;
	            	logf(">>>>MOVE DN\r\n\r\n");
                    //Go Back to Main Wait State
                    goto POSNMODE_CLEAR_AND_STARTWAITNEXTPOSN;
    	            break;
        	    case POSNMODE_MOVE_NEAREST:
            	    // request current machine posn
					GetPosition();
        	    	s_ulStartTime = MS_TIMER;
	        	    g_cPosnModeState = POSNMODE_MOVE_NEAREST_WAITCOORDS;
   	                //Ensure that StartProcess is clear
                    g_cStartProcess = 0;
					g_cDrillStateGoal=DRILLSTATE_IDLE;
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
	                    i=SpecifyGotoPosn(GOTOPOSN_NEAREST,0);
                        if (i!=GOTOPOSN_TOOFAR && i!=GOTOPOSN_OUTOFBOUNDS)
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
                    else if ( ( MS_TIMER - s_ulStartTime ) >=  (POSN_TIMEOUT_SEC * 1000L) )
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
					if (g_cPositionInspection==1)
					{
						//still working on it
						break;
					}
					#endif
					//Done
					SendCurPosnInd(STP_ALERT); //Send Cur Posn Index, though it isn't on a hole

					//Return to main POSNMODE State
                    goto POSNMODE_CLEAR_AND_STARTWAITNEXTPOSN;
POSNMODE_CLEAR_AND_STARTWAITNEXTPOSN:
   	                //Ensure that StartProcess is clear
                    g_cStartProcess = 0;
					g_cDrillStateGoal=DRILLSTATE_IDLE;
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
					strcpy(g_szPartPgmFilename,"Teach");
					g_PartPgmInfo.cLocked=0;
                    g_PartPgmInfo.cMiniFtFormat = 0;
                    g_PartPgmInfo.cChecksum=0;
					memset(g_PartPgmInfo.p_cChecksum16,0,16);
			        g_PosnMode.iCurPosnIndex=0;
					g_PosnMode.cFirstMove=1; //Trigger 1st move in Posn Init and Indicates that Next must go to Current, and not add or subtract one.  Other paths will overwrite this as usual.
                    g_cAllowKVisit=1; //in teach mode, all holes can be visited.
					g_cScaleBasedOnProbe = 0; //And Turn Off Scaling
					g_PartPgmInfo.cTeachModeData=1;
                	s_cTeachState = TEACH_WAIT;
                   	LEDOff();
	                break;

       	        case TEACH_WAIT:
           	        //if parameters indicate that the current program is not a taught program, then
               	    //clear the program and start over
				    if (g_PartPgmInfo.cTeachModeData==0)
				    {
						//If they missed init, or did something to bypass init... must init
						s_cTeachState = TEACH_INIT;
						break;
					}

                    // wait for user to set OID_K2 or to press the red button
					#ifdef UNIBUTTON
                    if (g_cUniButtonEvent == 1)
                    {
						// capured a single button press
                        g_Probe.cTeachCaptureTrig = TRUE;
						g_cUniButtonEvent=0; //clear event
    	            }
                    #endif

                    if (g_Probe.cTeachCaptureTrig == TRUE)
                    {   // g_Probe.cTeachCaptureTrig can be set by UniButton (above) or by a SET of OID_POSN_ADD
						g_Probe.cGotTeachCoords=0;
						ProbeTeachPosition();
                    	logf("sent p rqst\r\n");//debug
                    	g_Probe.cTeachCaptureTrig = FALSE;  // reset flag
                        s_cTeachState = TEACH_WAITCOORDS;
                        s_ulStartTime = MS_TIMER;
                    }

                    break;
                case TEACH_WAITCOORDS:
                    // wait here until we receive a msg that starts with "Done:K2Capture:"
                    // once that msg is rxvd, GotK2Coords will be set for us
                    if (g_Probe.cGotTeachCoords == X_AND_Y)
                    {
                    	logf("got TEACH coord ack\r\n");//debug

                        //Call the function to really create this position.
                    	CaptureTeachPosition();
                        g_Probe.cGotTeachCoords = 0; //reset flag
                        s_cTeachState = TEACH_WAIT; // go back and wait
                    }
                    else if ( ( MS_TIMER - s_ulStartTime ) >=  (PROBE_TIMEOUT_SEC * 1000L) )
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
		           DoFloat(FLOAT_UNFLOAT_STOP);   // Exit from float, and do not move to nearest hole.
				}
				LEDProbeK(); //use same flash pattern

				#ifdef CENTERVISION
				g_cCenterVisionResult = CENTERVISION_OFF;
				//g_cCenterVisionInspectType Set by OID when the inspection is activated
				g_cCenterVisionRequiredResults=CENTERVISION_CENTER; //default
				g_fCenterVisionExpectedDiameter = 0.0; //unknown
				g_VisionInspectResults.cContext=3;//Inspect
				g_VisionInspectResults.lposn=-1;
				g_VisionInspectResults.fXPositionExpected=0;
				g_VisionInspectResults.fYPositionExpected=0;
				g_cInspectMethod = g_ConfigData.cCommandInspectMethod;
				#ifdef CENTERVISION_ANALOG_POINT_LASER
				if (g_cInspectMethod == INSPECT_LASER)
				{
					SetToolOffset(g_ConfigData.LaserSensorOffset.fX,g_ConfigData.LaserSensorOffset.fY);
				}
				#endif
				#ifdef CENTERVISION_CAM
				if (g_cInspectMethod == INSPECT_CAMERA) //Inspection is with Camera
				{
					SetToolOffset(g_ConfigData.CamOffset.fX,g_ConfigData.CamOffset.fY);
				}
				#endif
				#ifdef CENTERVISION
				//Store these with Center Fision
				g_fCenterVisionOffsetX=g_MachineOffset.fX;
				g_fCenterVisionOffsetY=g_MachineOffset.fY;
				#endif
				//Use inspection flag
				g_cPositionInspection=1;
                #endif
				ResetNearestPosition();
			}
           	if (g_cFloatStatus == FLOATSTAT_FLOAT)
            {
            	break; //still in float...
            }
			#ifdef CLAMP_SYSTEM
			if (g_cEEOption!=EENONE)
			{
	           	if (g_cClampState!=CLAMP_UNCLAMP)
 				{
					//Should remove clamp
					g_cClampGoal=CLAMP_UNCLAMP;
					break;
				}
			}
			#endif
			#ifdef CENTERVISION
			PositionInspection();
			if (g_cPositionInspection==1)
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
				while (UDPDL_Tick())  // this should be called at least twice a second.
    	        {
        	    	//boot loader is operating
	            }
            }
#endif
			if (g_cGravCompStatus == GRAVCOMP_RUNNING)
			{
            	//must conclude it failed
	            g_cGravCompStatus=GRAVCOMP_NOTDONE;
    	        SmartToolMsgChar(STP_ALERT, OID_GRAVCOMP_STATUS, g_cGravCompStatus);
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
           	g_cAutoMove=0;
			g_ulStartAutoTime=0;
			g_cAutoRepeat=0;
            //shut down move trigger
            g_PosnMode.cDoMoveTrig = FALSE;
            //set light off
           	LEDOff();
            break;
        default:
            break;
    }

	// g_cModeState may have been changed in the state machine. Use the previous state
	s_cModeStatePrev = cEntryState;

	//DRILL_DIRECT_PROCESS_AND_OVERRIDE
	if (g_cDrillSync == DRILL_SYNC)
	{
	if (g_cDrillLoadProcessAndOverride>DRILL_LOAD_PROCESS_NOT_NEEDED)
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
	g_cClear=1;

//FIXME use better EEOption Pattern Idea to improve the pattern below....

    #ifdef HOMESYSTEM
    //Homing Logic
	while(g_cHomed == HOME_RUNNING)
	{
		g_cClear=0;
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
			g_cDrillStateGoalSent=0xFF; //Clear this to force send.
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
		if (g_cEEOption==EENONE || g_cEEOption==EEDRILLFILL)
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
			if (g_cBeepMode!=BEEPPROBEHOME && g_cBeepMode<BEEPSIGNALS) { BeepProbeHome(); }
            #endif
			SetDriveThroughBacklash(0,0,0);
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
			g_cReadRFID=RFID_READ_SEEK;
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

	if  (g_cHomed != HOME_RUNNING)
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
		logf("Homed=%d\r\n",g_cHomed);
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
                if (g_cModeState == MODE_POSN && g_cPosnModeState == POSNMODE_WAITNEXTPOSN) //FIXME Still annoyed by this pattern.
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
		SmartToolMsgChar(STP_ALERT, OID_AIR_PRESSURE, g_cDigInAirPressure);
		g_cAirPressureEvent = 2; //clear event
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
		SmartToolMsgChar(STP_ALERT, OID_TOOL_FLIP, g_ConfigData.cToolFlip);
        g_cPrevOrSensors = corient;
        if (corient == Y_NEG) { corient='n'; } else
        if (corient == Y_POS) { corient='p'; } else { corient='u'; }
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
		SmartToolMsgChar(STP_ALERT, OID_MACHINE_LOCK, g_cMachineLock);
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
		SmartToolMsgChar(STP_ALERT, OID_CLAMP, g_cClampState);
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

	if (g_cEEOption!=EENONE) //skip the entire clamp machine if the rail is not on.
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
	       			g_ulClampPressureHigh=MS_TIMER; //last time it was high
					g_uiClampPressureLBS=0;
					g_cClampExtend=VALVE_OFF;
					g_cClampRetract=VALVE_OFF;
					c=1;
				}
				if (g_uiClampPressureLBS>0)
				{
					//still at low pressure (or higher)
	       			g_ulClampPressureLow=MS_TIMER; //last time it was low
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
	       			g_ulClampPressureHigh=MS_TIMER; //last time it was high
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
				if (g_cALockMode==0) //Only needed if not prelocked locked
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
	       			g_ulClampPressureLow=MS_TIMER; //last time it was low
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
	       				g_ulClampPressureHigh=MS_TIMER; //last time it was high
					}
					//still at high or low pressure
	       			g_ulClampPressureLow=MS_TIMER; //last time it was low
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
					g_cLegsDown=VALVE_OFF; //let down pressure dissipate also
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
	                if (g_cALock==ALOCK_ON && g_ConfigData.uiALockDelay>0) //Lock Delay = 0 stops all ALock Off
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
    	   			g_ulClampPressureLow=MS_TIMER; //must be less than high pressure
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
			SmartToolMsgChar(STP_ALERT, OID_CLAMP, g_cClampState);
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
    p_c=(char *)0;
    //GENCIRCMFT2 Clamp Machine
//FIXME0 want, but not working
//	if (g_cDigInAirPressure == 1)
//	{
//		//no pressure... can't continue
//    	g_cClampState=CLAMP_UNKNOWN;
//	}
//	else

//FIXMEGENCIRCMFT2 test this to ensure it's not a problem
	if (g_cDrillState != DRILLSTATE_IDLE) //only change clamping if drill is idle
    {
		if (g_cDrillState != DRILLSTATE_ESTOP) //dont print this warning out if it's just ESTOP...
	    {
    	if (g_cClampState!=g_cClampGoal)
	    {
			logf("cci\r\n"); //Clamp Chanage Impossible until Drill is IDLE");
		}
		}
	}
	else
    if (g_cClampState!=g_cClampGoal)
    {
    	g_cClampState=CLAMP_TRANSITION;
    	c=0;
    	switch (g_cClampGoal)
        {
        	case CLAMP_UNKNOWN:
        		//This is not used as a goal, but only as the original state
				g_cClampGoal=CLAMP_UNCLAMP;
				c=1;
        		break;
        	case CLAMP_LOOSE:
				#ifdef DIGOUT_CHNUM_DRILLUP
        		//need to become loose.
                //start like unclamp
p_c="l0";
            	if (g_cColletClamp == 0 || g_cDrillUp == 0)
                {
p_c="l1";
					g_ulClampPressureHigh = MS_TIMER;
                	g_cColletClamp = 1; //loose
					#ifdef DIGOUT_CHNUM_DRILLUP
                    g_cDrillUp = 1;
					#endif
                    g_cDrillDown = 0;
                    c=1;
                    break;
                }
p_c="l2";
                //This is tricky: we are letting just enough air go into the up before
                //we turn off up again... This creates a cushion in case it drops on loose
   	    	    if ((MS_TIMER - g_ulClampPressureHigh) < 100) //30 ms for loose wait...
		        {
		        	//not done collet clamp unclamp
	    	    	break;
	        	}
p_c="l2";
				//then also turn off drill up
				g_cDrillUp = 0;
				g_cDrillDown = 0;
                if (g_cDigInDrillUp != ACTIVATED) //drill up is not ACTIVATED
                {
					g_cColletClamp = 1; //loose
				}
				else
				{
					g_cColletClamp = 1; //loose
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
p_c="u0";
            	if (g_cColletClamp == 0)
                {
					g_ulClampPressureHigh = MS_TIMER;
                	g_cColletClamp = 1;
                    c=1;
                }
p_c="u1";
   	    	    if ((MS_TIMER - g_ulClampPressureHigh) < g_ConfigData.uiAUnlockDelay)
		        {
		        	//not done collet clamp unclamp
	    	    	break;
	        	}
p_c="u2";
				#ifdef DIGOUT_CHNUM_DRILLUP
            	if (g_cDrillUp == 0)
				#else
            	if (g_cDrillDown != 0)
				#endif
                {
					g_ulClampPressureLow = MS_TIMER;
					#ifdef DIGOUT_CHNUM_DRILLUP
                    g_cDrillUp = 1;
					#endif
                    g_cDrillDown = 0;
                    c=1;
p_c="u3";
                }
				//This delay is not really needed, but we don't want a false alarm on the up sensor.
				//I set this delay to 100.
    	    	if ((MS_TIMER - g_ulClampPressureLow) < g_ConfigData.uiLowPressureDownDelay)
	    	    {
		        	//not supposed to be done yet
	    	    	break;
	        	}
                //REMOVED//must not see bottom sensor to be unclamped
                //REMOVEDif (g_cDigInDrillDown == ACTIVATED) //drill down is ACTIVATED
                //REMOVED{
                //REMOVED	//can't be considered unclamped yet if still see this
p_c="u4b";      //REMOVED
				//REMOVED	break;
                //REMOVED}
                //must see sensor to be unclamped
                if (g_cDigInDrillUp != ACTIVATED) //drill up is not ACTIVATED
                {
                	//can't be considered unclamped yet
p_c="u5b";
					//should be done soon... could do timeout
					break;
                }
                g_ulClampPressureZero = MS_TIMER;
		    	//Can be considered unclamped
               	g_cColletClamp = 1; //collect clamp is released.
				#ifdef DIGOUT_CHNUM_DRILLUP
				g_cDrillUp = 1;
				#endif
				g_cDrillDown = 0;
				c=1;
		    	//Now Done Unclamp
				g_cClampState=CLAMP_UNCLAMP;
p_c="ud";
				#ifndef DIGOUT_CHNUM_DRILLUP
				if (g_cClampGoal==CLAMP_LOOSE)
				{
					//trying to be loose instead...
					//just turn off collect clamp now
	               	g_cColletClamp = 0; //collect clamp is NOT released, and button is accessible
					//Now Done Loose
					g_cClampState=CLAMP_LOOSE;
p_c="ld";
				}
				#endif
//TRY THIS FOR ALL!!!!!!!
	               	g_cColletClamp = 0; //collect clamp is NOT released, and button is accessible

				break;
			case CLAMP_CLAMP:
				//need to become clamped
				if (g_cDrillDown == 0)
                {
	                //First Make sure collet is unclamped
    	        	if (g_cColletClamp == 0)
	                {
						g_ulClampPressureHigh = MS_TIMER;
        	        	g_cColletClamp = 1;
	                    c=1;
            	    }
   		    	    if ((MS_TIMER - g_ulClampPressureHigh) < g_ConfigData.uiAUnlockDelay)
			        {
		    	    	//not done collet clamp unclamp
p_c="c1";
	    	    		break;
		        	}
                    //Collet is unclamped, so start down
					g_ulClampPressureZero = MS_TIMER;
					#ifdef DIGOUT_CHNUM_DRILLUP
					g_cDrillUp = 0;
					#endif
                    g_cDrillDown = 1;
                    c=1;
                }
                //sensor must be off to be down
                if (g_cDigInDrillUp == ACTIVATED)
                {
                	//must not really be down yet.
					//reset timer, so that timer can be time from leaving sensor to collect lock
					g_ulClampPressureZero = MS_TIMER;
p_c="c2";
                    break;
                }
   	    	    if ((MS_TIMER - g_ulClampPressureZero) < g_ConfigData.uiLowPressureDelay)
		        {
		        	//not done yet down yet
p_c="c3";
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
            	if (g_cColletClamp == 1)
                {
					g_ulClampPressureLow = MS_TIMER;
                	g_cColletClamp = 0;
                    c=1;
                }
   	    	    if ((MS_TIMER - g_ulClampPressureLow) < g_ConfigData.uiALockDelay)
		        {
		        	//not done collet clamp unclamp
p_c="c5";
	    	    	break;
	        	}
                g_ulClampPressureHigh = MS_TIMER;
		    	//Can be considered unclamped
               	g_cColletClamp = 0; //clamp not released
				#ifdef DIGOUT_CHNUM_DRILLUP
				g_cDrillUp = 0;
				#endif
				g_cDrillDown = 1;
				c=1;
		        //Now done Clamp
       			g_cClampState=CLAMP_CLAMP;
p_c="cd";
				break;
		}
		if (c!=0)
		{
			digOut(DIGOUT_CHNUM_COLLETCLAMP, g_cColletClamp );
			#ifdef DIGOUT_CHNUM_DRILLUP
			//this system only has drill down doing both directions
			digOut(DIGOUT_CHNUM_DRILLUP, g_cDrillUp );
			#endif
			digOut(DIGOUT_CHNUM_DRILLDOWN, g_cDrillDown );
			#ifdef OUTPUT_HDCLAMP_STATES
			#ifdef DIGOUT_CHNUM_DRILLUP
			logf( "CLAMP collet=%d, UP=%d, DOWN=%d\r\n", g_cColletClamp, g_cDrillUp, g_cDrillDown );
			#else
			logf( "CLAMP collet=%d, DOWN=%d\r\n", g_cColletClamp, g_cDrillDown );
			#endif
			#endif
		}
	}
	#ifndef DIGOUT_CHNUM_DRILLUP
	//There is really no difference between unclamp and loose because there is only one value for UP or LOOSE
	//If the up sensor is not seen, drop from CLAMP_UNCLAMP back down to CLAMP_LOOSE to protect the system from dragging the drill.
	else if (g_cClampState==CLAMP_UNCLAMP) //AND GOAL IS UNCLAMP
	{
		//Goal Is Unclamped, and the State is Unclamped...
		//If the up sensor is not seen then make the state LOOSE
        if (g_cDigInDrillUp != ACTIVATED) //drill up is not ACTIVATED
        {
			g_cClampState=CLAMP_LOOSE;
		}
	}
	#endif
	//Always Alert Clamp
	if (g_cClampState!=g_cClampStateSent)
	{
		//send alert of changed state
		SmartToolMsgChar(STP_ALERT, OID_CLAMP, g_cClampState);
        g_cClampStateSent = g_cClampState;
        logf("Alrt Clamp=%d\r\n",g_cClampState);
	}

	if (g_cClampState!=CLAMP_UNCLAMP)
	{
		g_cClear=0;
	}
	if (p_c!=0)
	{
    if (g_szClampMessage!=p_c)
	{
		g_szClampMessage=p_c;
		logf("%s\r\n",g_szClampMessage);
	}
	}



	if (g_ConfigData.cUseCutterDetect != 0)
	{
		//just use this for lone
		g_ulCutterAirTotal = g_ulCutterAirTotal - g_uiCutterAir + anaIn(ADC_CHANNEL_DRILL_AIR_FLOW);
		g_uiCutterAir = g_ulCutterAirTotal/64;


		if (g_uiCutterAir<=1802)
		{
			if (g_cCutterDetected != CUTTER_DETECTED)
			{
				logf("CD!!!!!!!!!!!!!!\r\n");
				g_cCutterDetected = CUTTER_DETECTED;
			}
logf("l %u\r\n",g_uiCutterAir);
		}
		else if (g_uiCutterAir>1807)
		{
			if (g_cCutterDetected != CUTTER_NOT_DETECTED)
			{
				g_cCutterDetected = CUTTER_NOT_DETECTED;
				logf("CND-------------\r\n");
			}
logf("h %u\r\n",g_uiCutterAir);
		}
		else
		{
logf("m %u\r\n",g_uiCutterAir);
		}
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
			cdm='1';
            //Button should be off
			g_cDrillButton = 0;
//REMOVED // g_cCutterDetected not reliable when up..."
//REMOVED //			if (g_cCutterDetected == 1) //FIXME
//REMOVED //			{
//REMOVED //				//can't go to IDLE
//REMOVED //				break;
//REMOVED //			}
			g_cDrillState = DRILLSTATE_IDLE;
			break;
        case DRILLSTATE_IDLE:
			cdm='I';
            //Button should be off
			g_cDrillButton = 0;
			if (g_cDrillStateGoal == DRILLSTATE_DRILL)
			{
				if (g_cClampState!=CLAMP_CLAMP)
				{
					break;
				}
				g_cDrillState = DRILLSTATE_DRILL;
				cdm='D';
				g_ulDrillStateStartTime = MS_TIMER;
				g_uiMessageCode=0;
				g_cDrillButton = 1;
				uiop=OP_DRILL_STARTED;
				g_uiPositionOpsCache |= uiop; //since this is fir current position
				AddOpHistory(g_PosnMode.iCurPosnIndex,uiop);
				logf("*aoh%u\r\n",uiop);
				c = 2;//force setting of output
   				g_uiCutterAir=1800;
			}
            break;
		case DRILLSTATE_DRILL:
//This code has been written with the assumption that once the drill button is pressed, it will proceed to the limit and then retract
//and that there is nothing that can stop that cycle.   If the button is released early though, it might not have triggered.
//Anytime after the button delay time, if the drill is clear, assume it either did not fire, or has returned.
//The delay must be long enough to cover the time it takes to start the drill and feed to the point it has extended through the sensor.
			cdm='D';
			if (g_cClampState!=CLAMP_CLAMP)
			{
				//clear button variable
				g_cDrillButton = 0;
				c = 2;//force setting of output
			}

			//Calculate Elapsed Drill Time In One place for multiple uses below
			ui=(unsigned int)(MS_TIMER - g_ulDrillStateStartTime);

			//See if it has been held long enough.
			//This delay should be long enough that the bit would have extended through the sensor.
			if (ui < g_ConfigData.uiDrillButtonDelay)
			{
       	    	//not done holding button
           	    break;
            }
			if (g_cDrillButton == 1)
			{
				g_cDrillButton = 0;
				c = 2;//force setting of output
			}
			if (g_ConfigData.cUseCutterDetect != 0)
			{
				if (ui < g_ConfigData.uiPressureDelay)
				{
					//Time is shorter than the shortest acceptable drill time.
					//Air Must Should be going
					if (g_cCutterDetected == CUTTER_NOT_DETECTED)
					{
						//Air is not going
						//issue warning, stop cycle, and do not unclamp
						if (g_uiMessageCode==0)
						{
							g_uiMessageCode=MINIFTMC_DRILL_AIR_OFF_EARLY;
							SmartToolMsgMiniFtMessageCode(OID_NULLOID, MINIFTMC_DRILL_AIR_OFF_EARLY);
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
					if (g_uiMessageCode!=MINIFTMC_DRILL_AIR_ON_LONG)
					{
						g_uiMessageCode=MINIFTMC_DRILL_AIR_ON_LONG;
						SmartToolMsgMiniFtMessageCode(OID_NULLOID, MINIFTMC_DRILL_AIR_ON_LONG);
					}
					g_ConfigData.cProcessContinueMode == PROCESS_SINGLE; //no need to alert this out...
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
			uiop=OP_DRILL_SUCCESS;
			g_uiPositionOpsCache |= uiop; //since this is fir current position
			AddOpHistory(g_PosnMode.iCurPosnIndex,uiop);
			logf("*aoh%u\r\n",uiop);

//FIXMENOWzxcvzxcv>>>>
            if (g_ConfigData.uiPressure==0) //FIXME Minor  Add OID for this, or get rid of feature... right now I'm not sure we will use this
            {
				g_cDrillState = DRILLSTATE_WAIT_CONFIRM;
				cdm='W';
            }
            else
            {
				g_cDrillState = DRILLSTATE_DONE;
				cdm='X';
            }
            break;
		case DRILLSTATE_ESTOP:
			cdm='E';
			break;
		case DRILLSTATE_WAIT_CONFIRM:
			cdm='W';
            //Button should be off
			g_cDrillButton = 0;
        	//stay until OID message moves to done...
        	break;
        case DRILLSTATE_DONE:
			cdm='X';
            if (g_cModeState != MODE_POSN || g_cPosnModeState != POSNMODE_ACTION)
            {
            	//can go from done to IDLE
				g_cDrillState = DRILLSTATE_IDLE;
				g_cDrillStateGoal = DRILLSTATE_IDLE; //clear this too
            }
            //Button should be off
			g_cDrillButton = 0;
        	break;
		default:
            cdm='?';
			break;
    }
	if (g_cDrillButton != c)
	{
		digOut(DIGOUT_CHNUM_DRILLBUTTON, g_cDrillButton );
    }
	if (g_cDrillState != g_cDrillStatePrev)
	{
		logf("DS_%c\r\n",cdm);
		//send out drill state
        //FIXME0 should I have drill state.....
        //		should I have this work pendant....
        //		Or genernal action state??????
		SmartToolMsgChar( STP_ALERT, OID_DRILL_STATE, g_cDrillState);
 	}

	//capture the correct previous state
	g_cDrillStatePrev = g_cDrillState;

	if (g_cDrillState != DRILLSTATE_IDLE) //Also do not let it consider clear if drilling.
    {
		g_cClear=0;
    }

	//Let CIRCMFT operate as EENONE
	if (g_cEEOption==EENONE)
	{
		g_cClear=1;
		g_cClampGoal=CLAMP_UNCLAMP;
		g_cClampState=CLAMP_UNCLAMP;
	}
#endif

	if (g_cJogGoalY!=JOGSTOP || g_cJogGoalX!=JOGSTOP) // Check Reason that Jog Might have to be stopped
	{

    //Obstruction System - Stop Jog
	if (g_cObstructionCode!=0)
	{
		#ifdef Y_LIMIT_SENSORS
		if (g_cDigInYPosLimit==0)
		{
			if (g_cJogGoalY == JOGPOS) { goto handle_stop_jog; }
		}
		if (g_cDigInYNegLimit==0)
		{
			if (g_cJogGoalY == JOGNEG) { goto handle_stop_jog; }
		}
        #endif
		#ifdef OBSTRUCTION_SYSTEM_XP1
		if (g_cDigInObstructionXP1==OBSTRUCTION)
		{
    		if (g_cJogGoalX == JOGPOS) { goto handle_stop_jog; }
		}
		#endif
		#ifdef OBSTRUCTION_SYSTEM_XN1
		if (g_cDigInObstructionXN1==OBSTRUCTION)
		{
			if (g_cJogGoalX == JOGNEG) { goto handle_stop_jog; }
		}
		#endif
		#ifdef OBSTRUCTION_SYSTEM_MOS
		if (g_cDigInObstructionMOS==MO_OBSTRUCTION)
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
	    	goto handle_stop_jog_no_break; //must stop jog goal
		}
    }
	if (g_cAction > ACTION_READY)
	{
		logf("j:oa\r\n");
    	goto handle_stop_jog_no_break; //must stop jog goal
	}
	#ifdef JOG_ENABLE_TIME
	if (g_ConfigData.cJogEnableTimeout>0)
	{
	ulx=MS_TIMER - g_ulJogEnableTime;
	if (ulx>g_ConfigData.cJogEnableTimeout)
	{
		logf("j:et\r\n");
		//Jog must be enabled at a rate of more than 100 ms,
		//If jog is not continually enabled, then any jog move will be terminated.
    	goto handle_stop_jog_no_break; //must stop jog goal
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
    		goto handle_stop_jog_no_break; //must stop jog goal
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

	if (g_cJogX!=g_cJogGoalX)
	{
		//Check For Jogs
		if (g_cJogGoalX!=JOGSTOP)
		{
			//RunJogX
			RunJogX(g_cJogGoalX,g_fJogGoalX);
			if (g_cJogGoalX!=0)
			{
				g_cJogX=g_cJogGoalX;
				g_fJogX=g_fJogGoalX;
				g_ulJogStopX=MS_TIMER-50;
			}
			//#ifdef BEEPSYSTEM
            //if (g_cBeepMode!=BEEPJOG) { BeepJog(); }
            //#endif
        }
        else //IT's a STOP
        {
			//X is waiting to see acknowledgements of stop
			if ((MS_TIMER-g_ulJogStopX)>=30)
			{
				StopJogX(); //must contiune until confirmed stop
				g_ulJogStopX=MS_TIMER;
			}
        }
    }
	if (g_cJogY!=g_cJogGoalY)
	{
		if (g_cJogGoalY!=JOGSTOP)
		{
			//RunJogY
			RunJogY(g_cJogGoalY,g_fJogGoalY);
			if (g_cJogGoalY!=0)
			{
				g_cJogY=g_cJogGoalY;
				g_fJogY=g_fJogGoalY;
				g_ulJogStopY=MS_TIMER-50;
			}
            //I don't think this is needed FIXME minor  test on machine that has beep
			//#ifdef BEEPSYSTEM
            //if (g_cBeepMode!=BEEPJOG) { BeepJog(); }
            //#endif
		}
        else //IT's a STOP
        {
			//Y is waiting to see acknowledgements of stop
			if ((MS_TIMER-g_ulJogStopY)>=30)
			{
				StopJogY(); //must contiune until confirmed stop
				g_ulJogStopY=MS_TIMER;
			}
        }
    }
	#ifdef BEEPSYSTEM
    if (g_cJogX==JOGSTOP && g_cJogY==JOGSTOP)
    {
    	//not Jogging
        if (g_cBeepMode==BEEPJOG) { BeepOff(); }//will cause mode to be BEEPOFF, which will let other permanent modal modes activate....
    }
    else
    {
        if (g_cBeepMode!=BEEPJOG && g_cBeepMode<BEEPSIGNALS) { BeepJog(); }
    }
   	if (g_cFloatStatus != FLOATSTAT_FLOAT)
    {
    	if (g_cBeepMode==BEEPFLOAT) { BeepOff(); }
    }
    else
    {
    	if (g_cBeepMode!=BEEPFLOAT && g_cBeepMode<BEEPSIGNALS) { BeepFloat(); }
    }
	#endif

	//MOVE ERROR and MOVE STOP RESET CODE
	if (g_cMoveDone == MOVEDONE_ERROR || g_cMoveDone == MOVEDONE_STOP)
	{
		//If nothing above caught this flag, then now catch it:
		g_PosnMode.cOnCurPosn=0;		//moving actions always mean not on position
		LEDOff();
       	g_cMoveDone = MOVEDONE_TRUE;
	}

	//Periodic STP Output : Mode based
	// Because output depends on mode and changes during modes, this is the current location for this code
	#ifdef CENTERVISION
	if (g_cCenterVisionResult == CENTERVISION_OFF)
	{
		g_ulPositionUpdateThrottle=POSITION_UPDATE_THROTTLE_DEFAULT;
		g_ulPositionSendThrottle=POSITION_SEND_THROTTLE_DEFAULT;
	}
    #endif

	if ((MS_TIMER - g_ulPositionUpdateTime) >= g_ulPositionUpdateThrottle)
	{
		//time for position update
		g_ulPositionUpdateTime=MS_TIMER;
		GetPosition();
	}

	if (g_PosnMode.cFreshCurPosnSend==1)
	{
	if ((MS_TIMER - g_ulPositionSendTime) >= g_ulPositionSendThrottle)
	{
        if (fabs(g_PosnMode.fLastSentPosnX-g_PosnMode.fLastKnownPosnX) > g_fPosnLSCD ||
            fabs(g_PosnMode.fLastSentPosnY-g_PosnMode.fLastKnownPosnY) > g_fPosnLSCD )
		{
			g_PosnMode.fLastSentPosnX=g_PosnMode.fLastKnownPosnX;
			g_PosnMode.fLastSentPosnY=g_PosnMode.fLastKnownPosnY;

            //FIXME0000000 currently only sending nearest after everything is probed and probe is complete
            //I could send nearest as soon as partial information was ready, but it's much more complicated.
			if (g_cProbeCalculated==1)
			{
				if (g_PosnMode.iLastSentNearestPosn!=FindNearestPosition()) //FindNearest will cache for this X and Y
				{
					SendNearXY(STP_ALERT); //only uses cached value
					g_PosnMode.iLastSentNearestPosn=g_PosnMode.iNearestPosn; //even faster chache of last sent....
				}
			}

			//Send XY Last : Pendant will only update the screen for the above information
			//when it gets the XY coordinates.
			SendCurXY(STP_ALERT);
		}
		g_ulPositionSendTime=MS_TIMER;
		g_PosnMode.cFreshCurPosnSend=0;
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

    if (g_cLastAutoMove!=g_cAutoMove)
    {
		SmartToolMsgChar(STP_ALERT, OID_AUTOMOVE, g_cAutoMove);//echo for sync
        g_cLastAutoMove=g_cAutoMove;
    }

	if (g_cSentStationGoal != g_cStationGoal)
	{
		//Since there is OID for goal, use OID_STATION over 100
		SmartToolMsgChar(STP_ALERT, OID_STATION, 100 + g_cStation);
		g_cSentStationGoal = g_cStationGoal;
	}
	if (g_cSentStation != g_cStation)
	{
		SmartToolMsgChar(STP_ALERT, OID_STATION, g_cStation);
		g_cSentStation = g_cStation;
	}


} //ModeStateMachine


#ifdef USE_RFID_OMRON
nodebug void RFIDRun()
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
				SendRFIDData(STP_ALERT);
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
				if (g_cRunningMsgGoal != PositionStop) { StopPositionAdvanced(); }
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
			SendRFIDData(STP_ALERT);

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
			AlertObstructionCode();
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
					if (g_RfidStateAndData.cFullReading==1)//FIXME minor... see rfid notes.
					{
						g_szTagDatalen = RFID_FMS00_DATA_SIZE - 1; //reads only the last part and then sets byte 47 to ' '
						memcpy(g_szTagData,g_RfidStateAndData.szTagData+2,g_szTagDatalen); //64 + 1
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
						if (g_cRunningMsgGoal != PositionStop) { StopPositionAdvanced(); }
					}
				}
			}
			else //if (g_cTagState==TAG_STARTED)  || TAG_FOUND
			{
//FIXME Dual Tag problem presents a number of issues..... MUST IGNORE THIS FOR NOW for sake of time....
				if (g_cTagReadState==RFID_TAG_IS_PRESENT) //FIXMENOW FIXME SEVERE  || TagNotSameTag)
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
								if (g_cRunningMsgGoal != PositionStop) { StopPositionAdvanced(); logf("se\r\n"); }

							}
						}
						else
						{
							if (g_fTagEnd - g_ConfigData.RFIDConfig.fseekPastBorder > g_PosnMode.fLastKnownPosnX)
							{
								if (g_cRunningMsgGoal != PositionStop) { StopPositionAdvanced(); logf("se\r\n"); }
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
				g_RFIDData.ulseektime = 0; //FIXME SEVERE
				g_RFIDData.fsstart = 0;
				g_RFIDData.fpstart = 0;
				g_RFIDData.fpend = 0;
				g_RFIDData.fnstart = 0;
				g_RFIDData.fnend = 0;
				g_RFIDData.fhs1 = 0;
				g_RFIDData.fhs2 = 0;
				g_RFIDData.fhsf = 0;
				SendRFIDData(STP_ALERT);
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
				rfidReadNow(); //clear the throttle time so that it will read ASAP
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

				g_RFIDData.ulseektime = 0; //FIXME SEVERE
				g_RFIDData.fsstart = 0;
				g_RFIDData.fpstart = 0;
				g_RFIDData.fpend = 0;
				g_RFIDData.fnstart = 0;
				g_RFIDData.fnend = 0;
				g_RFIDData.fhs1 = 0;
				g_RFIDData.fhs2 = 0;
				g_RFIDData.fhsf = 0;
				SendRFIDData(STP_ALERT);
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
			g_szTagDatalen = RFID_FMS00_DATA_SIZE - 1; //using 47 not 48... it sets last to ' ' below
			memcpy(g_szTagData,g_RfidStateAndData.szTagData+2,g_szTagDatalen); //64 + 1
			g_RFIDData.uicrc16 = *(unsigned int *)(g_RfidStateAndData.szTagData+2 + g_szTagDatalen);
			g_szTagData[g_szTagDatalen++]=' ';
			g_szTagData[g_szTagDatalen]=0;
			g_RFIDData.uiendcode = (unsigned int)g_RfidStateAndData.iOmronResponseEndCode;
			logf("rfid: e=%2x d=\"%s\"\r\n", g_RfidStateAndData.iOmronResponseEndCode, g_szTagData);
			//Verify Tag is valid format
			p_c=g_szTagData;
			if (p_c[0]!='0' || p_c[1]!='0') { goto handle_as_bad_rfid_tag_error; }
			//valid tag format we know
			p_c+=3;
			if (*p_c!='T' && *p_c!='B') { goto handle_as_bad_rfid_tag_error; }
			p_c++;
			cindex=0;
			while(cindex<12)
			{
				ck=p_c[cindex++];
				if (ck<'0'||ck>'9') { goto handle_as_bad_rfid_tag_error; }
			}
			//Done checking tag format
			logf("Done ");
			g_RFIDData.cseekstate = RFID_SEEK_SUCCESS;
			SendRFIDData(STP_ALERT);
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

			SendRFIDData(STP_ALERT);
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
		if (g_ConfigData.RFIDConfig.fseekMove1 < 0) { g_cMoveDir = JOGNEG; }
		if (g_ConfigData.RFIDConfig.fseekMove1!=0)
		{
			//Setup move 1
			SetMoveSpeedParamsEven(g_ConfigData.RFIDConfig.fseekSpeed,10);
			#ifdef OUTPUT_RFID_HOME
			logf("M X %s %.4f\r\n","sm1",g_fTargetX);
			#endif
			RunPosition( g_fTargetX, g_fStartY );
		}
		break;
setup_seek_move2:
		g_RFIDData.cseekstate=RFID_SEEK_MOVE2;
		g_fTargetX = g_fStartX + g_ConfigData.RFIDConfig.fseekMove2;
		g_cMoveDir = JOGPOS;
		if (g_fTargetX < g_PosnMode.fLastKnownPosnX) { g_cMoveDir = JOGNEG; }
		if (g_ConfigData.RFIDConfig.fseekMove2!=0)
		{
			//Setup move 2
			SetMoveSpeedParamsEven(g_ConfigData.RFIDConfig.fseekSpeed,10);
			#ifdef OUTPUT_RFID_HOME
			logf("M X %s %.4f\r\n","sm2",g_fTargetX);
			#endif
			RunPosition( g_fTargetX, g_fStartY );
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
		RunPosition( g_fTargetX, g_fStartY );
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
		RunPosition( g_fTargetX, g_fStartY );
		break;
setup_return_move:
		g_RFIDData.cseekstate=RFID_SEEK_RETURN;
		//Setup return move
		g_fTargetX = g_fStartX;
		SetMoveSpeedParamsEven(g_ConfigData.RFIDConfig.fseekSpeed,10);
		#ifdef OUTPUT_RFID_HOME
		logf("M X %s %.4f\r\n","r",g_fTargetX);
		#endif
		RunPosition( g_fTargetX, g_fStartY );
		break;
setup_center_move:
		g_RFIDData.cseekstate=RFID_SEEK_CENTER;
		//Setup return move
		g_fTargetX = g_RFIDData.fposition;
		SetMoveSpeedParamsEven(g_ConfigData.RFIDConfig.fseekSpeed,10);
		#ifdef OUTPUT_RFID_HOME
		logf("M X %s %.4f\r\n","c",g_fTargetX);
		#endif
		RunPosition( g_fTargetX, g_fStartY );
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
nodebug void RFIDRun()
{
	unsigned long ulx;
	td_RFID_F0 * p_rfid_f0;
	td_RFID_F0 RFID_F0Data;
	int i;
	char sprintfbuf[16];
	char * p_cSerialNumber;
	//support immediate and continuous tag reading here...
	//This subsystem could be extended to support more than just RFID, but it currently does not. (a suggestion only)
	if ( g_cReadRFID != RFID_READ_OFF )
	{
		if ( g_cReadRFID == RFID_READ_STOP )
		{
			g_cReadRFID = RFID_READ_OFF;
			goto skip_rfid_read;
		}
		if ( g_cReadRFID == RFID_READ_SEEK )
		{
			if (g_cHomedX != HOME_RUNNING)
			{
				g_cReadRFID = RFID_READ_OFF;
				goto skip_rfid_read;
			}
		}

		if ( g_cReadRFID == RFID_READ_NOW )
		{
			//read right now.. and turn reading off
			SL031LoginSector();
			SL031ReadData();
			g_cReadRFID = RFID_READ_OFF;
		}
		else if ( g_cReadRFID == RFID_READ_CONTINUOUS )
		{
			//for continuous, read when state changes and show.
logf("cni\r\n"); //Continuous Not Implemented
			//Since it's not implemented
			g_cReadRFID = RFID_READ_OFF;
		}
		else if ( g_cReadRFID == RFID_READ_SEEK)
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
//				SendRFIDData(STP_ALERT);
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
	if (g_cImmediateFakeRfid==1)
	{
		g_cImmediateFakeRfid = 0;
		memcpy(g_bRFIDTagData,g_szTagData,g_szTagDatalen);
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
//testpath
				if (g_cRunningMsgGoal != PositionStop) { StopPositionAdvanced(); }
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
			SendRFIDData(STP_ALERT);

			//This is to ensure that this will be set to 1, only after a FindWinOK
			g_cWindowFound=0;

			//DJWR FIXED 20120314-1
			//Should use action code here
			g_cAction = ACTION_HOME;
		}
		if (g_cObstructionCode!=0)
		{
			logf("o s rf\r\n");
			AlertObstructionCode();
			StopPosition();
#warns "TEST THIS PATH and determine if any additional action or alternate code is needed other than letting move fail below"
			g_RFIDData.cseekstate=RFID_SEEK_FAIL_HARDSTOP;
		}
		if (g_RFIDData.cseekstate==RFID_SEEK_INIT)
		{
			//Init RFID Home ... Continued
			//Do not bother checking current move status
			//Setup Move
			//Setup Seek Move 1
			g_RFIDData.cseekstate=RFID_SEEK_MOVE1;
			SetupRFIDSeekMove(g_ConfigData.RFIDConfig.fseekMove1, "sm1");
			break;
		}
		if (g_cMoveDone != MOVEDONE_TRUE)
		{
			//Clear this on every loop through here
			//This is to ensure that this will be set to 1, only after a FindWinOK
			g_cWindowFound=0;
			if (g_cMoveDone == MOVEDONE_FALSE)
			{
				//continue waiting....
				break;
			}
			if (g_cMoveDone == MOVEDONE_ERROR)
			{
logf("mde\r\n");
				if (g_RFIDData.cseekstate==RFID_SEEK_RETURN)
				{
					//A move error on the last move... Can't return to start position
					g_RFIDData.cseekstate=RFID_SEEK_FAIL_HARDSTOP;
				}
				else if ((g_ConfigData.RFIDConfig.uioptions & RFID_OPTION_REVERSE_ON_HARDSTOP)==0)
				{
					//Hard Stop should be treated as an end case
					g_RFIDData.cseekstate=RFID_SEEK_FAIL_HARDSTOP;
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
#warns "//FIXME HIGH should see if this is invovled in RFID Home Stop or if that bypasses the entire thing.. and test STOP"
//FIXME MED .  Don't call a user STOP a hardstop"
logf("mdsc\r\n");
				g_RFIDData.cseekstate=RFID_SEEK_FAIL_HARDSTOP;
				g_cMoveDone = MOVEDONE_TRUE;
			}
		}
		//Move is done
#ifdef OUTPUT_RFID_HOME
		if (g_RFIDData.cseekstate!=RFID_SEEK_CENTER) //because this state loops too many times while checking for final read...
		{ //but for all other states, show md
logf("md\r\n");
		}
#endif

		if (g_cWindowFound == 1 && g_RFIDData.cseekstate != RFID_SEEK_CENTER)
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
		if (g_RFIDData.cseekstate==RFID_SEEK_RETURN)
		{
			//Move Back is done...  This is a failure to find a tag
			g_RFIDData.cseekstate = RFID_SEEK_FAIL_NOT_FOUND;
			//Allow code to continue on to final state handler
		}
		if (g_RFIDData.cseekstate==RFID_SEEK_CENTER)
		{
			//See if RFID Tag Had Login or Was Read Yet
			if (g_cRFIDResult == RFID_RESULT_PENDING)
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
			if (g_cRFIDResult == RFID_RESULT_READ_SUCCESS)
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

				memcpy(g_szTagData,g_bRFIDTagData,g_szTagDatalen);
				g_szTagData[g_szTagDatalen]=0; //even though we don't show it, null this so if it does get printed, it does not fail
				g_RFIDData.uicrc16 = 0; //NA
				g_RFIDData.uiendcode = 0; //NA
				p_rfid_f0 = (td_RFID_F0 *)g_szTagData;
				logf("rfid:\r\n");
				if (p_rfid_f0->cFormat=='0')
				{
					//old omron format means that user has supplied a tag data for rfid sim... so allow it....
					logf("%s\r\n",g_szTagData);
				}
				else
				{
					//newer binary format
					logf("%d\r\n",p_rfid_f0->cFormat);
					logf("%d\r\n",p_rfid_f0->cRailType);
					logf("%d\r\n",p_rfid_f0->cGroup);
					logf("%d\r\n",p_rfid_f0->cSegmentAndRailSide);
					logf("%lu\r\n",ntohul(p_rfid_f0->ulSerialNumber));
					logf("%lu\r\n",ntohul(p_rfid_f0->ulPosition));
					logf("%lu\r\n",ntohul(p_rfid_f0->ulSegmentPosition));
					//Verify Tag is valid format
					if (p_rfid_f0->cFormat!=0) { goto handle_bad_rfid_format; } //It's NOT format 0
					if (p_rfid_f0->cRailType>16)
					{
						//assume rail type and revisions 0 to 16 are valid at this time for this tag... so this one is not valid
						goto handle_bad_rfid_format;
					}
//logf("%d\r\n",1);
					if (p_rfid_f0->cGroup>16)
					{
						//assume groups 0 to 16 are valid... so this one is not valid
						goto handle_bad_rfid_format;
					}
					if ((p_rfid_f0->cSegmentAndRailSide&127)>32)
					{
						//assume segements 0 to 32 are valid... so this one is not valid
						goto handle_bad_rfid_format;
					}
					ulx=ntohul(p_rfid_f0->ulPosition);
					if (ulx>2000000)
					{
						//assume less than 2000 inches is valid... so this one is not valid
						goto handle_bad_rfid_format;
					}
//logf("%d\r\n",4);
					ulx=ntohul(p_rfid_f0->ulSegmentPosition);
					if (ulx>100000)
					{
						//assume less than 100 inches is valid... so this one is not valid
						goto handle_bad_rfid_format;
					}
					if ((p_rfid_f0->cSegmentAndRailSide&128)>0)
					{
						g_iRFIDRailOrientation = 1;
					}
					else
					{
						g_iRFIDRailOrientation = -1;
					}
					//Save the positions that relate RFID Rail Coordinates to the machine.
					//This assumes that RFID offset is given relative to machine primary coords. See RFIDOrient doc.
					g_fRFIDRailX = ((float)p_rfid_f0->ulPosition)/1000.0;  //Record the Tag's Rail Coordinate
					g_fRFIDMachineX = g_PosnMode.fPosnX + g_iRFIDRailOrientation * g_ConfigData.RFIDConfig.fRFIDOffset;
//logf("%f %f %f\r\n",g_PosnMode.fPosnX,g_fRFIDRailX,g_fRFIDMachineX);
#define SIMULATE_OLDER_FORMAT_TAG
#ifdef SIMULATE_OLDER_FORMAT_TAG
					RFID_F0Data=*p_rfid_f0; //copy all struct to this location
					if ((RFID_F0Data.cSegmentAndRailSide&127)>9)
					{
                       // too many digits for conversion
						goto handle_bad_rfid_format;
					}
					//now build older format data
					p_cSerialNumber = g_szTagData+16;
					memset(g_szTagData,'0',16); //zeros at front
					memset(p_cSerialNumber,' ',48); //spaces behind serial
					g_szTagData[2]='0' + RFID_F0Data.cSegmentAndRailSide&127;
					if ((RFID_F0Data.cSegmentAndRailSide&128)>0)
					{
						g_szTagData[3]='T';
					}
					else
					{
						g_szTagData[3]='B';
					}
//logf("%d\r\n",12);
					sprintf(sprintfbuf,"%lu",ntohul(RFID_F0Data.ulPosition));
					i=strlen(sprintfbuf);
					if (i>6)
					{
						goto handle_bad_rfid_format;
					}
					memcpy(g_szTagData+4+6-i,sprintfbuf,i);
					memcpy(g_szTagData+4+6,"072000",6);
					sprintf(p_cSerialNumber,"serial%lu",ntohul(RFID_F0Data.ulSerialNumber));

					g_szTagDatalen = RFID_FMS00_DATA_SIZE;
					g_szTagData[g_szTagDatalen]=0; //null this just to be clear.
					//show the constructed tag
					logf("%s\r\n",g_szTagData);
#endif
				}
				//Done checking tag format
				logf("Done ");
				g_RFIDData.cseekstate = RFID_SEEK_SUCCESS;
				SendRFIDData(STP_ALERT);
				g_cHomedX = HOME_DONE;
				g_cReadRFID = RFID_READ_OFF;
				//Done!
				break;
handle_bad_rfid_format:
				memdump("tag",g_bRFIDTagData,60);
				g_RFIDData.cseekstate = RFID_SEEK_FAIL_BAD_TAG_DATA;
			}
			else if (g_cRFIDResult == RFID_RESULT_ERROR)
			{
				g_RFIDData.cseekstate = RFID_SEEK_FAIL_RFID_ERROR;
			}
			else if (g_cRFIDResult == RFID_RESULT_NO_TAG)
			{
				g_RFIDData.cseekstate = RFID_SEEK_FAIL_NOT_FOUND;
			}
			else if (g_cRFIDResult == RFID_RESULT_READ_FAIL) //really just a bad read in this case
			{
				g_RFIDData.cseekstate = RFID_SEEK_FAIL_BAD_TAG_DATA;
			}
			else
			{
				//any other result is not expected, but callit an RFID ERROR
				g_RFIDData.cseekstate = RFID_SEEK_FAIL_RFID_ERROR;
			}
		}
		if (g_RFIDData.cseekstate<=RFID_SEEK_TERMINAL_STATE)
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
		SendRFIDData(STP_ALERT);
		g_cHomedX = HOME_FAILURE;
		g_cReadRFID = RFID_READ_OFF;
		logf("rfid done fail %d\r\n",g_RFIDData.cseekstate);
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
#warnt "FIXME MED RFID Features are not fully compelted"


#define OUTPUT_DFINT_STATES
#define OUTPUT_PRIMEDELAY
//FIXME dfint.... dfnow
nodebug void DFModeStateMachine(void)
{
    int i;
    //unsigned int uiSTPmsgSize;
    char cEntryState;
    char c;
	char * p_c;
    char * p_sStateName;
    float fDiameter;
    static char s_cDrillStatePrev;
    static char s_cSealStatePrev;
    static char s_cFillStatePrev;

    static unsigned long s_ulStartTime;
    static unsigned long s_ulSealStart;
    static unsigned long s_ulSealPressureOn;
    static unsigned long s_ulFillStart;
    static unsigned long s_ulClampStart;
    static unsigned long s_ulFillFastenerArrive;
	static unsigned long s_ulAccelArmTime;
	static unsigned long s_ulSamples;
    char defaultbuf[12];

	//Alert Out the Goal State
    if (g_cDrillStateGoalSent != g_cDrillStateGoal)
    {
    	logf("Send DSG %d\r\n", g_cDrillStateGoal);
		//send out drill state goal
		SmartToolMsgChar( STP_ALERT, OID_DRILL_STATE, g_cDrillStateGoal);
        g_cDrillStateGoalSent = g_cDrillStateGoal;
    }

#ifdef DRILL_DIRECT_READY
	//DRILL DIRECT CONTROL CODE
	if (g_cSmartDrill==1) //FIXME Optional Component System concept needs improvement
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
				g_pSTPSession = (void *)0; // no limit
				SendTool(STP_ALERT,update);
				g_pSTPSession = p_STPSession; //restore limit
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
			g_cSealantPressure=SEALANT_PRESSURE_OFF; //pressure off
			g_cSealantPinch=SEALANT_PINCH_ON; //pinch
			g_cSealantApply=SEALANT_APPLY_OFF; //tip retracted
            //Set it to AUTOPRIME TO BEGIN AUTO PATH
	        break;
		case SEALSTATE_START_LOAD:
			g_cSealantPressure=SEALANT_PRESSURE_OFF; //pressure off
			g_cSealantPinch=SEALANT_PINCH_ON; //pinch
			g_cSealantApply=SEALANT_APPLY_OFF; //tip retracted
	        s_ulSealStart=MS_TIMER; //start waiting.
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
			g_cSealantPinch=SEALANT_PINCH_OFF; //release pinch
			g_cSealantApply=SEALANT_APPLY_OFF; //tip retracted
	        break;
		case SEALSTATE_PRIME:
        	if (g_cSealantPressure==SEALANT_PRESSURE_OFF)
            {
				g_cSealantPressure=SEALANT_PRESSURE_ON; //pressure on
                s_ulSealPressureOn = MS_TIMER;
            }
			g_cSealantPinch=SEALANT_PINCH_OFF; //release pinch
			g_cSealantApply=SEALANT_APPLY_OFF; //tip retracted
	        break;
		case SEALSTATE_PRESSURE:
        	if (g_cSealantPressure==SEALANT_PRESSURE_OFF)
            {
				g_cSealantPressure=SEALANT_PRESSURE_ON; //pressure on
                s_ulSealPressureOn = MS_TIMER;
            }
			g_cSealantPinch=SEALANT_PINCH_ON; //pinch
			g_cSealantApply=SEALANT_APPLY_OFF; //tip retracted
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
			g_cSealantApply=SEALANT_APPLY_OFF; //tip retracted
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
				g_cSealantApply=SEALANT_APPLY_OFF; //tip retracted
				s_ulSealStart=MS_TIMER; //start waiting.
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
                s_ulSealStart=MS_TIMER; //start waiting for glob now
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
			g_cSealantPressure=SEALANT_PRESSURE_ON; //pressure on
			g_cSealantApply=SEALANT_APPLY_ON; //tip out
			s_ulSealStart=MS_TIMER; //start waiting.
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
			g_cSealantPressure=SEALANT_PRESSURE_OFF; //pressure off
			g_cSealantPinch=SEALANT_PINCH_ON; //pinch
			g_cSealantApply=SEALANT_APPLY_OFF; //tip retracted
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


nodebug void SetupRFIDSeekMove(float fmovedelta, char * label)
{
	g_fTargetX = g_fStartX + fmovedelta;
	//Setup Move to target
	SetMoveSpeedParamsX(g_ConfigData.RFIDConfig.fseekSpeed, 16, 16);
	#ifdef OUTPUT_RFID_HOME
	logf("M X %s %.4f\r\n",label,g_fTargetX);
	#endif
	RunFindWindowX( g_fTargetX );
}

//FIXME0000000000 relocate

#ifdef HOMESYSTEM
nodebug void ClearAllHomeStatus()
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

nodebug void ClearSentHomeStatus()
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

nodebug void SetAllHomeStatusPending()
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

nodebug void AlertHomeStatus()
{
	//Send back only Main Home Status
	SmartToolMsgChar(STP_ALERT, OID_HOME, g_cHomed);
    return;
}

nodebug void AlertHomeReport(char axis_code, char status, char status_reason)
{
	//Send back Axis Specific Home Report.
	td_oid_home_report oid_home_report;
	oid_home_report.caxis_code = axis_code;
	oid_home_report.cstatus = status;
	oid_home_report.cstatus_reason = status_reason;
	SmartToolMsg(STP_ALERT, OID_HOME_REPORT, sizeof(td_oid_home_report),(char*)&oid_home_report);
    return;
}
#endif

nodebug void SelectHomePositionAndPositionLimits()
{
	if (g_ConfigData.cToolFlip!=Y_NEG)
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
nodebug int LookupDiameterPrimeDelay(float fdiameter)
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
void main(void)
{
	int i;
	unsigned long ulTime;
    char * p;

    brdInit();

	#ifdef SOCKETCONSOLE
    InitSocketConsole();
    #endif

    InitDIO();  // initialize DIO as soon as possible

	#ifdef SHOW_MEM
    show_memory();
    #endif

	SockInit();

    // wait until enet is initialized b/c we need enet before we can connect below
	InitEthernet();

	#ifdef SOCKETCONSOLE
	InitListenSocketConsole();
	*g_szSerialNumber=0;//null this since it's not initialized yet, and socket console shows serial
    //If Debugging, wait 15 seconds or until we have a SocketConsole
	//logf("Waiting up to 15 seconds for SocketConsole\r\n");
    //SocketConsoleWaitConnect(15000);
    #endif
	logf( SMARTTOOL_SUBTYPE_STRING " " SYSTEM_VERSION_STRING );
    logf( "\r\n" );

	#ifdef USE_BOOTLOADER
	UDPDL_Init( SMARTTOOL_SUBTYPE_STRING " " SYSTEM_VERSION_STRING );  // for ethernet bootloader
	#endif

    //light on indicates booting
	#ifdef LEDSYSTEM
    g_cLED = 1;
	digOut(DIGOUT_CHNUM_LED, 1);
    #endif

    //beeper is off
	#ifdef BEEPSYSTEM
    g_cBeep = 0;
	digOut(DIGOUT_CHNUM_BEEPER, 0);
    #endif

	InitXmemStorage();

	#ifdef PROFILEPOINTS
    InitProfiling();
    #endif
    InitVars();
    InitConfig();

    //Init Motion Control System : First Call for base initialization only
	MemInitMC();
	#ifdef FORCELIMITING
	ReadForceInitMem();
	#endif

    #ifdef USE_TIMERB
    TimerBInit();
	#endif

    initCopyHexToByte();

    InitADC();
    // get user block data
    UBreadConfig();

    //Now that Config is loaded setup some boot time defaults
    g_cEEOption=g_ConfigData.cEEOptionDefault;
	g_cProbeMethod = g_ConfigData.cProbeMethodDefault;

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
	InitMC();
    StpInit();

	#ifdef HD_RAIL_STP
    InitRailSTP();
	#endif

    //now turn off light... soon the main loop will take control of the light
	#ifdef LEDSYSTEM
    g_cLED = 0;
    digOut(DIGOUT_CHNUM_LED, 0);
    #endif

    //Do Boot Beep after this point (config may have beep off)
    BeepBoot();

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
	SL031OpenAndConfigPort();
	#endif

	#ifdef SOCKETCONSOLE
//    logf("Switching logf via printf OFF\r\n");
//    _cPrintfOutput = 0;
    #endif

    //MAIN SYSTEM LOOP
	//	Main loop timing data in comments below was captured on an oscilloscope using Whistle motion control boards.
    //	Timing with a Galil motion control board may differ.
    ulTime=MS_TIMER;
	// Shortest main loop duration = 1.6ms; typical 3.0 ms if busy. (BL2100 20Mhz)

    while(1)
    {
		// kick the TCP engine
        tcp_tick(NULL);		// 	Min. duration 100 usec.
        					//	Typical duration when active: 150 - 200 usec.
                            //	Worst case 1.2 msec.

		#ifdef SOCKETCONSOLE
		CheckSocketConsole();
		RxSocketConsole();

//FIXME Minor  Clear this test code out in the future.
#if 0
#warns "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ TEMP RFID TEST CODE"
RFIDRun();
g_cHold=1;
#ifdef USE_BOOTLOADER
		while (UDPDL_Tick())  // this should be called at least twice a second.
        {
   	    	//boot loader is operating
        }
#endif
#endif

		if (g_cHold) { goto show_io; }
		#endif

		//RUN_DIO
		ReadDigitalInputs();			//	Duration: 100 usec average, 120 usec worst case.

	    DigOutService();//Duration New version not timed... est < 75 usec average

		//RUN_ADC
	    ReadADC();			//	Duration typical 45 usec, 150 usec worst case.
		#ifdef FORCELIMITING
		ReadForceSensor();
		#endif

		//MOTION CONTROL SERVICE
		//Syscheck, Communication, and Monitor all in one location.
		ServiceMC();		// 	Min. duration 320 usec.
        					//	Duration when active typically 600 - 800 usec.
                            //	Worst case duration: 1.5ms Whistle / 2.5 msec. Galil occurs every 8-12 passes.

		//Second call to debounce
		ReadDigitalInputs();			//	Duration: 100 usec average, 120 usec worst case.

		//STP CHECK AND RX
        ulTime=MS_TIMER-ulTime;
        if (ulTime>500) { logf("Non Eth=%lu\r\n",ulTime); }

        ServiceSTP();

        ulTime=MS_TIMER;

		#ifdef HD_RAIL_STP
        ServiceRailSTP();
        #endif

        #ifdef DIRECT_LASER_SENSOR_CONNECT
		ServiceLaserSensor();
		#endif
        #ifdef DIRECT_CVCAM_CONNECT
		#ifdef CENTERVISION_CAM
		ServiceCam();
		#endif
		#endif

        tcp_tick(NULL);  // kick the TCP engine

		//Handle Mode Specific Ops, Control State Machines, and other inlines services
        ModeStateMachine();   	//	Duration: Minimum 220 usec; 420 usec in E-STOP
        						//	500 usec if active; Worst case ~ 1.5 msec.

		DFModeStateMachine();

show_io:
		if (g_cShowIO==1)
        {
        	ShowIO();
        }
		if (g_cShowAna==1)
        {
        	ShowAna();
        }
    } //continue loop
}

nodebug void ServiceNow()
{
	int i;
	//call to debounce
	ReadDigitalInputs();			//	Duration: 100 usec average, 120 usec worst case.
	ServiceMC();		// 	Min. duration 320 usec.
    ServiceSTP();

	#ifdef SOCKETCONSOLE
	CheckSocketConsole();
	RxSocketConsole();
	#endif

    tcp_tick(NULL);  // kick the TCP engine
}
//	FIXME:	Check if conflicts exist between TimerB functions and analog inputs.
//	FIXME:  Calibrate soft timer in TimerB functions against scope readings.
//	TimerBStart();		//	Software elapsed timer.
//digOut(DIGOUT_CHNUM_TIME_SIGNAL, 1 );	//	Set debug digital signal for subsection duration checks.
//TEST HERE
//digOut(DIGOUT_CHNUM_TIME_SIGNAL, 0 );	//	Clear debug digital output
//	TimerBReset();		//	Stop soft timer, capture elapsed time since TimerBStart.
//	logf( " --> %ld usec. \r\n", g_lTBelapsed_usec );

////////////////////////////////////////////////////////////////////////////
// Part Program Loading / Parsing functions
////////////////////////////////////////////////////////////////////////////

//ClearPartProgram
//Load the Part Program structures with empty values
nodebug void ClearPartProgram()
{
	int i;
    g_iHoleCount = 0;
    g_PartPgmInfo.iNumVisited=0;
	g_PartPgmInfo.cLocked=1; //leave it locked until they clear from the teach screen
    g_PartPgmInfo.cTeachModeData=0;
    g_PartPgmInfo.cMiniFtFormat = 0;
    g_PartPgmInfo.cChecksum=0;
	memset(g_PartPgmInfo.p_cChecksum16,0,16);
    g_cPartPgmStatus=PP_NOTLOADED;
    g_PartPgmInfo.cErrorCount=0; //reset error count

    //make sure that storage is present
    if (g_cMemoryAllocated == 0)
    {
		LogParseError(MINIFTMC_PPERR_XMEM_NOT_ALLOCATED);
    }

	g_cToolCount = 0; //clear
    g_cProcessCount = 0;
    g_cFastenerCount = 0;
    g_cPatternCount = 0;
    g_iHoleCount = 0;
    g_cKHoleCount = 0;

	g_cRequireParameterRecognition = 0; //clear options

	g_cKHoleHoleIndexSet=0;

	//erase the reference to the data
	xp_ToolNames = 0;
	g_iToolNamesLen = 0;

	//erase the reference to the data
	xp_ProcessLayerDataBase = 0;
	xp_ProbeCommandDataBase = 0;

	//Program Options
	g_fAssumeApproxDataSetOrientation=-1; //-1 means it's not in use
	g_cAssumeApproxPositionsFromDataset=0;
	g_cRequireParameterRecognition=0;

	//ClearGolbalRotation(); //just clear two variables
	g_cRotationKP = 0;
	g_cRotationKS = 0;
	g_cRotationContext = 0;

	//Clear Probe Completely
	InitProbeValues();//does not send probe status back. this happens later.

    //also reset any job parameters
    StartOver();//SPSALL

    //And reset any which are sticky across jobs, but not programs
    //FIXME medium : should expand on this concept making it more fool proof;

	//	For rivet programs only - just to make sure values are not crazy if the data is missing from the part program:
	g_PartPgmInfo.K1OriginDistX = 0; 		// in inches, X offset of K1 from track origin bumper
    g_PartPgmInfo.K1EndDistX = 10.0;		// in inches, X offset of K1 from track end bumper
    g_PartPgmInfo.K1OriginDistY = 0; 		// in inches, Y offset of K1 from center of X track

    //Clear Scaling
	g_cScaleBasedOnProbe = 0; //And Turn Off Scaling

	//Program does not have default tool flip anymore...
	//A program either has tool flip settings or it does not adjust what is currently on the tool
	//g_ConfigData.cToolFlip = Y_POS;

	//Clear Options
	g_fAssumeApproxDataSetOrientation=-1; //-1 means it's not in use  0>= x < 360 means it's in use
	g_cAssumeApproxPositionsFromDataset=0;
	g_cRequireParameterRecognition=0;

	//Clear Lube Bypass
	g_cLubeBypass = 0;

    //Stop SendPPData
	g_cSendPPDataSessions = 0;
    //Clear PP Data
	g_ulPartProgramDataLength = 0;

	g_cBlockCount = 0;
}

nodebug void ParsePartProgramStart()
{
	char cSuccess;
	char *s;
    g_cPartPgmStatus=PP_LOADING;

    //init stats
    // COUNTING ASSUMPTION: Input file enums have enum id in order starting at 0, and will use a range of numbers,
    //						such that nothing will be skipped from 0 to N-1, where there are N enums.
    //						This is the only possible output from the PPparser.pl

	//first clear
	ClearPartProgram();

    if (g_PartPgmInfo.cErrorCount>0)
    {
    	//If the clear didn't even have zero errors, then return here.
	    g_cPartPgmStatus=PP_NOTLOADED;
		goto return_partpgmstatus;
    }

	cSuccess=1;
    if (g_sPPDirect==0)
    {
   		//we only support direct data now, but keep this error as-is for now
	   	g_cPartPgmStatus=PP_NOTLOADED;
		LogParseError(MINIFTMC_PPERR_FAILURE_TO_OPEN_FILE);
		goto return_partpgmstatus;
	}
	logf("file %s\r\n",g_szPartPgmFilename);//debug

    //Set Parse Section
    g_cBeingParsed = PARSE_SECTION;

    //Now Ready To Read The File In
	//The main loop will see g_cPartPgmStatus=PP_LOADING, and will repeat calls the the next function
    g_cPartPgmStatus=PP_LOADING;
	logf("Loading\r\n");
return_partpgmstatus:
	SmartToolMsgChar(STP_ALERT, OID_PARTPGM_STATUS, g_cPartPgmStatus);
}

//ParsePartProgramContinue
// cMaxLinesPerContinue
//   X  the number of lines to read each time the continue is called... as a char type max is 255
//   0  allow all the lines to be parsed right now.... same as blocking version
//FIXME00000000 return nodebug when done
nodebug void ParsePartProgramContinue(char cMaxLinesPerContinue)
{
    //local vars
    char *s;
    float fx,fy,fd;
	char cKIndex;
    char cPKIndex;
    char cSKIndex;
    char cPattern;
    char cToolFlip;
    char cToolType;
    char cProcess;
    char cFastener;
    unsigned int d2uDiameter;
    unsigned int uiOps;
    unsigned int uiStack;
    int icsnkadj;
	int i;
	unsigned int ui;
	unsigned int uiRemain;
	unsigned long ul;
	unsigned long ulRemain;
    char	*p_c;
	char tempproc[64];
	char tempproclen;
    char c;
	char cl;

    int iStartPos;
    char cLoopNow;

	static unsigned int s_uiSBN;
	static unsigned int s_uiSB;
	static unsigned long s_ulSBN;
	static unsigned long s_ulSB;

    static char s_cKHoleIndexRowSize;
	//static char s_cToolRowSize;
	static char s_cProcessRowSize;
    static char s_cHoleRowSize;
    static char s_cKHoleRowSize;
    static char s_cMaterialStackRowSize;
    static char s_cMaxMaterialLayers;
    static char s_cBytesOpField;
    static unsigned long s_ulP3DIV;

    static char s_cCount;
    static unsigned int s_uiCount;

	td_tool_data_fileformat * p_tool_data;
	td_process_data * p_process_data;
	td_hole_data_fileformat * p_hole_data_f;

    td_HoleData xHoleData;
    int ihole;

    static char s_cKHoleIndex;
    char checksumbuffer[16];

    if (g_cPartPgmStatus!=PP_LOADING)
    {
    	//should not be here and may not be prepared to parse, so return
        return;
    }

//   	szCurLine=szCurLineBuffer;

   	if (g_sPPDirect==0)
	{
		logf("ERROR:g_sPPDirect=0\r\n");
	}

loop_now:
    iStartPos = g_iPPDirectPos;
	cLoopNow = 0;
	while (1)
    {
       	//reading new format

        //continue to parse the file
		s=g_sPPDirect + g_iPPDirectPos; //The current buffer
		i=g_iPPDirectEnd - g_iPPDirectPos;
		switch (g_cBeingParsed)
        {
        	case PARSE_SECTION:
        		//looking for a section header.
        		//FIXME0000 consider requirement of section order but for now: parsing this way
        		if (i<2)
        		{
        			break; //come back when more data to process.
        		}
				g_iPPDirectPos+=2;
				s_uiSBN = 0;
				s_uiSB = 0;
                s_cCount = 0;
                s_uiCount = 0;
				s_ulSBN = 0;
				s_ulSB = 0;
        		logf("ps: %d %d\r\n",s[0],s[1]);//FIXME00000000 test
                c=s[0];
                if (c!=0)
                {
                	logf("Warning: Section Header MSB is not 0\r\n");
                }
        		c=s[1];
                g_cBeingParsed = c; //state is the same as the section code
                if (g_PartPgmInfo.cMiniFtFormat == 0)
                {
                	if (c!=SECTION_Header)
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
                    g_PartPgmInfo.p_cChecksum16[0]=0; //Header section
                    g_PartPgmInfo.p_cChecksum16[1]=1; //Header section
                }
				if (c==SECTION_Invalid)
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
				if (s_uiSBN==0)
				{
        			if (i<2)
        			{
	        			break; //come back when more data to process.
	        		}
					g_iPPDirectPos+=2;
					s_uiSBN=*((int *)s);
                    //And For Header, copy the 2 size bytes into the checksum buffer for init
                    g_PartPgmInfo.p_cChecksum16[2]=s[0]; //
                    g_PartPgmInfo.p_cChecksum16[3]=s[1]; //
                    //SPECIAL PATTERN
                    //Ensure that this data is processed now before content starts.
                    //Since Checksum is not on yet it will ignore the 1st 4 characters, but we saved them
                    //above so we can use them later.
                    //The data will be writen out to the PPData storage buffer.
                    cLoopNow = 1;
					break;
				}
        		if (i<s_uiSBN)
        		{
        			break; //come back when more data to process.
        		}
        		logf("%s Hv %ub\r\n","H",s_uiSBN);
				g_iPPDirectPos+=s_uiSBN;

                c=s[0];//major version
                if (c!=0x01)
                {
					goto minift_format_version_fail;
                }
                c=s[1];//minor version
				g_cFormatMinorVersion = c;
                if (c<0x02)
                {
minift_format_version_fail:
                	logf("Ver Fail:%d\r\n",c);
					LogParseError(MINIFTMC_PPERR_NOT_MINIFT_FORMAT);
					g_cBeingParsed = PARSE_FAIL;
					continue; //will not return to any states other than FAIL
                }
				c=s[2];
                g_PartPgmInfo.cChecksum=c;
                if (c==0)
                {
                	//ignore it: This is not recommended, but we support this.
                }
#ifdef MD5HASH
                else if (c==1)
                {
					//MD5
                	md5_init( &g_md5_state );

                    //We stored the 1st 4 bytes into this buffer.
                    //before processing the section content.
                    //Add them to the checksum now.
					md5_append( &g_md5_state, g_PartPgmInfo.p_cChecksum16, 4 );
                    //The rest of the data will be added the normal way when it leaves
                    //the function and processes all the data waiting
				}
#else
#warnt "MD5HASH is not defined: There can be no checking of MD5"
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
                	//a Kind of checksum that is not currently supported
					LogParseError(MINIFTMC_PPERR_UNSUPPORTED_CHECKSUM_TYPE);
				}

        		//Parse more...

                c=s[3]; //Char Encoding
                if (c==1) //ASCII
                {
                }
                else if (c==2) //UTF-8
                {
                	logf("!UTF-8 not tested");
                }
                else
                {
					LogParseError(MINIFTMC_PPERR_UNSUPPORTED_CHAR_ENCODING);
                }

				//just print
				c=s[s_uiSBN];
				s[s_uiSBN]=0;
				logf("text = \"%s\"\r\n",s+4);
                s[s_uiSBN]=c; //restore buffer...

		        g_cBeingParsed = PARSE_SECTION;
		        continue;

            case SECTION_Options:
				if (s_uiSBN==0)
				{
        			if (i<2)
        			{
	        			break; //come back when more data to process.
	        		}
					g_iPPDirectPos+=2;
					s_uiSBN=*((int *)s);
					if (s_uiSBN==0)
					{
				        g_cBeingParsed = PARSE_SECTION;
					}
					continue;
				}
        		if (i<s_uiSBN)
        		{
        			break; //come back
        		}

        		logf("%s Hv %ub\r\n","op",s_uiSBN);
				g_iPPDirectPos+=s_uiSBN;

				if (g_cFormatMinorVersion>=3) //for version 1.3 and up using binary option format
				{
        		//Parse more...
				ui=0;
				while(ui<s_uiSBN)
				{
                    fx=0;
					fy=0;
					c=s[ui++];
					cl=s[ui++];
					if (c==0)
					{
						break;
					}
					if (c==1)
					{
						//AssumeApproxDataSetOrientation fd
						fd=*((float *)(s+ui));	ui+=4;
						if (fd<0)
						{
							fd+=360;
						}
						if (fd<0 || fd>=360)
						{
							fd=0;
						}
						g_fAssumeApproxDataSetOrientation = fd;
                    	fx=fd;
						goto show_sbo;
						continue;
					}
					if (c==2)
					{
						//KHoleApproxPosition cindex,fx,fy
						cl=s[ui++];//use this char for cindex
						fx=*((float *)(s+ui));	ui+=4;
						fy=*((float *)(s+ui));	ui+=4;
						g_cKHolePrbeStart[cl]=PS_APPROXIMATE;
						g_cKHolePrbeStatus[cl]=PS_APPROXIMATE;
						g_cKHolePrbeStatusWarnings[cl]=0; //clear warning
						g_fKHolePrbeStartX[cl]=fx;
						g_fKHolePrbeStartY[cl]=fy;
						g_fKHolePrbeX[cl]=fx;
						g_fKHolePrbeY[cl]=fy;
//FIXMENOW test
						goto show_sbo;
						continue;
					}
					if (c==3)
					{
						//AssumeApproxPositionsFromDataset
						g_cAssumeApproxPositionsFromDataset = (s[ui]=='1'); ui++;
//FIXMENOW test
						goto show_sbo;
						continue;
					}
					if (c==4)
					{
						//DefaultSelectedOperations uint16
						uiOps=*((unsigned int *)(s+ui)); ui+=2;
						fx=uiOps; //show this variable
						g_ConfigData.uiProcessOperations = uiOps; //set this
						SmartToolMsgUInt(STP_ALERT, OID_PROCESS_OPERATIONS, uiOps);
						goto show_sbo;
						continue;
					}
					if (c==5)
					{
						//ProgramInspectionInterval uint16
						uiOps=*((unsigned int *)(s+ui)); ui+=2;
						if (uiOps>250)
						{
							uiOps=250;
						}
						g_ConfigData.cVisionAutoInspect=uiOps;
//FIXME
//Integrate into existing OID and test that OID....
						fx=uiOps; //show this variable
						goto show_sbo;
						continue;
					}
					if (c==6)
					{
						//RequireParameterRecognition
						g_cRequireParameterRecognition = (s[ui]=='1'); ui++;
						goto show_sbo;
						continue;
					}
					if (c==7)
					{
						//KHoleApproxRailPosition cindex,fx,fy
						cl=s[ui++];//use this char for cindex
						fx=*((float *)(s+ui));	ui+=4;
						fy=*((float *)(s+ui));	ui+=4;

						if (g_cHomedX != HOME_DONE || g_cHomedY != HOME_DONE)
						{
							//just log this error
							LogParseError(MINIFTMC_PPERR_XY_NOT_HOMED);
        				}
						else
						{
							#ifdef HOMESYSTEM_X_RFID
							//Convert fx from Rail to machine coordinates
							fx = g_fRFIDMachineX + g_iRFIDRailOrientation*(fx - g_fRFIDRailX);
							#else
							//just allow Home Coordinates to be Equal to rail coordinates
							#endif

							g_cKHolePrbeStart[cl]=PS_APPROXIMATE;
							g_cKHolePrbeStatus[cl]=PS_APPROXIMATE;
							g_cKHolePrbeStatusWarnings[cl]=0; //clear warning
							g_fKHolePrbeStartX[cl]=fx;//use RFID
							g_fKHolePrbeStartY[cl]=fy;//after Y Home, Y machine position is the rail position
							g_fKHolePrbeX[cl]=fx;
							g_fKHolePrbeY[cl]=fy;
						}
//FIXMENOW test
						goto show_sbo;
						continue;
					}
					//unrecognized option...
					ui+=cl; //move forward the length of the data
show_sbo:
logf("SBO %d %d %f %f\r\n",c,cl,fx,fy);
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
				if (s_uiSBN==0)
				{
        			if (i<2)
        			{
	        			break; //come back when more data to process.
	        		}
					g_iPPDirectPos+=2;
					s_uiSBN=*((int *)s);
					if (s_uiSBN==0)
					{
				        g_cBeingParsed = PARSE_SECTION;
					}
					continue;
				}
       			if (i<s_uiSBN)
       			{
        			break; //come back when more data to process.
        		}
        		logf("%s Hv %ub\r\n","co",s_uiSBN);
				g_iPPDirectPos+=s_uiSBN;
                //use this data
                g_cToolCount=s[0]; //number of tools
                g_cProcessCount=s[1]; //number of processes
                g_cFastenerCount=s[2]; //number of fasteners
                g_cPatternCount=s[3]; //number of patterns
                g_iHoleCount=*((int *)(s+4)); //number of holes
                g_cKHoleCount=s[6]; //number of KHoles
                s_cKHoleIndexRowSize=s[7];
                //s_cToolRowSize=s[8]; //bytes per tooltype data row //v1.1 only... cut out from v1.2+
                s_cProcessRowSize=s[8]; //bytes per process data row
                s_cHoleRowSize=s[9]; //bytes per hole data row
                s_cKHoleRowSize=s[10]; //bytes per khole data row
                s_cMaterialStackRowSize=s[11]; //FIXME Incomplete  Material Stack Not Being Used
                s_cMaxMaterialLayers=s[12];
                s_cBytesOpField=s[13]; //bytes per op field
				s_ulP3DIV=*((unsigned long*)(s+14));
                logf("p3div = %lu\r\n",s_ulP3DIV);

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
				if (s_uiSBN==0)
				{
        			if (i<2)
        			{
	        			break; //come back when more data to process.
	        		}
					s_uiSBN=*((int *)s);
					g_iPPDirectPos+=2;
                    if (s_uiSBN==0)
                    {
						g_cBeingParsed = PARSE_SECTION;
                    }
					//The Position In the file is the data we have saved, plus how far DirectPos has now moved since iStartPos
					//Anything Prior to iStartPos is included in g_ulPartProgramDataLength because it has been saved.
					xp_ToolNames = xp_PartProgramData + g_ulPartProgramDataLength + g_iPPDirectPos - iStartPos;
					g_iToolNamesLen = s_uiSBN;
					continue;
				}
                if (i==0)
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
				g_iPPDirectPos+=i;

        		if (s_uiSB==s_uiSBN)
        		{
        			//have enough
	        		logf("%s Hv %ub\r\n","t",s_uiSBN);
			        g_cBeingParsed = PARSE_SECTION;
        		}
		        continue;
            case SECTION_ProcessNames:
				if (s_uiSBN==0)
				{
        			if (i<2)
        			{
	        			break; //come back when more data to process.
	        		}
					s_uiSBN=*((int *)s);
					g_iPPDirectPos+=2;
                    if (s_uiSBN==0)
                    {
						g_cBeingParsed = PARSE_SECTION;
                    }
					continue;
				}
                if (i==0)
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
				g_iPPDirectPos+=i;

        		if (s_uiSB==s_uiSBN)
        		{
        			//have enough
	        		logf("%s Hv %ub\r\n","p",s_uiSBN);
			        g_cBeingParsed = PARSE_SECTION;
        		}
		        continue;
            case SECTION_FastenerTypeNames:
				if (s_uiSBN==0)
				{
        			if (i<2)
        			{
	        			break; //come back when more data to process.
	        		}
					s_uiSBN=*((int *)s);
					g_iPPDirectPos+=2;
                    if (s_uiSBN==0)
                    {
						g_cBeingParsed = PARSE_SECTION;
                    }
					continue;
				}
                if (i==0)
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
				g_iPPDirectPos+=i;

        		if (s_uiSB==s_uiSBN)
        		{
        			//have enough
	        		logf("%s Hv %ub\r\n","fa",s_uiSBN);
			        g_cBeingParsed = PARSE_SECTION;
        		}
		        continue;
            case SECTION_PatternNames:
				if (s_uiSBN==0)
				{
        			if (i<2)
        			{
	        			break; //come back when more data to process.
	        		}
					s_uiSBN=*((int *)s);
					g_iPPDirectPos+=2;
                    if (s_uiSBN==0)
                    {
						g_cBeingParsed = PARSE_SECTION;
                    }
					continue;
				}
                if (i==0)
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
				g_iPPDirectPos+=i;

        		if (s_uiSB==s_uiSBN)
        		{
        			//have enough
	        		logf("%s Hv %ub\r\n","pat",s_uiSBN);
			        g_cBeingParsed = PARSE_SECTION;
        		}
		        continue;
            case SECTION_HoleNames:
				if (s_ulSBN==0)
				{
        			if (i<4)
        			{
	        			break; //come back when more data to process.
	        		}
					s_ulSBN=*((unsigned long *)s);
					g_iPPDirectPos+=4;
                    if (s_ulSBN==0)
                    {
						g_cBeingParsed = PARSE_SECTION;
                    }
					continue;
				}
                if (i==0)
                {
        			break; //come back when more data to process.
                }
				//copy all of this into a special location up to what we need.
				//copy happens below where the entire input stream is preserved
				ulRemain = s_ulSBN - s_ulSB;
				if (i > ulRemain)
				{
					i = (int)ulRemain;
				}
				//move

				//add them
				s_ulSB += i;
				g_iPPDirectPos+=i;

        		if (s_ulSB==s_ulSBN)
        		{
        			//have enough
	        		logf("%s Have all %lu bytes.\r\n","h",s_ulSBN);
			        g_cBeingParsed = PARSE_SECTION;
        		}
		        continue;
			case SECTION_KHoleIndex:
				if (s_uiSBN==0)
				{
        			if (i<2)
        			{
	        			break; //come back when more data to process.
	        		}
					s_uiSBN=*((int *)s);
                    s_uiSB=g_cKHoleCount * s_cKHoleIndexRowSize;
					g_iPPDirectPos+=2;
                    if (s_uiSB!=s_uiSBN)
                    {
                    	logf("Bad len %s","khi");
						LogParseError(MINIFTMC_PPERR_SECTION_SIZE_DIFF);
                        g_cBeingParsed = PARSE_FAIL;
                        continue; //will not return to any states other than FAIL
                    }
					//s_uiSB = 0; //no need
					g_cKHoleHoleIndexSet=1;
					g_iKHoleHoleIndex[0]=0; //clear that first unused entery
                    if (g_cKHoleCount==0)
                    {
				        g_cBeingParsed = PARSE_SECTION;
                    }
					continue;
				}
           	    if (i<2)
       	        {
   	            	break; //come back
                }
				//Ignore this data... just let it write into the stored data section
                //add bytes
				//s_uiSB += s_cKHoleRowSize; no need
				g_iPPDirectPos += 2;

				//record index
				c=s_cCount+1;
				ihole=*((int *)s);
				logf("k %d %d\r\n",c,ihole);
				//save this to the index
                g_iKHoleHoleIndex[c]=ihole;

				//Count
                s_cCount++;
                if (s_cCount==g_cKHoleCount)
                {
                	logf("All %s\r\n","khi");
			        g_cBeingParsed = PARSE_SECTION;
        		}
				continue;
			case SECTION_ProcessData:
				if (s_uiSBN==0)
				{
        			if (i<2)
        			{
	        			break; //come back when more data to process.
	        		}
					s_uiSBN=*((int *)s);
                    s_uiSB=g_cProcessCount * s_cProcessRowSize;
					g_iPPDirectPos+=2;
                    if (s_uiSB!=s_uiSBN)
                    {
                    	logf("Bad len %s","pd");
						LogParseError(MINIFTMC_PPERR_SECTION_SIZE_DIFF);
                        g_cBeingParsed = PARSE_FAIL;
                        continue; //will not return to any states other than FAIL
                    }
					//s_uiSB = 0; no need
                    if (g_cProcessCount==0)
                    {
				        g_cBeingParsed = PARSE_SECTION;
                    }
					continue;
				}
                if (i<s_cProcessRowSize)
                {
                	break; //come back
                }
                //read process
                p_process_data = (td_process_data *)s;

                c=s_cCount+1;

logf("proc%d l=%d m=%d lbs=%d ops=%d depth=%d\r\n",
					c,
					p_process_data->layers,
					p_process_data->material,
					p_process_data->clamplbs,
					p_process_data->ops,
                	p_process_data->countersinkDepth);

logf(" pl=%d w=%d a=%d hsa=%d\r\n",
					p_process_data->proclayers,
					p_process_data->clampwarnlbs,
					p_process_data->clampalarmlbs,
					p_process_data->hardstopamps); //Times 100 for 2 digits of decimal
				logf(" ps %d \r\n",
							p_process_data->procstyle);

				if (c < MAXNUM_PROCESSES)
				{
					g_cProcessLayerCount[c]=p_process_data->layers;
					g_cProcessMaterials[c]=p_process_data->material;
					g_uiProcessPounds[c]=p_process_data->clamplbs;
					g_cProcessOps[c]=p_process_data->ops;
                	g_uiProcessCountersink[c]=p_process_data->countersinkDepth;
					g_cProcessProcLayerCount[c]=p_process_data->proclayers;
					g_uiProcessPoundsWarning[c]=p_process_data->clampwarnlbs;
					g_uiProcessPoundsAbort[c]=p_process_data->clampalarmlbs;
					g_uiProcessHardstopAmps[c]=p_process_data->hardstopamps;
					g_cProcessStyle[c]=p_process_data->procstyle;
                }
//FIXME00000000000000000000
                 //Should just check to see that sections are all present

                //add bytes
				//s_uiSB += s_cProcessRowSize; no need
				g_iPPDirectPos += s_cProcessRowSize;
				//Count
                s_cCount++;
                if (s_cCount==g_cProcessCount)
                {
                	logf("All %s\r\n","pd");
			        g_cBeingParsed = PARSE_SECTION;
        		}
                continue;
			case SECTION_ProcessLayerData:
			case SECTION_ProcessLayerData_Alt:
				if (s_uiSBN==0)
				{
        			if (i<2)
        			{
	        			break; //come back when more data to process.
	        		}
					s_uiSBN=*((int *)s);
					logf("pld SBN %u\r\n",s_uiSBN);
					g_iPPDirectPos+=2;
                    if (s_uiSBN==0)
                    {
						g_cBeingParsed = PARSE_SECTION;
                    }
					continue;
				}
                if (i==0)
                {
        			break; //come back when more data to process.
                }
				if (s_uiSB==0)
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
                    memcpy(((char*)g_uiProcessLayerDataIndex),(unsigned int*)s,i);
					//count these bytes as used
					s_uiSB += i;
					g_iPPDirectPos+=i;
#define SHOW_PLD
#ifdef SHOW_PLD
i=0;
while(i<=g_cProcessCount+1) //p0 is just a place holder and the last entery is just so the length of the last processes data can be known.
{
	logf("p %d ind %d\r\n",i,g_uiProcessLayerDataIndex[i]);
	i++;
}
#endif
					//The Position In the file is the data we have saved, plus how far DirectPos has now moved since iStartPos
					//Anything Prior to iStartPos is included in g_ulPartProgramDataLength because it has been saved.
					xp_ProcessLayerDataBase = xp_PartProgramData + g_ulPartProgramDataLength + g_iPPDirectPos - iStartPos;
#ifdef SHOW_PLD
					logf("xp_pld=%ld\r\n",xp_ProcessLayerDataBase);
#endif
					continue;
				}

				//copy all of this into a special location up to what we need.

				uiRemain = s_uiSBN - s_uiSB;
				logf("pld SBN %u SB %u  rem %u\r\n",s_uiSBN,s_uiSB,uiRemain);
				if (i > uiRemain)
				{
					i = uiRemain;
				}
				//move

				//add them
				s_uiSB += i;
				g_iPPDirectPos+=i;

        		if (s_uiSB==s_uiSBN)
        		{
        			//have enough
	        		logf("%s Hv %ub\r\n","pld",s_uiSBN);
			        g_cBeingParsed = PARSE_SECTION;
        		}
		        continue;
            case SECTION_HoleData:
				if (s_ulSBN==0)
				{
        			if (i<4)
        			{
	        			break; //come back when more data to process.
	        		}
					g_iPPDirectPos+=4;
					s_ulSBN=*((unsigned long *)s);
                    s_ulSB=((unsigned long)g_iHoleCount) * s_cHoleRowSize;
//FIXME Minor clean this temp code size limit of s_cHoleRowSize"
#define USE_TEMP_WARNING_FOR_s_cHoleRowSize
#ifdef USE_TEMP_WARNING_FOR_s_cHoleRowSize
					if (s_cHoleRowSize < 19 )
                    {
						LogParseError(MINIFTMC_PPERR_SECTION_SIZE_DIFF);
                        g_cBeingParsed = PARSE_FAIL;
                    	continue;
                    }
#endif
                    if (s_ulSB!=s_ulSBN)
                    {
                    	logf("Bad len %s","hd");
						LogParseError(MINIFTMC_PPERR_SECTION_SIZE_DIFF);
                        g_cBeingParsed = PARSE_FAIL;
                        continue; //will not return to any states other than FAIL
                    }
					//s_ulSB = 0; no need
                    if (g_iHoleCount==0)
                    {
				        g_cBeingParsed = PARSE_SECTION;
                    }
					//make sure index was set
					if (g_cKHoleCount>0)
					{
						if (g_cKHoleHoleIndexSet==0)
						{
							LogParseError(MINIFTMC_PPERR_SECTION_SIZE_DIFF); //not a specific error for this
    	                    g_cBeingParsed = PARSE_FAIL;
	                        continue; //will not return to any states other than FAIL
						}
					}
                    //set this counter
                    s_cKHoleIndex=1;
					continue;
				}
                if (i<s_cHoleRowSize)
                {
                	break; //come back
                }
                //read hole data

				p_hole_data_f=(td_hole_data_fileformat *)s;

                p_c=(char *)&ul;

                //FlatX
                ul=*((unsigned long *)&p_hole_data_f->flatx);
                if ((p_c[2] & 128) == 0)
                {
					p_c[3]=0; //clear MSB....
                }
                else
                {
                	p_c[3]=0xFF; //Clear MSG or sign extended MSB
                }
                fx=(long)ul; //it is signed....
                fx=fx/s_ulP3DIV;

                //FlatY
                ul=*((unsigned long *)&p_hole_data_f->flaty);
                if ((p_c[2] & 128) == 0)
                {
					p_c[3]=0; //clear MSB....
                }
                else
                {
                	p_c[3]=0xFF; //sign extended
                }
                fy=(long)ul; //it is signed....
                fy=fy/s_ulP3DIV;

				//cKIndex
				cKIndex = 0;

				//Primary KIndex
                cPKIndex=p_hole_data_f->ki_primary;
   				//Secondary KIndex
                cSKIndex=p_hole_data_f->ki_secondary;
                //Diameter
				d2uDiameter=p_hole_data_f->diameter;
                //Tool
				cToolType=p_hole_data_f->tooltype;
                //Process
				cProcess=p_hole_data_f->process;
                //Fastener
				cFastener=p_hole_data_f->fastener;
                //Pattern
                cPattern=p_hole_data_f->pattern;
				//Operations
               	uiOps=p_hole_data_f->ops; //FIXME same old width issue
                uiStack=p_hole_data_f->depthstack;
                icsnkadj=p_hole_data_f->countersink_adj;

				if (g_cFormatMinorVersion<=2) //for version 1.3 and up, they must set cPattern+=128 for K Holes that are not inline
				{
    	            if (cPKIndex==0)
	                {
						cPattern+=128;
        			}
					uiOps |= 1; //always allow access by default.. In 1.3 this bit can prevent hole access.
				}
                //store
				ihole = s_uiCount;

                //FIXME Test test perormance without logfs in this section to see load time effect.
                //Leave Off if not needed
				logf("i%d p%d s%d d%d t%d p%d fas%d pat%d o%d\r\n",ihole,cPKIndex,cSKIndex,d2uDiameter,cToolType,cProcess,cFastener,cPattern,uiOps);

	            //Check If this is a KHole Using the new approach
				if (s_cKHoleIndex<=g_cKHoleCount)
				{
					logf("lk %d ni %d i %d\r\n",s_cKHoleIndex,g_iKHoleHoleIndex[s_cKHoleIndex],ihole);
					if (ihole==g_iKHoleHoleIndex[s_cKHoleIndex])
					{
						logf("fik %d ni %d i %d\r\n",s_cKHoleIndex,g_iKHoleHoleIndex[s_cKHoleIndex],ihole);
						cKIndex=s_cKHoleIndex++;
					}
				}

                //Set special X mem arrays to hold this data.
                if (ihole<MAXNUM_POSNS)
                {
					xsetfloat(xp_fRawDataX+4*ihole,fx);
					xsetfloat(xp_fRawDataY+4*ihole,fy);
					xsetfloat(xp_fRawDataR+4*ihole,0); //start it at zero.
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
                    setholedata(ihole,&xHoleData);

					//we are currently using a fixed pitch of 16 to allow easy multiplication of ihole,
					//but we must ensure that there is enough room for the struct in case future development adds fields
					#if 16 < sizeof(td_HoleData)
					#fatal "Fixed Pitch of 16 is less than td_HoleData Size"
					#endif
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
                if (s_uiCount==g_iHoleCount)
                {
                	logf("All %s\r\n","hd");
                    if (s_cKHoleIndex-1 != g_cKHoleCount)
                    {
                    	logf("FAILURE found %d k but kc = %d\r\n",s_cKHoleIndex-1,g_cKHoleCount);
                        //FIXME MED HIGH better error
						LogParseError(MINIFTMC_PPERR_SECTION_SIZE_DIFF);
                        g_cBeingParsed = PARSE_FAIL;
                        continue; //will not return to any states other than FAIL
                    }
			        g_cBeingParsed = PARSE_SECTION;
        		}
                continue;
			case SECTION_KHoleData:
				if (s_uiSBN==0)
				{
        			if (i<2)
        			{
	        			break; //come back when more data to process.
	        		}
					s_uiSBN=*((int *)s);
                    s_uiSB=g_cKHoleCount * s_cKHoleRowSize;
					g_iPPDirectPos+=2;
                    if (s_uiSB!=s_uiSBN)
                    {
                    	logf("Bad len %s","khd");
						LogParseError(MINIFTMC_PPERR_SECTION_SIZE_DIFF);
                        g_cBeingParsed = PARSE_FAIL;
                        continue; //will not return to any states other than FAIL
                    }
					//s_uiSB = 0; no need
                    if (g_cKHoleCount==0)
                    {
				        g_cBeingParsed = PARSE_SECTION;
                    }
					continue;
				}
           	    if (i<s_cKHoleRowSize)
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
                if (s_cCount==g_cKHoleCount)
                {
                	logf("All %s\r\n","khd");
			        g_cBeingParsed = PARSE_SECTION;
        		}
				continue;
                //probe control field
				//This new section contains mainly the information in the program control field
			case SECTION_ProbeControl:
				if (s_uiSBN==0)
				{
        			if (i<2)
        			{
	        			break; //come back when more data to process.
	        		}
					s_uiSBN=*((int *)s);
					//logf("khi SBN %u\r\n",s_uiSBN);
					g_iPPDirectPos+=2;
                    if (s_uiSBN==0)
                    {
						logf("nd\r\n");
						g_cBeingParsed = PARSE_SECTION;
                    }
					continue;
				}
                if (i==0)
                {
					//logf(".\r\n");
        			break; //come back when more data to process.
                }
				//logf("i=%d\r\n",i);


				if (s_uiSB==0)
				{
					logf("buf = %d %d\r\n", s[0], s[1]);
					//The Position In the file is the data we have saved, plus how far DirectPos has now moved since iStartPos
					//Anything Prior to iStartPos is included in g_ulPartProgramDataLength because it has been saved.
					xp_ProbeCommandDataBase = xp_PartProgramData + g_ulPartProgramDataLength + g_iPPDirectPos - iStartPos;
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
				g_iPPDirectPos+=i;

        		if (s_uiSB==s_uiSBN)
        		{
        			//have enough
	        		logf("%s Hv %ub\r\n","kpc",s_uiSBN);
			        g_cBeingParsed = PARSE_SECTION;
        		}
		        continue;
            case SECTION_MaterialStack:
				if (s_uiSBN==0)
				{
        			if (i<2)
        			{
	        			break; //come back when more data to process.
	        		}
					s_uiSBN=*((int *)s);
					g_iPPDirectPos+=2;
					//FIXME MED Potentially I could do a check to see if the max used index is covered
                    //Check to see if there is enough memory
                    if (s_uiSBN > MAXSIZE_MATERIALS)
                    {
	   					LogParseError(MINIFTMC_PPERR_OVER_RESERVED_PPMEM);
				        g_cBeingParsed = PARSE_FAIL;
                    }
					s_uiSB = 0;
                    if (s_uiSBN==0)
                    {
						logf("nd\r\n");
				        g_cBeingParsed = PARSE_SECTION;
                    }
					continue;
				}
                if (i==0)
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
                root2xmem(xp_MaterialData + s_uiSB, s, i);
				//add them
				s_uiSB += i;
				g_iPPDirectPos+=i;

        		if (s_uiSB==s_uiSBN)
        		{
        			//have enough
	        		logf("%s Hv %ub\r\n","mat",s_uiSBN);
			        g_cBeingParsed = PARSE_SECTION;
        		}
                continue;
            case SECTION_StackData:
				if (s_uiSBN==0)
				{
        			if (i<2)
        			{
	        			break; //come back when more data to process.
	        		}
					s_uiSBN=*((int *)s);
					g_iPPDirectPos+=2;
                    //Check to see if there is enough memory
                    if (s_uiSBN > MAXSIZE_STACKDATA)
                    {
	   					LogParseError(MINIFTMC_PPERR_OVER_RESERVED_PPMEM);
				        g_cBeingParsed = PARSE_FAIL;
                    }
					s_uiSB = 0;
                    if (s_uiSBN==0)
                    {
						logf("nd\r\n");
				        g_cBeingParsed = PARSE_SECTION;
                    }
					continue;
				}
logf("hv %d\r\n",i);
                if (i==0)
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
                root2xmem(xp_StackData + s_uiSB, s, i);
				//add them
				s_uiSB += i;
				g_iPPDirectPos+=i;

        		if (s_uiSB==s_uiSBN)
        		{
        			//have enough
	        		logf("%s Hv %ub\r\n","sta",s_uiSBN);
			        g_cBeingParsed = PARSE_SECTION;
        		}
                continue;
            case SECTION_SynonymousHole:
				if (s_uiSBN==0)
				{
        			if (i<2)
        			{
	        			break; //come back when more data to process.
	        		}
					s_uiSBN=*((int *)s);
					g_iPPDirectPos+=2;
					s_uiSB = 0;
                    if (s_uiSBN==0)
                    {
				        g_cBeingParsed = PARSE_SECTION;
                    }
					continue;
				}
                if (i==0)
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
				g_iPPDirectPos+=i;

        		if (s_uiSB==s_uiSBN)
        		{
        			//have enough
	        		logf("%s Hv %ub\r\n","syn",s_uiSBN);
			        g_cBeingParsed = PARSE_SECTION;
        		}
                continue;
            case SECTION_Checksum:
				if (s_uiSBN==0)
				{
        			if (i<2)
        			{
	        			break; //come back when more data to process.
	        		}
					g_iPPDirectPos+=2;
					s_uiSBN=*((int *)s);
					logf("Checksum\r\n");
                    if (s_uiSBN!=16)
                    {
                    	logf("Bad len %s","check");
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
                if (i<16)
                {
                	//SPECIAL PATTERN:
                    //Do not let this data be put into the checksum
                    //Advance this number so that noting from this data will be put in.
                    iStartPos = g_iPPDirectPos;
                    break;
                }
				if (g_PartPgmInfo.cChecksum>0)
                {
                	//Both MD5 and MDARC4 use 16 byte checksum
                    memcpy(g_PartPgmInfo.p_cChecksum16,s,16);
#ifdef MD5HASH
					if (g_PartPgmInfo.cChecksum==1)
					{
						md5_finish( &g_md5_state, checksumbuffer ); //use line buffer to store 16 char hash (can't use direct data buffer because this stores data after the end of the position.)
						if (memcmp(checksumbuffer,g_PartPgmInfo.p_cChecksum16,16)!=0)
						{
							//different
							LogParseError(MINIFTMC_PPERR_CHECKSUM_FALURE);
						}
					}
					#ifdef MDARC4
                    else
                    #endif
#endif
#ifdef MDARC4
					if  (g_PartPgmInfo.cChecksum==2)
                    {
                    	//FIXME MED LOW  complete ARC4 path and test speed improvement
                    }
#endif
                }
				g_iPPDirectPos+=16;
				i=g_iPPDirectEnd - g_iPPDirectPos;
                if (i>0)
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
                logf("Unrec Section=%d\r\n",g_cBeingParsed);
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
	s=g_sPPDirect + iStartPos; //The start of the buffer sent to us
	i=g_iPPDirectPos - iStartPos; //The bytes from the start to where we parsed
	//logf("rev %d = %d - %d\r\n",i,g_iPPDirectPos,iStartPos);
	if (i>0)
    {
#ifdef MD5HASH
			if (g_PartPgmInfo.cChecksum==1)
			{
				md5_append( &g_md5_state, s, i );
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
   	if (g_ulPartProgramDataLength + i > MAX_PARTPGM_STORAGE)
    {
		LogParseError(MINIFTMC_PPERR_OVER_RESERVED_PPMEM);
        g_cBeingParsed = PARSE_FAIL;
    }
    else
    {
logf("@@@wr %lu %d\r\n",g_ulPartProgramDataLength,i);
	    root2xmem(xp_PartProgramData+g_ulPartProgramDataLength , s, i);
		g_ulPartProgramDataLength += i;
	}

    if (cLoopNow==1)
    {
    	goto loop_now;
    }
	//logf("ret\r\n");
    return; //come back later


complete_file_load:

	//Clear Op History
	ClearOpHistory();
	g_iActionHoleIndex = -1;

//FIXME HIGH  Should this be fatal, or should this be accepted if no processes, or what????
//	if (xp_ProcessLayerDataBase == 0)
//	{
//		//no layer data...
//	}

#ifdef SHOW_PLD
i=0;
while(i<=g_cProcessCount+1) //p0 is just a place holder and the last entery is just so the length of the last processes data can be known.
{
	logf("p %d ind %d\r\n",i,g_uiProcessLayerDataIndex[i]);
	i++;
}
#endif
logf(".\r\n");
	//Since Tool Map was reloaded, correct the loaded tool id
	if(g_szToolIDlen==0)
	{
		g_cLoadedTool=0;
	}
	else
	{
		g_cLoadedTool = LookupToolTypeCode(g_szToolTypeCode);
	}
	//and any tool searched previously may differ now
	g_cLastSearchedRequiredTool = 0;

logf("pdpcd\r\n");
    PreviewDisplayProbeCommandDataBase();

	if (g_cAssumeApproxPositionsFromDataset==1)
	{
		c=1;
		while(c<=g_cKHoleCount)
		{
			if (g_cKHolePrbeStart[c]!=PS_APPROXIMATE) //If they used another method to have these set during the options section, leave those values
			{
				g_cKHolePrbeStart[c]=PS_APPROXIMATE;
				g_cKHolePrbeStatus[c]=PS_APPROXIMATE;
				g_cKHolePrbeStatusWarnings[c]=0; //clear warning
				i=g_iKHoleHoleIndex[c];
				fx = xgetfloat(xp_fRawDataX+4*i);
				fy = xgetfloat(xp_fRawDataY+4*i);
				g_fKHolePrbeStartX[c]=fx;
				g_fKHolePrbeStartY[c]=fy;
				g_fKHolePrbeX[c]=fx;
				g_fKHolePrbeY[c]=fy;
			}
			c++;
		}
		SendAllProbeStart(STP_ALERT);//SPSALL
		SendAllProbeStatus(STP_ALERT);//SPSALL
		AlertProbeStatusUpdate();
//zxcvzxcv

	}
logf("cpeo\r\n");
	CreateProbeEvaluationOrder();

//I'm not sure what this applies to anymore:
//FIXME9999999999999  we never check the length of this and the memory size we have for it.

logf("che\r\n");
    if (g_PartPgmInfo.cErrorCount==0)
    {
    	//before alerting program is loaded, send out the name and checksum
		SmartToolMsgStr(STP_ALERT, OID_PARTPGM_NAME, g_szPartPgmFilename);
		SmartToolMsg(STP_ALERT, OID_PARTPGM_CHECKSUM, 16, g_PartPgmInfo.p_cChecksum16);

	    g_cPartPgmStatus=PP_LOADOK;
    }
    else
    {
	    g_cPartPgmStatus=PP_PARSEFAIL;
    }
	SmartToolMsgChar(STP_ALERT, OID_PARTPGM_STATUS, g_cPartPgmStatus);

   	//Now, in the case of failure, and after the status has been sent echo the 1st error
    if (g_PartPgmInfo.cErrorCount==0)
    {
    	PartProgramShowStatus();
    }
    else
    {
		SmartToolMsgMiniFtMessageCode(OID_PARTPGM_STATUS, g_PartPgmInfo.p_uiErrorMessages[0]);
   		PartProgramShowStatusPart2();
    }

    #ifdef OUTPUT_PROGRAM_MEM_USAGE
    //This does not show every kind of memory used, but displays the usage of
    //certain areas that could potentially by exceeded.
    logf("Prog Mem:\r\n");
    logf(" %lu data bytes used / %lu available\r\n", g_ulPartProgramDataLength, MAX_PARTPGM_STORAGE);
    #endif

	logf("  K1OD X=%.3f Y=%.3f EDX=%.3f\r\n",
    		g_PartPgmInfo.K1OriginDistX, g_PartPgmInfo.K1OriginDistY, g_PartPgmInfo.K1EndDistX );

    return;
}

//Part Program Abstracted Section Function
nodebug void PartProgramShowStatus()
{
	logf("PP OK %s:\r\n",g_szPartPgmFilename);
    logf("T=%d P=%d F=%d Pa=%d H=%d\r\n",
				g_cToolCount, g_cProcessCount, g_cFastenerCount, g_cPatternCount,  g_iHoleCount);
	//for (iTemp = 0 ; iTemp < g_iHoleCount; iTemp++)
	//{
	//		logf("%.5f %.5f \r\n",xgetfloat(xp_fRawDataX+4*iTemp) , xgetfloat(xp_fRawDataY+4*iTemp));
	//}
}

nodebug void PartProgramShowStatusPart2()
{
	int i;
    logf("PP FAIL status=%d errs=%d\r\n",g_cPartPgmStatus, g_PartPgmInfo.cErrorCount);//debug
	i = 0;
	while ( i < g_PartPgmInfo.cErrorCount)
    {
        logf(" #%d:%d\r\n",i,g_PartPgmInfo.p_uiErrorMessages[i]);
		i++;
    }
}

//Part Program Supporting Functions

//Add an error message to the part program structures error list
nodebug void LogParseError(unsigned int uiMessageCode)
{
	g_PartPgmInfo.p_uiErrorMessages[g_PartPgmInfo.cErrorCount]=uiMessageCode;
    g_PartPgmInfo.cErrorCount++;
}


//FIXME minor  move to lib

nodebug void CreateProbeEvaluationOrder()
{
	char c;
	//assume that they are in order...
	//assume everything requires a probe

	c = 1;
	while (c <= g_cKHoleCount)
	{
		g_cKHolePrbeEvalDCount[c]=1;
		g_cKHolePrbeEvalOrder[c]=c;
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

nodebug void RecalculateLocatingDirectives(char ki)
{
	if (ki==0)
	{
		//The initial Recalculation....
		//FIXMENOW00 FUTURE: Don't have any types which are not probe supported as of this moment so there is never any progress.
		return;
	}


	return;
}


//// TEACH MODE

nodebug void CaptureTeachPosition()
{
	int i;
	int index;
	int floatindex;
    int sr;
    char * s;
    char * nullafter;
    char c;
	float fx;
	float fy;
    td_HoleData xHoleData;

    if (g_PartPgmInfo.cTeachModeData==0)
    {
    	//not a teach mode program;
        return; //just return without adding //FIXME add alert
    }

    index=g_iHoleCount;

	fx = g_TeachMachinePosn.fX;
	fy = g_TeachMachinePosn.fY;
	//There is no name for these positions.
	//There is no part program

    if (index==0)
    {
    	//First Hole
		g_fKHolePrbeX[1]=fx;
		g_fKHolePrbeY[1]=fy;
        g_cKHolePrbeStatus[1]=PS_PROBED;
		g_cKHolePrbeStatusWarnings[1]=0; //clear warning
    }
	else
	{
		g_fKHolePrbeX[2]=fx;
		g_fKHolePrbeY[2]=fy;
		g_cKHolePrbeStatus[2]=PS_PROBED;
		g_cKHolePrbeStatusWarnings[2]=0; //clear warning
	}

    //save the position data
	#ifdef OUTPUT_TEACH
    logf("TEACH X=%.4f Y=%.4f into Hole %d\r\n",fx,fy,index); //debug code
	#endif

	floatindex = index*4;

	//X DATA
	xsetfloat(xp_fRawDataX+floatindex,fx);

    //Y DATA
	xsetfloat(xp_fRawDataY+floatindex,fy);

	//MX DATA
	xsetfloat(xp_fRawDataMX+floatindex,fx);

    //MY DATA
	xsetfloat(xp_fRawDataMY+floatindex,fy);

    //radius to half way to nearest point (stored square of true value)
	xsetfloat(xp_fRawDataR+floatindex,0); //start at 0

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

	setholedata(index,&xHoleData);

    //Init Operation History for this point to Zero
    xsetint(xp_uiOpHistory+2*index,0);

    //add to data point count
    g_iHoleCount++;

    //blink for capture
    LEDCount(3, 200, 400);
    //and beep one time
    Beep();

   	g_ulPartProgramDataLength = 0; //Don't set this until we complete at the end
	//Set the Position Data Here
	xsetfloat(xp_PartProgramData + 6 + 8*index, fx ); //no compression + no conversion... send as float
	xsetfloat(xp_PartProgramData + 10 + 8*index, fy ); //no compression + no conversion... send as float

	ResetNearestPosition();
	g_uiCreateMachineDataSetHashIndex = 0; //Ensure that it won't be used the entire time during capture
	return;
}

nodebug void DeletePosition()
{
	if (g_iHoleCount==0)
    {
        return;
    }
    //decrement data points
    g_iHoleCount--;
	return;
}

nodebug void CompleteTeachPosition()
{
	if (g_iHoleCount == 0)
	{
		return;
	}

	//Calculate the used length of the data
   	g_ulPartProgramDataLength = 6 + 8*g_iHoleCount;

	//Set the Header, Length, and Count
	xsetint(xp_PartProgramData, 0x6000); //special header
	xsetint(xp_PartProgramData + 2, (unsigned int)(g_ulPartProgramDataLength - 4));
	xsetint(xp_PartProgramData + 4, g_iHoleCount);
	//The rest is all data that was set above

	//modify the checksum
	*(unsigned long *)(g_PartPgmInfo.p_cChecksum16)=MS_TIMER;
	*(int *)(g_PartPgmInfo.p_cChecksum16+4)=g_iHoleCount;

  	//before alerting program is loaded, send out the name and checksum
	SmartToolMsgStr(STP_ALERT, OID_PARTPGM_NAME, g_szPartPgmFilename);
	SmartToolMsg(STP_ALERT, OID_PARTPGM_CHECKSUM, 16, g_PartPgmInfo.p_cChecksum16);

    g_cPartPgmStatus=PP_LOADOK;
	SmartToolMsg(STP_ALERT, OID_PARTPGM_STATUS,1,&g_cPartPgmStatus);

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


nodebug void LoadLinearProgram()
{
//FIXME MED LOW  Load Linear Program
//This code removed to  old_linear_program.txt
    return;
}

nodebug void InitXmemStorage()
{
	long maxnum2;
	long maxnum4;
    long maxnum16;

	g_xallocerrors = 0;
	g_cMemoryAllocated = 0;

    maxnum2 = (long)MAXNUM_POSNS * (long)2;
    maxnum4 = (long)MAXNUM_POSNS * (long)4;
    maxnum16 = (long)MAXNUM_POSNS * (long)16;

    //allocate memory for RawDataX
    xp_fRawDataX=xallocsafedetail("RawDataX",maxnum4);

    //allocate memory for RawDataY
    xp_fRawDataY=xallocsafedetail("RawDataY",maxnum4);

    //allocate memory for RawDataMX
    xp_fRawDataMX=xallocsafedetail("RawDataMX",maxnum4);

    //allocate memory for RawDataMY
    xp_fRawDataMY=xallocsafedetail("RawDataMY",maxnum4);

    //allocate memory for RawDataR
    xp_fRawDataR=xallocsafedetail("RawDataR",maxnum4);

    //allocate memory for HoleData Structures
    xp_tdHoleData = xallocsafedetail("HoleData",maxnum16);

    //allocate material storage space
    xp_MaterialData = xallocsafedetail("MaterialData",MAXSIZE_MATERIALS);

    //allocate stack storage space
    xp_StackData = xallocsafedetail("StackData",MAXSIZE_STACKDATA);

    //allocate memory for operation history bit flag array.
    xp_uiOpHistory=xallocsafedetail("OpHistory",maxnum2);

	//CLOSEST_HOLE_SYSTEM
	xp_uiMPosnHashList=xallocsafedetail("MPosnHashList",maxnum2);

    //Text Data Storage System
    //allocate memory for position name string refs
    xp_PartProgramData=xallocsafedetail("PartProgData",MAX_PARTPGM_STORAGE);
    //And initialize the length to 0
    g_ulPartProgramDataLength = 0;

	//Not Allocated but a reference.. (clear here anyway)
	xp_ProcessLayerDataBase = 0;
	xp_ProbeCommandDataBase = 0;

    //I have only about 50000 more as of 3-11-2011
    //xptest=xallocsafedetail("xptest",40000);

    if (g_xallocerrors == 0)
    {
   		g_cMemoryAllocated = 1;
    }
}


nodebug void ClearOpHistory()
{
	char c;
    //Clear OpHistory Array for this part program.
    xmemset(xp_uiOpHistory,0,2*g_iHoleCount);
	g_cBlockCount = (g_iHoleCount+127) >> 7;//must get full block for any part of the last 128, so add 127
logf("BC=%d\r\n",g_cBlockCount);
	c=0;
	while(c<OPHISTORYBLOCKCOUNT) //since there are only OPHISTORYBLOCKCOUNT (40 when using 5000 holes arrays), clear all spots
	{
		g_cOpHistoryBlockOrder[c]=c;
		g_ulOpHistoryBlockTime[c]=0;
		c++;
	}
}

nodebug void AddOpHistory(int index,unsigned int uiOperation)
{
	long xp;
	unsigned int ops;
	unsigned int newops;
	char cBlock;
	char c;
	char clen;
	char cMRUPosition;
	char *p_cdst;
	char *p_csrc;
    if (index<0)
	{
		return;
	}
	xp=xp_uiOpHistory + 2*index;
	ops=(unsigned int)xgetint(xp);
	newops = ops | uiOperation;
#ifdef OUTPUT_OPHIST
logf("SetOp i=%d ops %u => %u\r\n",index,ops,newops);
#endif
	ops=newops;
	xsetint(xp,ops);
    //Now Alert this operation via the OP_STARTED (we do this always)
	if (uiOperation!= OP_PROBE_WARNINGS) //don't bother sending OP_PROBE_WARNINGS back one at a time Pendant will request update after probe.
	{
    	SmartToolMsgUInt(STP_ALERT, OID_OP_STARTED, uiOperation);
	}
	//I currently am NOT sending OP_HISTORY back after an update,
	//and I am not putting a timestamp on OID_OP_STARTED right now.
	//I am keeping it simple, but when it reconnect it will have a low time for the last update
	//and it will end up getting more blocks.... This is OK.  The system is efficient.
	//I should at some point add the timestamp to OP_STARTED....

	if (g_cBlockCount == 0)
	{
		//must be no program loaded
		return;
	}

	cBlock = index >> 7;
	if (cBlock>=g_cBlockCount)
	{
		//too high
		logf("bcth\r\n");
		return;
	}
	g_ulOpHistoryBlockTime[cBlock]=SEC_TIMER; //set the time

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
	while(cBlock != g_cOpHistoryBlockOrder[c])
	{
		if (c==0)
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
	p_cdst=g_cOpHistoryBlockOrder+c;
	p_csrc=p_cdst+1;
	clen = cMRUPosition-c;
	memcpy(p_cdst,p_csrc,clen); //move everything else down
	g_cOpHistoryBlockOrder[cMRUPosition]=cBlock; //move this block to the MRU position
	//Note: there is and O(1) speed algorithym to do this using double linked lists thata requires at least 2 40 char arrays,
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

nodebug void SetOpHistory(int index,unsigned int uiOperations)
{
	long xp;
	//Set is less common than Add, therefore, share code and make Set Use Add.
	//To Set we just set the memory to the operations, but then call add anyway to take care of the rest.
	//That Add ends up doing an extra OR operation, but we hardly use SetOpHistory
	xp=xp_uiOpHistory + 2*index;
	xsetint(xp,uiOperations);
	AddOpHistory(index,uiOperations);
}

nodebug unsigned int GetOpHistory(int index)
{
	long xp;
	xp=xp_uiOpHistory + 2*index;
	return (unsigned int)xgetint(xp);
}

nodebug void SetDrillDir(int iProbeDir)
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
nodebug void InitProbeValues()
{
	char c;
logf("IPV\r\n");
    c=1;
    while(c<=MAXNUM_KHOLES) //Must Clear them all
    {
		g_fKHolePrbeX[c]=0;
		g_fKHolePrbeY[c]=0;
        g_cKHolePrbeStatus[c]=PS_NO_PROBE;
		g_cKHolePrbeStatusWarnings[c]=0; //clear warning
		g_cKHolePrbeStatusDistance[c]=0;
		g_fKHolePrbeStartX[c]=0;
		g_fKHolePrbeStartY[c]=0;
		g_cKHolePrbeStart[c]=PS_NO_PROBE;
        c++;
    }
	g_cProbeDistanceErrors = 0;
    g_cProbeComplete = 0;
	g_cProbeCalculated = 0;
	//Do not send them back, but depend on the caller to know when to do this.
	//The Only Place this is used Immediately calls another functon which does a reset at the start
}

nodebug void ResetProbeValues()
{
	char c;
logf("RPV\r\n");
    c=1;
    while(c<=MAXNUM_KHOLES)
    {
		g_fKHolePrbeX[c]=0;
		g_fKHolePrbeY[c]=0;
        g_cKHolePrbeStatus[c]=PS_NO_PROBE;
		g_cKHolePrbeStatusWarnings[c]=0; //clear warning
		g_cKHolePrbeStatusDistance[c]=0;
		if (g_cKHolePrbeStart[c]==PS_APPROXIMATE)
		{
			//leave it
		}
		else
		{
			//reset it
			g_fKHolePrbeStartX[c]=0;
			g_fKHolePrbeStartY[c]=0;
			g_cKHolePrbeStart[c]=PS_NO_PROBE;
		}
        c++;
    }
	g_cProbeDistanceErrors = 0;
    g_cProbeComplete = 0;
	g_cProbeCalculated = 0;
	//Do not send them back, but depend on the caller to know when to do this.
	//The Places this is used will call another function which will finally send the updated status back.
}

nodebug void SendProbeValues()
{
    td_STPsessions * p_STPSession;
	p_STPSession = g_pSTPSession; //backup
	g_pSTPSession = (void *)0; //no limit
	//Alert All Probe Start and Status
	SendAllProbeStart(STP_ALERT);//SPSALL
	SendAllProbeStatus(STP_ALERT);//SPSALL
	AlertProbeStatusUpdate();
    g_pSTPSession = p_STPSession; //restore limit
}

nodebug void CheckProbeComplete()
{
	char ck;
	ck=1;
	while(ck<=g_cKHoleCount)
	{
		if (g_cKHolePrbeStatus[ck]<PS_PROBED)
		{
           	//not all are probed
            g_cProbeComplete = 0;
			return;
		}
		if (g_cKHolePrbeStatusDistance[ck]>0)
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
nodebug void StartOver()
{
	//Does not clear all job parameters, but requires reprobe at least.
    //This clears whatever is needed when they select "Start Over".
    //If it does not clear until they load a new program, then it should go into
    // ClearPartProgram() instead.
logf("so\r\n");

    ResetProbeValues();
	RecalculateLocatingDirectives(0);
	SendProbeValues();//SPSALL

	//ClearGolbalRotation(); //just clear two variables
	g_cRotationKP = 0;
	g_cRotationKS = 0;
	g_cRotationContext = 0;

	//Clear DataSetIDTimes
	g_ulMachineDataSetIDTimeSec = 0;
	g_ulMachineDataSetIDTime = 0;
	//Alert new MachineDataSetID now
	SendXYDataID(STP_ALERT);
	//Stop SendXYData
	g_cSendXYDataSessions = 0;
	//Stop Sending Op History
	g_cSendOpHistory = 0;

	memset(g_cSendXYDataSession,0,MAXNUM_STP_SESSIONS);
	memset(g_cSendOpHistoryBlockOrderIndex,0xFF,MAXNUM_STP_SESSIONS);
	ClearOpHistory();
	g_iActionHoleIndex = -1;

//FIXME MED Pattern
    //g_iChosenPattern=0;

	g_uiActionCycles=0;

    g_cAllowKVisit=0;
   	g_cAutoMove=0;
	g_ulStartAutoTime=0;
	g_cAutoRepeat=0;

    ResetNearestPosition();
	g_uiCreateMachineDataSetHashIndex = 0;

	g_cToolLoaded = 0;
	g_cOverrideCalculated = 0;
	g_cProcessLoaded = 0;
	g_cLoadedProcess = 0;
	g_cDrillLoadProcessAndOverride = DRILL_LOAD_PROCESS_NOT_NEEDED;

    g_MachineOffset.fX=0;
    g_MachineOffset.fY=0;

	g_Probe.cRegistration = REGISTRATION_UNKNOWN;
    SmartToolMsgChar(STP_ALERT, OID_PROBE_REGISTRATION, g_Probe.cRegistration);

#ifdef DRILL_DIRECT_READY
	SmartDrillSTPSet(SMARTDRILL_OID_FAULT_CLEAR);
#endif
	g_DrillFault.cSeverity = 0;

#ifdef FASTENER_STP
	FastenerSTPSet(FASTENER_OID_FAULT_CLEAR);
	g_FastenerFault.cSeverity = 0;

	FastenerSTPSetUInt(FASTENER_OID_FASTENER_LOADED,0);//Clear this when starting over...
#endif
	g_ConfigData.uiProcessOperations = OP_DRILL | OP_SEAL | OP_FILL | OP_REMOVE | OP_INSPECT; //set this to the main set...
	SmartToolMsgUInt(STP_ALERT, OID_PROCESS_OPERATIONS, g_ConfigData.uiProcessOperations);


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
nodebug void StartSendPartProgramData(int iSession, unsigned long ulStart )
{
	char c;
	//Must have the file loaded.  Can't be loading the file at the same time
	if (g_cPartPgmStatus != PP_LOADOK)
    {
		return;
	}

	if (ulStart > g_ulPartProgramDataLength)
    {
    	ulStart=0;
    }

    //Get ready to send the list.
	g_cSendPPDataSession[iSession] = 1;
    g_ulSendPPDataIndex[iSession] = ulStart;

    //Find the count of sessions....
    c=0;
    g_cSendPPDataSessions = 0;
    while(c<MAXNUM_STP_SESSIONS)
    {
    	if (g_cSendPPDataSession[c] == 1)
        {
	    	g_cSendPPDataSessions++;
        }
        c++;
    }
	#ifdef OUTPUT_SENDPPDATA
    logf("PPDATA %lu\r\n",ulStart);//FIXME0 can remove soon
	#endif
    //Now there is a section in the main mode machine which will
    //continue calling the function below to send the file back
    return;
}

nodebug void ContinueSendPartProgramData()
{
	unsigned long ulx;
    unsigned int ui;
    char * p_buffer;
    char c;
    char cfound;
    char cfinal;
    char csendcount;
    unsigned long ulStart;

	//Do not check here, but check before call
    //if (g_cSendPPDataSessions == 0)
    //{
	// 	//nothing is being sent
    //    return;
    //}

	p_buffer = g_STPtxMsg.p_cObjectValue;

    c=0;
    cfound=0;
    while(c<MAXNUM_STP_SESSIONS)
    {
    	if (g_cSendPPDataSession[c] == 1)
        {
        	ulStart = g_ulSendPPDataIndex[c];
			if (ulStart>=g_ulPartProgramDataLength)
			{
            	//seems like we sent it all
            	g_cSendPPDataSession[c]=0;
				#ifdef OUTPUT_SENDPPDATA
		        logf("sa %lu %lu \r\n",ulStart,g_ulPartProgramDataLength);
				#endif
            }
            else
            {
	        	//count how many we are sending to
    	    	cfound++;
                csendcount = 0;
                while (csendcount<2)
                {
	            	//have something to send
                	//send the next part now
					//Create This Packet and Send Some Data
				    //Calculate How Much to send
					cfinal=1;
				    ulx = g_ulPartProgramDataLength - ulStart;
					if (ulx>480) // 512-32 = 480
					{
						ulx=480;
						cfinal=0;
					}
					ui = (unsigned int)ulx;

					//The Start of the Data
					writeHLongToNBuffer(p_buffer,ulStart);

                    //Flag if this is the last segment
                    p_buffer[4] = cfinal;

					//The Length of the Data is calculated because it knows that it's the payload - 5

					//The Data Itself
					xmem2root(p_buffer + 5, xp_PartProgramData+ulStart, ui);
					SmartToolMsg(STP_GET_RESP, OID_PARTPGM_DATA, ui + 5, p_buffer);
					#ifdef OUTPUT_SENDPPDATA
                    logf("s %lu %u %d\r\n",ulStart,ui,cfinal);
					#endif
                    ulStart += ui;  //add the send value to the index
					g_ulSendPPDataIndex[c] = ulStart;

                    //See if the end was reached...
					if (ulStart==g_ulPartProgramDataLength)
					{
						//NOTE: Could send back more data here if needed, but I decided to not do this currently.
                        //  see /archive/send_write_file_sections.txt

                        //Now Clear The Flag to indicate that this is complete
                       	g_cSendPPDataSession[c]=0;
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
    if (cfound==0)
    {
    	//has gone back to zero so clear the main variable
        g_cSendPPDataSessions = 0;
		#ifdef OUTPUT_SENDPPDATA
        logf("s cf0\r\n");
		#endif
    }
}

nodebug void SendProbeStatus(unsigned int uiMsgType,char cKIndex)
{
	td_oid_probe_status * p_oid_probe_status;
	char c;
//logf("sps\r\n");
	p_oid_probe_status=(td_oid_probe_status *)g_STPtxMsg.p_cObjectValue;
	c=cKIndex;
	p_oid_probe_status->cKIndex=c;
	p_oid_probe_status->fX=g_fKHolePrbeX[c];
	p_oid_probe_status->fY=g_fKHolePrbeY[c];
	p_oid_probe_status->cStatus=g_cKHolePrbeStatus[c];
	p_oid_probe_status->cMethod=g_cKHolePrbeStatusWarnings[c]<<4; //use to send warning in high half
	if (g_cKHolePrbeStatusDistance[c]!=0)
	{
		p_oid_probe_status->cMethod|=8; //set bit 4 to indicate distance issue
	}

	#ifdef OUTPUT_PROBE_SYS
	logf("%s K%d %f,%f %d\r\n","G PS",c,p_oid_probe_status->fX,p_oid_probe_status->fY,p_oid_probe_status->cStatus);
	#endif
	SmartToolMsg(uiMsgType, OID_PROBE_STATUS, sizeof(td_oid_probe_status),g_STPtxMsg.p_cObjectValue);
}

nodebug void SendAllProbeStatus(unsigned int uiMsgType)
{
	td_oid_probe_status * p_oid_probe_status;
	char c;
	p_oid_probe_status=(td_oid_probe_status *)g_STPtxMsg.p_cObjectValue;
	c=1;
//logf("saps\r\n");
	while(c<=g_cKHoleCount)
	{
		p_oid_probe_status->cKIndex=c;
		p_oid_probe_status->cStatus=g_cKHolePrbeStatus[c];
		p_oid_probe_status->cMethod=g_cKHolePrbeStatusWarnings[c]<<4; //use to send warning in high half
		if (g_cKHolePrbeStatusDistance[c]!=0)
		{
			p_oid_probe_status->cMethod|=8; //set bit 4 to indicate distance issue
		}
		p_oid_probe_status->fX=g_fKHolePrbeX[c];
		p_oid_probe_status->fY=g_fKHolePrbeY[c];
		#ifdef OUTPUT_PROBE_SYS
		logf("%s K%d %f,%f %d\r\n","G PS",c,p_oid_probe_status->fX,p_oid_probe_status->fY,p_oid_probe_status->cStatus);
		#endif
		SmartToolMsg(uiMsgType, OID_PROBE_STATUS, sizeof(td_oid_probe_status),g_STPtxMsg.p_cObjectValue);
		c++;
		if ((c&0x0F)==0)
		{
			PreventSTPTimeout(); //prevent client timeout
			CheckRailSTP();//timeout preventon is not enough along. Must send to NAC or Drill to service their STP tx timeout requirements
		}
	}
}

nodebug void SendProbeStart(unsigned int uiMsgType,char cKIndex)
{
	td_oid_probe_start * p_oid_probe_start;
	char c;
	p_oid_probe_start=(td_oid_probe_start *)g_STPtxMsg.p_cObjectValue;
	c=cKIndex;
	p_oid_probe_start->cKIndex=c;
	p_oid_probe_start->cStatus=g_cKHolePrbeStart[c];
	p_oid_probe_start->cMethod=0;
//#warnt "FIXME Minor  Not using saving Probe Method with probe start or status"
//see here and in this area
	p_oid_probe_start->fX=g_fKHolePrbeStartX[c];
	p_oid_probe_start->fY=g_fKHolePrbeStartY[c];
	#ifdef OUTPUT_PROBE_SYS
	logf("%s K%d %f,%f %d\r\n","G Pstart",c,p_oid_probe_start->fX,p_oid_probe_start->fY,p_oid_probe_start->cStatus);
	#endif
	SmartToolMsg(uiMsgType, OID_PROBE_START, sizeof(td_oid_probe_start),g_STPtxMsg.p_cObjectValue);
}

nodebug void SendAllProbeStart(unsigned int uiMsgType)
{
	td_oid_probe_start * p_oid_probe_start;
	char c;
	p_oid_probe_start=(td_oid_probe_start *)g_STPtxMsg.p_cObjectValue;
	c=1;
	while(c<=g_cKHoleCount)
	{
		p_oid_probe_start->cKIndex=c;
		p_oid_probe_start->cStatus=g_cKHolePrbeStart[c];
		p_oid_probe_start->cMethod=0;
		p_oid_probe_start->fX=g_fKHolePrbeStartX[c];
		p_oid_probe_start->fY=g_fKHolePrbeStartY[c];
		#ifdef OUTPUT_PROBE_SYS
		logf("%s K%d %f,%f %d\r\n","G Pstart",c,p_oid_probe_start->fX,p_oid_probe_start->fY,p_oid_probe_start->cStatus);
		#endif
		SmartToolMsg(uiMsgType, OID_PROBE_START, sizeof(td_oid_probe_start),g_STPtxMsg.p_cObjectValue);
		c++;
		if ((c&0x0F)==0)
		{
			PreventSTPTimeout(); //prevent client timeout
			CheckRailSTP();//timeout preventon is not enough along. Must send to NAC or Drill to service their STP tx timeout requirements
		}
	}
}

nodebug void AlertProbeStatusUpdate()
{
	//Send this single empty OID that tells pendant that it should update the k hole information display now
	//This is preferable to doing an update for each status update that returns.

//    SmartToolMsgEmpty(STP_ALERT, OID_PROBE_UPDATE_NOW);
//
	SmartToolMsgMiniFtMessageCode(OID_PROBE_STATUS, 0); //use message code zero... this is just a hack to allow this to work
}


//nodebug void SendKHoleDistance(unsigned int uiMsgType,float fExpected,float fFound)
//{
//	td_oid_khole_distance * p_oid_khole_distance;
//	p_oid_khole_distance=(td_oid_khole_distance *)g_STPtxMsg.p_cObjectValue;
//	p_oid_khole_distance->fexpected=fExpected;
//	p_oid_khole_distance->ffound=fFound;
//	SmartToolMsg(uiMsgType, OID_KHOLE_DISTANCE, sizeof(td_oid_khole_distance),g_STPtxMsg.p_cObjectValue);
//	return;
//}



//Machine Data Set Code
//SendXYData System

//Machine Data Set Code
nodebug void SendXYDataID(unsigned int uiMsgType)
{
	unsigned long * p_ul;
	char g_cXYDataID[8];
	p_ul=(unsigned long *)g_cXYDataID;
	p_ul[0]=g_ulMachineDataSetIDTimeSec;
	p_ul[1]=g_ulMachineDataSetIDTime;
	#ifdef OUTPUT_SENDXYDATA
	logf("XYDataID sent %d %d %d %d %d %d %d %d\r\n",
		g_cXYDataID[0],		g_cXYDataID[1],
		g_cXYDataID[2],		g_cXYDataID[3],
		g_cXYDataID[4],		g_cXYDataID[5],
		g_cXYDataID[6],		g_cXYDataID[7]);
	#endif
	SmartToolMsg(STP_ALERT, OID_POSNMODE_XYDATA_ID, 8, g_cXYDataID);
}

nodebug void StartSendXYData(int iSession, unsigned int uiStart )
{
	//Must have the file loaded.  Can't be loading the file at the same time
	if (g_cPartPgmStatus != PP_LOADOK)
    {
		return;
	}

	if (uiStart > g_iHoleCount)
    {
    	uiStart=0;
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

nodebug void ContinueSendXYData()
{
	unsigned int uix;
    unsigned int ui;
    char * p_buffer;
    char c;
    char csendcount;
	char cNotDone;
    unsigned int uiStart;
	float fX,fY;

	//do not check here, check before call
    //if (g_cSendXYDataSessions == 0)
    //{
    //	//nothing is being sent
    //    return;
    //}

    c=0;
	cNotDone = 0;
    while(c<MAXNUM_STP_SESSIONS)
    {
    	if (g_cSendXYDataSession[c] == 1)
        {
        	uiStart = g_uiSendXYDataIndex[c];
        	//count how many we are sending to
            csendcount = 0;
            while (1)
            {
				if (uiStart>=g_iHoleCount)
				{
           			//seems like we sent it all
   	        		g_cSendXYDataSession[c]=0;
					break;
    	        }
            	//have something to send
               	//send the next part now
				//Create This Packet and Send Some Data
			    //Calculate How Much to send
			    uix = g_uiCreateMachineDataSetIndex - uiStart;
				if (uix>64) // 512-32 = 480
				{
					uix=64;
				}
				if (uix>0)
				{
					p_buffer = g_STPtxMsg.p_cObjectValue;
					//The Start of the Data
					writeHInt16ToNBuffer(p_buffer,uiStart);
					p_buffer+=2;
					writeHInt16ToNBuffer(p_buffer,g_iHoleCount); //to make sure they know count when they get this.
					p_buffer+=2;
					writeHInt16ToNBuffer(p_buffer,uix);
					p_buffer+=2;
					#ifdef OUTPUT_SENDXYDATA
					logf("XYData Seg %u %d %u\r\n",uiStart, g_iHoleCount, uix);
					#endif
					ui = uiStart;
                    while (uix>0)
					{
						fX = xgetfloat((xp_fRawDataMX)+(4*ui));
						fY = xgetfloat((xp_fRawDataMY)+(4*ui));
						((float *)p_buffer)[0] = fX;
						p_buffer+=4;
						((float *)p_buffer)[0] = fY;
						p_buffer+=4;
						ui++;
						uix--;
    				}
					#ifdef OUTPUT_SENDXYDATA
                    logf("XYDATA %u to %u\r\n",uiStart,ui-1);
					#endif
					SmartToolMsg(STP_ALERT, OID_POSNMODE_XYDATA, p_buffer - g_STPtxMsg.p_cObjectValue, g_STPtxMsg.p_cObjectValue);

    	            uiStart = ui;  //Update the start
					g_uiSendXYDataIndex[c] = uiStart;
            	}
                //See if the end was reached...
				if (uiStart>=g_iHoleCount)
				{
            		//seems like we sent it all
		           	g_cSendXYDataSession[c]=0;
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

nodebug void StartSendOpHistory(int iSession, unsigned long ulAfterTime)
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

nodebug void ContinueSendOpHistory()
{
	unsigned int uix;
    unsigned int ui;
	unsigned long ulAfterTime;
	unsigned long ulBlockTime;
    char * p_buffer;
    char c;
    char csendcount;
	char cNotDone;
	char cBlockOrderIndex;
	char cBlock;
    unsigned int uiStart;
	int iPos;
	int len;
	long xp;

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

    c=0;
	cNotDone = 0;
    while(c<MAXNUM_STP_SESSIONS)
    {
		cBlockOrderIndex = g_cSendOpHistoryBlockOrderIndex[c];
		#ifdef OUTPUT_OPHIST_VERBOSE
		logf("OH cBOI=%d:\r\n",cBlockOrderIndex);//FIXME0 can remove soon
		#endif
		if (cBlockOrderIndex<0xFF)
		{
			//Load the time we need to check with
			//All blocks changed after this time need to be sent.
			ulAfterTime = g_ulSendOpHistoryAfterTime[c];
			//If index goes above block count it's done

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
				p_buffer = (char *)g_STPtxMsg.p_cObjectValue;
				*p_buffer++=cBlock; //send block
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
				*p_buffer++=len; //send length
				*((unsigned long *)(p_buffer))=ulBlockTime; //write time
				p_buffer+=4;
               	xp=xp_uiOpHistory + 2*iPos;
				len = len*2; //each hole has 2 bytes of data.
				xmem2root(p_buffer,xp,len);

				SmartToolMsg(STP_ALERT, OID_OP_HISTORY, 6+len, g_STPtxMsg.p_cObjectValue);

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
				p_buffer = (char *)g_STPtxMsg.p_cObjectValue;
				*p_buffer++=0xFF; //send block
				*p_buffer++=0; //send length
				*((unsigned long *)(p_buffer))=SEC_TIMER; //write time
				SmartToolMsg(STP_ALERT, OID_OP_HISTORY, 6, g_STPtxMsg.p_cObjectValue);

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

nodebug void SendRFIDData(unsigned int uiMsgType)
{
	char * p_c;
	td_RFIDData * p_RFIDData;
	p_RFIDData=(td_RFIDData *)g_STPtxMsg.p_cObjectValue;
	p_RFIDData->cstate=g_RFIDData.cstate;
	p_RFIDData->ccontext=g_RFIDData.ccontext;
	p_RFIDData->cseekstate=g_RFIDData.cseekstate;
	p_RFIDData->ultimestamp=htonul(g_RFIDData.ultimestamp);
	p_RFIDData->ulrfidtimestamp=htonul(g_RFIDData.ulrfidtimestamp);
	p_RFIDData->fposition=g_RFIDData.fposition;
	//bstring field sztagdata
	p_RFIDData->uicrc16=htons(g_RFIDData.uicrc16);
	p_RFIDData->uiendcode=htons(g_RFIDData.uiendcode);
	p_RFIDData->ulseektime=htonul(g_RFIDData.ulseektime);
	p_RFIDData->fsstart=g_RFIDData.fsstart;
	p_RFIDData->fpstart=g_RFIDData.fpstart;
	p_RFIDData->fpend=g_RFIDData.fpend;
	p_RFIDData->fnstart=g_RFIDData.fnstart;
	p_RFIDData->fnend=g_RFIDData.fnend;
	p_RFIDData->fhs1=g_RFIDData.fhs1;
	p_RFIDData->fhs2=g_RFIDData.fhs2;
	p_RFIDData->fhsf=g_RFIDData.fhsf;
	//Send BStrings after fixed size fields
	p_c = g_STPtxMsg.p_cObjectValue + sizeof(td_RFIDData);
	WriteBArray(p_c,g_szTagDatalen,g_szTagData);
	SmartToolMsg(uiMsgType, OID_RFID_DATA, p_c - g_STPtxMsg.p_cObjectValue, g_STPtxMsg.p_cObjectValue);
}

nodebug void SendCurPosnInd(unsigned int uiMsgType)
{
	int i;
	i=-1;
	if (g_PosnMode.iCurPosnIndex>=0 && g_PosnMode.cOnCurPosn==1)
	{
		i = g_PosnMode.iCurPosnIndex;
	}
	SmartToolMsgInt(uiMsgType, OID_POSNMODE_CURPOSN, i);
}

nodebug void SendNearPosnInd(unsigned int uiMsgType)
{
	int i;
    i=-1;
	if (g_cProbeCalculated==1)
	{
		i = FindNearestPosition();
	}
	SmartToolMsgInt(uiMsgType, OID_POSNMODE_NEARPOSN, i);
}

nodebug void SendGoalPosnInd(unsigned int uiMsgType)
{
	if (g_PosnMode.iGotoPosnIndex>=0)
    {
		SmartToolMsgInt(uiMsgType, OID_POSNMODE_GOALPOSN, g_PosnMode.iGotoPosnIndex);
	}
}

nodebug void SendCurXY(unsigned int uiMsgType)
{
	float fx,fy,frx,fry;
    char cm;
    char co;
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
	if (fx>10000 || fy>10000)
	{
		//skip sending the position (which is not valid)
        //logf("INVALID Last Known Posn!!!!!!!!!!!!!!!!!\r\n");
        return;
	}

	if (cm==PD_MACHINE)
    {
    	frx = fx; fry = fy;
        if (co!=PD_ZERO)
        {
    	    if (co==PD_PK)
	        {
//FIXME INCORRECT Not Implemented Yet
            }
    	    if (co==PD_SK)
	        {
//FIXME INCORRECT Not Implemented Yet
            }
    	    if (co==PD_NEAREST)
	        {
//FIXME INCORRECT Not Implemented Yet
            }
        }
    }
    else if (cm==PD_DATASET)
    {
        //FIXME0000000 New Multi Probe would require special rotate back to dataset.
//FIXME INCORRECT Not Implemented 100% correctly Yet
		RotateMachineToDataset(fx,fy,&frx,&fry);
        if (co!=PD_ZERO)
        {
    	    if (co==PD_PK)
	        {
//FIXME INCORRECT Not Implemented Yet
            }
    	    if (co==PD_SK)
	        {
//FIXME INCORRECT Not Implemented Yet
            }
    	    if (co==PD_NEAREST)
	        {
//FIXME INCORRECT Not Implemented Yet
            }
        }
    }
    else if (cm==PD_DATASET_NS)
    {
//FIXME INCORRECT Not Implemented 100% correctly Yet
		RotateMachineToDataset(fx,fy,&frx,&fry);
        if (co!=PD_ZERO)
        {
    	    if (co==PD_PK)
	        {
//FIXME INCORRECT Not Implemented Yet
            }
    	    if (co==PD_SK)
	        {
//FIXME INCORRECT Not Implemented Yet
            }
    	    if (co==PD_NEAREST)
	        {
//FIXME INCORRECT Not Implemented Yet
            }
        }
	}
    else if (cm==PD_NOTHING)
    {
    	frx = fx; fry = fy;
    }

	//initialize pointer to structure
	p_oid_posnmode_curxy=(td_oid_posnmode_curxy *)g_STPtxMsg.p_cObjectValue;
	//set fields
    //logf(" CUR %.3f,%.3f   M %.3f,%.3f\r\n",frx,fry,fx,fy);
	p_oid_posnmode_curxy->fDataSetX=frx;
	p_oid_posnmode_curxy->fDataSetY=fry;
	p_oid_posnmode_curxy->fMachineX=fx;
	p_oid_posnmode_curxy->fMachineY=fy;
	//Send back to all sessions: This is a noted exception to GET_RESP currently.
	//See notes about pending system removal
	SmartToolMsg(uiMsgType, OID_POSNMODE_CURXY, sizeof(td_oid_posnmode_curxy),g_STPtxMsg.p_cObjectValue);
}

nodebug void SendNearXY(unsigned int uiMsgType)
{
	td_oid_posnmode_nearxy * p_oid_posnmode_nearxy;
	//these are upated to reflect the last nearposn as calculated in FindNearestPosition();
	if (g_PosnMode.iNearestPosn >= 0)
    {
		p_oid_posnmode_nearxy=(td_oid_posnmode_nearxy *)g_STPtxMsg.p_cObjectValue;
		p_oid_posnmode_nearxy->fDataSetX=g_PosnMode.fNearestPosnDX;
		p_oid_posnmode_nearxy->fDataSetY=g_PosnMode.fNearestPosnDY;
		p_oid_posnmode_nearxy->fMachineX=g_PosnMode.fNearestPosnMX;
		p_oid_posnmode_nearxy->fMachineY=g_PosnMode.fNearestPosnMY;
        p_oid_posnmode_nearxy->iNearPosn=htons(g_PosnMode.iNearestPosn); //set right before this is used
		SmartToolMsg(uiMsgType, OID_POSNMODE_NEARXY, sizeof(td_oid_posnmode_nearxy),g_STPtxMsg.p_cObjectValue);
	}
}

nodebug void SendActivePremove(unsigned int uiMsgType)
{
	float fx;
    float fy;
    char c;
	td_oid_posnmode_active_premovexy * p_oid_posnmode_active_premovexy;
	p_oid_posnmode_active_premovexy=(td_oid_posnmode_active_premovexy *)g_STPtxMsg.p_cObjectValue;
    c = g_ConfigData.cMoveType;
    fx = 0;
    fy = 0;
	if (c == MOVETYPE_ORIGINAL || c == MOVETYPE_FAST)
    {
		CalculatePreMove(&fx, &fy);
    }
	p_oid_posnmode_active_premovexy->fX=fx;
	p_oid_posnmode_active_premovexy->fY=fy;
	SmartToolMsg(uiMsgType, OID_POSNMODE_ACTIVE_PREMOVEXY, sizeof(td_oid_posnmode_active_premovexy),g_STPtxMsg.p_cObjectValue);
}

nodebug void SendSystemComponents(unsigned int uiMsgType)
{
	td_SystemComponents * p_SystemComponents;
	p_SystemComponents = (td_SystemComponents *) g_STPtxMsg.p_cObjectValue;
	p_SystemComponents->cDrill = g_ConfigData.SystemComponents.cDrill;
	p_SystemComponents->cFastener = g_ConfigData.SystemComponents.cFastener;
	p_SystemComponents->cFastenerTray = g_ConfigData.SystemComponents.cFastenerTray;
	p_SystemComponents->cAux1 = g_ConfigData.SystemComponents.cAux1;
	p_SystemComponents->cAux2 = g_ConfigData.SystemComponents.cAux2;
	p_SystemComponents->cAux3 = g_ConfigData.SystemComponents.cAux3;
	p_SystemComponents->cAux4 = g_ConfigData.SystemComponents.cAux4;
	p_SystemComponents->cAux5 = g_ConfigData.SystemComponents.cAux5;
	SmartToolMsg( uiMsgType, OID_SYSTEM_COMPONENTS, sizeof(td_SystemComponents), g_STPtxMsg.p_cObjectValue );
}

char g_cProcStyle;

nodebug void LoadHoleParameters()
{
	char buffer[16];
    char cLayers;
	char cProcLayers;
    char cCountersink;
    char cw;
    unsigned int ui;
    unsigned int uilayer;
    float f;
    float * fdata;

	//uses the global g_HoleData and loads many more thigns into g_HoleParam

    //First Set these globals
	g_cRequiredTool = g_HoleData.cTool;
	g_cRequiredProcess = g_HoleData.cProcess;

	//mask hole ops with all the ops the process allows... and allow inspection, seal, and fill if the hole wants it. ( The process does not care about those operations. )
	g_uiHoleOps = g_HoleData.uiOps & (g_cProcessOps[g_cRequiredProcess] | OP_INSPECT | OP_SEAL | OP_FILL | OP_REMOVE );

//FIXME Operations-Limits  Tool used to be part of operations limit but it was removed.
//       					do we need better operation limits?

    cLayers = g_cProcessLayerCount[g_cRequiredProcess];
	cProcLayers  = g_cProcessProcLayerCount[g_cRequiredProcess];
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
	g_HoleParam.fDiameter = ((float)g_HoleData.d2uDiameter)/10000;
	g_HoleParam.fProcessCountersink = ((float)g_uiProcessCountersink[g_cRequiredProcess])/10000;
	g_HoleParam.fHoleCountersinkAdjust = ((float)g_HoleData.iCountersinkAdjust)/10000;
//FIXME666666666666 need print out for testing or breakpoints......
   	logf("Layer Count = %d\r\n",cLayers);
   	logf("plc %d csnk %d\r\n",cProcLayers,cCountersink);
    fdata = &g_HoleParam.flayer1;
    if (cLayers > 8)
    {
    	logf("Bad Layer Count");
        cLayers = 1;
    }
    if (cLayers == 1)
    {
    	g_HoleParam.flayer1 = ((float)g_HoleData.uiStack)/10000;
        cw=1;
    }
    else
    {
    	ui = g_HoleData.uiStack * 2;
        cw=0;
        while(cw < cLayers)
        {
        	uilayer = xgetint(xp_StackData+ui);
            f = ((float)uilayer)/10000;
            fdata[cw]=f;
        	logf("%d %u\r\n",cw,f);
            ui+=2;
        	cw++;
        }
    }
	while(cw < 8)
    {
    	fdata[cw]=0;
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

nodebug void SendHoleParameters(unsigned int uiMsgType)
{
	td_HoleParam xHoleParam;
	td_HoleParam * p_HoleParam;

	p_HoleParam = &xHoleParam;
	p_HoleParam->cProcess=g_HoleParam.cProcess;
	p_HoleParam->cToolType=g_HoleParam.cToolType;
	p_HoleParam->cFastenerType=g_HoleParam.cFastenerType;
	p_HoleParam->cLayers=g_HoleParam.cLayers;
	p_HoleParam->cCountersink=g_HoleParam.cCountersink;
	p_HoleParam->uiOperations=htons(g_HoleParam.uiOperations);
	p_HoleParam->fDiameter=g_HoleParam.fDiameter;
	p_HoleParam->fProcessCountersink=g_HoleParam.fProcessCountersink;
	p_HoleParam->fHoleCountersinkAdjust=g_HoleParam.fHoleCountersinkAdjust;
	p_HoleParam->flayer1=g_HoleParam.flayer1;
	p_HoleParam->flayer2=g_HoleParam.flayer2;
	p_HoleParam->flayer3=g_HoleParam.flayer3;
	p_HoleParam->flayer4=g_HoleParam.flayer4;
	p_HoleParam->flayer5=g_HoleParam.flayer5;
	p_HoleParam->flayer6=g_HoleParam.flayer6;
	p_HoleParam->flayer7=g_HoleParam.flayer7;
	p_HoleParam->flayer8=g_HoleParam.flayer8;
	p_HoleParam->cmat1=g_HoleParam.cmat1;
	p_HoleParam->cmat2=g_HoleParam.cmat2;
	p_HoleParam->cmat3=g_HoleParam.cmat3;
	p_HoleParam->cmat4=g_HoleParam.cmat4;
	p_HoleParam->cmat5=g_HoleParam.cmat5;
	p_HoleParam->cmat6=g_HoleParam.cmat6;
	p_HoleParam->cmat7=g_HoleParam.cmat7;
	p_HoleParam->cmat8=g_HoleParam.cmat8;

	SmartToolMsg(uiMsgType, OID_HOLE_PARAMETERS, sizeof(td_HoleParam),(char*)p_HoleParam);
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

nodebug void CalculateLayerOverride()
{
	char bl;
	char bmateriallayercount;
	char blayercount;
	char bl_out;

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
    if(bmateriallayercount>8) { bmateriallayercount=8; } //FIXME000000 is this check needed?
    p_fLayerThickness = &g_HoleParam.flayer1; //in memory these are all inline like an array
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
			SmartToolMsgMiniFtMessageCode(OID_NULLOID, MINIFTMC_TOOL_SHORT_WARNING_COUNTERSINK_WILL_START);
			logf("*w %f < %f\r\n",fLayer,fCountersinkStart);
		}
		if (fCountersinkEnd!=0)
		{
			//should have valid Start And End
			if (fLayer < fCountersinkEnd)
			{
				//Failure: Material Layers Deeper than Can be drilled without maaking a countersink too deep
				logf("*f %f < %f\r\n",fLayer,fCountersinkEnd);
				SmartToolMsgMiniFtMessageCode(OID_NULLOID, MINIFTMC_TOOL_SHORT_FAILURE);
if (g_cTestOpt==4)
{
goto bypass_failure;
}
    	        //turn off process to force stop
				g_cStartProcess = 0; //FIXME should be checked.
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
	p_layer->fAbsoluteEnd = fStart; //use ABSOLUTE motion.
	p_layer->fThickness = g_ConfigData.fReturnHeight; //not used, but for reference. Drill May want this to be set
    p_layer++; //increase one layer override
    bl++;
	logf("S LO: Air Abs:%f", fStart);

	//Go through and create material layers using thicknesses
    p_fLayerThickness = &g_HoleParam.flayer1; //in memory these are all inline like an array
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

        p_fLayerThickness++; //actually adds 4 because it knows it's a float pointer
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
	    p_layer->fAbsoluteEnd = fCountersinkStart; //use ABSOLUTE motion.
    	p_layer->fThickness = fThick; //not used, but for reference. Drill May want this to be set
	    p_layer++; //increase to next td_LayerOverride
		logf("L=%d cs=%f\r\n",((int)bl), fCountersinkStart);
	    bl++;

    	//csnk
	    //Find Thickness
    	fThick = -(fCountersinkEnd - fCountersinkStart); //Thickness of countersink
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
	    p_layer->fThickness = fThick; //not used, but for reference. Drill May want this to be set
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
		fLayer = fLayer - g_LoadedTool.fMinBreak; //set this end another min break down.
		p_layer->cLayerNumber = bl;
		p_layer->cOverrideActive = 1;
		p_layer->cUseHardStop = 0;
		p_layer->cUseThisLayer = 0; //SPECIAL TERMINATOR
		p_layer->fAbsoluteEnd = fLayer;
		p_layer->fThickness = 0;
	    p_layer++; //increase to next td_LayerOverride
		bl++;

		//This layer will not be counted or sent.
		//It only exists to store AbsoluteEnd for the case mentioned in the comment above.

		fLayer = fLayer - g_LoadedTool.fMinBreak; //set this end another min break down.
		p_layer->cLayerNumber = bl;
		p_layer->cOverrideActive = 1;
		p_layer->cUseHardStop = 0;
		p_layer->cUseThisLayer = 0; //SPECIAL TERMINATOR ...
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
		p_layer->fAbsoluteEnd = fStart - fThick; //use ABSOLUTE motion.
		p_layer->fThickness = g_ConfigData.fReturnHeight + fThick; //not used, but for reference. Drill May want this to be set
	    p_layer++; //increase to next td_LayerOverride
        bl++;

		if (g_HoleParam.cCountersink > 0)
		{
			//csnk
			//Find Thickness
			fThick = -(fCountersinkEnd - fCountersinkStart); //Thickness of countersink
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
			p_layer->fThickness = fThick; //not used, but for reference. Drill May want this to be set
		    p_layer++; //increase to next td_LayerOverride
			logf("L=%d ce=%f\r\n",((int)bl), fCountersinkEnd);
			bl++;
		}

    	//Set Fields to special value that will stop drilling at this layer
		p_layer->cLayerNumber = bl;
		p_layer->cOverrideActive = 1;
		p_layer->cUseHardStop = 0;
		p_layer->cUseThisLayer = 0; //SPECIAL TERMINATOR
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
		    if(bmateriallayercount>8) { bmateriallayercount=8; } //FIXME000000 is this check needed?
		    p_fLayerThickness = &g_HoleParam.flayer1; //in memory these are all inline like an array
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
		p_layer->fAbsoluteEnd = fStart - fThick; //use ABSOLUTE motion.
		p_layer->fThickness = g_ConfigData.fReturnHeight + fThick; //not used, but for reference. Drill May want this to be set
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

nodebug void SendLayerOverride()
{
	char c;
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

nodebug void ShowLayerOverride()
{
	char c;
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

nodebug void SendReqToolSearch(unsigned int uiMsgType)
{
	char c_op;
	char c_arg1;
	if (g_ToolServerSTPSession==0)
	{
		return;
	}
    c_op=search;
	c_arg1=required_tool_type;
	//Write MGMT out
	g_pSTPSession = (void *)g_ToolServerSTPSession; //limit to tool server
	SendToolMGMT(uiMsgType,c_op,c_arg1,0,"",0);
	g_pSTPSession = (void *)0;
	return;
}

nodebug void SendToolMGMT(unsigned int uiMsgType,char c_op,char carg1,unsigned int ui,char *s, int ilen)
{
	td_oid_tool_mgmt * p_oid_tool_mgmt;
	char * p_c;

	p_oid_tool_mgmt=(td_oid_tool_mgmt *)g_STPtxMsg.p_cObjectValue;
	p_oid_tool_mgmt->coperation=c_op;
	p_oid_tool_mgmt->carg1=carg1;
	p_oid_tool_mgmt->uiarg2=htons(ui);
	//bstring field szsarg3
	//Send BStrings or BArrays after fixed size fields
	p_c = ((char *)p_oid_tool_mgmt) + sizeof(td_oid_tool_mgmt);
	//SPECIAL: If required tool type search, then lookup the string and write it here instead of copy input
	if (carg1 == required_tool_type)
	{
		//special path: copy the required tool right to p_c
		s=p_c+1;
		LookupToolTypeString(s,g_cRequiredTool);
       	g_cLastSearchedRequiredTool=g_cRequiredTool;
		if (*s==0)
		{
			//nothing was there
			p_oid_tool_mgmt->carg1=complete;
logf("irt scequired tool\r\n");
 	     	g_cLastSearchedRequiredTool=0;
			ilen=0;
		}
		else
		{
			ilen=strlen(s);
		}
	}
	WriteBString(p_c,ilen,s);
	//Write out to tool server
logf("tm %d\r\n",c_op);
	SmartToolMsg(uiMsgType, OID_TOOL_MGMT, p_c - ((char*)p_oid_tool_mgmt), ((char*)p_oid_tool_mgmt));
}

nodebug void SendTool(unsigned int uiMsgType,char cOperation)
{
	td_oid_tool_rec * p_LoadedTool;
	char * p_c;
	p_LoadedTool = (td_oid_tool_rec *)g_STPtxMsg.p_cObjectValue;
	p_LoadedTool->cOperation=cOperation;
	p_LoadedTool->cToolStatus=g_LoadedTool.cToolStatus;
	p_LoadedTool->cHardstop=g_LoadedTool.cHardstop;
	p_LoadedTool->fDiameter=g_LoadedTool.fDiameter;
	p_LoadedTool->fLength=g_LoadedTool.fLength;
	p_LoadedTool->fMinBreak=g_LoadedTool.fMinBreak;
	p_LoadedTool->fLengthCountersink=g_LoadedTool.fLengthCountersink;
	p_LoadedTool->fCountersinkAdjust=g_LoadedTool.fCountersinkAdjust;
	p_LoadedTool->ulDTimeTicksMSW=htonul(g_LoadedTool.ulDTimeTicksMSW);
	p_LoadedTool->ulDTimeTicksLSW=htonul(g_LoadedTool.ulDTimeTicksLSW);
	p_LoadedTool->uiDCount=htons(g_LoadedTool.uiDCount);
	p_LoadedTool->uiDWarnCount=htons(g_LoadedTool.uiDWarnCount);
	p_LoadedTool->uiDLimitCount=htons(g_LoadedTool.uiDLimitCount);
	//bstring field szID
	//bstring field szToolTypeCode
	//Send BStrings after fixed size fields
	p_c = ((char *)p_LoadedTool) + sizeof(td_oid_tool_rec);
	WriteBString(p_c,g_szToolIDlen,g_szToolID);
	WriteBString(p_c,g_szToolTypeCodelen,g_szToolTypeCode);
logf("st\r\n");//asdfggggggg
logf("-+ %d \"%s\" %d \"%s\"\r\n", g_szToolIDlen, g_szToolID, g_szToolTypeCodelen, g_szToolTypeCode );
	SmartToolMsg(uiMsgType, OID_TOOL_REC, p_c - ((char *)p_LoadedTool), ((char *)p_LoadedTool));
}


#ifndef XMEM_FAST_COPY
//FIXME MED Should I have this defined?
//	I found out that it was notdefined using the following warning code
//#warnt "NOTE: XMEM_FAST_COPY WAS NOT DEFINED"
#endif

#ifdef TOOL_IN_RAM
nodebug void SaveToolToRam()
{
//FIXME IMMEDIATE
	logf("Saved Tool\r\n");
}

nodebug void LoadToolFromRam()
{
	char cvalid;
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

typedef struct {
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

typedef struct {
	char SN[32];
	unsigned int uiCyclesOnCutter;
	int iHighestStallCount;
	float fHighestStallDensity;
	float fHighestStallZone;
	float fAvgThrustTi;
	float fAvgThrustCf;
} td_Cutter;
//asdfggggg<<<<<<<<<<<<<<Does it ever set cutter?

nodebug void LoadToolToDrill()
{
#ifdef DRILL_DIRECT_READY
	char c;
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
		if (c>31) { c=31; }
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

nodebug void LoadToolHomeBackToDrill()
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

nodebug void RepositionDrill()
{
#ifdef DRILL_DIRECT_READY
	char c;
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
			g_cDrillStateGoal = DRILLSTATE_HOME; //request a home
			g_cDrillStateGoalCommanded = DRILLSTATE_IDLE; //will cause it to resend
			logf("dsgh%d %f\r\n",g_cDrillState,g_fLastSentHomeBack);
		}
#endif
	}
#endif
}

nodebug char CheckObstructionsAndMotionLimits(float fX,float fY)
{
	if (g_cObstructionCode!=0)
	{
		//Check Individual Sensors
		#ifdef Y_LIMIT_SENSORS
		if (g_cDigInYPosLimit==0)
    	{
        	if (fY>g_PosnMode.fLastKnownPosnY) { goto would_move_toward_obstruction; }
        }
    	if (g_cDigInYNegLimit==0)
        {
	    	if (fY<g_PosnMode.fLastKnownPosnY) { goto would_move_toward_obstruction; }
		}
		#endif
		#ifdef OBSTRUCTION_SYSTEM_XP1
		if (g_cDigInObstructionXP1==OBSTRUCTION)
		{
			//X+
			if (fX>g_PosnMode.fLastKnownPosnX) { goto would_move_toward_obstruction; }
		}
		#endif
		#ifdef OBSTRUCTION_SYSTEM_XN1
		if (g_cDigInObstructionXN1==OBSTRUCTION)
		{
			//X-
			if (fX<g_PosnMode.fLastKnownPosnX) { goto would_move_toward_obstruction; }
		}
		#endif
		#ifdef OBSTRUCTION_SYSTEM_MOS
		if (g_cDigInObstructionMOS==MO_OBSTRUCTION)
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
	logf("Mvbd");//bad dir
	#endif
	SmartToolMsgMiniFtMessageCode(OID_NULLOID, MINIFTMC_MOVE_WOULD_GO_OUT_OF_RANGE);
    return 1;
would_move_toward_obstruction:
	//can't do this move
	#ifdef USE_OUTPUT
	logf("Mvto");//mv toward obstruction
	#endif
	AlertObstructionCode();
    return 1;
}

nodebug void AlertObstructionCode()
{
	unsigned long ul;
	#ifdef OUTPUT_OBSTRUCTION_CODE
	logf("A");logf("oc=%x\r\n",g_cObstructionCode);
	#endif
	ul=g_cObstructionCode;
	SmartToolMsgULong(STP_ALERT, OID_LIMITS_AND_OBSTRUCTIONS, ul);
}

nodebug void AlertObstructionWarningCode()
{
	unsigned long ul;
	#ifdef OUTPUT_OBSTRUCTION_CODE
	logf("A");logf("owc=%x\r\n",g_cObstructionWarningCode);
	#endif
	ul=g_cObstructionWarningCode;
	SmartToolMsgULong(STP_ALERT, OID_LIMITS_AND_OBSTRUCTION_WARNINGS, ul);
}

nodebug void UpdateStationPlan()
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
	logf("%c\r\n",g_cStationCode[g_cStartStation]);
	logf("u=%u\r\n",g_ConfigData.uiProcessOperations);
}

//Just take the next step in stations, and then let advancestations work this out.
nodebug void NextStation()
{
	logf("ns\r\n");
	logf("%c\r\n",g_cStationCode[g_cStationGoal]);
	if (g_cStationGoal==STATION_DRILL)
    {
    	g_cStationGoal = STATION_INSPECT;
	}
	else if (g_cStationGoal==STATION_INSPECT)
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
	logf("%c\r\n",g_cStationCode[g_cStationGoal]);
}

nodebug void AdvanceStations()
{
	logf("as\r\n");
	logf("%c\r\n",g_cStationCode[g_cStationGoal]);
	if (g_cStationGoal==STATION_DRILL)
    {
    	if (g_cStationPlanDrill==0) { g_cStationGoal = STATION_INSPECT; }
logf("  %c\r\n",g_cStationCode[g_cStationGoal]);
	}
	if (g_cStationGoal==STATION_INSPECT)
    {
 		#ifdef SEAL_SYSTEM
    	if (g_cStationPlanInspect==0) { g_cStationGoal = STATION_SEAL; }
		#else
		#ifdef FASTENER_SYSTEM
    	if (g_cStationPlanInspect==0) { g_cStationGoal = STATION_PICKUP; }
		#else
    	if (g_cStationPlanInspect==0) { g_cStationGoal = STATION_UNSPEC; }
		#endif
		#endif
logf("  %c\r\n",g_cStationCode[g_cStationGoal]);
	}
	#ifdef SEAL_SYSTEM
    if (g_cStationGoal==STATION_SEAL)
    {
	   	if (g_cStationPlanSeal==0) { g_cStationGoal = STATION_PICKUP; }
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
    	if (g_cStationPlanFill==0 && g_cStationPlanRemove==0) { g_cStationGoal = STATION_UNSPEC; }
logf("  %c\r\n",g_cStationCode[g_cStationGoal]);
	}
    #endif
	logf("%c\r\n",g_cStationCode[g_cStationGoal]);
}

nodebug void PrepareStations()
{
    if (g_cStartProcess==1)
	{
   	//running the process
	//FIXME0000 note: in the future it will be more about what station is set than who you are.....
    //    So there will need to be less identity checking
	if (g_cEEOption!=EENONE)
	{
        //For GENHD, only tool check when process is running //FIXME00000 confirm this and consider move comment
		if (g_cStation==STATION_DRILL)
		{
		if (g_cStationPlanDrill==1)
        {
			if (g_DrillFault.cSeverity >= FAULT_SEVERITY_ALARM)
    	    {
 				//Do Not Continue Action
    	    	g_cDrillStateGoal=DRILLSTATE_IDLE;
            }
#warnt "This must cause abort"
//FIXME SEVERE  these could be stale from last load???????
//unless I adopt the pattern that they are cleared at the start of each move to position
	        else if (g_cToolLoaded!=1)
    	    {
				//not ready
			}
			//DRILL_DIRECT_PROCESS_AND_OVERRIDE
			else if (g_cDrillLoadProcessAndOverride==DRILL_LOAD_PROCESS_DONE)
			{
				//It's Ready
				if (g_cPosnModeState >= POSNMODE_FINALMOVE)
		        {
                	//ALWAYS DO SPIN UP HERE
	    	    	g_cDrillStateGoal=DRILLSTATE_SPINUP;
					g_ulSpinUp=MS_TIMER;
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

nodebug void StopProcess(void)
{
	td_STPsessions * p_STPSession;

	g_cStartProcess = 0;
	g_cAutoMove=0;
	//Clear Any Pending Move Trigger
	g_PosnMode.cDoMoveTrig = FALSE;

	if (g_cPosnModeState < POSNMODE_ACTION || g_cPosnModeState > POSNMODE_ACTION_COMPLETE) //do I need g_cPosnModeState != POSNMODE_WAITNEXTPOSN too???
	{
		g_cPosnModeState = POSNMODE_STOP_MOVEMENT;
	}
	g_cDrillStateGoal=DRILLSTATE_IDLE;
	#ifdef SEAL_SYSTEM
	g_cSealState=SEALSTATE_OFF;
	#endif
	#ifdef FASTENER_SYSTEM
	g_cFillState=FILLSTATE_OFF;
	g_cFastenerArrived=0; //allow clearance, but hope it will redetect it
	#endif
	//Echo the Stop Back to the clients
	//FIXME dfnow  may not be needed when everything is run here.
	p_STPSession = g_pSTPSession; //save global session
	g_pSTPSession = (void *)0; //no limit
	SmartToolMsgEmpty(STP_ALERT, OID_PROCESS_STOP); //FIXME:share:tom
	g_pSTPSession = p_STPSession; //restore limit
}

//////////////////////////////////////////////////////////////////////
// Tool Lookup
//////////////////////////////////////////////////////////////////////
nodebug char LookupToolTypeCode(char * tooltype)
{
	char buffer[256];
	int ibase;
	int i;
	int iw;
	int b;
	char ctool;
	char ctoolmatch;
	char * p_c;

	if (xp_ToolNames==0 || g_iToolNamesLen==0)
	{
		return 0;
	}
//Comment this out to show the entire list even when no tool has yet been loaded
//	if (*tooltype == 0)
//	{
//		return 0;
//	}
	ctoolmatch=0;
	ctool=0;
	ibase=0;
logf("c %d %d\r\n",g_cToolCount,g_iToolNamesLen);
	while (ibase < g_iToolNamesLen)
	{
		b=g_iToolNamesLen - ibase;
		if (b>64)
		{
			b=64;
		}
logf("c %d %d\r\n",ibase,b);
		xmem2root(buffer,xp_ToolNames + ibase,b);
		i=0;
		iw=0;
		p_c=buffer;
		while(i<b)
		{
			if (buffer[i]=='\r')
			{
				buffer[i]=0;
				ctool++;
logf("%d s=%s\r\n",ctool,p_c);
				if (strcmp(tooltype,p_c)==0)
				{
					//found the tool
					ctoolmatch = ctool;
logf("@@@\r\n");
//COMMENT THIS OUT TO SHOW THE ENTIRE LIST
//					goto found_tool_type_code;
				}
				if (ctool>=g_cToolCount)
				{
					goto no_tool_type_code;
				}
				i+=2;
				p_c=buffer+i;
				iw=i;
				continue;
			}
			i++;
		}
		ibase+=iw; //move the base up
	}
no_tool_type_code:
found_tool_type_code:
	return ctoolmatch;
}

nodebug void LookupToolTypeString(char * tooltypeout,char ctoolsearch)
{
	char buffer[256];
	int ibase;
	int i;
	int iw;
	int b;
	char ctool;
	char ctoolmatch;
	char * p_c;

	if (xp_ToolNames==0 || g_iToolNamesLen==0)
	{
		goto no_tool_type_code;
	}
	if (ctoolsearch==0)
	{
		goto no_tool_type_code;
	}
	ctoolmatch=0;
	ctool=0;
	ibase=0;
	while (ibase < g_iToolNamesLen)
	{
		b=g_iToolNamesLen - ibase;
		if (b>64)
		{
			b=64;
		}
		xmem2root(buffer,xp_ToolNames + ibase,b);
		i=0;
		iw=0;
		p_c=buffer;
		while(i<b)
		{
			if (buffer[i]=='\r')
			{
				buffer[i]=0;
				ctool++;
				if (ctool == ctoolsearch)
				{
logf("%d s=%s\r\n",ctool,p_c);
					//found the tool
					strcpy(tooltypeout,p_c);
					return;
				}
				if (ctool>=g_cToolCount)
				{
					goto no_tool_type_code;
				}
				i+=2;
				p_c=buffer+i;
				iw=i;
				continue;
			}
			i++;
		}
		ibase+=iw; //move the base up
	}
no_tool_type_code:
	*tooltypeout = 0;
}

//////////////////////////////////////////////////////////////////////
// Tool Sync
//////////////////////////////////////////////////////////////////////
nodebug void VerifyAndAlertTool()
{
	g_cToolLoaded=1;
	if (g_cRequiredTool>0)
    {
    	if (g_cLoadedTool != g_cRequiredTool)
        {
			//Tool Failed
			g_cToolLoaded=0;
            //Alert HoleParam
            SendHoleParameters(STP_ALERT);
			//..... the alert of parameters in this case is more to just make sure the pendant knows...
            //Alert Tool
            SendTool(STP_ALERT,improper_type);
        }
    	else if (g_LoadedTool.uiDCount>=g_LoadedTool.uiDLimitCount && g_LoadedTool.uiDLimitCount>0)
        {
			//Tool Failed Because of Limit
			g_cToolLoaded=0;
            //Alert Tool
            SendTool(STP_ALERT,improper_type);//FIXME FUTURE Create another type for this message
        }
    }
}

nodebug void VerifyAndAlertProcess()
{
	g_cProcessLoaded=1;
	if (g_cRequiredProcess>0)
    {
    	if (g_cLoadedProcess != g_cRequiredProcess)
        {
			//Process Failed
			g_cProcessLoaded=0;
            //Alert HoleParam
            SendHoleParameters(STP_ALERT);
            //Alert Process
			SmartToolMsgChar(STP_ALERT, OID_PROCESS, g_cLoadedProcess);
        }
    }
}

nodebug void LoadProcessAndOverride()
{
	//DRILL_DIRECT_PROCESS_AND_OVERRIDE
	if (g_cToolLoaded!=0)
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
				SmartToolMsgChar(STP_ALERT, OID_PROCESS, g_cLoadedProcess); //New Purpose of Alert is to just show what is loaded...
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
			SmartToolMsgChar(STP_ALERT, OID_PROCESS, g_cLoadedProcess); //New Purpose of Alert is to just show what is loaded...
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
		g_cDrillLoadProcessAndOverride=DRILL_LOAD_PROCESS_DONE;
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
	char UseLayerOverrideData;
	int CutterCycleLimit;
} td_SmartDrillStackStruct;

nodebug void LoadProcess()
{
	unsigned int index;
	int len;
	long xp;
    char * sbuf;
    char * buf;
	char layer;
	char proclayers;
	char flags;
	char flagsb;
	td_Layers * p_Layer;
	char layer_micropeck;
	char layer_lcdeltathrust;
	char layer_setback;
	char layer_earlyshiftdistance;
	char layer_harder;
	float fThrust;
	td_LayerOverride * p_LayerOverride;
	float fhardstopamps;
	td_SmartDrillStackStruct * p_SmartDrillStackStruct;
	char clube;

	logf("LoadProcess\r\n");

	if (xp_ProcessLayerDataBase == 0)
	{
		//no loaded process layer data
		logf("n\r\n");
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

	xp = xp_ProcessLayerDataBase + index;
	sbuf = g_STPtxMsg.p_cObjectValue + STP_OBJVALUE_MAXSIZE - len - 2;
	buf=sbuf;
	xmem2root(buf,xp,len);
	//memdump("buf",buf,len);

    p_LayerOverride = (td_LayerOverride *)g_LayerOverrides;

	p_Layer = (td_Layers *)g_STPtxMsg.p_cObjectValue;

	layer=1;
	while(layer<=proclayers)
	{
		if ( ((char *)p_Layer) + sizeof(td_Layers) > buf)
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
		if (flags & 1) { p_Layer->cCoolantType=1; }
		if (flags & 2) { p_Layer->cPeckType=1; }
		if (flags & 4) { p_Layer->cShiftRetract=1; }
		if (flags & 8) { p_Layer->cLcUseSlope=1; }
		if (flags & 16) { p_Layer->cH2SPeck=1; }
		if (flags & 32) { p_Layer->cCountersinkLayer=1; }
		if (flags & 64) { p_Layer->cUseHardStop=1; }
		//if (flags & 128) { p_Layer->cBurstInterval=1; } //FIXMENOW
		if (flagsb & 1) { layer_micropeck=1; }
		if (flagsb & 2) { layer_lcdeltathrust=1; }
		if (flagsb & 4) { layer_setback=1; }
		if (flagsb & 8) { layer_earlyshiftdistance=1; }
		if (flagsb & 16) { layer_harder=1; }

		p_Layer->uiRpm = *(unsigned int *)buf; buf+=2;
		p_Layer->fIpr = ((float)(*(int *)buf))/10000; buf+=2;
		if (layer_setback) { p_Layer->fSetback = ((float)(*(int *)buf))/10000; buf+=2; }
		if (layer_earlyshiftdistance) { p_Layer->fEarlyShiftDistance = ((float)(*(int *)buf))/10000; buf+=2; }


#ifdef GEN4LUBE
		#define LUBE_AUTO 2
		#define LUBE_SPRITZ 3
		if (p_Layer->cCoolantType) //CoolantOn
		{
			p_Layer->iLubeDurationMs = *(unsigned int *)buf; buf+=2;
			if (clube<SPRITZ)//not yet spritz
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
							SmartToolMsgMiniFtMessageCode(OID_NULLOID, MINIFTMC_LUBE_POSITION_WARNING);
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
			if (clube<LUBEMODE_AUTO)//not yet set
			{
				clube=LUBEMODE_AUTO; //auto
				if (layer==1)
				{
					//using lube on the 1st layer.
					if (g_ConfigData.fReturnHeight < g_ConfigData.LaserSensorAlgParam.fdelta_basespan)
					{
						logf("lrh %f %f\r\n", g_ConfigData.fReturnHeight, g_ConfigData.LaserSensorAlgParam.fdelta_basespan);
						//Warn about return height.
						SmartToolMsgMiniFtMessageCode(OID_NULLOID, MINIFTMC_LUBE_POSITION_WARNING);
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
		p_Layer->fThicknessMax = p_LayerOverride->fThickness; //Copy From Overrides

//FIXME SEVERE
		//Testing Display
//		logfdebuglogf("%d",123);
//		logfdebuglogf("%d",456);

		logf(" L ");
		logf(" %d ",layer);
		logf(" %u ",p_Layer->uiRpm);
		logf(" %f ",p_Layer->fIpr);
		if (flagsb & 4) { logf(" sb"); logf(" %f ", p_Layer->fSetback); }
		if (flagsb & 8) { logf(" es"); logf(" %f ", p_Layer->fEarlyShiftDistance); }
		if (flags & 1) { logf(" c"); logf(" %d ", p_Layer->iLubeDurationMs); }
		if (flags & 2) { logf(" p"); logf(" %u ", p_Layer->uiPeckDwell); logf(" %f ", p_Layer->fPeckIncrement); }
		if (flagsb & 1) { logf(" mp"); logf(" %f ", p_Layer->fMicroPeckRot); logf(" %f ", p_Layer->fMicroPeckSetback); }
		if (flagsb & 16) { logf(" ha"); }
		if (flagsb & 2) {
			logf(" lc");
			logf(" %f ", p_Layer->fThrustBaselineDistance);
			logf(" %f ", p_Layer->fThrustMin);
			logf(" %f ", p_Layer->fThrustMax);
			logf(" %f ", p_Layer->fLcDeltaThrust);
		} else {
			logf(" tmm");
			logf(" %f ", p_Layer->fThrustMin);
			logf(" %f ", p_Layer->fThrustMax);
		}
		logf(" ab");logf(" %f ", p_Layer->fAbsoluteEnd);
		logf(" th");logf(" %f ", p_Layer->fThicknessMax);
		if (flags & 4) { logf(" sr"); }
		if (flags & 8) { logf(" l"); }
		if (flags & 16) { logf(" h2s"); }
		if (flags & 32) { logf(" csl"); }
		if (flags & 64) { logf(" hs"); }
		if (flags & 128) { logf(" ss"); }
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

		p_Layer++; //Increments the entire size of the struct
	    p_LayerOverride++; //increase one layer override
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

nodebug void PreviewDisplayProbeCommandDataBase()
{
	long xp;
    int i;
	int p;
    char * sbuf;
	char ck;

	xp = xp_ProbeCommandDataBase;
	if (xp==0)
	{
		logf("npcf\r\n");
		return;
	}

	ck=1;
	while(ck<=g_cKHoleCount)
	{
tcp_tick(NULL); // call tcp_tick
		i=xgetint(xp); xp+=2;
		logf("kc %d  %d  \r\n",ck,i);
		if (i<512)
		{
			sbuf = g_STPtxMsg.p_cObjectValue + STP_OBJVALUE_MAXSIZE - i - 2;
			xmem2root(sbuf,xp,i);
			p=0;
			while(p<i)
			{
				logf(" %d %d\r\n",p,sbuf[p]);
				p++;
			}
			logf("dl %d %d\r\n\r\n",p,i); //end check for issue with data kill???
		}
		else
		{
			//fatal????
			//FIXME0000 FIXMENOW zxcvzxcvzxcvzxcvzxcvzxcvzxcv
			logf("no show\r\n");
		}
		ck++;
		xp+=i;
	}
}

nodebug void LoadProbeCommand(char ckprobe)
{
	//go down the list and find this record....
	//This could be more efficient, but it's fast enough and can be done in a simpler way after port to new tech platform in 8 months from now...
	long xp;
    int i;
	int p;
    char * sbuf;
	char ck;

	char temptestchar[4];

	xp = xp_ProbeCommandDataBase;
	if (xp==0)
	{
		logf("npcf\r\n");
		//Clear all probe command details
		ClearProbeCommand();
		//treat like probe
		g_cProbeCommand = KEYWORD_Probe;
		return;
	}

	ck=1;
	while(ck<=g_cKHoleCount)
	{
		i=xgetint(xp); xp+=2;
		logf("kc %d  %d  \r\n",ck,i);
		if (ck==ckprobe)
		{
			if (i>512)
			{
				//fatal????
				//FIXME0000 FIXMENOW zxcvzxcvzxcvzxcvzxcvzxcvzxcv
				logf("no show\r\n");
				i=0;
			}
			if (i==0)
			{
				//Do like "Probe" only
				i=1;
				sbuf = g_STPtxMsg.p_cObjectValue + STP_OBJVALUE_MAXSIZE - i - 2;
				sbuf[0]=KEYWORD_Probe;
			}
			else
			{
				sbuf = g_STPtxMsg.p_cObjectValue + STP_OBJVALUE_MAXSIZE - i - 2;
				xmem2root(sbuf,xp,i);
			}
			ParseProbeCommand(ckprobe,sbuf,i);
			break;
		}
		ck++;
		xp+=i;
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

nodebug void ClearProbeCommand()
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
	g_fProbeMaxDistShift=0; //also means no custom limit
	g_cProbeShiftLimX=0;
	g_fProbeShiftLimXMin=0;
	g_fProbeShiftLimXMax=0;
	g_cProbeShiftLimY=0;
	g_fProbeShiftLimYMin=0;
	g_fProbeShiftLimYMax=0;
	g_fProbeExpectedDiameter=0;
}

nodebug void ParseProbeCommand(char ckprobe, char * sbuf, int i)
{
	int p;
    char c;
	float f;
	float f2;
	float fswap;

	ClearProbeCommand();
	p=0;
	if (i==0)
	{
		//treat like probe
		g_cProbeCommand = KEYWORD_Probe;
		goto parse_done;
	}
	//group 1 required
	c=sbuf[p++];
    if (c==KEYWORD_Probe)
	{
		g_cProbeCommand = c;
		//continue to group 2
	}
	else if (c==TKP_PROBE_EDGE_VEC)
	{
		g_cProbeCommand = c;
		if (p+8>i)
		{
			//missing parameter arguments
			g_cProbeCommandMessage=MINIFTMC_PROBE_COMMAND_PARAMETERS;
			goto parse_fail;
		}
		g_fProbeVectorX = *(float *)(sbuf+p); p+=4;
		g_fProbeVectorY = *(float *)(sbuf+p); p+=4;
		logf("%s %f,%f\r\n","pev",g_fProbeVectorX,g_fProbeVectorY);
		//continue to group 2
	}
	else if (c==TKP_PROBE_EDGE_MVEC)
	{
		g_cProbeCommand = c;
		if (p+8>i)
		{
			//missing parameter arguments
			g_cProbeCommandMessage=MINIFTMC_PROBE_COMMAND_PARAMETERS;
			goto parse_fail;
		}
		g_fProbeMachineVectorX = *(float *)(sbuf+p); p+=4;
		g_fProbeMachineVectorY = *(float *)(sbuf+p); p+=4;
		logf("%s %f,%f\r\n","pemv",g_fProbeMachineVectorX,g_fProbeMachineVectorY);
		//continue to group 2
	}
	else
	{
		//failure to match group 1
		g_cProbeCommandMessage=MINIFTMC_PROBE_COMMAND_INVALID;
		goto parse_fail;
	}
	//group 2 optional and group 3 required
	while (p<i) //more to parse
	{
		c=sbuf[p++];
		if (c==KEYWORD_Offset)
		{
			if (g_cProbeExtraOffsetGiven==1)
			{
				//already specified
				g_cProbeCommandMessage=MINIFTMC_PROBE_COMMAND_REPEAT;
				goto parse_fail;
			}
			g_cProbeExtraOffsetGiven=1;
			if (p+8>i)
			{
				//missing parameter arguments
				g_cProbeCommandMessage=MINIFTMC_PROBE_COMMAND_PARAMETERS;
				goto parse_fail;
			}
			g_fProbeExtraOffsetX = *(float *)(sbuf+p); p+=4;
			g_fProbeExtraOffsetY = *(float *)(sbuf+p); p+=4;
			logf("%s %f,%f\r\n","eo",g_fProbeExtraOffsetX,g_fProbeExtraOffsetY);
			//continue with group 2
			continue;
		}
		if (c==TKP_MACHINEOFFSET)
		{
			if (g_cProbeExtraMachineOffsetGiven==1)
			{
				//already specified
				g_cProbeCommandMessage=MINIFTMC_PROBE_COMMAND_REPEAT;
				goto parse_fail;
			}
			g_cProbeExtraMachineOffsetGiven=1;
			if (p+8>i)
			{
				//missing parameter arguments
				g_cProbeCommandMessage=MINIFTMC_PROBE_COMMAND_PARAMETERS;
				goto parse_fail;
			}
			g_fProbeExtraMachineOffsetX = *(float *)(sbuf+p); p+=4;
			g_fProbeExtraMachineOffsetY = *(float *)(sbuf+p); p+=4;
			logf("%s %f,%f\r\n","emo",g_fProbeExtraMachineOffsetX,g_fProbeExtraMachineOffsetY);
			//continue with group 2
			continue;
		}
		//group 3 required
		if (c==KEYWORD_Require)
		{
			//continue to group 4
			break;
		}
		//did not match anything in group 2 or required group 3
		//Expected Require
		g_cProbeCommandMessage=MINIFTMC_PROBE_COMMAND_OPTIONS_INVALID;
		goto parse_fail;
	}
	//group 4 optional
	while (p<i) //more to parse
	{
		c=sbuf[p++];
		if (c==KEYWORD_MaxDistShift || c==TKP_MachMaxDistShift)
		{
			if (p+4>i)
			{
				//missing parameter arguments
				g_cProbeCommandMessage=MINIFTMC_PROBE_COMMAND_PARAMETERS;
				goto parse_fail;
			}
			g_fProbeMaxDistShift = *(float *)(sbuf+p); p+=4;
			logf("%s %f\r\n","mds",g_fProbeMaxDistShift);

			//continue with group 4
			continue;
		}
		if (c==KEYWORD_ShiftLimX || c==TKP_MachShiftLimX)
		{
			if (p+4>i)
			{
				//missing parameter arguments
				g_cProbeCommandMessage=MINIFTMC_PROBE_COMMAND_PARAMETERS;
				goto parse_fail;
			}
			f = *(float *)(sbuf+p); p+=4;
			f2 = *(float *)(sbuf+p); p+=4;
			logf("%s %f %f\r\n","slx",f,f2);
			if (f>f2) { fswap=f;f=f2;f2=fswap; }
			g_cProbeShiftLimX = MACHINEXY;
			if (c==KEYWORD_ShiftLimX) { g_cProbeShiftLimX = DATAXY; }
			g_fProbeShiftLimXMin = f;
			g_fProbeShiftLimXMax = f2;
			//continue with group 4
			continue;
		}
		if (c==KEYWORD_ShiftLimY || c==TKP_MachShiftLimY)
		{
			if (p+4>i)
			{
				//missing parameter arguments
				g_cProbeCommandMessage=MINIFTMC_PROBE_COMMAND_PARAMETERS;
				goto parse_fail;
			}
			f = *(float *)(sbuf+p); p+=4;
			f2 = *(float *)(sbuf+p); p+=4;
			logf("%s %f %f\r\n","sly",f,f2);
			if (f>f2) { fswap=f;f=f2;f2=fswap; }
			g_cProbeShiftLimY = MACHINEXY;
			if (c==KEYWORD_ShiftLimY) { g_cProbeShiftLimY = DATAXY; }
			g_fProbeShiftLimYMin = f;
			g_fProbeShiftLimYMax = f2;
			//continue with group 4
			continue;
		}
		if (c==KEYWORD_AuxA || c==TKP_MachAuxA)
		{
			if (p+4>i)
			{
				//missing parameter arguments
				g_cProbeCommandMessage=MINIFTMC_PROBE_COMMAND_PARAMETERS;
				goto parse_fail;
			}
			//Incomplete because Auxa is reserved
			f = *(float *)(sbuf+p); p+=4;
			f2 = *(float *)(sbuf+p); p+=4;
			logf("%s %f %f\r\n","auxa",f,f2);
			//continue with group 4
			continue;
		}
		if (c==TKP_EXPECTED_DIAMETER)//FIXME incomplete must implement this some day
		{
			if (p+12>i)
			{
				//missing parameter arguments
				g_cProbeCommandMessage=MINIFTMC_PROBE_COMMAND_PARAMETERS;
				goto parse_fail;
			}
			g_fProbeExpectedDiameter = *(float *)(sbuf+p); p+=4;
			f = *(float *)(sbuf+p); p+=4;
			f2 = *(float *)(sbuf+p); p+=4;
			logf("%s %f,%f,%f\r\n","exd",g_fProbeExpectedDiameter,f,f2);

			//continue with group 4
			continue;
		}
		//did not match anything in group 4
		g_cProbeCommandMessage=MINIFTMC_PROBE_COMMAND_REQUIRE_INVALID;
		goto parse_fail;
	}
parse_done:
    //done
	return;
parse_fail:
	//set the command byte to invalid, and also return with the ProbeCommandMessage Set
	g_cProbeCommand=PROBE_COMMAND_INVALID;
	if (g_cProbeCommandMessage==0)
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


nodebug void ClearPositionsDuringProbe(char ki)
{
	//Any point that depends on this point should be set to no probe and recalculated later.
	int i;
	char c;
	char ck;
	char cki;
	char cpk;
	char csk;

	ck=1;
	while(ck<=g_cKHoleCount)
	{
		i=g_iKHoleHoleIndex[ck];
		GETKIKPKS(cki,cpk,csk,i); //loads both cpk and csk
		if (cki!=ck)
		{
			logf("cyclical check fail\r\n");
			g_cKHolePrbeStatus[ck]=PS_PROBED_FAILED;
			return;
		}
        if (cpk==ki || csk==ki)
		{
			//this khole depends on ki
			if (g_cKHolePrbeStatus[ck]!=PS_NO_PROBE)
			{
				g_cKHolePrbeStatus[ck]=PS_NO_PROBE;
				SendProbeStatus(STP_ALERT,ck); //SPS
			}
			if (g_cKHolePrbeStart[ck]!=PS_NO_PROBE)
			{
				g_cKHolePrbeStart[ck]=PS_NO_PROBE;
				SendProbeStart(STP_ALERT,ck); //SPS
			}
		}
		ck++;
	}
//FIXME Probe System Point Dependance.. if point dependance can be multi tiered, then this check must be done for all the derivative points....
	return;
}

nodebug void RecalculatePositionsDuringProbe(char ki)
{
	//currently just determine if any K Holes use ths K Holes for reference and update their approximate positions
//this needs to be refactored, however it will properly implement the required functionallity in a logical manner and can
//be consdered acceptable source for the reorg of probing that is likely to be needed after all the new features are filled in.
	int i;
	char c;
	char ck;
	char cki;
	char cpk;
	char csk;
	float fx;
	float fy;
logf("rpdp %d\r\n",ki);
	ck=1;
	while(ck<=g_cKHoleCount)
	{
		i=g_iKHoleHoleIndex[ck];
		GETKIKPKS(cki,cpk,csk,i); //loads both cpk and csk
//logf(" %d %d %d  %d %d\r\n",cki,cpk,csk, g_cKHolePrbeStatus[cpk], g_cKHolePrbeStatus[csk]);
		if (cki!=ck)
		{
			logf("cyclical check fail\r\n");
			g_cKHolePrbeStatus[ck]=PS_PROBED_FAILED;
			return;
		}
        if (cpk==ki || csk==ki)
		{
			//this khole depends on ki
			if (g_cKHolePrbeStatus[cpk]>=PS_PROBED)
			{
				if (g_cKHolePrbeStatus[csk]>=PS_PROBED)
				{
					//both are probed, so calculation could happen with this
					c=CreateMachineDataSetForOneHole(i, &fx,&fy);
					if (c==0)
					{
						g_cKHolePrbeStatus[ck]=PS_PROBED_FAILED; //set fail code (This should never happen if program is valid and the parents are probed.)
logf("fail\r\n");
					}
					else
					{
//FIXME HIGHEST SEVERE shouldn't a redo of the position FORCE a reprobe of this one????? Isn't this the point here????
//Under what case would this not need to be done?
						//make this location the APPROXIMATE LOCATION defined by it's k holes
						g_cKHolePrbeStart[ck]=PS_APPROXIMATE;
	                    g_fKHolePrbeStartX[ck]=fx;
    	                g_fKHolePrbeStartY[ck]=fy;
logf("saprx %d\r\n",ck);
					}
					//1st send start
					SendProbeStart(STP_ALERT,ck); //SPS
				}
			}
		}
		ck++;
	}
	return;
}

nodebug void CheckKLocationDistances(char ck, char mark)
{
    char ck2;
	unsigned int ui;
	unsigned int ui2;
    float fx,fy,fdx,fdy,fddx,fddy,fdatax,fdatay;
    float fDistanceData,fDistance;
	float fmd;
	float dlim;

	fmd=g_ConfigData.fMaxKholeDistanceError;

	dlim = g_ConfigData.fMaxKholeDistanceCheck;// limit for checking distance
    dlim = dlim*dlim; //compare with the squared value

	g_cKHolePrbeStatusDistance[ck]=0;//clear the distance error on this... it will get set back later in this function if there is an error

	ui = g_iKHoleHoleIndex[ck];
	//Get FlatXY data value for this hole....
	fdatax = xgetfloat(xp_fRawDataX+4*ui);
	fdatay = xgetfloat(xp_fRawDataY+4*ui);
	//load fx and fy with this probe position
	fx=g_fKHolePrbeX[ck];
	fy=g_fKHolePrbeY[ck];
	logf("%c %d %f %f %f %f\r\n",'a',ck,fdatax,fdatay,fx,fy);
	ck2=1;
	while(ck2<=g_cKHoleCount)
	{
    	if (ck2!=ck && g_cKHolePrbeStatus[ck2]>=PS_PENDING_ACCEPTANCE)
		{
           	//another Probed KHole
			ui2 = g_iKHoleHoleIndex[ck2];
			//Get FlatXY data diff values for this hole....
			fddx = fdatax - xgetfloat(xp_fRawDataX+4*ui2);
			fddy = fdatay - xgetfloat(xp_fRawDataY+4*ui2);
			//Find expected distance
			fDistanceData = fddx*fddx+fddy*fddy;
			if (fDistanceData < dlim || ck2==1 || ck2==g_cKHoleCount) //only need to pass test if distance is less than KHoleDistanceCheck, ck2==1 or ck2=g_cKHoleCount
			{

			fDistanceData = sqrt(fDistanceData);

			//Get Probe diff values
            fdx = fx - g_fKHolePrbeX[ck2];
			fdy = fy - g_fKHolePrbeY[ck2];
			//Find Probed distance
			fDistance = sqrt(fdx*fdx+fdy*fdy);
			if (fabs(fDistanceData - fDistance) > fmd)
			{
				//Error is too large.
				//SendKHoleDistance(STP_ALERT,fDistanceData,fDistance);
				if (g_cDistanceErrorFlagSent==0)
				{
					//send this on the 1st one only...
					SmartToolMsgFloat(STP_ALERT, OID_KHOLE_MAX_DISTANCE_ERROR, g_ConfigData.fMaxKholeDistanceError);
					g_cDistanceErrorFlagSent=1;
				}
				//Set Distance Error Flags
				g_cKHolePrbeStatusDistance[ck]=1;
				g_cKHolePrbeStatusDistance[ck2]=1;
				//Show 1st 4 distance errors
				if (g_cDistanceErrorShown<4)
				{
					logf("%c %d %f %f %f %f\r\n",'f',ck2, fdatax-fddx, fdatay-fddy, g_fKHolePrbeX[ck2], g_fKHolePrbeY[ck2]);
					g_cDistanceErrorShown++;
				}
			}
			else
			{
				//Acceptable
				//ck does not fail, and ck2 might be better now
				if (mark)
				{
					if (g_cKHolePrbeStatusDistance[ck2]==1)
					{
						//give ck2 a chance to be checked later when this returns
						g_cKHolePrbeStatusDistance[ck2]=0xFF;
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
nodebug void ResetNearestPosition()
{
  	g_PosnMode.iNearestPosn = -1; //make sure this is cleared
	g_PosnMode.fLastSentPosnX = -0.123456; //random so that it will send back whatever new position it gets.
}

//MACHINEPOINTS
nodebug int FindNearestPosition()
{
    //	Search for the nearest program position ID to current location in g_PosnMode.fLastKnownPosnX,Y.
    int i, imax, iClosestIndex;
	float fSearchX, fSearchY;

    float fX, fY, fCurDistSquared, fClosestSquared, fR;
    float fdx, fdy, fd, fbucket;
    float fdx2, fdy2;
    char cSkipKholes;
    char cSearchMode;
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

   	if (g_PosnMode.iNearestPosn>=0)
   	{
   		//Test to see if the previous search can be recycled
		//If this position is in a box 0.001 around the last calculated... just use that.
		if (fabs(g_PosnMode.fNearestCalculatedForMachineX-fSearchX)<g_fPosnLSCD &&
			fabs(g_PosnMode.fNearestCalculatedForMachineY-fSearchY)<g_fPosnLSCD)
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
    g_PosnMode.fNearestCalculatedForMachineX=fSearchX;
    g_PosnMode.fNearestCalculatedForMachineY=fSearchY;

    iClosestIndex = -1;
    fClosestSquared = 10000000000.0; // initialize to an impossibly large inch value

    i = 0;
    imax = g_iHoleCount;

   	if (g_PosnMode.iNearestPosn>=0)
   	{
	    //before we search the whole list, see how far we are from the last best one.
		if (i<=g_PosnMode.iNearestPosn && g_PosnMode.iNearestPosn<=imax)
		{
			//still valid... try this one 1st
			//set it to the initial first place
			fX=g_PosnMode.fNearestPosnMX;
			fY=g_PosnMode.fNearestPosnMY;
			iClosestIndex = g_PosnMode.iNearestPosn;
	   		fdx = fX - fSearchX;
	  		fdy = fY - fSearchY;
  	    	fClosestSquared = fdx*fdx + fdy*fdy;
            g_PosnMode.fNearestDistanceX=fdx;
            g_PosnMode.fNearestDistanceY=fdy;
            g_PosnMode.fNearestDistanceSquared=fClosestSquared; //save new value
	    	if (fClosestSquared <= g_PosnMode.fNearestRadiusExclusive) //Not truely the radius, but exclusive radius squared.
	    	{
	    		//the distance to this hole is inside it's exclusive radius
				//logf("S %f %f\r\n",fClosestSquared , g_PosnMode.fNearestRadiusExclusive);
				return g_PosnMode.iNearestPosn;
	    	}
		}
	}

	//For Drill / Drillfill, don't check kholes.
	if(g_PartPgmInfo.cTeachModeData!=1)
	{
		cSkipKholes=1;
	}

	g_PosnMode.iNearestPosn = -1; //Clear default

	cSearchMode=0;

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
    if (fbucket <0 )
    {
    	fbucket=0;
    }
    else if (fbucket>(MPOSNHASHSIZE-1))
    {
    	fbucket=MPOSNHASHSIZE-1;
    }

    bucket = (t_MPOSNBUCKETTYPE)fbucket;
	#ifdef OUTPUT_NEAREST_INFO
	logf("f=%f ibucket=%d\r\n", fbucket, bucket);
    #endif

    fd = bucket*g_fMPosnHashBucketsize + g_fMPosnHashMinBucket;
    fax = fSearchX - fd; //distance from X to next bucket....
    fd += g_fMPosnHashBucketsize;
    fbx = fd - fSearchX; //distance from X to next bucket....

    bucketa = bucket;
    bucketb = bucket;

   	//start this bucket
try_this_bucket:
	i = g_uiMPosnHashList[bucket];
    while(1)
    {
        if (i==-1)
        {
        	//This is Empty... find next bucket to check
            if (bucket==bucketa)
            {
            	//was doing a so do b now
                if (bucketb<MPOSNHASHSIZE-1)
                {
				return_to_b:
                	if (fbx*fbx < fClosestSquared)
                    {
	                	bucketb++;
		                bucket = bucketb;
        	            fbx+=g_fMPosnHashBucketsize;
		    	        goto try_this_bucket;
                    }
                    //advance to limit
					bucketb = MPOSNHASHSIZE-1;
                }
            }
            //try a
            if (bucketa > 0)
            {
	            if (fax*fax < fClosestSquared)
    	        {
	    	        bucketa--;
            		bucket = bucketa;
                    fax+=g_fMPosnHashBucketsize;
	            	goto try_this_bucket;
                }
                //advance to limit
                bucketa = 0;
            }
            //try b
            if (bucketb<MPOSNHASHSIZE-1)
            {
            	goto return_to_b;
            }
            //reached
            //reached the limits
            break;
        }
        //this is another position
	    if (cSkipKholes==1)
	    {
			if (getkind(i)>0) // Is KHole
			{
				//move to next in list
   	            goto next_in_bucket_list;
			}
		}

		fX = xgetfloat((xp_fRawDataMX)+(4*i));

		//Find X Distance from us
	   	fdx = fX - fSearchX;
		fdx2 = fdx*fdx;
		if (fdx2 > fClosestSquared)
		{
			//delta X alone is farther than our closest
            goto next_in_bucket_list;
		}

        fY = xgetfloat((xp_fRawDataMY)+(4*i));

		//Find Distance
		fdy = fY - fSearchY;
		fdy2 = fdy*fdy;
		fCurDistSquared = fdx2 + fdy2;
		if (cSearchMode==1)
        {
        	if (fCurDistSquared<0.0001)
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

			fR = xgetfloat((xp_fRawDataR)+(4*i));
			if (fR>0 && fR>fClosestSquared)
			{
				//We are inside this positions exclusive radius!
				//No more searching needed.
				if (cSearchMode==0)
    			{
					goto found_by_exclusiveradius;
                }
                goto found_er_by_exclusiveradius;
			}
		}
	next_in_bucket_list:
		i = xgetint(xp_uiMPosnHashList+2*i);
	}
    if (iClosestIndex==-1)
    {
		#ifdef OUTPUT_NEAREST_INFO
	    logf("nearest was not found\r\n");
        #endif
    }

	if (iClosestIndex>0)
	{
		if (cSearchMode==0)
    	{
			fX = xgetfloat((xp_fRawDataMX)+(4*iClosestIndex));
			fY = xgetfloat((xp_fRawDataMY)+(4*iClosestIndex));
	        fR = xgetfloat((xp_fRawDataR)+(4*iClosestIndex));
	        fdx = fSearchX - fX;
	        fdy = fSearchY - fY;
found_by_exclusiveradius:
			g_PosnMode.iNearestPosn = iClosestIndex;
			g_PosnMode.fNearestPosnMX = fX;
			g_PosnMode.fNearestPosnMY = fY;
        	g_PosnMode.fNearestDistanceX=fdx;//These are in machine coords, but the only place they are used,
   		    g_PosnMode.fNearestDistanceY=fdy;//is in the determination of what a long distance move is.
    	    g_PosnMode.fNearestDistanceSquared=fClosestSquared;
			#ifdef OUTPUT_NEAREST_INFO
	        logf("p=%f,%f n=%d  %f,%f\r\n",fSearchX,fSearchY,g_PosnMode.iNearestPosn,fX,fY);
            #endif
            //but have to lookup the dataset value.
			g_PosnMode.fNearestPosnDX = xgetfloat((xp_fRawDataX)+(4*iClosestIndex));
			g_PosnMode.fNearestPosnDY = xgetfloat((xp_fRawDataY)+(4*iClosestIndex));
			g_PosnMode.fNearestRadiusExclusive = fR;
    		if (fR == 0)
	    	{
        		//GO BACK and find the nearest radius exclusive
                //Do this by searching with this point as the center
		        cSearchMode=1;
				cSkipKholes=0;
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
found_er_by_exclusiveradius:
    	    iClosestIndex=g_PosnMode.iNearestPosn;
            //Set the exclusive radius for the hole (we store the value^2 though)
            //ClosestDistance ^ 2 = (ExclusiveRadius * 2) ^ 2
            //Therefore    ClosestDistance ^ 2 = 4* (ExclusiveRadius) ^ 2
            //Therefore    ClosestDistance ^ 2 / 4.0 = (ExclusiveRadius) ^ 2
			fR=fClosestSquared/4.0;
			g_PosnMode.fNearestRadiusExclusive=fR;
			if (fR==0)
			{
				fR=0.0000001; //so it will not trigger another search in the future.
			}
			xsetfloat((xp_fRawDataR)+(4*iClosestIndex),fR);
            //logf("Found exclusive radius for %d = %f\r\n",iClosestIndex,fR);
        }
	}
    return iClosestIndex;
}

nodebug int MoveAllowed(unsigned int uiSourceOid)
{
	if (g_cAction > ACTION_READY) //ACTION_IDLE==0 and ACTION_READY==1
	{
		//MiniFT is working and not ready to move
        SmartToolMsgMiniFtMessageCode(uiSourceOid, MINIFTMC_MOVE_PREVENTED);
		g_cActionSent = 0xFF; //Force Resend //If MiniFT is Alerted Ready, Buttons can be enabled again
        return 0;
	}
	if (g_cMoveDone != MOVEDONE_TRUE)
    {   // disallow move commands unless carriage is done with previous move
        SmartToolMsgMiniFtMessageCode(uiSourceOid, MINIFTMC_WAIT_FOR_CARRIAGE_STOP);
		g_cActionSent = 0xFF; //Force Resend
        return 0;
    }
    if (g_cFloatStatus == FLOATSTAT_FLOAT)
	{
        SmartToolMsgMiniFtMessageCode(uiSourceOid, MINIFTMC_MOVE_PREVENTED_BY_FLOAT);
		g_cActionSent = 0xFF; //Force Resend
        return 0;
    }
#ifdef GENCIRCMFTX
//Consider this for ALLPATTERN candidate
	if (g_cDrillState!=DRILLSTATE_IDLE)
    {
    	//no message because the pendant should be preventing the buttons, but
        //just in case, it won't allow movement
		g_cActionSent = 0xFF; //Force Resend
    	return 0;
    }
#endif
    return 1;
}

nodebug int SpecifyGotoPosn(int iGotoPosn, int iIndex)
{
    // "GotoPosn" is the next position as specified by the user...not necessarily the next sequential position in the
    // list.  For exmple, the "GotoPosn" could be the previous position, the next sequential position, or some randomly specified position
    // iIndex is ignored if iGotoPosn is not GOTOPOSN_RANDOM_INDEX (pass a 0 to the function)
    int iRetVal;
    int i, sr;
    int iPosnIndex;
    int d;
    unsigned int uiMessageCode;
	td_HoleData GotoHoleData;

    iRetVal = 0;
    iPosnIndex = g_PosnMode.iCurPosnIndex;

	uiMessageCode=0;
logf("station %c g %c\r\n",g_cStationCode[g_cStation],g_cStationCode[g_cStationGoal]);

    switch (iGotoPosn)
    {
        case GOTOPOSN_NEXT:
		case GOTOPOSN_PREV:
	        // DEFAULT: probed BTOA, which means we're walking through it oppositely (A to B)...PosnA is the first posn, PosnB is the final position
            d=0;
            if (g_iDrillDir == PROBEDIR_ATOB)
            {
	            d=1;
	        }
            if (g_iDrillDir == PROBEDIR_BTOA)
            {
                //we're walking through it oppositely (B to A)
            	d=-1;
            }
            if (iGotoPosn==GOTOPOSN_PREV)
            {
            	d= -d; //reverses dir in this case
            }
         	//If Init Move, Must go to 1st position since they never moved at all.
            //This is allready set into the current position variable
            //Otherwise, must move in the direction specified.
            if (g_PosnMode.cFirstMove != 1)
            {
				iPosnIndex+=d;
            }
	        // keep iRetVal unchanged
            //skip K Holes unless we are allowed to visit them

        	if (g_PartPgmInfo.cTeachModeData!=1) //KHoles are never visitable this way in normal mode g_cAllowKVisit==0
   	        {
				while(1)
				{
					loadholedata(iPosnIndex,&GotoHoleData);
    		        if(GotoHoleData.cPattern>=128) //This is restricted From Next and Prev Currently
	        	    {
   		        		//continue move past this khole
       		    		iPosnIndex+=d;
						continue;
					}
					if ((GotoHoleData.uiOps & 1) == 0)
					{
logf("nvo%d\r\n",iPosnIndex);
						//can't visit this hole
       		    		iPosnIndex+=d;
						continue;
					}
					break;
        	    }
			}

			if (iPosnIndex < 0 || iPosnIndex >= g_iHoleCount)
		    {
				iRetVal = GOTOPOSN_OUTOFBOUNDS;
        		if (iGotoPosn==GOTOPOSN_NEXT)
		        {
                	uiMessageCode=MINIFTMC_FINAL_POSN_KHOLE_VISIT;
			        //if they hit next on the last hole, we'll allow Khole visitation
			        //also can activate repeat
					//Allow K Hole Visitation
			   	    g_cAllowKVisit=1;
		        }
                else
                {
                	uiMessageCode=MINIFTMC_FINAL_POSN;
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
				uiMessageCode=MINIFTMC_GOTOPOSN_OUT_OF_BOUNDS;
            }
	        //skip K Holes unless we are allowed to visit them
			loadholedata(iPosnIndex,&GotoHoleData);
    		if (g_cAllowKVisit==0)
	        {
   		        if(GotoHoleData.cPattern>=128) //This is restricted From Next and Prev Currently
				{
	       			//continue move past this khole
					iRetVal = GOTOPOSN_KHOLEPREVENTED;
					uiMessageCode=MINIFTMC_KHOLE_MOVE_PREVENTED;
				}
    		}
			if ((GotoHoleData.uiOps & 1) == 0)
			{
logf("nvo%d\r\n",iPosnIndex);
				iRetVal = GOTOPOSN_KHOLEPREVENTED;
				uiMessageCode=MINIFTMC_HOLE_MOVE_PREVENTED;
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
            if (i>=0) //only set the new position if one was found.
            {
            	iPosnIndex = i;
    			if (fabs(g_PosnMode.fNearestDistanceX) >= g_ConfigData.LongDistance.fX ||
                	fabs(g_PosnMode.fNearestDistanceY) >= g_ConfigData.LongDistance.fY)
                {
                	//This is too far for GOTOPOSN_NEAREST TO ALLOW
					iRetVal = GOTOPOSN_TOOFAR;
					uiMessageCode=0; //no message... just don't move to nearest
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
    if (uiMessageCode!=0)
    {
    	//show the message
        SmartToolMsgMiniFtMessageCode(0, uiMessageCode);
		g_cActionSent = 0xFF; //Force Resend //If the move was prevented somehow, sending READY again will allow buttons to be enabled
    }
	logf("t=%d\r\n",g_PosnMode.cDoMoveTrig);
logf("station %c g %c\r\n",g_cStationCode[g_cStation],g_cStationCode[g_cStationGoal]);
    return iRetVal;
}

nodebug void SpecifyGotoPosnAgain()
{
	int iPosnIndex;
    iPosnIndex = g_PosnMode.iCurPosnIndex;
   	g_PosnMode.iGotoPosnIndex = iPosnIndex;
	g_PosnMode.cDoMoveTrig = TRUE;
}

nodebug void SetPressureBasedOnProcess(char cProcess)
{
	if (cProcess == 0)
    {
		g_uiClampPressure = g_ConfigData.uiPressure;
		g_uiClampPressureWarning = 0;
		g_uiClampPressureAbort = 0;
		logf("Pdef=%d\r\n", g_uiClampPressure, cProcess );
    }
    else
    {
		g_uiClampPressure = g_uiProcessPounds[cProcess];
		g_uiClampPressureWarning = g_uiProcessPoundsWarning[cProcess];
		g_uiClampPressureAbort = g_uiProcessPoundsAbort[cProcess];
		logf("Pproc=%d p=%d\r\n", cProcess, g_uiClampPressure );
    }
#warnt "what if NAC is disconnected? "
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
nodebug void SetClampPressureLBS()
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
	if (f>10.05) { f=10.05; }

	logf("PRESSURE SET : pounds %d   volts %.3f \r\n",g_uiClampPressureLBS,f);
	anaOutVolts(DAC_CHANNEL_00, f);
	g_uiClampPressureLBSLastSet=g_uiClampPressureLBS;
}
#endif

nodebug void CalculatePreMove(float* p_fPreMoveX, float* p_fPreMoveY)
{
    if (g_GravComp.iDirX > 0)
    {   // gravcomp vector is +X (which means the physical gravity vector is -X)
        // Always move below the target
        *p_fPreMoveX = - g_ConfigData.PreMove.fX;  // make the pre-move negative so that we go below the target
    }
    else
    {   // gravcomp vector is -X (which means the physical gravity vector is +X)
        // Always move below the target
        *p_fPreMoveX = g_ConfigData.PreMove.fX;   // keep the pre-move positive so that we go below the target
    }

    if (g_GravComp.iDirY > 0)
    {   // gravcomp vector is +Y (which means the physical gravity vector is -Y)
        // Always move below the target
        *p_fPreMoveY = - g_ConfigData.PreMove.fY;  // make the pre-move negative so that we go below the target
    }
    else
    {   // gravcomp vector is -Y (which means the physical gravity vector is +Y)
        // Always move below the target
        *p_fPreMoveY = g_ConfigData.PreMove.fY;   // keep the pre-move positive so that we go below the target
    }
    logf("PreMove: X=%.3f Y=%.3f\r\n",*p_fPreMoveX, *p_fPreMoveY);//debug
}

nodebug void CalcProbeHome( void )
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
	RailOriginY = g_Probe.fHomeY - g_ConfigData.PosnLimit.fMinY;	// Y location of rail coordinate origin

	g_fKHolePrbeY[1]=RailOriginY + g_PartPgmInfo.K1OriginDistY; // K1.Y in machine coordinates.

	if(g_Probe.cXBumperDirection == X_BUMP_NEG)
    {
    	// We are at lower end of X travel, at the rail Cartesian origin.
		RailOriginX = g_Probe.fHomeX - g_ConfigData.PosnLimit.fMinX; // X location of right end of rail (not signed in Rivet)(so positive makes more to side)
		g_fKHolePrbeX[1] = RailOriginX + g_PartPgmInfo.K1OriginDistX; // K1.X in machine coordinates.
    }
	else if(g_Probe.cXBumperDirection == X_BUMP_POS)
	{
		// We are at far end of X travel (Xmax).
		RailOriginX = g_Probe.fHomeX + g_ConfigData.PosnLimit.fMaxX; // X location of end of rail
		g_fKHolePrbeX[1] = RailOriginX - g_PartPgmInfo.K1EndDistX; // K1.X in machine coordinates.
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
	logf( " KPMachinePosn= %.3f, %.3f\r\n",	g_fPKMX, g_fPKMY);
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
nodebug int CalcProbeAdjust()
{
	float	fdX, fdY;		// Temporary to hold result of difference calculation.
	float	fProbedMachineX, fProbedMachineY;
	float	fExpMachineX, fExpMachineY;
    int		i;				// Index in pattern of nearest point to curent location

	//	PROBE_ADJUST was done with the laser head, so account for the laser head offset from hammer head.
	fProbedMachineX = g_Probe.fProbeAdjustX;
   	fProbedMachineY = g_Probe.fProbeAdjustY;

	//	Ask FindNearestPosition() to look for the nearest dataset location to the rivet laser location
	ResetNearestPosition(); // Do not try to match to the previous nearest position.
    //Search with the offset, since we set the laser offset as a tool offset
	g_PosnMode.fLastKnownPosnX = fProbedMachineX;
	g_PosnMode.fLastKnownPosnY = fProbedMachineY;
    logf("ProbeAdjust: Probed machine location = %.3f,%.3f.\r\n", fProbedMachineX, fProbedMachineY);
    //But Remove the offset from these to find the value we would have if the tool was up there
   	RemoveMachineOffset(&fProbedMachineX,&fProbedMachineY);
    logf("ProbeAdjust: When offset was removed = %.3f,%.3f.\r\n", fProbedMachineX, fProbedMachineY);

	i = FindNearestPosition();
	if (i<0)		//	If returned index into dataset is less than zero, no nearest point was found.
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
nodebug int CalcProbeAdjust()
{
	return -2;
}
#endif

//SetGlobalRotation(char cpk, char csk)

//Global Machine Rotation System
//Set and Cache Globals needed for rotation
//Used both for creating of the Machine points, and for tranformation
// of points from machine data back into data set coordinates.

//FIXME66666666666666666666666 when will it get cleared and stop taking effect??????
//FIXME66666666666666666666666
//Tie in better with probe clearing etc....
//FIXME66666666666666666666666
//Perhaps change setglobal argument system....

nodebug void SetGlobalRotation(char cpk, char csk)
{
	int ipk,isk;
    float fmd,fmdx,fmdy;
    float fd,fdx,fdy;

	if (cpk!=g_cRotationKP || csk!=g_cRotationKS)
    {
    	//Update the Global Setting Indicators
		g_cRotationKP = cpk;
		g_cRotationKS = csk;

    	if (cpk==0)
        {
        	//no k holes yet.
			g_cRotationContext = 0;
            return;
        }

		g_cRotationContext = PS_PROBED;

		if (g_cKHolePrbeStatus[cpk]>=PS_PROBED)
		{
	    	g_fPKMX = g_fKHolePrbeX[cpk];
    	    g_fPKMY = g_fKHolePrbeY[cpk];
		}
		else if (g_cKHolePrbeStart[cpk]==PS_APPROXIMATE)
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
	    g_fPKX = xgetfloat((xp_fRawDataX)+(4*ipk));
   		g_fPKY = xgetfloat((xp_fRawDataY)+(4*ipk));

        logf("cpk=%d ipk=%d x,y= %f %f\r\n",cpk,ipk,g_fPKX,g_fPKY);

        if (csk==0)
        {
        	//just one K Hole... Only use the K position we just loaded
			g_cRotationContext = 0;
            return;
        }

		if (g_cKHolePrbeStatus[csk]>=PS_PROBED)
		{
		    g_fSKMX = g_fKHolePrbeX[csk];
	    	g_fSKMY = g_fKHolePrbeY[csk];
		}
		else if (g_cKHolePrbeStart[cpk]==PS_APPROXIMATE)
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
        g_fSKX = xgetfloat((xp_fRawDataX)+(4*isk));
	   	g_fSKY = xgetfloat((xp_fRawDataY)+(4*isk));

        logf("csk=%d isk=%d x,y= %f %f\r\n",csk,isk,g_fSKX,g_fSKY);

	    //Find Cos and Sin Theta for this pair
    	//Find Machine delta from the probed positions
		fmdx = g_fSKMX - g_fPKMX;
    	fmdy = g_fSKMY - g_fPKMY;
        //Find the distance in machine coordinates
	    fmd = sqrt(fmdx*fmdx + fmdy*fmdy);
		if (fmd <0.001)
		{
    		//Has to be a failure...
            //just set KS to nothing
            g_cRotationKS = 0;

			//Clear Probe and reset.
			SmartToolMsgMiniFtMessageCode(0, MINIFTMC_ROTATION_POINTS_TOO_CLOSE);
            logf("Pnts close!\r\n");

			if (g_cAction == ACTION_CALC)
			{
				logf("to IDLE\r\n");
				StartOver();//SPSALL
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
    	fd = sqrt(fdx*fdx + fdy*fdy);
        if (fd < 0.001)
	    {
    		//Has to be a failure
            //Clear Probe and reset.
			SmartToolMsgMiniFtMessageCode(0, MINIFTMC_ROTATION_POINTS_TOO_CLOSE);
            StartOver();//SPSALL
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
		if (g_ConfigData.cScaleMode==0)
		{
		#endif

		//cosine of unit vectors:
		// cosine theta = ax * bx + ay * by
		g_fCosTheta = fmdx * fdx + fmdy * fdy;

		//sine of unit vectors:
		// sine theta = ax * by - ay * bx
		g_fSinTheta = fmdx * fdy - fmdy * fdx;

	    logf(" theta c s = %f %f\n",g_fCosTheta,g_fSinTheta);

		#ifdef LINEAR_SCALE_OPTION
	 	}
		else
		{
		g_fScale = fmd / fd;
		g_fUnscale = fd / fmd;

		//The Cos and Sin for rotation from X,0 to fmdx,fmdy
		g_fCosScale = fmdx;
		g_fSinScale = fmdy;
		logf(" scale c s = %f %f\r\n",g_fCosScale,g_fSinScale);

		//The Cos and Sin for rotation from X,0 to fdx,fdy
		g_fCosTheta = fdx;
		g_fSinTheta = fdy;
		logf(" theta c s = %f %f\n",g_fCosTheta,g_fSinTheta);
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

nodebug void StartCreateMachineDataSet()
{
	int i;
	g_ulMachineDataSetIDTimeSec = SEC_TIMER;
	g_ulMachineDataSetIDTime = MS_TIMER;
	g_ulCreateMachineDataSetStartTime = MS_TIMER;
	g_uiCreateMachineDataSetIndex=0;
	g_uiCreateMachineDataSetHashIndex=0;

	//Stop SendXYData (because they will need new data)
	g_cSendXYDataSessions = 0;
	i = 0;
    while(i<MAXNUM_STP_SESSIONS)
    {
    	g_cSendXYDataSession[i] = 0;
		i++;
	}
	//Alert new MachineDataSetID now
	SendXYDataID(STP_ALERT);

    //FIXME000000000000 make sure that these can't be used for lookup during this time....
    // NO lookup during this time....

	g_fMPosnHashMinBucket=g_fKHolePrbeX[1]; //use first khole as a start for min and max
	g_fMPosnHashMaxBucket=g_fKHolePrbeY[1];

    //Clear the Hash List
	i=0;
    while(i<MPOSNHASHSIZE)
    {
		g_uiMPosnHashList[i]=-1;
		i++;
    }

    //Clear this so that it will recalculate them all here
    g_cRotationKP = 0;
    g_cRotationKS = 0;
}

nodebug void CreateMachineDataSet()
{
	unsigned int i,ki,istart,imax;
    char ck;
	char cki;
    char cpk;
    char csk;
    int ipk;
    int isk;
    char *p_c;
	char cwarn;
	unsigned int ui;

    // DataSet to Machine
    float fX, fY, frX, frY, fsrX, fsrY;       //  temporary variables for rotated location.
    float f;
    unsigned long ul;

    istart=g_uiCreateMachineDataSetIndex;
    imax=istart+64;
    if (imax>=g_iHoleCount)
    {
    	imax=g_iHoleCount;
    }

	if (g_PartPgmInfo.cTeachModeData == 1)
	{
		i = 0;
    	imax=g_iHoleCount;
		while (i<imax)
		{
			//MX DATA
			frX = xgetfloat(xp_fRawDataMX+4*i);
		    //MY DATA
			frY = xgetfloat(xp_fRawDataMY+4*i);
    	    if (frX<g_fMPosnHashMinBucket) { g_fMPosnHashMinBucket=frX; }
	        else if (frX>g_fMPosnHashMaxBucket) { g_fMPosnHashMaxBucket=frX; }
         	i++;
		}
		g_uiCreateMachineDataSetIndex = g_iHoleCount;
		goto skip_standard_create_machine_dataset;
	}
    //Now For all points in the pattern, set the rotated value

    //Use Rotate but not Tool Offset....
    //These are machine coords for the 0,0 position ...
    i=istart;
    while(i<imax)
    {
		GETKIKPKS(cki,cpk,csk,i); //loads both cpk and csk
		if (cki>0)
        {
        	//It's a K Hole or other Locating feature... use the value established by the probe system
			frX = g_fKHolePrbeX[cki]; //use first khole as a start for min and max
			frY = g_fKHolePrbeY[cki];
        }
        else
        {

		if (cpk!=g_cRotationKP || csk!=g_cRotationKS) //do check here to avoid the call even though the function also checks
        {
        	SetGlobalRotation(cpk, csk);
        }

		//Load the Point we are going to translate into machine coordinates

        fX = xgetfloat((xp_fRawDataX)+(4*i));
   		fY = xgetfloat((xp_fRawDataY)+(4*i));
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
		if (g_ConfigData.cScaleMode==0)
        {
		#endif

		//Use the reverse values since the values are meant for Machine to Dataset, and we are going back.
		//cos(-x) = cos(x) //no change needed
		//sin(-x) = -sin(x) //reverse sign on second term to reverse rotation.
		frX=fX * g_fCosTheta + fY * g_fSinTheta;
		frY=fY * g_fCosTheta - fX * g_fSinTheta;
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
		fsrX=fX * g_fCosTheta + fY * g_fSinTheta;
		fsrY=fY * g_fCosTheta - fX * g_fSinTheta;
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
		xsetfloat(xp_fRawDataMX+4*i,frX);
	    //MY DATA
		xsetfloat(xp_fRawDataMY+4*i,frY);

		//and while doing this, set warnings...
		cwarn=0;
		if (cki>0)
        {
        	//It's a K Hole or other Locating feature... use the value established by the probe system
			cwarn=g_cKHolePrbeStatusWarnings[cki]; //just copy
		}
		if (cpk>0 && cwarn==0)
		{
			//It has a primary locating hole
			cwarn=g_cKHolePrbeStatusWarnings[cpk];
		}
		if (csk>0 && cwarn==0)
		{
			//It has a secondary locating hole
			cwarn=g_cKHolePrbeStatusWarnings[csk];
		}
		if (cwarn>0)
		{
			//it has a warning
			//Use Operations to record this
			ui=OP_PROBE_WARNINGS;
			AddOpHistory(i,ui);
logf("h%d opw\r\n");
		}

        if (frX<g_fMPosnHashMinBucket) { g_fMPosnHashMinBucket=frX; }
        else if (frX>g_fMPosnHashMaxBucket) { g_fMPosnHashMaxBucket=frX; }
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
	    ul = MS_TIMER - g_ulCreateMachineDataSetStartTime;
    	f=((float)ul / (float)i);
	    logf("%d items %lu ms  %f per item\r\n",i,ul,f);
    }
    return;
}

nodebug void CreatePosnHashList()
{
	unsigned int i,istart,imax;

    float f;
    float finverted;
    t_MPOSNBUCKETTYPE bucket;
    unsigned long ul;

    istart=g_uiCreateMachineDataSetHashIndex;
    imax=istart+128;
    if (imax>=g_iHoleCount)
    {
    	imax=g_iHoleCount;
    }
	if (istart==0)
    {
    	//first one of these, so reset time for time test
		g_ulCreateMachineDataSetStartTime = MS_TIMER; //get diff time for 2nd part.
    }

    finverted = 1.0 / g_fMPosnHashBucketsize;

    i=istart;
    while(i<imax)
    {
	    f = xgetfloat(xp_fRawDataMX+4*i);
        //logf("X=%f ",f);
        f -= g_fMPosnHashMinBucket;

        //f = f / g_fMPosnHashBucketsize;
        f = f * finverted; //multiplication is faster

        bucket = (t_MPOSNBUCKETTYPE)f;
		//logf("f=%f ibucket=%d\r\n", f, bucket);

        //link the current bucket contents
		xsetint(xp_uiMPosnHashList+2*i, g_uiMPosnHashList[bucket]);
        //replace the bucket
        g_uiMPosnHashList[bucket]=i;

        i++;
	}
    g_uiCreateMachineDataSetHashIndex=i;

    if (g_uiCreateMachineDataSetHashIndex >= g_iHoleCount)
    {
    	//Must have just completed this loop
		#ifdef OUTPUT_POSITION_INFO
	    g_uiProgramRotationOutputIndex=0;
	    #endif

	    i = g_iHoleCount;
	    ul = MS_TIMER - g_ulCreateMachineDataSetStartTime;
    	f=((float)ul / (float)i);
	    logf("%d items %lu ms  %f per item\r\n",i,ul,f);
    }
    return;
}

#ifdef OUTPUT_POSITION_INFO
nodebug void ShowProgramRotationOutput()
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

nodebug char CreateMachineDataSetForOneHole(unsigned int i, float * fpX, float * fpY)
{
	unsigned int ki,istart,imax;
    char ck;
	char cki;
    char cpk;
    char csk;
    int ipk;
    int isk;
    char *p_c;

    // DataSet to Machine
    float fX, fY, frX, frY, fsrX, fsrY;       //  temporary variables for rotated location.
    float f;
    unsigned long ul;
	char result;

	result = 0; //invalid

    if (i>=g_iHoleCount)
    {
    	return 0;
    }

	if (g_PartPgmInfo.cTeachModeData == 1)
	{
		//MX DATA
		frX = xgetfloat(xp_fRawDataMX+4*i);
	    //MY DATA
		frY = xgetfloat(xp_fRawDataMY+4*i);
		result=1;
		goto skip_strait_to_output;
	}
    //Use Rotate but not Tool Offset....
    //These are machine coords for the 0,0 position ...

		GETKIKPKS(cki,cpk,csk,i); //loads both cpk and csk
		if (cpk==0)
        {
        	//It's a K Hole or other Locating feature that has No References
			frX = g_fKHolePrbeX[cki]; //use first khole as a start for min and max
			frY = g_fKHolePrbeY[cki];
			//this is not a derived hole that is placed accurately based on other features, so return 0 for the purposes of this function
			goto skip_strait_to_output;
        }
        else
        {

		if (cpk!=g_cRotationKP || csk!=g_cRotationKS) //do check here to avoid the call even though the function also checks
        {
        	SetGlobalRotation(cpk, csk);
        }
		if (g_cRotationContext < PS_PENDING_ACCEPTANCE )
		{
			//can't find exact machine data set for this hole
			logf("sta\r\n");
			return 0;
		}

		//Load the Point we are going to translate into machine coordinates

        fX = xgetfloat((xp_fRawDataX)+(4*i));
   		fY = xgetfloat((xp_fRawDataY)+(4*i));
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
		if (g_ConfigData.cScaleMode==0)
		{
		#endif
		//Use the reverse values since the values are meant for Machine to Dataset, and we are going back.
		//cos(-x) = cos(x) //no change needed
		//sin(-x) = -sin(x) //reverse sign on second term to reverse rotation.
		frX=fX * g_fCosTheta + fY * g_fSinTheta;
		frY=fY * g_fCosTheta - fX * g_fSinTheta;
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
		fsrX=fX * g_fCosTheta + fY * g_fSinTheta;
		fsrY=fY * g_fCosTheta - fX * g_fSinTheta;
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
		result=1; //valid

        }
skip_strait_to_output:
	*fpX = frX;
	*fpY = frY;
    return result;
}

//Rotate this data vec to machine vec in the same orientation of this hole's dataset.
nodebug char RotateVecDataSetToMachine(unsigned int i, float * fpX, float * fpY)
{
	unsigned int ki,istart,imax;
    char ck;
	char cki;
    char cpk;
    char csk;
    int ipk;
    int isk;
    char *p_c;

    // DataSet to Machine
    float fX, fY, frX, frY, fsrX, fsrY;       //  temporary variables for rotated location.
    float f;
    unsigned long ul;
	char result;

	result = 0; //invalid

    if (i>=g_iHoleCount)
    {
		return result;
    }

	if (g_PartPgmInfo.cTeachModeData == 1)
	{
		//no rotation
		result=1;
		return result;
	}
    //Use Rotate but not Tool Offset....
    //These are machine coords for the 0,0 position ...

	GETKIKPKS(cki,cpk,csk,i); //loads both cpk and csk
	if (cpk==0)
    {
		//this is not a derived hole that is placed accurately based on other features, so return 0 for the purposes of this function
		return result;
    }

	if (cpk!=g_cRotationKP || csk!=g_cRotationKS) //do check here to avoid the call even though the function also checks
	{
		SetGlobalRotation(cpk, csk);
	}
	if (g_cRotationContext < PS_APPROXIMATE )
	{
		//can't find exact vector OR even approximate vector for this
		logf("sta\r\n");
		return result;
	}

	fX = *fpX;
	fY = *fpY;
	logf("Vec %f,%f\r\n",fX,fY);

	//Rotate Dataset to Machine
	#ifdef LINEAR_SCALE_OPTION
	if (g_ConfigData.cScaleMode==0)
	{
	#endif
	//Use the reverse values since the values are meant for Machine to Dataset, and we are going back.
	//cos(-x) = cos(x) //no change needed
	//sin(-x) = -sin(x) //reverse sign on second term to reverse rotation.
	frX=fX * g_fCosTheta + fY * g_fSinTheta;
	frY=fY * g_fCosTheta - fX * g_fSinTheta;

	#ifdef LINEAR_SCALE_OPTION
	}
	else
	{
    //The CosTheta and SinTheta for rotation from X,0 to fdx,fdy
   	//Use Reverse Rotation to orient the dataset fdx,fdy along X
	//logf("Dataset Prior %f %f\r\n",fX,fY);
	fsrX=fX * g_fCosTheta + fY * g_fSinTheta;
	fsrY=fY * g_fCosTheta - fX * g_fSinTheta;
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
	result=1; //valid
    return result;
}

//Rotate this vec from machine orientation into data set orientation
nodebug char RotateVecMachineToDataSet(unsigned int i, float * fpX, float * fpY)
{
	unsigned int ki,istart,imax;
    char ck;
	char cki;
    char cpk;
    char csk;
    int ipk;
    int isk;
    char *p_c;

    // DataSet to Machine
    float fX, fY, frX, frY, fsrX, fsrY;       //  temporary variables for rotated location.
    float f;
    unsigned long ul;
	char result;

	result = 0; //invalid

    if (i>=g_iHoleCount)
    {
		return result;
    }

	if (g_PartPgmInfo.cTeachModeData == 1)
	{
		//no rotation
		result=1;
		return result;
	}
    //Use Rotate but not Tool Offset....
    //These are machine coords for the 0,0 position ...

	GETKIKPKS(cki,cpk,csk,i); //loads both cpk and csk
	if (cpk==0)
    {
		//this is not a derived hole that is placed accurately based on other features, so return 0 for the purposes of this function
		return result;
    }

	if (cpk!=g_cRotationKP || csk!=g_cRotationKS) //do check here to avoid the call even though the function also checks
	{
		SetGlobalRotation(cpk, csk);
	}
	if (g_cRotationContext < PS_APPROXIMATE )
	{
		//can't find exact vector OR even approximate vector for this
		logf("sta\r\n");
		return result;
	}

	fX = *fpX;
	fY = *fpY;
	logf("Vec %f,%f\r\n",fX,fY);

	//Rotate Machine to Dataset
	#ifdef LINEAR_SCALE_OPTION
	if (g_ConfigData.cScaleMode==0)
	{
	#endif

    //Rotate
    frX=fX * g_fCosTheta - fY * g_fSinTheta;
    frY=fY * g_fCosTheta + fX * g_fSinTheta;

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
	frX=fsrX * g_fCosTheta - fsrY * g_fSinTheta;
	frY=fsrY * g_fCosTheta + fsrX * g_fSinTheta;
	//logf("Dataset Post %f %f\r\n",frX,frY);
	}
	#endif

	logf("Rot to %f,%f\r\n",frX,frY);
	*fpX = frX;
	*fpY = frY;
	result=1; //valid
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

nodebug void ProbeModeSelectAndSetGlobalRotation()
{
	char cStatus;
	char ck1;
    char ck2;
    char ck1a;
    char ck2a;
    char ck;

	//While Probing we need to maintain the rotation system so that the displayed coordinates
    //are useful to the operator.

    //Find the nearest probed K Holes, and nearest approx K Holes as an alternate.
    ck1=0;
	ck2=0;
    ck1a=0;
    ck2a=0;
    ck=1;
//FIXME6666666666666666 find the nearest not just the 1st......
	while(ck<=g_cKHoleCount)
    {
		//FIXME0000000000000 notice same-XY assmption in all code here
        cStatus = g_cKHolePrbeStatus[ck];
		if (cStatus>=PS_PENDING_ACCEPTANCE)
		{
			if (ck1==0)
			{
				ck1=ck; //save this as k1 in this flatXY
			}
			else if (ck2==0)
			{
				ck2=ck; //save this as k2 in this flatXY
			}
		}
        else if (cStatus>=PS_EXTRAPOLATED && cStatus!=PS_PROBING)
        {
			if (ck1a==0)
            {
            	ck1a=ck;
            }
            else if (ck2a==0)
            {
            	ck2a=ck;
            }
        }
		ck++;
	}
    if (ck1 == 0)
    {
    	//didn't find any probed... use best approx
        ck1=ck1a;
        ck2=ck2a;
    }
    else if (ck2 == 0)
    {
    	//found one probed... use approx for number 2
    	ck2=ck1a;
    }
    //Now whatever we found, use this as the current global rotation.
	if (g_cRotationKP==ck1 && g_cRotationKS==ck2)
	{
		//force a recalculate inside SetGlobalRotation
		g_cRotationKP=0;
		g_cRotationKS=0;
	}
   	SetGlobalRotation(ck1, ck2);
}


//RotateMachineToDataset()
//
//This function provides coordinate translation from the machine position to data set positions.
//The rotation used is the rotation established by the Global Rotation System.
//During Probe Mode, the global rotation system is set by ProbeModeSelectAndSetGlobalRotation()

nodebug void RotateMachineToDataset(float fX, float fY, float* p_fRotatedX, float* p_fRotatedY)
{
    float frX,frY,fsrX,fsrY;	// Temporary rotated dataset location.
    // Machine to DataSet

	#ifdef OUTPUT_POSITION_INFO_MACHINE_TO_DATASET
	logf(">>>RM2D: M= %.4f, %.4f",fX,fY);
	#endif

	//Always use the machine offset value given
   	RemoveMachineOffset(&fX,&fY);

//FIXME MED TEACH MODE
// Fixme : I want to get rid of that teach mode flag if possible and use the g_cKHoleP here.....
	if (g_PartPgmInfo.cTeachModeData==1 || g_cRotationKP == 0)
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
	if (g_ConfigData.cScaleMode==0)
	{
	#endif

    //Rotate
    frX=fX * g_fCosTheta - fY * g_fSinTheta;
    frY=fY * g_fCosTheta + fX * g_fSinTheta;
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
	frX=fsrX * g_fCosTheta - fsrY * g_fSinTheta;
	frY=fsrY * g_fCosTheta + fsrX * g_fSinTheta;
	//logf("Dataset Post %f %f\r\n",frX,frY);
	}
	#endif

    //Translate 0,0 to dataset position matching the machine position used above
	frX+=g_fPKX;
    frY+=g_fPKY;

	#ifdef OUTPUT_POSITION_INFO_MACHINE_TO_DATASET
	logf("  D= %.4f, %.4f\r\n",frX,frY);
 	#endif

    *p_fRotatedX = frX;
    *p_fRotatedY = frY;
    return;
}

nodebug void ApplyMachineOffset(float * p_fX,float * p_fY)
{
	float fx,fy,fy1;
	float fyadj;
	fx=*p_fX;
	fy=*p_fY;

	if (g_cMachineOffsetCompensationAdjustment==0)
	{
		//just add normal offsets
		fx+=g_MachineOffset.fX;
		fy+=g_MachineOffset.fY;
	}
	else
	{
		//must apply special offset
		//find offset per inch
		fyadj=(g_MachineOffset2.fY - g_MachineOffset1.fY) / (g_MachineOffset2.fYExtension - g_MachineOffset1.fYExtension);

		//current extension = (fy - base)
		//extension passed offset one extension = (fy - base) - offset one extension

		fy1=fy;
		fx+=g_MachineOffset1.fX;
		fy = fy + g_MachineOffset1.fY + fyadj * ((fy - g_fMachineBaseExtension) - g_MachineOffset1.fYExtension);
logf("\r\n adj %.4f bo %.4f y %.4f %.4f \r\n",fyadj,(fy1 - g_fMachineBaseExtension),fy1,fy);
	}
	*p_fX=fx;
	*p_fY=fy;
}

nodebug void RemoveMachineOffset(float * p_fX,float * p_fY)
{
	float fx,fy;
	float fyadj;
	fx=*p_fX;
	fy=*p_fY;

	if (g_cMachineOffsetCompensationAdjustment==0)
	{
		//just normal offsets
		fx-=g_MachineOffset.fX;
		fy-=g_MachineOffset.fY;
	}
	else
	{
		//special offset
		//find offset per inch
		fyadj=(g_MachineOffset2.fY - g_MachineOffset1.fY) / (g_MachineOffset2.fYExtension - g_MachineOffset1.fYExtension);

		//Backwards version of above (solve for rvalue fy in above to get this equation)
		fx-=g_MachineOffset1.fX;
		fy = ( fy - g_MachineOffset1.fY + fyadj*(g_fMachineBaseExtension + g_MachineOffset1.fYExtension) ) / (1+fyadj);

	}
	*p_fX=fx;
	*p_fY=fy;
}


nodebug void SetToolOffset(float fToolOffsetX,float fToolOffsetY)
{
    td_MachineOffset * p_MachineOffset;
	//flipping controls an extra 180 degree rotation.
	//if tool is in pos direction, flip only for Tool to Movement conversion (machine offset if oposite tool offset.)
	//if tool is in neg direction, flip for Tool to Movement conversion, and flip again since tool is oriented on oposite side 180.
	//	two flips cancel out (-1 * -1 = 1) so do nothing.
	g_cMachineOffsetCompensationAdjustment=0;
	if (g_ConfigData.cToolFlip==Y_POS)
	{
		g_MachineOffset.fX = -fToolOffsetX;
		g_MachineOffset.fY = -fToolOffsetY;
	}
	else if(g_ConfigData.cToolFlip==Y_NEG)
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
    p_MachineOffset=(td_MachineOffset *)g_STPtxMsg.p_cObjectValue;
	p_MachineOffset->fX=g_MachineOffset.fX;
	p_MachineOffset->fY=g_MachineOffset.fY;
	SmartToolMsg(STP_ALERT, OID_MACHINE_OFFSET, sizeof(td_MachineOffset),(char *)p_MachineOffset);
}


//FIXME000000000 how would I alert if I'm using this....  relates to future of HD
nodebug void SetToolOffsetWithYComp(float fToolOffset1X,float fToolOffset1Y,float fToolOffset1YExtension,
							float fToolOffset2X,float fToolOffset2Y,float fToolOffset2YExtension)
{
	if (g_ConfigData.cToolFlip==Y_POS)
	{
		g_cMachineOffsetCompensationAdjustment=1;
		g_MachineOffset1.fX = -fToolOffset1X;
		g_MachineOffset1.fY = -fToolOffset1Y;
		g_MachineOffset1.fYExtension = fToolOffset1YExtension;
		g_MachineOffset2.fX = -fToolOffset2X;
		g_MachineOffset2.fY = -fToolOffset2Y;
		g_MachineOffset2.fYExtension = fToolOffset2YExtension;
	}
	else if(g_ConfigData.cToolFlip==Y_NEG)
	{
		g_cMachineOffsetCompensationAdjustment=1;
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

nodebug void ClearToolOffset()
{
    td_MachineOffset * p_MachineOffset;
	if (g_MachineOffset.fX!=0 || g_MachineOffset.fY!=0)
	{
		g_MachineOffset.fX = 0;
		g_MachineOffset.fY = 0;
		#ifdef OUTPUT_TOOL_OFFSET
		logf("STOP Tool Offset\r\n");
		#endif
		ResetNearestPosition();
	}
	if (g_cMachineOffsetCompensationAdjustment==1)
	{
		g_cMachineOffsetCompensationAdjustment=0; //turn off special machine offset logic.
		#ifdef OUTPUT_TOOL_OFFSET
		logf("STOP Tool Offset\r\n");
		#endif
		ResetNearestPosition();
	}
	#ifdef OUTPUT_TOOL_OFFSET
	logf("STOP Tool Offset\r\n");
	#endif
    p_MachineOffset=(td_MachineOffset *)g_STPtxMsg.p_cObjectValue;
	p_MachineOffset->fX=g_MachineOffset.fX;
	p_MachineOffset->fY=g_MachineOffset.fY;
	SmartToolMsg(STP_ALERT, OID_MACHINE_OFFSET, sizeof(td_MachineOffset),(char *)p_MachineOffset);
}

nodebug void SetMoveSpeeds(char bMakeLinear, char bFinalMove, float fdx, float fdy)
{
    float fsx,fsy,facx,facy,fdcx,fdcy;
    float fr,fc;

	if (bFinalMove==0)
    {
    	if (fdx < g_ConfigData.LongDistance.fX &&
    		fdy < g_ConfigData.LongDistance.fY)
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

    if (bMakeLinear!=0)
    {
		//One change is so small, don't bother doing extra calculation... it can move as fast as it wants in that line.
        //Avoid 0 ratio, and 0 speeds which could cause issues.
		//due to math problems, we had this set to 0.0002, but due to a different problem,
		//it's now set to 0.1
		fr=0.1;
		if (fdx < fdy)
        {
			if (fdx<fr) { goto skip_make_linear; }

        	//x is shorter and will need to be slower to arrive at the same time.
	    	fr=fdx/fdy;

            fc=fr*fsy; //set speed to this proportion based on y
            if (fsx >= fc)
            {
                fsx=fc;
            }
            else
            {
            	//x is set to move even slower than this... base y speed off x at desired ratio
                fsy=fsx/fr;
            }
			//only scale ac dc if moving fast enough in for it to matter
            if (fr>=0.01)
			{
	            fc=fr*facy; //set ac. to this proportion based on y
    	        if (facx >= fc)
        	    {
            		facx=fc;
	            }
    	        else
        	    {
            	  	//x is set to ac. even slower than this... base y ac. off x at desired ratio
                	facy=facx/fr;
	            }
	            fc=fr*fdcy; //set dc. to this proportion based on y
    	        if (fdcx >= fc)
        	    {
            	    fdcx=fc;
	            }
    	        else
        	    {
            		//x is set to dc. even slower than this... base y dc. off x at desired ratio
                	fdcy=fdcx/fr;
	            }
			}
        }
        else
        {
			if (fdy<fr) { goto skip_make_linear; }

        	//y is shorter or the same and will need to be slower or the same to arrive at the same time.
	        fr=fdy/fdx;

            fc=fr*fsx; //set speed to this proportion based on x
            if (fsy >= fc)
            {
                fsy=fc;
            }
            else
            {
            	//y is set to move even slower than this... base x speed off y at desired ratio
                fsx=fsy/fr;
            }
			//only scale ac dc if moving fast enough in for it to matter
            if (fr>=0.01)
			{
				fc=fr*facx; //set ac. to this proportion based on x
    	        if (facy >= fc)
        	    {
            	    facy=fc;
	            }
    	        else
        	    {
					//y is set to ac. even slower than this... base x ac. off y at desired ratio
                	facx=facy/fr;
	            }
				fc=fr*fdcx; //set dc. to this proportion based on x
    	        if (fdcy >= fc)
        	    {
            	    fdcy=fc;
	            }
    	        else
        	    {
            		//y is set to dc. even slower than this... base x dc. off y at desired ratio
                	fdcx=fdcy/fr;
	            }
			}
        }
skip_make_linear:
    }

	SetMoveSpeedParams(fsx, fsy, facx, facy, fdcx, fdcy);
	return;
}

nodebug void DoFloat(char cAction)
{
#ifdef OUTPUT_FLOAT_STATES
	logf("DoFloat %d\r\n",(int)cAction);
#endif
	if (g_cGravCompStatus != GRAVCOMP_PASS && cAction != FLOAT_UNFLOAT && cAction != FLOAT_UNFLOAT_STOP)
    {
        SmartToolMsgMiniFtMessageCode(0, MINIFTMC_FLOAT_NEEDS_GRAVCOMP);
        return;
    }
    if (g_cMoveDone != MOVEDONE_TRUE)
    {   // disallow float unless carriage is done with previous move
        SmartToolMsgMiniFtMessageCode(0, MINIFTMC_WAIT_FOR_CARRIAGE_STOP);
        return;
    }
    if (g_cModeState == MODE_ESTOP)
    {
        // cannot float in estop
        SmartToolMsgMiniFtMessageCode(0, MINIFTMC_FLOAT_PREVENTED_BY_ESTOP);
        return;
    }
    if (cAction != FLOAT_UNFLOAT && cAction != FLOAT_UNFLOAT_STOP)
    {
		if (g_cObstructionCode!=0)
		{
    	    // can't float : too close to pos or neg limit, or obstruction
	        AlertObstructionCode();
	        return;
	    }
	}
	g_PosnMode.cOnCurPosn=0;	//floating must clear the on position flag
	if (g_cModeState == MODE_POSN)
	{
		LEDOff();
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

	g_cFloatExitModePosnState=0; //just clear the flag... only time it gets set is when unfloating normal way
	if (cAction==FLOAT_UNFLOAT)
	{
		g_cFloatExitModePosnState=POSNMODE_MOVE_NEAREST; //set the flag
	}

	if (cAction == FLOAT_FLOAT)
	{
		//unclamp now, but don't need to wait for it.
		#ifdef CLAMP_SYSTEM
       	g_cClampGoal=CLAMP_UNCLAMP;
        #endif
       	RunFloat();
#ifdef OUTPUT_FLOAT_STATES
	logf("RunFloat()\r\n");
#endif
       	g_cFloatGoal = FLOATSTAT_FLOAT;
        BeepFloat();
    }
    else if (cAction == FLOAT_UNFLOAT || cAction == FLOAT_UNFLOAT_STOP)
    {
       	RunUnfloat();
#ifdef OUTPUT_FLOAT_STATES
	logf("RunUnfloat()\r\n");
#endif
		g_cFloatGoal = FLOATSTAT_NOFLOAT;
        #ifdef BEEPSYSTEM
        if (g_cBeepMode==BEEPFLOAT) { BeepOff(); }
	    #endif
    }
}

////////////////////////////////////////////////////////////////////////
// STP UTIL SECTION
////////////////////////////////////////////////////////////////////////

//Supporting STP Function

//non-reenterant
#define MAX_KNOWN_OID_NAME_LENGTH 29
nodebug char * DisplayOIDName(unsigned int ui)
{
	unsigned long xp_xstringaddress;
	unsigned long xp_xstring;
	static char szTempOIDName[MAX_KNOWN_OID_NAME_LENGTH+1];

	if( g_pSTPSession != 0)
	{
		if (g_pSTPSession->iSessionNum<0)
		{
			//in the current context, we are sending as a client and the message mapping may not apply
			return "na";
		}
	}

	if (ui < OID_MAX_NUMBER_Common)
	{
		xp_xstringaddress=xsOIDNAMESCommon + (ui * sizeof(long));
		goto found_string;
	}
	if (ui >= 100 && ui < OID_MAX_NUMBER_MiniFT)
	{
		ui = ui - 100; //Because 1st entery is for 100
		xp_xstringaddress=xsOIDNAMESMiniFT + (ui * sizeof(long));
		goto found_string;
	}
	if (ui >= 400 && ui < OID_MAX_NUMBER_ToolManagement)
	{
		ui = ui - 400;
		xp_xstringaddress=xsOIDNAMESToolManagement + (ui * sizeof(long));
		goto found_string;
	}
	return "Unknown OID";
found_string:
	// get address of desired string
 	xmem2root((void *)&xp_xstring, xp_xstringaddress, sizeof(long));

	// load string into data
    //since we don't know the size, copy the max size we know of.
    //There is no real harm as the worst case is that it would display only partial.
	xmem2root((void *)szTempOIDName, xp_xstring, MAX_KNOWN_OID_NAME_LENGTH);

	szTempOIDName[MAX_KNOWN_OID_NAME_LENGTH]=0; //ensure it has a null.

    return szTempOIDName;
}


//////////////////////////////////////////////////////////////////////////////////
///  Profiling Tool - A performance profiling tool
//////////////////////////////////////////////////////////////////////////////////

#ifdef PROFILEPOINTS
unsigned long g_ulXmemProfileBuffer;
unsigned int g_uiXmemProfileBufferIndex;
unsigned long g_ulLastProfileTime;

nodebug void InitProfiling()
{
#if DEVMODE == 1
	g_ulXmemProfileBuffer=xallocsafe(256);
	ClearProfiling();
#endif
}

nodebug void ClearProfiling()
{
#if DEVMODE == 1
	g_uiXmemProfileBufferIndex=0;
	g_ulLastProfileTime=MS_TIMER;
#endif
}

nodebug void ProfilePoint(char * p_cLocation)
{
#if DEVMODE == 1
	unsigned long ulNow;
	unsigned long ulDiff;
	unsigned int uiDiff;
	unsigned long xp;
	ulNow=MS_TIMER;
    ulDiff=ulNow - g_ulLastProfileTime;
	if (ulDiff>0xFFFF)
	{
		uiDiff=0xFFFF;
	}
	else
	{
		uiDiff=(unsigned int)ulDiff;
	}
	g_ulLastProfileTime=ulNow;
	xp=g_ulXmemProfileBuffer+g_uiXmemProfileBufferIndex;
	xsetint(xp,(int)uiDiff);
	xsetint(xp+2,(int)p_cLocation);
	g_uiXmemProfileBufferIndex+=4;
	if (g_uiXmemProfileBufferIndex>=256)
	{
		ClearProfiling(); //erase buffer.. start again
	}
#endif
}

nodebug void PrintProfilePoints()
{
#if DEVMODE == 1
	unsigned long xp;
	unsigned long xp_end;
	char * p_cLocation;
	unsigned int uiDiff;
	xp=g_ulXmemProfileBuffer;
	xp_end=xp+g_uiXmemProfileBufferIndex;
	while(xp<xp_end)
	{
		uiDiff=xgetint(xp);
		xp+=2;
		p_cLocation=(char *)xgetint(xp);
		xp+=2;
		logf("  %u\r\n%s\r\n",uiDiff,p_cLocation);
	}
#endif
}
#endif

//g_DateTime Maintenance
nodebug void ServiceDateTime()
{
	unsigned long ulNewDateTimeMS;
	unsigned long ul;
	unsigned long ulM;
	unsigned long ulL;
	unsigned long ulMSW;
	unsigned long ulLSW;
	unsigned long ulSec;
	unsigned int ui;
	unsigned long t1;
	unsigned long t2;
	char mdays;

	ulNewDateTimeMS=MS_TIMER;
	ul=ulNewDateTimeMS - g_ulDateTimeMS;
	g_ulDateTimeMS = ulNewDateTimeMS;

	//g_DateTime.ulticksMSW = 0;
	//g_DateTime.ulticksLSW = 0;

//logf("ul=%lu\r\n",ul);
//logf("dt %lu %lu\r\n", g_DateTime.ulticksMSW, g_DateTime.ulticksLSW);
	if (ul<=429496)
	{
		//ul is small enough that this won't carry
		ulLSW = ul * 10000;
	}
	else
	{
		//the multiplication is going to be LARGER than the 32 bit number available.
		//Do it in parts
		t1 = (ul & 0xffff) * 10000;
		t2 = ((ul >> 16) * 10000) + (t1 >> 16);
		ulLSW = ((t2 & 0xffff) << 16) | (t1 & 0xffff);
		ulMSW = (t2 >> 16);
//logf("add %lu %lu\r\n",ulMSW, ulLSW);
		g_DateTime.ulticksMSW += ulMSW;
	}

	//Now Add LSW to g_DateTime, and Increment MSW if needed
	ulL=g_DateTime.ulticksLSW; //save original LSW
	ulLSW+=ulL;				//find new LSW
	g_DateTime.ulticksLSW=ulLSW;
	if (ulLSW<ulL)
	{
		//it wrapped
		g_DateTime.ulticksMSW++;
	}
//logf("dt %lu %lu\r\n", g_DateTime.ulticksMSW, g_DateTime.ulticksLSW);

	if (g_DateTime.uiyear>0)
	{
		ul+=g_DateTime.uimillisecond;
		if (ul<1000)
		{
			g_DateTime.uimillisecond = (unsigned int)ul;
			goto done_roll;
		}
		g_DateTime.uimillisecond = (unsigned int)(ul % 1000);
		ul=ul/1000;
		ul+=g_DateTime.csecond;
		if(ul<60)
		{
			g_DateTime.csecond = (char)ul;
			goto done_roll;
		}
		g_DateTime.csecond = (char)(ul % 60);
		ul=ul/60;
		ul+=g_DateTime.cminute;
		if(ul<60)
		{
			g_DateTime.cminute = (char)ul;
			goto done_roll;
		}
		g_DateTime.cminute = (char)(ul % 60);
		ul=ul/60;
		ul+=g_DateTime.chour;
		if(ul<24)
		{
			g_DateTime.chour = (char)ul;
			goto done_roll;
		}
		g_DateTime.chour = (char)(ul % 24);
		ul=ul/24;
		ul+=g_DateTime.cdayOfMonth;
		//	unsigned int uiyear;
		//	char cmonth;

		while(1)
		{
			//If days is less than 28, then no point in finding true max days of this month
			if(ul<=28)
			{
				g_DateTime.cdayOfMonth = (char)ul;
				goto done_roll;
			}
			//Find max days of this month
			mdays = 31;
			if (g_DateTime.cmonth==2)
			{
				mdays = 28;
				ui = g_DateTime.uiyear;
				if (ui % 400 == 0)
				{
					//leap
					mdays = 29;
				}
				else if (ui % 100 == 0)
				{
					//not leap
				}
				else if (ui % 4 == 0)
				{
					//leap
					mdays = 29;
				}
				else
				{
					//not leap
				}
			}
			else if (g_DateTime.cmonth==4 || g_DateTime.cmonth==6 || g_DateTime.cmonth==9 || g_DateTime.cmonth==11)
			{
				mdays = 30;
			}
			//If less than or equal to max days, then this is the date
			if(ul<=mdays)
			{
				g_DateTime.cdayOfMonth = (char)ul;
				goto done_roll;
			}
			//add this month, and take away the number of days
			ul-=mdays;
			g_DateTime.cmonth++;
			if (g_DateTime.cmonth==13)
			{
				//advance the year and reset the month
				g_DateTime.uiyear++;
				g_DateTime.cmonth=1;
			}
		}
done_roll:
	}

}

//////////////////////////////////////////////////////////////////////////////////
///  Utillity Section
//////////////////////////////////////////////////////////////////////////////////

//Math
//Returns 0 on success
//returns another code on failure
nodebug char FindIntersection(float ax, float ay, float bx, float by,
							float cx, float cy, float dx, float dy,
							float *fx, float *fy)
{
	float v1x;
	float v1y;
	float v2x;
	float v2y;
	float distAB, rCos, rSin, newX, ABpos;

	v1x=bx-ax;
	v1y=by-ay;
	v2x=dx-cx;
	v2y=dy-cy;

	//  Fail if either line is undefined.
	if (ax==bx && ay==by || cx==dx && cy==dy)
	{
		return 1;
	}

	//Due to the system this is on, also fail if the distances of the segments seem too short.
	//This will lead to errors.   this could actually be a 0 distances with a rounding issue.
	if (fabs(v1x)<0.00001)
	{
		if (fabs(v1y)<0.00001)
		{
			//both are too small: segment too short for accuracy in this float library
			return 2;
		}
	}
	if (fabs(v2x)<0.00001)
	{
		if (fabs(v2y)<0.00001)
		{
			//both are too small: segment too short
			return 3;
		}
	}

	//Now continue the standard algorithm
	//  (1) Translate the system so that point A is on the origin.
	bx-=ax; by-=ay;
	cx-=ax; cy-=ay;
	dx-=ax; dy-=ay;
	//Now ax,ay point is at 0,0 but preserve those numbers because we will need to translate back

	//  Discover the length of segment A-B.
	distAB=sqrt(bx*bx+by*by);

	//  (2) Rotate the system so that point B is on the positive X axis.
	rCos=bx/distAB;
	rSin=by/distAB;
	newX=cx*rCos+cy*rSin;
	cy=cy*rCos-cx*rSin;		cx=newX;
	newX=dx*rCos+dy*rSin;
	dy=dy*rCos-dx*rSin;		dx=newX;

	//  Fail if the lines are parallel.
	if (cy==dy)
	{
		return 4;
	}

	//THIS ONLY APPLIES IF YOU WANT THE SEGMENTS TO CROSS, NOT JUST THE LINES
	////  Fail if segment C-D doesn't cross line A-B.
	//if (cy<0 && dy<0 || Cy>=0. && Dy>=0.) return 11;

	//  (3) Discover the position of the intersection point along line A-B.
	ABpos=dx+(cx-dx)*dy/(dy-cy);

	//ABPos is also a distance, so we can use that as follows in step 4

	//THIS ONLY APPLIES IF YOU WANT THE SEGMENTS TO CROSS, NOT JUST THE LINES
	////  Fail if segment C-D crosses line A-B outside of segment A-B.
	//if (ABpos<0. || ABpos>distAB) return 12;

	//  (4) Apply the discovered position to line A-B in the original coordinate system.
	*fx=ax+ABpos*rCos;
	*fy=ay+ABpos*rSin;

	return 0;
}

//System Function

nodebug void RebootRabbit()
{
    #ifdef USE_OUTPUT
    logf("Calling Library function forceSoftReset()... \n\n\n");
    #endif
    //This jumps to the start of memory according to documentation.
    forceSoftReset(); //Goodbye
}

//Simplifies xalloc
//Should not throw a runtime error, but returns 0 on fail
nodebug long xallocsafe(long lAllocBytes)
{
	long lBytes;
    long xp;

    lBytes=_xavail((long *)0,4,XALLOC_MAYBBB);
    if (lBytes>=lAllocBytes)
    {
	    lBytes=lAllocBytes;
	    xp=_xalloc(&lBytes,4,XALLOC_MAYBBB);
        if (lBytes>lAllocBytes)
        {
			logf(" Alloc %ld\r\n",lBytes);
        }
    }
    if (lBytes<lAllocBytes)
    {
    	logf("FATAL: Alloc Fail b=%ld\r\n",lBytes);
       	g_xallocerrors++;
    }

    return xp;
}

//Simplifies xalloc
//Should not throw a runtime error, but returns 0 on fail
nodebug long xallocsafedetail(char * name,long lAllocBytes)
{
	return xallocsaferamtypedetail(name,lAllocBytes,XALLOC_MAYBBB);
}

nodebug long xallocsaferamtypedetail(char * name,long lAllocBytes,word typeram)
{
	long lBytes;
    long xp;

    logf(name);
    logf(" %ld ",lAllocBytes);

    lBytes=_xavail((long *)0,4,typeram);
    if (lBytes>=lAllocBytes)
    {
	    lBytes=lAllocBytes;
	    xp=_xalloc(&lBytes,4,typeram);
        logf("location %ld next %ld ",xp,xp+lBytes);
        if (lBytes>lAllocBytes)
        {
			logf(" Alloc %ld\r\n",lBytes);
        }
        logf("\r\n");
    }
    if (lBytes<lAllocBytes)
    {
    	logf("FATAL: Alloc Fail b=%ld\r\n",lBytes);
       	g_xallocerrors++;
    }

    return xp;
}

//Data, string, and debug functions

//FIXME: Found out that error checking only detects if \0 \r \n is behind the number....
//Not good enough, and too strict as well.  May need a better check.
nodebug int SignedCharValue(char c)
{
    //FIXME0 test if used
    int i;
    i=c;
    if (i>=128)
    {
        i=i-256;
    }
    return i;
}

//string util
nodebug speed char * scanToNull(char * s)
{
    while (*s!=0)
    {
        s++;
    }
    return s;
}

nodebug speed char * scanToChar(char * s,char c)
{
    while (*s!=c && *s!=0)
    {
        s++;
    }
    return s;
}

nodebug speed char * scanPastWhiteSpace(char * s)
{
	while (*s==' ' || *s=='\t')
    {
    	s++;
    }
    return s;
}

nodebug speed char * scanToWhiteSpace(char * s)
{
	char c;
	while (1)
    {
    	c=*s;
        if (c==' ' || c=='\t' || c==0)
        {
        	break;
        }
    	s++;
    }
    return s;
}

nodebug speed char * scanToWhiteSpaceOrChar(char * s,char cDelimiter)
{
	char c;
	while (1)
    {
    	c=*s;
        if (c==' ' || c=='\t' || c==cDelimiter || c==0)
        {
        	break;
        }
    	s++;
    }
    return s;
}

//returns true if prefix starts str

nodebug useix int prefix(char *prefix, char *str)
{
#asm xmemok
	push	hl			;preserve prefix already in hl
c	str;				// load str to hl
	pop	de			;get back prefix
.loop:
	ld		a,(de)
	or		a			;see if at end of prefix
	jr		z,.donetrue	;jump if at end of prefix
	cp		(hl)		;*prefix - *str
	jr		c,.donefalse	;jump if prefix < str
	jr		nz,.donefalse	;jump if str > prefix
	inc	de
	inc	hl
	jr		.loop
.donefalse:
	ld		hl,0
    jr		.done
.donetrue:
	ld		hl,1
.done:
#endasm
}


//reutrns 1 if equal, 0 if not... like strcmp, but without the awkward reverse bool return value.

nodebug useix int equal(char *str1, char *str2)
{
#asm xmemok
	push	hl			;preserve str1 already in hl
c	str2;				// load str2 to hl
	pop	de			;get back str1
.loop:
	ld		a,(de)
	cp		(hl)		;*str1 - *str2
	jr		c,.donefalse	;jump if str1 < str2
	jr		nz,.donefalse	;jump if str2 > str1
	or		a			;see if at end of str1
	jr		z,.donetrue	;jump if at end of str1
	inc	de
	inc	hl
	jr		.loop
.donefalse:
	ld		hl,0
    jr		.done
.donetrue:
	ld		hl,1
.done:
#endasm
}


//works like string copy, but returns the pointer past the null.
//This is 100% perfect for a very fast version of our string storage copy.
// (see how simple it makes that function.)

nodebug useix char *strcpy_returnpost(char *dst, char *src)
{
#asm
	ex de,hl	//move dst from hl to de
c	src;				// load src to hl
	xor	a			;clear a
.copy:			;a = 0, dst = de, src = hl
	cp		(hl)		;test for null terminator
	ldi				;*dst++ = *src++
	jr		nz,.copy	;resume loop if not null terminator
    ex de,hl     //move dst back to hl for return
#endasm
}




nodebug char split(char cDelimiter,char * s,char ** p_szFields,char cMaxFields)
{
	char cFieldNo;

    cFieldNo=0;
	while(cFieldNo<cMaxFields)
    {
		s=scanPastWhiteSpace(s);
        if (*s=='"')
        {
	       	s++;
            //field starts here
            p_szFields[cFieldNo++]=s;
   	        s=scanToChar(s,'"');
            if (*s!='"')
            {
	          	//quote mismatch
            	return SPLIT_PARSE_ERROR_QUOTE_MISMATCH;
       	    }
            //terminate this value at the quote and move forward
	        *s++=0;
	        if (*s==cDelimiter)
    	    {
            	s++;
            	//next field
    	        continue;
	        }
	        if (*s==0)
	        {
	           	//nothing more to parse
    	        break;
	        }
        }
   	    else
       	{
        	//field starts here
            p_szFields[cFieldNo++]=s;
           	//go past non quoted value
            s=scanToWhiteSpaceOrChar(s,cDelimiter);
	        if (*s==cDelimiter)
	        {
	            *s++=0; //terminate this value, and move forward
    	        continue;
	        }
	        if (*s==0)
	        {
	           	//nothing more to parse
    	        break;
	        }
            //must be white space
            *s++=0; //terminate this value, and move forward
        }
      	//only white space allowed here
        //scan past any white space
        s=scanPastWhiteSpace(s);
        if (*s==0)
        {
           	//nothing more to parse
            break;
        }
	    if (*s==cDelimiter)
	    {
	        s++;
            continue;
	    }
       	//not null, not delimeter, this is an error
        return SPLIT_PARSE_ERROR_CHARS_AFTER_VALUE;
    }
    return cFieldNo;
}

nodebug void rchr(char *s,char c,char n)
{
	while(*s!=0)
	{
		if (*s==c) { *s=n; }
		s++;
	}
	return;
}

//hex conversion
char g_hexmap[256];

nodebug void initCopyHexToByte()
{
	char c;
    memset(g_hexmap,255,256);

    //NOTE: the reason to do this expanded is that the compiler may be able to condense this to 26 mem sets.
    // rather than what is actually more instructions to do it with loops, since constant addresses apply.
   	g_hexmap['0']=0;
   	g_hexmap['1']=1;
   	g_hexmap['2']=2;
   	g_hexmap['3']=3;
   	g_hexmap['4']=4;
   	g_hexmap['5']=5;
   	g_hexmap['6']=6;
   	g_hexmap['7']=7;
   	g_hexmap['8']=8;
   	g_hexmap['9']=9;

    c=10;	g_hexmap['a']=c;	g_hexmap['A']=c;
    c=11;	g_hexmap['b']=c;	g_hexmap['B']=c;
    c=12;	g_hexmap['c']=c;	g_hexmap['C']=c;
    c=13;	g_hexmap['d']=c;	g_hexmap['D']=c;
    c=14;	g_hexmap['e']=c;	g_hexmap['E']=c;
    c=15;	g_hexmap['f']=c;	g_hexmap['F']=c;
}

nodebug char copyHexToByte(char * bytes,char *szhex,char cCount,char cRequireEnd)
{
	char c;
	char b;
	char b2;
	char cInd;

    cInd=0;
    while (cInd<cCount)
    {
    	c=*szhex++;
        if (c==0) { break; }
        if (c==' ') { continue; }
        if (c=='\t') { continue; }
        if (c=='-') { continue; }
        if (c=='"') { continue; }
        //found different character
        b=g_hexmap[c];
        if (b==255) { break; } //not a valid hex char
    	c=*szhex++;
        b2=g_hexmap[c];
        if (b2==255) { break; } //not a valid hex char
        b=b << 4; // left shift
        b=b | b2;
        bytes[cInd++]=b;
    }
    if (cInd<cCount)
    {
    	//broke out and did not find all the needed bytes
        return 0;
    }
    if (cRequireEnd)
    {
    	c=*szhex;
    	if (c!=0)
        {
        	//null did not follow the hex buffer.
            return 0;
        }
    }
    return 1;
}


//generic and specific display functions

nodebug void memchardump(char * label,char * data,int iLen)
{
#ifdef USE_OUTPUT
    int i;
    char c;
    char nl;
    logf("%s[] = ",label);
    nl=0;
    logf("  [0]:");
    for (i=0;i<iLen;i++)
    {
        if (nl>=20)
        {
            logf("\r\n   ");
		    logf("[%d]:",i);
            nl=0;
        }
        c=data[i];
        if (c>20 && c<=127)
        {
            logf("%c",c);
        }
        else
        {
            logf(" %d ",c);
            if (c==10)
            {
                nl=40;
            }
        }
        nl++;
    }
    logf("\r\n");
#endif
}

nodebug void memdump(char * label,unsigned char * data,int iLen)
{
#ifdef USE_OUTPUT
    int i;
    logf("%s[] = ",label);

    for (i=0;i<iLen;i++)
    {
        if (i>0 &&  (i % 20) == 0)
        {
            logf("\r\n   ");
        }
        logf("%x ",(int)data[i]);
    }
    logf("\r\n");
#endif
}


//	The following are Timer B support functions for a high-resolution elapsed timer.
//	On the 20MHz Rabbit BL2100, each tick of Timer B is approx. 67 usec.
//	Rollover occurs at approx. 2.1 sec if we limit the counter to an integer value.
#ifdef USE_TIMERB
nodebug void TimerBInit(void)
{
	g_iTBcount = 0;
    g_iTBelapsedcounts = 0;
	g_lTBelapsed_usec = 0;
    #if __SEPARATE_INST_DATA__ && (_RK_FIXED_VECTORS)
	interrupt_vector timerb_intvec timerb_isr;
#else
	SetVectIntern(0x0B, timerb_isr);	   		// set up ISR
	SetVectIntern(0x0B, GetVectIntern(0xB));	// re-setup ISR to show example of retrieving ISR address
#endif

	WrPortI(TBCR, &TBCRShadow, 0x01);	// clock timer B with (perclk/2) and
													//     set interrupt level to 1

	WrPortI(TBM1R, NULL, 0x00);			// set initial match!
	WrPortI(TBL1R, NULL, 0x00);

	WrPortI(TBCSR, &TBCSRShadow, 0x03);	// enable timer B and B1 match interrupts
}

nodebug void TimerBStart(void)
{
	g_iTBcount = 0;
}

//	Read elapsed time, but do not interrupt the timer.
nodebug int TimerBRead(void)
{
    g_iTBelapsedcounts = g_iTBcount;
    return( g_iTBelapsedcounts );
}

//	Read timer and reset counter
nodebug int TimerBReset(void)
{
    g_iTBelapsedcounts = g_iTBcount;
	g_iTBcount = 0;
    g_lTBelapsed_usec = (long) g_iTBelapsedcounts * (long) TIMERB_TICK_USEC;
    return( g_iTBelapsedcounts );
}


////////////////////////////////////////////////////////////////////////////////
// interrupt routine for timer B
//
//  This is called whenever _either_ B1 or B2 matches the timer.  If you are
//  using both of them, you need to check the TBCSR register to see which one
//  triggered the interrupt (you need to read that register anyway to clear
//  the flag).
//
////////////////////////////////////////////////////////////////////////////////

#asm
timerb_isr::
	push	af							; save registers
	push	hl

	ioi	ld a, (TBCSR)			; load B1, B2 interrupt flags (clears flag); this
										; should be done as soon as possible in the ISR

	ld		hl, (g_iTBcount)
	inc	hl							; increment counter
	ld		(g_iTBcount), hl

	ld		hl, PADR
	ioi	ld (hl), 0xFF			; send a pulse to port A
	ioi	ld (hl), 0x00

	ld		a, 00h
	ioi	ld (TBM1R), a			; set up next B1 match (at timer=0000h)
	ioi	ld (TBL1R), a			; NOTE:  you _need_ to reload the match
										;	register after every interrupt!

done:
	pop	hl							; restore registers
	pop	af

	ipres								; restore interrupts
	ret								; return
#endasm
#endif	// USE_TIMERB

//void TestFloatResP10(float f)
//{
//	float delta;
//	float deltaseen;
//    float f2;
//
//    delta = 1.0;
//    deltaseen = delta;
//	f2=f;
//    while(1)
//    {
//    	f=f+delta;
//        if (f==f2)
//        {
//     		//wow.. delta is too small to see.
//            break;
//        }
//        f=f2; //restore f
//        deltaseen = delta;
//        delta = delta / 10;
//    }
//
//    logf ("f=%f can see delta=%f failed data=%f\r\n",f,deltaseen,delta);
//
//}

#ifdef SHOW_MEM
/Memory Display Library

#if __RABBITSYS
#error "MEMORY_USAGE.C is not currently supported under RabbitSys."
#endif
int table_entry( unsigned logaddr, long physaddr, long Size, char * use);
char readStackSeg();
void separator();

nodebug void show_memory()
{
	unsigned long mb,mt,ms,sz;
	static char datasegval;
   int i;
	const _sys_mem_origin_t * org_entry;
   long next;
   _rk_xbreak_t xsbreak;

	//********** Show Flash Present ***************
	logf(
		"     Dynamic C detects a flash of %08lx bytes.\n",
			(long)(_FLASH_SIZE_)<<12ul);
	if(_FLASH_SIZE_ != FLASH_SIZE)
		logf(
			"       However, the BIOS set FLASH_SIZE to %08lx bytes\n",
				(long)(FLASH_SIZE)<<12ul);
#if RAM_COMPILE
		logf(
			"     Flash is not being used. This program is compiled to RAM \n");
#elif FLASH_COMPILE
	logf(
		"     Flash starts at physical address 00000\n");
#else
		logf(
			"     This program is compiled to flash and copied to run in RAM \n");
		logf(
			"     The top of the flash is mapped to 0xC0000 \n");
#endif
	logf("\n");


#if __SEPARATE_INST_DATA__
		logf(
			"     Separate instruction and data space is enabled \n");
	logf("\n");
#endif

	//********** Show RAM Present ***************
	logf(
	   "     Dynamic C detects a RAM of %08lx bytes\n",(long)(_RAM_SIZE_)<<12ul);
	logf("     Ram starts at physical address %08lx\n",(long)(RAM_START)<<12ul);


	//********** Display MMU/MIU Registers ***************
	logf("\nMMU Registers:\n");
	logf("     SEGSIZE=%02x\n",MEMBREAK);
	#asm
		ioi ld a, (DATASEG)
		ld	(datasegval), a
	#endasm
	logf("     DATASEGVAL=%02x\n",datasegval);
	logf("     STACKSEGVAL=%02x\n",readStackSeg());
	logf("     XPC=%02x\n",XMEMSEGVAL);
	logf("\nMIU Bank Registers:\n");
	logf("     MB0CR=%02x\n",MB0CRShadow);
	logf("     MB1CR=%02x\n",MB1CRShadow);
	logf("     MB2CR=%02x\n",MB2CRShadow);
	logf("     MB3CR=%02x\n",MB3CRShadow);
	logf("\n");


	//********** Begin Usage Table ***************
	logf(
	 " \xda-----------------------------------------------------------------------------\xbf"
	 );
	logf(
	 " \xB3 PHYSADR LOGADR    SIZE  SIZE(Dec)  USAGE                                    \xB3"
	 );

	separator();
	logf(
#ifdef _FLASH_
	 " \xB3 FLASH                                                                       \xB3"
#else
	 " \xB3 RAM                                                                         \xB3"
#endif
	 );
	separator();

	//********** Root Code  ***************
	table_entry(0,(long)0,(long)ROOTCODESIZE,"Reserved by Compiler for Root Code");
	table_entry(0,(long)0,(long)StartUserCode-1,"used by BIOS");
	table_entry((unsigned)StartUserCode,
	  (long)StartUserCode,
	  (long)prog_param.RCE.aaa.a.addr-(long)StartUserCode,
	  "used by this program for root code");

#if __SEPARATE_INST_DATA__
	table_entry(0,(ROOTCONSTSEGVAL<<12l)+(long)ROOTCONSTORG,(long)ROOTCONSTSIZE,"Reserved for Root Constants");
#endif

	separator();

	//********** XMem Code  ***************
#if __SEPARATE_INST_DATA__
	table_entry(
		0xffffu,
	  (long)SID_XMEMORYORG,
	  (long)SID_XMEMORYSIZE,
	  "Reserved for Xmem Code");
#endif

	table_entry(
		0xffffu,
	  (long)XMEMORYORG,
	  (long)XMEMORYSIZE,
	  "Reserved for Xmem Code");

	mb = 0x0ffffful &
		((unsigned long)prog_param.XCB.aaa.a.addr +
		((unsigned long)prog_param.XCB.aaa.a.base<<12));
	mt = 0x0ffffful &
		((unsigned long)prog_param.XCE.aaa.a.addr +
		((unsigned long)prog_param.XCE.aaa.a.base<<12));
	ms = mt-mb;

	table_entry(
	   0xffffu,
	   mb,
	   ms,
	   "used by this program for xmem code");

#if (XMEM_RESERVE_SIZE>0)
	table_entry(
	   0xffffu,
	   FLASH_SIZE*4096L - DATAORG - XMEM_RESERVE_SIZE,
	   (long)XMEM_RESERVE_SIZE,
	   "used for the File System");
#endif
	separator();

	logf(
#ifdef _FLASH_
	 " \xB3 RAM                                                                         \xB3"
#else
	 " \xB3 MORE RAM (Code and BIOS in RAM option set in compiler options)              \xB3"
#endif
	 );
	separator();

	//********** Root RAM ***************
	table_entry(
	   (ROOTDATAORG) - (ROOTDATASIZE) + 1,
      paddr((void *) ((ROOTDATAORG) - (ROOTDATASIZE) + 1)),
	   (long) ROOTDATASIZE,
	   "Reserved for Root Data");

#if BBROOTDATASIZE > 0
	table_entry(
	   (BBROOTDATAORG) - (BBROOTDATASIZE) + 1,
      paddr((void *) ((BBROOTDATAORG) - (BBROOTDATASIZE) + 1)),
	   (long) BBROOTDATASIZE,
	   "Reserved for Battery Backed Root Data");
#endif

	table_entry(
	   prog_param.RDE.aaa.a.addr,
	   paddr((void *)prog_param.RDE.aaa.a.addr),
	   (long)(prog_param.RDB.aaa.a.addr - prog_param.RDE.aaa.a.addr)+1,
	   "used by this program for root data");

	table_entry(
	   (unsigned int)INTVEC_BASE,
	   paddr((void*)(INTVEC_BASE)),0x100ul,
	   "used for the internal vector table");

   table_entry(
      (unsigned int)XINTVEC_BASE,
      paddr((void*)(XINTVEC_BASE)),0x100ul,
      "used for the external vector table");

	separator();

	//********** Display xalloc used areas ***************
	// includes RAM space reserved for items already listed
   for (i = 0; i < _orgtablesize; ++i) {
   	org_entry = &_orgtable[i];
   	if (org_entry->type == RESVORG)
      	table_entry(org_entry->laddr, org_entry->paddr, org_entry->totalbytes,
                     "Reserved RAM space");
   }

	separator();

	//********** Stacks ***************
	table_entry(
	   0xffffu,
   	BaseOfStacks1,
	   SizeOfStacks1,
	   "Stack space");

	separator();

	//********** Flash Buffer ***************
	table_entry(
	   0xffffu,
	   flash_buf_phys,
	   (long)FLASH_BUF_SIZE,
	   "Used for the flash buffer");

#if ENABLE_ERROR_LOGGING
	//********** Error Log Buffer ***************
	table_entry(
	   0xffffu,
	   ERRLOG_PHYSICAL_ADDR&0xffff000ul,
	   (long)(ERRLOG_NBLOCKS*4096),
	   "Reserved for the Error log buffer");
	table_entry(
	   0xffffu,
	   ERRLOG_PHYSICAL_ADDR,
	   (long)ERRLOG_LOG_SIZE,
	   "Used for the Error log buffer");
#endif

#if (FS2_RAM_RESERVE!=0)
	table_entry(
	   0xffffu,
	   (long)FS2_RAM_PHYS,
	   FS2_RAM_RESERVE*4096L,
	   "Reserved for File System buffer");

#endif
	separator();

	//********** Display xalloc free areas ***************
   next = _rk_xubreak;
   while (next) {
   	xmem2root(&xsbreak, next, sizeof(_rk_xbreak_t));
      table_entry(0xe000u, next + sizeof(_rk_xbreak_t),
                  xsbreak.sbreak - xsbreak.limit, "Free XRAM space for xalloc");
      next = xsbreak.next;
   }

	// bottom of table
	logf(
	 " \xC0-----------------------------------------------------------------------------\xd9\n"
	 );

}


table_entry( unsigned logaddr,
             long physaddr,
             long Size,
             char * use
           )
{
	auto unsigned xAddr;
	auto unsigned long addr32;
   auto char     xBase;

	// *** base seg entry ***
	if(logaddr<ROOTDATAORG)
	{
		logf(" \xB3 %05lx   00:%04x   %05lx %06ld     %-40.38s \xB3",physaddr,logaddr,Size,Size,use);
	}
	// *** data seg entry ***
	else if(logaddr<(STACKORG))
	{
		logf(" \xB3 %05lx   %02lx:%04x   %05lx %06ld     %-40.38s \xB3",physaddr,(long)DATASEGVAL,logaddr,Size,Size,use);
	}
	// *** stack seg entry ***
	else if(logaddr<0xE000u)
	{
		logf(" \xB3 %05lx   %02x:%04x   %05lx %06ld     %-40.38s \xB3",physaddr,readStackSeg(),logaddr,Size,Size,use);
	}
	// *** xmem seg entry ***
	// if log address is in E000-FFFF range ignore logical entry parameter
	// and convert physical address to a an E000-FFFF logical address
	else
	{
		addr32 = physaddr-0xe000ul;
		xBase = (char)((addr32&0x000ff000L)>>12);
		xAddr = (unsigned)((addr32&0x00000fffL)|0xe000L);

		logf(" \xB3 %05lx   %02x:%04x   %05lx %06ld     %-40.38s \xB3",physaddr,xBase,xAddr,Size,Size,use);
	}
}


nodebug char readStackSeg(){
	static char retval;
   // **** read STACKSEG register ****
#asm
	ioi ld a,(STACKSEG)
	ld (retval),a
#endasm
	return retval;
}

nodebug void separator()
{
	logf(
	 " \xC3-----------------------------------------------------------------------------\xB4"
	 );
}

#endif

#define SHOWIO_DIN 24
#define SHOWIO_ANA 4
char g_clastdin[SHOWIO_DIN];
float g_flastanain[SHOWIO_ANA];

nodebug void ClearShowIO()
{
	int i;
	i = 0;
    while (i<SHOWIO_DIN)
    {
        g_clastdin[i++]=2;
    }
    return;
}

nodebug void ShowIO()
{
	char ci;
	char c;
	char lc;
	ci = 0;
    while (ci<SHOWIO_DIN)
    {
		c=digIn((int)ci);
        lc=g_clastdin[ci];
        g_clastdin[ci]=c;
        if (c!=lc)
        {
        	if (c>0)
            {
            	c='1';
            }
            else
            {
            	c='0';
            }
			logf("DIN%d=%c\r\n",ci,c);
        }
        ci++;
    }
    return;
}

nodebug void ClearShowAna()
{
	int i;
    i = 0;
    while (i<SHOWIO_ANA)
    {
    	g_flastanain[i++]=0;
    }
    return;
}

nodebug void ShowAna()
{
	char ci;
	char c;
	char lc;
    float f;
    float lf;
    ci = 0;
    while(ci<SHOWIO_ANA)
    {
    	f=anaInVolts((int)ci);
        lf=g_flastanain[ci];
		g_flastanain[ci]=f;

        if (fabs(f-lf) > 0.05)
        {
			logf("anaIn%d = %.4f\r\n",ci,f);
        }
        ci++;
    }
    return;
}

//EOF





#endif
