#if !defined(AFX_UNICODETREECTRL_H__38500A52_D18D_47E7_BCB5_3ED2A41F5FFC__INCLUDED_)
#define AFX_UNICODETREECTRL_H__38500A52_D18D_47E7_BCB5_3ED2A41F5FFC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// UnicodeTreeCtrl.h : Header-Datei
//

#include "UnicodeBase.h"

/////////////////////////////////////////////////////////////////////////////
// Fenster CUnicodeTreeCtrl 

typedef struct
{
	char*	cText;
	bool	bAllocated;
	DWORD	dwUserData;
} UNICODETREEITEM_DATA;

class CUnicodeTreeCtrl : public CTreeCtrl, public CUnicodeBase
{
private:
// Konstruktion
protected:
	int		mouse_x, mouse_y;
	bool	b_rdown;
	void	virtual	GetTextCallback(NMHDR* pNMHDR, LRESULT* pResult);
	int		RenderItem(HTREEITEM hItem, char* cDest, int iDepth);
public:
	CUnicodeTreeCtrl();
	void	InitUnicode();
	HTREEITEM InsertItem(LPTVINSERTSTRUCT lpInsertStruct);
	bool	DeleteItem(HTREEITEM hItem);
	bool	DeleteAllItems(HTREEITEM hRoot = NULL);
	void	SetItemData(HTREEITEM hItem, DWORD dwData);
	DWORD	GetItemData(HTREEITEM);
	char*	GetItemText(HTREEITEM);
	void	SetItem(TVITEM* pItem);
	bool	SetItemText(HTREEITEM hItem, LPCTSTR lpszItem);
	void	ShowItemCheckBox(HTREEITEM, bool);
	int		GetMouseX();
	int		GetMouseY();
	int		GetRButtonDown();
	int		Render2Buffer(char* cDest);

// Attribute
public:

// Operationen
public:

// Überschreibungen
	// Vom Klassen-Assistenten generierte virtuelle Funktionsüberschreibungen
	//{{AFX_VIRTUAL(CUnicodeTreeCtrl)
	public:
	virtual void OnFinalRelease();
	protected:
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementierung
public:
	virtual ~CUnicodeTreeCtrl();
	afx_msg void OnGetdispinfo(NMHDR* pNMHDR, LRESULT* pResult);

	// Generierte Nachrichtenzuordnungsfunktionen
protected:
	//{{AFX_MSG(CUnicodeTreeCtrl)
	afx_msg void OnKeydown(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnReturn(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDestroy();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
	// Generierte OLE-Dispatch-Zuordnungsfunktionen
	//{{AFX_DISPATCH(CUnicodeTreeCtrl)
		// HINWEIS - Der Klassen-Assistent fügt hier Member-Funktionen ein und entfernt diese.
	//}}AFX_DISPATCH
	DECLARE_DISPATCH_MAP()
	DECLARE_INTERFACE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ fügt unmittelbar vor der vorhergehenden Zeile zusätzliche Deklarationen ein.

#endif // AFX_UNICODETREECTRL_H__38500A52_D18D_47E7_BCB5_3ED2A41F5FFC__INCLUDED_
