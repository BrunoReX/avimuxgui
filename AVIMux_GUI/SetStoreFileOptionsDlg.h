#if !defined(AFX_SETSTOREFILEOPTIONSDLG_H__D6B2A967_EC8E_48C0_8602_29DEE2FB38F0__INCLUDED_)
#define AFX_SETSTOREFILEOPTIONSDLG_H__D6B2A967_EC8E_48C0_8602_29DEE2FB38F0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SetStoreFileOptionsDlg.h : Header-Datei
//

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CSetStoreFileOptionsDlg 

#include "SplitPointsDlg.h"
#include "ClusterTimeEdit.h"
#include "AC3FrameCountEdit.h"
#include "MKVAC3FrameCountEdit.h"
#include "..\Chapters.h"
#include "..\Buffers.h"

const int AIU_KB		= 0x0001;
const int AIU_FRAME		= 0x0002;

const int SIP_RIFF		= 0x0001;
const int SIP_FRAMES    = 0x0002;
const int SIP_AUTO		= 0x0003;

const int SOF_AVI		= 0x0000;
const int SOF_MKV		= 0x0001;

typedef struct
{
	DWORD	dwFlags;
	DWORD	dwIgnoreSize;
} OPENFILEOPTIONS;

const int SOFO_AVI_IGNORELARGECHUNKS		= 0x00000001;
const int SOFO_AVI_TRYTOREPAIRLARGECHUNKS	= 0x00000002;
const int SOFO_AVI_FORCEMP3VBR				= 0x00000004;
const int SOFO_AVI_REPAIRDX50				= 0x00000008;

const int SOFO_AVI_MASK                     = 0x0000000F;

const int SOFO_M2F2_DOM2F2CRCCHECK			= 0x00000020;

const int SOFO_MP3_CHECKCBRASK				= 0x00000100;
const int SOFO_MP3_CHECKCBRALWAYS			= 0x00000200;
const int SOFO_MP3_CHECKCBRNEVER			= 0x00000300;
const int SOFO_MP3_MASK						= 0x00000300;
const int SOFO_MP3_RESULTDLG				= 0x00000400;

const int SOFO_CONCAT_ALLOWCRAPAC3			= 0x00001000;

const int SOFO_CH_IMPORT					= 0x00010000;
const int SOFO_CH_FROMFILENAMES				= 0x00020000;
const int SOFO_CH_MASK                      = 0x00030000;

static const char* cLaceDefinitionFormats[] = {
	"general", "mp3", "ac3", "dts", "aac", "vorbis" };


typedef struct
{
	DWORD		dwPreload;
	DWORD		dwMaxFileSize;
	DWORD		dwUseMaxFileSize;
	DWORD		dwUseManualSplitPoints;
	DWORD		dwDontUseNumbering;
	char*		lpcNumbering;
	DWORD		dwFrames;
	DWORD		dwUseMaxFiles;
	DWORD		dwMaxFiles;
	CDialog*	cdMain;
	bool		bDispDoneDlg;
	bool		bDispOverwriteDlg;
	bool		bExitAfterwards;
	bool		bB0rk;
	int			i1stTimecode;
	int			iStdOutputFormat;
	int			iActiveButton;
	CChapters*	chapters;
	CSplitPoints* split_points;
} STOREFILEOPTIONS;

