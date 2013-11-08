//
// ConfigScript.c
//
//  Created on: Sep 25, 2013
//      Author:
//

//Pasing Code Quality Comment:
//This Library was created to support STP Config OID Assignments
//It was created using a code pattern which I have found useful, but is not as clean
//as a tokenized parsing system.  The goal was to complete this quickly.
//In The future this could be replaced by other code patterns, but this should be good enough
//to parse the values which it generates.

#include "ConfigScript.h"
#include "SocketConsole.h"
//FIXME: want to separate smart tool core from STP in terms of types vs. networking lib...
#include "STP.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "errno.h"

//Internal Function Predeclarations
int parseOIDNumericType(int oidtype, char * pc, void * pd, char **pcnext, int lineno, char * linestart);
int SearchAndUseValueAssociation(td_value_association * va, char * valname, void * pd, int lineno, char * linestart);

unsigned int lookupHashCode(char * label)
{
	char *pc = label;
	unsigned int hashcode = 0;
	while (*pc != 0)
	{
		char c = *pc++;

		hashcode = hashcode * ((unsigned int) c);
		hashcode += c;
	}
	return hashcode;
}

void InitConfigScript(td_script_definition * ConfigScriptDefinition)
{
	//create hash table which uses the existing array and structs to create a hash by identifier.
	int iparam = 1; //start at parameter 1... 0 is a null parameter always
	td_parameter_field * parameters = ConfigScriptDefinition->parameters;
	int hsize = ConfigScriptDefinition->parameterHashSize;
	int * parameterHashArray = ConfigScriptDefinition->parameterHashArray;

	int i = 0;
	while (i < hsize)
	{
		parameterHashArray[i++] = 0;
	}

	while (1)
	{
		td_parameter_field * parameter = &parameters[iparam];
		if (parameter->oidid == 0)
		{
			//a null entery means that the end of the list is reached
			break;
		}
		char * label = parameter->label;
		unsigned int hashcode = lookupHashCode(label);
		hashcode = hashcode % hsize;
		int last = parameterHashArray[hashcode];
		parameterHashArray[hashcode] = iparam; //the index of this param
		parameter->parameterNext = last; //could be 0 or another param with the same index;
		logf("param %s  %d ==> %d  last = %d \r\n", label, iparam, hashcode,
				last);
		iparam++;
	}

	return;
}

