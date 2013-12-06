// AC3FrameCountEdit.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "AVIMux_GUI.h"
#include "AC3FrameCountEdit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAC3FrameCountEdit

CAC3FrameCountEdit::CAC3FrameCountEdit()
{
	EnableAutomation();
}

CAC3FrameCountEdit::~CAC3FrameCountEdit()
{
}

void CAC3FrameCountEdit::SetAC3FrameCount(int iCount)
{
	count = iCount;
}

COLORREF CAC3FrameCountEdit::GetTxtColor()
{
	return (count > 9 || count < 2)?RGB(255,0,0):(count>5)?RGB(192,0,0):RGB(0,0,0);
}

void CAC3FrameCountEdit::OnFinalRelease()
{
	// Nachdem die letzte Referenz auf ein Automatisierungsobjekt freigegeben wurde,
	// wird OnFinalRelease aufgerufen. Die Basisklasse löscht das Objekt
	// automatisch. Fügen Sie zusätzlichen Bereinigungscode für Ihr Objekt
	// hinzu, bevor Sie die Basisklasse aufrufen.

	CUserDrawEdit::OnFinalRelease();
}


BEGIN_MESSAGE_MAP(CAC3FrameCountEdit, CUserDrawEdit)
	//{{AFX_MSG_MAP(CAC3FrameCountEdit)
		// HINWEIS - Der Klassen-Assistent fügt hier Zuordnungsmakros ein und entfernt diese.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BEGIN_DISPATCH_MAP(CAC3FrameCountEdit, CUserDrawEdit)
	//{{AFX_DISPATCH_MAP(CAC3FrameCountEdit)
		// HINWEIS - Der Klassen-Assistent fügt hier Zuordnungsmakros ein und entfernt diese.
	//}}AFX_DISPATCH_MAP
END_DISPATCH_MAP()

// Hinweis: Wir stellen Unterstützung für IID_IAC3FrameCountEdit zur Verfügung, um typsicheres Binden
//  von VBA zu ermöglichen. Diese IID muss mit der GUID übereinstimmen, die in der
//  Disp-Schnittstelle in der .ODL-Datei angegeben ist.

// {F1603F8D-CB9E-4D55-9974-98271F88CBD9}
static const IID IID_IAC3FrameCountEdit =
{ 0xf1603f8d, 0xcb9e, 0x4d55, { 0x99, 0x74, 0x98, 0x27, 0x1f, 0x88, 0xcb, 0xd9 } };

BEGIN_INTERFACE_MAP(CAC3FrameCountEdit, CUserDrawEdit)
	INTERFACE_PART(CAC3FrameCountEdit, IID_IAC3FrameCountEdit, Dispatch)
END_INTERFACE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CAC3FrameCountEdit 
