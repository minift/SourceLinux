//MiniFTSmartTool.h
//Generated by MakeOID20131018.1 
//Preserve any customized code if required.
#ifndef MiniFTSmartTool_H
#define MiniFTSmartTool_H
//Includes needed for common SmartTool Type
#include "SmartTool.h"

//MakeOID-Section:: MiniFTOID Enum
enum MiniFTOID
{
		MINIFT_OID_MINIFT_MESSAGE_CODE = 100,
		MINIFT_OID_MODE = 101,
		MINIFT_OID_ACTION = 102,
		MINIFT_OID_RESET_MC = 103,
		MINIFT_OID_ENCODER_RATIO = 104,
		MINIFT_OID_MC_CURRENT_LIMITS = 105,
		MINIFT_OID_MC_PEAK_CURRENT_LIMITS = 106,
		MINIFT_OID_BRAKE_ON_TIMEOUT = 107,
		MINIFT_OID_MCERR = 108,
		MINIFT_OID_STOREDEFAULT_CONFIG = 109,
		MINIFT_OID_RECALL_CONFIG = 110,
		MINIFT_OID_EEOPTION_DEFAULT = 111,
		MINIFT_OID_EEOPTION = 112,
		MINIFT_OID_BEEPER = 113,
		MINIFT_OID_SYSTEM_MONITOR = 114,
		MINIFT_OID_AIR_PRESSURE = 115,
		MINIFT_OID_LIMITS_AND_OBSTRUCTIONS = 116,
		MINIFT_OID_TOOL_VERIFYENABLE = 117,
		MINIFT_OID_HOLE_PARAMETERS = 118,
		MINIFT_OID_TOOL_RESERVED = 119,
		MINIFT_OID_PROCESS = 120,
		MINIFT_OID_RETURN_HEIGHT = 121,
		MINIFT_OID_GAUGE_LENGTH = 122,
		MINIFT_OID_SCALE_MODE = 123,
		MINIFT_OID_RESERVED_2 = 124,
		MINIFT_OID_PARTPGM_CLEAR = 125,
		MINIFT_OID_PARTPGM_REQUEST_FILE = 126,
		MINIFT_OID_PARTPGM_DIR = 127,
		MINIFT_OID_PARTPGM_NAME = 128,
		MINIFT_OID_PARTPGM_DATA = 129,
		MINIFT_OID_PARTPGM_CHECKSUM = 130,
		MINIFT_OID_PARTPGM_LINEARJOB = 131,
		MINIFT_OID_PARTPGM_STATUS = 132,
		MINIFT_OID_PARTPGM_LOCKED = 133,
		MINIFT_OID_STARTOVER = 134,
		MINIFT_OID_LOAD_YRAIL = 135,
		MINIFT_OID_GRAVCOMP_STATUS = 136,
		MINIFT_OID_GRAVCOMP_AXES = 137,
		MINIFT_OID_GRAVCOMP_CMD = 138,
		MINIFT_OID_GRAVCOMP_FLOAT = 139,
		MINIFT_OID_GRAVCOMP_SPEED = 140,
		MINIFT_OID_GRAVCOMP_ACCEL = 141,
		MINIFT_OID_GRAVCOMP_DECEL = 142,
		MINIFT_OID_GRAVCOMP_MOVEDIST = 143,
		MINIFT_OID_GRAVCOMP_ALGORITHM = 144,
		MINIFT_OID_GRAVCOMP_NOISE_LIMIT = 145,
		MINIFT_OID_GRAVCOMP_TRIGGERFACTOR = 146,
		MINIFT_OID_FLOAT_STATUS = 147,
		MINIFT_OID_GRAVCOMP_RESULTS = 148,
		MINIFT_OID_FLOAT_SPEEDLIMIT = 149,
		MINIFT_OID_JOG_SPEEDLIMIT = 150,
		MINIFT_OID_MAX_SPEED_X = 151,
		MINIFT_OID_MAX_SPEED_Y = 152,
		MINIFT_OID_X_RAIL_SURFACE_OFFSET = 153,
		MINIFT_OID_PROBE_METHOD = 154,
		MINIFT_OID_PROBE_METHOD_DEFAULT = 155,
		MINIFT_OID_PROBE_OFFSET = 156,
		MINIFT_OID_PROBE_DIR = 157,
		MINIFT_OID_DRILL_DIR = 158,
		MINIFT_OID_KHOLE_MAX_DISTANCE_ERROR = 159,
		MINIFT_OID_APPROX_LOCATION_ERROR = 160,
		MINIFT_OID_KHOLE_DISTANCE = 161,
		MINIFT_OID_PROBE = 162,
		MINIFT_OID_PROBE_POSITION = 163,
		MINIFT_OID_PROBE_STATUS = 164,
		MINIFT_OID_PROBE_START = 165,
		MINIFT_OID_PROBE_ACCEPT_REQUIRED = 166,
		MINIFT_OID_HOME = 167,
		MINIFT_OID_HOME_SPEED = 168,
		MINIFT_OID_HOME_ACCEL = 169,
		MINIFT_OID_HOME_DECEL = 170,
		MINIFT_OID_HOME_MOVEDIST = 171,
		MINIFT_OID_PROBE_ADJUST = 172,
		MINIFT_OID_PROBE_ADJUST_LIMIT = 173,
		MINIFT_OID_HOME_FINE_SPEED = 174,
		MINIFT_OID_HOME_REPORT = 175,
		MINIFT_OID_PATTERN = 176,
		MINIFT_OID_POSNMODE_CURPOSN = 177,
		MINIFT_OID_POSNMODE_NEARPOSN = 178,
		MINIFT_OID_POSNMODE_GOALPOSN = 179,
		MINIFT_OID_POSNMODE_CURXY = 180,
		MINIFT_OID_POSNMODE_NEARXY = 181,
		MINIFT_OID_POSNMODE_POSNSUMMARY = 182,
		MINIFT_OID_POSNMODE_MOVETONEXT = 183,
		MINIFT_OID_POSNMODE_MOVETOPREV = 184,
		MINIFT_OID_POSNMODE_MOVETOIND = 185,
		MINIFT_OID_POSNMODE_MOVEAGAIN = 186,
		MINIFT_OID_POSNMODE_MOVETYPE = 187,
		MINIFT_OID_POSNMODE_PREMOVEXY = 188,
		MINIFT_OID_POSNMODE_SPEED = 189,
		MINIFT_OID_POSNMODE_ACCEL = 190,
		MINIFT_OID_POSNMODE_DECEL = 191,
		MINIFT_OID_POSNMODE_FINALSPEED = 192,
		MINIFT_OID_ORTHO_SLOPE = 193,
		MINIFT_OID_POSNERR_LIMIT = 194,
		MINIFT_OID_POSNMODE_TOLERANCE = 195,
		MINIFT_OID_VELERR_LIMIT = 196,
		MINIFT_OID_LONG_DISTANCE = 197,
		MINIFT_OID_LONG_SPEED = 198,
		MINIFT_OID_POSNMODE_MOVEDONE = 199,
		MINIFT_OID_OP_STARTED = 200,
		MINIFT_OID_OP_HISTORY = 201,
		MINIFT_OID_DRILL_HOLE_ONE_TIME = 202,
		MINIFT_OID_AUTOMOVE = 203,
		MINIFT_OID_AUTOMOVE_DELAY = 204,
		MINIFT_OID_AUTOREPEAT = 205,
		MINIFT_OID_MACHINE_OFFSET = 206,
		MINIFT_OID_MACHINE_OFFSET_CADJ = 207,
		MINIFT_OID_MACHINE_OFFSET1 = 208,
		MINIFT_OID_MACHINE_OFFSET2 = 209,
		MINIFT_OID_STATION = 210,
		MINIFT_OID_TOOL_OFFSET = 211,
		MINIFT_OID_TOOL_FLIP = 212,
		MINIFT_OID_DRIVE_THROUGH_BACKLASH = 213,
		MINIFT_OID_DRILL_OFFSET1 = 214,
		MINIFT_OID_DRILL_OFFSET2 = 215,
		MINIFT_OID_OFFSET_SEAL = 216,
		MINIFT_OID_OFFSET_FILL = 217,
		MINIFT_OID_JOG = 218,
		MINIFT_OID_JOG_SPEED = 219,
		MINIFT_OID_JOG_ACCEL = 220,
		MINIFT_OID_JOG_DECEL = 221,
		MINIFT_OID_JOG_FACTOR = 222,
		MINIFT_OID_HOME_POSITION_Y_POS = 223,
		MINIFT_OID_POSITION_LIMIT_Y_POS = 224,
		MINIFT_OID_HOME_POSITION_Y_NEG = 225,
		MINIFT_OID_POSITION_LIMIT_Y_NEG = 226,
		MINIFT_OID_PROBE_REGISTRATION = 227,
		MINIFT_OID_OBSTRUCTION_CODE_MASK = 228,
		MINIFT_OID_MACHINE_LOCK_REQUIRED = 229,
		MINIFT_OID_MACHINE_LOCK = 230,
		MINIFT_OID_CLAMP = 231,
		MINIFT_OID_ALOCK = 232,
		MINIFT_OID_ALOCKDELAY = 233,
		MINIFT_OID_AUNLOCKDELAY = 234,
		MINIFT_OID_LEGLOCKDELAY = 235,
		MINIFT_OID_LEGUNLOCKDELAY = 236,
		MINIFT_OID_LEGSUPDELAY = 237,
		MINIFT_OID_LEGSDOWNDELAY = 238,
		MINIFT_OID_LOWPRESSUREDELAY = 239,
		MINIFT_OID_LOWPRESSUREDOWNDELAY = 240,
		MINIFT_OID_PRESSUREDELAY = 241,
		MINIFT_OID_PRESSUREDOWNDELAY = 242,
		MINIFT_OID_LOWPRESSURE = 243,
		MINIFT_OID_PRESSURE = 244,
		MINIFT_OID_AIR_CLEAR = 245,
		MINIFT_OID_LASER_SENSOR_OFFSET = 246,
		MINIFT_OID_CAM_OFFSET = 247,
		MINIFT_OID_VISION_INSPECT = 248,
		MINIFT_OID_VISION_IMAGE = 249,
		MINIFT_OID_VISION_DATA = 250,
		MINIFT_OID_VISION_INSPECT_RESULTS = 251,
		MINIFT_OID_LASER_SENSOR_ALG_PARAM = 252,
		MINIFT_OID_CAM_ALG_PARAM = 253,
		MINIFT_OID_VISION_AUTO_INSPECT = 254,
		MINIFT_OID_STOP_INTERFACE_TASK = 255,
		MINIFT_OID_PROCESS_START = 256,
		MINIFT_OID_PROCESS_STOP = 257,
		MINIFT_OID_PROCESS_CONTINUE_MODE = 258,
		MINIFT_OID_PROCESS_OPERATIONS = 259,
		MINIFT_OID_DRILL_STATE = 260,
		MINIFT_OID_DRILL_EXPLANATION = 261,
		MINIFT_OID_DRILL_EXPLANATION_DATA = 262,
		MINIFT_OID_SEAL_STATE = 263,
		MINIFT_OID_SEAL_CLAMP = 264,
		MINIFT_OID_SEAL_PRESSURE_DELAY = 265,
		MINIFT_OID_SEAL_PRESSURE_RELEASE_DELAY = 266,
		MINIFT_OID_SEAL_PRIME_DELAY = 267,
		MINIFT_OID_SEAL_GLOB_DELAY = 268,
		MINIFT_OID_SEAL_APPLY_DELAY = 269,
		MINIFT_OID_FILL_STATE = 270,
		MINIFT_OID_FILL_CLAMP = 271,
		MINIFT_OID_FILL_EXTEND_DELAY = 272,
		MINIFT_OID_FILL_RAM_DELAY = 273,
		MINIFT_OID_FASTENER_REQUEST = 274,
		MINIFT_OID_FASTENER_ARRIVED = 275,
		MINIFT_OID_FASTENER_POST_DELAY = 276,
		MINIFT_OID_ACCEL_ARM = 277,
		MINIFT_OID_ACCEL_TRIGGER = 278,
		MINIFT_OID_TOOL_Z_BASE = 279,
		MINIFT_OID_FASTENER_FAULT = 280,
		MINIFT_OID_POSNMODE_ACTIVE_PREMOVEXY = 281,
		MINIFT_OID_DRILL_FAULT = 282,
		MINIFT_OID_HOLE_RESULT_DATA = 283,
		MINIFT_OID_MOVEXY = 284,
		MINIFT_OID_POSN_DISPLAY = 285,
		MINIFT_OID_POSNMODE_XYDATA_ID = 286,
		MINIFT_OID_POSNMODE_XYDATA = 287,
		MINIFT_OID_RFID_CONFIG = 288,
		MINIFT_OID_RFID_DATA = 289,
		MINIFT_OID_HOME_RFID = 290,
		MINIFT_OID_READ_RFID = 291,
		MINIFT_OID_HOME_STOP = 292,
		MINIFT_OID_ESTOP_CLEAR_DELAY = 293,
		MINIFT_OID_DRILL_BUTTON_DELAY = 294,
		MINIFT_OID_USE_CUTTER_DETECT = 295,
		MINIFT_OID_JOG_ENABLE_TIMEOUT = 296,
		MINIFT_OID_DRILL_CYCLE_DELAY = 297,
		MINIFT_OID_INSPECT_METHOD = 298,
		MINIFT_OID_COMMAND_INSPECT_METHOD = 299,
		MINIFT_OID_FORCE_SENSOR_CALIBRATION = 300,
		MINIFT_OID_FORCE_SENSOR = 301,
		MINIFT_OID_FORCE_LIMITS = 302,
		MINIFT_OID_PROBE_FLAGS = 303,
		MINIFT_OID_MO_CAL = 304,
		MINIFT_OID_VISION_EXTERNAL_ANALYSIS = 305,
		MINIFT_OID_LIMITS_AND_OBSTRUCTION_WARNINGS = 306,
		MINIFT_OID_RFID_TAG_SET = 307,
		MINIFT_OID_PROBE_UPDATE_NOW = 308,
		MINIFT_OID_KHOLE_MAX_DISTANCE_CHECK = 309,
		MINIFT_OID_MAX_EDGE_SHIFT_PROBE_ACCEPT = 310,
		MINIFT_OID_ALLOW_DRILL_BEYOND_SHIFT_LIMITS = 311,
		MINIFT_OID_NAC_SERIAL_NUMBER = 312,
		MINIFT_OID_Y_RETRACT = 313,
		MINIFT_OID_SYSTEM_COMPONENTS = 314,
		MINIFT_OID_RFID_TAG_SET2 = 315,
		MINIFT_OID_SPRITZ_DURATION_MS = 316,
		MINIFT_OID_BURST_ON_DURATION_MS = 317,
		MINIFT_OID_BURST_INTERVAL_MS = 318,
		MINIFT_OID_ANGLE_SENSOR_A_NEG_CALIBRATION = 319,
		MINIFT_OID_ANGLE_SENSOR_A_POS_CALIBRATION = 320,
		MINIFT_OID_ANGLE_SENSOR_LEVEL_NOW = 321,
		MINIFT_OID_ANGLE_SENSOR_A_BASE_WIDTH = 322,
		MINIFT_OID_ANGLE_SENSOR_MC_CALIBRATION = 323,
		MINIFT_OID_AAXIS_MOVE_SPEED = 324,
		MINIFT_OID_CLAMP_STRAIN = 325,

