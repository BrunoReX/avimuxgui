#include "stdafx.h"
#include "FormatTime.h"
#include <stdio.h>

void Millisec2HMSF(__int64 qwMillisec,int* lpdwH,int* lpdwM,int* lpdwS,int* lpdwF)
{
	*lpdwH=(int)(qwMillisec/3600000);
	*lpdwM=(int)(qwMillisec%3600000/60000);
	*lpdwS=(int)(qwMillisec%60000/1000);
	*lpdwF=(int)(qwMillisec%1000);
}

/* the buffer requires 13 bytes */
void Millisec2Str(__int64 qwMillisec, char* lpcDest)
{
	int	dwH, dwM, dwS, dwF;

	Millisec2HMSF(qwMillisec,&dwH,&dwM,&dwS,&dwF);
	sprintf_s(lpcDest, 13, "%02d:%02d:%02d.%03d", dwH, dwM, dwS, dwF);
}

__int64 Str2Millisec(char* c) 
{
	__int64		iRes = 0;
	__int64		i = 0;
	int			iColons = 0;
	bool		bFrac = false;
	bool		bEnd = false;
	char		d;
	int			digit_count = 0;

	while ((d=*c++) && !bEnd) {
		if (d!=0x20) {
			if (d >= '0' && d <= '9') {
				i=10*i+(d-48);
				digit_count++;
			} else { 
				digit_count = 0;
			if (d == ':') {
				switch (iColons) {
					case 0: iRes += 3600000*i; break;
					case 1: iRes += 60000*i; break;
					case 2: iRes += 1000*i; break;
				}
				i=0;
				if (iColons == 2)
					bFrac = true;
				iColons++;
				
			} else
			if (d == '.' || d == ',') {
				bFrac = true;
				iRes += 1000*i;
				i=0;
			} else
			if (d == 'f') {
				iRes += i;
				iColons=2;
				i=0;
				bFrac=true;
				bEnd=true;
			} else
			if (d == 's') {
				iRes += i*1000;
				iColons=2;
				i=0;
				bFrac=true;
			} else
			if (d == 'm') {
				iRes += i*60000;
				iColons=2;
				i=0;
				bFrac=false;
			} else
			if (d == 'h') {
				iRes += 3600000*i;
				iColons=1;
				i=0;
				bFrac=false;
			}}
		}
	}
	if (!iColons) iRes = i*60000; 
	if (iColons == 1) iRes += 60000*i;
	if (bFrac) {
		while (digit_count < 3 && digit_count++)
			i*=10;
		while (digit_count > 3 && digit_count--)
			i/=10;

		iRes+=i;
	}
	else 
		if (iColons == 2) 
			iRes+=1000*i;
	if (iColons>3) return -1;

	return iRes;
}