// VideoInformationDlg.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "AVIMux_GUI.h"
#include "AVIMux_GUIDlg.h"
#include "VideoInformationDlg.h"
#include "VideoInformationDlgListbox.h"
#include "SetMainAVIHeaderFlagsDlg.h"
#include "windows.h"
#include "..\basestreams.h"
#include "Languages.h"
#include "FormatText.h"
#include "RIFFChunkTreeDlg.h"
#include "EBMLTreeDlg.h"
#include "Trees.h"
#include "..\matroska.h"
#include "..\utf-8.h"
#include "UnicodeTreeCtrl.h"
#include "ResizeableDialog.h"
#include ".\videoinformationdlg.h"
#include "Version.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CVideoInformationDlg 


CVideoInformationDlg::CVideoInformationDlg(CWnd* pParent /*=NULL*/)
	: CResizeableDialog(CVideoInformationDlg::IDD, pParent)
{
	EnableAutomation();
	dwKinfOfSource=0;

	//{{AFX_DATA_INIT(CVideoInformationDlg)
	//}}AFX_DATA_INIT
}


void CVideoInformationDlg::OnFinalRelease()
{
	// Nachdem die letzte Referenz auf ein Automatisierungsobjekt freigegeben wurde,
	// wird OnFinalRelease aufgerufen. Die Basisklasse löscht das Objekt
	// automatisch. Fügen Sie zusätzlichen Bereinigungscode für Ihr Objekt
	// hinzu, bevor Sie die Basisklasse aufrufen.

	CResizeableDialog::OnFinalRelease();
}

void CVideoInformationDlg::DoDataExchange(CDataExchange* pDX)
{
	CResizeableDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CVideoInformationDlg)
	DDX_Control(pDX, IDC_APPLYREPAIRS, m_Apply_Changes);
	DDX_Control(pDX, IDOK, m_OK);
	DDX_Control(pDX, IDC_SAVETREE, m_SaveTree_Button);
	DDX_Control(pDX, IDC_TREE1, m_Tree);
	DDX_Control(pDX, IDC_BUTTON1, m_BuildRIFFTree);
	DDX_Control(pDX, IDE_VIDEOINFORMATION, m_VideoInformationDlgListbox);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CVideoInformationDlg, CResizeableDialog)
	//{{AFX_MSG_MAP(CVideoInformationDlg)
	ON_BN_CLICKED(IDC_APPLYREPAIRS, OnApplyrepairs)
	ON_BN_CLICKED(IDC_BUTTON1, OnRIFFChunkTree)
	ON_BN_CLICKED(IDC_SAVETREE, OnSavetree)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
END_MESSAGE_MAP()

BEGIN_DISPATCH_MAP(CVideoInformationDlg, CResizeableDialog)
	//{{AFX_DISPATCH_MAP(CVideoInformationDlg)
		// HINWEIS - Der Klassen-Assistent fügt hier Zuordnungsmakros ein und entfernt diese.
	//}}AFX_DISPATCH_MAP
END_DISPATCH_MAP()

// Hinweis: Wir stellen Unterstützung für IID_IVideoInformationDlg zur Verfügung, um typsicheres Binden
//  von VBA zu ermöglichen. Diese IID muss mit der GUID übereinstimmen, die in der
//  Disp-Schnittstelle in der .ODL-Datei angegeben ist.

// {B0D1411A-21A1-4FB5-BDDC-A6ECF0404B06}
static const IID IID_IVideoInformationDlg =
{ 0xb0d1411a, 0x21a1, 0x4fb5, { 0xbd, 0xdc, 0xa6, 0xec, 0xf0, 0x40, 0x4b, 0x6 } };

BEGIN_INTERFACE_MAP(CVideoInformationDlg, CResizeableDialog)
	INTERFACE_PART(CVideoInformationDlg, IID_IVideoInformationDlg, Dispatch)
END_INTERFACE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CVideoInformationDlg 


#define LBE_DWFOURCC		0x00000001
#define LBE_BICOMPRESSION	0x00000002
#define LBE_FR_AVIH			0x00000004
#define LBE_FR_STRF			0x00000008
#define LBE_FR_ALL			0x0000000C
#define LBE_FR_ODML_DWDUR	0x00000010
#define LBE_TOTALFRAMES     0x00000020

DWORD CVideoInformationDlg::GetKindOfSource()
{
	return dwKinfOfSource;
}

static const char* YN[2] = { "no", "yes" };

bool CVideoInformationDlg::RenderChapters(CUnicodeTreeCtrl* cTree,HTREEITEM hParent,CHAPTERS* chapters)
{
//	return false;

	size_t i,j;
	HTREEITEM hChapters[2] = { 0, 0 };
	char	cLng[16]; cLng[0]=0;
	char	cStr[2048]; cStr[0]=0;
	char	buffer[2048];

	if (chapters) for (i=0;i<chapters->iCount;i++) {

		char	time[2][20];
		memset(buffer, 0, sizeof(buffer));
		memset(time, 0, sizeof(time));
		Millisec2Str(chapters->chapters[i]->iTimestart/1000000,time[0]);
		Millisec2Str(chapters->chapters[i]->iTimeend/1000000,time[1]);
		int		iDC = chapters->chapters[i]->display.size();
		CHAPTER_INFO* c = chapters->chapters[i];

		cLng[0]=0;
		cStr[0]=0;
		if (chapters->chapters[i]->display.size()) {
			strcpy(cLng, c->display[0].cLanguage->AsString());
			strcpy(cStr, c->display[0].cString->AsString());
		}

		char cUID[32]; memset(cUID, 0, sizeof(cUID));
		__int642hex(c->iUID, cUID, 4);

		if (c->bEdition) {

			sprintf(buffer,"Edition %d (%s, %s, %s, UID: 0x%s)", i+1,
				(c->bDefault?"default":"not default"),
				(c->bHidden?"hidden":"not hidden"),
				(c->bOrdered?"ordered":"not ordered"),
				cUID);
		} else {
			if (c->iTimeend != -1) 
				sprintf(buffer,"Chapter %2d: %s - %s (UID: 0x%s)",i+1,time[0],time[1],cUID);
			else
				sprintf(buffer,"Chapter %2d: %s (UID: 0x%s)",i+1,time[0],cUID);				
			
		}
		
		hChapters[0] = Tree_Insert(cTree,buffer,hParent);
		hChapters[1] = 0;
	
		AddTags(c->pTags, hChapters[0]);
		if (iDC>0) 
			for (j=0;j<c->display.size();j++) {
				char* buffer = (char*)calloc(1, 2048);
				strncpy(cLng, c->display[j].cLanguage->AsString(), 14);
				strncpy(cStr, c->display[j].cString->AsString(), 2000);

				sprintf(buffer,"%s: %s", 
					cLng, cStr);

				printf("%s\n", buffer);
				Tree_Insert(cTree, buffer, hChapters[0]);
				free(buffer);
				
		}
		

		RenderChapters(cTree,hChapters[0],(CHAPTERS*)chapters->chapters[i]->subchapters);
		
		if (!hChapters[1])
			hChapters[1] = hChapters[0];

		cTree->PostMessage(TVM_EXPAND, TVE_EXPAND, (LPARAM)hChapters[0]);
		cTree->PostMessage(TVM_EXPAND, TVE_EXPAND, (LPARAM)hChapters[1]);

	}

	return true;
}