		MINIFT_OID_MAX_NUMBER = 326

};
//MakeOID-Section::END

//MakeOID-Section:: MiniFT Value Defines
// MiniFTMode value association
enum MiniFTMode //byte
{
	MODE_IDLE = 0,
	MODE_PROBE = 1,
	MODE_PROBEK1 = 2,
	MODE_PROBEK2 = 3,
	MODE_PROBE_HOME = 4,
	MODE_PROBE_ADJUST = 5,
	MODE_POSN = 6,
	MODE_TEACH = 7,
	MODE_CALIBRATE = 8,
	MODE_INSPECT = 9,
	MODE_ESTOP = 10,
};

// ACTION_CODES value association
enum ACTION_CODES //byte
{
	ACTION_IDLE = 0,
	ACTION_READY = 1,
	ACTION_HOME = 2,
	ACTION_PROBE = 3,
	ACTION_CALC = 4,
	ACTION_RUNNING = 5,
	ACTION_MOVE = 6,
	ACTION_EXECUTE = 7,
	ACTION_INSPECT = 8,
	ACTION_ESTOP = 9,
};

// GRAVCOMP_AXES value association
enum GRAVCOMP_AXES //byte
{
	GRAVCOMP_ALLAXES = 0,
	GRAVCOMP_XAXIS = 1,
	GRAVCOMP_YAXIS = 2,
};

