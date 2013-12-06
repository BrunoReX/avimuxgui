// MKVAC3FrameCountEdit.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "AVIMux_GUI.h"
#include "MKVAC3FrameCountEdit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMKVAC3FrameCountEdit

CMKVAC3FrameCountEdit::CMKVAC3FrameCountEdit()
{
	EnableAutomation();
}

CMKVAC3FrameCountEdit::~CMKVAC3FrameCountEdit()
{
}

void CMKVAC3FrameCountEdit::SetFrameCount(int i)
{
	iCount = i;
}

COLORREF CMKVAC3FrameCountEdit::GetTxtColor()
{
	return (iCount==1)?RGB(0,0,0):RGB(255,0,0);
}


void CMKVAC3FrameCountEdit::OnFinalRelease()
{
	// Nachdem die letzte Referenz auf ein Automatisierungsobjekt freigegeben wurde,
	// wird OnFinalRelease aufgerufen. Die Basisklasse löscht das Objekt
	// automatisch. Fügen Sie zusätzlichen Bereinigungscode für Ihr Objekt
	// hinzu, bevor Sie die Basisklasse aufrufen.

	CUserDrawEdit::OnFinalRelease();
}


BEGIN_MESSAGE_MAP(CMKVAC3FrameCountEdit, CUserDrawEdit)
	//{{AFX_MSG_MAP(CMKVAC3FrameCountEdit)
		// HINWEIS - Der Klassen-Assistent fügt hier Zuordnungsmakros ein und entfernt diese.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BEGIN_DISPATCH_MAP(CMKVAC3FrameCountEdit, CUserDrawEdit)
	//{{AFX_DISPATCH_MAP(CMKVAC3FrameCountEdit)
		// HINWEIS - Der Klassen-Assistent fügt hier Zuordnungsmakros ein und entfernt diese.
	//}}AFX_DISPATCH_MAP
END_DISPATCH_MAP()

// Hinweis: Wir stellen Unterstützung für IID_IMKVAC3FrameCountEdit zur Verfügung, um typsicheres Binden
//  von VBA zu ermöglichen. Diese IID muss mit der GUID übereinstimmen, die in der
//  Disp-Schnittstelle in der .ODL-Datei angegeben ist.

// {152ED267-62E4-41CE-AFDF-7DF698D599CD}
static const IID IID_IMKVAC3FrameCountEdit =
{ 0x152ed267, 0x62e4, 0x41ce, { 0xaf, 0xdf, 0x7d, 0xf6, 0x98, 0xd5, 0x99, 0xcd } };

BEGIN_INTERFACE_MAP(CMKVAC3FrameCountEdit, CUserDrawEdit)
	INTERFACE_PART(CMKVAC3FrameCountEdit, IID_IMKVAC3FrameCountEdit, Dispatch)
END_INTERFACE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CMKVAC3FrameCountEdit 