void CVideoInformationDlg::AddTags(TAG_INDEX_LIST& tags, HTREEITEM hParent)
{
	char buffer[4096]; buffer[0]=0;
	if (!tags.empty()) {
		sprintf(buffer,"%-20s","Tags");
		HTREEITEM hTags = Tree_Insert(&m_Tree,buffer,hParent);
		for (size_t z=0;z<tags.size();z++) {
			char cName[4096]; cName[0]=0;
			char cValue[4096]; cValue[0]=0;
			char cLng[64]; cLng[0]=0;
			mkvfile->GetTag(tags[z], cName, cValue, cLng);
			if (!strcmp(cLng,"und")) 
				sprintf(buffer,"%-20s: %s",cName,cValue);
			else 
				sprintf(buffer,"%-20s: %s (Language: %s)",cName,cValue,cLng);
			Tree_Insert(&m_Tree,buffer,hTags);
		}
		m_Tree.Expand(hTags, TVE_EXPAND);
	}
}

bool mb(int i)
{
	MessageBox(NULL, "info", "info", MB_OK);
	return false;
}

bool CVideoInformationDlg::InitDialog_Matroska()
{
	char		size[1024];
	char		buffer[1024];
	CString		cStr[2];
	int			i, j;
	HTREEITEM	hSegments,hSegment,hTracks,hTrack,hChapters,hTrackType,hCodecID;

	memset(buffer, 0, sizeof(buffer));
	memset(size, 0, sizeof(size));

	m_Tree.InitUnicode();
	SetWindowText(LoadString(STR_VID_TITLE_FILE));
	(CButton*)GetDlgItem(IDC_APPLYREPAIRS)->EnableWindow(false);

//	m_BuildRIFFTree.SetWindowText("EBML-Tree");
	m_Tree.ShowWindow(SW_SHOW);
	m_VideoInformationDlgListbox.ShowWindow(SW_HIDE);
	m_BuildRIFFTree.ShowWindow(SW_HIDE);

	CVideoInformationDlgListbox*	clb;
	clb=&m_VideoInformationDlgListbox;

	char version[32];
	GetAMGVersionString(version, sizeof(version));
	sprintf(buffer,"%-20s: %s, %s",LoadString(IDS_VERSION), version, GetAMGVersionDate());
	Tree_Insert(&m_Tree,buffer,NULL);

	sprintf(buffer,"%-20s: %s","file type","matroska");
	Tree_Insert(&m_Tree,buffer);

	FormatSize(size,mkvfile->GetSize());
	sprintf(buffer,"%-20s: %s","file size",size);
	Tree_Insert(&m_Tree,buffer);

	sprintf(buffer,"%-20s: %d","number of segments",mkvfile->GetSegmentCount());
	hSegments=Tree_Insert(&m_Tree,buffer);

	int active_seg = mkvfile->GetActiveSegment();

	for (i=0;i<mkvfile->GetSegmentCount();i++) {
		mkvfile->SetActiveSegment(i);
		sprintf(buffer,"segment %d",i);
		hSegment = Tree_Insert(&m_Tree,buffer,hSegments);

		Millisec2Str(mkvfile->GetSegmentDuration()*mkvfile->GetTimecodeScale()/1000000,size);
		sprintf(buffer,"%-20s: %s","duration",size);
		Tree_Insert(&m_Tree,buffer,hSegment);

		char* WritingApp = mkvfile->GetSegmentWritingApp();
		if (strcmp(WritingApp,"")) {
			sprintf(buffer,"%-20s: %s","writing app",WritingApp);
		} else {
			sprintf(buffer,"%-20s: %s","writing app","n/a !!!");
		}
		Tree_Insert(&m_Tree,buffer,hSegment);

		char* MuxingApp = mkvfile->GetSegmentMuxingApp();
		if (strcmp(MuxingApp,"")) {
			sprintf(buffer,"%-20s: %s","muxing app",MuxingApp);
		} else {
			sprintf(buffer,"%-20s: %s","muxing app","n/a !!!");
		}
		Tree_Insert(&m_Tree,buffer,hSegment);

		char* SegmentTitle = mkvfile->GetSegmentTitle();
		if (SegmentTitle && strcmp(SegmentTitle,"")) {
			sprintf(buffer,"%-20s: %s","segment title",SegmentTitle);
			Tree_Insert(&m_Tree,buffer,hSegment);
		}

		for (int uid_type = 0; uid_type < 4; uid_type++) {
			char* UID = mkvfile->GetUID(uid_type);
			if (UID && strcmp(UID,"")) {
				memset(buffer, 0, sizeof(buffer));
				sprintf(buffer,"%-20s: ",MKV_UID_NAMES[uid_type]);
				__int128hex(UID, buffer+strlen(buffer), 1);
				Tree_Insert(&m_Tree,buffer,hSegment);
			}
		}

		QW2Str(mkvfile->GetTimecodeScale(), size, 1);
//		sprintf(size, "%d", mkvfile->GetTimecodeScale()*100);
//		MessageBox(size, "Info", MB_OK);
		sprintf(buffer,"%-20s: %s","Timecode Scale",size);
//		MessageBox(buffer, "Info", MB_OK);
		Tree_Insert(&m_Tree,buffer,hSegment);


		sprintf(buffer,"%-20s: %d","number of tracks",mkvfile->GetTrackCount());
		hTracks = Tree_Insert(&m_Tree,buffer,hSegment);

//		return 0;
		for (j=0;j<mkvfile->GetTrackCount();j++) {
			mkvfile->SetActiveTrack(j);
			sprintf(buffer,"track %d",mkvfile->GetTrackNumber());
			hTrack = Tree_Insert(&m_Tree,buffer,hTracks);

			sprintf(buffer,"%-20s: %s", "track type",MSTRT_names[mkvfile->GetTrackType()]);
			hTrackType = Tree_Insert(&m_Tree,buffer,hTrack);

			__int64 duration = mkvfile->GetTrackDuration(j);
			if (duration != TIMECODE_UNKNOWN) duration = duration * mkvfile->GetTimecodeScale() / 1000000;
			if (duration == TIMECODE_UNKNOWN) {
				sprintf(buffer,"%-20s: %s", "duration","not determined (no cuepoint found for this track)");
			} else {
				Millisec2Str(duration, size);
				sprintf(buffer,"%-20s: %s", "duration",size);
			}
			Tree_Insert(&m_Tree,buffer,hTrack);

			char* CodecID = mkvfile->GetCodecID();
			if (strcmp(CodecID,"")) {
				sprintf(buffer,"%-20s: %s","CodecID",CodecID);
				hCodecID = Tree_Insert(&m_Tree,buffer,hTrackType);

				if (!strcmp(CodecID,"V_MS/VFW/FOURCC")) {
					BITMAPINFOHEADER* bmi = (BITMAPINFOHEADER*)mkvfile->GetCodecPrivate();
					char	FourCC[5];
					*(int*)FourCC = bmi->biCompression;
					FourCC[4] = 0;
					sprintf(buffer,"%-20s: %s","biCompression",FourCC);
					Tree_Insert(&m_Tree,buffer,hCodecID);
					if (bmi->biBitCount) {
						sprintf(buffer,"%-20s: %d","biBitcount",bmi->biBitCount);
						Tree_Insert(&m_Tree,buffer,hCodecID);
					}
				}
				if (!strcmp(CodecID,"A_MS/ACM")) {
					WAVEFORMATEX* wfe = (WAVEFORMATEX*)mkvfile->GetCodecPrivate();
					sprintf(buffer,"%-20s:    0x%04X","wFormatTag",wfe->wFormatTag);
					Tree_Insert(&m_Tree,buffer,hCodecID);

					QW2Str(wfe->nAvgBytesPerSec,size,9);
					sprintf(buffer,"%-20s: %s","nAvgBytesPerSec",size);
					Tree_Insert(&m_Tree,buffer,hCodecID);

					if (wfe->wBitsPerSample) {
						sprintf(buffer,"%-20s: %9d","wBitsPerSample",wfe->wBitsPerSample);
						Tree_Insert(&m_Tree,buffer,hCodecID);
					}

					if (wfe->nSamplesPerSec) {
						sprintf(buffer,"%-20s: %9d","nSamplesPerSec",wfe->nSamplesPerSec);
						Tree_Insert(&m_Tree,buffer,hCodecID);
					}

					if (wfe->nChannels) {
						sprintf(buffer,"%-20s: %9d","nChannels",wfe->nChannels);
						Tree_Insert(&m_Tree,buffer,hCodecID);
					}

					if (wfe->nBlockAlign) {
						sprintf(buffer,"%-20s: %9d","nBlockAlign",wfe->nBlockAlign);
						Tree_Insert(&m_Tree,buffer,hCodecID);
					}
				}

			}
			m_Tree.Expand(hTrackType,TVE_EXPAND);
			m_Tree.Expand(hCodecID,TVE_EXPAND);

			switch (mkvfile->GetTrackType()) {
				case MSTRT_VIDEO: 
					int	dwX1,dwX2,dwY1,dwY2,dwDU;
					mkvfile->GetResolution(&dwX1,&dwY1,&dwX2,&dwY2,&dwDU);
					RECT crop;
					mkvfile->GetCropping(&crop);
					sprintf(buffer,"Resolution (Pixels) : %dx%d",dwX1,dwY1);
					Tree_Insert(&m_Tree,buffer,hTrackType);
					sprintf(buffer,"Cropped to          : (%dx%d)-(%dx%d)",crop.left, crop.top,
						dwX1-crop.right, dwY1-crop.bottom);
					Tree_Insert(&m_Tree,buffer,hTrackType);
					sprintf(buffer,"Resolution (Display): %dx%d %s",dwX2,dwY2,MDISPU_names[dwDU]);
					Tree_Insert(&m_Tree,buffer,hTrackType);

					break;
				case MSTRT_AUDIO:
					if (mkvfile->GetSamplingFrequency()) {
						sprintf(buffer,"%-20s: %d Hz","sampling frequency",(int)mkvfile->GetSamplingFrequency());
						Tree_Insert(&m_Tree,buffer,hTrackType);
					}
			
					if (mkvfile->GetChannelCount()) {
						sprintf(buffer,"%-20s: %d","channels",(int)mkvfile->GetChannelCount());
						Tree_Insert(&m_Tree,buffer,hTrackType);
					}

					if (mkvfile->GetBitDepth()) {
						sprintf(buffer,"%-20s: %d","bit depth",(int)mkvfile->GetBitDepth());
						Tree_Insert(&m_Tree,buffer,hTrackType);
					}
					break;
			}

			char* track_name = mkvfile->GetTrackName();
			if (strcmp(track_name,"")) {
				sprintf(buffer,"%-20s: %s","name",track_name);
				Tree_Insert(&m_Tree,buffer,hTrack);
			}


			__int64 track_uid;
			char cTrackUID[32]; memset(cTrackUID, 0, sizeof(cTrackUID));
			track_uid = mkvfile->GetTrackUID();
			__int642hex(track_uid, cTrackUID, 4);
			sprintf(buffer,"%-20s: 0x%s","UID",cTrackUID);
			Tree_Insert(&m_Tree,buffer,hTrack);

			sprintf(buffer,"%-20s: %s","lacing",YN[mkvfile->IsLaced()]);
			Tree_Insert(&m_Tree,buffer,hTrack);

			if (mkvfile->IsBitrateIndicated()) {
				QW2Str((__int64)((mkvfile->GetTrackBitrate()+500)/1000),size,10);
				sprintf(buffer, "%-20s: %s kbps",LoadString(IDS_VI_BITRATE),size);
				Tree_Insert(&m_Tree,buffer,hTrack);
			}

			sprintf(buffer,"%-20s: %s","default",YN[mkvfile->IsDefault()]);
			Tree_Insert(&m_Tree,buffer,hTrack);

			sprintf(buffer,"%-20s: %s","enabled",YN[mkvfile->IsEnabled()]);
			Tree_Insert(&m_Tree,buffer,hTrack);

			QW2Str(mkvfile->GetDefaultDuration(),size,10);
			sprintf(buffer,"%-20s: %s ns","default duration",size);
			Tree_Insert(&m_Tree,buffer,hTrack);

			sprintf(buffer,"%-20s: %s","language",mkvfile->GetLanguage());
			Tree_Insert(&m_Tree,buffer,hTrack);

			if (mkvfile->GetTrackCompressionDescriptorCount(j) == 1) {

				if (mkvfile->GetTrackCompression(j, 0) != COMPRESSION_NONE) {
					char* compr = "";
					if (mkvfile->GetTrackCompression(j, 0) == COMPRESSION_ZLIB) 
						compr = "zlib";
					if (mkvfile->GetTrackCompression(j, 0) == COMPRESSION_HDRSTRIPING) 
						compr = "header striping";

					sprintf(buffer,"%-20s: %s","compression",compr);
					Tree_Insert(&m_Tree,buffer,hTrack);
				}

			}
			AddTags(mkvfile->GetTrackTags(), hTrack);

			m_Tree.Expand(hTrack,TVE_EXPAND);
		}
		m_Tree.Expand(hTracks,TVE_EXPAND);

		if (mkvfile->GetChapterInfo()) {
			RenderChapters(&m_Tree,hChapters=Tree_Insert(&m_Tree,"chapters",hSegment),mkvfile->GetChapterInfo());
			m_Tree.Expand(hChapters,TVE_EXPAND);
		}

		if (mkvfile->GetAttachmentCount()) {
			HTREEITEM hAtts = Tree_Insert(&m_Tree,"attachments",hSegment);

			for (int j=0;j<mkvfile->GetAttachmentCount();j++) {
				char fn[1024]; fn[0]=0;
				mkvfile->GetAttachmentFilename(j, fn, sizeof(fn));

				sprintf(buffer,"%s %d: %s","Attachment", j+1, fn);
				HTREEITEM hAtt = Tree_Insert(&m_Tree, buffer, hAtts);			

				char csize[32]; csize[0]=0;
				QW2Str(mkvfile->GetAttachmentFileSize(j), csize, 1);
				sprintf(buffer, "%-20s: %s %s", "size", csize, LoadString(STR_BYTES));
				Tree_Insert(&m_Tree,buffer,hAtt);

				char cuid[32]; cuid[0]=0;
				__int642hex(mkvfile->GetAttachmentUID(j), cuid, 4, 1, 0);
		
				sprintf(buffer,"%-20s: 0x%s","UID",cuid);
				Tree_Insert(&m_Tree,buffer,hAtt);

				char descr[1024]; descr[0]=0;
				mkvfile->GetAttachmentFileDescription(j, descr, sizeof(descr));
				sprintf(buffer,"%-20s: %s","Description",descr);
				Tree_Insert(&m_Tree,buffer,hAtt);

				char mime[1024]; mime[0]=0;
				mkvfile->GetAttachmentMimeType(j, mime, sizeof(mime));
				sprintf(buffer,"%-20s: %s","Mime-Type",mime);
				Tree_Insert(&m_Tree,buffer,hAtt);

				char binary[1024];
				mkvfile->GetAttachmentBinaryData(j, binary, sizeof(binary));

			}

			m_Tree.Expand(hAtts,TVE_EXPAND);
		}

		AddTags(mkvfile->GetGlobalTags(), hSegment);
		m_Tree.Expand(hSegment,TVE_EXPAND);

	}

	mkvfile->SetActiveSegment(active_seg);
	m_Tree.Expand(hSegments,TVE_EXPAND);


	return 0;
}

