#if !defined(AFX_AC3FRAMECOUNTEDIT_H__45FEC79F_5169_4B55_93A0_CB550499FF1B__INCLUDED_)
#define AFX_AC3FRAMECOUNTEDIT_H__45FEC79F_5169_4B55_93A0_CB550499FF1B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AC3FrameCountEdit.h : Header-Datei
//

/////////////////////////////////////////////////////////////////////////////
// Fenster CAC3FrameCountEdit 

#include "UserDrawEdit.h"

class CAC3FrameCountEdit : public CUserDrawEdit
{
private:
	int	count;
// Konstruktion
public:
	CAC3FrameCountEdit();

// Attribute
public:
	void	SetAC3FrameCount(int iCount);
	COLORREF virtual GetTxtColor();
// Operationen
public:

// Überschreibungen
	// Vom Klassen-Assistenten generierte virtuelle Funktionsüberschreibungen
	//{{AFX_VIRTUAL(CAC3FrameCountEdit)
	public:
	virtual void OnFinalRelease();
	//}}AFX_VIRTUAL

// Implementierung
public:
	virtual ~CAC3FrameCountEdit();
	

	// Generierte Nachrichtenzuordnungsfunktionen
protected:
	//{{AFX_MSG(CAC3FrameCountEdit)
		// HINWEIS - Der Klassen-Assistent fügt hier Member-Funktionen ein und entfernt diese.
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
	// Generierte OLE-Dispatch-Zuordnungsfunktionen
	//{{AFX_DISPATCH(CAC3FrameCountEdit)
		// HINWEIS - Der Klassen-Assistent fügt hier Member-Funktionen ein und entfernt diese.
	//}}AFX_DISPATCH
	DECLARE_DISPATCH_MAP()
	DECLARE_INTERFACE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ fügt unmittelbar vor der vorhergehenden Zeile zusätzliche Deklarationen ein.

#endif // AFX_AC3FRAMECOUNTEDIT_H__45FEC79F_5169_4B55_93A0_CB550499FF1B__INCLUDED_
