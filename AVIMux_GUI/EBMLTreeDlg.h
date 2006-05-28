#if !defined(AFX_EBMLTREEDLG_H__19405BDE_1D21_4DC3_926E_9DF9F58D3F01__INCLUDED_)
#define AFX_EBMLTREEDLG_H__19405BDE_1D21_4DC3_926E_9DF9F58D3F01__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EBMLTreeDlg.h : Header-Datei
//

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CEBMLTreeDlg 

#include "..\basestreams.h"
#include "..\ebml.h"
#include "..\matroska.h"
#include "EBMLTree.h"
#include "ResizeableDialog.h"
#include "afxwin.h"
#include "HexViewListBox.h"
void AddChildren(CTreeCtrl* tree, HTREEITEM hParent, EBMLElement* eParent); 

class CEBMLTreeDlg : public CResizeableDialog
{
private:
	STREAM*		source;
	bool		bDoClose;
	HANDLE		hSem_close;
	int			font_size;
	EBML_Matroska* e_matroska;
	void CleanChildren(HTREEITEM hParent);
	void RecreateTreeFont();

	int			fThreadsafe;
// Konstruktion
public:
	CEBMLTreeDlg(CWnd* pParent = NULL);   // Standardkonstruktor
	void	SetSource(STREAM*	lpSource);
	STREAM* GetSource(void);

// Dialogfelddaten
	//{{AFX_DATA(CEBMLTreeDlg)
	enum { IDD = IDD_EBMLTREE_DLG };
	CButton	m_OK;
	CEBMLTree	m_EBMLTree;
	CButton	    m_Absolute;
	CButton     m_Relative;
	//}}AFX_DATA


// Überschreibungen
	// Vom Klassen-Assistenten generierte virtuelle Funktionsüberschreibungen
	//{{AFX_VIRTUAL(CEBMLTreeDlg)
	public:
	virtual void OnFinalRelease();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV-Unterstützung
	//}}AFX_VIRTUAL

// Implementierung
protected:

	// Generierte Nachrichtenzuordnungsfunktionen
	//{{AFX_MSG(CEBMLTreeDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangedTree1(NMHDR* pNMHDR, LRESULT* pResult);
	virtual void OnOK();
	afx_msg void OnAbsolute();
	afx_msg void OnRelative();
	afx_msg void OnFullexpand();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	// Generierte OLE-Dispatch-Zuordnungsfunktionen
	//{{AFX_DISPATCH(CEBMLTreeDlg)
		// HINWEIS - Der Klassen-Assistent fügt hier Member-Funktionen ein und entfernt diese.
	//}}AFX_DISPATCH
	DECLARE_DISPATCH_MAP()
	DECLARE_INTERFACE_MAP()
public:
	afx_msg void OnBnClickedFontLarger();
	afx_msg void OnBnClickedFontSmaller();
//	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedOk();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	CHexViewListBox m_HexEdit;
	afx_msg void OnBnClickedSaveTree();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ fügt unmittelbar vor der vorhergehenden Zeile zusätzliche Deklarationen ein.

#endif // AFX_EBMLTREEDLG_H__19405BDE_1D21_4DC3_926E_9DF9F58D3F01__INCLUDED_