void SaveConfigScript(td_script_definition * ConfigScriptDefinition)
{
	FILE *fp;
	char * filename = "config.txt";
	fp = fopen(filename, "w");
	if (fp == 0)
	{
		logf("Failed to open \"%s\"\r\n", filename);
		return;
	}
//FIXME SEVERE INCOMPELTE File Scripting is INCOMPLETE
	fprintf(fp, "#%s\r\n", "FIXME"); // sth.GetSmartToolTypeIDTEXT());
	fprintf(fp, "#%s\r\n", "FIXME"); //sth.SmartToolSystemVersion);
	fprintf(fp, "#Serial number:%s", "FIXME"); // + sth.SmartToolSerialNumber);
	fprintf(fp, "#Generation Time %s\r\n", "FIXME"); // + DateTime.Now.ToString("yyyyMMdd HH:mm:ss"));

	logf("STARTING CONFIG SAVE\r\n");
	int iparam = 1; //start at parameter 1... 0 is a null parameter always
	td_parameter_field * parameters = ConfigScriptDefinition->parameters;
	while (1)
	{
		td_parameter_field * parameter = &parameters[iparam];
		if (parameter->oidid == 0)
		{
			//a null means that the end of the list is reached
			break;
		}
		char * label = parameter->label;
		int oidtype = parameter->oidtype;
		void * pDataStorage = parameter->pDataStorage;

		if (oidtype == OID_TYPE_zero)
		{
			fprintf(fp, "# %s  #no value writen for type %d \r\n", label, oidtype);
		}
		else if (oidtype == OID_TYPE_int8)
		{
			fprintf(fp, "%s=%d\r\n", label, (int) (*(char *) pDataStorage));
		}
		else if (oidtype == OID_TYPE_uint8)
		{
			fprintf(fp, "%s=%u\r\n", label,
					(unsigned int) (*(char *) pDataStorage));
		}
		else if (oidtype == OID_TYPE_int16)
		{
			fprintf(fp, "%s=%d\r\n", label, (int) (*(short *) pDataStorage));
		}
		else if (oidtype == OID_TYPE_uint16)
		{
			fprintf(fp, "%s=%u\r\n", label,
					(unsigned int) (*(unsigned short *) pDataStorage));
		}
		else if (oidtype == OID_TYPE_int32)
		{
			fprintf(fp, "%s=%d\r\n", label, (int) (*(int *) pDataStorage));
		}
		else if (oidtype == OID_TYPE_uint32)
		{
			fprintf(fp, "%s=%u\r\n", label,
					(unsigned int) (*(unsigned int *) pDataStorage));
		}
		else if (oidtype == OID_TYPE_int64)
		{
			fprintf(fp, "%s=%ld\r\n", label, (long) (*(long *) pDataStorage));
		}
		else if (oidtype == OID_TYPE_uint64)
		{
			fprintf(fp, "%s=%lu\r\n", label,
					(unsigned long) (*(unsigned long *) pDataStorage));
		}
		else if (oidtype == OID_TYPE_float || oidtype == OID_TYPE_sfloat)
		{
			fprintf(fp, "%s=%f\r\n", label, (float) (*(float*) pDataStorage));
		}
		else if (oidtype == OID_TYPE_double)
		{
			fprintf(fp, "%s=%f\r\n", label, (double) (*(double*) pDataStorage)); //notice that this uses %f too:  float is promoted to double and it uses double inside
		}
		else if (oidtype == OID_TYPE_string)
		{
			fprintf(fp, "%s=\"%s\"\r\n", label, (char *) pDataStorage);
//FIXME HIGH... make this safer....  use max len????? use limited check?????
		}
		else
		{
			fprintf(fp, "#ERROR: %s  no output support for type %d \r\n", label,
					oidtype);
			logf("#ERROR: %s  no output support for type %d \r\n", label,
					oidtype);
		}

//
//#define OID_TYPE_bstring=14;
//#define OID_TYPE_barray=15;
//#define OID_TYPE_charstr=16;
//#define OID_TYPE_raw=17;
//#define OID_TYPE_struct=18;
//#define OID_TYPE_bstrarray=19;
//#define OID_TYPE_int16array=20;
//#define OID_TYPE_int32array=21;
//#define OID_TYPE_floatarray=22;
//#define OID_TYPE_doublearray=23;
//#define OID_TYPE_structarray=24;

		iparam++;
	}
	fclose(fp);
}

#define MAX_LINE_LENGTH 2048

char * scanPastWhiteSpace2(char * pcstart)
{
	char * pc = pcstart;
	while (1)
	{
		if (*pc == ' ')
		{
			pc++;
			continue;
		}
		if (*pc == '\t')
		{
			pc++;
			continue;
		}
		break;
	}
	return pc;
}
char * scanPastWord(char * pcstart)
{
	char * pc = pcstart;
	while (1)
	{
		char c = *pc;
		if (c >= 'a' && c <= 'z')
		{
			pc++;
			continue;
		}
		if (c >= 'A' && c <= 'Z')
		{
			pc++;
			continue;
		}
		if (c >= '0' && c <= '9')
		{
			if (pc != pcstart) //don't recognize 1st char as 0-9
			{
				pc++;
				continue;
			}
		}
		if (c == '.')
		{
			//allowed for these vars
			pc++;
			continue;
		}
		if (c == '_')
		{
			//allowed for these vars
			pc++;
			continue;
		}
		break;
	}
	return pc;
}

char * scanQuote(char * s)
{
	while (1)
	{
		if (*s == '\"')
		{
			break;
		}
		if (*s == 0)
		{
			break;
		}
		s++;
	}
	return s;
}

