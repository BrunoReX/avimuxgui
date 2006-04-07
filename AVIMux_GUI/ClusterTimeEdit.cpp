// ClusterTimeEdit.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "AVIMux_GUI.h"
#include "ClusterTimeEdit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CClusterTimeEdit

COLORREF CClusterTimeEdit::GetTxtColor()
{
	return (time>32767)?RGB(255,0,0):RGB(0,0,0);
}

void CClusterTimeEdit::SetTime(int iTime)
{
	time = iTime;
}

CClusterTimeEdit::CClusterTimeEdit()
{
	EnableAutomation();
}

CClusterTimeEdit::~CClusterTimeEdit()
{
}

void CClusterTimeEdit::OnFinalRelease()
{
	// Nachdem die letzte Referenz auf ein Automatisierungsobjekt freigegeben wurde,
	// wird OnFinalRelease aufgerufen. Die Basisklasse löscht das Objekt
	// automatisch. Fügen Sie zusätzlichen Bereinigungscode für Ihr Objekt
	// hinzu, bevor Sie die Basisklasse aufrufen.

	CUserDrawEdit::OnFinalRelease();
}


BEGIN_MESSAGE_MAP(CClusterTimeEdit, CUserDrawEdit)
	//{{AFX_MSG_MAP(CClusterTimeEdit)
		// HINWEIS - Der Klassen-Assistent fügt hier Zuordnungsmakros ein und entfernt diese.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BEGIN_DISPATCH_MAP(CClusterTimeEdit, CUserDrawEdit)
	//{{AFX_DISPATCH_MAP(CClusterTimeEdit)
		// HINWEIS - Der Klassen-Assistent fügt hier Zuordnungsmakros ein und entfernt diese.
	//}}AFX_DISPATCH_MAP
END_DISPATCH_MAP()

// Hinweis: Wir stellen Unterstützung für IID_IClusterTimeEdit zur Verfügung, um typsicheres Binden
//  von VBA zu ermöglichen. Diese IID muss mit der GUID übereinstimmen, die in der
//  Disp-Schnittstelle in der .ODL-Datei angegeben ist.

// {11377649-623D-4A81-933C-E1931DDDEBAC}
static const IID IID_IClusterTimeEdit =
{ 0x11377649, 0x623d, 0x4a81, { 0x93, 0x3c, 0xe1, 0x93, 0x1d, 0xdd, 0xeb, 0xac } };

BEGIN_INTERFACE_MAP(CClusterTimeEdit, CUserDrawEdit)
	INTERFACE_PART(CClusterTimeEdit, IID_IClusterTimeEdit, Dispatch)
END_INTERFACE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CClusterTimeEdit 
