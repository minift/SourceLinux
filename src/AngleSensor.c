#include "hwio.h"
#include "SocketConsole.h"
#include "MiniFTSmartTool.h"
#include "SmartToolUtil.h"
#include "MiniFTIO.h"
#include "AngleSensor.h"

//JLIME added this so it would compile.
#define FAULTCODE_ADC_OVERFLOW -4096

float g_fRadiansToDegrees = 57.29577951308232; //(360/2PI)
char g_cClampSensorError;
float g_fClampSensorANegVolts;
float g_fClampSensorAPosVolts;
float g_fClampSensorBNegVolts;
float g_fClampSensorANeg;
float g_fClampSensorAPos;
float g_fClampSensorBNeg;
float g_fClampSensorAHeightToDegrees;
float g_fClampSensorBHeightToDegrees;
float g_fASensorAngle;
//JLIME Added this variable to get this to compile
float g_fBSensorAngle;
char g_cAngleSensorDisplay;
unsigned int g_uiAngleSensorTime;


float ReadADCAngleSensor( int chan, int errflagvalue)
{
	int iCounts;
	float fVolts;
	// Convert raw ADC counts on specified channel to volts
	iCounts = anaIn(chan);
	//	First run a sanity check on the ADC input value.
	if(( iCounts <= 0) || ( iCounts > 2047))
	{
		//	RCM4xx function anain can return one error code - ADC OVERFLOW (-4096).
		if( iCounts == FAULTCODE_ADC_OVERFLOW )
		{
			logf("%d ADC_OVERFLOW\r\n",chan);
		}
		else	//	Only remaining possiblity is ADC busy. This is temporary.
		{
			//logf("%d ADC_BUSY\r\n",chan);
			//For now just print these and set the error indicator...
		}
		g_cClampSensorError &= errflagvalue;
		return 0;
	}

	//	Convert counts to volts.
	fVolts = anaCountsToVolts(iCounts);
	return fVolts;
}

void PrecalcAngleSensor( void )
{
	//Calculate these variables only when needed since division is a slower operation
	g_fClampSensorAHeightToDegrees = g_fRadiansToDegrees / g_ConfigData.fAngleSensorABaseWidth;
}

void CalcAngleSensor( void )
{
	//Note On angle calculation
	//Exact
	//this gives the exact number
	//angle = atan( height / width );
	//Approx
	//this is less than 0.01 degree off for an angle of 4.57
	//this is less than 0.003 degree off for an angle of 3.14
	//this is less than 0.0017 degree off for an angle of 2.57
	//angle = height / width;
	// radians_to_degrees = see above;
	//angle in degrees = angle * radians_to_degrees
	//Combine the math like this:
	//angle in degrees = (height / width) * radians_to_degrees
	//angle in degrees = height * radians_to_degrees / width
	//angle in degrees = height * (radians_to_degrees / width)
	//Now Since radians to degrees is a constant, and width only is changed when OIDs change
	// precalculate this value after config load and/or when OIDs are set.
	//g_fRadiansToDegrees = see above;
	//g_fClampSensorAHeightToDegrees = g_fRadiansToDegrees / AWidth
	//g_fClampSensorBHeightToDegrees = g_fRadiansToDegrees / BWidth
	float ADiff;
	float BDiff;
	unsigned int ui;
	char c;
	char cNeedCheckA;
	char cNeedCheckB;
	float fA;
	float fB;
	unsigned long ul;
	g_cClampSensorError = 0;

	//JLIME changed this to use appropriate channels.ADC_CLAMP_SENSOR_ANEG  ADC_CLAMP_SENSOR_APOS
	g_fClampSensorANegVolts = ReadADCAngleSensor(AIN_DVRT_A,1);
	g_fClampSensorAPosVolts = ReadADCAngleSensor(AIN_DVRT_B,2);

	if (g_fClampSensorANegVolts < g_ConfigData.AngleSensorANegCalibration.fmin_valid ||
		g_fClampSensorANegVolts > g_ConfigData.AngleSensorANegCalibration.fmax_valid)
	{
		g_cClampSensorError &= 1;
	}
	if (g_fClampSensorAPosVolts < g_ConfigData.AngleSensorAPosCalibration.fmin_valid ||
		g_fClampSensorAPosVolts > g_ConfigData.AngleSensorAPosCalibration.fmax_valid)
	{
		g_cClampSensorError &= 2;
	}

	if (g_cClampSensorError == 0)
	{
		//Set the Heights and Angles
		g_fClampSensorANeg = (g_fClampSensorANegVolts - g_ConfigData.AngleSensorANegCalibration.flevel) * g_ConfigData.AngleSensorANegCalibration.finch_volts;
		g_fClampSensorAPos = (g_fClampSensorAPosVolts - g_ConfigData.AngleSensorAPosCalibration.flevel) * g_ConfigData.AngleSensorAPosCalibration.finch_volts;
		ADiff = (g_fClampSensorAPos - g_fClampSensorANeg); //When A Neg is depressed more than A Pos the overall angle is towards the negative sensor

		g_fASensorAngle = ADiff * g_fClampSensorAHeightToDegrees;

		if (g_cAngleSensorDisplay>=1)
		{
			if (g_cAngleSensorDisplay==1)
			{
				//mode 1 - ongoing angle display
				ui = (unsigned int)MS_TIMER;
				if (ui - g_uiAngleSensorTime > 100)
				{
					logf("a %.2f b %.2f\r\n", g_fASensorAngle, g_fBSensorAngle);
					g_uiAngleSensorTime = ui;
				}
			}
			else if (g_cAngleSensorDisplay==2)
			{
				//mode 2 - a one time display of all the values
				logf("an v=%.3f d=%.3f\r\n", g_fClampSensorANegVolts, g_fClampSensorANeg);
				logf("ap v=%.3f d=%.3f\r\n", g_fClampSensorAPosVolts, g_fClampSensorAPos);
				logf("a %.2f\r\n", g_fASensorAngle);
				g_cAngleSensorDisplay = 0; //go back to off afterwards
			}
			else if (g_cAngleSensorDisplay==3)
			{
				logf("an %f ap %f ad %f \r\n", g_fClampSensorANeg, g_fClampSensorAPos, ADiff);
				logf("r2d=%f aw=%f r2d/aw=%f  conf=%f\r\n", g_fRadiansToDegrees, g_ConfigData.fAngleSensorABaseWidth, g_fClampSensorAHeightToDegrees, (g_fRadiansToDegrees/g_ConfigData.fAngleSensorABaseWidth));
				logf("diff*r2d/aw=%f\r\n", g_fASensorAngle);
				g_cAngleSensorDisplay = 0; //go back to off afterwards
				#ifdef MCAWHISTLE
				MCAShowAngleSensor();
				#endif
				//FIXME PORTFATAL must also import the whistle relationship with angle from NAC    we didn't do taht yet.
			}
		}
	}
	else
	{
		logf("Err: Ang.Sensor %d\r\n",g_cClampSensorError);
	}
}
