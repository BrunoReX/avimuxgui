#include "stdafx.h"
#include "debug.h"
#include "..\formattime.h"
#include "languages.h"
#include "muxing.h"
#include "riffchunktreedlg.h"
#include "silence.h"
#include "..\matroska.h"
#include "audiosource.h"
#include "global.h"

DWORD					bStop=false;
bool					bMuxing=false;
CAVIMux_GUIDlg*			dlg;

int MSG_UNBUFFERED		[] = { STR_MUXLOG_UNBUFFERED_OFF  , STR_MUXLOG_UNBUFFERED_ON };
int MSG_OVERLAPPEDOUT	[] = { STR_MUXLOG_OVERLAPPED_OFF  , STR_MUXLOG_OVERLAPPED_ON };
int MSG_CUEVIDEO        [] = { STR_MUXLOG_CUES_VIDEO_OFF  , STR_MUXLOG_CUES_VIDEO_ON };
int MSG_CUEAUDIO        [] = { STR_MUXLOG_CUES_AUDIO_OFF  , STR_MUXLOG_CUES_AUDIO_ON };
int MSG_CLUSTERINDEX    [] = { STR_MUXLOG_CLUSTERINDEX_OFF, STR_MUXLOG_CLUSTERINDEX_ON };
int MSG_RECLISTS        [] = { STR_MUXLOG_RECLISTS_OFF    , STR_MUXLOG_RECLISTS_ON };
int MSG_LOWOVHDAVI      [] = { STR_MUXLOG_LOWOVHDAVI_OFF   , STR_MUXLOG_LOWOVHDAVI_ON };

char* Message_index(int* pList, int iIndex)
{
	return LoadString(pList[iIndex]);
}


void ComposeVersionString(char* buffer)
{
	CString c;
	c.LoadString(IDS_VERSION_INFO);
	sprintf(buffer,"AVI-Mux GUI %s, %s  %s",c,__DATE__, __TIME__);
}

void StopMuxing(bool b)
{
	bStop=b;
}

bool DoStop()
{
	return !!bStop;
}

bool MuxingInProgress()
{
	return bMuxing;
}

void AddOverhead(__int64* qwStats,DWORD dwOverhead)
{
	if (dwOverhead)
	{
		qwStats[2]+=dwOverhead;
		qwStats[3]+=dwOverhead;
		qwStats[6]+=dwOverhead;
		qwStats[7]+=dwOverhead;
	}
}

const int ADS_NOTEVEN	= 0x01;

void WaitForMuxing(DEST_AVI_INFO* p)
{
	p->hMuxingSemaphore = OpenSemaphore(SEMAPHORE_ALL_ACCESS, false, GlobalMuxSemaphoreName());
	WaitForSingleObject(p->hMuxingSemaphore, INFINITE);
	p->hMuxingStartedSemaphore = OpenSemaphore(SEMAPHORE_ALL_ACCESS, false, GlobalMuxingStartedSemaphoreName());
	ReleaseSemaphore(p->hMuxingStartedSemaphore, 1, NULL);
}

void FinishMuxing(DEST_AVI_INFO* p)
{
	ReleaseSemaphore(p->hMuxingSemaphore, 1, NULL);
	CloseHandle(p->hMuxingSemaphore);
	WaitForSingleObject(p->hMuxingStartedSemaphore, INFINITE);
	CloseHandle(p->hMuxingStartedSemaphore);
}

void AddVideoSize(__int64* qwStats,DWORD dwSize, int iFlags = NULL)
{
	dwSize=dwSize;
	qwStats[0]+=dwSize;
	qwStats[4]+=dwSize;
	qwStats[3]+=dwSize;
	qwStats[7]+=dwSize;
	if (!(iFlags & ADS_NOTEVEN)) AddOverhead(qwStats,dwSize%2);
}

void AddAudioSize(__int64* qwStats,DWORD dwSize, int iFlags = NULL)
{
	dwSize=dwSize;
	qwStats[1]+=dwSize;
	qwStats[5]+=dwSize;
	qwStats[3]+=dwSize;
	qwStats[7]+=dwSize;
	if (!(iFlags & ADS_NOTEVEN)) AddOverhead(qwStats,dwSize%2);
}

void AddSubtitleSize(__int64* qwStats,DWORD dwSize, int iFlags = NULL)
{
	dwSize=dwSize;
	qwStats[3]+=dwSize;
	qwStats[7]+=dwSize;
	qwStats[8]+=dwSize;
	qwStats[9]+=dwSize;
	if (!(iFlags & ADS_NOTEVEN)) AddOverhead(qwStats,dwSize%2);
}

int RenderElement(CListCtrl* c,__int64 qwData,int i,int j,DWORD dwByteAccuracy)
{
	char			cBuffer[500];

	if (dwByteAccuracy) {
		QW2Str(qwData,cBuffer,15);
	} else FormatSize(cBuffer,qwData);
	c->SetItemText(i,j,cBuffer);

	return 1;
}

int RenderRateElement(CListCtrl* c,__int64 qwData,int i,int j,DWORD dwByteAccuracy, char* s)
{
	char			cBuffer[500];

	sprintf(cBuffer,s);
	if (dwByteAccuracy) {
		sprintf(cBuffer+strlen(s),"%I64d %s/s",qwData,LoadString(STR_KBYTE));
	} else {
		FormatSize(cBuffer+strlen(s),qwData*1000);
		strcat(cBuffer,"/s");
	}

	c->SetItemText(i,j,cBuffer);

	return 1;
}

/* Statistics:
      0 / 4 - video 
	  1 / 5 - audio
	  2 / 6 - overhead
	  3 / 7 - total
	  8 / 9 - subtitles
*/

const char* sem_name = "amg-sem-1";
const char* sem_name_finish = "amg-sem-finish-1";

int DisplayProgress_Thread(DISPLAY_PROGRESS_INFO* lpDPI)
{
	DEST_AVI_INFO*  lpDAI = lpDPI->lpDAI;
	__int64*		_qwStats = lpDPI->qwStats;
	__int64*		qwStats;
	DWORD*			lpdwFrameCountTotal = lpDPI->lpdwFrameCountTotal;
	DWORD			dwFrameCountTotal;
	MUX_STATE*		dwMuxState = lpDPI->dwMuxState; 
	DWORD*			dwMuxStatePos = lpDPI->dwMuxStatePos;
	__int64 iTotalDuration = lpDPI->iDuration;

	char			cBuffer[500];
	DWORD			dwDataTime;
	DWORD			dwNextPos;
	DWORD			dwQuit=0;
	DWORD			dwDataRate;
	DWORD			dwDataRateMin, dwDataRateMax, dwDataRateAvg, dwTimeBegin;
	DWORD			dwfps;
	DWORD			dwSize;
    CString			cStr;
	HANDLE			hSem;

	qwStats=(__int64*)malloc(dwSize=sizeof(__int64)*MUXSTATEENTRIES);

	dwTimeBegin = GetTickCount();
	dwDataRateMin = 1000000000;
	dwDataRateMax = 0;

	hSem = OpenSemaphore(SEMAPHORE_ALL_ACCESS,false,sem_name);

	do 
	{
		if (hSem) {
			WaitForSingleObject(hSem, INFINITE);
		}

		lpDPI->dwByteAccuracy=lpDAI->dlg->m_Progress_List.GetAccuracy()==PRAC_BYTES;
		memcpy(qwStats,_qwStats,dwSize);

		dwFrameCountTotal = *lpdwFrameCountTotal;
		__int64 iProgress = *(lpDPI->lpiProgress);

		if (hSem) {
			ReleaseSemaphore(hSem, 1, NULL);
		}

		dwNextPos=(*dwMuxStatePos)+1;
		dwNextPos%=MUXSTATEENTRIES;

		dwMuxState[*dwMuxStatePos].dwTime=GetTickCount(); 
		dwMuxState[*dwMuxStatePos].qwWritten=qwStats[7];
		dwMuxState[*dwMuxStatePos].dwFrames=dwFrameCountTotal;

		dwDataTime=(dwMuxState[*dwMuxStatePos].dwTime)-(dwMuxState[dwNextPos].dwTime);
		if (dwDataTime==0) dwDataTime++;
		dwfps=((dwMuxState[*dwMuxStatePos].dwFrames)-(dwMuxState[dwNextPos].dwFrames))*1000/dwDataTime;

		if (dwDataTime)
		{
			dwDataRate=(DWORD)(1000*((dwMuxState[*dwMuxStatePos].qwWritten)-(dwMuxState[dwNextPos].qwWritten))/dwDataTime/1024);
		}
		else
		{
			dwDataRate=0;
		}

		if (GetTickCount()-dwTimeBegin > 1000) {
			dwDataRateMin = min(dwDataRateMin, dwDataRate);
			dwDataRateMax = max(dwDataRateMax, dwDataRate);
		}

		dwDataRateAvg = (int)(qwStats[7] / max(1,(dwMuxState[*dwMuxStatePos].dwTime - dwTimeBegin)));

		*dwMuxStatePos=dwNextPos;

		char	cTotalTime[20];
		Millisec2Str(iTotalDuration,cTotalTime);
		char	cCurrTime[20];
		Millisec2Str(iProgress/1000000,cCurrTime);
 
		wsprintf(cBuffer,"%s / %s",cCurrTime,(iTotalDuration)?cTotalTime:"unknown");
		lpDAI->dlg->SetDlgItemText(IDC_FRAMES,cBuffer);

		CListCtrl* L = &lpDAI->dlg->m_Progress_List;
		DWORD	   A = lpDPI->dwByteAccuracy;
		RenderRateElement(L,(dwDataRateMin<1000000000)?dwDataRateMin:0,2,3,A,"> ");
		RenderRateElement(L,dwDataRateMax,2,4,A,"< ");
		RenderRateElement(L,dwDataRateAvg,2,5,A,"~ ");
		RenderRateElement(L,dwDataRate,2,2,A,"c: ");
		RenderElement(L,qwStats[0],0,1,A);
		RenderElement(L,qwStats[1],0,2,A);
		RenderElement(L,qwStats[2],0,3,A);
		RenderElement(L,qwStats[8],0,4,A);
		RenderElement(L,qwStats[3],0,5,A);
		RenderElement(L,qwStats[4],1,1,A);
		RenderElement(L,qwStats[5],1,2,A);
		RenderElement(L,qwStats[6],1,3,A);
		RenderElement(L,qwStats[9],1,4,A);
		RenderElement(L,qwStats[7],1,5,A);

		wsprintf(cBuffer,"%d",dwfps);

		if (iTotalDuration) lpDAI->dlg->m_Prg_Progress.SetPos((int)(iProgress/100/iTotalDuration));

		if (lpDPI->dwLeave) dwQuit=1;
		lpDAI->dlg->m_Protocol.InvalidateRect(NULL);
		lpDAI->dlg->m_Protocol.UpdateWindow();

		Sleep(250);
	
	} while (!dwQuit);
	
	HANDLE hSem_finish = OpenSemaphore(SEMAPHORE_ALL_ACCESS,false,sem_name_finish);
	lpDPI->dwLeave=0;
	delete qwStats;
	CloseHandle(hSem);
	ReleaseSemaphore(hSem_finish,1,NULL);
	CloseHandle(hSem_finish);
	return 1;
}

const int RSPF_CHAPTERS = 0x01;
const int RSPF_FRAMES   = 0x02;
const int RSP_ERROR		= -0x01;

int ResolveSplitpoints (DEST_AVI_INFO* lpDAI, int iFlags, int* piError = NULL)
{
	CSplitPoints* s = lpDAI->split_points;
	CChapters* c = lpDAI->chapters;
	SPLIT_POINT_DESCRIPTOR* d,*d2,d3;
	bool bChapters = !!(iFlags & RSPF_CHAPTERS);
	bool bFrames = !!(iFlags & RSPF_FRAMES);
	int  i,j,k;

	// resolve wildchar
	for (i=0;i<s->GetCount();i++) {
		d = s->At(i);
		CDynIntArray* x = d->aChapBegin;
		if (d->iFlags & SPD_BCHAP) {
			if ((j=x->Find(-2))!=-1) {
				s->Delete(i--);
				CDynIntArray* main_chapter_index = x->Duplicate(j-1);
				CDynIntArray* this_chapter_index = x->Duplicate(j);

				CDynIntArray* complete_chapter_index = x->Duplicate();
				complete_chapter_index->Set(j, 0);
				CChapters* main_chapter = c->GetSubChapters(main_chapter_index);
		
				SPLIT_POINT_DESCRIPTOR* p;

				if (main_chapter->GetChapterCount()) {

					p = new SPLIT_POINT_DESCRIPTOR; ZeroMemory(p, sizeof(*p));
					p->aChapBegin = this_chapter_index->Duplicate();
					p->aChapBegin->Set(j, 0);
					p->iFlags = SPD_BCHAP;
					s->Insert(p);
					
					for (k=0;k<main_chapter->GetChapterCount();k++) {
						p = new SPLIT_POINT_DESCRIPTOR; ZeroMemory(p, sizeof(*p));
						p->aChapBegin = complete_chapter_index->Duplicate();
						p->aChapBegin->Set(j, k);
						p->iFlags = SPD_BCHAP;
						s->Insert(p);
					} 
				} else {
					p = new SPLIT_POINT_DESCRIPTOR; ZeroMemory(p, sizeof(*p));
					p->aChapBegin = main_chapter_index->Duplicate();
					p->iFlags = SPD_BCHAP;
					s->Insert(p);
				}
				complete_chapter_index->DeleteAll();
				delete complete_chapter_index;
			}			
		}
	}
	
	// resolve chapter points to timestamps
	for (i=0;i<s->GetCount();i++) {
		d = s->At(i);
		if ((d->iFlags & SPD_BCHAP) && bChapters) {
			if (c->GetChapter(d->aChapBegin,&d->iBegin,NULL,NULL)==CHAP_INVALIDINDEX) {
				if (piError) *piError = i;
				return RSP_ERROR;
			};
		}
	}

	// remove timestamps of NULL 
	for (i=0;i<s->GetCount();i++) {
		d = s->At(i);
		if (!d->iBegin) s->Delete(i--);
	}

	// sort points
	for (i=0;i<s->GetCount();i++) {
		for (int j=0;j<s->GetCount()-1-i;j++) {
			d = s->At(j);
			d2 = s->At(j+1);
			if (d->iBegin>d2->iBegin) {
				memcpy(&d3,d,sizeof(d3));
				memcpy(d,d2,sizeof(d3));
				memcpy(d2,&d3,sizeof(d3));
			}
		}
	}

	// remove  equal ones
	for (i=0;i<s->GetCount()-1;i++) {
		d = s->At(i);
		d2 = s->At(i+1);
		__int64 diff = d->iBegin - d2->iBegin;
		if (diff < 2 && diff > -2) s->Delete(i--);
	}

	return 0;
}

