// AVIMux_GUIDlg.cpp : Implementierungsdatei
//


/*
WM_QUIT

	TODO:

	    - add: write progress to stdout
		- add: automatic segment name creation
		- add: allow to disable cleartype fonts
	    - check: sync is b0rked for AVI output when file starts with a deltaframe
		- add: reorder ssa timecodes for output
		- add: try vorbis-in-avi output

		- fix: last-job.amg is b0rked
		- fix: when pressing space on track title/lng, a check box appears

		- fix: free space warning when overwriting files

		- fixed: saves files now saves chapter file along with normal file
		- fixed: Chapter Editor allowed to set a title for a not-yet-defined
		         language code when selecting an edition with no language code
		- added: read files from stdin
		- added: can apply title and language code to video stream
		         - includes SET OPTION VIDEO NAME etc
		- added: new handler for TAB key
		- added: report win 9x/me/nt4 as fatal error
		- added: EBML tree / main window font size and quality changeable
		- added: EBML Tree / RIFF Tree now launchable from main window
		- added: got rid of separate video stream list
		- added: use double click to add file source
		- added: can remove files already added with dependency tracking
		- changed: chapteruids are now 64 bits per default
		- (hopefully) fix: mka splitting by time
		- (done) fix: RIFF Tree window sucks
		- (done) fix: shouldn't allow to change a chapter title that doesn't exist
		              (unlike in french legislation, you can't really change something
					  after something that doesn't exist)
		- (done) fix: problems with very small timecode scale values
		- (done) fix: LOAD cannot open files in network folders
		- (done) fix: small timecode scale values with out-of-timecode-order video 
		              could b0rk the stream because a clusters timecode underrun
					  wasn't caught
	    - (done) fix: the very first frames did not get a CuePoint
		- (done) fix: dropped first frame of files where the 2nd frame wasn't in the
		              same cluster as the first frame when the first frame was not
					  CuePointed
		- (done) fix: avi 1.0 size warning, disk space warning and disk file 
		              system warning to occur only when necessary
        - (done) fix: check vorbis input from mkv
		- (done) fix: doctype version is UINT not INT (EBML Tree was b0rked)
		- (done) fix: AVI overhead estimation for haali mode
		- (done) fix: remuxing laced vorbis got b0rked somewhere in 1.17.4.x
		- (done) fix: didn't work with audio+subs without video
		- (done): remove crappy video list box
		- (done) fix: CACHE completely rewritten, asynch i/o is now ok
		- (hopefully done) fix: crash of unicode tree under mysterious circumstances
  --------------------------------------------------------------------------------
   1.17.4.x
		- (done) fix: crash when loading extremely large xml file 
		- (done) fix: bug when loading chapters with end < 4sec in chapter editor from mkc
		- (done) fix: didn't load ordered/hidden flag in editions (new 1.17.4 bug)
  --------------------------------------------------------------------------------
		- (done) add: recognise A_AAC audio codec id
		- (done) add: raw video stream extraction
		- (done) add: reading and writing simpleblock
		- (done) fix: "he lc acc" display
		- (done) fix: crash on SMI input files
		- (done) add: make segment title edit unicode
		- (done) add: resizeable chapter editor
		- (done) fix: aac-to-avi with delay didn't work due to improperly initialized framing mode
		- (done) add: resizeable dialogs
		- (done) fix: information windows could crash when being opened several times with files with a lot of chapters
		- (done) add: reading header striping compression
		- (done) add: ebml header check for matroska v2-
		- (done) fix: chapter editor crashed when importing a file without segmentuid
		- (done) fix: couldn't read files with nonclusters between clusters
		- (done) fix: crash when closing multisegment file due to b0rk in FlushQueues
  -----------------------------------------------------------------------
		- (done) fix: creating a phantom file at the end of avi muxing
		- (done) fix: crash when dropping a wrong file on the chapter editor
		- (done) fix: bug in cache which sometimes prevented re-starting muxing  
  -----------------------------------------------------------------------
		- (done) add: hard linking
		- (done) fix: chapter handling with medium linking on splitting
		- (done) fix: WAVE input is b0rked for MKV output
		- (done) fix: use CrytpAquireContext + CryptGenRandom() instead of rand()
		- (done) fix: auto header size b0rked if clusters are indexed
        - (done) add: MKVMerge style xml file output
		- (done) fix: could not launch from a directory containing non-ansi characters
		- (done) fix: SSA files with weird spaces in the style defs b0rk
		- (done) fix: reading nonsense 0 byte video frame at joining point with MKV input
		- (done) fix: didn't index some L1 elements in seekhead in 1.17.1
		- (done) add: fill free space before 1st cluster with Cues
		- (done) fix: miss existence of last file when split points were indicated
  ------------------------------------------------------------------------
		- (done) add: mpeg layer 1/2 audio for avi-output
		- (done) fix: b0rked for mkv input files without cues
		- (done) fix: generate bad stream name from unicode filename
		- (done) fix: overwritedlg 0 is b0rked
		- (done) fix: demux audio/subs file name dialog still ansi
		- (done) fix: ebml tree is b0rked for segment size { 0xff }
*/


#include "stdafx.h"
#include "AudioSource.h"
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
#include "../strings.h"
#include "SubTitles.h"
#include "videosource.h"
#include "videosourcelistbox.h"
#include "WAVFile.h"
#include "..\matroska.h"
#include "Languages.h"
#include "TextFiles.h"
#include "Trees.h"
#include "AudioSourceTree.h"
#include "..\XML.h"
#include "XMLFiles.h"
#include "FileDialogs.h"
#include "AVIFile.h"
#include "UTF8Windows.h"
#include "Version.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg-Dialogfeld für Anwendungsbefehl "Info"

void ProcessMsgQueue(HWND hwnd)
{
	MSG _msg;

	while (PeekMessage(&_msg, hwnd, 0, 0, PM_REMOVE)) {
		::TranslateMessage(&_msg);
		::DispatchMessage(&_msg);
	}
}


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
	: CResizeableDialog(CAVIMux_GUIDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAVIMux_GUIDlg)
	//}}AFX_DATA_INIT
	// Beachten Sie, dass LoadIcon unter Win32 keinen nachfolgenden DestroyIcon-Aufruf benötigt
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CAVIMux_GUIDlg::DoDataExchange(CDataExchange* pDX)
{
	CResizeableDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAVIMux_GUIDlg)
	DDX_Control(pDX, IDC_CHAPTER_EDITOR, m_Chapter_Editor);
	DDX_Control(pDX, IDS_MAIN_AUDIONAME, m_Stream_Lng_Label);
	DDX_Control(pDX, IDC_OUTPUTRESOLUTION, m_OutputResolution);
	DDX_Control(pDX, IDC_DESTFILE, m_Prg_Dest_File);
	DDX_Control(pDX, IDC_STATUSLINE, m_StatusLine);
	DDX_Control(pDX, IDC_STREAM_LNG, m_Stream_Lng);
	DDX_Control(pDX, IDC_OUTPUTRESOLUTION_LABEL, m_OutputResolution_Label);
	DDX_Control(pDX, IDC_AUDIOTREE, m_StreamTree);
	DDX_Control(pDX, IDC_TITLELABEL, m_Title_Label);
	DDX_Control(pDX, IDC_TITLE, m_Title);
	DDX_Control(pDX, IDC_VIDEOSTRETCHFACTORLABEL, m_VideoStretchFactor_Label);
	DDX_Control(pDX, IDC_VIDEOSTRETCHFACTOR, m_VideoStretchFactor);
	DDX_Control(pDX, IDC_PROGRESS_LIST, m_Progress_List);
	DDX_Control(pDX, IDC_PROTOCOL, m_Protocol);
	DDX_Control(pDX, IDC_OUTPUTOPTIONS, m_Output_Options_Button);
	DDX_Control(pDX, IDC_START, m_Start_Button);
	DDX_Control(pDX, ID_LEAVE, m_Cancel_Button);
	DDX_Control(pDX, IDS_MAIN_OPENFILES, m_Open_Files_Label);
	DDX_Control(pDX, IDC_FRAMES, m_Prg_Frames);
	DDX_Control(pDX, IDS_MAIN_PRG_FRAMES, m_Prg_Frames_Label);
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
	DDX_Control(pDX, IDS_MAIN_SUBTITLES, m_Subtitles_Label);
	DDX_Control(pDX, IDS_MAIN_AUDIO, m_Audio_Label);
	DDX_Control(pDX, IDC_ENHLIST, m_Enh_Filelist);
	DDX_Control(pDX, IDS_MAIN_AUDIODELAY, m_Audiodelay_Label);
	DDX_Control(pDX, IDC_DELAY, m_Audiodelay);
	DDX_Control(pDX, IDC_AUDIONAME, m_AudioName);
	DDX_Control(pDX, IDS_MAIN_PROTOCOL, m_Protocol_Label);
	DDX_Control(pDX, IDC_SOURCEFILELIST, m_SourceFiles);
	//}}AFX_DATA_MAP
	DDX_Control(pDX, IDC_STOP, m_Stop_Button);
}

BEGIN_MESSAGE_MAP(CAVIMux_GUIDlg, CResizeableDialog)
	//{{AFX_MSG_MAP(CAVIMux_GUIDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_START, OnStart)
	ON_BN_CLICKED(IDC_ADDAVILIST, OnAddFileList)
	ON_WM_LBUTTONUP()
	ON_BN_CLICKED(IDC_MAXSIZE_EXTENDED, OnMaxsizeExtended)
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
	ON_NOTIFY(TVN_BEGINLABELEDIT, IDC_AUDIOTREE, OnBeginlabeleditAudiotree)
	ON_NOTIFY(TVN_ENDLABELEDIT, IDC_AUDIOTREE, OnEndlabeleditAudiotree)
	ON_EN_CHANGE(IDC_OUTPUTRESOLUTION, OnChangeOutputresolution)
	ON_WM_KEYDOWN()
	ON_BN_CLICKED(IDC_CHAPTER_EDITOR, OnChapterEditor)
	ON_WM_DESTROY()
	ON_CBN_KILLFOCUS(IDC_STREAM_LNG, OnKillfocusStreamLng)
	ON_NOTIFY(NM_KILLFOCUS, IDC_AUDIOTREE, OnKillfocusAudiotree)
	ON_EN_KILLFOCUS(IDC_DELAY, OnKillfocusDelay)
	ON_BN_CLICKED(IDC_STOP, OnStop)
	ON_NOTIFY(TVN_BEGINLABELEDITW, IDC_AUDIOTREE, OnBeginlabeleditAudiotree)
	ON_NOTIFY(TVN_ENDLABELEDITW, IDC_AUDIOTREE, OnEndlabeleditAudiotree)
	ON_EN_SETFOCUS(IDC_DELAY, OnSetfocusDelay)
	//}}AFX_MSG_MAP
	ON_WM_SIZE()
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_PROGRESS_LIST, OnLvnItemchangedProgressList)
	ON_BN_CLICKED(ID_LEAVE, OnBnClickedLeave)
	ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)
	ON_LBN_DBLCLK(IDC_SOURCEFILELIST, OnLbnDblclkSourcefilelist)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAVIMux_GUIDlg Nachrichten-Handler

#include "global.h"


HWND GetFocushWnd(CWnd* cwnd)
{
	CWnd* w = cwnd->GetFocus();
	
	if (!w)
		return NULL;

	return w->m_hWnd;
}

template <class T> bool insert_new_element(std::vector<T>& v, T item)
{
	std::vector<T>::iterator iter = v.begin();
	while (iter != v.end() && *iter != item)
		iter++;

	if (iter == v.end()) {
		v.push_back(item);
		return true;
	} else
		return false;
}

void CAVIMux_GUIDlg::BuildFileAndStreamDependency(
	DWORD file_to_remove,
	std::vector<HTREEITEM>& hitems, 
	std::vector<DWORD>& stream_numbers,
	std::vector<DWORD>& file_numbers_in_list)
{
	std::vector<HTREEITEM>::iterator hitem_iter;
	std::vector<DWORD>::iterator index_iter;
	std::vector<DWORD>::iterator file_iter;
	std::vector<DWORD> file_indexes;

	file_indexes.push_back(m_SourceFiles.GetFileInfo(file_to_remove)->file_id);

	/* build a file dependency list */
	file_iter = file_indexes.begin();
	for (; file_iter != file_indexes.end(); file_iter++) {
		file_to_remove = *file_iter;
		FILE_INFO* f = m_SourceFiles.GetFileInfo(file_to_remove);
		HTREEITEM hItem = m_StreamTree.GetRootItem();
	/* find all streams that have f->iOrgPos in it */
		while (hItem) {
			TREE_ITEM_INFO* tii = (TREE_ITEM_INFO*)m_StreamTree.GetItemData(hItem);
			DWORD count = tii->pMSI->lpdwFiles[0];
			DWORD* dwItem = tii->pMSI->lpdwFiles + 1;
			for (DWORD i=0; i<count; i++) {
				DWORD dwFileIndex = dwItem[i];
				if (dwFileIndex == file_to_remove) {
					insert_new_element(hitems, hItem);
					for (DWORD j=0; j<count; j++) 
						if (insert_new_element(file_indexes, dwItem[j]))
							file_iter = file_indexes.begin();
				}
			}
			hItem = m_StreamTree.GetNextSiblingItem(hItem);
		}
	}

	/* convert file_id numbers in file_indexes to indexes into m_SourceFiles */
	for (file_iter = file_indexes.begin(); file_iter != file_indexes.end(); file_iter++) {
		for (int i=0; i<m_SourceFiles.GetCount(); i++) {
			FILE_INFO* fi = m_SourceFiles.GetFileInfo(i);
			if (fi->file_id == *file_iter)
				file_numbers_in_list.push_back(i);
		}
	}

	/* convert HTREEITEMs to list of indexes into m_StreamTree */
	for (hitem_iter = hitems.begin(); hitem_iter != hitems.end(); hitem_iter++) {
		HTREEITEM h = m_StreamTree.GetRootItem();
		int counter = 0;
		while (*hitem_iter != h) {
			h = m_StreamTree.GetNextSiblingItem(h);
			counter++;
		}
		stream_numbers.push_back(counter);
	}

}


void CAVIMux_GUIDlg::SetDialogState_Config()
{
	iCurrentView=1;

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
	m_Output_Options_Button.ShowWindow(SW_SHOW);
	m_AudioName.ShowWindow(SW_HIDE);

// restore visibility of m_Audiodelay
	HTREEITEM h = m_StreamTree.GetSelectedItem();
	m_StreamTree.SelectItem(0);
	m_StreamTree.SelectItem(h);
	
	m_Audio_Label.ShowWindow(SW_SHOW);

	m_No_Audio.ShowWindow(SW_SHOW);
	m_All_Audio.ShowWindow(SW_SHOW);
	m_No_Subtitles.ShowWindow(SW_SHOW);
	m_All_Subtitles.ShowWindow(SW_SHOW);

// selecting audio stream by number is hidden
	m_Default_Audio.ShowWindow(SW_HIDE);
	m_Default_Audio_Label.ShowWindow(SW_HIDE);

	
	//	m_AvailableStreams_Header.ShowWindow(SW_SHOW);
	m_SourceFiles.ShowWindow(SW_SHOW);
	m_Add_Video_Source.ShowWindow(SW_SHOW);
	m_Open_Files_Label.ShowWindow(SW_SHOW);

	::ShowWindow(hTitleEdit, SW_SHOW);

	m_Title_Label.ShowWindow(SW_SHOW);
	m_StreamTree.ShowWindow(SW_SHOW);
	m_Subtitles_Label.ShowWindow(SW_SHOW);

	if (sfOptions.bB0rk) {
		m_VideoStretchFactor.ShowWindow(SW_SHOW);
		m_VideoStretchFactor_Label.ShowWindow(SW_SHOW);
	} else {
		m_VideoStretchFactor.ShowWindow(SW_HIDE);
		m_VideoStretchFactor_Label.ShowWindow(SW_HIDE);
	}

	m_Chapter_Editor.ShowWindow(SW_SHOW);
}

