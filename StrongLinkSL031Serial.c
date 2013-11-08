///////////////////////////////////////////////////////////////////////////////
//	StrongLinkSL031Serial.c
//
//	This module is responsible for communications to and from a Mifare model StrongLink SL031 RFID reader.
//
//	Revision history:
//		2013-10-21	TC		Original coding for port of executive functions to Angstrom Linux on a Beagle Bone platform.
//
///////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <sys/ioctl.h>

//	Application specific include files.

#include "SmartTool.h"
#include "SmartToolUtil.h"
#include "SocketConsole.h"
#include "StrongLinkSL031Serial.h"

//	Symbols used locally.
#define RFID_BAUDRATE   B115200

typedef struct termios td_tio;

//	Enable the following compile option when performing a unit test on this module.
//	Comment out this definition for inclusion in the full application.
//#define	SL031_SELFTEST	1

//	FIXME: The logf macro should be defined in a system level *.h file if NOT configured for SELFTEST.
//#ifdef SL031_SELFTEST
//	Send log data to the IDE STDIO window when doing stand-alone self-test.
//#define logf	printf
//#endif


///////////////////////////////////////////////////////////////////////////////
//  Symbols, defines, and macros used locally.

/////////////////////////////////////////////////
// ASCII Control Codes
/////////////////////////////////////////////////
//#define NULL			0x00

//	These commands stored in data form are not used directly, but see how the patterns are used below
//const byte SL031SelectCard[]=		{0xBA,0x02,0x01 };
//const byte SL031LoginSector0[]=		{0xBA,0x0A,0x02,0x00,0xAA,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
//const byte SL031LoginSector1[]=		{0xBA,0x0A,0x02,0x01,0xAA,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
//const byte SL031ReadBlock1[]=		{0xBA,0x03,0x03,0x01};
//const byte SL031WriteBlock1[]=		{0xBA,0x13,0x04,0x01,0x00,0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,0x99,0xAA,0xBB,0xCC,0xDD,0xEE,0xFF};
//const byte SL031ReadValue[]=		{0xBA,0x03,0x05,0x05};
//const byte SL031WriteValue[]=		{0xBA,0x07,0x06,0x04,0x00,0x00,0x00,0x01};
//const byte SL031IncrementValue[]=	{0xBA,0x07,0x08,0x04,0x00,0x00,0x00,0x20};
//const byte SL031DecrementValue[]=	{0xBA,0x07,0x09,0x04,0x03,0x00,0x00,0x00};
//const byte SL031CopyValue[]=		{0xBA,0x04,0x0A,0x04,0x05};
//const byte SL031ReadULPage5[]=		{0xBA,0x03,0x10,0x05};
//const byte SL031WriteULPage5[]=		{0xBA,0x07,0x11,0x05,0x11,0x22,0x33,0x44};
//const byte SL031CMD_Halt[]=		{0xBA,0x03,0x50,0x00};

//  The Mifare RFID reader will always be hard-wired to serial port tty05.
const char* sRfidDevice = "/dev/ttyO5";


///////////////////////////////////////////////////////////////////////////////
//  Globals
//  Scope is limited to this subsystem.
struct termios g_Tio;
int g_iRfidFd;

byte g_bRFIDCommandCode;
byte g_bRFIDStatusCode;
unsigned long g_ulRFIDStartTime;
int g_iLengthOfResponse;
int g_ibuflen;
int g_ibufpos;

byte g_bRFIDResult;
byte g_bRFIDTagData[SL031_SIZE_OF_TAG_DATA];
byte g_bRFIDTagDataLen;
byte g_bRFIDNextCommand;

byte g_bRfidRxBuffer[SL031_SIZE_OF_RCV_BUFFER];
byte g_bRfidTxBuffer[SL031_SIZE_OF_SND_BUFFER];

///////////////////////////////////////////////////////////////////////////////
//  Function prototypes - private functions.
///////////////////////////////////////////////////////////////////////////////
byte SL031ChecksumCalc( byte bByteArray[], int iLengthOfData );
int SL031PortOpen( void );
int SL031PortConfig( td_tio * pTio );
int SL031ReadCheck( void );
int SL031Read( int iBytes );
void SL031WriteDataWasPending( void );
char * SL031ResultStr( int iResultCode );

///////////////////////////////////////////////////////////////////////////////
//  Private functions.
///////////////////////////////////////////////////////////////////////////////

