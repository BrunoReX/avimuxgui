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
	mouse_x = 0;
	mouse_y = 0;
	b_rdown = 0;
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
	SendMessage(CCM_SETUNICODEFORMAT, utf8_IsUnicodeEnabled());
	int unicode = SendMessage(CCM_GETUNICODEFORMAT);
	printf("Unicode flag: %d\n", unicode);
	CUnicodeBase::InitUnicode(unicode);
	SendMessage(CCM_SETUNICODEFORMAT, IsUnicode());
}


BEGIN_MESSAGE_MAP(CUnicodeTreeCtrl, CTreeCtrl)
	//{{AFX_MSG_MAP(CUnicodeTreeCtrl)
	ON_NOTIFY_REFLECT(TVN_GETDISPINFO, OnGetdispinfo)
	ON_NOTIFY_REFLECT(TVN_GETDISPINFOW, OnGetdispinfo)
	ON_NOTIFY_REFLECT(TVN_KEYDOWN, OnKeydown)
	ON_NOTIFY_REFLECT(NM_RETURN, OnReturn)
	ON_WM_DESTROY()
	ON_WM_CREATE()
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
/*	TV_DISPINFO* pDispInfo = (TV_DISPINFO*)pNMHDR;
	HTREEITEM hItem = pDispInfo->item.hItem;
	TVITEM* item = &pDispInfo->item;
	UNICODETREEITEM_DATA* data = (UNICODETREEITEM_DATA*)CTreeCtrl::GetItemData(hItem);
	char* dest = item->pszText;  // <= write the output string here in UTF-8
*/	
//	strcpy(dest, data->cText); 

	*pResult = 0;
}

HTREEITEM CUnicodeTreeCtrl::GetTopMostParentItem(HTREEITEM hItem)
{
	HTREEITEM h = hItem;

	while (GetParentItem(h)) h = GetParentItem(h);

	return h;
}

void CUnicodeTreeCtrl::OnGetdispinfo(NMHDR* pNMHDR, LRESULT* pResult) 
{
	if (!pNMHDR) {
		*pResult = 0;
		return;
	}
		
	TV_DISPINFO* pTVDispInfo = (TV_DISPINFO*)pNMHDR;
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen

	HTREEITEM hItem = pTVDispInfo->item.hItem;

	if (!hItem) {
		*pResult = 0;
		return;
	}

	TVITEM* pItem = &pTVDispInfo->item;
	UNICODETREEITEM_DATA* data = (UNICODETREEITEM_DATA*)CTreeCtrl::GetItemData(hItem);
	
	if (pItem) {
		char* cDest = reinterpret_cast<char*>(pItem->pszText);
		if (cDest && ((pItem->mask & TVIF_TEXT) == TVIF_TEXT)) {
			if (data) {
				if (data->cText != LPSTR_TEXTCALLBACKA) {
					memset(cDest, 0, pItem->cchTextMax * (IsUnicode()?2:1));

					/* the following utf8->target conversion seems to b0rk under
					   certain conditions, i have no idea why. The problem MIGHT
					   have been that MultiByteToWideChar got 1+strlen(source) as
					   size of the input buffer */
					__try {
						int j = fromUTF8(data->cText, cDest, pItem->cchTextMax * (IsUnicode()?2:1));
					} __except(EXCEPTION_EXECUTE_HANDLER) {
						DWORD code = GetExceptionCode();
						char c[64]; sprintf(c, "b0rked at %08X", cDest);
						fromUTF8(c, cDest, 38); 
					}
					
				} else {
					GetTextCallback(pNMHDR, pResult);
					fromUTF8(cDest, cDest, pItem->cchTextMax * (IsUnicode()?2:1));
				}
			} else {
				*cDest = 0;
			}
		}
	}	

	*pResult = 0;
}


