// AVIMux_GUIDlg.cpp : Implementierungsdatei
//


#include "stdafx.h"
#include "AudioSource.h"
#include "AVIFile.h"
#include "AVIMux_GUI.h"
#include "AVIMux_GUIDlg.h"
#include "..\BaseStreams.h"
#include "..\Buffers.h"
#include "ChapterDlg.h"
#include "ConfigScripts.h"
#include "debug.h"
#include "EnhancedListBox.h"
#include "FormatText.h"
#include "IncResource.h"
#include "MessageLists.h"
#include "Muxing.h"
#include "mode2form2reader.h"
#include "ProtocolListCtrl.h"
#include "ProgressList.h"
#include "RIFFChunkTreeDlg.h"
#include "SetStoreFileOptionsDlg.h"
#include "Silence.h"
#include "sourcefilelistbox.h"
#include "streams.h"
#include "strings.h"
#include "SubTitles.h"
#include "SubtitlesListBox.h"
#include "videosource.h"
#include "videosourcelistbox.h"
#include "WAVFile.h"
#include "..\matroska.h"
#include "Languages.h"
#include "TextFiles.h"
#include "Trees.h"
#include "AudioSourceTree.h"
#include "..\XML.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg-Dialogfeld für Anwendungsbefehl "Info"

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialogfelddaten
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	CStatic	m_Version;
	//}}AFX_DATA

	// Vom Klassenassistenten generierte Überladungen virtueller Funktionen
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV-Unterstützung
	//}}AFX_VIRTUAL

// Implementierung
protected:
	//{{AFX_MSG(CAboutDlg)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	DDX_Control(pDX, IDC_ABOUTBOX_LABEL, m_Version);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAVIMux_GUIDlg Dialogfeld

CAVIMux_GUIDlg::CAVIMux_GUIDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAVIMux_GUIDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAVIMux_GUIDlg)
	//}}AFX_DATA_INIT
	// Beachten Sie, dass LoadIcon unter Win32 keinen nachfolgenden DestroyIcon-Aufruf benötigt
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CAVIMux_GUIDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAVIMux_GUIDlg)
	DDX_Control(pDX, IDC_STATUSLINE, m_StatusLine);
	DDX_Control(pDX, IDC_STREAM_LNG, m_Stream_Lng);
	DDX_Control(pDX, IDC_OUTPUTRESOLUTION_LABEL, m_OutputResolution_Label);
	DDX_Control(pDX, IDC_OUTPUTRESOLUTION, m_OutputResolution);
	DDX_Control(pDX, IDC_AUDIOTREE, m_AudioTree);
	DDX_Control(pDX, IDC_TITLELABEL, m_Title_Label);
	DDX_Control(pDX, IDC_TITLE, m_Title);
	DDX_Control(pDX, IDC_VIDEOSTRETCHFACTORLABEL, m_VideoStretchFactor_Label);
	DDX_Control(pDX, IDC_VIDEOSTRETCHFACTOR, m_VideoStretchFactor);
	DDX_Control(pDX, IDC_PROGRESS_LIST, m_Progress_List);
	DDX_Control(pDX, IDC_PROTOCOL, m_Protocol);
	DDX_Control(pDX, IDC_OUTPUTOPTIONS, m_Output_Options_Button);
	DDX_Control(pDX, IDC_START, m_Start_Button);
	DDX_Control(pDX, IDS_MAIN_OPENFILES, m_Open_Files_Label);
	DDX_Control(pDX, IDC_FRAMES, m_Prg_Frames);
	DDX_Control(pDX, IDS_MAIN_PRG_FRAMES, m_Prg_Frames_Label);
	DDX_Control(pDX, IDC_DESTFILE, m_Prg_Dest_File);
	DDX_Control(pDX, IDS_MAIN_PRG_DESTFILE, m_Prg_Dest_File_Label);
	DDX_Control(pDX, IDC_LEGACYPROGRESS, m_Prg_Legidx_Progress);
	DDX_Control(pDX, IDS_MAIN_PROGRESS, m_Progress_Group);
	DDX_Control(pDX, IDC_ADDAVILIST, m_Add_Video_Source);
	DDX_Control(pDX, IDC_PROGCTRL, m_Prg_Progress);
	DDX_Control(pDX, IDS_MAIN_PRG_LEGIDX, m_Prg_Legidx_Label);
	DDX_Control(pDX, IDS_MAIN_PRG_PROGRESS, m_Prg_Progress_Label);
	DDX_Control(pDX, IDC_DEFAULT_AUDIO_NUMBER, m_Default_Audio);
	DDX_Control(pDX, IDC_DEFAULT_AUDIO, m_Default_Audio_Label);
	DDX_Control(pDX, IDC_ALL_SUBTITLES, m_All_Subtitles);
	DDX_Control(pDX, IDC_ALL_AUDIO, m_All_Audio);
	DDX_Control(pDX, IDC_NO_SUBTITLES, m_No_Subtitles);
	DDX_Control(pDX, IDC_NO_AUDIO, m_No_Audio);
	DDX_Control(pDX, IDS_MAIN_VIDEO, m_Video_Headline);
	DDX_Control(pDX, IDS_MAIN_SUBTITLES, m_Subtitles_Headline);
	DDX_Control(pDX, IDS_MAIN_AVAILABLESTREAMS, m_AvailableStreams_Header);
	DDX_Control(pDX, IDS_MAIN_AUDIO, m_Audio_Headline);
	DDX_Control(pDX, IDC_ENHLIST, m_Enh_Filelist);
	DDX_Control(pDX, IDS_MAIN_AUDIODELAY, m_Audiodelay_Label);
	DDX_Control(pDX, IDC_DELAY, m_Audiodelay);
	DDX_Control(pDX, IDS_MAIN_AUDIONAME, m_Audioname_Label);
	DDX_Control(pDX, IDC_AUDIONAME, m_AudioName);
	DDX_Control(pDX, IDS_MAIN_PROTOCOL, m_Protocol_Label);
	DDX_Control(pDX, IDC_AVAILABLEVIDEOSTREAMS, m_VideoSources);
	DDX_Control(pDX, IDC_SOURCEFILELIST, m_SourceFiles);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAVIMux_GUIDlg, CDialog)
	//{{AFX_MSG_MAP(CAVIMux_GUIDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_START, OnStart)
	ON_BN_CLICKED(IDC_STOP, OnStop)
	ON_BN_CLICKED(IDC_ADDAVILIST, OnAddFileList)
	ON_WM_LBUTTONUP()
	ON_BN_CLICKED(IDC_MAXSIZE_EXTENDED, OnMaxsizeExtended)
	ON_BN_CLICKED(IDC_USEMAXSIZE, OnUsemaxsize)
	ON_BN_CLICKED(IDC_LOAD, OnLoad)
	ON_BN_CLICKED(IDC_OUTPUTOPTIONS, OnOutputoptions)
	ON_BN_CLICKED(IDC_NO_AUDIO, OnNoAudio)
	ON_BN_CLICKED(IDC_ALL_AUDIO, OnAllAudio)
	ON_BN_CLICKED(IDC_NO_SUBTITLES, OnNoSubtitles)
	ON_BN_CLICKED(IDC_ALL_SUBTITLES, OnAllSubtitles)
	ON_BN_CLICKED(IDC_DEFAULT_AUDIO, OnDefaultAudio)
	ON_WM_RBUTTONUP()
	ON_EN_CHANGE(IDC_AUDIONAME, OnChangeAudioname)
	ON_EN_CHANGE(IDC_DELAY, OnChangeDelay)
	ON_NOTIFY(TVN_SELCHANGEDW, IDC_AUDIOTREE, OnSelchangedAudiotree)
	ON_NOTIFY(TVN_ITEMEXPANDEDW, IDC_AUDIOTREE, OnItemexpandedAudiotree)
	ON_LBN_SELCHANGE(IDC_SOURCEFILELIST, OnSelchangeSourcefilelist)
	ON_CBN_EDITCHANGE(IDC_STREAM_LNG, OnEditchangeStreamLng)
	ON_CBN_SELCHANGE(IDC_STREAM_LNG, OnSelchangeStreamLng)
	ON_CBN_EDITUPDATE(IDC_STREAM_LNG, OnEditupdateStreamLng)
	ON_EN_UPDATE(IDC_AUDIONAME, OnUpdateAudioname)
	ON_NOTIFY(TVN_BEGINLABELEDIT, IDC_AUDIOTREE, OnBeginlabeleditAudiotree)
	ON_NOTIFY(TVN_ENDLABELEDIT, IDC_AUDIOTREE, OnEndlabeleditAudiotree)
	ON_WM_KEYDOWN()
	ON_NOTIFY(TVN_BEGINLABELEDITW, IDC_AUDIOTREE, OnBeginlabeleditAudiotree)
	ON_NOTIFY(TVN_ENDLABELEDITW, IDC_AUDIOTREE, OnEndlabeleditAudiotree)
	ON_NOTIFY(NM_RETURN, IDC_AUDIOTREE, OnReturnAudiotree)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAVIMux_GUIDlg Nachrichten-Handler

#include "global.h"

void CAVIMux_GUIDlg::SetDialogState_Config()
{
	// shows source file, video source etc
	m_Protocol.ShowWindow(SW_HIDE);
	m_Protocol_Label.ShowWindow(SW_HIDE);
	m_Progress_Group.ShowWindow(SW_HIDE);
	m_Prg_Dest_File.ShowWindow(SW_HIDE);
	m_Prg_Dest_File_Label.ShowWindow(SW_HIDE);
	m_Prg_Frames.ShowWindow(SW_HIDE);
	m_Prg_Frames_Label.ShowWindow(SW_HIDE);
	m_Prg_Legidx_Label.ShowWindow(SW_HIDE);
	m_Prg_Legidx_Progress.ShowWindow(SW_HIDE);
	m_Prg_Progress.ShowWindow(SW_HIDE);
	m_Prg_Progress_Label.ShowWindow(SW_HIDE);
	m_Progress_List.ShowWindow(SW_HIDE);

	m_OutputResolution.ShowWindow(SW_SHOW);
	m_OutputResolution_Label.ShowWindow(SW_SHOW);
	m_AudioName.ShowWindow(SW_HIDE);
	m_Audioname_Label.ShowWindow(SW_HIDE);
	m_Audiodelay.ShowWindow(SW_SHOW);
	m_Audiodelay_Label.ShowWindow(SW_SHOW);
	m_Video_Headline.ShowWindow(SW_SHOW);
	m_Audio_Headline.ShowWindow(SW_SHOW);
	m_VideoSources.ShowWindow(SW_SHOW);

	m_No_Audio.ShowWindow(SW_SHOW);
	m_All_Audio.ShowWindow(SW_SHOW);
	m_No_Subtitles.ShowWindow(SW_SHOW);
	m_All_Subtitles.ShowWindow(SW_SHOW);
	m_Default_Audio.ShowWindow(SW_SHOW);
	m_Default_Audio_Label.ShowWindow(SW_SHOW);
	m_AvailableStreams_Header.ShowWindow(SW_SHOW);
	m_SourceFiles.ShowWindow(SW_SHOW);
	m_Add_Video_Source.ShowWindow(SW_SHOW);
	m_Open_Files_Label.ShowWindow(SW_SHOW);
	m_Title.ShowWindow(SW_SHOW);
	m_Title_Label.ShowWindow(SW_SHOW);
	m_AudioTree.ShowWindow(SW_SHOW);
	m_Subtitles_Headline.ShowWindow(SW_SHOW);

	m_Stream_Lng.ShowWindow(SW_HIDE);

	if (sfOptions.bB0rk) {
		m_VideoStretchFactor.ShowWindow(SW_SHOW);
		m_VideoStretchFactor_Label.ShowWindow(SW_SHOW);
	} else {
		m_VideoStretchFactor.ShowWindow(SW_HIDE);
		m_VideoStretchFactor_Label.ShowWindow(SW_HIDE);
	}

	iCurrentView=1;
}

void CAVIMux_GUIDlg::SetDialogState_Muxing()
{
	// displays protocol and progress when muxing
	m_Protocol.ShowWindow(SW_SHOW);
	m_Protocol_Label.ShowWindow(SW_SHOW);
	m_Progress_Group.ShowWindow(SW_SHOW);
	m_Prg_Dest_File.ShowWindow(SW_SHOW);
	m_Prg_Dest_File_Label.ShowWindow(SW_SHOW);
	m_Prg_Frames.ShowWindow(SW_SHOW);
	m_Prg_Frames_Label.ShowWindow(SW_SHOW);
	m_Prg_Legidx_Label.ShowWindow(SW_SHOW);
	m_Prg_Legidx_Progress.ShowWindow(SW_SHOW);
	m_Prg_Progress.ShowWindow(SW_SHOW);
	m_Prg_Progress_Label.ShowWindow(SW_SHOW);
	m_Progress_List.ShowWindow(SW_SHOW);
	
	m_OutputResolution.ShowWindow(SW_HIDE);
	m_OutputResolution_Label.ShowWindow(SW_HIDE);
	m_AudioName.ShowWindow(SW_HIDE);
	m_Audioname_Label.ShowWindow(SW_HIDE);
	m_Audiodelay.ShowWindow(SW_HIDE);
	m_Audiodelay_Label.ShowWindow(SW_HIDE);
	m_Video_Headline.ShowWindow(SW_HIDE);
	m_Audio_Headline.ShowWindow(SW_HIDE);
	m_VideoSources.ShowWindow(SW_HIDE);

	m_No_Audio.ShowWindow(SW_HIDE);
	m_All_Audio.ShowWindow(SW_HIDE);
	m_No_Subtitles.ShowWindow(SW_HIDE);
	m_All_Subtitles.ShowWindow(SW_HIDE);
	m_Default_Audio.ShowWindow(SW_HIDE);
	m_Default_Audio_Label.ShowWindow(SW_HIDE);
	m_AvailableStreams_Header.ShowWindow(SW_HIDE);
	m_SourceFiles.ShowWindow(SW_HIDE);
	m_Add_Video_Source.ShowWindow(SW_HIDE);
	m_Open_Files_Label.ShowWindow(SW_HIDE);
	m_Title.ShowWindow(SW_HIDE);

	m_Title_Label.ShowWindow(SW_HIDE);
	m_AudioTree.ShowWindow(SW_HIDE);
	m_Stream_Lng.ShowWindow(SW_HIDE);
	m_Subtitles_Headline.ShowWindow(SW_HIDE);

	m_VideoStretchFactor.ShowWindow(SW_HIDE);
	m_VideoStretchFactor_Label.ShowWindow(SW_HIDE);

	iCurrentView=2;
}

const int GLS_NORMAL = 0x00;
const int GLS_USE	 = 0x01;
const int GLS_IGNUSEF= 0x02;

void SetDelay(AUDIO_STREAM_INFO* asi, int iDelay)
{
	asi->iDelay = iDelay;
}

__int64 GetLaceSetting(char* cName, CAttribs* settings, int iFlags)
{
	char cPath[3][100];

	strcpy(cPath[0],"output/mkv/lacing/");
	strcpy(cPath[1],"output/mkv/lacing/");
	strcpy(cPath[2],"output/mkv/lacing/");
	strcat(cPath[0],cName);
	strcat(cPath[0],"/use");
	strcat(cPath[1],cName);
	strcat(cPath[1],"/length");
	strcat(cPath[2],"general/length");

	if (iFlags == GLS_USE) return settings->GetInt(cPath[0]);

	if (iFlags == GLS_IGNUSEF || settings->GetInt(cPath[0])) {
		return (__int64)1000000 * settings->GetInt(cPath[1]);
	} else {
		return (__int64)1000000 * settings->GetInt(cPath[2]);
	}
}

__int64 SetLaceSetting(char* cName, CAttribs* settings, __int64 iUse, __int64 iLength)
{
	char cPath[3][100];

	strcpy(cPath[0],"output/mkv/lacing/");
	strcpy(cPath[1],"output/mkv/lacing/");
	strcpy(cPath[2],"output/mkv/lacing/");
	strcat(cPath[0],cName);
	strcat(cPath[0],"/use");
	strcat(cPath[1],cName);
	strcat(cPath[1],"/length");
	strcat(cPath[2],"general/length");

	settings->SetInt(cPath[0], iUse);
	settings->SetInt(cPath[1], iLength);

	return 0;
}

char    cUTF8Hdr[] = { 0xEF, 0xBB, 0xBF, 0 };
	
void CAVIMux_GUIDlg::AddProtocolLine(char* lpcText,DWORD dwDebugLevel, int dwCC)
{
	SYSTEMTIME time;
	GetLocalTime(&time);
	char	cBuffer[256];
	LVITEM	lvitem;
	char*	c;
	int		i=0;
	char    cASCII[1024]; cASCII[0]=0; char cUTF8[1024]; cUTF8[0] = 0;
	char*   cString = cASCII;
	int		bLogFile = (int)settings->GetInt("output/general/logfile/on");
	char    cNewLine[] = { 13, 10, 0 };
	char    cSpaces[17];
	memset(cSpaces, 32, 16);
	cSpaces[16] = 0;
	DWORD	dwWritten;

	if (!hLogFile && bLogFile) {
		SYSTEMTIME systemtime;
		GetSystemTime(&systemtime);
		char cTemp[50]; cTemp[0]=0;
		sprintf(cTemp, "%04d%02d%02d-%02d%02d%02d.txt", (int)systemtime.wYear, (int)systemtime.wMonth, (int)systemtime.wDay,
			(int)systemtime.wHour, (int)systemtime.wMinute, (int)systemtime.wSecond);
		strcat(cLogFileName, cTemp);
		hLogFile = CreateFile(cLogFileName, GENERIC_WRITE, 0, NULL,	OPEN_ALWAYS, NULL, NULL);
		WriteFile(hLogFile, cUTF8Hdr, 3, &dwWritten, NULL);
	}

	if (iDebugLevel<=(int)dwDebugLevel)
	{
		if (dwCC == APL_UTF8) {
			strcpy(cASCII, lpcText);
		} else {
			Str2UTF8(lpcText, cASCII);
		}
		do {
			if (c = strstr(cString,"\n")) {
				*(c-1)=0;
				*c=0;
			}
			ZeroMemory(&lvitem,sizeof(lvitem));
			lvitem.mask=LVIF_TEXT | LVIF_NORECOMPUTE;
			lvitem.iItem=m_Protocol.GetItemCount();
			lvitem.iSubItem=0;
			lvitem.pszText=cBuffer;
			cBuffer[0] = '\0';
			if (!i++) wsprintf(cBuffer,"%02d:%02d:%02d.%03d",time.wHour,time.wMinute,time.wSecond,time.wMilliseconds);
			if (bLogFile) {
				if (i==1) {
					WriteFile(hLogFile, cBuffer, strlen(cBuffer), &dwWritten, NULL);
					WriteFile(hLogFile, cSpaces, 2, &dwWritten, NULL);
				}
				if (i>1) {
					WriteFile(hLogFile, cSpaces, 14, &dwWritten, NULL);
				}
		
				strcpy(cUTF8,cString);
				WriteFile(hLogFile, cUTF8, strlen(cUTF8), &dwWritten, NULL);
				WriteFile(hLogFile, cNewLine, strlen(cNewLine), &dwWritten, NULL);
			}
			m_Protocol.SetItemText(m_Protocol.InsertItem(&lvitem),1,cString);
			m_Protocol.EnsureVisible(m_Protocol.GetItemCount()-1,false);

		} while ((cString=c) && (cString++));
	}
}

void CAVIMux_GUIDlg::SetDebugLevel(int iLevel)
{
	iDebugLevel=iLevel;
}

void CAVIMux_GUIDlg::AddProtocolLine(CString lpcText,DWORD dwDebugLevel, int dwCC)
{
	AddProtocolLine(lpcText.GetBuffer(255),dwDebugLevel, dwCC);
}

void CAVIMux_GUIDlg::AddProtocol_Separator()
{
	AddProtocolLine("=========================================",5);
}

void CAVIMux_GUIDlg::UpdateLanguage()
{
	RECT	rect;
	
	m_Open_Files_Label.SetWindowText(LoadString(STR_MAIN_S_OPENFILES));
	m_Add_Video_Source.SetWindowText(LoadString(STR_MAIN_B_ADDVIDEOSOURCE));

	SendDlgItemMessage(IDS_MAIN_AVAILABLESTREAMS,WM_SETTEXT,NULL,(LPARAM)LoadString(STR_MAIN_S_AVAILABLESTREAMS));

	m_Subtitles_Headline.SetWindowText(LoadString(STR_MAIN_S_SUBTITLES));
	m_Audio_Headline.SetWindowText(LoadString(STR_MAIN_S_AUDIO));
	m_Video_Headline.SetWindowText(LoadString(STR_MAIN_S_VIDEO));
	m_No_Audio.SetWindowText(LoadString(STR_MAIN_CB_NO_AUDIO));
	m_All_Audio.SetWindowText(LoadString(STR_MAIN_CB_ALL_AUDIO));
	m_Default_Audio_Label.SetWindowText(LoadString(STR_MAIN_CB_DEFAULT_AUDIO));
	m_No_Subtitles.SetWindowText(LoadString(STR_MAIN_CB_NO_SUBTITLES));
	m_All_Subtitles.SetWindowText(LoadString(STR_MAIN_CB_ALL_SUBTITLES));
	m_Protocol_Label.SetWindowText(LoadString(STR_MAIN_S_PROTOCOL));
	m_VideoStretchFactor_Label.SetWindowText(LoadString(STR_MAIN_S_STRETCHBY));
	m_Title_Label.SetWindowText(LoadString(STR_MAIN_S_TITLE));
	SendDlgItemMessage(IDC_OUTPUTOPTIONS,WM_SETTEXT,NULL,(LPARAM)LoadString(STR_MAIN_B_OUTPUTOPTIONS));
	SendDlgItemMessage(IDC_START,WM_SETTEXT,NULL,(LPARAM)LoadString(STR_MAIN_B_START));
	SendDlgItemMessage(IDC_STOP,WM_SETTEXT,NULL,(LPARAM)LoadString(STR_MAIN_B_STOP));
	SendDlgItemMessage(IDCANCEL,WM_SETTEXT,NULL,(LPARAM)LoadString(STR_MAIN_B_CANCEL));
	m_Progress_Group.SetWindowText(LoadString(STR_MAIN_S_PROGRESS));

	m_Progress_List.GetWindowRect(&rect);
	m_Progress_List.DeleteAllItems();
	for (int i=0;i<6;i++) m_Progress_List.DeleteColumn(0);
	m_Progress_List.InsertColumn(0,"",LVCFMT_RIGHT,(rect.right-rect.left)/6-5);
	m_Progress_List.InsertColumn(1,LoadString(STR_MAIN_S_PRG_VIDEO),LVCFMT_RIGHT,(rect.right-rect.left)/6);
	m_Progress_List.InsertColumn(2,LoadString(STR_MAIN_S_PRG_AUDIO),LVCFMT_RIGHT,(rect.right-rect.left)/6);
	m_Progress_List.InsertColumn(3,LoadString(STR_MAIN_S_PRG_OVERHEAD),LVCFMT_RIGHT,(rect.right-rect.left)/6);
	m_Progress_List.InsertColumn(4,LoadString(STR_MAIN_S_PRG_SUBTITLES),LVCFMT_RIGHT,(rect.right-rect.left)/6);
	m_Progress_List.InsertColumn(5,LoadString(STR_MAIN_S_PRG_TOTAL),LVCFMT_RIGHT,(rect.right-rect.left)/6);
	m_Progress_List.InsertItem(0,LoadString(STR_MAIN_S_PRG_CURRFILE));
	m_Progress_List.InsertItem(1,LoadString(STR_MAIN_S_PRG_ALLFILES));
	m_Progress_List.InsertItem(2,LoadString(STR_MAIN_S_PRG_TRANSFERRATE));

	m_Prg_Dest_File_Label.SetWindowText(LoadString(STR_MAIN_S_PRG_DESTFILE));
	m_Prg_Frames_Label.SetWindowText(LoadString(STR_MAIN_S_PRG_FRAMES));
	m_Prg_Legidx_Label.SetWindowText(LoadString(STR_MAIN_S_PRG_LEGIDX));
	m_Prg_Progress_Label.SetWindowText(LoadString(STR_MAIN_S_PRG_PROGRESS));

	m_Audioname_Label.SetWindowText(LoadString(STR_MAIN_S_AUDIONAME));
	m_Audiodelay_Label.SetWindowText(LoadString(STR_MAIN_S_AUDIODELAY));
	m_OutputResolution_Label.SetWindowText(LoadString(STR_MAIN_S_RESOLUTION));

	switch (iCurrentView)
	{
		case 1: SetDialogState_Config(); break;
		case 2: SetDialogState_Muxing(); break;
	}			
	switch (iButtonState)
	{
		case 2: ButtonState_START(); break;
		case 1: ButtonState_STOP(); break;
	}
}


