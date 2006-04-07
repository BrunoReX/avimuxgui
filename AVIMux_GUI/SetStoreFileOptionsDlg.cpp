// SetStoreFileOptionsDlg.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "AVIMux_GUI.h"
#include "SetStoreFileOptionsDlg.h"
#include "Languages.h"
#include "..\Chapters.h"
#include "Trees.h"
#include "ChapterDlg.h"
#include "FormatText.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CSetStoreFileOptionsDlg 


CSetStoreFileOptionsDlg::CSetStoreFileOptionsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSetStoreFileOptionsDlg::IDD, pParent)
{
	EnableAutomation();

	//{{AFX_DATA_INIT(CSetStoreFileOptionsDlg)
	//}}AFX_DATA_INIT
}


void CSetStoreFileOptionsDlg::OnFinalRelease()
{
	// Nachdem die letzte Referenz auf ein Automatisierungsobjekt freigegeben wurde,
	// wird OnFinalRelease aufgerufen. Die Basisklasse löscht das Objekt
	// automatisch. Fügen Sie zusätzlichen Bereinigungscode für Ihr Objekt
	// hinzu, bevor Sie die Basisklasse aufrufen.

	CDialog::OnFinalRelease();
}

static const int iLaceDefinitionCount = sizeof(cLaceDefinitionFormats)/sizeof(char*);
static int iCurrentLaceDefinition = 0;
static int iCurrentFloatWidthIndex = 0;
#define page_count 8

void CSetStoreFileOptionsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSetStoreFileOptionsDlg)
	DDX_Control(pDX, IDC_HAALIMODE, m_Haalimode);
	DDX_Control(pDX, IDC_RANDOMIZE_ELEMENT_ORDER, m_Randomize_Element_Order);
	DDX_Control(pDX, IDC_DTSFRAMECOUNT, m_DTSFrameCount);
	DDX_Control(pDX, IDC_DTSFRAMECOUNT_LABEL, m_DTSFrameCount_Label);
	DDX_Control(pDX, IDC_FLOATWIDTH, m_FloatWidth);
	DDX_Control(pDX, IDC_FLOATWIDTH_LABEL, m_FloatWidth_Label);
	DDX_Control(pDX, IDC_FLOATWIDTH_SPIN, m_FloatWidth_Spin);
	DDX_Control(pDX, IDC_MP3VBRFRAMECOUNT, m_MP3VBRFrameCount);
	DDX_Control(pDX, IDC_MP3VBRFRAMECOUNT_LABEL, m_MP3VBRFrameCount_Label);
	DDX_Control(pDX, IDC_SFO_O_AVISTRUCTURE2, m_AVI2);
	DDX_Control(pDX, IDC_SFO_ALLOWUNBUFFEREDWRITE, m_Output_AllowUnbufferedWrite);
	DDX_Control(pDX, IDC_SFO_ALLOWUNBUFFEREDREAD, m_Input_AllowUnbufferedRead);
	DDX_Control(pDX, IDC_AVOIDSEEKOPS, m_UseInputCache);
	DDX_Control(pDX, IDC_SFO_I_GENERAL, m_Input_General);
	DDX_Control(pDX, IDC_RIFFAVISIZE_UNIT, m_RIFFAVISize_Unit);
	DDX_Control(pDX, IDC_RIFFAVISIZE_LABEL, m_RIFFAVISize_Label);
	DDX_Control(pDX, IDC_RIFFAVISIZE, m_RIFFAVISize);
	DDX_Control(pDX, IDC_LOGFILE, m_Logfile);
	DDX_Control(pDX, IDC_INDEXINSEEKHEAD, m_IndexClustersInSeekhead);
	DDX_Control(pDX, IDC_TIMECODESCALE, m_TimecodeScale);
	DDX_Control(pDX, IDC_others, m_Others);
	DDX_Control(pDX, IDC_2ndCopieOfTracks, m_Write2ndCopyOfTracks);
	DDX_Control(pDX, IDC_TCS_MKV_LABEL, m_TimecodeScale_MKV_Label);
	DDX_Control(pDX, IDC_TCS_MKV, m_TimecodeScale_MKV);
	DDX_Control(pDX, IDC_TCS_MKA_LABEL, m_TimecodeScale_MKA_Label);
	DDX_Control(pDX, IDC_TCS_MKA, m_TimecodeScale_MKA);
	DDX_Control(pDX, IDC_SFO_O_MKVOPTIONS2, m_MKVOutputOptions_2);
	DDX_Control(pDX, IDC_MKV_CLUSTER, m_MKV_Cluster);
	DDX_Control(pDX, IDC_MKV_LACING, m_MKV_Lacing);
	DDX_Control(pDX, IDC_MKV_CUES, m_MKV_Cues);
	DDX_Control(pDX, IDC_WRITECUES_VIDEO, m_WriteCues_Video);
	DDX_Control(pDX, IDC_WRITECUES_AUDIO, m_WriteCues_Audio);
	DDX_Control(pDX, IDC_WRITECUES_ONLYAUDIOONLY, m_WriteCues_Audio_OnlyAudioOnly);
	DDX_Control(pDX, IDC_WRITECUES, m_WriteCues);
	DDX_Control(pDX, IDC_FORCEMKV10, m_ForceMKV10Compliance);
	DDX_Control(pDX, IDC_LACEVIDEO_SPIN, m_LaceVideo_Spin);
	DDX_Control(pDX, IDC_LACEVIDEO_COUNT, m_LaceVideo_Count);
	DDX_Control(pDX, IDC_LACEVIDEO, m_LaceVideo);
	DDX_Control(pDX, IDC_USELACINGEXCEPTION, m_UseLaceDefinition);
	DDX_Control(pDX, IDC_LACEEXCEPTIONFORMATSPIN, m_LaceDefinitionFormat_Spin);
	DDX_Control(pDX, IDC_LACEEXCEPTIONFORMAT, m_LaceDefinitionFormat);
	DDX_Control(pDX, IDC_MKV_DISPWH, m_DisplayWidth_Height);
	DDX_Control(pDX, IDC_OVERLAPPED, m_Overlapped);
	DDX_Control(pDX, IDC_IMKV_CH_FROMFILENAMES, m_IMKV_chFromFilenames);
	DDX_Control(pDX, IDC_IMKV_CH_IMPORT, m_IMKV_chImport);
	DDX_Control(pDX, IDC_IMKV_CHAPTERS_LABEL, m_IMKV_Chapters_Label);
	DDX_Control(pDX, IDC_IMKV_Label, m_IMKV_Label);
	DDX_Control(pDX, IDC_IM2F2_LABEL, m_IM2F2_Label);
	DDX_Control(pDX, IDC_IM2F2_CRCCHECK, m_IM2F2_CRCCheck);
	DDX_Control(pDX, IDC_IAVI_IGNORECHUNKS_LABEL, m_IAVI_IgnoreLargeChunks);
	DDX_Control(pDX, IDC_IAVI_TRYTOREPAIRLARGECHUNKS, m_IAVI_Try2RepairLargeChunks);
	DDX_Control(pDX, IDC_IAVI_REPAIRDX50, m_IAVI_RepairDX50);
	DDX_Control(pDX, IDC_IAVI_IGNOREKBYTE, m_IAVI_IgnoreSize);
	DDX_Control(pDX, IDC_IAVI_FORCEMP3VBR, m_IAVI_ForceMP3VBR);
	DDX_Control(pDX, IDC_IAVI_Label, m_IAVI_Label);
	DDX_Control(pDX, IDC_IMP3_DISPRESULT, m_IMP3_DispResult);
	DDX_Control(pDX, IDC_IMP3_LABEL, m_IMP3_Label);
	DDX_Control(pDX, IDC_IMP3_CHECKALWAYS, m_IMP3_CheckAlways);
	DDX_Control(pDX, IDC_IMP3_CHECKNEVER, m_IMP3_CheckNever);
	DDX_Control(pDX, IDC_IMP3_ASK, m_IMP3_Ask);
	DDX_Control(pDX, IDC_MKV_LACE, m_Lace);
	DDX_Control(pDX, IDC_LACESTYLE, m_Lacestyle);
	DDX_Control(pDX, IDC_MKV_AC3FRAMESPERBLOCK_SPIN, m_MKVAC3FrameCount_Spin);
	DDX_Control(pDX, IDC_MKV_AC3FRAMESPERBLOCK, m_MKVAC3FrameCount);
	DDX_Control(pDX, IDC_MKV_AC3FRAMESPERBLOCK_LABEL, m_MKV_AC3FramesPerBlock_Label);
	DDX_Control(pDX, IDC_SFO_OPTIONS, m_Options);
	DDX_Control(pDX, IDC_RADIO_AVI, m_Radio_AVI);
	DDX_Control(pDX, IDC_RADIO_GENERAL, m_Radio_General);
	DDX_Control(pDX, IDC_RADIO_MKV, m_Radio_MKV);
	DDX_Control(pDX, IDC_RADIO_INPUT_AVIMP3, m_Radio_Input_AVIMP3);
	DDX_Control(pDX, IDC_RADIO_INPUT_MKV, m_Radio_Input_MKV);
	DDX_Control(pDX, IDC_RADIO_INPUT_GENERAL, m_Radio_Input_General);
	DDX_Control(pDX, IDC_1STCL_30sec, m_1st_Cluster_30sec);
	DDX_Control(pDX, IDC_AUDIOINTERLEAVE, m_Audiointerleave);
	DDX_Control(pDX, IDC_MP3CBRFRAMEMODE, m_MP3CBRMode);
	DDX_Control(pDX, IDC_SPRELOAD, m_Preload_Label);
	DDX_Control(pDX, IDC_PRELOAD, m_Preload);
	DDX_Control(pDX, IDC_SAUDIOINTERLEAVE, m_Audiointerleave_Label);
	DDX_Control(pDX, IDC_RECLISTS, m_Reclists);
	DDX_Control(pDX, IDC_SFO_ODML_SETTINGS, m_OpenDML_settings);
	DDX_Control(pDX, IDC_SFO_O_AVISTRUCTURE, m_AVI);
	DDX_Control(pDX, IDC_MAXFRAMES, m_MaxFrames);
	DDX_Control(pDX, IDC_SFO_OSP_FRAMES, m_Frames_Label);
	DDX_Control(pDX, IDC_SFO_FRAMES, m_SFO_Frames);
	DDX_Control(pDX, IDC_USESPLITPOINTLIST, m_UseSplitPoints);
	DDX_Control(pDX, IDC_MAXSIZE_EXTENDED, m_MaxSize_extended);
	DDX_Control(pDX, IDC_SFO_O_SPLIT, m_Split);
	DDX_Control(pDX, IDC_STDOUTPUTFORMAT_LABEL, m_StdOutputFormat_Label);
	DDX_Control(pDX, IDC_MAXFILES, m_MaxFiles);
	DDX_Control(pDX, IDC_USEMAXSIZE, m_UseMaxSize);
	DDX_Control(pDX, IDC_USEMAXFILES, m_UseMaxFiles);
	DDX_Control(pDX, IDC_MAXFILESIZE, m_MaxFileSize);
	DDX_Control(pDX, IDC_USENUMBERING, m_UseNumbering);
	DDX_Control(pDX, IDC_FORMAT, m_Format);
	DDX_Control(pDX, IDC_MKV_POSITION, m_ClusterPosition);
	DDX_Control(pDX, IDC_MKV_PREVSIZE, m_PrevClusterSize);
	DDX_Control(pDX, IDC_SFO_STDIDXPERSTREAM, m_StdIndicesPerStream_Label);
	DDX_Control(pDX, IDC_AC3FRAMECOUNT, m_AC3FrameCount);
	DDX_Control(pDX, IDC_CLUSTERTIME, m_ClusterTime);
	DDX_Control(pDX, IDC_CLUSTERTIME_SPIN, m_ClusterTime_Spin);
	DDX_Control(pDX, IDC_CLUSTERTIME_LABEL, m_ClusterTime_Label);
	DDX_Control(pDX, IDC_MKV_LACESIZE_SPIN, m_LaceSize_Spin);
	DDX_Control(pDX, IDC_MKV_LACESIZE, m_LaceSize);
	DDX_Control(pDX, IDC_MAKEIDX1, m_MakeLegacyIndex);
	DDX_Control(pDX, IDC_STDI_NBROFFRAMES, m_STDI_NumberOfFrames);
	DDX_Control(pDX, IDC_OPENDML, m_OpenDML);
	DDX_Control(pDX, IDC_CLUSTERSIZE_SPIN, m_ClusterSize_Spin);
	DDX_Control(pDX, IDC_CLUSTERSIZE_LABEL, m_ClusterSize_Label);
	DDX_Control(pDX, IDC_CLUSTERSIZE, m_ClusterSize);
	DDX_Control(pDX, IDC_DONTWRITEBLOCKSIZE, m_DontWriteBlockSize);
	DDX_Control(pDX, IDC_AC3FRAMECOUNT_SPIN, m_AC3FrameCount_spin);
	DDX_Control(pDX, IDC_AC3FRAMECOUNT_LABEL, m_AC3FrameCount_Label);
	DDX_Control(pDX, IDC_1STTIMESTAMP, m_1stTimestamp);
	DDX_Control(pDX, IDC_1STTIMESTAMP_LABEL, m_1stTimestamp_Label);
	DDX_Control(pDX, IDC_STDOUTPUTFORMATS, m_CBStdOutputFormat);
	DDX_Control(pDX, IDC_GEN_CHAPTERS, m_BChapters);
	DDX_Control(pDX, IDC_SFO_O_GENERAL, m_General);
	DDX_Control(pDX, IDC_SFO_O_MKVOPTIONS, m_MKVOutputOptions);
	DDX_Control(pDX, IDC_STDI_RIFF, m_STDI_RIFF);
	DDX_Control(pDX, IDC_STDI_FRAMES, m_STDI_Frames);
	DDX_Control(pDX, IDC_STDI_AUTO, m_STDI_auto);
	DDX_Control(pDX, IDC_AI_KB, m_AI_KB);
	DDX_Control(pDX, IDC_AI_FRAMES, m_AI_Frames);
	DDX_Control(pDX, IDC_MKV_PAGE1, m_MKV_Page1);
	DDX_Control(pDX, IDC_MKV_PAGE2, m_MKV_Page2);
	DDX_Control(pDX, IDC_AVI_PAGE1, m_AVI_Page1);
	DDX_Control(pDX, IDC_AVI_PAGE2, m_AVI_Page2);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CSetStoreFileOptionsDlg, CDialog)
	//{{AFX_MSG_MAP(CSetStoreFileOptionsDlg)
	ON_BN_CLICKED(IDC_MAXSIZE_EXTENDED, OnMaxsizeExtended)
	ON_BN_CLICKED(IDC_GEN_CHAPTERS, OnGenChapters)
	ON_NOTIFY(UDN_DELTAPOS, IDC_AC3FRAMECOUNT_SPIN, OnDeltaposAc3framecountSpin)
	ON_NOTIFY(UDN_DELTAPOS, IDC_CLUSTERSIZE_SPIN, OnDeltaposClustersizeSpin)
	ON_BN_CLICKED(IDC_OPENDML, OnOpendml)
	ON_NOTIFY(UDN_DELTAPOS, IDC_MKV_LACESIZE_SPIN, OnDeltaposMkvLacesizeSpin)
	ON_BN_CLICKED(IDC_MKV_LACE, OnMkvLace)
	ON_NOTIFY(UDN_DELTAPOS, IDC_CLUSTERTIME_SPIN, OnDeltaposClustertimeSpin)
	ON_BN_CLICKED(IDC_USENUMBERING, OnUsenumbering)
	ON_BN_CLICKED(IDC_USEMAXSIZE, OnUsemaxsize)
	ON_BN_CLICKED(IDC_USEMAXFILES, OnUsemaxfiles)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_RADIO_AVI, OnRadioAvi)
	ON_BN_CLICKED(IDC_RADIO_MKV, OnRadioMkv)
	ON_BN_CLICKED(IDC_RADIO_GENERAL, OnRadioGeneral)
	ON_EN_CHANGE(IDC_MKV_AC3FRAMESPERBLOCK, OnChangeMkvAc3framesperblock)
	ON_NOTIFY(UDN_DELTAPOS, IDC_MKV_AC3FRAMESPERBLOCK_SPIN, OnDeltaposMkvAc3framesperblockSpin)
	ON_CBN_SELCHANGE(IDC_LACESTYLE, OnSelchangeLacestyle)
	ON_BN_CLICKED(IDC_RADIO_INPUT_AVIMP3, OnRadioInputAvimp3)
	ON_BN_CLICKED(IDC_RADIO_INPUT_MKV, OnRadioInputMkv)
	ON_NOTIFY(UDN_DELTAPOS, IDC_LACEEXCEPTIONFORMATSPIN, OnDeltaposLaceexceptionformatspin)
	ON_BN_CLICKED(IDC_USELACINGEXCEPTION, OnUselacingexception)
	ON_BN_CLICKED(IDC_LACEVIDEO, OnLacevideo)
	ON_NOTIFY(UDN_DELTAPOS, IDC_LACEVIDEO_SPIN, OnDeltaposLacevideoSpin)
	ON_BN_CLICKED(IDC_FORCEMKV10, OnForcemkv10)
	ON_BN_CLICKED(IDC_WRITECUES, OnWritecues)
	ON_BN_CLICKED(IDC_WRITECUES_VIDEO, OnWritecuesVideo)
	ON_BN_CLICKED(IDC_WRITECUES_AUDIO, OnWritecuesAudio)
	ON_BN_CLICKED(IDC_WRITECUES_ONLYAUDIOONLY, OnWritecuesOnlyaudioonly)
	ON_BN_CLICKED(IDC_MKV_PAGE1, OnMkvPage1)
	ON_BN_CLICKED(IDC_MKV_PAGE2, OnMkvPage2)
	ON_BN_CLICKED(IDC_MAKEIDX1, OnMakeidx1)
	ON_BN_CLICKED(IDC_RADIO_INPUT_GENERAL, OnRadioInputGeneral)
	ON_BN_CLICKED(IDC_SFO_ALLOWUNBUFFEREDREAD, OnSfoAllowunbufferedread)
	ON_BN_CLICKED(IDC_AVI_PAGE1, OnAviPage1)
	ON_BN_CLICKED(IDC_AVI_PAGE2, OnAviPage2)
	ON_EN_CHANGE(IDC_MP3VBRFRAMECOUNT, OnChangeMp3vbrframecount)
	ON_NOTIFY(UDN_DELTAPOS, IDC_FLOATWIDTH_SPIN, OnDeltaposFloatwidthSpin)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BEGIN_DISPATCH_MAP(CSetStoreFileOptionsDlg, CDialog)
	//{{AFX_DISPATCH_MAP(CSetStoreFileOptionsDlg)
		// HINWEIS - Der Klassen-Assistent fügt hier Zuordnungsmakros ein und entfernt diese.
	//}}AFX_DISPATCH_MAP
END_DISPATCH_MAP()

// Hinweis: Wir stellen Unterstützung für IID_ISetStoreFileOptionsDlg zur Verfügung, um typsicheres Binden
//  von VBA zu ermöglichen. Diese IID muss mit der GUID übereinstimmen, die in der
//  Disp-Schnittstelle in der .ODL-Datei angegeben ist.

// {5398DDAF-2C81-4DF8-91EF-EA3C66AE3467}
static const IID IID_ISetStoreFileOptionsDlg =
{ 0x5398ddaf, 0x2c81, 0x4df8, { 0x91, 0xef, 0xea, 0x3c, 0x66, 0xae, 0x34, 0x67 } };

