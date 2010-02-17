#ifndef f_AVIFILE_I
#define f_AVIFILE_I

#undef AVIF_TRUSTCKTYPE
const int AVIF_TRUSTCKTYPE  = 0x800;

const int AFE_LEGACYINDEX = 0x00010001;

/*
  Für diesen Quellcode gilt die GPL

  
  Klassen für Zugriff auf AVI-Dateien, incl. OpenDML > 4 GB


  grober Aufbau einer AVI-Datei (ohne JUNKs etc):

  RIFF AVI  maximal knapp 2 GB
    LIST hdrl
	  avih
	  LIST strl  eine pro Stream
		strh
	    strf
		<strn>  Name des Streams
		<strd>  ohne wesentliche Bedeutung
		<indx>  nur OpenDML
		<vprp>  nur Videostream OpenDML, darf ignoriert werden
	  <LIST odml>  nur OpenDML
	    <dmlh>
	<LIST INFO>  unwichtig
	LIST movi
	  <LIST rec>  werden von der Klasse ignoriert
	  00dc
	  00dc
	  00dc
	  01wb
	  00dc
	  00dc
	  00dc
	  01wb
	  ...
	  ...
	  ix00  nur OpenDML
	  ix00  nur OpenDML
	  ...
	  ...
	  ix01  nur OpenDML
	  ix01  nur OpenDML
	  ...
	  ...
	idx1
  <RIFF AVIX>  nur OpenDML, beliebig viele, maximal jeweils knapp 2 GB
    LIST movi
      ..siehe oben

 
  Status:
    AVI lesen:
      Standard-AVI:
	      OK
	  OpenDML-AVI:
	      Index: Super-, Standard- und 2-Field-Index unterstützt
			       2-Field-Index nicht getestet
		  timecode-, timecode-discontinuity-table: N/A
		  Dateien > 4 GB (NTFS): funktionsfähig
	  Videodekompression: beta
		  unkomprimierte Videos werden nicht unterstützt !
		  8-Bit-Daten werden nicht unterstützt !
		  Paletten werden nicht unterstützt
		  DirectStreamCopy: OK
		  Seeken: beliebig
	  Audiodekompression: beta
		  Audiostreams konstanter Datenrate: i.O.
		  Audiostreams mit Blockgrößen > 1 Byte: nicht getestet
		  Audiostreams variabler Datenrate: (nur Direct Stream Copy) Chunkweises lesen möglich
		  DirectStreamCopy: OK
		  Seeken: beliebig
	  
	AVI Schreiben:
	  Standard-AVI:
		  OK
	  OpenDML-AVI: 
		  OK

	  Schreibcache: festgelegt auf 4 MB


	RAM-Bedarf für Index:
	  28 Bytes / Chunk, d.h. 41 kByte/min = 2,4 MB/h für Video @ 25 fps
	                                      = 10,9 MB/h für Video @ 25 fps + 2x MP3-VBR-Audio

*/



#include "RIFFFile.h"
#include "..\BaseStreams.h"
#include "math.h"
#include "AVIIndices.h"
#include <vector>

#define FRAMETYPE unsigned int
const int FT_KEYFRAME     = 0x01;
const int FT_DELTAFRAME	  = 0x02;
const int FT_DROPPEDFRAME = 0x04;
const int FT_ALL		  = 0x07;

#define AVITYPE DWORD
const int AT_STANDARD = 0x01;
const int AT_OPENDML = 0x02;
const int AT_AUTODETECT = 0x03;

#define COLORSPACE DWORD
const int CS_RGB = 0x01;
const int CS_PACKEDYUV = 0x02;
const int CS_YV12 = 0x03;
const int CS_USER = 0x04;

#define FIELDMODE DWORD
const int FM_NONE = 0x00;
const int FM_SPLIT = 0x01;
const int FM_SWAPPED = 0x02;
const int FM_DISCARD_FIRST = 0x04;
const int FM_DISCARD_SECOND = 0x08;

const int CN_NEXT_CHUNK = 0xFFFFFFFF;
const int CN_CURRENT_CHUNK = 0x7FFFFFFF;
const int CN_PREV_CHUNK = 0x7FFFFFFE;

