#include "stdafx.h"
#include "UnicodeCalls.h"
#include "utf-8.h"
#include "stdlib.h"

CREATEFILE			_CreateFile[2]	= { NULL, NULL };
DRAGQUERYFILE		_DragQueryFile[2] = { NULL, NULL };
GETFILENAME			_GetOpenFileName[2]	= { NULL, NULL };
GETFILENAME			_GetSaveFileName[2] = { NULL, NULL };
GETDISKFREESPACEEX	_GetDiskFreeSpaceEx[2]	= { NULL, NULL };
TEXTOUT				_TextOut[2] = { NULL, NULL };
EXTEXTOUT			_ExtTextOut[2] = { NULL, NULL };
GETCHARABCWIDTHS	_GetCharABCWidths[2] = { NULL, NULL };
MESSAGEBOX			_MessageBox[2] = { NULL, NULL };
GETMODULEFILENAME	_GetModuleFileName[2] = { NULL, NULL };
CREATEWINDOWEX      _CreateWindowEx[2] = { NULL, NULL };
SETWINDOWLONG		_SetWindowLong[2] ={ NULL, NULL };
SETCURRENTDIRECTORY _SetCurrentDirectory[2] = { NULL, NULL };
GETCURRENTDIRECTORY _GetCurrentDirectory[2] = { NULL, NULL };
GETVOLUMEPATHNAME	_GetVolumePathName[2] = { NULL, NULL };
GETVOLUMEINFORMATION _GetVolumeInformation[2] = { NULL, NULL };

HINSTANCE   hKernel32		= NULL;
HINSTANCE   hShell32        = NULL;
HINSTANCE   hComDlg32		= NULL;
HINSTANCE   hGdi32          = NULL;
HINSTANCE	hUser32         = NULL;


void _CriticalError(char* cDLL)
{

}

void AccessKernel32()
{
	if (!hKernel32) {
		hKernel32 = LoadLibraryA("kernel32.dll");
		if (!hKernel32) {
			_CriticalError("kernel32.dll");
		}
	
		_CreateFile[0] = (CREATEFILE)GetProcAddress(hKernel32, "CreateFileA");
		_CreateFile[1] = (CREATEFILE)GetProcAddress(hKernel32, "CreateFileW");
		_GetDiskFreeSpaceEx[0] = (GETDISKFREESPACEEX)GetProcAddress(hKernel32, "GetDiskFreeSpaceExA");
		_GetDiskFreeSpaceEx[1] = (GETDISKFREESPACEEX)GetProcAddress(hKernel32, "GetDiskFreeSpaceExW");
		_GetModuleFileName[0] = (GETMODULEFILENAME)GetProcAddress(hKernel32,"GetModuleFileNameA");
		_GetModuleFileName[1] = (GETMODULEFILENAME)GetProcAddress(hKernel32,"GetModuleFileNameW");
		_SetCurrentDirectory[0] = (SETCURRENTDIRECTORY)GetProcAddress(hKernel32, "SetCurrentDirectoryA");
		_SetCurrentDirectory[1] = (SETCURRENTDIRECTORY)GetProcAddress(hKernel32, "SetCurrentDirectoryW");
		_GetCurrentDirectory[0] = (GETCURRENTDIRECTORY)GetProcAddress(hKernel32, "GetCurrentDirectoryA");
		_GetCurrentDirectory[1] = (GETCURRENTDIRECTORY)GetProcAddress(hKernel32, "GetCurrentDirectoryW");
		_GetVolumePathName[0] = (GETVOLUMEPATHNAME)GetProcAddress(hKernel32, "GetVolumePathNameA");
		_GetVolumePathName[1] = (GETVOLUMEPATHNAME)GetProcAddress(hKernel32, "GetVolumePathNameW");
		_GetVolumeInformation[0] = (GETVOLUMEINFORMATION)GetProcAddress(hKernel32, "GetVolumeInformationA");
		_GetVolumeInformation[1] = (GETVOLUMEINFORMATION)GetProcAddress(hKernel32, "GetVolumeInformationW");
	}
}

void AccessShell32()
{
	if (!hShell32) {
		hShell32 = LoadLibraryA("shell32.dll");
		if (!hShell32) {
			_CriticalError("shell32.dll");
		}

		_DragQueryFile[0] = (DRAGQUERYFILE)GetProcAddress(hShell32, "DragQueryFileA");
		_DragQueryFile[1] = (DRAGQUERYFILE)GetProcAddress(hShell32, "DragQueryFileW");
	}
}

void AccessComDlg32()
{
	if (!hComDlg32)	{
		hComDlg32 = LoadLibraryA("comdlg32.dll");
		if (!hComDlg32)
			_CriticalError("comdlg32.dll");

		_GetOpenFileName[0] = (GETFILENAME)GetProcAddress(hComDlg32, "GetOpenFileNameA");
		_GetOpenFileName[1] = (GETFILENAME)GetProcAddress(hComDlg32, "GetOpenFileNameW");
		_GetSaveFileName[0] = (GETFILENAME)GetProcAddress(hComDlg32, "GetSaveFileNameA");
		_GetSaveFileName[1] = (GETFILENAME)GetProcAddress(hComDlg32, "GetSaveFileNameW");
	}
}

