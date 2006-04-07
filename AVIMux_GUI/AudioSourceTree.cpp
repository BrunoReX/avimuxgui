// AudioSourceTree.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "AVIMux_GUI.h"
#include "AudioSourceTree.h"
#include "formattext.h"
#include "audiosource.h"
#include "AVIMux_GUIDlg.h"
#include "Muxing.h"
#include "..\basestreams.h"
#include "..\utf-8.h"
#include "UnicodeTreeCtrl.h"
#include "FileDialogs.h"
#include ".\audiosourcetree.h"

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

	tii->iID = TIIID_ASI;
	tii->pASI = asi;

	return tii;
}

TREE_ITEM_INFO*	BuildTIIfromSSI(SUBTITLE_STREAM_INFO* ssi)
{
	TREE_ITEM_INFO* tii = new TREE_ITEM_INFO;

	tii->iID = TIIID_SSI;
	tii->pSSI = ssi;

	return tii;
}

TREE_ITEM_INFO* BuildTIIfromVSI(VIDEO_STREAM_INFO* vsi)
{
	TREE_ITEM_INFO* tii = new TREE_ITEM_INFO;

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

	return 0;
}

CDynIntArray* CAudioSourceTree::GetItems(HTREEITEM hItem,int iID, int iCheck, CDynIntArray** indices)
{
	CDynIntArray* a = new CDynIntArray;
	if (indices) *indices = new CDynIntArray;
	TREE_ITEM_INFO*	tii;
	int i=0;

	while (hItem) {
		tii = (TREE_ITEM_INFO*)GetItemData(hItem);
		if (tii->iID == iID && ++i && (iCheck == -1 || Tree_GetCheckState(this,hItem))) {
			a->Insert((int)hItem);
			if (indices) 
				(*indices)->Insert(i-1);
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
		bool bDefault = false;
		
		if (tii->iID == TIIID_ASI && tii->pASI->audiosource->IsDefault()) 
			bDefault = true;
		
		if (tii->iID == TIIID_SSI && tii->pSSI->lpsubs->IsDefault()) 
			bDefault = true;

		if (tii->iID == TIIID_VSI && tii->pVSI->videosource->IsDefault())
			bDefault = true;
		
		c->AppendMenu(MF_STRING | bDefault*MF_CHECKED, IDM_SETDEFAULTTRACK, 
			LoadString(STR_MAIN_AS_DEFAULTTRACK));
		
		init_sep;

		if (tii->iID == TIIID_ASI) {
			AUDIO_STREAM_INFO* asi = tii->pASI;
			AUDIOSOURCE* a = asi->audiosource;
			if (asi->dwType == AUDIOTYPE_AAC) {
				put_sep;
				c->AppendMenu(MF_STRING, IDM_EXTRACT_TOADTS, LoadString(STR_MAIN_A_EXTRACTADTS));
			}
			if (asi->dwType == AUDIOTYPE_VORBIS) {
				put_sep;
				c->AppendMenu(MF_STRING, IDM_EXTRACT_OGGVORBIS, LoadString(STR_MAIN_A_EXTRACTOGGV));
			}
			if (a->GetFeature(FEATURE_EXTRACTBIN)) {
				put_sep;
				c->AppendMenu(MF_STRING, IDM_EXTRACT_BINARY, LoadString(STR_MAIN_A_EXTRBIN));
			}
			if (asi->dwType == AUDIOTYPE_AAC) {
				put_sep;
				c->AppendMenu(MF_STRING | (a->FormatSpecific(MMSGFS_AAC_ISSBR))?MF_CHECKED:MF_UNCHECKED,
					IDM_CHANGESBR, "SBR");
			}

		} else 
		if (tii->iID == TIIID_SSI) {
			SUBTITLE_STREAM_INFO* ssi = tii->pSSI;
			SUBTITLESOURCE* s = ssi->lpsubs;
			if (s->GetFeature(FEATURE_SUB_EXTRACT2TEXT)) {
				put_sep;
				c->AppendMenu(MF_STRING, IDM_EXTRACT_SUB2TEXT, LoadString(STR_MAIN_A_EXTRSUB2TEXT));
			}
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
	FILESTREAM* file;
	CAVIMux_GUIDlg* dlg;
	AUDIOSOURCE* a;

} EXTRACT_THREAD_DATA;

int ExtractThread(EXTRACT_THREAD_DATA*	lpETD)
{
	lpETD->dlg->SetDialogState_Muxing();
	lpETD->dlg->ButtonState_START();

	lpETD->dlg->AddProtocolLine("started extracting binary", 4);

	AUDIOSOURCE* a = lpETD->a;
	a->Enable(1);
	FILESTREAM* f = lpETD->file;
	char cTime[20];
	char* lpBuffer = new char[2<<20];
	int iLastTime = GetTickCount();
	a->ReInit();

	while (!a->IsEndOfStream() && !DoStop()) {
		__int64 iTimecode;
		__int64 iNS;
		f->Write(lpBuffer,a->Read(lpBuffer,1000000,NULL,&iNS,&iTimecode));
		if (GetTickCount()-iLastTime>100 || a->IsEndOfStream()) {
			Millisec2Str((iTimecode * a->GetTimecodeScale() + iNS)/ 1000000,cTime);
			lpETD->dlg->m_Prg_Frames.SetWindowText(cTime);
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
	FILESTREAM* f = lpETD->file;
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
			lpETD->dlg->m_Prg_Frames.SetWindowText(cTime);
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

	FILESTREAM* f = lpETD->file;
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
				lpETD->dlg->m_Prg_Frames.SetWindowText(cTime);
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
	char cFilter[256];
	char cDefExt[5];
	int iFmt;
	char* cFmt;
	int open = 0;
	HTREEITEM hItem;
	TREE_ITEM_INFO* tii;
	AUDIO_STREAM_INFO* asi;
	AUDIOSOURCE* a;
	SUBTITLE_STREAM_INFO* ssi;
	SUBTITLESOURCE* s;
	OPENFILENAME o; 
	memset(&o, 0, sizeof(o));

	switch (LOWORD(wParam))
	{
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
			ZeroMemory(cFilter,sizeof(cFilter));
			ZeroMemory(cDefExt,sizeof(cDefExt));
			hItem = GetSelectedItem();
			tii = (TREE_ITEM_INFO*)GetItemData(hItem);
			asi = tii->pASI;
			a = asi->audiosource;

			iFmt = a->GetFormatTag();
			cFmt = a->GetCodecID();
			
			if (iFmt == 0x0055 || cFmt && !strcmp(cFmt,"A_MPEG/L3")) {
				strcpy(cFilter,"*.mp3|*.mp3||");
				strcpy(cDefExt,"mp3");
			} else
			if (cFmt && !strcmp(cFmt,"A_MPEG/L2")) {
				strcpy(cFilter,"*.mp2|*.mp2||");
				strcpy(cDefExt,"mp2");
			} else
			if (iFmt == 0x2000 || cFmt && !strcmp(cFmt,"A_AC3")) {
				strcpy(cFilter,"*.ac3|*.ac3||");
				strcpy(cDefExt,"ac3");
			} else 
			if (iFmt == 0x2001 || cFmt && !strcmp(cFmt,"A_DTS")) {
				strcpy(cFilter,"*.dts|*.dts||");
				strcpy(cDefExt,"dts");
			} else {
				strcpy(cFilter,"*.raw|*.raw||");
				strcpy(cDefExt,"raw");
			}


			PrepareSimpleDialog(&o, m_hWnd, cFilter);
			o.Flags |= OFN_OVERWRITEPROMPT;
			o.lpstrDefExt = cDefExt;
			open = GetOpenSaveFileNameUTF8(&o, 0);
			
			if (open) {
				EXTRACT_THREAD_DATA* lpETD = new EXTRACT_THREAD_DATA;
				lpETD->file = new FILESTREAM;
				if (lpETD->file->Open(o.lpstrFile,STREAM_WRITE)!=STREAM_ERR) {
					lpETD->dlg = cMainDlg;
					lpETD->a = a;
					cMainDlg->m_Prg_Dest_File.SetWindowText(o.lpstrFile);
					thread=AfxBeginThread((AFX_THREADPROC)ExtractThread,lpETD);
				} else {
					MessageBox(LoadString(IDS_COULDNOTOPENOUTPUTFILE),LoadString(STR_GEN_ERROR),
						MB_OK | MB_ICONERROR);
				}
			}
			
			break;
		case IDM_EXTRACT_TOADTS:
			strcpy(cFilter,"*.aac|*.aac||");
			strcpy(cDefExt,"aac");
			hItem = GetSelectedItem();
			tii = (TREE_ITEM_INFO*)GetItemData(hItem);
			asi = tii->pASI;
			a = asi->audiosource;
			
			PrepareSimpleDialog(&o, m_hWnd, cFilter);
			o.Flags |= OFN_OVERWRITEPROMPT;
			o.lpstrDefExt = cDefExt;
			open = GetOpenSaveFileNameUTF8(&o, 0);

			//cfd=new CFileDialog(false, cDefExt, NULL, OFN_OVERWRITEPROMPT, cFilter);
			
			if (open) {
				EXTRACT_THREAD_DATA* lpETD = new EXTRACT_THREAD_DATA;
				lpETD->file = new FILESTREAM;
				if (lpETD->file->Open(o.lpstrFile,STREAM_WRITE)!=STREAM_ERR) {
					lpETD->dlg = cMainDlg;
					lpETD->a = a;
					cMainDlg->m_Prg_Dest_File.SetWindowText(o.lpstrFile);
					thread=AfxBeginThread((AFX_THREADPROC)ExtractThread_ADTS,lpETD);
				} else {
					MessageBox(LoadString(IDS_COULDNOTOPENOUTPUTFILE),LoadString(STR_GEN_ERROR),
						MB_OK | MB_ICONERROR);
				}
			}
			break;
		case IDM_EXTRACT_OGGVORBIS:
			strcpy(cFilter,"*.ogg|*.ogg||");
			strcpy(cDefExt,"ogg");
			hItem = GetSelectedItem();
			tii = (TREE_ITEM_INFO*)GetItemData(hItem);
			asi = tii->pASI;
			a = asi->audiosource;
		//	cfd=new CFileDialog(false, cDefExt, NULL, OFN_OVERWRITEPROMPT, cFilter);
		
			PrepareSimpleDialog(&o, m_hWnd, cFilter);
			o.Flags |= OFN_OVERWRITEPROMPT;
			o.lpstrDefExt = cDefExt;
			open = GetOpenSaveFileNameUTF8(&o, 0);
			
			if (open) {
				EXTRACT_THREAD_DATA* lpETD = new EXTRACT_THREAD_DATA;
				lpETD->file = new FILESTREAM;
				if (lpETD->file->Open(o.lpstrFile,STREAM_WRITE)!=STREAM_ERR) {
					lpETD->dlg = cMainDlg;
					lpETD->a = a;
					cMainDlg->m_Prg_Dest_File.SetWindowText(o.lpstrFile);
					thread=AfxBeginThread((AFX_THREADPROC)ExtractThread_OGGVorbis,lpETD);
				} else {
					MessageBox(LoadString(IDS_COULDNOTOPENOUTPUTFILE),LoadString(STR_GEN_ERROR),
						MB_OK | MB_ICONERROR);
				}
			}
			break;
		case IDM_EXTRACT_SUB2TEXT:
			ZeroMemory(cFilter,sizeof(cFilter));
			ZeroMemory(cDefExt,sizeof(cDefExt));
			hItem = GetSelectedItem();
			tii = (TREE_ITEM_INFO*)GetItemData(hItem);
			ssi = tii->pSSI;
			s = ssi->lpsubs;
			s->Enable(1);

			switch (s->GetFormat()) {
				case SUBFORMAT_SRT:
					strcpy(cFilter,"*.srt|*.srt||");
					strcpy(cDefExt,"srt");
					break;
				case SUBFORMAT_SSA:
					strcpy(cFilter,"*.ssa|*.ssa||");
					strcpy(cDefExt,"ssa");
					break;
			}

			s->Seek(0);
			s->SetRange(0,0x7FFFFFFFFFFFFFFF);
//			cfd=new CFileDialog(false, cDefExt, NULL, OFN_OVERWRITEPROMPT, cFilter);

			PrepareSimpleDialog(&o, m_hWnd, cFilter);
			o.Flags |= OFN_OVERWRITEPROMPT;
			o.lpstrDefExt = cDefExt;
			open = GetOpenSaveFileNameUTF8(&o, 0);

			if (open) {
				FILESTREAM* f = new FILESTREAM;
				if (f->Open(o.lpstrFile,STREAM_WRITE)!=STREAM_ERR) {
					char* lpBuffer = new char[2<<23];
					f->Write(lpBuffer,s->Render2Text(lpBuffer));
					f->Close();
					delete f;
					delete lpBuffer;
				} else {
					MessageBox(LoadString(IDS_COULDNOTOPENOUTPUTFILE),LoadString(STR_GEN_ERROR),
						MB_OK | MB_ICONERROR);
				}

			}
			s->Enable(0);
			break;
		case IDM_SETDEFAULTTRACK:
			hItem = GetSelectedItem();
			tii = (TREE_ITEM_INFO*)GetItemData(hItem);
			if (tii->iID == TIIID_ASI) 
				tii->pASI->audiosource->SetDefault(!tii->pASI->audiosource->IsDefault());
			
			if (tii->iID == TIIID_SSI)
				tii->pSSI->lpsubs->SetDefault(!tii->pSSI->lpsubs->IsDefault());
			
			if (tii->iID == TIIID_VSI)
				tii->pVSI->videosource->SetDefault(!tii->pVSI->videosource->IsDefault());

			InvalidateRect(NULL);
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

void CAudioSourceTree::OnTvnGetdispinfo(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTVDISPINFO pTVDispInfo = reinterpret_cast<LPNMTVDISPINFO>(pNMHDR);
	// TODO: Fügen Sie hier Ihren Kontrollbehandlungscode für die Benachrichtigung ein.
	*pResult = 0;

	AUDIO_STREAM_INFO* asi = NULL;
	TREE_ITEM_INFO*	   tii = NULL;
	char*	d = pTVDispInfo->item.pszText;
	bool	bBracket = false;

	char	bitrate[128];
	char	channels[128];
	char	sample_rate[128];

	char	c[1024];
	ZeroMemory(c,sizeof(c));
	memset(bitrate, 0, sizeof(bitrate));
	memset(channels, 0, sizeof(channels));
	memset(sample_rate, 0, sizeof(sample_rate));
	HTREEITEM hItem = pTVDispInfo->item.hItem;
	char* AAC_prof_name;
	bool bAddComma=false;
	int i,j;
	
	if ((pTVDispInfo->item.mask & TVIF_TEXT) == 0) {
		*pResult = 0;
		return;
	}
	
	tii = GetItemInfo(hItem);

	if (!0) { //bEditInProgess) {
		if (tii && tii->iID == TIIID_ASI) {
			*d = 0;
			
			if (tii->pASI->audiosource->IsDefault()) {
				strcat(d,"(default) ");
			}
			strcat(d,"audio: ");
			if (tii) asi = tii->pASI;
			if (asi && asi->bNameFromFormatTag) {
				int idatarate = (asi->audiosource->GetAvgBytesPerSec() + 62) /125;
				if (idatarate) 
					sprintf(bitrate,"%d kbps",idatarate);
				else 
					strcpy(bitrate, "unknown bitrate");

				sprintf(channels, "%d Ch", asi->audiosource->GetChannelCount());
				
				int isr = (int)(0.49+asi->audiosource->GetOutputFrequency());
				if ((isr % 1000) == 0) {
					sprintf(sample_rate, "%2dkHz", 
						(int)((0.49+asi->audiosource->GetOutputFrequency())/1000));
				} else {
					sprintf(sample_rate, "%5dHz", 
						(int)((0.49+asi->audiosource->GetOutputFrequency())));
				}

				switch (asi->dwType) {
					case AUDIOTYPE_MP3CBR:
						i = (int)asi->audiosource->FormatSpecific(MMSGFS_MPEG_LAYERVERSION);
						j = (int)asi->audiosource->FormatSpecific(MMSGFS_MPEG_VERSION);
						sprintf(c,"MPEG %d Layer %d (CBR %s, %s, %s",j,i,bitrate,channels,
							sample_rate);
						bBracket = true;
						break;
					case AUDIOTYPE_MP3VBR:
						i = (int)asi->audiosource->FormatSpecific(MMSGFS_MPEG_LAYERVERSION);
						j = (int)asi->audiosource->FormatSpecific(MMSGFS_MPEG_VERSION);
						sprintf(c,"MPEG %d Layer %d (VBR, %s, %s",j, i,channels,
							sample_rate);
						bBracket = true;
						break;
					case AUDIOTYPE_PLAINCBR:
						sprintf(c,"CBR (0x%04X", asi->audiosource->GetFormatTag());
						bBracket = true;
						break;
					case AUDIOTYPE_AC3:
						sprintf(c,"AC3 (%s %s, %s, %s",bitrate, asi->audiosource->IsCBR()?"CBR":"VBR",
							channels, sample_rate);
						bBracket = true;
						break;
					case AUDIOTYPE_PCM:
						sprintf(c,"PCM (%s, %s, %s", bitrate, channels, sample_rate);
						bBracket = true;
						break;
					case AUDIOTYPE_DTS:
						sprintf(c,"DTS (%s %s, %s, %s",bitrate, asi->audiosource->IsCBR()?"CBR":"VBR",
							channels, sample_rate);
						bBracket = true;
						break;
					case AUDIOTYPE_VORBIS:
						sprintf(c,"Vorbis (%s, %s, %s",bitrate, channels,
							sample_rate);
						bBracket = true;
						break;
					case AUDIOTYPE_AAC:
						if ((int)asi->audiosource->FormatSpecific(MMSGFS_AAC_ISSBR)) 
							AAC_prof_name = "HE";
						else switch (asi->audiosource->FormatSpecific(MMSGFS_AAC_PROFILE)) {
							case AAC_ADTS_PROFILE_LC: AAC_prof_name = "LC"; break;
							case AAC_ADTS_PROFILE_LTP: AAC_prof_name = "LTP"; break;
							case AAC_ADTS_PROFILE_MAIN: AAC_prof_name = "MAIN"; break;
							case AAC_ADTS_PROFILE_SSR: AAC_prof_name = "SSR"; break;
							default: AAC_prof_name = "unknown";
						}
						sprintf(c,"%s-AAC (MPEG %d, %s, %s, %s",
							AAC_prof_name, 
							(int)asi->audiosource->FormatSpecific(MMSGFS_AAC_MPEGVERSION),
							bitrate, channels, sample_rate);
						bBracket = true;
						break;
					case AUDIOTYPE_DIVX:
						sprintf(c,"divX%s, %s, %s, %s",bitrate, channels, sample_rate);
						break;
				/*	default:
						if (asi->audiosource->GetIDString()) {
							sprintf(c,"%s (%s",asi->audiosource->GetIDString(),cdatarate);
						}
						break;				
						*/
				}
			}

			strcat(d,c);

			if (asi) {
				int offset = asi->audiosource->GetOffset();
				if (offset) {
					sprintf(c,", bad: %d ",offset);
					strcat(d,c);
				}
				if (asi->iDelay) {
					sprintf(c,", delay: %d ms",asi->iDelay);
					strcat(d,c);
				}
			}

			if (asi && !asi->bNameFromFormatTag) {
				sprintf(c,asi->audiosource->GetCodecID());
				strcat(d,c);
			}

			if (!bBracket) {
				strcat(d," (");
				bBracket = true;
			} else {
				strcat(d,", ");
			}

			if (asi && asi->iSize) {
				char cSize[30];
				FormatSize(cSize,asi->iSize);
				strcat(d,cSize);
				bAddComma = true;
			}

			if (!(GetItemState(hItem,TVIS_EXPANDED) & TVIS_EXPANDED) && 
				  FindID(hItem,TIIID_LNGCODE,&tii) && tii->pText && strlen(tii->pText)) {
				if (bAddComma) strcat(d,", ");
				bAddComma = true;
				sprintf(c,"%s",tii->pText);
				strcat(d,c);
			}

			if (!(GetItemState(hItem,TVIS_EXPANDED) & TVIS_EXPANDED) && 
				  FindID(hItem,TIIID_STRNAME,&tii) && tii->pText && strlen(tii->pText)) {
				if (bAddComma) strcat(d,", ");
				bAddComma = false;
				sprintf(c,"%s",tii->pText);
				strcat(d,c);
			}

			if (bBracket) strcat(d,")");

		} else
		if (tii && tii->iID == TIIID_SSI) {
			*d = 0;
			if (tii->pSSI->lpsubs->IsDefault()) {
				strcat(d,"(default) ");
			}

			SUBTITLESOURCE* subs = tii->pSSI->lpsubs;

			strcat(d,"subtitle: ");
			if (tii->pSSI) {
				switch (tii->pSSI->lpsubs->GetFormat()) {
					case SUBFORMAT_SRT:
						sprintf(c,"SRT");
						break;
					case SUBFORMAT_SSA:
						sprintf(c,"SSA");
						break;
					case SUBFORMAT_VOBSUB:
						sprintf(c,"Vobsub");
						break;
				}
				strcat(d,c);
				bAddComma = false;
				bBracket = true;
				
				if (!(GetItemState(hItem,TVIS_EXPANDED) & TVIS_EXPANDED) && 
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
					strcat(d,c);
				}
				int _i; 
				if ((_i = subs->GetCompressionAlgo()) != COMPRESSION_NONE) {
					if (bAddComma) strcat(d,", ");
					if (bBracket) strcat(d," (");
					bAddComma = false;
					bBracket = false;
					sprintf(c,"compression: %s",((_i == COMPRESSION_ZLIB)?"zlib":"unknown"));
					strcat(d,c);
				}

				if (!bBracket) strcat(d,")");
			}

		} else
		if (tii && tii->iID == TIIID_VSI) {
			*d = 0;
			VIDEOSOURCE* v = tii->pVSI->videosource;
			char* codecid = v->GetCodecID();
			bool bracket = true;
			
			if (v->IsDefault()) 
				strcat(d,"(default) ");
			strcat(d, "video: ");
			
			if (codecid) {
				strcat(d, codecid);
				strcat(d, " ");
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
				strcat(d, fourcc);
				strcat(d, " ");
			}

			if (bracket) strcat(d, "(");
			bAddComma = false;
			c[0]=0;
			
			int idatarate = (int) (((double)v->GetSize() / (v->GetDurationUnscaled() / 8000000)));
			if (idatarate) {
				if (idatarate < 1000.)
					sprintf(bitrate,"%d kbps",idatarate);
				else
					sprintf(bitrate,"%4.2fMbps",(float)idatarate/1000.);
			} else 
				strcpy(bitrate, "unknown bitrate");
			if (bAddComma) strcat(d, ", ");
			strcat(d, bitrate);
			bAddComma = true;

			if (bAddComma) strcat(d, ", ");
			__int64 s = v->GetSize();
			FormatSize(c, s);
			strcat(d, c);
			bAddComma = true;

			s = (v->GetDuration() * v->GetTimecodeScale() + 499999) / 1000000;
			if (s > 0) {
				if (bAddComma) strcat(d, ", ");
				Millisec2Str(s, c);
				strcat(d, c);
				bAddComma = true;
			}

			if (!(GetItemState(hItem,TVIS_EXPANDED) & TVIS_EXPANDED)) {
				v->GetLanguageCode(c);
				if (c[0]) {
					if (bAddComma) strcat(d, ", ");
					strcat(d, c);
					bAddComma = true;
				}
				v->GetName(c);
				if (c[0]) {
					if (bAddComma) strcat(d, ", ");
					strcat(d, c);
					bAddComma = true;
				}
				strcat(d, ")");
			}
		} else

		if (tii && tii->iID == TIIID_LNGCODE) {
			sprintf(d,"language code: %s", tii->pText);
		} else

		if (tii && tii->iID == TIIID_STRNAME) {
			sprintf(d,"stream name: %s", tii->pText);
		}
	}

	*pResult = 0;



}
