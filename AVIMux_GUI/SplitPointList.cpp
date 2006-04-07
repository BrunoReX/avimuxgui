// SplitPointList.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "AVIMux_GUI.h"
#include "SplitPointList.h"
#include "AddSplitPointDlg.h"
#include "Languages.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSplitPointList

CSplitPointList::CSplitPointList()
{
	EnableAutomation();
}

CSplitPointList::~CSplitPointList()
{
}

void CSplitPointList::OnFinalRelease()
{
	// Nachdem die letzte Referenz auf ein Automatisierungsobjekt freigegeben wurde,
	// wird OnFinalRelease aufgerufen. Die Basisklasse löscht das Objekt
	// automatisch. Fügen Sie zusätzlichen Bereinigungscode für Ihr Objekt
	// hinzu, bevor Sie die Basisklasse aufrufen.

	CListBox::OnFinalRelease();
}


BEGIN_MESSAGE_MAP(CSplitPointList, CListBox)
	//{{AFX_MSG_MAP(CSplitPointList)
	ON_WM_RBUTTONUP()
	ON_WM_INITMENUPOPUP()
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BEGIN_DISPATCH_MAP(CSplitPointList, CListBox)
	//{{AFX_DISPATCH_MAP(CSplitPointList)
		// HINWEIS - Der Klassen-Assistent fügt hier Zuordnungsmakros ein und entfernt diese.
	//}}AFX_DISPATCH_MAP
END_DISPATCH_MAP()

// Hinweis: Wir stellen Unterstützung für IID_ISplitPointList zur Verfügung, um typsicheres Binden
//  von VBA zu ermöglichen. Diese IID muss mit der GUID übereinstimmen, die in der
//  Disp-Schnittstelle in der .ODL-Datei angegeben ist.

// {6DC593B2-51BA-4696-A486-693E9F8AA26E}
static const IID IID_ISplitPointList =
{ 0x6dc593b2, 0x51ba, 0x4696, { 0xa4, 0x86, 0x69, 0x3e, 0x9f, 0x8a, 0xa2, 0x6e } };

BEGIN_INTERFACE_MAP(CSplitPointList, CListBox)
	INTERFACE_PART(CSplitPointList, IID_ISplitPointList, Dispatch)
END_INTERFACE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CSplitPointList 

void CSplitPointList::OnRButtonUp(UINT nFlags, CPoint point) 
{
/*	int		iIndex;
	CString	cStr;
	
	cmPopupMenu=new CMenu;
	cmPopupMenu->CreatePopupMenu();
	cStr=LoadString(IDS_ADD);
	cmPopupMenu->AppendMenu(MF_STRING,IDM_ADDSPLITPOINT,cStr);

	iIndex=GetCurSel();
	if (iIndex!=LB_ERR)
	{
		cStr=LoadString(IDS_REMOVE);
		cmPopupMenu->AppendMenu(MF_STRING,IDM_REMOVESPLITPOINT,cStr);
		cStr=LoadString(IDS_CHANGE);
		cmPopupMenu->AppendMenu(MF_STRING,IDM_CHANGESPLITPOINT,cStr);
	}
	ClientToScreen(&point);
	cmPopupMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON,point.x,point.y,this);
*/
	CListBox::OnRButtonUp(nFlags, point);
}

void CSplitPointList::OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu) 
{
	CListBox::OnInitMenuPopup(pPopupMenu, nIndex, bSysMenu);
	
}

int CSplitPointList::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	Beep(300,100);

	if (CListBox::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// TODO: Speziellen Erstellungscode hier einfügen

	
	return 0;
}

bool CSplitPointList::CheckSplitPoint(DWORD dwSplitPos)
{
	char				Buffer[200];
	CString				cStr;
	CString				cStr2;

	if (lpVideoSource)
	{
		if ((int)dwSplitPos>=(int)lpVideoSource->GetNbrOfFrames())
		{
			cStr=LoadString(IDS_ERROR);
			cStr2=LoadString(IDS_FRAMENBRTOOHIGH);
			wsprintf(Buffer,cStr2.GetBuffer(200),dwSplitPos);
			MessageBox(Buffer,cStr,MB_OK | MB_ICONERROR);
			return false;
		}
		else
		if (!lpVideoSource->IsKeyFrame(dwSplitPos))
		{
			cStr=LoadString(IDS_WARNING);
			cStr2=LoadString(IDS_THISISNOKEYFRAME);
			wsprintf(Buffer,cStr2,dwSplitPos);
			return (MessageBox(Buffer,cStr,MB_ICONWARNING | MB_YESNO)==IDNO)?false:true;
		}
		else return true;
	}
	return false;
}

void CSplitPointList::AddSplitPoint(DWORD dwSplitPos)
{
	char				Buffer[200];

	if (dwSplitPos)
	{
		wsprintf(Buffer,"%d",dwSplitPos);
		AddString(Buffer);
	}
}

BOOL CSplitPointList::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	// TODO: Speziellen Code hier einfügen und/oder Basisklasse aufrufen
	AddSplitPointDlg*	aspd;
	DWORD				dwSplitPos;
	int					iIndex;
	char				Buffer[200];

	switch (LOWORD(wParam))
	{
		case IDM_ADDSPLITPOINT:
			aspd=new AddSplitPointDlg;
			aspd->SetSplitPos(0);
			if (aspd->DoModal())
			{
				dwSplitPos=aspd->GetSplitPos();
				if (CheckSplitPoint(dwSplitPos)) AddSplitPoint(dwSplitPos);
			}
			delete aspd;
			break;
		case IDM_REMOVESPLITPOINT:
			iIndex=GetCurSel();
			if (iIndex!=LB_ERR)
			{
				DeleteString(iIndex);
			}
			break;
		case IDM_CHANGESPLITPOINT:
			iIndex=GetCurSel();
			if (iIndex!=LB_ERR)
			{
				GetText(iIndex,Buffer);
				aspd=new AddSplitPointDlg;
				aspd->SetSplitPos(atoi(Buffer));
				if (aspd->DoModal())
				{
					dwSplitPos=aspd->GetSplitPos();
					if (CheckSplitPoint(dwSplitPos))
					{
						DeleteString(iIndex);
						AddSplitPoint(dwSplitPos);
					}
				}
				delete aspd;
			}
			break;
	}

	return CListBox::OnCommand(wParam, lParam);
}

void CSplitPointList::SetVideoSource(VIDEOSOURCE* lpSource)
{
	lpVideoSource=lpSource;
}