__int64 DistToNextSplitPoint(DEST_AVI_INFO* lpDAI, DWORD dwCurrFrame, __int64 iTimecode_ns)
{
	//if (!lpDAI->lpqwSplitPoints) 
	CSplitPoints* s = lpDAI->split_points;
	SPLIT_POINT_DESCRIPTOR* d;
	__int64 iLeft=0x7fffffffffffffff;

	int i;
	int iCount = s->GetCount();

	for (i=0;i<iCount;i++) {
		d = s->At(i);
		if (d->iBegin > iTimecode_ns) {
			iLeft = __min(iLeft,d->iBegin-iTimecode_ns);
		}
	}

	return iLeft;
}

int* _stdcall LegacyStateCB (DWORD dwPos,DWORD dwTotal,void *lpUserData)
{
	dlg->SendDlgItemMessage(IDC_LEGACYPROGRESS,PBM_SETRANGE32,0,dwTotal);
	dlg->SendDlgItemMessage(IDC_LEGACYPROGRESS,PBM_SETPOS,dwPos,0);
	return NULL;
}

void* _stdcall StartLegacyCB (void** lpUserData)
{	
	return (LegacyStateCB);
}

int MuxThread_AVI(DEST_AVI_INFO* lpDAI)
{
	WaitForMuxing(lpDAI);
	CAttribs*       s = lpDAI->settings;
	AVIFILEEX*		AVIOut, *NextAVIOut;
	FILESTREAM*		DestFile, *NextDestFile;
	char*			cBuffer;
	char			RawFilename[200];
	DWORD			i,j;
// Curr+Total: Video, Audio, Overhead, Total
	DWORD			dwFrameCountTotal=0;
	void*			lpBuffer;
	char*			cFilename,cSize[2][30];

	__int64			qwStats[10];
	__int64			qwMillisec=0;
	__int64			qwMaxBytes=0;
	__int64			qwNanoSecRead=0;
	__int64*		qwStreamSizes=NULL;
	__int64*		qwNanoSecNeeded=NULL;
	__int64*		qwNanoSecStored=NULL;
	__int64			iTimecode;

	__int64			i_vs_nspf = NULL;

	DWORD			dwMicroSecRead=0;
	DWORD			dwOverhead=0;
	int				dwHour,dwMin,dwSec,dwFrac;
	DWORD			dwFlags,dwFramesInCurrFile;
	int				iSize;
	DWORD			dwTime=0;
	DWORD			dwPreloadedFrames=0;
	DWORD			dwPartNbr=1;
	DWORD			dwNextAudioBlock;
	DWORD			dwRecListSize=0;
	DWORD			dwAudioBPS=NULL;
	DWORD			dwAudioBPVF=NULL;
	DWORD			dwLastFileBegin=NULL;
	DWORD			dwMaxDeviation=0;
	DWORD			dwMuxStatePos=0;
	DWORD*			dwFrameSizes=NULL;
	DWORD			dwReclistOverhead;
	DWORD			dwChunkOverhead;
	DWORD			dwFrameSizes_Len = 128;
	DWORD			dwFrameSizes_Pos = 0;
	DWORD			dwFrameSizes_Sum = 0;
	__int64			iProgress;

	int				iDelay=NULL;

	bool			bAC3broken=false;
	bool			bExternalSilence;
	bool			bFinishImmediately;
	bool			bPreloadCompensated=true;
	bool			bFinishPart=false;
	bool			bFirst;
	bool			bFilesize_reached;
	bool			bManual_splitpoint_reached;
	bool			bMustCorrectAudio=false;
	bool			bSplitInitiated=false;
	bool			bKeyframe=false;
	bool			bRecListEmpty=true;
	bool			bUnbufferedOutput = !!s->GetInt("output/general/unbuffered");
	bool			bOverlapped = 0;//!!lpDAI->settings->GetInt("output/general/overlapped");
	MUX_STATE*		lpmsState;
	CString			cStr[2];
	bool			bKeyframe_passed=false;
	__int64			iDistToSplitpoint = 0;
	char			Buffer[512],cName[512];
	char*			stream_report,*report;
	MSG_LIST*		msglist;
	SILENCE*		silence;
	WAVEFORMATEX*	lpwfe;
	VIDEOSOURCE*	v;
	CACHEDSTREAM*	cache;
	bool			bLegacyIndex = !!s->GetInt("output/avi/legacyindex");//lpDAI->avi.iLegacyIndex;

	HANDLE			hSem;

	bMuxing = true;

	DWORD			dwLegIndOverHead=0;

	lpDAI->dlg->m_Prg_Legidx_Label.ShowWindow(bLegacyIndex);
	lpDAI->dlg->m_Prg_Legidx_Progress.ShowWindow(bLegacyIndex);
	lpDAI->dlg->m_Prg_Legidx_Progress.SetPos(0);
	hSem = CreateSemaphore(NULL, 1, 1, sem_name);

	cacheAllowReadAhead(1);
	ZeroMemory(RawFilename,sizeof(RawFilename));
	memcpy(RawFilename,lpDAI->lpFileName,lstrlen(lpDAI->lpFileName)-4);
	StopMuxing(false);
	dwFrameSizes = (DWORD*)malloc(iSize=dwFrameSizes_Len*sizeof(DWORD));
	ZeroMemory(dwFrameSizes,iSize);

	stream_report = (char*)calloc(1,4096); 
	report = (char*)calloc(1,2048);
	cFilename = (char*)calloc(1,512);
	cBuffer = (char*)calloc(1,512);
	silence=new SILENCE;

	char dir[512];
	GetModuleFileName(NULL, dir, 512);
	silence->Init(dir);

	cStr[0]=LoadString(STR_MUXLOG_BEGIN);
	lpDAI->dlg->AddProtocolLine(cStr[0],5);

	bool bOpenDML  = !!s->GetInt("output/avi/opendml/on");
	bool bReclists = !!s->GetInt("output/avi/reclists");

	char cPattern[100]; cPattern[0]=0;
	int  iInterval = (int)s->GetInt("output/avi/opendml/stdindex/interval");
	int  iPattern  = (int)s->GetInt("output/avi/opendml/stdindex/pattern");
	int  ai_unit   = (int)s->GetInt("output/avi/audio interleave/unit");
	int  ai_value  = (int)s->GetInt("output/avi/audio interleave/value");

	switch (iPattern) {
		case SIP_RIFF:   sprintf(cPattern, LoadString(STR_MUXLOG_1STRIFFLIST_1));
		case SIP_FRAMES: sprintf(cPattern, LoadString(STR_MUXLOG_1STRIFFLIST_2), iInterval);
	}

	cStr[0]=LoadString(IDS_VI_AVITYPE);
	switch (bOpenDML)
	{
		case 0: cStr[1]=LoadString(IDS_VI_STANDARD); break;
		case 1: cStr[1]=(bLegacyIndex)?LoadString(IDS_VI_HYBRIDE):LoadString(IDS_VI_OPENDML);break;
	}
	wsprintf(Buffer,"%s: %s",cStr[0],cStr[1]);
	lpDAI->dlg->AddProtocolLine(Buffer,5);
	lpDAI->dlg->AddProtocolLine(LoadString(MSG_RECLISTS[bReclists]),4);
	lpDAI->dlg->AddProtocolLine(cPattern,4);

	// add audio interleave scheme to protocol
	switch(ai_unit) {
		case AIU_KB    : sprintf(cPattern, LoadString(STR_MUXLOG_AUDIOINTERLEAVE_KB), ai_value); break;
		case AIU_FRAME : sprintf(cPattern, LoadString(STR_MUXLOG_AUDIOINTERLEAVE_FR), ai_value); break;
	}
	lpDAI->dlg->AddProtocolLine(cPattern,4);

	qwMaxBytes=(__int64)(lpDAI->qwMaxSize)*1048576;
	
	if (qwMaxBytes<(__int64)(1<<25)*(1<<25))
	{
		QW2Str(qwMaxBytes,cSize[0],3);
		wsprintf(cBuffer,LoadString(STR_MUXLOG_MAXFILESIZE),cSize[0],LoadString(STR_BYTES));
		lpDAI->dlg->AddProtocolLine(cBuffer,5);
	}

	cStr[1]=LoadString(STR_MUXLOG_NAME);
	cStr[0]=LoadString(STR_MUXLOG_AUDIOSTREAMS);
	wsprintf(Buffer,cStr[0],lpDAI->dwNbrOfAudioStreams);
	lpDAI->dlg->AddProtocolLine(Buffer,5);
	for (i=0;i<lpDAI->dwNbrOfAudioStreams;i++)
	{
		wsprintf(Buffer,LoadString(STR_MUXLOG_STREAM),i+1);
		lpDAI->dlg->AddProtocolLine(Buffer,5);

		lpDAI->asi[i]->audiosource->GetName(cName);
		wsprintf(Buffer,LoadString(STR_MUXLOG_NAME),(strlen(cName)?cName:"n/a"));
		lpDAI->dlg->AddProtocolLine(Buffer,5, APL_UTF8);
		
		if (lpDAI->asi[i]->iDelay) {
			wsprintf(Buffer,LoadString(STR_MUXLOG_DELAY),lpDAI->asi[i]->iDelay);
			lpDAI->dlg->AddProtocolLine(Buffer,5);
		}

		AUDIOSOURCE* a = lpDAI->asi[i]->audiosource;

		if (a->GetFormatTag() == 0x2000) {
			a->SetFrameMode(j = (int)lpDAI->settings->GetInt("output/avi/ac3/frames per chunk"));
			sprintf(Buffer,LoadString(STR_MUXLOG_AC3AVIPATTERN), j);
			lpDAI->dlg->AddProtocolLine(Buffer,5);
		}
		if (a->GetFormatTag() == 0x2001) {
			a->SetFrameMode(j = (int)lpDAI->settings->GetInt("output/avi/dts/frames per chunk"));
			sprintf(Buffer,LoadString(STR_MUXLOG_DTSAVIPATTERN), j);
			lpDAI->dlg->AddProtocolLine(Buffer,5);
		}
		if (a->GetFormatTag() == 0x0055) {
			if (a->IsCBR() && !lpDAI->asi[i]->iDelay) {
				if  (!lpDAI->settings->GetInt("output/avi/mp3/cbr frame mode")) {
					a->SetFrameMode(FRAMEMODE_OFF);
					sprintf(Buffer,LoadString(STR_MUXLOG_MP3CBRDISFM), j);
				} else {
					a->SetFrameMode(FRAMEMODE_ON);
					sprintf(Buffer,LoadString(STR_MUXLOG_MP3CBRENFM), j);
				}
					lpDAI->dlg->AddProtocolLine(Buffer,5);
			} else {
				if (!a->IsCBR()) {
					a->SetFrameMode(j=(int)lpDAI->settings->GetInt("output/avi/mp3/frames per chunk"));
					sprintf(Buffer,LoadString(STR_MUXLOG_MP3VBRAVIPATTERN), j);
					lpDAI->dlg->AddProtocolLine(Buffer,5);
					lpDAI->asi[i]->lpASH->dwScale *= j;
					((WAVEFORMATEX*)lpDAI->asi[i]->lpFormat)->nBlockAlign *= (unsigned short)j;
				} else {
					a->SetFrameMode(FRAMEMODE_ON);
					if  (!lpDAI->settings->GetInt("output/avi/mp3/cbr frame mode")) {
						sprintf(Buffer,LoadString(STR_MUXLOG_MP3CBRCANTDISFM));
					} else {
						sprintf(Buffer,LoadString(STR_MUXLOG_MP3CBRENFM));
					}
					
					lpDAI->dlg->AddProtocolLine(Buffer,5);
				}
			}
		}

	}

	cStr[0]=LoadString(STR_MUXLOG_SUBTITLES);
	wsprintf(Buffer,cStr[0],lpDAI->dwNbrOfSubs);
	lpDAI->dlg->AddProtocolLine(Buffer,5);
	for (i=0;i<lpDAI->dwNbrOfSubs;i++)
	{
		wsprintf(Buffer,LoadString(STR_MUXLOG_STREAM),i+1);
		lpDAI->dlg->AddProtocolLine(Buffer,5);

		lpDAI->ssi[i]->lpsubs->GetName(cName);
		sprintf(Buffer,LoadString(STR_MUXLOG_NAME),cName);
		lpDAI->dlg->AddProtocolLine(Buffer,5, APL_UTF8);
	}

// Zero values for status display
	lpmsState=(MUX_STATE*)malloc(sizeof(MUX_STATE)*MUXSTATEENTRIES);
	for (i=0;i<MUXSTATEENTRIES;i++)
	{
		lpmsState[i].qwWritten=0;
		lpmsState[i].dwTime=GetTickCount();
		lpmsState[i].dwFrames=0; 
	}
// seek video to beginning
	lpDAI->videosource->ReInit();
	i=0;
	while (!lpDAI->videosource->IsKeyFrame(i++)) {
		for (j=0;j<lpDAI->dwNbrOfAudioStreams;) {
			lpDAI->asi[j++]->iDelay+=lpDAI->videosource->GetMicroSecPerFrame()/1000; }}
	i--;
	{ __int64 iTime = i*lpDAI->videosource->GetNanoSecPerFrame()+1;
		if (i) lpDAI->videosource->Seek(iTime);
	}
	lpDAI->dwMaxFrames-=i;

	if (i) {
		wsprintf(cBuffer,LoadString(STR_MUXLOG_DFRATBEGINNING),i);
		lpDAI->dlg->AddProtocolLine(cBuffer,4);
	}

	dlg=lpDAI->dlg;

// open first file
	wsprintf(cBuffer,lpDAI->lpFormat,RawFilename,1);

	lpDAI->dlg->AddProtocolLine(Message_index(MSG_UNBUFFERED, bUnbufferedOutput), 4);
	lpDAI->dlg->AddProtocolLine(Message_index(MSG_OVERLAPPEDOUT, bOverlapped), 4);

	DestFile=new FILESTREAM;
	
	if (DestFile->Open(cBuffer,
			(bUnbufferedOutput?STREAM_UNBUFFERED_WRITE:STREAM_WRITE) | 
			(bOverlapped?STREAM_OVERLAPPED:0))==STREAM_ERR) {

		cStr[0]=LoadString(IDS_COULDNOTOPENOUTPUTFILE);
		lpDAI->dlg->MessageBox(cStr[0],lpDAI->dlg->cstrError,MB_OK | MB_ICONERROR);
		lpDAI->dlg->ButtonState_STOP();
		lpDAI->dlg->SetDialogState_Config();
		FinishMuxing(lpDAI);
		cacheAllowReadAhead(0);
		return 0;
	}

	if (bUnbufferedOutput || bOverlapped) {
		cache = new CACHEDSTREAM(4, 1<<21);
		cache->Open(DestFile, CACHE_WRITE);
	} else {
		cache = (CACHEDSTREAM*)DestFile;
	}
	
	lpDAI->dlg->SetDlgItemText(IDC_DESTFILE,cBuffer);
	lstrcpy(cFilename,cBuffer);

// open first output file
	AVIOut=new AVIFILEEX;
	AVIOut->SetDebugState(DS_DEACTIVATE);
	AVIOut->Open(cache,FA_WRITE,(bOpenDML)?AT_OPENDML:AT_STANDARD);
	AVIOut->SetNumberOfStreams(lpDAI->dwNbrOfAudioStreams+1+lpDAI->dwNbrOfSubs);
	AVIOut->SetPadding(lpDAI->dwPadding);
	AVIOut->SetLegacyCallBack(StartLegacyCB);

	// setting Haali muxing mode if it has been chosen and is possible
	AVIOut->EnableLowOverheadMode(!!(lpDAI->settings->GetInt("output/avi/opendml/haalimode")));
	lpDAI->dlg->AddProtocolLine(LoadString(MSG_LOWOVHDAVI[!!AVIOut->IsLowOverheadMode()]),4);

	if (lpDAI->cTitle) AVIOut->SetTitle(lpDAI->cTitle->Get());

	ComposeVersionString(cBuffer);
	AVIOut->SetWritingAppName(cBuffer);
	if (s->GetInt("output/avi/opendml/stdindex/pattern") == SIP_FRAMES)
	{
		AVIOut->SetFramesPerIndex((int)s->GetInt("output/avi/opendml/stdindex/interval"));
	}
	if (bLegacyIndex) AVIOut->Enable(AFE_LEGACYINDEX);
	qwNanoSecNeeded=(__int64*)malloc(8*lpDAI->dwNbrOfAudioStreams);
	ZeroMemory(qwNanoSecNeeded,8*lpDAI->dwNbrOfAudioStreams);
	qwNanoSecStored=(__int64*)malloc(8*lpDAI->dwNbrOfAudioStreams);
	ZeroMemory(qwNanoSecStored,8*lpDAI->dwNbrOfAudioStreams);

// set stream headers and formats
	AVIOut->SetStreamHeader(0,lpDAI->videosource->GetAVIStreamHeader());
	AVIOut->SetStreamFormat(0,lpDAI->videosource->GetFormat());
	AVIOut->GetStdIndexOverhead();

// seek audio streams to beginning
	for (i=0;i<lpDAI->dwNbrOfAudioStreams;i++)
	{
		AUDIOSOURCE* a = lpDAI->asi[i]->audiosource;

		AVIOut->SetStreamHeader(i+1,lpDAI->asi[i]->lpASH);
		AVIOut->SetStreamFormat(i+1,lpDAI->asi[i]->lpFormat);
		a->GetName(Buffer);
		AVIOut->SetStreamName(i+1,Buffer);
		a->ReInit();
		AVIOut->SetStreamDefault(i+1, !!lpDAI->asi[i]->audiosource->IsDefault());
	}

// subtitles
	for (i=0;i<lpDAI->dwNbrOfSubs;i++)
	{
		AVIOut->SetStreamHeader(i+1+lpDAI->dwNbrOfAudioStreams,lpDAI->ssi[i]->lpash);
		AVIOut->SetStreamFormat(i+1+lpDAI->dwNbrOfAudioStreams,NULL);
		lpDAI->ssi[i]->lpsubs->SetBias(0,BIAS_ABSOLUTE | BIAS_UNSCALED);
		lpDAI->ssi[i]->lpsubs->ReInit();
		AVIOut->SetStreamDefault(i+1+lpDAI->dwNbrOfAudioStreams, !!lpDAI->ssi[i]->lpsubs->IsDefault());
	}
	ZeroMemory(qwStats,sizeof(qwStats));
	AVIOut->SetNanoSecPerFrame(lpDAI->videosource->GetNanoSecPerFrame());
	AVIOut->SetMaxRIFFAVISize(i = (1<<20)*(int)lpDAI->settings->GetInt("output/avi/opendml/riff avi size")); 

	QW2Str((__int64)((float(i)+(1<<19))/(1<<20)), cSize[0], 1);
	sprintf(Buffer, LoadString(STR_MUXLOG_FIRSTRIFFSIZE), cSize);
	lpDAI->dlg->AddProtocolLine(Buffer, 4);

	dwLegIndOverHead=8*((!bOpenDML)||(bLegacyIndex));
	AddOverhead(qwStats,AVIOut->GetHeaderSpace()+12+dwLegIndOverHead);
	lpBuffer=malloc(1<<22);
	dwTime=GetTickCount();
	dwAudioBPS=0;

	bManual_splitpoint_reached = false;
	bFilesize_reached = false;
	wsprintf(cBuffer,LoadString(STR_MUXLOG_NEWFILE),0,0,0,0,0,cFilename);
	lpDAI->dlg->AddProtocolLine(cBuffer,5);
	dwFramesInCurrFile=0;
	dwChunkOverhead=(bOpenDML)?(AVIOut->IsLegacyEnabled())?32:16:24;

// compensate for delay
	for (i=0;i<lpDAI->dwNbrOfAudioStreams;i++)
	{
		dwAudioBPS+=lpDAI->asi[i]->audiosource->GetAvgBytesPerSec();
		iDelay=lpDAI->asi[i]->iDelay;
		if (iDelay)
		{
			if (iDelay<0)
			{
				while (iDelay<0)
				{
					// delay < 0 => remove beginning of stream
					lpDAI->asi[i]->audiosource->Read(lpBuffer,20000,&dwMicroSecRead,NULL);
					iDelay+=(int)round(dwMicroSecRead/1000);
				}
			}
			else
			{
				// delay > 0 => duplicate beginning or insert silence
				lpwfe=(WAVEFORMATEX*)lpDAI->asi[i]->lpFormat;
				bExternalSilence=(silence->SetFormat(lpDAI->asi[i]->dwType,lpwfe->nChannels,lpwfe->nSamplesPerSec,
					(float)lpDAI->asi[i]->audiosource->GetAvgBytesPerSec()/125)==SSF_SUCCEEDED);
				while (iDelay>0)
				{
					if (!bExternalSilence)
					{
						// compatible silence file present
						iSize=lpDAI->asi[i]->audiosource->Read(lpBuffer,min(iDelay*1000,20000),&dwMicroSecRead,NULL);
						lpDAI->asi[i]->audiosource->Seek(0);
					}
					else
					{
						// compatible silence file not present
						iSize=silence->Read(lpBuffer,min(iDelay*1000,20000),&dwMicroSecRead,NULL);
					}
					
					dwFlags=AVIIF_KEYFRAME;
					AVIOut->AddChunk(i+1,&lpBuffer,iSize,dwFlags);
					if (!lpBuffer) lpBuffer=malloc(1<<20);
					iDelay-=(int)round(dwMicroSecRead/1000);

					AddAudioSize(qwStats,iSize);
					AddOverhead(qwStats,dwChunkOverhead);
					qwNanoSecNeeded[i]-=dwMicroSecRead*1000;
					qwNanoSecStored[i]+=dwMicroSecRead*1000;
				}
			}
		}
	}

	AVIOut->FlushWriteCache();
	dwAudioBPVF=(DWORD)((double)dwAudioBPS*(double)(lpDAI->videosource->GetNanoSecPerFrame())/1000000000);
// set preload
	for (i=0;i<lpDAI->dwNbrOfAudioStreams;i++)
	{
		qwNanoSecNeeded[i]+=lpDAI->dwPreload*1000000;
	}
	bPreloadCompensated=false;
	
	dwPreloadedFrames=(DWORD)(d_div(1000000.0f*(double)lpDAI->dwPreload,(double)lpDAI->videosource->GetNanoSecPerFrame(),
		"MuxThread::lpDAI->videosource->GetNanoSecPerFrame()"));

	lpDAI->dlg->m_Prg_Progress.SetRange32(0,10000);
	bFinishImmediately=false;
	lpDAI->videosource->InvalidateCache();
	bFirst=true;
	dwLastFileBegin=0;
	lpDAI->lpProtocol->EnsureVisible(lpDAI->lpProtocol->GetItemCount()-1,false);

	CWinThread* thread;
	DISPLAY_PROGRESS_INFO*	lpDPI;

	lpDPI = (DISPLAY_PROGRESS_INFO*)malloc(sizeof(*lpDPI));
// lpDAI,qwStats,&dwTime,dwFrameCountTotal,lpmsState,&dwMuxStatePos
	lpDPI->lpdwFrameCountTotal=&dwFrameCountTotal;
	lpDPI->qwStats=qwStats;
	lpDPI->dwTime=&dwTime;
	lpDPI->lpDAI=lpDAI;
	lpDPI->dwMuxState=lpmsState;
	lpDPI->dwMuxStatePos=&dwMuxStatePos;
	lpDPI->dwLeave=0;
	iProgress=0;
	lpDPI->dwByteAccuracy=1;
	lpDPI->lpiProgress = &iProgress;
	lpDPI->iDuration = lpDAI->videosource->GetDuration() * lpDAI->videosource->GetTimecodeScale() / 1000000;

	int iError = 0;
	if (ResolveSplitpoints(lpDAI, RSPF_CHAPTERS, &iError) == RSP_ERROR) {
		sprintf(cBuffer,LoadString(STR_MUXLOG_SPLITPOINTB0RKED),iError+1);
		lpDAI->dlg->MessageBox(cBuffer,LoadString(STR_GEN_ERROR),MB_OK | MB_ICONERROR);
		lpDAI->dlg->SetDialogState_Config();
		lpDAI->dlg->ButtonState_STOP();
		lpDAI->chapters->SetBias(0,BIAS_UNSCALED);
		CloseHandle(hSem);
		cacheAllowReadAhead(0);

		return 0;
	}

	thread=AfxBeginThread((AFX_THREADPROC)DisplayProgress_Thread,lpDPI);
	lpDPI->lpThis = thread;

//	ResolveSplitpoints(lpDAI, RSPF_CHAPTERS);
	
	v = lpDAI->videosource;
	i_vs_nspf = v->GetNanoSecPerFrame();
	
	iDistToSplitpoint = DistToNextSplitPoint(lpDAI,dwFrameCountTotal,0)-1;
	dwReclistOverhead=(bOpenDML)?12:28;
// mux
	//while ((dwFrameCountTotal<lpDAI->dwMaxFrames)&&(!bStop))
	while ((!v->IsEndOfStream())&&(!bStop))
	{
		dwChunkOverhead=(bOpenDML)?(AVIOut->IsLegacyEnabled())?32:16:24;
    // make rec-Lists?
		WaitForSingleObject(hSem, INFINITE);
		if (bReclists)
		{
			if (!AVIOut->IsRecListOpen())
			{
				AVIOut->BeginRECList();
				AddOverhead(qwStats,dwReclistOverhead);
			}
			bRecListEmpty=true;
			wsprintf(cBuffer,"begin rec list at frame %d",dwFrameCountTotal);
			lpDAI->dlg->AddProtocolLine(cBuffer,3);
		}
		j=0; dwRecListSize=0;
		bFinishImmediately=false;
		ReleaseSemaphore(hSem, 1, NULL);
	// make next video block
		while (
			//(dwFrameCountTotal<lpDAI->dwMaxFrames)
			(!v->IsEndOfStream())&&
			(((j<ai_value)&&(ai_unit==DAI_IS_FRAME))||((dwRecListSize<ai_value<<10)&&
													(ai_unit==DAI_IS_KB)))&&
			((bMustCorrectAudio)||(!bFinishPart)||(!(bKeyframe=v->IsKeyFrame(CN_CURRENT_CHUNK))))&&
			(!bFinishImmediately)&&
			(!bFirst))
		{
			WaitForSingleObject(hSem, INFINITE);
//			bKeyframe=lpDAI->videosource->IsKeyFrame(CN_CURRENT_CHUNK);
			bMustCorrectAudio=false;
			ADVANCEDREAD_INFO aari;
			ZeroMemory(&aari,sizeof(aari));
			if (v->GetFrame(lpBuffer,(DWORD*)&iSize,&iTimecode,&aari)!=VS_OK)
			{
				cStr[0]=LoadString(IDS_VIDEOSOURCECURRUPT);
				wsprintf(Buffer,cStr[0].GetBuffer(255),i+1);
				lpDAI->dlg->MessageBox(Buffer,lpDAI->dlg->cstrError,MB_OK | MB_ICONERROR);
				bStop=true;
			}
			int iRefCount;
			v->GetLatestReference(&iRefCount);
			iTimecode = aari.iNextTimecode * v->GetTimecodeScale();
			bKeyframe=!iRefCount;
			
			dwFlags=(bKeyframe)?AVIIF_KEYFRAME:0;
			bKeyframe_passed = bKeyframe;
			
			AVIOut->AddChunk(0,&lpBuffer,iSize,dwFlags);
			if (!lpBuffer) lpBuffer=malloc(1<<20);
			dwFrameSizes_Sum -= dwFrameSizes[dwFrameSizes_Pos];
			dwFrameSizes[dwFrameSizes_Pos++]=iSize;
			dwFrameSizes_Pos%=dwFrameSizes_Len;
			dwFrameSizes_Sum += iSize;

			dwRecListSize+=iSize+dwAudioBPVF;
		// update statistics
			AddVideoSize(qwStats,iSize);
			AddOverhead(qwStats,dwChunkOverhead);

			for (i=0;i<lpDAI->dwNbrOfAudioStreams;
			  qwNanoSecNeeded[i++]+=i_vs_nspf);

			j++;
			dwFrameCountTotal++;
			iProgress = v->GetNanoSecPerFrame() * dwFrameCountTotal;
		// max file size reached -> compensate for preload
			bFilesize_reached = (qwStats[3]+dwPreloadedFrames*(dwAudioBPVF+(dwFrameSizes_Sum/dwFrameSizes_Len))>=qwMaxBytes);
			bManual_splitpoint_reached = ((iDistToSplitpoint<(dwPreloadedFrames+25)*v->GetNanoSecPerFrame())||(lpDAI->dwMaxFrames-dwFrameCountTotal<dwPreloadedFrames+5));
			
			if  (bFilesize_reached||bManual_splitpoint_reached)
			{
				if (!bPreloadCompensated) for (i=0;i<lpDAI->dwNbrOfAudioStreams;qwNanoSecNeeded[i++]-=lpDAI->dwPreload*1000000);
				bPreloadCompensated=true;
				if (bFilesize_reached) bFinishPart=true;
				
				QW2Str(qwStats[3],cSize[0],10);
				if ((!bSplitInitiated)&&(bFilesize_reached))
				{
					if (dwPartNbr<lpDAI->dwMaxFiles) {
						wsprintf(cBuffer,LoadString(STR_MUXLOG_FILESIZEREACHED),dwFrameCountTotal-dwLastFileBegin,
							cSize[0],LoadString(STR_BYTES));
						lpDAI->dlg->AddProtocolLine(cBuffer,4);
					}
					bSplitInitiated=true;
					bKeyframe_passed=false;
				}
			}
			if ((qwStats[3]>qwMaxBytes)&&(bKeyframe_passed)&&(dwPartNbr<lpDAI->dwMaxFiles))
			{
				QW2Str(dwFrameCountTotal-dwLastFileBegin,cSize[0],3);
				wsprintf(cBuffer,LoadString(STR_MUXLOG_KEYFRAMEPASSED),cSize[0]);
				lpDAI->dlg->AddProtocolLine(cBuffer,4);
				bKeyframe_passed=false;
			}

			if (iDistToSplitpoint<v->GetNanoSecPerFrame())
			{
//				bFinishImmediately=true;
				bFinishPart=true;
			}
			bRecListEmpty=false;
			{
				iDistToSplitpoint-=v->GetNanoSecPerFrame();
			}
			ReleaseSemaphore(hSem, 1, NULL);
		}
		if (dwPartNbr==lpDAI->dwMaxFiles) 
		{
			bFinishPart=false;
			bFinishImmediately=false;
		}
	// add audio
		wsprintf(cBuffer,"  frames total: %6d",dwFrameCountTotal);
		lpDAI->dlg->AddProtocolLine(cBuffer,3);

		bFirst=false;
		dwAudioBPS=0;
		for (i=0;i<lpDAI->dwNbrOfAudioStreams;i++)
		{
			WaitForSingleObject(hSem, INFINITE);
			if (dwFrameCountTotal==lpDAI->dwMaxFrames)
			{
				iSize=1;
			}
			iSize=1;
			dwAudioBPS+=lpDAI->asi[i]->audiosource->GetAvgBytesPerSec();
			// as long as more audio is required
			while ((qwNanoSecNeeded[i]>=20000000)&&(iSize>0))
			{
				// read
				if (qwNanoSecNeeded[i]/1000<1500000)
				{
					dwNextAudioBlock=(DWORD)qwNanoSecNeeded[i]/1000;
				}
				else
				{
					dwNextAudioBlock=1000000;
				}
				iSize=lpDAI->asi[i]->audiosource->Read(lpBuffer,
					dwNextAudioBlock,NULL,&qwNanoSecRead);
				if (!iSize) {
					iSize=lpDAI->asi[i]->audiosource->Read(lpBuffer,
						dwNextAudioBlock,NULL,&qwNanoSecRead);
				}
				// if something has been read
				if (iSize>0)
				{
					// write it
					bRecListEmpty=false;
					dwFlags=AVIIF_KEYFRAME;
					AVIOut->AddChunk(i+1,&lpBuffer,iSize,dwFlags);
					if (!lpBuffer) lpBuffer=malloc(1<<20);
					qwNanoSecNeeded[i]-=qwNanoSecRead;
					qwNanoSecStored[i]+=qwNanoSecRead;
					// update statistics

					AddAudioSize(qwStats,iSize);
					AddOverhead(qwStats,dwChunkOverhead);
				}
				// otherwise: couldn't read from stream
				else
				{
					// if its end had not yet been reached: bad
					if (!lpDAI->asi[i]->audiosource->IsEndOfStream())
					{
						cStr[0]=LoadString(IDS_AUDIOSOURCECORRUPT);
						wsprintf(Buffer,cStr[0].GetBuffer(255),i+1);
						lpDAI->dlg->MessageBox(Buffer,lpDAI->dlg->cstrError,MB_OK | MB_ICONERROR);
						bStop=true;
					}
					AVIOut->FinishStream(i+1,qwNanoSecStored[i]/1000);
				}
			}
			ReleaseSemaphore(hSem,1,NULL);
		}
		
		wsprintf(cBuffer,"  audio: %6d bytes",iSize);
		lpDAI->dlg->AddProtocolLine(cBuffer,3);

		dwAudioBPVF=(DWORD)((double)dwAudioBPS*(double)(i_vs_nspf)/1000000000);
		// close rec-list if necessary
		if (bReclists)
		{
			if (!bRecListEmpty) 
			{
				AVIOut->EndRECList();
				lpDAI->dlg->AddProtocolLine("End rec list",3);
			}
		}
		bMustCorrectAudio=false;
		if (!bFinishImmediately)
		{
			for (i=0;i<lpDAI->dwNbrOfAudioStreams;bMustCorrectAudio|=(abs((int)(qwNanoSecNeeded[i++]/1000))>25000));
		}
		//if (lpDAI->dwMaxFrames==dwFrameCountTotal)
		if (v->IsEndOfStream())
		{
			bFinishPart=true;
			bMustCorrectAudio=false;
			lpDAI->dwMaxFiles=1<<30;
		}

	//	if (bStop) bFinishPart = true;
		if (
			  ((bFinishPart)&&
			  (!bMustCorrectAudio)&&
			  ((bFinishImmediately)||(v->IsKeyFrame(CN_CURRENT_CHUNK)))&&
			  (lpDAI->dwMaxFiles>dwPartNbr))
			  ||
			  (
			   (
			   //lpDAI->dwMaxFrames==dwFrameCountTotal
			   v->IsEndOfStream()
			   )||(bStop)
			  )
		   )
		{
			// add subtitles
			CACHEDSTREAM* nextcache;

			WaitForSingleObject(hSem, INFINITE);
			if (lpDAI->dwNbrOfSubs)
			{
				if (bReclists) 
				{
					AVIOut->BeginRECList();
					AddOverhead(qwStats,dwReclistOverhead);
				}
			
				for (i=0;i<lpDAI->dwNbrOfSubs;i++)
				{
					__int64	qwLen;

					lpDAI->ssi[i]->lpsubs->SetBias(i_vs_nspf*dwLastFileBegin,
						BIAS_ABSOLUTE | BIAS_UNSCALED);
					lpDAI->ssi[i]->lpsubs->SetRange(0,qwLen=i_vs_nspf*(dwFrameCountTotal-dwLastFileBegin));

					iSize=lpDAI->ssi[i]->lpsubs->Render2AVIChunk(lpBuffer);
					AVIOut->AddChunk(1+i+lpDAI->dwNbrOfAudioStreams,&lpBuffer,iSize,AVIIF_KEYFRAME);
					if (!lpBuffer) lpBuffer=malloc(1<<20);
					AddOverhead(qwStats,dwChunkOverhead);
					AddSubtitleSize(qwStats,iSize);
					AVIOut->FinishStream(1+lpDAI->dwNbrOfAudioStreams+i,qwLen/1000);
				}

				if (bReclists) AVIOut->EndRECList();
			}

			lpDAI->dlg->AddProtocolLine("subtitles added",3);

			for (i=0;i<lpDAI->dwNbrOfAudioStreams;i++)
			{
				AVIOut->FinishStream(i+1,qwNanoSecStored[i]/1000);
			}

			lpDAI->dlg->AddProtocolLine("stream lengths set",2);

			// new output file
			if (lpDAI->dwMaxFrames!=dwFrameCountTotal && !bStop)
			{
				bFinishPart=false;
				dwPartNbr++;
				bSplitInitiated=false;
				ZeroMemory(cFilename,sizeof(cFilename));
				wsprintf(cFilename,lpDAI->lpFormat,RawFilename,dwPartNbr);
				
				lstrcpy(cBuffer,cFilename);				
				
				NextDestFile=new FILESTREAM;

				if (NextDestFile->Open(cBuffer,
					(bUnbufferedOutput?STREAM_UNBUFFERED_WRITE:STREAM_WRITE) | 
					(bOverlapped?STREAM_OVERLAPPED:0))==STREAM_ERR) {

//				if (NextDestFile->Open(cBuffer,STREAM_WRITE)==STREAM_ERR)
//				{
					AVIOut->FlushWriteCache();
					AVIOut->Close();
					delete AVIOut;
					cache->Close();
					delete cache;
					cStr[0]=LoadString(IDS_COULDNOTOPENOUTPUTFILE);
					lpDAI->dlg->MessageBox(cStr[0],lpDAI->dlg->cstrError,MB_OK | MB_ICONERROR);
					lpDAI->dlg->ButtonState_STOP();
					lpDAI->dlg->SetDialogState_Config();

					ReleaseSemaphore(hSem, 1, NULL);
					HANDLE hSem_finish = CreateSemaphore(NULL,0,1,sem_name_finish);
					lpDPI->dwLeave=1;
	
					WaitForSingleObject(hSem_finish, INFINITE);
					FinishMuxing(lpDAI);
			
					CloseHandle(hSem_finish);
					CloseHandle(hSem);
					delete lpDPI;

					cacheAllowReadAhead(0);

					return 0;
				}
				if (bUnbufferedOutput || bOverlapped) {
					nextcache = new CACHEDSTREAM(4, 1<<21);
					nextcache->Open(NextDestFile, CACHE_WRITE);
				} else {
					nextcache = (CACHEDSTREAM*)NextDestFile;
				}

				NextAVIOut=new AVIFILEEX;
				NextAVIOut->SetDebugState(DS_DEACTIVATE);
				NextAVIOut->Open(nextcache,FA_WRITE,(bOpenDML)?AT_OPENDML:AT_STANDARD);
				NextAVIOut->SetNumberOfStreams(lpDAI->dwNbrOfAudioStreams+1+lpDAI->dwNbrOfSubs);
				NextAVIOut->SetNanoSecPerFrame(i_vs_nspf);
				NextAVIOut->SetMaxRIFFAVISize(i = (1<<20)*(int)lpDAI->settings->GetInt("output/avi/opendml/riff avi size")); 
				NextAVIOut->EnableLowOverheadMode(!!(lpDAI->settings->GetInt("output/avi/opendml/haalimode")));
				for (i=0;i<lpDAI->dwNbrOfAudioStreams+1+lpDAI->dwNbrOfSubs;i++)
				{
					NextAVIOut->SetStreamHeader(i,AVIOut->GetStreamHeader(i));
					NextAVIOut->SetStreamFormat(i,AVIOut->GetStreamFormat(i));
					NextAVIOut->SetStreamDefault(i, AVIOut->IsDefault(i));
				}
				for (i=0;i<lpDAI->dwNbrOfAudioStreams;i++)
				{
					lpDAI->asi[i]->audiosource->GetName(Buffer);
					NextAVIOut->SetStreamName(i+1,Buffer);
				}
				ComposeVersionString(Buffer);
				NextAVIOut->SetWritingAppName(Buffer);
			}

			AVIOut->FlushWriteCache();
			qwStreamSizes=(__int64*)malloc(sizeof(__int64)*(lpDAI->dwNbrOfAudioStreams));
			for (i=1;i<=lpDAI->dwNbrOfAudioStreams;qwStreamSizes[-1+i++]=AVIOut->GetStreamSize(i));
			AVIOut->Close();
			dwOverhead=AVIOut->GetStdIndexOverhead();
			cache->Close();
			delete AVIOut;
			delete cache;
			AddOverhead(qwStats,dwOverhead);
			cache = nextcache;

			j=1+lpDAI->dwNbrOfAudioStreams;
			for (i=1;j--;i=(i+1)%(1+lpDAI->dwNbrOfAudioStreams))
			{
				if (!i)
				{
					dwFramesInCurrFile=dwFrameCountTotal-dwLastFileBegin;
					qwMillisec=((__int64)dwFramesInCurrFile)*i_vs_nspf/1000000;
				}
				else
				{
					qwMillisec=qwNanoSecStored[i-1]/1000000;
				}

				Millisec2HMSF(qwMillisec,&dwHour,&dwMin,&dwSec,&dwFrac);

				if (!i)
				{
					cStr[0]=LoadString(STR_MUXLOG_FILECLOSED);
					dwMaxDeviation=max(dwMaxDeviation,(DWORD)max(0,qwStats[3]-(qwMaxBytes)));
					QW2Str(qwStats[3],cSize[0],3);
					QW2Str(max(0,qwStats[3]-(qwMaxBytes)),cSize[1],3);

					wsprintf(cBuffer,cStr[0],cSize[0],LoadString(STR_BYTES),cSize[1],LoadString(STR_BYTES),
					dwHour,dwMin,dwSec,dwFrac,dwFramesInCurrFile);
					lpDAI->dlg->AddProtocolLine(cBuffer,4);
				}
				else
				{
					if (lpDAI->dwNbrOfAudioStreams)
					{
						cStr[0]=LoadString(STR_MUXLOG_AUDIOSTREAMSIZE);
						QW2Str(qwStreamSizes[i-1],cSize[0],10);
						wsprintf(cBuffer,cStr[0],i,cSize[0],LoadString(STR_BYTES),
							dwHour,dwMin,dwSec,dwFrac);
						lpDAI->dlg->AddProtocolLine(cBuffer,4);
					}
				}
			}
			delete qwStreamSizes;
			qwStreamSizes=NULL;

			if (lpDAI->dwMaxFrames!=dwFrameCountTotal && !bStop)
			{
				lpDAI->dlg->AddProtocolLine("-------------------------------------------------------------",4);
				qwMillisec=(__int64)(dwFrameCountTotal*i_vs_nspf/1000000);
				
				Millisec2HMSF(qwMillisec,&dwHour,&dwMin,&dwSec,&dwFrac);
				cStr[0]=LoadString(STR_MUXLOG_NEWFILE);
				wsprintf(cBuffer,cStr[0],dwHour,dwMin,dwSec,dwFrac,dwFrameCountTotal,cFilename);
				lpDAI->dlg->AddProtocolLine(cBuffer,5);

				AVIOut=NextAVIOut;
				AVIOut->SetPadding(lpDAI->dwPadding);
				if (s->GetInt("output/avi/opendml/stdindex/pattern") == SIP_FRAMES) {
					AVIOut->SetFramesPerIndex((int)s->GetInt("output/avi/opendml/stdindex/interval"));
				}
				if (bLegacyIndex) AVIOut->Enable(AFE_LEGACYINDEX);
				AVIOut->SetLegacyCallBack(StartLegacyCB);

				for (i=0;i<4;qwStats[i++]=0); qwStats[8]=0;

				AddOverhead(qwStats,AVIOut->GetHeaderSpace()+12+dwLegIndOverHead);
				DestFile=NextDestFile;

				lpDAI->dlg->SetDlgItemText(IDC_DESTFILE,cFilename);
				for (i=0;i<lpDAI->dwNbrOfAudioStreams;qwNanoSecNeeded[i++]+=lpDAI->dwPreload*1000000);
				bPreloadCompensated=false;
				ZeroMemory(qwNanoSecStored,8*lpDAI->dwNbrOfAudioStreams);
				dwLastFileBegin=dwFrameCountTotal;
				dwFramesInCurrFile=0;
				if (iDistToSplitpoint<=0)
				{
					iDistToSplitpoint = DistToNextSplitPoint(lpDAI,dwFrameCountTotal+1,iTimecode)-1;
				}

			}
			else
			{
				bStop=true;
			}
			ReleaseSemaphore(hSem, 1, NULL);
		}
	}

	lpDPI->dwLeave=1;
	HANDLE hSem_finish = CreateSemaphore(NULL,0,1,sem_name_finish);
	WaitForSingleObject(hSem_finish, INFINITE);
	CloseHandle(hSem_finish);
	delete lpDPI;

	lpDAI->dlg->ButtonState_STOP();
	bStop=false;

	QW2Str(dwMaxDeviation,cSize[0],3);
	wsprintf(Buffer,LoadString(STR_MUXLOG_MAXEXCEED),cSize[0],LoadString(STR_BYTES));
	lpDAI->dlg->AddProtocolLine(Buffer,4);

	cStr[0]=LoadString(STR_MUXLOG_DONE);
	lpDAI->dlg->AddProtocolLine(cStr[0],5);
	cStr[0]=LoadString(IDS_READY);

	if (lpDAI->dwNbrOfAC3Streams)
	{
		msglist=new MSG_LIST;
		msglist->lpMsg=NULL;
		msglist->lpNext=NULL;
		for (i=0;i<lpDAI->dwNbrOfAC3Streams;i++)
		{
			bAC3broken|=lpDAI->lpAC3_logs[i]->bBroken;
		}
		if (bAC3broken)
		{
			cStr[0]=LoadString(IDS_CRAPAC3DATA);
			MSG_LIST_append(msglist,cStr[0].GetBuffer(255));
		
			for (i=0;i<lpDAI->dwNbrOfAC3Streams;i++)
			{
				if (lpDAI->lpAC3_logs[i]->bBroken)
				{
					MSG_LIST_2_Msg(lpDAI->lpAC3_logs[i]->lpMessages,stream_report);
					MSG_LIST_append(msglist,stream_report);
				}
			}
			MSG_LIST_append(msglist,"");
		}

		cStr[0]=LoadString(IDS_READY);
		MSG_LIST_append(msglist,cStr[0].GetBuffer(255));
		MSG_LIST_2_Msg(msglist,report);

		if (lpDAI->bDoneDlg) lpDAI->dlg->MessageBox(report,lpDAI->dlg->cstrInformation,MB_OK | MB_ICONINFORMATION);

		MSG_LIST_clear(msglist);
		msglist=NULL;
	}
	else
	{
		if (lpDAI->bDoneDlg) lpDAI->dlg->MessageBox(cStr[0],lpDAI->dlg->cstrInformation,MB_OK | MB_ICONINFORMATION);
	}
//	delete lpDPI;
	lpDAI->dlg->SetDialogState_Config();
	v->Enable(0);
	for (i=0;i<lpDAI->dwNbrOfAudioStreams;lpDAI->asi[i++]->audiosource->Enable(0));
	free(lpDAI->lpFormat);
	free(lpDAI->asi);
	if (lpDAI->ssi&&lpDAI->dwNbrOfSubs) free(lpDAI->ssi);
	free(lpDAI->lpFileName);
	
	for (i=0;i<lpDAI->dwNbrOfAC3Streams;i++)
	{
		if (lpDAI->lpAC3_logs[i]) 
		{
			if (lpDAI->lpAC3_logs[i]->lpMessages) delete lpDAI->lpAC3_logs[i]->lpMessages;
			delete lpDAI->lpAC3_logs[i];
		}
	}
	if (lpDAI->lpAC3_logs) free(lpDAI->lpAC3_logs);
	lpDAI->dlg->AddProtocol_Separator();
//	lpDAI->dlg->AddProtocolLine("================================================================",5);
	bool bExit = lpDAI->bExitAfterwards;
	CDialog* c = lpDAI->dlg;
	free(lpmsState);
	free(qwNanoSecNeeded);
	free(qwNanoSecStored);
	free(lpBuffer);
	delete report;
	delete cBuffer;
	delete stream_report;
	delete dwFrameSizes;
	delete cFilename;
	silence->Close();
	delete silence;
	if (bExit) c->PostMessage(WM_CLOSE,0,0);
	bMuxing = false;
	CloseHandle(hSem);	
	FinishMuxing(lpDAI);
	free(lpDAI);
	cacheAllowReadAhead(0);

	return 1;
}

