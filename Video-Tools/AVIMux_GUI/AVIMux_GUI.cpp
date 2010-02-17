// AVIMux_GUI.cpp : Legt das Klassenverhalten für die Anwendung fest.
//

#include "stdafx.h"
#include "AVIMux_GUI.h"
#include "AVIMux_GUIDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAVIMux_GUIApp

BEGIN_MESSAGE_MAP(CAVIMux_GUIApp, CWinApp)
	//{{AFX_MSG_MAP(CAVIMux_GUIApp)
		// HINWEIS - Hier werden Mapping-Makros vom Klassen-Assistenten eingefügt und entfernt.
		//    Innerhalb dieser generierten Quelltextabschnitte NICHTS VERÄNDERN!
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAVIMux_GUIApp Konstruktion

CAVIMux_GUIApp::CAVIMux_GUIApp()
{
	// ZU ERLEDIGEN: Hier Code zur Konstruktion einfügen
	// Alle wichtigen Initialisierungen in InitInstance platzieren
}

/////////////////////////////////////////////////////////////////////////////
// Das einzige CAVIMux_GUIApp-Objekt

CAVIMux_GUIApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CAVIMux_GUIApp Initialisierung

HINSTANCE GetInstance() {
	return theApp.m_hInstance;
}

bool error = false;
int nResponse;

void RunStuff(CAVIMux_GUIDlg& dlg)
{
	__try
	{
		nResponse = dlg.DoModal();
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		error = true;
	}
}

BOOL CAVIMux_GUIApp::InitInstance()
{
	CTraceFile* traceFile = GetApplicationTraceFile();
	traceFile->IncRefCounter();
	traceFile->SetTraceLevel(TRACE_LEVEL_INFO);
	traceFile->Trace(TRACE_LEVEL_INFO, _T("Application launched"), _T("Application launched"));

	AfxEnableControlContainer();

	// Standardinitialisierung
	// Wenn Sie diese Funktionen nicht nutzen und die Größe Ihrer fertigen 
	//  ausführbaren Datei reduzieren wollen, sollten Sie die nachfolgenden
	//  spezifischen Initialisierungsroutinen, die Sie nicht benötigen, entfernen.

#ifdef _AFXDLL
	Enable3dControls();			// Diese Funktion bei Verwendung von MFC in gemeinsam genutzten DLLs aufrufen
#else
	Enable3dControlsStatic();	// Diese Funktion bei statischen MFC-Anbindungen aufrufen
#endif

	CAVIMux_GUIDlg dlg;

	m_pMainWnd = &dlg;
	RunStuff(dlg);

	if (!error)
	{
		traceFile->Trace(TRACE_LEVEL_INFO, _T("Application terminated"), _T("Application terminated normally"));
	}
	else
	{
		traceFile->Trace(TRACE_LEVEL_FATAL, "Application terminated", "Application terminated irregularly");
		MessageBoxA(NULL, "Application terminated due to an exception.", "Fatal error", MB_OK | MB_ICONERROR);
	}


	if (nResponse == IDOK)
	{
		// ZU ERLEDIGEN: Fügen Sie hier Code ein, um ein Schließen des
		//  Dialogfelds über OK zu steuern
	}
	else if (nResponse == IDCANCEL)
	{
		// ZU ERLEDIGEN: Fügen Sie hier Code ein, um ein Schließen des
		//  Dialogfelds über "Abbrechen" zu steuern
	}

	traceFile->Close();
	delete traceFile;

	// Da das Dialogfeld geschlossen wurde, FALSE zurückliefern, so dass wir die
	//  Anwendung verlassen, anstatt das Nachrichtensystem der Anwendung zu starten.
	return FALSE;
}