UINT CAVIMux_GUIDlg::GetUserMessageID()
{
	return uiMessage;
}


void CAVIMux_GUIDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
		CDialog::OnSysCommand(nID, lParam);
}

// Wollen Sie Ihrem Dialogfeld eine Schaltfläche "Minimieren" hinzufügen, benötigen Sie 
// den nachstehenden Code, um das Symbol zu zeichnen. Für MFC-Anwendungen, die das 
// Dokument/Ansicht-Modell verwenden, wird dies automatisch für Sie erledigt.

void CAVIMux_GUIDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // Gerätekontext für Zeichnen

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Symbol in Client-Rechteck zentrieren
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Symbol zeichnen
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// Die Systemaufrufe fragen den Cursorform ab, die angezeigt werden soll, während der Benutzer
//  das zum Symbol verkleinerte Fenster mit der Maus zieht.
HCURSOR CAVIMux_GUIDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

#include "FILE_INFO.h"

void CAVIMux_GUIDlg::AddFile(CFileDialog* cfd)
{
}


DWORD DetectFileType(STREAM* lpSource, void** pReturn = NULL)
{
	union
	{
		MP3SOURCE*			mp3source;
		CBRAUDIOSOURCE*		cbrsource;
		AC3SOURCE*			ac3source;	
		DTSSOURCE*			dtssource;
		AACSOURCE*			aacsource;
		AVIFILEEX*			avisource;
		SUBTITLES*			subsource;
		MATROSKA*			mkvsource;
		OGGFILE*			oggfile;
	};
	VORBISFROMOGG*		vorbis;
	WAVEFILE*		wavfile;
	CTEXTFILE*		t;
	char c[102400]; ZeroMemory(c,sizeof(c));

	static const int resync_lengths[] = { 64, 1024, 8192, 131072 };
	int	i;

// OGG
	lpSource->SetOffset(0);
	lpSource->Seek(0);
	oggfile=new OGGFILE;
	if (oggfile->Open(lpSource, OGG_OPEN_READ)==OGG_OPEN_OK)
	{
		vorbis = new VORBISFROMOGG;
		vorbis->Open(oggfile);
		vorbis->Close();
		delete vorbis;
		oggfile->Close(false);
		delete oggfile;
		lpSource->SetOffset(0);
		return FILETYPE_OGGVORBIS;
	}
	delete oggfile;

// MKV
	lpSource->SetOffset(0);
	lpSource->Seek(0);
	mkvsource=new MATROSKA;
	if (mkvsource->Open(lpSource,MMODE_READ | MMODE_DUMMY)==MOPEN_OK)
	{
		mkvsource->Close();
		delete mkvsource;
		lpSource->SetOffset(0);
		return FILETYPE_MKV;
	}
	delete mkvsource;

// Subtitles
	lpSource->SetOffset(0);
	lpSource->Seek(0);
	subsource=new SUBTITLES;
	t=new CTEXTFILE(STREAM_READ,lpSource,CM_UTF8);
	if (subsource->Open(t))
	{
		subsource->Close();
		delete subsource;
		t->Close();
		delete t;
		lpSource->SetOffset(0);
		return FILETYPE_SUBS;
	}
	delete subsource;
	t->Seek(0);
// XML
	t->ReadLine(c);
	t->ReadLine(c);
	int j=0;
	if (!strncmp(c,"<!DOCTYPE",9)) {
		do { 
			i=t->ReadLine(c+j); 
			j+=i; } 
		while (i!=-1);
		XMLNODE* pTree = NULL;
		if (xmlBuildTree(&pTree, c) == XMLERR_OK) {
			ZeroMemory(c, sizeof(c));
			if (pReturn) *pReturn = pTree;
			return FILETYPE_XML;
		}
		t->Close();
		delete t;
	}

// AVI
	lpSource->Seek(0);
	avisource=new AVIFILEEX;
	if (avisource->Open(lpSource,FA_READ | FA_DUMMY,AT_AUTODETECT)==AFE_OK)
	{
		avisource->Close(false);
		delete avisource;
		lpSource->SetOffset(0);
		return FILETYPE_AVI;
	}
	delete avisource;
	for (i=0; i<4; i++)
	{

// AC3
		lpSource->SetOffset(0);
		lpSource->Seek(0);
		ac3source=new AC3SOURCE;
		ac3source->SetResyncRange(resync_lengths[i]);
		if (ac3source->Open(lpSource))
		{
			ac3source->Close();
			delete ac3source;
			lpSource->SetOffset(0);
			return FILETYPE_AC3;
		}
		ac3source->Close();
		delete ac3source;
		
// DTS
		lpSource->SetOffset(0);
		lpSource->Seek(0);
		
		dtssource=new DTSSOURCE;
		dtssource->SetResyncRange(resync_lengths[i]);
		if (dtssource->Open(lpSource))
		{
			dtssource->Close();
			delete dtssource;
			lpSource->SetOffset(0);
			return FILETYPE_DTS;
		}
		dtssource->Close();
		delete dtssource;

// MP3
		lpSource->SetOffset(0);
		lpSource->Seek(0);
		mp3source=new MP3SOURCE;
		mp3source->SetResyncRange(resync_lengths[i]);
		if (mp3source->Open(lpSource))
		{
			mp3source->Close();
			delete mp3source;
			lpSource->SetOffset(0);
			return FILETYPE_MP3;
		}
		mp3source->Close();
		delete mp3source;

// AAC
		lpSource->SetOffset(0);
		lpSource->Seek(0);
		aacsource=new AACSOURCE;
//		aacsource->SetResyncRange(resync_lengths[i]);
		if (aacsource->Open(lpSource))
		{
			aacsource->Close();
			delete aacsource;
			lpSource->SetOffset(0);
			return FILETYPE_AAC;
		}
		aacsource->Close();
		delete aacsource;

	}
	
// WAV
	lpSource->SetOffset(0);
	lpSource->Seek(0);
	wavfile=new WAVEFILE;
	if (wavfile->Open(lpSource,FA_READ))
	{
		wavfile->Close();
		delete wavfile;
		lpSource->SetOffset(0);
		return FILETYPE_WAV;
	}
	delete wavfile;

	return FILETYPE_UNKNOWN;
}


void  CAVIMux_GUIDlg::doAddFile(char* lpcName, int iFormat)
{
	char*	lpcExt;
	int		i;
	int				iIndex1;
	FILESTREAM*		filesource;
	STREAM*			source;
	CACHEDSTREAM*	cachesource = NULL;
	MODE2FORM2SOURCE* m2f2source;
	char			VideoFormat[100];
	union
	{
		MP3SOURCE*			mp3source;
		CBRAUDIOSOURCE*		cbrsource;
		AC3SOURCE*			ac3source;	
		AACSOURCE*			aacsource;	
		DTSSOURCE*			dtssource;
		VORBISFROMOGG*		vorbis;
	};
	OGGFILE*			oggfile;
	SUBTITLES*		subs;
	WAVEFILE*		wavfile;
	FILE_INFO*		fi;
	AUDIO_STREAM_INFO*	asi = NULL;
	AVIFILEEX*		AVIFile;
	HANDLE			hSem;

//	DWORD			dwCheckVBR;
	CListBox*		cListBox;
	CString			cStr,cStr2;
	DWORD			dwUseCache;
	DWORD			dwFmtUpStr;
	DWORD			dwFmtUpStr3;
	SUBTITLE_STREAM_INFO*	lpssi;

	char			Buffer[200],Fmt[50];
	DWORD			dwLength=lstrlen(lpcName);
	CBuffer*		cb;
	bool			bUnbuffered = !!settings->GetInt("input/unbuffered");

	filestreamAllowBufferedRead(bUnbuffered);
	for (i=dwLength;(i>=0)&&(lpcName[i]!='.');i--);

	lpcExt=new char[dwLength-i+2];
	ZeroMemory(lpcExt,dwLength-i+2);
	lstrcpy(lpcExt,&(lpcName[i+1]));
	for (i=lstrlen(lpcExt)-1;i>=0;lpcExt[i--]|=((lpcExt[i]>=64)&&(lpcExt[i]<=90))?0x20:0);

		ZeroMemory(Buffer,sizeof(Buffer));
		dwUseCache=(settings->GetInt("input/use cache"))?1:0;
		cb = new CBuffer;
		cb->SetSize(sizeof(FILE_INFO));
		cb->IncRefCount();

		fi = (FILE_INFO*)cb->GetData();
		ZeroMemory(fi,sizeof(FILE_INFO));
		fi->cache=NULL;
		fi->dwType=0;
		fi->file=NULL;
		fi->AVIFile=NULL;
		fi->bM2F2CRC=!!(ofOptions.dwFlags&SOFO_M2F2_DOM2F2CRCCHECK);
		fi->bInUse=false;
		fi->lpwav=NULL;
		fi->lpM2F2=NULL;
		fi->bAddedImmediately=1;
		fi->bMP3VBF_forced=!!(ofOptions.dwFlags&SOFO_AVI_FORCEMP3VBR);
		filesource=new FILESTREAM;
		if (filesource->Open(lpcName,STREAM_READ)==STREAM_ERR)
		{
			DecBufferRefCount(&cb);;
			return;
		}
		fi->file=filesource;

		m2f2source=new MODE2FORM2SOURCE;
		if (m2f2source->Open(filesource)==STREAM_OK)
		{
			source=(STREAM*)m2f2source;
			fi->dwType|=FILETYPE_M2F2;
			fi->lpM2F2=m2f2source;
		}
		else
		{
			source=filesource;
			m2f2source->Close();
			delete m2f2source;
			fi->lpM2F2=NULL;
//			source->Seek(0);
		}
		
		XMLNODE* xmlTree;
		fi->dwType|=(iFormat!=FILETYPE_UNKNOWN)?iFormat:DetectFileType(source, (void**)&xmlTree);

		filesource->Close();
		filesource->Open(lpcName, STREAM_READ | 
			(bUnbuffered?STREAM_OVERLAPPED:0));
		source->Seek(0);
		if (fi->dwType & FILETYPE_AVI)
		{
			fi->bAddedImmediately=0;
			// unnötige Seekoperationen vermeiden?
			if (dwUseCache)
			{
				cachesource=new CACHEDSTREAM(8,1<<19);
				cachesource->Open((STREAM*)source);
				cachesource->EnableReadAhead();
				fi->cache=cachesource;
			}
			else
			{
				cachesource=(CACHEDSTREAM*)source;
			}
			AVIFile=new AVIFILEEX;
			AVIFile->SetDebugState(DS_DEACTIVATE);
			AVIFile->SetMaxAllowedChunkSize(1024*(!!(ofOptions.dwFlags&SOFO_AVI_IGNORELARGECHUNKS))*ofOptions.dwIgnoreSize);
			AVIFile->TryToRepairLargeChunks(!!(ofOptions.dwFlags&SOFO_AVI_TRYTOREPAIRLARGECHUNKS));
			if (AVIFile->Open(cachesource,FA_READ,AT_AUTODETECT)==AFE_OK)
			{
			// Name fürs Format rausbekommen
				if (ofOptions.dwFlags&SOFO_AVI_REPAIRDX50)
				{
					if ((AVIFile->GetFormatTag(0)&0xffffdfdf)==MakeFourCC("DX50"))
					{
						AVIFile->GetStreamHeader(0)->fccHandler=MakeFourCC("divx");
					}
				}
				dwFmtUpStr=AVIFile->GetFormatTag(0) & (0xdfdfdfdf);
				dwFmtUpStr3=AVIFile->GetFormatTag(0) & (0xffdfdfdf);
				if (dwFmtUpStr3==MakeFourCC("DIV3"))
				{
					wsprintf(VideoFormat,"%s","divX 3.11 low motion");
				}
				else
				if (dwFmtUpStr3==MakeFourCC("DIV4"))
				{
					wsprintf(VideoFormat,"%s","divX 3.11 fast motion");
				}
				else
				if (dwFmtUpStr==MakeFourCC("DIVX"))
				{
					if ( ((BITMAPINFOHEADER*)(AVIFile->GetStreamFormat(0)))->biCompression==MakeFourCC("DX50"))
					{
						wsprintf(VideoFormat,"%s","divX 5");
					}
					else
					{
						wsprintf(VideoFormat,"%s","divX 4");
					}
				}
				else
				if ((dwFmtUpStr3==MakeFourCC("DMB1"))||
					(dwFmtUpStr==MakeFourCC("MJPG"))||
					(dwFmtUpStr==MakeFourCC("MJPX")))
				{
					wsprintf(VideoFormat,"%s","M-JPEG");
				}
				else
				if (dwFmtUpStr==MakeFourCC("HFYU"))
				{
					wsprintf(VideoFormat,"%s","huff-YUV");
				}
				else
				if (dwFmtUpStr==MakeFourCC("XVID"))
				{
					wsprintf(VideoFormat,"%s","XVID");
				}
				else
				if (AVIFile->GetFormatTag(0)==MakeFourCC("MJ2C"))
				{
					wsprintf(VideoFormat,"%s","MorganMotion JPEG 2000");
				}
				else
					wsprintf(VideoFormat,"%s","???");

				fi->AVIFile=AVIFile;
				AVIFile->SetProcessMode(SPM_SETALL,PM_DIRECTSTREAMCOPY);
				fi->dwType|=FILETYPE_AVI;
			// Infos anzeigen
				wsprintf(Fmt,"%s (%s%s): %s","AVI",(fi->dwType&FILETYPE_M2F2)?"M2F2, ":"",
					(AVIFile->GetAVIType()==AT_STANDARD)?"standard":"Open-DML",VideoFormat);
			}
			else
			{
				MessageBox ("Fehler beim Öffnen der Datei");
				filesource->Close();
				fi->dwType=0;
			}
		}
		else
		if (fi->dwType&FILETYPE_MP3)
		{
			// use input cache?
			if (dwUseCache)
			{
				cachesource=new CACHEDSTREAM(4,1<<19);
				cachesource->Open(filesource);
				fi->cache=cachesource;
			}
			else
			{
				cachesource=(CACHEDSTREAM*)source;
			}
			wsprintf(Fmt,"%s","MP3");
			fi->dwType|=FILETYPE_MP3;
		}
		else
		if (fi->dwType&FILETYPE_AC3)
		{
			// use input cache?
			if (dwUseCache)
			{
				cachesource=new CACHEDSTREAM(4,1<<19);
				cachesource->Open(source);
				fi->cache=cachesource;
			}
			else
			{
				cachesource=(CACHEDSTREAM*)source;
			}
			wsprintf(Fmt,"%s","AC3");
			fi->dwType|=FILETYPE_AC3;
		}
		else
		if (fi->dwType&FILETYPE_DTS)
		{
			// use input cache?
			if (dwUseCache)
			{
				cachesource=new CACHEDSTREAM(4,1<<19);
				cachesource->Open(source);
				fi->cache=cachesource;
			}
			else
			{
				cachesource=(CACHEDSTREAM*)source;
			}
			wsprintf(Fmt,"%s","DTS");
			fi->dwType|=FILETYPE_DTS;
		}
		else
		if (fi->dwType&FILETYPE_AAC)
		{
			// use input cache?
			if (dwUseCache)
			{
				cachesource=new CACHEDSTREAM(4,1<<19);
				cachesource->Open(source);
				fi->cache=cachesource;
			}
			else
			{
				cachesource=(CACHEDSTREAM*)source;
			}
			wsprintf(Fmt,"%s","AAC");
			fi->dwType|=FILETYPE_AAC;
		}
		else
		if (fi->dwType&FILETYPE_OGGVORBIS)
		{
			// use input cache?
			if (dwUseCache)
			{
				cachesource=new CACHEDSTREAM(4,1<<19);
				cachesource->Open(source);
				fi->cache=cachesource;
			}
			else
			{
				cachesource=(CACHEDSTREAM*)source;
			}
			wsprintf(Fmt,"%s","OGG/Vorbis");
			fi->dwType|=FILETYPE_OGGVORBIS;
		}
		else
		if (fi->dwType&FILETYPE_WAV)
		{
			if (dwUseCache)
			{
				cachesource=new CACHEDSTREAM(4,1<<19);
				cachesource->Open(source);
				fi->cache=cachesource;
			}
			else
			{
				cachesource=(CACHEDSTREAM*)source;
			}
			wsprintf(Fmt,"%s","WAV");
			fi->dwType|=FILETYPE_WAV;
		}
		else
		if (fi->dwType&FILETYPE_SUBS)
		{
			if (dwUseCache)
			{
				cachesource=new CACHEDSTREAM(4,1<<19);
				cachesource->Open(source);
				fi->cache=cachesource;
			}
			else
			{
				cachesource=(CACHEDSTREAM*)source;
			}
			wsprintf(Fmt,"%s","SUB");
			fi->dwType|=FILETYPE_SUBS;
		}
		else
		if (fi->dwType&FILETYPE_MKV)
		{
			if (dwUseCache)
			{
				cachesource=new CACHEDSTREAM(8,1<<19);
				cachesource->Open(source);
				cachesource->EnableReadAhead();
				fi->cache=cachesource;
			}
			else
			{
				cachesource=(CACHEDSTREAM*)source;
			}
			wsprintf(Fmt,"%s","MKV");
			fi->MKVFile = new MATROSKA;
			fi->bAddedImmediately=0;
			if (fi->MKVFile->Open(cachesource,MMODE_READ)==MOPEN_ERR) {
				delete fi->MKVFile;
				fi->MKVFile = NULL;
				fi->dwType&=~FILETYPE_MKV;
			} else {
				fi->dwType|=FILETYPE_MKV;
			}
		}
		
		if (/*fi->dwType &&*/ fi->dwType!=FILETYPE_SCRIPT)
		{
			LVITEM	lvitem;
			int		iIndex_Enh;

			ZeroMemory(&lvitem,sizeof(lvitem));
			lvitem.mask=LVIF_TEXT | LVIF_NORECOMPUTE;
			lvitem.iItem=1;
			lvitem.iSubItem=0;
			lvitem.pszText=lpcName;
			iIndex_Enh=m_Enh_Filelist.InsertItem(&lvitem);
			m_Enh_Filelist.SetItemData(iIndex_Enh,(DWORD)fi);

			char cSize[100];
			char cFormatStr[200];
			__int64 dwSize = (cachesource)?cachesource->GetSize():fi->file->GetSize();
			FormatSize(cSize,dwSize);

			sprintf(cFormatStr,"%%s, %%s: %%s",cSize);
			

			switch (fi->dwType&FILETYPE_MASK)
			{
				case FILETYPE_MP3:
					sprintf(Buffer,cFormatStr,Fmt,cSize,lpcName);
					m_Enh_Filelist.SetItemText(iIndex_Enh,1,LoadString(STR_MEDIATYPE_AUDIO));
					break;
				case FILETYPE_AC3:
					sprintf(Buffer,cFormatStr,Fmt,cSize,lpcName);
					m_Enh_Filelist.SetItemText(iIndex_Enh,1,LoadString(STR_MEDIATYPE_AUDIO));
					break;
				case FILETYPE_DTS:
					sprintf(Buffer,cFormatStr,Fmt,cSize,lpcName);
					m_Enh_Filelist.SetItemText(iIndex_Enh,1,LoadString(STR_MEDIATYPE_AUDIO));
					break;
				case FILETYPE_AVI:
					sprintf(Buffer,cFormatStr,Fmt,cSize,lpcName);
					m_Enh_Filelist.SetItemText(iIndex_Enh,1,LoadString(STR_MEDIATYPE_VIDEO));
					break;
				case FILETYPE_WAV:
					sprintf(Buffer,cFormatStr,Fmt,cSize,lpcName);
					m_Enh_Filelist.SetItemText(iIndex_Enh,1,LoadString(STR_MEDIATYPE_AUDIO));
					break;
				case FILETYPE_AAC:
					sprintf(Buffer,cFormatStr,Fmt,cSize,lpcName);
					m_Enh_Filelist.SetItemText(iIndex_Enh,1,LoadString(STR_MEDIATYPE_AUDIO));
					break;
				case FILETYPE_OGGVORBIS:
					sprintf(Buffer,cFormatStr,Fmt,cSize,lpcName);
					m_Enh_Filelist.SetItemText(iIndex_Enh,1,LoadString(STR_MEDIATYPE_AUDIO));
					break;
				case FILETYPE_SUBS:
					sprintf(Buffer,cFormatStr,Fmt,cSize,lpcName);
					m_Enh_Filelist.SetItemText(iIndex_Enh,1,LoadString(STR_MEDIATYPE_SUBTITLE));
					break;
				case FILETYPE_MKV:
					sprintf(Buffer,cFormatStr,Fmt,cSize,lpcName);
					m_Enh_Filelist.SetItemText(iIndex_Enh,1,LoadString(STR_MEDIATYPE_VIDEO));
					break;
				case FILETYPE_SCRIPT:
					break;
				default:
					sprintf(Buffer,cFormatStr,"unknown",cSize,lpcName);
					m_Enh_Filelist.SetItemText(iIndex_Enh,1,LoadString(STR_MEDIATYPE_VIDEO));
					break;
			}
		
			cListBox=(CListBox*)GetDlgItem(IDC_SOURCEFILELIST);
			iIndex1=cListBox->AddString(Buffer);
			fi->lpcName=new char[lstrlen(lpcName)+16];
			lstrcpy(fi->lpcName,lpcName);
			fi->source=cachesource;
			cListBox->SetItemData(iIndex1,(LPARAM)cb);

			switch ((fi->dwType)&(FILETYPE_MASK))
			{
				case FILETYPE_AVI:
					{
						m_Enh_Filelist.SetItemText(iIndex_Enh,2,"AVI");
					}
					break;
				case FILETYPE_MP3:
					{
						mp3source=new MP3SOURCE;
						if (!(mp3source->Open(cachesource)))
						{
							cStr=LoadString(IDS_CANTOPENMP3);
							MessageBox(cStr,cstrError,MB_OK | MB_ICONERROR);
							delete mp3source;
							cListBox->DeleteString(iIndex1);
							return;
						}
						fi->MP3File = mp3source;

						if (bAddAS_immed) {
							DWORD dwCheckVBR;
							asi = new AUDIO_STREAM_INFO;
							ZeroMemory(asi,sizeof(AUDIO_STREAM_INFO));
							if (((ofOptions.dwFlags&SOFO_MP3_MASK)==SOFO_MP3_CHECKCBRALWAYS)||((ofOptions.dwFlags&SOFO_MP3_MASK)==SOFO_MP3_CHECKCBRASK))
							{
								if (mp3source->ScanForCBR(1000))
								{
									cStr=LoadString(IDS_COULDBECBR);
									if ((ofOptions.dwFlags&SOFO_MP3_MASK)==SOFO_MP3_CHECKCBRASK) {
										dwCheckVBR=MessageBox(cStr,cstrConfirmation,MB_YESNO | MB_ICONQUESTION);
									}
									else {
										dwCheckVBR=IDYES;
									}
									if (dwCheckVBR==IDYES)
									{
										if (mp3source->ScanForCBR(SCANFORCBR_ALL))	{
											if ((ofOptions.dwFlags&SOFO_MP3_RESULTDLG)==SOFO_MP3_RESULTDLG)	{
												cStr=LoadString(IDS_FILEISMP3CBR);
												MessageBox(cStr,cstrInformation,MB_OK | MB_ICONINFORMATION);
											}
											asi->dwType=AUDIOTYPE_MP3CBR;
										}
										else
										{
											if ((ofOptions.dwFlags&SOFO_MP3_RESULTDLG)==SOFO_MP3_RESULTDLG)	{
												cStr=LoadString(IDS_FILEISMP3VBR);
												MessageBox(cStr,cstrInformation,MB_OK | MB_ICONINFORMATION);
											}
										}
									}
								}
							}
							asi->lpdwFiles = new DWORD[2];
							asi->lpdwFiles[0]=1;
							asi->lpdwFiles[1]=m_SourceFiles.GetCount()-1;
							asi->dwFlags |= ASIF_ALLOCATED;
							fi->bInUse = 1;

							FillMP3_ASI(&asi,mp3source);
							fi->bAddedImmediately=1;
							asi->bNameFromFormatTag = true;
						} else {
							fi->bAddedImmediately=0;
						}

					/*	cStr2=LoadString(STR_KBYTE);
						cStr=LoadString(IDS_FILE);
						Buffer[0]=0;
						wsprintf(Buffer,"%s %d, MP3-%s: %d %s, Ch: %d, Fr.: %d, Begin: %d",cStr,
							iIndex1+1,mp3source->IsCBR()?"CBR":"VBR",
							(int)(asi->audiosource->GetSize()>>10),cStr2,
							(int)mp3source->GetChannelCount(),(int)mp3source->GetFrequency(),
							(int)mp3source->GetOffset());
						
						*/
						//m_Enh_Filelist.SetItemText(iIndex_Enh,2,"MP3");

					}
					break;
				case FILETYPE_OGGVORBIS:
					{
						oggfile = new OGGFILE();
						oggfile->Open(cachesource, OGG_OPEN_READ);
						vorbis = new VORBISFROMOGG;
						vorbis->Open(oggfile);
						
						asi = NULL;
						if (bAddAS_immed) {
							asi = new AUDIO_STREAM_INFO;
							ZeroMemory(asi, sizeof(*asi));
							asi->audiosource = vorbis;
							asi->bNameFromFormatTag = true;
							asi->lpFormat = new byte[1<<16];
							asi->iFormatSize = vorbis->RenderSetupHeader(asi->lpFormat);
							asi->dwFlags = ASIF_ALLOCATED;
							asi->lpASH = new AVIStreamHeader;
							asi->dwType = AUDIOTYPE_VORBIS;
							asi->iSize = vorbis->GetSize();
							asi->lpdwFiles = new DWORD[2];
							asi->lpdwFiles[0]=1;
							asi->lpdwFiles[1]=m_SourceFiles.GetCount()-1;
							asi->dwFlags |= ASIF_ALLOCATED;
							fi->bInUse = 1;

							ZeroMemory(asi->lpASH, sizeof(AVIStreamHeader));
							fi->bAddedImmediately = 1;
						} else {
							fi->bAddedImmediately = 0;
							fi->VRBFile = vorbis;
						}
					}
					break;
				case FILETYPE_AC3:
					{
						ac3source=new AC3SOURCE;
						if (!(ac3source->Open(cachesource)))
						{
							cStr=LoadString(IDS_CANTOPENMP3);
							MessageBox(cStr,cstrError,MB_OK | MB_ICONERROR);
							delete mp3source;
							cListBox->DeleteString(iIndex1);
							return;
						}
						else
						{
							asi=NULL;
							if (bAddAS_immed) {
								FillAC3_ASI(&asi,ac3source);		
								cStr=LoadString(IDS_FILE);
								cStr2=LoadString(STR_KBYTE);
/*							wsprintf(Buffer,"%s %d, AC3: %d %s, Ch: %d, Br: %d, Fr: %d, Begin: %d",cStr.GetBuffer(255),iIndex1+1,
								(DWORD)(asi->audiosource->GetSize()>>10), cStr2.GetBuffer(255),
								ac3source->GetChannelCount(),
								ac3source->GetBitrate(),ac3source->GetFrequency(),ac3source->GetSource()->GetOffset());
							
							m_Enh_Filelist.SetItemText(iIndex_Enh,2,"AC3");*/
								asi->bNameFromFormatTag = true;
								asi->lpdwFiles = new DWORD[2];
								asi->lpdwFiles[0]=1;
								asi->lpdwFiles[1]=m_SourceFiles.GetCount()-1;
								asi->dwFlags |= ASIF_ALLOCATED;
								fi->bInUse = 1;


							} else {
								fi->bAddedImmediately = 0;
								fi->AC3File = ac3source;
							}
							
						}
					}
					break;
				case FILETYPE_DTS:
					{
						dtssource=new DTSSOURCE;
						if (!(dtssource->Open(cachesource)))
						{
							cStr=LoadString(IDS_CANTOPENMP3);
							MessageBox(cStr,cstrError,MB_OK | MB_ICONERROR);
							delete mp3source;
							cListBox->DeleteString(iIndex1);
							return;
						}

						asi=NULL;
						if (bAddAS_immed) {
							FillDTS_ASI(&asi,dtssource);		
							cStr=LoadString(IDS_FILE);
							cStr2=LoadString(STR_KBYTE);
						/*	wsprintf(Buffer,"%s %d, DTS: %d %s, Ch: %d, Br: %d, Fr: %d, Begin: %d",cStr.GetBuffer(255),iIndex1+1,
								(DWORD)(asi->audiosource->GetSize()>>10),cStr2.GetBuffer(255),
								dtssource->GetChannelCount(),
								(DWORD)dtssource->GetBitrate(),dtssource->GetFrequency(),dtssource->GetSource()->GetOffset());
							
							m_Enh_Filelist.SetItemText(iIndex_Enh,2,"DTS");*/
							asi->bNameFromFormatTag = true;
							asi->lpdwFiles = new DWORD[2];
							asi->lpdwFiles[0]=1;
							asi->lpdwFiles[1]=m_SourceFiles.GetCount()-1;
							asi->dwFlags |= ASIF_ALLOCATED;
							fi->bInUse = 1;
						} else {
							fi->bAddedImmediately = 0;
							fi->DTSFile = dtssource;
						}
					}
					break;
				case FILETYPE_AAC:
					{
						aacsource = new AACSOURCE;
						if (!(aacsource->Open(cachesource))) {
							cStr=LoadString(IDS_CANTOPENMP3);
							MessageBox(cStr,cstrError,MB_OK | MB_ICONERROR);
							delete aacsource;
							cListBox->DeleteString(iIndex1);
							return;
						}
						cStr=LoadString(IDS_FILE);
						cStr2=LoadString(STR_KBYTE);

						if (bAddAS_immed) {
							FillAAC_ASI(&asi, aacsource);
							wsprintf(Buffer,"%s %d, AAC: Fr: %d, MPEG: %d",cStr.GetBuffer(255),iIndex1+1,
								(DWORD)(asi->audiosource->GetSize()>>10),cStr2.GetBuffer(255),
								aacsource->GetFrequency(), aacsource->FormatSpecific(MMSGFS_AAC_MPEGVERSION));
						
							m_Enh_Filelist.SetItemText(iIndex_Enh,2,"AAC");
							asi->bNameFromFormatTag = true;

							char* pSemName;
							DWORD dwStatus;

							aacsource->PerformCFRCheck(&pSemName, &dwStatus);
							hSem = OpenSemaphore(SEMAPHORE_ALL_ACCESS, false, pSemName);

							SetDialogState_Muxing();
							m_Prg_Progress.SetRange32(0,1000);
							while (WaitForSingleObject(hSem, 100) == WAIT_TIMEOUT){
								m_Prg_Progress.SetPos(dwStatus);
								UpdateWindow();
							}
	
							SetDialogState_Config();

							if (!aacsource->IsCFR()) {
								MessageBox("It is not recommended to put this AAC file into an AVI file!",
									LoadString(STR_GEN_WARNING),MB_OK | MB_ICONWARNING);
							}

							fi->bAddedImmediately = 1;

							asi->lpdwFiles = new DWORD[2];
							asi->lpdwFiles[0]=1;
							asi->lpdwFiles[1]=m_SourceFiles.GetCount()-1;
							asi->dwFlags |= ASIF_ALLOCATED;
							fi->bInUse = 1;
						} else {
							fi->bAddedImmediately = 0;
							fi->AACFile = aacsource;
						}
					}
					break;
				case FILETYPE_WAV:
					{
						wavfile=new WAVEFILE;
						wavfile->Open(cachesource,FA_READ);
						fi->lpwav=wavfile;
						asi=new AUDIO_STREAM_INFO;
						ZeroMemory(asi,sizeof(AUDIO_STREAM_INFO));
						cbrsource=new CBRAUDIOSOURCE;
						cbrsource->Open((STREAM*)wavfile);
						asi->audiosource=cbrsource;
						asi->dwFlags|=ASIF_ALLOCATED;
						asi->lpASH=new AVIStreamHeader;
						ZeroMemory(asi->lpASH,sizeof(AVIStreamHeader));
						asi->lpASH->dwScale=wavfile->GetGranularity();
						asi->lpASH->dwRate=wavfile->GetStreamFormat()->nAvgBytesPerSec;
						asi->lpASH->dwSampleSize=wavfile->GetGranularity();
						asi->lpASH->fccType=MakeFourCC("auds");
						asi->lpFormat=new byte[(wavfile->GetStreamFormat()->cbSize+sizeof(WAVEFORMATEX))];
						memcpy(asi->lpFormat,wavfile->GetStreamFormat(),wavfile->GetStreamFormat()->cbSize+sizeof(WAVEFORMATEX));
						cStr=LoadString(IDS_FILE);
						
						WAVEFORMATEX* lpwfe = (WAVEFORMATEX*)asi->lpFormat;
						if (lpwfe->wFormatTag==1) {
							asi->dwType = AUDIOTYPE_PCM;
						} else {
							asi->dwType=AUDIOTYPE_PLAINCBR;
						}

						cStr2=LoadString(STR_KBYTE);
						wsprintf(Buffer,"%s %d: %d %s",cStr.GetBuffer(255),iIndex1+1,
							(DWORD)(asi->audiosource->GetSize()>>10),cStr2.GetBuffer(255));
						
						m_Enh_Filelist.SetItemText(iIndex_Enh,2,"WAV");
						asi->bNameFromFormatTag = true;
					}
					break;
				case FILETYPE_SUBS:
					{
						subs=new SUBTITLES;
						subs->Open(new CTEXTFILE(STREAM_READ,fi->source,CM_UTF8));

						lpssi=new SUBTITLE_STREAM_INFO;
						lpssi->lpfi=fi;
						lpssi->lpsubs=subs;
						lpssi->lpash=new AVIStreamHeader;
						lpssi->lpash->fccType=MakeFourCC("txts");
						
						cStr=LoadString(IDS_FILE);
						cStr2=LoadString(STR_KBYTE);
						char* e; char* f;
						splitpathname(lpcName,&f,&e,NULL);
						e=&f[strlen(f)-1];
						while (*e-- != '.');
						*(e+1)=0;
			
						lpssi->lpsubs->SetName(f);

						AddSubtitleStream(lpssi);
					}
					break;
				case FILETYPE_XML:
					if (chapters->ImportFromXML(xmlTree)) {
						m_SourceFiles.DeleteString(iIndex1);
					}
					//m_StatusLine.SetWindowText("XML chapter file loaded");
					break;
			}
		}
		
		if (fi->dwType==FILETYPE_UNKNOWN) {
			if (!LoadScript(lpcName,m_hWnd,GetUserMessageID()))	{
				cStr=LoadString(IDS_FILETYPENOTSUPPORTED);
				wsprintf(Buffer,cStr);
				MessageBox(Buffer,cstrError,MB_OK | MB_ICONERROR);
			} else {
				fi->file->Close();
				delete fi->lpcName;
				m_SourceFiles.DeleteString(iIndex1);
				delete fi->file;
				fi->file = NULL;
				DecBufferRefCount(&cb);
			}
		} else {
			char cMsg[500]; cMsg[0]=0;
			sprintf(cMsg, "loaded file: %s", fi->lpcName);
			m_StatusLine.SetWindowText(cMsg);
		}
		if (asi) {
			char* e; char* f;
			splitpathname(lpcName,&f,&e,NULL);
			e=&f[strlen(f)-1];
			while (*e-- != '.');
			*(e+1)=0;
			
			char f_[2048]; f_[0]=0;
			Str2UTF8(f, f_);
			asi->audiosource->SetName(f_);
			ucase(f_, f_);
			char* cDelayPos = strstr(f_,"DELAY");
			if (cDelayPos) cDelayPos+=6;
			char* cMSPos = strstr(f_,"MS");
			if (cMSPos) *cMSPos=0;
			if (cDelayPos && cMSPos) SetDelay(asi,atoi(cDelayPos));

			AddAudioStream(asi);
		}
		

	free(lpcExt);
	if (m_SourceFiles.GetCount()) {
		m_Add_Video_Source.EnableWindow(1);
	}

}

