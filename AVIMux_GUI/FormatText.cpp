#include "stdafx.h"
#include "formattext.h"
#include "math.h"
#include "languages.h"
#include "splitpointsdlg.h"
#include "dynarray.h"
#include "strings.h"

void FormatSize(char* d,__int64 qwSize)
{
	float fSizeOfSource;
	DWORD dwSizeOfSource;
	CString cStr;
	char Buffer[100];

	if (qwSize>>10<999)
	{
		cStr=LoadString(STR_KBYTE);
		wsprintf(Buffer,"%d %s",(DWORD)(qwSize>>10),cStr);
		dwSizeOfSource = (DWORD)(qwSize>>10);
		fSizeOfSource = ((float)qwSize) / 1024;
	}
	else
	if (qwSize>>20<999)
	{
		cStr=LoadString(STR_MBYTE);
		wsprintf(Buffer,"%d %s",(DWORD)(qwSize>>20),cStr);
		dwSizeOfSource = (DWORD)(qwSize>>20);
		fSizeOfSource = (float)(qwSize) / 1024 / 1024;
	}
	else
	{
		cStr=LoadString(STR_GBYTE);
		wsprintf(Buffer,"%d %s",(DWORD)(qwSize>>30),cStr);
		dwSizeOfSource = (DWORD)(qwSize>>20);
		fSizeOfSource = ((float)qwSize) / 1024 / 1024 / 1024;
	}

	DWORD	dwLen;
	dwLen = (DWORD)(min(log(fSizeOfSource)/log(10),2));
	sprintf(Buffer,"%%%d.%df %%s",2,2-dwLen);
	sprintf(d,Buffer,fSizeOfSource,cStr);
}





__int64 SONChapStr2Millisec(char* c) 
{
	__int64		iRes = 0;
	__int64		i = 0;
	int			iColons = 0;
	char		d;

	while (d=*c++) {
		if (d!=0x20) {
			if (d >= '0' && d <= '9') {
				i=10*i+(d-48);
			} else
			if (d == ':') {
				if (iColons==2) {
					iRes *= 60000;
					iRes += 1000*i+atoi(c+1)*((atoi(c+1)>25)?33:40);
				} else {
					iRes *= 60;
					iRes += i;
				}
				i=0;
				iColons++;
			} else return STRF_ERROR;
		}
	}

	return (iColons==3)?iRes:-1;
}

int String2SplitPointDescriptor(char* s, SPLIT_POINT_DESCRIPTOR* pDesc)
{
	bool bEnd;

	pDesc->cOrgText = new CStringBuffer(s);

	char* c[2];
	c[0] = s;
	c[1] = s;
	while (*c[1] && *c[1]++ != '-');
	if (!*c[1]) bEnd = false; else bEnd = true;

	(*(c[1]-!!bEnd)) = 0;

	if (!pDesc->aChapBegin) pDesc->aChapBegin = new CDynIntArray;
	if (!pDesc->aChapEnd) pDesc->aChapEnd = new CDynIntArray;

	pDesc->iFlags = 0;

	CDynIntArray* a[2] = { pDesc->aChapBegin, pDesc->aChapEnd };
	a[0]->DeleteAll();
	a[1]->DeleteAll();
	__int64* i[2] = { &pDesc->iBegin, &pDesc->iEnd };

	for (int j=0;j<1+!!bEnd;j++) {
		char* cCurr;

		// set begin / end flag
		pDesc->iFlags |= (j)?SPD_END:SPD_BEGIN;
		
		cCurr = getword(&c[j]);
		if (!strcmp(cCurr,"chapter")) {
			cCurr = getword(&c[j]);
			pDesc->iFlags |= (j)?SPD_ECHAP:SPD_BCHAP;
			int  k=0;
			while (cCurr[k]) {
				bool bDone = true;
				while (cCurr[k]) {
					if (cCurr[k] != '.') k++; else {
						cCurr[k] = 0;
						bDone = false;
					}
				}
				if (!strcmp(cCurr, "*")) {
					a[j]->Insert(-2);
				} else {
					a[j]->Insert(atoi(cCurr)-1);
				}
				cCurr+=strlen(cCurr)+1*!bDone;
				k=0;
			}

			// use chapter as point
		} else 
		if (!strcmp(cCurr,"frame"))  {
			cCurr = getword(&c[j]);
			*(i[j]) = atoi(cCurr);
			pDesc->iFlags |= (j)?SPD_EFRAME:SPD_BFRAME;
		} else {
			__int64 ms = Str2Millisec(cCurr);
			if (ms!=-1) {
				*(i[j]) = Str2Millisec(cCurr) * 1000000;
			} else {
				return -1;
			}
		} 
	}

	return 1;
}

int Str2Resolution(char* c, RESOLUTION* r)
{
	int i,j;
	i = 0; j = 0;
	bool bAspect_Ratio = false;
	char s[2][20];
	ZeroMemory(s,sizeof(s));

	while (*c) {
		if (*c == 'x' || *c == 'X') {
			i = 1; j = 0;
		} else
		if (*c == ':') {
			bAspect_Ratio = true;
			i = 1;
			j = 0;
		} else
		if ((*c >= '0' && *c <= '9') || *c == '.' || *c == ',') {
			s[i][j++] = *c;
			
		} else {
			return STRF_ERROR;
		}
		*c++;
	}

	if (atof(s[0])>0.001 && atof(s[1])>0.001) {

		if (!bAspect_Ratio) {
			r->iWidth = atoi(s[0]);
			r->iHeight = atoi(s[1]);
		} else {
			if (atof(s[1]) < 0.0001) strcpy(s[1],"1");
			r->iWidth = (int)(r->iHeight*atof(s[0])/atof(s[1]));
		}
	}

	return (i == 1 && r->iHeight && r->iWidth)?STRF_OK:STRF_ERROR;
}