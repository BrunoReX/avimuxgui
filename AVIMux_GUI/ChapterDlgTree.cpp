// ChapterDlgTree.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "chapterdlg.h"
#include "AVIMux_GUI.h"
#include "ChapterDlgTree.h"
#include "languages.h"
#include "textfiles.h"
#include "..\basestreams.h"
#include "formattext.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CChapterDlgTree

CChapterDlgTree::CChapterDlgTree()
{
	EnableAutomation();
}

CChapterDlgTree::~CChapterDlgTree()
{
}

void CChapterDlgTree::OnFinalRelease()
{
	// Nachdem die letzte Referenz auf ein Automatisierungsobjekt freigegeben wurde,
	// wird OnFinalRelease aufgerufen. Die Basisklasse löscht das Objekt
	// automatisch. Fügen Sie zusätzlichen Bereinigungscode für Ihr Objekt
	// hinzu, bevor Sie die Basisklasse aufrufen.

	CUnicodeTreeCtrl::OnFinalRelease();
}


BEGIN_MESSAGE_MAP(CChapterDlgTree, CUnicodeTreeCtrl)
	//{{AFX_MSG_MAP(CChapterDlgTree)
	ON_WM_DROPFILES()
	ON_WM_KEYDOWN()
//	ON_NOTIFY(TVN_SELCHANGED, IDC_CHAPTERTREE, OnSelchangedChaptertree)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BEGIN_DISPATCH_MAP(CChapterDlgTree, CUnicodeTreeCtrl)
	//{{AFX_DISPATCH_MAP(CChapterDlgTree)
		// HINWEIS - Der Klassen-Assistent fügt hier Zuordnungsmakros ein und entfernt diese.
	//}}AFX_DISPATCH_MAP
END_DISPATCH_MAP()

// Hinweis: Wir stellen Unterstützung für IID_IChapterDlgTree zur Verfügung, um typsicheres Binden
//  von VBA zu ermöglichen. Diese IID muss mit der GUID übereinstimmen, die in der
//  Disp-Schnittstelle in der .ODL-Datei angegeben ist.

// {BE1769F9-161E-4474-859C-1ECF844AE7FA}
static const IID IID_IChapterDlgTree =
{ 0xbe1769f9, 0x161e, 0x4474, { 0x85, 0x9c, 0x1e, 0xcf, 0x84, 0x4a, 0xe7, 0xfa } };

BEGIN_INTERFACE_MAP(CChapterDlgTree, CUnicodeTreeCtrl)
	INTERFACE_PART(CChapterDlgTree, IID_IChapterDlgTree, Dispatch)
END_INTERFACE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CChapterDlgTree 

/*
void CChapterDlgTree::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	// TODO: Code für die Behandlungsroutine für Nachrichten hier einfügen

	
}
*/
/*
void CChapterDlgTree::OnRButtonUp(UINT nFlags, CPoint point) 
{
	// TODO: Code für die Behandlungsroutine für Nachrichten hier einfügen und/oder Standard aufrufen
	CString		cStr;
	bool		bItemPresent=false;

	if (GetCount())
	{
			
		cmPopupMenu=new CMenu;
		cmPopupMenu->CreatePopupMenu();
			

		if (bItemPresent) cmPopupMenu->AppendMenu(MF_SEPARATOR,0);
		cStr=LoadString(IDS_EXTRACTDTS);
		cmPopupMenu->AppendMenu(MF_STRING,IDM_EXTRACTDTS,cStr);
		bItemPresent=true;
			
		ClientToScreen(&point);
		cmPopupMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON,point.x,point.y,this);
		delete cmPopupMenu;
		
	}
	
	CUnicodeTreeCtrl::OnRButtonUp(nFlags, point);
}
*/
void CChapterDlgTree::OnDropFiles(HDROP hDropInfo) 
{
	// TODO: Code für die Behandlungsroutine für Nachrichten hier einfügen und/oder Standard aufrufen
	DWORD dwCount=DragQueryFile(hDropInfo,0xFFFFFFFF,NULL,NULL);
	int	chap_count = 0;

	for (int i=0;i<(int)dwCount;i++)
	{
		char* lpcName = (char*)calloc(1,1000);
		DragQueryFile(hDropInfo,i,lpcName,1000);

		FILESTREAM* file = new FILESTREAM;
		file->Open(lpcName,STREAM_READ);
		CTEXTFILE*	f = new CTEXTFILE(STREAM_READ,file);

		// dvd maestro chapter text file
		char c[1000];
		f->ReadLine(c);
		if (!strcmp("$Spruce_IFrame_List",c)) {
			while (f->ReadLine(c)>-1) {
				__int64 iTime = SONChapStr2Millisec(c)*1000000;
				if (iTime>=0) {
					CChapters* chapters = ((CChapterDlg*)GetParent())->GetChapters();
					char cName[20];
					sprintf(cName,"chapter %02d",++chap_count);
					int j = chapters->AddChapter(iTime, -1, cName);
					chapters->SetChapterLng(j, "eng", 0);
					sprintf(cName,"Kapitel %02d",chap_count);
					chapters->SetChapterText(j, cName, 1);
					chapters->SetChapterLng(j, "ger", 1);
					sprintf(cName,"chapitre %02d",chap_count);
					chapters->SetChapterText(j, cName, 2);
					chapters->SetChapterLng(j, "fre", 2);
				
				}
			}
		}

	}

	((CChapterDlg*)GetParent())->UpdateChapters();

	CUnicodeTreeCtrl::OnDropFiles(hDropInfo);
}

