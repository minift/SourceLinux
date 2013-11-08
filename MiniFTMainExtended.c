// MiniFTMainExtended.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "MiniFTDefs.h"
#include "hwio.h"
#include "SmartTool.h"
#include "MiniFTSmartTool.h"
#include "SocketConsole.h"




//Data, string, and debug functions

//FIXME: Found out that error checking only detects if \0 \r \n is behind the number....
//Not good enough, and too strict as well.  May need a better check.
int SignedCharValue(char cCharacter)
{
//FIXME0 test if used
	int iResult;
	iResult = cCharacter;
	if (iResult >= 128)
	{
		iResult = iResult - 256;
	}
	return iResult;
}

//string util
char * scanToNull(char * s)
{
	while (*s != 0)
	{
		s++;
	}
	return s;
}

char * scanToChar(char * s, char c)
{
	while (*s != c && *s != 0)
	{
		s++;
	}
	return s;
}

char * scanPastWhiteSpace(char * s)
{
	while (*s == ' ' || *s == '\t')
	{
		s++;
	}
	return s;
}

char * scanToWhiteSpace(char * s)
{
	char c;
	while (1)
	{
		c = *s;
		if (c == ' ' || c == '\t' || c == 0)
		{
			break;
		}
		s++;
	}
	return s;
}

char * scanToWhiteSpaceOrChar(char * s, char cDelimiter)
{
	char c;
	while (1)
	{
		c = *s;
		if (c == ' ' || c == '\t' || c == cDelimiter || c == 0)
		{
			break;
		}
		s++;
	}
	return s;
}

//returns true if prefix starts str

int prefix(char *prefix, char *str)
{
	while (1)
	{
		if (*prefix == 0)
		{
			return 1;
		}
		if (*str == 0)
		{
			break;
		}
		if (*str != *prefix)
		{
			break;
		}
		prefix++;
		str++;
	}
	return 0;
}

//reutrns 1 if equal, 0 if not... like strcmp, but without the awkward reverse bool return value.

int equal(char *str1, char *str2)
{
	return (strcmp(str1, str2) == 0);
}

//works like string copy, but returns the pointer past the null.
//This is 100% perfect for a very fast version of our string storage copy.
// (see how simple it makes that function.)

char *strcpy_returnpost(char *dst, char *src)
{
	int i = strlen(src);
	memcpy(dst, src, i);
	return dst + i;
}

char split(char cDelimiter, char * s, char ** p_szFields, char cMaxFields)
{
	unsigned char cFieldNo;

	cFieldNo = 0;
	while (cFieldNo < cMaxFields)
	{
		s = scanPastWhiteSpace(s);
		if (*s == '"')
		{
			s++;
			//field starts here
			p_szFields[cFieldNo++] = s;
			s = scanToChar(s, '"');
			if (*s != '"')
			{
				//quote mismatch
				return SPLIT_PARSE_ERROR_QUOTE_MISMATCH;
			}
			//terminate this value at the quote and move forward
			*s++ = 0;
			if (*s == cDelimiter)
			{
				s++;
				//next field
				continue;
			}
			if (*s == 0)
			{
				//nothing more to parse
				break;
			}
		}
		else
		{
			//field starts here
			p_szFields[cFieldNo++] = s;
			//go past non quoted value
			s = scanToWhiteSpaceOrChar(s, cDelimiter);
			if (*s == cDelimiter)
			{
				*s++ = 0; //terminate this value, and move forward
				continue;
			}
			if (*s == 0)
			{
				//nothing more to parse
				break;
			}
			//must be white space
			*s++ = 0; //terminate this value, and move forward
		}
//only white space allowed here
//scan past any white space
		s = scanPastWhiteSpace(s);
		if (*s == 0)
		{
			//nothing more to parse
			break;
		}
		if (*s == cDelimiter)
		{
			s++;
			continue;
		}
//not null, not delimeter, this is an error
		return SPLIT_PARSE_ERROR_CHARS_AFTER_VALUE;
	}
	return cFieldNo;
}

void rchr(char *s, char c, char n)
{
	while (*s != 0)
	{
		if (*s == c)
		{
			*s = n;
		}
		s++;
	}
	return;
}


//hex conversion
byte g_hexmap[256];

