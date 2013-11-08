// StrongLinkSL031Serial.h

#ifndef STRONGLINKSL031SERIAL_H_
#define STRONGLINKSL031SERIAL_H_

/////////////////////////////////////////////////
// MPublic Defines
/////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// Change SL031 serial communications params here
//////////////////////////////////////////////////////////////////////////
#define SL031_LENGTH_BYTE_INDEX            1
#define SL031_DATA_START_INDEX							21
#define SL031_ERROR_CODE_START_INDEX					SL031_DATA_START_INDEX + 4
#define SL031_ERROR_CODE_SIZE
#define SL031_SIZE_OF_TAG_DATA 128
//	NOTE: String length of Tag data must not exceed input buffer size, currently set to 255 bytes.
#define SL031_SIZE_OF_LOGIN_BUFFER   32
#define SL031_SIZE_OF_SND_BUFFER   254
#define SL031_SIZE_OF_RCV_BUFFER   254
#if SL031_SIZE_OF_SND_BUFFER > 254
#error "Currently, buffer has an error when larger than 254 bytes."
#endif
#if SL031_SIZE_OF_RCV_BUFFER > 254
#error "Currently, buffer has an error when larger than 254 bytes."
#endif
#define	STDOUTSTRINGBUFFERSIZE  1024

//	Default timeout for receiving a response to a RFID command is 4 seconds.
#define RFID_TIMEOUT_MS 4000

//	Mifare RFID returned status code Values:
#define RFID_STATUS_SUCCESS 0x00
#define RFID_STATUS_NO_TAG 0x01
#define RFID_STATUS_LOGIN_SUCCESS 0x02
#define RFID_STATUS_LOGIN_FAIL 0x03
#define RFID_STATUS_FAIL_READ	0x04
#define RFID_STATUS_FAIL_WRITE	0x05
#define RFID_STATUS_UNABLE_TO_READ_AFTER_WRITE 0x06
#define RFID_STATUS_ADD_OVERFLOW 0x08
#define RFID_STATUS_DOWNLOAD_KEY_FAIL 0x09
#define RFID_STATUS_NOT_AUTHENTICATE 0x0D
#define RFID_STATUS_NOT_A_VALUE_BLOCK 0x0E
#define RFID_STATUS_CHECKSUM_ERROR 0xF0
#define RFID_STATUS_COMMAND_CODE_ERROR 0xF1
//Library Values Only
#define RFID_STATUS_TIMEOUT 0xF4
//	Values above here should be temporary until results are received
#define RFID_STATUS_PENDING_STATUS_VALUE 0xF5
#define RFID_STATUS_PENDING_READ 0xF5
#define RFID_STATUS_PENDING_WRITE 0xF6

typedef enum
{
	SL031_GET_MSG_ERROR = -1,
	SL031_GET_MSG_DONE,
	SL031_GET_MSG_BUSY,
	SL031_GET_MSG_NODATA
} td_SL031GetMsgStatusCode;

typedef enum
{
	SL031_SEND_MSG_ERROR = -1,
	SL031_SEND_MSG_DONE,
	SL031_SEND_MSG_BUSY
} td_SL031MsgSendReturnCode;

//  Globals - Public global variables.

extern byte g_bRFIDCommandCode;
extern byte g_bRFIDStatusCode;
extern unsigned long g_ulRFIDStartTime;
extern int g_iLengthOfResponse;
extern int g_ibuflen;
extern int g_ibufpos;

//	RFID Result codes.
#define RFID_RESULT_CLEAR 0
#define RFID_RESULT_PENDING 1
#define RFID_RESULT_ERROR 2
#define RFID_RESULT_NO_TAG 3
#define RFID_RESULT_LOGIN_FAIL 4
#define RFID_RESULT_LOGIN_SUCCESS 5
#define RFID_RESULT_READ_FAIL 6
#define RFID_RESULT_READ_BAD_DATA 7
#define RFID_RESULT_READ_SUCCESS 8
#define RFID_RESULT_WRITE_FAIL 9
#define RFID_RESULT_WRITE_SUCCESS 10
#define RFID_RESULT_FAIL 11
#define RFID_RESULT_SUCCESS 12

//	See codes above for possible values.
extern byte g_bRFIDResult;

extern byte g_bRFIDTagData[SL031_SIZE_OF_TAG_DATA];
extern byte g_bRFIDTagDataLen;
extern byte g_bRFIDReceiveBuffer[SL031_SIZE_OF_RCV_BUFFER];
extern byte g_bSendBuffer[SL031_SIZE_OF_SND_BUFFER];
extern byte g_bRFIDNextCommand;
#define SL031_CMD_HEADER 0xBA
#define SL031_CMD_SELECT_CARD 0x01
#define SL031_CMD_LOGIN_SECTOR 0x02
#define SL031_CMD_READ 0x03
#define SL031_CMD_WRITE 0x04

///////////////////////////////////////////////////////////////////////////////
//  Function prototypes - Public functions

//	Call Init once at system start-up.
int SL031Init( void );

//	Send a command to the RFID reader.
td_SL031MsgSendReturnCode SL031MsgSend( byte *p_SL031Msg );

//	Receive and process responses from the RFID reader.
int SL031CommRcv( void );

//	Send specific command packets to the RFID reader.
int SL031SelectCard( void );
int SL031LoginSector( void );
int SL031ReadData( void );	//	Read tag data.
int SL031WriteData( char * p_cTagData, int len );	// Write new tag data.

#endif /* STRONGLINKSL031SERIAL_H_ */
