// ResizeableDialog.cpp: Implementierungsdatei
//

#include "stdafx.h"
//#include "AVIMux_GUI.h"
#include "ResizeableDialog.h"
#include "../strings.h"
#include ".\resizeabledialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CResizeableDialog 

BOOL CALLBACK MyMonitorEnumProc(
  HMONITOR hMonitor,  // handle to display monitor
  HDC hdcMonitor,     // handle to monitor DC
  LPRECT lprcMonitor, // monitor intersection rectangle
  LPARAM dwData       // data
)
{
	std::vector<RECT>* displayRectangles = (std::vector<RECT>*)(void*)dwData;

	displayRectangles->push_back(*lprcMonitor);

	return true;
}

bool IsInRect(LPRECT pRect, int x, int y)
{
	if (x >= pRect->left && x <= pRect->right &&
		y >= pRect->top && y <= pRect->bottom)
	{
		return true;
	}

	return false;
}

bool IsInAnyRect(std::vector<RECT>* pRects, int x, int y)
{
	bool result = false;
	for (std::vector<RECT>::iterator iter = pRects->begin();
		iter != pRects->end(); iter++)
	{
		RECT rect = *iter;
		if (IsInRect(&rect, x, y))
			result = true;
	}

	return result;
}

CResizeableDialog::CResizeableDialog(UINT nID, CWnd* pParent /*=NULL*/)
	: CDialog(nID, pParent)
{
	EnableAutomation();
#ifdef I_ATTRIBUTES
	attribs = NULL;
#endif
	has_been_initialized = 0;
	manual_syscommand = 0;
	bMinimized = false;
	bMaximized = false;
	user_font = NULL;

	//{{AFX_DATA_INIT(CResizeableDialog)
		// HINWEIS: Der Klassen-Assistent fügt hier Elementinitialisierung ein
	//}}AFX_DATA_INIT

	EnumDisplayMonitors(NULL, NULL, MyMonitorEnumProc, (LPARAM)&displayRectangles);
}
CResizeableDialog::~CResizeableDialog()
{
/*	if (attribs) {
		attribs->Delete();
		delete attribs;
		attribs = NULL;
	}*/
}


#ifdef I_ATTRIBUTES
void CResizeableDialog::Attribs(CAttribs* a)
{
/*	if (attribs) {
		attribs->Delete();
		delete attribs;
	}*/
	attribs = a/*->Duplicate()*/;
}
#endif

void CResizeableDialog::OnSysCommand(UINT nID, LPARAM lParam)
{
	if (nID == SC_CLOSE)
		PostMessage(WM_COMMAND, IDCANCEL < 16);
	else {
		if (nID == SC_MINIMIZE) {
			bMinimized = true;
			bMaximized = false;
		} else
		if (nID == SC_RESTORE  || nID == 0xF122) {
			bMinimized = false;
			bMaximized = false;
			if (!manual_syscommand)
				has_been_initialized = 1;
			manual_syscommand = 0;
		} else
		if (nID == SC_MAXIMIZE || nID == 0xF032) {
			bMinimized = false;	
			bMaximized = true;
			if (!manual_syscommand)
				has_been_initialized = 1;
			manual_syscommand = 0;
		} 

		CDialog::OnSysCommand(nID, lParam);
	}
}

void CResizeableDialog::GetBorder(int &w, int &h)
{
	RECT r_wnd, r_clt;
	GetClientRect(&r_clt);
	GetWindowRect(&r_wnd);
	int h_clt = r_clt.bottom - r_clt.top;
	int h_wnd = r_wnd.bottom - r_wnd.top;
	int w_clt = r_clt.right - r_clt.left;
	int w_wnd = r_wnd.right - r_wnd.left;

	h = h_wnd - h_clt;
	w = w_wnd - w_clt;
}

void CResizeableDialog::OnFinalRelease()
{
	// Nachdem die letzte Referenz auf ein Automatisierungsobjekt freigegeben wurde,
	// wird OnFinalRelease aufgerufen. Die Basisklasse löscht das Objekt
	// automatisch. Fügen Sie zusätzlichen Bereinigungscode für Ihr Objekt
	// hinzu, bevor Sie die Basisklasse aufrufen.

	CDialog::OnFinalRelease();
}

void CResizeableDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CResizeableDialog)
		// HINWEIS: Der Klassen-Assistent fügt hier DDX- und DDV-Aufrufe ein
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CResizeableDialog, CDialog)
	//{{AFX_MSG_MAP(CResizeableDialog)
	ON_WM_SIZE()
	ON_WM_DELETEITEM()
	ON_WM_DESTROY()
	ON_WM_SYSCOMMAND()
	ON_WM_MOVE()
	//}}AFX_MSG_MAP
	ON_WM_ACTIVATE()
	ON_WM_SIZING()
	ON_WM_MOVING()
