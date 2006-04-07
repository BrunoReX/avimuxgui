#ifndef I_TABHANDLER
#define I_TABHANDLER

/* defines an interface to set the window to get the focus
   when pressing TAB in another window */

const int WM_TAB_SETNEXTWINDOW = WM_USER + 0x03F3;
const int WM_TAB_SELECTALLAFTERTAB = WM_USER + 0x3F5;
const int WM_TAB_HANDLERINSTALLED = WM_USER + 0x01F9;

void TABHandler_install(HWND hWnd);
void TABHandler_install(HWND hWnd, HWND hWndNext, bool bSelectAll);

#endif