BEGIN_INTERFACE_MAP(CSetStoreFileOptionsDlg, CDialog)
	INTERFACE_PART(CSetStoreFileOptionsDlg, IID_ISetStoreFileOptionsDlg, Dispatch)
END_INTERFACE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CSetStoreFileOptionsDlg 

void CSetStoreFileOptionsDlg::GetData(STOREFILEOPTIONS *lpsfoData,OPENFILEOPTIONS* lpofoData, CAttribs** lpsettings)
{
	*lpsfoData=sfoData;
	*lpofoData=ofoData;
	(*lpsettings)->Delete();
	delete (*lpsettings);
	*lpsettings = settings->Duplicate();
	settings->Delete();
	delete settings;
}

void CSetStoreFileOptionsDlg::SetData(STOREFILEOPTIONS *lpsfoData,OPENFILEOPTIONS* lpofoData, CAttribs* lpsettings)
{
	sfoData=*lpsfoData;
	ofoData=*lpofoData;
	settings = lpsettings->Duplicate();
}

void CSetStoreFileOptionsDlg::RefreshDlg()
{
	char	buffer[50];
	CString s;

	CheckDlgButton(IDC_RECLISTS,(settings->GetInt("output/avi/reclists"))?BST_CHECKED:BST_UNCHECKED);
	CheckDlgButton(IDC_OPENDML,(settings->GetInt("output/avi/opendml/on"))?BST_CHECKED:BST_UNCHECKED);
	CheckDlgButton(IDC_HAALIMODE,(settings->GetInt("output/avi/opendml/haalimode"))?BST_CHECKED:BST_UNCHECKED);
	m_MakeLegacyIndex.SetCheck(!!settings->GetInt("output/avi/legacyindex"));
	m_UseMaxFiles.SetCheck(!!sfoData.dwUseMaxFiles);
	m_UseNumbering.SetCheck(!sfoData.dwDontUseNumbering);
	CheckDlgButton(IDC_USEMAXSIZE,(sfoData.dwUseMaxFileSize)?BST_CHECKED:BST_UNCHECKED);
	CheckDlgButton(IDC_USESPLITPOINTLIST,(sfoData.dwUseManualSplitPoints)?BST_CHECKED:BST_UNCHECKED);
	m_STDI_RIFF.SetCheck(!!(settings->GetInt("output/avi/opendml/stdindex/pattern") == SIP_RIFF));
	m_STDI_auto.SetCheck(!!(settings->GetInt("output/avi/opendml/stdindex/pattern") == SIP_AUTO));
	m_STDI_Frames.SetCheck(!!(settings->GetInt("output/avi/opendml/stdindex/pattern") == SIP_FRAMES));
	CheckDlgButton(IDC_MP3CBRFRAMEMODE,(settings->GetInt("output/avi/mp3/cbr frame mode"))?BST_CHECKED:BST_UNCHECKED);

	itoa((int)settings->GetInt("output/avi/audio interleave/value"),buffer,10);
	SendDlgItemMessage(IDC_AUDIOINTERLEAVE,WM_SETTEXT,0,(LPARAM)buffer);
	itoa(sfoData.dwPreload,buffer,10);
	SendDlgItemMessage(IDC_PRELOAD,WM_SETTEXT,0,(LPARAM)buffer);
	itoa(sfoData.dwMaxFileSize,buffer,10);
	SendDlgItemMessage(IDC_MAXFILESIZE,WM_SETTEXT,0,(LPARAM)buffer);
	itoa(sfoData.dwFrames,buffer,10);
	SendDlgItemMessage(IDC_MAXFRAMES,WM_SETTEXT,0,(LPARAM)buffer);
	itoa(sfoData.dwMaxFiles,buffer,10);
	SendDlgItemMessage(IDC_MAXFILES,WM_SETTEXT,0,(LPARAM)buffer);
	itoa((int)settings->GetInt("output/avi/opendml/stdindex/interval"),buffer,10);
	SendDlgItemMessage(IDC_STDI_NBROFFRAMES,WM_SETTEXT,0,(LPARAM)buffer);
	itoa(sfoData.i1stTimecode,buffer,10);
	m_1stTimestamp.SetWindowText(buffer);
	itoa((int)settings->GetInt("output/avi/mp3/frames per chunk"), buffer, 10);
	m_MP3VBRFrameCount.SetWindowText(buffer);
	
	itoa((int)settings->GetInt("output/avi/ac3/frames per chunk"),buffer,10);
	m_AC3FrameCount.SetWindowText(buffer);
	m_AC3FrameCount_spin.SetPos((int)settings->GetInt("output/avi/ac3/frames per chunk"));

	itoa((int)settings->GetInt("output/avi/dts/frames per chunk"),buffer,10);
	m_DTSFrameCount.SetWindowText(buffer);

	m_DontWriteBlockSize.SetCheck(0);
	m_PrevClusterSize.SetCheck((int)settings->GetInt("output/mkv/clusters/prevclustersize"));
	m_ClusterPosition.SetCheck((int)settings->GetInt("output/mkv/clusters/position"));
	m_1st_Cluster_30sec.SetCheck((int)settings->GetInt("output/mkv/clusters/limit first"));
	
	m_Overlapped.SetCheck((int)settings->GetInt("output/general/overlapped"));
	m_DisplayWidth_Height.SetCheck((int)settings->GetInt("output/mkv/displaywidth_height"));//sfoData.mkv.iDisplayWidth_Height);
	m_ForceMKV10Compliance.SetCheck((int)settings->GetInt("output/mkv/force v1.0"));
	m_Logfile.SetCheck((int)settings->GetInt("output/general/logfile/on"));

	m_WriteCues.SetCheck((int)settings->GetInt("output/mkv/cues/on"));
	m_WriteCues_Video.SetCheck((int)settings->GetInt("output/mkv/cues/video/on"));
	m_WriteCues_Audio.SetCheck((int)settings->GetInt("output/mkv/cues/audio/on"));
	m_WriteCues_Audio_OnlyAudioOnly.SetCheck((int)settings->GetInt("output/mkv/cues/audio/only audio-only/on"));
	m_IndexClustersInSeekhead.SetCheck((int)settings->GetInt("output/mkv/clusters/index/on"));
	m_Randomize_Element_Order.SetCheck((int)settings->GetInt("output/mkv/randomize element order"));
	
	char c[20]; c[0]=0;
	itoa((int)settings->GetInt("output/mkv/TimecodeScale/mkv"),c,10);
	m_TimecodeScale_MKV.SetWindowText(c);
	itoa((int)settings->GetInt("output/mkv/TimecodeScale/mka"),c,10);
	m_TimecodeScale_MKA.SetWindowText(c);
	m_Write2ndCopyOfTracks.SetCheck((int)settings->GetInt("output/mkv/2nd Tracks"));

	m_UseInputCache.SetCheck((int)settings->GetInt("input/use cache"));

	itoa((int)settings->GetInt("output/avi/opendml/riff avi size"),c,10);
	m_RIFFAVISize.SetWindowText(c);


	if (sfoData.lpcNumbering)
	{
		SendDlgItemMessage(IDC_FORMAT,WM_SETTEXT,0,(LPARAM)sfoData.lpcNumbering);
	}
	if ((int)settings->GetInt("output/avi/audio interleave/unit") == AIU_KB)
	{
		CheckDlgButton(IDC_AI_KB,BST_CHECKED);
	}
	else
	{
		CheckDlgButton(IDC_AI_FRAMES,BST_CHECKED);
	}

// Input: AVI/MP3
	m_IMP3_CheckAlways.SetCheck((ofoData.dwFlags&SOFO_MP3_MASK)==SOFO_MP3_CHECKCBRALWAYS);
	m_IMP3_CheckNever.SetCheck((ofoData.dwFlags&SOFO_MP3_MASK)==SOFO_MP3_CHECKCBRNEVER);
	m_IMP3_Ask.SetCheck((ofoData.dwFlags&SOFO_MP3_MASK)==SOFO_MP3_CHECKCBRASK);
	m_IMP3_DispResult.SetCheck((ofoData.dwFlags&SOFO_MP3_RESULTDLG)==SOFO_MP3_RESULTDLG);
	m_IAVI_Try2RepairLargeChunks.SetCheck((ofoData.dwFlags&SOFO_AVI_TRYTOREPAIRLARGECHUNKS)==SOFO_AVI_TRYTOREPAIRLARGECHUNKS);
	m_IAVI_ForceMP3VBR.SetCheck((ofoData.dwFlags&SOFO_AVI_FORCEMP3VBR)==SOFO_AVI_FORCEMP3VBR);
	m_IAVI_RepairDX50.SetCheck((ofoData.dwFlags&SOFO_AVI_REPAIRDX50)==SOFO_AVI_REPAIRDX50);
	m_IAVI_IgnoreLargeChunks.SetCheck((ofoData.dwFlags&SOFO_AVI_IGNORELARGECHUNKS)==SOFO_AVI_IGNORELARGECHUNKS);
	itoa(ofoData.dwIgnoreSize,buffer,10);
	m_IAVI_IgnoreSize.SetWindowText(buffer);
	m_IM2F2_CRCCheck.SetCheck((ofoData.dwFlags&SOFO_M2F2_DOM2F2CRCCHECK)==SOFO_M2F2_DOM2F2CRCCHECK);
	m_IMKV_chFromFilenames.SetCheck((ofoData.dwFlags&SOFO_CH_FROMFILENAMES)==SOFO_CH_FROMFILENAMES);
	m_IMKV_chImport.SetCheck((ofoData.dwFlags&SOFO_CH_IMPORT)==SOFO_CH_IMPORT);
	m_Input_AllowUnbufferedRead.SetCheck((int)settings->GetInt("input/unbuffered"));
	m_Output_AllowUnbufferedWrite.SetCheck((int)settings->GetInt("output/general/unbuffered"));
	switch (settings->GetInt("output/mkv/floats/width")) {
		case 32: iCurrentFloatWidthIndex = 0; break;
		case 64: iCurrentFloatWidthIndex = 1; break;
		case 80: iCurrentFloatWidthIndex = 2; break;
	}

	UpdateClusterSize();
	UpdateLaceLength();
	UpdateAC3FrameCount();
	UpdateMKVAC3FrameCount();
	UpdateNumbering();
	UpdateMaxFiles();
	UpdateMaxFileSize();
	UpdateLaceDefition();
	UpdateVideoLaceSetting();
	UpdateForceV10();
	UpdateCueSettings();
	UpdateGeneralInput();
	UpdateFloatWidth();
	OnOpendml();
}

