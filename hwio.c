//hwio.c
//IO Library 1.0
//DIO		John Lime, Will Wilder
//ANA		John Lime, Will Wilder
//I2CDIO	Will Wilder

#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>

#include "SmartTool.h"
#include "hwio.h"
#include "SocketConsole.h"
#ifdef EXT_IO_I2C
#ifdef __CYGWIN__
//NO I2C support.
#else
#include <linux/i2c-dev.h>
#endif
#include <sys/ioctl.h>
#include <fcntl.h>
#endif

//FIXME PORTMED  profile this vs caching the open file and doing repeated lseek.....
//seek(iFile, 0, SEEK_SET);

//Must verify that these operations are non-blocking...  I fear that they are blocking...

//AIN_PREFIX_DIR is the prefix directory for analog input functions specifying the file path
#define AIN_PREFIX_DIR "/sys/bus/iio/devices/iio:device0/in_voltage"
#define AIN_ENDSTRING "_raw"
#define DIG_IO_DIR "/sys/class/gpio"
#define MAXBUFFER 64
#define BADCHANNEL -1
#define IOFAILURE -2

//Internal Function
int i2cInit();
char* getPinStates();
int digOutExtended(int ch, int v); //for use with the extended io and used internally by digOut
int i2cClose();

//Init Function
void HWIOInit()
{
	i2cInit();
}

//Digital IO On The Cape
//Digital Inputs
//Digital Outputs

//Digital Inputs
//Channels 1 to 10

//Channel Map Going with out 1 to 10

const char * digInFileMap[] =
{
	(const char *)0, //place holder
	DIG_IO_DIR "/gpio8/value",		//1
	DIG_IO_DIR "/gpio9/value",		//2
	DIG_IO_DIR "/gpio10/value",		//3
	DIG_IO_DIR "/gpio87/value",		//4
	DIG_IO_DIR "/gpio86/value",		//5
	DIG_IO_DIR "/gpio32/value",		//6
	DIG_IO_DIR "/gpio36/value",		//7
	DIG_IO_DIR "/gpio62/value",		//8
	DIG_IO_DIR "/gpio22/value",		//9
	DIG_IO_DIR "/gpio27/value"		//10
};


int digIn(int ci)
{
	int iFile; //File Handle
	char cData;
	int iResult;
	const char * pcFilename = 0;
	if (ci >= 1 && ci < sizeof(digInFileMap))
	{
		pcFilename = digInFileMap[ci];
	}
	if(pcFilename == 0)
	{
		logf("Error - Channel number invalid\n");
		return BADCHANNEL;
	}

	iFile = open(pcFilename, O_RDONLY);

	if (iFile < 0)
	{
		//perror("gpio/value");
		logf("Error - File is less than 0\n");
		return IOFAILURE; //not a valid ready   FIXME PORTHIGH  how to handle serious errors like this???
	}

	read(iFile, &cData, 1);

	if (cData != '0')
	{
		iResult = 0;
	}
	else
	{
		iResult = 1;
	}

	close(iFile);
	return iResult;
}

//Digital Outputs
//Channels 1 to 8

const char * digOutFileMap[] =
{
		(const char *)0, //place holder
		DIG_IO_DIR "/gpio35/value",		//1
		DIG_IO_DIR "/gpio34/value",		//2
		DIG_IO_DIR "/gpio67/value",		//3
		DIG_IO_DIR "/gpio66/value",		//4
		DIG_IO_DIR "/gpio68/value",		//5
		DIG_IO_DIR "/gpio69/value",		//6
		DIG_IO_DIR "/gpio44/value",		//7
		DIG_IO_DIR "/gpio45/value",		//8
};

void digOut(int ch, int v)
{
	int iError;
	const char * pcFilename = 0;
	int iFile; //file handle

	if (v < 0 || v > 1)
	{
		logf("Error - value is out of range\n");
		return;
	}

	if(ch>=DO_EXT1_1)
	{
		digOutExtended(ch, v);
		return;
	}

	if (ch >= 1 && ch < sizeof(digOutFileMap))
	{
		pcFilename = digOutFileMap[ch];
	}
	if(pcFilename == 0)
	{
		logf("Error - Channel number invalid\n");
		return;
	}

	//print out file path if you need to
	//logf("File path is: %s\n", pcFilename);

	iFile = open(pcFilename, O_WRONLY);

	if (iFile < 0)
	{
		logf("Error - file is less than 0\n");
		//perror("gpio/set-value");
		return;
	}

	char * pstr;
	if (v == 0)
	{
		pstr = "0";
	}
	else
	{
		pstr = "1";
	}
	iError = write(iFile, pstr, 2);
	if (iError < 0)
	{
		logf("Error - Writing to the file.\r\n");
		//perror("gpio/set-value");
		return;
	}

	close(iFile);
}



//Analog IO On The Cape
//Analog Inputs
//Analog Outputs (NOT IMPLEMENTED)

