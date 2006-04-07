#if !defined(AFX_CLUSTERTIMEEDIT_H__EAC57C48_D796_4138_AAD2_F579A0EA467B__INCLUDED_)
#define AFX_CLUSTERTIMEEDIT_H__EAC57C48_D796_4138_AAD2_F579A0EA467B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ClusterTimeEdit.h : Header-Datei
//

/////////////////////////////////////////////////////////////////////////////
// Fenster CClusterTimeEdit 

#include "UserDrawEdit.h"

class CClusterTimeEdit : public CUserDrawEdit
{
private:
	int		time;
// Konstruktion
public:
	CClusterTimeEdit();

// Attribute
public:
	COLORREF virtual GetTxtColor();
	void virtual SetTime(int iTime);
// Operationen
public:

// Überschreibungen
	// Vom Klassen-Assistenten generierte virtuelle Funktionsüberschreibungen
	//{{AFX_VIRTUAL(CClusterTimeEdit)
	public:
	virtual void OnFinalRelease();
	//}}AFX_VIRTUAL

// Implementierung
public:
	virtual ~CClusterTimeEdit();

	// Generierte Nachrichtenzuordnungsfunktionen
protected:
	//{{AFX_MSG(CClusterTimeEdit)
		// HINWEIS - Der Klassen-Assistent fügt hier Member-Funktionen ein und entfernt diese.
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
	// Generierte OLE-Dispatch-Zuordnungsfunktionen
	//{{AFX_DISPATCH(CClusterTimeEdit)
		// HINWEIS - Der Klassen-Assistent fügt hier Member-Funktionen ein und entfernt diese.
	//}}AFX_DISPATCH
	DECLARE_DISPATCH_MAP()
	DECLARE_INTERFACE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ fügt unmittelbar vor der vorhergehenden Zeile zusätzliche Deklarationen ein.

#endif // AFX_CLUSTERTIMEEDIT_H__EAC57C48_D796_4138_AAD2_F579A0EA467B__INCLUDED_
