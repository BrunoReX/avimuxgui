// AudioSourceTree.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "AVIMux_GUI.h"
#include "AudioSourceTree.h"
#include "formattext.h"
#include "audiosource.h"
#include "AVIMux_GUIDlg.h"
#include "Muxing.h"
#include "..\FileStream.h"
#include "..\..\Common\utf-8.h"
#include "UnicodeTreeCtrl.h"
#include "FileDialogs.h"
#include ".\audiosourcetree.h"
#include "Trees.h"
#include <sstream>
#include <iomanip>
#include <fstream>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAudioSourceTree

CAudioSourceTree::CAudioSourceTree()
{
	EnableAutomation();
	bDragging = false;
}

CAudioSourceTree::~CAudioSourceTree()
{
}

#define newTII(a,b) a=new TREE_ITEM_INFO; \
	a->iID = b; \
	a->pText = new char[256]; \
	memset(a->pText, 0, 256);

void CAudioSourceTree::AddTitleToStreamTree(HTREEITEM hParent, 
										//  MULTIMEDIASOURCE_INFO* msi,
										  char* cLng, char* cTitle)
{
	TREE_ITEM_INFO* tii;
	HTREEITEM hItem1, hItem2;

	/* first verify that the language hasn't been set so far */
	hItem1 = this->GetChildItem(hParent);
	while (hItem1) {
		tii = GetItemInfo(hItem1);
		if (tii->iID == TIIID_TITLE) {
			hItem2 = FindID(hItem1, TIIID_TITLELNG, &tii);
			if (hItem2 && tii && tii->pText) {
				if (!strcmp(cLng, tii->pText)) {
					hItem2 = FindID(hItem1, TIIID_STRNAME, &tii);
					if (tii->pText)
						delete tii->pText;

					if (cTitle)
					{
						tii->pText = _strdup(cTitle);
						tii = GetItemInfo(hParent);
						tii->pMSI->mms->GetTitleSet()->SetTitleForLanguage(cLng, cTitle);
					}
					else
					{
						DeleteTitleFromStreamTree(hItem1);
					}
					return;
				}
			}
		}
		hItem1 = GetNextSiblingItem(hItem1);
	}

	newTII(tii, TIIID_TITLE);
	SetItemData(
		hItem1=Tree_InsertCheck_Callback(this, hParent), 
		(DWORD)tii);
	ShowItemCheckBox(hItem1, false);

	newTII(tii, TIIID_TITLELNG);
	strcpy(tii->pText, cLng);
	SetItemData(
		hItem2=Tree_InsertCheck_Callback(this, hItem1),
		(DWORD)tii);
	ShowItemCheckBox(hItem2, false);

	newTII(tii, TIIID_STRNAME);
	strcpy(tii->pText, cTitle);
	SetItemData(
		hItem2=Tree_InsertCheck_Callback(this, hItem1),
		(DWORD)tii);
	ShowItemCheckBox(hItem2, false);
}

void CAudioSourceTree::DeleteTitleFromStreamTree(HTREEITEM hTitle)
{
	HTREEITEM hChild = GetChildItem(hTitle);

	std::string language;
	std::string title;

	while (hChild) {
		TREE_ITEM_INFO* tii = GetItemInfo(hChild);

		if (tii->iID == TIIID_TITLELNG)
			language = tii->pText;
		if (tii->iID == TIIID_STRNAME)
			title = tii->pText;
		
		delete tii->pText;
		
		hChild = GetNextSiblingItem(hChild);
	}

	HTREEITEM hParent = GetParentItem(hTitle);
	DeleteItem(hTitle);
	SelectItem(hParent);

	TREE_ITEM_INFO* tii = GetItemInfo(hParent);
	tii->pMSI->mms->GetTitleSet()->DeleteTitle(language.c_str());
}

void CAudioSourceTree::DeleteAllTitlesFromStreamTree(HTREEITEM hParent)
{
	HTREEITEM hChild = GetChildItem(hParent);

	while (hChild)
	{
		TREE_ITEM_INFO* tii = GetItemInfo(hChild);	
		if (tii->iID == TIIID_TITLE)
			DeleteTitleFromStreamTree(hChild);

		hChild = GetNextSiblingItem(hChild);
	}
}

void CAudioSourceTree::OnFinalRelease()
{
	// Nachdem die letzte Referenz auf ein Automatisierungsobjekt freigegeben wurde,
	// wird OnFinalRelease aufgerufen. Die Basisklasse löscht das Objekt
	// automatisch. Fügen Sie zusätzlichen Bereinigungscode für Ihr Objekt
	// hinzu, bevor Sie die Basisklasse aufrufen.

	CUnicodeTreeCtrl::OnFinalRelease();
}


BEGIN_MESSAGE_MAP(CAudioSourceTree, CUnicodeTreeCtrl)
	//{{AFX_MSG_MAP(CAudioSourceTree)
	ON_NOTIFY_REFLECT(NM_RCLICK, OnRclick)
	ON_NOTIFY(TVN_BEGINDRAG, IDC_AUDIOTREE, OnBegindrag)
	ON_WM_LBUTTONUP()
	ON_NOTIFY(TVN_BEGINDRAGW, IDC_AUDIOTREE, OnBegindrag)
	ON_NOTIFY_REFLECT(TVN_BEGINDRAG, OnBegindrag)
	ON_NOTIFY_REFLECT(TVN_BEGINDRAGW, OnBegindrag)
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
	ON_NOTIFY_REFLECT(TVN_GETDISPINFO, OnTvnGetdispinfo)
	ON_WM_KEYDOWN()
END_MESSAGE_MAP()

BEGIN_DISPATCH_MAP(CAudioSourceTree, CUnicodeTreeCtrl)
	//{{AFX_DISPATCH_MAP(CAudioSourceTree)
		// HINWEIS - Der Klassen-Assistent fügt hier Zuordnungsmakros ein und entfernt diese.
	//}}AFX_DISPATCH_MAP
END_DISPATCH_MAP()

// Hinweis: Wir stellen Unterstützung für IID_IAudioSourceTree zur Verfügung, um typsicheres Binden
//  von VBA zu ermöglichen. Diese IID muss mit der GUID übereinstimmen, die in der
//  Disp-Schnittstelle in der .ODL-Datei angegeben ist.

// {3FB9EBAC-133F-4456-AD29-21475221D982}
static const IID IID_IAudioSourceTree =
{ 0x3fb9ebac, 0x133f, 0x4456, { 0xad, 0x29, 0x21, 0x47, 0x52, 0x21, 0xd9, 0x82 } };

BEGIN_INTERFACE_MAP(CAudioSourceTree, CUnicodeTreeCtrl)
	INTERFACE_PART(CAudioSourceTree, IID_IAudioSourceTree, Dispatch)
