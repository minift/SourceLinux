//SmartToolUtil.h

#ifndef SMARTTOOLUTIL_H_
#define SMARTTOOLUTIL_H_

//Headers needed by this header
#include "SmartTool.h"

uint32 get_ms_time();
#define MS_TIMER get_ms_time()

uint32 get_sec_time();
#define SEC_TIMER get_sec_time()

uint32 get_us_time();
#define US_TIMER get_us_time()

//  Calculate difference between two uint32 time values.
uint32 GetDiff( uint32 ui1, uint32 ui2 );

extern int16 atofSafeFailure;
double atofSafe(char * s,char * ep);

void STATSInit();
void STATSShow();
void STATSStart();
void STATSEnd();
void STATSCount();

#endif /* SMARTTOOLUTIL_H_ */