void CSetStoreFileOptionsDlg::UpdateData()
{
	char	buffer[50];
	CString s;
	
	settings->SetInt("output/avi/reclists", !!(IsDlgButtonChecked(IDC_RECLISTS)==BST_CHECKED));
	settings->SetInt("output/avi/opendml/on", !!(IsDlgButtonChecked(IDC_OPENDML)==BST_CHECKED));
	settings->SetInt("output/avi/opendml/haalimode", !!(IsDlgButtonChecked(IDC_HAALIMODE)==BST_CHECKED));
	settings->SetInt("output/avi/legacyindex", !!m_MakeLegacyIndex.GetCheck());
	sfoData.dwUseMaxFileSize=!!m_UseMaxSize.GetCheck();
	sfoData.dwUseManualSplitPoints=!!(IsDlgButtonChecked(IDC_USESPLITPOINTLIST)==BST_CHECKED);
	sfoData.dwUseMaxFiles=!!m_UseMaxFiles.GetCheck();
	sfoData.dwDontUseNumbering=!(IsDlgButtonChecked(IDC_USENUMBERING)==BST_CHECKED);
	settings->SetInt("output/avi/mp3/cbr frame mode", !!(IsDlgButtonChecked(IDC_MP3CBRFRAMEMODE)==BST_CHECKED));
	settings->SetInt("output/mkv/lacing/style", m_Lacestyle.GetCurSel());
	sfoData.iStdOutputFormat = m_CBStdOutputFormat.GetCurSel();

	SendDlgItemMessage(IDC_AUDIOINTERLEAVE,WM_GETTEXT,sizeof(buffer),(LPARAM)buffer);
	//sfoData.dwAudioInterleaveVal=atoi(buffer);
	settings->SetInt("output/avi/audio interleave/value", atoi(buffer));

	SendDlgItemMessage(IDC_PRELOAD,WM_GETTEXT,sizeof(buffer),(LPARAM)buffer);
	sfoData.dwPreload=atoi(buffer);
	SendDlgItemMessage(IDC_MAXFILESIZE,WM_GETTEXT,sizeof(buffer),(LPARAM)buffer);
	sfoData.dwMaxFileSize=atoi(buffer);
	SendDlgItemMessage(IDC_MAXFRAMES,WM_GETTEXT,sizeof(buffer),(LPARAM)buffer);
	sfoData.dwFrames=atoi(buffer);
	SendDlgItemMessage(IDC_MAXFILES,WM_GETTEXT,sizeof(buffer),(LPARAM)buffer);
	sfoData.dwMaxFiles=atoi(buffer);

	SendDlgItemMessage(IDC_FORMAT,WM_GETTEXT,sizeof(buffer),(LPARAM)buffer);
	if (sfoData.lpcNumbering)
	{
		free(sfoData.lpcNumbering);
	}
	sfoData.lpcNumbering=(char*)malloc(lstrlen(buffer)+1);
	lstrcpy(sfoData.lpcNumbering,buffer);

	if (IsDlgButtonChecked(IDC_AI_KB)==BST_CHECKED) {
		settings->SetInt("output/avi/audio interleave/unit", AIU_KB);
	} else {
		settings->SetInt("output/avi/audio interleave/unit", AIU_FRAME);
	}

	if (IsDlgButtonChecked(IDC_STDI_RIFF)==BST_CHECKED) {
		settings->SetInt("output/avi/opendml/stdindex/pattern", SIP_RIFF);
	} else
	if (IsDlgButtonChecked(IDC_STDI_FRAMES)==BST_CHECKED) {
		settings->SetInt("output/avi/opendml/stdindex/pattern", SIP_FRAMES);
	} else
	if (IsDlgButtonChecked(IDC_STDI_AUTO)==BST_CHECKED)	{
		settings->SetInt("output/avi/opendml/stdindex/pattern", SIP_AUTO);
	}

	SendDlgItemMessage(IDC_STDI_NBROFFRAMES,WM_GETTEXT,sizeof(buffer),(LPARAM)buffer);
	settings->SetInt("output/avi/opendml/stdindex/interval", atoi(buffer));
//sfoData.dwStdIndexInterval=atoi(buffer);

	m_1stTimestamp.GetWindowText(s);
	sfoData.i1stTimecode = atoi(s);
	m_AC3FrameCount.GetWindowText(s);
	settings->SetInt("output/avi/ac3/frames per chunk", atoi(s));
//	sfoData.mkv.iDontWriteBlockSizes = !!m_DontWriteBlockSize.GetCheck();
	settings->SetInt("output/mkv/clusters/prevclustersize", !!m_PrevClusterSize.GetCheck());
	settings->SetInt("output/mkv/clusters/position", !!m_ClusterPosition.GetCheck());
	settings->SetInt("output/mkv/clusters/limit first", !!m_1st_Cluster_30sec.GetCheck());
	settings->SetInt("output/general/overlapped", !!m_Overlapped.GetCheck());
	settings->SetInt("output/mkv/displaywidth_height", !!m_DisplayWidth_Height.GetCheck());

	ofoData.dwFlags&=~SOFO_MP3_MASK;
	ofoData.dwFlags&=~SOFO_AVI_MASK;
	ofoData.dwFlags&=~SOFO_M2F2_DOM2F2CRCCHECK;
	ofoData.dwFlags&=~SOFO_CH_MASK;
	ofoData.dwFlags|=(SOFO_MP3_CHECKCBRALWAYS)*(m_IMP3_CheckAlways.GetCheck());
	ofoData.dwFlags|=(SOFO_MP3_CHECKCBRASK)*(m_IMP3_Ask.GetCheck());
	ofoData.dwFlags|=(SOFO_MP3_CHECKCBRNEVER)*(m_IMP3_CheckNever.GetCheck());
	ofoData.dwFlags|=(SOFO_MP3_RESULTDLG)*(m_IMP3_DispResult.GetCheck());
	ofoData.dwFlags|=(SOFO_AVI_FORCEMP3VBR)*(m_IAVI_ForceMP3VBR.GetCheck());
	ofoData.dwFlags|=(SOFO_AVI_IGNORELARGECHUNKS)*(m_IAVI_IgnoreLargeChunks.GetCheck());
	ofoData.dwFlags|=(SOFO_AVI_REPAIRDX50)*(m_IAVI_RepairDX50.GetCheck());
	ofoData.dwFlags|=(SOFO_AVI_TRYTOREPAIRLARGECHUNKS)*(m_IAVI_Try2RepairLargeChunks.GetCheck());
	ofoData.dwFlags|=(SOFO_M2F2_DOM2F2CRCCHECK)*(m_IM2F2_CRCCheck.GetCheck());
	ofoData.dwFlags|=(SOFO_CH_FROMFILENAMES)*(m_IMKV_chFromFilenames.GetCheck());
	ofoData.dwFlags|=(SOFO_CH_IMPORT)*(m_IMKV_chImport.GetCheck());

	m_IAVI_IgnoreSize.GetWindowText(s);
	ofoData.dwIgnoreSize = atoi(s);

	settings->SetInt("output/mkv/force v1.0", m_ForceMKV10Compliance.GetCheck());
	settings->SetInt("output/mkv/cues/on", m_WriteCues.GetCheck());
	settings->SetInt("output/mkv/cues/video/on", m_WriteCues_Video.GetCheck());
	settings->SetInt("output/mkv/cues/audio/on", m_WriteCues_Audio.GetCheck());
	settings->SetInt("output/mkv/cues/audio/only audio-only/on", m_WriteCues_Audio_OnlyAudioOnly.GetCheck());
	settings->SetInt("output/mkv/2nd Tracks", m_Write2ndCopyOfTracks.GetCheck());
	settings->SetInt("output/mkv/clusters/index/on", m_IndexClustersInSeekhead.GetCheck());
	settings->SetInt("output/mkv/randomize element order", m_Randomize_Element_Order.GetCheck());
	settings->SetInt("output/general/logfile/on", m_Logfile.GetCheck());
	settings->SetInt("input/use cache", m_UseInputCache.GetCheck());
	settings->SetInt("input/unbuffered", m_Input_AllowUnbufferedRead.GetCheck());
	settings->SetInt("output/general/unbuffered", m_Output_AllowUnbufferedWrite.GetCheck());
	
	m_RIFFAVISize.GetWindowText(s);
	settings->SetInt("output/avi/opendml/riff avi size", min(2044,atoi(s)));

	m_MP3VBRFrameCount.GetWindowText(s);
	int j = atoi(s); if (!j) j=1;
	settings->SetInt("output/avi/mp3/frames per chunk", j);

	m_DTSFrameCount.GetWindowText(s);
	j = atoi(s); if (!j) j=2;
	settings->SetInt("output/avi/dts/frames per chunk", min(50,j));

	m_TimecodeScale_MKA.GetWindowText(s);
	settings->SetInt("output/mkv/TimecodeScale/mka", atoi(s));
	m_TimecodeScale_MKV.GetWindowText(s);
	settings->SetInt("output/mkv/TimecodeScale/mkv", atoi(s));
	j=iCurrentFloatWidthIndex;
	int b[] = {32, 64, 80};
	settings->SetInt("output/mkv/floats/width", b[j]);
}