END_INTERFACE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CAudioSourceTree 

TREE_ITEM_INFO*	BuildTIIfromASI(AUDIO_STREAM_INFO* asi)
{
	TREE_ITEM_INFO* tii = new TREE_ITEM_INFO;

	tii->iHideText = 0;
	tii->iID = TIIID_ASI;
	tii->pASI = asi;

	return tii;
}

TREE_ITEM_INFO*	BuildTIIfromSSI(SUBTITLE_STREAM_INFO* ssi)
{
	TREE_ITEM_INFO* tii = new TREE_ITEM_INFO;

	tii->iHideText = 0;
	tii->iID = TIIID_SSI;
	tii->pSSI = ssi;

	return tii;
}

TREE_ITEM_INFO* BuildTIIfromVSI(VIDEO_STREAM_INFO* vsi)
{
	TREE_ITEM_INFO* tii = new TREE_ITEM_INFO;

	tii->iHideText = 0;
	tii->iID = TIIID_VSI;
	tii->pVSI = vsi;

	return tii;
}

int CALLBACK AudioTree_CompareFunc(LPARAM lParam1, LPARAM lParam2, 
		LPARAM lParamSort)
{
	TREE_ITEM_INFO* tii[2];
	UNICODETREEITEM_DATA* utid[2] = {
		(UNICODETREEITEM_DATA*)lParam1, (UNICODETREEITEM_DATA*)lParam2
	};
	
	tii[0] = (TREE_ITEM_INFO*)utid[0]->dwUserData;
	tii[1] = (TREE_ITEM_INFO*)utid[1]->dwUserData;

	if (tii[0]->iCurrPos < tii[1]->iCurrPos) return -1;
	return 1;
}


void CAudioSourceTree::Sort()
{
	TVSORTCB sort;
	int		 j = 0;

	sort.hParent = NULL;
	sort.lParam = 0;
	sort.lpfnCompare = AudioTree_CompareFunc;
	SortChildrenCB(&sort);

	HTREEITEM  hItem = GetRootItem();

	while (hItem) {
		TREE_ITEM_INFO* tii = (TREE_ITEM_INFO*)GetItemData(hItem);
		tii->iCurrPos = j;
		j+=2;
		hItem = GetNextSiblingItem(hItem);
	}

	InvalidateRect(NULL);
	UpdateWindow();
}

TREE_ITEM_INFO* CAudioSourceTree::GetItemInfo(HTREEITEM hItem)
{
	return (TREE_ITEM_INFO*)GetItemData(hItem);
}

TREE_ITEM_INFO* CAudioSourceTree::FindItemOriginalPosition(int org_pos)
{
	HTREEITEM hItem = GetRootItem();

	while (hItem) {
		if (GetItemInfo(hItem)->iOrgPos == org_pos)
			return GetItemInfo(hItem);
		hItem = GetNextSiblingItem(hItem);
	}

	return NULL;
}

HTREEITEM CAudioSourceTree::FindID(HTREEITEM hItem,int iID, TREE_ITEM_INFO** tii)
{
	hItem = GetChildItem(hItem);
	TREE_ITEM_INFO*	_tii;

	while (hItem) {
		_tii = (TREE_ITEM_INFO*)GetItemData(hItem);
		if (_tii->iID == iID) {
			if (tii) *tii = _tii;
			return hItem;
		}
		hItem = GetNextSiblingItem(hItem);
	}

	return NULL;
}

std::vector<HTREEITEM> CAudioSourceTree::GetItems(
	HTREEITEM hItem,
	int iID, 
	int iCheck, 
	std::vector<int>* indices)
{
	std::vector<HTREEITEM> a;
	
	TREE_ITEM_INFO*	tii;
	int i=0;

	while (hItem) {
		tii = (TREE_ITEM_INFO*)GetItemData(hItem);
		if (tii->iID == iID && ++i && (iCheck == -1 || Tree_GetCheckState(this,hItem))) {
			a.push_back(hItem);
			if (indices) 
				indices->push_back(i-1);
		}

		hItem = GetNextSiblingItem(hItem);
	}

	return a;
}

#define init_sep bPlaceSep = true
#define put_sep if (bPlaceSep) c->AppendMenu(MF_SEPARATOR, 0); bPlaceSep = false

void CAudioSourceTree::OpenContextMenu(CPoint point)
{
	CMenu* c = new CMenu;
	bool bPlaceSep = false;

	c->CreatePopupMenu();

	HTREEITEM hItem = GetSelectedItem();

	if (hItem && !MuxingInProgress()) {
		TREE_ITEM_INFO* tii = (TREE_ITEM_INFO*)GetItemData(hItem);
		TREE_ITEM_INFO* ptii = GetItemInfo(GetTopMostParentItem(hItem));
		bool bDefault = false;
		
/*		if (tii->iID == TIIID_ASI && tii->pASI->audiosource->IsDefault()) 
			bDefault = true;
		
		if (tii->iID == TIIID_SSI && tii->pSSI->lpsubs->IsDefault()) 
			bDefault = true;

		if (tii->iID == TIIID_VSI && tii->pVSI->videosource->IsDefault())
			bDefault = true;
*/
		if (tii->iID & TIIID_MSI)
			bDefault = !!tii->pMSI->mms->IsDefault();
		else if (ptii->iID & TIIID_MSI)
			bDefault = !!ptii->pMSI->mms->IsDefault();
		else
			bDefault = false;

		c->AppendMenu(MF_STRING | bDefault*MF_CHECKED, IDM_SETDEFAULTTRACK, 
			CUTF8(LoadString(STR_MAIN_AS_DEFAULTTRACK)).TStr());
		
		init_sep;

		if (tii->iID == TIIID_ASI) {
			AUDIO_STREAM_INFO* asi = tii->pASI;
			AUDIOSOURCE* a = asi->audiosource;
			if (asi->dwType == AUDIOTYPE_AAC) {
				put_sep;
				c->AppendMenu(MF_STRING, IDM_EXTRACT_TOADTS, 
					CUTF8(LoadString(STR_MAIN_A_EXTRACTADTS)).TStr());
			}
			if (asi->dwType == AUDIOTYPE_VORBIS) {
				put_sep;
				c->AppendMenu(MF_STRING, IDM_EXTRACT_OGGVORBIS, 
					CUTF8(LoadString(STR_MAIN_A_EXTRACTOGGV)).TStr());
			}
			if (a->GetFeature(FEATURE_EXTRACTBIN)) {
				put_sep;
				c->AppendMenu(MF_STRING, IDM_EXTRACT_BINARY, 
					CUTF8(LoadString(STR_MAIN_A_EXTRBIN)).TStr());
			}
			if (asi->dwType == AUDIOTYPE_AAC) {
				put_sep;
				c->AppendMenu(MF_STRING | (a->FormatSpecific(MMSGFS_AAC_ISSBR))?MF_CHECKED:MF_UNCHECKED,
					IDM_CHANGESBR, _T("SBR"));
			}
			
		} else 
		if (tii->iID == TIIID_SSI) {
			SUBTITLE_STREAM_INFO* ssi = tii->pSSI;
			SUBTITLESOURCE* s = ssi->lpsubs;
			if (s->GetFeature(FEATURE_SUB_EXTRACT2TEXT)) {
				put_sep;
				c->AppendMenu(MF_STRING, IDM_EXTRACT_SUB2TEXT, 
					CUTF8(LoadString(STR_MAIN_A_EXTRSUB2TEXT)).TStr());
			}
		} else
		if (tii->iID == TIIID_VSI) {
			VIDEO_STREAM_INFO* vsi = tii->pVSI;
			VIDEOSOURCE* v = vsi->videosource;
			if (v->GetFeature(FEATURE_EXTRACTBIN)) {
				put_sep;
				c->AppendMenu(MF_STRING, IDM_EXTRACT_BINARY, 
					CUTF8(LoadString(STR_MAIN_A_EXTRBIN)).TStr());
			}
		}

		if (tii->iID & TIIID_MSI) {
			c->AppendMenu(MF_STRING, IDM_NEWSTREAMLANGUAGE, 
				_T("New stream title..."));
		}

		if (tii->iID & TIIID_TITLE) {
			c->AppendMenu(MF_STRING, IDM_DELETESTREAMLANGUAGE, 
				_T("Delete stream title..."));
		}

		ClientToScreen(&point);
		c->TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON,point.x,point.y,this);
	}

	delete c;
}
/*
void CAudioSourceTree::OnRButtonUp(UINT nFlags, CPoint point) 
{
	// TODO: Code für die Behandlungsroutine für Nachrichten hier einfügen und/oder Standard aufrufen

	//Beep(100,200);

//	OpenContextMenu(point);

	CUnicodeTreeCtrl::OnRButtonUp(nFlags, point);
}
*/
typedef struct 
{
	CFileStream* file;
	CAVIMux_GUIDlg* dlg;
	int id;
	union {
		AUDIOSOURCE* a;
		VIDEOSOURCE* v;
		MULTIMEDIASOURCE* m;
	};

} EXTRACT_THREAD_DATA;

