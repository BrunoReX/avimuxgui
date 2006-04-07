// UnicodeTreeCtrl.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "AVIMux_GUI.h"
#include "UnicodeTreeCtrl.h"
#include "..\utf-8.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CUnicodeTreeCtrl


CUnicodeTreeCtrl::CUnicodeTreeCtrl()
{
	EnableAutomation();
}

CUnicodeTreeCtrl::~CUnicodeTreeCtrl()
{
}

void CUnicodeTreeCtrl::OnFinalRelease()
{
	// Nachdem die letzte Referenz auf ein Automatisierungsobjekt freigegeben wurde,
	// wird OnFinalRelease aufgerufen. Die Basisklasse löscht das Objekt
	// automatisch. Fügen Sie zusätzlichen Bereinigungscode für Ihr Objekt
	// hinzu, bevor Sie die Basisklasse aufrufen.

	CTreeCtrl::OnFinalRelease();
}

void CUnicodeTreeCtrl::InitUnicode()
{
	SendMessage(TVM_SETUNICODEFORMAT, utf8_IsUnicodeEnabled());

	CUnicodeBase::InitUnicode(utf8_IsUnicodeEnabled());
}


BEGIN_MESSAGE_MAP(CUnicodeTreeCtrl, CTreeCtrl)
	//{{AFX_MSG_MAP(CUnicodeTreeCtrl)
	ON_NOTIFY_REFLECT(TVN_GETDISPINFO, OnGetdispinfo)
	ON_WM_KEYDOWN()
	ON_NOTIFY_REFLECT(TVN_GETDISPINFOW, OnGetdispinfo)
	ON_NOTIFY_REFLECT(TVN_KEYDOWN, OnKeydown)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BEGIN_DISPATCH_MAP(CUnicodeTreeCtrl, CTreeCtrl)
	//{{AFX_DISPATCH_MAP(CUnicodeTreeCtrl)
		// HINWEIS - Der Klassen-Assistent fügt hier Zuordnungsmakros ein und entfernt diese.
	//}}AFX_DISPATCH_MAP
END_DISPATCH_MAP()

// Hinweis: Wir stellen Unterstützung für IID_IUnicodeTreeCtrl zur Verfügung, um typsicheres Binden
//  von VBA zu ermöglichen. Diese IID muss mit der GUID übereinstimmen, die in der
//  Disp-Schnittstelle in der .ODL-Datei angegeben ist.

// {E4D57A18-5B58-47AC-92B6-05466DA06BAB}
static const IID IID_IUnicodeTreeCtrl =
{ 0xe4d57a18, 0x5b58, 0x47ac, { 0x92, 0xb6, 0x5, 0x46, 0x6d, 0xa0, 0x6b, 0xab } };

BEGIN_INTERFACE_MAP(CUnicodeTreeCtrl, CTreeCtrl)
	INTERFACE_PART(CUnicodeTreeCtrl, IID_IUnicodeTreeCtrl, Dispatch)
END_INTERFACE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CUnicodeTreeCtrl 

void CUnicodeTreeCtrl::GetTextCallback(NMHDR* pNMHDR, LRESULT* pResult)
{
	TV_DISPINFO* pDispInfo = (TV_DISPINFO*)pNMHDR;
	TVITEM* item = &pDispInfo->item;
	char* dest = item->pszText;  // <= write the output string here in UTF-8

	
	*pResult = 0;

}

void CUnicodeTreeCtrl::OnGetdispinfo(NMHDR* pNMHDR, LRESULT* pResult) 
{
	TV_DISPINFO* pTVDispInfo = (TV_DISPINFO*)pNMHDR;
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen

	HTREEITEM hItem = pTVDispInfo->item.hItem;
	TVITEM* pItem = &pTVDispInfo->item;
	UNICODETREEITEM_DATA* data = (UNICODETREEITEM_DATA*)CTreeCtrl::GetItemData(hItem);
	char* cDest = pItem->pszText;

	if ((pItem->mask & TVIF_TEXT) == TVIF_TEXT) {
		
		if (data) if (data->cText != LPSTR_TEXTCALLBACK) {
			fromUTF8(data->cText, cDest);		
		} else {
			GetTextCallback(pNMHDR, pResult);
			fromUTF8(cDest, cDest);

		}
	}	

	*pResult = 0;
}


