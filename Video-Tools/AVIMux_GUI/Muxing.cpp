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
#include "version.h"
#include "..\UnicodeCalls.h"
#include "..\integers.h"
#include "..\UID.h"
#include "..\UnicodeCalls.h"
#include "UTF8Windows.h"
#include "OSVersion.h"
#include "..\FileStream.h"
#include "fopenutf8.h"

#define INT64_MAX 0x7FFFFFFFFFFFFFFF

DWORD					bStop=false;
bool					bMuxing=false;
CAVIMux_GUIDlg*			dlg;

int MSG_UNBUFFERED		[] = { STR_MUXLOG_UNBUFFERED_OFF   , STR_MUXLOG_UNBUFFERED_ON };
int MSG_OVERLAPPEDOUT	[] = { STR_MUXLOG_OVERLAPPED_OFF   , STR_MUXLOG_OVERLAPPED_ON };
int MSG_CUEVIDEO        [] = { STR_MUXLOG_CUES_VIDEO_OFF   , STR_MUXLOG_CUES_VIDEO_ON };
int MSG_CUEAUDIO        [] = { STR_MUXLOG_CUES_AUDIO_OFF   , STR_MUXLOG_CUES_AUDIO_ON };
int MSG_CUESUBS         [] = { STR_MUXLOG_CUES_SUBS_OFF    , STR_MUXLOG_CUES_SUBS_ON };
int MSG_CLUSTERINDEX    [] = { STR_MUXLOG_CLUSTERINDEX_OFF , STR_MUXLOG_CLUSTERINDEX_ON };
int MSG_RECLISTS        [] = { STR_MUXLOG_RECLISTS_OFF     , STR_MUXLOG_RECLISTS_ON };
int MSG_LOWOVHDAVI      [] = { STR_MUXLOG_LOWOVHDAVI_OFF   , STR_MUXLOG_LOWOVHDAVI_ON };
int MSG_JUNKB4HDR       [] = { STR_MUXLOG_ADDINGJUNK_OFF   , STR_MUXLOG_ADDINGJUNK_ON };
int MSG_RANDELORDER		[] = { STR_MUXLOG_RANDOMIZE_OFF	   , STR_MUXLOG_RANDOMIZE_ON };
int MSG_WRITECUEBLKNBR	[] = { STR_MUXLOG_CUES_WRBLOCKNBR_OFF, STR_MUXLOG_CUES_WRBLOCKNBR_ON };
int MSG_BENDSPECS       [] = { STR_MUXLOG_FREESTYLE_OFF,     STR_MUXLOG_FREESTYLE_ON };
int MSG_CACHE           [] = { STR_MUXLOG_CACHE_OFF,         STR_MUXLOG_CACHE_ON };



/* some helper functions for multimedia source arrays */
typedef std::vector<MULTIMEDIASOURCE*> MULTIMEDIASOURCEVECTOR;

bool GenerateMMSVector(DEST_AVI_INFO* lpDAI, MULTIMEDIASOURCEVECTOR& mmsv)
{
	if (lpDAI->videosource)
		mmsv.push_back(lpDAI->videosource);

	for (size_t j=0; j<lpDAI->dwNbrOfAudioStreams; j++)
		mmsv.push_back(lpDAI->asi[j]->audiosource);
	
	for (size_t j=0; j<lpDAI->dwNbrOfSubs; j++)
		mmsv.push_back(lpDAI->ssi[j]->lpsubs);

	return true;
}

bool IsEndOfMMS(MULTIMEDIASOURCEVECTOR mmsv)
{
	bool bRes = true;
	int count = mmsv.size();
	for (int j=0;j<count;bRes&=mmsv[j++]->IsEndOfStream());
	return bRes;
}

__int64 GetMMSVSize(MULTIMEDIASOURCEVECTOR mmsv)
{
	MULTIMEDIASOURCEVECTOR::iterator iter = mmsv.begin();
	__int64 size = 0;

	for (; iter != mmsv.end(); iter++)
		size += (*iter)->GetSize();

	return size;
}








const int FFC_FORMAT_AVI = 0x01;
const int FFC_FORMAT_MKV = 0x02;

char* Message_index(int* pList, int iIndex)
{
	return LoadString(pList[iIndex]);
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

int FirstFilenameCheck(DEST_AVI_INFO* lpDAI, char* RawFilename, int format, __int64* overwriteable_size);

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
	char cWindowText[1024]; cWindowText[0]=0;
	lpDAI->dlg->GetWindowText(cWindowText, sizeof(cWindowText));

	qwStats=(__int64*)malloc(dwSize=sizeof(__int64)*MUXSTATEENTRIES);

	dwTimeBegin = GetTickCount();
	dwDataRateMin = 1000000000;
	dwDataRateMax = 0;

	hSem = OpenSemaphore(SEMAPHORE_ALL_ACCESS,false,sem_name);

	lpDAI->dlg->UpdateProgressList();

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
			dwDataRate=(DWORD)(1000*((dwMuxState[*dwMuxStatePos].qwWritten)-(dwMuxState[dwNextPos].qwWritten))/dwDataTime/1024);
		else
			dwDataRate=0;
		
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

		if (iTotalDuration) 
			lpDAI->dlg->m_Prg_Progress.SetPos((int)(iProgress/100/iTotalDuration));

		strcpy(cBuffer, cWindowText);
		strcat(cBuffer, " - ");
		if (!iTotalDuration) 
			strcat(cBuffer, cCurrTime);
		else {
			char cPerc[32]; cPerc[0]=0;
			sprintf(cPerc, "%4.1f%%", (float)iProgress / (float)iTotalDuration / 10000.);
			strcat(cBuffer, cPerc);
		}

		lpDAI->dlg->SetWindowText(cBuffer);		

		if (lpDPI->dwLeave) 
			dwQuit=1;

		Sleep(250);
	
	} while (!dwQuit);
	
	HANDLE hSem_finish = OpenSemaphore(SEMAPHORE_ALL_ACCESS,false,sem_name_finish);
	lpDPI->dwLeave=0;
	delete qwStats;
	lpDAI->dlg->SetWindowText(cWindowText);
	CloseHandle(hSem);
	ReleaseSemaphore(hSem_finish,1,NULL);
	CloseHandle(hSem_finish);

	return 1;
}

const int RSPF_CHAPTERS = 0x01;
const int RSPF_FRAMES   = 0x02;
const int RSP_ERROR		= -0x01;

int RemoveZeroSplitpoints(CSplitPoints* s)
{
	SPLIT_POINT_DESCRIPTOR* d;
	int i;
	// remove timestamps of NULL 
	for (i=0;i<s->GetCount();i++) {
		d = s->At(i);
		if (!d->iBegin) s->Delete(i--);
	}

	return 1;
}

const int CHECKEXISTENCE_NEVERASK     = 0x01;
const int CHECKEXISTENCE_STOP         = 0x02;
const int CHECKEXISTENCE_ASKEACH      = 0x03;


const int CHECKEXISTENCE_DOESNOTEXIST = 0x00;
const int CHECKEXISTENCE_OVERWRITE    = 0x05;

int	CheckExistence(char* cFilename, int ask, __int64* filesize)
{
	CFileStream* f = new CFileStream;
	if (f->Open(cFilename, StreamMode::Read) != STREAM_OK) {
		delete f;
		return CHECKEXISTENCE_DOESNOTEXIST;
	}

	if (filesize)
		*filesize = f->GetSize();

	f->Close();
	delete f;

	if (CHECKEXISTENCE_NEVERASK == ask || CHECKEXISTENCE_OVERWRITE == ask)
		return CHECKEXISTENCE_OVERWRITE;

	char* cMsg1;
	char* cMsg2;
	char* cMsgT = (char*)calloc(1, 2048+strlen(cFilename));

	cMsg1 = LoadString(IDS_EXISTINGFILES, LOADSTRING_UTF8);
	cMsg2 = LoadString(STR_OVERWRITE, LOADSTRING_UTF8);

	sprintf(cMsgT, "%s%c%c%s%c%c%c%c%s", cMsg1,
		13,10,cFilename,13,10,13,10,cMsg2);

	int j = MessageBoxUTF8(0, cMsgT, LoadString(IDS_WARNING), MB_YESNO | MB_ICONWARNING);

	if (IDYES == j) 
		return CHECKEXISTENCE_OVERWRITE;

	if (IDNO == j)
		return CHECKEXISTENCE_STOP;

	return -1;
}