void CAVIMux_GUIDlg::AddAudioStream(AUDIO_STREAM_INFO* asi)
{
	TREE_ITEM_INFO*	tii;
	HTREEITEM		hItem1, hItem2;

	if (asi) {
		tii = BuildTIIfromASI(asi);
		tii->iOrgPos = m_AudioTree.GetRootCount();
		tii->iCurrPos = 2 * m_AudioTree.GetRootCount();

		m_AudioTree.SetItemData(hItem1=Tree_InsertCheck(&m_AudioTree,(char*)LPSTR_TEXTCALLBACK),
			(DWORD)tii);

		tii = new TREE_ITEM_INFO;
		tii->iID = TIIID_STRNAME;
		tii->pText = new char[256];
		ZeroMemory(tii->pText,256);
		asi->audiosource->GetName(tii->pText);
		asi->iSize = asi->audiosource->GetSize();
		m_AudioTree.SetItemData(hItem2=Tree_InsertCheck(&m_AudioTree,(char*)LPSTR_TEXTCALLBACK,hItem1),
			(DWORD)tii);
		m_AudioTree.ShowItemCheckBox(hItem2, false);

		tii = new TREE_ITEM_INFO;
		tii->iID = TIIID_LNGCODE;
		tii->pText = new char[256];
		ZeroMemory(tii->pText,256);
		asi->audiosource->GetLanguageCode(tii->pText);
		m_AudioTree.SetItemData(hItem2=Tree_InsertCheck(&m_AudioTree,(char*)LPSTR_TEXTCALLBACK,hItem1),
			(DWORD)tii);
		m_AudioTree.ShowItemCheckBox(hItem2, false);

		asi->audiosource->Enable(0);
	}
}

void CAVIMux_GUIDlg::AddSubtitleStream(SUBTITLE_STREAM_INFO* ssi)
{
	TREE_ITEM_INFO*	tii;
	HTREEITEM		hItem, hItem2;

	if (ssi) {
		tii = BuildTIIfromSSI(ssi);
		tii->iOrgPos = m_AudioTree.GetRootCount();
		tii->iCurrPos = 2 * m_AudioTree.GetRootCount();

		m_AudioTree.SetItemData(hItem=Tree_InsertCheck(&m_AudioTree,(char*)LPSTR_TEXTCALLBACK),
			(DWORD)tii);

		tii = new TREE_ITEM_INFO;
		tii->iID = TIIID_STRNAME;
		tii->pText = new char[256];
		ZeroMemory(tii->pText,256);
		ssi->lpsubs->GetName(tii->pText);
		m_AudioTree.SetItemData(hItem2=Tree_InsertCheck(&m_AudioTree,(char*)LPSTR_TEXTCALLBACK,hItem),
			(DWORD)tii);
		m_AudioTree.ShowItemCheckBox(hItem2, false);

		tii = new TREE_ITEM_INFO;
		tii->iID = TIIID_LNGCODE;
		tii->pText = new char[256];
		ZeroMemory(tii->pText,256);
		ssi->lpsubs->GetLanguageCode(tii->pText);
		
		m_AudioTree.SetItemData(hItem2=Tree_InsertCheck(&m_AudioTree,(char*)LPSTR_TEXTCALLBACK,hItem),
			(DWORD)tii);
		m_AudioTree.ShowItemCheckBox(hItem2, false);

	}
}

void CAVIMux_GUIDlg::OnAddFile() 
{
	CFileDialog*	cfd;
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen

	char	c[1000];

	c[0] = 0;
	strcat(c,"all supported file types|*.avi;*.mkv;*.mka;*.mks;*.mp2;*.mp3;*.ac3;*.dts;*.aac;*.ogg;*.srt;*.ssa;*.amg;*.wav;*.xml|");
	strcat(c,"AVI files (*.avi)|*.avi|");
	strcat(c,"Matroska files (*.mkv, *.mka, *.mks)|*.mkv;*.mka;*.mks|");
	strcat(c,"MPEG Layer 2/3 (*.mp2, *.mp3)|*.mp2;*.mp3|");
	strcat(c,"WAV files (*.wav)|*.wav|");
	strcat(c,"AC3 files (*.ac3)|*.ac3|");
	strcat(c,"DTS files (*.dts)|*.dts|");
	strcat(c,"AAC files (*.aac)|*.aac|");
	strcat(c,"OGG files (*.ogg)|*.ogg|");
	strcat(c,"supported subtitle files (*.srt, *.ssa)|*.srt;*.ssa|");
	strcat(c,"script files (*.amg)|*.amg|");
	strcat(c,"XML chapter files (*.xml)|*.xml|");
	strcat(c,"|");

	cfd= new CFileDialog(true,"avi","",0,c,NULL);
	if (cfd->DoModal()==IDOK)
	{
		doAddFile(cfd->GetPathName().GetBuffer(1024));

	};

	delete cfd;

}

void CAVIMux_GUIDlg::OnOK() 
{
	// TODO: Zusätzliche Prüfung hier einfügen
	CDialog::OnOK();
}

CProgressCtrl			slp;


int _stdcall AC3ResyncCB (__int64 qwPos,DWORD dwBad,DWORD dwUserData)
{
	MSG_LIST*	lpMsgList=((AC3_LOG*)dwUserData)->lpMessages;
	AC3_LOG*	lpac3log=(AC3_LOG*)dwUserData;
	char		buffer[128];
	int			dwHour,dwMin,dwSec,dwFrac;
	__int64	qwMillisec;
	DWORD		dwResult;

	lpac3log->bBroken=true;
	lpac3log->dwBrokenBytes+=dwBad;
	qwMillisec=(__int64)(d_div((double)qwPos*1000,(double)lpac3log->ac3source->GetAvgBytesPerSec(),
		"AC3ResyncCB::lpac3log->ac3source->GetAvgBytesPerSec()"));

	Millisec2HMSF(qwMillisec,&dwHour,&dwMin,&dwSec,&dwFrac);
	wsprintf(buffer,"  Stream %d, Pos. %1d:%02d:%02d.%03d: %d bytes  ",lpac3log->dwStream+1,
		dwHour,dwMin,dwSec,dwFrac,dwBad);
	MSG_LIST_append(lpMsgList,buffer);

	if (lpac3log->dwBrokenBytes>=(DWORD)lpac3log->ac3source->GetGranularity())
	{
		dwResult=AC3RCB_INSERTSILENCE | ((lpac3log->dwBrokenBytes/lpac3log->ac3source->GetGranularity())<<16);
		lpac3log->dwBrokenBytes%=lpac3log->ac3source->GetGranularity();
		return dwResult;
	}
	else
	{
		return AC3RCB_OK;
	}
}

void CAVIMux_GUIDlg::UpdateAudioName()
{
	char	cBuffer[256]; ZeroMemory(cBuffer,sizeof(cBuffer));
	AUDIO_STREAM_INFO*	asi = NULL;
	TREE_ITEM_INFO*		tii = NULL;

	HTREEITEM hItem = m_AudioTree.GetSelectedItem();
	if (hItem) {
		tii = m_AudioTree.GetItemInfo(hItem);
		m_AudioName.GetWindowText(cBuffer,sizeof(cBuffer));
		Str2UTF8(cBuffer,cBuffer);

		if (tii && tii->iID == TIIID_LNGCODE) {
			tii = m_AudioTree.GetItemInfo(m_AudioTree.GetParentItem(hItem));
			if (tii && tii->iID == TIIID_ASI) {
				tii->pASI->audiosource->SetLanguageCode(tii->pText);
			} else 
			if (tii && tii->iID == TIIID_SSI) {
				int i = m_Stream_Lng.GetCurSel();
				if (i != CB_ERR) {
					strcpy(tii->pText,(char*)m_Stream_Lng.GetItemData(i));
				}
				tii->pSSI->lpsubs->SetLanguageCode(tii->pText);
			}

		}
		if (tii && tii->iID == TIIID_STRNAME) {
			tii = m_AudioTree.GetItemInfo(m_AudioTree.GetParentItem(hItem));
			if (tii && tii->iID == TIIID_ASI) {
				tii->pASI->audiosource->SetName(cBuffer);
			} else
			if (tii && tii->iID == TIIID_SSI) {
				tii->pSSI->lpsubs->SetName(cBuffer);
			}
		}
	}
}

void CAVIMux_GUIDlg::UpdateAudiodelay()
{
	char	cBuffer[256];
	ZeroMemory(cBuffer,sizeof(cBuffer));
	AUDIO_STREAM_INFO*	asi = NULL;
	TREE_ITEM_INFO*		tii = NULL;

	HTREEITEM hItem = m_AudioTree.GetSelectedItem();
	
	if (hItem) {
		tii = m_AudioTree.GetItemInfo(hItem);
		if (tii && tii->iID == TIIID_ASI) {
			asi = tii->pASI;
		} else {
			if (hItem = m_AudioTree.GetParentItem(hItem)) {
				tii = m_AudioTree.GetItemInfo(hItem);
				if (tii && tii->iID == TIIID_ASI) {
					asi = tii->pASI;
				}
			}
		}
		if (asi) {
			m_Audiodelay.GetWindowText(cBuffer,sizeof(cBuffer));
			//asi->iDelay = atoi(cBuffer);
			SetDelay(asi,atoi(cBuffer));
		}
	}
}