void CSetStoreFileOptionsDlg::UpdateClusterSize()
{
	char cSize[20];
	FormatSize(cSize,settings->GetInt("output/mkv/clusters/size")*1024);
	m_ClusterSize.SetWindowText(cSize);

	sprintf(cSize,"%d ku",settings->GetInt("output/mkv/clusters/time")/1000);
	m_ClusterTime.SetWindowText(cSize);
	m_ClusterTime.SetTime((int)settings->GetInt("output/mkv/clusters/time"));

	m_1st_Cluster_30sec.EnableWindow(!!((int)settings->GetInt("output/mkv/clusters/time")>32767));

}

void CSetStoreFileOptionsDlg::UpdateLaceLength()
{
	char cSize[20]; cSize[0] = 0; int i;

	if (m_Lacestyle.GetCurSel()) {
		sprintf(cSize,"%d ms",i=(int)GetCurrentLaceDefinition()->GetInt("length"));
		if (i) {
			m_LaceSize.SetWindowText(cSize);
		} else {
			m_LaceSize.SetWindowText("off");
		}
	} else {
		m_LaceSize.SetWindowText("");
	}

	m_LaceSize_Spin.EnableWindow(m_Lacestyle.GetCurSel());
}

void CSetStoreFileOptionsDlg::UpdateAC3FrameCount()
{
	CString c;
	itoa((int)settings->GetInt("output/avi/ac3/frames per chunk"),c.GetBuffer(10),10);
	m_AC3FrameCount.SetAC3FrameCount((int)settings->GetInt("output/avi/ac3/frames per chunk"));
	m_AC3FrameCount.SetWindowText(c);
}

void CSetStoreFileOptionsDlg::UpdateFloatWidth()
{
	char* c[] = { "32", "64", "80" };

	if (m_ForceMKV10Compliance.GetCheck() && iCurrentFloatWidthIndex>1) iCurrentFloatWidthIndex = 1;
	m_FloatWidth.SetWindowText(c[iCurrentFloatWidthIndex]);
}

void CSetStoreFileOptionsDlg::UpdateVideoLaceSetting()
{
	CString c;
	
	if (m_Lacestyle.GetCurSel() == 3 && !m_ForceMKV10Compliance.GetCheck()) {
		m_LaceVideo.EnableWindow();
		itoa((int)settings->GetInt("output/mkv/lacing/video/frames"),c.GetBuffer(10),10);
		m_LaceVideo_Count.SetWindowText(c);
		m_LaceVideo_Count.EnableWindow();
		m_LaceVideo_Spin.EnableWindow();
	} else {
		m_LaceVideo_Count.SetWindowText("");
		m_LaceVideo_Count.EnableWindow(false);
		m_LaceVideo_Spin.EnableWindow(false);
		m_LaceVideo.EnableWindow(false);
	}

}

void CSetStoreFileOptionsDlg::UpdateMKVAC3FrameCount()
{
	CString c;
	//itoa(sfoData.mkv.iAC3FrameCount,c.GetBuffer(10),10);
	itoa((int)settings->GetInt("output/mkv/ac3/frames per block"),c.GetBuffer(10),10);
	m_MKVAC3FrameCount.SetFrameCount((int)settings->GetInt("output/mkv/ac3/frames per block"));//sfoData.mkv.iAC3FrameCount);
	m_MKVAC3FrameCount.SetWindowText(c);

}

void CSetStoreFileOptionsDlg::UpdateNumbering()
{
	m_Format.EnableWindow(!!m_UseNumbering.GetCheck());
}

void CSetStoreFileOptionsDlg::UpdateMaxFiles()
{
	m_MaxFiles.EnableWindow(!!m_UseMaxFiles.GetCheck());
}

void CSetStoreFileOptionsDlg::UpdateMaxFileSize()
{
	m_MaxFileSize.EnableWindow(!!m_UseMaxSize.GetCheck());
}

void CSetStoreFileOptionsDlg::UpdateLaceDefition()
{
	char s[100];
	s[0]=0;
	CAttribs* c = GetCurrentLaceDefinition();

	m_LaceDefinitionFormat.SetWindowText(cLaceDefinitionFormats[iCurrentLaceDefinition]);
	m_UseLaceDefinition.SetCheck(!!c->GetInt("use"));
	m_UseLaceDefinition.EnableWindow(!!iCurrentLaceDefinition);
	
	sprintf(s,"%I64d ms",c->GetInt("length"));

	m_LaceSize.SetWindowText(s);
	UpdateVideoLaceSetting();
}

void CSetStoreFileOptionsDlg::SaveCurrentLaceStyleDefinition()
{
	CAttribs* c = GetCurrentLaceDefinition();

	CString s;
	m_LaceSize.GetWindowText(s);
	c->SetInt("length",atoi(s));
	c->SetInt("use",!!m_UseLaceDefinition.GetCheck());
}

void CSetStoreFileOptionsDlg::OnOK() 
{
	// TODO: Zusätzliche Prüfung hier einfügen
	
	UpdateData();

	for (int i=0;i<page_count;i++) {
		pages[i]->DeleteAll();
		delete pages[i];
	}

	CDialog::OnOK();
}

void Show(CWnd* wnd, int i) 
{
	wnd->ShowWindow(i);
}

typedef struct
{
	CWnd* pParent;
	int	  iX;
	int	  iY;
} MOVEWINDOW;

void  Move(CWnd* cWnd, int _p)
{
	RECT	rect;
	MOVEWINDOW* p = (MOVEWINDOW*)_p;

	cWnd->GetWindowRect(&rect);

	p->pParent->ScreenToClient(&rect);
	rect.top-=p->iY;
	rect.bottom-=p->iY;
	rect.left-=p->iX;
	rect.right-=p->iX;
	cWnd->MoveWindow(&rect,true);
}

int z;
#define DoForPage(a,b,c) for (z=0;z<a->GetCount();z++) { c((CWnd*)a->At(z),(int)b); }

int CSetStoreFileOptionsDlg::ShowPage(int a) 
{
	for (int k=0;k<page_count;k++) { 
		if (a!=k) DoForPage(pages[k], SW_HIDE, Show) 
	}
	DoForPage(pages[a], SW_SHOW, Show);

	return 0;
}

BOOL CSetStoreFileOptionsDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Zusätzliche Initialisierung hier einfügen

	SetWindowText(LoadString(STR_SFO_TITLE));

	SendDlgItemMessage(IDOK,WM_SETTEXT,NULL,(LPARAM)LoadString(STR_GEN_OK));
	SendDlgItemMessage(IDCANCEL,WM_SETTEXT,NULL,(LPARAM)LoadString(STR_GEN_CANCEL));

	SendDlgItemMessage(IDC_SFO_OPTIONS,WM_SETTEXT,NULL,(LPARAM)LoadString(STR_SFO_OPTIONS));
	
	m_AVI.SetWindowText(LoadString(STR_SFO_O_AVISTRUCTURE));
	m_AVI2.SetWindowText(LoadString(STR_SFO_O_AVISTRUCTURE));
	m_Reclists.SetWindowText(LoadString(STR_SFO_OAS_CB_RECLISTS));
	m_OpenDML_settings.SetWindowText(LoadString(STR_SFO_OAS_S_ODML));
	m_OpenDML.SetWindowText(LoadString(STR_SFO_OAS_CB_ODML));

	m_StdIndicesPerStream_Label.SetWindowText(LoadString(STR_SFO_OAS_S_STDIDXPERSTREAM));
	m_STDI_RIFF.SetWindowText(LoadString(STR_SFO_OAS_RB_1PERRIFF));
	m_STDI_Frames.SetWindowText(LoadString(STR_SFO_OAS_RB_1PERXXXFRAMES));
	m_SFO_Frames.SetWindowText(LoadString(STR_SFO_OAS_S_FRAMES));
	m_STDI_auto.SetWindowText(LoadString(STR_SFO_OAS_RB_AUTO));
	m_MakeLegacyIndex.SetWindowText(LoadString(STR_SFO_OAS_CB_MAKELEGACYINDEX));

	m_Audiointerleave_Label.SetWindowText(LoadString(STR_SFO_OAS_S_AUDIOINTERLEAVE));
	m_AI_KB.SetWindowText(LoadString(STR_SFO_OAS_RB_KB));
	m_AI_Frames.SetWindowText(LoadString(STR_SFO_OAS_RB_FRAMES));
	m_Preload_Label.SetWindowText(LoadString(STR_SFO_OAS_S_PRELOAD));

	m_Split.SetWindowText(LoadString(STR_SFO_O_SPLIT));
	m_UseMaxSize.SetWindowText(LoadString(STR_SFO_OSP_CB_USEMAXSIZE));
	m_MaxSize_extended.SetWindowText(LoadString(STR_SFO_OSP_B_ADVANCED));
	m_UseNumbering.SetWindowText(LoadString(STR_SFO_OSP_CB_USENUMBERING));
	m_Frames_Label.SetWindowText(LoadString(STR_SFO_OSP_S_FRAMES));
	m_UseMaxFiles.SetWindowText(LoadString(STR_SFO_OSP_CB_USEMAXNBROFFILES));

	m_MP3CBRMode.SetWindowText(LoadString(STR_SFO_OAS_CB_MP3CBRFRAMEMODE));
	m_StdOutputFormat_Label.SetWindowText(LoadString(STR_SFO_OG_STDOFMT_LABEL));
	
	m_MKVOutputOptions.SetWindowText(LoadString(STR_SFO_O_MKVSTRUCTURE));
	m_MKVOutputOptions_2.SetWindowText(LoadString(STR_SFO_O_MKVSTRUCTURE2));
	m_Lace.SetWindowText(LoadString(STR_SFO_O_MKV_LACE));
	m_General.SetWindowText(LoadString(STR_SFO_O_GENERAL));
	m_BChapters.SetWindowText(LoadString(STR_SFO_O_CHAPTERS));
	m_ClusterSize_Label.SetWindowText(LoadString(STR_SFO_O_MKV_CLSIZE));
	m_ClusterTime_Label.SetWindowText(LoadString(STR_SFO_O_MKV_CLTIME));
	m_PrevClusterSize.SetWindowText(LoadString(STR_SFO_O_MKV_PREVSIZE));
	m_ClusterPosition.SetWindowText(LoadString(STR_SFO_O_MKV_POSITION));
	m_1st_Cluster_30sec.SetWindowText(LoadString(STR_SFO_O_MKV_LIMIT1STCL));
	m_DisplayWidth_Height.SetWindowText(LoadString(STR_SFO_O_MKV_DISPWH));
	m_ForceMKV10Compliance.SetWindowText(LoadString(STR_SFO_O_MKV_FORCE10));
	m_IndexClustersInSeekhead.SetWindowText(LoadString(STR_SFO_O_MKV_INDEXCLUSTERS));
	m_FloatWidth_Label.SetWindowText(LoadString(STR_SFO_O_MKV_FLOATWIDTH));
	m_Randomize_Element_Order.SetWindowText(LoadString(STR_SFO_O_MKV_RNDELEMORDER));
	m_Others.SetWindowText(LoadString(STR_SFO_O_MKV_OTHERS));

	m_UseInputCache.SetWindowText(LoadString(STR_SFO_O_AVOIDSEEKOPS));
	m_Input_General.SetWindowText(LoadString(STR_SFO_O_GENERAL));


	m_CBStdOutputFormat.AddString("AVI");
	m_CBStdOutputFormat.AddString("MKV");
	m_CBStdOutputFormat.SetCurSel(sfoData.iStdOutputFormat);
	m_AC3FrameCount_Label.SetWindowText(LoadString(STR_SFO_OAS_AC3CPF));

	DWORD s = (sfoData.bB0rk)?SW_SHOW:SW_HIDE;
	m_1stTimestamp.ShowWindow(s);
	m_1stTimestamp_Label.ShowWindow(s);
	m_DontWriteBlockSize.ShowWindow(s);

	m_Radio_AVI.SetWindowText(LoadString(STR_SFO_O_AVISTRUCTURE));
	m_Radio_MKV.SetWindowText(LoadString(STR_SFO_O_MKVSTRUCTURE_BTN));
	m_Radio_General.SetWindowText(LoadString(STR_SFO_O_GENERAL));
	m_Radio_Input_General.SetWindowText(LoadString(STR_SFO_O_GENERAL));

	m_IAVI_Label.SetWindowText(LoadString(STR_OFO_S_AVIFILES));
	m_IMP3_Label.SetWindowText(LoadString(STR_OFO_S_MP3FILES));
	m_IMP3_Ask.SetWindowText(LoadString(STR_OFO_MP3_RB_CHECKCBRASK));
	m_IMP3_CheckAlways.SetWindowText(LoadString(STR_OFO_MP3_RB_CHECKCBRALWAYS));
	m_IMP3_CheckNever.SetWindowText(LoadString(STR_OFO_MP3_RB_CHECKCBRNEVER));
	m_IMP3_DispResult.SetWindowText(LoadString(STR_OFO_MP3_CB_RESULTDLG));

	m_IAVI_IgnoreLargeChunks.SetWindowText(LoadString(STR_OFO_AVI_CB_MAXCHUNKSIZE));
	m_IAVI_Try2RepairLargeChunks.SetWindowText(LoadString(STR_OFO_AVI_CB_TRYTOREPAIR));
	m_IAVI_ForceMP3VBR.SetWindowText(LoadString(STR_OFO_AVI_CB_FORCEMP3VBR));
	m_IAVI_RepairDX50.SetWindowText(LoadString(STR_OFO_AVI_CB_REPAIRDX50));

	m_IM2F2_Label.SetWindowText(LoadString(STR_OFO_S_M2F2FILES));
	m_IM2F2_CRCCheck.SetWindowText(LoadString(STR_OFO_M2F2_CB_CRCCHECK));

	m_IMKV_chFromFilenames.SetWindowText(LoadString(STR_OFO_CH_FROMFILENAMES));
	m_IMKV_chImport.SetWindowText(LoadString(STR_OFO_CH_IMPORT));
	m_IMKV_Chapters_Label.SetWindowText(LoadString(STR_OFO_CHAPTERS));
	m_IMKV_Label.SetWindowText(LoadString(STR_OFO_MKV_LABEL));

	m_WriteCues.SetWindowText(LoadString(STR_SFO_O_MKV_WRITECUES));
	m_WriteCues_Audio_OnlyAudioOnly.SetWindowText(LoadString(STR_SFO_O_MKV_CUES_OAO));
	m_Write2ndCopyOfTracks.SetWindowText(LoadString(STR_SFO_O_MKV_2TRACKS));

	m_LaceVideo.SetCheck((int)settings->GetInt("output/mkv/lacing/video/on"));


	m_Lacestyle.AddString("off");
	m_Lacestyle.AddString("XIPH");
	m_Lacestyle.AddString("EBML / fixed");
	m_Lacestyle.AddString("Auto");
	m_Lacestyle.SetCurSel(max(0,(int)settings->GetInt("output/mkv/lacing/style")));

	RefreshDlg();

	// general output settings
	pages[0] = new CDynIntArray;
	CWnd* page_general [] = { 
		&m_General, &m_BChapters, &m_StdOutputFormat_Label,
		&m_CBStdOutputFormat, &m_Split, &m_UseMaxFiles, &m_UseMaxFiles,
		&m_UseMaxSize, &m_UseNumbering, &m_UseSplitPoints, &m_MaxFiles,
		&m_MaxFileSize, &m_MaxSize_extended, &m_MaxFrames, &m_Format,
		&m_Frames_Label, &m_Overlapped, &m_Logfile, &m_Output_AllowUnbufferedWrite
	};
	pages[0]->Insert((int*)page_general, sizeof(page_general)/sizeof(int));

	// AVI output settings
	pages[1] = new CDynIntArray;
	CWnd* page_AVI [] = {
		&m_AVI, &m_OpenDML, &m_OpenDML_settings, &m_Reclists,
		&m_MP3CBRMode, &m_STDI_auto, &m_STDI_Frames, &m_STDI_NumberOfFrames,
		&m_STDI_RIFF, &m_SFO_Frames, &m_StdIndicesPerStream_Label, &m_MakeLegacyIndex,
		&m_Audiointerleave_Label, &m_Audiointerleave, &m_AI_Frames, &m_AI_KB,
		&m_Preload, &m_Preload_Label, &m_RIFFAVISize, &m_RIFFAVISize_Label,
		&m_RIFFAVISize_Unit, &m_AVI_Page1, &m_AVI_Page2, &m_Haalimode };
	pages[1]->Insert((int*)page_AVI, sizeof(page_AVI)/sizeof(int));

	// AVI output settings page 2
	pages[7] = new CDynIntArray;
	CWnd* page_AVI2 [] = {
		&m_AVI2, &m_AC3FrameCount, &m_AC3FrameCount_Label, &m_AC3FrameCount_spin,
		&m_MP3VBRFrameCount, &m_MP3VBRFrameCount_Label, &m_DTSFrameCount,
		&m_DTSFrameCount_Label
	};
	pages[7]->Insert((int*)page_AVI2, sizeof(page_AVI2)/sizeof(int));

	// MKV output settings page 1
	pages[2] = new CDynIntArray;
	CWnd* pages_MKV [] = {
		&m_MKVOutputOptions, &m_Lace, &m_LaceSize, &m_LaceSize_Spin,
		&m_ClusterPosition, &m_ClusterSize, &m_ClusterSize_Label, &m_ClusterSize_Spin,
		&m_ClusterTime, &m_ClusterTime_Label, &m_ClusterTime_Spin, &m_PrevClusterSize,
		&m_1st_Cluster_30sec, &m_Lacestyle, &m_LaceDefinitionFormat,
		&m_LaceDefinitionFormat_Spin, &m_UseLaceDefinition, &m_LaceVideo, &m_LaceVideo_Count,
		&m_LaceVideo_Spin, &m_ForceMKV10Compliance, &m_MKV_Lacing,
		&m_MKV_Cluster, &m_MKV_Page1, &m_MKV_Page2, &m_IndexClustersInSeekhead };
	pages[2]->Insert((int*)pages_MKV, sizeof(pages_MKV)/sizeof(int));

	// MKV output settings page 2
	pages[5] = new CDynIntArray;
	CWnd* pages_MKV2 [] = {
		&m_MKVOutputOptions_2, &m_WriteCues, &m_WriteCues_Audio,
		&m_WriteCues_Audio_OnlyAudioOnly, &m_WriteCues_Video, &m_MKV_Cues,
		&m_Others, &m_Write2ndCopyOfTracks, &m_DisplayWidth_Height, &m_TimecodeScale_MKA,
		&m_TimecodeScale_MKA_Label, &m_TimecodeScale_MKV, &m_TimecodeScale_MKV_Label,
		&m_TimecodeScale, &m_FloatWidth, &m_FloatWidth_Label, &m_FloatWidth_Spin,
		&m_Randomize_Element_Order
	};
	pages[5]->Insert((int*)pages_MKV2, sizeof(pages_MKV2)/sizeof(int));

	// allow hackish settings to write incompliant files
	if (sfoData.bB0rk) {
		pages[2]->Insert((int)&m_1stTimestamp);
		pages[2]->Insert((int)&m_1stTimestamp_Label);
		pages[2]->Insert((int)&m_DontWriteBlockSize);
		pages[2]->Insert((int)&m_MKV_AC3FramesPerBlock_Label);
		pages[2]->Insert((int)&m_MKVAC3FrameCount);
		pages[2]->Insert((int)&m_MKVAC3FrameCount_Spin);
	}

	// AVI input settings
	pages[3] = new CDynIntArray;
	CWnd* pages_Open_AVIMP3 [] = {
		&m_IAVI_Label, &m_IMP3_Label, &m_IMP3_Ask, &m_IMP3_CheckAlways,
		&m_IMP3_CheckNever, &m_IMP3_DispResult, &m_IAVI_ForceMP3VBR, &m_IAVI_IgnoreLargeChunks,
		&m_IAVI_IgnoreSize, &m_IAVI_RepairDX50, &m_IAVI_Try2RepairLargeChunks, &m_IM2F2_CRCCheck,
		&m_IM2F2_Label};
	pages[3]->Insert((int*)pages_Open_AVIMP3,sizeof(pages_Open_AVIMP3)/sizeof(int));


	// MKV input settings
	pages[4] = new CDynIntArray;
	CWnd* pages_Open_MKV [] = {
		&m_IMKV_Label, &m_IMKV_Chapters_Label, &m_IMKV_chImport };
	pages[4]->Insert((int*)pages_Open_MKV,sizeof(pages_Open_MKV)/sizeof(int));

	// General input settings
	pages[6] = new CDynIntArray;
	CWnd* pages_Input_General [] = {
		&m_Input_General, &m_IMKV_chFromFilenames, &m_UseInputCache, &m_Input_AllowUnbufferedRead
	};
	pages[6]->Insert((int*)pages_Input_General, sizeof(pages_Input_General)/sizeof(int));

	CWnd* windows[] = { &m_General, &m_AVI, &m_MKVOutputOptions, &m_IAVI_Label, &m_IMKV_Label,
						&m_MKVOutputOptions_2, &m_Input_General, &m_AVI2 };

	RECT r1,r2;
	for (int i=0;i<sizeof(windows)/sizeof(windows[0]);i++) {
		windows[i]->GetWindowRect(&r1);
		m_Options.GetWindowRect(&r2);
		ScreenToClient(&r1);
		ScreenToClient(&r2);
		int w1 = r1.right-r1.left+1;
		int w2 = r2.right-r2.left+1;
		int h1 = r1.bottom-r1.top;
		int h2 = r2.bottom-r2.top;
	
		MOVEWINDOW m;
		m.iX = r1.left-r2.left-(w2-w1)/2;
		m.iY = r1.top-r2.top-2*(h2-h1)/3;
		m.pParent = this;
		DoForPage(pages[i],&m,Move);
	}
	windows[2]->GetWindowRect(&r1);
	m_Radio_AVI.GetWindowRect(&r2);

	r1.right+=80 + r2.right-r2.left;
	r1.bottom+=75;

	int j = GetSystemMetrics(SM_CXSCREEN)/2;
	int k = (r1.right-r1.left)/2;
	r1.left=(j-k);
	r1.right=(j+k);
	MoveWindow(&r1);
	
	pages[5]->Insert((int)&m_MKV_Page1);
	pages[5]->Insert((int)&m_MKV_Page2);

	pages[7]->Insert((int)&m_AVI_Page1);
	pages[7]->Insert((int)&m_AVI_Page2);

	switch (sfoData.iActiveButton & 0x7) {
		case 0: m_Radio_General.SetCheck(1); OnRadioGeneral(); break;
		case 1: m_Radio_AVI.SetCheck(1); OnRadioAvi(); break;
		case 2: m_Radio_MKV.SetCheck(1); OnRadioMkv(); break;
		case 3: m_Radio_Input_AVIMP3.SetCheck(1); OnRadioInputAvimp3(); break;
		case 4: m_Radio_Input_MKV.SetCheck(1); OnRadioInputMkv(); break;
		case 6: m_Radio_Input_General.SetCheck(1); OnRadioInputGeneral(); break;
	}