const int PM_PROCESS = 0x01;
const int PM_DIRECTSTREAMCOPY = 0x02;
const int SPM_SETALL = 0x1000;

const int DS_ACTIVATE = 0x01;
const int DS_DEACTIVATE = 0x02;

const int OA_OK = 0x01;
const int OA_NORIFF = -0x01;
const int OA_INVALIDHEADER = -0x02;
const int OA_NOINDEX = -0x03;
const int OA_INVALIDVIDEOHANDLER = -0x04;
const int OA_INVALIDAUDIOHANDLER = -0x05;
const int OA_COULDNOTOPEN = -0x06;
const int OA_INVALIDFILE = -0x07;

const int PI_COUNTCHUNKS = 0x01;
const int PI_PROCESSINDEX = 0x02;

const int SAS_MILLISEC = 0x01;
const int SAS_BYTES = 0x02;
const int SAS_VIDEOFRAMES = 0x03;
const int SAS_VIDEOPOS = 0x04;

const int SN_JUNK = 0xffffffff;

DWORD MakeFourCC (char* lpFourCC);


// information needed for each chunk
typedef struct
{
	__int64		qwPosition;			// position within file
	unsigned int	dwOffsetFieldB;		// offset of 2nd field to qwPosition
	unsigned int	dwLength;			// size of entire chunk
	FRAMETYPE	ftFrameType;		// video only: keyframe, deltaframe, dropped frame
	__int64		qwStreamPos;		// position of chunk inside the stream
} CHUNKINFO;
//////////////////////////////////////////////////////


/////////////////////////////////////

// Infos über jeden Stream
class STREAMINFO
{
public:
	STREAMINFO();
	virtual ~STREAMINFO();
	AVIStreamHeader*		lpHeader;		// Streamheader
	void*					lpFormat;		// Streamformat
	void*					lpOutputFormat; // BMP: 24 Bit bzw. PCM: frqu./16/channels
	void*					lpIndx;			// Zeiger auf Superindex (nur OpenDML)
	DWORD					dwChunkCount;	// Anzahl Chunks im Stream
	__int64					qwStreamLength; // gesamte Länge des Streams
	DWORD					dwProcessMode;  // Direct Stream Copy ?
	DWORD					dwPos;			// aktueller Chunk
	bool					bCompressed;	// ist der Stream komprimiert ?
	bool					bDefault;		// stream is default?
	void*					lpBufIn;		// nur Audio: Eingabepuffer für Audiodekompr.
	void*					lpBufOut;		// nur Audio: Ausgabepuffer für Audiodekompr.
	DWORD					dwOffset;		// nur Audio: Offset innerhalb des aktuellen Chunks
//	CHUNKINFO*				ciChunks;
	char*					lpcName;		// Name des Streams
	std::vector<CHUNKINFO>	chunks;
};
//////////////////////////////////////////



/////////////////////////////////////////////////////

// byte alignment
#pragma pack(push,1)

#pragma warning (disable:4200)

// Header, die für OpenDML nötig sind
typedef struct _aviindex_chunk {
	FOURCC fcc;
	DWORD cb;
	WORD wLongsPerEntry; 
	BYTE bIndexSubType; 
	BYTE bIndexType; 
	DWORD nEntriesInUse; 
	DWORD dwChunkId; 
	DWORD dwReserved[3]; 
	
	struct _aviindex_entry {
		DWORD adw;
		} aIndex[ ];
} AVIINDEXCHUNK, *PAVIINDEXCHUNK;

typedef struct _avistdindex_chunk {
	FOURCC fcc; // ’ix##’
	DWORD cb;
	WORD wLongsPerEntry; 
	BYTE bIndexSubType;
	BYTE bIndexType; 
	DWORD nEntriesInUse; 
	DWORD dwChunkId; 
	_int64 qwBaseOffset;
	DWORD dwReserved3; 
	struct _avistdindex_entry {
		DWORD dwOffset; 
		DWORD dwSize; 
		} aIndex[ ];
} AVISTDINDEX, * PAVISTDINDEX;

