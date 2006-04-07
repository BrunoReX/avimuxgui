// AVIMux_GUIDlg.h : Header-Datei
//

#if !defined(AFX_AVIMUX_GUIDLG_H__E08D67DC_40F7_4A39_919C_A12D2C827F08__INCLUDED_)
#define AFX_AVIMUX_GUIDLG_H__E08D67DC_40F7_4A39_919C_A12D2C827F08__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CAVIMux_GUIDlg Dialogfeld


#include "SplitPointsDlg.h"
#include "SourceFileListbox.h"
#include "SetStoreFileOptionsDlg.h"
#include "VideoSourceListBox.h"
#include "SubtitlesListBox.h"
#include "ProgressList.h"
#include "FillASIs.h"
#include "Languages.h"
#include "ProtocolListCtrl.h"
#include "..\Chapters.h"
#include "audiosourcetree.h"
#include "windows.h"
int Thread_OnStart(void* pData);

__int64	GetLaceSetting(char* cName, CAttribs* settings, int iFlags = 0);
void name_without_ext(char* lpcName, char* cNoExt);

static HANDLE hGlobalMuxingStartedSemaphore;
static HANDLE hGlobalMuxSemaphore;

#define APL_ASCII 0x01
#define APL_UTF8  0x02

class CAVIMux_GUIDlg : public CDialog
{
private:
	UINT			uiMessage;
	int				iUnicode_possible;
	int				chapter_level;
	CChapters*		chap[20];
	bool			bDo_OnSelchangeLngCode;
	bool			bAddAS_immed;
	void			ApplyStreamSettings(void);
	void			ApplyCurrentLanguageCode(void);
	void			ApplyCurrentLanguageName(void);
	HANDLE			hLogFile;
	char			cLogFileName[1024];
	void			OnGetAudiodispinfo(NMHDR* pNMHDR, LRESULT* pResult);
	CAttribs*		settings;
	char			lastjobfile[500];
	char			cfgfile[500];
// Konstruktion
public:
	CAVIMux_GUIDlg(CWnd* pParent = NULL);	// Standard-Konstruktor
	void			AddFile (CFileDialog* cfd);
	int				iCurrentView;
	int				iButtonState;
	int				iDebugLevel;
	DWORD*			dwLangs;
	char*			cExtFilename;
	CChapters*		chapters;
	bool	bEditInProgess;

	LANGUAGE_DESCRIPTOR**	lplpLanguages;
	DWORD			dwLanguages;
	void			SetDialogState_Config();
	UINT			GetUserMessageID();
	void			SetDialogState_Muxing();
	void			AddProtocolLine(char* lpcText,DWORD dwDebugLevel, int dwCC = APL_ASCII);
	void			AddProtocolLine(CString lpcText,DWORD dwDebugLevel, int dwCC = APL_ASCII);
	void			AddProtocol_Separator();
	void			SetDebugLevel(int iLevel);
	void			doSaveConfig(char* lpcFile, int clear = 1);
	CAttribs*		GetSettings();

	DWORD			Clear(void);
	STOREFILEOPTIONS sfOptions;

	CString			cstrInformation;
	CString			cstrWarning;
	CString			cstrError;
	CString			cstrConfirmation;
	OPENFILEOPTIONS	ofOptions;
	void			ButtonState_STOP(void);
	void			ButtonState_START(void);
	void			UpdateAudioName(void);
	void			UpdateLanguage(void);
	void			UpdateAudiodelay(void);
	void			OnAddFile();
	void			doAddFile(char* lpcName, int iFormat = FILETYPE_UNKNOWN);

	void			AddAudioStream(AUDIO_STREAM_INFO* asi);
	void			AddSubtitleStream(SUBTITLE_STREAM_INFO* ssi);


// Dialogfelddaten
	//{{AFX_DATA(CAVIMux_GUIDlg)
	enum { IDD = IDD_AVIMUX_GUI_DIALOG };
	CEdit	m_StatusLine;
	CComboBox	m_Stream_Lng;
	CStatic	m_OutputResolution_Label;
	CEdit	m_OutputResolution;
	CAudioSourceTree	m_AudioTree;
	CStatic	m_Title_Label;
	CEdit	m_Title;
	CStatic	m_VideoStretchFactor_Label;
	CEdit	m_VideoStretchFactor;
	CProgressList	m_Progress_List;
	CProtocolListCtrl	m_Protocol;
	CButton	m_Output_Options_Button;
	CButton	m_Start_Button;
	CStatic	m_Open_Files_Label;
	CEdit	m_Prg_Frames;
	CStatic	m_Prg_Frames_Label;
	CEdit	m_Prg_Dest_File;
	CStatic	m_Prg_Dest_File_Label;
	CProgressCtrl	m_Prg_Legidx_Progress;
	CButton	m_Progress_Group;
	CButton	m_Avoid_Seek_Ops;
	CButton	m_Add_Video_Source;
	CProgressCtrl	m_Prg_Progress;
	CStatic	m_Prg_Legidx_Label;
	CStatic	m_Prg_Progress_Label;
	CEdit	m_Default_Audio;
	CButton	m_Default_Audio_Label;
	CButton	m_All_Subtitles;
	CButton	m_All_Audio;
	CButton	m_No_Subtitles;
	CButton	m_No_Audio;
	CStatic	m_Video_Headline;
	CStatic	m_Subtitles_Headline;
	CStatic	m_AvailableStreams_Header;
	CStatic	m_Audio_Headline;
	CListCtrl	m_Enh_Filelist;
	CStatic	m_Audiodelay_Label;
	CEdit	m_Audiodelay;
	CStatic	m_Audioname_Label;
	CEdit	m_AudioName;
	CStatic	m_Subname_Label;
	CStatic	m_Protocol_Label;
	CEdit	m_SubtitleName;
	CVideoSourceListBox	m_VideoSources;
	CSourceFileListBox	m_SourceFiles;
	//}}AFX_DATA

	// Vom Klassenassistenten generierte Überladungen virtueller Funktionen
	//{{AFX_VIRTUAL(CAVIMux_GUIDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV-Unterstützung
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementierung
protected:
	HICON m_hIcon;

	// Generierte Message-Map-Funktionen
	//{{AFX_MSG(CAVIMux_GUIDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	virtual void OnOK();
	afx_msg void OnStart();
	virtual void OnCancel();
	virtual void OnSave();
	virtual void OnClear();
	afx_msg void OnStop();
	afx_msg void OnAddFileList();
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMaxsizeExtended();
	afx_msg void OnUsemaxsize();
	afx_msg void OnLoad();
	afx_msg void OnOutputoptions();
	afx_msg void OnNoAudio();
	afx_msg void OnAllAudio();
	afx_msg void OnNoSubtitles();
	afx_msg void OnAllSubtitles();
	afx_msg void OnDefaultAudio();
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnChangeAudioname();
	afx_msg void OnChangeDelay();
	afx_msg void OnSelchangedAudiotree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnItemexpandedAudiotree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelchangeSourcefilelist();
	afx_msg void OnEditchangeStreamLng();
	afx_msg void OnSelchangeStreamLng();
	afx_msg void OnEditupdateStreamLng();
	afx_msg void OnUpdateAudioname();
	afx_msg void OnBeginlabeleditAudiotree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEndlabeleditAudiotree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnReturnAudiotree(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ fügt unmittelbar vor der vorhergehenden Zeile zusätzliche Deklarationen ein.

#endif // !defined(AFX_AVIMUX_GUIDLG_H__E08D67DC_40F7_4A39_919C_A12D2C827F08__INCLUDED_)