//	m_Overlapped.EnableWindow(0);


	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX-Eigenschaftenseiten sollten FALSE zurückgeben
}

void CSetStoreFileOptionsDlg::OnMaxsizeExtended() 
{
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen
	CSplitPointsDlg*	dlg;
	CString				cStr;
	CString				cstrError;
	int	iIndex;

	cstrError=LoadString(IDS_ERROR);
	if (sfoData.cdMain->SendDlgItemMessage(IDC_AVAILABLEVIDEOSTREAMS,LB_GETCOUNT,0,0)==0)
	{
		cStr=LoadString(IDS_NOVIDEOSOURCE);
	//	MessageBox(cStr,cstrError,MB_ICONERROR | MB_OK);
	//	return;
	};

	dlg=new CSplitPointsDlg;

	dlg->Load(sfoData.split_points);
	iIndex=((iIndex=sfoData.cdMain->SendDlgItemMessage(IDC_AVAILABLEVIDEOSTREAMS,LB_GETCURSEL))==LB_ERR)?0:iIndex;
	dlg->SetVideoSource((VIDEOSOURCE*)sfoData.cdMain->SendDlgItemMessage(IDC_AVAILABLEVIDEOSTREAMS,LB_GETITEMDATA,iIndex,0));
	if (dlg->DoModal()==IDOK)
	{
		dlg->GetData(sfoData.split_points);
		CheckDlgButton(IDC_USESPLITPOINTLIST,BST_CHECKED);
	}
	delete dlg;	
}

void CSetStoreFileOptionsDlg::OnGenChapters() 
{
	CChapterDlg* ccdlg;
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen

	ccdlg = new CChapterDlg;
	ccdlg->SetChapters(sfoData.chapters);
	ccdlg->DoModal();

	delete ccdlg;
	
}

void CSetStoreFileOptionsDlg::OnDeltaposAc3framecountSpin(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen
	
	CString c;
	int  iPos;

	m_AC3FrameCount.GetWindowText(c);
	iPos = atoi(c);
	iPos = min(15,max(1,iPos-pNMUpDown->iDelta));
	//sfoData.iAC3FramesPerChunk = iPos;
	settings->SetInt("output/avi/ac3/frames per chunk", iPos);
	
	UpdateAC3FrameCount();

	*pResult = 0;
}

void CSetStoreFileOptionsDlg::OnDeltaposClustersizeSpin(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen
	
	int d = pNMUpDown->iDelta;

	if (d<0) {
		while (d && (settings->GetInt("output/mkv/clusters/size"))/*sfoData.mkv.iClusterSize*/ < 32768) {
			settings->SetInt("output/mkv/clusters/size", 2*settings->GetInt("output/mkv/clusters/size"));
//			sfoData.mkv.iClusterSize*=2;
			d++;
		}
	} else {
		while (d && /*sfoData.mkv.iClusterSize*/ (settings->GetInt("output/mkv/clusters/size")) > 4) {
			settings->SetInt("output/mkv/clusters/size", settings->GetInt("output/mkv/clusters/size")/2);
//			sfoData.mkv.iClusterSize/=2;
			d--;
		}
	}

	UpdateClusterSize();

	*pResult = 0;
}

void CSetStoreFileOptionsDlg::OnOpendml() 
{
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen
	
	int i=m_OpenDML.GetCheck();
	int j=m_MakeLegacyIndex.GetCheck();

	m_STDI_RIFF.EnableWindow(i);
	m_STDI_NumberOfFrames.EnableWindow(i);
	m_STDI_Frames.EnableWindow(i);
	m_STDI_auto.EnableWindow(i);
	m_SFO_Frames.EnableWindow(i);
	m_MakeLegacyIndex.EnableWindow(i);
	m_StdIndicesPerStream_Label.EnableWindow(i);

	UpdateMakeLegacyIndex();
}

void CSetStoreFileOptionsDlg::OnDeltaposMkvLacesizeSpin(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen

	int d = pNMUpDown->iDelta;
	int j = (int)GetCurrentLaceDefinition()->GetInt("length");
	int* i = &j;
	
	if (d<0) {
		if (*i < 100) {
			*i+=50;
		} else
		if (*i < 500) {
			*i+=100;
		} else
		if (*i < 1000) {
			*i+=250;
		}
	}
	if (d>0) {
		if (*i > 500) {
			*i-=250;
		} else
		if (*i > 100) {
			*i-=100;
		} else 
		if (*i > 0) {
			*i-=50;
		}
	}
	
	GetCurrentLaceDefinition()->SetInt("length",j);
	UpdateLaceLength();

	*pResult = 0;
}

void CSetStoreFileOptionsDlg::OnMkvLace() 
{
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen

	UpdateLaceLength();
}

void CSetStoreFileOptionsDlg::OnDeltaposClustertimeSpin(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen

	int d = pNMUpDown->iDelta;

	int _i = (int)settings->GetInt("output/mkv/clusters/time");
	int* i = &_i;
	
	if (d<0) {
		if (*i>=30000) {
			if (*i<60000 && !m_ForceMKV10Compliance.GetCheck()) *i += 15000;
		} else
		if (*i>=5000) {
			*i += 5000;
		} else *i += 1000;
	} else {
		if (*i<=5000) {
			if (*i>1000) *i -= 1000;
		} else 
		if (*i<=30000) {
			*i -= 5000;
		} else *i -= 15000;
	}

	settings->SetInt("output/mkv/clusters/time", _i);
	UpdateClusterSize();

	*pResult = 0;
}

void CSetStoreFileOptionsDlg::OnUsenumbering() 
{
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen
	UpdateNumbering();
}

void CSetStoreFileOptionsDlg::OnUsemaxsize() 
{
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen
	UpdateMaxFileSize();	
}

void CSetStoreFileOptionsDlg::OnUsemaxfiles() 
{
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen
	UpdateMaxFiles();
}

void CSetStoreFileOptionsDlg::OnClose() 
{
		// TODO: Code für die Behandlungsroutine für Nachrichten hier einfügen und/oder Standard aufrufen

	for (int i=0;i<3;i++) {
		pages[i]->DeleteAll();
		delete pages[i];
	}

	settings->Delete();
	delete settings;
	CDialog::OnClose();
}

void CSetStoreFileOptionsDlg::OnRadioGeneral() 
{
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen
	ShowPage(0);
	sfoData.iActiveButton &=~7;
}

void CSetStoreFileOptionsDlg::OnRadioAvi() 
{
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen
	ShowPage(1);
	sfoData.iActiveButton &=~7;
	sfoData.iActiveButton |= 1;

	switch (sfoData.iActiveButton & 0x10)
	{
		case 0x0: m_AVI_Page1.SetCheck(1); OnAviPage1(); break;
		case 0x10: m_AVI_Page2.SetCheck(1); OnAviPage2(); break;
	}
}

