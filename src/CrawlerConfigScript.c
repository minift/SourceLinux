/*
 * CrawlerConfigScript.c
 *
 *  Created on: Sep 25, 2013
 *      Author:
 */

#include "ConfigScript.h"
#include "CrawlerConfigScript.h"
#include "SocketConsole.h"

//FIXMENOW
char g_CrawlerSmartTool[256] = "Crawler";
char g_CrawlerSerialNumber[256] = "\0";
short g_sValueX;
float g_fValueX;
float g_fValueXX;
float g_fValueXY;
#define OID_VALUEX 12
#define OID_STRUCTX 13

short VXValueAssociationValues[] = { 1, 2, 3 };
char * VCValueAssociationNames[] = { "ONE", "TWO", "THREE" };
td_value_association VXValueAssociation = { "VXValueAssociation", VA, OID_TYPE_int16, 3 ,(void *)VXValueAssociationValues,(char **)VCValueAssociationNames, 0 };

td_parameter_field CrawlerConfigParameters[] =
{
		{ 0, 0, 0, 0, 0, 0, 0 },
		{ SPECIAL_VARIABLE_AUTHORIZEDSMARTTOOL, 0, "AuthorizedSmartTool", OID_TYPE_string, g_CrawlerSmartTool, 0, 0 },
		{ SPECIAL_VARIABLE_AUTHORIZEDSERIAL, 0, "AuthorizedSerial", OID_TYPE_string, g_CrawlerSerialNumber, 0, 0 },
		{ OID_VALUEX, 0, "ValueX", OID_TYPE_int16, (void *) &g_sValueX, &VXValueAssociation, 0 },
		{ OID_STRUCTX, 1, "StructX.X", OID_TYPE_float, (void *) &g_fValueXX, 0, 0 },
		{ OID_STRUCTX, 2, "StructX.Y", OID_TYPE_float, (void *) &g_fValueXY, 0, 0 },
		{ 0, 0, 0, 0, 0, 0, 0 },
};

#define CRAWLERCONFIGSCRIPTHASHSIZE 211
int CrawlerConfigLabelHashArray[CRAWLERCONFIGSCRIPTHASHSIZE];

td_script_definition CrawlerConfigScriptDefinition;

void InitCrawlerConfigScript()
{
	CrawlerConfigScriptDefinition.parameters = CrawlerConfigParameters;
	CrawlerConfigScriptDefinition.parameterHashArray = CrawlerConfigLabelHashArray;
	CrawlerConfigScriptDefinition.parameterHashSize = CRAWLERCONFIGSCRIPTHASHSIZE;
	InitConfigScript(&CrawlerConfigScriptDefinition);
}

void LoadCrawlerConfigScript()
{
	LoadConfigScript(&CrawlerConfigScriptDefinition);
	logf("AFTER THE LOAD: g_sValueX=%d\r\n",g_sValueX);


	logf("StructX.X=%f\r\n",g_fValueXX);
	logf("StructX.Y=%f\r\n",g_fValueXY);

}

void SaveCrawlerConfigScript()
{
	SaveConfigScript(&CrawlerConfigScriptDefinition);
}
