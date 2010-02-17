// SourceFileListBox.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "AVIMux_GUI.h"
#include "AVIMux_GUIDlg.h"
#include "AVIFile.h"
#include "SourceFileListBox.h"
#include "..\FileStream.h"
#include <algorithm>

#include "mode2form2reader.h"
#include "VideoInformationDlg.h"
#include "Languages.h"
#include "..\Buffers.h"
#include "..\UnicodeCalls.h"
#include "UTF8Windows.h"
#include "EBMLTreeDlg.h"
#include "RIFFChunkTreeDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include "FILE_INFO.h"
#include ".\sourcefilelistbox.h"


/////////////////////////////////////////////////////////////////////////////
// CSourceFileListBox

CSourceFileListBox::CSourceFileListBox()
{
	EnableAutomation();
}

CSourceFileListBox::~CSourceFileListBox()
{
}

void CSourceFileListBox::OnFinalRelease()
{

	CEnhancedListBox::OnFinalRelease();
}


BEGIN_MESSAGE_MAP(CSourceFileListBox, CEnhancedListBox)
	//{{AFX_MSG_MAP(CSourceFileListBox)
	ON_WM_RBUTTONUP()
	ON_WM_DROPFILES()
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
	ON_WM_KILLFOCUS()
END_MESSAGE_MAP()

BEGIN_DISPATCH_MAP(CSourceFileListBox, CEnhancedListBox)
	//{{AFX_DISPATCH_MAP(CSourceFileListBox)
		// HINWEIS - Der Klassen-Assistent fügt hier Zuordnungsmakros ein und entfernt diese.
	//}}AFX_DISPATCH_MAP
END_DISPATCH_MAP()

// Hinweis: Wir stellen Unterstützung für IID_ISourceFileListBox zur Verfügung, um typsicheres Binden
//  von VBA zu ermöglichen. Diese IID muss mit der GUID übereinstimmen, die in der
//  Disp-Schnittstelle in der .ODL-Datei angegeben ist.

// {15F96C04-73A2-4BD3-8A81-0D83268DEE82}
static const IID IID_ISourceFileListBox =
{ 0x15f96c04, 0x73a2, 0x4bd3, { 0x8a, 0x81, 0xd, 0x83, 0x26, 0x8d, 0xee, 0x82 } };

BEGIN_INTERFACE_MAP(CSourceFileListBox, CListBox)
	INTERFACE_PART(CSourceFileListBox, IID_ISourceFileListBox, Dispatch)
END_INTERFACE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CSourceFileListBox 

FILE_INFO* CSourceFileListBox::GetFileInfo(int i)
{
	if (i==-1) {
		i = GetCurSel();
		if (i == LB_ERR) {
			return NULL;
		}
	}
	
	DWORD d = GetItemData(i);
	if (d == LB_ERR) {
		return NULL;
	}

	CBuffer* c = (CBuffer*)d;
	if (!c)
		return NULL;

	return (FILE_INFO*)c->GetData();
}

FILE_INFO* CSourceFileListBox::GetFileInfoFromID(int i)
{
	return GetFileInfo(FileID2Index(i));
}

int CSourceFileListBox::FileID2Index(int id)
{
	for (int i=0; i<GetCount(); i++) {
		FILE_INFO* f = GetFileInfo(i);
		if (f && f->file_id == id)
			return i;
	}

	return -1;
}



void CSourceFileListBox::ItemDown()
{
	CEnhancedListBox::ItemDown();
}

void CSourceFileListBox::ItemUp()
{
	CEnhancedListBox::ItemUp();
}

bool CSourceFileListBox::CanMoveTo(int i, int direction)
{
	FILE_INFO* f1 = GetFileInfo(i);
	FILE_INFO* f2 = GetFileInfo(i+direction);
	if (!f1 || !f2 || f1->bInUse && f2->bInUse)
		return false;
	return true;
}

void CSourceFileListBox::SortItems()
{
	int i;
	for (i=0; i<GetCount(); SetSel(i++, 0));

	for (int i=0; i<GetCount(); i++)
		for (int j=0; j<GetCount()-i; j++)
			for (int k=j; k<GetCount()-i; k++) {
				FILE_INFO* f1 = GetFileInfo(j);
				FILE_INFO* f2 = GetFileInfo(k);

				if (f1 && f2 && f1->current_pos > f2->current_pos) {
					SetSel(k, true);
					ItemUp();
					SetSel(k-1, false);
				}

			}

	RedoNumbering();
}

