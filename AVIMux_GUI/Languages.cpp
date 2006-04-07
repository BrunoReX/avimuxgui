#include "stdafx.h"
#include "languages.h"
#include "..\utf-8.h"
#include "stdio.h"
#include "textfiles.h"
#include "global.h"

LANGUAGE_DESCRIPTOR*	lpCurrLang;

void SetCurrentLanguage(LANGUAGE_DESCRIPTOR* lpNewLanguage)
{
	lpCurrLang=lpNewLanguage;
}

LANGUAGE_DESCRIPTOR* GetCurrentLanguage(void)
{
	return lpCurrLang;
}

char*	LoadString(DWORD dwID)
{
	int		iMin,iMax,iMid;
	LANGUAGE_DESCRIPTOR* lpLD=lpCurrLang;

	iMin=0;
	iMax=lpLD->dwEntries;
	char* c;

	while (iMax>iMin+1)
	{
		iMid=(iMin+iMax)>>1;
		if (lpLD->lpdwIndices[iMid]<dwID)
		{
			iMin=iMid;
		}
		else
		if (lpLD->lpdwIndices[iMid]>dwID)
		{
			iMax=iMid;
		}
		else
		{
			iMin=iMid;
			iMax=iMid;
		}
	}

	if (lpLD->lpdwIndices[iMin]==dwID)
	{
		return (lpLD->lplpStrings[iMin]);
	}
	else
	if (lpLD->lpdwIndices[iMax]==dwID)
	{
		return (lpLD->lplpStrings[iMax]);
	}
	else
	{
		c = (char*)calloc(1,20);
		sprintf(c, "<%d> not found", dwID);
		return (c);
	}
}

LANGUAGE_DESCRIPTOR* LoadLanguageFile(char* lpcName)
{
	char	cBuffer[1024];

	DWORD*	lpdwIndices,i,dwLen;
	char**	lplpStrings;
	bool	bBackslash;

	LANGUAGE_DESCRIPTOR*		lpLD;
	
	FILESTREAM* s = new FILESTREAM;
	s->Open(lpcName,STREAM_READ);
	CTEXTFILE* f = new CTEXTFILE;
	f->Open(STREAM_READ,s);
	f->SelectOutputFormat(CM_ANSI);

	ZeroMemory(cBuffer,sizeof(cBuffer));
	
	f->ReadLine(cBuffer);

	if (lstrcmp("[AVI-Mux GUI Language File]",cBuffer))
	{
		f->Close();
		delete f;
		s->Close();
		delete s;
		return NULL;
	}

	lpLD = new LANGUAGE_DESCRIPTOR;
	ZeroMemory(lpLD,sizeof(LANGUAGE_DESCRIPTOR));

	f->ReadLine(cBuffer);
	f->ReadLine(cBuffer);
	if (lstrcmp("NAME",cBuffer))
	{
		f->Close();
		delete f;
		s->Close();
		delete s;
		return NULL;
	}
		
	f->ReadLine(cBuffer);

	//lpLD->lpcName=(char*)malloc(lstrlen(cBuffer)+1);
	newz(char,1+strlen(cBuffer), lpLD->lpcName);

	lstrcpy(lpLD->lpcName,cBuffer);
	
	f->ReadLine(cBuffer);

	//lpdwIndices=(DWORD*)calloc(sizeof(DWORD),4096);
	//lplpStrings=(char**)calloc(sizeof(char*),4096);
	newz(DWORD, 4096, lpdwIndices);
	newz(char*, 4096, lplpStrings);

	while (f->ReadLine(cBuffer)>-1)
	{
		lpdwIndices[lpLD->dwEntries]=atoi(cBuffer);
		f->ReadLine(cBuffer);

		bBackslash=false;
		if (lstrlen(cBuffer)>=2)
		{
			//lplpStrings[lpLD->dwEntries]=(char*)malloc(lstrlen(cBuffer)+1);
			newz(char,1+lstrlen(cBuffer),lplpStrings[lpLD->dwEntries]);
			dwLen=lstrlen(cBuffer);
			ZeroMemory(lplpStrings[lpLD->dwEntries],dwLen+1);
			
			for (i=0;i<dwLen;i++)
			{
				if (!bBackslash)
				{
					if (cBuffer[i]==92)
					{
						bBackslash=true;
					}
					else
					{
						lplpStrings[lpLD->dwEntries][i]=cBuffer[i];
						bBackslash=false;
					}
				}
				else
				{
					if (cBuffer[i]='n') 
					{
						lplpStrings[lpLD->dwEntries][i-1]=13;
						lplpStrings[lpLD->dwEntries][i]=10;
					}
					bBackslash=false;
				}
			}

			lpLD->dwEntries++;
			f->ReadLine(cBuffer);
		}
	}

	//lpLD->lpdwIndices=(DWORD*)malloc(4*lpLD->dwEntries);
	newz(DWORD, lpLD->dwEntries, lpLD->lpdwIndices);
	memcpy(lpLD->lpdwIndices,lpdwIndices,4*lpLD->dwEntries);

	//lpLD->lplpStrings=(char**)malloc(4*lpLD->dwEntries);
	newz(char*, lpLD->dwEntries, lpLD->lplpStrings);
	memcpy(lpLD->lplpStrings,lplpStrings,4*lpLD->dwEntries);

	f->Close();
	delete f;
	s->Close();
	delete s;

	delete lplpStrings;
	lplpStrings = NULL;
	delete lpdwIndices;
	lpdwIndices = NULL;

	return lpLD;
}

void UnloadLanguageFile(LANGUAGE_DESCRIPTOR* lpLD)
{
	for (int i=0;i<(int)lpLD->dwEntries;delete lpLD->lplpStrings[i++]);
	delete lpLD->lpcName;
	delete lpLD->lpdwIndices;
	delete lpLD->lplpStrings;
	delete lpLD;
}
