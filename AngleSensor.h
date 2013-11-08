//Angle.h


#ifndef ANGLE_H_
#define ANGLE_H_



///////////////////////////////////////////////////////////////////////////////
//Public Functions
///////////////////////////////////////////////////////////////////////////////
float ReadADCAngleSensor( int chan, int errflagvalue);
void PrecalcAngleSensor( void );
void CalcAngleSensor( void );
void ReadADC( void );


//JLIME g_cAngleSensorDisplay is used in MiniFTMain.c
extern char g_cAngleSensorDisplay;

#endif /* ANGLE_H_ */