//******************************************************************************
int SL031PortOpen( void )
{
    //  Open a pipe to the Rfid serial port.
	logf( "%12d SL031PortOpen: Opening %s to RFID device.\r\n", MS_TIMER, sRfidDevice );
	g_iRfidFd = open( sRfidDevice, O_RDWR | O_NONBLOCK );
	if( g_iRfidFd < 0 )
	{
		logf( "%12d SL031PortOpen: Error = %d\r\n", MS_TIMER, g_iRfidFd );
		logf( "%12d SL031PortOpen: Unable to open port tty05 to RFID reader.\r\n", MS_TIMER );
	}
	else
		logf( "%12d SL031PortOpen: FD = %d\r\n", MS_TIMER, g_iRfidFd );

	return g_iRfidFd;
}

int SL031PortConfig( td_tio * pTio )
{
    int iResult;

    //  Clear the termio struct.
	//memset( pTio, 0, sizeof(struct termios) );
	bzero( pTio,  sizeof(struct termios) );
	//logf( "SL031PortConfig: tio struct set to zero.\n" );

	cfsetispeed( pTio, RFID_BAUDRATE );    // set baud rates
	cfsetospeed( pTio, RFID_BAUDRATE );
	cfmakeraw( pTio );
	g_ibufpos = 0;	//	The Rx buffer is empty.

	pTio->c_iflag = 0;
	pTio->c_oflag = 0;
	//	CAUTION: BAUDRATE MUST be included, OR other parameters must be masked over BAUDRATE, if set above.
	pTio->c_cflag |= RFID_BAUDRATE | CS8 | CREAD | CLOCAL;    // 8n1, see termios.h for more information
	pTio->c_lflag = 0; //non-canonical mode
	pTio->c_cc[VMIN] = 1;	//	Blocking read until 1 character received.
	pTio->c_cc[VTIME] = 0;	//	Use no timeout on the read operation.
	//pTio->c_cc[VEOF] = 4;	//	EOF character is enabled as binary 4. Used only for Canonical mode.

    tcflush( g_iRfidFd, TCIFLUSH );
	iResult = tcsetattr( g_iRfidFd, TCSANOW, pTio );
	logf( "%12d SL031PortConfig: tcsetattr returned %d. \r\n", MS_TIMER, iResult );

    return( iResult );
}

//******************************************************************************
byte SL031ChecksumCalc( byte bByteArray[], int iLengthOfData )
{
	int i;
	byte bChecksum;

	bChecksum = 0;
	for( i = 0; i < iLengthOfData; i++ )
	{
		bChecksum = bChecksum ^ bByteArray[i];
	}
	logf( "%12d SL031CsCalc: Checksum = 0x%02x.\r\n", MS_TIMER, bChecksum );

	return( bChecksum );
}

//******************************************************************************
//	Return the number of data bytes in the port FIFO waiting to be read. Do not read from the port.
//	This function is non-blocking. It returns immediately.
//	FIXME: Neither method in Termios documentation seemed to work, so this function is not currently used.
int SL031ReadCheck( void )
{
	int	iRxCnt;
	static int s_iRxCntPrev;

	//	Set port IO configuration for non-canonical, and check only.
	g_Tio.c_lflag = 0;		//	Echo off.
	g_Tio.c_cc[VTIME] = 0;	//	Turn off the timeout timer.
	g_Tio.c_cc[VMIN] = 0; 	//	Non-blocking read - returns number of bytes received.

	//	FIXME: A call to read with VTIME and VMIN both zero SHOULD return data count in the port FIFO, but not actually read data.
	//		In actuality, this did not appear to work. Zero data bytes returned a -1 error code.
	//		Restore use of this function if it can be fixed.
	//iRxCnt = read( g_iRfidFd, g_bRfidRxBuffer + g_ibuflen, SL031_SIZE_OF_RCV_BUFFER - g_ibuflen );
	//	Alt. approach for checking data count in Rx FIFO. This also did not work.
	//iRxCnt = fcntl( g_iRfidFd, F_SETFL, FNDELAY);	//	Experimental. Did not reliably return data count.
	iRxCnt = 20;	//	Just say there is data to let the data read logic proceed.
	if( s_iRxCntPrev != iRxCnt )
	{
		s_iRxCntPrev = iRxCnt;
		logf( "SL031ReadCheck: %d bytes available.\r\n", iRxCnt );
	}

	return( iRxCnt );
}