bool CVideoInformationDlg::InitDialog_VideoSource()
{
	CVideoInformationDlgListbox*	clb;
	CString	cStr[5];
	char*	lpDest;
	int		x,y;
	DWORD	dwFourCC,dwChunkCount,dwMillisec;
	double	framerate1,dSeconds;
	char	buffer[200];
	

	SetWindowText(LoadString(STR_VID_TITLE_VIDEOSOURCE));
	(CButton*)GetDlgItem(IDC_APPLYREPAIRS)->EnableWindow(false);
	m_Tree.ShowWindow(SW_HIDE);
	m_VideoInformationDlgListbox.ShowWindow(SW_SHOW);

	lpDest=new char[10240];
	clb=&m_VideoInformationDlgListbox;
	cStr[0]=LoadString(IDS_VERSION);
	
	char version[32];
	GetAMGVersionString(version, sizeof(version));
	sprintf(lpDest,"%s: %s, %s",cStr[0].GetBuffer(255), version,
		GetAMGVersionDate());
	clb->AddString(lpDest);

	cStr[0]=LoadString(IDS_VI_RESOLUTION);
	lpVS->GetResolution(&x,&y);
	sprintf(lpDest,"%-30s: %dx%d",cStr[0].GetBuffer(255),x,y);
	clb->AddString(lpDest);

	dwFourCC=lpVS->GetFourCC();
	sprintf(lpDest,"%-30s: %c%c%c%c","FourCC",(dwFourCC)&0xFF,(dwFourCC>>8)&0xFF,(dwFourCC>>16)&0xFF,(dwFourCC>>24)&0xFF);
	clb->AddString(lpDest);

	cStr[0]=LoadString(IDS_VI_FRAMERATE);
	framerate1=1000000000/(double)lpVS->GetNanoSecPerFrame();
	_gcvt(framerate1,6,buffer);
	sprintf(lpDest,"%s (strh): %s fps",cStr[0].GetBuffer(255),buffer);
	clb->AddString(lpDest);

	cStr[0]=LoadString(IDS_VI_NBROFFRAMES);
	x=lpVS->GetNbrOfFrames();
	dwChunkCount=x;
	dSeconds=((double)x/(double)framerate1);
	dwMillisec=(DWORD)(round(1000*double(x)/(double)framerate1)%1000);
	sprintf(buffer,"%d:%02d:%02d.%03d",((DWORD)dSeconds/3600),((DWORD)dSeconds%3600)/60,((DWORD)dSeconds%60),dwMillisec);
	sprintf(lpDest,"%-30s:%10d (%s)",cStr[0].GetBuffer(255),x,buffer);
	clb->AddString(lpDest);

	cStr[0]=LoadString(IDS_VI_KEYFRAMES);
	sprintf(lpDest,"%-30s:%10d",cStr[0].GetBuffer(255),lpVS->GetNbrOfFrames(FT_KEYFRAME));
	clb->AddString(lpDest);
	cStr[0]=LoadString(IDS_VI_DELTAFRAMES);
	sprintf(lpDest,"%-30s:%10d",cStr[0].GetBuffer(255),lpVS->GetNbrOfFrames(FT_DELTAFRAME));
	clb->AddString(lpDest);
	cStr[0]=LoadString(IDS_VI_DROPPEDFRAMES);
	sprintf(lpDest,"%-30s:%10d",cStr[0].GetBuffer(255),lpVS->GetNbrOfFrames(FT_DROPPEDFRAME));
	clb->AddString(lpDest);

	clb->AddString("");

	cStr[0]=LoadString(IDS_VI_SIZEOFVIDEO);
	cStr[1]=LoadString(IDS_MBYTE);

	sprintf(lpDest,"%-30s: %d %s",cStr[0].GetBuffer(255),DWORD(lpVS->GetSize()>>20),cStr[1].GetBuffer(255));
	clb->AddString(lpDest);
	
	cStr[0]=LoadString(IDS_VI_VIDEODATARATE);
	cStr[1]=LoadString(STR_KBYTE);
	sprintf(lpDest,"%-30s: %d %s/s",cStr[0].GetBuffer(255),DWORD(lpVS->GetSize()/dSeconds)>>10,cStr[1].GetBuffer(255));
	clb->AddString(lpDest);

	delete lpDest;

	return true;
}

