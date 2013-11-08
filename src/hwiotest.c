/*
 * hwiotest.c
 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "hwio.h"

char lastDigIn[256];

int main_test(int argc, char *argv[])
{
	//char cContinue;
	int iResult = 0;
	int iPinNum = 1;
	int v = 1;
	int iExit = 1;

	if (argc == 3)
	{
		iPinNum = strtol(argv[1],0,10);
		v = strtol(argv[2],0,10);
		printf("setting out %d to %d \r\n", iPinNum, v);
		digOut(iPinNum, v);
		return 0;
	}

//#define DIGIN_TEST
//#define DIGOUT_TEST
#define ANAIN_TEST

#ifdef DIGIN_TEST

memset(lastDigIn,2,256);



while(iExit !=0)
{
	iPinNum = 1;
	while(iPinNum <= 7)
	{
		iResult = digIn(iPinNum);
		if (iResult != lastDigIn[iPinNum])
		{
			lastDigIn[iPinNum] = iResult;
			printf("in%d = %d\n", iPinNum, iResult);
		}
		iPinNum++;
	}

	if(iPinNum == 8)
	{
		iPinNum = 1;
	}
}
#endif

#if DIGOUT_TEST
while(iExit !=0)
{
	iPinNum = 1;
	while(iPinNum <= 8)
	{
		printf("Testing Digout on channel %d \n", iPinNum);
		digOut(iPinNum, 1);
		sleep(1);
		digOut(iPinNum, 0);
		sleep(1);
		iPinNum++;
	}
}
#endif

#ifdef ANAIN_TEST
//  Ana In test code
	while(iExit != 0)
	{
		iPinNum = 1;
		iResult = anaIn(iPinNum);
		printf("Result of anaIn(%d) %d\n",iPinNum, iResult);
	}

//idea for console based ana test

		//printf("Continue 'y' or 'n':");
		//scanf("%c", &cContinue);
		//printf("\r\n");
		//printf("You entered: %c\n", cContinue);

		//if(cContinue == 'n' || 'N')
		//{
			//iExit = 0;
		//}
		//else if(cContinue == 'y' || 'Y' )
		//{
			//iPinNum = 0;
		//}
		//else
		//{
			//printf("Non-valid input\n");
		//}

#endif

	return 0;
}



