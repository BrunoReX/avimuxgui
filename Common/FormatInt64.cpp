#include "stdafx.h"
#include "FormatInt64.h"
#include "math.h"
#include <stdio.h>
#include <string>

static char* Byte_UnitSet[] = {
	"B", "kB", "MB", "GB", "TB" 
};

static char* Byte_UnitSet_Speed[] = {
	"B/s", "kB/s", "MB/s", "GB/s", "TB/s" 
};

static char* Bit_UnitSet_Speed[] = {
	"bps", "kbps", "Mbps", "Gbps", "Tbps"
};

template<class T>
T Min(T first, T second)
{
	if (first <= second)
		return first;
	return second;
}

template<class T>
T Max(T first, T second)
{
	if (first >= second)
		return first;
	return second;
}

/* requires a buffer of 16 bytes */
void FormatSize(char* d, unsigned __int64 qwSize, FormatSizeModes Mode, FormatSizeKSizes KSize)
{
	char** unit_set = NULL;
	if (Mode == BYTES)
		unit_set = Byte_UnitSet;
	else if (Mode == BITS_PER_SECOND)
		unit_set = Bit_UnitSet_Speed;
	else if (Mode == BYTES_PER_SECOND)
		unit_set = Byte_UnitSet_Speed;

	double div = 1024.;
	if (KSize == SIZE_1000)
		div = 1000.;

	double fSizeOfSource;
	char* cUnit;
	char Buffer[128];
	Buffer[0]=0;
	if (qwSize<1000) {
		cUnit = unit_set[0];
		fSizeOfSource = (static_cast<double>(qwSize)) / 1;
	} else if (qwSize>>10<1000) {
		cUnit = unit_set[1];
		fSizeOfSource = (static_cast<double>(qwSize)) / div;
	} else if (qwSize>>20<1000) {
		cUnit = unit_set[2];
		fSizeOfSource = (static_cast<double>(qwSize)) / div / div;
	} else if (qwSize>>30<1000) {
		cUnit=unit_set[3];
		fSizeOfSource = (static_cast<double>(qwSize)) / div / div / div;
	} else {
		cUnit=unit_set[4];
		fSizeOfSource = (static_cast<double>(qwSize)) / div / div / div / div;
	}

	unsigned int dwLen;
	dwLen = static_cast<unsigned int>(Min(log(fSizeOfSource)/log(10.),2.0));
	sprintf_s(Buffer, "%%%d.%df %%s", 2, 2-Max(0U,dwLen));
	sprintf_s(d, 16, Buffer, fSizeOfSource, cUnit);
}

std::string FormatSize(unsigned __int64 qwSize, FormatSizeModes Mode, FormatSizeKSizes KSize)
{
	char** unit_set = NULL;
	if (Mode == BYTES)
		unit_set = Byte_UnitSet;
	else if (Mode == BITS_PER_SECOND)
		unit_set = Bit_UnitSet_Speed;
	else if (Mode == BYTES_PER_SECOND)
		unit_set = Byte_UnitSet_Speed;

	double div = 1024.;
	if (KSize == SIZE_1000)
		div = 1000.;

	double fSizeOfSource;
	char* cUnit;
	char Buffer[128];
	Buffer[0]=0;
	if (qwSize<1000) {
		cUnit = unit_set[0];
		fSizeOfSource = (static_cast<double>(qwSize)) / 1;
	} else if (qwSize>>10<1000) {
		cUnit = unit_set[1];
		fSizeOfSource = (static_cast<double>(qwSize)) / div;
	} else if (qwSize>>20<1000) {
		cUnit = unit_set[2];
		fSizeOfSource = (static_cast<double>(qwSize)) / div / div;
	} else if (qwSize>>30<1000) {
		cUnit=unit_set[3];
		fSizeOfSource = (static_cast<double>(qwSize)) / div / div / div;
	} else {
		cUnit=unit_set[4];
		fSizeOfSource = (static_cast<double>(qwSize)) / div / div / div / div;
	}

	unsigned int dwLen;
	dwLen = static_cast<unsigned int>(Min(log(fSizeOfSource)/log(10.),2.0));
	sprintf_s(Buffer, "%%%d.%df %%s", 2, 2-Max(0U,dwLen));
	
	char d[16]; d[0]=0;
	sprintf_s(d, 16, Buffer, fSizeOfSource, cUnit);
	return d;
}


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