END_MESSAGE_MAP()

BEGIN_DISPATCH_MAP(CResizeableDialog, CDialog)
	//{{AFX_DISPATCH_MAP(CResizeableDialog)
		// HINWEIS - Der Klassen-Assistent fügt hier Zuordnungsmakros ein und entfernt diese.
	//}}AFX_DISPATCH_MAP
END_DISPATCH_MAP()

// Hinweis: Wir stellen Unterstützung für IID_IResizeableDialog zur Verfügung, um typsicheres Binden
//  von VBA zu ermöglichen. Diese IID muss mit der GUID übereinstimmen, die in der
//  Disp-Schnittstelle in der .ODL-Datei angegeben ist.

// {82AB1335-7373-4172-9095-FAC273048AB4}
static const IID IID_IResizeableDialog =
{ 0x82ab1335, 0x7373, 0x4172, { 0x90, 0x95, 0xfa, 0xc2, 0x73, 0x4, 0x8a, 0xb4 } };

BEGIN_INTERFACE_MAP(CResizeableDialog, CDialog)
	INTERFACE_PART(CResizeableDialog, IID_IResizeableDialog, Dispatch)
END_INTERFACE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CResizeableDialog 

void CResizeableDialog::OnSize(UINT nType, int cx, int cy) 
{
	int redo;
	CDialog::OnSize(nType, cx, cy);
	
	if (!being_destroyed  && !bMinimized && cx > 0 && cy > 0) {
	resize_in_progress = 1;

		ReorderWindows(redo);

		/* do that again with dependencies require that */ 
		for (int j = 0; j < 5 && redo; j++)
			ReorderWindows(redo);

		resize_in_progress = 0;

#ifdef I_ATTRIBUTES
		if (has_been_initialized) {
			RECT r;
			GetWindowRect(&r);
			if (attribs) {
				if (!bMaximized && !bMinimized) {
					attribs->SetInt("window_size/width", r.right - r.left);
					attribs->SetInt("window_size/height", r.bottom - r.top);
				}
				attribs->SetInt("window_size/maximized", !!bMaximized);
			}
		}
#endif

		UpdateWindow();
	}

	/* if the window was minimized, maximized or un-maxi/mini-mized, once more... */
	if (nType == SIZE_MAXIMIZED || nType == SIZE_MINIMIZED)
		PostMessage(WM_SIZE, SIZE_RESTORED, cx + (cy << 16));

	// TODO: Code für die Behandlungsroutine für Nachrichten hier einfügen
}

#ifdef I_ATTRIBUTES
void CResizeableDialog::ReinitFont(CFont* f)
{
	CFont* old_user_font = user_font;

	if (f) {
		ApplyFont(f);
		user_font = f;
	} else {
		CAttribs* a = GetAttribs();
		if (a->Exists("font")) {
			CAttribs* f = a->GetAttr("font");
			LOGFONT logfont;
			GetFont()->GetLogFont(&logfont);

			if (f->Exists("size"))
				logfont.lfHeight = -(LONG)f->GetInt("size");

			if (f->Exists("quality")) {
				int b = (int)f->GetInt("quality");
				if (b >= 0)
					logfont.lfQuality = (BYTE)b;
			}

			char* fname = NULL;
			if (f->Exists("face")) {
				f->GetStr("face", &fname);
				strcpy(logfont.lfFaceName, fname);
				free(fname);
			}

			user_font = new CFont;
			user_font->CreateFontIndirect(&logfont);
			if (fname)
				delete fname;
			ApplyFont(user_font);
			GetFont()->GetLogFont(&logfont);
		
			if (old_user_font)
				delete old_user_font;
		}
	}
}