int ExtractThread(EXTRACT_THREAD_DATA*	lpETD)
{
	lpETD->dlg->SetDialogState_Muxing();
	lpETD->dlg->ButtonState_START();

	lpETD->dlg->AddProtocolLine("started extracting binary", 4);

	AUDIOSOURCE* a = lpETD->a;
	lpETD->m->Enable(1);
	CFileStream* f = lpETD->file;
	char cTime[20];
	char* lpBuffer = new char[1<<20];
	int iLastTime = GetTickCount();
	lpETD->m->ReInit();

	while (!a->IsEndOfStream() && !DoStop()) {
		__int64 iTimecode;
		__int64 iNS;

		if (lpETD->id == TIIID_ASI)
			f->Write(lpBuffer,a->Read(lpBuffer,1000000,NULL,&iNS,&iTimecode));
		if (lpETD->id == TIIID_VSI) {
			DWORD dwSize;
			ADVANCEDREAD_INFO ari;
			lpETD->v->GetFrame(lpBuffer, &dwSize, &iTimecode, &ari);
			iNS = ari.iDuration;
			f->Write(lpBuffer,dwSize);
		}

		if (GetTickCount()-iLastTime>100 || a->IsEndOfStream()) {
			Millisec2Str((iTimecode * a->GetTimecodeScale() + iNS)/ 1000000,cTime);
			lpETD->dlg->m_Prg_Frames.SetWindowText(CUTF8(cTime).TStr());
			iLastTime+=100;
		}
	}

	lpETD->dlg->SetDialogState_Config();
	lpETD->dlg->ButtonState_STOP();
	delete lpBuffer;
	StopMuxing(false);
	lpETD->file->Close();
	lpETD->dlg->AddProtocol_Separator();
	delete lpETD;

	a->Enable(0);

	return 1;
}

int ExtractThread_ADTS(EXTRACT_THREAD_DATA*	lpETD)
{
	lpETD->dlg->SetDialogState_Muxing();
	lpETD->dlg->ButtonState_START();

	lpETD->dlg->AddProtocolLine("started extracting to ADTS", 4);

	AUDIOSOURCE* a = (AACSOURCE*)lpETD->a;
	a->Enable(1);
	CFileStream* f = lpETD->file;
	char cTime[20];
	char* lpBuffer_in  = new char[1<<18];
	char* lpBuffer_out = new char[1<<18];
	int iLastTime = GetTickCount();
	a->ReInit();
	while (!a->IsEndOfStream() && !DoStop()) {
		__int64 iTimecode;
	
		f->Write(lpBuffer_out,Pack2ADTS(lpBuffer_in, lpBuffer_out, a, 
			a->Read(lpBuffer_in,1000000,NULL,NULL,&iTimecode)));
		
		if (GetTickCount()-iLastTime>100 || a->IsEndOfStream()) {
			Millisec2Str(iTimecode * a->GetTimecodeScale() / 1000000,cTime);
			lpETD->dlg->m_Prg_Frames.SetWindowText(CUTF8(cTime).TStr());
			iLastTime+=100;
		}
	}

	lpETD->dlg->SetDialogState_Config();
	lpETD->dlg->ButtonState_STOP();
	delete lpBuffer_in;
	delete lpBuffer_out;
	StopMuxing(false);
	lpETD->file->Close();
	lpETD->dlg->AddProtocol_Separator();

	delete lpETD;
	a->Enable(0);

	return 1;
}