BOOL CVideoInformationDlg::OnInitDialog() 
{
	static const sizes[13]={100,250,500,1000,2500,5000,10000,25000,50000,100000,200000,500000,1000000};

	char*	lpDest;
	DWORD	dwChunkSizes[14],dwSize;
	CString	cStr[5];
	int		x,y;
	DWORD	dwFourCC;
	double	framerate1,framerate2;
	char	buffer[100];
	char	nbr1[30],nbr2[30];
	DWORD	dwMillisec;
	double  dSeconds;
	DWORD	dwAudioStreams;
	DWORD	dwSubtitles;
	DWORD	dwNbrOfKFrames=0;
	int		i,j,k;
	DWORD	dwChunkCount;
	READSUPERINDEXPROTOCOL*	lpRSIP;
	CVideoInformationDlgListbox*	clb;
	bool	bNeeddwDurationRepair=false;
	DWORD	dw1,dw2;


	WAVEFORMATEX* strf;
	AVIStreamHeader* strh;

	CResizeableDialog::OnInitDialog();
	m_BuildRIFFTree.ShowWindow(SW_HIDE);

	m_OK.SetWindowText(LoadString(STR_GEN_OK));
	m_Apply_Changes.SetWindowText(LoadString(STR_VID_APPLYCHANGES));

	AttachWindow(m_OK, ATTB_BOTTOM, m_hWnd, -10);
	AttachWindow(m_Apply_Changes, ATTB_BOTTOM, m_OK.m_hWnd);
	AttachWindow(m_BuildRIFFTree, ATTB_BOTTOM, m_Apply_Changes);
	AttachWindow(m_SaveTree_Button, ATTB_BOTTOM, m_BuildRIFFTree);
	AttachWindow(m_SaveTree_Button, ATTB_RIGHT, m_hWnd, -16);
	AttachWindow(m_BuildRIFFTree, ATTB_RIGHT, m_SaveTree_Button, ATTB_LEFT, -16);

	AttachWindow(m_Apply_Changes, ATTB_LEFT, m_OK, ATTB_RIGHT, 16);

	int w, h;

	GetBorder(w, h);

	AttachWindow(m_VideoInformationDlgListbox, ATTB_BOTTOM, m_OK, ATTB_TOP, -16);
	AttachWindow(m_VideoInformationDlgListbox, ATTB_LEFT, m_hWnd, 16);
	AttachWindow(m_VideoInformationDlgListbox, ATTB_RIGHT, m_hWnd, -16);
	AttachWindow(m_VideoInformationDlgListbox, ATTB_TOP, m_hWnd, h);

	AttachWindow(m_Tree.m_hWnd, ATTB_TOPBOTTOM, m_VideoInformationDlgListbox);
	AttachWindow(m_Tree.m_hWnd, ATTB_LEFTRIGHT, m_VideoInformationDlgListbox);


	if (dwKinfOfSource==KOS_VIDEOSOURCE)	{
		return InitDialog_VideoSource();
	}
	if (dwKinfOfSource==KOS_MATROSKA) {
		return InitDialog_Matroska();
		m_SaveTree_Button.ShowWindow(1);
	}

	m_SaveTree_Button.ShowWindow(0);
	m_BuildRIFFTree.SetWindowText("RIFF-Tree");
	m_Tree.ShowWindow(SW_HIDE);
	m_VideoInformationDlgListbox.ShowWindow(SW_SHOW);

	SetWindowText(LoadString(STR_VID_TITLE_FILE));
	(CButton*)GetDlgItem(IDC_APPLYREPAIRS)->EnableWindow(true);

	ZeroMemory(dwChunkSizes,sizeof(dwChunkSizes));
	lpDest=new char[10240];
	clb=&m_VideoInformationDlgListbox;

	clb->SetFile(avifile);
	
	cStr[0]=LoadString(IDS_VERSION);
	
	char version[32];
	GetAMGVersionString(version, sizeof(version));
	sprintf(lpDest,"%s: %s, %s",cStr[0].GetBuffer(255), version,
		GetAMGVersionDate());
	clb->AddString(lpDest);
	clb->AddString("-----------------------------------------------");

	cStr[0]=LoadString(IDS_VI_AVITYPE);
	cStr[1]=LoadString(IDS_VI_STANDARD);
	cStr[2]=LoadString(IDS_VI_OPENDML);
	cStr[3]=LoadString(IDS_VI_HYBRIDE);
	sprintf(lpDest,"%s: %s",cStr[0].GetBuffer(255),
		(avifile->GetAVIType()==AT_STANDARD)?cStr[1].GetBuffer(255):
		(avifile->IsIdx1Present()?cStr[3].GetBuffer(255):cStr[2].GetBuffer(255)));
	clb->AddString(lpDest);

	if (lstrlen(avifile->GetWritingAppName())) {
		sprintf(lpDest, "%-30s: %s", "Writing-App", avifile->GetWritingAppName());
	} else {
		sprintf(lpDest, "%-30s: %s", "Writing-App", "n/a");
	}
	clb->AddString(lpDest);

	if (lstrlen(avifile->GetTitle())) {
		sprintf(lpDest, "%-30s: %s", "Title", avifile->GetTitle());
	} else {
		sprintf(lpDest, "%-30s: %s", "Title", "n/a");
	}
	clb->AddString(lpDest);

	avifile->GetVideoResolution(&x,&y);
	sprintf(lpDest,"%-30s: %dx%d",LoadString(IDS_VI_RESOLUTION),x,y);
	clb->AddString(lpDest);

	dwFourCC=avifile->GetFormatTag(0);
	sprintf(lpDest,"%-30s: %c%c%c%c","FourCC",(dwFourCC)&0xFF,(dwFourCC>>8)&0xFF,(dwFourCC>>16)&0xFF,(dwFourCC>>24)&0xFF);
	clb->AddString(lpDest);

	cStr[0]=LoadString(IDS_VI_BICOMPRESSION);
	dwFourCC=((BITMAPINFOHEADER*)(avifile->GetStreamFormat(0)))->biCompression;
	sprintf(lpDest,"%-30s: %c%c%c%c",cStr[0].GetBuffer(255),(dwFourCC)&0xFF,(dwFourCC>>8)&0xFF,(dwFourCC>>16)&0xFF,(dwFourCC>>24)&0xFF);
	clb->AddString(lpDest);

	cStr[0]=LoadString(IDS_VI_FRAMERATE);
	framerate1=1000000000/(double)avifile->GetNanoSecPerFrame();
	_gcvt(framerate1,6,buffer);
	sprintf(lpDest,"%-23s (strh): %s fps",cStr[0].GetBuffer(255),buffer);
	clb->AddString(lpDest);

	framerate2=1000000/(double)avifile->lpMainAVIHeader->dwMicroSecPerFrame;
	_gcvt(framerate2,6,buffer);
	sprintf(lpDest,"%-23s (avih): %s fps",cStr[0].GetBuffer(255),buffer);
	clb->AddString(lpDest);

	if (avifile->lpMainAVIHeader->dwMicroSecPerFrame!=round((double)(avifile->GetNanoSecPerFrame())/1000))
	{
		cStr[0]=LoadString(IDS_VI_FRWRITTENBYIDIOT);
		sprintf(lpDest,"  %s",cStr[0].GetBuffer(255));
		clb->AddString(lpDest);
	}
	else
	{
		clb->SetUnavailableRepairs(REPAIRS_FRAMERATE);
	}

	cStr[0]=LoadString(IDS_VI_NBROFFRAMES);
	x=avifile->GetNbrOfChunks(0);
	dwChunkCount=x;
	dSeconds=((double)x/(double)framerate1);
	dwMillisec=(DWORD)(round(1000*double(x)/(double)framerate1)%1000);
	sprintf(buffer,"%d:%02d:%02d.%03d",((DWORD)dSeconds/3600),((DWORD)dSeconds%3600)/60,((DWORD)dSeconds%60),dwMillisec);
	sprintf(lpDest,"%-30s:%10d (%s)",cStr[0].GetBuffer(255),x,buffer);
	clb->AddString(lpDest);

	cStr[0]=LoadString(IDS_VI_KEYFRAMES);
	sprintf(lpDest,"%-30s:%10d",cStr[0].GetBuffer(255),avifile->GetNbrOfFrames(FT_KEYFRAME));
	clb->AddString(lpDest);
	cStr[0]=LoadString(IDS_VI_DELTAFRAMES);
	sprintf(lpDest,"%-30s:%10d",cStr[0].GetBuffer(255),avifile->GetNbrOfFrames(FT_DELTAFRAME));
	clb->AddString(lpDest);
	cStr[0]=LoadString(IDS_VI_DROPPEDFRAMES);
	sprintf(lpDest,"%-30s:%10d",cStr[0].GetBuffer(255),avifile->GetNbrOfFrames(FT_DROPPEDFRAME));
	clb->AddString(lpDest);

	sprintf(lpDest,"%-30s:%10d","strh[0].dwLength",avifile->GetStreamHeader(0)->dwLength);
	clb->AddString(lpDest);
	clb->AddString("");
    avifile->GetFramesInFirstRIFF(&dw1, &dw2);
	sprintf(lpDest,"%-30s:%10d","MainAVIHeader.dwTotalFrames",dw1);
	clb->AddString(lpDest);
	cStr[0]=LoadString(IDS_VILB_REALFRAMESINRIFF);
	sprintf(lpDest,"%-30s:%10d",cStr[0],dw2);
	clb->AddString(lpDest);
	if (dw1!=dw2)
	{
		cStr[0]=LoadString(IDS_VI_DWTOTALFRAMESWRITTENBYIDIOT);
		sprintf(lpDest,"  %s",cStr[0].GetBuffer(255));
		clb->AddString(lpDest);
	}
	else
	{
		clb->SetUnavailableRepairs(REPAIRS_TOTALFRAMES);
	}
 
// suggested buffer size
	cStr[0]=LoadString(IDS_VILB_SBS);
	sprintf(lpDest,"%-30s:%10d",cStr[0].GetBuffer(255),avifile->lpMainAVIHeader->dwSuggestedBufferSize);
	clb->AddString(lpDest);

	clb->AddString("");
	cStr[0]=LoadString(IDS_VILB_FLAGSSET);
	DWORD	dwFlags=avifile->lpMainAVIHeader->dwFlags;
	sprintf(lpDest,"%-30s: 0x%08X",cStr[0].GetBuffer(255),dwFlags);
	clb->AddString(lpDest);
	if (dwFlags&AVIF_HASINDEX)
	{
		sprintf(lpDest,"%-30s  %s","","AVIF_HASINDEX");
		clb->AddString(lpDest);
	}
	if (dwFlags&AVIF_MUSTUSEINDEX)
	{
		sprintf(lpDest,"%-30s  %s","","AVIF_MUSTUSEINDEX");
		clb->AddString(lpDest);
	}
	if (dwFlags&AVIF_ISINTERLEAVED)
	{
		sprintf(lpDest,"%-30s  %s","","AVIF_ISINTERLEAVED");
		clb->AddString(lpDest);
	}
	if (dwFlags&AVIF_TRUSTCKTYPE)
	{
		sprintf(lpDest,"%-30s  %s","","AVIF_TRUSTCKTYPE");
		clb->AddString(lpDest);
	}
	if (dwFlags&AVIF_WASCAPTUREFILE)
	{
		sprintf(lpDest,"%-30s  %s","","AVIF_WASCAPTUREFILE");
		clb->AddString(lpDest);
	}
	if (dwFlags&AVIF_COPYRIGHTED)
	{
		sprintf(lpDest,"%-30s  %s","","AVIF_COPYRIGHTED");
		clb->AddString(lpDest);
	}
	

	cStr[0]=LoadString(IDS_VI_SIZEOFVIDEO);
	cStr[1]=LoadString(STR_BYTES);
	char cSize[30];
	QW2Str(avifile->GetStreamSize(0),cSize,16);
	sprintf(lpDest,"%-30s: %s %s",cStr[0],cSize,cStr[1]);
	clb->AddString(lpDest);
	
	cStr[0]=LoadString(IDS_VI_VIDEODATARATE);
	cStr[1]=LoadString(STR_KBYTE);
	sprintf(lpDest,"%-30s: %d %s/s",cStr[0],DWORD(avifile->GetStreamSize(0)/dSeconds)>>10,cStr[1]);
	clb->AddString(lpDest);

	dwAudioStreams=0;
	for (i=0;i<=(int)avifile->GetNbrOfStreams();i++) if (avifile->IsAudioStream(i)) dwAudioStreams++;
	cStr[0]=LoadString(IDS_VI_AUDIOSTREAMS);
	sprintf(lpDest,"%s: %d",cStr[0].GetBuffer(255),dwAudioStreams);
	clb->AddString(lpDest);

	clb->AddString(" ");
	lpRSIP=avifile->GetLoadSuperIndexProtocol();

	j=0;
	for (i=0;i<=(int)avifile->GetNbrOfStreams();i++)
	{
		if (avifile->IsAudioStream(i))
		{
			j++;
			strf=(WAVEFORMATEX*)avifile->GetStreamFormat(i);
			strh=avifile->GetStreamHeader(i);
		// Nummer
			cStr[0]=LoadString(IDS_VI_AUDIOSTREAM);
			cStr[1]=LoadString(IDS_VI_STREAM);
			sprintf(lpDest,"%s: %d (%s %d)",cStr[0].GetBuffer(255),j,cStr[1].GetBuffer(255),i);
			clb->AddString(lpDest);
		// Bitrate
			cStr[0]=LoadString(IDS_VI_BITRATE);
			QW2Str(avifile->GetAvgBytesPerSec(i)*8,buffer,16);
			sprintf(lpDest,"  %-25s: %s Bit/s",cStr[0],buffer);
			clb->AddString(lpDest);

			cStr[4]=LoadString(STR_BYTES);
			if ((!IsMP3SampleCount(strf->nBlockAlign))||(strf->wFormatTag!=0x0055))
			{
			// nicht (MP3-VBR) => Anzahl an dwSamplesize-großer Blöcke
				cStr[0]=LoadString(STR_VILB_STREAMSIZE_STRH);
				QW2Str(strh->dwLength,buffer,16);
				sprintf(lpDest,"  %-25s: %s units à %d %s",cStr[0],
					buffer,strh->dwSampleSize,cStr[4].GetBuffer(255));
				clb->AddString(lpDest);

				QW2Str(avifile->GetStreamSize(i),buffer,16);
				sprintf(lpDest,"  %-25s: %s %s",LoadString(STR_VILB_STREAMSIZE_INDEX),buffer,LoadString(STR_BYTES));
				clb->AddString(lpDest);
			}
			else
			{
				// MP3-VBR: Anzahl Chunks
				QW2Str(strh->dwLength,buffer,16);
				sprintf(lpDest,"  %-25s: %s %s",LoadString(STR_VILB_STREAMSIZE_STRH),buffer,LoadString(IDS_VI_FRAMES));
				clb->AddString(lpDest);
				QW2Str(avifile->GetStreamSize(i),buffer,16);
				sprintf(lpDest,"  %-25s: %s %s",LoadString(STR_VILB_STREAMSIZE_INDEX),buffer,LoadString(STR_BYTES));
				clb->AddString(lpDest);
			}
		// Anzahl Chunks		
			cStr[0]=LoadString(IDS_VI_AUDIOSTREAMCHUNKS);
			QW2Str(avifile->GetNbrOfChunks(i),buffer,16);
			sprintf(lpDest,"  %-25s: %s",cStr[0],buffer);
			clb->AddString(lpDest);
			dwChunkCount+=avifile->GetNbrOfChunks(i);
	// Format-Tag
			cStr[0]=LoadString(IDS_VI_AUDIOID);
			sprintf(lpDest,"  %-25s: %10s0x%04X",cStr[0].GetBuffer(255),"",avifile->GetFormatTag(i));
			clb->AddString(lpDest);
	// Samplingrate
			cStr[0]=LoadString(IDS_VI_SAMPLINGRATE);
			QW2Str(strf->nSamplesPerSec,buffer,16);
			sprintf(lpDest,"  %-25s: %s",cStr[0],buffer);
			clb->AddString(lpDest);
	// Kanäle
			cStr[0]=LoadString(IDS_VI_CHANNELS);
			QW2Str(strf->nChannels,buffer,16);
			sprintf(lpDest,"  %-25s: %s",cStr[0],buffer);
			clb->AddString(lpDest);
	// suggested buffer size
			cStr[0]=LoadString(IDS_VILB_SBS);
			QW2Str(strh->dwSuggestedBufferSize,buffer,16);
			sprintf(lpDest,"  %-25s: %s",cStr[0],buffer);
			clb->AddString(lpDest);

			clb->AddString(" ");
		}
	}

	dwSubtitles=0;
	for (i=0;i<=(int)avifile->GetNbrOfStreams();i++) if (avifile->IsTextStream(i)) dwSubtitles++;
	cStr[0]=LoadString(IDS_VILB_SUBTITLES);
	sprintf(lpDest,"%s: %d",cStr[0].GetBuffer(255),dwSubtitles);
	clb->AddString(lpDest);
	clb->AddString(" ");

	j=0;
	for (i=0;i<=(int)avifile->GetNbrOfStreams();i++)
	{
		if (avifile->IsTextStream(i))
		{
			j++;
			strf=(WAVEFORMATEX*)avifile->GetStreamFormat(i);
			strh=avifile->GetStreamHeader(i);
		// Nummer
			cStr[0]=LoadString(IDS_VILB_SUBTITLE);
			cStr[1]=LoadString(IDS_VI_STREAM);
			sprintf(lpDest,"%s: %d (%s %d)",cStr[0].GetBuffer(255),j,cStr[1].GetBuffer(255),i);
			clb->AddString(lpDest);
	// suggested buffer size
			cStr[0]=LoadString(IDS_VILB_SBS);
			sprintf(lpDest,"  %-25s: %d",cStr[0].GetBuffer(255),strh->dwSuggestedBufferSize);
			clb->AddString(lpDest);
			clb->AddString(" ");
		}
	}


	if (lpRSIP)
	{
	// Info über OpenDML-Index
		cStr[0]=LoadString(IDS_VI_OPENDMLINDEX);
		sprintf(lpDest,"%s: ",cStr[0].GetBuffer(255));
		clb->AddString(lpDest);
		for (i=0;i<(int)avifile->GetNbrOfStreams();i++)
		{
		// Stream
			cStr[0]=LoadString(IDS_VI_STREAM);
			sprintf(lpDest,"  %s: %d",cStr[0].GetBuffer(255),i);
			clb->AddString(lpDest);
		// Anzahl Einträge im Superindex
			cStr[0]=LoadString(IDS_VI_SUPERINDEXENTRIES);
			sprintf(lpDest,"      %s: %d ",cStr[0].GetBuffer(255),lpRSIP[i].dwEntries);
			clb->AddString(lpDest);
			cStr[0]=LoadString(IDS_VI_DEFECTIVE);
			cStr[1]=LoadString(IDS_VI_ENTRY);
			cStr[2]=LoadString(IDS_VI_REALVALUE);
			cStr[3]=LoadString(IDS_VI_DWDURATION);
		// Kopfzeile
			sprintf(lpDest,"                     %-20s    %-20s",cStr[3].GetBuffer(255),cStr[2].GetBuffer(255));
			clb->AddString(lpDest);
			clb->AddString("                     ------------------------------------------------");
		// Einzelne Einträge durchgehen
			for (j=0;j<(int)lpRSIP[i].dwEntries;j++)
			{
				QW2Str(lpRSIP[i].rsipEntries[j].dwDurationValue,nbr1,20);
				QW2Str(lpRSIP[i].rsipEntries[j].dwRealDuration,nbr2,25);
				sprintf(lpDest,"      %s %3d: %s%s       %s",cStr[1].GetBuffer(255),j+1,
					nbr1,nbr2,
					(lpRSIP[i].rsipEntries[j].dwDurationValue!=lpRSIP[i].rsipEntries[j].dwRealDuration)?cStr[0].GetBuffer(255):"OK");
				clb->AddString(lpDest);
				if (lpRSIP[i].rsipEntries[j].dwDurationValue!=lpRSIP[i].rsipEntries[j].dwRealDuration) bNeeddwDurationRepair=true;
			}
		}
		clb->AddString(" ");
	}
	if (!bNeeddwDurationRepair) clb->SetUnavailableRepairs(REPAIRS_ODML);
	cStr[0]=LoadString(IDS_VI_CHUNKSALTOGETHER);
	sprintf(lpDest,"%s: %d",cStr[0].GetBuffer(255),dwChunkCount);
	clb->AddString(lpDest);

	// Info über Chunkgrößen
	cStr[0]=LoadString(IDS_VILB_CHUNKSIZES);
	clb->AddString(cStr[0]);
	for (i=avifile->GetNbrOfStreams()-1;i>=0;i--)
	{
		for (j=avifile->GetNbrOfChunks(i)-1;j>=0;j--)
		{
			dwSize=avifile->GetChunkSize(i,j);
			if (dwSize>(DWORD)sizes[12])
			{
				dwChunkSizes[13]++;
			}
			else
			{
				k=0;
				while (k<=12)
				{
					if (dwSize<(DWORD)(sizes[k]))
					{
						dwChunkSizes[k]++;
						k=20;
					}
					k++;
				}
			}

		}
	}
	cStr[4]=LoadString(STR_BYTES);
	for (i=0;i<=13;i++)
	{
		if (i<13)
		{
			cStr[1]=LoadString(IDS_VILB_UPTO);
			sprintf(lpDest,"%8s %10d %s: %10d",cStr[1].GetBuffer(255),sizes[i],cStr[4].GetBuffer(255),dwChunkSizes[i]);
		}
		else
		{
			cStr[1]=LoadString(IDS_VILB_OVER);
			sprintf(lpDest,"%8s %10d %s: %10d",cStr[1].GetBuffer(255),sizes[12],cStr[4].GetBuffer(255),dwChunkSizes[i]);
		}
		clb->AddString(lpDest);
	}


	if (lpFI->dwType&FILETYPE_M2F2) clb->SetUnavailableRepairs(0xFFFFFFFF);

	// TODO: Zusätzliche Initialisierung hier einfügen
	
	free(lpDest);


	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX-Eigenschaftenseiten sollten FALSE zurückgeben
}