void CSourceFileListBox::RedoNumbering()
{
	for (int i=0; i<GetCount(); i++) {
		FILE_INFO* f = GetFileInfo(i);
		if (f)
			f->current_pos = 2*i;
	}
}

void CSourceFileListBox::OnRButtonUp(UINT nFlags, CPoint point) 
{
	CMenu*		cmPopupMenu;
	int			i;
	FILE_INFO*	lpFI;
	CString		cStr;
	bool		bShowMenu=false;
//	CBuffer*	cb;
	MATROSKA*	m;

	cmPopupMenu=new CMenu;
	cmPopupMenu->CreatePopupMenu();
	
	cStr=LoadString(IDS_ADD);
	cmPopupMenu->AppendMenu(MF_STRING,IDM_ADDFILE,cStr);
	bShowMenu=true;

//	cStr=LoadString(IDS_ADD_ADVANCEDOPTIONS);
//	cmPopupMenu->AppendMenu(MF_STRING,IDM_OPENADVANCEDOPTIONS,cStr);
//	bShowMenu=true;

	if (GetCount())	{
		lpFI = GetFileInfo();
		if (lpFI) {
			if (bShowMenu) cmPopupMenu->AppendMenu(MF_SEPARATOR,0);
/*			if ((!lpFI->bInUse)&& (
				(lpFI->dwType & FILETYPE_AVI) ||
				(lpFI->dwType & FILETYPE_MKV)))*/ {
				cStr=LoadString(IDS_REMOVE);
				cmPopupMenu->AppendMenu(MF_STRING,IDM_REMOVE,cStr);
				bShowMenu=true;
			}
			
			cStr=LoadString(IDS_CLEARALL);
			cmPopupMenu->AppendMenu(MF_STRING,IDM_CLEARALL,cStr);
			
			if (lpFI->dwType& (FILETYPE_AVI | FILETYPE_MKV)) {
				if (bShowMenu) cmPopupMenu->AppendMenu(MF_SEPARATOR,0);
				cStr=LoadString(IDS_VIDEOINFORMATION);
				cmPopupMenu->AppendMenu(MF_STRING,IDM_VIDEOINFORMATION,cStr);
				bShowMenu=true;

				if ((lpFI->dwType & FILETYPE_MKV) == FILETYPE_MKV) {
					m = lpFI->MKVFile;
					int seg_count = lpFI->MKVFile->GetSegmentCount();
					if (seg_count > 1) {
						cmPopupMenu->AppendMenu(MF_SEPARATOR,0);
						for (i = 0; i<seg_count; i++) {
						//	lpFI->MK
							char title[1024]; title[0]=0;
							sprintf(title, "Segment %d: ", i);
							strcat(title, lpFI->MKVFile->GetSegmentTitle(i));
							UTF82Str(title, title);
							cmPopupMenu->AppendMenu(MF_STRING | 
								((m->GetActiveSegment() == i)?MF_CHECKED:MF_UNCHECKED) |
								((lpFI->bInUse)?MF_GRAYED:MF_ENABLED),IDM_SETACTIVESEGMENT + i,title);
						}
					}
				}
			}

			if (lpFI->dwType&FILETYPE_M2F2) {
				if (bShowMenu) cmPopupMenu->AppendMenu(MF_SEPARATOR,0);
				cStr=LoadString(IDS_EXTRACTM2F2);
				cmPopupMenu->AppendMenu(MF_STRING,IDM_EXTRACTM2F2,cStr);
				cStr=LoadString(IDS_M2F2CRC);
				cmPopupMenu->AppendMenu(MF_STRING | ((lpFI->bM2F2CRC)?MF_CHECKED:MF_UNCHECKED),IDM_M2F2CRC,cStr);
				bShowMenu=true;
			}

		/*	if (lpFI->dwType&FILETYPE_MKV) */ {
				if (bShowMenu) cmPopupMenu->AppendMenu(MF_SEPARATOR,0);
				cmPopupMenu->AppendMenu(MF_STRING, IDM_EBMLTREE, "EBML Tree");
				cmPopupMenu->AppendMenu(MF_STRING, IDM_RIFFTREE, "RIFF Tree");
			}
		}
	}
	
	if (bShowMenu)
	{
		ClientToScreen(&point);
		cmPopupMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON,point.x,point.y,this);
	}
	delete cmPopupMenu;

	CEnhancedListBox::OnRButtonUp(nFlags, point);
}

