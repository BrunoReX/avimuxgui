// SubtitlesListBox.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "AVIMux_GUI.h"
#include "AVIMux_GUIDlg.h"
#include "SubtitlesListBox.h"
#include "SubTitles.h"
#include "Languages.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSubtitlesListBox

CSubtitlesListBox::CSubtitlesListBox()
{
	EnableAutomation();
	iDown=-1;
}

CSubtitlesListBox::~CSubtitlesListBox()
{
}

void CSubtitlesListBox::OnFinalRelease()
{
	// Nachdem die letzte Referenz auf ein Automatisierungsobjekt freigegeben wurde,
	// wird OnFinalRelease aufgerufen. Die Basisklasse löscht das Objekt
	// automatisch. Fügen Sie zusätzlichen Bereinigungscode für Ihr Objekt
	// hinzu, bevor Sie die Basisklasse aufrufen.

	CListBox::OnFinalRelease();
}


BEGIN_MESSAGE_MAP(CSubtitlesListBox, CEnhancedListBox)
	//{{AFX_MSG_MAP(CSubtitlesListBox)
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONUP()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BEGIN_DISPATCH_MAP(CSubtitlesListBox, CEnhancedListBox)
	//{{AFX_DISPATCH_MAP(CSubtitlesListBox)
		// HINWEIS - Der Klassen-Assistent fügt hier Zuordnungsmakros ein und entfernt diese.
	//}}AFX_DISPATCH_MAP
END_DISPATCH_MAP()

// Hinweis: Wir stellen Unterstützung für IID_ISubtitlesListBox zur Verfügung, um typsicheres Binden
//  von VBA zu ermöglichen. Diese IID muss mit der GUID übereinstimmen, die in der
//  Disp-Schnittstelle in der .ODL-Datei angegeben ist.

// {E058EC63-6AD7-4002-A1BC-3C7FF027D5E7}
static const IID IID_ISubtitlesListBox =
{ 0xe058ec63, 0x6ad7, 0x4002, { 0xa1, 0xbc, 0x3c, 0x7f, 0xf0, 0x27, 0xd5, 0xe7 } };

BEGIN_INTERFACE_MAP(CSubtitlesListBox, CEnhancedListBox)
	INTERFACE_PART(CSubtitlesListBox, IID_ISubtitlesListBox, Dispatch)
END_INTERFACE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CSubtitlesListBox 

void CSubtitlesListBox::OnLButtonUp(UINT nFlags, CPoint point) 
{
	// TODO: Code für die Behandlungsroutine für Nachrichten hier einfügen und/oder Standard aufrufen
	int		i,b;
	CAVIMux_GUIDlg*	cMainDlg = (CAVIMux_GUIDlg*)GetParent();

	char	name[200];
	SUBTITLE_STREAM_INFO*	lpssi;

	if (iDown!=-1)
	{
		i=ItemFromPoint(point,b);
		lpssi=(SUBTITLE_STREAM_INFO*)GetItemData(iDown);
		lpssi->lpsubs->GetName(name);
	}
	else
	{
		if (GetSelCount())
		{
			lpssi=(SUBTITLE_STREAM_INFO*)GetItemData(GetCaretIndex());
			lpssi->lpsubs->GetName(name);
		}
		
	}

	if (GetSelCount())
	{
		cMainDlg->SendMessage(cMainDlg->GetUserMessageID(),IDM_DISPLAYSUBNAME,(LPARAM)name);
	}

	CEnhancedListBox::OnLButtonUp(nFlags, point);
}


void CSubtitlesListBox::OnLButtonDown(UINT nFlags, CPoint point) 
{
	// TODO: Code für die Behandlungsroutine für Nachrichten hier einfügen und/oder Standard aufrufen

	CEnhancedListBox::OnLButtonDown(nFlags, point);
}

void CSubtitlesListBox::OnRButtonUp(UINT nFlags, CPoint point) 
{
	// TODO: Code für die Behandlungsroutine für Nachrichten hier einfügen und/oder Standard aufrufen
	CMenu*		cmPopupMenu;
	int			iIndex;
	SUBTITLE_STREAM_INFO*	lpSSI;
	CString		cStr;
	bool		bItemPresent=false;

	if (GetCount())
	{
		iIndex=GetCurSel();
		if (iIndex!=LB_ERR)
		{
			lpSSI=(SUBTITLE_STREAM_INFO*)GetItemData(iIndex);
			cmPopupMenu = new CMenu;
			cmPopupMenu->CreatePopupMenu();
			if (lpSSI->lpsubs->GetFormat()==SUBFORMAT_SRT)
			{
				if (bItemPresent) cmPopupMenu->AppendMenu(MF_SEPARATOR,0);
				cStr=LoadString(STR_SUBT_EXTRACTSRT);
				cmPopupMenu->AppendMenu(MF_STRING,IDM_EXTRACTSRT,cStr);
				bItemPresent=true;
			}
			if (lpSSI->lpsubs->GetFormat()==SUBFORMAT_SSA)
			{
				cStr=LoadString(STR_SUBT_EXTRACTSSA);
				cmPopupMenu->AppendMenu(MF_STRING,IDM_EXTRACTSSA,cStr);
				bItemPresent=true;
			}

			ClientToScreen(&point);
			cmPopupMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON,point.x,point.y,this);
			delete cmPopupMenu;
		}
	}
	
	CEnhancedListBox::OnRButtonUp(nFlags, point);
}

BOOL CSubtitlesListBox::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	// TODO: Speziellen Code hier einfügen und/oder Basisklasse aufrufen
	CFileDialog*		cfd;
	FILESTREAM*			dest;
	int					iIndex;
	CString				cStr1,cStr2;
	SUBTITLE_STREAM_INFO* lpSSI;
	void*				lpBuffer;
	DWORD				dwSize;

	switch (LOWORD(wParam))
	{
		case IDM_EXTRACTSRT:
		case IDM_EXTRACTSSA:
			iIndex=GetCurSel();
			lpSSI = (SUBTITLE_STREAM_INFO*)GetItemData(iIndex);
			if (iIndex!=LB_ERR)
			{
				cfd=new CFileDialog(false,subformat_names[lpSSI->lpsubs->GetFormat()]);
				if (cfd->DoModal()==IDOK)
				{
					dest = new FILESTREAM;
					if (dest->Open(cfd->GetPathName().GetBuffer(255),STREAM_WRITE)!=STREAM_OK)
					{
						MessageBox(LoadString(IDS_COULDNOTOPENOUTPUTFILE),LoadString(IDS_ERROR),MB_OK | MB_ICONERROR);
					}
					lpBuffer = malloc(1<<22);
					lpSSI->lpsubs->SetRange(0,0xFFFFFFFFFFFFFFF);
					dwSize = lpSSI->lpsubs->Render2Text(lpBuffer);
					dest->Write(lpBuffer,dwSize);
					dest->Close();
					delete dest;
					delete lpBuffer;
				}
				delete cfd;
			}
				
			break;
	}

	
	return CEnhancedListBox::OnCommand(wParam, lParam);
}