void initCopyHexToByte()
{
	byte c;
	memset(g_hexmap, 255, 256);

 //NOTE: the reason to do this expanded is that the compiler may be able to condense this to 26 mem sets.
 // rather than what is actually more instructions to do it with loops, since constant addresses apply.
	g_hexmap['0'] = 0;
	g_hexmap['1'] = 1;
	g_hexmap['2'] = 2;
	g_hexmap['3'] = 3;
 	g_hexmap['4'] = 4;
 	g_hexmap['5'] = 5;
 	g_hexmap['6'] = 6;
 	g_hexmap['7'] = 7;
 	g_hexmap['8'] = 8;
 	g_hexmap['9'] = 9;

 	c = 10;
 	g_hexmap['a'] = c;
 	g_hexmap['A'] = c;
 	c = 11;
 	g_hexmap['b'] = c;
 	g_hexmap['B'] = c;
 	c = 12;
 	g_hexmap['c'] = c;
 	g_hexmap['C'] = c;
 	c = 13;
 	g_hexmap['d'] = c;
 	g_hexmap['D'] = c;
 	c = 14;
 	g_hexmap['e'] = c;
 	g_hexmap['E'] = c;
 	c = 15;
 	g_hexmap['f'] = c;
 	g_hexmap['F'] = c;
}

byte copyHexToByte(byte * bytes, byte *szhex, byte cCount, byte cRequireEnd)
{
 	byte c;
 	byte b;
 	byte b2;
 	byte cInd;

 	cInd = 0;
 	while (cInd < cCount)
 	{
 		c = *szhex++;
 		if (c == 0)
 		{
 			break;
 		}
 		if (c == ' ')
 		{
 			continue;
 		}
 		if (c == '\t')
 		{
 			continue;
 		}
 		if (c == '-')
 		{
 			continue;
 		}
 		if (c == '"')
 		{
 			continue;
 		}
 //found different character
 		b = g_hexmap[c];
 		if (b == 255)
 		{
 			break;
 		} //not a valid hex byte
 		c = *szhex++;
 		b2 = g_hexmap[c];
 		if (b2 == 255)
 		{
 			break;
 		} //not a valid hex byte
 		b = b << 4; // left shift
 		b = b | b2;
 		bytes[cInd++] = b;
 	}
 	if (cInd < cCount)
 	{
 //broke out and did not find all the needed bytes
 		return 0;
 	}
 	if (cRequireEnd)
 	{
 		c = *szhex;
 		if (c != 0)
 		{
 			//null did not follow the hex buffer.
 			return 0;
 		}
 	}
 	return 1;
}

 //generic and specific display functions

void memchardump(char * label, char * data, int iLen)
{
#ifdef USE_OUTPUT
 	int i;
 	byte c;
 	byte nl;
 	logf("%s[] = ", label);
 	nl = 0;
 	logf("  [0]:");
 	for (i = 0; i < iLen; i++)
 	{
 		if (nl >= 20)
 		{
 			logf("\r\n   ");
 			logf("[%d]:", i);
 			nl = 0;
 		}
 		c = data[i];
 		if (c > 20 && c <= 127)
 		{
 			logf("%c", c);
 		}
 		else
 		{
 			logf(" %d ", c);
 			if (c == 10)
 			{
 				nl = 40;
 			}
 		}
 		nl++;
 	}
 	logf("\r\n");
#endif
}

void memdump(char * label, unsigned char * data, int iLen)
{
#ifdef USE_OUTPUT
 	int i;
 	logf("%s[] = ", label);

 	for (i = 0; i < iLen; i++)
 	{
 		if (i > 0 && (i % 20) == 0)
 		{
 			logf("\r\n   ");
 		}
 		logf("%x ", (int) data[i]);
 	}
 	logf("\r\n");
#endif
}







//////////////////////////////////////////////////////////////////////////////////
///  Profiling Tool - A performance profiling tool
//////////////////////////////////////////////////////////////////////////////////

#ifdef PROFILEPOINTS

#define PROFILEPOINTSMAX 256

uint32 g_tProfileTime[PROFILEPOINTSMAX];
char * g_szaProfileMessage[PROFILEPOINTSMAX];
int g_iProfileIndex = 0;
byte g_bProfileWrapped = 0;

void ClearProfiling()
{
	g_iProfileIndex = 0;
	g_bProfileWrapped = 0;
}

