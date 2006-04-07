// SourceFileListBox.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "AVIMux_GUI.h"
#include "AVIMux_GUIDlg.h"
#include "AVIFile.h"
#include "SourceFileListBox.h"
#include "..\BaseStreams.h"

#include "mode2form2reader.h"
#include "VideoInformationDlg.h"
#include "Languages.h"
#include "..\Buffers.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include "FILE_INFO.h"


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
	//}}AFX_MSG_MAP
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
	return (FILE_INFO*)c->GetData();
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

	if (GetCount())
	{
		lpFI = GetFileInfo();
		//iIndex=GetCurSel();
		//if (iIndex!=LB_ERR)
		if (lpFI)
		{
//			cb = (CBuffer*)GetItemData(iIndex);
//			lpFI=(FILE_INFO*)cb->GetData();

			if (bShowMenu) cmPopupMenu->AppendMenu(MF_SEPARATOR,0);
			if ((!lpFI->bInUse)&&(lpFI->dwType&FILETYPE_AVI))
			{
				cStr=LoadString(IDS_REMOVE);
				cmPopupMenu->AppendMenu(MF_STRING,IDM_REMOVE,cStr);
				bShowMenu=true;
			}
			cStr=LoadString(IDS_CLEARALL);
			cmPopupMenu->AppendMenu(MF_STRING,IDM_CLEARALL,cStr);
			
			if (lpFI->dwType& (FILETYPE_AVI | FILETYPE_MKV))
			{
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

			if (lpFI->dwType&FILETYPE_M2F2)
			{
				if (bShowMenu) cmPopupMenu->AppendMenu(MF_SEPARATOR,0);
				cStr=LoadString(IDS_EXTRACTM2F2);
				cmPopupMenu->AppendMenu(MF_STRING,IDM_EXTRACTM2F2,cStr);
				cStr=LoadString(IDS_M2F2CRC);
				cmPopupMenu->AppendMenu(MF_STRING | ((lpFI->bM2F2CRC)?MF_CHECKED:MF_UNCHECKED),IDM_M2F2CRC,cStr);
				bShowMenu=true;
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

BOOL CSourceFileListBox::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	FILE_INFO*	lpFI = NULL;
	FILESTREAM*	dest;
	EXTRACTM2F2_THREAD_DATA*	lpETD;
	CWinThread*	thread;
	CFileDialog*	cfd;
	CVideoInformationDlg*	cvid;
	CString		cStr1,cStr2;
	int			iIndex;
	CBuffer*		cb = NULL;
	CAVIMux_GUIDlg*	cMainDlg = (CAVIMux_GUIDlg*)GetParent();

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
/*				iIndex=GetCurSel();
				if (iIndex!=LB_ERR)
				{
				
					cb = (CBuffer*)GetItemData(iIndex);
					lpFI=(FILE_INFO*)cb->GetData();
*/
					dest=new FILESTREAM;
					if (dest->Open(cfd->GetPathName().GetBuffer(255),STREAM_WRITE)==STREAM_ERR)
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
/*			iIndex=GetCurSel();
			if (iIndex!=LB_ERR)
			{
				cb = (CBuffer*)GetItemData(iIndex);
				lpFI=(FILE_INFO*)cb->GetData();
				(lpFI->bM2F2CRC)=!(lpFI->bM2F2CRC);
			}*/
			break;
		case IDM_VIDEOINFORMATION:
			if (lpFI = GetFileInfo()) {
/*			iIndex=GetCurSel();
			if (iIndex!=LB_ERR)
			{
				cb = (CBuffer*)GetItemData(iIndex);
				lpFI=(FILE_INFO*)cb->GetData();*/
				cvid=new CVideoInformationDlg;
				cvid->SetFile(lpFI);
				cvid->DoModal();
				delete cvid;
			}
			break;
		case IDM_REMOVE:
			if (lpFI = GetFileInfo()) {
			iIndex=GetCurSel();
			if (iIndex!=LB_ERR)
			{
/*				CBuffer* cb = (CBuffer*)GetItemData(iIndex);
				lpFI=(FILE_INFO*)cb->GetData();*/
				if (lpFI->lpcName) free(lpFI->lpcName);
				if (lpFI->lpM2F2)
				{
					lpFI->lpM2F2->Close();
					delete lpFI->lpM2F2;
					lpFI->lpM2F2=NULL;
				}
				if (lpFI->cache)
				{
					lpFI->cache->Close();
					delete lpFI->cache;
				}
				if (lpFI->AVIFile)
				{
					lpFI->AVIFile->Close(false);
					delete lpFI->AVIFile;
				}
				if (lpFI->file)
				{
					lpFI->file->Close();
					delete lpFI->file;
				}	
				DeleteString(iIndex);				
				DecBufferRefCount(&cb);
			}}
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
	char*	lpcName;
	CAVIMux_GUIDlg*  cdlg;

	dwCount=DragQueryFile(hDropInfo,0xFFFFFFFF,NULL,NULL);

	for (int i=0;i<(int)dwCount;i++)
	{
		lpcName = (char*)calloc(1,1000);
		DragQueryFile(hDropInfo,i,lpcName,1000);
		cdlg = (CAVIMux_GUIDlg*)GetParent();
		cdlg->PostMessage(cdlg->GetUserMessageID(),IDM_DOADDFILE,(LPARAM)lpcName);
	}

	CEnhancedListBox::OnDropFiles(hDropInfo);
}