typedef struct _avifieldindex_chunk {
	FOURCC fcc; // ’ix##’
	DWORD cb;
	WORD wLongsyPerEntry; 
	BYTE bIndexSubType;
	BYTE bIndexType; 
	DWORD nEntriesInUse; 
	DWORD dwChunkId;
	__int64 qwBaseOffset; 
	DWORD dwReserved3; 
	struct _avifieldindex_entry {
		DWORD dwOffset;
		DWORD dwSize;
		DWORD dwOffsetField2;
		} aIndex[ ];
} AVIFIELDINDEX, * PAVIFIELDINDEX;


typedef struct {
	DWORD CompressedBMHeight;
	DWORD CompressedBMWidth;
	DWORD ValidBMHeight;
	DWORD ValidBMWidth;
	DWORD ValidBMXOffset;
	DWORD ValidBMYOffset;
	DWORD VideoXOffsetInT;
	DWORD VideoYValidStartLine;
} VIDEO_FIELD_DESC;


typedef struct {
	DWORD	dwVideoFormatToken;
	DWORD	VideoFormat;
	DWORD	dwVerticalRefreshRate;
	DWORD	dwHTotalInT;
	DWORD	dwVTotalInLines;
	DWORD	dwFrameAspectRatio;
	DWORD	dwFrameWidthInPixels;
	DWORD	dwFrameHeightInLines;
	DWORD	nbFieldsPerFrame;
	VIDEO_FIELD_DESC	FieldInfo[1];
} VIDEOPROPERTYHEADER;

typedef struct _avisuperindex_chunk {
	FOURCC fcc; // ’ix##’
	DWORD cb;
	WORD wLongsPerEntry; 
	BYTE bIndexSubType; 
	BYTE bIndexType; 
	DWORD nEntriesInUse;
	DWORD dwChunkId; 
	DWORD dwReserved[3];
	struct _avisuperindex_entry {
		__int64 qwOffset;
		DWORD dwSize;
		DWORD dwDuration;
	} aIndex[ ];
} AVISUPERINDEX, * PAVISUPERINDEX;

typedef struct {
	DWORD dwTotalFrames;
	char  cFill[244];
} ODMLExtendedAVIHeader, *LPODMLExtendedAVIHeader;
///////////////////////////////////////////////////


// wenn wir mal einen Framecache für bereits dekomprimierte Frames bauen wollen...
typedef struct {
	DWORD dwAccCount;
	DWORD iFrameNbr;
} FRAMEINFO, *LPFRAMEINFO;

const int FrameCacheSize = 3;
////////////////////////////

typedef struct
{
	DWORD		dwDurationValue;
	DWORD		dwRealDuration;
	__int64	qwFilePos;
} READSUPERINDEXPROTOCOLENTRY;

typedef struct
{
	DWORD							dwEntries;
	__int64						qwFilePos;
	READSUPERINDEXPROTOCOLENTRY*	rsipEntries;	
} READSUPERINDEXPROTOCOL;

#pragma pack(pop)


class FRAME
{
	private:
		COLORSPACE		csColorSpace;
		DWORD			dwWidth,dwHeight;
		DWORD			dwBitDepth;
		bool			bExternalBuffer;
		char*			lpBuffer;
		char*			lpOrgBuffer;
		DWORD			dwBufferSize;
		DWORD			ImageSize(void);
		void*			lpUserData;
	public:
		FRAME(void);
		virtual ~FRAME(void);
		bool			IsExternalBuffer(void);
		DWORD			GetLineLength(void);
		bool			SetWidth(DWORD);
		bool			SetHeight(DWORD);
		bool			SetBitDepth(DWORD);
		bool			SetUserData(void*);
		bool			SetColorSpace(COLORSPACE csNewColorSpace);
		DWORD			GetWidth(void);
		DWORD			GetHeight(void);
		DWORD			GetBitDepth(void);
		DWORD			GetBufferSize(void);
		void*			GetUserData(DWORD);
		void*			GetBuffer(DWORD);
		bool			Realloc(void);
		bool			UseInternalBuffer(void);
		bool			UseExternalBuffer(void*,DWORD);
};




