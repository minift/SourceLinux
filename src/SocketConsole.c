//SocketConsole Library
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdarg.h>
#include "SocketConsole.h"
#include "hwio.h"
#include "SmartToolUtil.h"


//FIXME VHDJWRRECHECK PORT RECHECK
#define USE_OUTPUT

//Global Storage
int _ConsoleServerSock = 0;
int _ConsoleSock = 0;
int iRxLen = 0;
int iPos = 0;

unsigned long _ulConsoleSockLastRxTimeMs;

char _cConsoleSockCommandMode = 0;
char _cPrintfOutput = 1;
char _cConsoleOutput = 0;


////////////////////////////////////////////////////////////////////////////////
// SocketConsole
////////////////////////////////////////////////////////////////////////////////
void InitSocketConsole()
{
	//Current Nothing Extra To Init
}

void InitListenSocketConsole()
{
	logf("InitListenSocketConsole\r\n");
    struct sockaddr_in stSockAddr;
    _ConsoleServerSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

    if(-1 == _ConsoleServerSock)
    {
      perror("can not create socket");
      exit(EXIT_FAILURE);
    }

    memset(&stSockAddr, 0, sizeof(stSockAddr));
    stSockAddr.sin_family = AF_INET;
    stSockAddr.sin_port = htons(SC_LISTENPORT);
    stSockAddr.sin_addr.s_addr = htonl(INADDR_ANY);//inet_addr("192.168.0.156");        //

    int flags = fcntl(_ConsoleServerSock, F_GETFL, 0);
    if (flags < 0)
    {
        perror("error fcntl get failed");
        close(_ConsoleServerSock);
        exit(EXIT_FAILURE);
    }
    if ( fcntl(_ConsoleServerSock, F_SETFL, flags | O_NONBLOCK) < 0)
    {
        perror("error fcntl set failed");
        close(_ConsoleServerSock);
        exit(EXIT_FAILURE);
        //FIXME VHDJWRRECHECK PORT RECHECK
#warning "must not exit on failure here???"
    }

    if(-1 == bind(_ConsoleServerSock,(struct sockaddr *)&stSockAddr, sizeof(stSockAddr)))
    {
      perror("error bind failed");
      close(_ConsoleServerSock);
      exit(EXIT_FAILURE);
    }

    if(-1 == listen(_ConsoleServerSock, 10))
    {
      perror("error listen failed");
      close(_ConsoleServerSock);
      exit(EXIT_FAILURE);
    }

    //Reset timeout
	_ulConsoleSockLastRxTimeMs = MS_TIMER;
    //Clear variable
    _cConsoleSockCommandMode = 0;
    _cConsoleOutput = 0;
}

void CheckSocketConsole()
{
    if (_ConsoleSock!=0)
    {
    	//don't accept another connection yet
    	return;
    }

    _ConsoleSock = accept(_ConsoleServerSock, NULL, NULL);
    if ( _ConsoleSock < 0)
    {
#warning "don't make an error every time it can't accept... check error message though to see if a real error ocurred."
//    	logf("failed to accept socket:%d\r\n",_ConsoleSock);
    	_ConsoleSock=0;
    	return;
    }

    int flags = fcntl(_ConsoleSock, F_GETFL, 0);
    if (flags < 0)
    {
        perror("error fcntl get failed");
    	close(_ConsoleSock);
    	_ConsoleSock=0;
    	//still not connected
    	return;
    }
    if ( fcntl(_ConsoleSock, F_SETFL, flags | O_NONBLOCK) < 0)
    {
        perror("error fcntl set failed");
    	close(_ConsoleSock);
    	_ConsoleSock=0;
    	//still not connected
    	return;
    }

    logf("Socket Console Connection Accepted\r\n");

    //Reset timeout
	_ulConsoleSockLastRxTimeMs = MS_TIMER;
    //Clear variable
    _cConsoleSockCommandMode = 0;
    _cConsoleOutput = 1; //turn on output to this
    //Resent buffer
    iRxLen=0;
	iPos=0;
    return;
}

