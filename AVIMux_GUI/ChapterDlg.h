#if !defined(AFX_CHAPTERDLG_H__6A384E54_BF30_4BEB_AE02_39E2CA7BDD22__INCLUDED_)
#define AFX_CHAPTERDLG_H__6A384E54_BF30_4BEB_AE02_39E2CA7BDD22__INCLUDED_

#include "..\chapters.h"
#include "ChapterDlgTree.h"
#include "UnicodeListControl.h"
#include "ChapterDlgList.h"
#include "resource.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ChapterDlg.h : Header-Datei
//

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CChapterDlg 

int RenderChapters2File(FILE* f, CChapters* c);
typedef struct
{
	CChapters*	c;
	int			iIndex;
	char*		cText;
} CHAPTER_ENTRY;

class CChapterDlg : public CDialog
{
private:
	CChapters*	c;
	int		iSelectedChapterLanguageEntry;
	HTREEITEM hSelectedChapter;
	void	UpdateChapterDisplayLngEdit(int iItem);
// Konstruktion
public:
	CChapterDlg(CWnd* pParent = NULL);   // Standardkonstruktor
	int SetChapters(CChapters* chapters);
	CChapters* GetChapters();
	void UpdateChapters();
//	void virtual GetTextCallback(NMHDR* pNMHDR, LRESULT* pResult);
// Dialogfelddaten
	//{{AFX_DATA(CChapterDlg)
	enum { IDD = IDD_CHAPTERDIALOG };
	CComboBox	m_ChapterDisplay_Lng;
	CUnicodeTreeCtrl	m_ChapterDisplay_Edit;
	CChapterDlgList 	m_ChapterDisplay;
	CButton				m_Cancel;
	CButton				m_OK;
	CChapterDlgTree		m_Chapters;
	CButton				m_Saveas;
	CButton				m_Subchapter;
	//}}AFX_DATA


// Überschreibungen
	// Vom Klassen-Assistenten generierte virtuelle Funktionsüberschreibungen
	//{{AFX_VIRTUAL(CChapterDlg)
	public:
	virtual void OnFinalRelease();
	virtual BOOL DestroyWindow();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV-Unterstützung
//	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	//}}AFX_VIRTUAL

// Implementierung
protected:

	// Generierte Nachrichtenzuordnungsfunktionen
	//{{AFX_MSG(CChapterDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnEndlabeleditTree1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnMakesubchapter();
	afx_msg void OnSaveas();
	afx_msg void OnBeginlabeleditTree1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnRclickChaptertree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelchangedChaptertree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnItemchangedChapterDisplay(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelchangeChapterdisplayLng();
	afx_msg void OnEndlabeleditChapterdisplayEdit(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKeydownChapterDisplay(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBeginlabeleditChapterdisplayEdit(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	// Generierte OLE-Dispatch-Zuordnungsfunktionen
	//{{AFX_DISPATCH(CChapterDlg)
		// HINWEIS - Der Klassen-Assistent fügt hier Member-Funktionen ein und entfernt diese.
	//}}AFX_DISPATCH
	DECLARE_DISPATCH_MAP()
	DECLARE_INTERFACE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ fügt unmittelbar vor der vorhergehenden Zeile zusätzliche Deklarationen ein.

#endif // AFX_CHAPTERDLG_H__6A384E54_BF30_4BEB_AE02_39E2CA7BDD22__INCLUDED_
