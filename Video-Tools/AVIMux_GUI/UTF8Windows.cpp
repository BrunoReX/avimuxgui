
/* Appearently, the Windows XP SP 2 installation of one specific user sucks
      WM_GETTEXT messages don't pass when the buffer size exceeds about 15k characters,
	  and WideCharToMultiByte doesn't seem to care too much about cbMultiByte...

   The mess you see below was required to find that out.... 
*/

#include "stdafx.h"
#include "UTF8Windows.h"
#include "..\UnicodeCalls.h"
#include "..\UTF-8.h"
#include "UnicodeBase.h"
#include "TABHandler.h"


LRESULT CALLBACK UTF8WindowProc(
  HWND hwnd,      // handle to window
  UINT uMsg,      // message identifier
  WPARAM wParam,  // first message parameter
  LPARAM lParam   // second message parameter
);


typedef LRESULT(CALLBACK *WINDOWPROC)(HWND, UINT, WPARAM, LPARAM);

typedef struct {
	void*	old_wnd_proc;
	uint8	unicode;
	uint32	userdata;
	bool	was_key_down;
	CUnicodeBase* ucb;
	HWND	hwndNext;
} USERDATA_UTF8WINDOW;

uint32 WINAPI CreateWindowExUTF8(
	uint32 dwExStyle, char* lpClassName, char* lpWindowName, uint32 dwStyle,
	int x, int y, int nWidth, int nHeight, uint32 hWndParent, uint32 hMenu,
	uint32 hInstance, void* lpParam)
{
	char* lcn = NULL;
	char* lwn = NULL;

	fromUTF8(lpClassName, &lcn);
	fromUTF8(lpWindowName, &lwn);

	uint32 res = (*UCreateWindowEx())(dwExStyle, lcn, lwn, dwStyle, x, y, nWidth, nHeight,
		hWndParent, hMenu, hInstance, lpParam);

	if (!res) {
		int i = utf8_IsUnicodeEnabled();
		if (i) {
			utf8_EnableRealUnicode(false);
			res = CreateWindowExUTF8(dwExStyle, lcn, lwn, dwStyle, x, y, nWidth, nHeight,
				hWndParent, hMenu, hInstance, lpParam);
			utf8_EnableRealUnicode(true);
		} 
	} else {
		USERDATA_UTF8WINDOW* data = new USERDATA_UTF8WINDOW;
		data->was_key_down = false;
		data->unicode = utf8_IsUnicodeEnabled();
		data->old_wnd_proc = (void*)(*USetWindowLong())((HWND)res, GWL_WNDPROC, (LONG)UTF8WindowProc);
		data->userdata = NULL;
		data->ucb = new CUnicodeBase();
		data->ucb->InitUnicode(data->unicode);
		data->hwndNext = 0;
		SetLastError(0);
		LONG i = (*USetWindowLong())((HWND)res, GWL_USERDATA, (LONG)data);
		if (!i)
			i = GetLastError();
		
	}

	free(lcn);
	free(lwn);

	return res;
};

/* window procedure for utf-8 windows */

LRESULT CALLBACK UTF8WindowProc(
  HWND hwnd,      // handle to window
  UINT uMsg,      // message identifier
  WPARAM wParam,  // first message parameter
  LPARAM lParam   // second message parameter
)
{
	USERDATA_UTF8WINDOW* data = (USERDATA_UTF8WINDOW*)GetWindowLong(hwnd, GWL_USERDATA);
	int result = 0;

	ASSERT(data);

	switch (uMsg) {
		case WM_KEYDOWN:
			data->was_key_down = 1;
			break;
	/*	case WM_CHAR:
			if (wParam == VK_TAB) {
				if (data->hwndNext) {
					SetFocus(data->hwndNext);
					return 1;
				}
			}
			break;*/
		case WM_WASKEYDOWN:
			result = data->was_key_down;
			data->was_key_down = 0;
			return result;
			break;
		case WM_GETDLGCODE:
			result = (*((WINDOWPROC)data->old_wnd_proc))(hwnd, uMsg, wParam, lParam);
			result |= DLGC_WANTTAB;
			return result;
			break;
/*		case WM_TAB_SETNEXTWINDOW:
			data->hwndNext = (HWND)wParam;
			break;
		case WM_TAB_HANDLERINSTALLED:
			return 1;
			break;*/
	}

	return (*((WINDOWPROC)data->old_wnd_proc))(hwnd, uMsg, wParam, lParam);
}