void examineArray(char * label, char * array, int iStart, int iEnd)
{
	int i = iStart;
	logf("examineArray:%s:\r\n", label);
	while (i < iEnd)
	{
		logf("%d : %d  %c  \r\n", i, (int) array[i], array[i]);
		i++;
	}
}

#define PARSE_START 0
#define PARSE_EQUALS_NEXT 1
#define PARSE_VALUE_NEXT 2
#define PARSE_IN_QUOTES 3
#define PARSE_EOL 15

//0 start of pattern
//1 expecting equals sign
//2 expecting value
//3 in quotes
//4 expecting end of line

void LoadConfigScript(td_script_definition * ConfigScriptDefinition)
{

	FILE *fp;
	char * filename = "config2.txt";
	char buffer[MAX_LINE_LENGTH];
	char valname[MAX_LINE_LENGTH];
	char * pc;
	char * var;
	char * linestart;
	char c;
	int r;

	td_parameter_field * parameters = ConfigScriptDefinition->parameters;
	int hsize = ConfigScriptDefinition->parameterHashSize;
	int * parameterHashArray = ConfigScriptDefinition->parameterHashArray;
	td_parameter_field * parameter = NULL;

	logf("STARTING CONFIG LOAD\r\n");

	fp = fopen(filename, "r");
	if (fp == 0)
	{
		logf("Failed to open \"%s\"\r\n", filename);
		return;
	}

	int parseState = 0;

	int lineno = 0;
	while (1)
	{
		int remain = MAX_LINE_LENGTH;
		char * pszResult = fgets(buffer, remain, fp);
		lineno++;
		linestart = buffer;
		//logf("line %d:%s\r\n",lineno,buffer);

		if (pszResult == 0)
		{
			if (parseState == PARSE_START)
			{
				//acceptable for file to end
			}
			else
			{
				//still trying to find the remaining parts of a previous element
				logf("line %d:ERROR: Never Saw end of last value at the end of the file.\r\n", lineno);
			}
			break;
		}

		//Process this line
		//logf("PROCESS LINE %d %s\r\n", parseState, buffer);
		pc = buffer;
		if (parseState == PARSE_START)
		{
			//logf("@PS\r\n");
			pc = scanPastWhiteSpace2(pc);
			c = *pc;
			if (c == 0 || c == '\r' || c == '\n')
			{
				//blank line
				continue;
			}
			if (c == '#')
			{
				//comments start here to end of line...
				continue;
			}
			var = pc;
			pc = scanPastWord(pc);
			c = *pc; //save the value after
			*pc = 0; //null after the word
			logf("var %s\r\n", var);
			//Now identify this var
			if (*var == 0)
			{
				//looks like even the 1st characters were not acceptable as a word
				//Restore buffer
				*pc = c;
				logf("line %d:Invalid Variable Format:%s\r\n", lineno, linestart);
				continue;
			}

			unsigned int hashcode = lookupHashCode(var);
			hashcode = hashcode % hsize;
			int iparam = parameterHashArray[hashcode];
			parameter = NULL;
			while (iparam != 0)
			{
				parameter = &parameters[iparam];
//				logf("Checking %s vs %s\r\n", parameter->label, var);
				if (strcmp(parameter->label, var) == 0)
				{
					//match
					break;
				}
				iparam = parameter->parameterNext;
			}
			if (iparam == 0)
			{
				//not a known parameter, register error now, but continue parsing
				parameter = NULL;
				logf("line %d:Variable and/or Field not recognized:%s\r\n", lineno, linestart);
			}
			else
			{
//				logf("Match %s\r\n", parameter->label);
			}
			//Restore buffer
			*pc = c;
			parseState = PARSE_EQUALS_NEXT;
		}
		if (parseState == PARSE_EQUALS_NEXT)
		{
			//logf("@PEN\r\n");
			pc = scanPastWhiteSpace2(pc);
			c = *pc;
			if (c == '=')
			{
				parseState = PARSE_VALUE_NEXT;
				pc++;
			}
			else if (c == '#')
			{
				//comments to end of line...
				continue;
			}
			else if (c == 0 || c == '\r' || c == '\n')
			{
				//nothing more now...
				continue;
			}
			else
			{
				logf("line %d:Expected Equals Sign after variable:%s\r\n", lineno, linestart);
				parseState = PARSE_START;
				continue;
			}
		}
		if (parseState == PARSE_VALUE_NEXT)
		{
			//logf("@PVN\r\n");
			pc = scanPastWhiteSpace2(pc);
			char * val = pc;
			c = *pc;
			if (c == '#')
			{
				//comments to end of line...
				continue;
			}
			if (c == 0 || c == '\r' || c == '\n')
			{
				//nothing more now...
				continue;
			}
			if (parameter == NULL )
			{
				//already failed so don't bother parsing the value...
				//do continue assuming the next line will be a new value
				parseState = PARSE_START;
				continue;
			}
			pc = scanPastWord(pc);
			if (pc != val)
			{
				//looks like a word
				c = *pc;
				*pc = 0;
				if ((pc - val) > sizeof(valname))
				{
					val[sizeof(valname)] = 0;
					logf("line %d:word %s is longer than allowed size of %d characters.\r\n", lineno, val, linestart);
					parseState = PARSE_START;
					continue;
				}
				strcpy(valname, val);
				*pc = c;
				//
				td_value_association * va = parameter->pValueAssociation;
				if (va == 0)
				{
					logf("line %d:word %s unrecognized. No Value Association:%s\r\n", lineno, valname, linestart);
					parseState = PARSE_START;
					continue;
				}
				if (va->type != parameter->oidtype)
				{
					logf("line %d:value association %s has type %d, but parameter has type %d:%s\r\n", lineno, va->szName, va->type, parameter->oidtype,
							linestart);
					parseState = PARSE_START;
					continue;
				}
				r = SearchAndUseValueAssociation(va, valname, parameter->pDataStorage, lineno, linestart);
				if (r == 0)
				{
					parseState = PARSE_START;
					continue;
				}
			}
			else
			{
				int iErrLev; //0 no error  1 warning  2 error
				char * pcnext;
				int oidtype = parameter->oidtype;
				if (oidtype == OID_TYPE_zero)
				{
					logf("line %d:Error field type has no value\r\n");
				}
				else if (oidtype == OID_TYPE_int8 || oidtype == OID_TYPE_int16 || oidtype == OID_TYPE_int32 ||
						oidtype == OID_TYPE_uint8 || oidtype == OID_TYPE_uint16 || oidtype == OID_TYPE_uint32 ||
						oidtype == OID_TYPE_float || oidtype == OID_TYPE_sfloat || oidtype == OID_TYPE_double)
				{
					iErrLev = parseOIDNumericType(oidtype, pc, parameter->pDataStorage, &pcnext, lineno, linestart);
					if (iErrLev == 2)
					{
						//value found was not acceptable enough to bother continuing with this line
						parseState = PARSE_START;
						continue;
					}
					if (iErrLev == 1)
					{
						//can continue and use value, but value might not be exactly what they had in the file
						//FOR NOW ALL ERRORS WERE PUT TO LEVEL 2
					}
					pc = pcnext;
				}
				else if (oidtype == OID_TYPE_string)
				{
					c = *pc;
					if (c != '"')
					{
						//expected 1st quote (error)
						logf("line %d:quoted string expected:%s\r\n", lineno, linestart);
						parseState = PARSE_START;
						continue;
					}
					//start quote...   For now accept only single line quotes
					pc++;
					val = pc;
					pc = scanQuote(pc);
					c = *pc;
					if (c != '"')
					{
						//expected 2nd quote (error)
						logf("line %d:second quoted string expected:%s\r\n", lineno, linestart);
						parseState = PARSE_START;
						continue;
					}
					*pc = 0;
					if ((pc - val) > sizeof(valname))
					{
						val[sizeof(valname)] = 0;
						logf("line %d:word %s is longer than allowed size of %d characters.\r\n", lineno, val, linestart);
						parseState = PARSE_START;
						continue;
					}
					strcpy(valname, val);
					*pc = c;
					pc++;

					if (parameter->oidid == SPECIAL_VARIABLE_AUTHORIZEDSMARTTOOL)
					{
						char * s = (char*) parameter->pDataStorage;
						//instead of loading this value... use the memory for a check
						if (strcmp(valname, s) == 0)
						{
							//OK
							logf("AuthorizedSmartTool Confirmed. \"%s\"\r\n",s);
						}
						else
						{
							logf("line %d:AuthorizedSmartTool \"%s\" does not match this smart tool type of \"%s\"\r\n", lineno, valname, s);
							parseState = PARSE_START;
							continue;
						}
					}
					else if (parameter->oidid == SPECIAL_VARIABLE_AUTHORIZEDSERIAL)
					{
						char * s = (char*) parameter->pDataStorage;
						if (*s == 0)
						{
							//did not load a serial number yet.... therefore allow this
							strcpy(s, valname);
							logf("Loaded Serial Number \"%s\"\r\n", s);
						}
						else if (strcmp(valname, s) == 0)
						{
							//OK
							logf("Serial Matched\r\n");
						}
						else
						{
							logf("line %d:AuthorizedSerial \"%s\" does not match this smart tool serial number \"%s\"\r\n", lineno, valname, s);
							parseState = PARSE_START;
							continue;
						}
					}
					else
					{
						logf("line %d: word %s is longer than allowed size of %d characters.\r\n", lineno, val, linestart);
						logf("WARNING: no handling for string type at this time!!!!!!!!!!!!!!!!!!1\r\n");
					}
				}
				else
				{
					logf("WARNING: no handling for this field type at this time!!!!!!!!!!!!!!!!!!1\r\n");
				}
			}
			parseState = PARSE_EOL;
		}
		if (parseState == PARSE_EOL)
		{
			pc = scanPastWhiteSpace2(pc);
			c = *pc;
			if (c == '#')
			{
				//comments to end of line...
				parseState = PARSE_START;
				continue;
			}
			if (c == 0 || c == '\r' || c == '\n')
			{
				//nothing more now...
				parseState = PARSE_START;
				continue;
			}

			logf("line %d:ERROR More Text After Value, or Bad Value Format: %s\r\n", lineno, linestart);

			parseState = PARSE_START;
			continue;
		}
	}
	fclose(fp);

}