int ExtractThread_OGGVorbis(EXTRACT_THREAD_DATA*	lpETD)
{
	char cBuffer[50]; cBuffer[0]=0; __int64 j;
	lpETD->dlg->AddProtocolLine("started extracting to OGG", 4);
	sprintf(cBuffer, "using page size of %d kByte", 
		(j=lpETD->dlg->GetSettings()->GetInt("output/ogg/pagesize")+512)>>10);
	lpETD->dlg->AddProtocolLine(cBuffer, 4);

	ADVANCEDREAD_INFO ARI;

	lpETD->dlg->SetDialogState_Muxing();
	lpETD->dlg->ButtonState_START();

	DWORD	dwSize[3]; void* pHeader[3];
	DWORD	dwArg[2] = { (DWORD)dwSize, (DWORD)pHeader };
	__int64 arg = *((__int64*)dwArg);
	

	AUDIOSOURCE* a = lpETD->a;
	a->Enable(1);
	a->FormatSpecific(MMSGFS_VORBIS_CONFIGPACKETS, arg);
	a->ReInit();

	CFileStream* f = lpETD->file;
	OGGFILE* ogg = new OGGFILE;
	ogg->Open(f, OGG_OPEN_WRITE);
	ogg->SetMaxPageSize((int)j);
	ogg->SetTimestampMode(OGG_TSM_ABSOLUTE);
	ogg->WritePacket(pHeader[0], dwSize[0], 0);
	ogg->RenderPage();
	ogg->WritePacket(pHeader[1], dwSize[1], 0);
	ogg->WritePacket(pHeader[2], dwSize[2], 0);
	ogg->RenderPage();
	
	char cTime[20];
	char* lpBuffer_in  = new char[1<<18];
	int iLastTime = GetTickCount();
	__int64 iFirstTimecode = -0x7FFFFFFF;

	while (!a->IsEndOfStream() && !DoStop()) {
		__int64 iTimecode = 0;
	
		int size = a->Read(lpBuffer_in, 1, NULL, NULL, &iTimecode, &ARI);
		if (iFirstTimecode == -0x7FFFFFFF)
			iFirstTimecode = iTimecode;

		iTimecode -= iFirstTimecode;
		
		if (size!=AS_ERR) {
			__int64 samples = 0;
			if (ARI.iNextTimecode <= 0) {
				samples = (((__int64)a->GetFrequency() * 
					(iTimecode) * a->GetTimecodeScale()) + 500000000) / 1000000000;
			} else {
				samples = (((__int64)a->GetFrequency() * 
					(ARI.iNextTimecode) * a->GetTimecodeScale()) + 500000000) / 1000000000;
			}
		
			if (ARI.iFramecount) {
				ogg->WritePackets(lpBuffer_in, ARI.iFramecount, ARI.iFramesizes, samples);
			} else {
				ogg->WritePacket(lpBuffer_in, size, samples);
			}

			if (GetTickCount()-iLastTime>100 || a->IsEndOfStream()) {
				Millisec2Str(iTimecode * a->GetTimecodeScale() / 1000000,cTime);
				lpETD->dlg->m_Prg_Frames.SetWindowText(CUTF8(cTime).TStr());
				iLastTime+=100;
			}	
		} else {
			StopMuxing(true);
		}
	}

	lpETD->dlg->SetDialogState_Config();
	lpETD->dlg->ButtonState_STOP();
	delete lpBuffer_in;
	StopMuxing(false);

	ogg->Close(true);
	lpETD->dlg->AddProtocol_Separator();
	delete lpETD;
	a->Enable(0);
	
	return 1;
}

BOOL CAudioSourceTree::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	// TODO: Speziellen Code hier einfügen und/oder Basisklasse aufrufen
	CWinThread*	thread;
