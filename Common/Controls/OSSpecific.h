#pragma once

#include "windows.h"
#include <string>

typedef HRESULT (WINAPI *pfnDrawThemeParentBackground)(HWND, HDC, RECT*);
typedef BOOL (WINAPI *pfnGlobalMemoryStatusEx)(MEMORYSTATUSEX*);
typedef BOOL (WINAPI* pfnGlobalMemoryStatus)(MEMORYSTATUS*);

class OSSPECIFIC
{
public:
	HMODULE hUxTheme;
	HMODULE hKernel32;

public:
	/* OS-Version */
	OSVERSIONINFOW Version;
	wchar_t OSVersionName[1024];

	/* Themes */
	bool SupportsThemes;
	bool CanDrawThemeParentBackground;
	pfnDrawThemeParentBackground DrawThemeParentBackground;

	/* Get memory information */
	pfnGlobalMemoryStatusEx GlobalMemoryStatusEx;
	pfnGlobalMemoryStatus GlobalMemoryStatus;

public:
	static OSSPECIFIC& GetInstance();

public:
	OSSPECIFIC();
	virtual ~OSSPECIFIC();
};

//void GetOSSpecific(OSSPECIFIC& osspecific);