uint32 WINAPI GetWindowTextLengthUTF8(HWND hWnd)
{
	USERDATA_UTF8WINDOW* data = (USERDATA_UTF8WINDOW*)GetWindowLong(hWnd, GWL_USERDATA);

	ASSERT(data);

	int len;

	len = 1+GetWindowTextLengthA(hWnd);

	int fkt = (data->unicode)?2:1;

	char* pBuffer = (char*)malloc(fkt*len);

	if (data->unicode)
		SendMessageW(hWnd, WM_GETTEXT, (WPARAM)len, (LPARAM)pBuffer);
	else
		SendMessageA(hWnd, WM_GETTEXT, (WPARAM)len, (LPARAM)pBuffer);

	int result = data->ucb->toUTF8(pBuffer, NULL, 0)-1;
	free(pBuffer);

	return result;
}

uint32 WINAPI GetWindowTextUTF8(HWND hWnd, char* pBuffer, int max_len)
{
	USERDATA_UTF8WINDOW* data = (USERDATA_UTF8WINDOW*)GetWindowLong(hWnd, GWL_USERDATA);
	
	ASSERT(data);

	int len = GetWindowTextLength(hWnd)+1;
	int fkt = (data->unicode)?2:1;

	char* _pBuffer = (char*)malloc(fkt*len);

	if (data->unicode)
		SendMessageW(hWnd, WM_GETTEXT, (WPARAM)len, (LPARAM)_pBuffer);
	else
		SendMessageA(hWnd, WM_GETTEXT, (WPARAM)len, (LPARAM)_pBuffer);

	return data->ucb->toUTF8(_pBuffer, pBuffer, max_len);
}

void mb(int i) {
	/*char c[32]; c[0]=0;
	sprintf(c, "%d", i);
	MessageBox(0, c, "Info:", MB_OK);*/
}

uint32 WINAPI GetWindowTextUTF8(HWND hWnd, char** pBuffer)
{
	USERDATA_UTF8WINDOW* data = (USERDATA_UTF8WINDOW*)GetWindowLong(hWnd, GWL_USERDATA);

	ASSERT(data);
	int len;

	*pBuffer = (char*)malloc(len=1+GetWindowTextLengthUTF8(hWnd));
	GetWindowTextUTF8(hWnd, *pBuffer, len);

	return len;
}

uint32 WINAPI SetWindowTextUTF8(HWND hwnd, char* pBuffer) 
{
	USERDATA_UTF8WINDOW* data = (USERDATA_UTF8WINDOW*)GetWindowLong(hwnd, GWL_USERDATA);

	if (!data)
		return 0;

	int i;

	char* c = NULL;
	data->ucb->fromUTF8(pBuffer, &c);

	if (data && data->unicode)
		i = ::SendMessageW(hwnd, WM_SETTEXT, 0, (LPARAM)c);
	else
		i = ::SendMessageA(hwnd, WM_SETTEXT, 0, (LPARAM)c);

	free(c);

	return i;
}


const int EDIT_EXSTYLE = 0x204;
const int EDIT_STYLE = 0x50010080;

int MessageBoxUTF8(HWND hWnd, char* pText, char* pTitle, UINT uType)
{
	char* utext = NULL;
	char* utitle = NULL;

	fromUTF8(pText, &utext);
	fromUTF8(pTitle, &utitle);

	int result = (*UMessageBox())(hWnd, utext, utitle, uType);

	free(utext);
	free(utitle);

	return result;
}


void WINAPI CreateEditUTF8(RECT& r, HWND hParent, HINSTANCE hInstance, HFONT font, HWND& hwnd)
{
	HWND h = (HWND)CreateWindowExUTF8(EDIT_EXSTYLE, "EDIT", "", EDIT_STYLE &~ WS_TABSTOP, r.left, r.top,
		r.right - r.left, r.bottom -r.top, (uint32)hParent, NULL, (uint32)hInstance, NULL);
	
	::SendMessageA(h, WM_SETFONT, (WPARAM)font, 1);
	::SendMessageW(h, WM_SETFONT, (WPARAM)font, 1);
		
	hwnd = h;
}