//Analog Inputs
//Channels 1 to 7

int16 anaInChannel[] =
{
		0,	//place holder
		1,	//1
		0,	//2
		2,	//3
		3,	//4
		4,	//5
		5,	//6
		6,	//7
};

int anaIn(int ci)
{
    int iFile;
    char cFilenameBuffer[MAXBUFFER];
    char cDataBuffer[32];

    if (ci < 1 || ci > 7)
    {
		logf("Error - anaIn %d Channel number invalid\n", ci);
    	return BADCHANNEL;
    }
    ci=anaInChannel[ci];

	snprintf(cFilenameBuffer, sizeof(cFilenameBuffer), AIN_PREFIX_DIR "%d" AIN_ENDSTRING, ci);

	//print out file path if you need to
	//logf("File path is: %s", cBuffer);

    int iResult = 0;

    iFile = open(cFilenameBuffer, O_RDONLY);
    if(iFile < 0)
    {
    	logf("Open Failed %s.\r\n", cFilenameBuffer);
    	return iResult;
    }

    //seeking is not needed if the file is just being opened now
    //seek(iFile, 0, SEEK_SET);
    int nb=read(iFile, cDataBuffer, 62);
    //logf("got %d return value\r\n", nb);
    if (nb<0)
    {
    	close(iFile); //always must close the file when not caching, because otherwise we keep opening new file handles
    	return iResult;
    }
    cDataBuffer[nb]=0;
    //logf("data was \"%s\" \r\n",cDataBuffer);
    long int counts = strtol(cDataBuffer, 0, 10);

    close(iFile);
    return counts;
}

float anaCountsToVolts(int counts)
{
	//FIXME PORTHIGH  analog conversion....
//	if (counts < 0)
//	{
		return (float)counts;
//	}
//	float f=((float)(10*counts))/4096;
//	return f;

		//f = (flaot) (counts - 345) * (0.007094);
		//f = (float) Count / 1024 * 5.0;/
}

float anaInVolts(int ci)
{
	return anaCountsToVolts(anaIn(ci));
}


//////////////////////////////////////////////////////////////////////////////////////////////
// IO FOR EXTENDED I2C IO CHIP

#ifdef EXT_IO_I2C
#define DEVICE_ADDR 0x20

int i2c_Handle;

char pinStates[2];

int i2cInit()
{
	i2c_Handle = open("/dev/i2c-1", O_RDWR);
	if(i2c_Handle<0)
	{
		logf("%s","error opening /dev/i2c-*");
		return i2c_Handle;
	}
	logf("opened /dev*\n");

#ifdef __CYGWIN__
	//no support I2C_SLAVE
	int l_ioctl = 0;
#else
	int l_ioctl = ioctl(i2c_Handle, I2C_SLAVE, DEVICE_ADDR);

	if ( l_ioctl < 0)
	{
		logf("%s","i2cSetAddress error in myI2C::i2cSetAddress");

	}
#endif
	pinStates[0] = 0x00;
	pinStates[1] = 0x00;

	return l_ioctl;
}

char* getPinStates(){
	return pinStates;
}

int digOutExtended(int ch, int v)
{
	int bitNumber;
	byte testBit;
	if (ch<DO_EXT1_1)
	{
		return 0;
	}
	if (ch<=DO_EXT1_8) //ch in EXT1_1 to EXT1_8
	{
		//First Bank of EXT1
		bitNumber = ch - DO_EXT1_1;
		testBit = 1<<bitNumber;
		logf("bitNumber=%d testByte=%d\r\n", bitNumber, testBit);
		logf("ps0 %x\r\n",pinStates[0]);
		//set the bit high always 1st
		pinStates[0] |= testBit;
		if(v==1){
			//leave the bit high
		} else {
			//xor it low
			pinStates[0] ^= testBit;
		}
		logf("ps0 %x\r\n",pinStates[0]);
	}
	else if (ch<=DO_EXT1_16) //ch in EXT1_9 to EXT1_16
	{
		//First Bank of EXT1
		bitNumber = ch - DO_EXT1_9;
		testBit = 1<<bitNumber;
		logf("bitNumber=%d testByte=%d\r\n", bitNumber, testBit);
		logf("ps1 %x\r\n",pinStates[1]);
		//set the bit high always 1st
		pinStates[1] |= testBit;
		if(v==1){
			//leave the bit high
		} else{
			//xor it low
			pinStates[1] ^= testBit;
		}
		logf("ps1 %x\r\n",pinStates[1]);
	}
	else
	{
		return 0;
	}

	write(i2c_Handle,pinStates,sizeof(pinStates));

	return 1;
}




int i2cClose()
{
	return close(i2c_Handle);
}

#if 0
int main_i2c_sample_code(void)
{
	logf("attempting to open i2c\n");

	i2cInit();
	digOutExtended(0,1);

	digOutExtended(8,1);
	i2cClose();

	return 0;
}
#endif

#endif