// JOGVALUE value association
enum JOGVALUE //short
{
	JOGSTOP = 0,
	JOGPOS = 1,
	JOGNEG = 2,
};

// ProbeMethods value association
enum ProbeMethods //byte
{
	PROBE_NONE = 0,
	PROBE_INSTANT = 1,
	PROBE_INSTANT_OFFSET = 2,
	PROBE_MANUAL = 3,
	PROBE_LASER = 4,
	PROBE_CAM = 5,
	PROBE_HARDSTOP = 6,
	PROBE_ABSOLUTE = 7,
};

// InspectMethods value association
enum InspectMethods //byte
{
	INSPECT_MANUAL = 3,
	INSPECT_LASER = 4,
	INSPECT_CAMERA = 5,
};

// ProbeDir value association
enum ProbeDir //short
{
	PROBEDIR_NONE = 0,
	PROBEDIR_ATOB = 1,
	PROBEDIR_BTOA = 2,
};

// DrillDir value association
enum DrillDir //short
{
	DRILLDIR_NONE = 0,
	DRILLDIR_ATOB = 1,
	DRILLDIR_BTOA = 2,
	DRILLDIR_SAME = 3,
	DRILLDIR_REVERSE = 4,
};

// ProbeCodes value association
enum ProbeCodes //byte
{
	PC_CLEAR = 0,
	PC_PROBE = 1,
	PC_MOVE = 2,
	PC_MOVE_PROBE = 3,
	PC_MOVE_PROBE_ALL = 4,
	PC_STOP = 5,
	PC_COMPLETE = 6,
	PC_ADD = 7,
	PC_DELETE = 8,
	PC_REAPPLY = 9,
};

// ProbeStatus value association
enum ProbeStatus //byte
{
	PS_NO_PROBE = 0,
	PS_PROBED_APPROX_DIFF_FAILURE = 1,
	PS_PROBED_KHOLE_DIFF_FAILURE = 2,
	PS_PROBED_FAILED = 3,
	PS_EXTRAPOLATED = 4,
	PS_APPROXIMATE = 5,
	PS_PROBING = 6,
	PS_PENDING_ACCEPTANCE = 7,
	PS_PROBED = 8,
	PS_PROBED_ACCEPTED = 9,
};

