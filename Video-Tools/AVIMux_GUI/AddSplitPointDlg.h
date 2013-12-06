#if !defined(AFX_ADDSPLITPOINTDLG_H__0CBC624C_5F59_47D4_BECD_F3B54FB140DB__INCLUDED_)
#define AFX_ADDSPLITPOINTDLG_H__0CBC624C_5F59_47D4_BECD_F3B54FB140DB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AddSplitPointDlg.h : Header-Datei
//

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld AddSplitPointDlg 

class AddSplitPointDlg : public CDialog
{
private:
	DWORD	dwSplitPos;
	// Konstruktion
public:
	AddSplitPointDlg(CWnd* pParent = NULL);   // Standardkonstruktor
	void SetSplitPos(DWORD dwSplitPos);
	char		Buffer[50];
	DWORD		GetSplitPos();
// Dialogfelddaten
	//{{AFX_DATA(AddSplitPointDlg)
	enum { IDD = IDD_ADDSPLITPOINTDLG };
		// HINWEIS: Der Klassen-Assistent fügt hier Datenelemente ein
	//}}AFX_DATA


// Überschreibungen
	// Vom Klassen-Assistenten generierte virtuelle Funktionsüberschreibungen
	//{{AFX_VIRTUAL(AddSplitPointDlg)
	public:
	virtual void OnFinalRelease();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV-Unterstützung
	//}}AFX_VIRTUAL

// Implementierung
protected:

	// Generierte Nachrichtenzuordnungsfunktionen
	//{{AFX_MSG(AddSplitPointDlg)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnCancelMode();
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	// Generierte OLE-Dispatch-Zuordnungsfunktionen
	//{{AFX_DISPATCH(AddSplitPointDlg)
		// HINWEIS - Der Klassen-Assistent fügt hier Member-Funktionen ein und entfernt diese.
	//}}AFX_DISPATCH
	DECLARE_DISPATCH_MAP()
	DECLARE_INTERFACE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ fügt unmittelbar vor der vorhergehenden Zeile zusätzliche Deklarationen ein.

#endif // AFX_ADDSPLITPOINTDLG_H__0CBC624C_5F59_47D4_BECD_F3B54FB140DB__INCLUDED_