//******************************************************************************
//	Read the requested number of data bytes from the serial port into the input buffer.
//	CAUTION: This function is blocking if there is insufficient data.
//	Caller should verify that sufficient data has arrived by calling SL031ReadCheck. Then this call returns immediately.
int SL031Read( int iBytes )
{
	int	iRxCnt;

	//	Set port IO for non-canonical, and check only
	g_Tio.c_lflag = 0;		//	Echo off.
	g_Tio.c_cc[VTIME] = 0;	//	Turn off the timeout timer.
	g_Tio.c_cc[VMIN] = iBytes; 	//	Blocking read. Return only when desired byte count is present.

	//	Check max data length for safety.
	if( iBytes > ( SL031_SIZE_OF_RCV_BUFFER - g_ibuflen ))
	{
		iBytes = SL031_SIZE_OF_RCV_BUFFER - g_ibuflen;
		logf( "SL031Read: iBytes reduced to %d.\r\n", iBytes );
	}

	//iRxCnt = read( g_iRfidFd, &g_bRfidRxBuffer[iIdx], iBytes );
	iRxCnt = read( g_iRfidFd, g_bRfidRxBuffer + g_ibuflen, iBytes ); //use 1ms timeout... bytes being asked for are present!
	if( iRxCnt >0 )
		logf( "%12d SL031Read: %d bytes read.\r\n", MS_TIMER, iRxCnt );

	return( iRxCnt );
}

//******************************************************************************
//
void SL031WriteDataWasPending( void )
{
	g_bRFIDNextCommand = 0;

	SL031MsgSend( g_bRfidTxBuffer );

	g_bRFIDStatusCode = RFID_STATUS_PENDING_WRITE;
	g_ulRFIDStartTime = 0;	//MS_TIMER;	//	Mark the time when the command was sent.
	g_bRFIDResult = RFID_RESULT_PENDING;
}

//******************************************************************************
td_SL031MsgSendReturnCode SL031MsgSend( byte *p_SL031Msg )
{
	td_SL031MsgSendReturnCode iSL031SndMsgState;
	int iIdx, iNumBytesWritten;
	int iLengthOfMsg;
	char cData;

	// Send message to SL031.

	iLengthOfMsg = (int) p_SL031Msg[SL031_LENGTH_BYTE_INDEX] + 2;
	iNumBytesWritten = write( g_iRfidFd, p_SL031Msg, iLengthOfMsg );

	//	Debug log of data sent:
	logf( "SL031MsgSend:\r\n" );
	iIdx = 0;
	while( iIdx < iLengthOfMsg )
	{
		cData = p_SL031Msg[iIdx];
		logf( "  %d: 0x%02x\r\n", iIdx, cData );
		iIdx++;
	}

	if( iNumBytesWritten == iLengthOfMsg )
	{
		iSL031SndMsgState = SL031_SEND_MSG_DONE;
	}
	else
	{
		logf( "%12d SL031MsgSend: Error %d sending message to SL031.\r\n", MS_TIMER, iNumBytesWritten );
		iSL031SndMsgState = SL031_SEND_MSG_ERROR;
	}

	return( iSL031SndMsgState );
}



///////////////////////////////////////////////////////////////////////////////
//  Public functions:
///////////////////////////////////////////////////////////////////////////////


//	This entry point hides internal details of port initialization.
int SL031Init( void )
{
	int iResult;

	iResult = SL031PortOpen();
	if( iResult != 0 )
		iResult = SL031PortConfig( &g_Tio );

	return( iResult );
}

///////////////////////////////////////////////////////////////////////////////
//	SL031 Rx Message Handler.
///////////////////////////////////////////////////////////////////////////////

//	Translate the result code for a read or write operation to a string.
//	IMPORTANT: This function depends on the two enum sets (read AND write) being compatible.
//		If they are NOT compatible, separate functions will be needed for read and write results.
char * SL031ResultStr( int iResultCode )
{
	char * pStr;

	switch( iResultCode )
	{
	case SL031_GET_MSG_ERROR:
		pStr = "ERROR";
		break;
	case SL031_GET_MSG_DONE:
		pStr = "DONE";
		break;
	case SL031_GET_MSG_BUSY:
		pStr = "BUSY";
		break;
	case SL031_GET_MSG_NODATA:
		pStr = "NODATA";
		break;
	default:
		pStr = "UNKNOWN";
		break;
	}
	return( pStr );
}