void CVideoInformationDlg::OnApplyrepairs() 
{
	CVideoInformationDlgListbox*	clb;
	clb=(CVideoInformationDlgListbox*)GetDlgItem(IDE_VIDEOINFORMATION);
	CHANGEAVIHEADER*	lpcahRepairs=clb->GetRepairs();
	CHANGEAVIHEADER*	lpcahCurr=lpcahRepairs;
	char				lpcFilename[500];
	HANDLE				hFile;
	DWORD				dwWritten;
	CString				cStr[2];
	CAVIMux_GUIDlg*		cMainDlg = (CAVIMux_GUIDlg*)GetParent();

	lstrcpy(lpcFilename,lpFI->lpcName);

	if (lpcahCurr->dwValid!=1)
	{
		cStr[0]=LoadString(IDS_VILB_NOREPAIRS);
		cStr[1]=LoadString(IDS_INFORMATION);
		MessageBox(cStr[0],cStr[1],MB_OK | MB_ICONINFORMATION);
	}
	else
	{
		cMainDlg->SendDlgItemMessage(IDC_SOURCEFILELIST,WM_COMMAND,IDM_REMOVE,0);
		cStr[0]=LoadString(IDS_APPLYREPAIRS);
		cStr[1]=LoadString(IDS_INFORMATION);
		MessageBox(cStr[0],cStr[1],MB_OK | MB_ICONINFORMATION);

		hFile=CreateFile(lpcFilename,GENERIC_READ | GENERIC_WRITE,FILE_SHARE_READ,NULL,OPEN_EXISTING,NULL,NULL);
		if (hFile==INVALID_HANDLE_VALUE)
		{
			cStr[0]=LoadString(IDS_COULDNOTOPENOUTPUTFILE);
			cStr[1]=LoadString(IDS_ERROR);
			MessageBox(cStr[0],cStr[1],MB_OK | MB_ICONERROR);
		}
		else
		{
			while (lpcahCurr)
			{
				if (lpcahCurr->dwValid==1)
				{
					SetFilePointer64(hFile,lpcahCurr->qwFilePos);
					WriteFile(hFile,&(lpcahCurr->qwNewVal),lpcahCurr->dwSize,&dwWritten,NULL);
				}
				lpcahCurr=(CHANGEAVIHEADER*)lpcahCurr->lpNext;
			}
			CloseHandle(hFile);
			clb->ClearRepairs();
			PostMessage(WM_COMMAND,IDOK);
		}
	}
}