////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Das wichtigste: Zugriff auf AVIs

typedef int* (_stdcall *LEGACYSTATECALLBACK)(DWORD,DWORD,void*);
typedef void* (_stdcall *STARTLEGACYCALLBACK)(void**);
bool IsMP3SampleCount(int x);

typedef struct
{
	DWORD*		lpdwChunks;
	DWORD*		lpdwSize;
	__int64*	qwFilePos;
	__int64*	qwFileSize;
	void*		lpUserData;
} REBUILDINDEXSTATE;

typedef int (_stdcall *REBUILDINDEXSTATECALLBACK)(REBUILDINDEXSTATE*  lpris);
typedef int (_stdcall *SHALLREBUILDINDEXCALLBACK)(REBUILDINDEXSTATECALLBACK*  riscb,void*);


typedef struct
{
	DWORD	dwDelta;
	DWORD	dwKey;
	DWORD	dwDropped;
} FRAMETYPES;


typedef struct
{
	DWORD	dwHDRL;
	DWORD	dwAVIH;
	DWORD	dwMicroSecPerFrame;
	DWORD	dwRateSTRH0;
	DWORD	dwScaleSTRH0;
	DWORD	dwFlags;
	DWORD	dwTotalFrames;
} ABSOLUTEPOSITIONS;

typedef struct
{
	STARTLEGACYCALLBACK			lpslcb;
	void*						lplscbud;
	SHALLREBUILDINDEXCALLBACK	lpsri;
	void*						lpsriud;
	REBUILDINDEXSTATECALLBACK	lpriscb;
} CALLBACKFUNCTIONS;

class AVIFILEEX : public RIFFFILE
{
	private:
		FRAME*				FrameCache[FrameCacheSize];
		DWORD				dwFrameCachePos;
		bool				bOpened;
		DWORD				dwAccess;
		char*				cWritingApp;
		DWORD				dwRIFFSize;
		DWORD				dwRealFramesInRIFF;
		DWORD				dwFramesPerIndex;
		DWORD				dwFramesInCurrIndex;
		bool				bDebug;
		bool				bDummyMode;
		bool				bMoveHDRLAround;
		AVITYPE				atType; 
		__int64				qwMoviPos;
		__int64				qwFirstDataChunkPos;
		DWORD*				dwLargestChunks;
		bool				bidx1present;
		ODMLExtendedAVIHeader* 
							lpExtAVIHeader;
		AVIINDEXENTRY*		lpIdx1;
		ABSOLUTEPOSITIONS	abs_pos;
		FIELDMODE			fmFieldMode;
		BITMAPINFOHEADER    bmi24bpp;
		BITMAPINFOHEADER*   strfVideo;
		STREAMINFO*			siStreams;
//		std::vector<STREAMINFO> streams;
		DWORD				dwPadding;

		CALLBACKFUNCTIONS   cbfs;


