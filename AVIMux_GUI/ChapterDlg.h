#if !defined(AFX_CHAPTERDLG_H__6A384E54_BF30_4BEB_AE02_39E2CA7BDD22__INCLUDED_)
#define AFX_CHAPTERDLG_H__6A384E54_BF30_4BEB_AE02_39E2CA7BDD22__INCLUDED_

#include "..\chapters.h"
#include "ResizeableDialog.h"
#include "ChapterDlgTree.h"
#include "UnicodeListControl.h"
#include "ChapterDlgList.h"
#include "UserDrawEdit.h"
#include "resource.h"
#include "..\Filestream.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ChapterDlg.h : Header-Datei
//

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CChapterDlg 

#include "ChapterSegmentUIDEdit.h"
#include "AttachedWindows.h"

int RenderChapters2File(FILESTREAM* f, CChapters* c);

int RenderChapters2File(FILE* f, CChapters* c);



int AddChaptersToTree(CUnicodeTreeCtrl* tree,HTREEITEM hParent, CChapters* c,
					  int start=0, int end=CHAP_LAST);

class CChapterDlg : public CResizeableDialog
{
private:
	CChapters*	c;
	int		iSelectedChapterLanguageEntry;
	CHAPTER_ENTRY* selected_chapter_entry;
	int		chapter_title_changed;

	HTREEITEM hSelectedChapter;
	void	UpdateChapterDisplayLngEdit(int iItem);

	HTREEITEM		GetSelectedChapter();
	HWND	hChapterTitle;
	void	ApplyNewChapterTitle();

	void	InitChapterDisplayColumns();
	void	OnDisplayChapterUids(); 

	int		timer_set;

// Konstruktion
public:
	CChapterDlg(CWnd* pParent = NULL);   // Standardkonstruktor
	int SetChapters(CChapters* chapters);
	CChapters* GetChapters();
	void EnableChapterTitleEdit(bool enabled);
	void DeleteChapterLanguageDisplay();
	void AfterLastDeleted();

	void UpdateChapterUIDEdit(CChapters* c, int i);
	void UpdateChapterSegmentUID(CHAPTER_ENTRY* pCE);

// Dialogfelddaten
	//{{AFX_DATA(CChapterDlg)
	enum { IDD = IDD_CHAPTERDIALOG };
	CStatic	m_ChapterSegmentUID_Label;
	CStatic	m_ChapterUID_Label;
	CEdit	m_Chapter_Title;
	CStatic	m_Chapters_Usage_Label;
	CUserDrawEdit	m_ChapterUID;
	CChapterSegmentUIDEdit	m_ChapterSegmentUID;
	CComboBox	m_ChapterDisplay_Lng;
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
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
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
	afx_msg void OnChangeChaptersegmentuid();
	afx_msg void OnCancel();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnKeydownChapterDisplay(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	// Generierte OLE-Dispatch-Zuordnungsfunktionen
	//{{AFX_DISPATCH(CChapterDlg)
		// HINWEIS - Der Klassen-Assistent fügt hier Member-Funktionen ein und entfernt diese.
	//}}AFX_DISPATCH
	DECLARE_DISPATCH_MAP()
	DECLARE_INTERFACE_MAP()
public:
	afx_msg void OnStnClickedChaptersUsageLabel();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnDestroy();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ fügt unmittelbar vor der vorhergehenden Zeile zusätzliche Deklarationen ein.

#endif // AFX_CHAPTERDLG_H__6A384E54_BF30_4BEB_AE02_39E2CA7BDD22__INCLUDED_
