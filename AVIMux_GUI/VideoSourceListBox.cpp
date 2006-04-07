// VideoSourceListBox.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "AVIMux_GUI.h"
#include "AVIMux_GUIDlg.h"
#include "VideoInformationDlg.h"
#include "VideoSourceListBox.h"
#include "VideoSource.h"
#include "Muxing.h"
#include "../FormatTime.h"
#include "Languages.h"
#include "FileDialogs.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CVideoSourceListBox

CVideoSourceListBox::CVideoSourceListBox()
{
	EnableAutomation();
}

CVideoSourceListBox::~CVideoSourceListBox()
{
}

void CVideoSourceListBox::OnFinalRelease()
{
	// Nachdem die letzte Referenz auf ein Automatisierungsobjekt freigegeben wurde,
	// wird OnFinalRelease aufgerufen. Die Basisklasse löscht das Objekt
	// automatisch. Fügen Sie zusätzlichen Bereinigungscode für Ihr Objekt
	// hinzu, bevor Sie die Basisklasse aufrufen.

	CEnhancedListBox::OnFinalRelease();
}


BEGIN_MESSAGE_MAP(CVideoSourceListBox, CEnhancedListBox)
	//{{AFX_MSG_MAP(CVideoSourceListBox)
	ON_WM_RBUTTONUP()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BEGIN_DISPATCH_MAP(CVideoSourceListBox, CEnhancedListBox)
	//{{AFX_DISPATCH_MAP(CVideoSourceListBox)
		// HINWEIS - Der Klassen-Assistent fügt hier Zuordnungsmakros ein und entfernt diese.
	//}}AFX_DISPATCH_MAP
END_DISPATCH_MAP()

// Hinweis: Wir stellen Unterstützung für IID_IVideoSourceListBox zur Verfügung, um typsicheres Binden
//  von VBA zu ermöglichen. Diese IID muss mit der GUID übereinstimmen, die in der
//  Disp-Schnittstelle in der .ODL-Datei angegeben ist.

// {3A30BEE7-54FF-4809-866A-B4F58D6278C3}
static const IID IID_IVideoSourceListBox =
{ 0x3a30bee7, 0x54ff, 0x4809, { 0x86, 0x6a, 0xb4, 0xf5, 0x8d, 0x62, 0x78, 0xc3 } };

BEGIN_INTERFACE_MAP(CVideoSourceListBox, CListBox)
	INTERFACE_PART(CVideoSourceListBox, IID_IVideoSourceListBox, Dispatch)
END_INTERFACE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CVideoSourceListBox 

void CVideoSourceListBox::OnRButtonUp(UINT nFlags, CPoint point) 
{
	// TODO: Code für die Behandlungsroutine für Nachrichten hier einfügen und/oder Standard aufrufen
	CMenu*			cmPopupMenu;
	int				iIndex;
	CString			cStr;
	bool			bShowMenu=false;

	cmPopupMenu=new CMenu;
	cmPopupMenu->CreatePopupMenu();
	
	if (GetCount())
	{
		iIndex=GetCurSel();
		if (iIndex!=LB_ERR)
		{
			if (bShowMenu) cmPopupMenu->AppendMenu(MF_SEPARATOR,0);
			
			if (bShowMenu) cmPopupMenu->AppendMenu(MF_SEPARATOR,0);
			cStr=LoadString(IDS_VSLB_INFO);
			cmPopupMenu->AppendMenu(MF_STRING,IDM_VIDEOINFORMATION,cStr);
			bShowMenu=true;

			if (bShowMenu) cmPopupMenu->AppendMenu(MF_SEPARATOR,0);
			
			cmPopupMenu->AppendMenu(MF_STRING,IDM_EXTRACT_BINARY,LoadString(STR_MAIN_A_EXTRBIN));
		}
	}
	
	if (bShowMenu)
	{
		ClientToScreen(&point);
		cmPopupMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON,point.x,point.y,this);
	}
	delete cmPopupMenu;

	CEnhancedListBox::OnRButtonUp(nFlags, point);
	
	CEnhancedListBox::OnRButtonUp(nFlags, point);
}

