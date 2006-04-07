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
	int iItemCount;
	char** cText;
} UNICODELISTCONTROL_DATA;

void CUnicodeListCtrl::InitUnicode()
{
	SendMessage(LVM_SETUNICODEFORMAT, utf8_IsUnicodeEnabled());

	CUnicodeBase::InitUnicode(SendMessage(CCM_GETUNICODEFORMAT));
}

int CUnicodeListCtrl::InsertItem(LV_ITEM *pItem)
{
	char* c = pItem->pszText;

	pItem->pszText = LPSTR_TEXTCALLBACK;
	int iIndex = CListCtrl::InsertItem(pItem);

	UNICODELISTCONTROL_DATA* data = new UNICODELISTCONTROL_DATA;

	data->iItemCount = 1;
	data->cText = (char**)calloc(data->iItemCount, sizeof(char*));

	int k=1+strlen(c);
	data->cText[0] = new char[k];
	ZeroMemory(data->cText[0], k);
	strcpy(data->cText[0], c);

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
		for (int i = 0; i < data->iItemCount; i++) {
			if (data->cText[i]) {
				delete[] data->cText[i];
				data->cText[i] = 0;
			}
		}
		free(data->cText);
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

int CUnicodeListCtrl::SetItemText(int nItem, int nSubItem, LPTSTR lpszText)
{
	UNICODELISTCONTROL_DATA* data = (UNICODELISTCONTROL_DATA*)CListCtrl::GetItemData(nItem);
	ASSERT(data);
	int k;

	if (data && (DWORD)data != LB_ERR) {
		if (nSubItem >= data->iItemCount) {
			if (!data->cText)
				data->cText = (char**)calloc(sizeof(char*) * (1+nSubItem), 1);
			else {
				data->cText = (char**)realloc(data->cText, sizeof(char*) * (1+nSubItem));
				for (int j=data->iItemCount;j<=nSubItem;j++)
					data->cText[j] = NULL;
			}
			data->iItemCount = nSubItem+1;
		}

		if (data->cText[nSubItem])
			delete[] data->cText[nSubItem];

		data->cText[nSubItem] = new char[k=strlen(lpszText)+1];

		ZeroMemory(data->cText[nSubItem], k);

		strcpy(data->cText[nSubItem], lpszText);

	}

	return 1;
}

void CUnicodeListCtrl::GetTextCallback(NMHDR* pNMHDR, LRESULT* pResult)
{
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;
	LVITEM* item = &pDispInfo->item;
	char* dest = item->pszText;  // <= write the output string here in UTF-8
	
	*pResult = 0;
}


void CUnicodeListCtrl::GetItemText(int nItem, int nSubItem, char* cDest, int imax)
{
	UNICODELISTCONTROL_DATA* data = (UNICODELISTCONTROL_DATA*)CListCtrl::GetItemData(nItem);

	strncpy(cDest, data->cText[nSubItem], imax);
}


char* CUnicodeListCtrl::GetItemText(int nItem, int nSubItem)
{
	UNICODELISTCONTROL_DATA* data = (UNICODELISTCONTROL_DATA*)CListCtrl::GetItemData(nItem);

	if (data && data->iItemCount > nSubItem) return data->cText[nSubItem];
	return "";
}

void CUnicodeListCtrl::OnGetdispinfo(NMHDR* pNMHDR, LRESULT* pResult) 
{
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen
	LVITEM* item = &pDispInfo->item;
	char* dest = item->pszText;
	char* c = NULL;
	bool	bAllocated = false;

	if ((item->mask & LVIF_TEXT) == LVIF_TEXT) {

		UNICODELISTCONTROL_DATA* data = (UNICODELISTCONTROL_DATA*)CListCtrl::GetItemData(item->iItem);

		if (data) {
			if (item->iSubItem >= data->iItemCount) {
				dest[0] = 0;
				*pResult = 0;
				return;
			}

			if (data->cText[item->iSubItem] != LPSTR_TEXTCALLBACK) {
				c = data->cText[item->iSubItem];
			} else {
				c = (char*)calloc(4096, sizeof(c));
				bAllocated = true;
				GetTextCallback(pNMHDR, pResult);
			}

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