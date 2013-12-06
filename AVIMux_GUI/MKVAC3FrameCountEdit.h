#if !defined(AFX_MKVAC3FRAMECOUNTEDIT_H__766D3D55_7087_440A_BC9F_4A4258B283B3__INCLUDED_)
#define AFX_MKVAC3FRAMECOUNTEDIT_H__766D3D55_7087_440A_BC9F_4A4258B283B3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MKVAC3FrameCountEdit.h : Header-Datei
//

#include "userdrawedit.h"

/////////////////////////////////////////////////////////////////////////////
// Fenster CMKVAC3FrameCountEdit 

class CMKVAC3FrameCountEdit : public CUserDrawEdit
{
private:
	int iCount;
// Konstruktion
public:
	CMKVAC3FrameCountEdit();

// Attribute
public:

// Operationen
public:
	void SetFrameCount(int i);
	COLORREF GetTxtColor();

// �berschreibungen
	// Vom Klassen-Assistenten generierte virtuelle Funktions�berschreibungen
	//{{AFX_VIRTUAL(CMKVAC3FrameCountEdit)
	public:
	virtual void OnFinalRelease();
	//}}AFX_VIRTUAL

// Implementierung
public:
	virtual ~CMKVAC3FrameCountEdit();

	// Generierte Nachrichtenzuordnungsfunktionen
protected:
	//{{AFX_MSG(CMKVAC3FrameCountEdit)
		// HINWEIS - Der Klassen-Assistent f�gt hier Member-Funktionen ein und entfernt diese.
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
	// Generierte OLE-Dispatch-Zuordnungsfunktionen
	//{{AFX_DISPATCH(CMKVAC3FrameCountEdit)
		// HINWEIS - Der Klassen-Assistent f�gt hier Member-Funktionen ein und entfernt diese.
	//}}AFX_DISPATCH
	DECLARE_DISPATCH_MAP()
	DECLARE_INTERFACE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ f�gt unmittelbar vor der vorhergehenden Zeile zus�tzliche Deklarationen ein.

#endif // AFX_MKVAC3FRAMECOUNTEDIT_H__766D3D55_7087_440A_BC9F_4A4258B283B3__INCLUDED_