		READSUPERINDEXPROTOCOL*	lpRSIP;
		VIDEOPROPERTYHEADER	vprp;
		FRAMETYPES			frametypes;
		DWORD				dwHDRLBufferStart;
		bool				bCreateLegacyIndexForODML;
		std::string			m_fileTitle;
		DWORD				dwMaxAllowedChunkSize;
		bool				bTryToRepairLargeChunks;
		bool				bLowOverheadMode;
		int					iStreamOfLastChunk;
		int					video_stream_index;
// R/W
		int					GetStreamNbrFromFourCC(DWORD dwFourCC);
// Öffnen
		bool				IsList(LISTHEADER* lplhListHdr,char* lpFourCC);
		bool				CheckRIFF_AVI(void);
		bool				GetAVIH(char* lpBuffer,CHUNKHEADER* lpchChunkHdr);
		bool				ProcessHDRL(char* lpBuffer,DWORD dwLength);
		bool				ProcessSTRL(char* lpBuffer,DWORD dwLength,DWORD dwStreamNbr);
		bool				ProcessIdx1(AVIINDEXENTRY* lpBuffer,DWORD dwCount);
		bool				CheckIndxCount(void);
		bool				ProcessODML(char* lpBuffer,DWORD dwLength);
		bool				ProcessExtIndex(_aviindex_chunk* lpIndx,DWORD dwStreamNbr, READSUPERINDEXPROTOCOL* lpRSIP = NULL);
		bool				ProcessBaseIndx(_aviindex_chunk* lpIndx,DWORD dwProcessMode, void* lpData = NULL);
		bool				ProcessINFO(DWORD dwLength);
		bool				ProcessHeader(void);
// read-only
		int					LoadVideoChunk(DWORD,DWORD* lpdwSize=NULL);
		int					LoadPartialChunk(DWORD,DWORD,void*);
		int					TranslateChunkNumber(DWORD,DWORD);
		int					VBR_FrameCountInChunk(DWORD dwStream, DWORD dwChunk);
		int					VBR_FrameCountTillChunk(DWORD dwStream, DWORD dwChunk);
		int					VBR_MaxFrameSize(DWORD dwStream);
// write-only
		DWORD				dwChunkCount,dwRecCount;
		DWORD				dwCacheSize;
		DWORD				dwHeaderSpace;
		DWORD				dwMoviPos;
		DWORD				dwMoviSize;
		DWORD				dwMaxRIFFAVISize,dwMaxRIFFAVIXSize;
		__int64				qwNSPF;
		CHUNK*				FirstChunk,*LastChunk;
		LIST*				FirstList,*LastList;
		DWORD				dwStdIndex_RIFFAVIXOverhead;

		INDEX*				Index,*LastIndex,*RECIndex;
		SUPERINDEX*			SupIndex,*LastSupIndex;
		_int64				qwFilePos;
		__int64				qwRIFFStart;