typedef struct
{
	FILE_INFO*			lpFI;
	STREAM*				lpDest;
	CListBox*			lpLB;
}  EXTRACTM2F2_THREAD_DATA;

int ExtractThread(EXTRACTM2F2_THREAD_DATA*	lpETD)
{
	DWORD				dwSize;
	__int64				qwRead;
	void*				lpBuffer;
	DWORD				dwTime;
	MODE2FORM2SOURCE*	lpM2F2;
	CString				cStr1,cStr2;
	bool				bOldInUseVal;
	CAVIMux_GUIDlg*		cMainDlg = (CAVIMux_GUIDlg*)lpETD->lpLB->GetParent();

	dwSize=1;
	bOldInUseVal=lpETD->lpFI->bInUse;
	SendMessage(cMainDlg->m_hWnd,WM_COMMAND,IDM_PROTOCOL,0);
	lpETD->lpFI->bInUse=true;
	lpETD->lpLB->GetParent()->SendDlgItemMessage(IDC_PROGBAR,PBM_SETRANGE32,0,(DWORD)(lpETD->lpFI->source->GetSize()>>10));
	lpBuffer=malloc(1<<20);
	lpM2F2=(MODE2FORM2SOURCE*)lpETD->lpFI->lpM2F2;
	lpM2F2->Seek(0);
	if (lpETD->lpFI->bM2F2CRC) lpM2F2->CheckCRC();
	qwRead=0;
	dwTime=GetTickCount();
	while (dwSize&&(qwRead<lpETD->lpFI->source->GetSize()))
	{
		dwSize=lpM2F2->Read(lpBuffer,2324);
		if (dwSize) lpETD->lpDest->Write(lpBuffer,dwSize);
		qwRead+=dwSize;
		if (GetTickCount()-dwTime>100)
		{
			lpETD->lpLB->GetParent()->SendDlgItemMessage(IDC_PROGBAR,PBM_SETPOS,(DWORD)(qwRead>>10),0);
			dwTime+=100;
		}
	}
	lpETD->lpDest->Close();
	lpETD->lpLB->GetParent()->SendDlgItemMessage(IDC_PROGBAR,PBM_SETPOS,(DWORD)(qwRead>>10),0);
	if (qwRead<lpETD->lpFI->source->GetSize())
	{
		cStr1=LoadString(IDS_M2F2READERROR);
		cStr2=LoadString(IDS_ERROR);
		lpETD->lpLB->MessageBox(cStr1,cStr2);
	}

	free (lpBuffer);
	lpETD->lpFI->bInUse=bOldInUseVal;
	cMainDlg->SendMessage(WM_COMMAND,IDM_CONFIGSOURCES,0);
	return 1;
}

int file_index_to_delete = -1;

BOOL CSourceFileListBox::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	FILE_INFO*	lpFI = NULL;
	CFileStream*	dest;
	EXTRACTM2F2_THREAD_DATA*	lpETD;
	CWinThread*	thread;
	CFileDialog*	cfd;
	CVideoInformationDlg*	cvid;
	CRIFFChunkTreeDlg*	crctd;
	CEBMLTreeDlg* cetd;
	CString		cStr1,cStr2;
	CBuffer*		cb = NULL;
	CAVIMux_GUIDlg*	cMainDlg = (CAVIMux_GUIDlg*)GetParent();
	char c[65536]; c[0]=0;