HTREEITEM CUnicodeTreeCtrl::InsertItem(LPTVINSERTSTRUCTA lpInsertStruct)
{
	TVITEMA* pItem = &lpInsertStruct->item;
	UNICODETREEITEM_DATA* data = new UNICODETREEITEM_DATA;

	ZeroMemory(data,sizeof(*data));
	HTREEITEM r;

	if (pItem->mask & TVIF_TEXT) {
		char* c = pItem->pszText;

		if (!c) 
			c = "";

		if (c != LPSTR_TEXTCALLBACKA) {
			CUTF8 utf8Text(c);
			size_t utf8TextLen = strlen(utf8Text.UTF8());
			data->cText = new char[1 + utf8TextLen];
			strcpy_s(data->cText, 1+utf8TextLen, utf8Text.UTF8());
			data->cText[utf8TextLen] = 0;
			data->bAllocated = 1;
			/*size_t slen = strlen(c);
			size_t passed_len = pItem->cchTextMax;
			size_t len = min(slen, passed_len);
			_ASSERT(passed_len == slen);
				
			data->cText = new char[1+len];
			data->bAllocated = 1;
			memcpy(data->cText, c, len);
			data->cText[len] = 0;
			pItem->cchTextMax = len;*/
		} else {
			data->cText = c;
		}

		pItem->pszText = LPSTR_TEXTCALLBACKA;
	}
	
#ifndef _UNICODE
	CTreeCtrl::SetItemData(r=CTreeCtrl::InsertItem(lpInsertStruct), (DWORD)data);
#else
	TVINSERTSTRUCTW newStruct;
	newStruct.hParent = lpInsertStruct->hParent;
	newStruct.hInsertAfter = lpInsertStruct->hInsertAfter;
	newStruct.itemex.cChildren = lpInsertStruct->itemex.cChildren;
	newStruct.itemex.cchTextMax = lpInsertStruct->itemex.cchTextMax;
	newStruct.itemex.hItem = lpInsertStruct->itemex.hItem;
	newStruct.itemex.iImage = lpInsertStruct->itemex.iImage;
	newStruct.itemex.iIntegral = lpInsertStruct->itemex.iIntegral;
	newStruct.itemex.iSelectedImage = lpInsertStruct->itemex.iSelectedImage;
	newStruct.itemex.lParam = lpInsertStruct->itemex.lParam;
	newStruct.itemex.mask = lpInsertStruct->itemex.mask;
	newStruct.itemex.state = lpInsertStruct->itemex.state;
	newStruct.itemex.stateMask = lpInsertStruct->itemex.stateMask;
	newStruct.itemex.pszText = LPSTR_TEXTCALLBACKW;
	CTreeCtrl::SetItemData(r=CTreeCtrl::InsertItem(&newStruct), (DWORD)data);
#endif
	return r;
}

HTREEITEM CUnicodeTreeCtrl::InsertItem(LPTVINSERTSTRUCTW lpInsertStruct)
{
	TVITEMW* pItem = &lpInsertStruct->item;
	UNICODETREEITEM_DATA* data = new UNICODETREEITEM_DATA;

	ZeroMemory(data,sizeof(*data));
	HTREEITEM r;

	if (pItem->mask & TVIF_TEXT) {
		wchar_t* c = pItem->pszText;

		if (!c) 
			c = L"";

		if (c != LPSTR_TEXTCALLBACKW) {
			CUTF8 utf8Text(c);
			size_t utf8TextLen = strlen(utf8Text.UTF8());
			data->cText = new char[1 + utf8TextLen];
			strcpy_s(data->cText, 1+utf8TextLen, utf8Text.UTF8());
			data->cText[utf8TextLen] = 0;
			data->bAllocated = 1;
			/*size_t slen = strlen(c);
			size_t passed_len = pItem->cchTextMax;
			size_t len = min(slen, passed_len);
			_ASSERT(passed_len == slen);
				
			data->cText = new char[1+len];
			data->bAllocated = 1;
			memcpy(data->cText, c, len);
			data->cText[len] = 0;
			pItem->cchTextMax = len;*/
		} else {
			data->cText = LPSTR_TEXTCALLBACKA;
		}

		pItem->pszText = LPSTR_TEXTCALLBACKW;
	}
	
#ifdef _UNICODE
	CTreeCtrl::SetItemData(r=CTreeCtrl::InsertItem(lpInsertStruct), (DWORD)data);
#else
	TVINSERTSTRUCTA newStruct;
	newStruct.hParent = lpInsertStruct->hParent;
	newStruct.hInsertAfter = lpInsertStruct->hInsertAfter;
	newStruct.itemex.cChildren = lpInsertStruct->itemex.cChildren;
	newStruct.itemex.cchTextMax = lpInsertStruct->itemex.cchTextMax;
	newStruct.itemex.hItem = lpInsertStruct->itemex.hItem;
	newStruct.itemex.iImage = lpInsertStruct->itemex.iImage;
	newStruct.itemex.iIntegral = lpInsertStruct->itemex.iIntegral;
	newStruct.itemex.iSelectedImage = lpInsertStruct->itemex.iSelectedImage;
	newStruct.itemex.lParam = lpInsertStruct->itemex.lParam;
	newStruct.itemex.mask = lpInsertStruct->itemex.mask;
	newStruct.itemex.state = lpInsertStruct->itemex.state;
	newStruct.itemex.stateMask = lpInsertStruct->itemex.stateMask;
	newStruct.itemex.pszText = LPSTR_TEXTCALLBACKA;
	CTreeCtrl::SetItemData(r=CTreeCtrl::InsertItem(&newStruct), (DWORD)data);
#endif
	return r;
}