void ProfilePoint(char * p_cLocation)
{
	g_tProfileTime[g_iProfileIndex]=MS_TIMER;
	g_tProfileMessage[g_iProfileIndex]=p_cLocation;
	g_iProfileIndex++;
	if (iProfileIndex>=PROFILEPOINTSMAX)
	{
		g_iProfileIndex=0;
		g_bProfileWrapped=1;
	}
}

void PrintProfilePointsPart(int i,int end)
{
	char * pc;
	uint32 uitime;
	while(i<end)
	{
		uitime=g_tProfileTime[i];
		pc=g_pzaProfileMesssage[i];
		logf("%u\t%s\r\n",uitime,pc);
		i++;
	}
}

void PrintProfilePoints()
{
	if (g_bProfileWrapped==1)
	{
		PrintProfilePointsPart(g_iProfileIndex,PROFILEPOINTSMAX);
	}
	PrintProfilePointsPart(0,g_iProfileIndex);
	return;
}

#endif

//////////////////////////////////////////////////////////////////////////////////
///  Utillity Section
//////////////////////////////////////////////////////////////////////////////////

//Math
//Returns 0 on success
//returns another code on failure
byte FindIntersection(float ax, float ay, float bx, float by, float cx, float cy, float dx, float dy, float *fx, float *fy)
{
	float v1x;
	float v1y;
	float v2x;
	float v2y;
	float distAB, rCos, rSin, newX, ABpos;

	v1x = bx - ax;
	v1y = by - ay;
	v2x = dx - cx;
	v2y = dy - cy;

	//  Fail if either line is undefined.
	if ((ax == bx) && (ay == by))
	{
		return 1;
	}
	if ((cx == dx) && (cy == dy))
	{
		return 1;
	}

	//Due to the system this is on, also fail if the distances of the segments seem too short.
	//This will lead to errors.   this could actually be a 0 distances with a rounding issue.
	if (fabs(v1x) < 0.00001)
	{
		if (fabs(v1y) < 0.00001)
		{
			//both are too small: segment too short for accuracy in this float library
			return 2;
		}
	}
	if (fabs(v2x) < 0.00001)
	{
		if (fabs(v2y) < 0.00001)
		{
			//both are too small: segment too short
			return 3;
		}
	}

	//Now continue the standard algorithm
	//  (1) Translate the system so that point A is on the origin.
	bx -= ax;
	by -= ay;
	cx -= ax;
	cy -= ay;
	dx -= ax;
	dy -= ay;
	//Now ax,ay point is at 0,0 but preserve those numbers because we will need to translate back

	//  Discover the length of segment A-B.
	distAB = sqrt(bx * bx + by * by);

	//  (2) Rotate the system so that point B is on the positive X axis.
	rCos = bx / distAB;
	rSin = by / distAB;
	newX = cx * rCos + cy * rSin;
	cy = cy * rCos - cx * rSin;
	cx = newX;
	newX = dx * rCos + dy * rSin;
	dy = dy * rCos - dx * rSin;
	dx = newX;

	//  Fail if the lines are parallel.
	if (cy == dy)
	{
		return 4;
	}

	//THIS ONLY APPLIES IF YOU WANT THE SEGMENTS TO CROSS, NOT JUST THE LINES
	////  Fail if segment C-D doesn't cross line A-B.
	//if (cy<0 && dy<0 || Cy>=0. && Dy>=0.) return 11;

	//  (3) Discover the position of the intersection point along line A-B.
	ABpos = dx + (cx - dx) * dy / (dy - cy);

	//ABPos is also a distance, so we can use that as follows in step 4

	//THIS ONLY APPLIES IF YOU WANT THE SEGMENTS TO CROSS, NOT JUST THE LINES
	////  Fail if segment C-D crosses line A-B outside of segment A-B.
	//if (ABpos<0. || ABpos>distAB) return 12;

	//  (4) Apply the discovered position to line A-B in the original coordinate system.
	*fx = ax + ABpos * rCos;
	*fy = ay + ABpos * rSin;

	return 0;
}




#define SHOWIO_DIN_START 1
#define SHOWIO_DIN_END 10
#define SHOWIO_ANA_START 1
#define SHOWIO_ANA_END 4
byte g_clastdin[SHOWIO_DIN_END + 1];
float g_flastanain[SHOWIO_ANA_END + 1];

void ClearShowIO()
{
	int i;
	i = SHOWIO_DIN_START;
	while (i <= SHOWIO_DIN_END)
	{
		g_clastdin[i++] = 2;
	}
	return;
}

