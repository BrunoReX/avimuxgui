#if !defined(AFX_UNICODELISTCONTROL_H__55B72F65_DCC2_44B5_B02B_617974B4C4FA__INCLUDED_)
#define AFX_UNICODELISTCONTROL_H__55B72F65_DCC2_44B5_B02B_617974B4C4FA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// UnicodeListControl.h : Header-Datei
//

/////////////////////////////////////////////////////////////////////////////
// Fenster CUnicodeListControl 

#include "UnicodeBase.h"


class CUnicodeListCtrl : public CListCtrl, public CUnicodeBase
{
private:
	
// Konstruktion
public:
	CUnicodeListCtrl();
	void	InitUnicode();

	int		InsertItem(LV_ITEM* pItem );
	BOOL	DeleteItem( int nItem );
	BOOL	DeleteAllItems();
	BOOL	SetItemText(int nItem, int nSubItem, LPTSTR lpszText);
	void    GetItemText(int nItem, int nSubItem, char* cDest, int imax);
	char*   GetItemText(int nItem, int nSubItem);
	void	virtual GetTextCallback(NMHDR* pNMHDR, LRESULT* pResult);


// Attribute
public:

// Operationen
public:

// Überschreibungen
	// Vom Klassen-Assistenten generierte virtuelle Funktionsüberschreibungen
	//{{AFX_VIRTUAL(CUnicodeListControl)
	public:
	virtual void OnFinalRelease();
	virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);
	//}}AFX_VIRTUAL

// Implementierung
public:
	virtual ~CUnicodeListCtrl();

	// Generierte Nachrichtenzuordnungsfunktionen
protected:
	//{{AFX_MSG(CUnicodeListControl)
	afx_msg void OnGetdispinfo(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDestroy();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
	// Generierte OLE-Dispatch-Zuordnungsfunktionen
	//{{AFX_DISPATCH(CUnicodeListControl)
		// HINWEIS - Der Klassen-Assistent fügt hier Member-Funktionen ein und entfernt diese.
	//}}AFX_DISPATCH
	DECLARE_DISPATCH_MAP()
	DECLARE_INTERFACE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ fügt unmittelbar vor der vorhergehenden Zeile zusätzliche Deklarationen ein.

#endif // AFX_UNICODELISTCONTROL_H__55B72F65_DCC2_44B5_B02B_617974B4C4FA__INCLUDED_