// ProbeRegistration value association
enum ProbeRegistration //byte
{
	REGISTRATION_UNKNOWN = 0,
	REGISTRATION_APPROX = 1,
	REGISTRATION_EXACT = 2,
};

// EEOptions value association
enum EEOptions //byte
{
	EENONE = 0,
	EEKEYPRESSER = 1,
	EEHDRAIL = 2,
	EERIVET = 3,
	EECIRCRIVET = 4,
	EEDRILLFILL = 5,
	EEGSPINDLE = 6,
	EEHDDRILLFILL = 7,
	EEHDGEN3RAIL = 8,
	EECIRCMFT1 = 9,
	EEFD1 = 10,
	EEFD2 = 11,
	reserved12 = 12,
	EEDRILLFILLADV = 13,
	EEHDGEN4RAIL = 14,
	EEHDGEN4CDRAIL = 15,
	EECIRCMFT2 = 16,
	EEHDGEN5RAIL = 17,
	EEFLOORBEAM = 18,
	EEDEFAULT = 64,
};

// AxisCode value association
enum AxisCode //byte
{
	AXIS_NULL = 0,
	AXIS_X = 1,
	AXIS_Y = 2,
	AXIS_CLAMP = 3,
	AXIS_A = 4,
	AXIS_Z = 5,
	AXIS_FF = 6,
	AXIS_FS = 7,
	AXIS_FTA = 8,
};

// HomeStatus value association
enum HomeStatus //byte
{
	HOME_NOT_DONE = 0,
	HOME_RUNNING = 1,
	HOME_FAILURE = 2,
	HOME_DONE = 3,
	HOME_PENDING = 4,
};

// HomeStatusReason value association
enum HomeStatusReason //byte
{
	HOMESR_NULL = 0,
	HOMESR_COM = 1,
	HOMESR_SENSOR = 2,
	HOMESR_MOTOR = 3,
	HOMESR_DISABLED = 4,
	HOMESR_UNKNOWN = 5,
};

// ProcessContinueMode value association
enum ProcessContinueMode //byte
{
	PROCESS_SINGLE = 0,
	PROCESS_CONTINUOUS = 1,
};

// YOrientation value association
enum YOrientation //byte
{
	Y_POS = 0,
	Y_NEG = 1,
	Y_UNKNOWN = 2,
};

// INSPECTTYPE value association
enum INSPECTTYPE //byte
{
	INSPECT_PROBE = 0,
	INSPECT_FULL = 1,
	INSPECT_FAST = 2,
};

// LSMETHODS value association
enum LSMETHODS //byte
{
	LSMETHOD_INTENSITY = 0,
	LSMETHOD_DELTA = 1,
	LSMETHOD_SYMETRIC = 2,
	LSMETHOD_GATE = 3,
	LSMETHOD_INSTANT = 4,
	LSMETHOD_EXTERNAL_BASE = 16,
	LSMETHOD_EXTERNAL_SYMETRIC = 17,
	LSMETHOD_EXTERNAL_STRATA_MIDS = 18,
	LSMETHOD_EXTERNAL_LRMIN = 19,
	LSMETHOD_EXTERNAL_LRFIRSTDELTA = 20,
	LSMETHOD_EXTERNAL_VALUELR = 21,
};

// DELTAMODES value association
enum DELTAMODES //byte
{
	DELTA_BASE = 1,
	DELTA_BASETOP = 2,
};

// OPERATION_BIT_FIELD_VA value association
enum OPERATION_BIT_FIELD_VA //ushort
{
	OP_BASE = 0,
	OP_VISIT = 1,
	OP_PROBE = 2,
	OP_DRILL = 4,
	OP_COUNTERSINK = 8,
	OP_SEAL = 16,
	OP_FILL = 32,
	OP_REMOVE = 64,
	OP_INSPECT = 128,
	OP_DRILL_ABORT = 512,
	OP_DRILL_STARTED = 1024,
	OP_DRILL_FAULT = 2048,
	OP_DRILL_SUCCESS = 4096,
	OP_PROBE_WARNINGS = 32768,
	OP_ENABLE_BITS_MASK = 127,
	OP_MAXVAL = 65535,
};

// MachineLocks value association
enum MachineLocks //byte
{
	NoLock = 0,
	XLock = 1,
	YLock = 2,
	XAndYLock = 3,
};

// POSNDISPLAYMODES value association
enum POSNDISPLAYMODES //byte
{
	PD_MACHINE = 0,
	PD_DATASET = 1,
	PD_DATASET_NS = 2,
	PD_NOTHING = 4,
};

// POSNDISPLAYORIGIN value association
enum POSNDISPLAYORIGIN //byte
{
	PD_ZERO = 0,
	PD_PK = 1,
	PD_SK = 2,
	PD_NEAREST = 3,
};

// POSNDISPLAYCONTENT value association
enum POSNDISPLAYCONTENT //byte
{
	PD_XY = 0,
	PD_DIST = 1,
	PD_IDS = 2,
};

// ToolVerifyEnable value association
enum ToolVerifyEnable //byte
{
	TOOLVERIFY_OFF = 0,
	TOOLVERIFY_PRE = 1,
	TOOLVERIFY_ARRIVE = 2,
	TOOLVERIFY_ACTION = 3,
};

// PartPgmStatus value association
enum PartPgmStatus //byte
{
	PP_NOTLOADED = 0,
	PP_LOADOK = 1,
	PP_NOTFOUND = 2,
	PP_PARSEFAIL = 3,
	PP_LOADING = 4,
};

// PartPgmLocked value association
enum PartPgmLocked //byte
{
	PARTPGM_NOT_LOCKED = 0,
	PARTPGM_LOCKED = 1,
};

// GravcompStatus value association
enum GravcompStatus //byte
{
	GRAVCOMP_NOTDONE = 0,
	GRAVCOMP_RUNNING = 1,
	GRAVCOMP_PASS = 2,
	GRAVCOMP_FAILX = 3,
	GRAVCOMP_FAILY = 4,
};

