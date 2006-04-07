#if !defined(AFX_EBMLTREE_H__95AB7B5B_0E25_4C40_8D1D_8B46E7E34591__INCLUDED_)
#define AFX_EBMLTREE_H__95AB7B5B_0E25_4C40_8D1D_8B46E7E34591__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EBMLTree.h : Header-Datei
//

/////////////////////////////////////////////////////////////////////////////
// Fenster CEBMLTree 

#include "..\EBML.h"
#include "UnicodeTreeCtrl.h"

typedef struct
{
	__int64 iItemPosition;
	__int64 iRelPosition;
	int		iHeaderSize;
	EBMLElement* pElement;
} EBMLITEM_DESCRIPTOR;

class CEBMLTree : public CUnicodeTreeCtrl
{
// Konstruktion
private:
	int		iMode;
protected:
	void	virtual GetTextCallback(NMHDR* pNMHDR, LRESULT* pResult);

public:
	CEBMLTree();
	void SetMode(int _iMode);

// Attribute
public:

// Operationen
public:

// Überschreibungen
	// Vom Klassen-Assistenten generierte virtuelle Funktionsüberschreibungen
	//{{AFX_VIRTUAL(CEBMLTree)
	public:
	virtual void OnFinalRelease();
	//}}AFX_VIRTUAL

// Implementierung
public:
	virtual ~CEBMLTree();

	// Generierte Nachrichtenzuordnungsfunktionen
protected:
	//{{AFX_MSG(CEBMLTree)
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
	// Generierte OLE-Dispatch-Zuordnungsfunktionen
	//{{AFX_DISPATCH(CEBMLTree)
		// HINWEIS - Der Klassen-Assistent fügt hier Member-Funktionen ein und entfernt diese.
	//}}AFX_DISPATCH
	DECLARE_DISPATCH_MAP()
	DECLARE_INTERFACE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ fügt unmittelbar vor der vorhergehenden Zeile zusätzliche Deklarationen ein.

#endif // AFX_EBMLTREE_H__95AB7B5B_0E25_4C40_8D1D_8B46E7E34591__INCLUDED_