//	CFileDialog*	cfd;
	CAVIMux_GUIDlg*	cMainDlg = (CAVIMux_GUIDlg*)GetParent();
	
	std::basic_string<TCHAR> strFilter;
	std::basic_string<TCHAR> strDefExt;

	int iFmt;
	char* cFmt;
	int open = 0;
	HTREEITEM hItem;
	TREE_ITEM_INFO* tii;
	AUDIO_STREAM_INFO* asi;
	VIDEO_STREAM_INFO* vsi;
	MULTIMEDIA_STREAM_INFO* msi;
	AUDIOSOURCE* a;
	VIDEOSOURCE* v;
	SUBTITLE_STREAM_INFO* ssi;
	SUBTITLESOURCE* s;
	OPENFILENAME o; 
	memset(&o, 0, sizeof(o));

	switch (LOWORD(wParam))
	{
		case IDM_NEWSTREAMLANGUAGE:
			hItem = GetSelectedItem();
			tii = GetItemInfo(hItem);
			msi = tii->pMSI;
			AddTitleToStreamTree(hItem, "<new>", "<new>");
			break;
		case IDM_DELETESTREAMLANGUAGE:
			hItem = GetSelectedItem();
			DeleteTitleFromStreamTree(hItem);
			break;

		case IDM_CHANGESBR:
			hItem = GetSelectedItem();
			tii = (TREE_ITEM_INFO*)GetItemData(hItem);
			asi = tii->pASI;
			a = asi->audiosource;
			a->FormatSpecific(MMSSFS_AAC_SETSBR, !a->FormatSpecific(MMSGFS_AAC_ISSBR));
			FillAAC_ASI(&asi, (AACSOURCE*)a);
			InvalidateRect(NULL);
			break;
		case IDM_EXTRACT_BINARY:
			hItem = GetSelectedItem();
			tii = (TREE_ITEM_INFO*)GetItemData(hItem);
			asi = tii->pASI;
			msi = tii->pMSI;
			vsi = tii->pVSI;


			if (tii->iID == TIIID_ASI) {
				a = asi->audiosource;
				v = NULL;
			} else {
				a = NULL;
				v = vsi->videosource;
			}


			if (a) {
				iFmt = a->GetFormatTag();
				cFmt = a->GetCodecID();
			} else {
				cFmt = "";
				iFmt = 0;
			}
			
			if (iFmt == 0x0055 || cFmt && !strcmp(cFmt,"A_MPEG/L3")) {
				strFilter = _T("*.mp3|*.mp3||"); 
				strDefExt = _T("mp3");
			} else
			if (iFmt == 0x0050 || cFmt && !strcmp(cFmt,"A_MPEG/L2")) {
				strFilter = _T("*.mp2|*.mp2||");
				strDefExt = _T("mp2");
			} else
			if (iFmt == 0x2000 || cFmt && !strcmp(cFmt,"A_AC3")) {
				strFilter = _T("*.ac3|*.ac3||");
				strDefExt = _T("ac3");
			} else 
			if (iFmt == 0x2001 || cFmt && !strcmp(cFmt,"A_DTS")) {
				strFilter = _T("*.dts|*.dts||");
				strDefExt = _T("dts");
			} else {
				strFilter = _T("*.raw|*.raw||");
				strDefExt = _T("raw");
			}


			PrepareSimpleDialog(&o, m_hWnd, strFilter.c_str());
			o.Flags |= OFN_OVERWRITEPROMPT;
			o.lpstrDefExt = strDefExt.c_str();
			open = GetOpenSaveFileNameUTF8(&o, 0);
			
			if (open) {
				EXTRACT_THREAD_DATA* lpETD = new EXTRACT_THREAD_DATA;
				lpETD->file = new CFileStream;
				if (lpETD->file->Open(o.lpstrFile,StreamMode::Write) != STREAM_ERR) {
					lpETD->dlg = cMainDlg;
					lpETD->id = tii->iID;
					if (a)
						lpETD->a = a;
					else
					if (v)
						lpETD->v = v;
					cMainDlg->m_Prg_Dest_File.SetWindowText(o.lpstrFile);
					thread=AfxBeginThread((AFX_THREADPROC)ExtractThread,lpETD);
				} else {
					MessageBox(
						CUTF8(LoadString(IDS_COULDNOTOPENOUTPUTFILE)).TStr(),
						CUTF8(LoadString(STR_GEN_ERROR)).TStr(),
						MB_OK | MB_ICONERROR);
				}
			}
			
			break;
		case IDM_EXTRACT_TOADTS:
			strFilter = _T("*.aac|*.aac||");
			strDefExt = _T("aac");
			hItem = GetSelectedItem();
			tii = (TREE_ITEM_INFO*)GetItemData(hItem);
			asi = tii->pASI;
			a = asi->audiosource;
			
			PrepareSimpleDialog(&o, m_hWnd, strFilter.c_str());
			o.Flags |= OFN_OVERWRITEPROMPT;
			o.lpstrDefExt = strDefExt.c_str();
			open = GetOpenSaveFileNameUTF8(&o, 0);

			//cfd=new CFileDialog(false, cDefExt, NULL, OFN_OVERWRITEPROMPT, cFilter);
			
			if (open) {
				EXTRACT_THREAD_DATA* lpETD = new EXTRACT_THREAD_DATA;
				lpETD->file = new CFileStream;
				if (lpETD->file->Open(o.lpstrFile,StreamMode::Write)!=STREAM_ERR) {
					lpETD->dlg = cMainDlg;
					lpETD->a = a;
					cMainDlg->m_Prg_Dest_File.SetWindowText(o.lpstrFile);
					thread=AfxBeginThread((AFX_THREADPROC)ExtractThread_ADTS,lpETD);
				} else {
					MessageBox(
						CUTF8(LoadString(IDS_COULDNOTOPENOUTPUTFILE)).TStr(),
						CUTF8(LoadString(STR_GEN_ERROR)).TStr(),
						MB_OK | MB_ICONERROR);
				}
			}
			break;
		case IDM_EXTRACT_OGGVORBIS:
			strFilter = _T("*.ogg|*.ogg||");
			strDefExt = _T("ogg");

			hItem = GetSelectedItem();
			tii = (TREE_ITEM_INFO*)GetItemData(hItem);
			asi = tii->pASI;
			a = asi->audiosource;
		//	cfd=new CFileDialog(false, cDefExt, NULL, OFN_OVERWRITEPROMPT, cFilter);
		
			PrepareSimpleDialog(&o, m_hWnd, strFilter.c_str());
			o.Flags |= OFN_OVERWRITEPROMPT;
			o.lpstrDefExt = strDefExt.c_str();
			open = GetOpenSaveFileNameUTF8(&o, 0);
			
			if (open) {
				EXTRACT_THREAD_DATA* lpETD = new EXTRACT_THREAD_DATA;
				lpETD->file = new CFileStream;
				if (lpETD->file->Open(o.lpstrFile,StreamMode::Write)!=STREAM_ERR) {
					lpETD->dlg = cMainDlg;
					lpETD->a = a;
					cMainDlg->m_Prg_Dest_File.SetWindowText(o.lpstrFile);
					thread=AfxBeginThread((AFX_THREADPROC)ExtractThread_OGGVorbis,lpETD);
				} else {
					MessageBox(
						CUTF8(LoadString(IDS_COULDNOTOPENOUTPUTFILE)).TStr(),
						CUTF8(LoadString(STR_GEN_ERROR)).TStr(),
						MB_OK | MB_ICONERROR);
				}
			}
			break;
		case IDM_EXTRACT_SUB2TEXT:
			hItem = GetSelectedItem();
			tii = (TREE_ITEM_INFO*)GetItemData(hItem);
			ssi = tii->pSSI;
			s = ssi->lpsubs;
			s->Enable(1);

			switch (s->GetFormat()) {
				case SUBFORMAT_SRT:
					strFilter = _T("*.srt|*.srt||");
					strDefExt = _T("srt");
					break;
				case SUBFORMAT_SSA:
					strFilter = _T("*.ssa|*.ssa||");
					strDefExt = _T("ssa");
					break;
			}

			s->Seek(0);
			s->SetRange(0,0x7FFFFFFFFFFFFFFF);
//			cfd=new CFileDialog(false, cDefExt, NULL, OFN_OVERWRITEPROMPT, cFilter);

			PrepareSimpleDialog(&o, m_hWnd, strFilter.c_str());
			o.Flags |= OFN_OVERWRITEPROMPT;
			o.lpstrDefExt = strDefExt.c_str();
			open = GetOpenSaveFileNameUTF8(&o, 0);

			if (open) {
				CFileStream* f = new CFileStream;
				if (f->Open(o.lpstrFile, StreamMode::Write)!=STREAM_ERR) {
					char* lpBuffer = new char[2<<23];
					f->Write(lpBuffer,s->Render2Text(lpBuffer));
					f->Close();
					delete f;
					delete lpBuffer;
				} else {
					MessageBox(
						CUTF8(LoadString(IDS_COULDNOTOPENOUTPUTFILE)).TStr(),
						CUTF8(LoadString(STR_GEN_ERROR)).TStr(),
						MB_OK | MB_ICONERROR);
				}

			}
			s->Enable(0);
			break;
		case IDM_SETDEFAULTTRACK:
			hItem = GetSelectedItem();
			tii = (TREE_ITEM_INFO*)GetItemData(hItem);
			if (!(tii->iID & TIIID_MSI))
				tii = GetItemInfo(GetTopMostParentItem(hItem));
			tii->ResetFont();

			if (tii) {
				if (tii->iID == TIIID_ASI) 
					tii->pASI->audiosource->SetDefault(!tii->pASI->audiosource->IsDefault());
				
				if (tii->iID == TIIID_SSI)
					tii->pSSI->lpsubs->SetDefault(!tii->pSSI->lpsubs->IsDefault());
				
				if (tii->iID == TIIID_VSI)
					tii->pVSI->videosource->SetDefault(!tii->pVSI->videosource->IsDefault());

				InvalidateRect(NULL);
			}

			break;
	}
	
	return CUnicodeTreeCtrl::OnCommand(wParam, lParam);
}

LRESULT CAudioSourceTree::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	// TODO: Speziellen Code hier einfügen und/oder Basisklasse aufrufen

	switch (message) {
		case WM_RBUTTONDOWN:
			b_rdown = 1;
			mouse_x = LOWORD(lParam);
			mouse_y = HIWORD(lParam);
			break;
/*		case TVM_HITTEST:
			LRESULT t = CUnicodeTreeCtrl::WindowProc(message, wParam, lParam);
			break; */
	}
	
	return CUnicodeTreeCtrl::WindowProc(message, wParam, lParam);
}