//Internal Functions

int parseOIDNumericType(int oidtype, char * pc, void * pd, char **pcnext, int lineno, char * linestart)
{
	int iErrLev = 0;
	char * pcn;
	char cannonicallooptestbuffer[64];

	if (oidtype == OID_TYPE_int8 || oidtype == OID_TYPE_int16 || oidtype == OID_TYPE_int32)
	{
		long lValue = strtol(pc, &pcn, 0);
		if (errno == ERANGE)
		{
			logf("line %d:ERROR Out of Range: %s\r\n", lineno, linestart);
			iErrLev = 2;
			return iErrLev;
		}
		if (pcn == pc)
		{
			logf("line %d:ERROR Value Format Error: %s\r\n", lineno, linestart);
			iErrLev = 2;
			return iErrLev;
		}
		if (pcn != 0)
		{
			pc = pcn;
			*pcnext = pc;
		}
		long lValueCast;
		if (oidtype == OID_TYPE_int8)
		{
			lValueCast = (*(int8 *) pd = (int8) lValue);
		}
		else if (oidtype == OID_TYPE_int16)
		{
			lValueCast = (*(int16 *) pd = (int16) lValue);
		}
		else if (oidtype == OID_TYPE_int32)
		{
			lValueCast = (*(int32 *) pd = (int32) lValue);
		}
		if (oidtype == OID_TYPE_int64)
		{
			lValueCast = (*(int64 *) pd = (int64) lValue);
		}
		if (lValueCast != lValue)
		{
			//casting error detected....
			logf("line %d:ERROR Value Out of Range for OID Type: %s\r\n", lineno, linestart);
			iErrLev = 2;
			return iErrLev;
		}
		else
		{
			logf("parsed value to %ld\r\n", lValue);
		}
	}
	else if (oidtype == OID_TYPE_uint8 || oidtype == OID_TYPE_uint16 || oidtype == OID_TYPE_uint32)
	{
		unsigned long ulValue = strtoul(pc, &pcn, 0);
		if (errno == ERANGE)
		{
			logf("line %d:ERROR Out of Range: %s\r\n", lineno, linestart);
			iErrLev = 2;
			return iErrLev;
		}
		if (pcn == pc)
		{
			logf("line %d:ERROR Value Format Error: %s\r\n", lineno, linestart);
			iErrLev = 2;
			return iErrLev;
		}
		if (pcn != 0)
		{
			pc = pcn;
			*pcnext = pc;
		}
		unsigned long ulValueCast;
		if (oidtype == OID_TYPE_uint8)
		{
			ulValueCast = (*(uint8 *) pd = (uint8) ulValue);
		}
		else if (oidtype == OID_TYPE_uint16)
		{
			ulValueCast = (*(uint16 *) pd = (uint16) ulValue);
		}
		else if (oidtype == OID_TYPE_uint32)
		{
			ulValueCast = (*(uint32 *) pd = (uint32) ulValue);
		}
		else if (oidtype == OID_TYPE_uint64)
		{
			ulValueCast = (*(uint64 *) pd = (uint64) ulValue);
		}
		if (ulValueCast != ulValue)
		{
			//casting error detected....
			logf("line %d:ERROR Value Out of Range for OID Type: %s\r\n", lineno, linestart);
			iErrLev = 2;
			return iErrLev;
		}
		else
		{
			logf("parsed value to %lu\r\n", ulValue);
		}
	}
	else if (oidtype == OID_TYPE_float || oidtype == OID_TYPE_sfloat || oidtype == OID_TYPE_double)
	{
		double d = 0;
		double dcast;
		d = strtod(pc, &pcn);
		if (errno == ERANGE)
		{
			logf("line %d:ERROR Out of Range: %s\r\n", lineno, linestart);
			iErrLev = 2;
			return iErrLev;
		}
		if (pcn == pc)
		{
			logf("line %d:ERROR Value Format Error: %s\r\n", lineno, linestart);
			iErrLev = 2;
			return iErrLev;
		}
		if (oidtype == OID_TYPE_float || oidtype == OID_TYPE_sfloat)
		{
			float f;
			dcast = (f = (float) d);
			*(float *) pd = f;
			sprintf(cannonicallooptestbuffer, "%f", f);
			int ilen = strlen(cannonicallooptestbuffer);
			if (memcmp(cannonicallooptestbuffer, pc, ilen) != 0)
			{
				//not the same
				logf("line %d:WARNING: Value does not appear to be in cannonical and returnable float format %f: %s\r\n", lineno, f, linestart);
				//NO ERROR iErrLev = 2;
				//return iErrLev;
			}
		}
		else if (oidtype == OID_TYPE_double)
		{
			dcast = (*(double *) pd = (double) d);
			sprintf(cannonicallooptestbuffer, "%f", d);
			int ilen = strlen(cannonicallooptestbuffer);
			if (memcmp(cannonicallooptestbuffer, pc, ilen) != 0)
			{
				//not the same
				logf("line %d:WARNING: Value does not appear to be in cannonical and returnable double format %f: %s\r\n", lineno, d, linestart);
				//NO ERROR iErrLev = 2;
				//return iErrLev;
			}
		}
		if (pcn != 0)
		{
			pc = pcn;
			*pcnext = pc;
		}
		if (dcast != d)
		{
			//casting error detected....
			logf("line %d:ERROR Value Casting change. Value may not have exact representation %f => %f: %s\r\n", lineno, d, dcast, linestart);
			iErrLev = 2;
			return iErrLev;
		}
		if (oidtype == OID_TYPE_float || oidtype == OID_TYPE_sfloat)
		{
			float f = (float) d;
			logf("parsed value to %f\r\n", f);
		}
		else if (oidtype == OID_TYPE_double)
		{
			logf("parsed value to %f\r\n", d);
		}
	}
	else
	{
		logf("line %d:ERROR OID Type was not handled by parsing function parseOIDNumericType: %s", lineno, linestart);
		iErrLev = 2;
		return iErrLev;
	}
	return iErrLev;
}