void CAVIMux_GUIDlg::SetDialogState_Muxing()
{
	iCurrentView=2;

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
	m_Output_Options_Button.ShowWindow(SW_HIDE);
	m_AudioName.ShowWindow(SW_HIDE);
	m_Stream_Lng_Label.ShowWindow(SW_HIDE);
	m_Audiodelay.ShowWindow(SW_HIDE);
	m_Audiodelay_Label.ShowWindow(SW_HIDE);
	m_Audiodelay.ShowWindow(SW_HIDE);

	m_Audio_Label.ShowWindow(SW_HIDE);

	m_No_Audio.ShowWindow(SW_HIDE);
	m_All_Audio.ShowWindow(SW_HIDE);
	m_No_Subtitles.ShowWindow(SW_HIDE);
	m_All_Subtitles.ShowWindow(SW_HIDE);
	m_Default_Audio.ShowWindow(SW_HIDE);
	m_Default_Audio_Label.ShowWindow(SW_HIDE);
	m_SourceFiles.ShowWindow(SW_HIDE);
	m_Add_Video_Source.ShowWindow(SW_HIDE);
	m_Open_Files_Label.ShowWindow(SW_HIDE);

	::ShowWindow(hTitleEdit, SW_HIDE);

	m_Title_Label.ShowWindow(SW_HIDE);
	m_StreamTree.ShowWindow(SW_HIDE);
	m_Stream_Lng.ShowWindow(SW_HIDE);
	m_Subtitles_Label.ShowWindow(SW_HIDE);

	m_VideoStretchFactor.ShowWindow(SW_HIDE);
	m_VideoStretchFactor_Label.ShowWindow(SW_HIDE);

	m_Chapter_Editor.ShowWindow(SW_HIDE);

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
	
RECT protocol_client_rect;

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
			if (!i++) 
				sprintf(cBuffer,"%02d:%02d:%02d.%03d",time.wHour,time.wMinute,time.wSecond,time.wMilliseconds);
			
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
			RECT r;
			m_Protocol.GetItemRect(m_Protocol.GetItemCount()-1, &r, LVIR_BOUNDS);
			m_Protocol.InvalidateRect(&r);
			m_Protocol.UpdateWindow();

		} while ((cString=c) && (cString++));
	}

	RECT r;
	m_Protocol.GetClientRect(&r);
	if (memcmp(&r, &protocol_client_rect, sizeof(RECT)))
		UpdateProtocolColumn();

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

void CAVIMux_GUIDlg::UpdateProgressList()
{
	static int diff = 2;

	if (!m_Progress_List.m_hWnd)
		return;

/*	m_Progress_List.DeleteAllItems();
	while (m_Progress_List.GetColumn(0, NULL))
		m_Progress_List.DeleteColumn(0);
*/
	RECT rect;
	m_Progress_List.GetClientRect(&rect);
//	m_Progress_List.DeleteAllItems();

	int w = (rect.right-rect.left)/6;

	LVCOLUMN lvcol; memset(&lvcol, 0, sizeof(lvcol));
	lvcol.mask = LVCF_WIDTH;
	if (!m_Progress_List.GetColumn(0, &lvcol) && !lvcol.cx) {
		m_Progress_List.InsertColumn(0,"",LVCFMT_RIGHT, w-diff);
		m_Progress_List.InsertColumn(1,LoadString(STR_MAIN_S_PRG_VIDEO),LVCFMT_RIGHT, w);
		m_Progress_List.InsertColumn(2,LoadString(STR_MAIN_S_PRG_AUDIO),LVCFMT_RIGHT, w);
		m_Progress_List.InsertColumn(3,LoadString(STR_MAIN_S_PRG_OVERHEAD),LVCFMT_RIGHT, w);
		m_Progress_List.InsertColumn(4,LoadString(STR_MAIN_S_PRG_SUBTITLES),LVCFMT_RIGHT, w);
		m_Progress_List.InsertColumn(5,LoadString(STR_MAIN_S_PRG_TOTAL),LVCFMT_RIGHT, w);
		m_Progress_List.InsertItem(0,LoadString(STR_MAIN_S_PRG_CURRFILE));
		m_Progress_List.InsertItem(1,LoadString(STR_MAIN_S_PRG_ALLFILES));
		m_Progress_List.InsertItem(2,LoadString(STR_MAIN_S_PRG_TRANSFERRATE));
	} else {
		for (int j=1;j<6;j++) 
			m_Progress_List.SetColumnWidth(j, w);
		m_Progress_List.SetColumnWidth(0, w - diff);
	}

	m_Progress_List.InvalidateRect(NULL);
	m_Progress_List.UpdateWindow();
/* use smaller size until no vertical scroll bar is visible */
	SCROLLBARINFO sbi; memset(&sbi, 0, sizeof(sbi));
	sbi.cbSize = sizeof(sbi);
	m_Progress_List.GetScrollBarInfo(OBJID_HSCROLL, &sbi);

	if (!(sbi.rgstate[0] & STATE_SYSTEM_INVISIBLE) && diff < 30) {
		diff ++;
		UpdateProgressList();
	}
}

void CAVIMux_GUIDlg::UpdateLanguage()
{
	m_Open_Files_Label.SetWindowText(LoadString(STR_MAIN_S_OPENFILES));
	m_Add_Video_Source.SetWindowText(LoadString(STR_MAIN_B_ADDVIDEOSOURCE));

	m_Subtitles_Label.SetWindowText(LoadString(STR_MAIN_S_SUBTITLES));
	m_Audio_Label.SetWindowText(LoadString(STR_MAIN_S_AUDIO));
//	m_Video_Headline.SetWindowText(LoadString(STR_MAIN_S_VIDEO));
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
	SendDlgItemMessage(ID_LEAVE,WM_SETTEXT,NULL,(LPARAM)LoadString(STR_MAIN_B_CANCEL));
	m_Progress_Group.SetWindowText(LoadString(STR_MAIN_S_PROGRESS));

	m_Progress_List.DeleteAllItems();
	for (int j=0;j<6;j++)
		m_Progress_List.DeleteColumn(0);
	UpdateProgressList();

	m_Prg_Dest_File_Label.SetWindowText(LoadString(STR_MAIN_S_PRG_DESTFILE));
	m_Prg_Frames_Label.SetWindowText(LoadString(STR_MAIN_S_PRG_FRAMES));
	m_Prg_Legidx_Label.SetWindowText(LoadString(STR_MAIN_S_PRG_LEGIDX));
	m_Prg_Progress_Label.SetWindowText(LoadString(STR_MAIN_S_PRG_PROGRESS));

	m_Stream_Lng_Label.SetWindowText(LoadString(STR_MAIN_S_AUDIONAME));
	m_Audiodelay_Label.SetWindowText(LoadString(STR_MAIN_S_AUDIODELAY));
	m_OutputResolution_Label.SetWindowText(LoadString(STR_MAIN_S_RESOLUTION));
	m_StreamTree.InvalidateRect(NULL);

	switch (iCurrentView) {
		case 1: SetDialogState_Config(); break;
		case 2: SetDialogState_Muxing(); break;
	}			
	switch (iButtonState) {
		case 2: ButtonState_START(); break;
		case 1: ButtonState_STOP(); break;
	}

	m_Chapter_Editor.SetWindowText(LoadString(STR_SFO_O_CHAPTERS));
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
	else {
		if (nID == SC_CLOSE && !bEditInProgess)
			OnCancel();
		else
			CResizeableDialog::OnSysCommand(nID, lParam);
	}
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
		CResizeableDialog::OnPaint();
	}
}

// Die Systemaufrufe fragen den Cursorform ab, die angezeigt werden soll, während der Benutzer
//  das zum Symbol verkleinerte Fenster mit der Maus zieht.
HCURSOR CAVIMux_GUIDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

#include "FILE_INFO.h"
#include ".\avimux_guidlg.h"

void CAVIMux_GUIDlg::AddFile(CFileDialog* cfd)
{
}



