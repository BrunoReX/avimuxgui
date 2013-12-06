// ProgressList.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "AVIMux_GUI.h"
#include "ProgressList.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CProgressList

CProgressList::CProgressList()
{
	EnableAutomation();
}

CProgressList::~CProgressList()
{
}

void CProgressList::OnFinalRelease()
{
	// Nachdem die letzte Referenz auf ein Automatisierungsobjekt freigegeben wurde,
	// wird OnFinalRelease aufgerufen. Die Basisklasse l�scht das Objekt
	// automatisch. F�gen Sie zus�tzlichen Bereinigungscode f�r Ihr Objekt
	// hinzu, bevor Sie die Basisklasse aufrufen.

	CListCtrl::OnFinalRelease();
}


BEGIN_MESSAGE_MAP(CProgressList, CListCtrl)
	//{{AFX_MSG_MAP(CProgressList)
	ON_WM_RBUTTONUP()
	ON_WM_CONTEXTMENU()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BEGIN_DISPATCH_MAP(CProgressList, CListCtrl)
	//{{AFX_DISPATCH_MAP(CProgressList)
		// HINWEIS - Der Klassen-Assistent f�gt hier Zuordnungsmakros ein und entfernt diese.
	//}}AFX_DISPATCH_MAP
END_DISPATCH_MAP()

// Hinweis: Wir stellen Unterst�tzung f�r IID_IProgressList zur Verf�gung, um typsicheres Binden
//  von VBA zu erm�glichen. Diese IID muss mit der GUID �bereinstimmen, die in der
//  Disp-Schnittstelle in der .ODL-Datei angegeben ist.

// {A6FDB251-2995-4EAB-A9FC-BE58FDC112B3}
static const IID IID_IProgressList =
{ 0xa6fdb251, 0x2995, 0x4eab, { 0xa9, 0xfc, 0xbe, 0x58, 0xfd, 0xc1, 0x12, 0xb3 } };

BEGIN_INTERFACE_MAP(CProgressList, CListCtrl)
	INTERFACE_PART(CProgressList, IID_IProgressList, Dispatch)
END_INTERFACE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen f�r Nachrichten CProgressList 

void CProgressList::OnRButtonUp(UINT nFlags, CPoint point) 
{
	// TODO: Code f�r die Behandlungsroutine f�r Nachrichten hier einf�gen und/oder Standard aufrufen
	CMenu*		cmPopupMenu;
	CString		cStr;
	bool		bItemPresent=false;

	cmPopupMenu=new CMenu;
	cmPopupMenu->CreatePopupMenu();
			
//	if (bItemPresent) cmPopupMenu->AppendMenu(MF_SEPARATOR,0);
	cStr="Byte-Genauigkeit"; 
	cmPopupMenu->AppendMenu(MF_STRING,IDM_BYTEACCURACY,cStr);
	bItemPresent=true;

	ClientToScreen(&point);
	cmPopupMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON,point.x,point.y,this);
	delete cmPopupMenu;
	
	CListCtrl::OnRButtonUp(nFlags, point);
}

void CProgressList::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	// TODO: Code f�r die Behandlungsroutine f�r Nachrichten hier einf�gen
	CMenu*		cmPopupMenu;
	CString		cStr;
	bool		bItemPresent=false;

	cmPopupMenu=new CMenu;
	cmPopupMenu->CreatePopupMenu();
			
	if (GetAccuracy()==PRAC_SCALED) {
		cStr="Byte-Genauigkeit"; 
	} else cStr="skaliert";
	cmPopupMenu->AppendMenu(MF_STRING,IDM_BYTEACCURACY,cStr);
	bItemPresent=true;

	cmPopupMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON,point.x,point.y,this);
	delete cmPopupMenu;
	
//	CListCtrl::OnRButtonUp(nFlags, point);	
}

int CProgressList::GetAccuracy(void)
{
	return dwAccuracy;
}

void CProgressList::SetAccuracy(DWORD _dwAccuracy)
{
	dwAccuracy=_dwAccuracy;
}

BOOL CProgressList::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	// TODO: Speziellen Code hier einf�gen und/oder Basisklasse aufrufen
	switch (LOWORD(wParam))
	{
		case IDM_BYTEACCURACY:
			if (GetAccuracy()==PRAC_BYTES) {
				SetAccuracy(PRAC_SCALED) ;
			} else SetAccuracy(PRAC_BYTES);
			break;
	}
			
	return CListCtrl::OnCommand(wParam, lParam);
}
