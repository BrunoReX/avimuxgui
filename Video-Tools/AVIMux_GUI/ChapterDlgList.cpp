// ChapterDlgList.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "AVIMux_GUI.h"
#include "ChapterDlgList.h"
#include "UnicodeListControl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CChapterDlgList

CChapterDlgList::CChapterDlgList()
{
	EnableAutomation();
}

CChapterDlgList::~CChapterDlgList()
{
}

void CChapterDlgList::OnFinalRelease()
{
	// Nachdem die letzte Referenz auf ein Automatisierungsobjekt freigegeben wurde,
	// wird OnFinalRelease aufgerufen. Die Basisklasse l�scht das Objekt
	// automatisch. F�gen Sie zus�tzlichen Bereinigungscode f�r Ihr Objekt
	// hinzu, bevor Sie die Basisklasse aufrufen.

	CUnicodeListCtrl::OnFinalRelease();
}


BEGIN_MESSAGE_MAP(CChapterDlgList, CUnicodeListCtrl)
	//{{AFX_MSG_MAP(CChapterDlgList)
		// HINWEIS - Der Klassen-Assistent f�gt hier Zuordnungsmakros ein und entfernt diese.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BEGIN_DISPATCH_MAP(CChapterDlgList, CUnicodeListCtrl)
	//{{AFX_DISPATCH_MAP(CChapterDlgList)
		// HINWEIS - Der Klassen-Assistent f�gt hier Zuordnungsmakros ein und entfernt diese.
	//}}AFX_DISPATCH_MAP
END_DISPATCH_MAP()

// Hinweis: Wir stellen Unterst�tzung f�r IID_IChapterDlgList zur Verf�gung, um typsicheres Binden
//  von VBA zu erm�glichen. Diese IID muss mit der GUID �bereinstimmen, die in der
//  Disp-Schnittstelle in der .ODL-Datei angegeben ist.

// {84F5D783-13D6-46E7-97AD-3E987D2E77B7}
static const IID IID_IChapterDlgList =
{ 0x84f5d783, 0x13d6, 0x46e7, { 0x97, 0xad, 0x3e, 0x98, 0x7d, 0x2e, 0x77, 0xb7 } };

BEGIN_INTERFACE_MAP(CChapterDlgList, CUnicodeListCtrl)
	INTERFACE_PART(CChapterDlgList, IID_IChapterDlgList, Dispatch)
END_INTERFACE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen f�r Nachrichten CChapterDlgList 
