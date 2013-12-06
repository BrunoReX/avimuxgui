#ifndef I_MUXING
#define I_MUXING

#define DAI_IS_KB		AIU_KB
#define DAI_IS_FRAME	AIU_FRAME

#include "audiosource.h"
#include "subtitles.h"
#include "videosource.h"
//#include "audiosourcelist.h"
#include "avimux_guidlg.h"
#include "windows.h"

const int DAI_DF_FRAMES		= 0x01;
const int DAI_DF_DURATION	= 0x02;

int FormatOutputFileName(char* cDest, char* cFormat, char* cRawFileName,
						 int iCurrentFile, SPLIT_POINT_DESCRIPTOR* pSPD,
						 int* flags = NULL);

typedef struct 
{
	VIDEOSOURCE*		videosource;
	VIDEOSOURCE*		vs_vfr;
	CListCtrl*			lpProtocol;
	AUDIO_STREAM_INFO**	asi;
	SUBTITLE_STREAM_INFO**	ssi;
	CAVIMux_GUIDlg*		dlg;
	DWORD				dwNbrOfAudioStreams;
	DWORD				dwNbrOfVideoStreams;
	DWORD				dwNbrOfSubs;
	DWORD				dwMaxFiles;

// duration
	int					iDurationFlags;
	DWORD				dwMaxFrames;
	__int64				iMaxDuration;


	char*				lpFileName;
	DWORD				dwPadding;
	char*				lpFormat;
	AC3_LOG**			lpAC3_logs;
	DWORD				dwNbrOfAC3Streams;
	HANDLE				hDebugFile;
	OPENFILEOPTIONS		ofoOptions;
//	bool				bDoneDlg;
	bool				bExitAfterwards;
	int					iOutputFormat;
	int					iOverlapped;
	HANDLE				hMuxingSemaphore, hMuxingStartedSemaphore;
	CSplitPoints*		split_points;
	DWORD				dwEstimatedNumberOfFiles;
	__int64				qwEstimatedSize;

// stretch factors
	double				dVideoStretchFactor;

	CAttribs*			settings;

	// title of video
	CStringBuffer*		cTitle;

	CChapters*			chapters;
// b0rking files
	int					i1stTimecode;

} DEST_AVI_INFO;

const int DOF_AVI	=  0x01;
const int DOF_MKV	=  0x02;


typedef struct 
{
	__int64				qwWritten;
	DWORD				dwTime;
	DWORD				dwFrames;
} MUX_STATE;

typedef struct
{
	DEST_AVI_INFO*	lpDAI;
	__int64*		qwStats;
	DWORD*			dwTime;
	DWORD*			lpdwFrameCountTotal;
	MUX_STATE*		dwMuxState;
	DWORD*			dwMuxStatePos;
	DWORD			dwLeave;
	CWinThread*		lpThis;
	DWORD			dwByteAccuracy;

	__int64			iDuration;
	__int64*		lpiProgress;
} DISPLAY_PROGRESS_INFO;

#define MUXSTATEENTRIES 0x10

int MuxThread_AVI(DEST_AVI_INFO* lpDAI);
int MuxThread_MKV(DEST_AVI_INFO* lpDAI);

void FormatSize(char* d,__int64 qwSize);
void StopMuxing(bool b);
void Millisec2HMSF(__int64 qwMillisec,DWORD* lpdwH,DWORD* lpdwM,DWORD* lpdwS,DWORD* lpdwF);
bool MuxingInProgress();
bool DoStop();

#endif