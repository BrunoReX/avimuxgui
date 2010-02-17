// UnicodeListControl.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "AVIMux_GUI.h"
#include "UnicodeListControl.h"
#include "..\utf-8.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CUnicodeListCtrl

CUnicodeListCtrl::CUnicodeListCtrl()
{
	EnableAutomation();
}

CUnicodeListCtrl::~CUnicodeListCtrl()
{
}

void CUnicodeListCtrl::OnFinalRelease()
{
	// Nachdem die letzte Referenz auf ein Automatisierungsobjekt freigegeben wurde,
	// wird OnFinalRelease aufgerufen. Die Basisklasse löscht das Objekt
	// automatisch. Fügen Sie zusätzlichen Bereinigungscode für Ihr Objekt
	// hinzu, bevor Sie die Basisklasse aufrufen.

	CListCtrl::OnFinalRelease();
}


BEGIN_MESSAGE_MAP(CUnicodeListCtrl, CListCtrl)
	//{{AFX_MSG_MAP(CUnicodeListCtrl)
	ON_NOTIFY_REFLECT(LVN_GETDISPINFO, OnGetdispinfo)
	ON_NOTIFY_REFLECT(LVN_GETDISPINFOW, OnGetdispinfo)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BEGIN_DISPATCH_MAP(CUnicodeListCtrl, CListCtrl)
	//{{AFX_DISPATCH_MAP(CUnicodeListControl)
		// HINWEIS - Der Klassen-Assistent fügt hier Zuordnungsmakros ein und entfernt diese.
	//}}AFX_DISPATCH_MAP
END_DISPATCH_MAP()

// Hinweis: Wir stellen Unterstützung für IID_IUnicodeListControl zur Verfügung, um typsicheres Binden
//  von VBA zu ermöglichen. Diese IID muss mit der GUID übereinstimmen, die in der
//  Disp-Schnittstelle in der .ODL-Datei angegeben ist.

// {E0977612-8AE0-419C-A28C-66B7799DC371}
static const IID IID_IUnicodeListCtrl =
{ 0xe0977612, 0x8ae0, 0x419c, { 0xa2, 0x8c, 0x66, 0xb7, 0x79, 0x9d, 0xc3, 0x71 } };

BEGIN_INTERFACE_MAP(CUnicodeListCtrl, CListCtrl)
	INTERFACE_PART(CUnicodeListCtrl, IID_IUnicodeListCtrl, Dispatch)
END_INTERFACE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CUnicodeListControl 

BOOL CUnicodeListCtrl::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext) 
{
	// TODO: Speziellen Code hier einfügen und/oder Basisklasse aufrufen
	
	return CWnd::Create(lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, nID, pContext);
}

typedef struct
{
	std::vector<std::string> texts;
} UNICODELISTCONTROL_DATA;

void CUnicodeListCtrl::InitUnicode()
{
	SendMessage(LVM_SETUNICODEFORMAT, utf8_IsUnicodeEnabled());

	CUnicodeBase::InitUnicode(SendMessage(CCM_GETUNICODEFORMAT));
}

int CUnicodeListCtrl::InsertItem(LV_ITEMA *pItem)
{
	char* itemText = pItem->pszText;

#ifdef _UNICODE
	LV_ITEMW newItem;
	newItem.mask = pItem->mask;
	newItem.cchTextMax = pItem->cchTextMax;
	newItem.cColumns = pItem->cColumns;
	newItem.pszText = LPSTR_TEXTCALLBACKW;
	newItem.state = pItem->state;
	newItem.stateMask = pItem->stateMask;
	newItem.puColumns = pItem->puColumns;
	newItem.lParam = pItem->lParam;
	int iIndex = CListCtrl::InsertItem(&newItem);	
#else
	pItem->pszText = LPSTR_TEXTCALLBACK;
	int iIndex = CListCtrl::InsertItem(pItem);
#endif
	UNICODELISTCONTROL_DATA* data = new UNICODELISTCONTROL_DATA;

//	data->iItemCount = 1;
//	data->cText = (char**)calloc(data->iItemCount, sizeof(char*));
	data->texts.push_back(std::string(itemText));

//	int k=1+strlen(c);
//	data->cText[0] = new char[k];
//	ZeroMemory(data->cText[0], k);
//	strcpy(data->cText[0], c);

	CListCtrl::SetItemData(iIndex, (LPARAM)data);

	return iIndex;
}

int CUnicodeListCtrl::InsertItem(LV_ITEMW *pItem)
{
	wchar_t* text = pItem->pszText;
#ifdef _UNICODE
	pItem->pszText = LPSTR_TEXTCALLBACKW;
	int iIndex = CListCtrl::InsertItem(pItem);
#else
	LV_ITEMA newItem;
	newItem.mask = pItem->mask;
	newItem.cchTextMax = pItem->cchTextMax;
	newItem.cColumns = pItem->cColumns;
	newItem.pszText = LPSTR_TEXTCALLBACKA;
	newItem.state = pItem->state;
	newItem.stateMask = pItem->stateMask;
	newItem.puColumns = pItem->puColumns;
	newItem.lParam = pItem->lParam;
	int iIndex = CListCtrl::InsertItem(&newItem);
#endif
	UNICODELISTCONTROL_DATA* data = new UNICODELISTCONTROL_DATA;

//	data->iItemCount = 1;
//	data->cText = (char**)calloc(data->iItemCount, sizeof(char*));
	CUTF8 utf8Text(text);
	data->texts.push_back(std::string(utf8Text.UTF8()));

//	int k=1+strlen(c);
//	data->cText[0] = new char[k];
//	ZeroMemory(data->cText[0], k);
//	strcpy(data->cText[0], c);

	CListCtrl::SetItemData(iIndex, (LPARAM)data);

	return iIndex;
}

