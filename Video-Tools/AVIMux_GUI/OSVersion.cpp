#include "stdafx.h"
#include "OSVersion.h"
#include <string>

CUTF8 currentOsVersionString;

char cOSVersionString[64];
bool bWin2kplus = true;
bool bOSSupportsUnicode = true;
bool bDetermined = false;

std::basic_string<TCHAR> GetOperatingSystemByGetVersionEx()
{
	std::basic_string<TCHAR> result;

	OSVERSIONINFOEX	ovi;
	ovi.dwOSVersionInfoSize = sizeof(ovi);
	GetVersionEx((OSVERSIONINFO*)&ovi);

	switch (ovi.dwMajorVersion) {
		case 6: switch (ovi.dwMinorVersion) {
					case 0: result = _T("Windows Vista"); break;
					case 1: result = _T("WIndows 7"); break;
				}
		case 5: switch (ovi.dwMinorVersion) {
					case 0: result = _T("Windows 2000"); break;
					case 1: result = _T("Windows XP"); break;
					case 2: result = _T("Windows Server 2003"); break;
				} break;
		case 4: bWin2kplus = false;
			    switch (ovi.dwMinorVersion) {
				case 0: if (ovi.dwPlatformId == VER_PLATFORM_WIN32_NT) {
								result = _T("Windows NT 4"); 
							} else {
								result = _T("Windows 95");
							} break;
					case 10: result = _T("Windows 98"); 
							 break;
					case 90: result = _T("Windows ME"); 
							 break;

				}; break;
		case 3: result = _T("Windows NT 3.51"); 
			    bWin2kplus = false;
				break;
	}

	if (ovi.szCSDVersion[0]) {
		result.append(_T(" ("));
		result.append(ovi.szCSDVersion);
		result.append(_T(")"));
	}

//	currentOsVersionString = result;
	return result;
}

void DetermineOSVersion()
{
	currentOsVersionString = GetOperatingSystemByGetVersionEx();
/*	char* cwinver;

	OSVERSIONINFOEX	ovi;
	ovi.dwOSVersionInfoSize = sizeof(ovi);
	GetVersionEx((OSVERSIONINFO*)&ovi);

	switch (ovi.dwMajorVersion) {
		case 6: switch (ovi.dwMinorVersion) {
					case 0: cwinver = "Windows Vista"; break;
					case 1: cwinver = "Windows 7"; break;
				}
		case 5: switch (ovi.dwMinorVersion) {
					case 0: cwinver = "Windows 2000"; break;
					case 1: cwinver = "Windows XP"; break;
					case 2: cwinver = "Windows Server 2003"; break;
				} break;
		case 4: switch (ovi.dwMinorVersion) {
				case 0: if (ovi.dwPlatformId == VER_PLATFORM_WIN32_NT) {
								cwinver = "Windows NT 4"; bWin2kplus = false;
							} else {
								cwinver = "Windows 95"; bOSSupportsUnicode = false; bWin2kplus = false;
							} break;
					case 10: cwinver = "Windows 98"; 
						     bOSSupportsUnicode = false;
							 bWin2kplus = false;
							 break;
					case 90: cwinver = "Windows ME"; 
							 bOSSupportsUnicode = false;
							 bWin2kplus = false;
							 break;

				}; break;
		case 3: cwinver = "Windows NT 3.51"; bWin2kplus = false; break;
	}
*/
	bDetermined = true;
	//strcpy(cOSVersionString, cwinver);
}

bool DoesOSSupportUnicode()
{
	if (!bDetermined) 
		DetermineOSVersion();

	return bOSSupportsUnicode;
}

bool IsOSWin2kplus()
{
	if (!bDetermined)
		DetermineOSVersion();
	
	return bWin2kplus;
}

/*
bool GetOSVersionString(char* buf, int buf_len)
{
	if (!bDetermined)
		DetermineOSVersion();

	strncpy(buf, cOSVersionString, buf_len);
	buf[buf_len-1] = 0;

	return true;
}*/

bool GetOSVersionString(std::string& result)
{
	if (!bDetermined) 
		DetermineOSVersion();
	result = currentOsVersionString.Str();
	return true;
}

bool GetOSVersionString(std::wstring& result)
{
	if (!bDetermined) 
		DetermineOSVersion();
	result = currentOsVersionString.WStr();
	return true;
}

std::basic_string<TCHAR> GetOperatingSystemByAvailableAPIFunctions()
{
	HMODULE hKernel32 = LoadLibrary(_T("kernel32.dll"));
	bool isVista = !!GetProcAddress(hKernel32, "GetLocaleInfoEx");
	bool isXP = !!GetProcAddress(hKernel32, "GetNativeSystemInfo");
	bool is2000 = !!GetProcAddress(hKernel32, "ReplaceFile");
	FreeLibrary(hKernel32);

	if (isVista)
		return _T("Windows Vista");
	if (isXP)
		return _T("Windows XP");
	if (is2000)
		return _T("Windows 2000");
	return _T("unknown");
}