int ResolveSplitpoints (DEST_AVI_INFO* lpDAI, int iFlags, int* piError = NULL)
{
	CSplitPoints* s = lpDAI->split_points;
	CChapters* c = lpDAI->chapters;
	SPLIT_POINT_DESCRIPTOR* d,*d2,d3;
	bool bChapters = !!(iFlags & RSPF_CHAPTERS);
	bool bFrames = !!(iFlags & RSPF_FRAMES);
	int  i,j,k;
	int  chp = 0;

	// resolve wildchar
	for (i=0;i<s->GetCount();i++) {
		d = s->At(i);
		CDynIntArray* x = d->aChapBegin;
		if (d->iFlags & SPD_BCHAP) {
			chp = RSPF_CHAPTERS;
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
					p->chapter = lpDAI->chapters;
					p->iFlags = SPD_BCHAP;
					s->Insert(p);
					
					for (k=0;k<main_chapter->GetChapterCount();k++) {
						p = new SPLIT_POINT_DESCRIPTOR; ZeroMemory(p, sizeof(*p));
						p->aChapBegin = complete_chapter_index->Duplicate();
						p->aChapBegin->Set(j, k);
						//p->cOrgText = new CStringBuffer(main_chapter->GetChapterText(k, 
						//	"eng", CHAP_GCT_RETURN_FIRST));
						p->chapter = lpDAI->chapters;
						p->iIndex  = k;
						p->iFlags = SPD_BCHAP;
						s->Insert(p);
					} 
				} else {
					p = new SPLIT_POINT_DESCRIPTOR; ZeroMemory(p, sizeof(*p));
					p->aChapBegin = main_chapter_index->Duplicate();
					p->chapter = lpDAI->chapters;
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
			d->chapter = lpDAI->chapters;
		}
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

	return chp;
}

__int64 DistToNextSplitPoint(DEST_AVI_INFO* lpDAI, DWORD dwCurrFrame, __int64 iTimecode_ns)
{
	//if (!lpDAI->lpqwSplitPoints) 
	CSplitPoints* s = lpDAI->split_points;
	SPLIT_POINT_DESCRIPTOR* d;
	__int64 iLeft=INT64_MAX;

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

bool DeleteFileUTF8(char* file_name)
{
	if (utf8_IsUnicodeEnabled()) {
		char* t = NULL;
		UTF82WStr(file_name, &t);
		bool res = !!DeleteFileW((LPCWSTR)t);
		free(t);
		return res;
	} else {
		char* t = NULL;
		UTF82Str(file_name, &t);
		bool res = !!DeleteFileA((LPCSTR)t);
		free(t);
		return res;
	}
}

#include "../strings.h"

/* free space functions */
/** \brief Determines the amount of free disk space 
 *
 * \param fileName Name of a file on the target disk in UTF-8 encoding
 */
__int64 GetDirectoryFreeSpace(char* filename)
{
	char* f = NULL;
	char* e = NULL;
	char* path = (char*)calloc(1, 32768);

	CUTF8 utf8FullFileName(filename, CharacterEncoding::UTF8);
	std::wstring fileName;
	std::wstring fileNameExtension;
	std::wstring pathName;

	splitpathname<wchar_t>(utf8FullFileName, fileName, fileNameExtension, pathName);

	ULARGE_INTEGER free_bytes;
	GetDiskFreeSpaceExW(pathName.c_str(), &free_bytes, NULL, NULL);

	return free_bytes.QuadPart;
}

bool FreeSpaceErrorMessage(int msg_id, __int64 free_space, 
						   __int64 overwriteable_space, 
						   __int64 required_space)
{
	char c[65536]; c[0]=0;
	char cfreespace[64];
	char coverwriteable_space[64];
	char crequired_space[64];

	FormatSize(cfreespace, free_space);
	FormatSize(coverwriteable_space, overwriteable_space);
	FormatSize(crequired_space, required_space);

	sprintf(c, LoadString(msg_id), crequired_space, cfreespace, 
		coverwriteable_space);

	int result = MessageBox(NULL, c, LoadString(STR_GEN_ERROR), 
		MB_YESNO | MB_ICONERROR | MB_DEFBUTTON2);

	if (result == IDNO)
		return false;
	
	return true;
}

bool CheckDiskSpace(bool bDoCheck, __int64 total_size, __int64 disk_space,
					__int64 tolerance, __int64 overwriteable_size)
{
	if (!bDoCheck)
		return true;

	if (total_size < disk_space + tolerance) {
		/* enoug disc space */
		return true;
	} else
	if (total_size - overwriteable_size < disk_space + tolerance) {
		/* need to delete some files before starting */
		return true;
	} else {
		bool bContinue = true;
		/* no free space */
		if (overwriteable_size > 0) {
			bContinue = FreeSpaceErrorMessage(STR_MUXLOG_NODISCSPACEATALL, disk_space, overwriteable_size,
				total_size);
		} else {
			bContinue = FreeSpaceErrorMessage(STR_MUXLOG_NODISCSPACE, disk_space, overwriteable_size,
				total_size);
		}

		return bContinue;
	}
}



bool IsNTFS(char* cFileName, wchar_t** file_system, wchar_t** root) 
{
	char* cPath = new char[32768];
	
	std::string fileName;
	std::string fileExtension;
	std::string pathName;
	splitpathname<char>(const_cast<char*>(cFileName), fileName, fileExtension, pathName);
	
	strcpy(cPath, pathName.c_str());
	wchar_t* wcPath = new wchar_t[32768];
	fromUTF8(cPath, wcPath);
	
	wchar_t* wcVolumePath = new wchar_t[32768];
	
	(*UGetVolumePathName())(wcPath, wcVolumePath, 32768);
	if (root)
		*root = wcVolumePath; 
	
	wchar_t* wcFileSystem = new wchar_t[16];
    (*UGetVolumeInformation())(wcVolumePath, NULL, 0, NULL, NULL, NULL, wcFileSystem, 16);
	if (file_system)
		*file_system = wcFileSystem;

	char* cFileSystem = NULL;
	WStr2Str((char*)wcFileSystem, &cFileSystem);

	bool result;
	result = (!_stricmp(cFileSystem, "NTFS"));

	delete wcPath;
	delete cFileSystem;
	if (!file_system)
		delete wcFileSystem;
	if (!root)
		delete wcVolumePath;
	delete cPath;

	return result;
}

bool VerifySuitableFileSystem(char* cFilename, __int64 max_size)
{
	bool result = true;

	wchar_t* file_system = NULL;
	wchar_t* root_dir = NULL;

	bool bIsNTFS = IsNTFS(cFilename, &file_system, &root_dir);

	if (!bIsNTFS && max_size > 4150000000) {
		char* msg = LoadString(IDS_OVER4GBONLYNTFS, LOADSTRING_UTF8);
		wchar_t* wmsg = NULL;
		UTF82WStr(msg, (char**)&wmsg);
		wchar_t m[32768]; m[0]=0;

		// TODO: Testen
		_swprintf_c(m, 32768, wmsg, root_dir, file_system);

		wchar_t* title = NULL;
		UTF82WStr(LoadString(STR_GEN_ERROR, LOADSTRING_UTF8), (char**)&title);

		result = (IDYES == MessageBoxW(NULL, m, title, MB_YESNO | MB_ICONERROR | MB_DEFBUTTON2));

		delete wmsg;
	}

	delete file_system;
	delete root_dir;

	return result;
}


/* log window helper functions */
bool AddSizeStringToLog(DEST_AVI_INFO* lpDAI, char* text, int type, __int64 value)
{
	char csize[64]; memset(csize, 0, sizeof(csize));
	if (type == 1) {
		QW2Str(value, csize, 1);
		strcat(csize, " ");
		strcat(csize, LoadString(STR_BYTES, LOADSTRING_UTF8));
	} else
	if (type == 2)
		FormatSize(csize, value);

	char cfinal[1024]; memset(cfinal, 0, sizeof(cfinal));
	sprintf(cfinal, "%s %s", text, csize);

	lpDAI->dlg->AddProtocolLine(cfinal, 4, APL_UTF8);

	return true;
}

bool AddNumberToLog(DEST_AVI_INFO* lpDAI, char* text, __int64 value)
{
	char cnbr[64]; memset(cnbr, 0, sizeof(cnbr));
	char cfinal[1024]; memset(cfinal, 0, sizeof(cfinal));

	QW2Str(value, cnbr, 1);
	sprintf(cfinal, "%s %s", text, cnbr);

	lpDAI->dlg->AddProtocolLine(cfinal, 4, APL_UTF8);

	return true;
}

typedef struct {
	bool	enabled;
	int		cache_line_size;
	int		cache_lines;
} CACHE_INFO;

void RetrieveCacheInfo(CACHE_INFO& cache_info, CAttribs* attr)
{
	cache_info.enabled = !!attr->GetIntWithDefault("cache/enabled", 1);
	cache_info.cache_line_size = 1 << (int)attr->GetIntWithDefault("cache/size per cacheline", 20);
	cache_info.cache_lines = (int)attr->GetIntWithDefault("cache/cachelines", 8);
}

void AddCacheInfoToLog(DEST_AVI_INFO* lpDAI, CACHE_INFO* cache_info)
{
	char c1[1024]; c1[0]=0;
	char c2[64]; c2[0]=0;

	lpDAI->dlg->AddProtocolLine(Message_index(MSG_CACHE, cache_info->enabled), 4);
	QW2Str(cache_info->cache_line_size >> 10, c2, 1);
	sprintf(c1, LoadString(STR_MUXLOG_CACHE_INFO, LOADSTRING_UTF8), cache_info->cache_lines, c2);
	lpDAI->dlg->AddProtocolLine(c1, 4, APL_UTF8);
}

int MuxThread_AVI(DEST_AVI_INFO* lpDAI)
{
	WaitForMuxing(lpDAI);
	CAttribs*       s = lpDAI->settings;
	AVIFILEEX*		AVIOut, *NextAVIOut;
	CFileStream*		DestFile, *NextDestFile;
	char*			cBuffer;
	char			RawFilename[32768];
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
//	DWORD			dwChunkOverhead;
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
	bool			bInjectSpace=false;
	bool			bFilesize_reached;
	bool			bManual_splitpoint_reached;
	bool			bMustCorrectAudio=false;
	bool			bSplitInitiated=false;
	bool			bKeyframe=false;
	bool			bRecListEmpty=true;


	__int64			iPreload = s->GetInt("output/avi/audio preload");
	MUX_STATE*		lpmsState;
	CString			cStr[2];
	bool			bKeyframe_passed=false;
	__int64			iDistToSplitpoint = 0;

	MULTIMEDIASOURCEVECTOR mms;
	int				estimated_number_of_chunks = 0;
	__int64			estimated_raw_stream_size = 0;
	int				estimated_overhead = 0;
	int				overhead_per_file = 0;
	int				overhead_per_reclist = 0;
	int				overhead_per_chunk = 0;
	int				estimated_number_of_reclists = 0;
	__int64			estimated_total_size = 0;
	__int64			free_disk_space = 0;

	std::vector<__int64> injected_space;

	int iError = 0;

	char			Buffer[512],cName[512];
	char*			stream_report,*report;
	MSG_LIST*		msglist;
	SILENCE*		silence;
	WAVEFORMATEX*	lpwfe;
	VIDEOSOURCE*	v;
	CACHE*			cache;

	HANDLE			hSem;

	bMuxing = true;

	GenerateMMSVector(lpDAI, mms);
	estimated_raw_stream_size = GetMMSVSize(mms);

	DWORD			dwLegIndOverHead=0;

	v = lpDAI->videosource;
	hSem = CreateSemaphore(NULL, 1, 1, sem_name);

	ZeroMemory(RawFilename,sizeof(RawFilename));
	memcpy(RawFilename,lpDAI->lpFileName,lstrlen(lpDAI->lpFileName)-4);

	StopMuxing(false);
	dwFrameSizes = (DWORD*)malloc(iSize=dwFrameSizes_Len*sizeof(DWORD));
	ZeroMemory(dwFrameSizes,iSize);

	stream_report = (char*)calloc(1,32768);
	cFilename = (char*)calloc(1,32768);
	cBuffer = (char*)calloc(1,32768);
	silence=new SILENCE;

	char dir[32768];
	GetModuleFileName(NULL, dir, 512);
	silence->Init(dir);

	std::basic_string<TCHAR> strVersion = ComposeVersionString();

	cStr[0]=LoadString(STR_MUXLOG_BEGIN);
	lpDAI->dlg->AddProtocolLine(cStr[0],5);

	bool bUnbufferedOutput = !!s->GetInt("output/general/unbuffered");
	bool bOverlapped = !!lpDAI->settings->GetInt("output/general/overlapped");
	bool bThreaded = !!lpDAI->settings->GetInt("output/general/threaded");
	bool bLegacyIndex = !!s->GetInt("output/avi/legacyindex");
	bool bOpenDML  = !!s->GetInt("output/avi/opendml/on");
	bool bReclists = !!s->GetInt("output/avi/reclists");
	bool bHaalimode = !!s->GetInt("output/avi/opendml/haalimode") && bOpenDML && !bLegacyIndex;
	bool bCheckDiskSpace = !!s->GetInt("output/general/check disk space/enabled");
	__int64 iDiscSpaceTolerance = s->GetInt("output/general/check disk space/lower limit") * (1<<20);

	CACHE_INFO cache_info;
	RetrieveCacheInfo(cache_info, s->GetAttr("output/general"));
	lpDAI->dlg->m_Prg_Legidx_Label.ShowWindow(bLegacyIndex);
	lpDAI->dlg->m_Prg_Legidx_Progress.ShowWindow(bLegacyIndex);
	lpDAI->dlg->m_Prg_Legidx_Progress.SetPos(0);

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
	switch (bOpenDML) {
		case 0: cStr[1]=LoadString(IDS_VI_STANDARD); 
				overhead_per_file = 4096;
				overhead_per_reclist = 28;
				overhead_per_chunk = 24;
				break;
		case 1: cStr[1]=(bLegacyIndex)?LoadString(IDS_VI_HYBRIDE):LoadString(IDS_VI_OPENDML);
				overhead_per_file = 65536;
				overhead_per_reclist = 12;
				overhead_per_chunk = 16;
				break;
	}

	sprintf(Buffer,"%s: %s",cStr[0],cStr[1]);
	lpDAI->dlg->AddProtocolLine(Buffer,5);
	lpDAI->dlg->AddProtocolLine(LoadString(MSG_LOWOVHDAVI[bHaalimode]),4);
	lpDAI->dlg->AddProtocolLine(LoadString(MSG_RECLISTS[bReclists]),4);
	lpDAI->dlg->AddProtocolLine(cPattern,4);




	// add audio interleave scheme to protocol
	switch(ai_unit) {
		case AIU_KB: 
			sprintf(cPattern, LoadString(STR_MUXLOG_AUDIOINTERLEAVE_KB), ai_value); 
			if (bReclists)
				estimated_number_of_reclists = (int)(estimated_raw_stream_size / ai_value / 1024);
			
			break;
		case AIU_FRAME : 
			sprintf(cPattern, LoadString(STR_MUXLOG_AUDIOINTERLEAVE_FR), ai_value); 
			if (bReclists)
				estimated_number_of_reclists = (int)(v->GetDuration() * v->GetTimecodeScale() / v->GetNanoSecPerFrame() / ai_value);
			break;
	}
	lpDAI->dlg->AddProtocolLine(cPattern,4);

	qwMaxBytes=(__int64)(s->GetInt("output/general/file size/max"))*1048576;
	
	if (qwMaxBytes<(__int64)(1<<25)*(1<<25)) {
		QW2Str(qwMaxBytes,cSize[0],3);
		wsprintf(cBuffer,LoadString(STR_MUXLOG_MAXFILESIZE),cSize[0],LoadString(STR_BYTES));
		lpDAI->dlg->AddProtocolLine(cBuffer,5);
	}
 
	cStr[1]=LoadString(STR_MUXLOG_NAME);
	cStr[0]=LoadString(STR_MUXLOG_AUDIOSTREAMS);
	
	/* begin estimating chunks here */
	if (v)
		estimated_number_of_chunks += v->GetNbrOfFrames();

	sprintf(Buffer,cStr[0],lpDAI->dwNbrOfAudioStreams);
	lpDAI->dlg->AddProtocolLine(Buffer,5);
	for (i=0;i<lpDAI->dwNbrOfAudioStreams;i++)
	{
		lpDAI->asi[i]->iScaleF = NULL;
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
		__int64 a_duration = a->GetDuration() * a->GetTimecodeScale();
		__int64 frame_duration = a->GetFrameDuration();
		j = 0;

		if (a->GetFormatTag() == 0x2000) {
			a->SetFrameMode(j = (int)lpDAI->settings->GetInt("output/avi/ac3/frames per chunk"));
			sprintf(Buffer,LoadString(STR_MUXLOG_AC3AVIPATTERN), j);
			lpDAI->dlg->AddProtocolLine(Buffer,5);
		} else
		if (a->GetFormatTag() == 0x2001) {
			a->SetFrameMode(j = (int)lpDAI->settings->GetInt("output/avi/dts/frames per chunk"));
			sprintf(Buffer,LoadString(STR_MUXLOG_DTSAVIPATTERN), j);
			lpDAI->dlg->AddProtocolLine(Buffer,5);
		} else
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
					if (lpDAI->asi[i]->iScaleF)
						delete lpDAI->asi[i]->iScaleF;
					lpDAI->asi[i]->iScaleF = new int;
					*lpDAI->asi[i]->iScaleF = j;
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
		} else {
			j = 1;
			/* audio format with one chunk per interleave interval */
		}

		if (frame_duration > 1000 && j > 0) {
			estimated_number_of_chunks += (int)(a_duration / (j * frame_duration));
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


	AddCacheInfoToLog(lpDAI, &cache_info);
	lpDAI->dlg->AddProtocolLine(Message_index(MSG_UNBUFFERED, bUnbufferedOutput), 4);
	lpDAI->dlg->AddProtocolLine(Message_index(MSG_OVERLAPPEDOUT, bOverlapped), 4);

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
	ZeroMemory(qwStats,sizeof(qwStats));
	int buffer_size = 1<<22;
	lpBuffer=malloc(1<<22);
	memset(lpBuffer, 0, buffer_size);

	thread=AfxBeginThread((AFX_THREADPROC)DisplayProgress_Thread,lpDPI);
	lpDPI->lpThis = thread;

	if (ResolveSplitpoints(lpDAI, RSPF_CHAPTERS, &iError) == RSP_ERROR) {
		sprintf(cBuffer,LoadString(STR_MUXLOG_SPLITPOINTB0RKED),iError+1);
		lpDAI->dlg->MessageBox(cBuffer,LoadString(STR_GEN_ERROR),MB_OK | MB_ICONERROR);
		lpDAI->dlg->SetDialogState_Config();
		lpDAI->dlg->ButtonState_STOP();
		lpDAI->chapters->SetBias(0,BIAS_UNSCALED);
		lpDPI->dwLeave = 1;
		ReleaseSemaphore(hSem, 1, NULL);
		FinishMuxing(lpDAI);
		return 0;
	}

	__int64 overwriteable_size = 0;
	int check = CHECKEXISTENCE_OVERWRITE;

	estimated_overhead = overhead_per_chunk * estimated_number_of_chunks +
		                 overhead_per_reclist * estimated_number_of_reclists +
						 overhead_per_file * lpDAI->dwEstimatedNumberOfFiles;
	estimated_total_size = estimated_raw_stream_size + estimated_overhead;

	if (qwMaxBytes > 0)
		lpDAI->dwEstimatedNumberOfFiles = (DWORD)(estimated_total_size / qwMaxBytes + 1);
	if (lpDAI->split_points && lpDAI->split_points->GetCount())
		lpDAI->dwEstimatedNumberOfFiles = max(lpDAI->split_points->GetCount()+1, lpDAI->dwEstimatedNumberOfFiles);

	CUTF8 utf8FileNameFormat(lpDAI->fileNameFormat.c_str());

	if (lpDAI->settings->GetInt("gui/general/overwritedlg")) {
		check = FirstFilenameCheck(lpDAI, RawFilename, FFC_FORMAT_AVI, &overwriteable_size);
		if (check == -1)
			goto finish;
	}

	if (bHaalimode) {
		overhead_per_chunk = 8;
		overhead_per_reclist += 8 * mms.size();
	}


	int space_method = 1;

	AddSizeStringToLog(lpDAI, LoadString(STR_MUXLOG_ESTIMATEDRAWSIZE, LOADSTRING_UTF8), 
		space_method, estimated_raw_stream_size);

	AddSizeStringToLog(lpDAI, LoadString(STR_MUXLOG_ESTIMATEDOVERHEAD, LOADSTRING_UTF8),
		space_method, estimated_overhead);

	AddSizeStringToLog(lpDAI, LoadString(STR_MUXLOG_ESTIMATEDTOTALSIZE, LOADSTRING_UTF8),
		space_method, estimated_total_size);

	AddSizeStringToLog(lpDAI, LoadString(STR_MUXLOG_OVERWRITEABLE, LOADSTRING_UTF8),
		space_method, overwriteable_size);

	free_disk_space = GetDirectoryFreeSpace(RawFilename);
	AddSizeStringToLog(lpDAI, LoadString(STR_MUXLOG_FREEDISKSPACE, LOADSTRING_UTF8),
		space_method, free_disk_space);

	__int64 max_size = min(estimated_total_size, qwMaxBytes);

	if ((max_size > (__int64)(1<<20) * 2000) && !bOpenDML) {
		char* msg = LoadString(IDS_OVER2GBONLYODML);
		int res = MessageBox(NULL, msg, LoadString(STR_GEN_ERROR), MB_YESNO | MB_ICONERROR);
		if (res == IDNO) {
			bStop = true;
			goto finish;
		} else {
			bOpenDML = true;
		}
	}

	if (!CheckDiskSpace(bCheckDiskSpace, estimated_total_size, free_disk_space,
		iDiscSpaceTolerance, overwriteable_size)) {
			bStop = true;
			goto finish;
	}

	if (!VerifySuitableFileSystem(RawFilename, max_size)) {
		bStop = true;
		goto finish;
	}

	memset(cFilename, 0, sizeof(cFilename));
	FormatOutputFileName(cFilename, utf8FileNameFormat.TStr(), /*lpDAI->lpFormat,*/ RawFilename,
		dwPartNbr, NULL);

	if (check != CHECKEXISTENCE_OVERWRITE)
		if (CheckExistence(cFilename, check, NULL) == CHECKEXISTENCE_STOP) {
			bStop = true;
			goto finish;
		} 

	lpDAI->dlg->SetDlgItemText(IDC_DESTFILE,cFilename);

	DeleteFileUTF8(cFilename);
	DestFile=new CFileStream;
	if (DestFile->Open(cFilename, StreamMode::Read | 
			(bUnbufferedOutput?StreamMode::UnbufferedWrite:StreamMode::Write) | 
			(bThreaded?StreamMode::Threaded:StreamMode::None) |
			(bOverlapped?StreamMode::Overlapped:StreamMode::None))==STREAM_ERR) {

		cStr[0]=LoadString(IDS_COULDNOTOPENOUTPUTFILE);
		lpDAI->dlg->MessageBox(cStr[0],lpDAI->dlg->cstrError,MB_OK | MB_ICONERROR);
		lpDAI->dlg->ButtonState_STOP();
		lpDAI->dlg->SetDialogState_Config();
		FinishMuxing(lpDAI);
		lpDPI->dwLeave=1;

//		cacheAllowReadAhead(0);
		return 0;
	}

	if (bUnbufferedOutput || bOverlapped || cache_info.enabled) {
		cache = new CACHE(cache_info.cache_lines, cache_info.cache_line_size);
		cache->Open(DestFile, CACHE_OPEN_READ | CACHE_OPEN_WRITE | CACHE_OPEN_ATTACH);
		cache->Enable(CACHE_IMMED_WRITEBACK);
	} else {
		cache = (CACHE*)DestFile;
	}
	
// open first output file
	AVIOut=new AVIFILEEX;
	AVIOut->SetDebugState(DS_DEACTIVATE);
	AVIOut->Open(cache,FA_WRITE,(bOpenDML)?AT_OPENDML:AT_STANDARD);
	AVIOut->SetNumberOfStreams(lpDAI->dwNbrOfAudioStreams+1+lpDAI->dwNbrOfSubs);
	AVIOut->SetPadding(lpDAI->dwPadding);
	AVIOut->SetLegacyCallBack(StartLegacyCB);
	AVIOut->MoveHDRL(i=!!(int)(s->GetInt("output/avi/move hdrl")));
	lpDAI->dlg->AddProtocolLine(LoadString(MSG_JUNKB4HDR[!!i]), 4);
// setting Haali muxing mode if it has been chosen and is possible
	if (bLegacyIndex) 
		AVIOut->Enable(AFE_LEGACYINDEX);
	
	AVIOut->EnableLowOverheadMode(!!(lpDAI->settings->GetInt("output/avi/opendml/haalimode")));

	if (lpDAI->cTitle) {
		lpDAI->cTitle->SetOutputFormat(CSB_ASCII);
		AVIOut->SetTitle(lpDAI->cTitle->Get());
		lpDAI->cTitle->SetOutputFormat(CSB_UTF8);
	}

	//ComposeVersionString(cBuffer);
	cBuffer[0];
	strcpy_s(cBuffer, 64, strVersion.c_str());
	AVIOut->SetWritingAppName(cBuffer);
	if (s->GetInt("output/avi/opendml/stdindex/pattern") == SIP_FRAMES)
	{
		AVIOut->SetFramesPerIndex((int)s->GetInt("output/avi/opendml/stdindex/interval"));
	}
	qwNanoSecNeeded=(__int64*)malloc(8*lpDAI->dwNbrOfAudioStreams);
	ZeroMemory(qwNanoSecNeeded,8*lpDAI->dwNbrOfAudioStreams);
	qwNanoSecStored=(__int64*)malloc(8*lpDAI->dwNbrOfAudioStreams);
	ZeroMemory(qwNanoSecStored,8*lpDAI->dwNbrOfAudioStreams);

// set stream headers and formats
	AVIOut->SetStreamHeader(0,lpDAI->videosource->GetAVIStreamHeader());
	AVIOut->SetStreamFormat(0,lpDAI->videosource->GetFormat());
	v->GetName(Buffer);
	AVIOut->SetStreamName(0, Buffer);
	AVIOut->GetStdIndexOverhead();

// seek audio streams to beginning
	for (i=0;i<lpDAI->dwNbrOfAudioStreams;i++) {
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
		lpDAI->ssi[i]->lpsubs->GetName(Buffer);
		AVIOut->SetStreamName(i+1+lpDAI->dwNbrOfAudioStreams, Buffer);
		lpDAI->ssi[i]->lpsubs->SetBias(0,BIAS_ABSOLUTE | BIAS_UNSCALED);
		lpDAI->ssi[i]->lpsubs->ReInit();
		AVIOut->SetStreamDefault(i+1+lpDAI->dwNbrOfAudioStreams, !!lpDAI->ssi[i]->lpsubs->IsDefault());
	}
	AVIOut->SetNanoSecPerFrame(lpDAI->videosource->GetNanoSecPerFrame());
	AVIOut->SetMaxRIFFAVISize(i = (1<<20)*(int)lpDAI->settings->GetInt("output/avi/opendml/riff avi size")); 

	QW2Str((__int64)((float(i)+(1<<19))/(1<<20)), cSize[0], 1);
	sprintf(Buffer, LoadString(STR_MUXLOG_FIRSTRIFFSIZE), cSize);
	lpDAI->dlg->AddProtocolLine(Buffer, 4);

	dwLegIndOverHead=8*((!bOpenDML)||(bLegacyIndex));
	AddOverhead(qwStats,AVIOut->GetHeaderSpace()+12+dwLegIndOverHead);
	dwTime=GetTickCount();
	dwAudioBPS=0;

	bManual_splitpoint_reached = false;
	bFilesize_reached = false;
	wsprintf(cBuffer,LoadString(STR_MUXLOG_NEWFILE),0,0,0,0,0,cFilename);
	lpDAI->dlg->AddProtocolLine(cBuffer,5, APL_UTF8);
	dwFramesInCurrFile=0;
//	dwChunkOverhead=(bOpenDML)?(AVIOut->IsLegacyEnabled())?32:16:24;

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
					dwMicroSecRead = 0;
					iSize=0;
					if (!bExternalSilence)
					{
						// compatible silence file not present
						AUDIOSOURCE* a = lpDAI->asi[i]->audiosource;
						int fm = a->GetFrameMode();
						if (fm == -1) fm=1;
						a->SetFrameMode(1);
						BYTE* bBuffer = (BYTE*)lpBuffer;
						for (int j=0;j<fm;j++) {
							DWORD m = 0;
							iSize+=a->Read(bBuffer+iSize,min(iDelay*1000,20000),&m,NULL);
							a->Seek(0);
							dwMicroSecRead+=m;
						}
						lpDAI->asi[i]->audiosource->Seek(0);
						a->SetFrameMode(fm);
					}
					else
					{
						// compatible silence file present
						iSize=silence->Read(lpBuffer,min(iDelay*1000,20000),&dwMicroSecRead,NULL);
					}
					
					dwFlags=AVIIF_KEYFRAME;
					AVIOut->AddChunk(i+1,&lpBuffer,iSize,dwFlags);
					if (!lpBuffer) lpBuffer=malloc(1<<22);
					iDelay-=(int)round(dwMicroSecRead/1000);

					AddAudioSize(qwStats,iSize);
					AddOverhead(qwStats,overhead_per_chunk);
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
		qwNanoSecNeeded[i]+= iPreload * 1000000;
	}
	bPreloadCompensated=false;
	
	dwPreloadedFrames=(DWORD)(d_div(1000000.0f*(double)iPreload,(double)lpDAI->videosource->GetNanoSecPerFrame(),
		"MuxThread::lpDAI->videosource->GetNanoSecPerFrame()"));

	lpDAI->dlg->m_Prg_Progress.SetRange32(0,10000);
	bFinishImmediately=false;
	lpDAI->videosource->InvalidateCache();
	bFirst=true;
	dwLastFileBegin=0;
	lpDAI->lpProtocol->EnsureVisible(lpDAI->lpProtocol->GetItemCount()-1,false);


	srand(GetTickCount());
	/* injecting space */
	if (lpDAI->settings->GetInt("output/avi/inject/enabled")) {
		bInjectSpace = true;
		lpDAI->dlg->AddProtocolLine("injecting spaces enabled", 4);
		__int64 prob = lpDAI->settings->GetInt("output/avi/inject/probability");
		char c[64];
		sprintf(c, "probability: %1.2f%%", 100. * (double)prob/10000.);
		lpDAI->dlg->AddProtocolLine(c, 4);
		sprintf(c, "injecting %d bytes", lpDAI->settings->GetInt("output/avi/inject/size"));
		lpDAI->dlg->AddProtocolLine(c, 4);
	}
//	ResolveSplitpoints(lpDAI, RSPF_CHAPTERS);
	
	i_vs_nspf = v->GetNanoSecPerFrame();
	
	iDistToSplitpoint = DistToNextSplitPoint(lpDAI,dwFrameCountTotal,0)-1;
	dwReclistOverhead=(bOpenDML)?12:28;
// mux
	//while ((dwFrameCountTotal<lpDAI->dwMaxFrames)&&(!bStop))

	// lpDAI->dlg->m_Protocol.ShowWindow(SW_HIDE);

	while ((!v->IsEndOfStream())&&(!bStop))
	{
//		dwChunkOverhead=(bOpenDML)?(AVIOut->IsLegacyEnabled())?32:16:24;
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
		//	wsprintf(cBuffer,"begin rec list at frame %d",dwFrameCountTotal);
		//	lpDAI->dlg->AddProtocolLine(cBuffer,3);
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
			
			int orig_size = iSize;
			bool bInjected = false;
			if (bInjectSpace) {
				__int64 prob = lpDAI->settings->GetInt("output/avi/inject/probability");
				int random = rand() % 10000;
				if (random < prob) {
					int s = (int)lpDAI->settings->GetInt("output/avi/inject/size");
					iSize += s;
					bInjected = true;
					//__int64 
				}
			}

			__int64 chunk_file_pos;
			memset(((char*)lpBuffer)+orig_size, 0, (int)lpDAI->settings->GetInt("output/avi/inject/size"));
			AVIOut->AddChunk(0,&lpBuffer,iSize,dwFlags, &chunk_file_pos);
			if (bInjected)
				injected_space.push_back(chunk_file_pos + orig_size + 8);
			if (!lpBuffer) 
				lpBuffer=malloc(1<<22);
			dwFrameSizes_Sum -= dwFrameSizes[dwFrameSizes_Pos];
			dwFrameSizes[dwFrameSizes_Pos++]=iSize;
			dwFrameSizes_Pos%=dwFrameSizes_Len;
			dwFrameSizes_Sum += iSize;

			dwRecListSize+=iSize+dwAudioBPVF;
		// update statistics
			AddVideoSize(qwStats,iSize);
			AddOverhead(qwStats, overhead_per_chunk);

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
				if (!bPreloadCompensated) 
					for (i=0;i<lpDAI->dwNbrOfAudioStreams;qwNanoSecNeeded[i++]-=iPreload*1000000);

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
	//	wsprintf(cBuffer,"  frames total: %6d",dwFrameCountTotal);
	//	lpDAI->dlg->AddProtocolLine(cBuffer,3);

		bFirst=false;
		dwAudioBPS=0;
		for (i=0;i<lpDAI->dwNbrOfAudioStreams;i++)
		{
			WaitForSingleObject(hSem, INFINITE);
			/*if (dwFrameCountTotal==lpDAI->dwMaxFrames)
			{
				iSize=1;
			}*/
			iSize=1;
			dwAudioBPS+=lpDAI->asi[i]->audiosource->GetAvgBytesPerSec();
			// as long as more audio is required
			while ((qwNanoSecNeeded[i]>=20000000)&&(iSize>0))
			{
				// read
				if (qwNanoSecNeeded[i]/1000<1500000) {
					dwNextAudioBlock=(DWORD)qwNanoSecNeeded[i]/1000;
				} else {
					dwNextAudioBlock=1000000;
				}

				iSize=lpDAI->asi[i]->audiosource->Read(lpBuffer,
					dwNextAudioBlock,NULL,&qwNanoSecRead);
				
/*				if (!iSize) {
					iSize=lpDAI->asi[i]->audiosource->Read(lpBuffer,
						dwNextAudioBlock,NULL,&qwNanoSecRead);
				}
				*/
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
					AddOverhead(qwStats, overhead_per_chunk);
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
		
	//	wsprintf(cBuffer,"  audio: %6d bytes",iSize);
	//	lpDAI->dlg->AddProtocolLine(cBuffer,3);

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
			CACHE* nextcache = NULL;

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
					memset(lpBuffer, 0, buffer_size);
					iSize=lpDAI->ssi[i]->lpsubs->Render2AVIChunk(lpBuffer);
					AVIOut->AddChunk(1+i+lpDAI->dwNbrOfAudioStreams,&lpBuffer,iSize,AVIIF_KEYFRAME);
					if (!lpBuffer) lpBuffer=malloc(buffer_size);
					AddOverhead(qwStats, overhead_per_chunk);
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

			if (bInjectSpace) {
				strcat(cFilename, ".spaces.txt");
				FILE* f = fopenutf8(cFilename, "wb", DoesOSSupportUnicode());
				if (!f)
					MessageBox(0, "Could not open file for space list",
						"Fatal error", MB_OK | MB_ICONERROR);

				std::vector<__int64>::iterator iter = injected_space.begin();
				for (; iter != injected_space.end(); iter++) {
					fprintf(f, "%I64d\x0D\x0A", *iter);
				}
				fclose(f);
			}


			// new output file
			NextAVIOut = NULL;
			if (lpDAI->dwMaxFrames!=dwFrameCountTotal && !bStop)
			{
				bFinishPart=false;
				dwPartNbr++;
				bSplitInitiated=false;
				ZeroMemory(cFilename,sizeof(cFilename));
				//wsprintf(cFilename,lpDAI->lpFormat,RawFilename,dwPartNbr);
				FormatOutputFileName(cFilename,/*lpDAI->lpFormat*/utf8FileNameFormat.TStr(), RawFilename,
					dwPartNbr, NULL);

				if (!v->IsEndOfStream()) {
	
					if (check != CHECKEXISTENCE_OVERWRITE)
						if (CheckExistence(cFilename, check, NULL) == CHECKEXISTENCE_STOP) {
							bStop = true;
							goto finish2;
					} 

				
					strcpy(cBuffer,cFilename);				
					DeleteFileUTF8(cFilename);
					NextDestFile=new CFileStream;
					if (NextDestFile->Open(cBuffer, StreamMode::Read |
						(bUnbufferedOutput?StreamMode::UnbufferedWrite:StreamMode::Write) | 
						(bThreaded?StreamMode::Threaded:StreamMode::None) |
						(bOverlapped?StreamMode::Overlapped:StreamMode::None))==STREAM_ERR) {

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

						return 0;
					}
					if (bUnbufferedOutput || bOverlapped || cache_info.enabled) {
						nextcache = new CACHE(cache_info.cache_lines, cache_info.cache_line_size);
						nextcache->Open(NextDestFile, CACHE_OPEN_READ | CACHE_OPEN_WRITE | CACHE_OPEN_ATTACH);
						nextcache->Enable(CACHE_IMMED_WRITEBACK);
					} else {
						nextcache = (CACHE*)NextDestFile;
					}

					NextAVIOut=new AVIFILEEX;
					NextAVIOut->SetDebugState(DS_DEACTIVATE);
					NextAVIOut->Open(nextcache,FA_WRITE,(bOpenDML)?AT_OPENDML:AT_STANDARD);
					NextAVIOut->SetNumberOfStreams(lpDAI->dwNbrOfAudioStreams+1+lpDAI->dwNbrOfSubs);
					NextAVIOut->SetNanoSecPerFrame(i_vs_nspf);
					NextAVIOut->SetMaxRIFFAVISize(i = (1<<20)*(int)lpDAI->settings->GetInt("output/avi/opendml/riff avi size")); 
					NextAVIOut->EnableLowOverheadMode(!!(lpDAI->settings->GetInt("output/avi/opendml/haalimode")));
					NextAVIOut->MoveHDRL(!!(int)(s->GetInt("output/avi/move hdrl")));

					for (i=0;i<lpDAI->dwNbrOfAudioStreams+1+lpDAI->dwNbrOfSubs;i++)
					{
						NextAVIOut->SetStreamHeader(i,AVIOut->GetStreamHeader(i));
						NextAVIOut->SetStreamFormat(i,AVIOut->GetStreamFormat(i));
						NextAVIOut->SetStreamDefault(i, AVIOut->IsDefault(i));
					}
					if (v) {
						v->GetName(Buffer);
						AVIOut->SetStreamName(0, Buffer);
					}
					for (i=0;i<lpDAI->dwNbrOfAudioStreams;i++)
					{
						lpDAI->asi[i]->audiosource->GetName(Buffer);
						NextAVIOut->SetStreamName(i+1,Buffer);
					}
					cBuffer[0]=0;
					strcpy_s(cBuffer, 64, strVersion.c_str());
					//ComposeVersionString(Buffer);
					NextAVIOut->SetWritingAppName(Buffer);
					if (lpDAI->cTitle) {
						lpDAI->cTitle->SetOutputFormat(CSB_ASCII);
						NextAVIOut->SetTitle(lpDAI->cTitle->Get());
						lpDAI->cTitle->SetOutputFormat(CSB_UTF8);
					}

				}
			}

finish2:
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
				if (!i) {
					dwFramesInCurrFile=dwFrameCountTotal-dwLastFileBegin;
					qwMillisec=((__int64)dwFramesInCurrFile)*i_vs_nspf/1000000;
				} else {
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


			if (NextAVIOut)
			{
				injected_space.clear();
				lpDAI->dlg->AddProtocolLine("-------------------------------------------------------------",4);
				qwMillisec=(__int64)(dwFrameCountTotal*i_vs_nspf/1000000);
				
				Millisec2HMSF(qwMillisec,&dwHour,&dwMin,&dwSec,&dwFrac);
				cStr[0]=LoadString(STR_MUXLOG_NEWFILE);
				wsprintf(cBuffer,cStr[0],dwHour,dwMin,dwSec,dwFrac,dwFrameCountTotal,cFilename);
				lpDAI->dlg->AddProtocolLine(cBuffer,5, APL_UTF8);

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
				for (i=0;i<lpDAI->dwNbrOfAudioStreams;qwNanoSecNeeded[i++]+=iPreload*1000000);
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
finish:
	HANDLE hSem_finish = CreateSemaphore(NULL,0,1,sem_name_finish);
	lpDPI->dwLeave=1;
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
//					MSG_LIST_2_Msg(lpDAI->lpAC3_logs[i]->lpMessages,stream_report);
//					MSG_LIST_append(msglist,stream_report);
				}
			}
			MSG_LIST_append(msglist,"");
		}

		cStr[0]=LoadString(IDS_READY);
		MSG_LIST_append(msglist,cStr[0].GetBuffer(255));
//		MSG_LIST_2_Msg(msglist,report);

		if (s->GetInt("gui/general/finished_muxing_dialog")) 
			lpDAI->dlg->MessageBox(report,lpDAI->dlg->cstrInformation,
				MB_OK | MB_ICONINFORMATION);

		MSG_LIST_clear(msglist);
		msglist=NULL;
	}
	else
	{
		if (s->GetInt("gui/general/finished_muxing_dialog")) 
			lpDAI->dlg->MessageBox(cStr[0],lpDAI->dlg->cstrInformation,MB_OK | MB_ICONINFORMATION);
	}
//	delete lpDPI;
	lpDAI->dlg->SetDialogState_Config();
	v->Enable(0);
	for (i=0;i<lpDAI->dwNbrOfAudioStreams;i++) {
		if (lpDAI->asi[i]->iScaleF) {
			int j=*lpDAI->asi[i]->iScaleF;
			delete lpDAI->asi[i]->iScaleF;
			lpDAI->asi[i]->iScaleF=NULL;
			lpDAI->asi[i]->lpASH->dwScale /= j;
			((WAVEFORMATEX*)lpDAI->asi[i]->lpFormat)->nBlockAlign /= (unsigned short)j;
		}

		lpDAI->asi[i]->audiosource->Enable(0);
	}
//	free(lpDAI->lpFormat);
//	free(lpDAI->asi);
//	if (lpDAI->ssi&&lpDAI->dwNbrOfSubs) free(lpDAI->ssi);
	free(lpDAI->lpFileName);
	
	for (i=0;i<lpDAI->dwNbrOfAC3Streams;i++)
	{

		if (lpDAI->lpAC3_logs[i]) 
		{
//			if (lpDAI->lpAC3_logs[i]->lpMessages) delete lpDAI->lpAC3_logs[i]->lpMessages;
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
//	delete report;
	delete cBuffer;
	delete stream_report;
	delete dwFrameSizes;
	delete cFilename;
	silence->Close();
	delete silence;
	if (bExit) c->PostMessage(WM_COMMAND, ID_LEAVE, 0);
	bMuxing = false;
	CloseHandle(hSem);	
	FinishMuxing(lpDAI);
	delete lpDAI->split_points;
	delete lpDAI->settings;
	free(lpDAI);
//	cacheAllowReadAhead(0);

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



/* Format:

     $Name      : raw filename
	 $Nbr[{len}]: number of file
	 $Chn[{lng}]: chapter in in language lng 
*/

const int FOFN_USE_CHAPTER_NAME = 0x04;
const int FOFN_CAN_USE_CHAPTER_NAME = 0x80;

int FormatOutputFileName(char* cDest, const char* cFormat, const char* cRawFileName,
						 int iCurrentFile, SPLIT_POINT_DESCRIPTOR* pSPD,
						 int* flags)
{
	char*	p = cDest;
	int		chn_name = 0;
	int		allow_chn = 0;
	if (flags)
		allow_chn = (*flags & FOFN_CAN_USE_CHAPTER_NAME)?1:0;


	while (*cFormat) {
		
		while (*cFormat && *cFormat != '$') *cDest++ = *cFormat++;
		if (*cFormat == '$') {
			if (!_strnicmp(cFormat+1,"Name",4)) {
				cFormat+=5;
				strcat(cDest, cRawFileName);
				cDest+=strlen(cDest);
			} else
			if (!_strnicmp(cFormat+1,"Nbr",3)) {
				cFormat+=4;
				int nbr_len = 1;
				if (*cFormat == '{') {
					int end = 0;
					end = (DWORD)strstr(cFormat+1, "}") - DWORD(cFormat+1);
					if (!end) {
						cFormat+=strlen(cFormat);
						return 0;
					}
					char c[16]; memset(c,0,sizeof(c));
					strncpy(c, cFormat+1, end);
					nbr_len=atoi(c);
					cFormat += 2+end;
				}

				char cTempF[20]; cTempF[0]=0; sprintf(cTempF, "%%0%dd", nbr_len);
				sprintf(cDest, cTempF, iCurrentFile);
				cDest+=strlen(cDest);
			} else
			if (!_strnicmp(cFormat+1,"Chn",3)) {
				chn_name = 1;
				char cDesiredLng[16]; memset(cDesiredLng,0,sizeof(cDesiredLng));
				cFormat += 4;
				if (*cFormat == '{') {
					cFormat++;
					int end = (DWORD)strstr(cFormat, "}") - DWORD(cFormat);
					if (!end) {
						cFormat +=strlen(cFormat);
						return 0;
					}
					strncpy(cDesiredLng, cFormat, end);
					cFormat += 1+end;
				}
				char* cText = NULL;
				if (pSPD && pSPD->chapter && allow_chn) {
					CChapters* c = NULL; int i;
					c = pSPD->chapter->GetChapter(pSPD->aChapBegin, &i);

					cText = c->GetChapterText(i, cDesiredLng, 
						CHAP_GCT_RETURN_FIRST);
					if (cText)
						strcat(cDest, cText);
					cDest+=strlen(cDest);
				}
				
			} else
				return 0;
		}

	}

	*cDest++ = 0;

	while (*p)
		if (*p == '/' || *p == '?' || *p == '*') *p++ = '_'; else *p++;

	if (flags) {
		*flags &=~ FOFN_USE_CHAPTER_NAME;
		if (chn_name)
			*flags |= FOFN_USE_CHAPTER_NAME;
	}

	return 1;
}

int FormatSegmentTitle(char* cDest, char* cRawSegmentName, char* cRawFilename, 
					   char* cFinalFilename, int current_file_nbr)
{

	return 1;
}

int string_comp(const void* c1, const void* c2)
{
	return strcmp((const char*)c1, (const char*)c2);
}

int FirstFilenameCheck(DEST_AVI_INFO* lpDAI, char* RawFilename, int format,
					   __int64* pOverwriteableSize) 
{
	int check = CHECKEXISTENCE_ASKEACH;
	int flags = (format == FFC_FORMAT_AVI)?0:FOFN_CAN_USE_CHAPTER_NAME;
	int file_count = lpDAI->dwEstimatedNumberOfFiles;
	int double_count = 0;
	__int64 overwritable_size = 0;

	char** ppAll = NULL;

	/* if (manual split points are used and a maximum file size is set) and (format is not avi)
	*/
	if ((lpDAI->settings->GetInt("output/general/file size/max") * (1<<20)) < lpDAI->qwEstimatedSize && 
		lpDAI->split_points->GetCount()
		/*&& format != FFC_FORMAT_AVI*/) {
		char* cMsg = LoadString(STR_WRN_CHAPTERSANDMAXSIZE, LOADSTRING_UTF8);
		char* cTit = LoadString(IDS_WARNING, LOADSTRING_UTF8);

		int result = MessageBoxUTF8(0, cMsg, cTit, MB_YESNOCANCEL | MB_ICONWARNING | MB_DEFBUTTON2);
		if (result == IDYES)
			check = CHECKEXISTENCE_OVERWRITE;
		if (result == IDNO)
			check = CHECKEXISTENCE_ASKEACH;
		if (result == IDCANCEL)
			check = -1;
	} else {
		if (/*file_count > 1 || lpDAI->split_points->GetCount()*/1) {
			char cExF[1<<18]; memset(cExF, 0, sizeof(cExF));

			// if chapters 
			if (lpDAI->split_points->GetCount())
				file_count = lpDAI->split_points->GetCount()+1;
			
			int pos = 0;
			int len = 0;
			ppAll = (char**)calloc(file_count, sizeof(char*));

			for (int j = 1;j<=file_count; j++) {
				char cFN[4096]; memset(cFN, 0, sizeof(cFN));
				__int64 s;
				CUTF8 utf8FileNameFormat(lpDAI->fileNameFormat.c_str());
				FormatOutputFileName(cFN, utf8FileNameFormat.TStr() /*lpDAI->lpFormat*/, RawFilename, j, 
					lpDAI->split_points->At(j-1), &flags);
				len = strlen(cFN);
				if (CheckExistence(cFN, CHECKEXISTENCE_NEVERASK, &s) == CHECKEXISTENCE_OVERWRITE) {
					strcat(cExF, "  ");
					pos+=2;
					strcat(cExF+pos, cFN);
					pos += len;
					cExF[pos++] = 13;
					cExF[pos++] = 10;
		//			overwritable_size += s;
				}

				ppAll[j-1] = (char*)calloc(1+len, sizeof(char));
				strcpy(ppAll[j-1], cFN);
			}

			qsort(ppAll, file_count, sizeof(char*), string_comp);
			for (int j=0;j<file_count-1;j++) {
				if (!strcmp(ppAll[j], ppAll[j+1])) {
					double_count++;
					memmove(ppAll[j], ppAll[j+1], (sizeof(char*) * (file_count-j-1)));
					file_count--;
				}
			}

			for (int j=0;j<file_count;j++) {
				CFileStream* f = new CFileStream;
				if (f->Open(ppAll[j], StreamMode::Read) == STREAM_OK) {
					overwritable_size += f->GetSize();
					f->Close();
				}
				delete f;
			}

			if (pOverwriteableSize)
				*pOverwriteableSize = overwritable_size;

			if (double_count) {
				char* cMsg1 = LoadString(STR_WRN_NONUNIQUEFILENAMES, LOADSTRING_UTF8);
				char* cMsg2 = LoadString(IDS_WARNING, LOADSTRING_UTF8);
				CUTF8 utf8FileNameFormat(lpDAI->fileNameFormat.c_str());
				const char* cCfg = utf8FileNameFormat.TStr(); //lpDAI->lpFormat;
				char* cMsg = (char*)calloc(strlen(cMsg1)+strlen(cCfg)+100,sizeof(char));

				sprintf(cMsg, cMsg1, cCfg);

				int result = MessageBoxUTF8(0, cMsg, cMsg2, 
					MB_YESNO | MB_ICONWARNING | MB_DEFBUTTON2);
				if (result == IDNO)
					check = -1;
				if (result == IDYES)
					check = CHECKEXISTENCE_ASKEACH;

				delete cMsg;
			}

			if (cExF[0] && check != -1) {
				char* cMsg1 = LoadString(IDS_EXISTINGFILES, LOADSTRING_UTF8);
				char* cMsg2 = LoadString((file_count>1)?IDS_CONTINUE:STR_OVERWRITE, LOADSTRING_UTF8);
				char* cMsg3 = LoadString(STR_TOTAL_FILE_SIZE, LOADSTRING_UTF8);
				char cSize[64]; cSize[0]=0; FormatSize(cSize, overwritable_size);

				char* cMsg = (char*)calloc(1, 1024+strlen(cMsg1)+strlen(cMsg2)+strlen(cMsg3)+strlen(cSize)+pos);
				sprintf(cMsg, "%s%c%c%c%c%s %c%c%s: %s %c%c%c%c%s", cMsg1, 13, 10, 13, 10, cExF,
					13, 10, cMsg3, cSize,
					13, 10, 13, 10, cMsg2);

				int buttons = MB_ICONWARNING | MB_DEFBUTTON2;
				if (file_count > 1)
					buttons |= MB_YESNOCANCEL;
				else
					buttons |= MB_YESNO;

				int result = MessageBoxUTF8(0, cMsg, 
					LoadString(IDS_CONFIRMATION, LOADSTRING_UTF8),
					/*MB_YESNOCANCEL | MB_ICONWARNING | MB_DEFBUTTON2*/ buttons);

				if (result == IDYES)
					check = CHECKEXISTENCE_OVERWRITE;

				if (result == IDNO)
					if (file_count > 1)
						check = CHECKEXISTENCE_ASKEACH;
					else
						check = -1;

				if (result == IDCANCEL)
					check = -1;

				delete cMsg;
			}
		}
	}

	return check;
}


#define MKV_MUX_CONDITION (((v && !v->IsEndOfStream()) || (!v && !IsEndOfMMS(mms))) && !bStop) 

class  MATROSKA_TRACK_SOURCE {
private:
	MATROSKA*	matroska;
	int			track;
protected:
	int virtual Process(ADDBLOCK* a) { return 1; };
public:
	MATROSKA_TRACK_SOURCE();
	int	virtual Open(MATROSKA* m, int t, MULTIMEDIASOURCE* a);
	int	Put(ADDBLOCK* a);
};

MATROSKA_TRACK_SOURCE::MATROSKA_TRACK_SOURCE()
{
	matroska = NULL;
	track = -1;
}

int MATROSKA_TRACK_SOURCE::Open(MATROSKA* m, int t, MULTIMEDIASOURCE* a)
{
	matroska = m;
	track = t;

	return 1;
}

int MATROSKA_TRACK_SOURCE::Put(ADDBLOCK* a)
{
	ASSERT(matroska);

	Process(a);
	matroska->Write(a);

	return 1;
}

class MATROSKA_TRACK_SOURCE_HDRSTIP: public MATROSKA_TRACK_SOURCE {
private:
	int	byte_count;
	unsigned char bytes[64];
protected:
	int virtual Process(ADDBLOCK* a);
public:
	int virtual Open(MATROSKA* m, int t, MULTIMEDIASOURCE* a);
};

int MATROSKA_TRACK_SOURCE_HDRSTIP::Open(MATROSKA* m, int t, MULTIMEDIASOURCE* a)
{
	MATROSKA_TRACK_SOURCE::Open(m, t, a);
	byte_count = a->GetStrippableHeaderBytes(bytes, 64);

	if (byte_count < 0)
		byte_count = 0;

	if (byte_count) {
		m->AddTrackCompression(t, COMPRESSION_HDRSTRIPPING, bytes, byte_count);
	}

	return 1;
}

int MATROSKA_TRACK_SOURCE_HDRSTIP::Process(ADDBLOCK* a)
{
	if (byte_count) {
		if (a->iFrameCountInLace < 2) {
			a->cData->Cut(0, byte_count);
		} else {
			int pos = 0;
		
			for (int i=0;i<a->iFrameCountInLace;i++) {
				a->cData->Cut(pos, byte_count);
				a->iFrameSizes[i]-=byte_count;
				pos += a->iFrameSizes[i];
			}
		}
	}

	return 1;
}

MATROSKA_TRACK_SOURCE* GenerateMatroskaTrackSource(MATROSKA* m, 
												   int track, 
												   MULTIMEDIASOURCE* mms, 
												   bool bHeaderStriping)
{
	if (bHeaderStriping) {
		MATROSKA_TRACK_SOURCE_HDRSTIP* mtsh = new MATROSKA_TRACK_SOURCE_HDRSTIP;
		mtsh->Open(m, track, mms);
		return mtsh;
	} else {
		MATROSKA_TRACK_SOURCE* mts = new MATROSKA_TRACK_SOURCE;
		mts->Open(m, track, mms);
		return mts;
	}

	return NULL;
}



int MuxThread_MKV(DEST_AVI_INFO* lpDAI)
{
	std::map<unsigned int, std::wstring> laceStyleTexts;
	laceStyleTexts[LACESTYLE_XIPH] = L"xiph";
	laceStyleTexts[LACESTYLE_EBML] = L"ebml";
	laceStyleTexts[LACESTYLE_FIXED] = L"fixed";
	laceStyleTexts[LACESTYLE_AUTO] = L"auto";
	laceStyleTexts[0] = L"none";

	std::map<bool, std::wstring> enabledDisabledTexts;
	enabledDisabledTexts[true] = L"enabled";
	enabledDisabledTexts[false] = L"disabled";

	WaitForMuxing(lpDAI);

	CLocalTracer trace(GetApplicationTraceFile(), "MuxThread_MKV(...)");
	std::basic_string<TCHAR> muxSettingsTitleString = _T("Mux settings");

	CAttribs*   s = lpDAI->settings;
	MATROSKA*	m;
	MUX_STATE*	lpmsState;
	CFileStream* DestFile;
	VIDEOSOURCE* v = lpDAI->videosource;
	AUDIOSOURCE* a;
	SPLIT_POINT_DESCRIPTOR pSPD;
	char		cBuffer[65536];
	cBuffer[0] = 0;
	char		RawFilename[65536];
	char		cFilename[65536];
	int			i,j;
	__int64*	qwNanoSecNeeded=NULL;
	__int64*	qwNanoSecStored=NULL;

	__int64*	qwNanoSecStored_subs=NULL;

	__int64		qwVideoRead = 0;
	__int64		earliest_video_timecode;

	__int64		qwVideoReadTotal = 0;
	__int64		qwNanoSecRead;
	__int64		qwStats[10];
	__int64		qwBytesWritten = 0;
	__int64		qwMaxAudio;
	__int64		qwBias;
	__int64		qwMaxBytes = NULL;
	__int64		iLace_ns;
	__int64		iInputTimecode = 0;
	__int64		iDist2Splitpoint;
	__int64		iLastTimecode;
	int check = CHECKEXISTENCE_ASKEACH;
	int flags = 0;

	LACE_DESCRIPTOR*	audio_lace_config = NULL;

	__int64		i_vs_nspf;
	bool		bUnbufferedOutput = !!lpDAI->settings->GetInt("output/general/unbuffered");
	bool		bOverlapped = !!lpDAI->settings->GetInt("output/general/overlapped");
	bool		bThreaded = !!lpDAI->settings->GetInt("output/general/threaded");

	__int64		iProgress;
	int			total_codecprivate_size;
	int			iSourceLaces;
	int			dwHour,dwMin,dwSec,dwFrac;

	int			iCurrentFile = 1;
	int			iTrackCount;
	bool		bSplitFile = false;

	int			iFramecount;

	__int64		estimated_raw_stream_size = 0;
	__int64		estimated_overhead = 0;
	__int64		estimated_total_size = 0;
	__int64		free_disk_space = 0;
	__int64		overwriteable_size = 0;

	int	force_matroska_1 = (int)lpDAI->settings->GetInt("output/mkv/force v1");
	int	force_matroska_2 = (int)lpDAI->settings->GetInt("output/mkv/force v2");
	int matroska_version = (force_matroska_1)?1:2;
	int allow_a_aac = (int)lpDAI->settings->GetInt("output/mkv/use a_aac");
	CUTF8 utf8FileNameFormat(lpDAI->fileNameFormat.c_str());

	trace.Trace(TRACE_LEVEL_INFO, muxSettingsTitleString, CFormatHelper::FormatSimple<std::wstring>(
		L"Using file name pattern: %1", utf8FileNameFormat));

	char mkv_ver[128]; mkv_ver[0]=0;
	sprintf(mkv_ver, LoadString(STR_MUXLOG_MATROSKAVERSION), matroska_version);
	lpDAI->dlg->AddProtocolLine(mkv_ver, 4);

	trace.Trace(TRACE_LEVEL_INFO, muxSettingsTitleString, CFormatHelper::FormatSimple<int>(
		L"Creating matroska version %1 files", matroska_version));

	int free_style = !force_matroska_1 && !force_matroska_2;
	lpDAI->dlg->AddProtocolLine(Message_index(MSG_BENDSPECS, free_style), 4);
	
	if (force_matroska_1)
		lpDAI->settings->SetInt("output/mkv/lacing/video/on", 0);

	bool bLaceVideo = !!lpDAI->settings->GetInt("output/mkv/lacing/video/on");
	if ((int)s->GetInt("output/mkv/lacing/style") != LACESTYLE_AUTO) 
	{
		if (bLaceVideo) 
		{
			trace.Trace(TRACE_LEVEL_WARN, muxSettingsTitleString, _T("Can't use video lacing because lace setting is not 'auto'"));
		}
		bLaceVideo = false;
	}
	trace.Trace(TRACE_LEVEL_INFO, muxSettingsTitleString, CFormatHelper::FormatSimple(
		L"Video lacing is %1", enabledDisabledTexts[bLaceVideo]));
	
	int	iVideoFramesPerLace = (int)lpDAI->settings->GetInt("output/mkv/lacing/video/frames");

	int	hard_linking = (int)lpDAI->settings->GetInt("output/mkv/hard linking");
	trace.Trace(TRACE_LEVEL_INFO, muxSettingsTitleString, CFormatHelper::FormatSimple(
		L"Creating hard links is %1", enabledDisabledTexts[!!hard_linking]));

	bool header_stripping = !!lpDAI->settings->GetInt("output/mkv/compression/header striping");
	trace.Trace(TRACE_LEVEL_INFO, muxSettingsTitleString, CFormatHelper::FormatSimple(
		L"Header striping is %1", enabledDisabledTexts[header_stripping]));

	
	bool		bDoLace[100];
	char*		cCodecID = NULL;

	char		cSegmentFamily[16];
	char		cSegmentUID[16];
	char		cNextUID[16];
	char		cPrevUID[16];
	bool		bPrevUID = false;

	srand((int)rdtsc() & 0xFFFFFFFF);

	generate_uid(cSegmentFamily, 16);
	generate_uid(cSegmentUID, 16);


	lpDAI->dlg->m_Prg_Legidx_Label.ShowWindow(0);
	lpDAI->dlg->m_Prg_Legidx_Progress.ShowWindow(0);

	MULTIMEDIASOURCEVECTOR mms;
	MATROSKA_TRACK_SOURCE** mts = NULL;

	int			i_mms_count;
	__int64		iMaxDuration;

	MGO_DESCR	mgo;
	mgo.iFinalSize = 0;
	bMuxing = true;

	HANDLE hSem;

//	i_mms_count = lpDAI->dwNbrOfVideoStreams + lpDAI->dwNbrOfAudioStreams + lpDAI->dwNbrOfSubs;
//	mms = new MULTIMEDIASOURCE*[i_mms_count];

	GenerateMMSVector(lpDAI, mms);
	i_mms_count = mms.size();
	estimated_raw_stream_size = GetMMSVSize(mms);

	mts = new MATROSKA_TRACK_SOURCE*[i_mms_count];

	int ifloatlength = (int)lpDAI->settings->GetInt("output/mkv/floats/width");
	if (!free_style)
		ifloatlength = max(32,min(64,ifloatlength));
	else 
		ifloatlength = max(32,min(80,ifloatlength));
	
	sprintf(cBuffer, LoadString(STR_MUXLOG_FLOATWIDTH), ifloatlength);
	lpDAI->dlg->AddProtocolLine(cBuffer, 4);
	SetEBMLFloatMode(ifloatlength);

	i=0;

	if (lpDAI->dwNbrOfVideoStreams) {
		iMaxDuration = v->GetDurationUnscaled();
	} else {
		iMaxDuration = 0;
		for (j=0;j<i_mms_count;j++) {
			iMaxDuration = max(iMaxDuration, mms[j]->GetDurationUnscaled());
		}
	}

	CACHE_INFO cache_info;
	RetrieveCacheInfo(cache_info, s->GetAttr("output/general"));
	AddCacheInfoToLog(lpDAI, &cache_info);

	if (v) {
		v->SetStretchFactor(lpDAI->dVideoStretchFactor);
		i_vs_nspf = v->GetNanoSecPerFrame();
		v->ReInit();
	}

	for (i=0;i<(int)lpDAI->dwNbrOfAudioStreams;i++) {
		lpDAI->asi[i]->audiosource->ReInit();
		lpDAI->asi[i]->audiosource->SetBias((__int64)lpDAI->asi[i]->iDelay * 1000000, BIAS_UNSCALED);
	}

	/* retrieve cue write settings */
	int bCues = (int)lpDAI->settings->GetInt("output/mkv/cues/on");
	bool bCuesV = !!lpDAI->settings->GetInt("output/mkv/cues/video/on");
	int bCuesA = (int)lpDAI->settings->GetInt("output/mkv/cues/audio/on");
	bool bCuesS = !!lpDAI->settings->GetInt("output/mkv/cues/subs/on");
	int bCuesWriteBlockNumber = (int)lpDAI->settings->GetInt("output/mkv/cues/write blocknumber");
	int bCuesAOAO = (int)lpDAI->settings->GetInt("output/mkv/cues/audio/only audio-only/on");
	int hdr_size_set = (int)s->GetInt("output/mkv/headers/size");
	int cue_ratio = (int)lpDAI->settings->GetInt("output/mkv/cues/size ratio") * 1024;
	int cued_streams = ((v && bCuesV)?1:0) + (bCuesA && (!bCuesAOAO || !v)?lpDAI->dwNbrOfAudioStreams:0);
	double hours = iMaxDuration / 1000000000. / 3600.;
	int hdr_size;
	if (hdr_size_set) {
		hdr_size = hdr_size_set<<10; 
	} else {
		hdr_size = (int)(2048 + 512 * i_mms_count + lpDAI->chapters->GetSize(0) +
			(float)cue_ratio * (double)((double)cued_streams * hours));
	}

	hSem = CreateSemaphore(NULL, 1, 1, sem_name);
	if (v) {
		mgo.fFPS = (float)1000000000 / v->GetNanoSecPerFrame();
		mgo.iClusterSize = (int)s->GetInt("output/mkv/clusters/size");
		mgo.iClusterTime = (int)s->GetInt("output/mkv/clusters/time");
		mgo.iMatroskaVersion = matroska_version;
		mgo.iDuration = (int)(v->GetDuration() * v->GetTimecodeScale() / 1000000000);
		mgo.iFinalSize = lpDAI->videosource->GetSize()/1024;
		for (i=0;i<(int)lpDAI->dwNbrOfAudioStreams;i++) {
			mgo.iFinalSize += lpDAI->asi[i]->audiosource->GetSize()/1024;
		}
		mgo.iFlags = MGOF_CLUSTERS_BYSIZE | MGOF_CLUSTERS_BYTIME;
		if (hdr_size && !hdr_size_set)
			mgo.iFlags |= MGOF_NOCUEOVERHEAD;
		
		if (s->GetInt("output/mkv/clusters/prevclustersize")) 
			mgo.iFlags |= MGOF_CLUSTERS_PREVSIZE;
		if (s->GetInt("output/mkv/clusters/position")) 
			mgo.iFlags |= MGOF_CLUSTERS_POSITION;
		
		if (!(int)lpDAI->settings->GetInt("output/mkv/clusters/index/on")) {
			mgo.iFlags |= MGOF_CLUSTERS_NOINDEX;
		}

		mgo.iStreamCount = lpDAI->dwNbrOfAudioStreams;
		mgo.pStreams = (MGO_STREAMDESCR*)calloc(lpDAI->dwNbrOfAudioStreams,sizeof(MGO_STREAMDESCR));
		
		int key_frame_count = v->GetNbrOfFrames(FT_KEYFRAME);
		int frame_count     = v->GetNbrOfFrames();

		if (key_frame_count > 0 && frame_count > 0)
			mgo.iKeyframeInterval = frame_count / key_frame_count;
		else
			mgo.iKeyframeInterval = 0;

		if (mgo.iKeyframeInterval && mgo.iKeyframeInterval < 50)
			mgo.iKeyframeInterval *= (1+50/mgo.iKeyframeInterval);

		mgo.iKeyframeInterval = max(mgo.iKeyframeInterval, 50);

		for (i=0;i<(int)lpDAI->dwNbrOfAudioStreams;i++) {
			mgo.pStreams[i].fDurPerLace = (float)s->GetInt("output/mkv/lacing/length");
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
				if (!mgo.pStreams[i].iFramesize) 
					mgo.pStreams[i].iFramesize = 500;
				mgo.pStreams[i].fFrameDuration = 24;
				mgo.pStreams[i].iFlags |=  MGOSF_FRAMEDUR_IND | MGOSF_FRAMECOUNT_IND;
				mgo.pStreams[i].fDurPerLace = (float)GetLaceSetting("mp3",s)/1000000;
				mgo.pStreams[i].fFrameCountPerLace = (float)GetLaceSetting("mp3",s) / mgo.pStreams[i].fFrameDuration;
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
		estimated_overhead = matroska_guess_overhead(&mgo)+2048+256*(mgo.iStreamCount+1);
		if (mgo.iFlags & MGOF_NOCUEOVERHEAD)
			estimated_overhead += hdr_size;
		delete mgo.pStreams;
	}

	char cSize[30];

	estimated_total_size = estimated_overhead + estimated_raw_stream_size;


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

	for (i=0;i<(int)lpDAI->dwNbrOfSubs;i++) {
		lpDAI->ssi[i]->lpsubs->ReInit();
	}

	lpDAI->dlg->AddProtocolLine(LoadString(STR_MUXLOG_BEGIN),5);
//	MATROSKA_WriteBlockSizes(!lpDAI->mkv.iDontWriteBlockSizes);

	iLace_ns = s->GetInt("output/mkv/lacing/length") * 1000000;

	int iError = 0;
	int resolve_split_points_result;
	if ((resolve_split_points_result=ResolveSplitpoints(lpDAI, RSPF_CHAPTERS, &iError)) == RSP_ERROR) {
		sprintf(cBuffer,LoadString(STR_MUXLOG_SPLITPOINTB0RKED),iError+1);
		lpDAI->dlg->MessageBox(cBuffer,LoadString(STR_GEN_ERROR),MB_OK | MB_ICONERROR);
		lpDAI->dlg->SetDialogState_Config();
		lpDAI->dlg->ButtonState_STOP();
		lpDAI->chapters->SetBias(0,BIAS_UNSCALED);
//		cacheAllowReadAhead(0);
		lpDPI->dwLeave = 1;
		bStop = 1;
		goto start;
	}

	memset(&pSPD, 0, sizeof(pSPD));
	if (lpDAI->split_points->At(0))
		memcpy(&pSPD, lpDAI->split_points->At(0), sizeof(SPLIT_POINT_DESCRIPTOR));

	if (pSPD.iBegin > 0) 
		pSPD.iBegin = NULL;

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
		//lpDAI->lpFormat[strlen(lpDAI->lpFormat)-1]='a';
		lpDAI->fileNameFormat[lpDAI->fileNameFormat.size() - 1] = _T('a');
	}

	flags |= FOFN_CAN_USE_CHAPTER_NAME;
	cBuffer[0]=0; memset(cBuffer, 0, sizeof(cBuffer));
	FormatOutputFileName(cBuffer, /*lpDAI->lpFormat*/utf8FileNameFormat.TStr(), RawFilename, iCurrentFile,
		&pSPD, &flags);

	qwMaxBytes = (s->GetInt("output/general/file size/max"))*(1<<20);

	if (qwMaxBytes > 0)
		lpDAI->dwEstimatedNumberOfFiles = (DWORD)(estimated_total_size / qwMaxBytes + 1);

	if (lpDAI->split_points && lpDAI->split_points->GetCount())
		lpDAI->dwEstimatedNumberOfFiles = max(lpDAI->split_points->GetCount() + 1, lpDAI->dwEstimatedNumberOfFiles);

	check = CHECKEXISTENCE_OVERWRITE;
	if (lpDAI->settings->GetInt("gui/general/overwritedlg")) {
		check = FirstFilenameCheck(lpDAI, RawFilename, FFC_FORMAT_MKV, &overwriteable_size);
		if (check == -1)
			bStop = 1;
	}

	int space_method = 1;

	AddSizeStringToLog(lpDAI, LoadString(STR_MUXLOG_ESTIMATEDRAWSIZE, LOADSTRING_UTF8),
		space_method, estimated_raw_stream_size);

	AddSizeStringToLog(lpDAI, LoadString(STR_MUXLOG_ESTIMATEDOVERHEAD, LOADSTRING_UTF8),
		space_method, estimated_overhead);

	AddSizeStringToLog(lpDAI, LoadString(STR_MUXLOG_ESTIMATEDTOTALSIZE, LOADSTRING_UTF8),
		space_method, estimated_total_size);
	
	AddSizeStringToLog(lpDAI, LoadString(STR_MUXLOG_OVERWRITEABLE, LOADSTRING_UTF8),
		space_method, overwriteable_size);

	free_disk_space = GetDirectoryFreeSpace(RawFilename);
	AddSizeStringToLog(lpDAI, LoadString(STR_MUXLOG_FREEDISKSPACE, LOADSTRING_UTF8),
		space_method, free_disk_space);

	bool bCheckDiskSpace = !!s->GetInt("output/general/check disk space/enabled");
	__int64 iDiscSpaceTolerance = s->GetInt("output/general/check disk space/lower limit") * (1<<20);

	if (!CheckDiskSpace(bCheckDiskSpace, estimated_total_size, free_disk_space,
		iDiscSpaceTolerance, overwriteable_size)) {
			bStop = true;
	} else
	if (!VerifySuitableFileSystem(RawFilename, min(estimated_total_size, qwMaxBytes))) {
		bStop = true;
	}
	
	lpDAI->dlg->AddProtocolLine(Message_index(MSG_UNBUFFERED, bUnbufferedOutput), 4);
	lpDAI->dlg->AddProtocolLine(Message_index(MSG_OVERLAPPEDOUT, bOverlapped), 4);

	RemoveZeroSplitpoints(lpDAI->split_points);

start:

	while MKV_MUX_CONDITION { //(( (v && !v->IsEndOfStream()) || (!v && !IsEndOfMMS(mms, i_mms_count)) ) && !bStop) {
		WaitForSingleObject(hSem, INFINITE);
		if (!hard_linking) {
			ZeroMemory(qwNanoSecNeeded,8*lpDAI->dwNbrOfAudioStreams);
			ZeroMemory(qwNanoSecStored,8*lpDAI->dwNbrOfAudioStreams);
		}
		
		cBuffer[0]=0; memset(cBuffer, 0, sizeof(cBuffer));
		FormatOutputFileName(cBuffer, /*lpDAI->lpFormat*/utf8FileNameFormat.TStr(), RawFilename, iCurrentFile++,
			&pSPD, &flags);
		
		if (check != CHECKEXISTENCE_OVERWRITE)
			if (CheckExistence(cBuffer, check, NULL) == CHECKEXISTENCE_STOP) {
				bStop = true;
				goto start;
			} 

		// delete output file if it already exists
		DeleteFileUTF8(cBuffer);
		DestFile=new CFileStream;
		if (DestFile->Open(cBuffer, StreamMode::Read | 
			(bUnbufferedOutput?StreamMode::UnbufferedWrite:StreamMode::Write) | 
			(bOverlapped?StreamMode::Overlapped:StreamMode::None) |
			(bThreaded?StreamMode::Threaded:StreamMode::None))==STREAM_ERR) {
			lpDAI->dlg->MessageBox(LoadString(IDS_COULDNOTOPENOUTPUTFILE),lpDAI->dlg->cstrError,MB_OK | MB_ICONERROR);
			lpDAI->dlg->ButtonState_STOP();
			lpDAI->dlg->SetDialogState_Config();

			bStop = true;
			goto start;
		}
		lpDAI->dlg->SetDlgItemText(IDC_DESTFILE,cBuffer);
		lstrcpy(cFilename,cBuffer);
		trace.Trace(TRACE_LEVEL_INFO, _T("Muxing"), CFormatHelper::FormatSimple(
			L"New file created: %1", cFilename));
		m = new MATROSKA;
		CACHE* cache;

		if (bUnbufferedOutput || bOverlapped || cache_info.enabled) {
			cache = new CACHE(cache_info.cache_lines, cache_info.cache_line_size);
 			cache->Open(DestFile, CACHE_OPEN_WRITE | CACHE_OPEN_READ | CACHE_OPEN_ATTACH);
			cache->Enable(CACHE_IMMED_WRITEBACK);
		} else {
			cache = (CACHE*)DestFile;
		}
		m->Open(cache,MMODE_WRITE);
		if (v) {
			__int64 j = lpDAI->settings->GetInt("output/mkv/TimecodeScale/mkv");
			m->SetTimecodeScale(j);
			v->SetTimecodeScale(j);
			trace.Trace(TRACE_LEVEL_INFO, muxSettingsTitleString, CFormatHelper::FormatSimple(
				L"Timecode scale: %1", j));
		} else {
			__int64 timecodeScale = lpDAI->settings->GetInt("output/mkv/TimecodeScale/mka");
			m->SetTimecodeScale(timecodeScale);
			trace.Trace(TRACE_LEVEL_INFO, muxSettingsTitleString, CFormatHelper::FormatSimple(
				L"Timecode scale: %1", timecodeScale));
		}

		Millisec2HMSF(-qwBias/1000000,&dwHour,&dwMin,&dwSec,&dwFrac);
		sprintf(cBuffer,LoadString(STR_MUXLOG_NEWFILE),dwHour,dwMin,dwSec,dwFrac,dwFrameCountTotal,cFilename);
		lpDAI->dlg->AddProtocolLine(cBuffer,5,APL_UTF8);

		QW2Str(m->GetTimecodeScale(), cSize, 1);
		sprintf(cBuffer, LoadString(STR_MUXLOG_OUTPUTTIMECODESCALE),cSize);
		lpDAI->dlg->AddProtocolLine(cBuffer,4);

		__int64 min_cue_int = lpDAI->settings->GetInt("output/mkv/cues/minimum interval");
		min_cue_int = min(60000, max(500, min_cue_int));
		m->SetMinimumCueInterval((min_cue_int-1) * 1000000);

		m->SetMaxClusterSize((int)s->GetInt("output/mkv/clusters/size"));
		m->SetMaxClusterTime((int)s->GetInt("output/mkv/clusters/time"), (int)s->GetInt("output/mkv/clusters/limit first"));//lpDAI->mkv.iLimit1stCluster);
		
//		m->EnableClusterPosition((int)s->GetInt("output/mkv/clusters/position"));
		m->Enable(MF_WRITE_CLUSTER_POSITION, 0,
			s->GetInt("output/mkv/clusters/position")?MFA_ENABLE:MFA_DISABLE);

		//m->EnablePrevClusterSize((int)s->GetInt("output/mkv/clusters/prevclustersize"));
		bool writePrevClusterSize = !!s->GetInt("output/mkv/clusters/prevclustersize");
		m->Enable(MF_WRITE_PREV_CLUSTER_SIZE, 0,
			writePrevClusterSize ? MFA_ENABLE : MFA_DISABLE);
		trace.Trace(TRACE_LEVEL_INFO, muxSettingsTitleString, CFormatHelper::FormatSimple(
			L"Writing size of previous cluster into clusters is %1", enabledDisabledTexts[writePrevClusterSize]));

		m->SetTrackCount(iTrackCount=lpDAI->dwNbrOfAudioStreams+lpDAI->dwNbrOfVideoStreams+lpDAI->dwNbrOfSubs);
		m->SetSegmentDuration((float)iMaxDuration / m->GetTimecodeScale());
		
		
		m->EnableRandomizeElementOrder(j=(int)s->GetInt("output/mkv/randomize element order"));
		

		m->SetNonclusterIndexMode((int)s->GetInt("output/mkv/headers/index in first seekhead"));
		
		lpDAI->dlg->AddProtocolLine(LoadString(MSG_RANDELORDER[j]), 4);

		if (lpDAI->settings->GetInt("output/mkv/2nd Tracks"))
			m->SetTracksCopiesCount(2);
	
		// set cue creation scheme
		m->EnableCues(CUE_VIDEO | CUE_AUDIO, 0);
//		m->EnableCueBlockNumber(bCuesWriteBlockNumber);
		m->Enable(MF_WRITE_CUE_BLOCK_NUMBER, 0,
			bCuesWriteBlockNumber?MFA_ENABLE:MFA_DISABLE);

		if (bCues) {
			trace.Trace(TRACE_LEVEL_INFO, muxSettingsTitleString, CFormatHelper::FormatSimple(
				L"Cues for video streams are %1", enabledDisabledTexts[bCuesV]));
			if (bCuesV) 
				m->EnableCues(CUE_VIDEO, 1);

			trace.Trace(TRACE_LEVEL_INFO, muxSettingsTitleString, CFormatHelper::FormatSimple(
				L"Cues for subtitle streams are %1", enabledDisabledTexts[bCuesS]));
			if (bCuesS)
				m->EnableCues(CUE_SUBS, 1);

			if (bCuesA) {
				bool setToEnabled = false;
				if (bCuesAOAO && !lpDAI->dwNbrOfVideoStreams) {
					m->EnableCues(CUE_AUDIO, 1);
					setToEnabled = true;
				}
				if (!bCuesAOAO) {
					m->EnableCues(CUE_AUDIO, 1);
					setToEnabled = true;
				}
				trace.Trace(TRACE_LEVEL_INFO, muxSettingsTitleString, CFormatHelper::FormatSimple(
					L"Cues for audio streams are %1", enabledDisabledTexts[setToEnabled]));
			}
		}

		lpDAI->dlg->AddProtocolLine(Message_index(MSG_CUEVIDEO, m->IsCuesEnabled(CUE_VIDEO)), 4);
		lpDAI->dlg->AddProtocolLine(Message_index(MSG_CUEAUDIO, m->IsCuesEnabled(CUE_AUDIO)), 4);
		lpDAI->dlg->AddProtocolLine(Message_index(MSG_CUESUBS, m->IsCuesEnabled(CUE_SUBS)), 4);
		lpDAI->dlg->AddProtocolLine(Message_index(MSG_WRITECUEBLKNBR, 
			/*m->IsCueBlockNumberEnabled()*/
			m->Enable(MF_WRITE_CUE_BLOCK_NUMBER, 0, MFA_RETRIEVE_ONLY)), 4);
		int hdr_size;

		if (hdr_size_set) {
			hdr_size = hdr_size_set<<10; 
			//m->EnableClusterIndex((int)s->GetInt("output/mkv/clusters/index/on"));
			m->Enable(MF_WRITE_CLUSTER_INDEX, 0,
				(int)s->GetInt("output/mkv/clusters/index/on")?MFA_ENABLE:MFA_DISABLE);
		}
		else {

			SPLIT_POINT_DESCRIPTOR* d = lpDAI->split_points->At(0);
			iDist2Splitpoint = INT64_MAX;
			if (lpDAI->split_points->GetCount() && d) {
				iDist2Splitpoint = d->iBegin - iInputTimecode;
			}

			__int64 duration = 0;
			if (iDist2Splitpoint != INT64_MAX)
				duration = iDist2Splitpoint;

			if (v)
				duration = min(v->GetDurationUnscaled() - iInputTimecode, iDist2Splitpoint);
			else 
				if (iMaxDuration)
					duration = min(iMaxDuration, iDist2Splitpoint);
				else
					duration = 3600000000000;

			//m->EnableClusterIndex(0);
			m->Enable(MF_WRITE_CLUSTER_INDEX, 0, MFA_DISABLE);

			double hours = 0.;

			hours = (double)duration / 1000000000. / 3600.;

			int streams = (!!(v && m->IsCuesEnabled(CUE_VIDEO))) + (!!m->IsCuesEnabled(CUE_AUDIO)) * lpDAI->dwNbrOfAudioStreams;

			hdr_size = (int)(2048 + 512 * i_mms_count + lpDAI->chapters->GetSize(0) +
				(float)cue_ratio * (double)((double)streams * hours));
		}
		lpDAI->dlg->AddProtocolLine(Message_index(MSG_CLUSTERINDEX, 
			/*m->IsClusterIndexEnabled()*/
			m->Enable(MF_WRITE_CLUSTER_INDEX, 0, MFA_RETRIEVE_ONLY)), 4);

		total_codecprivate_size = 0;
		// set video attributes
		if (lpDAI->dwNbrOfVideoStreams) {
			int x, y;
			RESOLUTION r2;
			v->GetResolution(&x,&y);
			v->GetOutputResolution(&r2);
			m->SetResolution(0,x,y,r2.iWidth,r2.iHeight);
			m->SetCropping(0, &r2.rcCrop);
			m->SetFlags(0, 1, bLaceVideo, !!v->IsDefault());
			m->SetTrackNumber(0,1);
			m->SetTrackType(0,MSTRT_VIDEO);
			if (!v->GetCodecID()) {
				m->SetCodecID(0,"V_MS/VFW/FOURCC");
				m->SetCodecPrivate(0,v->GetFormat(),((BITMAPINFOHEADER*)v->GetFormat())->biSize);
				total_codecprivate_size += ((BITMAPINFOHEADER*)v->GetFormat())->biSize;
			} else {
				m->SetCodecID(0,v->GetCodecID());
				if (v->GetFormat()) {
					m->SetCodecPrivate(0,v->GetFormat(),v->GetFormatSize());
				}
			}
			m->EnableDisplayWidth_Height((int)s->GetInt("output/mkv/displaywidth_height"));
			if (v->IsCFR()) m->SetDefaultDuration(0,i_vs_nspf);
			m->SetCacheData(0,1,0);
		//	m->SetTrackLanguageCode(0,"und");
			char cName[512]; memset(cName, 0, sizeof(cName));
			v->GetName(cName);
			//m->SetTrackName(0, cName);
			m->GetTrackTitleSet()->Import(v->GetTitleSet());
			
			std::string languageCode;
			v->GetLanguageCode(languageCode); // cName
			strcpy(cName, languageCode.c_str());
			m->SetTrackLanguageCode(0, cName);
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
			char*           ids = lpDAI->asi[j]->audiosource->GetCodecID();
			m->SetTrackNumber(i,i+1);
			m->SetTrackType(i,MSTRT_AUDIO);

			m->SetBitDepth(i,as->GetBitDepth());
			m->SetSamplingFrequency(i,(float)as->GetFrequency(),(float)as->GetOutputFrequency());
			m->SetChannelCount(i,as->GetChannelCount());
			char cName[500]; cName[0]=0;
			
			//char cLangCode[10];
			std::string languageCode;
			//lpDAI->asi[j]->audiosource->GetName(cName);
			lpDAI->asi[j]->audiosource->GetLanguageCode(languageCode); // cLangCode
			//m->SetTrackName(i,cName);
			m->GetTrackTitleSet(i)->Import(lpDAI->asi[j]->audiosource->GetTitleSet());
			
			switch (lpDAI->asi[j]->audiosource->GetFormatTag()) {
				case 0x2000:
					lpDAI->asi[j]->audiosource->SetFrameMode((int)lpDAI->settings->GetInt("output/mkv/ac3/frames per block"));
					break;
				default:
					lpDAI->asi[j]->audiosource->SetFrameMode(FRAMEMODE_SINGLEFRAMES);
					break;
			}

			//lpDAI->asi[j]->audiosource->GetLanguageCode(cBuffer);
			lpDAI->asi[j]->audiosource->GetLanguageCode(languageCode);

			m->SetTrackLanguageCode(i, (char*)languageCode.c_str()); // cBuffer);
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
							case 1: m->SetCodecID(i, "A_MPEG/L1"); break;
							case 2: m->SetCodecID(i, "A_MPEG/L2"); break;
							case 3: m->SetCodecID(i, "A_MPEG/L3"); break;
						} 
						audio_lace_config[j].iLength = GetLaceSetting("mp3", s);						
						break;
					case 0x00FF: 
						cCodecID = new char[128];
						ZeroMemory(cCodecID, 128);

						if (allow_a_aac) {
							strcpy(cCodecID, "A_AAC");
							WAVEFORMATEX* p = (WAVEFORMATEX*)lpDAI->asi[j]->lpFormat;
							
							m->SetCodecPrivate(i, p+1, p->cbSize);
						} else {
							strcpy(cCodecID,"A_AAC/");
							a = lpDAI->asi[j]->audiosource;
							switch (a->FormatSpecific(MMSGFS_AAC_MPEGVERSION)) {
								case 2: strcat(cCodecID, "MPEG2/"); break;
								case 4: strcat(cCodecID, "MPEG4/"); break;
							}
							switch (a->FormatSpecific(MMSGFS_AAC_PROFILE)) {
								case AACSOURCE::AdtsProfile::LC: strcat(cCodecID, "LC"); break;
								case AACSOURCE::AdtsProfile::LTP: strcat(cCodecID, "LTP"); break;
								case AACSOURCE::AdtsProfile::Main: strcat(cCodecID, "MAIN"); break;
								case AACSOURCE::AdtsProfile::SSR: strcat(cCodecID, "SSR"); break;
							}
							if (a->FormatSpecific(MMSGFS_AAC_ISSBR)) {
								strcat(cCodecID, "/SBR");
							}
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
				m->SetCodecID(i,as->GetCodecID());
				__int64 iDur = lpDAI->asi[j]->audiosource->GetFrameDuration();
				if (iDur > 0) m->SetDefaultDuration(i,iDur);
				if (lpDAI->asi[j]->lpFormat) {
					m->SetCodecPrivate(i,w,lpDAI->asi[j]->iFormatSize);
				}

				if (!strcmp(as->GetCodecID(),"A_AC3")) {
					audio_lace_config[j].iLength = GetLaceSetting("ac3",s);
				} else 
				if (!strcmp(as->GetCodecID(),"A_DTS")) {
					audio_lace_config[j].iLength = GetLaceSetting("dts",s);
				} else
				if (!strncmp(as->GetCodecID(),"A_MPEG/L",8)) {
					audio_lace_config[j].iLength = GetLaceSetting("mp3",s);
				} else
				if (!strncmp(as->GetCodecID(),"A_AAC",5)) {
					audio_lace_config[j].iLength = GetLaceSetting("aac",s);
				} else
				if (!strncmp(as->GetCodecID(),"A_VORBIS",8)) {
					audio_lace_config[j].iLength = GetLaceSetting("vorbis",s);
				} else
					audio_lace_config[j].iLength = GetLaceSetting("general",s);
			}
			if (audio_lace_config[j].iLength == 0) {
				bDoLace[j] = 0;
			}
			m->SetFlags(i,1,bDoLace[j],as->IsDefault());
			lpDAI->asi[j]->audiosource->GetName(cName);

			if ((int)s->GetInt("output/mkv/lacing/style") || cName[0] || !languageCode.empty() || lpDAI->asi[j]->iDelay) {
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
			if (!languageCode.empty()) {
				wsprintf(cBuffer,LoadString(STR_MUXLOG_LANGUAGECODE),languageCode.c_str());
				lpDAI->dlg->AddProtocolLine(cBuffer,4, APL_UTF8);
			}
			if (lpDAI->asi[j]->iDelay) {
				wsprintf(cBuffer,LoadString(STR_MUXLOG_DELAY),lpDAI->asi[j]->iDelay);
				lpDAI->dlg->AddProtocolLine(cBuffer,4);
			}

			total_codecprivate_size += m->GetCodecPrivateSize(i);
			
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

			char cName[500]; cName[0]=0;
			//char cLangCode[10];
			std::string languageCode;
		//	subs->GetName(cName);
			subs->GetLanguageCode(languageCode); // cLangCode
			if (cName[0] || !languageCode.empty()) {
				wsprintf(cBuffer,LoadString(STR_MUXLOG_STREAM),i+1);
				lpDAI->dlg->AddProtocolLine(cBuffer,5);

				if (cName[0]) {
					sprintf(cBuffer,LoadString(STR_MUXLOG_NAME),cName);
					lpDAI->dlg->AddProtocolLine(cBuffer,5, APL_UTF8);
				}
				if (!languageCode.empty()) {
					sprintf(cBuffer,LoadString(STR_MUXLOG_LANGUAGECODE),languageCode.c_str());
					lpDAI->dlg->AddProtocolLine(cBuffer,5, APL_UTF8);
				}

			}
			//m->SetTrackName(j,cName);
			m->GetTrackTitleSet(j)->Import(subs->GetTitleSet());

//			subs->GetLanguageCode(cBuffer);
			m->SetTrackLanguageCode(j, (char*)languageCode.c_str());
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
			if (subs->GetCompressionAlgo() == COMPRESSION_ZLIB) {
				m->AddTrackCompression(j, COMPRESSION_ZLIB, NULL, 0);
				sprintf(cBuffer, LoadString(STR_MUXLOG_COMPRESSION_TYPE), "zlib");
			} else if (subs->GetCompressionAlgo() == COMPRESSION_NONE) {
				sprintf(cBuffer, LoadString(STR_MUXLOG_COMPRESSION_TYPE), "none");
			} else sprintf(cBuffer, LoadString(STR_MUXLOG_COMPRESSION_TYPE), "unknown");

			lpDAI->dlg->AddProtocolLine(cBuffer, 4);

			total_codecprivate_size += m->GetCodecPrivateSize(j);
		}
		m->SetInitialHeaderSize(hdr_size + total_codecprivate_size);
		{
			char cSize[20];
			QW2Str(hdr_size, cSize, 1);
			sprintf(cBuffer, LoadString(STR_MUXLOG_HEADERSIZE), cSize);
			lpDAI->dlg->AddProtocolLine(cBuffer, 4);
		}

		m->EnableCueAutosize((int)lpDAI->settings->GetInt("output/mkv/cues/autosize"));

		if (m->IsCueAutoSizeEnabled()) {
			m->SetCueTargetSizeRatio((double)s->GetInt("output/mkv/cues/target size ratio") / 1000.);
			double d = m->GetCueTargetSizeRatio() * 100.;
			char cMsg[128]; cMsg[0]=0;
			sprintf(cMsg, LoadString(STR_MUXLOG_CUETARGETSIZERATIO), d);
			lpDAI->dlg->AddProtocolLine(cMsg, 4);
		}

		/* generate matroska track sources */
		if (v) {
			MATROSKA_TRACK_SOURCE* source = GenerateMatroskaTrackSource(m,
				GetStreamNumber(lpDAI, STREAMTYPE_VIDEO, 0), v, header_stripping);
			mts[GetStreamNumber(lpDAI, STREAMTYPE_VIDEO, 0)] = source;
		}
		for (i=0;i<(int)lpDAI->dwNbrOfAudioStreams;i++) {
			MATROSKA_TRACK_SOURCE* source = GenerateMatroskaTrackSource(m,
				GetStreamNumber(lpDAI, STREAMTYPE_AUDIO, i), lpDAI->asi[i]->audiosource, 
				header_stripping);
			mts[GetStreamNumber(lpDAI, STREAMTYPE_AUDIO, i)] = source;
		}

		if (hard_linking) {
			m->SetUID(UIDTYPE_SEGMENTFAMILY, cSegmentFamily);
			if (bPrevUID)
				m->SetUID(UIDTYPE_PREVUID, cPrevUID);
		}
		m->SetUID(UIDTYPE_SEGMENTUID, cSegmentUID);
		m->SetMatroskaVersion(matroska_version);

		m->BeginWrite();
		{
			std::basic_string<TCHAR> version = ComposeVersionString();
			m->SetAppName(version.c_str());
		}

		iFramecount = 0;
		ReleaseSemaphore(hSem, 1, NULL);
		iLastTimecode = 0;
		earliest_video_timecode = (__int64)INT_MAX * INT_MAX;
		__int64 last_frame_timecode = -25346;

		while (((v && !v->IsEndOfStream()) || (!v && !IsEndOfMMS(mms))) && (!bSplitFile) && (!bStop)) {
			ADDBLOCK	a;
			ADVANCEDREAD_INFO aari;

			ZeroMemory(&a,sizeof(a));
			ZeroMemory(&aari,sizeof(aari));

		// get video frame
			WaitForSingleObject(hSem, INFINITE);
			if (lpDAI->dwNbrOfVideoStreams) {
				if (!bLaceVideo) {
					v->GetFrame(buffer,(DWORD*)&iRead,&a.iTimecode,&aari);
					if (iRead == 0) {
						Sleep(1);
					}
					v->GetLatestReference(&a.iRefCount,&a.iReferences[0],&a.iReferences[1]);
					a.iReferences[0] = (__int64)a.iReferences[0];
					a.iReferences[1] = (__int64)a.iReferences[1];

					if (a.iRefCount>0) {
						if (!a.iReferences[0])
							a.iReferences[0] = last_frame_timecode - a.iTimecode;
						if (a.iRefCount>1)
							if (!a.iReferences[1])
								a.iRefCount = 1;
					}

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
					last_frame_timecode = a.iTimecode;
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

					earliest_video_timecode = min(earliest_video_timecode, a.iTimecode);

					a.iFlags |= ABTC_UNSCALED;

					//m->Write(&a);
					mts[GetStreamNumber(lpDAI, STREAMTYPE_VIDEO, 0)]->Put(&a);

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
				qwVideoReadTotal += 100000000;//*m->GetTimecodeScale();
				qwVideoRead = qwVideoReadTotal + qwBias;
			}

			iInputTimecode = qwVideoRead - qwBias;

			bool bSizeLimit = qwBytesWritten > s->GetInt("output/general/file size/max")*(1<<20);

			bool bSplitpoint = false;
			iDist2Splitpoint = INT64_MAX;
			__int64 iDist2End;
			if (lpDAI->dwNbrOfVideoStreams) {
				iDist2End = v->GetDurationUnscaled() - qwVideoReadTotal;
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

					if (iMSD > iDist2Splitpoint/1000) {
						if (iDist2Splitpoint > 0)
							iMSD = (__int64)(iDist2Splitpoint/1000);
						else
							iMSD = 50000;
					}

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

							mts[GetStreamNumber(lpDAI, STREAMTYPE_AUDIO, i)]->Put(&a);

							m->Write(&a);
							DecBufferRefCount(&a.cData);
							if (a.iFrameSizes) delete[] a.iFrameSizes;
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

								delete[] aari.iFramesizes;

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
						//	m->Write(&a);
							mts[GetStreamNumber(lpDAI, STREAMTYPE_AUDIO, i)]->Put(&a);

							DecBufferRefCount(&a.cData);
							if (a.iFrameSizes) delete[] a.iFrameSizes;
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
				if (iNextTC != TIMECODE_UNKNOWN) 
					iNextTC = iNextTC * subs->GetTimecodeScale() + qwBias;
				

				if (iNextTC != TIMECODE_UNKNOWN && qwVideoRead > iNextTC) {
					int j = lpDAI->dwNbrOfVideoStreams + lpDAI->dwNbrOfAudioStreams + i;
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
						DecBufferRefCount(&a.cData);
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
				if (bSizeLimit || bSplitpoint || IsEndOfMMS(mms)) {
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
		
			if (v) {
				m->SetDefaultDuration(0,(qwVideoRead-earliest_video_timecode)/iFramecount);
				if (!hard_linking)
					m->SetChapters(lpDAI->chapters, qwVideoRead);
				else
					m->SetChapters(lpDAI->chapters, -2);
			} else 
				m->SetChapters(lpDAI->chapters);
			

			
			CLUSTER_STATS clstats;
			m->GetClusterStats(&clstats);
			char cSize1[20];
			char cSize2[20];
			char cSize3[20];
			QW2Str(clstats.iCount, cSize1, 1);
			QW2Str(clstats.iOverhead, cSize2, 1);
			sprintf(cBuffer, LoadString(STR_MUXLOG_CLUSTERS),cSize1,cSize2);
			lpDAI->dlg->AddProtocolLine(cBuffer,4);
			

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
						QW2Str(stats[i].iCount,cSize1,1);
						QW2Str(stats[i].iTotalHdrSize,cSize2,1);
						QW2Str(stats[i].iFrameCount,cSize3,1);
						sprintf(cBuffer,LoadString(STR_MUXLOG_LACESTATS),cLaceSchemes[i],cSize1,cSize3,cSize2);
						lpDAI->dlg->AddProtocolLine(cBuffer,4);
					}
				}
			}
			
			generate_uid(cNextUID, 16);
			if (hard_linking)
				if MKV_MUX_CONDITION 
					m->SetUID(UIDTYPE_NEXTUID, cNextUID);
			m->Close();
			
			QW2Str(m->GetCueCount(1), cSize1, 1);
			QW2Str(m->GetCueCount(2), cSize2, 1);

			sprintf(cBuffer,LoadString(STR_MUXLOG_CUES),cSize1, cSize2);
			lpDAI->dlg->AddProtocolLine(cBuffer,4);
			
			memcpy(cPrevUID, cSegmentUID, 16);
			bPrevUID = true;
			memcpy(cSegmentUID, cNextUID, 16);

			if (!hard_linking)
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

			if ((v && !v->IsEndOfStream()) || (!v && !IsEndOfMMS(mms))) {
				qwStats[0] = 0;
				qwStats[1] = 0;
				qwStats[2] = 0;
				qwStats[3] = 0;
				qwStats[8] = 0;
			}

			bSplitFile = false;
			if (lpDAI->split_points->GetCount() && lpDAI->split_points->At(0)->iBegin - iInputTimecode <= 0) {
				memcpy(&pSPD, lpDAI->split_points->At(0), sizeof(pSPD));
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
	ReleaseSemaphore(hSem, 1, NULL);
	WaitForSingleObject(hSem_finish, INFINITE);

	CloseHandle(hSem_finish);
//	delete lpDAI->lpFormat;
	lpDAI->split_points->DeleteAll();
	delete lpDAI->split_points;
	delete lpDPI;

	if (s->GetInt("gui/general/finished_muxing_dialog"))
		lpDAI->dlg->MessageBox(LoadString(IDS_READY),lpDAI->dlg->cstrInformation,MB_OK | MB_ICONINFORMATION);

	lpDAI->dlg->SetDialogState_Config();
	lpDAI->dlg->ButtonState_STOP();
	lpDAI->chapters->SetBias(0,BIAS_UNSCALED);
	bStop=false;

	delete[] qwNanoSecNeeded;
	delete[] qwNanoSecStored;

	for (i=0;i<(int)lpDAI->dwNbrOfAudioStreams;lpDAI->asi[i++]->audiosource->Enable(0));

	bMuxing = false;
//	delete mms;

	DecBufferRefCount(&lpDAI->cTitle);
	FinishMuxing(lpDAI);

	lpDAI->settings->Delete();
	delete lpDAI->settings;
	delete lpDAI;

	trace.Trace(TRACE_LEVEL_INFO, "Muxing", "Muxing process finished");
//	cacheAllowReadAhead(0);
	if (bExit) c->PostMessage(WM_COMMAND, ID_LEAVE, 0);
	return 1;
}
