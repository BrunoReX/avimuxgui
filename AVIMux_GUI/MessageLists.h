/*

  provides possibiliy to easily create a text based LOG 

*/

#ifndef I_MESSAGELISTS
#define I_MESSAGELISTS

//#include "FILE_INFO.h"
#include "stdafx.h"
//#include "audiosource.h"

typedef struct
{
	char*	lpMsg;
	void*	lpNext;
} MSG_LIST;



void MSG_LIST_clear(MSG_LIST* lpMsgList);
void MSG_LIST_append(MSG_LIST* lpMsgList,char* lpMsg);
void MSG_LIST_2_Msg(MSG_LIST* lpMsgList,char* lpMsg);


#endif
