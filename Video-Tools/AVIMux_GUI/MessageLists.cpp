#include "FILE_INFO.h"
#include "stdafx.h"
#include "MessageLists.h"


void MSG_LIST_clear(MSG_LIST* lpMsgList)
{
	if (lpMsgList)
	{
		free(lpMsgList->lpMsg);
		MSG_LIST_clear((MSG_LIST*)(lpMsgList->lpNext));
		delete lpMsgList;
	}
}

void MSG_LIST_append(MSG_LIST* lpMsgList,char* lpMsg)
{

	while (lpMsgList->lpNext)
	{
		lpMsgList=(MSG_LIST*)(lpMsgList->lpNext);
	}
	lpMsgList->lpNext=new MSG_LIST;
	lpMsgList->lpMsg=(char*)malloc(lstrlen(lpMsg)+1);
	lstrcpy(lpMsgList->lpMsg,lpMsg);
	lpMsgList=(MSG_LIST*)lpMsgList->lpNext;
	lpMsgList->lpNext=NULL;
	lpMsgList->lpMsg=NULL;
}

void MSG_LIST_2_Msg(MSG_LIST* lpMsgList,char* lpMsg)
{
	lpMsg[0]=0;
	while (lpMsgList->lpMsg)
	{
		if (lpMsg[0]) lstrcat(lpMsg,"\n");
		lstrcat(lpMsg,lpMsgList->lpMsg);
		lpMsgList=(MSG_LIST*)lpMsgList->lpNext;
	}

}