//******************************************************************************
//  Read and process incoming data from the Mifare SL031 RFID reader.
int SL031CommRcv( void )
{
	int	iResult;
	int iRxCnt, iIdx, iCsIdx, iDataIdx, iDataLen;
	int	iPacketStartIdx;
	int iRemainingSpace;
	int ibuflen;
    char cData;			//	Temporary current data byte value.
	char cRxChecksum;	//	Value of the received checksum in the incoming data packet.
	char cChecksum;		//	Calculated checksum for the packet. 
	//static char norepeatchar1;
	//static char norepeatchar2;
	static int s_iErrorPrev;

	iResult = SL031_GET_MSG_DONE;	//	Assume that a full packet will be processed.

	ibuflen = g_ibuflen;	//	Keep a local working copy of the next-available-data-index pointer in the Rx buffer.

	/*	Check if there is incoming data to be read.
	 * 	//	FIXME: Neither approach tried seemed to work.
	 * 	//	Reactivate the following if an approach is found that returns accurate count of data bytes ready to be read.
	iIdx = 0;
	iRxCnt = SL031ReadCheck();
	//	If an error occurred on ReadCheck, ...
	if( iRxCnt < 0 )
	{	//	If this is a different error than the one previously reported, ...
		if( s_iErrorPrev != iRxCnt )
		{
			//	Remember and log the negative error code.
			s_iErrorPrev = iRxCnt;
			logf( "SL031ReadCheck: Error: iRxCnt = %d\r\n", iRxCnt );
		}
		//	Return the error code to the caller as the result code for this pass.
		iResult = iRxCnt;
		iRxCnt = 0;		//	For any error case, the actual data count is zero.
	}
	*/

	//	If there is data in the port FIFO waiting to be received, ...
	//else if( iRxCnt > 0 )
	{	//	Calculate remaining capacity in the Rx data buffer.
		iRemainingSpace = SL031_SIZE_OF_RCV_BUFFER - ibuflen;
		//	If data count received but not read exceeds buffer capacity, ...
		//if( iRxCnt > iRemainingSpace )
		{	//	Read only as much as will fit in the Rx buffer.
			iRxCnt = iRemainingSpace;
		}

		//	Read as much as we can of incoming packet data.
		//	Use the local private wrapper function on the read operation to ensure port configuration is set correctly.
		iRxCnt = SL031Read( iRxCnt );

		//	If an error occurred on the read, ...
		if( iRxCnt < 0 )
		{	//	If this is a different error than the one previously reported, ...
			if( s_iErrorPrev != iRxCnt )
			{
				//	Remember and log the negative error code.
				s_iErrorPrev = iRxCnt;
				logf( "SL031Read: Error: iRxCnt = %d\r\n", iRxCnt );
			}
			//	Return the error code to the caller as the result code for this pass.
			iResult = iRxCnt;
			iRxCnt = 0;		//	For any error case, the actual received data count is zero.
		}
		else if( iRxCnt > 0 )
		{
			//	Mark the new end of data index in the Rx Buffer.
			ibuflen += iRxCnt;
			logf( "%12d SL031Read: iRxCnt = %d\r\n", MS_TIMER, iRxCnt );
		}
		//	NOTE: Log nothing if there is no data. Otherwise a log cascade will result while waiting for data.

		//	If the input buffer is empty, return that status to the caller.
		//	NOTE: If a partial packet was read before, but no more data was available on this pass,
		//		that case does NOT return a NODATA status.	Give it more time for the rest of the data to arrive.
		if( ibuflen == 0 )
		{
			iResult = SL031_GET_MSG_NODATA;
		}
	}

	//	FIXME: Reactivate if a pre-read data count can be obtained in function ReadCheck.
	//else	//	Port status check showed no data to be read.
	//{
	//	//	If the receive buffer is empty, return that status to the caller.
	//	if( ibuflen == 0 )
	//		iResult = SL031_GET_MSG_NODATA;
	//}

	//	If no read error occurred, and there is anything is in the buffer, ...
	//	FIXME: Should we skip this if iResult from read returned a negative error value?
	//	NOTE: iResult could be zero (no new data read), but there could be residual data left from the last pass.
	if(( iResult >= 0 ) && ( ibuflen > 0 ))
	{
		if( iRxCnt > 0 )	//	If new data has been received, ... show the new buffer contents.
		{
			//	Log buffer contents from ibufpos forward (this is the newly received data).
			logf( "%12d SL031CommRcv: buffer:\r\n", MS_TIMER );
			iIdx = g_ibufpos;		//	Start at current parse pos...
			while( iIdx < ibuflen )
			{
				cData = g_bRfidRxBuffer[iIdx];
				if( cData > 0 )	//	Because of error showing so many zeros.... dont' show zeros
				{
					logf( "  %d: 0x%02x\r\n", iIdx, cData );
				}
				iIdx++;
			}
    	}	//	Endif at least one new data byte was received.

		//	Parse received data.
		iIdx = g_ibufpos;	//	Start at current parse pos...
		while( iIdx < ibuflen )
		{
			//	Look first for a response message type (0xBD). 		
			cData = g_bRfidRxBuffer[iIdx];
			if( cData == 189 )	//	Found a response preamble 0xBD
			{
				//	If nearing the end of the input data buffer, ...			
				if( iIdx > (SL031_SIZE_OF_RCV_BUFFER-32)) //254-32 = 222
				{
					//	Shift remaining unprocessed data back to the start of buffer.
					memcpy( g_bRfidRxBuffer, g_bRfidRxBuffer+iIdx, ibuflen-iIdx );
					ibuflen = ibuflen-iIdx;
					iIdx = 0;
					logf( "%12d SL031CommRcv: buffer shift.\r\n", MS_TIMER );
				}

				break;
			}
			iIdx++;
		}
		//	g_ibufpos = iIdx;

		//	Handle command if possible
		//	if(	iIdx != norepeatchar1 && ibuflen != norepeatchar2 )
		//	{
		//		logf("iIdx = %d ibuflen=%d\r\n", c, ibuflen );
		//		norepeatchar1 = iIdx;
		//		norepeatchar2 = ibuflen;
		//	}

		//	If at least two data bytes have arrived, we should have received a preamble and data length.
		if( iIdx < ibuflen-1 )
		{
			//	Extract and parse the incoming data.		
			cData = g_bRfidRxBuffer[iIdx];
			if( cData == 189 )	//	Found a response preamble 0xBD.
			{
				iPacketStartIdx = iIdx;	//	Remember where the packet started.

				//	Fetch the data length value from the packet.
				//	DataLength excludes preamble and datalength bytes, but does include the checksum.
				iIdx++;
				iDataLen = g_bRfidRxBuffer[iIdx]; //	Data length includes status, data, and checksum...
				//	Calculate the index of the checksum that will end this data packet.
				//	For example, if iIdx==16 here, and 3 is datalen, then 17 and 18 are data bytes, and 19 is index of the checksum.
   				iCsIdx = iIdx + iDataLen;
				//	If the location of the checksum is within received data, ...
				if( iCsIdx < ibuflen )	
				{
					//	We have received at least one full packet.
					logf( "%12d SL031: Can proceed.\r\n", MS_TIMER );
					iIdx++;
					g_bRFIDCommandCode = g_bRfidRxBuffer[iIdx]; //read and save the command byte for later
					g_bRFIDStatusCode = g_bRfidRxBuffer[iIdx+1]; //read and save the status byte for later
					iDataIdx = iIdx + 2;	//	Mark the index of the first data byte in the packet.

					//	Get the received checksum value from the received packet.
					cRxChecksum = g_bRfidRxBuffer[iCsIdx];
					//	Calculate the checksum for this packet to validate it.
					//	Do not include the checksum data byte in the checksum calculation.
					//logf( "PacketStartIdx = %d, bytes in checksum = %d.\r\n", iPacketStartIdx, iDataLen+1 );
					cChecksum = SL031ChecksumCalc( &g_bRfidRxBuffer[iPacketStartIdx], iDataLen+1 );

					//	Set the next-available-data-index pointer to the index of the first byte available after the checksum that terminated the received packet.
					//iIdx++;
					g_ibufpos = iCsIdx+1;

					//	Determine the StatusCode and ResultCode for this packet.
					//	If the received checksum and calculated checksum don't match, ...
					if( cRxChecksum != cChecksum )
					{	//	Packet is corrupted.
						logf( "%12d SL031: CHECKSUM FAILURE: Checksum = 0x%02x; Calculated = 0x%02x.\r\n", MS_TIMER, cRxChecksum, cChecksum );
						g_bRFIDCommandCode = 0;	//	Therefore the CommandCode is suspect.
						g_bRFIDStatusCode = RFID_STATUS_CHECKSUM_ERROR; //	Not the one from the tag, but an error on RX
						g_bRFIDResult = RFID_RESULT_ERROR;
					}
					//	Otherwise, the packet appears valid.
					//	If the RFID reader is not seeing a tag, ...
					else if( g_bRFIDStatusCode == RFID_STATUS_NO_TAG )
					{	//	Keep the received command and status codes intact.
						g_bRFIDResult = RFID_RESULT_NO_TAG;
						logf("%12d SL031: No tag.\r\n", MS_TIMER );
					}

					//	Reading an RFID tag is a 2-step process.
					//	First check for a login code.					
					else if( g_bRFIDCommandCode == SL031_CMD_LOGIN_SECTOR )
					{
						//	Don't set the final result state, because this is always followed with another operation
						//	in our current scheme.  The second operation will end up causing the success or fail.
						if( g_bRFIDStatusCode == RFID_STATUS_LOGIN_SUCCESS )
						{
							g_bRFIDResult = RFID_RESULT_LOGIN_SUCCESS;
							if( g_bRFIDNextCommand == SL031_CMD_READ )
							{
								//	Read the RFID tag now.
								SL031ReadData();
							}
							else if( g_bRFIDNextCommand == SL031_CMD_WRITE )
							{
								// 	Continue write now.
								SL031WriteDataWasPending();
							}

							logf( "%12d SL031: Login success.\r\n", MS_TIMER );
						}
						else	//	Anything else is a failure.
						{
							g_bRFIDResult = RFID_RESULT_LOGIN_FAIL;
							logf( "%12d SL031: Login fail.\r\n", MS_TIMER );
							g_bRFIDNextCommand = 0;	//	Clear the next Cmd
						}
					}	//	Endif a login command.

					//	If a read command, ...
					else if( g_bRFIDCommandCode == SL031_CMD_READ )
					{	//	Check if successful.
						if( g_bRFIDStatusCode == RFID_STATUS_SUCCESS )
						{	//	YES! Extract the tag data.
							g_bRFIDTagDataLen = iDataLen;
							memcpy( g_bRFIDTagData, g_bRfidRxBuffer + iDataIdx, iDataLen );
							//	Log the tag data received.
							logf( "SL031: TAG:\r\n" );
							iIdx = 0;
							while( iIdx < iDataLen )
							{
								cData = g_bRFIDTagData[iIdx];
								logf( "  %d: 0x%02x\r\n", iIdx, cData );
								iIdx++;
							}
							g_bRFIDResult = RFID_RESULT_READ_SUCCESS;
						}
						else
						{
							g_bRFIDResult = RFID_RESULT_READ_FAIL;
						}
                    }

					//	If a write command, ...
					else if( g_bRFIDCommandCode == SL031_CMD_WRITE )
					{	//	Check if successful.
						if( g_bRFIDStatusCode == RFID_STATUS_SUCCESS )
						{
							g_bRFIDResult = RFID_RESULT_WRITE_SUCCESS;
							logf( "%12d SL031: Write success.\r\n", MS_TIMER );
						}
						else
						{
							g_bRFIDResult = RFID_RESULT_WRITE_FAIL;
						}
					}

					else	//	Status code received. Check for success.
					{	//	Log status code received.
						logf( "%12d SL031: StatusCode = %d\r\n", MS_TIMER, (int) g_bRFIDStatusCode );
						if (g_bRFIDStatusCode==RFID_STATUS_SUCCESS)
						{
							g_bRFIDResult = RFID_RESULT_SUCCESS;
						}
						else
						{
							g_bRFIDResult = RFID_RESULT_FAIL;
						}
					}
				}
				else	//	Checksum has not yet been received. 
				{
					//	We do not yet have enough data bytes for a full packet.
					//	Parse no further for now.
					iResult = SL031_GET_MSG_BUSY;
				}
			}	//	Endif start of a new packet has been found.
		}	//	Endif there is data in the input buffer. 

		//	If we have successfully parsed to the end of data, ...
		if( g_ibufpos == ibuflen )
		{
			//	We can clear the input buffer.
			logf("%12d SL031: All received data processed.\r\n", MS_TIMER );
			//	Set data pointers to start of buffer.
			ibuflen = 0;
			g_ibufpos = 0;
		}
		else
			iResult = SL031_GET_MSG_BUSY;
	}
	g_ibuflen = ibuflen; 	//	Save the next-available-data-index index for the next pass.

	
	//	We may have sent a command to the RFID reader and are now waiting for a reply.
	//	Manage the wait for expected responses here.
	if( g_bRFIDStatusCode >= RFID_STATUS_PENDING_STATUS_VALUE )
	{
		//	FIXME: Make new timeout mechanism work.
		if(( MS_TIMER - g_ulRFIDStartTime ) > RFID_TIMEOUT_MS )
		{
			logf( "\n%12d SL031: Timeout.\r\n", MS_TIMER );
			g_bRFIDStatusCode = RFID_STATUS_TIMEOUT;
			g_bRFIDResult = RFID_RESULT_ERROR;
		}
		//
	}

	//	Return DONE if a full packet has been processed, NODATA or BUSY if not.
	return( iResult );
}

