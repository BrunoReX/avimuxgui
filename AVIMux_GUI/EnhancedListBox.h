#if !defined(AFX_ENHANCEDLISTBOX_H__B0C81196_7CF5_4CA7_9F4A_26FCDD140F58__INCLUDED_)
#define AFX_ENHANCEDLISTBOX_H__B0C81196_7CF5_4CA7_9F4A_26FCDD140F58__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EnhancedListBox.h : Header-Datei
//

#include "UnicodeBase.h"

/////////////////////////////////////////////////////////////////////////////
// Fenster CEnhancedListBox 

class CEnhancedListBox : public CListBox, public CUnicodeBase
{
// Konstruktion
public:
	CEnhancedListBox();

// Attribute
public:
	bool	bMovingAllowed;
	bool	bMButtonDown;
	int		iItemMButtonDown;
	CPoint	ptMButtonDown;

// Operationen
public:
	void virtual ItemDown(void);
	void virtual ItemUp(void);
	void virtual RedoNumbering();
	void	AllowMoving(bool bAllowMoving);
// Überschreibungen
	// Vom Klassen-Assistenten generierte virtuelle Funktionsüberschreibungen
	//{{AFX_VIRTUAL(CEnhancedListBox)
	public:
	virtual void OnFinalRelease();
	//}}AFX_VIRTUAL

// Implementierung
public:
	virtual ~CEnhancedListBox();

	// Generierte Nachrichtenzuordnungsfunktionen
protected:
	//{{AFX_MSG(CEnhancedListBox)
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMButtonUp(UINT nFlags, CPoint point);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
	// Generierte OLE-Dispatch-Zuordnungsfunktionen
	//{{AFX_DISPATCH(CEnhancedListBox)
		// HINWEIS - Der Klassen-Assistent fügt hier Member-Funktionen ein und entfernt diese.
	//}}AFX_DISPATCH
	DECLARE_DISPATCH_MAP()
	DECLARE_INTERFACE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ fügt unmittelbar vor der vorhergehenden Zeile zusätzliche Deklarationen ein.

#endif // AFX_ENHANCEDLISTBOX_H__B0C81196_7CF5_4CA7_9F4A_26FCDD140F58__INCLUDED_