HTREEITEM CUnicodeTreeCtrl::InsertItem(LPTVINSERTSTRUCT lpInsertStruct)
{
	TVITEM* pItem = &lpInsertStruct->item;
	UNICODETREEITEM_DATA* data = new UNICODETREEITEM_DATA;
//	data->cText = NULL;
//	data->dwUserData = 0;
	//data->bAllocated = 0;
	ZeroMemory(data,sizeof(*data));
	HTREEITEM r;

	char* c = pItem->pszText;

	if (c != LPSTR_TEXTCALLBACK) {
		data->cText = new char[1+strlen(c)];
		data->bAllocated = 1;
		strcpy(data->cText, c);
	} else {
		data->cText = c;
	}

	pItem->pszText = LPSTR_TEXTCALLBACK;
	
	CTreeCtrl::SetItemData(r=CTreeCtrl::InsertItem(lpInsertStruct), (DWORD)data);
	return r;
}

void CUnicodeTreeCtrl::SetItem(TVITEM* pItem)
{
	UNICODETREEITEM_DATA* data = (UNICODETREEITEM_DATA*)CTreeCtrl::GetItemData(pItem->hItem);

	if ((pItem->mask & TVIF_TEXT) == TVIF_TEXT) {
		if (data->bAllocated) {
			delete data->cText;
			data->bAllocated = 0;
			data->cText = NULL;
		}

		if (pItem->pszText != LPSTR_TEXTCALLBACK) {
			data->cText = new char[1+strlen(pItem->pszText)];
			strcpy(data->cText, pItem->pszText);
			data->bAllocated = 1;
		} else {
			data->cText = pItem->pszText;
			data->bAllocated = 0;
		}
	}

	if ((pItem->mask & TVIF_PARAM) == TVIF_PARAM) {
		data->dwUserData = pItem->lParam;
	} ; //else data->dwUserData = 0;

	pItem->mask &=~ (TVIF_PARAM | TVIF_TEXT);
	CTreeCtrl::SetItem(pItem);
}

void CUnicodeTreeCtrl::ShowItemCheckBox(HTREEITEM hItem, bool bShow)
{
    TVITEM tvItem;

    tvItem.mask = TVIF_HANDLE | TVIF_STATE;
    tvItem.hItem = hItem;
    tvItem.stateMask = TVIS_STATEIMAGEMASK;

    /*
    Since state images are one-based, 1 in this macro turns the check off, and 
    2 turns it on.
    */
    tvItem.state = INDEXTOSTATEIMAGEMASK((bShow ? 1 : 0));

    SetItem(&tvItem);
	return;
}

void CUnicodeTreeCtrl::SetItemData(HTREEITEM hItem, DWORD dwData)
{
	UNICODETREEITEM_DATA* data = (UNICODETREEITEM_DATA*)CTreeCtrl::GetItemData(hItem);
	
	if (data) { 
		data->dwUserData = dwData;
	}

	RECT r;
	GetItemRect(hItem, &r, false);
	InvalidateRect(&r);
}

DWORD CUnicodeTreeCtrl::GetItemData(HTREEITEM hItem)
{
	if (!hItem) return NULL;
	UNICODETREEITEM_DATA* data = (UNICODETREEITEM_DATA*)CTreeCtrl::GetItemData(hItem);

	if (data) {
		return data->dwUserData;
	} else {
		return NULL;
	}
}