///////////////////////////////////////////////////////////////////////////////
//	Public function calls to send specific commands to the SL031 RFID reader.
///////////////////////////////////////////////////////////////////////////////

//******************************************************************************
int SL031SelectCard( void )
{
	int iIdx, iResult;
	iIdx = 0;
	g_bRfidTxBuffer[iIdx++] = SL031_CMD_HEADER;
	g_bRfidTxBuffer[iIdx++] = 0x02;	//	Data length
	g_bRfidTxBuffer[iIdx++] = 0x01;	//	Command: Select card
	g_bRfidTxBuffer[iIdx] = SL031ChecksumCalc( g_bRfidTxBuffer, iIdx );
	iResult = SL031MsgSend( g_bRfidTxBuffer );

	g_bRFIDStatusCode = RFID_STATUS_PENDING_STATUS_VALUE;
	g_ulRFIDStartTime = 0;	//MS_TIMER;		//	Mark the time when the command was sent.
	g_bRFIDResult = RFID_RESULT_PENDING;
	g_bRFIDNextCommand = 0;		//	Never have do pending after this command.

	return( iResult );
}

//******************************************************************************
int SL031LoginSector( void )
{
	int iIdx, iResult;
	iIdx=0;
	g_bRfidTxBuffer[iIdx++] = SL031_CMD_HEADER;
	g_bRfidTxBuffer[iIdx++] = 0x0A; //length
	g_bRfidTxBuffer[iIdx++] = SL031_CMD_LOGIN_SECTOR; //login sector
	g_bRfidTxBuffer[iIdx++] = 0x00; //sector 0
	g_bRfidTxBuffer[iIdx++] = 0xAA;
	g_bRfidTxBuffer[iIdx++] = 0xFF;
	g_bRfidTxBuffer[iIdx++] = 0xFF;
	g_bRfidTxBuffer[iIdx++] = 0xFF;
	g_bRfidTxBuffer[iIdx++] = 0xFF;
	g_bRfidTxBuffer[iIdx++] = 0xFF;
	g_bRfidTxBuffer[iIdx++] = 0xFF;
	g_bRfidTxBuffer[iIdx] = SL031ChecksumCalc( g_bRfidTxBuffer, iIdx );
	iResult = SL031MsgSend( g_bRfidTxBuffer );

	g_bRFIDStatusCode = RFID_STATUS_PENDING_STATUS_VALUE;
	g_ulRFIDStartTime = 0;	//MS_TIMER;	//	Mark the time when the command was sent..
	g_bRFIDResult = RFID_RESULT_PENDING;

	return( iResult );
}

