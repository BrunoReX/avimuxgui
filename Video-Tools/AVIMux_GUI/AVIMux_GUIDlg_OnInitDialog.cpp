#include "stdafx.h"
#include "AVIMux_GUIDlg.h"
#include "global.h"
#include "AVIMux_GUI.h"
#include "..\UnicodeCalls.h"
#include "UTF8Windows.h"
#include "TABHandler.h"
#include "LanguageCodes.h"
#include "OSVersion.h"
#include "../Filenames.h"
#include "Version.h"
#include "..\FileStream.h"
#include "TraceFile.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BOOL CAVIMux_GUIDlg::OnInitDialog()
{
	char*	Buffer;
	int		buffer_size = 65536;
	int		i;
	char*	dir;
	char*	lf;
	char	cwinver[64];

	//GetApplicationTraceFile()->SetTraceLevel(TRACE_LEVEL_NONE);

	srand(GetTickCount());
	settings = NULL;
	tab_stop_hist = -1;
	iUnicode_possible = 0;
	bAddAS_immed = 1;
	current_language_index = 0;

	GetOSVersionString(cwinver, 64);
    
	if (!IsOSWin2kplus()) {
		char c[4096];
		sprintf(c, "The following operating system has been found: %s. This application might work or not on Win 9x/ME/NT <=4. You should use Windows 2000/XP/2003.", cwinver);
		MessageBox(c, "Problem", MB_OK | MB_ICONERROR);
	//	PostMessage(WM_QUIT);
	//	return 0;
	}

	utf8_EnableRealUnicode(DoesOSSupportUnicode());
//	printf("Unicode should be working\n");

	bEditInProgess = 0;	

	if (!(uiMessage = RegisterWindowMessage("mymessage_1"))) {
		MessageBox("Could not register user-defined window message!",
			"Error",MB_OK | MB_ICONERROR);
		PostMessage(WM_QUIT,0,0);
		return 0;
	}
	
	srand(GetTickCount());
	ZeroMemory(GlobalMuxingStartedSemaphoreName(0), 30);
	ZeroMemory(GlobalMuxSemaphoreName(0), 30);
	for (i=0;i<29;i++) {
		*GlobalMuxSemaphoreName(i)='a'+rand()%26;
		*GlobalMuxingStartedSemaphoreName(i)='a'+rand()%26;
	}
	hGlobalMuxSemaphore = CreateSemaphore(NULL, 1, 1, GlobalMuxSemaphoreName(0));
	hGlobalMuxingStartedSemaphore = CreateSemaphore(NULL, 0, 1, GlobalMuxingStartedSemaphoreName(0));

	cExtFilename = NULL;
	newz(DWORD, 16, dwLangs);
	chapters = NULL;

	dwLangs[0] = IDM_LANG1;
	dwLangs[1] = IDM_LANG2;
	dwLangs[2] = IDM_LANG3;
	dwLangs[3] = IDM_LANG4;
	dwLangs[4] = IDM_LANG5;
	dwLangs[5] = IDM_LANG6;
	dwLangs[6] = IDM_LANG7;
	dwLangs[7] = IDM_LANG8;
	dwLangs[8] = IDM_LANG9;
	dwLangs[9] = IDM_LANG10;
	dwLangs[10] = IDM_LANG11;
	dwLangs[11] = IDM_LANG12;
	dwLangs[12] = IDM_LANG13;
	dwLangs[13] = IDM_LANG14;
	dwLangs[14] = IDM_LANG15;
	dwLangs[15] = IDM_LANG16;

	Buffer=new char[65536];
	dir=new char[65536];
	lf=new char[65536];

	(*UGetModuleFileName())(NULL, Buffer, buffer_size/2);
	toUTF8(Buffer, dir);
	for (i=strlen(dir);i>0;i--) {
		if (dir[i]=='\\') {
			dir[i]=0;
			i=-1;
		}	
	}

	char* udir = NULL;
	appdir = dir;
	fromUTF8(dir, &udir);
	(*USetCurrentDirectory())(udir);
	free(udir);

	char odir[65536];
	Filename2LongFilename(dir, odir, sizeof(odir));
	strcpy(dir, odir);

	cfgfile = dir;
	cfgfile.append("\\config.ini");

	strcpy(lastjobfile,dir);
	strcat(lastjobfile,"\\last-job.amg");

	guifile = dir;
	guifile.append("\\gui.amg.xml");
	
//	strcpy(guifile, dir);
//	strcat(guifile, "\\gui.amg.xml");

	strcpy(lf,dir);
	strcat(lf,"\\languages.amg");
	Filename2LongFilename(lf, odir, sizeof(odir));
	strcpy(lf, odir);

	lngcodefile = dir;
	lngcodefile.append("\\language_codes.txt");
	char* temp = _strdup(lngcodefile.c_str());
	Filename2LongFilename(temp, odir, sizeof(odir));
	lngcodefile = odir;
	free(temp);

	CFileStream* F = new CFileStream();
	CTextFile* textfile = new CTextFile();

	if (F->Open(lf, STREAM_READ) != STREAM_OK || textfile->Open(STREAM_READ, F) != STREAM_OK) {
		MessageBox("Couldn't open file\n\nlanguages.amg\n\nPossible reasons are either that you have tried to run AVI-Mux GUI directly from a zip archive or that you deleted or removed this file.","Fatal error",MB_OK | MB_ICONERROR);
		PostMessage(WM_QUIT);
		return 0;
	}

	textfile->SetOutputEncoding(CHARACTER_ENCODING_UTF8);
	textfile->ReadLine(Buffer);
	int equalsignpos = strcspn(Buffer, "=");
	if (equalsignpos == strlen(Buffer)) {
		MessageBox("languages.amg file is invalid\n\nExpected: number=<number_of_language_files>","Fatal error",MB_OK | MB_ICONERROR);
		textfile->Close();
		F->Close();
		delete textfile;
		delete F;
		PostMessage(WM_QUIT);
		return 0;
	}
	Buffer[equalsignpos]=0;
	if (!strncmp(Buffer,"number",6))
	{
		DWORD dwLngCount = 0;
		dwLanguages=atoi(Buffer+7);
		lplpLanguages=(LANGUAGE_DESCRIPTOR**)new LANGUAGE_DESCRIPTOR[dwLanguages];
		for (i=0;i<(int)dwLanguages;i++)
		{
//			lstrcpy(lf,dir);
//			lstrcat(lf,"\\");
			lf[0]=0;
			memset(Buffer, 0, buffer_size);
			textfile->ReadLine(Buffer);
			lstrcat(lf,Buffer);
			Buffer[0]=0;
			if (!(lplpLanguages[dwLngCount]=LoadLanguageFile(lf)))
			{
				wsprintf(Buffer,"Couldn't open language file: \n\n%s\n\nIf you changed the original directory stucture inside the downloaded file, then shame on you! If you use Win 9x/ME, the problem could be a language file using UTF-8 coding. Open the file in Windows Editor and resave it using ANSI coding in that case.",lf);
				MessageBox(Buffer,"Error",MB_OK | MB_ICONERROR);
			} else dwLngCount++;
		}
		if (!dwLngCount) {
			wsprintf(Buffer,"Couldn't open any language file!");
			MessageBox(Buffer,"Error",MB_OK | MB_ICONERROR);
			PostMessage(WM_QUIT);
			return 0;
		}
		dwLanguages = dwLngCount;
	}

	textfile->Close();
	delete textfile;
	F->Close();
	delete F;
	
	SetCurrentLanguage(lplpLanguages[0]);
	cLogFileName[0]=0;
	strcpy(cLogFileName, dir);
	strcat(cLogFileName, "\\AVI-Mux GUI - Logfile - ");

	delete[] dir;
	delete[] lf;
	delete[] Buffer;

	CResizeableDialog::OnInitDialog();
	UpdateLanguage();
	m_Protocol.InitUnicode();
	// Hinzufügen des Menübefehls "Info..." zum Systemmenü.

	Buffer=new char[200];
	// Symbol für dieses Dialogfeld festlegen. Wird automatisch erledigt
	//  wenn das Hauptfenster der Anwendung kein Dialogfeld ist
	SetIcon(m_hIcon, TRUE);			// Großes Symbol verwenden
	SetIcon(m_hIcon, FALSE);		// Kleines Symbol verwenden
	
// Einstellungen aus config.ini laden -- VERALTET!
	ofOptions.dwFlags=0;
	GetPrivateProfileString("config","format","$Name ($Nbr)",Buffer,200,(char*)cfgfile.c_str());

	if (strstr(Buffer, "%%d") || strstr(Buffer, "%%s"))
		strcpy(Buffer, "$Name ($Nbr)");

	sfOptions.lpcNumbering=new char[1+lstrlen(Buffer)];
	lstrcpy(sfOptions.lpcNumbering,Buffer);
	
	GetPrivateProfileString("config","maxframes","0",Buffer,200,(char*)cfgfile.c_str());
	sfOptions.dwFrames=atoi(Buffer);
	GetPrivateProfileString("config","maxchunksize","0",Buffer,200,(char*)cfgfile.c_str());
	ofOptions.dwIgnoreSize=atoi(Buffer);
	GetPrivateProfileString("config","maxfiles","2",Buffer,200,(char*)cfgfile.c_str());
	sfOptions.dwMaxFiles=atoi(Buffer);
	GetPrivateProfileString("config","usemaxfiles","0",Buffer,200,(char*)cfgfile.c_str());
	sfOptions.dwUseMaxFiles=atoi(Buffer);
	GetPrivateProfileString("config","mp3cbrframemode","1",Buffer,200,(char*)cfgfile.c_str());
	i=GetPrivateProfileInt("config","avoidseekops",1,(char*)cfgfile.c_str());

//	sfOptions.bDispDoneDlg=true;
	sfOptions.bExitAfterwards=false;

	i=GetPrivateProfileInt("config","noaudio",0,(char*)cfgfile.c_str());
	CheckDlgButton(IDC_NO_AUDIO,(i==1)?BST_CHECKED:BST_UNCHECKED);
	if (!i)
	{
		i=GetPrivateProfileInt("config","allaudio",1,(char*)cfgfile.c_str());
		CheckDlgButton(IDC_ALL_AUDIO,(i==1)?BST_CHECKED:BST_UNCHECKED);
		if (!i)
		{
			i=GetPrivateProfileInt("config","defaudio",1,(char*)cfgfile.c_str());
			CheckDlgButton(IDC_DEFAULT_AUDIO,(i==1)?BST_CHECKED:BST_UNCHECKED);
		}
	}
	GetPrivateProfileString("config","defaudionbr","0",Buffer,200,(char*)cfgfile.c_str());
	SendDlgItemMessage(IDC_DEFAULT_AUDIO_NUMBER,WM_SETTEXT,0,(LPARAM)Buffer);

	i=GetPrivateProfileInt("config","nosubtitles",0,(char*)cfgfile.c_str());
	CheckDlgButton(IDC_NO_SUBTITLES,(i==1)?BST_CHECKED:BST_UNCHECKED);
	if (!i)
	{
		i=GetPrivateProfileInt("config","allsubtitles",1,(char*)cfgfile.c_str());
		CheckDlgButton(IDC_ALL_SUBTITLES,(i==1)?BST_CHECKED:BST_UNCHECKED);
	}


	i=GetPrivateProfileInt("config","openfileoptionsflags",
		SOFO_MP3_CHECKCBRASK,(char*)cfgfile.c_str());
	ofOptions.dwFlags|=i;
	
	sfOptions.dwUseManualSplitPoints=0;

	ofOptions.dwFlags |= SOFO_CH_IMPORT;
	char* lng;
	newz(char, 1+strlen(Buffer), lng);
	strcpy(lng,Buffer);

	PostMessage(GetUserMessageID(),IDM_SETLANGUAGE,(LPARAM)lng);

// ein paar Strings laden

	cstrInformation=LoadString(IDS_INFORMATION);
	cstrWarning=LoadString(IDS_WARNING);
	cstrError=LoadString(IDS_ERROR);
	cstrConfirmation=LoadString(IDS_CONFIRMATION);

	m_SourceFiles.AllowMoving(true);

	SetDialogState_Config();
	UpdateLanguage();
	m_Progress_List.SetAccuracy(PRAC_BYTES);
// neue Quelldateilistbox
	RECT	rect;
	m_Enh_Filelist.GetWindowRect(&rect);

	m_VideoStretchFactor.SetWindowText("1");


	RECT r;

	free (Buffer);

    m_Protocol.GetWindowRect(&rect);
	m_Protocol.InsertColumn(0,"Time",LVCFMT_CENTER,(rect.right-rect.left)/7);
	m_Protocol.InsertColumn(1,"Message",LVCFMT_LEFT,(rect.right-rect.left)*6/7-2);

// chapters
	chapters = new CChapters();

	m_Prg_Dest_File.EnableWindow(0);
	m_StreamTree.InitUnicode();

/*	if (m_StreamTree.IsWindowUnicode() != !!m_StreamTree.IsUnicode()) {
		MessageBox("Your system is b0rked: The result of IsUnicodeWindow() is inconsistent with the result of the CCM_GETUNICODEFORMAT message",
			"Warning", MB_OK | MB_ICONERROR);
	}
*/
	if (utf8_IsUnicodeEnabled() && !m_StreamTree.IsUnicode()) {
		char c[8192]; c[0]=0;
		sprintf(c, "Although this system is or pretends to be %s and supports Unicode, switching the stream tree to Unicode failed. Possible reasons are an archaic version of Internet Explorer (below 4.0) or you are running AVI-Mux GUI from an emulator not being able to switch tree views between ANSI and Unicode on runtime.\n\nThus, characters requiring Unicode will not be displayed correctly.", cwinver);
		MessageBox(c, "Warning", MB_OK);
	}

	ButtonState_STOP();
	m_Output_Options_Button.SetFocus();

	SetDebugLevel(4);
	
	// IDM_ABOUTBOX muss sich im Bereich der Systembefehle befinden.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		char*	cStr=LoadString(IDS_ABOUTBOX);
		strAboutMenu=cStr;
		if (!strAboutMenu.IsEmpty()) {	
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	char*	ncfgfn;
	newz(char, 10+strlen(cfgfile.c_str()), ncfgfn);
	strcpy(ncfgfn,cfgfile.c_str());
	strcat(ncfgfn,".amg");
	PostMessage(GetUserMessageID(), IDM_DOADDFILE, (LPARAM)ncfgfn);

	newz(char, 10+strlen(cfgfile.c_str()), ncfgfn);
	strcpy(ncfgfn,guifile.c_str());
	PostMessage(GetUserMessageID(),IDM_DOADDFILE,(LPARAM)ncfgfn);

	sfOptions.bB0rk = false;
	sfOptions.i1stTimecode = 0;
	sfOptions.active_avi_page = 1;
	sfOptions.active_mkv_page = 1;

	chapter_level = 0;
	chap[0] = chapters;
	sfOptions.chapters = chapters;
	SetDialogState_Config();
	sfOptions.split_points = new CSplitPoints;
	sfOptions.iActiveButton = 0;

	settings = new CAttribs;
	settings->SetInt("input/audio/mp3/check_cbr",23);
	settings->SetInt("input/general/use cache", 1);
	settings->SetInt("input/general/unbuffered", 1);
	settings->SetInt("input/general/overlapped", 1);

	settings->SetInt("input/avi/force mp3 vbr", 0);
	settings->SetInt("input/avi/large chunks/ignore", 0);
	settings->SetInt("input/avi/large chunks/repair", 0);
	settings->SetInt("input/avi/repair DX50", 1);

	settings->SetInt("input/m2f2/crc check", 1);

	settings->SetInt("output/mkv/ac3/frames per block", 1);
	settings->SetInt("output/mkv/displaywidth_height", 1);
	settings->SetInt("output/mkv/clusters/size", 512);
	settings->SetInt("output/mkv/clusters/time", 30000);
	settings->SetInt("output/mkv/clusters/prevclustersize", 1);
	settings->SetInt("output/mkv/clusters/position", 1);
	settings->SetInt("output/mkv/clusters/limit first", 1);
	settings->SetInt("output/mkv/clusters/index/on", 0);

	settings->SetInt("output/mkv/lacing/general/length", 500);
	settings->SetInt("output/mkv/lacing/general/use", 1);
	settings->SetInt("output/mkv/lacing/length", 1000);
	settings->SetInt("output/mkv/lacing/style", 3);
	settings->SetInt("output/mkv/lacing/mp3/use", 1);
	settings->SetInt("output/mkv/lacing/mp3/length", 500);
	settings->SetInt("output/mkv/lacing/ac3/use", 1);
	settings->SetInt("output/mkv/lacing/ac3/length", 200);
	settings->SetInt("output/mkv/lacing/aac/use", 1);
	settings->SetInt("output/mkv/lacing/aac/length", 200);
	settings->SetInt("output/mkv/lacing/dts/use", 1);
	settings->SetInt("output/mkv/lacing/dts/length", 100);
	settings->SetInt("output/mkv/lacing/vorbis/use", 1);
	settings->SetInt("output/mkv/lacing/vorbis/length", 50);
	settings->SetInt("output/mkv/lacing/video/on", 0);
	settings->SetInt("output/mkv/lacing/video/frames", 4);

	settings->SetInt("output/mkv/force v1",1);
	settings->SetInt("output/mkv/force v2",0);
	settings->SetInt("output/mkv/floats/width", 32);

	settings->SetInt("output/mkv/cues/on",1);
	settings->SetInt("output/mkv/cues/video/on",1);
	settings->SetInt("output/mkv/cues/audio/on",1);
	settings->SetInt("output/mkv/cues/subs/on",1);
	settings->SetInt("output/mkv/cues/audio/only audio-only/on",0);
	settings->SetInt("output/mkv/cues/write blocknumber", 1);
	settings->SetInt("output/mkv/compression/header striping", 0);

	settings->SetInt("output/mkv/cues/autosize", 1);
	settings->SetInt("output/mkv/cues/minimum interval", 2000);
	settings->SetInt("output/mkv/cues/size ratio", 20);
	settings->SetInt("output/mkv/cues/target size ratio", 980);

	settings->SetInt("output/mkv/TimecodeScale/mkv",500000);
	settings->SetInt("output/mkv/TimecodeScale/mka",10000);
	settings->SetInt("output/mkv/2nd Tracks",1);
	settings->SetInt("output/mkv/randomize element order", 1);
	settings->SetInt("output/mkv/headers/index in first seekhead", 1);
	settings->SetInt("output/mkv/headers/size", 0);

	settings->SetInt("output/mkv/hard linking", 0);
	settings->SetInt("output/mkv/use a_aac", 0);

	settings->SetInt("output/avi/opendml/riff avi size", 1);
	settings->SetInt("output/avi/opendml/on", 1);
	settings->SetInt("output/avi/opendml/haalimode", 0);
	settings->SetInt("output/avi/opendml/stdindex/pattern", SIP_AUTO);
	settings->SetInt("output/avi/opendml/stdindex/interval", 15000);
	settings->SetInt("output/avi/audio interleave/unit", AIU_KB);
	settings->SetInt("output/avi/audio interleave/value", 100);

	settings->SetInt("output/avi/audio preload", 200);

	settings->SetInt("output/avi/ac3/frames per chunk", 2);
	settings->SetInt("output/avi/dts/frames per chunk", 2);
	settings->SetInt("output/avi/mp3/frames per chunk", 1);
	settings->SetInt("output/avi/mp3/cbr frame mode", 1);
	settings->SetInt("output/avi/reclists", 1);
	settings->SetInt("output/avi/legacyindex", 1);
	settings->SetInt("output/avi/move hdrl", 0);
	settings->SetInt("output/general/logfile/on",0);
	settings->SetInt("output/general/unbuffered", 1);
	settings->SetInt("output/general/overlapped", 1);
	settings->SetInt("output/general/threaded", 1);
	settings->SetInt("output/general/cache/enabled", 1);
	settings->SetInt("output/general/cache/size per cacheline", 21);
	settings->SetInt("output/general/cache/cachelines", 4);
	settings->SetInt("output/general/file size/max", 2030);
	settings->SetInt("output/general/file size/limited", 0);
	settings->SetInt("output/general/numbering/enabled", 0);
	settings->SetInt("output/general/prefered format", 0);
	settings->SetInt("output/general/check disk space/enabled", 1);
	settings->SetInt("output/general/check disk space/lower limit", 10);

	settings->SetInt("output/ogg/pagesize", 65025);
	settings->SetInt("output/avi/inject/enabled", 0);
	settings->SetInt("output/avi/inject/probability", 2); //in n/10000
	settings->SetInt("output/avi/inject/size", 32);

	settings->Add("gui/chapter_editor", 0, ATTRTYPE_ATTRIBS, NULL); 
	settings->SetInt("gui/chapter_editor/window_size/width", 800);
	settings->SetInt("gui/chapter_editor/window_size/height", 600);	
	
	settings->Add("gui/file_information", 0, ATTRTYPE_ATTRIBS, NULL);
	settings->SetInt("gui/file_information/window_size/width", 800);
	settings->SetInt("gui/file_information/window_size/height", 600);	
	
	settings->Add("gui/ebml_tree", 0, ATTRTYPE_ATTRIBS, NULL);
	settings->SetInt("gui/ebml_tree/window_size/width", 800);
	settings->SetInt("gui/ebml_tree/window_size/height", 600);	
	
	settings->Add("gui/main_window", 0, ATTRTYPE_ATTRIBS, NULL);
	settings->SetInt("gui/main_window/window_size/width", 800);
	settings->SetInt("gui/main_window/window_size/height", 600);
	settings->SetInt("gui/main_window/font/quality", 0);

	settings->Add("gui/settings_window", 0, ATTRTYPE_ATTRIBS, NULL);
	settings->SetInt("gui/settings_window/window_size/width", 420);
	settings->SetInt("gui/settings_window/window_size/height", 507);
	
	settings->Add("gui/riff_tree", 0, ATTRTYPE_ATTRIBS, NULL);
	settings->SetInt("gui/riff_tree/window_size/width", 800);
	settings->SetInt("gui/riff_tree/window_size/height", 600);

	settings->SetInt("gui/main_window/source_files/highlight", 1);
	settings->SetInt("gui/main_window/source_files/lowlight", 1);
	settings->Add("gui/chapter_editor/default_save_extension", FATTR_ADDATTR_CREATE,
		ATTRTYPE_UTF8, "mkc");
	settings->Add("gui/chapter_editor/tree_default_title_languages", FATTR_ADDATTR_CREATE,
		ATTRTYPE_UTF8, "eng");

	settings->SetInt("gui/general/finished_muxing_dialog", 1);
	settings->SetInt("gui/general/overwritedlg", 1);
	settings->SetInt("gui/general/finished_muxing_dialog", 1);
	settings->SetInt("gui/main_window/streams/highlight/no_avi_output", 1);
	settings->SetInt("gui/main_window/streams/highlight/default", 1);

	settings->SetInt("gui/output/default_file_name_source", FILENAME_NOTHING);

	Attribs(settings->GetAttr("gui/main_window"));
	ReinitPosition();

	hLogFile = NULL;

	char title[100]; title[0]=0; GetWindowText(title,100); CString c;
	char version[32];
	GetAMGVersionString(version, sizeof(version));
	strcat(title, " ");
	strcat(title, version);
	SetWindowText(title);

/*	CString windowtitle;
	GetWindowText(windowtitle);
	windowtitle += " - ";
	windowtitle += cwinver;
	SetWindowText(windowtitle);
*/

	m_Add_Video_Source.EnableWindow(0);

	m_StatusLine.SetDisabledTextColor(RGB(0,0,0));
	m_Prg_Dest_File.SetDisabledTextColor(RGB(0,0,0));
	m_Audiodelay.ShowWindow(SW_HIDE);
	m_Audiodelay_Label.ShowWindow(SW_HIDE);

	m_Stream_Lng.ShowWindow(SW_HIDE);
	m_Stream_Lng_Label.ShowWindow(SW_HIDE);

	m_Title.GetWindowRect(&r);
	ScreenToClient(&r);

	m_Title.ShowWindow(SW_HIDE);
	hTitleEdit = m_Title.m_hWnd;


/* load language codes from file */
	LANGUAGE_CODES* lngcd = GetLanguageCodesObject();
	F = new CFileStream;
	textfile = new CTextFile;
	if (F->Open((char*)lngcodefile.c_str(), STREAM_READ) == STREAM_OK) {
		textfile->Open(STREAM_READ, F);
		textfile->SetOutputEncoding(CHARACTER_ENCODING_UTF8);
		int j = 0;
		char* buf = new char[j=1+(size_t)textfile->GetSize()];
		textfile->Read(buf, j-1);
		buf[j-1]=0;
		if ((j=lngcd->LoadFromString(buf))<1) {
			char c[256];
			sprintf(c, "Could not load language_codes.txt: Error parsing line %d", -j);
			MessageBox(c, "Fatal Error",
				MB_OK | MB_ICONERROR);
			PostMessage(WM_QUIT);
		}
		delete buf;
		textfile->Close();
		delete textfile;
		F->Close();
		delete F;
	} else {
		MessageBox("Could not load language_codes.txt.", "Fatal Error",
			MB_OK | MB_ICONERROR);
		PostMessage(WM_QUIT);
		return 0;
	}

	for (i=0;i<lngcd->GetCount();i++) {
		char buf[65536];
		buf[0]=0;
		sprintf(buf , "%s - %s", lngcd->GetCode(i), lngcd->GetFullName(i));
		m_Stream_Lng.SetItemData(m_Stream_Lng.AddString(buf),(LPARAM)lngcd->GetCode(i));
	}

	CreateEditUTF8(r, m_hWnd, GetInstance(), (HFONT)GetFont()->m_hObject, hTitleEdit);

	if (__argc>1) {
		for (i=1;i<__argc;i++)
		{
			if (!_stricmp(__argv[i],"-b0rk")) {
				sfOptions.bB0rk = true;
			} else if (!_stricmp(__argv[i], "-stdin")) {
				char* f2load = new char[128];
				strcpy(f2load, "*stdin*");
				PostMessage(GetUserMessageID(),IDM_DOADDFILE,(LPARAM)f2load);
			} else {
				char* f2load;
				newz(char,1+strlen(__argv[i]), f2load);
				strcpy(f2load,__argv[i]);
				PostMessage(GetUserMessageID(),IDM_DOADDFILE,(LPARAM)f2load);
			}
		}
	}

	int border = 16;

/******************************************/
/* create layout of page 1 of main window */
/******************************************/

	AttachWindow(m_Open_Files_Label, ATTB_LEFT, *this, border);
	AttachWindow(m_Open_Files_Label, ATTB_RIGHT, *this, -border);
	AttachWindow(m_SourceFiles, ATTB_LEFTRIGHT, m_Open_Files_Label);
	AttachWindow(m_SourceFiles, ATTB_TOP, m_Open_Files_Label, ATTB_BOTTOM, 1);
	AttachWindow(m_SourceFiles, *this, ATTB_HEIGHTRATIO, 1, 0.15);
	AttachWindow(m_Add_Video_Source, ATTB_TOP, m_SourceFiles, ATTB_BOTTOM, 4);
	AttachWindow(m_Add_Video_Source, ATTB_LEFTRIGHT, m_SourceFiles);

	AttachWindow(m_OutputResolution, ATTB_TOP, m_Add_Video_Source, ATTB_BOTTOM, 16);
	AttachWindow(m_OutputResolution, ATTB_RIGHT, m_SourceFiles);
	AttachWindow(m_OutputResolution_Label, ATTB_RIGHT, m_OutputResolution, ATTB_LEFT, -2);
	AttachWindow(m_OutputResolution_Label, ATTB_VCENTER, m_OutputResolution);

	AttachWindow(m_Title_Label, ATTB_LEFT, m_Add_Video_Source, 2);
	AttachWindow(m_Title_Label, ATTB_TOPBOTTOM, m_OutputResolution_Label);

	AttachWindow(hTitleEdit, ATTB_VCENTER, m_Title_Label);
	AttachWindow(hTitleEdit, ATTB_RIGHT, m_OutputResolution_Label, ATTB_LEFT, -8);
	AttachWindow(hTitleEdit, ATTB_LEFT, m_Title_Label, ATTB_RIGHT, 4);

	AttachWindow(m_No_Audio, ATTB_TOP, hTitleEdit, ATTB_BOTTOM, 1);
	AttachWindow(m_No_Audio, ATTB_LEFT, hTitleEdit);

	AttachWindow(m_Audio_Label, ATTB_VCENTER, m_No_Audio);
	AttachWindow(m_Audio_Label, ATTB_LEFT, m_Title_Label);

	AttachWindow(m_All_Audio, ATTB_LEFT, m_No_Audio, ATTB_RIGHT, 4);
	AttachWindow(m_All_Audio, ATTB_VCENTER, m_No_Audio);
	
	AttachWindow(m_Default_Audio_Label, ATTB_LEFT, m_All_Audio, ATTB_RIGHT, 4);
	AttachWindow(m_Default_Audio_Label, ATTB_VCENTER, m_No_Audio);

	AttachWindow(m_Default_Audio, ATTB_LEFT, m_Default_Audio_Label, ATTB_RIGHT, 4);
	AttachWindow(m_Default_Audio, ATTB_VCENTER, m_No_Audio);
	
	AttachWindow(m_No_Subtitles, ATTB_LEFT, m_No_Audio);
	AttachWindow(m_No_Subtitles, ATTB_TOP, m_No_Audio, ATTB_BOTTOM, 1);

	AttachWindow(m_Subtitles_Label, ATTB_VCENTER, m_No_Subtitles);
	AttachWindow(m_Subtitles_Label, ATTB_LEFT, m_Audio_Label);

	AttachWindow(m_All_Subtitles, ATTB_LEFT, m_All_Audio);
	AttachWindow(m_All_Subtitles, ATTB_VCENTER, m_No_Subtitles);

	AttachWindow(m_StatusLine, ATTB_BOTTOM, *this, ATTB_BOTTOM, -border);
	AttachWindow(m_StatusLine, ATTB_LEFT, m_Open_Files_Label);
	AttachWindow(m_StatusLine, ATTB_RIGHT, m_SourceFiles);

	AttachWindow(m_StreamTree, ATTB_BOTTOM, m_StatusLine, ATTB_TOP, -4);
	AttachWindow(m_StreamTree, ATTB_LEFT, m_SourceFiles);
	AttachWindow(m_StreamTree, ATTB_TOP, m_No_Subtitles, ATTB_BOTTOM, 8);
	
	AttachWindow(m_Cancel_Button, ATTB_RIGHT, m_StatusLine);
	AttachWindow(m_Cancel_Button, ATTB_BOTTOM, m_StreamTree);

	AttachWindow(m_Start_Button, ATTB_RIGHT, m_Cancel_Button, ATTB_LEFT, -4);
	AttachWindow(m_Start_Button, ATTB_BOTTOM, m_Cancel_Button);
	AttachWindow(m_Stop_Button, ATTB_LEFTRIGHT | ATTB_TOPBOTTOM, m_Start_Button);
	AttachWindow(m_StreamTree, ATTB_RIGHT, m_Start_Button, ATTB_LEFT, -8);

	AttachWindow(m_Output_Options_Button, ATTB_BOTTOM, m_Start_Button, ATTB_TOP, -32);
	AttachWindow(m_Output_Options_Button, ATTB_LEFT, m_Start_Button);
	AttachWindow(m_Output_Options_Button, ATTB_RIGHT, m_Cancel_Button);
	AttachWindow(m_Chapter_Editor, ATTB_LEFTRIGHT, m_Output_Options_Button);
	AttachWindow(m_Chapter_Editor, ATTB_BOTTOM, m_Output_Options_Button, ATTB_TOP, -1);

	AttachWindow(m_Audiodelay_Label, ATTB_LEFTRIGHT, m_Output_Options_Button);
	AttachWindow(m_Audiodelay_Label, ATTB_TOP, m_StreamTree);

	AttachWindow(m_Audiodelay, ATTB_TOP, m_Audiodelay_Label, ATTB_BOTTOM, 1);
	AttachWindow(m_Audiodelay, ATTB_LEFT, m_Audiodelay_Label);
	
	AttachWindow(m_Stream_Lng_Label, ATTB_TOP, m_Audiodelay_Label);
	AttachWindow(m_Stream_Lng_Label, ATTB_LEFTRIGHT, m_Audiodelay_Label);
	AttachWindow(m_Stream_Lng, ATTB_TOP, m_Stream_Lng_Label, ATTB_BOTTOM, 1);
	AttachWindow(m_Stream_Lng, ATTB_LEFTRIGHT, m_Stream_Lng_Label);

	
/******************************************/
/* create layout of page 2 of main window */
/******************************************/

	AttachWindow(m_Progress_Group, ATTB_LEFT, *this, border);
	AttachWindow(m_Progress_Group, ATTB_RIGHT, *this, -border);
	AttachWindow(m_Progress_Group, ATTB_TOP, m_Open_Files_Label);

	AttachWindow(m_Progress_List, ATTB_LEFT, m_Progress_Group, 8);
	AttachWindow(m_Progress_List, ATTB_TOP, m_Progress_Group, 16);
	AttachWindow(m_Progress_List, ATTB_RIGHT, m_Progress_Group, -8);

	AttachWindow(m_Prg_Frames, ATTB_RIGHT, m_Progress_List);
	AttachWindow(m_Prg_Frames, ATTB_TOP, m_Progress_List, ATTB_BOTTOM, 8);
	AttachWindow(m_Prg_Frames_Label, ATTB_RIGHT, m_Prg_Frames, ATTB_LEFT, -1);
	AttachWindow(m_Prg_Frames_Label, ATTB_VCENTER, m_Prg_Frames);

	AttachWindow(m_Prg_Progress, ATTB_TOPBOTTOM, m_Prg_Frames);
	AttachWindow(m_Prg_Progress, ATTB_RIGHT, m_Prg_Frames_Label, ATTB_LEFT, -4);
	
	AttachWindow(m_Prg_Progress_Label, ATTB_VCENTER, m_Prg_Progress);
	AttachWindow(m_Prg_Progress_Label, ATTB_LEFT, m_Progress_List, 4);

	AttachWindow(m_Prg_Dest_File_Label, ATTB_LEFT, m_Prg_Progress_Label);
	AttachWindow(m_Prg_Legidx_Label, ATTB_LEFT, m_Prg_Dest_File_Label);

	AttachWindow(m_Prg_Dest_File, ATTB_TOP, m_Prg_Progress, ATTB_BOTTOM, 1);
	AttachWindow(m_Prg_Legidx_Progress, ATTB_TOP, m_Prg_Dest_File, ATTB_BOTTOM, 1);

	AttachWindow(m_Prg_Legidx_Progress, ATTB_LEFT, m_Prg_Legidx_Label, ATTB_RIGHT, 1);
	AttachWindow(m_Prg_Legidx_Progress, ATTB_RIGHT, m_Progress_List);
	AttachWindow(m_Prg_Legidx_Label, ATTB_VCENTER, m_Prg_Legidx_Progress);

	AttachWindow(m_Prg_Dest_File, ATTB_LEFTRIGHT, m_Prg_Legidx_Progress);
	AttachWindow(m_Prg_Dest_File_Label, ATTB_VCENTER, m_Prg_Dest_File);
	
	AttachWindow(m_Prg_Progress, ATTB_LEFT, m_Prg_Legidx_Progress);
	AttachWindow(m_Prg_Progress, ATTB_RIGHT, m_Prg_Frames_Label, ATTB_LEFT, -4);

	AttachWindow(m_Protocol_Label, ATTB_TOP, m_Progress_Group, ATTB_BOTTOM, 8);
	AttachWindow(m_Protocol_Label, ATTB_LEFT, m_Progress_Group);

	AttachWindow(m_Protocol, ATTB_LEFT, m_Protocol_Label);
	AttachWindow(m_Protocol, ATTB_TOP, m_Protocol_Label, ATTB_BOTTOM, 4);
	AttachWindow(m_Protocol, ATTB_BOTTOM, m_Cancel_Button, ATTB_TOP, -8);
	AttachWindow(m_Protocol, ATTB_RIGHT, m_Progress_Group);

	auto_apply_file_title = 1;

	TABHandler_install(hTitleEdit, m_OutputResolution.m_hWnd, false);

	return false;  
}
