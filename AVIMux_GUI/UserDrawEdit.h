#if !defined(AFX_USERDRAWEDIT_H__785B397F_8B73_496B_8393_94AEA2EC65F3__INCLUDED_)
#define AFX_USERDRAWEDIT_H__785B397F_8B73_496B_8393_94AEA2EC65F3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// UserDrawEdit.h : Header-Datei
//

/////////////////////////////////////////////////////////////////////////////
// Fenster CUserDrawEdit 

class CUserDrawEdit : public CEdit
{
// Konstruktion
public:
	CUserDrawEdit();

// Attribute
public:
	COLORREF virtual GetTxtColor();

// Operationen
public:

// Überschreibungen
	// Vom Klassen-Assistenten generierte virtuelle Funktionsüberschreibungen
	//{{AFX_VIRTUAL(CUserDrawEdit)
	public:
	virtual void OnFinalRelease();
	//}}AFX_VIRTUAL

// Implementierung
public:
	virtual ~CUserDrawEdit();

	// Generierte Nachrichtenzuordnungsfunktionen
protected:
	//{{AFX_MSG(CUserDrawEdit)
	afx_msg void OnPaint();
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
	// Generierte OLE-Dispatch-Zuordnungsfunktionen
	//{{AFX_DISPATCH(CUserDrawEdit)
		// HINWEIS - Der Klassen-Assistent fügt hier Member-Funktionen ein und entfernt diese.
	//}}AFX_DISPATCH
	DECLARE_DISPATCH_MAP()
	DECLARE_INTERFACE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ fügt unmittelbar vor der vorhergehenden Zeile zusätzliche Deklarationen ein.

#endif // AFX_USERDRAWEDIT_H__785B397F_8B73_496B_8393_94AEA2EC65F3__INCLUDED_