void CResizeableDialog::ReinitPosition()
{
	int width = -INT_MAX;
	int height = -INT_MAX;
	int left = -INT_MAX;
	int top = -INT_MAX;
	int maximized = 0;

	if (attribs) 
		left = (int)attribs->GetIntWithDefault("window_position/left", -INT_MAX);
	if (attribs)
		top = (int)attribs->GetIntWithDefault("window_position/top", -INT_MAX);

	if (attribs && attribs->GetIntWithDefault("window_size/width"))
		width = (int)attribs->GetInt("window_size/width");
	if (attribs && attribs->GetIntWithDefault("window_size/height"))
		height = (int)attribs->GetInt("window_size/height");
	if (attribs && attribs->GetIntWithDefault("window_size/maximized"))
		maximized = (int)attribs->GetInt("window_size/maximized");

	RECT r;
	GetWindowRect(&r);

	if (width == -INT_MAX) width = r.right - r.left;
	if (height == -INT_MAX) height = r.bottom - r.top;
	if (left == -INT_MAX) left = r.left;
	if (top == -INT_MAX) top = r.top;

	RECT;
	GetWindowRect(&r);
	ScreenToClient(&r);

	int screen_width = GetSystemMetrics(SM_CXSCREEN);
	int screen_height = GetSystemMetrics(SM_CYSCREEN);



	left = (left!=-INT_MAX)?left:(screen_width - width) / 2;
	top = (top!=-INT_MAX)?top:(screen_height - height) / 2;

	r.left = left;
	r.top = top;
	r.right = r.left + width;
	r.bottom = r.top + height;

	if ((IsInAnyRect(&displayRectangles, r.left, r.top) &&
		IsInAnyRect(&displayRectangles, r.left, r.bottom) &&
		IsInAnyRect(&displayRectangles, r.right, r.top) &&
		IsInAnyRect(&displayRectangles, r.right, r.bottom)) ||
		maximized)
	{
		/* each of the 4 points is on the screen, accept this */
	}
	else 
	{
	/* put window on screen */
		if (r.right-r.left > screen_width) {
			r.left = 0; r.right = screen_width;
		}
		if (r.bottom-r.top > screen_height) {
			r.top = 0; r.bottom = screen_height;
		}
		if (r.right > screen_width) {
			int diff = r.right - screen_width;
			r.right = screen_width;
			r.left -= diff;
		}
		if (r.bottom > screen_height) {
			int diff = r.bottom - screen_height;
			r.bottom = screen_height;
			r.top -= diff;
		}
		if (r.left < 0) {
			r.right -= r.left;
			r.left = 0;
		}
		if (r.top < 0) {
			r.bottom -= r.top;
			r.top = 0;
		}
	}
	width = r.right - r.left;
	height = r.bottom - r.top;
		
	WINDOWPLACEMENT wp;
	memset(&wp, 0, sizeof(wp));
	GetWindowPlacement(&wp);

	wp.rcNormalPosition.left = r.left;
	wp.rcNormalPosition.right = r.right;
	wp.rcNormalPosition.top = r.top;
	wp.rcNormalPosition.bottom = r.bottom;

	if (maximized)
		wp.showCmd = SW_SHOWMAXIMIZED;
	else
		wp.showCmd = SW_SHOW;

	SetWindowPlacement(&wp);
}

#endif

BOOL CResizeableDialog::OnInitDialog() 
{
	being_destroyed = 0;
	resize_in_progress = 0;
	user_font = NULL;

	CDialog::OnInitDialog();

#ifdef I_ATTRIBUTES
	ReinitFont(NULL);
	ReinitPosition();
#endif

	// TODO: Zusätzliche Initialisierung hier einfügen
	
	RECT r;
	GetWindowRect(&r);
	for (int j=0; j<2; j++)
		PostMessage(WM_SIZE, 0, (r.right-r.left) | ((r.bottom-r.top)<<16));

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX-Eigenschaftenseiten sollten FALSE zurückgeben
}

BOOL CALLBACK EnumChildProcSetFont(HWND hwnd, LPARAM lParam)
{
	CFont* f = (CFont*)lParam;

	SendMessage(hwnd, WM_SETFONT, (WPARAM)f->m_hObject, (LPARAM)true);

	return true;
}


void CResizeableDialog::ApplyFont(CFont* f)
{
	SetFont(f);
	EnumChildWindows(*this, EnumChildProcSetFont, (LPARAM)f);
}


void CResizeableDialog::AttachWindow(HWND hWnd, int border, HWND hWndAttachTo, int distance)
{
	::AttachWindow(hWnd, border, hWndAttachTo, distance, attached_windows);
}

void CResizeableDialog::AttachWindow(HWND hWnd, int border, HWND hWndAttachTo, int target_border, int distance)
{
	::AttachWindow(hWnd, border, hWndAttachTo, target_border, distance, attached_windows);
}

void CResizeableDialog::ReorderWindows(int& redo)
{
	int redraw;

	::ReorderWindows(attached_windows, redraw, redo, m_hWnd);
}

void CResizeableDialog::AttachWindow(HWND hWnd, HWND hWndAttachTo, int flags, float width_ratio, float height_ratio)
{
	::AttachWindow(hWnd, hWndAttachTo, flags, width_ratio, height_ratio, attached_windows);
}

void CResizeableDialog::OnDeleteItem(int nIDCtl, LPDELETEITEMSTRUCT lpDeleteItemStruct) 
{
	// TODO: Code für die Behandlungsroutine für Nachrichten hier einfügen und/oder Standard aufrufen
	
	CDialog::OnDeleteItem(nIDCtl, lpDeleteItemStruct);
}