void CAudioSourceTree::OnRclick(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen
//				b_rdown = 0;
	CPoint p;
	TVHITTESTINFO tvhi;

	tvhi.pt.x = GetMouseX();
	tvhi.pt.y = GetMouseY();
	HitTest(&tvhi);

	if (GetSelectedItem() == tvhi.hItem && tvhi.flags == TVHT_ONITEMLABEL) {
		p.x = GetMouseX();
		p.y = GetMouseY();
		OpenContextMenu(p);
	}
	
	*pResult = 0;
}


void CAudioSourceTree::OnBegindrag(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen
	
	bDragging = true;

	dragged_item = pNMTreeView->itemNew;
	drag_source_point = pNMTreeView->ptDrag;

	*pResult = 0;
}


void CAudioSourceTree::OnLButtonUp(UINT nFlags, CPoint point) 
{
	unsigned int	iFlags;
	HTREEITEM	hItem;
	// TODO: Code für die Behandlungsroutine für Nachrichten hier einfügen und/oder Standard aufrufen
	if (bDragging) {
		bDragging = false;
		hItem = HitTest(point, &iFlags);
		TREE_ITEM_INFO* tii2 = (TREE_ITEM_INFO*)GetItemData(hItem);
	
		if (hItem && (tii2->iID == TIIID_ASI || tii2->iID == TIIID_SSI || tii2->iID == TIIID_VSI)) {
			TREE_ITEM_INFO* tii1 = (TREE_ITEM_INFO*)GetItemData(dragged_item.hItem);

			bool bDown = (point.y > drag_source_point.y);

			if (iFlags & TVHT_ABOVE) {
				tii1->iCurrPos = tii2->iCurrPos + 1*(-1* bDown);
			}
			if (iFlags & TVHT_BELOW) {
				tii1->iCurrPos = tii2->iCurrPos - 1*(-1* bDown);
			}
			if (iFlags & TVHT_ONITEM) {
				tii1->iCurrPos = tii2->iCurrPos - 1*(-1* bDown);
			}
			
			Sort();
		}
	}

	CUnicodeTreeCtrl::OnLButtonUp(nFlags, point);
}

int CAudioSourceTree::GetRootCount()
{
	HTREEITEM hItem;
	int iCount = !!(hItem = GetRootItem());

	while (hItem = GetNextSiblingItem(hItem)) iCount++;

	return iCount;
}


int CAudioSourceTree::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CUnicodeTreeCtrl::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// TODO: Speziellen Erstellungscode hier einfügen
	
	return 0;
}

void CAudioSourceTree::GetAllInfo(HTREEITEM hParent, std::vector<TREE_ITEM_INFO*>& result)
{
	HTREEITEM hItem;

	if (hParent)
	{
		hItem = GetChildItem(hParent);
	}
	else
	{
		hItem = GetRootItem();
	}

	while (hItem)
	{
		TREE_ITEM_INFO* item_info = GetItemInfo(hItem);
		result.push_back(item_info);
		GetAllInfo(hItem, result);
		hItem = GetNextSiblingItem(hItem);
	}
}

