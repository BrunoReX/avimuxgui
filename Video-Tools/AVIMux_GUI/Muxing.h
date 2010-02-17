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

int FormatOutputFileName(char* cDest, const char* cFormat, const char* cRawFileName,
						 int iCurrentFile, SPLIT_POINT_DESCRIPTOR* pSPD,
						 int* flags = NULL);

class DEST_AVI_INFO 
{
public:
	class OutputFormat {
	public:
		enum OutputFormats {
			None = 0x00,
			AVI = 0x01,
			MKV = 0x02
		};
	};
public:
	DEST_AVI_INFO()
		: videosource(NULL),
		  vs_vfr(NULL),
		  lpProtocol(NULL),
		  asi(NULL),
		  ssi(NULL),
		  dlg(NULL),
		  dwNbrOfAudioStreams(0),
		  dwNbrOfVideoStreams(0),
		  dwNbrOfSubs(0),
		  dwMaxFiles(0),
		  iDurationFlags(0),
		  dwMaxFrames(0),
		  iMaxDuration(0),
		  lpFileName(NULL),
		  dwPadding(0),
		  lpAC3_logs(NULL),
		  dwNbrOfAC3Streams(0),
		  hDebugFile(NULL),
		  bExitAfterwards(false),
		  OutputFileFormat(OutputFormat::None),
		  iOverlapped(0),
		  hMuxingSemaphore(NULL),
		  hMuxingStartedSemaphore(NULL),
		  split_points(NULL),
		  dwEstimatedNumberOfFiles(0),
		  qwEstimatedSize(0),
		  dVideoStretchFactor(0.0),
		  settings(NULL),
		  cTitle(NULL),
		  chapters(NULL),
		  i1stTimecode(0)
	{
	}

public:
	VIDEOSOURCE*		videosource;
	VIDEOSOURCE*		vs_vfr;
	CListCtrl*			lpProtocol;
	std::vector<AUDIO_STREAM_INFO*> asi;
	std::vector<SUBTITLE_STREAM_INFO*> ssi;
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
	//char*				lpFormat;
	std::basic_string<TCHAR> fileNameFormat;

	AC3_LOG**			lpAC3_logs;
	DWORD				dwNbrOfAC3Streams;
	HANDLE				hDebugFile;
	OPENFILEOPTIONS		ofoOptions;
//	bool				bDoneDlg;
	bool				bExitAfterwards;
	OutputFormat::OutputFormats	OutputFileFormat;
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

};

/*const int DOF_AVI	=  0x01;
const int DOF_MKV	=  0x02;
*/

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