bool CUnicodeTreeCtrl::DeleteItem(HTREEITEM hItem)
{
	UNICODETREEITEM_DATA* data = (UNICODETREEITEM_DATA*)CTreeCtrl::GetItemData(hItem);
	HTREEITEM hChild; 

	if (hChild = GetChildItem(hItem)) {
		DeleteAllItems(hChild);
	}

	if (data && data->bAllocated) {
		delete data->cText;
		data->bAllocated = 0;
		data->cText = NULL;
	}

	CTreeCtrl::DeleteItem(hItem);

	if (data) delete data;

	return true;
}

char* CUnicodeTreeCtrl::GetItemText(HTREEITEM hItem)
{
	UNICODETREEITEM_DATA* data = (UNICODETREEITEM_DATA*)CTreeCtrl::GetItemData(hItem);

	if (data->cText != LPSTR_TEXTCALLBACK) return data->cText;
	return NULL;
}

bool CUnicodeTreeCtrl::DeleteAllItems(HTREEITEM hRoot)
{
	if (!hRoot) {
		hRoot = GetRootItem();
		if (!hRoot) return false;
	}

	HTREEITEM hCurrent, hNext;

	hCurrent = hRoot;

	do {
		hNext = GetNextSiblingItem(hCurrent);
		DeleteItem(hCurrent);
		hCurrent = hNext;		
	} while (hNext);

	return true;
}

void CUnicodeTreeCtrl::OnKeydown(NMHDR* pNMHDR, LRESULT* pResult) 
{
	TV_KEYDOWN* pTVKeyDown = (TV_KEYDOWN*)pNMHDR;
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen

	char nChar = (char)(pTVKeyDown->wVKey);
	if (nChar == VK_F2 && GetSelectedItem()) {
		PostMessage(TVM_EDITLABEL, 0, (LPARAM)GetSelectedItem());
	}

	
	*pResult = 0;
}

LRESULT CUnicodeTreeCtrl::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	// TODO: Speziellen Code hier einfügen und/oder Basisklasse aufrufen
	switch (message) {
		case WM_RBUTTONDOWN:
			b_rdown = 1;
			mouse_x = LOWORD(lParam);
			mouse_y = HIWORD(lParam);
			break;
	}

	return CTreeCtrl::WindowProc(message, wParam, lParam);
}

int CUnicodeTreeCtrl::GetMouseX()
{
	return mouse_x;
}

int CUnicodeTreeCtrl::GetMouseY()
{
	return mouse_y;
}

int CUnicodeTreeCtrl::GetRButtonDown()
{
	return b_rdown;
}

int CUnicodeTreeCtrl::RenderItem(HTREEITEM hItem, char* cDest, int iDepth)
{
	char* cText = cDest;
	int isize = 0;

	if (hItem) {
		for (int i=0;i<iDepth;i++) {
			isize+=2;
			*cText++=32;*cText++=32;
		}
	
		UNICODETREEITEM_DATA* uti = (UNICODETREEITEM_DATA*)CTreeCtrl::GetItemData(hItem);
		if (uti->cText) {
			strcpy(cText, uti->cText);
			isize+=strlen(uti->cText);
		} else {
			CString s = GetItemText(hItem);
			strcpy(cText, s);
			isize+=strlen(s);
		}
		cText = cText + strlen(cText);
		*cText++ = 13;
		*cText++ = 10;
		isize+=2;
		isize+=RenderItem(GetChildItem(hItem), cText, iDepth+1);
		cText += strlen(cText);
		isize+=RenderItem(GetNextSiblingItem(hItem), cText, iDepth);
	}


	return isize;
}

int CUnicodeTreeCtrl::Render2Buffer(char* cDest)
{
	HTREEITEM hItem = GetRootItem();

	*cDest++ = (char)0xEF;
	*cDest++ = (char)0xBB;
	*cDest++ = (char)0xBF;

	return RenderItem(hItem, cDest, 0);;
}
