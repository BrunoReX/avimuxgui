// EnhancedListBox.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "AVIMux_GUI.h"
#include "EnhancedListBox.h"
#include ".\enhancedlistbox.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEnhancedListBox

CEnhancedListBox::CEnhancedListBox()
{
	EnableAutomation();
	bMButtonDown=false;
	ptMButtonDown.x=0;
	ptMButtonDown.y=0;
	bMovingAllowed=true;
	iItemMButtonDown=0;
}

CEnhancedListBox::~CEnhancedListBox()
{
}

void CEnhancedListBox::OnFinalRelease()
{
	// Nachdem die letzte Referenz auf ein Automatisierungsobjekt freigegeben wurde,
	// wird OnFinalRelease aufgerufen. Die Basisklasse löscht das Objekt
	// automatisch. Fügen Sie zusätzlichen Bereinigungscode für Ihr Objekt
	// hinzu, bevor Sie die Basisklasse aufrufen.

	CListBox::OnFinalRelease();
}


BEGIN_MESSAGE_MAP(CEnhancedListBox, CListBox)
	//{{AFX_MSG_MAP(CEnhancedListBox)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_MBUTTONDOWN()
	ON_WM_MBUTTONUP()
	//}}AFX_MSG_MAP
	ON_WM_CHAR()
	ON_WM_GETDLGCODE()
	ON_WM_KEYUP()
	ON_WM_KEYDOWN()
END_MESSAGE_MAP()

BEGIN_DISPATCH_MAP(CEnhancedListBox, CListBox)
	//{{AFX_DISPATCH_MAP(CEnhancedListBox)
		// HINWEIS - Der Klassen-Assistent fügt hier Zuordnungsmakros ein und entfernt diese.
	//}}AFX_DISPATCH_MAP
END_DISPATCH_MAP()

// Hinweis: Wir stellen Unterstützung für IID_IEnhancedListBox zur Verfügung, um typsicheres Binden
//  von VBA zu ermöglichen. Diese IID muss mit der GUID übereinstimmen, die in der
//  Disp-Schnittstelle in der .ODL-Datei angegeben ist.

// {7566FCBE-39C6-441A-9D51-E627E440BBA2}
static const IID IID_IEnhancedListBox =
{ 0x7566fcbe, 0x39c6, 0x441a, { 0x9d, 0x51, 0xe6, 0x27, 0xe4, 0x40, 0xbb, 0xa2 } };

BEGIN_INTERFACE_MAP(CEnhancedListBox, CListBox)
	INTERFACE_PART(CEnhancedListBox, IID_IEnhancedListBox, Dispatch)
END_INTERFACE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CEnhancedListBox 

void CEnhancedListBox::OnLButtonDown(UINT nFlags, CPoint point) 
{
	// TODO: Code für die Behandlungsroutine für Nachrichten hier einfügen und/oder Standard aufrufen
	if (nFlags & MK_SHIFT) {
		bMButtonDown=true;
	} else {
		CListBox::OnLButtonDown(nFlags, point);
	}
}

void CEnhancedListBox::ItemDown(void)
{
	DWORD	dwSelCount,dwICount,dwItemData;
	DWORD*	lpdwSel;
	int		i,iIndex;
	char*	lpcText = NULL;
	bool	bHasStrings = !! (GetStyle() & LBS_HASSTRINGS);
		

	dwSelCount=GetSelCount();
	dwICount=GetCount();
	if (dwSelCount)	{
		lpdwSel=(DWORD*)malloc(4*dwSelCount);
		GetSelItems(dwSelCount,(int*)lpdwSel);
		for (i=dwSelCount-1;i>=0;i--) {
			if (lpdwSel[i]!=dwICount-1 && CanMoveTo(lpdwSel[i], 1))	{
				if (bHasStrings) {
					lpcText=(char*)calloc(1, 1+GetTextLen(lpdwSel[i]));
					GetText(lpdwSel[i],lpcText);
				}
				dwItemData=GetItemData(lpdwSel[i]);
				DeleteString(lpdwSel[i]);
				iIndex=InsertString(lpdwSel[i]+1,lpcText);
				SetItemData(iIndex,dwItemData);
				SetSel(iIndex,true);
				if (lpcText)
					free(lpcText);
			}
		}
		free(lpdwSel);
	}
}

bool CEnhancedListBox::CanMoveTo(int i, int direction)
{
	return true;
}