void CUnicodeTreeCtrl::SetItem(TVITEMA* pItem)
{
	UNICODETREEITEM_DATA* data = (UNICODETREEITEM_DATA*)CTreeCtrl::GetItemData(pItem->hItem);

	if ((pItem->mask & TVIF_TEXT) == TVIF_TEXT) {
		if (data->bAllocated) {
			delete[] data->cText;
			data->bAllocated = 0;
			data->cText = NULL;
		}

		if (pItem->pszText != LPSTR_TEXTCALLBACKA) {
			CUTF8 utf8NewText(pItem->pszText);
			int utf8TextLength = utf8NewText.Size();
			data->cText = new char[1 + utf8TextLength];
			strcpy_s(data->cText, utf8TextLength, utf8NewText.UTF8());
			data->cText[utf8TextLength] = 0;
			data->bAllocated = 1;

			//int len = min(strlen(pItem->pszText), pItem->cchTextMax);
			//data->cText = new char[len+1];
			
			//strncpy(data->cText, pItem->pszText, len);
			//data->cText[len] = 0;
			//data->bAllocated = 1;
		} else {
			data->cText = pItem->pszText;
			data->bAllocated = 0;
		}

		pItem->pszText = LPSTR_TEXTCALLBACKA;
	}

	if ((pItem->mask & TVIF_PARAM) == TVIF_PARAM) {
		data->dwUserData = pItem->lParam;
	}; 

	pItem->mask &=~ (TVIF_PARAM /*| TVIF_TEXT*/);

#ifdef _UNICODE
	// TVITEMA and TVITEMW have same size
	CTreeCtrl::SetItem(reinterpret_cast<TVITEMW*>(pItem));
#else
	CTreeCtrl::SetItem(pItem);
#endif
}

void CUnicodeTreeCtrl::SetItem(TVITEMW* pItem)
{
	UNICODETREEITEM_DATA* data = (UNICODETREEITEM_DATA*)CTreeCtrl::GetItemData(pItem->hItem);

	if ((pItem->mask & TVIF_TEXT) == TVIF_TEXT) {
		if (data->bAllocated) {
			delete[] data->cText;
			data->bAllocated = 0;
			data->cText = NULL;
		}

		if (pItem->pszText != LPSTR_TEXTCALLBACKW) {
			CUTF8 utf8NewText(pItem->pszText);
			int utf8TextLength = utf8NewText.Size();
			data->cText = new char[1 + utf8TextLength];
			strcpy_s(data->cText, utf8TextLength, utf8NewText.UTF8());
			data->cText[utf8TextLength] = 0;
			data->bAllocated = 1;
		} else {
			data->cText = LPSTR_TEXTCALLBACKA;
			data->bAllocated = 0;
		}

		pItem->pszText = LPSTR_TEXTCALLBACKW;
	}

	if ((pItem->mask & TVIF_PARAM) == TVIF_PARAM) {
		data->dwUserData = pItem->lParam;
	}; 

	pItem->mask &=~ (TVIF_PARAM /*| TVIF_TEXT*/);

#ifdef _UNICODE
	CTreeCtrl::SetItem(pItem);
#else
	// TVITEMA and TVITEMW have same size
	CTreeCtrl::SetItem(reinterpret_cast<TVITEMA*>(pItem));
#endif
}

