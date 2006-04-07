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
#include "version.h"
#include "..\UnicodeCalls.h"
#include "XMLFiles.h"
#include "..\matroska.h"

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
	clipboard = new CChapters;

	hContextMenuTarget = NULL;
}

CChapterDlgTree::~CChapterDlgTree()
{
	clipboard->Delete();
	delete clipboard;
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

void FillChapterSegmentUIDs(CChapters* c, int start_index, int end_index, char* cUID)
{
	if (end_index == CHAP_LAST)
		end_index = c->GetChapterCount();

	for (int i=start_index;i<end_index && i<c->GetChapterCount();i++) {
		if (!c->IsSegmentUIDValid(i))
			c->SetSegmentUID(i, 1, cUID);
		if (c->HasSubChapters(i))
			FillChapterSegmentUIDs(c->GetSubChapters(i), 0, CHAP_LAST, cUID);
	}
}

void CChapterDlgTree::OnDropFiles(HDROP hDropInfo) 
{
	// TODO: Code für die Behandlungsroutine für Nachrichten hier einfügen und/oder Standard aufrufen
	DWORD dwCount=DragQueryFile(hDropInfo,0xFFFFFFFF,NULL,NULL);
	int	chap_count = 0;

	POINT	point;
	DragQueryPoint(hDropInfo, &point);
	TVHITTESTINFO hittest;
	hittest.pt = point;
	HitTest(&hittest);

	HTREEITEM hItemOn = NULL;
	CHAPTER_ENTRY* pCE = NULL;
	

	switch (hittest.flags) {
		case TVHT_ONITEM:
		case TVHT_ONITEMLABEL:
			hItemOn = hittest.hItem;
			pCE = (CHAPTER_ENTRY*)GetItemData(hItemOn);
			break;
	
		case TVHT_NOWHERE:

			break;
	}

	for (int i=0;i<(int)dwCount;i++) {
		char* lpcName = NULL;
		char* u;
		int size;

		u = (char*)calloc(2, size = 1+(*UDragQueryFile())((uint32)hDropInfo,i,NULL,0));

		bool matroska=false;
		(*UDragQueryFile())((uint32)hDropInfo,i,u,size);
		toUTF8(u, &lpcName);

		FILESTREAM* file = new FILESTREAM;
		file->Open(lpcName,STREAM_READ);
		CTEXTFILE*	textfile = new CTEXTFILE(STREAM_READ,file,CM_UTF8);

		XMLNODE* xml = NULL;
		CChapters* c = NULL;

		if (FileIsXML(textfile)) {
			char* text = (char*)calloc((size_t)file->GetSize() + 1024, 1);
			Textfile2String(textfile, text);
			if (xmlBuildTree(&xml, text) == XMLERR_OK) {
				c = new CChapters;
				int xml_result = c->ImportFromXML(xml);

				if (xml_result != CHAP_IMPXML_OK) {
					c->Delete();
					delete c;
					c = NULL;
				}

				if (xml_result ==  CHAP_IMPXML_NONUNIQUE_UID)
					MessageBox(LoadString(STR_ERR_IMPCHAP_NONUNIQUEUID),
						LoadString(IDS_ERROR), MB_OK | MB_ICONERROR);
			}

			free(text);
		}

		textfile->Seek(0);
		// dvd maestro chapter text file
		char t[1000];
		textfile->ReadLine(t);
		if (!strcmp("$Spruce_IFrame_List",t)) {
			CChapters* chapters = ((CChapterDlg*)GetParent())->GetChapters();
			CChapters* edition;
			chapters->AddEmptyEdition();
			edition = chapters;
			chapters = chapters->GetSubChapters(chapters->GetChapterCount()-1);
			while (textfile->ReadLine(t)>-1) {
				__int64 iTime = SONChapStr2Millisec(t)*1000000;
				if (iTime>=0) {
					char cName[20];
					sprintf(cName,"Chapter %02d",++chap_count);
					int j = chapters->AddChapter(iTime, -1, cName);
					chapters->SetChapterLng(j, "eng", 0);
					sprintf(cName,"Kapitel %02d",chap_count);
					chapters->SetChapterText(j, cName, 1);
					chapters->SetChapterLng(j, "ger", 1);
					sprintf(cName,"Chapitre %02d",chap_count);
					chapters->SetChapterText(j, cName, 2);
					chapters->SetChapterLng(j, "fre", 2);
					sprintf(cName,"Capitulo %02d",chap_count);
					chapters->SetChapterText(j, cName, 3);
					chapters->SetChapterLng(j, "spa", 3);
					sprintf(cName,"Capitolo %02d",chap_count);
					chapters->SetChapterText(j, cName, 4);
					chapters->SetChapterLng(j, "ita", 4);
					
				}
			}

			AddChaptersToTree(this, NULL, edition, edition->GetChapterCount()-1,
				edition->GetChapterCount()-1);
			InvalidateRect(NULL);
			UpdateWindow();
		}

		textfile->Close();
		delete textfile;

		MATROSKA* m = new MATROSKA();
		file->Seek(0);
		if (m->Open(file, MMODE_READ) == MOPEN_OK) {
			CChapters* _c = new CChapters(m->GetChapterInfo());
			c = new CChapters();
		
			c->Import(_c);
			delete _c;
			matroska = true;
		}

		if (c && (c->GetChapterCount() || matroska)) {

			/* file has been dropped onto an edition */
			if (pCE && pCE->c->IsEdition(pCE->iIndex)) {
				/* if a file has only one edition, ordered or unordered, this
				   edition can be merged into an existing chapter */
				if (c->GetChapterCount() == 1) {
					/* import the chapters of the source file's only edition into it */
					CChapters* sub = pCE->c->GetSubChapters(pCE->iIndex);

					IMPORTCHAPTERSTRUCT import_struct;
					import_struct.index_start = sub->GetChapterCount();
					import_struct.can_merge = false;
					import_struct.chapters = c->GetSubChapters(0);
					sub->Import(import_struct);

					if (m->GetTrackCount())	for (int i=import_struct.index_start;i<sub->GetChapterCount();i++) 
						if (!sub->IsSegmentUIDValid(i))
							sub->SetSegmentUID(i, 1, m->GetSegmentUID());
					
					AddChaptersToTree(this, hItemOn, sub, import_struct.index_start, CHAP_LAST);
					InvalidateRect(NULL);
					UpdateWindow();
				} else

				/* if no chapters are present in the dropped file, add the entire segment, 
				   using ChapterSegmentUID */
				if (c->GetChapterCount() == 0) {
					if (m->GetSegmentUID()) {
						CChapters* sub = pCE->c->GetSubChapters(pCE->iIndex);

						sub->AddChapter(0, -1, "");
						sub->SetChapterEnd(CHAP_LAST, m->GetSegmentDuration() * m->GetTimecodeScale());
						sub->SetSegmentUID(CHAP_LAST, true, m->GetSegmentUID());
						sub->SetChapterText(CHAP_LAST, m->GetSegmentTitle(), 0);
						sub->SetChapterLng(CHAP_LAST, "und", 0);
						pCE->c->SetIsOrdered(pCE->iIndex, 1);
						AddChaptersToTree(this, hItemOn, sub, sub->GetChapterCount()-1, CHAP_LAST);
						InvalidateRect(NULL);
						UpdateWindow();
					} else 
						MessageBox(LoadString(STR_ERR_NOSEGMENTUID), LoadString(IDS_ERROR),
							MB_OK | MB_ICONERROR);

				}
			} else if (!pCE) {
				/*	file has been dropped onto nirvana -> import source file editions
					into the root and set chaptersegmentuid to the source file's 
					segmentuid if necessary. If the source file does not have any edition,
					create a new one and import the entire segment as one chapter */
				CChapters* chapters = ((CChapterDlg*)GetParent())->GetChapters();
				int start = chapters->GetChapterCount();

				if (c->GetChapterCount()) {
					IMPORTCHAPTERSTRUCT import_struct;
					import_struct.chapters = c;
					import_struct.can_merge = false;
					chapters->Import(import_struct);
					if (m->GetTrackCount())
						FillChapterSegmentUIDs(chapters, start, CHAP_LAST, m->GetSegmentUID());
					AddChaptersToTree(this, NULL, chapters, start, CHAP_LAST);
					InvalidateRect(NULL);
					UpdateWindow();
					HTREEITEM h = GetSelectedItem();
					SelectItem(NULL);
					SelectItem(h);
				} else {
					if (m->GetSegmentUID()) {
						CChapters* edition = chapters->GetSubChapters(chapters->AddEmptyEdition());
						edition->AddChapter(0, -1, "");

						edition->SetChapterEnd(CHAP_LAST, m->GetSegmentDuration() * m->GetTimecodeScale());
						edition->SetSegmentUID(CHAP_LAST, true, m->GetSegmentUID());
					
						edition->SetChapterText(CHAP_LAST, m->GetSegmentTitle(), 0);
						edition->SetChapterLng(CHAP_LAST, "und", 0);
						edition->HideChapter(CHAP_LAST, true);

						chapters->SetChapterText(start, m->GetSegmentTitle(), 0);
						chapters->SetChapterLng(start, "und", 0);

						chapters->SetIsOrdered(CHAP_LAST, 1);

						AddChaptersToTree(this, NULL, chapters, start, CHAP_LAST);
						InvalidateRect(NULL);
						UpdateWindow();
					} else
						MessageBox(LoadString(STR_ERR_NOSEGMENTUID), LoadString(IDS_ERROR),
							MB_OK | MB_ICONERROR);

				}
			}
		} else if (c) {
			// chapters have been created, but no chapters have been found:
			if (xml) {
				// probably an XML tags file
				CChapters* chapters = ((CChapterDlg*)GetParent())->GetChapters();
				chapters->ImportFromXML(xml);

				/* reselect selected item, so that ChapterDisplay gets updated if the currently
				   selected item was affected */
				HTREEITEM hItem = GetSelectedItem();
				SelectItem(NULL);
				SelectItem(hItem);
				InvalidateRect(NULL);
				UpdateWindow();
			}
		}
		
		if (matroska)
			m->Close();
		
		delete m;

		file->Close();
		delete file;

		if (c) {
			c->Delete();
			delete c;
		}

		if (xml)
			xmlDeleteNode(&xml); 

		free(lpcName);
		free(u);
	}

	CUnicodeTreeCtrl::OnDropFiles(hDropInfo);
}

void CChapterDlgTree::GetTextCallback(NMHDR* pNMHDR, LRESULT* pResult) 
{
	TV_DISPINFO* pTVDispInfo = (TV_DISPINFO*)pNMHDR;
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen
	char cTxt[1024]; memset(cTxt, 0, sizeof(cTxt));

	CHAPTER_ENTRY* pCE = (CHAPTER_ENTRY*)GetItemData(pTVDispInfo->item.hItem);

	if (pTVDispInfo->item.mask & TVIF_TEXT) {
		if (pCE && pCE->cText) {
			SINGLE_CHAPTER_DATA scd;
			pCE->c->GetChapter(pCE->iIndex, &scd);

			if (scd.bIsEdition) {
				strcat(pTVDispInfo->item.pszText, "Edition ");
			}

			if (scd.bIsEdition || display_uids) {
				if (pCE->c->GetUIDLen(pCE->iIndex) <= 4)
					sprintf(cTxt, "%08X: ", (unsigned __int32)pCE->c->GetUID(pCE->iIndex));
				else {
					__int642hex(pCE->c->GetUID(pCE->iIndex), cTxt);
					strcat(cTxt, ": ");
				}
			}

			strcat(pTVDispInfo->item.pszText, cTxt);
			if (scd.bIsEdition) {

				if (scd.bHidden)
					strcat(pTVDispInfo->item.pszText, "hidden, ");

				if (scd.bOrdered)
					strcat(pTVDispInfo->item.pszText, "ordered, ");
				else
					strcat(pTVDispInfo->item.pszText, "not ordered, ");

				if (scd.bDefault)
					strcat(pTVDispInfo->item.pszText, "default, ");
				else
					strcat(pTVDispInfo->item.pszText, "not default, ");

				pTVDispInfo->item.pszText[strlen(pTVDispInfo->item.pszText)-2] = '>';

				char* pChapterText = pCE->c->GetChapterText(pCE->iIndex, "eng", CHAP_GCT_RETURN_FIRST);

				if (pChapterText)
					strcat(pTVDispInfo->item.pszText, pChapterText);
			} else {

				if (scd.bEnabled) {
					strcat(pTVDispInfo->item.pszText, "enabled, ");
				} else 
					strcat(pTVDispInfo->item.pszText, "disabled, ");
				
				if (scd.bHidden) 
					strcat(pTVDispInfo->item.pszText, "hidden, ");
				
				char* pChapterText = pCE->c->GetChapterText(pCE->iIndex, "eng", CHAP_GCT_RETURN_FIRST);
		
				if (!pChapterText)
					pChapterText = "";

				FormatChapterEntry(pCE->c->GetChapterBegin(pCE->iIndex),
					pCE->c->GetChapterEnd(pCE->iIndex), pChapterText, cTxt);

				strcat(pTVDispInfo->item.pszText, cTxt);
			}

		} else {
			pTVDispInfo->item.pszText[0]=0;
		}
	}
	
	*pResult = 0;
}

HTREEITEM CChapterDlgTree::GetParentMostItem(HTREEITEM hItem)
{
	HTREEITEM hParent = hItem;
	HTREEITEM h;

	while (h=GetParentItem(hParent))
		hParent = h;

	return hParent;
}

void CChapterDlgTree::SetDisplayUIDs(bool bEnable)
{
	display_uids = !!bEnable;
}

void CChapterDlgTree::SetChapters(CChapters* _c)
{
	c = _c;
}

int RemoveFromTree(CUnicodeTreeCtrl* tree, HTREEITEM hParent)
{
	HTREEITEM hLast = hParent;
	HTREEITEM hNext = NULL;

	if (!hLast) return 0;

	while (hLast) {
		void* p = (void*)tree->GetItemData(hLast);
		RemoveFromTree(tree,tree->GetChildItem(hLast));
		hNext = tree->GetNextSiblingItem(hLast);
		tree->DeleteItem(hLast);
		hLast = hNext;
		delete[] ((CHAPTER_ENTRY*)p)->cText;
		delete p;
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


int CALLBACK TreeCompareFunc(LPARAM lParam1, LPARAM lParam2, 
		LPARAM lParamSort)
{
	CUnicodeTreeCtrl* tree = (CUnicodeTreeCtrl*)lParamSort;
	CHAPTER_ENTRY* p1 = (CHAPTER_ENTRY*)((UNICODETREEITEM_DATA*)lParam1)->dwUserData;
	CHAPTER_ENTRY* p2 = (CHAPTER_ENTRY*)((UNICODETREEITEM_DATA*)lParam2)->dwUserData;

	if (p1->iIndex < p2->iIndex) return -1;
	if (p1->iIndex > p2->iIndex) return 1;
	return 0;

}

CHAPTER_ENTRY* CChapterDlgTree::GetSelectedChapterEntry()
{
	HTREEITEM	hItem = GetSelectedItem();
	if (hItem)
		return (CHAPTER_ENTRY*)GetItemData(hItem);
	else
		return NULL;
}

void CChapterDlgTree::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	// TODO: Code für die Behandlungsroutine für Nachrichten hier einfügen und/oder Standard aufrufen
	HTREEITEM hItem = GetSelectedItem();
	HTREEITEM hNext = NULL;
	HTREEITEM hNew = NULL;
	HTREEITEM hParent = GetParentItem(hItem);
	TVINSERTSTRUCT	tvi;
	TVSORTCB	sort;

	sort.lpfnCompare = TreeCompareFunc;
	sort.lParam      = (DWORD)this;


	CHAPTER_ENTRY* pCE = NULL;
	CHAPTER_ENTRY* pNew = NULL;
	int u = !!SendDlgItemMessage(IDC_CHAPTERTREE, TVM_GETUNICODEFORMAT);

	if (hItem) {
		pCE = (CHAPTER_ENTRY*)GetItemData(hItem);
	} else {
		pCE = (CHAPTER_ENTRY*)calloc(1,sizeof(CHAPTER_ENTRY));
		pCE->c = c;
	}

	sort.hParent     = GetParentItem(hItem);

	switch (nChar) {
		case VK_DELETE: 
			hNext = GetNextSiblingItem(hItem);
			
			if (hItem) {
	
				pCE->c->DeleteChapter(pCE->iIndex);
				RemoveFromTree(this,GetChildItem(hItem));
//				delete pCE;

				while (hNext) {
					pCE = ((CHAPTER_ENTRY*)GetItemData(hNext));
					pCE->iIndex--;
					hNext = GetNextSiblingItem(hNext);
				}
			
				DeleteItem(hItem);

				if (!GetRootItem()) {
					// Baum leer
					((CChapterDlg*)(GetParent()))->AfterLastDeleted();
				}

			}
			break;
		case VK_INSERT:
			pCE->c->AddChapter(0,-1,"title");
			char cBuffer[200]; memset(cBuffer, 0, sizeof(cBuffer));

			pNew = (CHAPTER_ENTRY*)calloc(1,sizeof(CHAPTER_ENTRY));
			pNew->c = pCE->c;
			pNew->iIndex = pCE->c->GetChapterCount()-1;
			if (!hParent) pCE->c->SetIsEdition(pNew->iIndex, 1);

			if (u) {
				Str2UTF8("title", cBuffer);
			}

			tvi.hParent=hParent;
			tvi.hInsertAfter=TVI_LAST;
			tvi.item.mask=TVIF_TEXT;
			tvi.item.pszText= LPSTR_TEXTCALLBACK;
			pNew->cText = new char[1024]; memset(pNew->cText, 0, 1024);
			strcpy(pNew->cText, "title");
			hNew = InsertItem(&tvi);
	
			SetItemData(hNew,(DWORD)pNew);
			Expand(hItem,TVE_EXPAND);
			SelectItem(hNew);

			break;
		case VK_PRIOR: 
			if (pCE->c->Swap(pCE->iIndex, pCE->iIndex-1)) {
				DWORD p = GetItemData(hItem);
				HTREEITEM hPrev2;
				HTREEITEM hPrev = GetPrevSiblingItem(hPrev2=GetPrevSiblingItem(hItem));
				if (!hPrev) hPrev = TVI_FIRST;
				CHAPTER_ENTRY* pCEP = (CHAPTER_ENTRY*)GetItemData(hPrev2);
				pCEP->iIndex++;
				pCE->iIndex--;
				SortChildrenCB(&sort);
				UpdateWindow();
			}
			return;
			break;
		case VK_NEXT: 
			if (pCE->c->Swap(pCE->iIndex, pCE->iIndex+1)) {
				DWORD p = GetItemData(hItem);
				HTREEITEM hNext = GetNextSiblingItem(hItem);
				CHAPTER_ENTRY* pCEN = (CHAPTER_ENTRY*)GetItemData(hNext);
				pCEN->iIndex--;
				pCE->iIndex++;
				SortChildrenCB(&sort);
				UpdateWindow();
			}
			return;
			break;
	}

	CUnicodeTreeCtrl::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CChapterDlgTree::OpenContextMenu(CPoint point)
{
	CMenu* c = new CMenu;
	CMenu* csub = new CMenu;
	CMenu* cUIDs = new CMenu;
	bool bPlaceSep = false;

	c->CreatePopupMenu();
	csub->CreatePopupMenu();
	cUIDs->CreatePopupMenu();

	HTREEITEM hItemHit, hItem;
	HTREEITEM hSelectedItem = GetSelectedItem();

	UINT flags;

	hItemHit = HitTest(point, &flags);

	if (flags == TVHT_ONITEMLABEL)
		hItem = hItemHit;
	else
		hItem = hSelectedItem;

	hContextMenuTarget = hItem;

	if (hItem) {
		CHAPTER_ENTRY* pCE = (CHAPTER_ENTRY*)GetItemData(hItem);

		bool bEnabled = !!(pCE->c->IsEnabled(pCE->iIndex));
		bool bHidden  = !!(pCE->c->IsHidden(pCE->iIndex));
		bool bEdition = !!(pCE->c->IsEdition(pCE->iIndex));
		bool bOrdered = !!(pCE->c->IsOrdered(pCE->iIndex));
		bool bDefault = !!(pCE->c->IsDefault(pCE->iIndex));
		bool bChapsInClipboard = clipboard->GetChapterCount() && !clipboard->IsEdition(0);

		for (int j=0;j<CHIPE_COUNT;j++) {
			csub->AppendMenu(
				MF_STRING | ((pCE->c->GetChapterPhysicalEquiv(pCE->iIndex)==CHI_PHYSICAL_EQUIVALENTS[j])?MF_CHECKED:MF_UNCHECKED), 
				IDM_CHI_PHYSICALEQUIV_BASE + j, 
				physicalequiv2string(CHI_PHYSICAL_EQUIVALENTS[j]));
		}

		cUIDs->AppendMenu(MF_STRING | ((pCE->c->GetUIDLen(pCE->iIndex) == 4)?MF_CHECKED:MF_UNCHECKED),
			IDM_CHAPTER_REINIT_UID_32, "32 bit");
		cUIDs->AppendMenu(MF_STRING | ((pCE->c->GetUIDLen(pCE->iIndex) == 8)?MF_CHECKED:MF_UNCHECKED),
			IDM_CHAPTER_REINIT_UID_64, "64 bit");



		if (bEdition) {
			c->AppendMenu(MF_STRING | (bHidden?MF_CHECKED:MF_UNCHECKED), 
				IDM_HIDECHAPTER, LoadString(STR_CHPDLG_POPUP_EDITIONHIDDEN));

			c->AppendMenu(MF_STRING | (bOrdered?MF_CHECKED:MF_UNCHECKED), 
				IDM_ORDEREDEDITION, LoadString(STR_CHPDLG_POPUP_EDITIONORDERD));

			c->AppendMenu(MF_STRING | (bDefault?MF_CHECKED:MF_UNCHECKED), 
				IDM_DEFAULTEDITION, LoadString(STR_CHPDLG_POPUP_EDITIONDEFAULT));

			if (clipboard->GetChapterCount() && clipboard->IsEdition(0)) {
				c->AppendMenu(MF_STRING, IDM_EDITION_TO_CHAPTER, "Convert Editions in clipboard to Chapters");
			}

		} else {
			c->AppendMenu(MF_STRING | (bEnabled)?MF_CHECKED:MF_UNCHECKED, 
				IDM_ENABLECHAPTER, LoadString(STR_CHPDLG_POPUP_CHAPTERENABLED));

			c->AppendMenu(MF_STRING | (bHidden)?MF_CHECKED:MF_UNCHECKED,
				IDM_HIDECHAPTER, LoadString(STR_CHPDLG_POPUP_CHAPTERHIDDEN));

			c->AppendMenu(MF_POPUP, (UINT)csub->m_hMenu,"physical equivalent"); 
		}
		c->AppendMenu(MF_SEPARATOR);

		if (pCE->c->HasSubChapters(pCE->iIndex)) {
			c->AppendMenu(MF_STRING, IDM_CHAPTER_NUMERATE, LoadString(STR_CHPDLG_POPUP_NUMERATE));
			c->AppendMenu(MF_SEPARATOR);
		}

		c->AppendMenu(MF_STRING | (clipboard->GetChapterCount() &&
			clipboard->IsEdition(0) != pCE->c->IsEdition(pCE->iIndex))?MF_GRAYED:0, 
			IDM_CHAPTER_COPY, LoadString(STR_CHPDLG_POPUP_COPY));
		
		c->AppendMenu(MF_STRING | (clipboard->GetChapterCount() &&
			clipboard->IsEdition(0) != pCE->c->IsEdition(pCE->iIndex))?MF_GRAYED:0, 
			IDM_CHAPTER_COPY + 1000, LoadString(STR_CHPDLG_POPUP_COPYALL));

		c->AppendMenu(MF_STRING | (clipboard->GetChapterCount() &&
			clipboard->IsEdition(0) != pCE->c->IsEdition(pCE->iIndex))?MF_GRAYED:0, 
			IDM_CHAPTER_COPY_CHILDREN, LoadString(STR_CHPDLG_POPUP_COPYCHILDREN));
		c->AppendMenu(MF_STRING | (clipboard->GetChapterCount() &&
			clipboard->IsEdition(0) != pCE->c->IsEdition(pCE->iIndex))?MF_GRAYED:0, 
			IDM_CHAPTER_COPY_CHILDREN + 1000, LoadString(STR_CHPDLG_POPUP_COPYALLCHILDREN));

		

		c->AppendMenu(MF_SEPARATOR);
		
		c->AppendMenu(MF_STRING | ((clipboard->GetChapterCount() && 
			clipboard->IsEdition(0) == pCE->c->IsEdition(pCE->iIndex))?MF_ENABLED:MF_GRAYED),
			IDM_CHAPTER_PASTE_AFTER, LoadString(STR_CHPDLG_POPUP_PASTEONSAME));

		c->AppendMenu(MF_STRING | (bChapsInClipboard?MF_ENABLED:MF_GRAYED),
			IDM_CHAPTER_PASTE_AS_CHILD, LoadString(STR_CHPDLG_POPUP_PASTEASSUBSCHAPS));

		c->AppendMenu(MF_SEPARATOR);

		c->AppendMenu(MF_STRING | (bChapsInClipboard?MF_ENABLED:MF_GRAYED),
			IDM_CHAPTER_PASTE_INTO_NEW_EDITION, LoadString(STR_CHPDLG_POPUP_PASTEALLINTOONEEDITION));

		c->AppendMenu(MF_STRING | (bChapsInClipboard?MF_ENABLED:MF_GRAYED),
			IDM_CHAPTER_PASTE_EACH_INTO_EDITION, LoadString(STR_CHPDLG_POPUP_PASTEEACHINTOONEEDITION));

		c->AppendMenu(MF_SEPARATOR);
		c->AppendMenu(MF_STRING | (clipboard->GetChapterCount()?MF_ENABLED:MF_GRAYED),
			IDM_CHAPTER_DELETE_CLIPBOARD, LoadString(STR_CHPDLG_POPUP_DELETECLIPBOARD));

		c->AppendMenu(MF_SEPARATOR);
		c->AppendMenu(MF_POPUP, (UINT)cUIDs->m_hMenu, LoadString(STR_CHPDLG_POPUP_REINITUIDS));

		CMenu* cUIDs = new CMenu;
		cUIDs->CreatePopupMenu();
		cUIDs->AppendMenu(MF_STRING | ((pCE->c->GetUIDLen(pCE->iIndex) == 4)?MF_CHECKED:MF_UNCHECKED),
			IDM_CHAPTER_REINIT_UID_32 + 1000, "32 bit");
		cUIDs->AppendMenu(MF_STRING | ((pCE->c->GetUIDLen(pCE->iIndex) == 8)?MF_CHECKED:MF_UNCHECKED),
			IDM_CHAPTER_REINIT_UID_64 + 1000, "64 bit");
		c->AppendMenu(MF_POPUP, (UINT)cUIDs->m_hMenu, LoadString(STR_CHPDLG_POPUP_REINITALLUIDS));


		ClientToScreen(&point);
		c->TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON,point.x,point.y,this);
	}

	delete csub;
	delete c;
}

BOOL CChapterDlgTree::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	// TODO: Speziellen Code hier einfügen und/oder Basisklasse aufrufen
	CHAPTER_ENTRY* pCE = NULL;
	HTREEITEM hItem = (hContextMenuTarget?hContextMenuTarget:GetSelectedItem());
	RECT r;
	GetItemRect(hItem, &r, false);

	WORD  command = LOWORD(wParam);
	int   all = 0;

	if (hItem)
	pCE = (CHAPTER_ENTRY*)GetItemData(hItem);

	if (command > 41000 && command < 42000) {
		all = 1;
		command -= 1000;
	}		

	int start = (all?0:pCE->iIndex);
	int end = (all?pCE->c->GetChapterCount()-1:pCE->iIndex);

	for (int k=start;k<=end;k++) {
		if (command == IDM_ENABLECHAPTER) {
			pCE->c->EnableChapter(pCE->iIndex, !pCE->c->IsEnabled(pCE->iIndex));
			InvalidateRect(&r);
			UpdateWindow();
		} else
		if (command == IDM_HIDECHAPTER) {
			pCE->c->HideChapter(pCE->iIndex, !pCE->c->IsHidden(pCE->iIndex));
			InvalidateRect(&r);
			UpdateWindow();
		} else
		if (command == IDM_DEFAULTEDITION) {
			pCE->c->SetIsDefault(pCE->iIndex, !pCE->c->IsDefault(pCE->iIndex));
			InvalidateRect(NULL);
			UpdateWindow();
		} else
		if (command == IDM_ORDEREDEDITION) {
			pCE->c->SetIsOrdered(pCE->iIndex, !pCE->c->IsOrdered(pCE->iIndex));
			InvalidateRect(&r);
			UpdateWindow();
		} else
		if (command >= IDM_CHI_PHYSICALEQUIV_BASE && 
			command < IDM_CHI_PHYSICALEQUIV_BASE + CHIPE_COUNT) {
			pCE->c->SetChapterPhysicalEquiv(pCE->iIndex, 
				CHI_PHYSICAL_EQUIVALENTS[command - IDM_CHI_PHYSICALEQUIV_BASE]);
				
			InvalidateRect(&r);
			UpdateWindow();
		} else
		if (command == IDM_CHAPTER_COPY) {
			int iIndex = clipboard->Import(pCE->c, 0, 0, k, k);
			clipboard->InvalidateUID(iIndex);
		} else
		if (command == IDM_CHAPTER_COPY_CHILDREN) {
			CChapters* c = pCE->c->GetSubChapters(k);
			for (int j=0;j<c->GetChapterCount();j++) {
				int iIndex = clipboard->Import(c, 0, 0, j, j);
				clipboard->InvalidateUID(iIndex);
			}
		} else
		if (command == IDM_CHAPTER_PASTE_AFTER) {
			int j = pCE->c->Import(clipboard);

			AddChaptersToTree(this, GetParentItem(hItem), pCE->c,
				j, CHAP_LAST);
			clipboard->Delete();
			UpdateWindow();
		} else
		if (command == IDM_CHAPTER_PASTE_AS_CHILD) {
			int j = pCE->c->GetSubChapters(pCE->iIndex)->Import(clipboard);

			AddChaptersToTree(this, hItem, pCE->c->GetSubChapters(pCE->iIndex),
				j, CHAP_LAST);
			clipboard->Delete();
			InvalidateRect(NULL);
			UpdateWindow();
		} else
		if (command == IDM_CHAPTER_PASTE_INTO_NEW_EDITION) {
			int iIndex = c->AddEmptyEdition();
			c->GetSubChapters(iIndex)->Import(clipboard);
			AddChaptersToTree(this, NULL, c, iIndex, iIndex);
			InvalidateRect(NULL);
			UpdateWindow();
			clipboard->Delete();
		} else
		if (command == IDM_CHAPTER_PASTE_EACH_INTO_EDITION) {
			int count = clipboard->GetChapterCount();
			for (int j=0;j<count;j++) {
				int iIndex = c->AddEmptyEdition();
				c->SetUIDLen(iIndex, clipboard->GetUIDLen(j),0);
				c->SetIsOrdered(iIndex, 1);
				c->GetSubChapters(iIndex)->Import(clipboard, 0, 0, j, j);
				c->CopyChapterDisplay(clipboard, j, iIndex);
				AddChaptersToTree(this, NULL, c, iIndex, iIndex);
			}
			InvalidateRect(NULL);
			UpdateWindow();
			clipboard->Delete();
		} else
		if (command == IDM_CHAPTER_REINIT_UID_64) {
			pCE->c->SetUIDLen(k, 8, CHAP_SETUIDLEN_RECURSIVE);
			InvalidateRect(NULL);
			UpdateWindow();
			((CChapterDlg*)GetParent())->UpdateChapterUIDEdit(pCE->c,pCE->iIndex);
		} else
		if (command == IDM_CHAPTER_REINIT_UID_32) { 
			pCE->c->SetUIDLen(k, 4, CHAP_SETUIDLEN_RECURSIVE);
			InvalidateRect(NULL);
			UpdateWindow();
			((CChapterDlg*)GetParent())->UpdateChapterUIDEdit(pCE->c,pCE->iIndex);
		} else
		if (command == IDM_CHAPTER_DELETE_CLIPBOARD) {
			clipboard->Delete();
		} else
		if (command == IDM_EDITION_TO_CHAPTER) {
			for (int z=0;z<clipboard->GetChapterCount();z++) {
				if (!clipboard->IsEdition(z))
					MessageBox("Fatal internal error: Chapter in Clipboard where should have been an Edition",
					"Fatal error", MB_OK);
				clipboard->SetIsEdition(z, 0);

				if (clipboard->HasSubChapters(z)) {
					clipboard->SetChapterBegin(z, clipboard->GetSubChapters(z)->GetChapterBegin(0));
					clipboard->SetChapterEnd(z, clipboard->GetSubChapters(z)->GetChapterEnd(CHAP_LAST));
				} else {
					clipboard->SetChapterBegin(z, 0);
					clipboard->SetChapterEnd(z, -1);
				}
				
			}
		} else
		if (command == IDM_CHAPTER_NUMERATE) {
			CChapters* sub = pCE->c->GetSubChapters(pCE->iIndex);
			for (int i=0;i<sub->GetChapterCount();i++) {
				char c[64]; c[0]=0;
				sprintf(c, "Chapter %02d", i+1);
				sub->SetChapterText(i, c, 0);
				sub->SetChapterLng(i, "eng", 0);

				sprintf(c, "Kapitel %02d", i+1);
				sub->SetChapterText(i, c, 1);
				sub->SetChapterLng(i, "ger", 1);

				sprintf(c, "Chapitre %02d", i+1);
				sub->SetChapterText(i, c, 2);
				sub->SetChapterLng(i, "fre", 2);

				sprintf(c, "Capitulo %02d", i+1);
				sub->SetChapterText(i, c, 3);
				sub->SetChapterLng(i, "spa", 3);

				sprintf(c, "Capitolo %02d", i+1);
				sub->SetChapterText(i, c, 4);
				sub->SetChapterLng(i, "ita", 4);

				while (sub->GetChapterDisplayCount(i) > 4)
					sub->DeleteChapterDisplay(i, 4);
			}

			HTREEITEM hChild = GetChildItem(hItem);
			while (hChild) {
				RECT r; GetItemRect(hChild, &r, true);
				InvalidateRect(&r, false);
				hChild = GetNextSiblingItem(hChild);
			}
			UpdateWindow();
		}
	}
	
	return CUnicodeTreeCtrl::OnCommand(wParam, lParam);
}
