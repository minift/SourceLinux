//hwio.h

#ifndef HWIO_H_
#define HWIO_H_

//A common interface for Digital and Analog IO
//and Physical IO Definitions


//FIXME PORTMED must eventually control compile of the extended module with an option in a better way
#define EXT_IO_I2C



//HW BASED PHYSICAL IO MAPPING

//Digital Inputs

#define DI_BBB_1	1
#define DI_BBB_2	2
#define DI_BBB_3	3
#define DI_BBB_4	4
#define DI_BBB_5	5
#define DI_BBB_6	6
#define DI_BBB_7	7
#define DI_BBB_8	8
#define DI_BBB_9	9
#define DI_BBB_10	10
#define DI_BBB_11	11
#define DI_BBB_12	12

//Digital Outputs

#define DO_BBB_1	1
#define DO_BBB_2	2
#define DO_BBB_3	3
#define DO_BBB_4	4
#define DO_BBB_5	5
#define DO_BBB_6	6
#define DO_BBB_7	7
#define DO_BBB_8	8
#define DO_BBB_9	9

//The Second set on the remote io 1 starts higher
//The hw library uses 64 to 64+15 for the 1st set
#define DO_EXT1_1 	64
#define DO_EXT1_2 	65
#define DO_EXT1_3 	66
#define DO_EXT1_4 	67
#define DO_EXT1_5 	68
#define DO_EXT1_6 	69
#define DO_EXT1_7 	70
#define DO_EXT1_8 	71
#define DO_EXT1_9 	72
#define DO_EXT1_10 	73
#define DO_EXT1_11 	74
#define DO_EXT1_12 	75
#define DO_EXT1_13 	76
#define DO_EXT1_14 	77
#define DO_EXT1_15 	78
#define DO_EXT1_16 	79

//Analog Inputs
//Starts at 0

#define AIN_BBB_1	1
#define AIN_BBB_2	2
#define AIN_BBB_3	3
#define AIN_BBB_4	4
#define AIN_BBB_5	5
#define AIN_BBB_6	6
#define AIN_BBB_7	7

//IO Functions
void HWIOInit();
int digIn(int ci);
void digOut(int ch, int v);
int anaIn(int ci);
float anaCountsToVolts(int counts);
float anaInVolts(int ci);

#endif