BOOL CUnicodeListCtrl::DeleteItem(int nItem) 
{
	if (nItem >= GetItemCount()) {
		return false;
	}

	UNICODELISTCONTROL_DATA* data = (UNICODELISTCONTROL_DATA*)GetItemData(nItem);

	if (data) {
		/*for (int i = 0; i < data->iItemCount; i++) {
			if (data->cText[i]) {
				delete[] data->cText[i];
				data->cText[i] = 0;
			}
		}*/
		//free(data->cText);
	}

	delete data;
	data = NULL;

	CListCtrl::DeleteItem(nItem);

	return true;
}

BOOL CUnicodeListCtrl::DeleteAllItems()
{
	for (int i=GetItemCount()-1; i>=0;DeleteItem(i--));
	return true;
}

int CUnicodeListCtrl::SetItemText(int nItem, int nSubItem, LPCSTR lpszText)
{
	if (nSubItem < 0)
		return 0;

	UNICODELISTCONTROL_DATA* data = (UNICODELISTCONTROL_DATA*)CListCtrl::GetItemData(nItem);
	ASSERT(data);

	if (data && (DWORD)data != LB_ERR) {
		size_t subItemIndex = static_cast<size_t>(nSubItem);
		if (subItemIndex >= data->texts.size()) {
			data->texts.resize(subItemIndex + 1);
		}
		data->texts[subItemIndex] = lpszText;
	}

	return 1;
}

int CUnicodeListCtrl::SetItemText(int nItem, int nSubItem, LPCWSTR lpszText)
{
	if (nSubItem < 0)
		return 0;

	UNICODELISTCONTROL_DATA* data = (UNICODELISTCONTROL_DATA*)CListCtrl::GetItemData(nItem);
	ASSERT(data);

	if (data && (DWORD)data != LB_ERR) {

		size_t subItemIndex = static_cast<size_t>(nSubItem);
		if (subItemIndex >= data->texts.size()) {
			data->texts.resize(subItemIndex + 1);
		}
		data->texts[nSubItem] = CUTF8(lpszText).UTF8();
	}

	return 1;
}

void CUnicodeListCtrl::GetTextCallback(NMHDR* pNMHDR, LRESULT* pResult)
{
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;
	LVITEM* item = &pDispInfo->item;

	// NOT SUPPORTED
	TCHAR* dest = item->pszText;  // <= write the output string here in UTF-8
	
	*pResult = 0;
}


void CUnicodeListCtrl::GetItemText(int nItem, int nSubItem, char* cDest, int imax)
{
	UNICODELISTCONTROL_DATA* data = (UNICODELISTCONTROL_DATA*)CListCtrl::GetItemData(nItem);

	strncpy(cDest, data->texts[nSubItem].c_str(), imax);
}


char* CUnicodeListCtrl::GetItemText(int nItem, int nSubItem)
{
	if (nSubItem < 0)
		return NULL;
	UNICODELISTCONTROL_DATA* data = (UNICODELISTCONTROL_DATA*)CListCtrl::GetItemData(nItem);

	size_t subItemIndex = static_cast<size_t>(nSubItem);
	if (data && data->texts.size() > subItemIndex) 
		return const_cast<char*>(data->texts[subItemIndex].c_str());
	
	return "";
}

void CUnicodeListCtrl::OnGetdispinfo(NMHDR* pNMHDR, LRESULT* pResult) 
{
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen
	LVITEM* item = &pDispInfo->item;
	char* dest = reinterpret_cast<char*>(item->pszText);
	char* c = NULL;
	bool	bAllocated = false;

	if ((item->mask & LVIF_TEXT) == LVIF_TEXT) {

		UNICODELISTCONTROL_DATA* data = (UNICODELISTCONTROL_DATA*)CListCtrl::GetItemData(item->iItem);

		if (data) {
			if (item->iSubItem >= static_cast<int>(data->texts.size())) {
				dest[0] = 0;
				*pResult = 0;
				return;
			}

			/*if (data->cText[item->iSubItem] != LPSTR_TEXTCALLBACK) {
				c = data->cText[item->iSubItem];
			} else {
				c = (char*)calloc(4096, sizeof(c));
				bAllocated = true;
				GetTextCallback(pNMHDR, pResult);
			}*/

			c = const_cast<char*>(data->texts[item->iSubItem].c_str());

			if (c)
				(fromUTF8)(c, dest);
			else
				dest[0]=0;

			if (bAllocated) {
				delete c;
				return;
			}
		} else {
			dest[0] = 0;
		}

	}

	*pResult = 0;
}

void CUnicodeListCtrl::OnDestroy()
{
	while (GetItemCount())
		DeleteItem(0);

}