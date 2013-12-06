#include "stdafx.h"
#include "version.h"
#include "resource.h"

char* GetAMGVersionString(char* buffer, int buf_len)
{
	CString c;
	c.LoadString(IDS_VERSION_INFO);

	_snprintf(buffer, buf_len, c);
	buffer[buf_len-1] = 0;

	return buffer;	
}

char* GetAMGVersionDate()
{
	return __DATE__;
}

char* GetAMGVersionTime()
{
	return __TIME__;
}

void ComposeVersionString(char* buffer)
{
	char ver[32];
	sprintf(buffer,"AVI-Mux GUI %s, %s  %s", GetAMGVersionString(ver, 32),
		__DATE__, __TIME__);
}
