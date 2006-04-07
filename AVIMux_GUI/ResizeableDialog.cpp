// ResizeableDialog.cpp: Implementierungsdatei
//

#include "stdafx.h"
//#include "AVIMux_GUI.h"
#include "ResizeableDialog.h"
#include "../strings.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CResizeableDialog 


CResizeableDialog::CResizeableDialog(UINT nID, CWnd* pParent /*=NULL*/)
	: CDialog(nID, pParent)
{
	EnableAutomation();
#ifdef I_ATTRIBUTES
	attribs = NULL;
#endif
	has_been_initialized = 0;
	bMinimized = false;
	user_font = NULL;

	//{{AFX_DATA_INIT(CResizeableDialog)
		// HINWEIS: Der Klassen-Assistent fügt hier Elementinitialisierung ein
	//}}AFX_DATA_INIT
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
		if (nID == SC_MINIMIZE)
			bMinimized = true;
		if (nID == SC_RESTORE)
			bMinimized = false;
		if (nID == SC_MAXIMIZE)
			bMinimized = false;			

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
			if (attribs)
				attribs->SetInt("window_size/width", r.right - r.left);
			if (attribs)
				attribs->SetInt("window_size/height", r.bottom - r.top);
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

	if (attribs) 
		left = (int)attribs->GetIntWithDefault("window_position/left", -INT_MAX);
	if (attribs)
		top = (int)attribs->GetIntWithDefault("window_position/top", -INT_MAX);

	if (attribs && attribs->GetIntWithDefault("window_size/width"))
		width = (int)attribs->GetInt("window_size/width");
	if (attribs && attribs->GetIntWithDefault("window_size/height"))
		height = (int)attribs->GetInt("window_size/height");

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

	width = r.right - r.left;
	height = r.top - r.bottom;
			 
	MoveWindow(&r);
	PostMessage(WM_SIZE, 0, (width | (height << 16)));
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

	has_been_initialized = 1;
	// TODO: Zusätzliche Initialisierung hier einfügen
	
	RECT r;
	GetWindowRect(&r);
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
	AttachWindow(hWnd, hWndLabel, ATTB_VCENTER);
}

void CResizeableDialog::OnDestroy() 
{
	being_destroyed = 1;



	CDialog::OnDestroy();
	
	if (user_font)
		delete user_font;

	// TODO: Code für die Behandlungsroutine für Nachrichten hier einfügen
	
}

void CResizeableDialog::AttachRow(HWND* hWnd, int distance) {
	if (!hWnd)
		return;

	while (hWnd[0] && hWnd[1]) {
		AttachWindow(hWnd[1], ATTB_TOP, hWnd[0], ATTB_BOTTOM, distance);
		hWnd++;
	}
}

void CResizeableDialog::AttachUpDown(HWND hWnd, HWND hUpDown)
{
	AttachWindow(hUpDown, ATTB_TOPBOTTOM, hWnd);
	AttachWindow(hUpDown, ATTB_LEFT, hWnd, ATTB_RIGHT, 1);
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
	if (has_been_initialized) {
		if (attribs)
			attribs->SetInt("window_position/left", r.left);
		if (attribs)
			attribs->SetInt("window_position/top", r.top);
	}
#endif
	// TODO: Code für die Behandlungsroutine für Nachrichten hier einfügen
	
}