void CChapterDlgTree::GetTextCallback(NMHDR* pNMHDR, LRESULT* pResult) 
{
	TV_DISPINFO* pTVDispInfo = (TV_DISPINFO*)pNMHDR;
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen

	CHAPTER_ENTRY* pCE = (CHAPTER_ENTRY*)GetItemData(pTVDispInfo->item.hItem);

	if (pTVDispInfo->item.mask & TVIF_TEXT) {
		if (pCE && pCE->cText) {
			SINGLE_CHAPTER_DATA scd;
			pCE->c->GetChapter(pCE->iIndex, &scd);
			if (scd.bEnabled) {
				strcat(pTVDispInfo->item.pszText, "enabled, ");
			} else strcat(pTVDispInfo->item.pszText, "disabled, ");
			if (scd.bHidden) {
				strcat(pTVDispInfo->item.pszText, "hidden, ");
			};// else strcat(pTVDispInfo->item.pszText, "not hidden, ");

			strcat(pTVDispInfo->item.pszText, pCE->cText);
		} else {
			pTVDispInfo->item.pszText[0]=0;
		}
	}
	
	*pResult = 0;
}

void CChapterDlgTree::SetChapters(CChapters* _c)
{
	c = _c;
}

int RemoveFromTree(CUnicodeTreeCtrl* tree, HTREEITEM hParent)
{
	HTREEITEM hLast = hParent;

	if (!hLast) return 0;

	while (hLast) {
		void* p = (void*)tree->GetItemData(hLast);
		delete p;
		RemoveFromTree(tree,tree->GetChildItem(hLast));
		hLast = tree->GetNextSiblingItem(hLast);
	}

	return 0;
}

int FormatChapterEntry(__int64 iBegin, __int64 iEnd, char* cText, char* cBuffer)
{
	char cFrom[30]; char cTill[30];

	Millisec2Str(iBegin/1000000,cFrom);
	if (iEnd != -1) {
		Millisec2Str(iEnd/1000000,cTill);
	} else {
		sprintf(cTill, "end");
	}
	sprintf(cBuffer,"%s - %s> %s",cFrom,cTill,cText);

	return 0;
}

/*void CChapterDlgTree::OnKeydownChaptertree(NMHDR* pNMHDR, LRESULT* pResult) 
{
	TV_KEYDOWN* pTVKeyDown = (TV_KEYDOWN*)pNMHDR;
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen
	
/*	HTREEITEM hItem = GetSelectedItem();
	HTREEITEM hNext = NULL;
	HTREEITEM hNew = NULL;
	HTREEITEM hParent = GetParentItem(hItem);
	TVINSERTSTRUCT	tvi;


	CHAPTER_ENTRY* pCE = NULL;
	CHAPTER_ENTRY* pNew = NULL;
	int u = !!SendDlgItemMessage(IDC_CHAPTERTREE, TVM_GETUNICODEFORMAT);

	if (hItem) {
		pCE = (CHAPTER_ENTRY*)GetItemData(hItem);
	} else {
		pCE = (CHAPTER_ENTRY*)calloc(1,sizeof(CHAPTER_ENTRY));
		pCE->c = c;
	}

	switch (pTVKeyDown->wVKey) {
		case VK_DELETE: 
			hNext = GetNextSiblingItem(hItem);
			
			if (hItem) {
	
				RemoveFromTree(this,GetChildItem(hItem));
				pCE->c->DeleteChapter(pCE->iIndex);
				delete pCE;

				while (hNext) {
					pCE = ((CHAPTER_ENTRY*)GetItemData(hNext));
					pCE->iIndex--;
					hNext = GetNextSiblingItem(hNext);
				}
			
				DeleteItem(hItem);
			}
			break;
		case VK_INSERT:
			pCE->c->AddChapter(0,-1,"title");
			char cBuffer[200];
			FormatChapterEntry(0,-1,"title",cBuffer);
			pNew = (CHAPTER_ENTRY*)calloc(1,sizeof(CHAPTER_ENTRY));
			pNew->c = pCE->c;
			pNew->iIndex = pCE->c->GetChapterCount()-1;

			if (u) {
				Str2UTF8(cBuffer, cBuffer);
			}

			tvi.hParent=hParent;
			tvi.hInsertAfter=TVI_LAST;
			tvi.item.mask=TVIF_TEXT;
			tvi.item.pszText= LPSTR_TEXTCALLBACK;
			pNew->cText = new char[1024];
			strcpy(pNew->cText, cBuffer);
			hNew = m_Chapters.InsertItem(&tvi);
	
			m_Chapters.SetItemData(hNew,(DWORD)pNew);
			m_Chapters.Expand(hItem,TVE_EXPAND);
			m_Chapters.SelectItem(hNew);
			m_Chapters.EditLabel(hNew);
			break;
	}

	*pResult = 0;*/