void CVideoInformationDlg::OnOK() 
{
	// TODO: Zusätzliche Prüfung hier einfügen
	CVideoInformationDlgListbox*	clb;
	clb=(CVideoInformationDlgListbox*)GetDlgItem(IDE_VIDEOINFORMATION);
	clb->ClearRepairs(false);

//	m_Tree.DeleteAllItems();

	CResizeableDialog::OnOK();
}

void CVideoInformationDlg::SetFile(FILE_INFO* _lpFI)
{
	lpFI=_lpFI;
	if (lpFI->dwType & FILETYPE_AVI) {
		avifile=_lpFI->AVIFile;
		dwKinfOfSource=KOS_AVIFILEEX;
	} else if (lpFI->dwType & FILETYPE_MKV) {
		mkvfile=_lpFI->MKVFile;
		dwKinfOfSource=KOS_MATROSKA;
	}
}

FILE_INFO* CVideoInformationDlg::GetFile()
{
	return lpFI;
}

void CVideoInformationDlg::SetVideoSource(VIDEOSOURCE* _lpVS)
{
	lpVS=_lpVS;
	dwKinfOfSource=KOS_VIDEOSOURCE;
}

void CVideoInformationDlg::OnRIFFChunkTree() 
{
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen
	CRIFFChunkTreeDlg*	crctd;
//	CEBMLTreeDlg* cetg;

	if (GetKindOfSource()==KOS_AVIFILEEX)
	{
		crctd=new CRIFFChunkTreeDlg;
		crctd->SetSource(avifile->GetSource());
		crctd->DoModal();

		delete crctd;

		SendDlgItemMessage(IDC_BUTTON1,WM_SETTEXT,0,(LPARAM)"RIFF");
	} 
/*	if (GetKindOfSource()==KOS_MATROSKA) {
		cetg = new CEBMLTreeDlg;

		cetg->Attribs();
		cetg->SetSource(mkvfile->GetSource());
		cetg->DoModal();
		delete cetg;
	}
*/
}