// GravCompFloat value association
enum GravCompFloat //byte
{
	FLOAT_TOGGLE = 0,
	FLOAT_FLOAT = 1,
	FLOAT_UNFLOAT = 2,
	FLOAT_UNFLOAT_STOP = 3,
};

// GravCompAlgorithm value association
enum GravCompAlgorithm //byte
{
	GC_ORIGINAL = 1,
	GC_FILTERED = 2,
	GC_SHORT = 3,
	GC_IMMEDIATE = 4,
	GC_DRIFT = 5,
};

// FloatStatus value association
enum FloatStatus //byte
{
	FLOATSTAT_NOFLOAT = 0,
	FLOATSTAT_FLOAT = 1,
};

// MoveType value association
enum MoveType //byte
{
	MOVETYPE_ORIGINAL = 1,
	MOVETYPE_DIRECT = 2,
	MOVETYPE_FAST = 3,
	MOVETYPE_ROUGH = 4,
	MOVETYPE_VROUGH = 5,
};

// MoveDone value association
enum MoveDone //byte
{
	MOVEDONE_FALSE = 0,
	MOVEDONE_TRUE = 1,
	MOVEDONE_ERROR = 2,
	MOVEDONE_STOP = 3,
};

// AutoMove value association
enum AutoMove //byte
{
	AUTOMOVE_OFF = 0,
	AUTOMOVE_MOVE = 1,
	AUTOMOVE_ACTION = 2,
};

// AutoRepeat value association
enum AutoRepeat //byte
{
	NO_AUTOREPEAT = 0,
	AUTOREPEAT = 1,
};

// Station value association
enum Station //byte
{
	STATION_UNSPEC = 0,
	STATION_DRILL = 1,
	STATION_SEAL = 2,
	STATION_PICKUP = 3,
	STATION_FILL = 4,
	STATION_MOVING = 5,
	STATION_INSPECT = 6,
	STATION_LASERPOINTER = 7,
	STATION_RESERVED8 = 8,
	STATION_RESERVED9 = 9,
};

// VisionInfo value association
enum VisionInfo //byte
{
	VisionInfoPosition = 1,
	VisionInfoPositionExpected = 2,
	VisionInfoDiameter = 4,
	VisionInfoCountersink = 8,
	VisionInfoPositionProbeEdgeVec = 16,
};

// RFIDSEEKMETHODS value association
enum RFIDSEEKMETHODS //byte
{
	FIRST_DETECTION = 1,		// (complete at first sight.)
	CENTER_FAST = 2,		// (complete at first start and first finish.)
	CENTER_1PASS = 3,		// (complete after 1 st fine speed start and finish.)
	CENTER_2PASS = 4,		// (complete after 2 fine passes going oposite directions.)
};

// RFIDOPTIONS value association
enum RFIDOPTIONS //ushort
{
	RFID_OPTION_BASE = 0,
	RFID_OPTION_REVERSE_ON_HARDSTOP = 1,
	RFID_OPTION_ALERT_PROGRESS_DATA = 2,
	RFID_OPTION_READS_UPPER_TAG = 4,
	RFID_OPTION_MAX = 7,
};

// RFIDSTATE value association
enum RFIDSTATE //byte
{
	RFID_INIT = 0,
	RFID_NOT_SUPPORTED = 1,		// MiniFT System Build Does not support RFID
	RFID_NOT_ENABLED = 2,
	RFID_NO_CONNECTION = 3,
	RFID_NOT_PRESENT = 4,		// (no tag found and no specific error reported... RFID can't see anything, but is working properly)
	RFID_PRESENT = 5,		// (tag is present.  In the case of seeking, this means it's complete also)
	RFID_ERROR = 6,		// (read attempt failed for some reason.)
};

// RFIDCONTEXT value association
enum RFIDCONTEXT //byte
{
	RFID_CONTEXT_READ = 0,
	RFID_CONTEXT_SEEK = 1,
};

// RFIDSEEKSTATE value association
enum RFIDSEEKSTATE //byte
{
	RFID_SEEK_NULL = 0,		// OID was created to show the result of a READ, and there is no SEEK status.
	RFID_SEEK_INIT = 1,		// (seek commanded, but nothing else done yet)
	RFID_SEEK_MOVE1 = 2,		// (move 1 pending)
	RFID_SEEK_MOVE2 = 3,		// (move 2 pending)
	RFID_SEEK_MOVEOFF = 4,		// (move off pending)
	RFID_SEEK_FINE1 = 5,		// (fine move 1 pending)
	RFID_SEEK_FINE2 = 6,		// (fine move 2 pending)
	RFID_SEEK_RETURN = 7,		// (return to start move)
	RFID_SEEK_CENTER = 8,		// (center on tag center)
	RFID_SEEK_TERMINAL_STATE = 9,		// (used for checking if seekstate is terminal: all states from here down are terminal states)
	RFID_SEEK_SUCCESS = 10,		// (ended seek operation)
	RFID_SEEK_FAIL_INIT = 11,		// (either not supported, not enabled, or no connection. See state field)
	RFID_SEEK_FAIL_NOT_FOUND = 12,		// (was able to follow all the specified motions, but never found a tag)
	RFID_SEEK_FAIL_NOT_LOCATED = 13,		// (was able to follow all the specified motions, found a tag, but couldn'tisolate the location according to the specified motion requirements.)
	RFID_SEEK_FAIL_HARDSTOP = 14,		// (ended after following specified motion options with the last eventbeing a hardstop.)
	RFID_SEEK_FAIL_RFID_ERROR = 15,		// (encountered errors from the RFID reader while looking for the tag.It might continue looking for a short while, but if it never finds a validtag then it will result in this state
	RFID_SEEK_FAIL_BAD_TAG_DATA = 16,		// (found tag, but tag did not conform to format or rules.)
};

// RFIDREAD value association
enum RFIDREAD //byte
{
	RFID_READ_OFF = 0,
	RFID_READ_NOW = 1,
	RFID_READ_CONTINUOUS = 2,
	RFID_READ_SEEK = 3,
	RFID_READ_STOP = 4,
};

// ProbeFlags value association
enum ProbeFlags //byte
{
	AUTO_MOVE_PROBE = 1,
	AUTO_COMPLETE = 2,
};


//Global Value Defines
#define MAX_FILENAME_LEN 128
#define MAX_STRING_LEN 32
//MakeOID-Section::END




