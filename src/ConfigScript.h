/*
 * ConfigScript.h
 *
 *  Created on: Sep 25, 2013
 *      Author:
 */

#ifndef CONFIGSCRIPT_H_
#define CONFIGSCRIPT_H_

#define OID_TYPE_zero 1
#define OID_TYPE_int8 2
#define OID_TYPE_uint8 3
#define OID_TYPE_int16 4
#define OID_TYPE_uint16 5
#define OID_TYPE_int32 6
#define OID_TYPE_uint32 7
#define OID_TYPE_int64 8
#define OID_TYPE_uint64 9
#define OID_TYPE_float 10
#define OID_TYPE_sfloat 11
#define OID_TYPE_double 12
#define OID_TYPE_string 13
#define OID_TYPE_bstring 14
#define OID_TYPE_barray 15
#define OID_TYPE_charstr 16
#define OID_TYPE_raw 17
#define OID_TYPE_struct 18
#define OID_TYPE_bstrarray 19
#define OID_TYPE_int16array 20
#define OID_TYPE_int32array 21
#define OID_TYPE_floatarray 22
#define OID_TYPE_doublearray 23
#define OID_TYPE_structarray 24

#define SPECIAL_VARIABLE_AUTHORIZEDSMARTTOOL 1
#define SPECIAL_VARIABLE_AUTHORIZEDSERIAL 2

#define VA 1
#define VA_BITFLAG 2
//FIXMENOW

typedef struct struct_value_association
{
	char * szName;
	int vatype;
	int type;
	int count;
	void * pvValueList; //Value List   a pointer to the type defined by int type
	char ** szNameList; //Name Array   a char ** is a pointer to a char array
	char ** szNameHash; //Name Hash    (void if no hash given) a char ** is a pointer to a char array
} td_value_association;

typedef struct struct_parameter_field
{
	int oidid;
	int fieldIndex;
	char * label;
	int oidtype;
	void * pDataStorage;
	td_value_association * pValueAssociation;
	int parameterNext; //index of next in hash chain only!
} td_parameter_field;

typedef struct struct_script_definition
{
	td_parameter_field * parameters;
	int * parameterHashArray;
	int parameterHashSize;
} td_script_definition;


void InitConfigScript(td_script_definition * ConfigScriptDefinition);
void SaveConfigScript(td_script_definition * ConfigScriptDefinition);
void LoadConfigScript(td_script_definition * ConfigScriptDefinition);

#endif /* CONFIGSCRIPT_H_ */
