#if !defined(AFX_UNICODETREECTRL_H__38500A52_D18D_47E7_BCB5_3ED2A41F5FFC__INCLUDED_)
#define AFX_UNICODETREECTRL_H__38500A52_D18D_47E7_BCB5_3ED2A41F5FFC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// UnicodeTreeCtrl.h : Header-Datei
//

#include "UnicodeBase.h"
#include "TreeItemDeleter.h"

/////////////////////////////////////////////////////////////////////////////
// Fenster CUnicodeTreeCtrl 

typedef struct
{
	char*	cText;
	//std::string text;
	bool	bAllocated;
	DWORD_PTR dwUserData;
} UNICODETREEITEM_DATA;


class CUnicodeTreeCtrl : public CTreeCtrl, public CUnicodeBase
{
private:
// Konstruktion
protected:
	int		mouse_x, mouse_y;
	bool	b_rdown;
	void	virtual	GetTextCallback(NMHDR* pNMHDR, LRESULT* pResult);
	int		RenderItem(HTREEITEM hItem, std::string& dest, int iDepth);
public:
	CUnicodeTreeCtrl();
	void	InitUnicode();
	HTREEITEM InsertItem(LPTVINSERTSTRUCTA lpInsertStruct);
	HTREEITEM InsertItem(LPTVINSERTSTRUCTW lpInsertStruct);
	HTREEITEM GetTopMostParentItem(HTREEITEM hItem);
	
	//bool   
//	bool	PrepareDeleteItem(HTREEITEM hItem, TreeItemDeleter<UNICODETREEITEM_DATA>& deleter);

protected:
	typedef DWORD_PTR (CUnicodeTreeCtrl::*fnGetItemData)(HTREEITEM hItem) const;

public:

#ifdef NOT_DEFINED
	template<class T>
	bool PrepareDeleteItem(HTREEITEM hItem, TreeItemDeleter<T>& deleter, fnGetItemData getItemData)
	{
		HTREEITEM hChild; 

		//CTreeCtrl::SetItemData(hItem, NULL);
		if (hChild = GetChildItem(hItem)) {
			PrepareDeleteAllItems(hChild, deleter, getItemData);
		}

		T* data = (T*)(this->*getItemData)(hItem); //CTreeCtrl::GetItemData(hItem);
		deleter += data;

	/*	CTreeCtrl::DeleteItem(hItem);

		if (data && data->bAllocated) {
			delete[] data->cText;
			data->bAllocated = 0;
			data->cText = NULL;
		}

		if (data) {
			delete data;
			data = NULL;
		}
	*/
		return true;
	}

	template<class T>
	bool PrepareDeleteAllItems(HTREEITEM hRoot, TreeItemDeleter<T>& deleter, fnGetItemData getItemData)
	{
		if (!hRoot) {
			hRoot = GetRootItem();
			if (!hRoot)
				return false;
		}

		HTREEITEM hCurrent, hNext;

		hCurrent = hRoot;

		do {
			hNext = GetNextSiblingItem(hCurrent);
			PrepareDeleteItem(hCurrent, deleter, getItemData);
			hCurrent = hNext;		
		} while (hNext);

		return true;
	}
#endif
//	bool	PrepareDeleteAllItems(HTREEITEM hRoot, TreeItemDeleter<UNICODETREEITEM_DATA>& deleter);


	bool	DeleteItem(HTREEITEM hItem);
	bool    DeleteAllItems(HTREEITEM hRoot = NULL);

	void	SetItemData(HTREEITEM hItem, DWORD dwData);
	DWORD_PTR GetItemData(HTREEITEM) const;
	char*	GetItemText(HTREEITEM);
	void	SetItem(TVITEMA* pItem);
	void	SetItem(TVITEMW* pItem);
	bool	SetItemText(HTREEITEM hItem, LPCTSTR lpszItem);
	void	ShowItemCheckBox(HTREEITEM, bool);
	int		GetMouseX();
	int		GetMouseY();
	int		GetRButtonDown();

	// Renders the entire tree into a utf8 encoded string
	int		Render2Buffer(std::string& dest);

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