		DWORD				BeginRIFFAVIX(void);
		DWORD				EndRIFFAVI(void);
		DWORD				EndRIFFAVIX(void);
		bool				bRECListOpen;
		void				FillMP3VBR(AVIStreamHeader* lphdr,MPEGLAYER3WAVEFORMAT* lpmp3,STREAMINFO* siStr);
		void				FillMP3CBR(AVIStreamHeader* lphdr,MPEGLAYER3WAVEFORMAT* lpmp3,STREAMINFO* siStr);
		void				FillTXTSHeaders(AVIStreamHeader* lphdr);
		int					CreateLegacyIndexForODML(bool bLegacyIndex=true);
	public:
		MainAVIHeader*		lpMainAVIHeader;
		AVIFILEEX(void);
		virtual ~AVIFILEEX(void);
// öffnen / schließen		
		int					strfSize(DWORD dwStreamNbr,void* strf);
		DWORD				Open(STREAM* lpStream, DWORD dwAccess, AVITYPE atAVIType);
		bool				Close(bool bCloseSource=true);
		std::basic_string<TCHAR> m_fileName;
// R/W
		bool				DebugMsg (char* lpMsg);
		DWORD				GetProcessMode(DWORD dwNbr);
		_int64				GetByteStreamPos(DWORD);
		DWORD				GetCurrChunk(DWORD);
		AVITYPE				GetAVIType(void);
		DWORD				GetFrameCount(void);
		DWORD				GetMicroSecPerFrame(void);
		__int64				GetNanoSecPerFrame(void);
		DWORD				GetNbrOfStreams(void);
		int					GetAudioStreamCount(void);
		_int64				GetFileSize(void);
		DWORD				GetKindOfStream(DWORD dwStreamNbr);
		AVIStreamHeader*	GetStreamHeader(DWORD dwStreamNbr);
		void*				GetStreamFormat(DWORD dwStreamNbr);
		void*				GetStreamOutputFormat(DWORD dwStreamNbr);
		_int64				GetStreamSize(DWORD);
		int					GetVideoStreamNumber();
		bool				IsAudioStream (DWORD);
		bool				IsTextStream(DWORD dwStreamNbr);
		bool				IsVideoStream (DWORD);
		bool				IsIdx1Present(void) { return bidx1present; }
		void				SetDebugState(DWORD dwDebugState);
		void				SetFieldMode(FIELDMODE fmNewMode);
		void				SetProcessMode(DWORD dwNbr, DWORD dwProcessMode);
		char*				GetWritingAppName();
// read-only
	//	DWORD				DecompressBeginAudio(DWORD,DWORD);
	//	bool				DecompressBeginVideo(BITMAPINFOHEADER*);  // sollte NULL sein !
	//	DWORD				DecompressEndAudio(DWORD);
	//	bool				DecompressEndVideo(void);
		DWORD				FindKeyFrame(DWORD);
		ABSOLUTEPOSITIONS*	GetAbsolutePositions(void) { return &abs_pos; }
		int					GetAudioChunk(DWORD,DWORD,void*);
		DWORD				GetAudioData(DWORD,DWORD,void*);
		DWORD				GetAvgBytesPerSec(DWORD);
		int					GetChannels(DWORD);
		DWORD				GetChunkSize(DWORD,DWORD);
		_int64				GetFilePosOfChunk(DWORD dwStreamNbr,DWORD dwChunkNbr);
		DWORD				GetFormatTag(DWORD);
		void				GetFramesInFirstRIFF(DWORD*,DWORD*);
		DWORD				GetNbrOfChunks(DWORD);
		DWORD				GetNbrOfFrames(DWORD);
		int					GetStreamFrequency(DWORD);
		int					GetStreamGranularity(DWORD);
		int					GetStreamName(DWORD,char*);
		_int64				GetStreamPosOfChunk(DWORD,DWORD);
		int					GetVideoChunk(DWORD dwChunkNbr,void*,DWORD* lpdwSize=NULL);
		int					GetVideoResolution(int* lpWidth,int* lpHeight);
		std::string			GetTitle(void);
		bool				IsCBR(DWORD);
		bool				IsEndOfStream(DWORD);
		bool				IsKeyFrame(DWORD);
		bool				IsDefault(DWORD);
		DWORD				LoadAudioData(DWORD,DWORD,void*);
		int					SeekAudioStream(DWORD,_int64,DWORD);
		int					SeekByteStream(DWORD,_int64);
		int					SeekVideoStream(DWORD);
		void				SetMaxAllowedChunkSize(DWORD dwSize);
		int					SetShallRebuildIndexCallback(SHALLREBUILDINDEXCALLBACK  sri,void* lpUserData);
		void				TryToRepairLargeChunks(bool bTry);
		int					VBR_FrameCountTillPos(DWORD dwStream, __int64 iPos);
// write-only
		int					AddChunk(DWORD,void*,DWORD,DWORD, __int64* file_pos_of_chunk = NULL);
		int					BeginRECList(void);
		int					EndRECList(void);
		int					IsRecListOpen(void);
		int					FinishStream(DWORD dwStreamNbr,__int64 qwMicroSec);
		int					FlushWriteCache(void);
		DWORD				GetHeaderSpace(void);
		DWORD				GetStdIndexOverhead(void);
		DWORD				GetPadding(void);
		void				SetWritingAppName(char* cName);
		int					SetFramesPerIndex(DWORD);
		int					SetMicroSecPerFrame(DWORD);
		int					SetNanoSecPerFrame(__int64);
		int					SetNumberOfStreams(DWORD);
		int					SetOutputResolution(int x, int y);
		void				SetPadding(DWORD);
		int					SetStreamName(DWORD,char*);
		int					SetStreamHeader(DWORD,AVIStreamHeader*);
		int					SetStreamFormat(DWORD,void*);
		int					SetStreamDefault(DWORD, bool);
		void				SetTitle(const std::string& title);
		void				MoveHDRL(bool bEnabled);
		int					WriteStandardIndex(void);
// write-only, OpenDML-only
		int					Enable(int iFlag, int iValue = 1);
		void				EnableLowOverheadMode(bool bEnabled = true);

		bool				IsWriteODML();
		bool				IsLowOverheadMode();
		bool				IsLegacyEnabled(void);
		int					SetLegacyCallBack(STARTLEGACYCALLBACK stcb);
		int					SetMaxRIFFAVISize(DWORD dwMaxSize);
		int					SetMaxRIFFAVIXSize(DWORD dwMaxSize);
		READSUPERINDEXPROTOCOL*	GetLoadSuperIndexProtocol(void);

};


#endif