void CAVIMux_GUIDlg::OnStart() 
{
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen
	CFileDialog*	cfd = NULL;
	DEST_AVI_INFO*	lpDAI;
	int				iIndex;
	int				i;//,j;
//	DWORD			dwAC3Streams;
	DWORD			dwAllAudio;
	DWORD			dwChunkOverhead;
	DWORD			dwDefaultAudio;
	DWORD			dwNoAudio;
	DWORD			dwEstimatedChunks;
	DWORD			dwEstimatedNbrOfFiles;
	DWORD			dwEstimatedReclists;
	DWORD			dwEstimatedOverhead;
	DWORD			dwIndexOverhead;
	DWORD			dwOvhUnit;
	DWORD			dwReclistOverhead;

	CDynIntArray*	audio_streams = NULL;
	CDynIntArray*	subtitles = NULL;

//	DWORD			dwSize;
	DWORD			dwUseMaxSize=0;

	__int64			qwEstimatedSize,qwFree;
	__int64			qwSizeOfExistingFiles;
	__int64			qwSpaceRequired;

	char			Buffer[200],Buffer2[200];
	CString			cStr,cStr2;
	char			cDrive[10];
	char			cFileSystem[10];
	char			cDebugFile[200];
	char			LongBuffer[1024];
	char			LongMsg[1024];
	FILESTREAM*		fsTemp;
	bool			bFileExists;
//	AC3_LOG*		lpAC3_log;
//	DWORD*			AC3_table;
	bool			bFileNumberNeglected;
	char			cBuffer[256];
	float			fPlaytime;
	RESOLUTION		resOutput;

	HTREEITEM		hItem;

	qwEstimatedSize=0;
	qwSizeOfExistingFiles=0;
	lpDAI=new DEST_AVI_INFO;
	ZeroMemory(lpDAI,sizeof(DEST_AVI_INFO));
	lpDAI->dwNbrOfVideoStreams = 1;
	if (SendDlgItemMessage(IDC_AVAILABLEVIDEOSTREAMS,LB_GETCOUNT,0,0)==0)
	{
//		cStr=LoadString(IDS_NOVIDEOSOURCE);
//		MessageBox(cStr,cstrError,MB_ICONERROR | MB_OK);
//		return;
		lpDAI->dwNbrOfVideoStreams = 0;
	};
	ButtonState_START();
	ApplyStreamSettings();
	lpDAI->bDoneDlg = sfOptions.bDispDoneDlg;
	lpDAI->dlg=this;
	lpDAI->hDebugFile=INVALID_HANDLE_VALUE;
// Formatierung für Numerierung
	if (sfOptions.dwDontUseNumbering) {
		lpDAI->lpFormat=new char[1+lstrlen("%s.avi")];
		lstrcpy(lpDAI->lpFormat,"%s.avi");
	}
	else
	{
		lpDAI->lpFormat=new char[(1+lstrlen(sfOptions.lpcNumbering))];
		lstrcpy(lpDAI->lpFormat,sfOptions.lpcNumbering);
	}
	lpDAI->lpProtocol=&m_Protocol;
	lpDAI->split_points = new CSplitPoints;
	lpDAI->settings = settings->Duplicate();
// maximale Dateizahl
	lpDAI->dwMaxFiles=(sfOptions.dwUseMaxFiles&&sfOptions.dwMaxFiles)?sfOptions.dwMaxFiles:1<<31-2;
// Audiointerleave
	if (!lpDAI->settings->GetInt("output/avi/audio interleave/value")) {
		lpDAI->settings->SetInt("output/avi/audio interleave/value", 1);
	}
// Video
	qwEstimatedSize = 0;
	lpDAI->videosource = NULL;

	if (lpDAI->dwNbrOfVideoStreams) {
		iIndex = m_VideoSources.GetCurSel();
		if (iIndex==LB_ERR) iIndex = 0;
		lpDAI->videosource=(VIDEOSOURCE*)m_VideoSources.GetItemData(iIndex);
//		lpDAI->videosource->Seek(0);
		qwEstimatedSize+=lpDAI->videosource->GetSize();
	}
	
	lpDAI->i1stTimecode = sfOptions.i1stTimecode;

	if (sfOptions.dwUseManualSplitPoints) sfOptions.split_points->Duplicate(lpDAI->split_points);
	
	m_VideoStretchFactor.GetWindowText(cStr);
	lpDAI->dVideoStretchFactor = atof(cStr);
	m_Title.GetWindowText(cStr);
	if (cStr.GetLength()) {
		char cUTF8[1000]; cUTF8[0]=0;
		Str2UTF8(cStr.GetBuffer(1000),cUTF8);
		lpDAI->cTitle = new CStringBuffer(cUTF8);
	}

//	lpDAI->mkv.iLaceStyle = sfOptions.mkv.iLaceStyle;
	lpDAI->chapters = chapters;

	if (lpDAI->dwNbrOfVideoStreams) {
		m_OutputResolution.GetWindowText(cStr);
		lpDAI->videosource->GetResolution(&resOutput.iWidth, &resOutput.iHeight);
		if (Str2Resolution(cStr.GetBuffer(256),&resOutput) == STRF_OK) {
			lpDAI->videosource->SetOutputResolution(&resOutput);
		}
	}

// maximale Größe in MB beachten?
	dwUseMaxSize=sfOptions.dwUseMaxFileSize;
// maximale Dateigröße

	if (dwUseMaxSize)
	{
		lpDAI->qwMaxSize=sfOptions.dwMaxFileSize;//atoi(Buffer);
		if (!lpDAI->qwMaxSize)
		{
			if (!lpDAI->settings->GetInt("output/avi/opendml/on")) lpDAI->qwMaxSize=2000; else lpDAI->qwMaxSize=1<<31-1;
		}
	}	
	else
	{
		lpDAI->qwMaxSize=(1<<31-1);
	}

// Padding
	lpDAI->dwPadding=2;
// maximale Frames
	lpDAI->dwMaxFrames=sfOptions.dwFrames;//atoi(Buffer);
	if (!(lpDAI->dwMaxFrames) && lpDAI->dwNbrOfVideoStreams) lpDAI->dwMaxFrames=lpDAI->videosource->GetNbrOfFrames();
	
	if (lpDAI->dwNbrOfVideoStreams) {
		lpDAI->dwMaxFrames=min((int)lpDAI->dwMaxFrames,(int)lpDAI->videosource->GetNbrOfFrames());
		if (lpDAI->dwMaxFrames) {
			lpDAI->iDurationFlags = DAI_DF_FRAMES;
		} else {
			lpDAI->iMaxDuration = lpDAI->videosource->GetDuration();
			lpDAI->iDurationFlags = DAI_DF_DURATION;
		}
	}

// Preload
	lpDAI->dwPreload=sfOptions.dwPreload;//atoi(Buffer);
// Legacy Index
	//lpDAI->avi.iLegacyIndex=((sfOptions.dwLegacyIndex)&&(lpDAI->avi.iOpenDML))?1:0;
	lpDAI->settings->SetInt("output/avi/legacyindex", 
		(lpDAI->settings->GetInt("output/avi/opendml/on") && lpDAI->settings->GetInt("output/avi/legacyindex")));
// Audiostreams
	dwNoAudio=(IsDlgButtonChecked(IDC_NO_AUDIO)==BST_CHECKED)?1:0;

	audio_streams = new CDynIntArray;
	audio_streams = m_AudioTree.GetItems(m_AudioTree.GetRootItem(),TIIID_ASI,-1);

	for (i=0;i<audio_streams->GetCount();i++) {
		TREE_ITEM_INFO*	tii = m_AudioTree.GetItemInfo((HTREEITEM)audio_streams->At(i));
		tii->pASI->audiosource->Enable(0);
	}
	audio_streams->DeleteAll();
	delete audio_streams;
	if (dwNoAudio)
	{
		audio_streams = new CDynIntArray;
	}
	else
	{
		dwDefaultAudio=(IsDlgButtonChecked(IDC_DEFAULT_AUDIO)==BST_CHECKED)?1:0;
		if (dwDefaultAudio)
		{
			SendDlgItemMessage(IDC_DEFAULT_AUDIO_NUMBER,WM_GETTEXT,sizeof(Buffer),(LPARAM)Buffer);
			dwDefaultAudio=atoi(Buffer);
			hItem = m_AudioTree.GetRootItem();
			for (i=0;i<(int)dwDefaultAudio-1;i++) {
				hItem = m_AudioTree.GetNextSiblingItem(hItem);
			}

			audio_streams = new CDynIntArray;
			audio_streams->Insert((int)hItem);

		}
		else
		{
			dwAllAudio=(IsDlgButtonChecked(IDC_ALL_AUDIO)==BST_CHECKED)?1:0;
			if (!dwAllAudio) {
				audio_streams = m_AudioTree.GetItems(m_AudioTree.GetRootItem(),TIIID_ASI,0);
			} else {
				audio_streams = m_AudioTree.GetItems(m_AudioTree.GetRootItem(),TIIID_ASI,-1);
			}
		}
	}
	
	for (i=0;i<audio_streams->GetCount();i++) {
		TREE_ITEM_INFO*	tii = m_AudioTree.GetItemInfo((HTREEITEM)audio_streams->At(i));
		tii->pASI->audiosource->Enable(1);
	}

	lpDAI->dwNbrOfAudioStreams=audio_streams->GetCount();
	lpDAI->asi=new AUDIO_STREAM_INFO*[1+audio_streams->GetCount()];

// Geht nicht, wenn weder Video noch Audio verfügbar ist
	if (!lpDAI->dwNbrOfAudioStreams && !lpDAI->dwNbrOfVideoStreams) {
		ButtonState_STOP();
		return;
	}

// setup for AC3 log
//	dwAC3Streams=0;

	for (i=0;i<(int)lpDAI->dwNbrOfAudioStreams;i++)
	{
		TREE_ITEM_INFO*	tii = m_AudioTree.GetItemInfo((HTREEITEM)audio_streams->At(i));
		lpDAI->asi[i]=tii->pASI;
//		lpDAI->asi[i]->audiosource->Seek(0);
		
		qwEstimatedSize+=lpDAI->asi[i]->audiosource->GetSize();
		if (lpDAI->asi[i]->dwType==AUDIOTYPE_AC3)
		{
	//		dwAC3Streams++;
		}
/*		if (lpDAI->asi[i]->dwType==AUDIOTYPE_MP3CBR)
		{
			if (sfOptions.dwMP3CBRFrameMode||(lpDAI->asi[i]->iDelay))
			{
				lpDAI->asi[i]->audiosource->SetFrameMode(FRAMEMODE_ON);
			}
			else
			{
				lpDAI->asi[i]->audiosource->SetFrameMode(FRAMEMODE_OFF);
			}
		}*/
	}

/*	if (dwAC3Streams)
	{
		AC3_table=new DWORD[1+dwAC3Streams];
		for (i=0,j=0;i<(int)lpDAI->dwNbrOfAudioStreams;i++)
		{
			if (lpDAI->asi[i]->dwType==AUDIOTYPE_AC3) 
			{
				if (lpDAI->asi[i]->audiosource->IsCBR()) {
					lpDAI->asi[i]->audiosource->SetFrameMode(lpDAI->settings->GetInt("output/avi/ac3/frames per chunk"));//sfOptions.iAC3FramesPerChunk);
				} else {
					lpDAI->asi[i]->audiosource->SetFrameMode(2);
				}
				AC3_table[j++]=i;
			}
		}
	}
	else
	{
		AC3_table=NULL;
	}

	lpDAI->lpAC3_logs=NULL;
	if (dwAC3Streams)
	{
		lpDAI->lpAC3_logs=new AC3_LOG*[1+dwAC3Streams];
	}

	lpDAI->dwNbrOfAC3Streams=dwAC3Streams;

	for (i=0;i<(int)dwAC3Streams;i++)
	{
		lpAC3_log=new AC3_LOG;
		ZeroMemory(lpAC3_log,sizeof(AC3_LOG));
		lpAC3_log->dwStream=AC3_table[i];
		lpAC3_log->lpMessages=new MSG_LIST;
		lpAC3_log->ac3source=((AC3SOURCE*)(lpDAI->asi[AC3_table[i]]->audiosource));
		ZeroMemory(lpAC3_log->lpMessages,sizeof(MSG_LIST));

		lpDAI->lpAC3_logs[i]=lpAC3_log;
	}

	if (AC3_table) free(AC3_table);*/

// Subtitles
	subtitles = new CDynIntArray;
	subtitles = m_AudioTree.GetItems(m_AudioTree.GetRootItem(),TIIID_SSI,-1);

	for (i=0;i<subtitles->GetCount();i++) {
		TREE_ITEM_INFO*	tii = m_AudioTree.GetItemInfo((HTREEITEM)subtitles->At(i));
		tii->pSSI->lpsubs->Enable(0);
	}
	subtitles->DeleteAll();
	delete subtitles;

	if (IsDlgButtonChecked(IDC_ALL_SUBTITLES)==BST_CHECKED) {
		subtitles = m_AudioTree.GetItems(m_AudioTree.GetRootItem(),TIIID_SSI,-1);
	} else
	if (IsDlgButtonChecked(IDC_NO_SUBTITLES)==BST_CHECKED) {
		subtitles = new CDynIntArray;
	} else {
		subtitles = m_AudioTree.GetItems(m_AudioTree.GetRootItem(),TIIID_SSI,1);
	}

	if (subtitles->GetCount())
	{
		lpDAI->ssi=new SUBTITLE_STREAM_INFO*[subtitles->GetCount()];
		for (i=0;i<subtitles->GetCount();i++)
		{
			HTREEITEM hItem = (HTREEITEM)subtitles->At(i);
			TREE_ITEM_INFO* tii = m_AudioTree.GetItemInfo(hItem);
			lpDAI->ssi[i]=tii->pSSI;
			wsprintf(Buffer,"Subtitle %d",i+1);
			if (!(lpDAI->ssi[i]->lpsubs->GetName(NULL)))
			{
				lpDAI->ssi[i]->lpsubs->SetName(Buffer);
			}
			lpDAI->ssi[i]->lpsubs->Enable(1);
/*			hChild = m_AudioTree.GetChildItem(hItem);
			while (hChild) {
				tii = (TREE_ITEM_INFO*)m_AudioTree.GetItemData(hChild);
				switch (tii->iID) {
					case TIIID_LNGCODE:
						lpDAI->ssi[i]->lpsubs->SetLanguageCode(tii->pText);
						break;
				}
			}
*/		}
	}
	lpDAI->dwNbrOfSubs=subtitles->GetCount();

	if (lpDAI->dwNbrOfVideoStreams) {
		if (lpDAI->iDurationFlags == DAI_DF_FRAMES) {
			qwEstimatedSize=(__int64)(qwEstimatedSize*d_div((double)lpDAI->dwMaxFrames,(double)lpDAI->videosource->GetNbrOfFrames(),
				"OnStart()::lpDAI->videosource->GetNbrOfFrames()"));
		} else	{
			qwEstimatedSize=(__int64)(qwEstimatedSize*d_div((double)lpDAI->iMaxDuration,(double)lpDAI->videosource->GetDuration(),
				"OnStart()::lpDAI->videosource->GetNbrOfFrames()"));
		}
	}

// StdAVI>2GB abfangen
	bool bOpenDML = !!lpDAI->settings->GetInt("output/avi/opendml/on");
	bool bRecLists = !!lpDAI->settings->GetInt("output/avi/reclists");
	bool bLegacy = !!lpDAI->settings->GetInt("output/avi/legacyindex");
	bool bLowOvhd = !!lpDAI->settings->GetInt("output/avi/opendml/haalimode");
	if ((!bOpenDML)&&(lpDAI->qwMaxSize>2040)&&(qwEstimatedSize>2040*(1<<20)))
	{
		cStr=LoadString(IDS_OVER2GBONLYODML);
		switch (MessageBox(cStr,cstrError,MB_YESNO | MB_ICONERROR))
		{
			case IDYES:
				lpDAI->settings->SetInt("output/avi/opendml/on", 1);
				bOpenDML = 1;
				break;
			case IDNO:
				//lpDAI->videosource->AllowAVIOutput(false);
				break;
		}
	}

// more than 9 streams with standard AVI?
	if (lpDAI->videosource && (!bOpenDML)&&(lpDAI->dwNbrOfAudioStreams+lpDAI->dwNbrOfSubs>=9))
	{
		cStr=LoadString(IDS_TOOMANYSTRFORSTDAVI);
		switch (MessageBox(cStr,cstrError,MB_YESNO | MB_ICONERROR))
		{
			case IDYES:
				lpDAI->settings->SetInt("output/avi/opendml/on", 1);
				bOpenDML = 1;
				break;
			case IDNO:
				lpDAI->videosource->AllowAVIOutput(false);
				break;
		}
	}
// Standard-Index
	if (settings->GetInt("output/avi/opendml/stdindex/pattern")==SIP_AUTO)
	{
			lpDAI->settings->SetInt("output/avi/opendml/stdindex/pattern", SIP_FRAMES);
			lpDAI->settings->SetInt("output/avi/opendml/stdindex/interval", 15000);
	}

// OpenDML + many audio streams => check standard index setting
	int iStdIndP = (int)lpDAI->settings->GetInt("output/avi/opendml/stdindex/pattern");
	if ((lpDAI->dwNbrOfAudioStreams>3)&&(bOpenDML)&&(iStdIndP == SIP_RIFF))
	{
		cStr=LoadString(IDS_ODML3AUDIO);
		wsprintf(cBuffer,cStr);
		if (MessageBox(cBuffer,cstrWarning,MB_YESNO | MB_ICONWARNING | MB_DEFBUTTON2)==IDNO)
		{
			OnStop();
			return;
		};
	}

// Open-file-options
	lpDAI->ofoOptions=ofOptions;

//	lpDAI->avi.iInterleaveScheme=(sfOptions.dwAudioInterleaveUnit==AIU_KB)?DAI_IS_KB:DAI_IS_FRAME;

// Guess overhead for AVI
	dwEstimatedChunks=lpDAI->dwMaxFrames;
	if (lpDAI->dwNbrOfVideoStreams) {
		fPlaytime=(float)lpDAI->dwMaxFrames*lpDAI->videosource->GetNanoSecPerFrame()/1000000000;
	} else {
		fPlaytime = 0;
	}
	dwIndexOverhead=NULL;
	DWORD chunks = 0;
	for (i=0;i<(int)lpDAI->dwNbrOfAudioStreams;i++)
	{
		switch (lpDAI->asi[i]->dwType)
		{
			case AUDIOTYPE_MP3VBR:
				switch (lpDAI->asi[i]->audiosource->GetFrequency())
				{
					case 48000: dwEstimatedChunks+=(DWORD)(fPlaytime*1000/(24*settings->GetInt("output/avi/mp3/frames per chunk"))); break;
					case 44100: dwEstimatedChunks+=(DWORD)(fPlaytime*1000/(26.112*settings->GetInt("output/avi/ac3/frames per chunk"))); break;
				}
				break;
			case AUDIOTYPE_MP3CBR:
				dwEstimatedChunks+=(DWORD)(fPlaytime/0.75); break;
			case AUDIOTYPE_AC3:
				dwEstimatedChunks+=(DWORD)(fPlaytime*1000/(32*settings->GetInt("output/avi/ac3/frames per chunk"))); break; //sfOptions.iAC3FramesPerChunk)); break;
			case AUDIOTYPE_DTS:
				chunks = (DWORD)(fPlaytime*1000/21);
				chunks /= settings->GetInt("output/avi/dts/frames per chunk");
				dwEstimatedChunks += chunks; break;
			default:
				dwEstimatedChunks+=(DWORD)(fPlaytime/0.75); break;
		}
	}

	if (!bOpenDML && !bLegacy && bLowOvhd) dwEstimatedChunks = 0;

	dwChunkOverhead=dwEstimatedChunks*8;
	dwReclistOverhead=0;

	dwOvhUnit=(bOpenDML)?8:16;
	dwIndexOverhead=dwEstimatedChunks*dwOvhUnit;
	CAttribs* ai = lpDAI->settings->GetAttr("output/avi/audio interleave");
	if (bRecLists)
	{
		switch ((int)ai->GetInt("unit"))
		{
			case AIU_KB:
				dwEstimatedReclists=(DWORD)(qwEstimatedSize/1000/(int)ai->GetInt("value")); break;
			case AIU_FRAME:
				dwEstimatedReclists=lpDAI->dwMaxFrames/(int)ai->GetInt("value"); break;
		}
		dwReclistOverhead=dwEstimatedReclists*12;
		if (!bOpenDML)
		{
			dwIndexOverhead+=dwEstimatedReclists*16;
		}
	}

	dwEstimatedNbrOfFiles=(DWORD)d_div((double)qwEstimatedSize,(double)lpDAI->qwMaxSize*(1<<20),
		"OnStart()::lpDAI->qwMaxSize")+1;

	dwEstimatedNbrOfFiles=min(dwEstimatedNbrOfFiles,lpDAI->dwMaxFiles);

	if (bOpenDML && bLegacy) { 
		dwIndexOverhead*=3;
	}

	dwEstimatedOverhead=dwChunkOverhead+dwIndexOverhead+dwReclistOverhead+
		dwEstimatedNbrOfFiles*((bOpenDML)?65536:4096);

	// mehrere Dateien, aber kein %d?
	bFileNumberNeglected=false;
	if (dwEstimatedNbrOfFiles>1)
	{
		ZeroMemory(Buffer,sizeof(Buffer));
		ZeroMemory(Buffer2,sizeof(Buffer2));
		wsprintf(Buffer,lpDAI->lpFormat,Buffer,0);
		wsprintf(Buffer2,lpDAI->lpFormat,Buffer2,1);
		if (!lstrcmpi(Buffer,Buffer2)) {
			bFileNumberNeglected=true;
			cStr=LoadString(IDS_NOPERCENT_D);
			wsprintf(Buffer,cStr);
			if (MessageBox(Buffer,cstrConfirmation,MB_YESNO | MB_ICONWARNING | MB_DEFBUTTON2)==IDNO) {
					OnStop();
					return;
			}
		}
	}

	int iAllowAVIOutput = 1;
	if (!cExtFilename) {
		char* cFormats[2];
		char* c;
		char* cFmt2;
		if (lpDAI->dwNbrOfVideoStreams) {
			cFmt2 = "mkv";
		} else {
			cFmt2 = "mka";
		}
		char* cStd[2] = {"avi",cFmt2};

		cFormats[0] = new char[64]; ZeroMemory(cFormats[0],64);
		cFormats[1] = new char[64]; ZeroMemory(cFormats[1],64);
		c = new char[256]; ZeroMemory(c,256);

		int	i = sfOptions.iStdOutputFormat;
		int j;

		int iAudio2AVIpossibe = 1;
		for (j=0;j<(int)lpDAI->dwNbrOfAudioStreams;j++) {
			if (!lpDAI->asi[j]->audiosource->IsAVIOutputPossible()) {
				iAudio2AVIpossibe = false;
			}
		}
		for (j=0;j<(int)lpDAI->dwNbrOfSubs;j++) {
			if (!lpDAI->ssi[j]->lpsubs->IsAVIOutputPossible()) {
				iAudio2AVIpossibe = false;
			}
		}

		if (lpDAI->dwNbrOfVideoStreams && lpDAI->videosource->IsAVIOutputPossible() && iAudio2AVIpossibe) {
			strcat(cFormats[i],"AVI files (*.avi)|*.avi|");
			iAllowAVIOutput = 1;
		} else iAllowAVIOutput = 0;
		
		strcat(cFormats[i^1],"Matroska-Files (*.");
		strcat(cFormats[i^1],cFmt2);
		strcat(cFormats[i^1],")|*.");
		strcat(cFormats[i^1],cFmt2);
		strcat(cFormats[i^1],"|");
		strcat(c,cFormats[0]);
		strcat(c,cFormats[1]);
		strcat(c,"|");

		delete cFormats[0]; delete cFormats[1];

		cfd=new CFileDialog(false,cStd[i],"",0,c,NULL);
		delete c;
		cfd->m_ofn.Flags&=~OFN_OVERWRITEPROMPT;
	}

	if (cExtFilename || cfd->DoModal()==IDOK)
	{
	// Dateiname
		if (cfd) {
			lpDAI->lpFileName=new char[1+cfd->GetPathName().GetLength()];
			wsprintf(cDrive,"%c:\\",cfd->GetPathName().GetAt(0));
			lstrcpy(lpDAI->lpFileName,cfd->GetPathName().GetBuffer(255));
		}
		else
		{
			lpDAI->lpFileName=new char[1+lstrlen(cExtFilename)];
			wsprintf(cDrive,"%c:\\",cExtFilename[0]);
			lstrcpy(lpDAI->lpFileName,cExtFilename);
		}

	// determine output file type: avi or mkv
		int iPos = strlen(lpDAI->lpFileName)-1;
		while (lpDAI->lpFileName[iPos-1]!='.') iPos--;
		char* cExt = &lpDAI->lpFileName[iPos];

		i = strlen(lpDAI->lpFormat)-1;
		while (lpDAI->lpFormat[i-1]!='.') i--;
		lpDAI->lpFormat[i] = 0;

		if (!stricmp(cExt,"avi")) {
			if (!iAllowAVIOutput) {
				sprintf(Buffer, "Error: User is too stupid\nReason: User tried to choose AVI even though it has not been listed among the available output formats!");
				MessageBox(Buffer, "Error", MB_OK | MB_ICONERROR);
				OnStop();
				return;
			}
			lpDAI->iOutputFormat = DOF_AVI;
			strcat(lpDAI->lpFormat,"avi");
			QW2Str(dwEstimatedOverhead,Buffer2,10);
			wsprintf(Buffer,LoadString(STR_MUXLOG_ESTOVERHEAD),Buffer2,LoadString(STR_BYTES));
			AddProtocolLine(Buffer,5);

		}
		
		if (!stricmp(cExt,"mkv") || !stricmp(cExt,"mka")) {
			lpDAI->iOutputFormat = DOF_MKV;
			strcat(lpDAI->lpFormat,cExt);
		}


		GetVolumeInformation(cDrive,NULL,0,NULL,NULL,NULL,cFileSystem,sizeof(cFileSystem));
    // Idioten abfangen, die > 4GB auf FAT speichern wollen
		if (lpDAI->qwMaxSize>4000)
		{
			if (lstrcmp(cFileSystem,"NTFS"))
			{
				cStr=LoadString(IDS_OVER4GBONLYNTFS);
				wsprintf(Buffer,cStr,cDrive,cFileSystem);
				if (MessageBox(Buffer,cstrError,MB_YESNO | MB_ICONQUESTION)==IDNO)
				{
					OnStop();
					if (cfd) delete cfd;
					return;
				}
				else
				{
					lpDAI->qwMaxSize=4000;
				}
			}
		}
    // kann es überhaupt passen?
		char* cFilename, *cExtension, *cPath;
		cPath = new char[512];

		splitpathname(lpDAI->lpFileName,&cFilename,&cExtension,&cPath);

		GetDiskFreeSpaceEx(cPath,NULL,NULL,(ULARGE_INTEGER*)&qwFree);
		delete cPath;
		if (cExtFilename) { 
			delete cExtFilename;
			cExtFilename = NULL; }
    // welche Dateien gibts schon?
		ZeroMemory(LongBuffer,sizeof(LongBuffer));
		cStr=LoadString(IDS_EXISTINGFILES);
		bFileExists=false;
		fsTemp=new FILESTREAM;
		for (i=(bFileNumberNeglected)?1:dwEstimatedNbrOfFiles;i;i--)
		{
			ZeroMemory(Buffer,sizeof(Buffer));
			memcpy(Buffer,lpDAI->lpFileName,lstrlen(lpDAI->lpFileName)-4);
			wsprintf(Buffer,lpDAI->lpFormat,Buffer,i);
			if (fsTemp->Open(Buffer,FA_READ)==STREAM_OK)
			{
				/*if (!lpDAI->lpqwSplitPoints) */qwSizeOfExistingFiles+=fsTemp->GetSize();
				wsprintf(LongBuffer,"%s%c%c%s",LongBuffer,13,10,Buffer);
				bFileExists=true;
				fsTemp->Close();
			};
			
		}
		if (bFileExists && sfOptions.bDispOverwriteDlg)
		{
			cStr2=LoadString(IDS_CONTINUE);
			wsprintf(LongMsg,"%s%c%c%s%c%c%c%c%s",cStr.GetBuffer(255),
				13,10,LongBuffer,13,10,13,10,cStr2.GetBuffer(255));
			if (MessageBox(LongMsg,cstrConfirmation,MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2)==IDNO)
			{
				free(fsTemp);
				if (cfd) delete cfd;
				OnStop();
				return;
			}
		}
		
		free(fsTemp);
		qwFree+=qwSizeOfExistingFiles;
		qwSpaceRequired=(__int64)((qwEstimatedSize+1.1*dwEstimatedOverhead));

		if (lpDAI->iOutputFormat == DOF_AVI) {
			QW2Str(qwSpaceRequired,Buffer2,10);
			wsprintf(Buffer,LoadString(STR_MUXLOG_ESTTOTAL),Buffer2,LoadString(STR_BYTES));
			AddProtocolLine(Buffer,5);
		}

		qwSpaceRequired/=1048576;
		if (qwFree/1048576<(1.01*qwSpaceRequired))
		{
			cStr=LoadString(IDS_NOTENOUGHSPACE);
			wsprintf(Buffer,cStr.GetBuffer(255),(DWORD)(qwFree/1048576),qwSpaceRequired);
			if (MessageBox(Buffer,cstrWarning,MB_YESNO | MB_ICONWARNING | MB_DEFBUTTON2)==IDNO)
			{
				OnStop();
				if (cfd) delete cfd;
				return;
			}
		}
	// Namen ohne Erweiterung
		wsprintf(cDebugFile,"%s - debug-output.txt",lpDAI->lpFileName);
	//	subtitles->DeleteAll();
	//	delete subtitles;
		lpDAI->bExitAfterwards=sfOptions.bExitAfterwards;
		
		if (lpDAI->dwNbrOfVideoStreams) lpDAI->videosource->Enable(1);
		SetDialogState_Muxing();
		doSaveConfig(lastjobfile);
		if (lpDAI->iOutputFormat == DOF_AVI) AfxBeginThread((AFX_THREADPROC)MuxThread_AVI,lpDAI);
		if (lpDAI->iOutputFormat == DOF_MKV) AfxBeginThread((AFX_THREADPROC)MuxThread_MKV,lpDAI);
		sfOptions.bDispDoneDlg=true;
		sfOptions.bDispOverwriteDlg=true;
		sfOptions.bExitAfterwards=false;

	}
	else
	{
		OnStop();
	}

	if (audio_streams) {
		audio_streams->DeleteAll();
		delete audio_streams;
	}
	if (subtitles) {
		subtitles->DeleteAll();
		delete subtitles;
	}
	if (cfd) delete cfd;
}