//******************************************************************************
//	Read tag data.
int SL031ReadData()
{
	int	iResult;

	if( g_bRFIDResult == RFID_RESULT_PENDING )
	{
		//already waiting for something like login, so when that is over do this instead
		g_bRFIDNextCommand = SL031_CMD_READ;
		return( SL031_GET_MSG_BUSY );
	}
	g_bRFIDNextCommand = 0;

//FIXME RFID MED  IMPLEMENT READ MULTIPLE BLOCKS????   currently doing 16 byte format
	g_bRfidTxBuffer[0] = SL031_CMD_HEADER;
	g_bRfidTxBuffer[1] = 3; //length
	g_bRfidTxBuffer[2] = SL031_CMD_READ;
	g_bRfidTxBuffer[3] = 1; //set block
	g_bRfidTxBuffer[4] = SL031ChecksumCalc( g_bRfidTxBuffer, 4 );
	iResult = SL031MsgSend( g_bRfidTxBuffer );

	g_bRFIDStatusCode = RFID_STATUS_PENDING_READ;
	g_ulRFIDStartTime = 0;	//MS_TIMER;	//	Mark the time when the command was sent.
	g_bRFIDResult = RFID_RESULT_PENDING;

	return( iResult );
}

//******************************************************************************
//	Write tag data.
int SL031WriteData( char * p_cTagData, int len )
{
	int	iResult;

	iResult = 0;
	if (len>16) { len=16; } //TEMP SINCE WE WRITE ONE BLOCK

	g_bRfidTxBuffer[0] = SL031_CMD_HEADER;
	g_bRfidTxBuffer[1] = 19; //length
	g_bRfidTxBuffer[2] = SL031_CMD_WRITE;
	g_bRfidTxBuffer[3] = 1; //set block
	memcpy( g_bRfidTxBuffer+4, p_cTagData, 16 );
	g_bRfidTxBuffer[20] = SL031ChecksumCalc( g_bRfidTxBuffer, 20 );

	if( g_bRFIDResult == RFID_RESULT_PENDING )
	{
		//	already waiting for something like login, so when that is over do this instead
		g_bRFIDNextCommand = SL031_CMD_WRITE;
		return( SL031_SEND_MSG_BUSY );
	}
	g_bRFIDNextCommand = 0;

	iResult = SL031MsgSend( g_bRfidTxBuffer );

	g_bRFIDStatusCode = RFID_STATUS_PENDING_WRITE;
	g_ulRFIDStartTime = 0;	//MS_TIMER;	//	Mark the time when the command was sent.
	g_bRFIDResult = RFID_RESULT_PENDING;

	return( iResult );
}

