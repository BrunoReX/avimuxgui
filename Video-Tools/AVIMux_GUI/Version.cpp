#include "stdafx.h"
#include "version.h"
#include "resource.h"

/*char* GetAMGVersionString(char* buffer, int buf_len)
{
	CString c;
	c.LoadString(IDS_VERSION_INFO);

	_snprintf(buffer, buf_len, c);
	buffer[buf_len-1] = 0;

	return buffer;	
}*/

#ifdef _UNICODE
std::wstring GetAMGVersionStringW()
{
	CString c;
	c.LoadString(IDS_VERSION_INFO);

	std::wstring result = c.GetBuffer(64);
	return result;	
}
#else
std::string GetAMGVersionStringA()
{
	CString c;
	c.LoadString(IDS_VERSION_INFO);

	std::string result = c.GetBuffer(64);
	return result;	
}
#endif

std::basic_string<TCHAR> GetAMGVersionDate()
{
	return CUTF8(__DATE__).TStr();
}

std::basic_string<TCHAR> GetAMGVersionTime()
{
	return CUTF8(__TIME__).TStr();
}

std::basic_string<TCHAR> ComposeVersionString()
{
	std::basic_ostringstream<TCHAR> sstrResult;
	sstrResult << _T("AVI-Mux GUI ") << GetAMGVersionString() << _T(", ") << GetAMGVersionDate() << GetAMGVersionTime();
/*	char ver[32];
	sprintf(buffer,"AVI-Mux GUI %s, %s  %s", GetAMGVersionString(ver, 32),
		__DATE__, __TIME__);*/
	return sstrResult.str();
}
