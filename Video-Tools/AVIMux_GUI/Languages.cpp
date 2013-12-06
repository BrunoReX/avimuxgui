#include "stdafx.h"
#include "languages.h"
#include "..\utf-8.h"
#include "stdio.h"
#include "textfiles.h"
#include "global.h"
#include "../FileStream.h"



LANGUAGE_DESCRIPTOR*	lpCurrLang;

void SetCurrentLanguage(LANGUAGE_DESCRIPTOR* lpNewLanguage)
{
	lpCurrLang=lpNewLanguage;
}

LANGUAGE_DESCRIPTOR* GetCurrentLanguage(void)
{
	return lpCurrLang;
}

char*	LoadString(DWORD dwID, int charset)
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
		if (lpLD->lpdwIndices[iMid]>dwID) {
			iMax=iMid;
		} else {
			iMin=iMid;
			iMax=iMid;
		}
	}

	if (lpLD->lpdwIndices[iMin]==dwID)	{
		if (charset == LOADSTRING_ANSI)
			return (lpLD->lplpStrings[iMin]);
		else
			return (lpLD->lplpStringsUTF8[iMin]);
	}
	else
	if (lpLD->lpdwIndices[iMax]==dwID)
	{
		if (charset == LOADSTRING_ANSI)
			return (lpLD->lplpStrings[iMax]);
		else
			return (lpLD->lplpStringsUTF8[iMax]);
	} else {
		c = (char*)calloc(1,20);
		sprintf(c, "<%d> not found", dwID);
		return (c);
	}
}

LANGUAGE_DESCRIPTOR* LoadLanguageFile(char* lpcName)
{
	char* cBuffer = NULL;

	DWORD*	lpdwIndices,i,dwLen;
	char**	lplpStrings;
	char**  lplpStringsUTF8;
	bool	bBackslash;

	LANGUAGE_DESCRIPTOR*		lpLD;
	
	CFileStream* s = new CFileStream;
	s->Open(lpcName, STREAM_READ);
	CTextFile* f = new CTextFile;
	f->Open(STREAM_READ, s);
	f->SetOutputEncoding(CHARACTER_ENCODING_UTF8);
	
	f->ReadLine(&cBuffer);

	if (strcmp("[AVI-Mux GUI Language File]", cBuffer)) {
		free(cBuffer);
		f->Close();
		delete f;
		s->Close();
		delete s;
		return NULL;
	}

	free(cBuffer);
	lpLD = new LANGUAGE_DESCRIPTOR;
	ZeroMemory(lpLD,sizeof(LANGUAGE_DESCRIPTOR));

	f->ReadLine(&cBuffer);
	free(cBuffer);
	f->ReadLine(&cBuffer);
	if (lstrcmp("NAME", cBuffer)) {
		free(cBuffer);
		f->Close();
		delete f;
		s->Close();
		delete s;
		return NULL;
	}
	free(cBuffer);
	f->ReadLine(&cBuffer);

	newz(char,1+strlen(cBuffer), lpLD->lpcName);

	UTF82Str(cBuffer, lpLD->lpcName);
	free(cBuffer);
	f->ReadLine(&cBuffer);

	newz(DWORD, 4096, lpdwIndices);
	newz(char*, 4096, lplpStrings);
	newz(char*, 4096, lplpStringsUTF8);

	while (f->ReadLine(&cBuffer)>-1)
	{
		lpdwIndices[lpLD->dwEntries]=atoi(cBuffer);
		free(cBuffer);
		f->ReadLine(&cBuffer);

		bBackslash=false;
		if (lstrlen(cBuffer)>=2)
		{
			newz(char,1+lstrlen(cBuffer),lplpStringsUTF8[lpLD->dwEntries]);
			dwLen=lstrlen(cBuffer);
			
			for (i=0;i<dwLen;i++)
			{
				if (!bBackslash)
				{
					if (cBuffer[i]==92) {
						bBackslash=true;
					} else {
						lplpStringsUTF8[lpLD->dwEntries][i]=cBuffer[i];
						bBackslash=false;
					}
				} else {
					if (cBuffer[i]='n') {
						lplpStringsUTF8[lpLD->dwEntries][i-1]=13;
						lplpStringsUTF8[lpLD->dwEntries][i]=10;
					}
					bBackslash=false;
				}
			}

			UTF82Str(lplpStringsUTF8[lpLD->dwEntries], &lplpStrings[lpLD->dwEntries]);

			lpLD->dwEntries++;
			free(cBuffer);
			f->ReadLine(&cBuffer);
		}
		free(cBuffer);
	}
	free(cBuffer);

	newz(DWORD, lpLD->dwEntries, lpLD->lpdwIndices);
	memcpy(lpLD->lpdwIndices,lpdwIndices,4*lpLD->dwEntries);

	newz(char*, lpLD->dwEntries, lpLD->lplpStrings);
	newz(char*, lpLD->dwEntries, lpLD->lplpStringsUTF8);
	memcpy(lpLD->lplpStrings,lplpStrings,4*lpLD->dwEntries);
	memcpy(lpLD->lplpStringsUTF8,lplpStringsUTF8,4*lpLD->dwEntries);


	f->Close();
	delete f;
	s->Close();
	delete s;

	delete lplpStrings;
	lplpStrings = NULL;
	delete lplpStringsUTF8;
	lplpStrings = NULL;
	delete lpdwIndices;
	lpdwIndices = NULL;

	return lpLD;
}

void UnloadLanguageFile(LANGUAGE_DESCRIPTOR* lpLD)
{
	for (int i=0;i<(int)lpLD->dwEntries;i++) {
		delete lpLD->lplpStrings[i];
		delete lpLD->lplpStringsUTF8[i];
	}
	delete lpLD->lpcName;
	delete lpLD->lpdwIndices;
	delete lpLD->lplpStrings;
	delete lpLD->lplpStringsUTF8;
	delete lpLD;
}