void CEnhancedListBox::ItemUp(void)
{
	DWORD	dwSelCount,dwICount;
	DWORD*	lpdwSel;
	DWORD	i,iIndex;
	char*	lpcText = NULL;
	DWORD	dwItemData;
	bool	bHasStrings = !! (GetStyle() & LBS_HASSTRINGS);

	dwSelCount=GetSelCount();
	dwICount=GetCount();
	if (dwSelCount) {
		lpdwSel=(DWORD*)malloc(4*dwSelCount);
		GetSelItems(dwSelCount,(int*)lpdwSel);
		for (i=0;i<dwSelCount;i++) {
			if (lpdwSel[i] && CanMoveTo(lpdwSel[i], -1)) {
				if (bHasStrings) {
					lpcText=(char*)calloc(1, 1 + GetTextLen(lpdwSel[i]));
					GetText(lpdwSel[i], lpcText);
				}
				dwItemData=GetItemData(lpdwSel[i]);
				DeleteString(lpdwSel[i]);
				
				iIndex=InsertString(lpdwSel[i]-1,lpcText);
				SetItemData(iIndex,dwItemData);
				SetSel(iIndex,true);
				if (lpcText)
					free(lpcText);
			}
		}
		free(lpdwSel);
	}
}

void CEnhancedListBox::OnLButtonUp(UINT nFlags, CPoint point) 
{
	// TODO: Code für die Behandlungsroutine für Nachrichten hier einfügen und/oder Standard aufrufen
	bMButtonDown=false;
	CListBox::OnLButtonUp(nFlags, point);
}

void CEnhancedListBox::AllowMoving(bool bAllowMoving)
{
	bMovingAllowed=bAllowMoving;
}

void CEnhancedListBox::RedoNumbering()
{
}

void CEnhancedListBox::OnMouseMove(UINT nFlags, CPoint point) 
{
	// TODO: Code für die Behandlungsroutine für Nachrichten hier einfügen und/oder Standard aufrufen
	int	iOutside;

	if (bMButtonDown)
	{
		int	i=ItemFromPoint(point,iOutside);
		if (bMovingAllowed)	{
			if (i>iItemMButtonDown)	{
				ItemDown();
				RedoNumbering();
			}
			if (i<iItemMButtonDown)	{
				ItemUp();
				RedoNumbering();
			}
			iItemMButtonDown=i;
		} else {
			CListBox::OnMouseMove(nFlags, ptMButtonDown);
		}
	} else {
		CListBox::OnMouseMove(nFlags, point);
	}
}

void CEnhancedListBox::OnMButtonDown(UINT nFlags, CPoint point) 
{
	// TODO: Code für die Behandlungsroutine für Nachrichten hier einfügen und/oder Standard aufrufen
	int 	iOutside;

	iItemMButtonDown=ItemFromPoint(point,iOutside);
	ptMButtonDown=point;
	bMButtonDown=true;
}

void CEnhancedListBox::OnMButtonUp(UINT nFlags, CPoint point) 
{
	// TODO: Code für die Behandlungsroutine für Nachrichten hier einfügen und/oder Standard aufrufen
	bMButtonDown=false;
		
}

void CEnhancedListBox::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: Fügen Sie hier Ihren Meldungsbehandlungscode ein, und/oder benutzen Sie den Standard.
	__super::OnChar(nChar, nRepCnt, nFlags);
}

UINT CEnhancedListBox::OnGetDlgCode()
{
	// TODO: Fügen Sie hier Ihren Meldungsbehandlungscode ein, und/oder benutzen Sie den Standard.

	return __super::OnGetDlgCode() | DLGC_WANTALLKEYS;
}

void CEnhancedListBox::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: Fügen Sie hier Ihren Meldungsbehandlungscode ein, und/oder benutzen Sie den Standard.
/*	if (nChar == VK_PRIOR) {
		ItemUp();
		RedoNumbering();
	} else
	if (nChar == VK_NEXT) {
		ItemDown();
		RedoNumbering();
	} 
*/
	__super::OnKeyUp(nChar, nRepCnt, nFlags);
}

void CEnhancedListBox::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: Fügen Sie hier Ihren Meldungsbehandlungscode ein, und/oder benutzen Sie den Standard.
	if (nChar == VK_PRIOR) {
		ItemUp();
		RedoNumbering();
	} else
	if (nChar == VK_NEXT) {
		ItemDown();
		RedoNumbering();
	}  else

	__super::OnKeyDown(nChar, nRepCnt, nFlags);
}