void RxSocketConsole()
{
    int inb;
    int irxgoal;
    int irx;
    int ipos;
    int icmdlen;
    int sock;
    char * s;
    char * scmd;
    char * sstop;
    char c;
    static char buffer[1024+4];

    sock=_ConsoleSock; //local reference to socket
    if (sock == 0)
    {
    	return; //nothing open
    }

    inb=iRxLen;//how many bytes are waiting in buffer
    ipos=iPos;
    if (ipos>(512+128))
    {
    	memcpy(buffer, buffer + ipos, inb - ipos);
    	inb-=ipos;
    	ipos=0;
    }

   	irxgoal=1024 - inb;

    if (irxgoal > 0)
    {
        //now read up to irxgoal more bytes into position starting at inb
        irx = read( sock, buffer + inb, irxgoal);
        if (irx < 0)
        {
			if (errno == EAGAIN)
			{
				//would block
				goto save_and_exit_rxsocketconsole;
			}
            // error reading packet
            #ifdef USE_OUTPUT
            logf("SC data read error\r\n");
            #endif
           	close(_ConsoleSock);
           	_ConsoleSock = 0;
            inb = 0;
            ipos = 0;
			goto save_and_exit_rxsocketconsole;
        }
        inb+=irx;
        _ulConsoleSockLastRxTimeMs=MS_TIMER; //update the last rx time
    }
	buffer[inb]=0;

	//For This version, fully process buffer here
	s=buffer + ipos;
	sstop = buffer + inb;
	while(1)
	{
		scmd = s;
		while (s < sstop)
		{
			c=*s;
			if (c=='\n' || c=='\r')
			{
				break;
			}
			s++;
		}
		icmdlen = s - scmd;
		if (s==sstop)
		{
			//never found \n or \r
			if (icmdlen == 0)
			{
				//nothing was here: reset buffer
				inb=0;
				ipos=0;
			}
			else
			{
				//store ipos as the start of the current command
				//return later to get the rest of the command
				ipos = (scmd - buffer);
				if (ipos==0 && inb>=1024)
				{
					//command is too long... It should be don't by now.
					//Do full clear
					logf("Command too long\r\n");
					inb=0;
					ipos=0;
				}
			}
			break;
		}
		*s=0; //null out the \n or \r
		s++; //move s to next part of buffer
		if (_cConsoleSockCommandMode == 0)
		{
			if (icmdlen==0)
			{
				//no need to print a \r\n
				goto continue_loop_rxsocketconsole;
			}
			if (icmdlen==1)
			{
				c=*scmd;
				if (c=='o')
				{
					_cConsoleOutput = !_cConsoleOutput;
					//toggle output
					goto continue_loop_rxsocketconsole;
				}
				if (c=='p')
				{
					_cPrintfOutput = !_cPrintfOutput;
					//toggle output
					goto continue_loop_rxsocketconsole;
				}
				if (c=='?')
				{
					SocketConsoleShowIdentity();
					goto continue_loop_rxsocketconsole;
				}
				if (c=='c')
				{
					//close socket (and stop processing commands)
					SocketConsoleClose();
					break;
				}
				//continue and pass it on to the custom command handling
			}
		} //end mode 0.. if not mode 0, pass to custom handler
		HandleSocketConsoleCommand(scmd, icmdlen);
		//continue through parse loop
		continue_loop_rxsocketconsole:
		#ifdef SOCKETCONSOLE_USE_PROMPT
		//print "" so that it will add prompt even in the cases that it printed nothing
		logf("");
		#endif
	}
save_and_exit_rxsocketconsole:
	iRxLen=inb; //save the number of bytes in the buffer;
	iPos=ipos;
//exit_rxsocketconsole:
    return;
}

void SocketConsoleClose()
{
	logf("Closing Socket...\r\n");
   	close(_ConsoleSock);
   	_ConsoleSock = 0;
}

void SocketConsoleWaitConnect(unsigned long ulWait)
{
	unsigned long ulTime;
	ulTime=MS_TIMER;
	while(MS_TIMER - ulTime < ulWait && _ConsoleSock==0)
	{
		//do nothing but watch for connection
		CheckSocketConsole();
	}
}

void SocketConsoleShutdown()
{
	logf("SocketConsoleShutdown\r\n");
	if (_ConsoleSock != 0)
	{
		close(_ConsoleSock);
		_ConsoleSock = 0;
	}

	if (_ConsoleServerSock != 0)
	{
		close(_ConsoleServerSock);
		_ConsoleServerSock = 0;
	}
}

//logf
//_logwf

//Use the standard _logf system right now
//#define USE_LOGF_ADVANCED

void _logwf(char *format, ...)
{
	va_list args;
	int count;
	int irx;
	char buffer[1024];
	char *pbuffer = buffer;
	if (_cConsoleOutput == 1 || _cPrintfOutput == 1)
    {
    	pbuffer = buffer;
		#ifdef SOCKETCONSOLE_USE_PROMPT
		//Prompt
    	*pbuffer++='\b';
		#endif
		va_start(args,format);
		vsprintf(pbuffer,format,args);
		va_end(args);
		count=strlen(buffer);
		buffer[count]=0;
        if (_cPrintfOutput == 1)
        {
        	//never use the prompt for this... so don't append prompt until after
        	fwrite(pbuffer,count-1,1,stdout);
        }
		#ifdef SOCKETCONSOLE_USE_PROMPT
		//Prompt
		buffer[count++]=PROMPT_CHAR;
		buffer[count]=0;
		#endif
		//Now send these bytes if possible
		if (_cConsoleOutput == 1)
        {

			irx = write(_ConsoleSock,buffer,count);
			if (irx < count)
			{
				if (irx < 0)
   	    	    {
					if (errno == EAGAIN)
					{
						//would block
					}
					else
					{
						// error reading packet
						#ifdef USE_OUTPUT
						logf("SC data write error\r\n");
						#endif
						close(_ConsoleSock);
						_ConsoleSock = 0;
					}
				}
				else
				{
					//wrote less than full or error
				}
			}
        }
    }
	return;
}