/*	iIndex=GetCurSel();
	if (iIndex!=LB_ERR) {
		cb = (CBuffer*)GetItemData(iIndex);
		if ((DWORD)cb != LB_ERR) lpFI=(FILE_INFO*)cb->GetData();
	}
*/

	lpFI = GetFileInfo();

	switch (LOWORD(wParam))
	{
		case IDM_EXTRACTM2F2:
			cfd=new CFileDialog(false,"avi");
			if (cfd->DoModal()==IDOK)
			{
				if (lpFI = GetFileInfo()) {
					dest=new CFileStream;
					if (dest->Open(cfd->GetPathName().GetBuffer(255),StreamMode::Write)==STREAM_ERR)
					{
						cStr1=LoadString(IDS_COULDNOTOPENOUTPUTFILE);
						cStr2=LoadString(IDS_ERROR);
						MessageBox(cStr1,cStr2,MB_OK | MB_ICONERROR);
						return CListBox::OnCommand(wParam, lParam);
					}
					lpETD=new EXTRACTM2F2_THREAD_DATA;
					lpETD->lpFI=lpFI;
					lpETD->lpDest=dest;
					lpETD->lpLB=this;
					thread=AfxBeginThread((AFX_THREADPROC)ExtractThread,lpETD);
				}
			}
			delete cfd;
			break;
		case IDM_M2F2CRC:
			if (lpFI = GetFileInfo()) {
				(lpFI->bM2F2CRC)=!(lpFI->bM2F2CRC);
			}
			break;
		case IDM_VIDEOINFORMATION:
			if (lpFI = GetFileInfo()) {
				cvid=new CVideoInformationDlg;
				cvid->SetFile(lpFI);

				cvid->Attribs( ((CAVIMux_GUIDlg*)GetParent())->GetSettings()->GetAttr("gui/file_information"));
				cvid->DoModal();
				delete cvid;
			}
			break;
		case IDM_REMOVE:
			if (file_index_to_delete > -1)
				lpFI = GetFileInfo(file_index_to_delete);
			else {
				file_index_to_delete = GetCurSel();
				lpFI = GetFileInfo();
			}
			if (lpFI) {
				FILE_INFO* fi = lpFI;

				if (!fi->bInUse) {

					//if (fi->lpcName) free(fi->lpcName);
					//fi->lpcName=NULL;
					if (fi->dwType & FILETYPE_AVI) {
						fi->AVIFile->Close(false);
						delete fi->AVIFile;
					}
					if (fi->dwType & FILETYPE_MKV) {
						fi->MKVFile->Close();
						delete fi->MKVFile;
					}
					if (fi->source) {
						fi->source->Close();
						delete fi->source;
					}

					DecBufferRefCount(&cb);
					DeleteString(file_index_to_delete);				
				} else {
					std::vector<HTREEITEM> htreeitems;
					std::vector<DWORD> dwFileIndexes;
					std::vector<DWORD> dwStreamNumbers;

					cMainDlg->BuildFileAndStreamDependency(GetCurSel(),
						htreeitems, dwStreamNumbers, dwFileIndexes);
					std::sort(dwFileIndexes.begin(), dwFileIndexes.end());
					sprintf(c, "%s: \n", LoadString(STR_FILELIST_REMOVE_FILES, LOADSTRING_UTF8));
					for (size_t i=0; i<dwFileIndexes.size(); i++) {
						FILE_INFO* f = GetFileInfo(dwFileIndexes[i]);
						strcat(c, "  ");
						strcat(c, f->Name.TStr());
						strcat(c, "\x0D");
					}

					strcat(c, "\x0D");
					strcat(c, LoadString(STR_FILELIST_REMOVE_STREAMS, LOADSTRING_UTF8));
					strcat(c, ": \n");
					for (size_t i=0; i<dwStreamNumbers.size(); i++) {
						char d[16]; d[0]=0;
						sprintf(d, "  %d: ", dwStreamNumbers[i]);
						strcat(c, d);

						char buf[512];
						TV_DISPINFO tvi;
						tvi.item.pszText = buf;
						tvi.item.cchTextMax = 256;
						tvi.item.mask = TVIF_TEXT;
						tvi.item.hItem = htreeitems[i];

						LRESULT lres;
					
						cMainDlg->m_StreamTree.OnTvnGetdispinfo((NMHDR*)&tvi, &lres);
						
						strcat(c, tvi.item.pszText);
						strcat(c, "\x0D");
					}
					strcat(c, "\x0D");
					strcat(c, LoadString(STR_FILELIST_REMOVE_CONTINUE, LOADSTRING_UTF8));

				//	char* u  = NULL;
				//	fromUTF8(c, &u);
				//	char* hdr = NULL;
				//	fromUTF8(LoadString(STR_GEN_WARNING, LOADSTRING_UTF8), &hdr);
				
					int msg_res = MessageBoxUTF8(NULL, c, LoadString(STR_GEN_WARNING, LOADSTRING_UTF8),
						MB_YESNO | MB_DEFBUTTON2 | MB_ICONWARNING);

				//	int msg_res = (*UMessageBox())(0, u, hdr, MB_YESNO | MB_DEFBUTTON2 | MB_ICONWARNING);
					if (msg_res == IDYES) {
						int i;
						for (i=0;i<(int)htreeitems.size();i++)
							cMainDlg->m_StreamTree.DeleteItem(htreeitems[i]);
						std::sort(dwFileIndexes.begin(), dwFileIndexes.end());
						for (i=dwFileIndexes.size()-1;i>=0;i--) {
							int k = dwFileIndexes[i];
							FILE_INFO* f = GetFileInfo(k);
							f->bInUse = false;
							file_index_to_delete = k;
							SendMessage(WM_COMMAND, IDM_REMOVE);
						}
					}

				//	free(u);
				//	free(hdr);

				}

				file_index_to_delete = -1;
			}

			break;
		case IDM_ADDFILE:
			cMainDlg->PostMessage(cMainDlg->GetUserMessageID(),IDM_ADDFILE);
			break;
		case IDM_OPENADVANCEDOPTIONS:
			cMainDlg->PostMessage(cMainDlg->GetUserMessageID(),IDM_OPENADVANCEDOPTIONS);
			break;
		case IDM_CLEARALL:
			cMainDlg->PostMessage(cMainDlg->GetUserMessageID(),IDM_CLEARALL);
			break;
		case IDM_EBMLTREE:
			cetd = new CEBMLTreeDlg;
			cetd->Attribs( ((CAVIMux_GUIDlg*)GetParent())->GetSettings()->GetAttr("gui/ebml_tree"));
			cetd->SetSource(lpFI->source);
			cetd->DoModal();
			delete cetd;
			break;
		case IDM_RIFFTREE:
			crctd = new CRIFFChunkTreeDlg;
			crctd->Attribs( ((CAVIMux_GUIDlg*)GetParent())->GetSettings()->GetAttr("gui/riff_tree"));
			crctd->SetSource(lpFI->source);
			crctd->DoModal();
			delete crctd;
			break;

	}

	if (lpFI && (lpFI->dwType & FILETYPE_MKV) == FILETYPE_MKV) {
		if (LOWORD(wParam)>=IDM_SETACTIVESEGMENT && LOWORD(wParam) <= IDM_SETACTIVESEGMENT + lpFI->MKVFile->GetSegmentCount()) {
			lpFI->MKVFile->SetActiveSegment(LOWORD(wParam)-IDM_SETACTIVESEGMENT);
		}
	}


	return CEnhancedListBox::OnCommand(wParam, lParam);
}