DWORD CAVIMux_GUIDlg::Clear()
{
	// TODO: Zusätzlichen Bereinigungscode hier einfügen
	int					i;
	int					iNbr;
	AUDIO_STREAM_INFO*	asi;
	FILE_INFO*			fi;
	VIDEOSOURCE*		al;
	VIDEO_SOURCE_INFORMATION*	lpvsi;
	SUBTITLE_STREAM_INFO* ssi;
	AUDIOSOURCELIST*		asl;
	TREE_ITEM_INFO*			tii;
	HTREEITEM			hItem,hChild;

	m_Add_Video_Source.EnableWindow(0);
	chapters->Delete();
	iNbr=m_AudioTree.GetCount();
	hItem = m_AudioTree.GetRootItem();
	m_OutputResolution.SetWindowText("");
	m_Title.SetWindowText("");
	while (hItem)
	{
		HTREEITEM hoc;

		hChild = m_AudioTree.GetChildItem(hItem);
		while (hChild) {
			tii=m_AudioTree.GetItemInfo(hChild);
			if (tii->iID == TIIID_ASI) asi = tii->pASI;
			if (tii->iID == TIIID_LNGCODE && tii->pText) delete tii->pText;
			if (tii->iID == TIIID_STRNAME && tii->pText) delete tii->pText;
			tii->pASI = NULL;
			delete tii;
			hoc = hChild;
			hChild = m_AudioTree.GetNextSiblingItem(hChild);
			m_AudioTree.DeleteItem(hoc);
		}

		tii=m_AudioTree.GetItemInfo(hItem);
		
		hoc = hItem;
		hItem = m_AudioTree.GetNextSiblingItem(hItem);
		m_AudioTree.DeleteItem(hoc);

		if (tii->iID == TIIID_ASI) {
			asi=tii->pASI;
			asl = (AUDIOSOURCELIST*)asi->audiosource;
			asl->Close();
			delete asl;
			if (asi->dwFlags&ASIF_ALLOCATED) {
				free (asi->lpASH);
				free (asi->lpFormat);
			}
			delete asi;
		} else
		if (tii->iID == TIIID_SSI) {
			ssi= tii->pSSI;
			ssi->lpsubs->Close();
			delete ssi->lpsubs;
			delete ssi;
			tii->pSSI = NULL;
		}

		delete tii;
	}

	iNbr=m_VideoSources.GetCount();
	for (i=0;i<iNbr;i++)
	{
		al=(VIDEOSOURCE*)m_VideoSources.GetItemData(i);
		lpvsi=(VIDEO_SOURCE_INFORMATION*)al->GetUserData();

		delete lpvsi->lpdwFiles;
		delete lpvsi;
		al->Close(false);
		delete al;
	}

	iNbr=m_SourceFiles.GetCount();
	for (i=0;i<iNbr;i++)
	{
		CBuffer*	cb;

		cb = (CBuffer*)m_SourceFiles.GetItemData(i);
		fi=(FILE_INFO*)cb->GetData();
		if (fi->lpcName) free(fi->lpcName);
		fi->lpcName=NULL;
		if (fi->lpM2F2)
		{
			fi->lpM2F2->Close();
			delete fi->lpM2F2;
			fi->lpM2F2=NULL;
		}
		if (fi->cache)
		{
			//CACHESTATS	cs;
			//((CACHEDSTREAM*)fi->cache)->GetCacheStats(&cs);
			//fprintf(stderr,"Hits: %d, Misses: %d",cs.iHits,cs.iMisses);
			fi->cache->Close();
			delete fi->cache;
		}
		if (fi->file)
		{
			fi->file->Close();
			delete fi->file;
		}
		if (fi->dwType & FILETYPE_AVI)
		{
			fi->AVIFile->Close(false);
			delete fi->AVIFile;
		}
		if (fi->dwType & FILETYPE_MKV)
		{
			fi->MKVFile->Close();
			delete fi->MKVFile;
		}
		DecBufferRefCount(&cb);
	}

	m_VideoSources.ResetContent();
	m_SourceFiles.ResetContent();
	m_SourceFiles.AllowMoving(true);
	m_Audiodelay.SetWindowText("0");
	m_AudioName.SetWindowText("");
	sfOptions.split_points->DeleteAll();
	sfOptions.chapters->Delete();

	return 1;
}


void CAVIMux_GUIDlg::OnCancel() 
{
	char	Filename[500];
	Clear();

	lstrcpy(Filename,cfgfile);
	lstrcat(Filename,".amg");

	// write preferences as script file

	doSaveConfig(Filename, 0);
	
	if (sfOptions.lpcNumbering) free(sfOptions.lpcNumbering);
	sfOptions.lpcNumbering=NULL;

	for (int i=0;i<(int)dwLanguages;UnloadLanguageFile(lplpLanguages[i++]));
	delete lplpLanguages;

	m_Protocol.DeleteAllItems();
	delete m_Protocol.GetFont();
	m_AudioTree.DeleteAllItems();
	delete m_AudioTree.GetFont();
	
	delete dwLangs;	
	delete chapters;
	delete sfOptions.split_points;
	settings->Delete();
	delete settings;
	CloseHandle(hGlobalMuxSemaphore);
	if (hLogFile) CloseHandle(hLogFile);

	CDialog::OnCancel();
}

void CAVIMux_GUIDlg::OnStop() 
{
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen
	ButtonState_STOP();
	StopMuxing(true);
}

void CAVIMux_GUIDlg::ButtonState_STOP()
{
	GetDlgItem(IDC_START)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_STOP)->ShowWindow(SW_HIDE);//EnableWindow(0);
	GetDlgItem(IDCANCEL)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_OUTPUTOPTIONS)->ShowWindow(SW_SHOW);
	
	iButtonState = 1;
}

void CAVIMux_GUIDlg::ButtonState_START()
{
	GetDlgItem(IDC_START)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_STOP)->ShowWindow(SW_SHOW);
	GetDlgItem(IDCANCEL)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_OUTPUTOPTIONS)->ShowWindow(SW_HIDE);
	iButtonState = 2;
}

void name_without_ext(char* lpcName, char* cNoExt)
{
	char*					cFilename = NULL;
	char*					cExtension = NULL;

	splitpathname(lpcName,&cFilename,&cExtension);
	strcpy(cNoExt,cFilename);
	int j = strlen(cNoExt);
	while (*(cNoExt+j)!='.' && j) {
		j--;
	}
	if (j) *(cNoExt+j)=0;
}




void CAVIMux_GUIDlg::OnLButtonUp(UINT nFlags, CPoint point) 
{
	// TODO: Code für die Behandlungsroutine für Nachrichten hier einfügen und/oder Standard aufrufen
	
	CDialog::OnLButtonUp(nFlags, point);
}

void CAVIMux_GUIDlg::OnMaxsizeExtended() 
{
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen

}

void CAVIMux_GUIDlg::OnClear() 
{
	CString		cStr;

	cStr=LoadString(IDS_REALLYCLEARALL);

	if (MessageBox(cStr,cstrConfirmation,MB_YESNO | MB_ICONQUESTION)==IDYES)
	{
		Clear();
	}
}


void CAVIMux_GUIDlg::OnUsemaxsize() 
{
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen
	
}

typedef struct
{
	DWORD   dwID;
	DWORD	dwSize;
	DWORD	dwVersion;
	DWORD	dwUseMaxSize;
	DWORD	dwMaxSize;
	DWORD	dwPreload;
	DWORD	dwAudioInterleave;
	DWORD	dwOpenDML;
	DWORD	dwRECLists;
	DWORD	dwMaxFrames;
	char	cFormat[50];
	DWORD   dwUseSplitList;
	DWORD   dwMakeLegacyIndex;
} CONFIGDATA, *LPCONFIGDATA;

void CAVIMux_GUIDlg::doSaveConfig(char* lpcFile, int clear)
{
	FILE*			file;
	int				i,j;
	bool			bStore;
	DWORD			dwNbrOfFiles;
	DWORD			dwNbrOfVS;
	DWORD			dwNbrOfSelAS;
	DWORD			dwNbrOfAS;
	CBuffer*		cb;
	FILE_INFO*		lpfi[2];
	DWORD			dwlpfic=0;
	DWORD*			lpdwSelectedFiles;
	CString			s;

	VIDEOSOURCE*	lpvs;
	VIDEO_SOURCE_INFORMATION*	lpvsi;
	AUDIO_STREAM_INFO*	lpasi;
	TREE_ITEM_INFO*	tii;
	HTREEITEM		hItem;
	char			cBuffer[8192];
	bool			bWith;

	CDynIntArray*	audio_stream_items, *audio_stream_indices;
	CDynIntArray*	subs_stream_items, *subs_stream_indices;
	ApplyStreamSettings();

	// Files
		file=fopen(lpcFile,"w");
		fprintf(file, cUTF8Hdr);
		if (clear) fprintf(file,"CLEAR\n");
		fprintf(file,"LANGUAGE %s\n",GetCurrentLanguage()->lpcName);
		fprintf(file,"SET INPUT OPTIONS\n");
		fprintf(file,"WITH SET OPTION\nUNBUFFERED %d\n", settings->GetInt("input/unbuffered"));
		fprintf(file,"ADD_IMMED 0\n");
		dwNbrOfFiles=SendDlgItemMessage(IDC_SOURCEFILELIST,LB_GETCOUNT,0,0);
		ZeroMemory(lpfi,8);
		bWith=true;
		for (i=0;i<(int)dwNbrOfFiles;i++)
		{
			cb = (CBuffer*)SendDlgItemMessage(IDC_SOURCEFILELIST,LB_GETITEMDATA,i,0);
			lpfi[dwlpfic]=(FILE_INFO*)cb->GetData();

			bStore=!lpfi[dwlpfic^1];
			if (!bStore) bStore=(!!lpfi[dwlpfic]->cache)!=(!!lpfi[dwlpfic^1]->cache);
			if (bStore)	{
				if (!bWith) {
					fprintf(file,"WITH SET OPTION\n"); bWith = true;
				}
				fprintf(file,"USE CACHE %d\n",!!lpfi[dwlpfic]->cache);
			}

			bStore=!lpfi[dwlpfic^1];
			if (!bStore) bStore=(lpfi[dwlpfic]->bMP3VBF_forced)!=(lpfi[dwlpfic^1]->bMP3VBF_forced);
			if (bStore)	{
				if (!bWith) {
					fprintf(file,"WITH SET OPTION\n"); bWith = true;
				}
				fprintf(file,"AVI FORCE MP3VBR %d\n",lpfi[dwlpfic]->bMP3VBF_forced);
			}

		/*	bStore=!lpfi[dwlpfic^1];
			if (!bStore) bStore=(lpfi[dwlpfic]->bAddedImmediately)!=(lpfi[dwlpfic^1]->bAddedImmediately);
			if (bStore)	{
				if (!bWith) {
					fprintf(file,"WITH SET OPTION\n"); bWith = true;
				}
				fprintf(file,"ADD_IMMED %d\n",!!lpfi[dwlpfic]->bAddedImmediately);
			}

		*/	if (bWith) fprintf(file,"END WITH\n");
			bWith=false;
			fprintf(file,"LOAD ");
			fprintf(file,lpfi[dwlpfic]->lpcName);
			fprintf(file,"\n");
			dwlpfic^=1;
		}

		if (!bWith) {
			fprintf(file,"WITH SET OPTION\n"); bWith = true;
		}
		fprintf(file,"ADD_IMMED %d\n", bAddAS_immed);
		fprintf(file,"USE CACHE %d\n",(settings->GetInt("input/use cache")));
		fprintf(file,"AVI FORCE MP3VBR %d\n",!!(ofOptions.dwFlags&SOFO_AVI_FORCEMP3VBR));

		switch (ofOptions.dwFlags&SOFO_MP3_MASK) {
			case SOFO_MP3_CHECKCBRASK: fprintf(file,"MP3 VERIFY CBR ASK\n"); break;
			case SOFO_MP3_CHECKCBRNEVER: fprintf(file,"MP3 VERIFY CBR NEVER\n"); break;
			case SOFO_MP3_CHECKCBRALWAYS: fprintf(file,"MP3 VERIFY CBR ALWAYS\n"); break;
		}

		fprintf(file,"MP3 VERIFY RESDLG %d\n",!!(ofOptions.dwFlags&SOFO_MP3_RESULTDLG));
		fprintf(file,"M2F2 CRC %d\n",!!(ofOptions.dwFlags&SOFO_M2F2_DOM2F2CRCCHECK));
		fprintf(file,"AVI FIXDX50 %d\n",!!(ofOptions.dwFlags&SOFO_AVI_REPAIRDX50));
		switch ((int)!!(ofOptions.dwFlags&SOFO_AVI_IGNORELARGECHUNKS)) {
			case 0: fprintf(file,"AVI IGNLCHUNKS OFF\n"); break;
			case 1: fprintf(file,"AVI IGNLCHUNKS ON\n");
				    fprintf(file,"AVI IGNLCHUNKS %d\n",ofOptions.dwIgnoreSize); break;
		}
		fprintf(file,"AVI TRY2FIXLCHUNKS %d\n",!!(ofOptions.dwFlags&SOFO_AVI_TRYTOREPAIRLARGECHUNKS));

		fprintf(file,"WITH CHAPTERS\n");
		fprintf(file,"IMPORT %d\n",!!(ofOptions.dwFlags&SOFO_CH_IMPORT));
		fprintf(file,"FROMFILENAMES %d\n",!!(ofOptions.dwFlags&SOFO_CH_FROMFILENAMES));
		fprintf(file,"END WITH\n");

		if (bWith) fprintf(file,"END WITH\n");
		bWith=false;

	// video sources
		dwNbrOfVS=SendDlgItemMessage(IDC_AVAILABLEVIDEOSTREAMS,LB_GETCOUNT,0,0);
		for (i=0;i<(int)dwNbrOfVS;i++)
		{
			lpvs=(VIDEOSOURCE*)SendDlgItemMessage(IDC_AVAILABLEVIDEOSTREAMS,LB_GETITEMDATA,i,0);
			lpvsi=(VIDEO_SOURCE_INFORMATION*)lpvs->GetUserData();
			lpdwSelectedFiles=(DWORD*)lpvsi->lpdwFiles;
			if (lpdwSelectedFiles)
			{
				if (lpdwSelectedFiles[0])
				{
					fprintf(file,"DESELECT FILE 0\n");
					for (j=1;j<=(int)(lpdwSelectedFiles[0]);j++) {
						fprintf(file,"SELECT FILE %d\n",lpdwSelectedFiles[j]+1);
					}	
					fprintf(file,"ADD VIDEOSOURCE\n");
				}
			}
		}

		fprintf(file,"SET OUTPUT OPTIONS\n");
		fprintf(file,"WITH SET OPTION\n");
		fprintf(file,"UNBUFFERED %d\n",(int)settings->GetInt("output/general/unbuffered"));
		fprintf(file,"LOGFILE %d\n",(int)settings->GetInt("output/general/logfile/on"));
		fprintf(file,"STDOUTPUTFMT ");
		switch (sfOptions.iStdOutputFormat) {
			case SOF_AVI: fprintf(file,"AVI\n"); break;
			case SOF_MKV: fprintf(file,"MKV\n"); break;
		}
		fprintf(file,"END WITH\n");
			
	// audio streams
		//fprintf(file,"ADD_IMMED 0\n");
		audio_stream_items = m_AudioTree.GetItems(m_AudioTree.GetRootItem(),TIIID_ASI,1,&audio_stream_indices);
		subs_stream_items = m_AudioTree.GetItems(m_AudioTree.GetRootItem(),TIIID_SSI,1,&subs_stream_indices);
		
		dwNbrOfAS=m_AudioTree.GetCount();
		dwNbrOfSelAS=audio_stream_indices->GetCount();
	
		hItem = m_AudioTree.GetRootItem();
		i=0; j=0;
		while (hItem)
		{
			tii = m_AudioTree.GetItemInfo(hItem);
			if (tii && tii->iID == TIIID_ASI) {
				lpasi=tii->pASI;
				DWORD* f = lpasi->lpdwFiles;
				if (f) {
					fprintf(file,"DESELECT FILE 0\n");
					for (j=1;j<=(int)(f[0]);j++) {
						fprintf(file,"SELECT FILE %d\n",f[j]+1);
					}	
					fprintf(file,"ADD MMSOURCE\n");
				}
			}

			hItem = m_AudioTree.GetNextSiblingItem(hItem);
		}

		fprintf(file,"DESELECT AUDIO 0\n");
		for (i=0;i<(int)dwNbrOfSelAS;i++)
		{
			tii = m_AudioTree.GetItemInfo((HTREEITEM)audio_stream_items->At(i));
			lpasi = tii->pASI;

			fprintf(file,"SELECT AUDIO %d\n",audio_stream_indices->At(i)+1);
		}
		bWith=false;

		hItem = m_AudioTree.GetRootItem();
		i=0; j=0;
		
		fprintf(file,"WITH SET OPTION\n"); bWith = true;
		int iCount = m_AudioTree.GetRootCount();
/*
		if (iCount) {
			fprintf(file, "STREAMORDER %d", iCount);
			
			for (j=0; j<iCount; j++) {
				hItem = m_AudioTree.GetRootItem();
				for (i=0; i<iCount;i++) {
					tii = m_AudioTree.GetItemInfo(hItem);
					if (tii->iOrgPos == j) fprintf(file, " %d",tii->iCurrPos >> 1);
					hItem = m_AudioTree.GetNextSiblingItem(hItem);
				}
			}
			fprintf(file, "\n");
		}
*/
		hItem = m_AudioTree.GetRootItem();
		i=0; j=0;
		while (hItem)
		{
			tii = m_AudioTree.GetItemInfo(hItem);
			if (tii && tii->iID == TIIID_ASI) {
				lpasi=tii->pASI;
				if (lpasi->iDelay) {
					fprintf(file,"DELAY %d %d\n",i+1,lpasi->iDelay);
				}
				lpasi->audiosource->GetName(cBuffer);
				if (lstrlen(cBuffer)) {
					fprintf(file,"AUDIO NAME %d %s\n",i+1,cBuffer);
				}
				lpasi->audiosource->GetLanguageCode(cBuffer);
				if (lstrlen(cBuffer)) {
					fprintf(file,"AUDIO LNGCODE %d %s\n",i+1,cBuffer);
				}
				if (lpasi->audiosource->FormatSpecific(MMSGFS_AAC_ISSBR)) {
					fprintf(file,"AUDIO SBR %d %d\n",i+1,1);
				}
				fprintf(file,"AUDIO DEFAULT %d %d\n",i+1,lpasi->audiosource->IsDefault());

				i++;
			} else
			if (tii && tii->iID == TIIID_SSI) {

				tii->pSSI->lpsubs->GetName(cBuffer);
				fprintf(file,"SUBTITLE NAME %d %s\n",j+1,cBuffer);
				
				tii->pSSI->lpsubs->GetLanguageCode(cBuffer);
				if (cBuffer && strlen(cBuffer)) {
					fprintf(file,"SUBTITLE LNGCODE %d %s\n",j+1,cBuffer);
				}
				fprintf(file,"SUBTITLE DEFAULT %d %d\n",j+1,tii->pSSI->lpsubs->IsDefault());

				j++;
			}
			iCount++;
			
			hItem = m_AudioTree.GetNextSiblingItem(hItem);
		}

		if (bWith) fprintf(file,"END WITH\n");
		bWith=false;


		// subtitles

		fprintf(file,"DESELECT SUBTITLE 0\n");
		
		for (i=0;i<subs_stream_indices->GetCount();i++)
		{
			fprintf(file,"SELECT SUBTITLE %d\n",subs_stream_indices->At(i)+1);
		}

		fprintf(file,"WITH SET OPTION\n");

		fprintf(file,"NO AUDIO %d\n",!!(IsDlgButtonChecked(IDC_NO_AUDIO)==BST_CHECKED));
		fprintf(file,"ALL AUDIO %d\n",!!(IsDlgButtonChecked(IDC_ALL_AUDIO)==BST_CHECKED));
		fprintf(file,"NO SUBTITLES %d\n",!!(IsDlgButtonChecked(IDC_NO_SUBTITLES)==BST_CHECKED));
		fprintf(file,"ALL SUBTITLES %d\n",!!(IsDlgButtonChecked(IDC_ALL_SUBTITLES)==BST_CHECKED));
	// other output options
		fprintf(file,"OPENDML %d\n",!!settings->GetInt("output/avi/opendml/on"));
		fprintf(file,"LEGACY %d\n",!!settings->GetInt("output/avi/legacyindex"));
		fprintf(file,"RECLISTS %d\n",!!settings->GetInt("output/avi/reclists"));
		fprintf(file,"FRAMES %d\n",sfOptions.dwFrames);
		fprintf(file,"PRELOAD %d\n",sfOptions.dwPreload);
		fprintf(file,"MP3 CBR FRAMEMODE %d\n",(int)settings->GetInt("output/avi/mp3/cbr frame mode"));
		fprintf(file,"MAXFILESIZE %s\n",(sfOptions.dwUseMaxFileSize)?"ON":"OFF");
		fprintf(file,"AVI AC3FPC %d\n",settings->GetInt("output/avi/ac3/frames per chunk"));
		fprintf(file,"AVI MP3FPC %d\n",settings->GetInt("output/avi/mp3/frames per chunk"));
		fprintf(file,"AVI DTSFPC %d\n",settings->GetInt("output/avi/dts/frames per chunk"));
		fprintf(file,"OGG PAGESIZE %d\n",settings->GetInt("output/ogg/pagesize"));
		fprintf(file,"AVI RIFFAVISIZE %d\n", (int)settings->GetInt("output/avi/opendml/riff avi size"));
		fprintf(file,"AVI HAALIMODE %d\n", (int)settings->GetInt("output/avi/opendml/haalimode"));
		fprintf(file,"OVERLAPPED %d\n",(int)settings->GetInt("output/general/overlapped"));
		if (sfOptions.dwUseMaxFileSize)
		{
			fprintf(file,"MAXFILESIZE %d\n",sfOptions.dwMaxFileSize);
		}
		fprintf(file,"MAXFILES %s\n",(sfOptions.dwUseMaxFiles)?"ON":"OFF");
		if (sfOptions.dwUseMaxFiles)
		{
			fprintf(file,"MAXFILES %d\n",sfOptions.dwMaxFiles);
		}
		fprintf(file,"NUMBERING %s\n",(sfOptions.dwDontUseNumbering)?"OFF":"ON");
		if (!sfOptions.dwDontUseNumbering)
		{
			fprintf(file,"NUMBERING %s\n",sfOptions.lpcNumbering);
		}
		CAttribs* ai = settings->GetAttr("output/avi/audio interleave");
		fprintf(file,"AUDIO INTERLEAVE %d %s\n",(int)ai->GetInt("value"),//sfOptions.dwAudioInterleaveVal,
			(/*sfOptions.dwAudioInterleaveUnit*/(int)ai->GetInt("unit")==AIU_KB)?"KB":"FR");

		fprintf(file,"SPLIT POINTS %s\n",(sfOptions.dwUseManualSplitPoints)?"ON":"OFF");
		for (i=0;i<sfOptions.split_points->GetCount();i++) {
			fprintf(file,"SPLIT POINTS ADD %s\n",sfOptions.split_points->At(i)->cOrgText->Get());
		}

		fprintf(file,"STDIDX ");
		switch ((int)settings->GetInt("output/avi/opendml/stdindex/pattern")) {
			case SIP_RIFF:
				fprintf(file, "RIFF\n"); break;
			case SIP_AUTO:
				fprintf(file, "AUTO\n"); break;
			default:
				fprintf(file, "%d FRAMES\n",(int)settings->GetInt("output/avi/opendml/stdindex/interval")); break;
		}
		
		switch (m_Progress_List.GetAccuracy()) {
			case PRAC_BYTES: fprintf(file, "PRLAC BYTE\n"); break;
			case PRAC_SCALED: fprintf(file, "PRLAC SCALED\n"); break;
		}
	
		m_OutputResolution.GetWindowText(s);
		if (s.GetLength()) {
			fprintf(file,"RESOLUTION %s\n",s);
		}
		m_Title.GetWindowText(s);
		if (s.GetLength()) {
			Str2UTF8(s.GetBuffer(8192), cBuffer);
			fprintf(file,"TITLE %s\n",cBuffer);
		}
		
		fprintf(file,"END WITH\n");
	// mkv options
		fprintf(file,"WITH SET OPTION MKV\n");
		fprintf(file,"LACE %d\n",(int)settings->GetInt("output/mkv/lacing/style"));
//		char* audio_formats[] = { "general", "ac3", "dts", "mp3", "aac" };
		for (i=0;i<sizeof(cLaceDefinitionFormats)/sizeof(char*);i++) {
			fprintf(file,"LACESIZE %s %I64d %I64d\n",cLaceDefinitionFormats[i],
				GetLaceSetting((char*)cLaceDefinitionFormats[i], settings, GLS_USE),
				GetLaceSetting((char*)cLaceDefinitionFormats[i], settings, GLS_IGNUSEF)/1000000);
		}

		fprintf(file,"CLUSTERSIZE %d\n",(int)settings->GetInt("output/mkv/clusters/size"));
		fprintf(file,"CLUSTERTIME %d\n",(int)settings->GetInt("output/mkv/clusters/time"));
		fprintf(file,"CLUSTERINDEX %d\n",settings->GetInt("output/mkv/clusters/index/on"));

		fprintf(file,"PREVCLUSTERSIZE %d\n",(int)settings->GetInt("output/mkv/clusters/prevclustersize"));
		fprintf(file,"CLUSTERPOSITION %d\n",(int)settings->GetInt("output/mkv/clusters/position"));
		fprintf(file,"LIMIT1STCLUSTER %d\n",(int)settings->GetInt("output/mkv/clusters/limit first"));

		fprintf(file,"DISPWH %d\n",(int)settings->GetInt("output/mkv/displaywidth_height"));
		fprintf(file,"CUES %d\n",settings->GetInt("output/mkv/cues/on"));
		fprintf(file,"CUES VIDEO %d\n",settings->GetInt("output/mkv/cues/video/on"));
		fprintf(file,"CUES AUDIO %d\n",settings->GetInt("output/mkv/cues/audio/on"));
		fprintf(file,"CUES AUDIO ONLY_AUDIO_ONLY %d\n",settings->GetInt("output/mkv/cues/audio/only audio-only/on"));
		fprintf(file,"TIMECODESCALE MKA %d\n",settings->GetInt("output/mkv/TimecodeScale/mka"));
		fprintf(file,"TIMECODESCALE MKV %d\n",settings->GetInt("output/mkv/TimecodeScale/mkv"));
		fprintf(file,"FORCE_V1.0 %d\n",settings->GetInt("output/mkv/force v1.0"));
		fprintf(file,"FLOAT_WIDTH %d\n",(int)settings->GetInt("output/mkv/floats/width"));
		fprintf(file,"RANDOMIZE_ORDER %d\n",(int)settings->GetInt("output/mkv/randomize element order"));

		fprintf(file,"2ND_COPY_OF_TRACKS %d\n",settings->GetInt("output/mkv/2nd Tracks"));
		fprintf(file,"END WITH\n");
	// chapters
		if (sfOptions.chapters->GetChapterCount()) {
			fprintf(file,"WITH SET OPTION CHAPTERS\nCLEAR\n");
			RenderChapters2File(file,sfOptions.chapters);
			fprintf(file,"END WITH\n");
		} else {
			fprintf(file,"SET OPTION CHAPTERS CLEAR\n");
		}

		fclose(file);
		audio_stream_indices->DeleteAll();
		delete audio_stream_indices;
		audio_stream_items->DeleteAll();
		delete audio_stream_items;
		subs_stream_items->DeleteAll();
		delete subs_stream_items;
		subs_stream_indices->DeleteAll();
		delete subs_stream_indices;
}