int SearchAndUseValueAssociation(td_value_association * va, char * valname, void * pd, int lineno, char * linestart)
{
	int vaindex = 0;
	int oidtype = va->type;
	while (vaindex < va->count)
	{
		char * valuename = va->szNameList[vaindex];
		if (valuename != 0)
		{
			int diff = strcmp(valuename, valname);
			void * pv = va->pvValueList;
			if (diff == 0)
			{
				if (oidtype == OID_TYPE_int8)
				{
					int8 v = ((int8 *) pv)[vaindex];
					logf("%s ==> %d\r\n", valuename, v);
					if (diff == 0)
					{
						*((int8 *) pd) = v;
						break;
					}
				}
				else if (oidtype == OID_TYPE_int16)
				{
					int16 v = ((int16 *) pv)[vaindex];
					logf("%s ==> %d\r\n", valuename, v);
					if (diff == 0)
					{
						*((int16 *) pd) = v;
						break;
					}
				}
				else if (oidtype == OID_TYPE_int32)
				{
					int32 v = ((int32 *) pv)[vaindex];
					logf("%s ==> %d\r\n", valuename, v);
					if (diff == 0)
					{
						*((int32 *) pd) = v;
						break;
					}
				}
				else if (oidtype == OID_TYPE_int64)
				{
					int64 v = ((int64 *) pv)[vaindex];
					logf("%s ==> %ld\r\n", valuename, v);
					if (diff == 0)
					{
						*((int64 *) pd) = v;
						break;
					}
				}
				else if (oidtype == OID_TYPE_uint8)
				{
					uint8 v = ((uint8 *) pv)[vaindex];
					logf("%s ==> %d\r\n", valuename, v);
					if (diff == 0)
					{
						*((uint8 *) pd) = v;
						break;
					}
				}
				else if (oidtype == OID_TYPE_uint16)
				{
					uint16 v = ((uint16 *) pv)[vaindex];
					logf("%s ==> %d\r\n", valuename, v);
					if (diff == 0)
					{
						*((uint16 *) pd) = v;
						break;
					}
				}
				else if (oidtype == OID_TYPE_uint32)
				{
					uint32 v = ((uint32 *) pv)[vaindex];
					logf("%s ==> %d\r\n", valuename, v);
					if (diff == 0)
					{
						*((uint32 *) pd) = v;
						break;
					}
				}
				else if (oidtype == OID_TYPE_uint64)
				{
					uint64 v = ((uint64 *) pv)[vaindex];
					logf("%s ==> %ld\r\n", valuename, v);
					if (diff == 0)
					{
						*((uint64 *) pd) = v;
						break;
					}
				}
				else if (oidtype == OID_TYPE_float || oidtype == OID_TYPE_sfloat)
				{
					float v = ((float *) pv)[vaindex];
					logf("%s ==> %f\r\n", valuename, v);
					if (diff == 0)
					{
						*((float *) pd) = v;
						break;
					}
				}
				else if (oidtype == OID_TYPE_double)
				{
					double v = ((double *) pv)[vaindex];
					logf("%s ==> %f\r\n", valuename, v);
					if (diff == 0)
					{
						*((double *) pd) = v;
						break;
					}
				}
				else
				{
					logf("line %d:value association %s type not supported by function:%s\r\n", lineno, va->szName, linestart);
					return 0;
				}
			}
		}
		vaindex++;
	}
	if (vaindex == va->count)
	{
		//reach the end
		logf("line %d:value association %s has no value \"%s\" :%s\r\n", lineno, va->szName, valname, linestart);
		return 0;
	}
	else
	{
		return 1;
	}
}
//FIXMENOW