void CSourceFileListBox::OnDropFiles(HDROP hDropInfo) 
{
	// TODO: Code für die Behandlungsroutine für Nachrichten hier einfügen und/oder Standard aufrufen
	DWORD	dwCount;
	CAVIMux_GUIDlg*  cdlg;

	dwCount=DragQueryFile(hDropInfo,0xFFFFFFFF,NULL,NULL);

	for (int i=0;i<(int)dwCount;i++)
	{
		int size = (*UDragQueryFile())((uint32)hDropInfo, i, NULL, 0);
		char* temp = (char*)calloc(2, size + 1);
		char* lpcName = NULL;	

		(*UDragQueryFile())((uint32)hDropInfo, i, temp, size + 1);
		
		toUTF8(temp, &lpcName);
		free(temp);

		cdlg = (CAVIMux_GUIDlg*)GetParent();
		cdlg->PostMessage(cdlg->GetUserMessageID(),IDM_DOADDFILE,(LPARAM)lpcName);
	}

	CEnhancedListBox::OnDropFiles(hDropInfo);
}

void CSourceFileListBox::OnPaint() 
{
//	CPaintDC dc(this); // device context for painting
	CEnhancedListBox::OnPaint();
	
	// TODO: Code für die Behandlungsroutine für Nachrichten hier einfügen
	
	// Kein Aufruf von CEnhancedListBox::OnPaint() für Zeichnungsnachrichten
}

void CSourceFileListBox::SetFlag(int listbox_index, int flag_index, DWORD flag_value,
								 bool value, bool reset_others)
{
	FILE_INFO* f = GetFileInfo(listbox_index);
	if (!f)
		return;

	DWORD* pFlag = &f->dwFlags[flag_index];

	bool bOldValue = !!(*pFlag & flag_value);
	if (value)
		*pFlag |= flag_value;
	else
		*pFlag &=~ flag_value;

	RECT r;
	GetItemRect(listbox_index, &r);
	if (value != bOldValue)
		InvalidateRect(&r, false);

	if (reset_others) {
		for (int j=GetCount()-1; j>=0; j--)
			if (j!=listbox_index) {
				bOldValue = !!(GetFileInfo(j)->dwFlags[flag_index] & flag_value);
				if (bOldValue) {
					GetFileInfo(j)->dwFlags[flag_index] &=~ flag_value;
					GetItemRect(j, &r);
					InvalidateRect(&r, false);
				}
			}
	}

	UpdateWindow();
}