void CAVIMux_GUIDlg::OnSave() 
{
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen
	CFileDialog*	cfd;

	cfd=new CFileDialog(false,"amg");
	if (cfd->DoModal()==IDOK)
	{
		doSaveConfig(cfd->GetPathName().GetBuffer(255));
	}
	delete cfd;
}

void CAVIMux_GUIDlg::OnLoad() 
{
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen
	CFileDialog*	cfd;
	CONFIGDATA		conf;
	FILESTREAM*		file;
	CString			cStr;
	DWORD			dwX;

	cfd=new CFileDialog(true,"amg");
	if (cfd->DoModal()==IDOK)
	{
		ZeroMemory(&conf,sizeof(conf));
		file=new FILESTREAM;
		file->Open(cfd->GetPathName().GetBuffer(255),STREAM_READ);
		file->Read(&conf.dwID,4);
		file->Read(&conf.dwSize,4);
		file->Read(&conf.dwVersion,conf.dwSize);
		file->Read(&dwX,4);

		sfOptions.dwUseMaxFileSize=conf.dwUseMaxSize;
		sfOptions.dwUseManualSplitPoints=conf.dwUseSplitList;
		sfOptions.dwMaxFileSize=conf.dwMaxSize;
		sfOptions.dwPreload=conf.dwPreload;
		sfOptions.dwFrames=conf.dwMaxFrames;
		sfOptions.lpcNumbering=new char[1+lstrlen(conf.cFormat)];
		lstrcpy(sfOptions.lpcNumbering,conf.cFormat);

		file->Close();
		delete file;
	}
	delete cfd;
}

bool	bSetOutputOptions;

BOOL CAVIMux_GUIDlg::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	// TODO: Speziellen Code hier einfügen und/oder Basisklasse aufrufen
	int	i;
	char*	cParam=(char*)lParam;
	WPARAM	wOldWParam = wParam;

	switch (LOWORD(wParam))
	{
		case IDM_LANG1:
		case IDM_LANG2:
		case IDM_LANG3:
		case IDM_LANG4:
		case IDM_LANG5:
		case IDM_LANG6:
		case IDM_LANG7:
		case IDM_LANG8:
		case IDM_LANG9:
		case IDM_LANG10:
		case IDM_LANG11:
		case IDM_LANG12:
		case IDM_LANG13:
		case IDM_LANG14:
		case IDM_LANG15:
		case IDM_LANG16:
			for (i=0;i<16;i++) if (dwLangs[i]==LOWORD(wParam)) SetCurrentLanguage(lplpLanguages[i]);
			UpdateLanguage();
			break;
		case IDM_PROTOCOL:
			SetDialogState_Muxing();
			break;
		case IDM_SAVECONFIG:
			OnSave();
			break;
		case IDM_CONFIGSOURCES:
			SetDialogState_Config();
			break;
		case IDM_IMMED_ADD_AUDIO:
			bAddAS_immed = !bAddAS_immed;
			break;
	}		
	wParam = wOldWParam;
	return CDialog::OnCommand(wParam, lParam);
}

void CAboutDlg::OnOK() 
{
		
	CDialog::OnOK();
}

void CAVIMux_GUIDlg::OnOutputoptions() 
{
	if (!bEditInProgess) {
		CSetStoreFileOptionsDlg		*csfod;

		csfod=new CSetStoreFileOptionsDlg;
		sfOptions.cdMain=this;
		csfod->SetData(&sfOptions,&ofOptions, settings);
	
		if (csfod->DoModal() == IDOK)
		{
			csfod->GetData(&sfOptions,&ofOptions, &settings);
		}
		delete csfod;
	}

}

void CAVIMux_GUIDlg::OnNoAudio() 
{
	
	if (IsDlgButtonChecked(IDC_NO_AUDIO)) 
	{
		CheckDlgButton(IDC_ALL_AUDIO,BST_UNCHECKED);
		CheckDlgButton(IDC_DEFAULT_AUDIO,BST_UNCHECKED);
	}
	
}

void CAVIMux_GUIDlg::OnAllAudio() 
{
	
	if (IsDlgButtonChecked(IDC_ALL_AUDIO)) 
	{
		CheckDlgButton(IDC_NO_AUDIO,BST_UNCHECKED);
		CheckDlgButton(IDC_DEFAULT_AUDIO,BST_UNCHECKED);
	}
}

void CAVIMux_GUIDlg::OnNoSubtitles() 
{
	
	if (IsDlgButtonChecked(IDC_NO_SUBTITLES)) CheckDlgButton(IDC_ALL_SUBTITLES,BST_UNCHECKED);
	
}

void CAVIMux_GUIDlg::OnAllSubtitles() 
{
	
	if (IsDlgButtonChecked(IDC_ALL_SUBTITLES)) CheckDlgButton(IDC_NO_SUBTITLES,BST_UNCHECKED);
	
}

void CAVIMux_GUIDlg::OnDefaultAudio() 
{

	if (IsDlgButtonChecked(IDC_DEFAULT_AUDIO))
	{
		CheckDlgButton(IDC_NO_AUDIO,BST_UNCHECKED);
		CheckDlgButton(IDC_ALL_AUDIO,BST_UNCHECKED);
	}
}

void CAVIMux_GUIDlg::OnRButtonUp(UINT nFlags, CPoint point) 
{

	CMenu*		cmPopupMenu,*cmLanguages;
	int			i;
	CString		cStr;
	bool		bShowMenu=false;


	cmPopupMenu=new CMenu;
	cmPopupMenu->CreatePopupMenu();

	cmLanguages=new CMenu;
	cmLanguages->CreatePopupMenu();
		
	for (i=0;i<(int)dwLanguages;i++)
	{
		cmLanguages->AppendMenu(MF_STRING,dwLangs[i],lplpLanguages[i]->lpcName);
	}

	cStr=LoadString(STR_MAIN_B_SAVECONFIG);
	cmPopupMenu->AppendMenu(MF_STRING,IDM_SAVECONFIG,cStr);
	cmPopupMenu->AppendMenu(MF_SEPARATOR,0);

	cStr=LoadString(IDS_LANGUAGE);
	cmPopupMenu->AppendMenu(MF_POPUP,(UINT)(cmLanguages->m_hMenu),LoadString(IDS_LANGUAGE));
	bShowMenu=true;

	switch (iCurrentView)
	{
		case 1:	cStr=LoadString(STR_MAIN_M_DISPLAYPROTOCOL); 
				cmPopupMenu->AppendMenu(MF_STRING,IDM_PROTOCOL,cStr);
				cmPopupMenu->AppendMenu(MF_STRING |
					(bAddAS_immed?MF_CHECKED:MF_UNCHECKED),IDM_IMMED_ADD_AUDIO,
					LoadString(STR_MAIN_IMMEDADDMP3));
				break;

		case 2:	cStr=LoadString(STR_MAIN_M_HIDEPROTOCOL); 
				cmPopupMenu->AppendMenu(MF_STRING,IDM_CONFIGSOURCES,cStr);
				break;
	}

	if (bShowMenu)
	{
		ClientToScreen(&point);
		cmPopupMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON,point.x,point.y,this);
	}
	if (cmPopupMenu) delete cmPopupMenu;
	if (cmLanguages) delete cmLanguages;
			
	CDialog::OnRButtonUp(nFlags, point);
}

BOOL CAboutDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Zusätzliche Initialisierung hier einfügen
	
	SetWindowText(LoadString(STR_AB_TITLE));
	CString c;
	CString d;
	c.LoadString(IDS_VERSION_INFO);
	sprintf(d.GetBuffer(255), "%s, %s", c, __DATE__);
	m_Version.SetWindowText(d);
	

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX-Eigenschaftenseiten sollten FALSE zurückgeben
}

void CAVIMux_GUIDlg::OnChangeAudioname() 
{
//	UpdateAudioName();	
	ApplyStreamSettings();
}

void CAVIMux_GUIDlg::OnChangeDelay() 
{
	UpdateAudiodelay();
}