DWORD DetectFileType(STREAM* lpSource, void** pReturn = NULL)
{
	union
	{
		MP3SOURCE*			mp3source;
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
	if (FileIsXML(t)) {
		char* c = (char*) calloc(1, (size_t)lpSource->GetSize());
		Textfile2String(t, c);
		XMLNODE* pTree = NULL;
		if (xmlBuildTree(&pTree, c) == XMLERR_OK) {
			ZeroMemory(c, sizeof(c));
			if (pReturn) *pReturn = pTree;
			t->Close();
			delete t;
			delete c;
			return FILETYPE_XML;
		} else 
			delete c;
	} else {
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
		if (ac3source->Open(lpSource) != AS_ERR)
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
		if (dtssource->Open(lpSource) != AS_ERR)
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
		if (mp3source->Open(lpSource) != AS_ERR)
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
		if (aacsource->Open(lpSource) != AS_ERR)
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
	if (wavfile->Open(lpSource,FA_READ) != WAV_ERR)
	{
		wavfile->Close();
		delete wavfile;
		lpSource->SetOffset(0);
		return FILETYPE_WAV;
	}
	delete wavfile;

	return FILETYPE_UNKNOWN;
}


void  CAVIMux_GUIDlg::doAddFile(char* _lpcName, int iFormat, int delete_file,
								HANDLE hSemaphore)
{
	char*	lpcExt;
	int		i;
	int				iIndex1;
	FILESTREAM*		filesource;
	STREAM*			source;
	CACHE*			cachesource = NULL;

	MODE2FORM2SOURCE* m2f2source;
	char			VideoFormat[100];
	union
	{
		MP3SOURCE*			mp3source;
		WAVSOURCE*			wavsource;
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

	CString			cStr,cStr2;
	DWORD			dwUseCache;
	DWORD			dwFmtUpStr;
	DWORD			dwFmtUpStr3;
	SUBTITLE_STREAM_INFO*	lpssi;

	char			Buffer[200],Fmt[50];
	DWORD			dwLength=lstrlen(_lpcName);
	CBuffer*		cb;

	char lpcName[65536];
	if (!strncmp(_lpcName, "\\\\?\\UNC\\", 8)) {
		strcpy(lpcName, "\\\\");
		strcat(lpcName, _lpcName + 8);
	} else {
		if (!strncmp(_lpcName, "\\\\?\\", 4))
			strcpy(lpcName, _lpcName + 4);
		else
			strcpy(lpcName, _lpcName);
	}

	if (!strcmp(_lpcName, "*stdin*")) {
		FILE* f = fopen("temp_std_file", "wb");
		char c;
		char d = 13;
		while ((c = getchar()) != EOF) {
			if (c == 0x0A) 
				fwrite(&d, sizeof(d), 1, f);
			fwrite(&c, sizeof(c), 1, f);
		}
		fclose(f);
		doAddFile("temp_std_file");
		DeleteFile("temp_std_file");
	}

	bool			bUnbuffered = !!settings->GetInt("input/general/unbuffered");
	bool			bOverlapped = !!settings->GetInt("input/general/overlapped");

	DWORD			dwCacheOpenMode = CACHE_OPEN_READ | CACHE_OPEN_ATTACH;
	DWORD			dwFileOpenMode = STREAM_READ;
	if (bUnbuffered)
		dwFileOpenMode |= STREAM_UNBUFFERED;
	if (bOverlapped)
		dwFileOpenMode |= STREAM_OVERLAPPED;

	for (i=dwLength;(i>=0)&&(lpcName[i]!='.');i--);

	lpcExt=new char[dwLength-i+2];
	ZeroMemory(lpcExt,dwLength-i+2);
	lstrcpy(lpcExt,&(lpcName[i+1]));
	for (i=lstrlen(lpcExt)-1;i>=0;lpcExt[i--]|=((lpcExt[i]>=64)&&(lpcExt[i]<=90))?0x20:0);

		ZeroMemory(Buffer,sizeof(Buffer));
		dwUseCache=(settings->GetInt("input/general/use cache"))?1:0;
		cb = new CBuffer;
		cb->SetSize(sizeof(FILE_INFO));
		cb->IncRefCount();

		fi = (FILE_INFO*)cb->GetData();
		ZeroMemory(fi,sizeof(FILE_INFO));
//		fi->cache=NULL;
		fi->dwType=0;
		fi->file=NULL;
		fi->AVIFile=NULL;
		fi->bM2F2CRC=!!(int)settings->GetInt("input/m2f2/crc check");//(ofOptions.dwFlags&SOFO_M2F2_DOM2F2CRCCHECK);
		fi->bInUse=false;
		fi->lpwav=NULL;
		fi->lpM2F2=NULL;
		fi->bAddedImmediately=1;
		fi->bMP3VBF_forced=!!(int)settings->GetInt("input/avi/force mp3 vbr");//(ofOptions.dwFlags&SOFO_AVI_FORCEMP3VBR);
		filesource=new FILESTREAM;
		if (filesource->Open(lpcName,STREAM_READ)==STREAM_ERR) {
			DecBufferRefCount(&cb);
			return;
		}
		fi->file=filesource;
		fi->file_id = m_SourceFiles.GetCount();
		fi->current_pos = 2 * fi->file_id;

		m2f2source=new MODE2FORM2SOURCE;
		if (m2f2source->Open(filesource)==STREAM_OK) {
			/* don't open m2f2 files unbuffered */
			dwFileOpenMode &=~ (STREAM_OVERLAPPED | STREAM_UNBUFFERED);
			source=(STREAM*)m2f2source;
			fi->dwType|=FILETYPE_M2F2;
			fi->lpM2F2=m2f2source;
		} else {
			source=filesource;
			m2f2source->Close();
			delete m2f2source;
			fi->lpM2F2=NULL;
		}
		
		XMLNODE* xmlTree;
		fi->dwType|=(iFormat!=FILETYPE_UNKNOWN)?iFormat:DetectFileType(source, 
			(void**)&xmlTree);

		source->Close();
		delete source;

		filesource = new FILESTREAM;
		if (filesource->Open(lpcName, dwFileOpenMode) <= 0) {
			if ((dwFileOpenMode & STREAM_UNBUFFERED) ||
				(dwFileOpenMode & STREAM_OVERLAPPED)) {
				
				dwFileOpenMode &=~ STREAM_UNBUFFERED | STREAM_OVERLAPPED;
				MessageBox("Failed to open file in unbuffered and/or overlapped mode!",
					"Error", MB_OK | MB_ICONERROR);
			}
			if (filesource->Open(lpcName, dwFileOpenMode) <= 0)
				MessageBox("Weird: Could not open file again", "Error", MB_OK | MB_ICONERROR);
		};
		fi->file = filesource;
		if (fi->dwType & FILETYPE_M2F2) {
			m2f2source=new MODE2FORM2SOURCE;
			m2f2source->Open(filesource);
			source=(STREAM*)m2f2source;
			fi->lpM2F2=m2f2source;
		} else
			source = filesource;

		source->Seek(0);

		if (fi->dwType & FILETYPE_AVI)
		{
			fi->bAddedImmediately=0;
			if (dwUseCache)	{
				cachesource=new CACHE(8,1<<19);
				cachesource->Open((STREAM*)source, dwCacheOpenMode);
			} else {
				cachesource=(CACHE*)source;
			}
			AVIFile=new AVIFILEEX;
			AVIFile->SetDebugState(DS_DEACTIVATE);
			int ignore_large_chunks = (int)settings->GetInt("input/avi/large chunks/ignore");
			int repair_large_chunks = (int)settings->GetInt("input/avi/large chunks/repair");
			int repair_dx50 = (int)settings->GetInt("input/avi/repair DX50");
			AVIFile->SetMaxAllowedChunkSize(1024*(!!ignore_large_chunks)*ofOptions.dwIgnoreSize);
			AVIFile->TryToRepairLargeChunks(!!repair_large_chunks);
			if (AVIFile->Open(cachesource,FA_READ,AT_AUTODETECT)==AFE_OK)
			{
				if (repair_dx50) {
					if ((AVIFile->GetFormatTag(0)&0xffffdfdf)==MakeFourCC("DX50"))	{
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
					sprintf(VideoFormat,"%s","M-JPEG");
				}
				else
				if (dwFmtUpStr==MakeFourCC("HFYU")) {
					sprintf(VideoFormat,"%s","huff-YUV");
				} else
				if (dwFmtUpStr==MakeFourCC("XVID")) {
					sprintf(VideoFormat,"%s","XVID");
				} else
				if (AVIFile->GetFormatTag(0)==MakeFourCC("MJ2C")) {
					sprintf(VideoFormat,"%s","MorganMotion JPEG 2000");
				} else
					sprintf(VideoFormat,"%s","???");

				fi->AVIFile=AVIFile;
				AVIFile->SetProcessMode(SPM_SETALL,PM_DIRECTSTREAMCOPY);
				fi->dwType|=FILETYPE_AVI;
			// Infos anzeigen
				wsprintf(Fmt,"%s (%s%s): %s","AVI",(fi->dwType&FILETYPE_M2F2)?"M2F2, ":"",
					(AVIFile->GetAVIType()==AT_STANDARD)?"standard":"Open-DML",VideoFormat);
			}
			else
			{
				MessageBox (LoadString(STR_ERR_OPENAVIERROR));
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
				cachesource=new CACHE(4,1<<19);
				cachesource->Open(filesource, dwCacheOpenMode);
//				fi->cache=cachesource;
			}
			else
			{
				cachesource=(CACHE*)source;
			}
			sprintf(Fmt,"%s","MP3");
			fi->dwType|=FILETYPE_MP3;
		}
		else
		if (fi->dwType&FILETYPE_AC3)
		{
			// use input cache?
			if (dwUseCache)	{
				cachesource=new CACHE(4,1<<19);
				cachesource->Open(source, dwCacheOpenMode);
//				fi->cache=cachesource;
			} else {
				cachesource=(CACHE*)source;
			}
			sprintf(Fmt,"%s","AC3");
			fi->dwType|=FILETYPE_AC3;
		}
		else
		if (fi->dwType&FILETYPE_DTS)
		{
			// use input cache?
			if (dwUseCache)	{
				cachesource=new CACHE(4,1<<19);
				cachesource->Open(source, dwCacheOpenMode);
//				fi->cache=cachesource;
			} else {
				cachesource=(CACHE*)source;
			}
			sprintf(Fmt,"%s","DTS");
			fi->dwType|=FILETYPE_DTS;
		}
		else
		if (fi->dwType&FILETYPE_AAC)
		{
			// use input cache?
			if (dwUseCache)
			{
				cachesource=new CACHE(4,1<<19);
				cachesource->Open(source, dwCacheOpenMode);
//				fi->cache=cachesource;
			}
			else
			{
				cachesource=(CACHE*)source;
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
				cachesource=new CACHE(8,1<<16);
				cachesource->Open(source, dwCacheOpenMode);
//				fi->cache=cachesource;
			}
			else
			{
				cachesource=(CACHE*)source;
			}
			wsprintf(Fmt,"%s","OGG/Vorbis");
			fi->dwType|=FILETYPE_OGGVORBIS;
		}
		else
		if (fi->dwType&FILETYPE_WAV)
		{
			if (dwUseCache)
			{
				cachesource=new CACHE(4,1<<19);
				cachesource->Open(source, dwCacheOpenMode);
//				fi->cache=cachesource;
			}
			else
			{
				cachesource=(CACHE*)source;
			}
			wsprintf(Fmt,"%s","WAV");
			fi->dwType|=FILETYPE_WAV;
		}
		else
		if (fi->dwType&FILETYPE_SUBS)
		{
			if (dwUseCache)
			{
				cachesource=new CACHE(4,1<<19);
				cachesource->Open(source, dwCacheOpenMode);
//				fi->cache=cachesource;
			}
			else
			{
				cachesource=(CACHE*)source;
			}
			wsprintf(Fmt,"%s","SUB");
			fi->dwType|=FILETYPE_SUBS;
		}
		else
		if (fi->dwType&FILETYPE_MKV)
		{
			if (dwUseCache)
			{
				cachesource=new CACHE(8,1<<19);
				cachesource->Open(source, dwCacheOpenMode);
//				cachesource->Enable(CACHE_CREATE_LOG);
				cachesource->Enable(CACHE_CAN_GROW);
//				fi->cache=cachesource;
			}
			else
			{
				cachesource=(CACHE*)source;
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
		
		if (fi->dwType!=FILETYPE_SCRIPT) {
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
			__int64 dwSize = (cachesource)?cachesource->GetSize():fi->file->GetSize();
			FormatSize(cSize,dwSize);
		
			iIndex1=m_SourceFiles.AddString(lpcName);
			fi->lpcName=new char[lstrlen(lpcName)+16];
			lstrcpy(fi->lpcName,lpcName);
			fi->source=cachesource;
			m_SourceFiles.SetItemData(iIndex1,(LPARAM)cb);
			m_SourceFiles.InvalidateRect(NULL);
			m_SourceFiles.UpdateWindow();

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
							m_SourceFiles.DeleteString(iIndex1);
							return;
						}
						fi->MP3File = mp3source;

						if (bAddAS_immed) {
							DWORD dwCheckVBR;
							asi = new AUDIO_STREAM_INFO;
							ZeroMemory(asi,sizeof(AUDIO_STREAM_INFO));
							if (((ofOptions.dwFlags&SOFO_MP3_MASK)==SOFO_MP3_CHECKCBRALWAYS)||((ofOptions.dwFlags&SOFO_MP3_MASK)==SOFO_MP3_CHECKCBRASK))
							{
								if (mp3source->ScanForCBR(1000)) {
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
							asi->lpdwFiles[1]= fi->file_id;
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

						if (oggfile->GetNumberOfStreams() != 1) {
							fi->dwType = FILETYPE_UNKNOWN;
							oggfile->Close();
							delete oggfile;
							oggfile = NULL;

							MessageBoxUTF8(0, LoadString(STR_ERR_OMG, LOADSTRING_UTF8),
								LoadString(IDS_ERROR, LOADSTRING_UTF8), MB_OK | MB_ICONERROR);

						} else {
							fi->OGGFile = oggfile;

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
								asi->lpdwFiles[1]=fi->file_id;
								asi->dwFlags |= ASIF_ALLOCATED;
								fi->bInUse = 1;

								ZeroMemory(asi->lpASH, sizeof(AVIStreamHeader));
								fi->bAddedImmediately = 1;
							} else {
								fi->bAddedImmediately = 0;
								fi->VRBFile = vorbis;
							}
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
							m_SourceFiles.DeleteString(iIndex1);
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
								asi->lpdwFiles[1]=fi->file_id;
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
							m_SourceFiles.DeleteString(iIndex1);
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
							asi->lpdwFiles[1]=fi->file_id;
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
							m_SourceFiles.DeleteString(iIndex1);
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
							asi->lpdwFiles[1]=fi->file_id;
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
						wavsource = new WAVSOURCE();
						wavsource->Open(wavfile);
						asi->audiosource=wavsource;
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
						fi->bInUse = true;
						lpssi->lpsubs=subs;
						lpssi->lpash=new AVIStreamHeader;
						lpssi->lpash->fccType=MakeFourCC("txts");

						lpssi->lpdwFiles = new DWORD[2];
						lpssi->lpdwFiles[0]=1;
						lpssi->lpdwFiles[1]=fi->file_id;
						
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
					{
						int res = 0;

						if ((res = chapters->ImportFromXML(xmlTree)) == CHAP_IMPXML_OK) {
							m_SourceFiles.DeleteString(iIndex1);
						} else
						if (res == CHAP_IMPXML_NONUNIQUE_UID) {
							MessageBox(
								LoadString(STR_ERR_IMPCHAP_NONUNIQUEUID),
								LoadString(IDS_ERROR), MB_OK | MB_ICONERROR);
						} else if (res == CHAP_IMPXML_NO_CHAPTER_FILE) {
							settings->Import(xmlTree);
							Attribs(settings->GetAttr("gui/main_window"));
							ReinitFont(NULL);
							ReinitPosition();
							m_SourceFiles.DeleteString(iIndex1);
						}

						filesource->Close();
						delete filesource;
						fi->file = NULL;
						if (fi->lpcName)
							free(fi->lpcName);
						xmlDeleteNode(&xmlTree);
					}
					break;
			}
		}
		
		if (fi->dwType==FILETYPE_UNKNOWN) {
			m_SourceFiles.DeleteString(iIndex1);
			if (!LoadScript(_lpcName,m_hWnd,GetUserMessageID()))	{
				cStr=LoadString(IDS_FILETYPENOTSUPPORTED);
				sprintf(Buffer,cStr);
				MessageBox(Buffer,cstrError,MB_OK | MB_ICONERROR);
			} else {
				fi->file->Close();
				delete fi->file;
				fi->file = NULL;
				delete fi->lpcName;
				DecBufferRefCount(&cb);
			}
		} else {
			char cMsg[4096]; cMsg[0]=0;
			sprintf(cMsg, " loaded file: %s", lpcName);
			char u[4096]; u[0]=0;
			m_StatusLine.SetWindowText(cMsg);
			m_StatusLine.InvalidateRect(NULL);
			m_StatusLine.UpdateWindow();
		}
		if (asi && asi->audiosource) {
			char* e; char* f;
			splitpathname(lpcName,&f,&e,NULL);
			e=&f[strlen(f)-1];
			while (*e-- != '.');
			*(e+1)=0;
			
			asi->audiosource->SetName(f);
			ucase(f, f);
			char* cDelayPos = strstr(f,"DELAY");
			if (cDelayPos) cDelayPos+=6;
			char* cMSPos = strstr(f,"MS");
			if (cMSPos) *cMSPos=0;
			if (cDelayPos && cMSPos) SetDelay(asi,atoi(cDelayPos));

			AddAudioStream(asi);
		}
		

	free(lpcExt);
	if (m_SourceFiles.GetCount()) {
		m_Add_Video_Source.EnableWindow(1);
	}

//	MessageBox("done", "info", MB_OK);

}

void CAVIMux_GUIDlg::AddAudioStream(AUDIO_STREAM_INFO* asi)
{
	TREE_ITEM_INFO*	tii;
	HTREEITEM		hItem1, hItem2;

	if (asi) {
		tii = BuildTIIfromASI(asi);
		tii->iOrgPos = m_StreamTree.GetRootCount();
		tii->iCurrPos = 2 * m_StreamTree.GetRootCount();

		m_StreamTree.SetItemData(hItem1=Tree_InsertCheck(&m_StreamTree,(char*)LPSTR_TEXTCALLBACK),
			(DWORD)tii);

		tii = new TREE_ITEM_INFO;
		tii->iID = TIIID_STRNAME;
		tii->pText = new char[256];
		ZeroMemory(tii->pText,256);
		asi->audiosource->GetName(tii->pText);
		asi->iSize = asi->audiosource->GetSize();
		m_StreamTree.SetItemData(hItem2=Tree_InsertCheck(&m_StreamTree,(char*)LPSTR_TEXTCALLBACK,hItem1),
			(DWORD)tii);
		m_StreamTree.ShowItemCheckBox(hItem2, false);

		tii = new TREE_ITEM_INFO;
		tii->iID = TIIID_LNGCODE;
		tii->pText = new char[256];
		ZeroMemory(tii->pText,256);
		asi->audiosource->GetLanguageCode(tii->pText);
		m_StreamTree.SetItemData(hItem2=Tree_InsertCheck(&m_StreamTree,(char*)LPSTR_TEXTCALLBACK,hItem1),
			(DWORD)tii);
		m_StreamTree.ShowItemCheckBox(hItem2, false);

		asi->audiosource->Enable(0);
	}
}

void CAVIMux_GUIDlg::AddVideoStream(VIDEO_STREAM_INFO* vsi)
{
	TREE_ITEM_INFO*	tii;
	HTREEITEM		hItem1, hItem2;

	if (!vsi)
		return;

		tii = BuildTIIfromVSI(vsi);
		tii->iOrgPos = m_StreamTree.GetRootCount();
		tii->iCurrPos = 2 * m_StreamTree.GetRootCount();

		m_StreamTree.SetItemData(hItem1=Tree_InsertCheck(&m_StreamTree,
			(char*)LPSTR_TEXTCALLBACK),	(DWORD)tii);

		tii = new TREE_ITEM_INFO;
		tii->iID = TIIID_STRNAME;
		tii->pText = new char[256];
		ZeroMemory(tii->pText,256);
		vsi->videosource->GetName(tii->pText);
		vsi->iSize = vsi->videosource->GetSize();
		m_StreamTree.SetItemData(hItem2=Tree_InsertCheck(&m_StreamTree,
			(char*)LPSTR_TEXTCALLBACK,hItem1), (DWORD)tii);
		m_StreamTree.ShowItemCheckBox(hItem2, false);

		tii = new TREE_ITEM_INFO;
		tii->iID = TIIID_LNGCODE;
		tii->pText = new char[256];
		ZeroMemory(tii->pText,256);
		vsi->videosource->GetLanguageCode(tii->pText);
		m_StreamTree.SetItemData(hItem2=Tree_InsertCheck(&m_StreamTree,
			(char*)LPSTR_TEXTCALLBACK,hItem1), (DWORD)tii);
		m_StreamTree.ShowItemCheckBox(hItem2, false);

		vsi->videosource->Enable(0);
}


void CAVIMux_GUIDlg::AddSubtitleStream(SUBTITLE_STREAM_INFO* ssi)
{
	TREE_ITEM_INFO*	tii;
	HTREEITEM		hItem, hItem2;

	if (ssi) {
		tii = BuildTIIfromSSI(ssi);
		tii->iOrgPos = m_StreamTree.GetRootCount();
		tii->iCurrPos = 2 * m_StreamTree.GetRootCount();

		m_StreamTree.SetItemData(hItem=Tree_InsertCheck(&m_StreamTree,(char*)LPSTR_TEXTCALLBACK),
			(DWORD)tii);

		tii = new TREE_ITEM_INFO;
		tii->iID = TIIID_STRNAME;
		tii->pText = new char[256];
		ZeroMemory(tii->pText,256);
		ssi->lpsubs->GetName(tii->pText);
		m_StreamTree.SetItemData(hItem2=Tree_InsertCheck(&m_StreamTree,(char*)LPSTR_TEXTCALLBACK,hItem),
			(DWORD)tii);
		m_StreamTree.ShowItemCheckBox(hItem2, false);

		tii = new TREE_ITEM_INFO;
		tii->iID = TIIID_LNGCODE;
		tii->pText = new char[256];
		ZeroMemory(tii->pText,256);
		ssi->lpsubs->GetLanguageCode(tii->pText);
		
		m_StreamTree.SetItemData(hItem2=Tree_InsertCheck(&m_StreamTree,(char*)LPSTR_TEXTCALLBACK,hItem),
			(DWORD)tii);
		m_StreamTree.ShowItemCheckBox(hItem2, false);

	}
}

void CAVIMux_GUIDlg::OnAddFile() 
{
//	CFileDialog*	cfd;
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen

	char	c[1000];

	c[0] = 0;
	strcat(c,"all supported file types|*.avi;*.mk*;*.mp2;*.mp3;*.ac3;*.dts;*.aac;*.ogg;*.srt;*.ssa;*.amg;*.wav;*.xml|");
	strcat(c,"AVI files (*.avi)|*.avi|");
	strcat(c,"Matroska files (*.mk*)|*.mk*|");
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

	OPENFILENAME o;

	PrepareSimpleDialog(&o, m_hWnd, c);
	if (GetOpenSaveFileNameUTF8(&o, 1)) {
		doAddFile(o.lpstrFile);

		delete o.lpstrFile;
	}

}

void CAVIMux_GUIDlg::OnOK() 
{
	// TODO: Zusätzliche Prüfung hier einfügen
	CResizeableDialog::OnOK();
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

	HTREEITEM hItem = m_StreamTree.GetSelectedItem();
	if (hItem) {
		tii = m_StreamTree.GetItemInfo(hItem);
		m_AudioName.GetWindowText(cBuffer,sizeof(cBuffer));
		Str2UTF8(cBuffer,cBuffer);

		if (tii && tii->iID == TIIID_LNGCODE) {
			tii = m_StreamTree.GetItemInfo(m_StreamTree.GetParentItem(hItem));
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
			tii = m_StreamTree.GetItemInfo(m_StreamTree.GetParentItem(hItem));
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

	HTREEITEM hItem = m_StreamTree.GetSelectedItem();
	
	if (hItem) {
		tii = m_StreamTree.GetItemInfo(hItem);
		if (tii && tii->iID == TIIID_ASI) {
			asi = tii->pASI;
		} else {
			if (hItem = m_StreamTree.GetParentItem(hItem)) {
				tii = m_StreamTree.GetItemInfo(hItem);
				if (tii && tii->iID == TIIID_ASI) {
					asi = tii->pASI;
				}
			}
		}
		if (asi) {
			m_Audiodelay.GetWindowText(cBuffer,sizeof(cBuffer));
			SetDelay(asi,atoi(cBuffer));
		}
	}
}


void CAVIMux_GUIDlg::OnStart() 
{
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen
	CFileDialog*	cfd = NULL;
	DEST_AVI_INFO*	lpDAI;
	int				i;//,j;
	DWORD			dwAllAudio;
	DWORD			dwChunkOverhead;
	DWORD			dwDefaultAudio;
	DWORD			dwNoAudio;
	DWORD			dwEstimatedChunks;
	DWORD			dwIndexOverhead;
	DWORD			dwOvhUnit;
	DWORD			dwReclistOverhead;

	CDynIntArray*	audio_streams = NULL;
	CDynIntArray*	video_streams = NULL;
	CDynIntArray*	subtitles = NULL;

	DWORD			dwUseMaxSize=0;

	char			Buffer[200];
	CString			cStr,cStr2;
	char			cDrive[10];
	char			cDebugFile[200];
//	char			LongBuffer[1024];

	bool			bFileNumberNeglected;
	char			cBuffer[256];
	float			fPlaytime;
	RESOLUTION		resOutput;

	HTREEITEM		hItem;

	lpDAI=new DEST_AVI_INFO;
	ZeroMemory(lpDAI,sizeof(DEST_AVI_INFO));
	lpDAI->dwNbrOfVideoStreams = 1;

	ButtonState_START();
	ApplyStreamSettings();
	lpDAI->bDoneDlg = sfOptions.bDispDoneDlg;
	lpDAI->dlg=this;
	lpDAI->hDebugFile=INVALID_HANDLE_VALUE;
	lpDAI->lpProtocol=&m_Protocol;
	lpDAI->split_points = new CSplitPoints;
	lpDAI->settings = settings->Duplicate();

// Formatierung für Numerierung
	if (!lpDAI->settings->GetInt("output/general/numbering/enabled")) {
		lpDAI->lpFormat=new char[5+lstrlen("$Name")];
		lstrcpy(lpDAI->lpFormat,"$Name");
	} else {
		lpDAI->lpFormat=new char[(5+lstrlen(sfOptions.lpcNumbering))];
		lstrcpy(lpDAI->lpFormat,sfOptions.lpcNumbering);
	}

// maximale Dateizahl
	lpDAI->dwMaxFiles=(sfOptions.dwUseMaxFiles&&sfOptions.dwMaxFiles)?sfOptions.dwMaxFiles:(INT_MAX - 1);
// Audiointerleave
	if (!lpDAI->settings->GetInt("output/avi/audio interleave/value")) {
		lpDAI->settings->SetInt("output/avi/audio interleave/value", 1);
	}
// Video
	lpDAI->videosource = NULL;

	video_streams = m_StreamTree.GetItems(m_StreamTree.GetRootItem(), TIIID_VSI, 0);
	lpDAI->dwNbrOfVideoStreams = video_streams->GetCount();
	if (lpDAI->dwNbrOfVideoStreams) {
		TREE_ITEM_INFO* tii = (TREE_ITEM_INFO*)m_StreamTree.GetItemData((HTREEITEM)video_streams->At(0));
		lpDAI->videosource=tii->pVSI->videosource;
		lpDAI->videosource->Seek(0);
//		qwEstimatedSize+=lpDAI->videosource->GetSize();
		lpDAI->videosource->Enable(1);
	}
	
	lpDAI->i1stTimecode = sfOptions.i1stTimecode;

	if (sfOptions.dwUseManualSplitPoints) sfOptions.split_points->Duplicate(lpDAI->split_points);
	
	m_VideoStretchFactor.GetWindowText(cStr);
	lpDAI->dVideoStretchFactor = atof(cStr);

	char* cTitle = NULL;
	GetWindowTextUTF8(hTitleEdit, &cTitle);
	if (cTitle[0]) {
		lpDAI->cTitle = new CStringBuffer(cTitle, CSB_UTF8);
	}
	free(cTitle);

	lpDAI->chapters = chapters;

	if (lpDAI->dwNbrOfVideoStreams) {
		m_OutputResolution.GetWindowText(cStr);
		
		int in_x, in_y;
		lpDAI->videosource->GetResolution(&in_x, &in_y);
		lpDAI->videosource->GetOutputResolution(&resOutput);

		if (cStr.GetLength() == 0 || Str2Resolution(cStr.GetBuffer(256),in_x,in_y,&resOutput,&resOutput) == STRF_OK) {
			lpDAI->videosource->SetOutputResolution(&resOutput);
		}
	}

// maximale Größe in MB beachten?
//	dwUseMaxSize=sfOptions.dwUseMaxFileSize;
// maximale Dateigröße

	dwUseMaxSize = (DWORD)settings->GetInt("output/general/file size/limited");

	if (dwUseMaxSize)
	{
		__int64 max_file_size = settings->GetInt("output/general/file size/max");

//		lpDAI->qwMaxSize=sfOptions.dwMaxFileSize;//atoi(Buffer);
		if (!max_file_size)
		{
			if (!lpDAI->settings->GetInt("output/avi/opendml/on")) 
				lpDAI->settings->SetInt("output/general/file size/max", 2030); 
			else 
				lpDAI->settings->SetInt("output/general/file size/max", INT_MAX);
		}
	}	
	else
	{
		lpDAI->settings->SetInt("output/general/file size/max", INT_MAX);
	}

	__int64 maximum_file_size = lpDAI->settings->GetInt("output/general/file size/max");

// Padding
	lpDAI->dwPadding=2;
// maximale Frames
	lpDAI->dwMaxFrames=sfOptions.dwFrames;//atoi(Buffer);
	if (!(lpDAI->dwMaxFrames) && lpDAI->dwNbrOfVideoStreams) lpDAI->dwMaxFrames=lpDAI->videosource->GetNbrOfFrames();
	
	if (lpDAI->dwNbrOfVideoStreams) {
		int vf = (int)lpDAI->videosource->GetNbrOfFrames();
		if (vf == -1) vf = 0;

		lpDAI->dwMaxFrames=min(lpDAI->dwMaxFrames,vf);
		if (lpDAI->dwMaxFrames) {
			lpDAI->iDurationFlags = DAI_DF_FRAMES;
		} else {
			lpDAI->iMaxDuration = lpDAI->videosource->GetDuration();
			lpDAI->iDurationFlags = DAI_DF_DURATION;
		}
	}

// Preload
//	lpDAI->dwPreload=sfOptions.dwPreload;//atoi(Buffer);
// Legacy Index
	//lpDAI->avi.iLegacyIndex=((sfOptions.dwLegacyIndex)&&(lpDAI->avi.iOpenDML))?1:0;
	lpDAI->settings->SetInt("output/avi/legacyindex", 
		(lpDAI->settings->GetInt("output/avi/opendml/on") && lpDAI->settings->GetInt("output/avi/legacyindex")));
// Audiostreams
	dwNoAudio=(IsDlgButtonChecked(IDC_NO_AUDIO)==BST_CHECKED)?1:0;

	audio_streams = m_StreamTree.GetItems(m_StreamTree.GetRootItem(),TIIID_ASI,-1);

	for (i=0;i<audio_streams->GetCount();i++) {
		TREE_ITEM_INFO*	tii = m_StreamTree.GetItemInfo((HTREEITEM)audio_streams->At(i));
		tii->pASI->audiosource->Enable(0);
	}
	audio_streams->DeleteAll();
	delete audio_streams;
	if (dwNoAudio) {
		audio_streams = new CDynIntArray;
	} else {
		dwDefaultAudio=(IsDlgButtonChecked(IDC_DEFAULT_AUDIO)==BST_CHECKED)?1:0;
		if (dwDefaultAudio)
		{
			SendDlgItemMessage(IDC_DEFAULT_AUDIO_NUMBER,WM_GETTEXT,sizeof(Buffer),(LPARAM)Buffer);
			dwDefaultAudio=atoi(Buffer);
			hItem = m_StreamTree.GetRootItem();
			for (i=0;i<(int)dwDefaultAudio-1;i++) {
				hItem = m_StreamTree.GetNextSiblingItem(hItem);
			}

			audio_streams = new CDynIntArray;
			audio_streams->Insert((int)hItem);

		}
		else
		{
			dwAllAudio=(IsDlgButtonChecked(IDC_ALL_AUDIO)==BST_CHECKED)?1:0;
			if (!dwAllAudio) {
				audio_streams = m_StreamTree.GetItems(m_StreamTree.GetRootItem(),TIIID_ASI,0);
			} else {
				audio_streams = m_StreamTree.GetItems(m_StreamTree.GetRootItem(),TIIID_ASI,-1);
			}
		}
	}
	
	for (i=0;i<audio_streams->GetCount();i++) {
		TREE_ITEM_INFO*	tii = m_StreamTree.GetItemInfo((HTREEITEM)audio_streams->At(i));
		tii->pASI->audiosource->Enable(1);
	}

	lpDAI->dwNbrOfAudioStreams=audio_streams->GetCount();
	lpDAI->asi=new AUDIO_STREAM_INFO*[1+audio_streams->GetCount()];

// Geht nicht, wenn weder Video noch Audio verfügbar ist
	if (!lpDAI->dwNbrOfAudioStreams && !lpDAI->dwNbrOfVideoStreams) {
		ButtonState_STOP();
		return;
	}

	for (i=0;i<(int)lpDAI->dwNbrOfAudioStreams;i++)	{
		TREE_ITEM_INFO*	tii = m_StreamTree.GetItemInfo((HTREEITEM)audio_streams->At(i));
		lpDAI->asi[i]=tii->pASI;
	}

// Subtitles
	subtitles = new CDynIntArray;
	subtitles = m_StreamTree.GetItems(m_StreamTree.GetRootItem(),TIIID_SSI,-1);

	for (i=0;i<subtitles->GetCount();i++) {
		TREE_ITEM_INFO*	tii = m_StreamTree.GetItemInfo((HTREEITEM)subtitles->At(i));
		tii->pSSI->lpsubs->Enable(0);
	}
	subtitles->DeleteAll();
	delete subtitles;

	if (IsDlgButtonChecked(IDC_ALL_SUBTITLES)==BST_CHECKED) {
		subtitles = m_StreamTree.GetItems(m_StreamTree.GetRootItem(),TIIID_SSI,-1);
	} else
	if (IsDlgButtonChecked(IDC_NO_SUBTITLES)==BST_CHECKED) {
		subtitles = new CDynIntArray;
	} else {
		subtitles = m_StreamTree.GetItems(m_StreamTree.GetRootItem(),TIIID_SSI,1);
	}

	if (subtitles->GetCount())
	{
		lpDAI->ssi=new SUBTITLE_STREAM_INFO*[subtitles->GetCount()];
		for (i=0;i<subtitles->GetCount();i++)
		{
			HTREEITEM hItem = (HTREEITEM)subtitles->At(i);
			TREE_ITEM_INFO* tii = m_StreamTree.GetItemInfo(hItem);
			lpDAI->ssi[i]=tii->pSSI;
			wsprintf(Buffer,"Subtitle %d",i+1);
			if (!(lpDAI->ssi[i]->lpsubs->GetName(NULL)))
				lpDAI->ssi[i]->lpsubs->SetName(Buffer);
			
			lpDAI->ssi[i]->lpsubs->Enable(1);
		}
	}
	lpDAI->dwNbrOfSubs=subtitles->GetCount();

// StdAVI>2GB abfangen
	bool bOpenDML = !!lpDAI->settings->GetInt("output/avi/opendml/on");
	bool bRecLists = !!lpDAI->settings->GetInt("output/avi/reclists");
	bool bLegacy = !!lpDAI->settings->GetInt("output/avi/legacyindex");
	bool bLowOvhd = !!lpDAI->settings->GetInt("output/avi/opendml/haalimode");

// more than 9 streams with standard AVI?
/*	if (lpDAI->videosource && (!bOpenDML)&&(lpDAI->dwNbrOfAudioStreams+lpDAI->dwNbrOfSubs>=9))
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
*/
// Standard-Index
	if (settings->GetInt("output/avi/opendml/stdindex/pattern")==SIP_AUTO) {
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

// Guess overhead for AVI
	dwEstimatedChunks=lpDAI->dwMaxFrames;
	if (lpDAI->dwNbrOfVideoStreams) {
		fPlaytime=(float)lpDAI->dwMaxFrames*lpDAI->videosource->GetNanoSecPerFrame()/1000000000;
	} else {
		fPlaytime = 0;
	}
	dwIndexOverhead=NULL;
	DWORD chunks = 0;
/*	for (i=0;i<(int)lpDAI->dwNbrOfAudioStreams;i++)	{
		switch (lpDAI->asi[i]->dwType) {
			case AUDIOTYPE_MP3VBR:
				switch (lpDAI->asi[i]->audiosource->GetFrequency())	{
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
				chunks = (int)(chunks / settings->GetInt("output/avi/dts/frames per chunk"));
				dwEstimatedChunks += chunks; break;
			default:
				dwEstimatedChunks+=(DWORD)(fPlaytime/0.75); break;
		}
	}

	if (!bOpenDML && !bLegacy && bLowOvhd)*/ dwEstimatedChunks = 0;

	dwChunkOverhead=dwEstimatedChunks*8;
	dwReclistOverhead=0;

	dwOvhUnit=(bOpenDML)?8:16;
	dwIndexOverhead=dwEstimatedChunks*dwOvhUnit;
	CAttribs* ai = lpDAI->settings->GetAttr("output/avi/audio interleave");
	if (bRecLists) {
		switch ((int)ai->GetInt("unit"))
		{
			case AIU_KB:
//				dwEstimatedReclists=(DWORD)(qwEstimatedSize/1000/(int)ai->GetInt("value"));
				break;
			case AIU_FRAME:
//				dwEstimatedReclists=lpDAI->dwMaxFrames/(int)ai->GetInt("value");
				break;
		}
	}

	// mehrere Dateien, aber kein %d?
	bFileNumberNeglected=false;

	OPENFILENAME o;
	memset(&o, 0, sizeof(o));

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

		int	i = (int)settings->GetInt("output/general/prefered format");//sfOptions.iStdOutputFormat;
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

		PrepareSimpleDialog(&o, m_hWnd, c);
		o.lpstrDefExt = cStd[i];
		o.Flags &=~ OFN_OVERWRITEPROMPT;
	}

	if (cExtFilename || (GetOpenSaveFileNameUTF8(&o, 0))) {
	// Dateiname
		if (o.lpstrFilter)
			delete[] (char*)o.lpstrFilter;

		if (o.lpstrFile) {

			lpDAI->lpFileName = new char[1+strlen(o.lpstrFile)];
			strcpy(lpDAI->lpFileName, o.lpstrFile);
			sprintf(cDrive,"%c:\\", lpDAI->lpFileName[0]);
		}
		else
		{
			lpDAI->lpFileName=new char[1+lstrlen(cExtFilename)*3];
			wsprintf(cDrive,"%c:\\",cExtFilename[0]);
			Str2UTF8(cExtFilename, lpDAI->lpFileName);
		}

	// determine output file type: avi or mkv
		int iPos = strlen(lpDAI->lpFileName)-1;
		while (lpDAI->lpFileName[iPos-1]!='.') iPos--;
		char* cExt = &lpDAI->lpFileName[iPos];

		i = strlen(lpDAI->lpFormat)-1;
		while (i>=0 && lpDAI->lpFormat[i-1]!='.') i--;
		if (i<=0) {
			i = strlen(lpDAI->lpFormat)+1;
			strcat(lpDAI->lpFormat, ".");
		}


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
		}
		
		if (!stricmp(cExt,"mkv") || !stricmp(cExt,"mka")) {
			lpDAI->iOutputFormat = DOF_MKV;
			strcat(lpDAI->lpFormat,cExt);
		}

	// Namen ohne Erweiterung
		wsprintf(cDebugFile,"%s - debug-output.txt",lpDAI->lpFileName);
		lpDAI->bExitAfterwards=sfOptions.bExitAfterwards;
		
		if (lpDAI->dwNbrOfVideoStreams) lpDAI->videosource->Enable(1);
		SetDialogState_Muxing();
		doSaveConfig(lastjobfile);
		if (lpDAI->iOutputFormat == DOF_AVI) AfxBeginThread((AFX_THREADPROC)MuxThread_AVI,lpDAI);
		if (lpDAI->iOutputFormat == DOF_MKV) AfxBeginThread((AFX_THREADPROC)MuxThread_MKV,lpDAI);
		sfOptions.bDispDoneDlg=true;
		settings->SetInt("output/general/overwritedlg", 1);
		sfOptions.bExitAfterwards=false;

	} else {
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
	SUBTITLE_STREAM_INFO* ssi;
	VIDEO_STREAM_INFO* vsi;
	AUDIOSOURCELIST*		asl;
	TREE_ITEM_INFO*			tii;
	HTREEITEM			hItem,hChild;

	m_Add_Video_Source.EnableWindow(0);
	chapters->Delete();
//	delete chapters;

	iNbr=m_StreamTree.GetCount();
	hItem = m_StreamTree.GetRootItem();
	m_OutputResolution.SetWindowText("");
	
	SetWindowTextUTF8(hTitleEdit, "");
	while (hItem)
	{
		HTREEITEM hoc;

		hChild = m_StreamTree.GetChildItem(hItem);
		while (hChild) {
			tii=m_StreamTree.GetItemInfo(hChild);
			if (tii->iID == TIIID_ASI) asi = tii->pASI;
			if (tii->iID == TIIID_LNGCODE && tii->pText) delete tii->pText;
			if (tii->iID == TIIID_STRNAME && tii->pText) delete tii->pText;
			tii->pASI = NULL;
			delete tii;
			hoc = hChild;
			hChild = m_StreamTree.GetNextSiblingItem(hChild);
			m_StreamTree.DeleteItem(hoc);
		}

		tii=m_StreamTree.GetItemInfo(hItem);
		
		hoc = hItem;
		hItem = m_StreamTree.GetNextSiblingItem(hItem);
		m_StreamTree.DeleteItem(hoc);

		if (tii->iID == TIIID_ASI || tii->iID == TIIID_VSI || tii->iID == TIIID_SSI) {
			delete tii->pMSI->lpdwFiles;
		}

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
		if (tii->iID == TIIID_VSI) {
			vsi = tii->pVSI;
			vsi->videosource->Close(false);
			delete vsi->videosource;
			delete vsi;
			tii->pVSI = NULL;
		}


		delete tii;
	}

	iNbr=m_SourceFiles.GetCount();
	for (i=0;i<iNbr;i++)
	{
		CBuffer*	cb;

		cb = (CBuffer*)m_SourceFiles.GetItemData(i);
		fi=(FILE_INFO*)cb->GetData();
		if (fi->lpcName) free(fi->lpcName);
		fi->lpcName=NULL;
		if (fi->dwType & FILETYPE_AVI) {
			fi->AVIFile->Close(false);
			delete fi->AVIFile;
		}
		if (fi->dwType & FILETYPE_MKV) {
			fi->MKVFile->Close();
			delete fi->MKVFile;
		}
		if (fi->source) {
			fi->source->Close();
			delete fi->source;
		}

		DecBufferRefCount(&cb);
	}

//	m_VideoSources.ResetContent();
	m_SourceFiles.ResetContent();
	m_SourceFiles.AllowMoving(true);
	m_Audiodelay.SetWindowText("0");
	m_AudioName.SetWindowText("");
	sfOptions.split_points->DeleteAll();

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
	
	if (sfOptions.lpcNumbering) 
		free(sfOptions.lpcNumbering);
	sfOptions.lpcNumbering=NULL;

	for (int i=0;i<(int)dwLanguages;UnloadLanguageFile(lplpLanguages[i++]));
	delete lplpLanguages;

	m_Protocol.DeleteAllItems();
	m_StreamTree.DeleteAllItems();
	
	delete dwLangs;	
	delete chapters;
	delete sfOptions.split_points;
	CloseHandle(hGlobalMuxSemaphore);
	if (hLogFile) CloseHandle(hLogFile);

	CResizeableDialog::OnCancel();
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
	GetDlgItem(ID_LEAVE)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_OUTPUTOPTIONS)->ShowWindow(SW_SHOW);
	
	iButtonState = 1;
}

void CAVIMux_GUIDlg::ButtonState_START()
{
	GetDlgItem(IDC_START)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_STOP)->ShowWindow(SW_SHOW);
	GetDlgItem(ID_LEAVE)->ShowWindow(SW_HIDE);
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
	
	CResizeableDialog::OnLButtonUp(nFlags, point);
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

FILE* fopenutf8(char* filename, char* access, int unicode)
{
	FILE* file;
	if (unicode) {
		char* c = NULL;
		UTF82WStr(filename,&c);
		char mode[8]; mode[0]=0;
		UTF82WStr(access, mode, 8);
		file=_wfopen((const unsigned short*)c,(const unsigned short*)mode);
		free(c);
	} else {
		file = fopen(filename, access);
	}
	return file;
}

void CAVIMux_GUIDlg::SaveGUIConfig()
{
	if (!settings)
		return;

	FILE* file = fopenutf8(guifile, "w", iUnicode_possible);

	ASSERT(file);

	if (!file)
		return;

	char* txt_all = (char*)malloc(1<<20);
	ZeroMemory(txt_all, 1<<20);
	sprintf(txt_all,"%c%c%c%s%c%c%s%c%c",0xEF,0xBB,0xBF,"<?xml version=\"1.0\" encoding=\"utf-8\"?>",13,10,
		"<!DOCTYPE GUI SYSTEM \"avimuxguiconfig.dtd\">", 13, 10);

	XMLNODE* node = NULL;
	xmlAddSibling(&node, "gui", "", false);
	CAttribs* a = settings->GetAttr("gui");
	node->pChild = *a;

	xmlTreeToString(node, txt_all+strlen(txt_all), 1048000);

	xmlDeleteNode(&node);
	fwrite(txt_all, 1, strlen(txt_all), file);

	free(txt_all);
	fclose(file);
}

void CAVIMux_GUIDlg::doSaveConfig(char* lpcFile, int clear)
{
	FILE*			file;
	int				i,j, k;
	bool			bStore;
//	DWORD			dwNbrOfFiles;
	DWORD			dwNbrOfSelAS;
	DWORD			dwNbrOfAS;
//	CBuffer*		cb;
	FILE_INFO*		lpfi[2];
	DWORD			dwlpfic=0;
	DWORD*			lpdwSelectedFiles;
	CString			s;

//	VIDEO_STREAM_INFO*	lpvsi;
	AUDIO_STREAM_INFO*	lpasi;
	MULTIMEDIA_STREAM_INFO* lpmsi;
	TREE_ITEM_INFO*	tii;
	HTREEITEM		hItem;
	char			cBuffer[8192];
	bool			bWith;
	int*			files_added;

	CDynIntArray*	audio_stream_items, *audio_stream_indices;
	CDynIntArray*	subs_stream_items, *subs_stream_indices;
	ApplyStreamSettings();

	// Files
		file = fopenutf8(lpcFile, "w", iUnicode_possible);

		fprintf(file, cUTF8Hdr);
		if (clear) fprintf(file,"CLEAR\n");
		fprintf(file,"LANGUAGE %s\n",GetCurrentLanguage()->lpcName);
		fprintf(file,"SET INPUT OPTIONS\n");
		fprintf(file,"WITH SET OPTION\nUNBUFFERED %d\n", settings->GetInt("input/general/unbuffered"));
		fprintf(file,"OVERLAPPED %d\n", settings->GetInt("input/general/overlapped"));
		fprintf(file,"ADD_IMMED 0\n");

		ZeroMemory(lpfi,8);
		bWith=true;
		for (i=0;i<(int)m_SourceFiles.GetCount();i++)
		{
			lpfi[dwlpfic]=m_SourceFiles.GetFileInfo(m_SourceFiles.FileID2Index(i));
//			cb = (CBuffer*)SendDlgItemMessage(IDC_SOURCEFILELIST,LB_GETITEMDATA,i,0);
//			lpfi[dwlpfic]=(FILE_INFO*)cb->GetData();

			bStore=!lpfi[dwlpfic^1];
/*			if (!bStore) bStore=(!!lpfi[dwlpfic]->cache)!=(!!lpfi[dwlpfic^1]->cache);
			if (bStore)	{
				if (!bWith) {
					fprintf(file,"WITH SET OPTION\n"); bWith = true;
				}
				fprintf(file,"USE CACHE %d\n",!!lpfi[dwlpfic]->cache);
			}
*/
			bStore=!lpfi[dwlpfic^1];
			if (!bStore) bStore=(lpfi[dwlpfic]->bMP3VBF_forced)!=(lpfi[dwlpfic^1]->bMP3VBF_forced);
			if (bStore)	{
				if (!bWith) {
					fprintf(file,"WITH SET OPTION\n"); bWith = true;
				}
				fprintf(file,"AVI FORCE MP3VBR %d\n",lpfi[dwlpfic]->bMP3VBF_forced);
			}

			bStore=!lpfi[dwlpfic^1];
			if (!bStore) bStore=(lpfi[dwlpfic]->bAddedImmediately)!=(lpfi[dwlpfic^1]->bAddedImmediately);
			if (bStore)	{
				if (!bWith) {
					fprintf(file,"WITH SET OPTION\n"); bWith = true;
				}
			//	fprintf(file,"ADD_IMMED %d\n",!!lpfi[dwlpfic]->bAddedImmediately);
			}

			if (bWith) fprintf(file,"END WITH\n");
			bWith=false;
			fprintf(file,"LOAD ");
			fprintf(file,lpfi[dwlpfic]->lpcName);
			fprintf(file,"\n");
			dwlpfic^=1;
		}

		if (!bWith) {
			fprintf(file,"WITH SET OPTION\n"); bWith = true;
		}

		/* commands for fixing file order here */
		if (m_SourceFiles.GetCount()) {
			m_SourceFiles.RedoNumbering();
			fprintf(file, "FILEORDER %d", m_SourceFiles.GetCount());

			for (int i=0; i<m_SourceFiles.GetCount(); i++) 
				for (int j=0; j<m_SourceFiles.GetCount(); j++) 
					if (m_SourceFiles.GetFileInfo(j)->file_id == i)
						fprintf(file, " %d", j);
			
			fprintf(file, "\n");
		}

		fprintf(file,"ADD_IMMED %d\n", bAddAS_immed);
		fprintf(file,"USE CACHE %d\n",(settings->GetInt("input/general/use cache")));
		fprintf(file,"AVI FORCE MP3VBR %d\n",!!settings->GetInt("input/avi/force mp3 vbr"));//(ofOptions.dwFlags&SOFO_AVI_FORCEMP3VBR));

		switch (ofOptions.dwFlags&SOFO_MP3_MASK) {
			case SOFO_MP3_CHECKCBRASK: fprintf(file,"MP3 VERIFY CBR ASK\n"); break;
			case SOFO_MP3_CHECKCBRNEVER: fprintf(file,"MP3 VERIFY CBR NEVER\n"); break;
			case SOFO_MP3_CHECKCBRALWAYS: fprintf(file,"MP3 VERIFY CBR ALWAYS\n"); break;
		}

		fprintf(file,"MP3 VERIFY RESDLG %d\n",!!(ofOptions.dwFlags&SOFO_MP3_RESULTDLG));
		fprintf(file,"M2F2 CRC %d\n",!!settings->GetInt("input/m2f2/crc check"));//(ofOptions.dwFlags&SOFO_M2F2_DOM2F2CRCCHECK));
		fprintf(file,"AVI FIXDX50 %d\n",!!settings->GetInt("input/avi/repair DX50"));//(ofOptions.dwFlags&SOFO_AVI_REPAIRDX50));
		switch ((int)!!settings->GetInt("input/avi/large chunks/ignore")) {
			case 0: fprintf(file,"AVI IGNLCHUNKS OFF\n"); break;
			case 1: fprintf(file,"AVI IGNLCHUNKS ON\n");
				    fprintf(file,"AVI IGNLCHUNKS %d\n",ofOptions.dwIgnoreSize); break;
		}
		fprintf(file,"AVI TRY2FIXLCHUNKS %d\n",!!settings->GetInt("input/avi/large chunks/repair"));//(ofOptions.dwFlags&SOFO_AVI_TRYTOREPAIRLARGECHUNKS));

		fprintf(file,"WITH CHAPTERS\n");
		fprintf(file,"IMPORT %d\n",!!(ofOptions.dwFlags&SOFO_CH_IMPORT));
		fprintf(file,"FROMFILENAMES %d\n",!!(ofOptions.dwFlags&SOFO_CH_FROMFILENAMES));
		fprintf(file,"END WITH\n");

		if (bWith) fprintf(file,"END WITH\n");
		bWith=false;

		hItem = m_StreamTree.GetRootItem();

		files_added = new int[m_SourceFiles.GetCount()];
		memset(files_added, 0, sizeof(int) * m_SourceFiles.GetCount());

		audio_stream_items = m_StreamTree.GetItems(m_StreamTree.GetRootItem(),TIIID_ASI,1,&audio_stream_indices);
		subs_stream_items = m_StreamTree.GetItems(m_StreamTree.GetRootItem(),TIIID_SSI,1,&subs_stream_indices);
		
		dwNbrOfAS=m_StreamTree.GetCount();
		dwNbrOfSelAS=audio_stream_indices->GetCount();
		DWORD dwStreamCount = m_StreamTree.GetRootCount();
		for (i=0; i<(int)dwStreamCount; i++) {
			TREE_ITEM_INFO* tii = m_StreamTree.FindItemOriginalPosition(i);

			if (tii->iID != TIIID_SSI) {
				lpmsi=tii->pMSI;
				
				lpdwSelectedFiles=(DWORD*)lpmsi->lpdwFiles;
				if (lpdwSelectedFiles && lpdwSelectedFiles[0]) {
					fprintf(file,"DESELECT FILE 0\n");

					bool bSel = false;
					for (j=1;j<=(int)(lpdwSelectedFiles[0]);j++) {
						DWORD dwFileID = lpdwSelectedFiles[j];
						DWORD dwFileIndex = m_SourceFiles.FileID2Index(dwFileID);
						if (!files_added[dwFileIndex])
							for (k=1;k<=(int)lpdwSelectedFiles[0];k++) {
								if (lpdwSelectedFiles[k] == dwFileID) {
									bSel = true;
									fprintf(file,"SELECT FILE %d\n", dwFileIndex+1);//lpdwSelectedFiles[k]+1);
									files_added[dwFileIndex] = 1; 
								}
							}
					}
							
					if (bSel)
						fprintf(file,"ADD MMSOURCE\n");
				}
			}
			hItem = m_StreamTree.GetNextSiblingItem(hItem);
		}

		fprintf(file,"SET OUTPUT OPTIONS\n");
		fprintf(file,"WITH SET OPTION\n");
		fprintf(file,"UNBUFFERED %d\n",(int)settings->GetInt("output/general/unbuffered"));
		fprintf(file,"LOGFILE %d\n",(int)settings->GetInt("output/general/logfile/on"));
		fprintf(file,"STDOUTPUTFMT ");
		switch ((int)settings->GetInt("output/general/prefered format")) {
			case SOF_AVI: fprintf(file,"AVI\n"); break;
			case SOF_MKV: fprintf(file,"MKV\n"); break;
		}
		fprintf(file,"END WITH\n");

		delete files_added;
		fprintf(file,"DESELECT AUDIO 0\n");
		for (i=0;i<(int)dwNbrOfSelAS;i++)
		{
			tii = m_StreamTree.GetItemInfo((HTREEITEM)audio_stream_items->At(i));
			lpasi = tii->pASI;

			fprintf(file,"SELECT AUDIO %d\n",audio_stream_indices->At(i)+1);
		}
		bWith=false;

		hItem = m_StreamTree.GetRootItem();
		i=0; j=0;
		
		fprintf(file,"WITH SET OPTION\n"); bWith = true;
		int iCount = m_StreamTree.GetRootCount();

		if (iCount) {
			fprintf(file, "STREAMORDER %d", iCount);
			
			for (j=0; j<iCount; j++) {
				hItem = m_StreamTree.GetRootItem();
				for (i=0; i<iCount;i++) {
					tii = m_StreamTree.GetItemInfo(hItem);
					if (tii->iOrgPos == j) 
						fprintf(file, " %d",tii->iCurrPos >> 1);
					hItem = m_StreamTree.GetNextSiblingItem(hItem);
				}
			}
			fprintf(file, "\n");
		}

		hItem = m_StreamTree.GetRootItem();
		i=0; j=0; k=0;
		while (hItem)
		{
			tii = m_StreamTree.GetItemInfo(hItem);
			if (tii && tii->iID == TIIID_ASI) {
				lpasi=tii->pASI;
				if (lpasi->iDelay)
					fprintf(file,"DELAY %d %d\n",i+1,lpasi->iDelay);
				
				lpasi->audiosource->GetName(cBuffer);
				if (lstrlen(cBuffer))
					fprintf(file,"AUDIO NAME %d %s\n",i+1,cBuffer);
				
				lpasi->audiosource->GetLanguageCode(cBuffer);
				if (lstrlen(cBuffer))
					fprintf(file,"AUDIO LNGCODE %d %s\n",i+1,cBuffer);
				
				if (lpasi->audiosource->FormatSpecific(MMSGFS_AAC_ISSBR))
					fprintf(file,"AUDIO SBR %d %d\n",i+1,1);
				
				fprintf(file,"AUDIO DEFAULT %d %d\n",i+1,lpasi->audiosource->IsDefault());

				i++;
			} else
			if (tii && tii->iID == TIIID_SSI) {

				tii->pSSI->lpsubs->GetName(cBuffer);
				fprintf(file,"SUBTITLE NAME %d %s\n",k+1,cBuffer);
				
				tii->pSSI->lpsubs->GetLanguageCode(cBuffer);
				if (cBuffer && strlen(cBuffer)) {
					fprintf(file,"SUBTITLE LNGCODE %d %s\n",k+1,cBuffer);
				}
				fprintf(file,"SUBTITLE DEFAULT %d %d\n",k+1,tii->pSSI->lpsubs->IsDefault());

				k++;
			}
			if (tii && tii->iID == TIIID_VSI) {

				tii->pMSI->mms->GetName(cBuffer);
				fprintf(file,"VIDEO NAME %d %s\n",j+1,cBuffer);
				
				tii->pMSI->mms->GetLanguageCode(cBuffer);
				if (cBuffer && strlen(cBuffer)) {
					fprintf(file,"VIDEO LNGCODE %d %s\n",j+1,cBuffer);
				}
				fprintf(file,"VIDEO DEFAULT %d %d\n",j+1,tii->pMSI->mms->IsDefault());

				j++;
			}
			iCount++;
			
			hItem = m_StreamTree.GetNextSiblingItem(hItem);
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
		fprintf(file,"PRELOAD %d\n",settings->GetInt("output/avi/audio preload"));//sfOptions.dwPreload);
		fprintf(file,"MP3 CBR FRAMEMODE %d\n",(int)settings->GetInt("output/avi/mp3/cbr frame mode"));
		fprintf(file,"MAXFILESIZE %s\n",(settings->GetInt("output/general/file size/limited"))?"ON":"OFF");
		fprintf(file,"AVI AC3FPC %d\n",settings->GetInt("output/avi/ac3/frames per chunk"));
		fprintf(file,"AVI MP3FPC %d\n",settings->GetInt("output/avi/mp3/frames per chunk"));
		fprintf(file,"AVI DTSFPC %d\n",settings->GetInt("output/avi/dts/frames per chunk"));
		fprintf(file,"AVI ADDJUNKBEFOREHEADERS %d\n",settings->GetInt("output/avi/move hdrl"));
		fprintf(file,"OGG PAGESIZE %d\n",settings->GetInt("output/ogg/pagesize"));
		fprintf(file,"AVI RIFFAVISIZE %d\n", (int)settings->GetInt("output/avi/opendml/riff avi size"));
		fprintf(file,"AVI HAALIMODE %d\n", (int)settings->GetInt("output/avi/opendml/haalimode"));
		fprintf(file,"OVERLAPPED %d\n",(int)settings->GetInt("output/general/overlapped"));
	//	if (settings->GetInt("output/general/file size/limited"))
		fprintf(file,"MAXFILESIZE %d\n",settings->GetInt("output/general/file size/max"));// sfOptions.dwMaxFileSize);
		
		fprintf(file,"MAXFILES %s\n",(sfOptions.dwUseMaxFiles)?"ON":"OFF");
		if (sfOptions.dwUseMaxFiles)
		{
			fprintf(file,"MAXFILES %d\n",sfOptions.dwMaxFiles);
		}
		fprintf(file,"NUMBERING %s\n",(settings->GetInt("output/general/numbering/enabled"))?"ON":"OFF");
//		if (!sfOptions.dwDontUseNumbering)
		fprintf(file,"NUMBERING %s\n",sfOptions.lpcNumbering);

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
		
		char* cTitle = NULL;
		GetWindowTextUTF8(hTitleEdit, &cTitle);
		if (cTitle[0])
			fprintf(file,"TITLE %s\n",cTitle);
		free(cTitle);
		
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
		fprintf(file,"HARDLINKING %d\n",settings->GetInt("output/mkv/hard linking"));
		fprintf(file,"NONCLUSTERINDEXMODE %d\n", settings->GetInt("output/mkv/headers/index in first seekhead"));
		fprintf(file,"HEADERSIZE %d\n", settings->GetInt("output/mkv/headers/size"));
		fprintf(file,"HEADERSTRIPPING %d\n", settings->GetInt("output/mkv/compression/header striping"));
		fprintf(file,"USE_A_AAC %d\n", settings->GetInt("output/mkv/use a_aac"));


		fprintf(file,"PREVCLUSTERSIZE %d\n",(int)settings->GetInt("output/mkv/clusters/prevclustersize"));
		fprintf(file,"CLUSTERPOSITION %d\n",(int)settings->GetInt("output/mkv/clusters/position"));
		fprintf(file,"LIMIT1STCLUSTER %d\n",(int)settings->GetInt("output/mkv/clusters/limit first"));

		fprintf(file,"DISPWH %d\n",(int)settings->GetInt("output/mkv/displaywidth_height"));
		fprintf(file,"CUES %d\n",settings->GetInt("output/mkv/cues/on"));
		fprintf(file,"CUES VIDEO %d\n",settings->GetInt("output/mkv/cues/video/on"));
		fprintf(file,"CUES AUDIO %d\n",settings->GetInt("output/mkv/cues/audio/on"));
		fprintf(file,"CUES SUBS %d\n",settings->GetInt("output/mkv/cues/subs/on"));
		fprintf(file,"CUES AUTOSIZE %d\n",settings->GetInt("output/mkv/cues/autosize"));
		fprintf(file,"CUES MINIMUMINTERVAL %d\n",settings->GetInt("output/mkv/cues/minimum interval"));
		fprintf(file,"CUES SIZERATIO %d\n",settings->GetInt("output/mkv/cues/size ratio"));
		fprintf(file,"CUES TARGETSIZERATIO %d\n", settings->GetInt("output/mkv/cues/target size ratio"));
		fprintf(file,"CUES WRITEBLOCKNUMBER %d\n", settings->GetInt("output/mkv/cues/write blocknumber"));


		fprintf(file,"CUES AUDIO ONLY_AUDIO_ONLY %d\n",settings->GetInt("output/mkv/cues/audio/only audio-only/on"));
		fprintf(file,"TIMECODESCALE MKA %d\n",settings->GetInt("output/mkv/TimecodeScale/mka"));
		fprintf(file,"TIMECODESCALE MKV %d\n",settings->GetInt("output/mkv/TimecodeScale/mkv"));
		fprintf(file,"FORCE_V1 %d\n",settings->GetInt("output/mkv/force v1"));
		fprintf(file,"FORCE_V2 %d\n",settings->GetInt("output/mkv/force v2"));
		fprintf(file,"FLOAT_WIDTH %d\n",(int)settings->GetInt("output/mkv/floats/width"));
		fprintf(file,"RANDOMIZE_ORDER %d\n",(int)settings->GetInt("output/mkv/randomize element order"));

		fprintf(file,"2ND_COPY_OF_TRACKS %d\n",settings->GetInt("output/mkv/2nd Tracks"));
		fprintf(file,"END WITH\n");
	// chapters
		
		if (chapters->GetChapterCount()) {
		/*	fprintf(file,"WITH SET OPTION CHAPTERS\nCLEAR\n");
			RenderChapters2File(file,sfOptions.chapters);
			fprintf(file,"END WITH\n");*/
			fprintf(file,"SET OPTION CHAPTERS CLEAR\n");

			XMLNODE* xml = NULL;

			chapters->CreateXMLTree(&xml, NULL, NULL);
			char* txt_all = (char*)malloc(1<<22);
			ZeroMemory(txt_all, 1<<22);
			sprintf(txt_all,"%c%c%c%s%c%c%s%c%c",0xEF,0xBB,0xBF,"<?xml version=\"1.0\" encoding=\"utf-8\"?>",13,10,
				"<!DOCTYPE Segment SYSTEM \"matroskasegment.dtd\">", 13, 10);

			xmlTreeToString((XMLNODE*)xml, txt_all+strlen(txt_all), 4*1048000);
			char lpcChFile[65536];
			strcpy(lpcChFile, lpcFile);
            strcat(lpcChFile, ".chapters.xml");
			FILE* f = fopenutf8(lpcChFile, "wb", iUnicode_possible);
			if (!f) {
				strcpy(lpcChFile, lpcFile);
				strcat(lpcChFile, ".chapters.");
				char c[16]; for (int j=0;j<8;j++) c[j]=rand()%26+'A'; c[8]=0;
				strcat(lpcChFile, c);
				strcat(lpcChFile, ".xml");
				if (!(f = fopenutf8(lpcChFile, "wb", iUnicode_possible))) {
					/* b0rked */
				}
			}
			fprintf(f, txt_all);
			fclose(f);

			char* e = NULL;
			char* fn = NULL;
		//	char p[65536]; memset(p, 0, sizeof(p));
			splitpathname(lpcChFile, &fn, &e, NULL);//(char**)&p);
			fprintf(file, "LOAD %s\n", fn);
			xmlDeleteNode(&xml);

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
	if (cfd->DoModal()==IDOK) {
		char u[4096]; u[0]=0;
		Str2UTF8(cfd->GetPathName().GetBuffer(256), u);
		doSaveConfig(u);
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
	if (cfd->DoModal()==IDOK) {
		char u[4096]; u[0]=0;
		Str2UTF8(cfd->GetPathName().GetBuffer(255), u);
		ZeroMemory(&conf,sizeof(conf));
		file=new FILESTREAM;
		file->Open(u,STREAM_READ);
		file->Read(&conf.dwID,4);
		file->Read(&conf.dwSize,4);
		file->Read(&conf.dwVersion,conf.dwSize);
		file->Read(&dwX,4);

		sfOptions.dwUseManualSplitPoints=conf.dwUseSplitList;
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

	if (HIWORD(wParam) == EN_CHANGE) {

		if (HWND(lParam) == hTitleEdit) {
			int was_key_down = ::SendMessage(hTitleEdit, WM_WASKEYDOWN, 0, 0);
			if (was_key_down) {
				auto_apply_file_title = 0;

				char* c = NULL;
				GetWindowTextUTF8(hTitleEdit, &c);
				if (!c || !c[0])
					auto_apply_file_title = 1;
			}
		}
		
		return CResizeableDialog::OnCommand(wParam, lParam);
	} else switch (LOWORD(wParam)) {
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
			for (i=0;i<16;i++) 
				if (dwLangs[i]==LOWORD(wParam)) {
					SetCurrentLanguage(lplpLanguages[i]);
					current_language_index = i;
				}

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
	return CResizeableDialog::OnCommand(wParam, lParam);
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
//		sfOptions.cdMain=this;
		csfod->SetData(&sfOptions, &ofOptions, settings);
		csfod->Attribs(settings->GetAttr("gui/settings_window"));
		if (csfod->DoModal() == IDOK)
			csfod->GetData(&sfOptions,&ofOptions, &settings);
		
		delete csfod;
	}

}

void CAVIMux_GUIDlg::OnNoAudio() 
{
	if (IsDlgButtonChecked(IDC_NO_AUDIO)) {
		CheckDlgButton(IDC_ALL_AUDIO,BST_UNCHECKED);
		CheckDlgButton(IDC_DEFAULT_AUDIO,BST_UNCHECKED);
	}
}

void CAVIMux_GUIDlg::OnAllAudio() 
{
	if (IsDlgButtonChecked(IDC_ALL_AUDIO)) {
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
		cmLanguages->AppendMenu(MF_STRING |
		(i == current_language_index?MF_CHECKED:MF_UNCHECKED),dwLangs[i],lplpLanguages[i]->lpcName);
	

	cStr=LoadString(STR_MAIN_B_SAVECONFIG);
	cmPopupMenu->AppendMenu(MF_STRING,IDM_SAVECONFIG,cStr);
	cmPopupMenu->AppendMenu(MF_SEPARATOR,0);

	cStr=LoadString(IDS_LANGUAGE);
	cmPopupMenu->AppendMenu(MF_POPUP,(UINT)(cmLanguages->m_hMenu),LoadString(IDS_LANGUAGE));
	bShowMenu=true;

	switch (iCurrentView) {
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

	if (bShowMenu) {
		ClientToScreen(&point);
		cmPopupMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON,point.x,point.y,this);
	}
	if (cmPopupMenu) delete cmPopupMenu;
	if (cmLanguages) delete cmLanguages;
			
	CResizeableDialog::OnRButtonUp(nFlags, point);
}

BOOL CAboutDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Zusätzliche Initialisierung hier einfügen
	
	SetWindowText(LoadString(STR_AB_TITLE));
	char d[256];
	char version[32];
	GetAMGVersionString(version, sizeof(version));
	sprintf(d, "%s, %s", version, GetAMGVersionDate());
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


#define MAP_INTEGER(id,a) \
if (!strcmp(w, id)) { \
	w=getword(&cParam); \
	if (isposint(w)) \
		settings->SetInt(a, atoi(w)); \
	else bError=true;\
}
#define MAP_FLAG(id, a) \
if (!strcmp(w, id)) { \
	w=getword(&cParam); \
	if (isposint(w)) \
		settings->SetInt(a, !!atoi(w)); \
	else bError=true;\
}

LRESULT CAVIMux_GUIDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	char* c;
	int i,j;
	TREE_ITEM_INFO*		tii;
	AUDIO_STREAM_INFO*	lpasi;
	SUBTITLE_STREAM_INFO* lpssi;
	MULTIMEDIA_STREAM_INFO* lpmsi;
	char*	cParam=(char*)lParam;
	char	cBuffer[256];
	char*	w;
	char*					entire_line = NULL;
	bool					bError=false;
	bool					B[4];

	// TODO: Speziellen Code hier einfügen und/oder Basisklasse aufrufen
	ZeroMemory(B,sizeof(B));

	if (message == WM_NOTIFY) {
		int	idCtrl = (int) wParam; 
		NMHDR* pnmh = (LPNMHDR) lParam; 

		if (idCtrl == IDC_AUDIOTREE) {
			NMTVDISPINFO* lptvdi = (LPNMTVDISPINFO)lParam;
			int is_unicode = SendDlgItemMessage(IDC_AUDIOTREE, TVM_GETUNICODEFORMAT);
			unsigned int imsg = (is_unicode)?TVN_GETDISPINFOW:TVN_GETDISPINFOA;
			if (lptvdi->hdr.code == imsg) {
				OnGetAudiodispinfo((NMHDR*)lptvdi, (long*)&i);
			} else
				return CResizeableDialog::WindowProc(message, wParam, lParam);
		} else {
			// Sleep(1);
		}
	} else if (message == WM_KEYDOWN) {
		Sleep(1);
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
						current_language_index = i;
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
					Tree_SetCheckState(&m_StreamTree,Tree_Index2Item(&m_StreamTree,lParam,TIIID_ASI),true);
				} else {
					for (i=0;i<(int)m_StreamTree.GetCount();i++) {
						Tree_SetCheckState(&m_StreamTree,Tree_Index2Item(&m_StreamTree,i,TIIID_ASI),true);
					}
				}
				break;
			case IDM_SELVIDEO:
				if (lParam!=-1) {
					Tree_SetCheckState(&m_StreamTree,Tree_Index2Item(&m_StreamTree,lParam,TIIID_VSI),true);
				} else {
					for (i=0;i<(int)m_StreamTree.GetCount();i++) {
						Tree_SetCheckState(&m_StreamTree,Tree_Index2Item(&m_StreamTree,i,TIIID_VSI),true);
					}
				}
				break;
			case IDM_SELFILE:
				SendDlgItemMessage(IDC_SOURCEFILELIST,LB_SETSEL,1,lParam);
				break;
			case IDM_DESELAUDIO:
				if (lParam!=-1) {
					Tree_SetCheckState(&m_StreamTree,Tree_Index2Item(&m_StreamTree,lParam,TIIID_ASI),false);
				} else {
					for (i=0;i<(int)m_StreamTree.GetCount();i++) {
						Tree_SetCheckState(&m_StreamTree,Tree_Index2Item(&m_StreamTree,i,TIIID_ASI),false);
					}
				}
				break;
			case IDM_SELSUB:
				if (lParam!=-1) {
					Tree_SetCheckState(&m_StreamTree,Tree_Index2Item(&m_StreamTree,lParam,TIIID_SSI),true);
				} else {
					for (i=0;i<(int)m_StreamTree.GetCount();i++) {
						Tree_SetCheckState(&m_StreamTree,Tree_Index2Item(&m_StreamTree,i,TIIID_SSI),true);
					}
				}
				break;
			case IDM_DESELSUB:
				if (lParam!=-1) {
					Tree_SetCheckState(&m_StreamTree,Tree_Index2Item(&m_StreamTree,lParam,TIIID_SSI),false);
				} else {
					for (i=0;i<(int)m_StreamTree.GetCount();i++) {
						Tree_SetCheckState(&m_StreamTree,Tree_Index2Item(&m_StreamTree,i,TIIID_SSI),false);
					}
				}
				break;
			case IDM_DESELVIDEO:
				if (lParam!=-1) {
					Tree_SetCheckState(&m_StreamTree,Tree_Index2Item(&m_StreamTree,lParam,TIIID_VSI),false);
				} else {
					for (i=0;i<(int)m_StreamTree.GetCount();i++) {
						Tree_SetCheckState(&m_StreamTree,Tree_Index2Item(&m_StreamTree,i,TIIID_VSI),false);
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
				MAP_FLAG("OVERWRITEDLG", "output/general/overwritedlg") else
/*				if (!strcmp(w,"OVERWRITEDLG")) {
					settings->SetInt("output/general/overwritedlg", !!atoi(cParam));
					//sfOptions.bDispOverwriteDlg=!!atoi(cParam);
				}
				else*/
				MAP_FLAG("OPENDML", "output/avi/opendml/on") else
				MAP_FLAG("LEGACY", "output/avi/legacyindex") else
				MAP_FLAG("RECLISTS", "output/avi/reclists") else
				MAP_INTEGER("PRELOAD", "output/avi/audio preload") else
				MAP_FLAG("LOGFILE", "output/general/logfile/on") else
				if (!strcmp(w,"MAXFILESIZE")) {
					w=getword(&cParam);
					if (!strcmp(w,"OFF")) {
						settings->SetInt("output/general/file size/limited", 0);
					}
					else
					if (!strcmp(w,"ON")) {
						settings->SetInt("output/general/file size/limited", 1);
					}
					else
					if (isint(w)) {
						settings->SetInt("output/general/file size/max", atoi(w));//sfOptions.dwMaxFileSize=atoi(w);
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

						tii = m_StreamTree.GetItemInfo(Tree_Index2Item(&m_StreamTree,j));
						lpasi = tii->pASI;

						w=getword(&cParam);
						if (isint(w)) {
							SetDelay(lpasi,atoi(w));
							m_StreamTree.UpdateWindow();
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
						tii = m_StreamTree.GetItemInfo(Tree_Index2Item(&m_StreamTree,j,TIIID_ASI,&j));
						if (tii) lpasi = tii->pASI; else lpasi=NULL;

						if (B[0] || B[1]) {
							m_StreamTree.FindID(Tree_Index2Item(&m_StreamTree,j),B[0]?TIIID_STRNAME:TIIID_LNGCODE,&tii);
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
						m_StreamTree.Invalidate();
						m_StreamTree.UpdateWindow();
					} 
					else bError=true;
				}
				else
				if (!strcmp(w,"NUMBERING"))
				{
					w=cParam;
					if (!strcmp(w,"OFF")) {
						settings->SetInt("output/general/numbering/enabled", 0);
					}
					else
					if (!strcmp(w,"ON")) {
						settings->SetInt("output/general/numbering/enabled", 1);
					}
					else
					{
						if (sfOptions.lpcNumbering)	{
							free(sfOptions.lpcNumbering);
						}
						char cBuffer[256]; cBuffer[0]=0;
						if (strstr(w, "%d") || strstr(w, "%s"))
							strcpy(cBuffer, "$Name ($Nbr)");
						else 
							strcpy(cBuffer, w);

						sfOptions.lpcNumbering=new char[1+lstrlen(cBuffer)];
						lstrcpy(sfOptions.lpcNumbering,cBuffer);
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
						tii = m_StreamTree.GetItemInfo(Tree_Index2Item(&m_StreamTree,j,TIIID_SSI,&j));
						lpssi = tii->pSSI;

						
						if (B[0] || B[1]) {
							m_StreamTree.FindID(Tree_Index2Item(&m_StreamTree,j),B[0]?TIIID_STRNAME:TIIID_LNGCODE,&tii);
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
						m_StreamTree.Invalidate();
						m_StreamTree.UpdateWindow();
					} else
						bError=true;
			
				} 
				else
				if (!strcmp(w,"VIDEO")) {
					w=getword(&cParam);
					if ((B[0]=!strcmp(w,"NAME")) || (B[1]=!strcmp(w,"LNGCODE")) || (B[2]=!strcmp(w,"DEFAULT")))	{ // VIDEO
						w=getword(&cParam);
						j=atoi(w)-1;
						tii = m_StreamTree.GetItemInfo(Tree_Index2Item(&m_StreamTree,j,TIIID_VSI,&j));
						lpmsi = tii->pMSI;

						
						if (B[0] || B[1]) {
							m_StreamTree.FindID(Tree_Index2Item(&m_StreamTree,j),B[0]?TIIID_STRNAME:TIIID_LNGCODE,&tii);
							if (tii->pText) delete tii->pText;
							tii->pText = new char[1024];
							strcpy(tii->pText,cParam);
						}

						if (B[0]) {
							lpmsi->mms->SetName(cParam);
						} else 
						if (B[1]) {
							lpmsi->mms->SetLanguageCode(cParam);
						} else 
						if (B[2]) {
							w=cParam;
							if (isposint(w)) {
								lpmsi->mms->SetDefault(atoi(w));
							}
						}
						m_StreamTree.Invalidate();
						m_StreamTree.UpdateWindow();
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
						HTREEITEM hItem = m_StreamTree.GetRootItem();
						while (i < item_count) {
							TREE_ITEM_INFO* tii = m_StreamTree.GetItemInfo(hItem);
							if (tii) {
								tii->iCurrPos = 2*index[i++];
							}
							hItem = m_StreamTree.GetNextSiblingItem(hItem);
						}
						delete index;
						m_StreamTree.Sort();

					} else
						bError = true;
						
				} else


				if (!strcmp(w,"MKV")) {
					w=getword(&cParam);
					if (!strcmp(w,"LACE")) {
						w=getword(&cParam);
						if (isposint(w)) {
							settings->SetInt("output/mkv/lacing/style", max(0,min(3,atoi(w))));
						} else bError = true;
					} else
					if (!strcmp(w,"LACESIZE")) {
						w=getword(&cParam);
						if (isposint(w)) {
							settings->SetInt("output/mkv/lacing/length", atoi(w));
							SetLaceSetting("general",settings,1,atoi(w));
						} else {
							char* cFormat = w;
							char* cUse = getword(&cParam);
							char* cLength = getword(&cParam);
							SetLaceSetting(cFormat,settings,atoi(cUse),atoi(cLength));
						}
					} else
					MAP_INTEGER	("CLUSTERSIZE", "output/mkv/clusters/size") else
					MAP_INTEGER	("CLUSTERTIME", "output/mkv/clusters/time") else
					MAP_INTEGER ("HARDLINKING", "output/mkv/hard linking") else
					MAP_INTEGER	("NONCLUSTERINDEXMODE", "output/mkv/headers/index in first seekhead") else
					MAP_INTEGER	("HEADERSIZE", "output/mkv/headers/size") else
					MAP_FLAG    ("HEADERSTRIPPING", "output/mkv/compression/header striping") else
					MAP_FLAG	("PREVCLUSTERSIZE", "output/mkv/clusters/prevclustersize") else
					MAP_FLAG	("CLUSTERPOSITION", "output/mkv/clusters/position") else
					MAP_FLAG	("LIMIT1STCLUSTER", "output/mkv/clusters/limit first") else
					MAP_FLAG    ("USE_A_AAC", "output/mkv/use a_aac") else
					MAP_FLAG	("DISPWH", "output/mkv/displaywidth_height") else
					if (!strcmp(w,"CUES")) {
						w=getword(&cParam);
						if (isposint(w)) {
							settings->SetInt("output/mkv/cues/on",!!atoi(w));
						} else {
							MAP_FLAG("AUTOSIZE", "output/mkv/cues/autosize") else
							MAP_INTEGER("TARGETSIZERATIO", "output/mkv/cues/target size ratio") else
							MAP_INTEGER("MINIMUMINTERVAL", "output/mkv/cues/minimum interval") else
							MAP_INTEGER("SIZERATIO", "output/mkv/cues/size ratio") else
							MAP_INTEGER("WRITEBLOCKNUMBER", "output/mkv/cues/write blocknumber") else
							MAP_FLAG("VIDEO", "output/mkv/cues/video/on") else
							MAP_FLAG("SUBS", "output/mkv/cues/subs/on") else
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
						MAP_INTEGER("MKA", "output/mkv/TimecodeScale/mka") else
						MAP_INTEGER("MKV", "output/mkv/TimecodeScale/mkv") else
						bError=true;
					} else

					MAP_FLAG("FORCE_V1.0", "output/mkv/force v1") else
					MAP_FLAG("FORCE_V2.0", "output/mkv/force v2") else
					MAP_FLAG("FORCE_V1", "output/mkv/force v1") else
					MAP_FLAG("FORCE_V2", "output/mkv/force v2") else
					MAP_INTEGER("FLOAT_WIDTH", "output/mkv/floats/width") else
					MAP_FLAG("2ND_COPY_OF_TRACKS", "output/mkv/2nd Tracks") else
					MAP_FLAG("CLUSTERINDEX", "output/mkv/clusters/index/on") else
					MAP_FLAG("RANDOMIZE_ORDER", "output/mkv/randomize element order") else
					bError = true;
				} else 
				if (!strcmp(w,"AVI")) {
					w=getword(&cParam);
					MAP_INTEGER("AC3FPC", "output/avi/ac3/frames per chunk") else
					MAP_INTEGER("DTSFPC", "output/avi/dts/frames per chunk") else
					MAP_INTEGER("MP3FPC", "output/avi/mp3/frames per chunk") else
					MAP_INTEGER("RIFFAVISIZE", "output/avi/opendml/riff avi size") else
					MAP_FLAG("ADDJUNKBEFOREHEADERS", "output/avi/move hdrl") else
					MAP_FLAG("HAALIMODE", "output/avi/opendml/haalimode") else
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

						SINGLE_CHAPTER_DATA scd; memset(&scd, 0, sizeof(scd));

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
						if (!chapter_level)
							scd.bIsEdition = 1;
						

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
						settings->SetInt("output/general/prefered format", SOF_AVI);
					} else 
					if (!strcmp(w,"MKV")) {
						settings->SetInt("output/general/prefered format", SOF_MKV);
					} else bError=true;
				} else
				if (!strcmp(w,"RESOLUTION")) {
					w=getword(&cParam);
					m_OutputResolution.SetWindowText(w);
				} else
				if (!strcmp(w,"TITLE")) {
					w=cParam;
					SetWindowTextUTF8(hTitleEdit, w);
				} else
				MAP_FLAG("OVERLAPPED", "output/general/overlapped") else
				MAP_FLAG("UNBUFFERED", "output/general/unbuffered") else
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
				} else/*
				if (!strcmp(w,"UNBUFFERED")) {
					w=getword(&cParam);
					if (isposint(w)) {
						settings->SetInt("input/unbuffered", !!atoi(w));
					} else bError = true;
				} else*/
				MAP_FLAG("UNBUFFERED", "input/general/unbuffered") else
				MAP_FLAG("OVERLAPPED", "input/general/overlapped") else
				if (!strcmp(w,"USE")) {
					w=getword(&cParam);
					if (!strcmp(w,"CACHE")) {
						w=getword(&cParam);
						settings->SetInt("input/general/use cache",!!atoi(w));
					} else bError=true;
				} else
				if (!strcmp(w, "FILEORDER")) {
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
/*						HTREEITEM hItem = m_StreamTree.GetRootItem();
						while (i < item_count) {
							TREE_ITEM_INFO* tii = m_StreamTree.GetItemInfo(hItem);
							if (tii) {
								tii->iCurrPos = 2*index[i++];
							}
							hItem = m_StreamTree.GetNextSiblingItem(hItem);
						}*/
						for (i=0; i<item_count; i++)
							m_SourceFiles.GetFileInfo(i)->current_pos = 2*index[i];

						m_SourceFiles.SortItems();
						delete index;
						m_StreamTree.Sort();

					} else
						bError = true;
				}
				else
				if (!strcmp(w,"M2F2")) { 
					w=getword(&cParam);
					if (!strcmp(w,"CRC")) {
						w=getword(&cParam);
						settings->SetInt("input/m2f2/crc check", !!atoi(w));
					} else bError=true;
				}
				else
				if (!strcmp(w,"AVI")) {
					w=getword(&cParam);
					if (!strcmp(w,"FORCE")) {
						w=getword(&cParam);
						if (!strcmp(w,"MP3VBR")) {
							w=getword(&cParam);
							settings->SetInt("input/avi/force mp3 vbr", !!atoi(w));
						} else bError=true;
					} 
					else
					if (!strcmp(w,"FIXDX50")) {
						w=getword(&cParam);
						settings->SetInt("input/avi/repair DX50", !!atoi(w));
					} 
					else
					if (!strcmp(w,"IGNLCHUNKS")) {
						w=getword(&cParam);
						if (!strcmp(w,"ON")) {
							settings->SetInt("input/avi/large chunks/ignore", 1);
						}
						else
						if (!strcmp(w,"OFF")) {
							settings->SetInt("input/avi/large chunks/ignore", 0);
						}
						else
						if (isposint(w)) {
							ofOptions.dwIgnoreSize=atoi(w);
						} else bError=true;
					} 
					else
					if (!strcmp(w,"TRY2FIXLCHUNKS")) {
						w=getword(&cParam);
						settings->SetInt("input/avi/large chunks/repair", !!atoi(w));
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

	return CResizeableDialog::WindowProc(message, wParam, lParam);
}

void CAVIMux_GUIDlg::ApplyStreamSettings()
{
	HTREEITEM hItem, hChild;
	TREE_ITEM_INFO* tii[2];
	AUDIO_STREAM_INFO* asi = NULL;
	SUBTITLE_STREAM_INFO* ssi = NULL;
	VIDEO_STREAM_INFO* vsi = NULL;

	hItem = m_StreamTree.GetRootItem();
	while (hItem) {
		vsi = NULL;
		asi = NULL;
		ssi = NULL;
		tii[0] = m_StreamTree.GetItemInfo(hItem);
		if (tii[0]->iID == TIIID_ASI)
			asi = tii[0]->pASI;
		 
		if (tii[0]->iID == TIIID_SSI)
			ssi = tii[0]->pSSI;

		if (tii[0]->iID == TIIID_VSI)
			vsi = tii[0]->pVSI;

		hChild = m_StreamTree.GetChildItem(hItem);
		while (hChild) {

			tii[1] = m_StreamTree.GetItemInfo(hChild);
			switch (tii[1]->iID) {
				case TIIID_LNGCODE: 
					if (asi) asi->audiosource->SetLanguageCode(tii[1]->pText);
					if (ssi) ssi->lpsubs->SetLanguageCode(tii[1]->pText);
					if (vsi) vsi->videosource->SetLanguageCode(tii[1]->pText);
					break;
				case TIIID_STRNAME:
					if (asi) asi->audiosource->SetName(tii[1]->pText);
					if (ssi) ssi->lpsubs->SetName(tii[1]->pText);
					if (vsi) vsi->videosource->SetName(tii[1]->pText);
					
					break;
			}

			hChild = m_StreamTree.GetNextSiblingItem(hChild);
		}

		hItem = m_StreamTree.GetNextSiblingItem(hItem);
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
	if (hItem) tii = m_StreamTree.GetItemInfo(hItem);
	if (hItem) hParent = m_StreamTree.GetParentItem(hItem);
	if (hParent) ptii = m_StreamTree.GetItemInfo(hParent);

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

	RECT r;
	m_StreamTree.GetItemRect(hItem, &r, false);
	m_StreamTree.InvalidateRect(&r);

	hItem = pNMTreeView->itemNew.hItem;
	if (hItem) hParent = m_StreamTree.GetParentItem(hItem);
	tii = m_StreamTree.GetItemInfo(hItem);
	if (hParent) ptii = m_StreamTree.GetItemInfo(hParent);

	//m_AudioName.SetWindowText("");
	if (tii && tii->iID == TIIID_ASI || ptii && ptii->iID == TIIID_ASI) {
		if (tii && tii->iID == TIIID_ASI) 
			asi = tii->pASI; 
		else 
			asi = ptii->pASI;

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
		m_Stream_Lng_Label.ShowWindow(SW_SHOW);
	} else {
		m_Stream_Lng.ShowWindow(SW_HIDE);
		m_Stream_Lng_Label.ShowWindow(SW_HIDE);
	}

	if (tii && (tii->iID == TIIID_ASI)) {
		if (iCurrentView == 1) {
			m_Audiodelay.ShowWindow(SW_SHOW);
			m_Audiodelay_Label.ShowWindow(SW_SHOW);
		}
	} else {
		m_Audiodelay.ShowWindow(SW_HIDE);
		m_Audiodelay_Label.ShowWindow(SW_HIDE);
	}

	ApplyStreamSettings();

	m_StreamTree.GetItemRect(hItem, &r, false);
	m_StreamTree.InvalidateRect(&r);

	m_StreamTree.UpdateWindow();

	bDo_OnSelchangeLngCode = true;

	*pResult = 0;
}

void CAVIMux_GUIDlg::OnItemexpandedAudiotree(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen
	m_StreamTree.Invalidate();
	m_StreamTree.UpdateWindow();

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

		if (auto_apply_file_title) {
			if (fi->dwType & FILETYPE_MKV) {
				SetWindowTextUTF8(hTitleEdit, fi->MKVFile->GetSegmentTitle());
			} else if (fi->dwType & FILETYPE_AVI) {
				SetWindowTextUTF8(hTitleEdit, fi->AVIFile->GetTitle());
			} else SetWindowTextUTF8(hTitleEdit, "");
		}		
	}
}

void CAVIMux_GUIDlg::ApplyCurrentLanguageCode()
{
	HTREEITEM hItem = m_StreamTree.GetSelectedItem();
	TREE_ITEM_INFO* tii = hItem?m_StreamTree.GetItemInfo(hItem):0;
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

void CAVIMux_GUIDlg::OnEditchangeStreamLng() 
{
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen
}

void CAVIMux_GUIDlg::OnSelchangeStreamLng() 
{
	ApplyCurrentLanguageCode();

	RECT r;
	m_StreamTree.GetItemRect(m_StreamTree.GetSelectedItem(), &r, false);

	m_StreamTree.InvalidateRect(&r);
	m_StreamTree.UpdateWindow();
}

void CAVIMux_GUIDlg::OnEditupdateStreamLng() 
{
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen
	OnSelchangeStreamLng();

}

void CAVIMux_GUIDlg::OnGetAudiodispinfo(NMHDR* pNMHDR, LRESULT* pResult) 
{
	return m_StreamTree.OnTvnGetdispinfo(pNMHDR, pResult);
/*	TV_DISPINFO* pTVDispInfo = (TV_DISPINFO*)pNMHDR;
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen
	
	AUDIO_STREAM_INFO* asi = NULL;
	TREE_ITEM_INFO*	   tii = NULL;
	char*	d = pTVDispInfo->item.pszText;
	bool	bBracket = false;

	char	bitrate[128];
	char	channels[128];
	char	sample_rate[128];

	char	c[1024];
	ZeroMemory(c,sizeof(c));
	memset(bitrate, 0, sizeof(bitrate));
	memset(channels, 0, sizeof(channels));
	memset(sample_rate, 0, sizeof(sample_rate));
	HTREEITEM hItem = pTVDispInfo->item.hItem;
	char* AAC_prof_name;
	bool bAddComma=false;
	int i,j;
	
	if ((pTVDispInfo->item.mask & TVIF_TEXT) == 0) {
		*pResult = 0;
		return;
	}
	
	tii = m_StreamTree.GetItemInfo(hItem);

	if (!bEditInProgess) {
		if (tii && tii->iID == TIIID_ASI) {
			*d = 0;
			
			if (tii->pASI->audiosource->IsDefault()) {
				strcat(d,"(default) ");
			}
			strcat(d,"audio: ");
			if (tii) asi = tii->pASI;
			if (asi && asi->bNameFromFormatTag) {
				int idatarate = (asi->audiosource->GetAvgBytesPerSec() + 62) /125;
				if (idatarate) 
					sprintf(bitrate,"%d kbps",idatarate);
				else 
					strcpy(bitrate, "unknown bitrate");

				sprintf(channels, "%d Ch", asi->audiosource->GetChannelCount());
				
				int isr = (int)(0.49+asi->audiosource->GetOutputFrequency());
				if ((isr % 1000) == 0) {
					sprintf(sample_rate, "%2dkHz", 
						(int)((0.49+asi->audiosource->GetOutputFrequency())/1000));
				} else {
					sprintf(sample_rate, "%5dHz", 
						(int)((0.49+asi->audiosource->GetOutputFrequency())));
				}

				switch (asi->dwType) {
					case AUDIOTYPE_MP3CBR:
						i = (int)asi->audiosource->FormatSpecific(MMSGFS_MPEG_LAYERVERSION);
						j = (int)asi->audiosource->FormatSpecific(MMSGFS_MPEG_VERSION);
						sprintf(c,"MPEG %d Layer %d (CBR %s, %s, %s",j,i,bitrate,channels,
							sample_rate);
						bBracket = true;
						break;
					case AUDIOTYPE_MP3VBR:
						i = (int)asi->audiosource->FormatSpecific(MMSGFS_MPEG_LAYERVERSION);
						j = (int)asi->audiosource->FormatSpecific(MMSGFS_MPEG_VERSION);
						sprintf(c,"MPEG %d Layer %d (VBR, %s, %s",j, i,channels,
							sample_rate);
						bBracket = true;
						break;
					case AUDIOTYPE_PLAINCBR:
						sprintf(c,"CBR (0x%04X", asi->audiosource->GetFormatTag());
						bBracket = true;
						break;
					case AUDIOTYPE_AC3:
						sprintf(c,"AC3 (%s %s, %s, %s",bitrate, asi->audiosource->IsCBR()?"CBR":"VBR",
							channels, sample_rate);
						bBracket = true;
						break;
					case AUDIOTYPE_PCM:
						sprintf(c,"PCM (%s, %s, %s", bitrate, channels, sample_rate);
						bBracket = true;
						break;
					case AUDIOTYPE_DTS:
						sprintf(c,"DTS (%s %s, %s, %s",bitrate, asi->audiosource->IsCBR()?"CBR":"VBR",
							channels, sample_rate);
						bBracket = true;
						break;
					case AUDIOTYPE_VORBIS:
						sprintf(c,"Vorbis (%s, %s, %s",bitrate, channels,
							sample_rate);
						bBracket = true;
						break;
					case AUDIOTYPE_AAC:
						if ((int)asi->audiosource->FormatSpecific(MMSGFS_AAC_ISSBR)) 
							AAC_prof_name = "HE";
						else switch (asi->audiosource->FormatSpecific(MMSGFS_AAC_PROFILE)) {
							case AAC_ADTS_PROFILE_LC: AAC_prof_name = "LC"; break;
							case AAC_ADTS_PROFILE_LTP: AAC_prof_name = "LTP"; break;
							case AAC_ADTS_PROFILE_MAIN: AAC_prof_name = "MAIN"; break;
							case AAC_ADTS_PROFILE_SSR: AAC_prof_name = "SSR"; break;
							default: AAC_prof_name = "unknown";
						}
						sprintf(c,"%s-AAC (MPEG %d, %s, %s, %s",
							AAC_prof_name, 
							(int)asi->audiosource->FormatSpecific(MMSGFS_AAC_MPEGVERSION),
							bitrate, channels, sample_rate);
						bBracket = true;
						break;
					case AUDIOTYPE_DIVX:
						sprintf(c,"divX%s, %s, %s, %s",bitrate, channels, sample_rate);
						break;
				/*	default:
						if (asi->audiosource->GetIDString()) {
							sprintf(c,"%s (%s",asi->audiosource->GetIDString(),cdatarate);
						}
						break;				
						// hier new end comment
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
				sprintf(c,asi->audiosource->GetCodecID());
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

			if (!(m_StreamTree.GetItemState(hItem,TVIS_EXPANDED) & TVIS_EXPANDED) && 
				  m_StreamTree.FindID(hItem,TIIID_LNGCODE,&tii) && tii->pText && strlen(tii->pText)) {
				if (bAddComma) strcat(d,", ");
				bAddComma = true;
				sprintf(c,"%s",tii->pText);
				strcat(d,c);
			}

			if (!(m_StreamTree.GetItemState(hItem,TVIS_EXPANDED) & TVIS_EXPANDED) && 
				  m_StreamTree.FindID(hItem,TIIID_STRNAME,&tii) && tii->pText && strlen(tii->pText)) {
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
				
				if (!(m_StreamTree.GetItemState(hItem,TVIS_EXPANDED) & TVIS_EXPANDED) && 
					  m_StreamTree.FindID(hItem,TIIID_LNGCODE,&tii) && tii->pText && strlen(tii->pText)) {
					sprintf(c," (%s",tii->pText);
					strcat(d,c);
					bAddComma = true;
					bBracket = false;
				}
				if (!(m_StreamTree.GetItemState(hItem,TVIS_EXPANDED) & TVIS_EXPANDED) && 
					  m_StreamTree.FindID(hItem,TIIID_STRNAME,&tii) && tii->pText && strlen(tii->pText)) {
					if (bAddComma) strcat(d,", ");
					if (bBracket) strcat(d," (");
					bAddComma = true;
					bBracket = false;
					sprintf(c,"%s",tii->pText);
					strcat(d,c);
				}
				int _i; 
				if ((_i = subs->GetCompressionAlgo()) != COMPRESSION_NONE) {
					if (bAddComma) strcat(d,", ");
					if (bBracket) strcat(d," (");
					bAddComma = false;
					bBracket = false;
					sprintf(c,"compression: %s",((_i == COMPRESSION_ZLIB)?"zlib":"unknown"));
					strcat(d,c);
				}

				if (!bBracket) strcat(d,")");
			}

		} else
		if (tii && tii->iID == TIIID_VSI) {
			*d = 0;
			VIDEOSOURCE* v = tii->pVSI->videosource;
			char* codecid = v->GetCodecID();
			bool bracket = true;
			
			if (v->IsDefault()) 
				strcat(d,"(default) ");
			strcat(d, "video: ");
			
			if (codecid) {
				strcat(d, codecid);
				strcat(d, " ");
			};
			
			if (!codecid || !strcmp(codecid, "V_MS/VFW/FOURCC")) {
				DWORD dwfourcc = v->GetFourCC();
				char fourcc[32]; memset(fourcc, 0, sizeof(fourcc));
				char fmts[64];
				if (!codecid)
					strcpy(fmts, "FourCC: %c%c%c%c");
				else {
					strcpy(fmts, "(FourCC: %c%c%c%c, ");
					bracket = false;
				}

				sprintf(fourcc, fmts, (dwfourcc >> 0) & 0xFF,
					(dwfourcc >> 8) & 0xFF, (dwfourcc >> 16) & 0xFF,
					(dwfourcc >> 24) & 0xFF);
				strcat(d, fourcc);
				strcat(d, " ");
			}

			if (bracket) strcat(d, "(");
			bAddComma = false;
			c[0]=0;
			
			int idatarate = (int) (((double)v->GetSize() / (v->GetDurationUnscaled() / 8000000)));
			if (idatarate) {
				if (idatarate < 1000.)
					sprintf(bitrate,"%d kbps",idatarate);
				else
					sprintf(bitrate,"%4.2fMbps",(float)idatarate/1000.);
			} else 
				strcpy(bitrate, "unknown bitrate");
			if (bAddComma) strcat(d, ", ");
			strcat(d, bitrate);
			bAddComma = true;

			if (bAddComma) strcat(d, ", ");
			__int64 s = v->GetSize();
			FormatSize(c, s);
			strcat(d, c);
			bAddComma = true;

			s = (v->GetDuration() * v->GetTimecodeScale() + 499999) / 1000000;
			if (s > 0) {
				if (bAddComma) strcat(d, ", ");
				Millisec2Str(s, c);
				strcat(d, c);
				bAddComma = true;
			}

			if (!(m_StreamTree.GetItemState(hItem,TVIS_EXPANDED) & TVIS_EXPANDED)) {
				v->GetLanguageCode(c);
				if (c[0]) {
					if (bAddComma) strcat(d, ", ");
					strcat(d, c);
					bAddComma = true;
				}
				v->GetName(c);
				if (c[0]) {
					if (bAddComma) strcat(d, ", ");
					strcat(d, c);
					bAddComma = true;
				}
				strcat(d, ")");
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
*/}

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
		
	tii = m_StreamTree.GetItemInfo(hItem);
	if (tii->iID != TIIID_STRNAME) {
		*pResult = 1;
		return;
	}
	
	if (unicode) {
		HWND edit = TreeView_GetEditControl((HWND)m_StreamTree.m_hWnd);
		UTF82WStr(tii->pText, c);
		SendMessageW(edit, WM_SETTEXT, 0, (LPARAM)c);
	} else {
		CEdit* edit = m_StreamTree.GetEditControl();
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
	
	if ((tii = m_StreamTree.GetItemInfo(hItem)) && tii->iID == TIIID_STRNAME) {

		if (unicode) {
			HWND edit = TreeView_GetEditControl(m_StreamTree.m_hWnd);
			SendMessageW(edit, WM_GETTEXT, 256, (LPARAM)c);
			WStr2UTF8(c, c);
		} else {
			CEdit* edit = m_StreamTree.GetEditControl();
			edit->GetWindowText(c, 512);
			Str2UTF8(c, c);
		}
		strcpy(tii->pText, c);

		HTREEITEM hNext = m_StreamTree.GetNextSiblingItem(hItem);

		m_StreamTree.PostMessage(TVM_SELECTITEM, TVGN_CARET, (LPARAM) (HTREEITEM) hNext);
		m_Stream_Lng.PostMessage(WM_SETFOCUS, 0, 0);
		tab_stop_hist = 1;

	}

	bEditInProgess = 0;

	*pResult = 1;
}

CAttribs* CAVIMux_GUIDlg::GetSettings()
{
	return settings;
}

void CAVIMux_GUIDlg::OnChangeOutputresolution() 
{
	m_OutputResolution.InvalidateRect(NULL);
	m_OutputResolution.UpdateWindow();

}

BOOL CAVIMux_GUIDlg::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult) 
{
	// TODO: Speziellen Code hier einfügen und/oder Basisklasse aufrufen
	if (wParam == IDC_AUDIOTREE) {
		Sleep(1);
		int code = ((NMHDR*)lParam)->code; // TVN_BEGINDRAG

	}

	return CResizeableDialog::OnNotify(wParam, lParam, pResult);
}


void CAVIMux_GUIDlg::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	// TODO: Code für die Behandlungsroutine für Nachrichten hier einfügen und/oder Standard aufrufen
	
	CResizeableDialog::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CAVIMux_GUIDlg::OnChapterEditor() 
{
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen
	CChapterDlg* ccdlg;
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen

	ccdlg = new CChapterDlg;
	ccdlg->SetChapters(chapters);
//	if (!settings->Exists("gui/chapter_editor"))
	ccdlg->Attribs(settings->GetAttr("gui/chapter_editor"));
	ccdlg->DoModal();

	delete ccdlg;
		
}

void CAVIMux_GUIDlg::OnDestroy() 
{
//	delete m_Protocol.GetFont();
	std::vector<CFont*>::iterator f = additional_fonts.begin();

	CResizeableDialog::OnDestroy();
	
	SaveGUIConfig();

	for (; f != additional_fonts.end(); f++)
		delete *f;

	if (settings) {
		settings->Delete();
		delete settings;
	}

	// TODO: Code für die Behandlungsroutine für Nachrichten hier einfügen
}

bool CAVIMux_GUIDlg::ManualTabStopOrder()
{
	HWND hFocus = GetFocushWnd(this);

	return (iCurrentView == 1 &&
		    hFocus != m_Cancel_Button &&
		    hFocus != m_Start_Button &&
			hFocus != m_Chapter_Editor &&
			hFocus != m_Output_Options_Button &&
			hFocus != m_SourceFiles &&
			hFocus != m_Add_Video_Source &&
			hFocus != m_SourceFiles &&
			hFocus != hTitleEdit &&
			hFocus != m_OutputResolution &&
			hFocus != m_No_Audio &&
			hFocus != m_No_Subtitles &&
			hFocus != m_All_Audio &&
			hFocus != m_All_Subtitles &&
			hFocus != m_Default_Audio);
}

void CAVIMux_GUIDlg::OnKillfocusStreamLng() 
{
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen
	if (tab_stop_hist == 1 && ManualTabStopOrder()) {
		tab_stop_hist = -1;
		
		HTREEITEM hItem = m_StreamTree.GetSelectedItem();
		HTREEITEM hParent = m_StreamTree.GetParentItem(hItem);
		HTREEITEM hNext = m_StreamTree.GetNextSiblingItem(hParent);
		if (hNext) {
			HTREEITEM hName = m_StreamTree.GetChildItem(hNext);
			m_StreamTree.PostMessage(TVM_SELECTITEM, TVGN_CARET, (LPARAM) (HTREEITEM) hName);
			m_StreamTree.SetFocus();
		}
	}
}

void CAVIMux_GUIDlg::OnKillfocusAudiotree(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen
	HTREEITEM hItem = m_StreamTree.GetSelectedItem();
	TREE_ITEM_INFO* tii = m_StreamTree.GetItemInfo(hItem);

	if (bEditInProgess || !ManualTabStopOrder()) {
		*pResult = 0;
		return;
	}

	if (tii && tii->iID == TIIID_STRNAME) {
		tab_stop_hist = 1;
		m_StreamTree.SendMessage(TVM_SELECTITEM, TVGN_CARET, (LPARAM) (HTREEITEM) m_StreamTree.GetNextSiblingItem(hItem));
		m_Stream_Lng.SetFocus();
	} else
	if (tii && tii->iID == TIIID_ASI) {
		m_Audiodelay.SetFocus();
		tab_stop_hist = -1;
	}

	*pResult = 0;
}

void CAVIMux_GUIDlg::OnKillfocusDelay() 
{
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen
	HTREEITEM hItem = m_StreamTree.GetSelectedItem();

	if (hItem == hDelayedStream && ManualTabStopOrder()) {
		HTREEITEM hName = m_StreamTree.GetChildItem(hItem);

		m_StreamTree.SelectItem(hName);
		m_StreamTree.SetFocus();
	}
	
}

void CAVIMux_GUIDlg::OnSetfocusDelay() 
{
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen
	hDelayedStream = m_StreamTree.GetSelectedItem();
}

void CAVIMux_GUIDlg::UpdateProtocolColumn()
{
	RECT rect;

	if (m_Protocol.m_hWnd) {
		m_Protocol.GetClientRect(&rect);
		m_Protocol.SetColumnWidth(0,(rect.right-rect.left)/7);
		m_Protocol.SetColumnWidth(1,(rect.right-rect.left)*6/7-1);
	}
}

void CAVIMux_GUIDlg::OnSize(UINT nType, int cx, int cy)
{
	CResizeableDialog::OnSize(nType, cx, cy);

	UpdateProgressList();
	UpdateProtocolColumn();

	// TODO: Fügen Sie hier Ihren Meldungsbehandlungscode ein.
}

void CAVIMux_GUIDlg::OnLvnItemchangedProgressList(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: Fügen Sie hier Ihren Kontrollbehandlungscode für die Benachrichtigung ein.
	*pResult = 0;
}

void CAVIMux_GUIDlg::OnBnClickedLeave()
{
	// TODO: Fügen Sie hier Ihren Kontrollbehandlungscode für die Benachrichtigung ein.
	OnCancel();

}

void CAVIMux_GUIDlg::OnBnClickedCancel()
{
	// TODO: Fügen Sie hier Ihren Kontrollbehandlungscode für die Benachrichtigung ein.
}

void CAVIMux_GUIDlg::OnLbnDblclkSourcefilelist()
{
	// TODO: Fügen Sie hier Ihren Kontrollbehandlungscode für die Benachrichtigung ein.
	int iIndex = m_SourceFiles.GetCurSel();
	if (iIndex == LB_ERR)
		return;

	OnAddFileList();
}
