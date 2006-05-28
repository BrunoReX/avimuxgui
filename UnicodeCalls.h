#ifndef I_UNICODE_CALLS
#define I_UNICODE_CALLS

#include "windows.h"
#include "types.h"

int fromUTF8(void* s, void* d);
int toUTF8(void* s, void* d);
int fromUTF8(void* s, char** d);
int toUTF8(void* s, char** d);


typedef HANDLE (WINAPI *CREATEFILE)(
	void* lpFilename, DWORD dwDesiredAccess, DWORD dwShareMode,
	LPSECURITY_ATTRIBUTES lpSecurityAttributes, 
	DWORD dwCreationDisposition,
	DWORD dwFlagsAndAttributes,
	HANDLE hTemplateFile);

typedef UINT (WINAPI *DRAGQUERYFILE)(
	uint32 hDrop, UINT iFile, void* pBuffer, UINT cch);

typedef BOOL (WINAPI *GETFILENAME)(
    void* lpofn);

typedef BOOL (WINAPI *GETDISKFREESPACEEX)(
	void* lpDirectoryName, uint64* lpFreeBytesAvailableToCaller,
	uint64* lpTotalNumberOfBytes, uint64* lpTotalNumberOfFreeBytes);

typedef BOOL (WINAPI *TEXTOUT)(
	HDC hdc, int nXStart, int nYStart, void* pText, int cbString);

typedef BOOL (WINAPI *GETCHARABCWIDTHS)(
	HDC hdc, UINT uFirstChar, UINT uLastChar, void* pABC);

typedef int (WINAPI *MESSAGEBOX)(
	HWND hWnd, void* pText, void* pTitle, UINT uType);

typedef BOOL (WINAPI *EXTEXTOUT)(
	HDC hdc, int x, int y, UINT fuOptions, void* lpRC, void* lpString, UINT cbCount, void* lpDX);

typedef uint32 (WINAPI *GETMODULEFILENAME)(
	HMODULE hModule, char* lpFilename, uint32 nSize);

typedef uint32 (WINAPI *CREATEWINDOWEX)(
	uint32 dwExStyle, char* lpClassName, char* lpWindowName, uint32 dwStyle,
	int x, int y, int nWidth, int nHeight, uint32 hWndParent, uint32 hMenu,
	uint32 hInstance, void* lpParam);

typedef bool (WINAPI* SETCURRENTDIRECTORY)(
	void*);

typedef bool (WINAPI* GETCURRENTDIRECTORY) (
	DWORD, void*);

typedef uint32(WINAPI *SETWINDOWLONG)(
	HWND hwnd, uint32 index, uint32 value);
typedef uint32(WINAPI *GETWINDOWLONG)(
	HWND hwnd, uint32 index);

typedef bool(WINAPI* GETVOLUMEPATHNAME)(
	void*, void*, DWORD);

typedef BOOL(WINAPI* GETVOLUMEINFORMATION)(
	void*, void*, DWORD, DWORD*, DWORD*, DWORD*, void*, DWORD);

/*
typedef uint32(WINAPI *SENDMESSAGE)(
	HWND hWnd, uint32 uMsg, uint32 wParam, uint32 lParam);
*/
CREATEFILE			UCreateFile();
DRAGQUERYFILE		UDragQueryFile();
GETFILENAME			UGetOpenFileName();
GETFILENAME			UGetSaveFileName();
GETDISKFREESPACEEX	UGetDiskFreeSpaceEx();
TEXTOUT				UTextOut();
EXTEXTOUT			UExtTextOut();
MESSAGEBOX          UMessageBox();
GETMODULEFILENAME	UGetModuleFileName();
CREATEWINDOWEX		UCreateWindowEx();
SETWINDOWLONG		USetWindowLong();
SETCURRENTDIRECTORY USetCurrentDirectory();
GETCURRENTDIRECTORY UGetCurrentDirectory();
GETVOLUMEPATHNAME	UGetVolumePathName();
GETVOLUMEINFORMATION UGetVolumeInformation();

FILE* fopenutf8(char* filename, char* access, int unicode);

#endif