//}

void CChapterDlgTree::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	// TODO: Code für die Behandlungsroutine für Nachrichten hier einfügen und/oder Standard aufrufen
	HTREEITEM hItem = GetSelectedItem();
	HTREEITEM hNext = NULL;
	HTREEITEM hNew = NULL;
	HTREEITEM hParent = GetParentItem(hItem);
	TVINSERTSTRUCT	tvi;


	CHAPTER_ENTRY* pCE = NULL;
	CHAPTER_ENTRY* pNew = NULL;
	int u = !!SendDlgItemMessage(IDC_CHAPTERTREE, TVM_GETUNICODEFORMAT);

	if (hItem) {
		pCE = (CHAPTER_ENTRY*)GetItemData(hItem);
	} else {
		pCE = (CHAPTER_ENTRY*)calloc(1,sizeof(CHAPTER_ENTRY));
		pCE->c = c;
	}

	switch (nChar) {
		case VK_DELETE: 
			hNext = GetNextSiblingItem(hItem);
			
			if (hItem) {
	
				RemoveFromTree(this,GetChildItem(hItem));
				pCE->c->DeleteChapter(pCE->iIndex);
				delete pCE;

				while (hNext) {
					pCE = ((CHAPTER_ENTRY*)GetItemData(hNext));
					pCE->iIndex--;
					hNext = GetNextSiblingItem(hNext);
				}
			
				DeleteItem(hItem);

				if (!GetRootItem()) {
					// Baum leer
				}
			}
			break;
		case VK_INSERT:
			pCE->c->AddChapter(0,-1,"title");
			char cBuffer[200];
			FormatChapterEntry(0,-1,"title",cBuffer);
			pNew = (CHAPTER_ENTRY*)calloc(1,sizeof(CHAPTER_ENTRY));
			pNew->c = pCE->c;
			pNew->iIndex = pCE->c->GetChapterCount()-1;

			if (u) {
				Str2UTF8(cBuffer, cBuffer);
			}

			tvi.hParent=hParent;
			tvi.hInsertAfter=TVI_LAST;
			tvi.item.mask=TVIF_TEXT;
			tvi.item.pszText= LPSTR_TEXTCALLBACK;
			pNew->cText = new char[1024];
			strcpy(pNew->cText, cBuffer);
			hNew = InsertItem(&tvi);
	
			SetItemData(hNew,(DWORD)pNew);
			Expand(hItem,TVE_EXPAND);
			SelectItem(hNew);
			EditLabel(hNew);
			break;
	}


	CUnicodeTreeCtrl::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CChapterDlgTree::OpenContextMenu(CPoint point)
{
	CMenu* c = new CMenu;
	bool bPlaceSep = false;

	c->CreatePopupMenu();

	HTREEITEM hItem = GetSelectedItem();

	if (hItem) {
		CHAPTER_ENTRY* pCE = (CHAPTER_ENTRY*)GetItemData(hItem);

		bool bEnabled = !!(pCE->c->IsChapterEnabled(pCE->iIndex));
		bool bHidden = !!(pCE->c->IsChapterHidden(pCE->iIndex));

		c->AppendMenu(MF_STRING, IDM_ENABLECHAPTER, (!bEnabled)?"Enable chapter":"Disable chapter");
		c->AppendMenu(MF_STRING, IDM_HIDECHAPTER, (!bHidden)?"Hide chapter":"Show chapter");

		ClientToScreen(&point);
		c->TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON,point.x,point.y,this);
	}

	delete c;
}

BOOL CChapterDlgTree::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	// TODO: Speziellen Code hier einfügen und/oder Basisklasse aufrufen
	CHAPTER_ENTRY* pCE;
	HTREEITEM hItem =GetSelectedItem();
	RECT r;
	GetItemRect(hItem, &r, false);

	switch (LOWORD(wParam))
	{
		case IDM_ENABLECHAPTER:
			pCE = (CHAPTER_ENTRY*)GetItemData(hItem);
			pCE->c->EnableChapter(pCE->iIndex, !pCE->c->IsChapterEnabled(pCE->iIndex));
			InvalidateRect(&r);
			UpdateWindow();
			break;
		case IDM_HIDECHAPTER:
			pCE = (CHAPTER_ENTRY*)GetItemData(hItem);
			pCE->c->HideChapter(pCE->iIndex, !pCE->c->IsChapterHidden(pCE->iIndex));
			InvalidateRect(&r);
			UpdateWindow();
			break;

	}

	
	return CUnicodeTreeCtrl::OnCommand(wParam, lParam);
}
