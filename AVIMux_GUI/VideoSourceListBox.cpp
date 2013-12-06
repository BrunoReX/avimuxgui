// VideoSourceListBox.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "AVIMux_GUI.h"
#include "VideoInformationDlg.h"
#include "VideoSourceListBox.h"
#include "VideoSource.h"
#include "Languages.h"

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
	// wird OnFinalRelease aufgerufen. Die Basisklasse l�scht das Objekt
	// automatisch. F�gen Sie zus�tzlichen Bereinigungscode f�r Ihr Objekt
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
		// HINWEIS - Der Klassen-Assistent f�gt hier Zuordnungsmakros ein und entfernt diese.
	//}}AFX_DISPATCH_MAP
END_DISPATCH_MAP()

// Hinweis: Wir stellen Unterst�tzung f�r IID_IVideoSourceListBox zur Verf�gung, um typsicheres Binden
//  von VBA zu erm�glichen. Diese IID muss mit der GUID �bereinstimmen, die in der
//  Disp-Schnittstelle in der .ODL-Datei angegeben ist.

// {3A30BEE7-54FF-4809-866A-B4F58D6278C3}
static const IID IID_IVideoSourceListBox =
{ 0x3a30bee7, 0x54ff, 0x4809, { 0x86, 0x6a, 0xb4, 0xf5, 0x8d, 0x62, 0x78, 0xc3 } };

BEGIN_INTERFACE_MAP(CVideoSourceListBox, CListBox)
	INTERFACE_PART(CVideoSourceListBox, IID_IVideoSourceListBox, Dispatch)
END_INTERFACE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen f�r Nachrichten CVideoSourceListBox 

void CVideoSourceListBox::OnRButtonUp(UINT nFlags, CPoint point) 
{
	// TODO: Code f�r die Behandlungsroutine f�r Nachrichten hier einf�gen und/oder Standard aufrufen
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

BOOL CVideoSourceListBox::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	// TODO: Speziellen Code hier einf�gen und/oder Basisklasse aufrufen
	CVideoInformationDlg*	lpvidlg;
	int						iIndex;
	VIDEOSOURCE*			lpVS;

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
	}
	
	return CEnhancedListBox::OnCommand(wParam, lParam);
}