LRESULT CAVIMux_GUIDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	char* c;
	int i,j;
	TREE_ITEM_INFO*		tii;
	AUDIO_STREAM_INFO*	lpasi;
	SUBTITLE_STREAM_INFO* lpssi;
	char*	cParam=(char*)lParam;
	char	cBuffer[256];
	char*	w;
	char*					entire_line = NULL;
	bool					bError=false;
	bool					B[4];

	// TODO: Speziellen Code hier einfügen und/oder Basisklasse aufrufen
	ZeroMemory(B,sizeof(B));

	if (message==WM_NOTIFY) {
		int	idCtrl = (int) wParam; 
		NMHDR* pnmh = (LPNMHDR) lParam; 

		if (idCtrl == IDC_AUDIOTREE) {

			NMTVDISPINFO* lptvdi = (LPNMTVDISPINFO)lParam;
			int is_unicode = SendDlgItemMessage(IDC_AUDIOTREE, TVM_GETUNICODEFORMAT);
			unsigned int imsg = (is_unicode)?TVN_GETDISPINFOW:TVN_GETDISPINFOA;
			if (lptvdi->hdr.code == imsg) {
				OnGetAudiodispinfo((NMHDR*)lptvdi, (long*)&i);
			}

		}
	} else

	if (message==uiMessage) {
		switch (LOWORD(wParam)) {
			case IDM_DOADDFILE:
				c = (char*)lParam;
				doAddFile((char*)lParam);
				delete c;
				break;
			case IDM_SETLANGUAGE:
				c = (char*)lParam;
				for (i=0;i<(int)dwLanguages;i++) {
					if (!strcmpi(c,lplpLanguages[i]->lpcName)) {
						SetCurrentLanguage(lplpLanguages[i]);
					}
				}
				UpdateLanguage();
				delete c;
				break;
			case IDM_CLEARALL_NC:
				Clear();
				break;
			case IDM_SELAUDIO:
				if (lParam!=-1) {
					Tree_SetCheckState(&m_AudioTree,Tree_Index2Item(&m_AudioTree,lParam,TIIID_ASI),true);
				} else {
					for (i=0;i<(int)m_AudioTree.GetCount();i++) {
						Tree_SetCheckState(&m_AudioTree,Tree_Index2Item(&m_AudioTree,i,TIIID_ASI),true);
					}
				}
				break;
			case IDM_SELFILE:
				SendDlgItemMessage(IDC_SOURCEFILELIST,LB_SETSEL,1,lParam);
				break;
			case IDM_DESELAUDIO:
				if (lParam!=-1) {
					Tree_SetCheckState(&m_AudioTree,Tree_Index2Item(&m_AudioTree,lParam,TIIID_ASI),false);
				} else {
					for (i=0;i<(int)m_AudioTree.GetCount();i++) {
						Tree_SetCheckState(&m_AudioTree,Tree_Index2Item(&m_AudioTree,i,TIIID_ASI),false);
					}
				}
				break;
			case IDM_SELSUB:
				if (lParam!=-1) {
					Tree_SetCheckState(&m_AudioTree,Tree_Index2Item(&m_AudioTree,lParam,TIIID_SSI),true);
				} else {
					for (i=0;i<(int)m_AudioTree.GetCount();i++) {
						Tree_SetCheckState(&m_AudioTree,Tree_Index2Item(&m_AudioTree,i,TIIID_SSI),true);
					}
				}
				break;
			case IDM_DESELSUB:
				if (lParam!=-1) {
					Tree_SetCheckState(&m_AudioTree,Tree_Index2Item(&m_AudioTree,lParam,TIIID_SSI),false);
				} else {
					for (i=0;i<(int)m_AudioTree.GetCount();i++) {
						Tree_SetCheckState(&m_AudioTree,Tree_Index2Item(&m_AudioTree,i,TIIID_SSI),false);
					}
				}
				break;
			case IDM_DESELFILE:
				SendDlgItemMessage(IDC_SOURCEFILELIST,LB_SETSEL,0,lParam);
				break;
			case IDM_ADDAVILIST:
				OnAddFileList();
				break;
			case IDM_SETOUTPUTOPTIONS:
				bSetOutputOptions=true;
				break;
			case IDM_SETINPUTOPTIONS:
				bSetOutputOptions=false;
				break;
			case IDM_STARTMUXING:
				if (cExtFilename) delete cExtFilename;
				cExtFilename = new char[1+strlen((LPSTR)lParam)];
				strcpy(cExtFilename,(LPSTR)lParam);
				delete (char*)lParam;
				OnStart();
				break;
			case IDM_ADDFILE:
				OnAddFile();
				break;
			case IDM_CLEARALL:
				OnClear();
				break;
			case IDM_DISPLAYSUBNAME:
				break;
			case IDM_DISPLAYAUDIONAME:
				m_AudioName.SetWindowText((LPSTR)lParam);
				break;
			case IDM_DISPLAYAUDIODELAY:
				m_Audiodelay.SetWindowText((LPSTR)lParam);
				break;
			case IDM_GETNEWSUBNAME:
				break;
			case IDM_PROTOCOL:
				SetDialogState_Muxing();
				break;
			case IDM_CONFIGSOURCES:
				SetDialogState_Config();
				break;
		case IDM_SETOPTION:
			//entire_line = (char*)calloc(1,1+strlen(cParam));
			newz(char, 1+strlen(cParam), entire_line);
			if (bSetOutputOptions)
			{
				bError=false;
				strcpy(entire_line,cParam);
				w=getword(&cParam);
				if (!strcmp(w,"DONEDLG")) {
					sfOptions.bDispDoneDlg=!!atoi(cParam);
				}
				else
				if (!strcmp(w,"CLOSEAPP")) {
					sfOptions.bExitAfterwards=!!atoi(cParam);
				}
				else
				if (!strcmp(w,"OVERWRITEDLG")) {
					sfOptions.bDispOverwriteDlg=!!atoi(cParam);
				}
				else
				if (!strcmp(w,"OPENDML")) {
					settings->SetInt("output/avi/opendml/on", !!atoi(cParam));
				}
				else
				if (!strcmp(w,"LEGACY")) {
					settings->SetInt("output/avi/legacyindex", !!atoi(cParam));
				}
				else
				if (!strcmp(w,"RECLISTS")) {
					settings->SetInt("output/avi/reclists", !!atoi(cParam));
				}
				else
				if (!strcmp(w,"PRELOAD")) {
					if (isint(cParam)) {
						sfOptions.dwPreload=atoi(cParam);
					} else bError=true;
				} 
				else
				if (!strcmp(w,"LOGFILE")) {
					if (isint(cParam)) {
						settings->SetInt("output/general/logfile/on",atoi(cParam));
					} else bError=true;
				}
				else
				if (!strcmp(w,"MAXFILESIZE")) {
					w=getword(&cParam);
					if (!strcmp(w,"OFF")) {
						sfOptions.dwUseMaxFileSize=0;
					}
					else
					if (!strcmp(w,"ON")) {
						sfOptions.dwUseMaxFileSize=1;
					}
					else
					if (isint(w)) {
						sfOptions.dwMaxFileSize=atoi(w);
					} else bError=true;
				}
				else
				if (!strcmp(w,"MAXFILES"))	{
					w=getword(&cParam);
					if (!strcmp(w,"OFF")) {
						sfOptions.dwUseMaxFiles=0;
					}
					else
					if (!strcmp(w,"ON")) {
						sfOptions.dwUseMaxFiles=1;
					}
					else
					if (isposint(w)) {
						sfOptions.dwMaxFiles=atoi(w);
					} else bError=true;
				}
				else
				if (!strcmp(w,"DELAY"))
				{
					w=getword(&cParam);
					if (isposint(w)) {
						j=atoi(w)-1;

						tii = m_AudioTree.GetItemInfo(Tree_Index2Item(&m_AudioTree,j));
						lpasi = tii->pASI;

						w=getword(&cParam);
						if (isint(w)) {
							SetDelay(lpasi,atoi(w));
							m_AudioTree.UpdateWindow();
						} else bError=true;
					} else bError=true;
				}
				else
				if (!strcmp(w,"AUDIO")) {
					w=getword(&cParam);
					if (!strcmp(w,"INTERLEAVE")) { // AUDIO INTERLEAVE
						w=getword(&cParam);
						if (isposint(w)) {
							settings->SetInt("output/avi/audio interleave/value", atoi(w));
							w=getword(&cParam);
							if (!strcmp(w,"KB")) {
								settings->SetInt("output/avi/audio interleave/unit", AIU_KB);
							}
							else
							if (!strcmp(w,"FR")) {
								settings->SetInt("output/avi/audio interleave/unit", AIU_FRAME);
							} else bError=true;
						} else bError=true;
					}
					else
					if ((B[0]=!strcmp(w,"NAME")) || (B[1]=!strcmp(w,"LNGCODE")) || (B[2]=!strcmp(w,"SBR")) ||
						(B[3]=!strcmp(w,"DEFAULT")))	{ // AUDIO NAME/LNGCODE
						w=getword(&cParam);
						j=atoi(w)-1;
						tii = m_AudioTree.GetItemInfo(Tree_Index2Item(&m_AudioTree,j,TIIID_ASI,&j));
						if (tii) lpasi = tii->pASI; else lpasi=NULL;

						if (B[0] || B[1]) {
							m_AudioTree.FindID(Tree_Index2Item(&m_AudioTree,j),B[0]?TIIID_STRNAME:TIIID_LNGCODE,&tii);
							if (tii->pText) delete tii->pText;
							tii->pText = new char[1024];
							strcpy(tii->pText,cParam);

							if (B[0]) { 
								lpasi->audiosource->SetName(cParam);
							} else {
								lpasi->audiosource->SetLanguageCode(cParam);
							}
						} else
						if (B[2]) {
							w=cParam;
							if (isposint(w)) {
								lpasi->audiosource->FormatSpecific(MMSSFS_AAC_SETSBR, atoi(w));
								FillAAC_ASI(&lpasi, (AACSOURCE*)lpasi->audiosource);
							} else 
								bError=true;
						} else 
						if (B[3]) {
							w=cParam;
							if (isposint(w)) {
								lpasi->audiosource->SetDefault(atoi(w));
							}
						}
						m_AudioTree.Invalidate();
						m_AudioTree.UpdateWindow();
					} 
					else bError=true;
				}
				else
				if (!strcmp(w,"NUMBERING"))
				{
					w=cParam;
					if (!strcmp(w,"OFF")) {
						sfOptions.dwDontUseNumbering=1;
					}
					else
					if (!strcmp(w,"ON")) {
						sfOptions.dwDontUseNumbering=0;
					}
					else
					{
						if (sfOptions.lpcNumbering)	{
							free(sfOptions.lpcNumbering);
						}
						sfOptions.lpcNumbering=new char[1+lstrlen(w)];
						lstrcpy(sfOptions.lpcNumbering,w);
					}
				}
				else
				if (!strcmp(w,"DEFAULT")) { 
					w=getword(&cParam);
					if (!strcmp(w,"AUDIO")) {
						w=getword(&cParam);
						if (!strcmp(w,"OFF")) {
							CheckDlgButton(IDC_DEFAULT_AUDIO,BST_UNCHECKED);
						}
						else
						if (!strcmp(w,"ON")) {
							CheckDlgButton(IDC_DEFAULT_AUDIO,BST_CHECKED);
						}
						else
						if (isposint(w)) {
							SendDlgItemMessage(IDC_DEFAULT_AUDIO_NUMBER, WM_SETTEXT,0,atoi(w));
						} else bError=true;
					} else bError=true;
				}
				else
				if (!strcmp(w,"FRAMES")) {
					w=getword(&cParam);
					if (isposint(w)) {
						sfOptions.dwFrames=atoi(w);
					} else bError=true;
				}
				else
				if (!strcmp(w,"MP3")) { 
					w=getword(&cParam);
					if (!strcmp(w,"CBR")) {
						w=getword(&cParam);
						if (!strcmp(w,"FRAMEMODE")) {
							settings->SetInt("output/avi/mp3/cbr frame mode", !!atoi(cParam));
						} else bError=true;
					} else bError=true;
				}
				else
				if (!strcmp(w,"ALL")) {
					w=getword(&cParam);
					if (!strcmp(w,"SUBTITLES")) {
						w=getword(&cParam);
						CheckDlgButton(IDC_ALL_SUBTITLES,!!atoi(w)?BST_CHECKED:BST_UNCHECKED);
						OnAllSubtitles();
					}
					else
					if (!strcmp(w,"AUDIO")) {
						w=getword(&cParam);
						CheckDlgButton(IDC_ALL_AUDIO,!!atoi(w)?BST_CHECKED:BST_UNCHECKED);
						OnAllAudio();
					}
					else bError=true;
				}
				else
				if (!strcmp(w,"NO")) {
					w=getword(&cParam);
					if (!strcmp(w,"SUBTITLES"))	{
						w=getword(&cParam);
						CheckDlgButton(IDC_NO_SUBTITLES,!!atoi(w)?BST_CHECKED:BST_UNCHECKED);
						OnNoAudio();
					}
					else
					if (!strcmp(w,"AUDIO")) {
						w=getword(&cParam);
						CheckDlgButton(IDC_NO_AUDIO,!!atoi(&((char*)lParam)[9])?BST_CHECKED:BST_UNCHECKED);
						OnNoAudio();
					}
					else bError=true;
				}
				else
				if (!strcmp(w,"SPLIT")) {
					w=getword(&cParam);
					if (!strcmp(w,"POINTS")) {
						w=getword(&cParam);
						if (!strcmp(w,"OFF")) {
							sfOptions.dwUseManualSplitPoints=0;
						}
						else
						if (!strcmp(w,"ON")) {
							sfOptions.dwUseManualSplitPoints=1;
						}
						else
						if (!strcmp(w,"ADD"))
						{
							SPLIT_POINT_DESCRIPTOR* d = new SPLIT_POINT_DESCRIPTOR;
							ZeroMemory(d,sizeof(*d));
							String2SplitPointDescriptor(cParam,d);
							sfOptions.split_points->Insert(d);
						}
						else bError=true;
					} else bError=true;
				}
				else
				if (!strcmp(w,"STDIDX")) {
					w=getword(&cParam);
					if (!strcmp(w,"RIFF")) {
						settings->SetInt("output/avi/opendml/stdindex/pattern", SIP_RIFF);
					}
					else
					if (!strcmp(w,"AUTO")) {
						settings->SetInt("output/avi/opendml/stdindex/pattern", SIP_AUTO);
					}
					else
					if (isposint(w)) {
						j=atoi(w);
						w=getword(&cParam);
						if (!strcmp(w,"FRAMES"))	{
							settings->SetInt("output/avi/opendml/stdindex/pattern", SIP_FRAMES);
							settings->SetInt("output/avi/opendml/stdindex/interval", j);
						} else bError=true;
					} else bError=true;
				}
				else
				if (!strcmp(w,"SUBTITLE")) {
					w=getword(&cParam);
					if ((B[0]=!strcmp(w,"NAME")) || (B[1]=!strcmp(w,"LNGCODE")) || (B[2]=!strcmp(w,"DEFAULT")))	{ // SUB NAME
						w=getword(&cParam);
						j=atoi(w)-1;
						tii = m_AudioTree.GetItemInfo(Tree_Index2Item(&m_AudioTree,j,TIIID_SSI,&j));
						lpssi = tii->pSSI;

						
						if (B[0] || B[1]) {
							m_AudioTree.FindID(Tree_Index2Item(&m_AudioTree,j),B[0]?TIIID_STRNAME:TIIID_LNGCODE,&tii);
							if (tii->pText) delete tii->pText;
							tii->pText = new char[1024];
							strcpy(tii->pText,cParam);
						}

						if (B[0]) {
							lpssi->lpsubs->SetName(cParam);
						} else 
						if (B[1]) {
							lpssi->lpsubs->SetLanguageCode(cParam);
						} else 
						if (B[2]) {
							w=cParam;
							if (isposint(w)) {
								lpssi->lpsubs->SetDefault(atoi(w));
							}
						}
						m_AudioTree.Invalidate();
						m_AudioTree.UpdateWindow();
					} else
						bError=true;
			
				} 
				else
				if (!strcmp(w,"PRLAC")) {
					w=getword(&cParam);
					if (!strcmp(w,"BYTE")) {
						m_Progress_List.SetAccuracy(PRAC_BYTES);
					} 
					else
					if (!strcmp(w,"SCALED")) {
						m_Progress_List.SetAccuracy(PRAC_SCALED);
					} else bError=true;
				}
				else
				if (!strcmp(w,"STREAMORDER")) {
					w=getword(&cParam);
					if (isposint(w)) {
						int item_count = atoi(w);
						int i = 0;
						int* index = new int[item_count];
						while (i < item_count) {
							w=getword(&cParam);
							index[i++] = atoi(w);
						}
						i = 0;
						HTREEITEM hItem = m_AudioTree.GetRootItem();
						while (i < item_count) {
							TREE_ITEM_INFO* tii = m_AudioTree.GetItemInfo(hItem);
							tii->iCurrPos = index[i++];
							hItem = m_AudioTree.GetNextSiblingItem(hItem);
						}
						delete index;
						m_AudioTree.Sort();

					} else
						bError = true;
						
				} else
				if (!strcmp(w,"MKV")) {
					w=getword(&cParam);
					if (!strcmp(w,"LACE")) {
						w=getword(&cParam);
						if (isposint(w)) {
							//sfOptions.mkv.iLaceStyle = atoi(w);
							settings->SetInt("output/mkv/lacing/style", max(0,min(3,atoi(w))));
						} else bError = true;
					} else
					if (!strcmp(w,"LACESIZE")) {
						w=getword(&cParam);
						if (isposint(w)) {
//							sfOptions.mkv.iLaceLength = atoi(w);
							settings->SetInt("output/mkv/lacing/length", atoi(w));
							SetLaceSetting("general",settings,1,atoi(w));
						} else {
							char* cFormat = w;
							char* cUse = getword(&cParam);
							char* cLength = getword(&cParam);
							SetLaceSetting(cFormat,settings,atoi(cUse),atoi(cLength));
						}
					} else
					if (!strcmp(w,"CLUSTERSIZE")) {
						w=getword(&cParam);
						if (isposint(w)) {
//							sfOptions.mkv.iClusterSize = atoi(w);
							settings->SetInt("output/mkv/clusters/size", atoi(w));
						} else bError = true;
					} else
					if (!strcmp(w,"CLUSTERTIME")) {
						w=getword(&cParam);
						if (isposint(w)) {
							//sfOptions.mkv.iClusterTime = atoi(w);
							settings->SetInt("output/mkv/clusters/time", atoi(w));
						} else bError = true;
					} else 
					if (!strcmp(w,"PREVCLUSTERSIZE")) {
						w=getword(&cParam);
						if (isposint(w)) {
							settings->SetInt("output/mkv/clusters/prevclustersize", !!atoi(w));
						} else bError = true;
					} else 
					if (!strcmp(w,"CLUSTERPOSITION")) {
						w=getword(&cParam);
						if (isposint(w)) {
							settings->SetInt("output/mkv/clusters/position", !!atoi(w));
						} else bError = true;
					} else
					if (!strcmp(w,"LIMIT1STCLUSTER")) {
						w=getword(&cParam);
						if (isposint(w)) {
							//sfOptions.mkv.iLimit1stCluster = !!atoi(w);
							settings->SetInt("output/mkv/clusters/limit first", !!atoi(w));
						} else bError = true;
					} else 
					if (!strcmp(w,"DISPWH")) {
						w=getword(&cParam);
						if (isposint(w)) {
							settings->SetInt("output/mkv/displaywidth_height", !!atoi(w));
						} else bError = true;
					} else 
					if (!strcmp(w,"CUES")) {
						w=getword(&cParam);
						if (isposint(w)) {
							settings->SetInt("output/mkv/cues/on",!!atoi(w));
						} else {
							if (!strcmp(w,"VIDEO")) {
								w=getword(&cParam);
								if (isposint(w)) {
									settings->SetInt("output/mkv/cues/video/on",!!atoi(w));
								} else bError = true;
							} else 
							if (!strcmp(w,"AUDIO")) {
								w=getword(&cParam);
								if (isposint(w)) {
									settings->SetInt("output/mkv/cues/audio/on",!!atoi(w));
								} else {
									if (!strcmp(w,"ONLY_AUDIO_ONLY")) {
										w=getword(&cParam);
										if (isposint(w)) {
											settings->SetInt("output/mkv/cues/audio/only audio-only/on",!!atoi(w));
										} else bError = true;
									} else bError = true;
								} 
							} else bError = true;
						} 
					} else 
					if (!strcmp(w,"TIMECODESCALE")) {
						w=getword(&cParam);
						if (!strcmp(w,"MKA")) {
							w=getword(&cParam);
							if (isposint(w)) {
								settings->SetInt("output/mkv/TimecodeScale/mka", atoi(w));
							} else bError=true;
						} else
						if (!strcmp(w,"MKV")) {
							w=getword(&cParam);
							if (isposint(w)) {
								settings->SetInt("output/mkv/TimecodeScale/mkv", atoi(w));
							} else bError=true;
						} else bError=true;

					} else
					if (!strcmp(w,"FORCE_V1.0")) {
						w=getword(&cParam);
						if (isposint(w)) {
							settings->SetInt("output/mkv/force v1.0", !!atoi(w));
						} else bError = true;
					} else
					if (!strcmp(w,"FLOAT_WIDTH")) {
						w=getword(&cParam);
						if (isposint(w)) {
							settings->SetInt("output/mkv/floats/width", atoi(w));
						} else bError = true;
					} else
					if (!strcmp(w,"2ND_COPY_OF_TRACKS")) {
						w=getword(&cParam);
						if (isposint(w)) {
							settings->SetInt("output/mkv/2nd Tracks", !!atoi(w));
						} else bError = true;
					} else
					if (!strcmp(w,"CLUSTERINDEX")) {
						w=getword(&cParam);
						if (isposint(w)) {
							settings->SetInt("output/mkv/clusters/index/on", !!atoi(w));
						} else bError = true;
					} else
					if (!strcmp(w,"RANDOMIZE_ORDER")) {
						w=getword(&cParam);
						if (isposint(w)) {
							settings->SetInt("output/mkv/randomize element order", !!atoi(w));
						} else bError = true;
					} else
/*					if (!strcmp(w,"AC3FPB")) {
						w=getword(&cParam);
						if (isposint(w)) {
							sfOptions.mkv.iAC3FrameCount = atoi(w);
						} else bError = true;
					} else */bError = true;
				} else 
				if (!strcmp(w,"AVI")) {
					w=getword(&cParam);
					if (!strcmp(w,"AC3FPC")) {
						w=getword(&cParam);
						if (isposint(w)) {
							settings->SetInt("output/avi/ac3/frames per chunk", atoi(w));
						} else bError = true;
					} else 
					if (!strcmp(w,"DTSFPC")) {
						w=getword(&cParam);
						if (isposint(w)) {
							settings->SetInt("output/avi/dts/frames per chunk", atoi(w));
						} else bError = true;
					} else 
					if (!strcmp(w,"MP3FPC")) {
						w=getword(&cParam);
						if (isposint(w)) {
							settings->SetInt("output/avi/mp3/frames per chunk", atoi(w));
						} else bError = true;
					} else 
					if (!strcmp(w,"RIFFAVISIZE")) {
						w=getword(&cParam);
						if (isposint(w)) {
							settings->SetInt("output/avi/opendml/riff avi size", atoi(w));
						} else bError = true;
					} else 
					if (!strcmp(w,"HAALIMODE")) {
						w=getword(&cParam);
						if (isposint(w)) {
							settings->SetInt("output/avi/opendml/haalimode", atoi(w));
						} else bError = true;
					} else 
						
						bError = true;
				} else
				if (!strcmp(w,"OGG")) {
					w=getword(&cParam);
					if (!strcmp(w, "PAGESIZE")) {
						w=getword(&cParam);
						if (isposint(w)) {
							settings->SetInt("output/ogg/pagesize", atoi(w));
						} else bError=true;
					} else bError=true;
				} else
				if (!strcmp(w,"CHAPTERS")) {
					w=getword(&cParam);
					if (!strcmp(w,"SUB")) {
						w=getword(&cParam);
						if (!strcmp(w,"END")) {
							if (chapter_level) {
								chapters = chap[--chapter_level];
							} else bError=true;
						} else 
						if (!strcmp(w,"BEGIN")) {
							chap[++chapter_level] = chapters->GetSubChapters(CHAP_LAST);
							chapters = chap[chapter_level];
						} else bError=true;
					} else
					if (!strcmp(w,"ADD")) {
						char* v=getword(&cParam);
						w=getword(&cParam);
						char* lng=getword(&cParam);
						char* benabled = getword(&cParam);
						char* bhidden = getword(&cParam);
						char* tx = cParam;

						SINGLE_CHAPTER_DATA scd;
						scd.iBegin = Str2Millisec(v)*1000000;
						scd.iEnd = (!strcmp(w,"END"))?-1:Str2Millisec(w)*1000000;
						strcpy(scd.cText, tx);
						if (lng) strcpy(scd.cLng, lng); else strcpy(scd.cLng,"und");
						scd.bEnabled=(benabled)?!!atoi(benabled):1;
						scd.bHidden=(bhidden)?!!atoi(bhidden):0;
						
/*						if (!strcmp(w,"END")) {
							chapters->AddChapter(Str2Millisec(v)*1000000,-1,cParam);	
						} else {
							chapters->AddChapter(Str2Millisec(v)*1000000,Str2Millisec(w)*1000000,cParam);
						}*/
						chapters->AddChapter(&scd);
					} else
					if (!strcmp(w,"APPEND")) {
					//	w=getword(&cParam);
						if (chapters->GetChapterCount()) {
							if (chapters->GetChapterEnd(CHAP_LAST) == -1) {
								chapters->SetChapterEnd(CHAP_LAST,Str2Millisec(w)*1000000);
							}
						}
						chapters->AddChapter(Str2Millisec(w)*1000000,-1,cParam);
					} else 
					if (!strcmp(w,"APPENDREL")) {
						w=getword(&cParam);
						__int64 iTime;
						if (chapters->GetChapterCount()) {
							iTime = chapters->GetChapterBegin(CHAP_LAST);
						} else if (chapter_level && chap[chapter_level-1]->GetChapterCount()) {
							iTime = chap[chapter_level-1]->GetChapterBegin(CHAP_LAST);

						} else iTime = 0;
						chapters->AddChapter(iTime+Str2Millisec(w)*1000000,-1,cParam);
			
					} else
					if (!strcmp(w,"CLEAR")) {
						chapters->Delete();
					} else

						bError=true;
				} else	
				if (!strcmp(w,"STDOUTPUTFMT")) {
					w=getword(&cParam);
					if (!strcmp(w,"AVI")) {
						sfOptions.iStdOutputFormat = SOF_AVI;
					} else 
					if (!strcmp(w,"MKV")) {
						sfOptions.iStdOutputFormat = SOF_MKV;
					} else bError=true;
				} else
				if (!strcmp(w,"RESOLUTION")) {
					w=getword(&cParam);
					m_OutputResolution.SetWindowText(w);
				} else
				if (!strcmp(w,"TITLE")) {
					w=cParam;
					UTF82Str(w, w);
					m_Title.SetWindowText(w);
				} else
				if (!strcmp(w,"OVERLAPPED")) {					
					w=getword(&cParam);
					if (isposint(w)) {
						settings->SetInt("output/general/overlapped", !!atoi(w));
					} else bError=true;
				} else
				if (!strcmp(w,"UNBUFFERED")) {
					w=getword(&cParam);
					if (isposint(w)) {
						settings->SetInt("output/general/unbuffered", !!atoi(w));
					} else bError = true;
				} else

					bError=true;
			}
			else
			if (!bSetOutputOptions)
			{
				w=getword(&cParam);
				if (!strcmp(w,"ADD_IMMED")) {
					w=getword(&cParam);
					if (isposint(w)) {
						bAddAS_immed = !!atoi(w);
					} else bError = true;
				} else
				if (!strcmp(w,"UNBUFFERED")) {
					w=getword(&cParam);
					if (isposint(w)) {
						settings->SetInt("input/unbuffered", !!atoi(w));
					} else bError = true;
				} else
				if (!strcmp(w,"USE")) {
					w=getword(&cParam);
					if (!strcmp(w,"CACHE")) {
						w=getword(&cParam);
						settings->SetInt("input/use cache",!!atoi(w));
					} else bError=true;
				}
				else
				if (!strcmp(w,"M2F2")) { 
					w=getword(&cParam);
					if (!strcmp(w,"CRC")) {
						w=getword(&cParam);
						ofOptions.dwFlags&=~SOFO_M2F2_DOM2F2CRCCHECK;
						ofOptions.dwFlags|=SOFO_M2F2_DOM2F2CRCCHECK*(!!atoi(w));
					} else bError=true;
				}
				else
				if (!strcmp(w,"AVI")) {
					w=getword(&cParam);
					if (!strcmp(w,"FORCE")) {
						w=getword(&cParam);
						if (!strcmp(w,"MP3VBR")) {
							w=getword(&cParam);
							ofOptions.dwFlags&=~SOFO_AVI_FORCEMP3VBR;
							ofOptions.dwFlags|=SOFO_AVI_FORCEMP3VBR*(!!atoi(w));
						} else bError=true;
					} 
					else
					if (!strcmp(w,"FIXDX50")) {
						w=getword(&cParam);
						ofOptions.dwFlags&=~SOFO_AVI_REPAIRDX50;
						ofOptions.dwFlags|=SOFO_AVI_REPAIRDX50*(!!atoi(w));
					} 
					else
					if (!strcmp(w,"IGNLCHUNKS")) {
						w=getword(&cParam);
						if (!strcmp(w,"ON")) {
							ofOptions.dwFlags|=SOFO_AVI_IGNORELARGECHUNKS;
						}
						else
						if (!strcmp(w,"OFF")) {
							ofOptions.dwFlags&=~SOFO_AVI_IGNORELARGECHUNKS;
						}
						else
						if (isposint(w)) {
							ofOptions.dwIgnoreSize=atoi(w);
						} else bError=true;
					} 
					else
					if (!strcmp(w,"TRY2FIXLCHUNKS")) {
						w=getword(&cParam);
						ofOptions.dwFlags&=~SOFO_AVI_TRYTOREPAIRLARGECHUNKS;
						ofOptions.dwFlags|=SOFO_AVI_TRYTOREPAIRLARGECHUNKS * (!!atoi(w));
					} else bError=true;
				}
				else
				if (!strcmp(w,"MP3")) {
					w=getword(&cParam);
					if (!strcmp(w,"VERIFY")) {
						w=getword(&cParam);
						if (!strcmp(w,"CBR")) {
							w=getword(&cParam);
							if (!strcmp(w,"NEVER")) {
								ofOptions.dwFlags&=~SOFO_MP3_MASK;
								ofOptions.dwFlags|=SOFO_MP3_CHECKCBRNEVER;
							}
							else
							if (!strcmp(w,"ALWAYS")) {
								ofOptions.dwFlags&=~SOFO_MP3_MASK;
								ofOptions.dwFlags|=SOFO_MP3_CHECKCBRALWAYS;
							}
							else
							if (!strcmp(w,"ASK")) {
								ofOptions.dwFlags&=~SOFO_MP3_MASK;
								ofOptions.dwFlags|=SOFO_MP3_CHECKCBRASK;
							}
							else bError=true;
						}
						else
						if (!strcmp(w,"RESDLG")) {
							w=getword(&cParam);
							ofOptions.dwFlags&=~SOFO_MP3_RESULTDLG;
							ofOptions.dwFlags|=SOFO_MP3_RESULTDLG*!!(atoi(w));
						} else bError=true;
					} else bError=true;
				} else
				if (!strcmp(w,"CHAPTERS")) {
					w=getword(&cParam);
					if (!strcmp(w,"IMPORT")) {
						w=getword(&cParam);
						if (isposint(w)) {
							ofOptions.dwFlags&=~SOFO_CH_IMPORT;
							ofOptions.dwFlags|=(!!atoi(w))*SOFO_CH_IMPORT;
						} else bError=true;
					} else
					if (!strcmp(w,"FROMFILENAMES")) {
						w=getword(&cParam);
						if (isposint(w)) {
							ofOptions.dwFlags&=~SOFO_CH_FROMFILENAMES;
							ofOptions.dwFlags|=(!!atoi(w))*SOFO_CH_FROMFILENAMES;
						} else bError=true;
					} else bError=true;
				} else bError=true;

			}
			if (bError) {
				wsprintf(cBuffer,LoadString(STR_LOAD_UNKNOWNOPT),entire_line,w);
				MessageBox(cBuffer,LoadString(STR_GEN_ERROR),MB_OK | MB_ICONERROR);
			}
			if (entire_line) {
				delete entire_line;
				entire_line = NULL;
			}
			delete (char*)lParam;
			break;
		}		

	}

	return CDialog::WindowProc(message, wParam, lParam);
}

