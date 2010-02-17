#include "stdafx.h"
#include "OSSpecific.h"

void GetOSSpecific(OSSPECIFIC& osspecific);

HRESULT WINAPI doNothing(HWND hwnd, HDC hdc, RECT* pRect)
{
	return S_OK;
}

OSSPECIFIC& OSSPECIFIC::GetInstance() 
{
	static bool initialized = false;
	static OSSPECIFIC s_instance;

	if (!initialized) {
		GetOSSpecific(s_instance);
		initialized = true;
	}

	return s_instance;
}

OSSPECIFIC::OSSPECIFIC()
{
	hUxTheme = LoadLibrary(_T("UxTheme.dll"));
	hKernel32 = LoadLibrary(_T("kernel32.dll"));
	 
	memset(&Version, 0, sizeof(Version));
	memset(&OSVersionName, 0, sizeof(OSVersionName));
	SupportsThemes = false;
	CanDrawThemeParentBackground = false;
	DrawThemeParentBackground = NULL;
	GlobalMemoryStatusEx = NULL;
	GlobalMemoryStatus = NULL;
}

OSSPECIFIC::~OSSPECIFIC()
{
	if (hUxTheme)
		FreeLibrary(hUxTheme);
	if (hKernel32)
		FreeLibrary(hKernel32);
}

void GetOSSpecific(OSSPECIFIC& osspecific)
{
	osspecific.Version.dwOSVersionInfoSize = sizeof(osspecific.Version);
	GetVersionExW(&osspecific.Version);

	switch (osspecific.Version.dwMajorVersion)
	{
	case 4: // 95, 98, ME, NT 4
		break;
	case 5: // 2000, XP, Server 2003
		break;
	case 6: // Vista
		break;
	}

	osspecific.CanDrawThemeParentBackground = false;
	osspecific.SupportsThemes = false;

	if (!osspecific.hUxTheme) {
		osspecific.DrawThemeParentBackground = doNothing;
	} else {
		void* mth = GetProcAddress(osspecific.hUxTheme, "DrawThemeParentBackground");
		if (mth) {
			osspecific.DrawThemeParentBackground = (pfnDrawThemeParentBackground)mth;
			osspecific.CanDrawThemeParentBackground = true;
			osspecific.SupportsThemes = true;
		} else {
			osspecific.DrawThemeParentBackground = doNothing;
		}
	}

	osspecific.GlobalMemoryStatus = reinterpret_cast<pfnGlobalMemoryStatus>(
		GetProcAddress(osspecific.hKernel32, "GlobalMemoryStatus"));
	osspecific.GlobalMemoryStatusEx = reinterpret_cast<pfnGlobalMemoryStatusEx>(
		GetProcAddress(osspecific.hKernel32, "GlobalMemoryStatusEx"));
}