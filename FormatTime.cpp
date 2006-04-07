#include "formattime.h"
#include "stdafx.h"

void Millisec2HMSF(__int64 qwMillisec,int* lpdwH,int* lpdwM,int* lpdwS,int* lpdwF)
{
	*lpdwH=(DWORD)(qwMillisec/3600000);
	*lpdwM=(DWORD)(qwMillisec%3600000/60000);
	*lpdwS=(DWORD)(qwMillisec%60000/1000);
	*lpdwF=(DWORD)(qwMillisec%1000);
}

void Millisec2Str(__int64 qwMillisec, char* lpcDest)
{
	int	dwH, dwM, dwS, dwF;

	Millisec2HMSF(qwMillisec,&dwH,&dwM,&dwS,&dwF);
	wsprintf(lpcDest,"%02d:%02d:%02d.%03d",dwH,dwM,dwS,dwF);
}

__int64 Str2Millisec(char* c) 
{
	__int64		iRes = 0;
	__int64		i = 0;
	int			iColons = 0;
	bool		bFrac = false;
	bool		bEnd = false;
	char		d;

	while ((d=*c++) && !bEnd) {
		if (d!=0x20) {
			if (d >= '0' && d <= '9') {
				i=10*i+(d-48);
			} else
			if (d == ':') {
				switch (iColons) {
					case 0: iRes += 3600000*i; break;
					case 1: iRes += 60000*i; break;
					case 2: iRes += 1000*i; break;
				}
				i=0;
				iColons++;
			} else
			if (d == '.') {
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
			}
		}
	}
	if (!iColons) iRes = i*60000; 
	if (iColons == 1) iRes += 60000*i;
	if (bFrac) iRes+=i; else if (iColons == 2) iRes+=1000*i;
	if (iColons>2) return -1;

	return iRes;
}