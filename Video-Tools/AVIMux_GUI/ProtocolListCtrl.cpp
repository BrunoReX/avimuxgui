// ProtocolListCtrl.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "AVIMux_GUI.h"
#include "ProtocolListCtrl.h"
#include "Languages.h"
#include ".\protocollistctrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CProtocolListCtrl

CProtocolListCtrl::CProtocolListCtrl()
{
	EnableAutomation();
}

CProtocolListCtrl::~CProtocolListCtrl()
{
}

void CProtocolListCtrl::OnFinalRelease()
{
	// Nachdem die letzte Referenz auf ein Automatisierungsobjekt freigegeben wurde,
	// wird OnFinalRelease aufgerufen. Die Basisklasse l�scht das Objekt
	// automatisch. F�gen Sie zus�tzlichen Bereinigungscode f�r Ihr Objekt
	// hinzu, bevor Sie die Basisklasse aufrufen.

	CUnicodeListCtrl::OnFinalRelease();
}


BEGIN_MESSAGE_MAP(CProtocolListCtrl, CUnicodeListCtrl)
	//{{AFX_MSG_MAP(CProtocolListCtrl)
	ON_WM_RBUTTONUP()
	ON_WM_CONTEXTMENU()
	//}}AFX_MSG_MAP
	ON_WM_SIZE()
END_MESSAGE_MAP()

BEGIN_DISPATCH_MAP(CProtocolListCtrl, CUnicodeListCtrl)
	//{{AFX_DISPATCH_MAP(CProtocolListCtrl)
		// HINWEIS - Der Klassen-Assistent f�gt hier Zuordnungsmakros ein und entfernt diese.
	//}}AFX_DISPATCH_MAP
END_DISPATCH_MAP()

// Hinweis: Wir stellen Unterst�tzung f�r IID_IProtocolListCtrl zur Verf�gung, um typsicheres Binden
//  von VBA zu erm�glichen. Diese IID muss mit der GUID �bereinstimmen, die in der
//  Disp-Schnittstelle in der .ODL-Datei angegeben ist.

// {700D694F-3771-458E-A843-28918E5A2442}
static const IID IID_IProtocolListCtrl =
{ 0x700d694f, 0x3771, 0x458e, { 0xa8, 0x43, 0x28, 0x91, 0x8e, 0x5a, 0x24, 0x42 } };

BEGIN_INTERFACE_MAP(CProtocolListCtrl, CUnicodeListCtrl)
	INTERFACE_PART(CProtocolListCtrl, IID_IProtocolListCtrl, Dispatch)
END_INTERFACE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen f�r Nachrichten CProtocolListCtrl 

void CProtocolListCtrl::OnRButtonUp(UINT nFlags, CPoint point) 
{

	CUnicodeListCtrl::OnRButtonUp(nFlags, point);
}

void CProtocolListCtrl::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	// TODO: Code f�r die Behandlungsroutine f�r Nachrichten hier einf�gen
	CMenu*		cmPopupMenu;
	CString		cStr;
	bool		bItemPresent=false;

	cmPopupMenu=new CMenu;
	cmPopupMenu->CreatePopupMenu();

	cmPopupMenu->AppendMenu(MF_STRING,IDM_SAVEPROTOCOLAS,LoadString(STR_SAVEAS));
	bItemPresent=true;

	cmPopupMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON,point.x,point.y,this);
	delete cmPopupMenu;	
}

BOOL CProtocolListCtrl::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	CFileDialog* cfd;
	int			 count,i;
	char*		 text;
	char*		 time;
	FILE*		 file;
	// TODO: Speziellen Code hier einf�gen und/oder Basisklasse aufrufen
	switch (LOWORD(wParam))
	{
		case IDM_SAVEPROTOCOLAS:
			cfd = new CFileDialog (false,"txt");
			if (cfd->DoModal()==IDOK) {
				file=fopen(cfd->GetPathName(),"w");
				fprintf(file, "%c%c%c", 0xEF, 0xBB, 0xBF);

				count=GetItemCount();
				for (i=0;i<count;i++) {
					time=GetItemText(i,0);
					text=GetItemText(i,1);
					fprintf(file,"%15s %s\n",time,text);
					}
				fclose(file);
			};
			delete cfd;
			break;
	}
				
	return CUnicodeListCtrl::OnCommand(wParam, lParam);
}

void CProtocolListCtrl::OnSize(UINT nType, int cx, int cy)
{
	CUnicodeListCtrl::OnSize(nType, cx, cy);

	// TODO: F�gen Sie hier Ihren Meldungsbehandlungscode ein.
}