bool CUnicodeTreeCtrl::SetItemText(HTREEITEM hItem, LPCTSTR lpszItem)
{
	UNICODETREEITEM_DATA* data = (UNICODETREEITEM_DATA*)CTreeCtrl::GetItemData(hItem);

	if (!data)
		return false;

	if (data->bAllocated) {
		delete[] data->cText;
		data->bAllocated = 0;
		data->cText = NULL;
	}

	if (lpszItem != LPSTR_TEXTCALLBACK) {
		CUTF8 utf8NewText(lpszItem);
		size_t utf8NewTextLength = utf8NewText.Size();
		data->cText = new char[utf8NewTextLength + 1];
		strcpy(data->cText, utf8NewText.UTF8());
		data->bAllocated = 1;
	} else {
		data->cText = (char*)lpszItem;
	}

	return true;
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

DWORD_PTR CUnicodeTreeCtrl::GetItemData(HTREEITEM hItem) const
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

	CTreeCtrl::SetItemData(hItem, NULL);
	if (hChild = GetChildItem(hItem)) {
		DeleteAllItems(hChild);
	}

	CTreeCtrl::DeleteItem(hItem);

	if (data && data->bAllocated) {
		delete[] data->cText;
		data->bAllocated = 0;
		data->cText = NULL;
	}

	if (data) {
		delete data;
		data = NULL;
	}

	return true;
}




char* CUnicodeTreeCtrl::GetItemText(HTREEITEM hItem)
{
	UNICODETREEITEM_DATA* data = (UNICODETREEITEM_DATA*)CTreeCtrl::GetItemData(hItem);

	if (!data)
		return "";

	if (data->cText != (char*)LPSTR_TEXTCALLBACK) 
		return data->cText;

	return NULL;
}





void _stdcall DeleteItemData(UNICODETREEITEM_DATA* data)
{
	if (data && data->bAllocated) {
		delete[] data->cText;
		data->bAllocated = 0;
		data->cText = NULL;
	}

	if (data) {
		delete data;
		data = NULL;
	}
}

bool CUnicodeTreeCtrl::DeleteAllItems(HTREEITEM hRoot)
{
	if (!hRoot) {
		hRoot = GetRootItem();
		if (!hRoot)
			return true;
	}
		/*TreeItemDeleter<UNICODETREEITEM_DATA> deleter(&DeleteItemData);
		PrepareDeleteAllItems(hRoot, deleter, &CTreeCtrl::GetItemData);
		deleter();

		return true;
		/*if (!hRoot)
			return false;*/
	/*} else {*/

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
		*pResult = 0;
	} else {
		CTreeCtrl::OnKeyDown(nChar, 1, pTVKeyDown->flags);
		*pResult = 0;
	}

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
		case WM_KEYDOWN:
	//		Sleep(1);
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

int CUnicodeTreeCtrl::RenderItem(HTREEITEM hItem, std::string& dest, int iDepth)
{
	int isize = 0;

	if (hItem) {
		if (iDepth > 1) {
			dest.push_back('|');
		}

		for (int i=0;i<iDepth;i++) {
			isize+=2;
			dest.push_back(32);
			dest.push_back(32);
		}
	
		UNICODETREEITEM_DATA* uti = (UNICODETREEITEM_DATA*)CTreeCtrl::GetItemData(hItem);
		if (uti->cText) {
			dest.append(uti->cText);
			isize+=strlen(uti->cText);
		} else {
			char* s = GetItemText(hItem);
			dest.append(s);
			isize+=strlen(s);
		}
		dest.push_back(13);
		dest.push_back(10);
		isize+=2;
		isize += RenderItem(GetChildItem(hItem), dest, iDepth+1);
		isize += RenderItem(GetNextSiblingItem(hItem), dest, iDepth);
	}


	return isize;
}

int CUnicodeTreeCtrl::Render2Buffer(std::string& dest)
{
	HTREEITEM hItem = GetRootItem();

#pragma warning(push)
#pragma warning(disable: 4309)
	dest.push_back(static_cast<char>(0xEF));
	dest.push_back(static_cast<char>(0xBB));
	dest.push_back(static_cast<char>(0xBF));
#pragma warning(pop)

	return RenderItem(hItem, dest, 0);;
}

void CUnicodeTreeCtrl::OnReturn(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen
	
	PostMessage(TVM_ENDEDITLABELNOW, 0, 0);
	
	*pResult = 1;
}

void CUnicodeTreeCtrl::OnDestroy() 
{
	DeleteAllItems();

	CTreeCtrl::OnDestroy();
	
	// TODO: Code für die Behandlungsroutine für Nachrichten hier einfügen
	
}

int CUnicodeTreeCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CTreeCtrl::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// TODO: Speziellen Erstellungscode hier einfügen
	InitUnicode();
	
	return 0;
}