typedef struct 
{
	FILESTREAM* file;
	CAVIMux_GUIDlg* dlg;
	VIDEOSOURCE* v;

} EXTRACT_THREAD_VIDEO_DATA;

int ExtractThread(EXTRACT_THREAD_VIDEO_DATA*	lpETD)
{
	lpETD->dlg->SetDialogState_Muxing();
	lpETD->dlg->ButtonState_START();

	lpETD->dlg->AddProtocolLine("started extracting binary", 4);

	VIDEOSOURCE* v = lpETD->v;
	v->Enable(1);
	FILESTREAM* f = lpETD->file;
	char cTime[20];
	char* lpBuffer = new char[2<<20];
	int iLastTime = GetTickCount();
	v->ReInit();

	while (!v->IsEndOfStream() && !DoStop()) {
		__int64 iTimecode;
		__int64 iNS = 0;
		DWORD dwSize;
		v->GetFrame(lpBuffer,&dwSize,&iTimecode);
		f->Write(lpBuffer,dwSize);
		if (GetTickCount()-iLastTime>100 || v->IsEndOfStream()) {
			Millisec2Str((iTimecode * v->GetTimecodeScale() + iNS)/ 1000000,cTime);
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

	v->Enable(0);

	return 1;
}


BOOL CVideoSourceListBox::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	// TODO: Speziellen Code hier einfügen und/oder Basisklasse aufrufen
	CVideoInformationDlg*	lpvidlg;
	int						iIndex;
	VIDEOSOURCE*			lpVS;
	// TODO: Speziellen Code hier einfügen und/oder Basisklasse aufrufen
	CWinThread*	thread;
//	CFileDialog*	cfd;
	CAVIMux_GUIDlg*	cMainDlg = (CAVIMux_GUIDlg*)GetParent();
	char cFilter[256];
	char cDefExt[5];
	int open = 0;
	VIDEOSOURCE* v;
	OPENFILENAME o; 
	memset(&o, 0, sizeof(o));


	switch (LOWORD(wParam))
	{
		case IDM_VIDEOINFORMATION:
			iIndex=GetCurSel();
			if (iIndex!=LB_ERR)
			{
				lpVS=(VIDEOSOURCE*)GetItemData(iIndex);
				lpvidlg=new CVideoInformationDlg;
				lpvidlg->SetVideoSource(lpVS);
				lpvidlg->DoModal();
				delete lpvidlg;
			}
			break;
		case IDM_EXTRACT_BINARY:
			ZeroMemory(cFilter,sizeof(cFilter));
			ZeroMemory(cDefExt,sizeof(cDefExt));
			
			iIndex = GetCurSel();
			if (iIndex == LB_ERR)
				iIndex = 0;

			v = (VIDEOSOURCE*)GetItemData(iIndex);

	
			strcpy(cFilter,"*.raw|*.raw||");
			strcpy(cDefExt,"mp3");
			 

			PrepareSimpleDialog(&o, m_hWnd, cFilter);
			o.Flags |= OFN_OVERWRITEPROMPT;
			o.lpstrDefExt = cDefExt;
			open = GetOpenSaveFileNameUTF8(&o, 0);
			
			if (open) {
				EXTRACT_THREAD_VIDEO_DATA* lpETD = new EXTRACT_THREAD_VIDEO_DATA;
				lpETD->file = new FILESTREAM;
				if (lpETD->file->Open(o.lpstrFile,STREAM_WRITE)!=STREAM_ERR) {
					lpETD->dlg = cMainDlg;
					lpETD->v = v;
					cMainDlg->m_Prg_Dest_File.SetWindowText(o.lpstrFile);
					thread=AfxBeginThread((AFX_THREADPROC)ExtractThread,lpETD);
				} else {
					MessageBox(LoadString(IDS_COULDNOTOPENOUTPUTFILE),LoadString(STR_GEN_ERROR),
						MB_OK | MB_ICONERROR);
				}
			}
			
			break;
	}
	
	return CEnhancedListBox::OnCommand(wParam, lParam);
}