//MakeOID-Section:: TYPEDEFS (oid complete)

typedef struct
{
	uint16 uiOID;
	uint16 uiCode;
} td_oid_minift_message_code;

typedef struct
{
	float fX;
	float fY;
} td_EncoderRatio;

typedef struct
{
	float fX;
	float fY;
} td_MCCurrentLimit;

typedef struct
{
	float fX;
	float fY;
} td_MCPeakCurrentLimit;

typedef struct
{
	float fxtemp;
	float fytemp;
	uint16 uiloop_avg_ms;
} td_oid_system_monitor;

typedef struct
{
	byte cProcess;
	byte cToolType;
	byte cFastenerType;
	byte cLayers;
	byte cCountersink;
	uint16 uiOperations;
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
	byte cmat1;
	byte cmat2;
	byte cmat3;
	byte cmat4;
	byte cmat5;
	byte cmat6;
	byte cmat7;
	byte cmat8;
} __attribute__((packed)) td_HoleParam;

typedef struct
{
	float fX;
	float fY;
} td_GravCompSpeed;

typedef struct
{
	float fX;
	float fY;
} td_GravCompAcc;

typedef struct
{
	float fX;
	float fY;
} td_GravCompDec;

typedef struct
{
	float fX;
	float fY;
} td_GravCompMoveDist;

typedef struct
{
	float fX;
	float fY;
} td_GravCompNoiseLimit;

typedef struct
{
	float fX;
	float fY;
} td_GravCompTriggerFactor;

typedef struct
{
	float fxp;
	float fxn;
	float fyp;
	float fyn;
} td_GravCompResults;

typedef struct
{
	float fX;
	float fY;
} td_ProbeOffset;

typedef struct
{
	float fexpected;
	float ffound;
} td_oid_khole_distance;

typedef struct
{
	byte ccode;
	byte cKIndex;
} td_oid_probe;

typedef struct
{
	float fX;
	float fY;
} td_oid_probe_position;

typedef struct
{
	byte cKIndex;
	byte cStatus;
	byte cMethod;
	float fX;
	float fY;
} __attribute__((packed)) td_oid_probe_status;

typedef struct
{
	byte cKIndex;
	byte cStatus;
	byte cMethod;
	float fX;
	float fY;
} __attribute__((packed)) td_oid_probe_start;

typedef struct
{
	float fX;
	float fY;
} td_HomeSpeed;

typedef struct
{
	float fX;
	float fY;
} td_HomeAcc;

typedef struct
{
	float fX;
	float fY;
} td_HomeDec;

typedef struct
{
	float fX;
	float fY;
} td_HomeMoveDist;

typedef struct
{
	float fX;
	float fY;
} td_HomeFineSpeed;

typedef struct
{
	byte caxis_code;
	byte cstatus;
	byte cstatus_reason;
} td_oid_home_report;

typedef struct
{
	float fDataSetX;
	float fDataSetY;
	float fMachineX;
	float fMachineY;
} td_oid_posnmode_curxy;

typedef struct
{
	float fDataSetX;
	float fDataSetY;
	float fMachineX;
	float fMachineY;
	int16 iNearPosn;
} td_oid_posnmode_nearxy;

typedef struct
{
	float fX;
	float fY;
} td_PreMove;

typedef struct
{
	float fX;
	float fY;
} td_PosnSpeed;

typedef struct
{
	float fX;
	float fY;
} td_PosnAcc;

typedef struct
{
	float fX;
	float fY;
} td_PosnDec;

typedef struct
{
	float fX;
	float fY;
} td_PosnFinalSpeed;

typedef struct
{
	float fX;
	float fY;
} td_PosnErrLimit;

typedef struct
{
	float fVLimitMarginX;
	float fVErrorX;
	float fVLimitMarginY;
	float fVErrorY;
} td_VelErrLimit;

typedef struct
{
	float fX;
	float fY;
} td_LongDistance;

typedef struct
{
	float fX;
	float fY;
} td_LongSpeed;

typedef struct
{
	float fX;
	float fY;
} td_MachineOffset;

typedef struct
{
	float fX;
	float fY;
	float fYExtension;
} td_MachineOffset1;

typedef struct
{
	float fX;
	float fY;
	float fYExtension;
} td_MachineOffset2;

typedef struct
{
	float fX;
	float fY;
} td_oid_tool_offset;

typedef struct
{
	float fX;
	float fY;
	float fYExtension;
} td_DrillOffset1;

typedef struct
{
	float fX;
	float fY;
	float fYExtension;
} td_DrillOffset2;

typedef struct
{
	float fx;
	float fy;
} td_OffsetSeal;

typedef struct
{
	float fx;
	float fy;
} td_OffsetFill;

typedef struct
{
	float fX;
	float fY;
} td_oid_jog;

typedef struct
{
	float fX;
	float fY;
} td_JogSpeed;

typedef struct
{
	float fX;
	float fY;
} td_JogAcc;

typedef struct
{
	float fX;
	float fY;
} td_JogDec;

typedef struct
{
	float fX;
	float fY;
} td_JogFactor;

typedef struct
{
	float fX;
	float fY;
} td_HomePosnYPos;

typedef struct
{
	float fMinX;
	float fMaxX;
	float fMinY;
	float fMaxY;
} td_PosnLimitYPos;

typedef struct
{
	float fX;
	float fY;
} td_HomePosnYNeg;

typedef struct
{
	float fMinX;
	float fMaxX;
	float fMinY;
	float fMaxY;
} td_PosnLimitYNeg;

typedef struct
{
	float fX;
	float fY;
} td_LaserSensorOffset;

typedef struct
{
	float fX;
	float fY;
} td_CamOffset;

typedef struct
{
	uint16 uiSequence;
	uint16 uiFlags;
	uint16 uiWidth;
	uint16 uiHeight;
	float fX;
	float fY;
	float fDiameter;
	float fXPixels;
	float fYPixels;
	float fDiameterPixels;
	float fPixelsPerInch;
	byte cEdgeStatus;
	byte cEdgeNote;
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
} __attribute__((packed)) td_oid_vision_image;

