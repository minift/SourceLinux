#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include "SmartTool.h"
#include "SmartToolUtil.h"
#include "SocketConsole.h"

uint32 get_ms_time()
{
	struct timeval tv;
	struct timezone tz;
	gettimeofday(&tv, &tz);
	uint32 time = (uint32) (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
//	logf("MS %d", time);
	return time;
}

uint32 get_sec_time()
{
	struct timeval tv;
	struct timezone tz;
	gettimeofday(&tv, &tz);
	uint32 time = (uint32) (tv.tv_sec);
//	logf("S %d", time);
	return time;
}

uint32 get_us_time()
{
	struct timeval tv;
	struct timezone tz;
	gettimeofday(&tv, &tz);
	uint32 time = (uint32) (tv.tv_usec);
//	logf("US %d", time);
	return time;
}

//  Calculate difference between two uint32 time values.
uint32 GetDiff( uint32 ui1, uint32 ui2 )
{
    return( ui1 - ui2 );
}


int16 atofSafeFailure = 0;

double atofSafe(char * s,char * ep)
{
	double d = 0;
	char * rep;
	//char cannonicallooptestbuffer[64];
	atofSafeFailure = 0; //clear
	d = strtod(s, &rep);
	if (errno == ERANGE)
	{
		logf("ERROR Out of Range\r\n");
		atofSafeFailure = -1;
		return 0;
	}
	//THIS IS A good test, but hard to make work
	//because of the somewhat random standard float format
	//snprintf(cannonicallooptestbuffer, 64, "%.0f", d);
	//logf("INTERNAL = %s \r\n",cannonicallooptestbuffer);
	//int ilen = strlen(cannonicallooptestbuffer);
	//if (memcmp(cannonicallooptestbuffer, s, ilen) != 0)
	//{
	//	//not the same
	//	logf("WARNING Value does not appear to be in cannonical and returnable double format %.0f.\r\n", d);
	//	atofSafeFailure = -3;
	//	return 0;
	//}

	if (rep != 0)
	{
		if (rep == s)
		{
			logf("ERROR Value Format Error\r\n");
			atofSafeFailure = -2;
			return 0;
		}
		if (ep != 0)
		{
			if (rep==ep)
			{
				//consumed to the expected point
			}
			else
			{
				logf("ERROR Value Format Error\r\n");
				atofSafeFailure = -4;
				return 0;
			}
		}
	}
	//no error
	return d;
}



//FIXME PORTLOW
//LOW QUALLITY STATS FUNCTIONS FOR DEBUGGING
//I threw this together and placed it here for use doing timing....
//USE LIKE THIS
//STATSInit() <===== at program start up
//STATSStart()
//FunctionAGoesHere();
//STATSEnd(1)
//STATSStart()
//FunctionBGoesHere();
//STATSEnd(2)
//STATSCount() <===== somewhere in your program.  Will print out the stats eventually
uint32 STATSmaxTimeSection[256];
uint32 STATSstartTime = 0;
uint32 STATScount = 0;
uint32 STATSshowTime = 0;
void STATSInit()
{
	int i=0;
	while(i<256)
	{
		STATSmaxTimeSection[i++]=0;
	}
	STATScount = 0;
	STATSshowTime = MS_TIMER;
}
void STATSShow()
{
	int i=0;
	logf("STATS count=%d\r\n",STATScount);
	while(i<256)
	{
		int max=STATSmaxTimeSection[i];
		if (max>0)
		{
			logf(" %d \t%d\r\n",i,max);
		}
		i++;
	}
}
void STATSStart()	{ STATSstartTime=US_TIMER; }
void STATSEnd(int x)
{
	uint32 STATSeTime = 0;
	STATSeTime=US_TIMER-STATSstartTime;
	if(STATSmaxTimeSection[x]<STATSeTime)
	{
		STATSmaxTimeSection[x]=STATSeTime;
	}
}
void STATSCount()
{
	STATScount++;

	uint32 STATSeTime = MS_TIMER - STATSshowTime;
	if (STATSeTime>120000) //every 2 minutes
	{
		//took 1000 counts.... show data now
		STATSShow();
		STATSInit(); //to reset
		STATSshowTime = MS_TIMER;
	}
}