void CAVIMux_GUIDlg::ApplyStreamSettings()
{
	HTREEITEM hItem, hChild;
	TREE_ITEM_INFO* tii[2];
	AUDIO_STREAM_INFO* asi;
	SUBTITLE_STREAM_INFO* ssi;

	hItem = m_AudioTree.GetRootItem();
	while (hItem) {
		tii[0] = m_AudioTree.GetItemInfo(hItem);
		if (tii[0]->iID == TIIID_ASI) {
			ssi = 0;
			asi = tii[0]->pASI;
		} 
		if (tii[0]->iID == TIIID_SSI) {
			asi = 0;
			ssi = tii[0]->pSSI;
		}

		hChild = m_AudioTree.GetChildItem(hItem);
		while (hChild) {

			tii[1] = m_AudioTree.GetItemInfo(hChild);
			switch (tii[1]->iID) {
				case TIIID_LNGCODE: 
					if (asi) asi->audiosource->SetLanguageCode(tii[1]->pText);
					if (ssi) ssi->lpsubs->SetLanguageCode(tii[1]->pText);
					break;
				case TIIID_STRNAME:
					if (asi) asi->audiosource->SetName(tii[1]->pText);
					if (ssi) ssi->lpsubs->SetName(tii[1]->pText);
					break;
			}

			hChild = m_AudioTree.GetNextSiblingItem(hChild);
		}

		hItem = m_AudioTree.GetNextSiblingItem(hItem);
	}

}

void CAVIMux_GUIDlg::OnSelchangedAudiotree(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen
	
	AUDIO_STREAM_INFO* asi = NULL;
	SUBTITLE_STREAM_INFO* ssi = NULL;
	TREE_ITEM_INFO*	   tii = NULL;
	TREE_ITEM_INFO*	   ptii = NULL;
	HTREEITEM hItem, hParent;
	char	c[200];
	ZeroMemory(c,sizeof(c));
	CString s;
	int i,j;
	bool bLngCode=false, bName=false;

	bDo_OnSelchangeLngCode = false;
	hItem = pNMTreeView->itemOld.hItem;
	hParent = NULL;
	if (hItem) tii = m_AudioTree.GetItemInfo(hItem);
	if (hItem) hParent = m_AudioTree.GetParentItem(hItem);
	if (hParent) ptii = m_AudioTree.GetItemInfo(hParent);

	if ((tii && tii->iID == TIIID_ASI) || (ptii && ptii->iID == TIIID_ASI)) {
		if (hItem) {
			if (tii->iID == TIIID_ASI) {
				asi = tii->pASI;
			} else asi = ptii->pASI;
			
			if (asi) {
				m_Audiodelay.GetWindowText(s);
				//asi->iDelay = atoi(s);
				SetDelay(asi, atoi(s));
			}
		}
	}

	bName = false;
	if (tii) {
		bLngCode = (tii->iID == TIIID_LNGCODE);
		bName = (tii->iID == TIIID_STRNAME);
	}

	if (tii && (bLngCode || bName )) {
		if (bName) {
			m_AudioName.GetWindowText(s);
		} else {
			i = m_Stream_Lng.GetCurSel();
			if (i == CB_ERR) {
				m_Stream_Lng.GetWindowText(s);
			} else {
				s=(char*)m_Stream_Lng.GetItemData(i);
			}
		}
		if (bLngCode && s.GetLength()) {
			char c[1024]; ZeroMemory(c,sizeof(c));
			Str2UTF8(s.GetBuffer(256),c);
			strcpy(tii->pText,c);
		}
	}

	hItem = pNMTreeView->itemNew.hItem;
	if (hItem) hParent = m_AudioTree.GetParentItem(hItem);
	tii = m_AudioTree.GetItemInfo(hItem);
	if (hParent) ptii = m_AudioTree.GetItemInfo(hParent);

	//m_AudioName.SetWindowText("");
	if (tii && tii->iID == TIIID_ASI || ptii && ptii->iID == TIIID_ASI) {
		if (tii->iID == TIIID_ASI) asi = tii->pASI; else asi = ptii->pASI;

		sprintf(c,"%d",asi->iDelay);
		m_Audiodelay.SetWindowText(c);
	}

	if (tii) {
		bLngCode = (tii->iID == TIIID_LNGCODE);
		bName = (tii->iID == TIIID_STRNAME);
	}
	if (tii && (bLngCode || bName)) {
		char s[1024]; ZeroMemory(s,sizeof(s));
		UTF82Str(tii->pText,s);
		if (bName) {
			m_AudioName.SetWindowText(s);
		} else {
			m_Stream_Lng.SetCurSel(-1);
			j=0;
			for (i=0;i<m_Stream_Lng.GetCount() && !j;i++) {
				if (!strcmp((char*)m_Stream_Lng.GetItemData(i),tii->pText)) {
					m_Stream_Lng.SetCurSel(i);
					j=1;
				}
			}
			if (!j) {
				m_Stream_Lng.SetWindowText(tii->pText);
			}
		}
	}

	if (tii && (tii->iID == TIIID_LNGCODE)) {
		m_Stream_Lng.ShowWindow(SW_SHOW);
		m_AudioName.ShowWindow(SW_HIDE);
		m_Audioname_Label.ShowWindow(SW_SHOW);
	} else {
		m_Stream_Lng.ShowWindow(SW_HIDE);
		m_AudioName.ShowWindow(SW_HIDE);
		m_Audioname_Label.ShowWindow(SW_HIDE);
	}

	ApplyStreamSettings();

	m_AudioTree.Invalidate();
	m_AudioTree.UpdateWindow();

	bDo_OnSelchangeLngCode = true;

	*pResult = 0;
}

void CAVIMux_GUIDlg::OnItemexpandedAudiotree(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen
	m_AudioTree.Invalidate();
	m_AudioTree.UpdateWindow();

	*pResult = 0;
}

void CAVIMux_GUIDlg::OnSelchangeSourcefilelist() 
{
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen
	FILE_INFO*	fi;
	int iIndex;

	if ((iIndex=m_SourceFiles.GetCurSel())!=LB_ERR) {
		CBuffer* cb = (CBuffer*)m_SourceFiles.GetItemData(iIndex);
		fi = (FILE_INFO*)cb->GetData();

		if (fi->dwType & FILETYPE_MKV) {
			char cTitle[200];
			UTF82Str(fi->MKVFile->GetSegmentTitle(),cTitle);
			m_Title.SetWindowText(cTitle);
		} else if (fi->dwType & FILETYPE_AVI) {
			char cTitle[200];
			UTF82Str(fi->AVIFile->GetTitle(),cTitle);
			m_Title.SetWindowText(cTitle);
		} 

		else m_Title.SetWindowText("");
	}
}

void CAVIMux_GUIDlg::ApplyCurrentLanguageCode()
{
	HTREEITEM hItem = m_AudioTree.GetSelectedItem();
	TREE_ITEM_INFO* tii = hItem?m_AudioTree.GetItemInfo(hItem):0;
	int j;

	if (tii) {
		if (tii->iID == TIIID_LNGCODE) {
			char cLng[10];
			if ((j=m_Stream_Lng.GetCurSel()) != CB_ERR) {
				strncpy(cLng,(char*)m_Stream_Lng.GetItemData(j),9);
			} else {
				m_Stream_Lng.GetWindowText(cLng,sizeof(cLng));
			}
			strcpy(tii->pText, cLng);
		}
	}
}

void CAVIMux_GUIDlg::ApplyCurrentLanguageName()
{
/*	HTREEITEM hItem = m_AudioTree.GetSelectedItem();
	TREE_ITEM_INFO* tii = hItem?(TREE_ITEM_INFO*)m_AudioTree.GetItemData(hItem):0;

	if (tii) {
		if (tii->iID == TIIID_STRNAME) {
			char cLng[256];
			m_AudioName.GetWindowText(cLng,256);
			Str2UTF8(cLng, cLng);
			strcpy(tii->pText, cLng);
		}
	}	
	m_AudioTree.InvalidateRect(NULL);
	m_AudioTree.UpdateWindow();
	*/
}

void CAVIMux_GUIDlg::OnEditchangeStreamLng() 
{
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen
}

void CAVIMux_GUIDlg::OnSelchangeStreamLng() 
{
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen
//	if (bDo_OnSelchangeLngCode) UpdateAudioName();	
	ApplyCurrentLanguageCode();
	m_AudioTree.InvalidateRect(NULL);
	m_AudioTree.UpdateWindow();
}

void CAVIMux_GUIDlg::OnEditupdateStreamLng() 
{
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen
	OnSelchangeStreamLng();

}

void CAVIMux_GUIDlg::OnUpdateAudioname() 
{
	// TODO: Wenn es sich hierbei um ein RICHEDIT-Steuerelement handelt, sendet es
	// sendet diese Benachrichtigung nur, wenn die Funktion CDialog::OnInitDialog()
	// überschrieben wird, um die EM_SETEVENTMASK-Nachricht an das Steuerelement
	// mit dem ENM_UPDATE-Attribut Ored in die Maske lParam zu senden.
	
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen

	ApplyCurrentLanguageName();
}

void CAVIMux_GUIDlg::OnGetAudiodispinfo(NMHDR* pNMHDR, LRESULT* pResult) 
{
	TV_DISPINFO* pTVDispInfo = (TV_DISPINFO*)pNMHDR;
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen
	
	AUDIO_STREAM_INFO* asi = NULL;
	TREE_ITEM_INFO*	   tii = NULL;
	char*	d = pTVDispInfo->item.pszText;
	bool	bBracket = false;
	char	c[256];
	ZeroMemory(c,sizeof(c));
	HTREEITEM hItem = pTVDispInfo->item.hItem;
	char* AAC_prof_name;
	bool bAddComma=false;
	int i,j;
	
	if ((pTVDispInfo->item.mask & TVIF_TEXT) == 0) {
		*pResult = 0;
		return;
	}
	
	tii = m_AudioTree.GetItemInfo(hItem);

	if (!bEditInProgess) {
		if (tii && tii->iID == TIIID_ASI) {
			*d = 0;
			
			if (tii->pASI->audiosource->IsDefault()) {
				strcat(d,"(default) ");
			}
			strcat(d,"audio: ");
			if (tii) asi = tii->pASI;
			if (asi && asi->bNameFromFormatTag) {
				int idatarate = asi->audiosource->GetAvgBytesPerSec()/125;
				char cdatarate[128]; cdatarate[0]=0;
				if (idatarate) sprintf(cdatarate,"%d kbps",idatarate);
				  else strcpy(cdatarate,"unknown bitrate");

				switch (asi->dwType) {
					case AUDIOTYPE_MP3CBR:
						i = (int)asi->audiosource->FormatSpecific(MMSGFS_MPEG_LAYERVERSION);
						j = (int)asi->audiosource->FormatSpecific(MMSGFS_MPEG_VERSION);
						sprintf(c,"MPEG %d Layer %d (CBR, %s",j,i,cdatarate);
						bBracket = true;
						break;
					case AUDIOTYPE_MP3VBR:
						i = (int)asi->audiosource->FormatSpecific(MMSGFS_MPEG_LAYERVERSION);
						j = (int)asi->audiosource->FormatSpecific(MMSGFS_MPEG_VERSION);
						sprintf(c,"MPEG %d Layer %d (VBR",j, i);
						bBracket = true;
						break;
					case AUDIOTYPE_PLAINCBR:
						sprintf(c,"CBR (?");
						bBracket = true;
						break;
					case AUDIOTYPE_AC3:
						sprintf(c,"AC3 (%s, %s",cdatarate, asi->audiosource->IsCBR()?"CBR":"VBR");
						bBracket = true;
						break;
					case AUDIOTYPE_PCM:
						sprintf(c,"PCM");
						break;
					case AUDIOTYPE_DTS:
						sprintf(c,"DTS (%s, %s",cdatarate, asi->audiosource->IsCBR()?"CBR":"VBR");
						bBracket = true;
						break;
					case AUDIOTYPE_VORBIS:
						sprintf(c,"Vorbis (%s %dCh, %dHz",cdatarate, asi->audiosource->GetChannelCount(),
							asi->audiosource->GetFrequency());
						bBracket = true;
						break;
					case AUDIOTYPE_AAC:
						switch (asi->audiosource->FormatSpecific(MMSGFS_AAC_PROFILE)) {
							case AAC_ADTS_PROFILE_LC: AAC_prof_name = "LC"; break;
							case AAC_ADTS_PROFILE_LTP: AAC_prof_name = "LTP"; break;
							case AAC_ADTS_PROFILE_MAIN: AAC_prof_name = "MAIN"; break;
							case AAC_ADTS_PROFILE_SSR: AAC_prof_name = "SSR"; break;
							default: AAC_prof_name = "unknown";

						}
						sprintf(c,"%sAAC (MPEG %d, %s, %dHz, %dCh",
							(int)asi->audiosource->FormatSpecific(MMSGFS_AAC_ISSBR)?"HE-":"",
							(int)asi->audiosource->FormatSpecific(MMSGFS_AAC_MPEGVERSION),
							AAC_prof_name, asi->audiosource->GetFrequency(),
							asi->audiosource->GetChannelCount());
						bBracket = true;
						break;
					case AUDIOTYPE_DIVX:
						sprintf(c,"divX %s %dCh, %dHz",cdatarate, asi->audiosource->GetChannelCount(),
							asi->audiosource->GetFrequency());
						break;
				/*	default:
						if (asi->audiosource->GetIDString()) {
							sprintf(c,"%s (%s",asi->audiosource->GetIDString(),cdatarate);
						}
						break;				
						*/
				}
			}

			strcat(d,c);

			if (asi) {
				int offset = asi->audiosource->GetOffset();
				if (offset) {
					sprintf(c,", bad: %d ",offset);
					strcat(d,c);
				}
				if (asi->iDelay) {
					sprintf(c,", delay: %d ms",asi->iDelay);
					strcat(d,c);
				}
			}

			if (asi && !asi->bNameFromFormatTag) {
				sprintf(c,asi->audiosource->GetIDString());
				strcat(d,c);
			}

			if (!bBracket) {
				strcat(d," (");
				bBracket = true;
			} else {
				strcat(d,", ");
			}

			if (asi && asi->iSize) {
				char cSize[30];
				FormatSize(cSize,asi->iSize);
				strcat(d,cSize);
				bAddComma = true;
			}

			if (!(m_AudioTree.GetItemState(hItem,TVIS_EXPANDED) & TVIS_EXPANDED) && 
				  m_AudioTree.FindID(hItem,TIIID_LNGCODE,&tii) && tii->pText && strlen(tii->pText)) {
				if (bAddComma) strcat(d,", ");
				bAddComma = true;
				sprintf(c,"%s",tii->pText);
				strcat(d,c);
			}

			if (!(m_AudioTree.GetItemState(hItem,TVIS_EXPANDED) & TVIS_EXPANDED) && 
				  m_AudioTree.FindID(hItem,TIIID_STRNAME,&tii) && tii->pText && strlen(tii->pText)) {
				if (bAddComma) strcat(d,", ");
				bAddComma = false;
				sprintf(c,"%s",tii->pText);
				strcat(d,c);
			}

			if (bBracket) strcat(d,")");

		} else
		if (tii && tii->iID == TIIID_SSI) {
			*d = 0;
			if (tii->pSSI->lpsubs->IsDefault()) {
				strcat(d,"(default) ");
			}

			SUBTITLESOURCE* subs = tii->pSSI->lpsubs;

			strcat(d,"subtitle: ");
			if (tii->pSSI) {
				switch (tii->pSSI->lpsubs->GetFormat()) {
					case SUBFORMAT_SRT:
						sprintf(c,"SRT");
						break;
					case SUBFORMAT_SSA:
						sprintf(c,"SSA");
						break;
					case SUBFORMAT_VOBSUB:
						sprintf(c,"Vobsub");
						break;
				}
				strcat(d,c);
				bAddComma = false;
				bBracket = true;
				
				if (!(m_AudioTree.GetItemState(hItem,TVIS_EXPANDED) & TVIS_EXPANDED) && 
					  m_AudioTree.FindID(hItem,TIIID_LNGCODE,&tii) && tii->pText && strlen(tii->pText)) {
					sprintf(c," (%s",tii->pText);
					strcat(d,c);
					bAddComma = true;
					bBracket = false;
				}
				if (!(m_AudioTree.GetItemState(hItem,TVIS_EXPANDED) & TVIS_EXPANDED) && 
					  m_AudioTree.FindID(hItem,TIIID_STRNAME,&tii) && tii->pText && strlen(tii->pText)) {
					if (bAddComma) strcat(d,", ");
					if (bBracket) strcat(d," (");
					bAddComma = true;
					bBracket = false;
					sprintf(c,"%s",tii->pText);
					strcat(d,c);
				}
				int _i; 
				if ((_i = subs->GetCompressionAlgo()) != COMP_NONE) {
					if (bAddComma) strcat(d,", ");
					if (bBracket) strcat(d," (");
					bAddComma = false;
					bBracket = false;
					sprintf(c,"compression: %s",((_i == COMP_ZLIB)?"zlib":"unknown"));
					strcat(d,c);
				}

				if (!bBracket) strcat(d,")");
			}

		} else

		if (tii && tii->iID == TIIID_LNGCODE) {
			sprintf(d,"language code: %s", tii->pText);
		} else

		if (tii && tii->iID == TIIID_STRNAME) {
			sprintf(d,"stream name: %s", tii->pText);
		}
	}

	*pResult = 0;
}

void CAVIMux_GUIDlg::OnBeginlabeleditAudiotree(NMHDR* pNMHDR, LRESULT* pResult) 
{
	TV_DISPINFO* pTVDispInfo = (TV_DISPINFO*)pNMHDR;
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen
	
	AUDIO_STREAM_INFO* asi = NULL;
	TREE_ITEM_INFO*	   tii = NULL;
	char*	d = pTVDispInfo->item.pszText;
	bool	bBracket = false;
	char	c[256];
	ZeroMemory(c,sizeof(c));
	HTREEITEM hItem = pTVDispInfo->item.hItem;
	int unicode = SendDlgItemMessage(IDC_AUDIOTREE, TVM_GETUNICODEFORMAT);
		
	tii = m_AudioTree.GetItemInfo(hItem);
	if (tii->iID != TIIID_STRNAME) {
		*pResult = 1;
		return;
	}
	
	if (unicode) {
		HWND edit = TreeView_GetEditControl((HWND)m_AudioTree.m_hWnd);
		UTF82WStr(tii->pText, c);
		SendMessageW(edit, WM_SETTEXT, 0, (LPARAM)c);
	} else {
		CEdit* edit = m_AudioTree.GetEditControl();
		UTF82Str(tii->pText, c);
		edit->SetWindowText(c);
	}

	bEditInProgess = 1;

	*pResult = 0;
}

void CAVIMux_GUIDlg::OnEndlabeleditAudiotree(NMHDR* pNMHDR, LRESULT* pResult) 
{
	TV_DISPINFO* pTVDispInfo = (TV_DISPINFO*)pNMHDR;
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen
	
	TREE_ITEM_INFO*	   tii = NULL;
	HTREEITEM hItem = pTVDispInfo->item.hItem;
	int unicode = SendDlgItemMessage(IDC_AUDIOTREE, TVM_GETUNICODEFORMAT);

	char	c[512];
	ZeroMemory(c,sizeof(c));
		
	if (tii = m_AudioTree.GetItemInfo(hItem)) {

		if (unicode) {
			HWND edit = TreeView_GetEditControl(m_AudioTree.m_hWnd);
			SendMessageW(edit, WM_GETTEXT, 256, (LPARAM)c);
			WStr2UTF8(c, c);
		} else {
			CEdit* edit = m_AudioTree.GetEditControl();
			edit->GetWindowText(c, 512);
			Str2UTF8(c, c);
		}
		strcpy(tii->pText, c);
	}

	bEditInProgess = 0;

	*pResult = 0;
}


void CAVIMux_GUIDlg::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	// TODO: Code für die Behandlungsroutine für Nachrichten hier einfügen und/oder Standard aufrufen
	
	if (nChar == 13 && bEditInProgess) return;

	CDialog::OnKeyDown(nChar, nRepCnt, nFlags);
}


void CAVIMux_GUIDlg::OnReturnAudiotree(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen
	
	*pResult = 0;
}

CAttribs* CAVIMux_GUIDlg::GetSettings()
{
	return settings;
}
