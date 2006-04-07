#include "stdafx.h"
#include "FormatInt64.h"


void QW2Str(__int64 qwX, char* lpDest,int dwLen)
{
	if (!lpDest)
		return;

	*lpDest = 0;

	char	cBuffer[64];
//	memset(cBuffer, 0, sizeof(cBuffer)); // should actually work without, but doesn't
	int		i=0;
	int		d=0;
	int		j;
	int		neg=0;
	if (qwX<0) {
		neg=1;
		qwX*=-1;
	}

	while (qwX||!i)
	{
		d++;
		cBuffer[i++]=(char)((qwX%10)+48);
		qwX/=10;
		if (qwX&&!(d%3)) cBuffer[i++]=',';
	}
	while ((int)dwLen>i)
	{
		cBuffer [i++]=32;
	}
	if (neg) lpDest[0]='-';
	for (j=0;j<i;j++)
	{
		lpDest[j+neg]=cBuffer[i-j-1];
	}
	lpDest[j+neg]=0;
}