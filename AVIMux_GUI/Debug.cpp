#include "stdafx.h"
#include "debug.h"

#include "IncResource.h"
#include "math.h"

#define DEBUG_ON

__int64 QW_div(__int64 x, __int64 y, char* lpName)
{
	CString cStr;
	char	Buffer[100];
#ifdef DEBUG_ON
	if (!y)
	{
		cStr.LoadString(IDS_ERROR);
		wsprintf(Buffer,"Division by Zero: %c%c%s",13,10,lpName);
		MessageBox(NULL,Buffer,cStr.GetBuffer(255),MB_OK | MB_ICONERROR);
		return 0;
	}
#endif
	return x/y;
}

double d_div(double x, double y, char* lpName)
{
	CString cStr;
	char	Buffer[100];
#ifdef DEBUG_ON
	if (fabs(y)<0.001)
	{
		cStr.LoadString(IDS_ERROR);
		wsprintf(Buffer,"Division by Zero: %c%c%s",13,10,lpName);
		MessageBox(NULL,Buffer,cStr.GetBuffer(255),MB_OK | MB_ICONERROR);
		return 0;
	}
#endif
	return x/y;

}