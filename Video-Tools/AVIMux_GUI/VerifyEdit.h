#if !defined(AFX_VERIFYEDIT_H__E5441069_6C81_4C25_A1E7_698A512B17F7__INCLUDED_)
#define AFX_VERIFYEDIT_H__E5441069_6C81_4C25_A1E7_698A512B17F7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// VerifyEdit.h : Header-Datei
//

#include "UserDrawEdit.h"

/////////////////////////////////////////////////////////////////////////////
// Fenster CVerifyEdit 

class CVerifyEdit : public CUserDrawEdit
{
// Konstruktion
public:
	CVerifyEdit();

	COLORREF	GetTxtColor();

	int			virtual	VerifyElement();
	int			virtual GetColorIndex();



// Attribute
public:

// Operationen
public:

// Überschreibungen
	// Vom Klassen-Assistenten generierte virtuelle Funktionsüberschreibungen
	//{{AFX_VIRTUAL(CVerifyEdit)
	public:
	virtual void OnFinalRelease();
	//}}AFX_VIRTUAL

// Implementierung
public:
	virtual ~CVerifyEdit();

	// Generierte Nachrichtenzuordnungsfunktionen
protected:
	COLORREF	colors[256];

	//{{AFX_MSG(CVerifyEdit)
		// HINWEIS - Der Klassen-Assistent fügt hier Member-Funktionen ein und entfernt diese.
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
	// Generierte OLE-Dispatch-Zuordnungsfunktionen
	//{{AFX_DISPATCH(CVerifyEdit)
		// HINWEIS - Der Klassen-Assistent fügt hier Member-Funktionen ein und entfernt diese.
	//}}AFX_DISPATCH
	DECLARE_DISPATCH_MAP()
	DECLARE_INTERFACE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ fügt unmittelbar vor der vorhergehenden Zeile zusätzliche Deklarationen ein.

#endif // AFX_VERIFYEDIT_H__E5441069_6C81_4C25_A1E7_698A512B17F7__INCLUDED_