class CSetStoreFileOptionsDlg : public CDialog
{
private:
	STOREFILEOPTIONS	sfoData;
	OPENFILEOPTIONS     ofoData;
	CAttribs*			settings;
	CDynIntArray*		pages[10];
// Konstruktion
protected:
	int			ShowPage(int a);
	CAttribs*	GetCurrentLaceDefinition();
public:
	CSetStoreFileOptionsDlg(CWnd* pParent = NULL);   // Standardkonstruktor
	void		SetData(STOREFILEOPTIONS* lpsfoData,OPENFILEOPTIONS* lpofoData, CAttribs* lpsettings);
	void		GetData(STOREFILEOPTIONS* lpsfoData,OPENFILEOPTIONS* lpofoData, CAttribs** lpsettings);
	void		RefreshDlg();
	void		UpdateData();
	void		UpdateClusterSize();
	void		UpdateLaceLength();
	void		UpdateVideoLaceSetting();
	void		UpdateAC3FrameCount();
	void		UpdateMKVAC3FrameCount();
	void		UpdateNumbering();
	void		UpdateMaxFileSize();
	void		UpdateMaxFiles();
	void		SaveCurrentLaceStyleDefinition();
	void		UpdateLaceDefition();
	void		UpdateForceV10();
	void		UpdateCueSettings();
	void		UpdateMakeLegacyIndex();
	void		UpdateGeneralInput();
	void		UpdateFloatWidth();


// Dialogfelddaten
	//{{AFX_DATA(CSetStoreFileOptionsDlg)
	enum { IDD = IDD_SETOUTPUTOPTIONS };
	CButton	m_Haalimode;
	CButton	m_Randomize_Element_Order;
	CEdit	m_DTSFrameCount;
	CStatic	m_DTSFrameCount_Label;
	CEdit	m_FloatWidth;
	CStatic	m_FloatWidth_Label;
	CSpinButtonCtrl	m_FloatWidth_Spin;
	CEdit	m_MP3VBRFrameCount;
	CStatic	m_MP3VBRFrameCount_Label;
	CButton	m_AVI2;
	CButton	m_Output_AllowUnbufferedWrite;
	CButton	m_Input_AllowUnbufferedRead;
	CButton	m_UseInputCache;
	CButton	m_Input_General;
	CStatic	m_RIFFAVISize_Unit;
	CStatic	m_RIFFAVISize_Label;
	CEdit	m_RIFFAVISize;
	CButton	m_Logfile;
	CButton	m_IndexClustersInSeekhead;
	CButton	m_TimecodeScale;
	CButton	m_Others;
	CButton	m_Write2ndCopyOfTracks;
	CStatic	m_TimecodeScale_MKV_Label;
	CEdit	m_TimecodeScale_MKV;
	CStatic	m_TimecodeScale_MKA_Label;
	CEdit	m_TimecodeScale_MKA;
	CButton	m_MKVOutputOptions_2;
	CButton	m_MKV_Cluster;
	CButton	m_MKV_Lacing;
	CButton	m_MKV_Cues;
	CButton	m_WriteCues_Video;
	CButton	m_WriteCues_Audio;
	CButton	m_WriteCues_Audio_OnlyAudioOnly;
	CButton	m_WriteCues;
	CButton	m_ForceMKV10Compliance;
	CSpinButtonCtrl	m_LaceVideo_Spin;
	CEdit	m_LaceVideo_Count;
	CButton	m_LaceVideo;
	CButton	m_UseLaceDefinition;
	CSpinButtonCtrl	m_LaceDefinitionFormat_Spin;
	CEdit	m_LaceDefinitionFormat;
	CButton	m_DisplayWidth_Height;
	CButton	m_Overlapped;
	CButton	m_IMKV_chFromFilenames;
	CButton	m_IMKV_chImport;
	CButton	m_IMKV_Chapters_Label;
	CButton	m_IMKV_Label;
	CButton	m_IM2F2_Label;
	CButton	m_IM2F2_CRCCheck;
	CButton	m_IAVI_IgnoreLargeChunks;
	CButton	m_IAVI_Try2RepairLargeChunks;
	CButton	m_IAVI_RepairDX50;
	CEdit	m_IAVI_IgnoreSize;
	CButton	m_IAVI_ForceMP3VBR;
	CButton	m_IAVI_Label;
	CButton	m_IMP3_DispResult;
	CButton	m_IMP3_Label;
	CButton m_IMP3_CheckAlways;
	CButton m_IMP3_CheckNever;
	CButton m_IMP3_Ask;
	CStatic	m_Lace;
	CComboBox	m_Lacestyle;
	CSpinButtonCtrl	m_MKVAC3FrameCount_Spin;
	CMKVAC3FrameCountEdit	m_MKVAC3FrameCount;
	CStatic	m_MKV_AC3FramesPerBlock_Label;
	CButton	m_Options;
	CButton	m_Radio_AVI;
	CButton	m_Radio_General;
	CButton	m_Radio_MKV;
	CButton m_Radio_Input_AVIMP3;
	CButton m_Radio_Input_MKV;
	CButton m_Radio_Input_General;
	CButton	m_1st_Cluster_30sec;
	CEdit	m_Audiointerleave;
	CButton	m_MP3CBRMode;
	CStatic	m_Preload_Label;
	CEdit	m_Preload;
	CStatic	m_Audiointerleave_Label;
	CButton	m_Reclists;
	CButton	m_OpenDML_settings;
	CButton	m_AVI;
	CEdit	m_MaxFrames;
	CStatic	m_Frames_Label;
	CStatic	m_SFO_Frames;
	CButton	m_UseSplitPoints;
	CButton	m_MaxSize_extended;
	CButton	m_Split;
	CStatic	m_StdOutputFormat_Label;
	CEdit	m_MaxFiles;
	CButton	m_UseMaxSize;
	CButton	m_UseMaxFiles;
	CEdit	m_MaxFileSize;
	CButton	m_UseNumbering;
	CEdit	m_Format;
	CButton	m_ClusterPosition;
	CButton	m_PrevClusterSize;
	CStatic	m_StdIndicesPerStream_Label;
	CAC3FrameCountEdit	m_AC3FrameCount;
	CClusterTimeEdit	m_ClusterTime;
	CSpinButtonCtrl	m_ClusterTime_Spin;
	CStatic	m_ClusterTime_Label;
	CSpinButtonCtrl	m_LaceSize_Spin;
	CEdit	m_LaceSize;
	CButton	m_MakeLegacyIndex;
	CEdit	m_STDI_NumberOfFrames;
	CButton	m_OpenDML;
	CSpinButtonCtrl	m_ClusterSize_Spin;
	CStatic	m_ClusterSize_Label;
	CEdit	m_ClusterSize;
	CButton	m_DontWriteBlockSize;
	CSpinButtonCtrl	m_AC3FrameCount_spin;
	CStatic	m_AC3FrameCount_Label;
	CEdit	m_1stTimestamp;
	CStatic	m_1stTimestamp_Label;
	CComboBox	m_CBStdOutputFormat;
	CButton	m_BChapters;
	CButton	m_General;
	CButton	m_MKVOutputOptions;
	CButton m_STDI_RIFF;
	CButton m_STDI_Frames;
	CButton m_STDI_auto;
	CButton m_AI_KB;
	CButton m_AI_Frames;
	CButton m_MKV_Page1;
	CButton m_MKV_Page2;
	CButton m_AVI_Page1;
	CButton m_AVI_Page2;
	//}}AFX_DATA


// Überschreibungen
	// Vom Klassen-Assistenten generierte virtuelle Funktionsüberschreibungen
	//{{AFX_VIRTUAL(CSetStoreFileOptionsDlg)
	public:
	virtual void OnFinalRelease();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV-Unterstützung
	//}}AFX_VIRTUAL

// Implementierung
protected:

	// Generierte Nachrichtenzuordnungsfunktionen
	//{{AFX_MSG(CSetStoreFileOptionsDlg)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	afx_msg void OnMaxsizeExtended();
	afx_msg void OnGenChapters();
	afx_msg void OnDeltaposAc3framecountSpin(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposClustersizeSpin(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnOpendml();
	afx_msg void OnDeltaposMkvLacesizeSpin(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnMkvLace();
	afx_msg void OnDeltaposClustertimeSpin(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnUsenumbering();
	afx_msg void OnUsemaxsize();
	afx_msg void OnUsemaxfiles();
	afx_msg void OnClose();
	afx_msg void OnRadioAvi();
	afx_msg void OnRadioMkv();
	afx_msg void OnRadioGeneral();
	afx_msg void OnChangeMkvAc3framesperblock();
	afx_msg void OnDeltaposMkvAc3framesperblockSpin(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelchangeLacestyle();
	afx_msg void OnRadioInputAvimp3();
	afx_msg void OnRadioInputMkv();
	afx_msg void OnDeltaposLaceexceptionformatspin(NMHDR* pNMHDR, LRESULT* pResult);
	virtual void OnCancel();
	afx_msg void OnUselacingexception();
	afx_msg void OnLacevideo();
	afx_msg void OnDeltaposLacevideoSpin(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnForcemkv10();
	afx_msg void OnWritecues();
	afx_msg void OnWritecuesVideo();
	afx_msg void OnWritecuesAudio();
	afx_msg void OnWritecuesOnlyaudioonly();
	afx_msg void OnMkvPage1();
	afx_msg void OnMkvPage2();
	afx_msg void OnMakeidx1();
	afx_msg void OnRadioInputGeneral();
	afx_msg void OnSfoAllowunbufferedread();
	afx_msg void OnAviPage1();
	afx_msg void OnAviPage2();
	afx_msg void OnChangeMp3vbrframecount();
	afx_msg void OnDeltaposFloatwidthSpin(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	// Generierte OLE-Dispatch-Zuordnungsfunktionen
	//{{AFX_DISPATCH(CSetStoreFileOptionsDlg)
		// HINWEIS - Der Klassen-Assistent fügt hier Member-Funktionen ein und entfernt diese.
	//}}AFX_DISPATCH
	DECLARE_DISPATCH_MAP()
	DECLARE_INTERFACE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ fügt unmittelbar vor der vorhergehenden Zeile zusätzliche Deklarationen ein.

#endif // AFX_SETSTOREFILEOPTIONSDLG_H__D6B2A967_EC8E_48C0_8602_29DEE2FB38F0__INCLUDED_
