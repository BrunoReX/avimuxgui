#include "stdafx.h"
#include "OSVersion.h"

char cOSVersionString[64];
bool bWin2kplus = true;
bool bOSSupportsUnicode = true;
bool bDetermined = false;

void DetermineOSVersion()
{
	char* cwinver;

	OSVERSIONINFOEX	ovi;
	ovi.dwOSVersionInfoSize = sizeof(ovi);
	GetVersionEx((OSVERSIONINFO*)&ovi);

	switch (ovi.dwMajorVersion) {
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

	bDetermined = true;
	strcpy(cOSVersionString, cwinver);
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

bool GetOSVersionString(char* buf, int buf_len)
{
	if (!bDetermined)
		DetermineOSVersion();

	strncpy(buf, cOSVersionString, buf_len);
	buf[buf_len-1] = 0;

	return true;
}