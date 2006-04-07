#include "stdafx.h"
#include "AVIMux_GUIDlg.h"
#include "global.h"

void  MovieWindow_Up(CWnd* parent,CWnd* cWnd,DWORD dwHeight)
{
	RECT	rect;

	cWnd->GetWindowRect(&rect);
	parent->ScreenToClient(&rect);
	rect.top-=dwHeight;
	rect.bottom-=dwHeight;
	cWnd->MoveWindow(&rect,true);
}

/*
char* languages [][2] = {
	{"English",""},
	{"German", "ger"}, 
	{"English", "eng"},
	{"Finnish", "fin"},
	{"French", "fre"},
	{"Japanese", "jpn"},
	{"Polish", "pol"},
	{"Portuguese", "por"},

	{"Russian", "rus"},
	{"Spanish", "spa"},
	{"undefined", "und" }
};
*/

BOOL CAVIMux_GUIDlg::OnInitDialog()
{
	char*	Buffer;
	int		i;
	char*	dir;
	char*	lf;
	FILE*	f;

	srand(GetTickCount());
	iUnicode_possible = 0;
	bAddAS_immed = 1;

	OSVERSIONINFOEX	ovi;
	ovi.dwOSVersionInfoSize = sizeof(ovi);
	GetVersionEx((OSVERSIONINFO*)&ovi);
	iUnicode_possible = ovi.dwPlatformId == VER_PLATFORM_WIN32_NT;

	utf8_EnableRealUnicode(!!iUnicode_possible);
	SendDlgItemMessage(IDC_AUDIOTREE, TVM_SETUNICODEFORMAT, iUnicode_possible, 0);
	bEditInProgess = 0;	

	if (!(uiMessage = RegisterWindowMessage("mymessage_1"))) {
		MessageBox("Could not register user-defined window message!","Error",MB_OK | MB_ICONERROR);
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

	Buffer=new char[512];
	dir=new char[512];
	lf=new char[512];

	GetModuleFileName(NULL, dir, 512);
	for (i=lstrlen(dir);i>0;i--)
	{
		if (dir[i]=='\\')
		{
			dir[i]=0;
			i=-1;
		}	
	}

	lstrcpy(cfgfile,dir);
	lstrcat(cfgfile,"\\config.ini");
	lstrcpy(lastjobfile,dir);
	lstrcat(lastjobfile,"\\last-job.amg");

	lstrcpy(lf,dir);
	lstrcat(lf,"\\languages.amg");

	f=fopen(lf,"r");
	if (!f)
	{
		MessageBox("Couldn't open file\n\nlanguages.amg","Error",MB_OK | MB_ICONERROR);
		PostMessage(WM_QUIT);
		return 0;
	}
	fgets(Buffer,200,f);
	Buffer[lstrlen(Buffer)-1]=0;
	if (!strncmp(Buffer,"number",6))
	{
		DWORD dwLngCount = 0;
		dwLanguages=atoi(&(Buffer[7]));
		lplpLanguages=(LANGUAGE_DESCRIPTOR**)new LANGUAGE_DESCRIPTOR[dwLanguages];
		for (i=0;i<(int)dwLanguages;i++)
		{
			lstrcpy(lf,dir);
			lstrcat(lf,"\\");
			ZeroMemory(Buffer,512);
			fgets(Buffer,200,f);
			Buffer[lstrlen(Buffer)-1]=0;
			lstrcat(lf,Buffer);
			if (!(lplpLanguages[dwLngCount]=LoadLanguageFile(lf)))
			{
				wsprintf(Buffer,"Couldn't open language file:\n\n%s\n\nIf you changed the original directory stucture inside the downloaded file, then shame on you! If you use Win 9x/ME, the problem could be a language file using UTF-8 coding. Open the file in Windows Editor and resave it using ANSI coding in that case.",lf);
				MessageBox(Buffer,"Error",MB_OK | MB_ICONERROR);
			} else dwLngCount++;
		}
		if (!dwLngCount) {
			PostMessage(WM_QUIT);
			wsprintf(Buffer,"Couldn't open any language file!");
			MessageBox(Buffer,"Error",MB_OK | MB_ICONERROR);
			return 0;
		}
		dwLanguages = dwLngCount;
	}

	fclose(f);
	SetCurrentLanguage(lplpLanguages[0]);
	cLogFileName[0]=0;
	strcpy(cLogFileName, dir);
	strcat(cLogFileName, "\\AVI-Mux GUI - Logfile - ");

	delete dir;
	delete lf;
	delete Buffer;

	CDialog::OnInitDialog();
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
	GetPrivateProfileString("config","maxsize","2000",Buffer,200,cfgfile);
	sfOptions.dwMaxFileSize=atoi(Buffer);
	GetPrivateProfileString("config","format","%s (%d).avi",Buffer,200,cfgfile);
	sfOptions.lpcNumbering=new char[1+lstrlen(Buffer)];
	lstrcpy(sfOptions.lpcNumbering,Buffer);
	GetPrivateProfileString("config","preload","200",Buffer,200,cfgfile);
	sfOptions.dwPreload=atoi(Buffer);
	GetPrivateProfileString("config","maxframes","0",Buffer,200,cfgfile);
	sfOptions.dwFrames=atoi(Buffer);
	GetPrivateProfileString("config","maxchunksize","0",Buffer,200,cfgfile);
	ofOptions.dwIgnoreSize=atoi(Buffer);
	GetPrivateProfileString("config","maxfiles","2",Buffer,200,cfgfile);
	sfOptions.dwMaxFiles=atoi(Buffer);
	GetPrivateProfileString("config","usemaxfiles","0",Buffer,200,cfgfile);
	sfOptions.dwUseMaxFiles=atoi(Buffer);
	GetPrivateProfileString("config","usenumbering","1",Buffer,200,cfgfile);
	sfOptions.dwDontUseNumbering=!atoi(Buffer);
	GetPrivateProfileString("config","mp3cbrframemode","1",Buffer,200,cfgfile);
	i=GetPrivateProfileInt("config","avoidseekops",1,cfgfile);
	sfOptions.dwUseMaxFileSize=GetPrivateProfileInt("config","usemaxsize",1,cfgfile);

	sfOptions.bDispDoneDlg=true;
	sfOptions.bDispOverwriteDlg=true;
	sfOptions.bExitAfterwards=false;
	sfOptions.iStdOutputFormat=0;

	i=GetPrivateProfileInt("config","noaudio",0,cfgfile);
	CheckDlgButton(IDC_NO_AUDIO,(i==1)?BST_CHECKED:BST_UNCHECKED);
	if (!i)
	{
		i=GetPrivateProfileInt("config","allaudio",1,cfgfile);
		CheckDlgButton(IDC_ALL_AUDIO,(i==1)?BST_CHECKED:BST_UNCHECKED);
		if (!i)
		{
			i=GetPrivateProfileInt("config","defaudio",1,cfgfile);
			CheckDlgButton(IDC_DEFAULT_AUDIO,(i==1)?BST_CHECKED:BST_UNCHECKED);
		}
	}
	GetPrivateProfileString("config","defaudionbr","0",Buffer,200,cfgfile);
	SendDlgItemMessage(IDC_DEFAULT_AUDIO_NUMBER,WM_SETTEXT,0,(LPARAM)Buffer);

	i=GetPrivateProfileInt("config","nosubtitles",0,cfgfile);
	CheckDlgButton(IDC_NO_SUBTITLES,(i==1)?BST_CHECKED:BST_UNCHECKED);
	if (!i)
	{
		i=GetPrivateProfileInt("config","allsubtitles",1,cfgfile);
		CheckDlgButton(IDC_ALL_SUBTITLES,(i==1)?BST_CHECKED:BST_UNCHECKED);
	}


	i=GetPrivateProfileInt("config","openfileoptionsflags",
		SOFO_AVI_REPAIRDX50 | SOFO_M2F2_DOM2F2CRCCHECK | SOFO_MP3_CHECKCBRASK,cfgfile);
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
	m_Enh_Filelist.InsertColumn(0,LoadString(STR_FILELIST_FILENAME),LVCFMT_LEFT,(rect.right-rect.left)/2);
	m_Enh_Filelist.InsertColumn(1,LoadString(STR_FILELIST_MEDIATYPE),LVCFMT_LEFT,(rect.right-rect.left)/10);
	m_Enh_Filelist.InsertColumn(2,LoadString(STR_FILELIST_FORMAT),LVCFMT_LEFT,(rect.right-rect.left)/10);
	m_Enh_Filelist.InsertColumn(3,LoadString(STR_FILELIST_SIZE),LVCFMT_LEFT,(rect.right-rect.left)/10);
// Protokollheader; Protokoll zurechtrücken
	RECT	rect_label,rect_protocol,rect_files,rect_this;
	
	m_SourceFiles.GetWindowRect(&rect_files);
	m_Protocol.GetWindowRect(&rect_protocol);
	m_Protocol_Label.GetWindowRect(&rect_label);
	ScreenToClient(&rect_files);
	ScreenToClient(&rect_protocol);
	ScreenToClient(&rect_label);

	rect_label.left=rect_files.left;
	m_Protocol_Label.MoveWindow(&rect_label);
	rect_protocol.left=rect_files.left;
	m_Protocol.MoveWindow(&rect_protocol);

// Status/Fortschrittsanzeige hochschieben
	m_Open_Files_Label.GetWindowRect(&rect_label);
	m_Progress_Group.GetWindowRect(&rect);
	ScreenToClient(&rect_label);
	ScreenToClient(&rect);

	m_VideoStretchFactor.SetWindowText("1");

	DWORD	dwDeltaH = rect.top - rect_label.top;
	
	MovieWindow_Up(this,&m_Progress_Group,dwDeltaH);
	MovieWindow_Up(this,&m_Prg_Dest_File,dwDeltaH);
	MovieWindow_Up(this,&m_Prg_Dest_File_Label,dwDeltaH);
	MovieWindow_Up(this,&m_Prg_Frames,dwDeltaH);
	MovieWindow_Up(this,&m_Prg_Frames_Label,dwDeltaH);
	MovieWindow_Up(this,&m_Prg_Legidx_Label,dwDeltaH);
	MovieWindow_Up(this,&m_Prg_Legidx_Progress,dwDeltaH);
	MovieWindow_Up(this,&m_Prg_Progress,dwDeltaH);
	MovieWindow_Up(this,&m_Prg_Progress_Label,dwDeltaH);
	MovieWindow_Up(this,&m_Progress_List,dwDeltaH);
	
	RECT r;
	m_AudioName.GetWindowRect(&r);
	ScreenToClient(&r);
	r.bottom+=100;
	m_Stream_Lng.MoveWindow(&r);

	for (i=0;i<sizeof(languages)/sizeof(char*[2]);i++) {
		Buffer[0]=0;
		sprintf(Buffer,"%s - %s",languages[i][0],languages[i][1]);
		m_Stream_Lng.SetItemData(m_Stream_Lng.AddString(Buffer),(LPARAM)languages[i][1]);
	}

	free (Buffer);

	m_Start_Button.GetWindowRect(&rect);
	GetWindowRect(&rect_this);

	rect_this.bottom=rect.bottom+rect.bottom-rect.top+20;

	MoveWindow(&rect_this,true);
// protocole listbox headers
	m_Prg_Dest_File.GetWindowRect(&rect);
	dwDeltaH=abs(rect.top-rect.bottom);
	m_Protocol.GetWindowRect(&rect);
	m_Protocol.InsertColumn(0,"Time",LVCFMT_CENTER,(rect.right-rect.left)/7);
	m_Protocol.InsertColumn(1,"Message",LVCFMT_LEFT,(rect.right-rect.left)*6/7-2-dwDeltaH);

// create font for protocol
	CFont *font;
	font = GetFont();
	LOGFONT logfont;
	font->GetLogFont(&logfont);

	font = new CFont;

	font->CreateFont(-11*logfont.lfHeight/8,0,0,0,500,false,false,false,DEFAULT_CHARSET,OUT_TT_PRECIS,
		CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,FF_ROMAN,"Microsoft Sans Serif");
// chapters
	chapters = new CChapters();

	m_Protocol.SetFont(font);

	font = new CFont;
	font->CreateFont(-11*logfont.lfHeight/8,0,0,0,500,false,false,false,DEFAULT_CHARSET,OUT_TT_PRECIS,
		CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,FF_ROMAN,"Microsoft Sans Serif");
	m_AudioTree.SetFont(font);
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
		if (!strAboutMenu.IsEmpty())
		{	
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}


	char*	ncfgfn;
	newz(char, 10+strlen(cfgfile), ncfgfn);
	//ncfgfn=(char*)calloc(1,strlen(cfgfile)+10);
	strcpy(ncfgfn,cfgfile);
	strcat(ncfgfn,".amg");
	PostMessage(GetUserMessageID(),IDM_DOADDFILE,(LPARAM)ncfgfn);
	sfOptions.bB0rk = false;
	sfOptions.i1stTimecode = 0;

	if (__argc>1)
	{
		for (i=1;i<__argc;i++)
		{
			if (!stricmp(__argv[i],"-b0rk")) {
				sfOptions.bB0rk = true;
			} else {
				char* f2load;// = (char*)calloc(1,1+strlen(__argv[i]));
				newz(char,1+strlen(__argv[i]), f2load);
				strcpy(f2load,__argv[i]);
				PostMessage(GetUserMessageID(),IDM_DOADDFILE,(LPARAM)f2load);
			}
		}
	}

	chapter_level = 0;
	chap[0] = chapters;
	sfOptions.chapters = chapters;
	SetDialogState_Config();
	sfOptions.split_points = new CSplitPoints;
	sfOptions.iActiveButton = 0;

	settings = new CAttribs;
	settings->SetInt("input/audio/mp3/check_cbr",23);
	settings->SetInt("input/use cache", 1);
	settings->SetInt("input/unbuffered", 1);

	settings->SetInt("output/mkv/ac3/frames per block", 1);
	settings->SetInt("output/mkv/displaywidth_height", 1);
	settings->SetInt("output/mkv/clusters/size", 512);
	settings->SetInt("output/mkv/clusters/time", 30000);
	settings->SetInt("output/mkv/clusters/prevclustersize", 1);
	settings->SetInt("output/mkv/clusters/position", 1);
	settings->SetInt("output/mkv/clusters/limit first", 1);
	settings->SetInt("output/mkv/clusters/index/on", 1);

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

	settings->SetInt("output/mkv/force v1.0",1);
	settings->SetInt("output/mkv/floats/width", 32);

	settings->SetInt("output/mkv/cues/on",1);
	settings->SetInt("output/mkv/cues/video/on",1);
	settings->SetInt("output/mkv/cues/audio/on",1);
	settings->SetInt("output/mkv/cues/audio/only audio-only/on",0);
	settings->SetInt("output/mkv/TimecodeScale/mkv",500000);
	settings->SetInt("output/mkv/TimecodeScale/mka",10000);
	settings->SetInt("output/mkv/2nd Tracks",1);
	settings->SetInt("output/mkv/randomize element order", 1);

	settings->SetInt("output/avi/opendml/riff avi size", 1);
	settings->SetInt("output/avi/opendml/on", 1);
	settings->SetInt("output/avi/opendml/haalimode", 0);
	settings->SetInt("output/avi/opendml/stdindex/pattern", SIP_AUTO);
	settings->SetInt("output/avi/opendml/stdindex/interval", 15000);
	settings->SetInt("output/avi/audio interleave/unit", AIU_KB);
	settings->SetInt("output/avi/audio interleave/value", 100);

	settings->SetInt("output/avi/ac3/frames per chunk", 2);
	settings->SetInt("output/avi/dts/frames per chunk", 2);
	settings->SetInt("output/avi/mp3/frames per chunk", 1);
	settings->SetInt("output/avi/mp3/cbr frame mode", 1);
	settings->SetInt("output/avi/reclists", 1);
	settings->SetInt("output/avi/legacyindex", 1);
	settings->SetInt("output/general/logfile/on",0);
	settings->SetInt("output/general/unbuffered", 0);
	settings->SetInt("output/general/overlapped", 0);

	settings->SetInt("output/ogg/pagesize", 65025);

	hLogFile = NULL;

	char title[100]; title[0]=0; GetWindowText(title,100); CString c;
	c.LoadString(IDS_VERSION_INFO);
	strcat(title, " ");
	strcat(title, c);
	SetWindowText(title);

	m_Add_Video_Source.EnableWindow(0);

	return false;  // Geben Sie TRUE zurück, außer ein Steuerelement soll den Fokus erhalten
}
