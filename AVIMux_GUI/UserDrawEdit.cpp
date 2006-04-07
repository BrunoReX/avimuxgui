// UserDrawEdit.cpp: Implementierungsdatei
//
// Supports a function call which returns the color to be used for the text as result
//

#include "stdafx.h"
#include "AVIMux_GUI.h"
#include "UserDrawEdit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CUserDrawEdit

CUserDrawEdit::CUserDrawEdit()
{
	EnableAutomation();
}

CUserDrawEdit::~CUserDrawEdit()
{
}

void CUserDrawEdit::OnFinalRelease()
{
	// Nachdem die letzte Referenz auf ein Automatisierungsobjekt freigegeben wurde,
	// wird OnFinalRelease aufgerufen. Die Basisklasse löscht das Objekt
	// automatisch. Fügen Sie zusätzlichen Bereinigungscode für Ihr Objekt
	// hinzu, bevor Sie die Basisklasse aufrufen.

	CEdit::OnFinalRelease();
}


BEGIN_MESSAGE_MAP(CUserDrawEdit, CEdit)
	//{{AFX_MSG_MAP(CUserDrawEdit)
	ON_WM_PAINT()
	ON_WM_DRAWITEM()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BEGIN_DISPATCH_MAP(CUserDrawEdit, CEdit)
	//{{AFX_DISPATCH_MAP(CUserDrawEdit)
		// HINWEIS - Der Klassen-Assistent fügt hier Zuordnungsmakros ein und entfernt diese.
	//}}AFX_DISPATCH_MAP
END_DISPATCH_MAP()

// Hinweis: Wir stellen Unterstützung für IID_IUserDrawEdit zur Verfügung, um typsicheres Binden
//  von VBA zu ermöglichen. Diese IID muss mit der GUID übereinstimmen, die in der
//  Disp-Schnittstelle in der .ODL-Datei angegeben ist.

// {35FBD8C3-6A94-4CF0-BD8D-DB8B69E33DCC}
static const IID IID_IUserDrawEdit =
{ 0x35fbd8c3, 0x6a94, 0x4cf0, { 0xbd, 0x8d, 0xdb, 0x8b, 0x69, 0xe3, 0x3d, 0xcc } };

BEGIN_INTERFACE_MAP(CUserDrawEdit, CEdit)
	INTERFACE_PART(CUserDrawEdit, IID_IUserDrawEdit, Dispatch)
END_INTERFACE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CUserDrawEdit 

COLORREF CUserDrawEdit::GetTxtColor()
{
	return RGB(0,0,0);
}

void CUserDrawEdit::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	// TODO: Code für die Behandlungsroutine für Nachrichten hier einfügen

	CString c;
	RECT r;
	CFont* font = GetFont();
	GetWindowText(c);
	GetClientRect(&r);

	dc.SetTextColor(GetTxtColor());
	dc.SetTextAlign(TA_CENTER);
	dc.SetBkMode(TRANSPARENT);
	dc.SelectObject(font);

	LOGBRUSH b;
	b.lbColor = GetSysColor(COLOR_BTNFACE);
	b.lbStyle = BS_SOLID;
	HBRUSH brush = CreateBrushIndirect(&b);

	LOGPEN p;
	p.lopnColor = 0;
	p.lopnStyle = PS_NULL;
	HPEN pen = CreatePenIndirect(&p);

	dc.SelectObject(brush);
	dc.SelectObject(pen);
	dc.Rectangle(&r);

	TextOut(dc,(r.left+r.right)>>1,1,c,strlen(c));

	DeleteObject(brush);
	DeleteObject(pen);
	ReleaseDC(&dc);

	
	// Kein Aufruf von CEdit::OnPaint() für Zeichnungsnachrichten
}

void CUserDrawEdit::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct) 
{
	// TODO: Code für die Behandlungsroutine für Nachrichten hier einfügen und/oder Standard aufrufen
	
	SetTextColor(lpDrawItemStruct->hDC,255);

	CEdit::OnDrawItem(nIDCtl, lpDrawItemStruct);
}