void ShowIO()
{
	byte ci;
	byte c;
	byte lc;
	ci = SHOWIO_DIN_START;
	while (ci <= SHOWIO_DIN_END)
	{
		c = digIn((int) ci);
		lc = g_clastdin[ci];
		g_clastdin[ci] = c;
		if (c != lc)
		{
			if (c > 0)
			{
				c = '1';
			}
			else
			{
				c = '0';
			}
			logf("DIN%d=%c\r\n", ci, c);
		}
		ci++;
	}
	return;
}

void ClearShowAna()
{
	int i;
	i = SHOWIO_ANA_START;
	while (i <= SHOWIO_ANA_END)
	{
		g_flastanain[i++] = 0;
	}
	return;
}

void ShowAna()
{
	byte ci;
	float f;
	float lf;
	ci = SHOWIO_ANA_START;
	while (ci <= SHOWIO_ANA_END)
	{
		f = anaInVolts((int) ci);
		lf = g_flastanain[ci];
		g_flastanain[ci] = f;

		if (fabs(f - lf) > 0.05)
		{
			logf("anaIn%d = %.4f\r\n", ci, f);
		}
		ci++;
	}
	return;
}


//FIXME PORTHIGH2  move this as I also work on this
byte g_cConfigLoadSuccess = 0;
byte g_cConfigSaveSuccess = 0;

void LoadConfigFile()
{
	//Load Configuration From Script

	//FIXME PORTHIGH2 replace this with script read
	InitConfigHARDCODED();

	//FIXME PORTHIGH2 build a method that will exit when this fails at boot
	//FIXME PORTMED2 build a method that will allow operation, but stay in estop, Then they can load config files and save them.
	g_cConfigLoadSuccess = TRUE;
}
//FIXME PORTMED2 build a way to determine if some script values violate system hardcoded limits....
//  some like current could be limited....
//FIXME PORTMED build a method to back the file so that every write is also given a date copy.
//      AND/OR  every time it starts, if the file size and/or date is diff than the last backup,
//		make a new one...
//FIXME PORTMED build the way to refresh properly and deal with issues about setting up of parameters...
//FIXME PORTMED make a management method for trying last working values etc...

//FIXME PORTMED make way to identify on screen that config load failed...
//		What about Error messages which pend until you connect????
//		Or a status OID that updates display ????

//FIXME PORTLOW solve the problem of distributed config

//FIXME PORTHIGH --Make permanent note about how the base vals are for ref only???

//FIXME PORTMED move minift messages...

void SaveConfigFile()
{
	g_cConfigSaveSuccess = TRUE;
}


////////////////////
//Temp Value Init Code



////////////////////////////////////////////////////////////////////////////////
// InitConfigHARDCODED
////////////////////////////////////////////////////////////////////////////////
//FIXME PORTHIGH4  this will go away when config driven....
void InitConfigHARDCODED(void)
{
	g_cConfigLoadSuccess = 0; //indicate that this was not the config loaded from the user block
	//Config initialization
	//MakeOID-generated:: CONFIG DEFAULTS (oid complete - except gen20 and gen25 options)
	//FIXME PORTFATAL store these in an archive and create loadable configs for any units...
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
	#ifdef GENCIRCMFTL26
	g_ConfigData.EncoderRatio.fX = 78021.11289; //Slightly better calculation 1.0001021352234934253708141108784
	g_ConfigData.EncoderRatio.fY = 35200;
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
	g_ConfigData.cbeeper = 0; //1;
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
	g_ConfigData.cProbeAcceptRequired = 1;
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
#ifdef GENCIRCMFTL26
	g_ConfigData.PosnSpeed.fY = 4.0; //try 4 then 5, but eventually can try Y=8, 9  or 10 is max
#endif
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
	g_ConfigData.cObstructionCodeMask = 252;
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
	g_ConfigData.CamAlgParam.cCMode = 1;
	g_ConfigData.CamAlgParam.cAux1 = 0;
	g_ConfigData.CamAlgParam.fmove_required = 2.0;
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
	g_ConfigData.cUseCutterDetect = 0;//FIXME PORTFATAL
	g_ConfigData.cJogEnableTimeout = 200;
	g_ConfigData.uiDrillCycleDelay = 6000;
	g_ConfigData.cInspectMethod = INSPECT_CAMERA;
	g_ConfigData.cCommandInspectMethod = INSPECT_CAMERA;
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

