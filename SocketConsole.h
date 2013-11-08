//SocketConsole Library
////////////////////////////////////////////////////////////////////////////////
// SocketConsole
// -Provides Basic Support for system access of a TCPIP socket
// -Implements a version of _logf that can be used by
////////////////////////////////////////////////////////////////////////////////

#ifndef SOCKETCONSOLE_H_
#define SOCKETCONSOLE_H_

#define SC_LISTENPORT 5523
#define SOCKETCONSOLE_USE_PROMPT
#define SOCKETCONSOLE_OUTPUT

//Functions
void InitSocketConsole();
void InitListenSocketConsole();
void CheckSocketConsole();
void RxSocketConsole();
void SocketConsoleClose();
void SocketConsoleWaitConnect(unsigned long ulWait);
void SocketConsoleShutdown();

void _logwf(char *format, ...);
void _logwfdebuglogwf(char *format, ...);

#define logf _logwf

//CallBack Function Supplied by Program Using Library
void HandleSocketConsoleCommand(char * scmd, int icmdlen);
void SocketConsoleShowIdentity();


#define PROMPT_CHAR '%'

extern char _cConsoleSockCommandMode;

////////////////////////////////////////////////////////////////////////////////
// SocketConsole
////////////////////////////////////////////////////////////////////////////////

#endif /* SOCKETCONSOLE_H_ */