//******************************************************************************
//	Enable the following for stand-alone self-test only.
#ifdef SL031_SELFTEST
int main( void )
{
	bool bDone;
	int	iResult, iPass, iResPrev;

	//	Initialize local status variables.
	iResult = -2;
	iPass = 0;
	bDone = FALSE;

	//	Open and configure the RFID serial port.
	SL031Init();

    /*  Send the card select command.
	iResult = SL031SelectCard();
	printf( "%12d SL031Test: SelectCard returned iResult = %d\r\n", MS_TIMER, iResult );
	iResult = -2;	//	Force a change on first attempt to read.
	*/

    /*  Send the login command.
	iResult = SL031LoginSector();
	printf( "%12d SL031Test: Login returned iResult = %d\r\n", MS_TIMER, iResult );
	iResult = -2;	//	Force a change on first attempt to read.
	*/

    //  Send the read tag command.
	iResult = SL031ReadData();
	//iResult = write( g_iRfidFd, "Hello MTM\r\n", 11 );
	//printf( "%12d SL031Test: ReadData returned iResult = %d\r\n", MS_TIMER, iResult );
	printf( "SL031Test: SL031 Send returned iResult = %d\r\n", iResult );
	iResult = -2;	//	Force a change on first attempt to read.
	//	close( g_iRfidFd );
    tcflush( g_iRfidFd, TCIFLUSH );

	bDone = FALSE;
	while( bDone == FALSE )
	{
		//  Read the response from the Rfid reader.
		iResPrev = iResult;
		//logf( "%12d SL031Test: Calling CommRcv.\r\n", MS_TIMER );
		iResult = SL031CommRcv();
		//logf( "%12d SL031Test: CommRcv returned %d\r\n", MS_TIMER, iResult );
		if( iResult == SL031_GET_MSG_DONE )
		{
			bDone = TRUE;
		}
		iPass++;

		if( iResPrev != iResult )
		{	//	Log the change in iResult with a time tag prefix.
			logf( "%12d SL031Test: CommRcv iResult = %d (%s).\r\n", MS_TIMER, iResult, SL031ResultStr(iResult) );
		}
	}

	//	FIXME: Exercise other public API calls too, especially tag read.
	logf( "Done. Passes = %d.\r\n", iPass );
	return 0;
}
#endif	//	SL031_SELFTEST