void CAudioSourceTree::OnTvnGetdispinfo(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTVDISPINFO pTVDispInfo = reinterpret_cast<LPNMTVDISPINFO>(pNMHDR);
	// TODO: Fügen Sie hier Ihren Kontrollbehandlungscode für die Benachrichtigung ein.
	*pResult = 0;

	AUDIO_STREAM_INFO* asi = NULL;
	TREE_ITEM_INFO*	   tii = NULL;
	TCHAR*	d = pTVDispInfo->item.pszText;
	bool	bBracket = false;

	std::basic_string<TCHAR> bitrate;
	std::basic_string<TCHAR> channels;
	std::basic_string<TCHAR> sample_rate;
	std::basic_string<TCHAR> AAC_prof_name;

	std::basic_ostringstream<TCHAR> sstrDisplayText;
	std::basic_ostringstream<TCHAR> sstrBitrate;
	std::basic_ostringstream<TCHAR> sstrChannels;
	std::basic_ostringstream<TCHAR> sstrSampleRate;
	std::basic_ostringstream<TCHAR> sstrStreamInfo;

	char	c[1024];
	ZeroMemory(c,sizeof(c));

	HTREEITEM hItem = pTVDispInfo->item.hItem;
	bool bAddComma=false;
	int i,j;
	
	if ((pTVDispInfo->item.mask & TVIF_TEXT) == 0) {
		*pResult = 0;
		return;
	}
	
	tii = GetItemInfo(hItem);

	if (tii && tii->iHideText == 1)
	{
		*pResult = 0;
		return;
	}

	if (!0) { //bEditInProgess) {
		if (tii && ((tii->iID & TIIID_ASI) == TIIID_ASI)) {
			//*d = 0;
			
			sstrDisplayText << _T("audio: ");

			if (tii) asi = tii->pASI;
			if (asi && asi->bNameFromFormatTag) {
				int idatarate = (asi->audiosource->GetAvgBytesPerSec() + 62) /125;
				if (idatarate) {
					sstrBitrate << idatarate << _T("kbps");
					bitrate = sstrBitrate.str();
				}
				else 
					bitrate = _T("unknown bitrate");

				//char* szChannels = asi->audiosource->GetChannelString();
				CUTF8 utf8Channels(asi->audiosource->GetChannelString());
				//free(szChannels);
				sstrChannels << (std::basic_string<TCHAR>)utf8Channels << _T(" Ch");
				channels = sstrChannels.str();

				int isr = (int)(0.49+asi->audiosource->GetOutputFrequency());
				if ((isr % 1000) == 0) {
					sstrSampleRate << (int)((0.49+asi->audiosource->GetOutputFrequency())/1000) << _T("kHz");
				} else {
					sstrSampleRate << (int)((0.49+asi->audiosource->GetOutputFrequency())) << _T("Hz");
				}
				sample_rate = sstrSampleRate.str();

				switch (asi->dwType) {
					case AUDIOTYPE_MP3CBR:
						i = (int)asi->audiosource->FormatSpecific(MMSGFS_MPEG_LAYERVERSION);
						j = (int)asi->audiosource->FormatSpecific(MMSGFS_MPEG_VERSION);
						sstrStreamInfo << _T("MPEG ") << j <<
							_T(" Layer ") << i <<
							_T(" (CBR ") << bitrate << _T(", ") << channels <<
							_T(", ") << sample_rate;
						bBracket = true;
						break;
					case AUDIOTYPE_MP3VBR:
						i = (int)asi->audiosource->FormatSpecific(MMSGFS_MPEG_LAYERVERSION);
						j = (int)asi->audiosource->FormatSpecific(MMSGFS_MPEG_VERSION);
						sstrStreamInfo << _T("MPEG ") << j << _T(" Layer ") << i
							<< _T(" (VBR") << _T(", ") << channels << _T(", ") 
							<< sample_rate;
						bBracket = true;
						break;
					case AUDIOTYPE_PLAINCBR:
						sstrStreamInfo << _T("CBR (0x") << std::hex << std::setw(4) << 
							std::setfill(_T('0')) << asi->audiosource->GetFormatTag();
						bBracket = true;
						break;
					case AUDIOTYPE_AC3:
						sstrStreamInfo << _T("AC3 (") << bitrate.c_str() << _T(" ") 
							<< (asi->audiosource->IsCBR()?_T("CBR, "):_T("VBR, "))
							<< channels << _T(", ") << sample_rate;
						bBracket = true;
						break;
					case AUDIOTYPE_PCM:
						sstrStreamInfo << _T("PCM (") << bitrate << _T(", ") 
							<< channels << _T(", ") << sample_rate;
						bBracket = true;
						break;
					case AUDIOTYPE_DTS:
						sstrStreamInfo << _T("DTS (") << bitrate << _T(" ")
							<< (asi->audiosource->IsCBR()?_T("CBR, "):_T("VBR, "))
							<< channels << _T(", ") << sample_rate;
						bBracket = true;
						break;
					case AUDIOTYPE_VORBIS:
						sstrStreamInfo << _T("Vorbis (") << bitrate << _T(", ") 
							<< channels << _T(", ") << sample_rate;
						bBracket = true;
						break;
					case AUDIOTYPE_AAC:
						if ((int)asi->audiosource->FormatSpecific(MMSGFS_AAC_ISSBR)) 
							AAC_prof_name = _T("HE");
						else switch (asi->audiosource->FormatSpecific(MMSGFS_AAC_PROFILE)) {
							case AACSOURCE::AdtsProfile::LC: AAC_prof_name = _T("LC"); break;
							case AACSOURCE::AdtsProfile::LTP: AAC_prof_name = _T("LTP"); break;
							case AACSOURCE::AdtsProfile::Main: AAC_prof_name = _T("MAIN"); break;
							case AACSOURCE::AdtsProfile::SSR: AAC_prof_name = _T("SSR"); break;
							default: AAC_prof_name = _T("unknown");
						}
						sstrStreamInfo << AAC_prof_name << _T("-AAC (MPEG ") <<
							(int)asi->audiosource->FormatSpecific(MMSGFS_AAC_MPEGVERSION) <<
							_T(", ") << bitrate << _T(", ") << channels << _T(", ") <<
							sample_rate;
						bBracket = true;
						break;
					case AUDIOTYPE_DIVX:
						sstrStreamInfo << _T("divX, ") << bitrate << _T(", ") << channels <<
							_T(", ") << sample_rate;

						break;
				/*	default:
						if (asi->audiosource->GetIDString()) {
							sprintf(c,"%s (%s",asi->audiosource->GetIDString(),cdatarate);
						}
						break;				
						*/
				}
			} else {
//			if (asi && !asi->bNameFromFormatTag) {
				sstrDisplayText << asi->audiosource->GetCodecID(); //c;

//			}
			}

			sstrDisplayText << sstrStreamInfo.str();

			if (asi) {
				int offset = asi->audiosource->GetOffset();
				if (offset) {
					sstrDisplayText << _T(", bad: ") << offset;
				}
				if (asi->iDelay) {
					sstrDisplayText << _T(", delay: ") << asi->iDelay << _T(" ms");
				}
			}


			if (!bBracket) {
				sstrDisplayText << _T(" (");
				bBracket = true;
			} else {
				sstrDisplayText << _T(", ");
			}

			if (asi && asi->iSize) {
				char cSize[30];
				FormatSize(cSize,asi->iSize);
				sstrDisplayText << CUTF8(cSize).TStr();

				bAddComma = true;
			}

			if (!(GetItemState(hItem,TVIS_EXPANDED) & TVIS_EXPANDED))
			{
				std::string languageCode;
				asi->audiosource->GetLanguageCode(languageCode);
				if (!languageCode.empty())
				{
					if (bAddComma) 
						sstrDisplayText << _T(", ");
					sstrDisplayText << CUTF8(languageCode.c_str()).TStr();
				}
			}

			if (!(GetItemState(hItem,TVIS_EXPANDED) & TVIS_EXPANDED))
			{ 
				char* title;
				asi->audiosource->GetPreferredTitle(&title);
				
				if (title && title[0])
				{
					if (bAddComma) 
						sstrDisplayText << _T(", ");
					sstrDisplayText << CUTF8(title).TStr();
				}			
			}
			
			if (bBracket)
				sstrDisplayText << _T(")");
		} else if (tii && ((tii->iID & TIIID_SSI) == TIIID_SSI)) {
			*d = 0;

			SUBTITLESOURCE* subs = tii->pSSI->lpsubs;

			sstrDisplayText << _T("subtitle: ");

			if (tii->pSSI) {
				switch (tii->pSSI->lpsubs->GetFormat()) {
					case SUBFORMAT_SRT:
						sstrDisplayText << _T("SRT");
						break;
					case SUBFORMAT_SSA:
						sstrDisplayText << _T("SSA");
						break;
					case SUBFORMAT_VOBSUB:
						sstrDisplayText << _T("Vobsub");
						break;
				}

				bAddComma = false;
				bBracket = true;
				
				if (!(GetItemState(hItem,TVIS_EXPANDED) & TVIS_EXPANDED))
				{
					//char lngcode[16]; memset(lngcode, 0, sizeof(lngcode));
					
					std::string languageCode;
					subs->GetLanguageCode(languageCode);

					//if (lngcode[0])
					if (!languageCode.empty())
					{
						//sprintf(c," (%s", lngcode);
						sstrDisplayText << _T(" (") << CUTF8(languageCode.c_str()).TStr();

						bAddComma = true;
						bBracket = false;
					}

					char* title;
					subs->GetPreferredTitle(&title);
					if (title && title[0])
					{
						if (bAddComma)
							sstrDisplayText << _T(", ");

						if (bBracket) 
							sstrDisplayText << _T("( ");

						bAddComma = true;
						bBracket = false;
						sstrDisplayText << CUTF8(title).TStr();

					}
					/*&& 
					  FindID(hItem,TIIID_LNGCODE,&tii) && tii->pText && strlen(tii->pText)) {
					sprintf(c," (%s",tii->pText);
					strcat(d,c);
					bAddComma = true;
					bBracket = false;
				}
				if (!(GetItemState(hItem,TVIS_EXPANDED) & TVIS_EXPANDED) && 
					  FindID(hItem,TIIID_STRNAME,&tii) && tii->pText && strlen(tii->pText)) {
					if (bAddComma) strcat(d,", ");
					if (bBracket) strcat(d," (");
					bAddComma = true;
					bBracket = false;
					sprintf(c,"%s",tii->pText);
					strcat(d,c); */
				}
				int _i; 
				if ((_i = subs->GetCompressionAlgo()) != COMPRESSION_NONE) {
					if (bAddComma) 
						sstrDisplayText << _T(", ");

					if (bBracket) 
						sstrDisplayText << _T(" (");

					bAddComma = false;
					bBracket = false;
					//sprintf(c,"compression: %s",((_i == COMPRESSION_ZLIB)?"zlib":"unknown"));
					sstrDisplayText << _T("compression: ") << 
						((_i == COMPRESSION_ZLIB)?_T("zlib"):_T("unknown"));

				}

				if (!bBracket) 
					sstrDisplayText << _T(")");
			}

		} else
		if (tii && ((tii->iID & TIIID_VSI) == TIIID_VSI)) {
			*d = 0;
			VIDEOSOURCE* v = tii->pVSI->videosource;
			char* codecid = v->GetCodecID();
			bool bracket = true;
			
			/*if (v->IsDefault()) 
				strcat(d,"(default) ");*/
			//strcat(d, "video: ");
			sstrDisplayText << _T("video: ");
			
			if (codecid) {
				sstrDisplayText << CUTF8(codecid).TStr();
				sstrDisplayText << _T(" ");
			};
			
			if (!codecid || !strcmp(codecid, "V_MS/VFW/FOURCC")) {
				DWORD dwfourcc = v->GetFourCC();
				char fourcc[32]; memset(fourcc, 0, sizeof(fourcc));
				char fmts[64];
				if (!codecid)
					strcpy(fmts, "FourCC: %c%c%c%c");
				else {
					strcpy(fmts, "(FourCC: %c%c%c%c, ");
					bracket = false;
				}

				sprintf(fourcc, fmts, (dwfourcc >> 0) & 0xFF,
					(dwfourcc >> 8) & 0xFF, (dwfourcc >> 16) & 0xFF,
					(dwfourcc >> 24) & 0xFF);
				sstrDisplayText << CUTF8(fourcc).TStr();
				sstrDisplayText << _T(" ");
			}

			if (bracket) 
				sstrDisplayText << _T("(");
			bAddComma = false;
			c[0]=0;
			
			int idatarate = (int) (((double)v->GetSize() / (v->GetDurationUnscaled() / 8000000)));
			if (idatarate) {
				if (idatarate < 1000.)
					sstrBitrate << idatarate << _T("kbps");
				else {
					sstrBitrate.precision(5);
					sstrBitrate << std::dec << (float)idatarate/1000. << _T("MBps");
				}
				bitrate = sstrBitrate.str();
			} else {
				bitrate = _T("unknown bitrate");
			}
			if (bAddComma)
				sstrDisplayText << _T(", ");

			sstrDisplayText << bitrate;
			bAddComma = true;

			if (bAddComma) 
				sstrDisplayText << _T(", ");

			__int64 s = v->GetSize();
			FormatSize(c, s);
			sstrDisplayText << CUTF8(c).TStr();

			bAddComma = true;

			s = (v->GetDuration() * v->GetTimecodeScale() + 499999) / 1000000;
			if (s > 0) {
				if (bAddComma) 
					sstrDisplayText << _T(", ");
				Millisec2Str(s, c);
				sstrDisplayText << CUTF8(c).TStr();
				bAddComma = true;
			}

			if (!(GetItemState(hItem,TVIS_EXPANDED) & TVIS_EXPANDED)) {
				std::string languageCode;
				v->GetLanguageCode(languageCode);
				if (!languageCode.empty()) {
					if (bAddComma) 
						sstrDisplayText << _T(", ");
					sstrDisplayText << CUTF8(languageCode.c_str()).TStr();
					bAddComma = true;
				}
				char* title;
				v->GetPreferredTitle(&title);
				if (title && title[0]) {
					if (bAddComma)
						sstrDisplayText << _T(", ");

					sstrDisplayText << CUTF8(title).TStr();

					bAddComma = true;
				}
				sstrDisplayText << _T(")");

			}
		} else

		if (tii && tii->iID == TIIID_LNGCODE) {
			sprintf(c,"language code: %s", tii->pText);
			sstrDisplayText << c;
		} else
		if (tii && tii->iID == TIIID_TITLELNG) {
			sprintf(c,"language code: %s", tii->pText);
			sstrDisplayText << c;
		} else

		if (tii && tii->iID == TIIID_STRNAME) {
			sprintf(c,"stream name: %s", tii->pText);
			sstrDisplayText << c;
		} else

		if (tii && tii->iID == TIIID_TITLE) {
			sstrDisplayText << _T("Title: ");

			if (!(GetItemState(hItem,TVIS_EXPANDED) & TVIS_EXPANDED) && 
				  FindID(hItem,TIIID_LNGCODE,&tii) && tii->pText && strlen(tii->pText)) {
				if (bAddComma)
					sstrDisplayText << _T(", ");

				bAddComma = true;
				sprintf(c,"%s",tii->pText);
				sstrDisplayText << c;
			}

			if (!(GetItemState(hItem,TVIS_EXPANDED) & TVIS_EXPANDED) && 
				  FindID(hItem,TIIID_STRNAME,&tii) && tii->pText && strlen(tii->pText)) {
				if (bAddComma)
					sstrDisplayText << _T(", ");

				bAddComma = false;
				sprintf(c,"%s",tii->pText);
				sstrDisplayText << c;

			}
		} else {
			sprintf(c, "Internal error: tii->iID = %d", tii?tii->iID:-17);
						sstrDisplayText << c;
		}

		_tcscpy(d, sstrDisplayText.str().c_str());
	}

	*pResult = 0;
}

void CAudioSourceTree::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: Fügen Sie hier Ihren Meldungsbehandlungscode ein, und/oder benutzen Sie den Standard.

	TREE_ITEM_INFO* tii = GetItemInfo(GetSelectedItem());
	if (!tii)
			return CUnicodeTreeCtrl::OnKeyDown(nChar, nRepCnt, nFlags);

	/* if item is stream name or language, ignore <space> button */
	if (tii->iID == TIIID_STRNAME || tii->iID == TIIID_LNGCODE) {
		if (nChar != ' ')
			return CUnicodeTreeCtrl::OnKeyDown(nChar, nRepCnt, nFlags);
		else 
			return;
	}

	if (tii->iID & TIIID_MSI) {
		if (nChar == VK_NEXT) {
			tii->iCurrPos+=3;
			Sort();
			return;
		} else if (nChar == VK_PRIOR) {
			tii->iCurrPos-=3;
			Sort();
			return;
		} else if (nChar == VK_INSERT) {

		}
			return CUnicodeTreeCtrl::OnKeyDown(nChar, nRepCnt, nFlags);
	}

	return CUnicodeTreeCtrl::OnKeyDown(nChar, nRepCnt, nFlags);
}
