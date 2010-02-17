#include "stdafx.h"
#include "TABHandler.h"
#include "../UnicodeCalls.h"
#include "windows.h"
#include <map>

typedef struct
{
	LONG_PTR old_wnd_proc;
	void* call_back;
	HWND hwndNext;
	bool bSelectAll;
} TABHANDLER_USERDATA;

typedef LRESULT(CALLBACK *WINDOWPROC)(HWND, UINT, WPARAM, LPARAM);

typedef std::map<HWND, TABHANDLER_USERDATA*> TAB_HANDLER_DATA_MAP;
TAB_HANDLER_DATA_MAP tab_handler_data_map;

LRESULT CALLBACK TABHandler_WindowProc(
  HWND hwnd,      // handle to window
  UINT uMsg,      // message identifier
  WPARAM wParam,  // first message parameter
  LPARAM lParam   // second message parameter
)
{
	TAB_HANDLER_DATA_MAP::iterator iter = tab_handler_data_map.find(hwnd);
	_ASSERT(iter != tab_handler_data_map.end());

	/* that should really not occur */
	if (iter == tab_handler_data_map.end()) {
		MessageBox(NULL, _T("B0rked!!!"), _T("Error"), MB_OK);
	}

	TABHANDLER_USERDATA* data = iter->second;
	LRESULT result;

	switch (uMsg) {
		case WM_CHAR:
			if (wParam == VK_TAB && data->hwndNext) {
				if (IsWindowEnabled(data->hwndNext)) {
					SetFocus((HWND)data->hwndNext);
					if (data->bSelectAll)
						::PostMessage(data->hwndNext, EM_SETSEL, (WPARAM)0, (LPARAM)-1);
					return 1;
				} else {
					EnableWindow(data->hwndNext, true);
					::SendMessage(data->hwndNext, WM_CHAR, wParam, lParam); 
					EnableWindow(data->hwndNext, false);
					return 1;
				}
			} else {
				return (*((WINDOWPROC)data->old_wnd_proc))(hwnd, uMsg, wParam, lParam);
			}
			break;
		case WM_GETDLGCODE:
			result = (*((WINDOWPROC)data->old_wnd_proc))(hwnd, uMsg, wParam, lParam);
			result |= DLGC_WANTTAB;
			return result;
			break;
		case WM_TAB_SETNEXTWINDOW:
			data->hwndNext = (HWND)wParam;
			return 1;
			break;
		case WM_TAB_HANDLERINSTALLED:
			return 1;
			break;
		case WM_TAB_SELECTALLAFTERTAB:
			data->bSelectAll = !!wParam;
			break;
		case WM_DESTROY:
			/* deinstall TAB window handler */
			result = (*((WINDOWPROC)data->old_wnd_proc))(hwnd, uMsg, wParam, lParam);
			SetWindowLongPtr(hwnd, GWL_WNDPROC, (LONG_PTR)data->old_wnd_proc);
			delete data;
			iter->second = NULL;
			return result;
			break;
		default:
			return (*((WINDOWPROC)data->old_wnd_proc))(hwnd, uMsg, wParam, lParam);
			break;
	}

	return 0;
}



void TABHandler_install(HWND hWnd)
{
	int result = 0;
	if (result = SendMessage(hWnd, WM_TAB_HANDLERINSTALLED, 0, 0))
		return;

	TABHANDLER_USERDATA* th = new TABHANDLER_USERDATA;

	/* callback not yet implemented for TAB key */
	th->call_back = NULL;

	/* with a certain manifest file I have, the edit window of a non-unicode combobox
	   is unicode. I don't understand it, but it is that way :/ */
	if (IsWindowUnicode(hWnd))
		th->old_wnd_proc = (LONG_PTR)SetWindowLongPtrW(hWnd, GWL_WNDPROC, 
			(LONG_PTR)TABHandler_WindowProc);
	else
		th->old_wnd_proc = (LONG_PTR)SetWindowLongPtrA(hWnd, GWL_WNDPROC, 
			(LONG_PTR)TABHandler_WindowProc);

	th->hwndNext = NULL;
	th->bSelectAll = false;
	tab_handler_data_map[hWnd] = th;
}

void TABHandler_install(HWND hWnd, HWND hWndNext, bool bSelectAll)
{
	TABHandler_install(hWnd);
	::SendMessage(hWnd, WM_TAB_SETNEXTWINDOW, (WPARAM)hWndNext, 0);
	::SendMessage(hWnd, WM_TAB_SELECTALLAFTERTAB, (WPARAM)!!bSelectAll, 0);
}
