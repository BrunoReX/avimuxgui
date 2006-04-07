#ifndef I_UTF8WINDOWS
#define I_UTF8WINDOWS

#include "..\types.h"

void mb(int i);

const int WM_WASKEYDOWN = WM_USER + 0x0001;

uint32 WINAPI CreateWindowExUTF8(
	uint32 dwExStyle, char* lpClassName, char* lpWindowName, uint32 dwStyle,
	int x, int y, int nWidth, int nHeight, uint32 hWndParent, uint32 hMenu,
	uint32 hInstance, void* lpParam);

uint32 WINAPI GetWindowTextUTF8(HWND hwnd, char* pBuffer, int max_len);
uint32 WINAPI GetWindowTextLengthUTF8(HWND hwnd);
uint32 WINAPI GetWindowTextUTF8(HWND hwnd, char** pBuffer);

uint32 WINAPI SetWindowTextUTF8(HWND hwnd, char* pBuffer);
int MessageBoxUTF8(HWND hWnd, char* pText, char* pTitle, UINT uType);

void WINAPI CreateEditUTF8(RECT& r, HWND hParent, HINSTANCE hInstance, HFONT font, HWND& hwnd);

#endif