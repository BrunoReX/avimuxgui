#if !defined(AFX_AUDIOSOURCETREE_H__DBEF7E65_CA96_491A_9D46_FDA849441A13__INCLUDED_)
#define AFX_AUDIOSOURCETREE_H__DBEF7E65_CA96_491A_9D46_FDA849441A13__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AudioSourceTree.h : Header-Datei
//

//#include "audiosourcelist.h"
#include "subtitles.h"
#include "../dynarray.h"
#include "trees.h"
#include "audiosource.h"
#include "unicodetreectrl.h"
#include "videosource.h"

typedef struct 
{
	DWORD*				lpdwFiles;
	AUDIOSOURCE*		audiosource;

	DWORD				dwType;
	DWORD				dwFlags;
	AVIStreamHeader*	lpASH;
	
	union {
		void*				lpFormat;
		void*               lpFormatMKV;
	};
	union {
		int					iFormatSize;
		int					iFormatSizeMKV;
	};

	// for vorbis only
	void*				lpFormatAVI;
	int					iFormatSizeAVI;

	int*				iScaleF;
	int					iDelay;
	__int64				iSize;

	bool				bNameFromFormatTag;
} AUDIO_STREAM_INFO;

typedef struct 
{
	DWORD*	lpdwFiles;
	MULTIMEDIASOURCE* mms;
} MULTIMEDIA_STREAM_INFO;

const int ASIF_ALLOCATED =	0x01;
const int ASIF_AVISOURCE =	0x02;

class TREE_ITEM_INFO
{
private:
	CFont* lastUsedFont;

public:
	TREE_ITEM_INFO()
	{
		iID = 0;
		iOrgPos = 0;
		iCurrPos = 0;
		iHideText = 0;
		iTextWidth = 0;
		pData = NULL;
		lastUsedFont = 0;
	}

	CFont* LastUsedFont() const {
		return lastUsedFont;
	}

	void SetFont(CFont* newFont) 
	{
		lastUsedFont = newFont;
	}

	void ResetFont()
	{
		if (lastUsedFont)
		{
			lastUsedFont->DeleteObject();
			delete lastUsedFont;
		}
		lastUsedFont = NULL;
	}

	int		iID;
	int		iOrgPos;
	int		iCurrPos;
	int     iHideText;
	int     iTextWidth;
	union
	{
		void*                  pData;
		AUDIO_STREAM_INFO*     pASI;
		SUBTITLE_STREAM_INFO*  pSSI;
		VIDEO_STREAM_INFO*     pVSI;
		MULTIMEDIA_STREAM_INFO* pMSI;
		char*                  pText;
	};
};

const int TIIID_MSI     = 0x08;
const int TIIID_ASI		= 0x01 | TIIID_MSI;
const int TIIID_SSI		= 0x02 | TIIID_MSI;
const int TIIID_VSI     = 0x04 | TIIID_MSI;

const int TIIID_LNGCODE = 0x10;
const int TIIID_STRNAME = 0x20;
const int TIIID_TITLE   = 0x40;
const int TIIID_TITLELNG= 0x80;

TREE_ITEM_INFO*	BuildTIIfromASI(AUDIO_STREAM_INFO* asi);
TREE_ITEM_INFO*	BuildTIIfromSSI(SUBTITLE_STREAM_INFO* ssi);
TREE_ITEM_INFO* BuildTIIfromVSI(VIDEO_STREAM_INFO* vsi);

/////////////////////////////////////////////////////////////////////////////
// Fenster CAudioSourceTree 

class CAudioSourceTree : public CUnicodeTreeCtrl
{
private:
	bool	bDragging;
	TVITEM	dragged_item;
	CPoint	drag_source_point;
// Konstruktion
public:
	CAudioSourceTree();
	void	OpenContextMenu(CPoint point);
	void	Sort();
	int		GetRootCount();

// Attribute
public:
	void			AddTitleToStreamTree(HTREEITEM hParent, char* cLng, char* cTitle);
	void			DeleteTitleFromStreamTree(HTREEITEM hTitle);
	void			DeleteAllTitlesFromStreamTree(HTREEITEM hParent);
// Operationen
public:
	HTREEITEM	FindID(HTREEITEM hItem, int iID, TREE_ITEM_INFO** tii = NULL);
	
	std::vector<HTREEITEM> GetItems(
		HTREEITEM hItem, 
		int iID, 
		int iCheck, 
		std::vector<int>* indices = NULL);
	
	TREE_ITEM_INFO* GetItemInfo(HTREEITEM hItem);
	TREE_ITEM_INFO*	FindItemOriginalPosition(int original_position);
	void GetAllInfo(HTREEITEM hParent, std::vector<TREE_ITEM_INFO*>& result);
// Überschreibungen
	// Vom Klassen-Assistenten generierte virtuelle Funktionsüberschreibungen
	//{{AFX_VIRTUAL(CAudioSourceTree)
	public:
	virtual void OnFinalRelease();
	protected:
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementierung
public:
	virtual ~CAudioSourceTree();

	// Generierte Nachrichtenzuordnungsfunktionen
protected:
	//{{AFX_MSG(CAudioSourceTree)
	afx_msg void OnRclick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBegindrag(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
	// Generierte OLE-Dispatch-Zuordnungsfunktionen
	//{{AFX_DISPATCH(CAudioSourceTree)
		// HINWEIS - Der Klassen-Assistent fügt hier Member-Funktionen ein und entfernt diese.
	//}}AFX_DISPATCH
	DECLARE_DISPATCH_MAP()
	DECLARE_INTERFACE_MAP()
public:
	afx_msg void OnTvnGetdispinfo(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ fügt unmittelbar vor der vorhergehenden Zeile zusätzliche Deklarationen ein.

#endif // AFX_AUDIOSOURCETREE_H__DBEF7E65_CA96_491A_9D46_FDA849441A13__INCLUDED_