void CResizeableDialog::AttachLabel(HWND hWnd, HWND hWndLabel)
{
	AttachWindow(hWndLabel, ATTB_VCENTER, hWnd);
}

void CResizeableDialog::OnDestroy() 
{
	being_destroyed = 1;



	CDialog::OnDestroy();
	
	if (user_font)
		delete user_font;

	// TODO: Code für die Behandlungsroutine für Nachrichten hier einfügen
	
}

void CResizeableDialog::AttachRow(HWND* hWnd, int distance, int additional_alignment) {
	if (!hWnd)
		return;

	while (hWnd[0] && hWnd[1]) {
		AttachWindow(hWnd[1], ATTB_TOP, hWnd[0], ATTB_BOTTOM, distance);
		if (additional_alignment)
			AttachWindow(hWnd[1], additional_alignment, hWnd[0]);

		hWnd++;
	}
}

void CResizeableDialog::AttachVCenterAndLeftBorder(HWND hWnd, HWND hVCenterTo, 
												   HWND hLeftBorderTo, int indent) 
{
	AttachWindow(hWnd, ATTB_VCENTER, hVCenterTo);
	AttachWindow(hWnd, ATTB_LEFT, hLeftBorderTo, ATTB_LEFT, indent);
}

void CResizeableDialog::AttachUpDown(HWND hWnd, HWND hUpDown)
{
	AttachWindow(hUpDown, ATTB_TOPBOTTOM, hWnd);
	AttachWindow(hUpDown, ATTB_LEFT, hWnd, ATTB_RIGHT, 1);
}

void CResizeableDialog::AttachWindowBeneath(HWND hWnd, HWND hWndTo, int distance,
											int additional_alignment, int indent)
{
	AttachWindow(hWnd, ATTB_TOP, hWndTo, ATTB_BOTTOM, distance);
	AttachWindow(hWnd, additional_alignment, hWndTo, additional_alignment, indent);
}

void CResizeableDialog::AttachRowBeneath(HWND* hWnd_row, HWND hWndTo, int first_distance,
										 int distance, int additional_alignment,
										 int indent)
{
	if (!hWnd_row || !hWnd_row[0])
		return;

	AttachWindowBeneath(hWnd_row[0], hWndTo, first_distance, additional_alignment, indent);
	AttachRow(hWnd_row, distance, additional_alignment);
}

BOOL CResizeableDialog::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	// TODO: Speziellen Code hier einfügen und/oder Basisklasse aufrufen

	return CDialog::OnCommand(wParam, lParam);
}

void CResizeableDialog::OnMove(int x, int y) 
{
	CDialog::OnMove(x, y);
	
	RECT r;
	GetWindowRect(&r);

#ifdef I_ATTRIBUTES
	if (has_been_initialized)
	{
		WINDOWPLACEMENT wp;
		memset(&wp, 0, sizeof(wp));
		wp.length = sizeof(wp);
		GetWindowPlacement(&wp);

/*		if (!bMaximized) {
			if (attribs)
				attribs->SetInt("window_position/left", r.left);
			if (attribs)
				attribs->SetInt("window_position/top", r.top);
		}*/
		if (attribs)
		{
			attribs->SetInt("window_position/left", wp.rcNormalPosition.left);
			attribs->SetInt("window_position/top", wp.rcNormalPosition.top);
			attribs->SetInt("window_size/width", 
				wp.rcNormalPosition.right - wp.rcNormalPosition.left);
			attribs->SetInt("window_size/height", 
				wp.rcNormalPosition.bottom - wp.rcNormalPosition.top);

			attribs->SetInt("window_size/maximized", !!(wp.showCmd == SW_SHOWMAXIMIZED));
		}
	}
#endif
	// TODO: Code für die Behandlungsroutine für Nachrichten hier einfügen
	
}

void CResizeableDialog::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized)
{
	CDialog::OnActivate(nState, pWndOther, bMinimized);

	if (nState == WA_ACTIVE) {
		ReinitPosition();
	}
	// TODO: Fügen Sie hier Ihren Meldungsbehandlungscode ein.
}

void CResizeableDialog::OnSizing(UINT fwSide, LPRECT pRect)
{
	CDialog::OnSizing(fwSide, pRect);

	has_been_initialized = 1;
	// TODO: Fügen Sie hier Ihren Meldungsbehandlungscode ein.
}

void CResizeableDialog::OnMoving(UINT fwSide, LPRECT pRect)
{
	CDialog::OnMoving(fwSide, pRect);

	has_been_initialized = 1;
	// TODO: Fügen Sie hier Ihren Meldungsbehandlungscode ein.
}
