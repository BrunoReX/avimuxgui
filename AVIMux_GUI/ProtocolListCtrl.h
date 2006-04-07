#if !defined(AFX_PROTOCOLLISTCTRL_H__A174523A_0EC1_4434_91EF_66120692226D__INCLUDED_)
#define AFX_PROTOCOLLISTCTRL_H__A174523A_0EC1_4434_91EF_66120692226D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ProtocolListCtrl.h : Header-Datei
//

#include "UnicodeListControl.h"

/////////////////////////////////////////////////////////////////////////////
// Fenster CProtocolListCtrl 


class CProtocolListCtrl : public CUnicodeListCtrl
{
// Konstruktion
public:
	CProtocolListCtrl();
	
// Attribute
public:

// Operationen
public:

// Überschreibungen
	// Vom Klassen-Assistenten generierte virtuelle Funktionsüberschreibungen
	//{{AFX_VIRTUAL(CProtocolListCtrl)
	public:
	virtual void OnFinalRelease();
	protected:
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementierung
public:
	virtual ~CProtocolListCtrl();

	// Generierte Nachrichtenzuordnungsfunktionen
protected:
	//{{AFX_MSG(CProtocolListCtrl)
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
	// Generierte OLE-Dispatch-Zuordnungsfunktionen
	//{{AFX_DISPATCH(CProtocolListCtrl)
		// HINWEIS - Der Klassen-Assistent fügt hier Member-Funktionen ein und entfernt diese.
	//}}AFX_DISPATCH
	DECLARE_DISPATCH_MAP()
	DECLARE_INTERFACE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ fügt unmittelbar vor der vorhergehenden Zeile zusätzliche Deklarationen ein.

#endif // AFX_PROTOCOLLISTCTRL_H__A174523A_0EC1_4434_91EF_66120692226D__INCLUDED_
