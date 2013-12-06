// AVIMux_GUI.cpp : Legt das Klassenverhalten f�r die Anwendung fest.
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
		// HINWEIS - Hier werden Mapping-Makros vom Klassen-Assistenten eingef�gt und entfernt.
		//    Innerhalb dieser generierten Quelltextabschnitte NICHTS VER�NDERN!
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAVIMux_GUIApp Konstruktion

CAVIMux_GUIApp::CAVIMux_GUIApp()
{
	// ZU ERLEDIGEN: Hier Code zur Konstruktion einf�gen
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
	// Wenn Sie diese Funktionen nicht nutzen und die Gr��e Ihrer fertigen 
	//  ausf�hrbaren Datei reduzieren wollen, sollten Sie die nachfolgenden
	//  spezifischen Initialisierungsroutinen, die Sie nicht ben�tigen, entfernen.

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
		// ZU ERLEDIGEN: F�gen Sie hier Code ein, um ein Schlie�en des
		//  Dialogfelds �ber OK zu steuern
	}
	else if (nResponse == IDCANCEL)
	{
		// ZU ERLEDIGEN: F�gen Sie hier Code ein, um ein Schlie�en des
		//  Dialogfelds �ber "Abbrechen" zu steuern
	}

	traceFile->Close();
	delete traceFile;

	// Da das Dialogfeld geschlossen wurde, FALSE zur�ckliefern, so dass wir die
	//  Anwendung verlassen, anstatt das Nachrichtensystem der Anwendung zu starten.
	return FALSE;
}