void AccessGDI32()
{
	if (!hGdi32) {
		hGdi32 = LoadLibraryA("gdi32.dll");
		if (!hGdi32) {
			_CriticalError("gdi32.dll");
		}

		_TextOut[0] = (TEXTOUT)GetProcAddress(hGdi32, "TextOutA");
		_TextOut[1] = (TEXTOUT)GetProcAddress(hGdi32, "TextOutW");
		_GetCharABCWidths[0] = (GETCHARABCWIDTHS)GetProcAddress(hGdi32, "GetCharABCWidthsA");
		_GetCharABCWidths[1] = (GETCHARABCWIDTHS)GetProcAddress(hGdi32, "GetCharABCWidthsW");
		_ExtTextOut[0] = (EXTEXTOUT)GetProcAddress(hGdi32, "ExtTextOutA");
		_ExtTextOut[1] = (EXTEXTOUT)GetProcAddress(hGdi32, "ExtTextOutW");
	}
}

void AccessUser32()
{
	if (!hUser32) {
		hUser32 = LoadLibraryA("user32.dll");
		if (!hUser32)
			_CriticalError("user32.dll");

		_MessageBox[0] = (MESSAGEBOX)GetProcAddress(hUser32, "MessageBoxA");
		_MessageBox[1] = (MESSAGEBOX)GetProcAddress(hUser32, "MessageBoxW");
		_CreateWindowEx[0] = (CREATEWINDOWEX)GetProcAddress(hUser32, "CreateWindowExA");
		_CreateWindowEx[1] = (CREATEWINDOWEX)GetProcAddress(hUser32, "CreateWindowExW");
		_SetWindowLong[0] = (SETWINDOWLONG)GetProcAddress(hUser32, "SetWindowLongA");
		_SetWindowLong[1] = (SETWINDOWLONG)GetProcAddress(hUser32, "SetWindowLongW");
	}
}

#define UDEFINE(type,function,pt,load)  \
	type function() { \
	if (!pt[0] && !pt[1]) load(); \
	return pt[!!utf8_IsUnicodeEnabled()]; \
};

UDEFINE(CREATEFILE, UCreateFile, _CreateFile, AccessKernel32)
UDEFINE(DRAGQUERYFILE, UDragQueryFile, _DragQueryFile, AccessShell32);
UDEFINE(GETFILENAME, UGetOpenFileName, _GetOpenFileName, AccessComDlg32);
UDEFINE(GETFILENAME, UGetSaveFileName, _GetSaveFileName, AccessComDlg32);
UDEFINE(GETDISKFREESPACEEX, UGetDiskFreeSpaceEx, _GetDiskFreeSpaceEx, AccessKernel32);
UDEFINE(TEXTOUT, UTextOut, _TextOut, AccessGDI32);
UDEFINE(EXTEXTOUT, UExtTextOut, _ExtTextOut, AccessGDI32);
UDEFINE(GETCHARABCWIDTHS, UGetCharABCWidths, _GetCharABCWidths, AccessGDI32);
UDEFINE(MESSAGEBOX, UMessageBox, _MessageBox, AccessUser32);
UDEFINE(GETMODULEFILENAME, UGetModuleFileName, _GetModuleFileName, AccessKernel32);
UDEFINE(SETWINDOWLONG, USetWindowLong, _SetWindowLong, AccessUser32);
UDEFINE(SETCURRENTDIRECTORY, USetCurrentDirectory, _SetCurrentDirectory, AccessKernel32);
UDEFINE(GETCURRENTDIRECTORY, UGetCurrentDirectory, _GetCurrentDirectory, AccessKernel32);
UDEFINE(GETVOLUMEPATHNAME, UGetVolumePathName, _GetVolumePathName, AccessKernel32);
UDEFINE(GETVOLUMEINFORMATION, UGetVolumeInformation, _GetVolumeInformation, AccessKernel32);

CREATEWINDOWEX      UCreateWindowEx();
UDEFINE(CREATEWINDOWEX, UCreateWindowEx, _CreateWindowEx, AccessUser32);




int fromUTF8(void* s, void* d)
{
	if (!s || !d)
		return 0;

	if (utf8_IsUnicodeEnabled())
		return UTF82WStr((char*)s, (char*)d)/2;
	else
		return UTF82Str((char*)s, (char*)d);

}

int toUTF8(void* s, void* d)
{
	if (!s || !d)
		return 0;

	if (utf8_IsUnicodeEnabled())
		return WStr2UTF8((char*)s, (char*)d);
	else
		return Str2UTF8((char*)s, (char*)d);

	return 0;
}

int fromUTF8(void* s, char** d)
{
	if (!s || !d)
		return 0;

	if (utf8_IsUnicodeEnabled())
		return UTF82WStr((char*)s, (char**)d)/2;
	else
		return UTF82Str((char*)s, (char**)d);

}

int toUTF8(void* s, char** d)
{
	if (!s || !d)
		return 0;

	if (utf8_IsUnicodeEnabled())
		WStr2UTF8((char*)s, (char**)d);
	else
		Str2UTF8((char*)s, (char**)d);

	return 1;
}

FILE* fopenutf8(char* filename, char* access, int unicode)
{
	FILE* file;
	if (unicode) {
		char* c = NULL;
		UTF82WStr(filename,&c);
		char mode[8]; mode[0]=0;
		UTF82WStr(access, mode, 8);
		file=_wfopen((const unsigned short*)c,(const unsigned short*)mode);
		free(c);
	} else {
		file = fopen(filename, access);
	}
	return file;
}