void CSourceFileListBox::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) 
{
	// TODO: Code einfügen, um das angegebene Element zu zeichnen
	CDC	dc;
	LPDRAWITEMSTRUCT d = lpDrawItemStruct;
	FILE_INFO* lpFI = GetFileInfo(d->itemID);

	if (!lpFI)
		return;

    dc.Attach(d->hDC);
	COLORREF crOldBkColor = dc.GetBkColor();
	COLORREF bkColor;

	char* u = NULL;
	CFont* font = GetFont();

	LOGFONT logfont;
	font->GetLogFont(&logfont);

	char cTempUTF8[65536]; cTempUTF8[0]=0;
	sprintf(cTempUTF8, "[%s] %s", lpFI->cFileformatString, lpFI->Name.UTF8());
	
	int j = fromUTF8(cTempUTF8, &u);
	if (IsUnicode())
		j /= 2;

	if (lpFI->dwFlags[0] & FILEINFO_FLAG0_DEEMPH) {
		if ((d->itemState & ODS_SELECTED)) {
			
			if (GetFocus() == this) {
				dc.SetTextColor(GetSysColor(COLOR_HIGHLIGHTTEXT));
				bkColor = (GetSysColor(COLOR_HIGHLIGHT));
			} else {
				dc.SetTextColor(GetSysColor(COLOR_WINDOWTEXT));
				bkColor = (GetSysColor(COLOR_BTNFACE));
			}
		} else {
			dc.SetTextColor(GetSysColor(COLOR_GRAYTEXT));
			bkColor = (GetSysColor(COLOR_WINDOW));
		}
	} else {
		if ((d->itemState & ODS_SELECTED)) {
			if (GetFocus() == this) {
				dc.SetTextColor(GetSysColor(COLOR_HIGHLIGHTTEXT));
				bkColor = (GetSysColor(COLOR_HIGHLIGHT));
			} else {
				dc.SetTextColor(GetSysColor(COLOR_WINDOWTEXT));
				bkColor = (GetSysColor(COLOR_BTNFACE));
			}
		} else {
			dc.SetTextColor(GetSysColor(COLOR_WINDOWTEXT));
			bkColor = (GetSysColor(COLOR_WINDOW));
		}
	}

	dc.SetTextAlign(TA_LEFT);
	dc.SetBkMode(TRANSPARENT);

	if (lpFI->dwFlags[0] & FILEINFO_FLAG0_BOLD) {
		logfont.lfWeight = FW_BOLD;
	}

	if (lpFI->dwFlags[0] & FILEINFO_FLAG0_EMPH) {
		if (bkColor == GetSysColor(COLOR_HIGHLIGHT))
			dc.SetTextColor(bkColor ^ 0xFFFFFF);
		else
			dc.SetTextColor(RGB(255,0,0));
	}

	CFont* used_font = new CFont();
	used_font->CreateFontIndirect(&logfont);

	dc.SelectObject(used_font);
	used_font->DeleteObject();
	delete used_font;

	LOGBRUSH b;
	b.lbColor = bkColor;
	b.lbStyle = BS_SOLID;
	HBRUSH brush = CreateBrushIndirect(&b);

	LOGPEN p;
	p.lopnColor = 0;
	p.lopnStyle = PS_NULL;
	HPEN pen = CreatePenIndirect(&p);

	dc.SelectObject(brush);
	dc.SelectObject(pen);
	d->rcItem.right++;
	d->rcItem.bottom++;
	dc.Rectangle(&d->rcItem);

	(*UTextOut())(dc,d->rcItem.left,d->rcItem.top,u,j-1);

	DeleteObject(brush);
	DeleteObject(pen);
	
	dc.Detach();
	free(u);
}

void CSourceFileListBox::OnKillFocus(CWnd* pNewWnd)
{
	CEnhancedListBox::OnKillFocus(pNewWnd);

	InvalidateRect(NULL, false);
	// TODO: Fügen Sie hier Ihren Meldungsbehandlungscode ein.
}