typedef struct
{
	__int64	iLength;
} LACE_DESCRIPTOR;

const int STREAMTYPE_VIDEO    = 0x01;
const int STREAMTYPE_AUDIO    = 0x02;
const int STREAMTYPE_SUBTITLE = 0x03;

int GetStreamNumber(DEST_AVI_INFO* lpDAI, int iType, int iNumber)
{
	switch (iType) {
		case STREAMTYPE_VIDEO:
			return iNumber;
		case STREAMTYPE_AUDIO:
			return iNumber+lpDAI->dwNbrOfVideoStreams;
		case STREAMTYPE_SUBTITLE:
			return iNumber+lpDAI->dwNbrOfAudioStreams+lpDAI->dwNbrOfVideoStreams;
	}

	return -1;
}

bool IsEndOfMMS(MULTIMEDIASOURCE** mms, int iCount)
{
	bool bRes = true;
	for (int j=0;j<iCount;bRes&=mms[j++]->IsEndOfStream());
	return bRes;
}

int MuxThread_MKV(DEST_AVI_INFO* lpDAI)
{
	WaitForMuxing(lpDAI);
	CAttribs*   s = lpDAI->settings;
	MATROSKA*	m;
	MUX_STATE*	lpmsState;
	FILESTREAM* DestFile;
	VIDEOSOURCE* v = lpDAI->videosource;
	AUDIOSOURCE* a;
	char		cBuffer[500];
	cBuffer[0] = 0;
	char		RawFilename[200];
	char		cFilename[500];
	int			i,j;
	__int64*	qwNanoSecNeeded=NULL;
	__int64*	qwNanoSecStored=NULL;

	__int64*	qwNanoSecStored_subs=NULL;

	__int64		qwVideoRead = 0;
	__int64		qwVideoReadTotal = 0;
	__int64		qwNanoSecRead;
	__int64		qwStats[10];
	__int64		qwBytesWritten = 0;
	__int64		qwMaxAudio;
	__int64		qwBias;
	__int64		iLace_ns;
	__int64		iInputTimecode;
	__int64		iDist2Splitpoint;
	__int64		iLastTimecode;
	LACE_DESCRIPTOR*	audio_lace_config = NULL;

	__int64		i_vs_nspf;
	bool		bUnbufferedOutput = !!lpDAI->settings->GetInt("output/general/unbuffered");
	bool		bOverlapped = 0;//!!lpDAI->settings->GetInt("output/general/overlapped");

	__int64		iProgress;
	int			iSourceLaces;
	int			dwHour,dwMin,dwSec,dwFrac;

	int			iCurrentFile = 1;
	int			iTrackCount;
	bool		bSplitFile = false;

	int			iFramecount;

	if (lpDAI->settings->GetInt("output/mkv/force v1.0")) {
		lpDAI->settings->SetInt("output/mkv/lacing/video/on", 0);
	}

	int			bLaceVideo = (int)lpDAI->settings->GetInt("output/mkv/lacing/video/on");
	if ((int)s->GetInt("output/mkv/lacing/style") != LACESTYLE_AUTO) bLaceVideo = false;
	int			iVideoFramesPerLace = (int)lpDAI->settings->GetInt("output/mkv/lacing/video/frames");
	bool		bDoLace[100];
	char*		cCodecID = NULL;
	int			iMKVOverhead = 0;

	lpDAI->dlg->m_Prg_Legidx_Label.ShowWindow(0);
	lpDAI->dlg->m_Prg_Legidx_Progress.ShowWindow(0);

	MULTIMEDIASOURCE** mms;
	int			i_mms_count;
	__int64		iMaxDuration;

	MGO_DESCR	mgo;
	bMuxing = true;

	HANDLE hSem;

	i_mms_count = lpDAI->dwNbrOfVideoStreams + lpDAI->dwNbrOfAudioStreams + lpDAI->dwNbrOfSubs;
	mms = new MULTIMEDIASOURCE*[i_mms_count];

	int ifloatlength = (int)lpDAI->settings->GetInt("output/mkv/floats/width");
	sprintf(cBuffer, "size of floats: %d bits", ifloatlength);
	lpDAI->dlg->AddProtocolLine(cBuffer, 4);
	SetEBMLFloatMode(ifloatlength);

	i=0;
	if (lpDAI->dwNbrOfVideoStreams) mms[i++] = lpDAI->videosource;
	for (j=0;j++<(int)lpDAI->dwNbrOfAudioStreams;mms[i++]=lpDAI->asi[j-1]->audiosource);
	for (j=0;j++<(int)lpDAI->dwNbrOfSubs;mms[i++]=lpDAI->ssi[j-1]->lpsubs);

	if (lpDAI->dwNbrOfVideoStreams) {
		iMaxDuration = v->GetDurationUnscaled();
	} else {
		iMaxDuration = 0;
		for (j=0;j<i_mms_count;j++) {
			iMaxDuration = max(iMaxDuration, mms[j]->GetDurationUnscaled());
		}
	}
	cacheAllowReadAhead(1);

	lpDAI->dlg->AddProtocolLine(Message_index(MSG_UNBUFFERED, bUnbufferedOutput), 4);
	lpDAI->dlg->AddProtocolLine(Message_index(MSG_OVERLAPPEDOUT, bOverlapped), 4);


	if (v) {
		hSem = CreateSemaphore(NULL, 1, 1, sem_name);
		mgo.fFPS = (float)1000000000 / v->GetNanoSecPerFrame();
		mgo.iClusterSize = (int)s->GetInt("output/mkv/clusters/size");//lpDAI->mkv.iClusterSize;
		mgo.iClusterTime = (int)s->GetInt("output/mkv/clusters/time");//lpDAI->mkv.iClusterTime;
		mgo.iDuration = (int)(v->GetDuration() * v->GetTimecodeScale() / 1000000000);
		mgo.iFinalSize = lpDAI->videosource->GetSize()/1024;
		for (i=0;i<(int)lpDAI->dwNbrOfAudioStreams;i++) {
			mgo.iFinalSize += lpDAI->asi[i]->audiosource->GetSize()/1024;
		}
		mgo.iFlags = MGOF_CLUSTERS_BYSIZE | MGOF_CLUSTERS_BYTIME;
		if (s->GetInt("output/mkv/clusters/prevclustersize")) mgo.iFlags |= MGOF_CLUSTERS_PREVSIZE;
		if (s->GetInt("output/mkv/clusters/position")) mgo.iFlags |= MGOF_CLUSTERS_POSITION;
		if (!(int)lpDAI->settings->GetInt("output/mkv/clusters/index/on")) {
			mgo.iFlags |= MGOF_CLUSTERS_NOINDEX;
		}

		mgo.iStreamCount = lpDAI->dwNbrOfAudioStreams;
		mgo.pStreams = (MGO_STREAMDESCR*)calloc(lpDAI->dwNbrOfAudioStreams,sizeof(MGO_STREAMDESCR));
		mgo.iKeyframeInterval = 50;
		for (i=0;i<(int)lpDAI->dwNbrOfAudioStreams;i++) {
			mgo.pStreams[i].fDurPerLace = (float)s->GetInt("output/mkv/lacing/length");//lpDAI->mkv.iLaceLength;
			mgo.pStreams[i].iFlags = MGOSF_LACEDUR_IND | MGOSF_FRAMESIZE_IND;
			if ((int)s->GetInt("output/mkv/lacing/style")) {
				mgo.pStreams[i].iFlags |= MGOSF_LACE;
				mgo.pStreams[i].iLaceStyle = (int)s->GetInt("output/mkv/lacing/style");
			}
			CAttribs* s = lpDAI->settings;

			switch (lpDAI->asi[i]->dwType) {
			case AUDIOTYPE_MP3CBR:
				mgo.pStreams[i].iFlags |= MGOSF_CBR;
			case AUDIOTYPE_MP3VBR:
				mgo.pStreams[i].iFramesize = lpDAI->asi[i]->audiosource->GetGranularity();
				if (!mgo.pStreams[i].iFramesize) mgo.pStreams[i].iFramesize = 500;
				mgo.pStreams[i].fFrameDuration = 24;
				mgo.pStreams[i].iFlags |=  MGOSF_FRAMEDUR_IND;
				mgo.pStreams[i].fDurPerLace = (float)GetLaceSetting("mp3",s)/1000000;
				break;
			case AUDIOTYPE_AC3:
				mgo.pStreams[i].iFlags |= MGOSF_CBR;
				mgo.pStreams[i].fFrameDuration = 32;
				mgo.pStreams[i].iFlags |=  MGOSF_FRAMEDUR_IND;
				mgo.pStreams[i].iFramesize = lpDAI->asi[i]->audiosource->GetGranularity();
				if (!mgo.pStreams[i].iFramesize) mgo.pStreams[i].iFramesize = 1000;
				mgo.pStreams[i].fDurPerLace = (float)GetLaceSetting("ac3",s)/1000000;
				break;
			case AUDIOTYPE_DTS:
				mgo.pStreams[i].iFlags |= MGOSF_CBR;
				mgo.pStreams[i].iFramesize = lpDAI->asi[i]->audiosource->GetGranularity();
				mgo.pStreams[i].fFrameDuration = 10;
				mgo.pStreams[i].iFlags |=  MGOSF_FRAMEDUR_IND;
				mgo.pStreams[i].fDurPerLace = (float)GetLaceSetting("dts",s)/1000000;
				break;
			case AUDIOTYPE_AAC:
				mgo.pStreams[i].iFramesize = 500; 
				mgo.pStreams[i].fFrameDuration = 20;
				mgo.pStreams[i].iFlags |=  MGOSF_FRAMEDUR_IND;
				mgo.pStreams[i].fDurPerLace = (float)GetLaceSetting("aac",s)/1000000;
				break;
			case AUDIOTYPE_VORBIS:
				mgo.pStreams[i].iFramesize = 300;
				mgo.pStreams[i].fFrameDuration = 10;
				mgo.pStreams[i].iFlags |=  MGOSF_FRAMEDUR_IND;
				mgo.pStreams[i].fDurPerLace = (float)GetLaceSetting("vorbis",s)/1000000;
				break;
			default:
				mgo.pStreams[i].iFramesize = lpDAI->asi[i]->audiosource->GetGranularity();
				break;
			}
		}
		iMKVOverhead = matroska_guess_overhead(&mgo)+2048+256*(mgo.iStreamCount+1);
		delete mgo.pStreams;
	}

	char cSize[30];
	QW2Str(iMKVOverhead,cSize,15);
	
	sprintf(cBuffer,LoadString(STR_MUXLOG_ESTOVERHEAD),cSize,LoadString(STR_BYTES));

	lpDAI->dlg->AddProtocolLine(cBuffer,4);
	QW2Str(mgo.iFinalSize*1024,cSize,15);
	sprintf(cBuffer,LoadString(STR_MUXLOG_ESTRAW),cSize,LoadString(STR_BYTES));
	lpDAI->dlg->AddProtocolLine(cBuffer,4);

	j=sizeof(__int64)*lpDAI->dwNbrOfAudioStreams;

	qwNanoSecNeeded=new __int64[lpDAI->dwNbrOfAudioStreams];
	qwNanoSecStored=new __int64[lpDAI->dwNbrOfAudioStreams];
	qwNanoSecStored_subs=new __int64[lpDAI->dwNbrOfSubs];
	ZeroMemory(qwStats,sizeof(qwStats));
	ZeroMemory(qwNanoSecNeeded,j);
	ZeroMemory(qwNanoSecStored,j);
	ZeroMemory(qwNanoSecStored_subs,sizeof(__int64)*lpDAI->dwNbrOfSubs);

	ZeroMemory(RawFilename,sizeof(RawFilename));
	memcpy(RawFilename,lpDAI->lpFileName,lstrlen(lpDAI->lpFileName)-4);
	StopMuxing(false);

	if (v) {
		v->SetStretchFactor(lpDAI->dVideoStretchFactor);
		i_vs_nspf = v->GetNanoSecPerFrame();
		v->ReInit();
	}

	qwBias = lpDAI->i1stTimecode*1000000;

// progress info
	CWinThread* thread;
	DISPLAY_PROGRESS_INFO*	lpDPI;

	DWORD		dwFrameCountTotal = 0;
	DWORD		dwTime = GetTickCount();
	DWORD		dwMuxStatePos = 0;

// init mux state
	lpmsState=(MUX_STATE*)malloc(sizeof(MUX_STATE)*MUXSTATEENTRIES);
	for (i=0;i<MUXSTATEENTRIES;i++)
	{
		lpmsState[i].qwWritten=0;
		lpmsState[i].dwTime=GetTickCount();
		lpmsState[i].dwFrames=0; 
	}

	iProgress = 0;

	lpDPI = (DISPLAY_PROGRESS_INFO*)malloc(sizeof(*lpDPI));
	lpDPI->lpdwFrameCountTotal=&dwFrameCountTotal;
	lpDPI->lpiProgress=&iProgress;
	lpDPI->iDuration=iMaxDuration / 1000000;
	lpDAI->dlg->m_Prg_Progress.SetRange32(0,10000);
	
	lpDPI->qwStats=qwStats;
	lpDPI->dwTime=&dwTime;
	lpDPI->lpDAI=lpDAI;
	lpDPI->dwMuxState=lpmsState;
	lpDPI->dwMuxStatePos=&dwMuxStatePos;
	lpDPI->dwLeave=0;
	lpDPI->dwByteAccuracy=1;


	char*	buffer = (char*)malloc(1<<20);
	int		iRead;
	__int64 iLastFrame_Timecode=0;

	for (i=0;i<(int)lpDAI->dwNbrOfAudioStreams;i++) {
		lpDAI->asi[i]->audiosource->ReInit();
		lpDAI->asi[i]->audiosource->SetBias((__int64)lpDAI->asi[i]->iDelay * 1000000, BIAS_UNSCALED);
	}

	for (i=0;i<(int)lpDAI->dwNbrOfSubs;i++) {
		lpDAI->ssi[i]->lpsubs->ReInit();
	}

	lpDAI->dlg->AddProtocolLine(LoadString(STR_MUXLOG_BEGIN),5);
//	MATROSKA_WriteBlockSizes(!lpDAI->mkv.iDontWriteBlockSizes);

	iLace_ns = s->GetInt("output/mkv/lacing/length") * 1000000;

	int iError = 0;
	if (ResolveSplitpoints(lpDAI, RSPF_CHAPTERS, &iError) == RSP_ERROR) {
		sprintf(cBuffer,LoadString(STR_MUXLOG_SPLITPOINTB0RKED),iError+1);
		lpDAI->dlg->MessageBox(cBuffer,LoadString(STR_GEN_ERROR),MB_OK | MB_ICONERROR);
		lpDAI->dlg->SetDialogState_Config();
		lpDAI->dlg->ButtonState_STOP();
		lpDAI->chapters->SetBias(0,BIAS_UNSCALED);
		cacheAllowReadAhead(0);

		return 0;
	}

	thread=AfxBeginThread((AFX_THREADPROC)DisplayProgress_Thread,lpDPI);
	lpDPI->lpThis = thread;

	for (i=0;i<lpDAI->split_points->GetCount();i++) {
		SPLIT_POINT_DESCRIPTOR* p = lpDAI->split_points->At(i);

		char cTime[30];
		Millisec2Str(p->iBegin / 1000000,cTime);
		sprintf(cBuffer,LoadString(STR_MUXLOG_SPLITPOINTAT),i+1,cTime);
		lpDAI->dlg->AddProtocolLine(cBuffer,4);
	}

	audio_lace_config = new LACE_DESCRIPTOR[lpDAI->dwNbrOfAudioStreams];
	for (i=0;i<100;i++) bDoLace[i] = !!(int)s->GetInt("output/mkv/lacing/style");

	// set filename to mka if audio-only
	if (!lpDAI->dwNbrOfVideoStreams) {
		lpDAI->lpFormat[strlen(lpDAI->lpFormat)-1]='a';
	}
	while (( (v && !v->IsEndOfStream()) || (!v && !IsEndOfMMS(mms, i_mms_count)) ) && !bStop) {
		WaitForSingleObject(hSem, INFINITE);
		ZeroMemory(qwNanoSecNeeded,8*lpDAI->dwNbrOfAudioStreams);
		ZeroMemory(qwNanoSecStored,8*lpDAI->dwNbrOfAudioStreams);
		wsprintf(cBuffer,lpDAI->lpFormat,RawFilename,iCurrentFile++);

		DestFile=new FILESTREAM;
		if (DestFile->Open(cBuffer,
			(bUnbufferedOutput?STREAM_UNBUFFERED_WRITE:STREAM_WRITE) | 
			(bOverlapped?STREAM_OVERLAPPED:0))==STREAM_ERR) {
			lpDAI->dlg->MessageBox(LoadString(IDS_COULDNOTOPENOUTPUTFILE),lpDAI->dlg->cstrError,MB_OK | MB_ICONERROR);
			lpDAI->dlg->ButtonState_STOP();
			lpDAI->dlg->SetDialogState_Config();

			ReleaseSemaphore(hSem, 1, NULL);
			HANDLE hSem_finish = CreateSemaphore(NULL,0,1,sem_name_finish);
			lpDPI->dwLeave=1;
	
			WaitForSingleObject(hSem_finish, INFINITE);
			
			CloseHandle(hSem_finish);
			CloseHandle(hSem);
			FinishMuxing(lpDAI);
			cacheAllowReadAhead(0);

			delete lpDPI;
			return 0;
		}
		lpDAI->dlg->SetDlgItemText(IDC_DESTFILE,cBuffer);
		lstrcpy(cFilename,cBuffer);

		m = new MATROSKA;
		CACHEDSTREAM* cache;

		if (bUnbufferedOutput || bOverlapped) {
			cache = new CACHEDSTREAM(4, 1<<21);
			cache->Open(DestFile, CACHE_WRITE);
		} else {
			cache = (CACHEDSTREAM*)DestFile;
		}
		m->Open(cache,MMODE_WRITE);
		if (v) {
			__int64 j = lpDAI->settings->GetInt("output/mkv/TimecodeScale/mkv");
			m->SetTimecodeScale(j);
			v->SetTimecodeScale(j);
		} else {
			m->SetTimecodeScale(lpDAI->settings->GetInt("output/mkv/TimecodeScale/mka"));
		}

		Millisec2HMSF(-qwBias/1000000,&dwHour,&dwMin,&dwSec,&dwFrac);
		wsprintf(cBuffer,LoadString(STR_MUXLOG_NEWFILE),dwHour,dwMin,dwSec,dwFrac,dwFrameCountTotal,cFilename);
		lpDAI->dlg->AddProtocolLine(cBuffer,5);

		QW2Str(m->GetTimecodeScale(), cSize, 1);
		sprintf(cBuffer, LoadString(STR_MUXLOG_OUTPUTTIMECODESCALE),cSize);
		lpDAI->dlg->AddProtocolLine(cBuffer,4);

		m->SetMaxClusterSize((int)s->GetInt("output/mkv/clusters/size"));
		m->SetMaxClusterTime((int)s->GetInt("output/mkv/clusters/time"), (int)s->GetInt("output/mkv/clusters/limit first"));//lpDAI->mkv.iLimit1stCluster);
		m->EnableClusterPosition((int)s->GetInt("output/mkv/clusters/position"));
		m->EnablePrevClusterSize((int)s->GetInt("output/mkv/clusters/prevclustersize"));
		m->SetTrackCount(iTrackCount=lpDAI->dwNbrOfAudioStreams+lpDAI->dwNbrOfVideoStreams+lpDAI->dwNbrOfSubs);
		m->SetSegmentDuration((float)iMaxDuration / m->GetTimecodeScale());
		m->EnableClusterIndex((int)s->GetInt("output/mkv/clusters/index/on"));
		m->EnableRandomizeElementOrder(j=(int)s->GetInt("output/mkv/randomize element order"));
		if (j) {
			lpDAI->dlg->AddProtocolLine("randomizing element order enabled", 4);
		} else {
			lpDAI->dlg->AddProtocolLine("randomizing element order disabled", 4);
		}

		if (lpDAI->settings->GetInt("output/mkv/2nd Tracks")) {
			m->SetTracksCopiesCount(2);
		}
	
		// set cue creation scheme
		int bCues = (int)lpDAI->settings->GetInt("output/mkv/cues/on");
		int bCuesV = (int)lpDAI->settings->GetInt("output/mkv/cues/video/on");
		int bCuesA = (int)lpDAI->settings->GetInt("output/mkv/cues/audio/on");
		int bCuesAOAO = (int)lpDAI->settings->GetInt("output/mkv/cues/audio/only audio-only/on");
		m->EnableCues(CUE_VIDEO | CUE_AUDIO, 0);

		if (bCues) {
			if (bCuesV) {
				m->EnableCues(CUE_VIDEO, 1);
			}
			if (bCuesA) {
				if (bCuesAOAO && !lpDAI->dwNbrOfVideoStreams) {
					m->EnableCues(CUE_AUDIO, 1);
				}
				if (!bCuesAOAO) {
					m->EnableCues(CUE_AUDIO, 1);
				}
			}
		}

		lpDAI->dlg->AddProtocolLine(Message_index(MSG_CUEVIDEO, m->IsCuesEnabled(CUE_VIDEO)), 4);
		lpDAI->dlg->AddProtocolLine(Message_index(MSG_CUEAUDIO, m->IsCuesEnabled(CUE_AUDIO)), 4);
		lpDAI->dlg->AddProtocolLine(Message_index(MSG_CLUSTERINDEX, m->IsClusterIndexEnabled()), 4);

		// set video attributes
		if (lpDAI->dwNbrOfVideoStreams) {
			int x, y;
			RESOLUTION r2;
			v->GetResolution(&x,&y);
			v->GetOutputResolution(&r2);
			m->SetResolution(0,x,y,r2.iWidth,r2.iHeight);
			m->SetFlags(0,1,bLaceVideo,1);
			m->SetTrackNumber(0,1);
			m->SetTrackType(0,MSTRT_VIDEO);
			if (!v->GetIDString()) {
				m->SetCodecID(0,"V_MS/VFW/FOURCC");
				m->SetCodecPrivate(0,v->GetFormat(),((BITMAPINFOHEADER*)v->GetFormat())->biSize);
			} else {
				m->SetCodecID(0,v->GetIDString());
				if (v->GetFormat()) {
					m->SetCodecPrivate(0,v->GetFormat(),v->GetFormatSize());
				}
			}
			m->EnableDisplayWidth_Height((int)s->GetInt("output/mkv/displaywidth_height"));//lpDAI->mkv.iDisplayWidth_Height);
			if (v->IsCFR()) m->SetDefaultDuration(0,i_vs_nspf);
			m->SetCacheData(0,1,0);
			m->SetTrackLanguageCode(0,"und");
		}

		if (lpDAI->cTitle) m->SetSegmentTitle(lpDAI->cTitle->Get());
		m->SetLaceStyle((int)s->GetInt("output/mkv/lacing/style"));
		if (bLaceVideo) {
			char msg[50]; msg[0]=0;
			sprintf(msg, LoadString(STR_MUXLOG_VIDEOLACERATE), iVideoFramesPerLace);
			lpDAI->dlg->AddProtocolLine(msg, 4);
		} else {
			lpDAI->dlg->AddProtocolLine(LoadString(STR_MUXLOG_VIDEOLACINGOFF), 4);
		}
		
		wsprintf(cBuffer,LoadString(STR_MUXLOG_AUDIOSTREAMS),lpDAI->dwNbrOfAudioStreams);
		lpDAI->dlg->AddProtocolLine(cBuffer,5);

		// set all sound attributes
		for (j=0;j<(int)lpDAI->dwNbrOfAudioStreams;j++) 
		{
			i = GetStreamNumber(lpDAI, STREAMTYPE_AUDIO, j);
			WAVEFORMATEX*	w   = (WAVEFORMATEX*)lpDAI->asi[j]->lpFormat;
			AUDIOSOURCE*	as  = lpDAI->asi[j]->audiosource;
			char*           ids = lpDAI->asi[j]->audiosource->GetIDString();
			m->SetTrackNumber(i,i+1);
			m->SetTrackType(i,MSTRT_AUDIO);

			m->SetBitDepth(i,as->GetBitDepth());
			m->SetSamplingFrequency(i,(float)as->GetFrequency(),(float)as->GetOutputFrequency());
			m->SetChannelCount(i,as->GetChannelCount());
			char cName[500];
			char cLangCode[10];
			lpDAI->asi[j]->audiosource->GetName(cName);
			lpDAI->asi[j]->audiosource->GetLanguageCode(cLangCode);
			m->SetTrackName(i,cName);
			
			switch (lpDAI->asi[j]->audiosource->GetFormatTag()) {
				case 0x2000:
					lpDAI->asi[j]->audiosource->SetFrameMode((int)lpDAI->settings->GetInt("output/mkv/ac3/frames per block"));
					break;
				default:
					lpDAI->asi[j]->audiosource->SetFrameMode(FRAMEMODE_SINGLEFRAMES);
					break;
			}

			lpDAI->asi[j]->audiosource->GetLanguageCode(cBuffer);
			m->SetTrackLanguageCode(i,cBuffer);
			CAttribs* lacing = lpDAI->settings->GetAttr("output/mkv/lacing");
			CAttribs* s = lpDAI->settings;
			if (!ids || w && w->wFormatTag && strcmp(ids, "A_VORBIS")
				&& strcmp(ids, "A_FLAC") && strncmp(ids, "A_REAL", 6)) {
				switch (w->wFormatTag) {
					case 0x0001: m->SetCodecID(i,"A_PCM/INT/LIT");
						break;
					case 0x0055: 
						m->SetDefaultDuration(i,lpDAI->asi[j]->audiosource->GetFrameDuration());
						switch (lpDAI->asi[j]->audiosource->FormatSpecific(MMSGFS_MPEG_LAYERVERSION)) {
							case 1: m->SetCodecID(i,"A_MPEG/L1"); break;
							case 2: m->SetCodecID(i,"A_MPEG/L2"); break;
							case 3: m->SetCodecID(i,"A_MPEG/L3"); break;
						} 
						audio_lace_config[j].iLength = GetLaceSetting("mp3",s);						
						break;
					case 0x00FF: 
						cCodecID = new char[128];
						ZeroMemory(cCodecID, 128);
						strcpy(cCodecID,"A_AAC/");
						a = lpDAI->asi[j]->audiosource;
						switch (a->FormatSpecific(MMSGFS_AAC_MPEGVERSION)) {
							case 2: strcat(cCodecID, "MPEG2/"); break;
							case 4: strcat(cCodecID, "MPEG4/"); break;
						}
						switch (a->FormatSpecific(MMSGFS_AAC_PROFILE)) {
							case AAC_ADTS_PROFILE_LC: strcat(cCodecID, "LC"); break;
							case AAC_ADTS_PROFILE_LTP: strcat(cCodecID, "LTP"); break;
							case AAC_ADTS_PROFILE_MAIN: strcat(cCodecID, "MAIN"); break;
							case AAC_ADTS_PROFILE_SSR: strcat(cCodecID, "SSR"); break;
						}
						if (a->FormatSpecific(MMSGFS_AAC_ISSBR)) {
							strcat(cCodecID, "/SBR");
						}
						m->SetCodecID(i,cCodecID);
						m->SetDefaultDuration(i,lpDAI->asi[j]->audiosource->GetFrameDuration());
						audio_lace_config[j].iLength = GetLaceSetting("aac",s);						

						break;
					case 0x2000: 
						m->SetCodecID(i,"A_AC3");
						m->SetDefaultDuration(i,lpDAI->settings->GetInt("output/mkv/ac3/frames per block")*lpDAI->asi[j]->audiosource->GetFrameDuration());
						audio_lace_config[j].iLength = GetLaceSetting("ac3",s);						
						break;
					case 0x2001: m->SetCodecID(i,"A_DTS"); 
						m->SetDefaultDuration(i,lpDAI->asi[j]->audiosource->GetFrameDuration());
						audio_lace_config[j].iLength = GetLaceSetting("dts",s);						
						break;
					default:
						m->SetCodecID(i,"A_MS/ACM");
						m->SetCodecPrivate(i,w,sizeof(*w)+w->cbSize);
						audio_lace_config[j].iLength = GetLaceSetting("general",s);
				}
			} else {
				m->SetCodecID(i,as->GetIDString());
				__int64 iDur = lpDAI->asi[j]->audiosource->GetFrameDuration();
				if (iDur > 0) m->SetDefaultDuration(i,iDur);
				if (lpDAI->asi[j]->lpFormat) {
					m->SetCodecPrivate(i,w,lpDAI->asi[j]->iFormatSize);
				}

				if (!strcmp(as->GetIDString(),"A_AC3")) {
					audio_lace_config[j].iLength = GetLaceSetting("ac3",s);
				} else 
				if (!strcmp(as->GetIDString(),"A_DTS")) {
					audio_lace_config[j].iLength = GetLaceSetting("dts",s);
				} else
				if (!strncmp(as->GetIDString(),"A_MPEG/L",8)) {
					audio_lace_config[j].iLength = GetLaceSetting("mp3",s);
				} else
				if (!strncmp(as->GetIDString(),"A_AAC/MPEG",10)) {
					audio_lace_config[j].iLength = GetLaceSetting("aac",s);
				} else
				if (!strncmp(as->GetIDString(),"A_VORBIS",8)) {
					audio_lace_config[j].iLength = GetLaceSetting("vorbis",s);
				} else
					audio_lace_config[j].iLength = GetLaceSetting("general",s);
			}
			if (audio_lace_config[j].iLength == 0) {
				bDoLace[j] = 0;
			}
			m->SetFlags(i,1,bDoLace[j],as->IsDefault());
			lpDAI->asi[j]->audiosource->GetName(cName);

			if ((int)s->GetInt("output/mkv/lacing/style") || cName[0] || cLangCode[0] || lpDAI->asi[j]->iDelay) {
				wsprintf(cBuffer,LoadString(STR_MUXLOG_STREAM),i);
				lpDAI->dlg->AddProtocolLine(cBuffer,5);
			}
			
			if ((int)s->GetInt("output/mkv/lacing/style")) {
				if (bDoLace[j]) {
					sprintf(cBuffer,LoadString(STR_MUXLOG_TRACKLACESIZE), (int)(audio_lace_config[j].iLength/1000000));
					lpDAI->dlg->AddProtocolLine(cBuffer,4);
				} else {
					sprintf(cBuffer,LoadString(STR_MUXLOG_TRACKLACINGOFF));
					lpDAI->dlg->AddProtocolLine(cBuffer,4);
				}
			}

			if (strlen(cName)) {
				wsprintf(cBuffer,LoadString(STR_MUXLOG_NAME),cName);
				lpDAI->dlg->AddProtocolLine(cBuffer,4, APL_UTF8);
			}
			if (strlen(cLangCode)) {
				wsprintf(cBuffer,LoadString(STR_MUXLOG_LANGUAGECODE),cLangCode);
				lpDAI->dlg->AddProtocolLine(cBuffer,4, APL_UTF8);
			}
			if (lpDAI->asi[j]->iDelay) {
				wsprintf(cBuffer,LoadString(STR_MUXLOG_DELAY),lpDAI->asi[j]->iDelay);
				lpDAI->dlg->AddProtocolLine(cBuffer,4);
			}
			
		} 
		if (!(int)s->GetInt("output/mkv/lacing/style")) {
			lpDAI->dlg->AddProtocolLine(LoadString(STR_MUXLOG_TRACKLACEALLOFF),4);
		}
		// subtitle streams
		
		wsprintf(cBuffer,LoadString(STR_MUXLOG_SUBTITLES),lpDAI->dwNbrOfSubs);
		lpDAI->dlg->AddProtocolLine(cBuffer,5);
	
		for (i=0;i<(int)lpDAI->dwNbrOfSubs;i++) 
		{
			SUBTITLESOURCE*	subs = lpDAI->ssi[i]->lpsubs;
			int	j = GetStreamNumber(lpDAI, STREAMTYPE_SUBTITLE, i);

			m->SetFlags(j,1,0,subs->IsDefault());
			m->SetTrackNumber(j,j+1);
			m->SetTrackType(j,MSTRT_SUBT);

			char cName[500];
			char cLangCode[10];
			subs->GetName(cName);
			subs->GetLanguageCode(cLangCode);
			if (cName[0] || cLangCode[0]) {
				wsprintf(cBuffer,LoadString(STR_MUXLOG_STREAM),i+1);
				lpDAI->dlg->AddProtocolLine(cBuffer,5);

				if (cName[0]) {
					sprintf(cBuffer,LoadString(STR_MUXLOG_NAME),cName);
					lpDAI->dlg->AddProtocolLine(cBuffer,5, APL_UTF8);
				}
				if (cLangCode[0]) {
					sprintf(cBuffer,LoadString(STR_MUXLOG_LANGUAGECODE),cLangCode);
					lpDAI->dlg->AddProtocolLine(cBuffer,5, APL_UTF8);
				}

			}
			m->SetTrackName(j,cName);

			subs->GetLanguageCode(cBuffer);
			m->SetTrackLanguageCode(j,cBuffer);
			cBuffer[0]=0;
			switch (subs->GetFormat()) {
				case SUBFORMAT_SRT:
					m->SetCodecID(j,"S_TEXT/UTF8");
					sprintf(cBuffer, LoadString(STR_MUXLOG_SUBTITLE_FORMAT), "SRT");
					break;
				case SUBFORMAT_SSA:
					m->SetCodecID(j,"S_TEXT/SSA");
					sprintf(cBuffer, LoadString(STR_MUXLOG_SUBTITLE_FORMAT), "SSA");
					break;
				case SUBFORMAT_VOBSUB:
					m->SetCodecID(j,"S_VOBSUB");
					sprintf(cBuffer, LoadString(STR_MUXLOG_SUBTITLE_FORMAT), "VOBSUB");
					break;
			}

			lpDAI->dlg->AddProtocolLine(cBuffer, 4, APL_UTF8);
			m->SetCodecPrivate(j,buffer,subs->RenderCodecPrivate(buffer));

			cBuffer[0]=0;
			if (subs->GetCompressionAlgo() == COMP_ZLIB) {
				m->SetTrackCompression(j, COMP_ZLIB);
				sprintf(cBuffer, LoadString(STR_MUXLOG_COMPRESSION_TYPE), "zlib");
			} else if (subs->GetCompressionAlgo() == COMP_NONE) {
				sprintf(cBuffer, LoadString(STR_MUXLOG_COMPRESSION_TYPE), "none");
			} else sprintf(cBuffer, LoadString(STR_MUXLOG_COMPRESSION_TYPE), "unknown");

			lpDAI->dlg->AddProtocolLine(cBuffer, 4);
		}
		m->BeginWrite();
		{
			char	buffer[200];
			ComposeVersionString(buffer);
			m->SetAppName(buffer);
		}

		iFramecount = 0;
		ReleaseSemaphore(hSem, 1, NULL);
		iLastTimecode = 0;
		while (((v && !v->IsEndOfStream()) || (!v && !IsEndOfMMS(mms, i_mms_count))) && (!bSplitFile) && (!bStop)) {
			ADDBLOCK	a;
			ADVANCEDREAD_INFO aari;

			ZeroMemory(&a,sizeof(a));
			ZeroMemory(&aari,sizeof(aari));

		// get video frame
			WaitForSingleObject(hSem, INFINITE);
			if (lpDAI->dwNbrOfVideoStreams) {
				if (!bLaceVideo) {
					v->GetFrame(buffer,(DWORD*)&iRead,&a.iTimecode,&aari);
					v->GetLatestReference(&a.iRefCount,&a.iReferences[0],&a.iReferences[1]);
					a.iReferences[0] = (__int64)a.iReferences[0];
					a.iReferences[1] = (__int64)a.iReferences[1];
					int iDiff = (int)(- iLastTimecode + a.iTimecode + a.iReferences[0]);
					if (a.iRefCount && abs(iDiff)>0 && abs(iDiff)<3) {
						int iNew = (int)(iLastTimecode - a.iTimecode);
						char cTime[20];
						char cText[200];
						Millisec2Str(a.iTimecode,cTime);
						sprintf(cText,"fixed: frame at %s refers to %I64d, but must be %d!",
							cTime,a.iReferences[0],iNew);
						lpDAI->dlg->AddProtocolLine(cText,4);
						a.iReferences[0] = iNew;
					}
					iLastTimecode = a.iTimecode;
					iFramecount++;
					AddVideoSize(qwStats,iRead,ADS_NOTEVEN);
		
					a.cData = new CBuffer(iRead,buffer,CBN_REF1);
					a.iDuration = aari.iDuration * v->GetTimecodeScale();
					a.iStream = 0;
					a.iFlags |= ABSSM_INDEX;
	
					aari.iNextTimecode *= v->GetTimecodeScale();
					qwVideoReadTotal = aari.iNextTimecode;
					aari.iNextTimecode +=qwBias;
					qwVideoRead = aari.iNextTimecode;
					
					a.iTimecode *= v->GetTimecodeScale();
					a.iTimecode+=qwBias;
					a.iFlags |= ABTC_UNSCALED;
					m->Write(&a);
					DecBufferRefCount(&a.cData);
	
					dwFrameCountTotal++;
					qwBytesWritten = qwStats[3];
				} else {
				// lace video.  HACK!!!
					BYTE* bBuffer = (BYTE*)buffer;
					int iTotalRead = 0;
					a.iFrameSizes = new int[iVideoFramesPerLace];
					v->GetFrame(buffer,(DWORD*)&iRead,&a.iTimecode,&aari);
					a.iFrameSizes[0] = iRead;
					a.iFrameCountInLace++;
					v->GetLatestReference(&a.iRefCount,&a.iReferences[0],&a.iReferences[1]);
					iTotalRead += iRead;
					iLastTimecode = a.iTimecode;
					iFramecount++;
					__int64 dummy;
					while (!v->IsKeyFrame(CN_CURRENT_CHUNK) && a.iFrameCountInLace<iVideoFramesPerLace && !v->IsEndOfStream()) {
						v->GetFrame(buffer+iTotalRead,(DWORD*)&iRead,&dummy,&aari);
						a.iFrameSizes[a.iFrameCountInLace++] = iRead;
						iTotalRead += iRead;
						iFramecount++;
					}
					AddVideoSize(qwStats,iTotalRead,ADS_NOTEVEN);
				
					a.cData = new CBuffer(iTotalRead,buffer,CBN_REF1);
					a.iDuration += aari.iDuration * v->GetTimecodeScale();
					a.iStream = 0;
					a.iFlags |= ABSSM_INDEX | ABTC_UNSCALED;

					aari.iNextTimecode *= v->GetTimecodeScale();
					qwVideoReadTotal = aari.iNextTimecode;
					aari.iNextTimecode += qwBias;
					qwVideoRead = aari.iNextTimecode;
					a.iTimecode+=qwBias;
					a.iTimecode *= v->GetTimecodeScale();
					m->Write(&a);
					DecBufferRefCount(&a.cData);
					delete a.iFrameSizes;
	
					dwFrameCountTotal++;
					qwBytesWritten = qwStats[3];
				}
			} else {
				qwVideoRead += 100*m->GetTimecodeScale();
				qwVideoReadTotal += 100*m->GetTimecodeScale();
			}

			iInputTimecode = qwVideoRead - qwBias;

			bool bSizeLimit = qwBytesWritten > lpDAI->qwMaxSize*(1<<20);

			bool bSplitpoint = false;
			iDist2Splitpoint = 0x7FFFFFFF;
			__int64 iDist2End;
			if (lpDAI->dwNbrOfVideoStreams) {
				iDist2End = v->GetDurationUnscaled() - iInputTimecode;
			} else {
				if (iMaxDuration) {
					iDist2End = iMaxDuration - iInputTimecode;
				} else {
					iDist2End = 1000000000;
				}
			}
			iDist2Splitpoint = __min(iDist2Splitpoint, iDist2End);
			int reason = 0;
			if (iDist2Splitpoint == iDist2End) reason = 1;
			SPLIT_POINT_DESCRIPTOR* d = NULL;
			
			d = lpDAI->split_points->At(0);
			if (lpDAI->split_points->GetCount() && d) {
				iDist2Splitpoint = d->iBegin - iInputTimecode;
				bSplitpoint = (iDist2Splitpoint <= 0);
				if (bSplitpoint) {
					Sleep(1);
				}
			}

			bool bFrameLimit = dwFrameCountTotal == lpDAI->dwMaxFrames;
			
			// add all audio data
			for (i=0;i<(int)lpDAI->dwNbrOfAudioStreams;i++) {
				iRead = 1;
				iLace_ns = audio_lace_config[i].iLength;
				if (!iLace_ns) iLace_ns = 50000000;
				AUDIOSOURCE* as = lpDAI->asi[i]->audiosource;
				if (!(lpDAI->iDurationFlags & DAI_DF_DURATION)) {
					if (iMaxDuration) {
						qwMaxAudio = (iMaxDuration-qwBias) - qwNanoSecStored[i];
					} else {
						qwMaxAudio = 1000000000;
					}
				} else {
					if (lpDAI->iMaxDuration) {
						qwMaxAudio = lpDAI->iMaxDuration;
					} else {
						qwMaxAudio = 1000000000;
					}
				}
				if (qwMaxAudio > iLace_ns) qwMaxAudio = iLace_ns;

				while (iRead>0 && (qwNanoSecStored[i]<=qwVideoRead) && (!bStop) && !as->IsEndOfStream() && (qwMaxAudio>0 || !v)) {
					ADVANCEDREAD_INFO aari;
					ZeroMemory(&aari,sizeof(aari));
					__int64 iMSD;
					
					if (v) {
						iMSD = (__int64)((qwVideoRead-qwNanoSecStored[i])/1000 + iLace_ns/1000);
					} else {
						iMSD = (__int64)(iLace_ns / 1000);
					}

					if (iMSD > iDist2Splitpoint/1000) iMSD = (__int64)(iDist2Splitpoint/1000);

					if (!bDoLace[i]) {
						ZeroMemory(&a,sizeof(a));
						ZeroMemory(&aari,sizeof(aari));
						iRead = as->Read(buffer,(int)iMSD,NULL,&qwNanoSecRead,&a.iTimecode,&aari);
						if (iRead >= 0) {
							iProgress = (a.iTimecode) * as->GetTimecodeScale() + qwNanoSecRead;

							if (aari.iNextTimecode != TIMECODE_UNKNOWN) {
								iMSD-=(int)((aari.iNextTimecode-a.iTimecode)*as->GetTimecodeScale()/1000);
							}

							a.cData = new CBuffer(iRead,buffer,CBN_REF1);
							a.iRefCount = 0;
							a.iTimecode = a.iTimecode*as->GetTimecodeScale() + qwBias;
							a.iFrameCountInLace = aari.iFramecount;
							a.iFrameSizes = aari.iFramesizes;
						
							if (aari.iNextTimecode != TIMECODE_UNKNOWN) {
								qwNanoSecStored[i] = aari.iNextTimecode*as->GetTimecodeScale()+qwBias;
							}
							
							a.iStream = GetStreamNumber(lpDAI, STREAMTYPE_AUDIO, i);
							a.iFlags |= ABSSM_INDEX | ABTC_UNSCALED;
							a.iDuration = qwNanoSecRead;
							m->Write(&a);
							DecBufferRefCount(&a.cData);
							if (a.iFrameSizes) delete a.iFrameSizes;
							AddAudioSize(qwStats,iRead,ADS_NOTEVEN);
						} else iRead = 0;
					} else {
						ZeroMemory(&a,sizeof(a));
						ZeroMemory(&aari,sizeof(aari));
						bool bFirstRead = true;
						int iTotalRead = 0;
						__int64 cTC = 0;
			
						iSourceLaces=0;
						if (iMSD<0 && reason == 1 && !as->IsEndOfStream()) {
							iMSD = 1;
						}
						while (iMSD > 0 && !as->IsEndOfStream() && iRead > 0 && !aari.iDuration && a.iFrameCountInLace<100 && !aari.iFileEnds) {
							
							iRead = as->Read(buffer+iTotalRead,(int)iMSD,NULL,&qwNanoSecRead,&cTC,&aari);
							if (iRead>0) {
								iProgress = (cTC * as->GetTimecodeScale()) + qwNanoSecRead;
							}

							if (aari.iNextTimecode != TIMECODE_UNKNOWN) {
								iMSD-=(int)((aari.iNextTimecode-cTC)*as->GetTimecodeScale()/1000);
							}

							if (iRead>0) iTotalRead += iRead;
					
							if (bFirstRead) a.iTimecode = cTC;
						// blocks > 2040 bytes -> lacing doesn't make sense; only keep existing lacing
							if (iRead > 0 && ((iRead/max(1,aari.iFramecount)<2040 || (int)s->GetInt("output/mkv/lacing/style") != LACESTYLE_XIPH) || aari.iFramecount)) {
								if (aari.iNextTimecode != TIMECODE_UNKNOWN) {
									qwMaxAudio += qwNanoSecStored[i] - (aari.iNextTimecode*as->GetTimecodeScale() + qwBias);
								}

								bFirstRead = false;
						// source block not laced
								if (aari.iFramecount<2) {
									a.iFrameCountInLace++;
									a.iFrameSizes = (int*)realloc(a.iFrameSizes,sizeof(int)*a.iFrameCountInLace);
									a.iFrameSizes[a.iFrameCountInLace-1] = iRead;
								}
								else for (j=0;j<aari.iFramecount;j++) {
									a.iFrameCountInLace++;
									a.iFrameSizes = (int*)realloc(a.iFrameSizes,sizeof(int)*a.iFrameCountInLace);
									a.iFrameSizes[a.iFrameCountInLace-1] = aari.iFramesizes[j];
									iSourceLaces++;
								}
								a.iDuration += qwNanoSecRead;

								if (aari.iNextTimecode != TIMECODE_UNKNOWN) {
									qwNanoSecStored[i] = (aari.iNextTimecode*as->GetTimecodeScale() + qwBias);
								}

								delete aari.iFramesizes;

							} else {
								if (iRead>0) {
									bDoLace[i] = false;
									char cBuffer[100];
									sprintf(cBuffer,"switched off lacing for audio stream stream %d",i+1);
									lpDAI->dlg->AddProtocolLine(cBuffer,4);
									qwMaxAudio = -1;
									iMSD = -1;

									if (aari.iNextTimecode != TIMECODE_UNKNOWN) {
										qwNanoSecStored[i] = (aari.iNextTimecode*as->GetTimecodeScale() + qwBias);
									};
								}
							}

						}
						if (iTotalRead > 0) {
							a.cData = new CBuffer(iTotalRead,buffer,CBN_REF1);
							a.iTimecode = a.iTimecode * as->GetTimecodeScale() + qwBias;
						
							a.iStream = GetStreamNumber(lpDAI, STREAMTYPE_AUDIO, i);
							a.iFlags = ABSSM_INDEX | ABTC_UNSCALED;
							m->Write(&a);
							DecBufferRefCount(&a.cData);
							if (a.iFrameSizes) delete a.iFrameSizes;
							AddAudioSize(qwStats,iTotalRead,ADS_NOTEVEN);
						} else iRead = 0;
					}
				}
			}

			if (v) {
				iProgress = qwVideoReadTotal;
			} 

			// subtitles
			for (i=0;i<(int)lpDAI->dwNbrOfSubs;i++) {
				SUBTITLESOURCE*	subs = lpDAI->ssi[i]->lpsubs;
				__int64 iNextTC = subs->GetNextTimecode();
				if (iNextTC != TIMECODE_UNKNOWN) {
					iNextTC = iNextTC * subs->GetTimecodeScale() + qwBias;
				}

				if (iNextTC != TIMECODE_UNKNOWN && qwVideoRead > iNextTC) {
					int j = 1+lpDAI->dwNbrOfAudioStreams+i;
					ADVANCEDREAD_INFO aari;
					ADDBLOCK a;
					
					ZeroMemory(&aari,sizeof(aari));
					ZeroMemory(&a,sizeof(a));

					if (subs->Read(buffer,&iRead,&a.iTimecode,&aari)==SUBS_OK && iRead) {
						a.iStream = j;
						a.iFlags |= ABSSM_INDEX | ABTC_UNSCALED;
						a.iDuration = aari.iDuration * subs->GetTimecodeScale();
						a.cData = new CBuffer(iRead,buffer,CBN_REF1);
						a.iTimecode *= subs->GetTimecodeScale();
						a.iTimecode += qwBias;
						qwNanoSecStored_subs[i] = a.iTimecode;
						m->Write(&a);

						AddSubtitleSize(qwStats,iRead,ADS_NOTEVEN);
					}
				}
			}

			AddOverhead(qwStats,(int)m->GetOverhead());

			if (lpDAI->dwNbrOfVideoStreams) {
				if (bSizeLimit || bFrameLimit || v->IsEndOfStream() || bSplitpoint) {
					if (v->IsKeyFrame(CN_CURRENT_CHUNK)) {
						bSplitFile = true;
					}
				}
			} else {
				if (bSizeLimit || bSplitpoint || IsEndOfMMS(mms, i_mms_count)) {
					bSplitFile = true;
				}
			}

			if (bSplitFile && (iCurrentFile >= (int)lpDAI->dwMaxFiles+1) && (v && !v->IsEndOfStream()) && !bSplitpoint) {
				bSplitFile = false;
			}

			ReleaseSemaphore(hSem, 1, NULL);
		}

		if (bSplitFile || bStop) {
			WaitForSingleObject(hSem, INFINITE);			
			lpDAI->chapters->SetBias(qwBias,BIAS_UNSCALED);
			//qwBias += -qwVideoRead;
			if (v) {
				m->SetDefaultDuration(0,qwVideoRead/iFramecount);
				m->SetChapters(lpDAI->chapters, qwVideoRead);
			} else {
				m->SetChapters(lpDAI->chapters);
			}

			{
				CLUSTER_STATS clstats;
				m->GetClusterStats(&clstats);
				char cSize1[20];
				char cSize2[20];
				QW2Str(clstats.iCount, cSize1, 1);
				QW2Str(clstats.iOverhead, cSize2, 1);
				sprintf(cBuffer, LoadString(STR_MUXLOG_CLUSTERS),cSize1,cSize2);
				lpDAI->dlg->AddProtocolLine(cBuffer,4);

				QW2Str(m->GetCueCount(), cSize1, 1);
				sprintf(cBuffer,LoadString(STR_MUXLOG_CUES),cSize1);
				lpDAI->dlg->AddProtocolLine(cBuffer,4);
			}

			int iLaceSchemes [] = { 0, LACESTYLE_XIPH, LACESTYLE_EBML, LACESTYLE_FIXED };
			char* cLaceSchemes [] = { "no", "Xiph", "EBML", "fixed" };
			for (j=0;j<iTrackCount;j++) {
				LACE_STATS stats[4];
				for (i=1;i<4;i++) {
					m->GetLaceStatistics(j,iLaceSchemes[i],&stats[i]);
				}
				if (stats[1].iCount + stats[2].iCount + stats[3].iCount) {
					sprintf(cBuffer,"track %d",j);
					lpDAI->dlg->AddProtocolLine(cBuffer,4);
				}
				for (i=1;i<4;i++) {
					int iCount = stats[i].iCount;

					if (iCount>0) {
						char cSize1[20];
						char cSize2[20];
						char cSize3[20];
						QW2Str(stats[i].iCount,cSize1,1);
						QW2Str(stats[i].iTotalHdrSize,cSize2,1);
						QW2Str(stats[i].iFrameCount,cSize3,1);
						sprintf(cBuffer,LoadString(STR_MUXLOG_LACESTATS),cLaceSchemes[i],cSize1,cSize3,cSize2);
						lpDAI->dlg->AddProtocolLine(cBuffer,4);
					}
				}
			}
			m->Close();
			qwBias+= -qwVideoRead; //-m->GetSegmentDuration() * m->GetTimecodeScale();
			cache->Close();

			{
				char cSize1[20];
				QW2Str(m->GetSeekheadSize(), cSize1, 1);
				sprintf(cBuffer,LoadString(STR_MUXLOG_SEEKHEAD_SIZE),cSize1,
					LoadString(STR_BYTES));
				lpDAI->dlg->AddProtocolLine(cBuffer,4);
			}

			AddOverhead(qwStats,(int)m->GetOverhead());

			delete m;
//			DestFile->Close();
//			delete DestFile;
			if ((v && !v->IsEndOfStream()) || (!v && !IsEndOfMMS(mms, i_mms_count))) {
				qwStats[0] = 0;
				qwStats[1] = 0;
				qwStats[2] = 0;
				qwStats[3] = 0;
				qwStats[8] = 0;
			}

			bSplitFile = false;
			if (lpDAI->split_points->GetCount() && lpDAI->split_points->At(0)->iBegin - iInputTimecode <= 0) {
				lpDAI->split_points->Delete(0);
			}

			ReleaseSemaphore(hSem, 1, NULL);
		}
	}

	QW2Str(qwStats[6],cSize,15);
	sprintf(cBuffer,LoadString(STR_MUXLOG_OVERHEADWRITTEN),cSize,LoadString(STR_BYTES));
	lpDAI->dlg->AddProtocolLine(cBuffer,5);

	lpDAI->dlg->AddProtocolLine(LoadString(STR_MUXLOG_DONE),5);
	lpDAI->dlg->AddProtocol_Separator();
//	lpDAI->dlg->AddProtocolLine("================================================================",5);

	bool bExit = lpDAI->bExitAfterwards;
	CDialog* c = lpDAI->dlg;
	HANDLE hSem_finish = CreateSemaphore(NULL,0,1,sem_name_finish);
	lpDPI->dwLeave=1;
	
	WaitForSingleObject(hSem_finish, INFINITE);

	CloseHandle(hSem_finish);
	delete lpDAI->lpFormat;
	lpDAI->split_points->DeleteAll();
	delete lpDAI->split_points;
	delete lpDPI;

	if (lpDAI->bDoneDlg) lpDAI->dlg->MessageBox(LoadString(IDS_READY),lpDAI->dlg->cstrInformation,MB_OK | MB_ICONINFORMATION);

	lpDAI->dlg->SetDialogState_Config();
	lpDAI->dlg->ButtonState_STOP();
	lpDAI->chapters->SetBias(0,BIAS_UNSCALED);
	bStop=false;

	delete qwNanoSecNeeded;
	delete qwNanoSecStored;

	for (i=0;i<(int)lpDAI->dwNbrOfAudioStreams;lpDAI->asi[i++]->audiosource->Enable(0));

	bMuxing = false;
	delete mms;
	FinishMuxing(lpDAI);
	delete lpDAI;
	cacheAllowReadAhead(0);
	if (bExit) c->PostMessage(WM_CLOSE,0,0);
	return 1;
}