BOOL CVideoInformationDlg::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	char	Buffer[50];
	// TODO: Speziellen Code hier einfügen und/oder Basisklasse aufrufen
	switch (LOWORD(wParam))
	{
		case IDM_BUILDRIFFSTATE:
			sprintf(Buffer,"%d MB",lParam);
			SendDlgItemMessage(IDC_BUTTON1,WM_SETTEXT,0,(LPARAM)Buffer);
			break;
	}


	return CResizeableDialog::OnCommand(wParam, lParam);
}

void CVideoInformationDlg::OnSavetree() 
{
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen
	char* cBuffer = (char*)calloc(1,1<<20);
	CFileDialog* dlg;
	
	dlg= new CFileDialog(false,"txt","",OFN_OVERWRITEPROMPT,"UTF-8 Text file (*.txt)|*.txt||",NULL);
	if (dlg->DoModal()==IDOK)
	{
		m_Tree.Render2Buffer(cBuffer);
		FILE* f = fopen(dlg->GetPathName().GetBuffer(1024), "wb");
		fwrite(cBuffer, 1, strlen(cBuffer), f);
		fclose(f);
	}

	delete cBuffer;
}


void CVideoInformationDlg::OnBnClickedOk()
{
	// TODO: Fügen Sie hier Ihren Kontrollbehandlungscode für die Benachrichtigung ein.
	// TODO: Zusätzliche Prüfung hier einfügen
	CVideoInformationDlgListbox*	clb;
	clb=(CVideoInformationDlgListbox*)GetDlgItem(IDE_VIDEOINFORMATION);
	clb->ClearRepairs(false);

//	m_Tree.DeleteAllItems();

	CResizeableDialog::OnOK();

//	CResizeableDialog::OnOK();
}