typedef struct
{
	byte cContext;
	byte cStatus;
	byte cMethod;
	byte cInfo;
	int32 lposn;
	float fXPosition;
	float fYPosition;
	float fXPositionExpected;
	float fYPositionExpected;
	float fDiameterExpected;
	float fDiameter;
	float fAlgDiameter;
	byte cResultMessage;
	float fCountersinkExpected;
	float fCountersinkDepth;
	byte cCountersinkAccepted;
	float fNEdgeHeight;
	float fPEdgeHeight;
	float fNCsnkEdgeHeight;
	float fPCsnkEdgeHeight;
	uint16 uiImageSequence;
	float finspecttime;
	byte cEdgeStatus;
	byte cEdgeNote;
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
} __attribute__((packed)) td_VisionInspectResults;

typedef struct
{
	float fsearch_speed;
	float fseek_speed;
	float frscan_speed;
	float frscan_speed_fast;
	float fscan_speed;
	float fscan_speed_fast;
	float fprobe_diameter;
	float funknown_diameter;
	byte cmode;
	byte cmode_fast;
	byte cuse_avg;
	byte cfull_scan;
	byte cgdata_sel;
	byte cassume_posn;
	byte cassume_posn_fast;
	byte crect_center;
	byte cloops;
	byte cdelta_mode;
	int16 idelta_flat;
	float fdelta_basespan;
	int16 idelta_pos;
	int16 idelta_neg;
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

typedef struct
{
	float fmove_speed;
	byte cInfoMask;
	byte cAMode;
	byte cCMode;
	byte cAux1;
	float fmove_required;
	float fmax_over_exp_diameter;
	float fmax_under_exp_diameter;
	float fmax_csnk_diff;
	float fmax_over_csnk;
	float fmax_under_csnk;
} td_CamAlgParam;

typedef struct
{
	float fdiameter1;
	int16 idelay1;
	float fdiameter2;
	int16 idelay2;
	float fdiameter3;
	int16 idelay3;
	float fdiameter4;
	int16 idelay4;
	float fdiameter5;
	int16 idelay5;
} __attribute__((packed)) td_PrimeDelay;

typedef struct
{
	byte cDevice;
	byte cSeverity;
	int32 lFaultCode;
} __attribute__((packed)) td_FastenerFault;

typedef struct
{
	float fX;
	float fY;
} td_oid_posnmode_active_premovexy;

typedef struct
{
	byte cDevice;
	byte cSeverity;
	int32 lFaultCode;
} __attribute__((packed)) td_DrillFault;

typedef struct
{
	int16 iHoleNumber;
	int16 iHoleResult;
} td_HoleResultData;

typedef struct
{
	float fMachineX;
	float fMachineY;
} td_oid_movexy;

typedef struct
{
	byte cmode;
	byte corigin;
	byte ccontent;
} td_PosnDisplay;

typedef struct
{
	byte cenabled;
	byte cmethod;
	uint16 uioptions;
	uint16 uicontinuousReadCycleTime;
	uint16 uiseekReadCycleTime;
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

typedef struct
{
	byte cstate;
	byte ccontext;
	byte cseekstate;
	uint32 ultimestamp;
	uint32 ulrfidtimestamp;
	float fposition;
	//byte * sztagdata; //not stored in struct
	uint16 uicrc16;
	uint16 uiendcode;
	uint32 ulseektime;
	float fsstart;
	float fpstart;
	float fpend;
	float fnstart;
	float fnend;
	float fhs1;
	float fhs2;
	float fhsf;
} __attribute__((packed)) td_RFIDData;

typedef struct
{
	int16 iZeroX;
	int16 iZeroY;
	int16 iZeroZ;
	int16 iCountsPerGX;
	int16 iCountsPerGY;
	int16 iCountsPerGZ;
} td_ForceSensorCalibration;

typedef struct
{
	float fX;
	float fY;
	float fZ;
	byte cErrFlag;
} td_ForceSensor;

typedef struct
{
	uint16 uiSensorInterval;
	uint16 uiMinUpdateDelta;
	byte cActive;
	byte cCurrentUnderMethod;
	uint16 uiCurrentOverX;
	uint16 uiCurrentUnderX;
	uint16 uiCurrentOverY;
	uint16 uiCurrentUnderY;
	uint16 uiFullGravX;
	uint16 uiFullGravY;
	uint16 uiFlatForceX;
	uint16 uiFlatForceY;
} td_ForceLimits;

typedef struct
{
	uint16 uim1;
	uint16 uim2;
	uint16 uim3;
	uint16 uim4;
	uint16 uim5;
	uint16 uim6;
} td_MOCal;

typedef struct
{
	byte cscan;
	byte cgroup;
	byte cdir;
	byte cfoundCenter;
	byte cfoundEdges;
	byte cresult;
	float frangeN;
	float frangeP;
	float fedgeN;
	float fedgeP;
	float ffeatureN;
	float ffeatureP;
	float fcenter;
	float foffsetNP;
	float fdiffNP;
} __attribute__((packed))  td_oid_vision_external_analysis;

typedef struct
{
	byte format[2];
	byte segment[1];
	byte tb[1];
	byte posnTag[6];
	byte lenRail[6];
	byte SerialNumber[32];
} td_oid_rfid_tag_set;

typedef struct
{
	byte cDrill;
	byte cFastener;
	byte cFastenerTray;
	byte cAux1;
	byte cAux2;
	byte cAux3;
	byte cAux4;
	byte cAux5;
} td_SystemComponents;

typedef struct
{
	byte cFormat;
	byte cRailType;
	byte cGroup;
	byte cSegment;
	byte cRailSide;
	uint32 ulSerialNumber;
	uint32 ulPosition;
	uint32 ulSegmentPosition;
} __attribute__((packed)) td_oid_rfid_tag_set2;

typedef struct
{
	float flevel;
	float finch_volts;
	float fmin_valid;
	float fmax_valid;
} td_AngleSensorANegCalibration;

typedef struct
{
	float flevel;
	float finch_volts;
	float fmin_valid;
	float fmax_valid;
} td_AngleSensorAPosCalibration;

typedef struct
{
	float flevel;
	float fdeadzone;
	float fspeed_slope;
	float fmin_valid;
	float fmax_valid;
	float fCountsPerDegree;
} td_AngleSensorMCCalibration;

typedef struct
{
	int16 iCountsFree;
	int16 iCountsClamped;
} td_ClampStain;

//MakeOID-Section::END


//MakeOID-Section:: Global Smart Tool ID And Version Constant Defines 

extern const char * SmartToolTypeMiniFT;

extern const char * MiniFTSysVersion;

extern const uint16 MiniFTCOMVersion;

extern char * pszOIDNamesMiniFT[];

extern td_SmartTool g_MiniFTSmartTool;


//MakeOID-Section:: CONFIG TYPEDEF AND PREDECLARATION

typedef struct
{
	td_EncoderRatio EncoderRatio;
	td_MCCurrentLimit MCCurrentLimit;
	td_MCPeakCurrentLimit MCPeakCurrentLimit;
	uint16 uiBrakeOnTimeout;
	byte cEEOptionDefault;
	byte cbeeper;
	byte cToolVerifyEnable;
	float fReturnHeight;
	byte cScaleMode;
	byte cGravCompAxes;
	td_GravCompSpeed GravCompSpeed;
	td_GravCompAcc GravCompAcc;
	td_GravCompDec GravCompDec;
	td_GravCompMoveDist GravCompMoveDist;
	byte cGravCompAlgorithm;
	td_GravCompNoiseLimit GravCompNoiseLimit;
	td_GravCompTriggerFactor GravCompTriggerFactor;
	float fFloatSpeedLimit;
	float fJogSpeedLimit;
	float fMaxSpeedX;
	float fMaxSpeedY;
	byte cProbeMethodDefault;
	td_ProbeOffset ProbeOffset;
	int16 iProbeDir;
	int16 iDrillDir;
	float fMaxKholeDistanceError;
	float fApproxLocationError;
	byte cProbeAcceptRequired;
	td_HomeSpeed HomeSpeed;
	td_HomeAcc HomeAcc;
	td_HomeDec HomeDec;
	td_HomeMoveDist HomeMoveDist;
	float fProbeAdjustLimit;
	td_HomeFineSpeed HomeFineSpeed;
	byte cMoveType;
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
	byte cDrillHoleOneTime;
	byte cToolFlip;
	byte cDriveThroughBacklash;
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
	byte cObstructionCodeMask;
	byte cMachineLockRequired;
	uint16 uiALockDelay;
	uint16 uiAUnlockDelay;
	uint16 uiLegsLockDelay;
	uint16 uiLegsUnlockDelay;
	uint16 uiLegsUpDelay;
	uint16 uiLegsDownDelay;
	uint16 uiLowPressureDelay;
	uint16 uiLowPressureDownDelay;
	uint16 uiPressureDelay;
	uint16 uiPressureDownDelay;
	uint16 uiLowPressure;
	uint16 uiPressure;
	uint16 uiAirClear;
	td_LaserSensorOffset LaserSensorOffset;
	td_CamOffset CamOffset;
	td_LaserSensorAlgParam LaserSensorAlgParam;
	td_CamAlgParam CamAlgParam;
	byte cVisionAutoInspect;
	byte cProcessContinueMode;
	uint16 uiProcessOperations;
	byte cSealClamp;
	int16 iSealPressureDelay;
	int16 iSealPressureReleaseDelay;
	td_PrimeDelay PrimeDelay;
	int16 iSealGlobDelay;
	int16 iSealApplyDelay;
	byte cFillClamp_;
	int16 iFillExtendDelay_;
	int16 iFillRamDelay_;
	int16 iFastenerPostDelay_;
	float fToolZBase;
	td_PosnDisplay PosnDisplay;
	td_RFIDConfig RFIDConfig;
	uint16 uiEStopClearDelay;
	uint16 uiDrillButtonDelay;
	byte cUseCutterDetect;
	byte cJogEnableTimeout;
	uint16 uiDrillCycleDelay;
	byte cInspectMethod;
	byte cCommandInspectMethod;
	td_ForceSensorCalibration ForceSensorCalibration;
	td_ForceLimits ForceLimits;
	byte cProbeFlags;
	td_MOCal MOCal;
	float fMaxKholeDistanceCheck;
	float fMaxEdgeShiftProbeAccept;
	byte cAllowDrillBeyondShiftLimits;
	td_SystemComponents SystemComponents;
    uint16 uiSpritzDurationMs;
    uint16 uiBurstOnDurationMs;
	uint16 uiBurstIntervalMs;
	td_AngleSensorANegCalibration AngleSensorANegCalibration;
	td_AngleSensorAPosCalibration AngleSensorAPosCalibration;
	float fAngleSensorABaseWidth;
	td_AngleSensorMCCalibration AngleSensorMCCalibration;
	float fAAxisMoveSpeed;
	td_ClampStain ClampStain;
} td_ConfigData;

extern td_ConfigData g_ConfigData;

//MakeOID-Section::END




//MakeOID-Section:: GLOBAL PREDECLARATIONS (for header)

extern byte g_cModeState;
extern byte g_cAction;
extern byte g_cEEOption;
extern td_HoleParam g_HoleParam;
extern byte g_cLoadedProcess;
extern float g_fGaugeLength;
extern char g_szPartPgmFilename[MAX_FILENAME_LEN];
extern byte g_cPartPgmStatus;
extern byte g_cGravCompStatus;
extern byte g_cFloatStatus;
extern td_GravCompResults g_GravCompResults;
extern float g_fXRailSurfaceOffset;
extern byte g_cProbeMethod;
extern byte g_cPattern;
extern byte g_cMoveDone;
extern td_MachineOffset g_MachineOffset;
extern byte g_cMachineOffsetCompensationAdjustment;
extern td_MachineOffset1 g_MachineOffset1;
extern td_MachineOffset2 g_MachineOffset2;
extern byte g_cStation;
extern td_JogFactor g_JogFactor;
extern byte g_cALockMode;
extern td_VisionInspectResults g_VisionInspectResults;
extern byte g_cDrillState;
extern byte g_cDrillExplanation;
extern byte g_cSealState;
extern byte g_cFillState;
extern byte g_cFastenerArrived;
extern byte g_cAccelArm_;
extern byte g_cAccelTrigger_;
extern td_FastenerFault g_FastenerFault;
extern td_DrillFault g_DrillFault;
extern td_HoleResultData g_HoleResultData;
extern td_RFIDData g_RFIDData;
extern byte g_szTagDatalen;
extern char g_szTagData[65]; //64 + 1
extern byte g_cReadRFID;
extern td_ForceSensor g_ForceSensor;
//MakeOID-Section::END


//MakeOID-Section:: FUNCTION PREDECLARATIONS

void MiniFTInitConfig(void);

//MakeOID-Section::END

#endif