void CSetStoreFileOptionsDlg::OnRadioMkv() 
{
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen
	ShowPage(2);
	sfoData.iActiveButton &=~7;
	sfoData.iActiveButton |= 2;

	switch (sfoData.iActiveButton & 0x08)
	{
		case 0x0: m_MKV_Page1.SetCheck(1); OnMkvPage1(); break;
		case 0x8: m_MKV_Page2.SetCheck(1); OnMkvPage2(); break;
	}
}

void CSetStoreFileOptionsDlg::OnRadioInputAvimp3() 
{
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen
	ShowPage(3);
	sfoData.iActiveButton &=~7;
	sfoData.iActiveButton |= 3;
	
}

void CSetStoreFileOptionsDlg::OnRadioInputMkv() 
{
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen
	ShowPage(4);
	sfoData.iActiveButton &=~7;
	sfoData.iActiveButton |= 4;
	
}


void CSetStoreFileOptionsDlg::OnChangeMkvAc3framesperblock() 
{
	// TODO: Wenn dies ein RICHEDIT-Steuerelement ist, sendet das Steuerelement diese

	// Benachrichtigung nicht, bevor Sie nicht die Funktion CDialog::OnInitDialog()

	// überschreiben und CRichEditCrtl().SetEventMask() aufrufen, wobei

	// eine ODER-Operation mit dem Attribut ENM_CHANGE und der Maske erfolgt.

	

	// TODO: Fügen Sie hier Ihren Code für die Benachrichtigungsbehandlungsroutine des Steuerelements hinzu

	
}


void CSetStoreFileOptionsDlg::OnDeltaposMkvAc3framesperblockSpin(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen
	
	CString c;
	int  iPos;

	m_MKVAC3FrameCount.GetWindowText(c);
	iPos = atoi(c);
	iPos = min(15,max(1,iPos-pNMUpDown->iDelta));
	//sfoData.mkv.iAC3FrameCount = iPos;
	settings->SetInt("output/mkv/ac3/frames per block", iPos);
	
	UpdateMKVAC3FrameCount();

	*pResult = 0;
}

void CSetStoreFileOptionsDlg::OnSelchangeLacestyle() 
{
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen
	UpdateLaceDefition();
	UpdateLaceLength();	
}

CAttribs* CSetStoreFileOptionsDlg::GetCurrentLaceDefinition()
{
	CAttribs* c = settings->GetAttr("output/mkv/lacing")->GetAttr((char*)cLaceDefinitionFormats[iCurrentLaceDefinition]);

	return c;
}


void CSetStoreFileOptionsDlg::OnDeltaposLaceexceptionformatspin(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen
	int iDelta = pNMUpDown->iDelta;

	if (iDelta < 0 && iCurrentLaceDefinition > 0) iCurrentLaceDefinition+=iDelta;
	if (iDelta > 0 && iCurrentLaceDefinition < iLaceDefinitionCount-1) iCurrentLaceDefinition+=iDelta;

	UpdateLaceDefition();
	UpdateLaceLength();

	*pResult = 0;
}

void CSetStoreFileOptionsDlg::OnCancel() 
{
	// TODO: Zusätzlichen Bereinigungscode hier einfügen
	
	settings->Delete();
	delete settings;

	CDialog::OnCancel();
}

void CSetStoreFileOptionsDlg::OnUselacingexception() 
{
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen
	SaveCurrentLaceStyleDefinition();
}

void CSetStoreFileOptionsDlg::OnLacevideo() 
{
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen
	int j;
	CAttribs* c = settings->GetAttr("output")->GetAttr("mkv")->
		GetAttr("lacing")->GetAttr("video");

	c->SetInt("on", j=m_LaceVideo.GetCheck());

	UpdateVideoLaceSetting();
	
}

void CSetStoreFileOptionsDlg::OnDeltaposLacevideoSpin(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen
	
	int iDelta = pNMUpDown->iDelta;

	int i = (int)settings->GetInt("output/mkv/lacing/video/frames");
	if (iDelta > 0) {
		if (i>2) i--;
	} else {
		if (i<5) i++;
	}
	settings->SetInt("output/mkv/lacing/video/frames", i);

	UpdateVideoLaceSetting();

	*pResult = 0;
}

void CSetStoreFileOptionsDlg::UpdateForceV10() 
{
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen
	bool b = (bool)!!m_ForceMKV10Compliance.GetCheck();

	UpdateVideoLaceSetting();	
	UpdateFloatWidth();
	int j = (int)settings->GetInt("output/mkv/clusters/time");
	if (b) {
		if (j > 30000) {
			settings->SetInt("output/mkv/clusters/time", 30000);
//			sfoData.mkv.iClusterTime = 30000;
			UpdateClusterSize();
		}
	}
}

void CSetStoreFileOptionsDlg::UpdateCueSettings()
{
	if (!m_WriteCues.GetCheck()) {
		m_WriteCues_Audio.EnableWindow(0);
		m_WriteCues_Video.EnableWindow(0);
		m_WriteCues_Audio_OnlyAudioOnly.EnableWindow(0);
	} else {
		m_WriteCues_Audio.EnableWindow(1);
		m_WriteCues_Video.EnableWindow(1);
		if (!m_WriteCues_Audio.GetCheck()) {
			m_WriteCues_Audio_OnlyAudioOnly.EnableWindow(0);
		} else {
			m_WriteCues_Audio_OnlyAudioOnly.EnableWindow(1);
		}
	}
}

void CSetStoreFileOptionsDlg::UpdateMakeLegacyIndex()
{
	bool b = false;

	if (m_OpenDML.GetCheck() && m_MakeLegacyIndex.GetCheck()) {
		b = true;
	}

/*	m_RIFFAVISize.EnableWindow(b);
	m_RIFFAVISize_Label.EnableWindow(b);
	m_RIFFAVISize_Unit.EnableWindow(b);
*/
	m_Haalimode.EnableWindow(m_OpenDML.GetCheck() && !m_MakeLegacyIndex.GetCheck());

}

void CSetStoreFileOptionsDlg::OnForcemkv10() 
{
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen
	UpdateForceV10();
}

void CSetStoreFileOptionsDlg::OnWritecues() 
{
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen
	UpdateCueSettings();	
}

void CSetStoreFileOptionsDlg::OnWritecuesVideo() 
{
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen
	UpdateCueSettings();	
}

void CSetStoreFileOptionsDlg::OnWritecuesAudio() 
{
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen
	UpdateCueSettings();	
}

void CSetStoreFileOptionsDlg::OnWritecuesOnlyaudioonly() 
{
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen
	UpdateCueSettings();	
}

void CSetStoreFileOptionsDlg::OnMkvPage1() 
{
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen

	// Button 2, Subbutton 1
	ShowPage(2);
	m_MKV_Page2.SetCheck(0);
	sfoData.iActiveButton &=~0x08;
	sfoData.iActiveButton |= 2;   // button 2
	sfoData.iActiveButton |= 0;   // subbutton 1
}

void CSetStoreFileOptionsDlg::OnMkvPage2() 
{
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen

	// Button 2, Subbutton 2
	ShowPage(5);
	m_MKV_Page1.SetCheck(0);
	sfoData.iActiveButton &=~0x0F;
	sfoData.iActiveButton |= 2;   // button 2
	sfoData.iActiveButton |= 8;   // subbutton 2
	
}

void CSetStoreFileOptionsDlg::OnMakeidx1() 
{
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen
	UpdateMakeLegacyIndex();	
}

void CSetStoreFileOptionsDlg::OnRadioInputGeneral() 
{
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen
	ShowPage(6);
	sfoData.iActiveButton &=~7;
	sfoData.iActiveButton |= 6;

}

void CSetStoreFileOptionsDlg::OnSfoAllowunbufferedread() 
{
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen
	UpdateGeneralInput();	
}

void CSetStoreFileOptionsDlg::UpdateGeneralInput()
{
	m_UseInputCache.EnableWindow(!m_Input_AllowUnbufferedRead.GetCheck());

	if (m_Input_AllowUnbufferedRead.GetCheck()) {
		m_UseInputCache.SetCheck(1);
	}
}

void CSetStoreFileOptionsDlg::OnAviPage1() 
{
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen
	ShowPage(1);	
	m_MKV_Page2.SetCheck(0);
	sfoData.iActiveButton &=~0x17;
	sfoData.iActiveButton |= 1;    // button 1
	sfoData.iActiveButton |= 0;    // subbutton 1
}

void CSetStoreFileOptionsDlg::OnAviPage2() 
{
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen
	ShowPage(7);	
	sfoData.iActiveButton &=~0x17;
	sfoData.iActiveButton |= 0x01; // button 1
	sfoData.iActiveButton |= 0x10; // subbutton 2

	m_MKV_Page1.SetCheck(0);
}

void CSetStoreFileOptionsDlg::OnChangeMp3vbrframecount() 
{
	CString s; m_MP3VBRFrameCount.GetWindowText(s);

	if (atoi(s)>50) {
		m_MP3VBRFrameCount.SetWindowText("50");
	}
}

void CSetStoreFileOptionsDlg::OnDeltaposFloatwidthSpin(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen
	int iDelta = pNMUpDown->iDelta;

	if (iDelta < 0 && iCurrentFloatWidthIndex < 1) {
		iCurrentFloatWidthIndex -= iDelta;
	} else if (iDelta < 0 && iCurrentFloatWidthIndex < 2 && !m_ForceMKV10Compliance.GetCheck() && sfoData.bB0rk) {
		iCurrentFloatWidthIndex -= iDelta;
	}
	if (iDelta > 0 && iCurrentFloatWidthIndex > 0) iCurrentFloatWidthIndex -= iDelta;
	
	UpdateFloatWidth();
	*pResult = 0;
}
