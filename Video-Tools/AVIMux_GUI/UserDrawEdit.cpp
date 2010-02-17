// UserDrawEdit.cpp: Implementierungsdatei
//
// Supports a function call which returns the color to be used for the text as result
//

#include "stdafx.h"
#include "AVIMux_GUI.h"
#include "UserDrawEdit.h"
#include "..\utf-8.h"
#include "..\UnicodeCalls.h"
#include "osversion.h"
#include <vector>

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
	bHasFocus = false;
	iTextAlignment = TA_LEFT;
	color = NULL;
	clrDisabled = GetSysColor(COLOR_GRAYTEXT);

	InitUnicode(DoesOSSupportUnicode());
	
}

void CUserDrawEdit::SetDisabledTextColor(COLORREF color)
{
	clrDisabled = color;
}

void CUserDrawEdit::SetTextAlign(int align)
{
	iTextAlignment = align;
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
	ON_WM_KILLFOCUS()
	ON_WM_SETFOCUS()
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
	return color;
}

void CUserDrawEdit::SetColor(COLORREF c)
{
	color = c;
}

void CUserDrawEdit::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

	RECT r;
	CFont* font = GetFont();
	
	int textSize = GetWindowTextLength();
	std::vector<TCHAR> text(textSize+1);
	GetWindowText(&text[0], textSize+1);

	COLORREF backcolor, textcolor;

	if (IsWindowEnabled()) {
		backcolor = GetSysColor(COLOR_WINDOW);
		textcolor = GetTxtColor();
	} else { 
		backcolor = GetSysColor(COLOR_BTNFACE);
		textcolor = clrDisabled;
	}

	GetClientRect(&r);
	r.right++; r.bottom++;

	dc.SetTextColor(textcolor);
	dc.SetTextAlign(iTextAlignment);

	dc.SetBkMode(OPAQUE);
	dc.SelectObject(font);

	LOGBRUSH b;
	b.lbColor = backcolor;
	b.lbStyle = BS_SOLID;
	HBRUSH brush = CreateBrushIndirect(&b);

	LOGPEN p;
	p.lopnColor = 0;
	p.lopnStyle = PS_NULL;
	HPEN pen = CreatePenIndirect(&p);

	dc.SelectObject(brush);
	dc.SelectObject(pen);
	dc.Rectangle(&r);

	dc.SetBkColor(backcolor);
	
	if (dc.GetTextAlign() == TA_CENTER)
		ExtTextOut(dc, (r.left+r.right)>>1, 1, ETO_CLIPPED, &r, &text[0], textSize, NULL);
	else
		ExtTextOut(dc, r.left, 1, ETO_CLIPPED, &r, &text[0], textSize, NULL);

	DeleteObject(brush);
	DeleteObject(pen);
	ReleaseDC(&dc);
}

void CUserDrawEdit::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct) 
{
	// TODO: Code für die Behandlungsroutine für Nachrichten hier einfügen und/oder Standard aufrufen
	
	SetTextColor(lpDrawItemStruct->hDC,255);

	CEdit::OnDrawItem(nIDCtl, lpDrawItemStruct);
}

void CUserDrawEdit::OnKillFocus(CWnd* pNewWnd) 
{
	bHasFocus = false;

	CEdit::OnKillFocus(pNewWnd);

	InvalidateRect(NULL);
	UpdateWindow();
}

void CUserDrawEdit::OnSetFocus(CWnd* pOldWnd) 
{
	CEdit::OnSetFocus(pOldWnd);

	// TODO: Code für die Behandlungsroutine für Nachrichten hier einfügen
	bHasFocus = true;	
	InvalidateRect(NULL);
	UpdateWindow();
}
