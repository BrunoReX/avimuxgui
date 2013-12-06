/*

Interface for subtitles.

Do NOT try to use the SUBTITLEFROMMATROSKA source for
SSA sources without wrapping a SUBTITLESOURCELIST around!


*/


#ifndef I_SUBTITLES
#define I_SUBTITLES

#include "../basestreams.h"
#include "file_info.h"
#include "../multimedia_source.h"
#include "textfiles.h"

const int SUBS_ERR = -0x01;
const int SUBS_OK  = 0x01;

typedef struct
{
	char*		lpcName;			//  0
	char*		lpcFontName;		//  1
	char*		lpcFontsize;		//  2
	char*		lpcPrimaryColour;	//  3
	char*		lpcSecondaryColour;	//  4
	char*		lpcTertiaryColour;	//  5
	char*		lpcBackColour;		//  6
	char*		lpcBold;			//  7
	char*		lpcItalic;			//  8
	char*		lpcBorderStyle;		//  9
	char*		lpcOutline;			// 10
	char*		lpcShadow;			// 11
	char*		lpcAlignment;		// 12
	char*		lpcMarginL;			// 13
	char*		lpcMarginR;			// 14
	char*		lpcMarginV;			// 15
	char*		lpcAlphaLevel;		// 16
	char*		lpcEncoding;		// 17
} SSA_STYLE_STRUCT;

typedef struct
{
	char*		lpcMarked;  // 0
	char*		lpcBegin;   // 1
	char*		lpcEnd;     // 2
	union
	{
		char*				lpcStyle;   // 3
		SSA_STYLE_STRUCT*	lpsssStyle;
	};
	char*		lpcName;    // 4
	char*		lpcMarginL; // 5
	char*		lpcMarginR; // 6
	char*		lpcMarginV; // 7
	char*		lpcEffect;  // 8
	char*		lpcText;    // 9
} SSA_ENTRY_STRUCT;

typedef struct
{
	union
	{
		SSA_ENTRY_STRUCT	sesStruct;
		char*				lpcArray[10];
	};
} SSA_SPECIFIC;

typedef struct
{
	// global
	DWORD			dwX1,dwY1,dwX2,dwY2;
	int				iCharCoding;

	// SRT stuff
	char*			lpcText;
	DWORD			dwNoPos;
	__int64			qwBegin;
	__int64			qwEnd;
	void*			lpNext;

	// SSA specific
	SSA_SPECIFIC*	lpSSA;
} SUBTITLE_DESCRIPTOR;

typedef struct
{
	char*		lpcTitle;					//  0
	char*		lpcOriginalScript;			//  1
	char*		lpcOriginalTranslation;		//  2
	char*		lpcOriginalEditing;			//  3
	char*		lpcOriginalTiming;			//  4
	char*		lpcOriginalScriptChecking;	//  5
	char*		lpcSyncPoint;				//  6
	char*		lpcScriptUpdatedBy;			//  7
	char*		lpcUpdateDetails;			//  8
	char*		lpcScriptType;				//  9
	char*		lpcCollisions;				// 10
	char*		lpcPlayResY;				// 11
	char*		lpcPlayDepth;				// 12
	char*		lpcTimer;					// 13
} SSA_HEADER_STRUCT;

typedef struct
{
	union
	{
		SSA_HEADER_STRUCT	shsStruct;
		char*				lpcArray[14];
	};
} SSA_HEADER;

typedef struct
{
	union
	{
		SSA_STYLE_STRUCT	sssStruct;
		char*				lpcArray[18];
	};
} SSA_STYLE;

const int SUB_BIAS_ABSOLUTE	= 0x01;
const int SUB_BIAS_RELATIVE	= 0x02;

const int SUBFORMAT_SRT			= 0x01;
const int SUBFORMAT_SSA			= 0x02;
const int SUBFORMAT_VOBSUB		= 0x03;


static char*	subformat_names[]=
{ "", "SRT", "SSA" };

typedef struct
{
	QUEUE*				queue; // will contain CBuffers with SUBTITLE_DESCRIPTORS

} SUBTITLESOURCE_INFO;



// basic subtitle class
class SUBTITLESOURCE: public MULTIMEDIASOURCE
{
	private:
		SUBTITLESOURCE_INFO		info;
		int	virtual				ReadLine(char* lpBuffer);
		CTextFile*				lpSource;
		DWORD					dwFormat;

	protected:
		SUBTITLE_DESCRIPTOR*	subs, *lastsub, *curr_sub;
		SSA_HEADER*				lpSSAHeader;
		SSA_STYLE**				lplpSSAStyles;
		int						iDisplayOrderCount;
		__int64					qwBegin,qwEnd,dwNbrOfStyles;
		int virtual				doClose();
		CTextFile	virtual *GetSource();
		void		virtual AddSSAStyle(SSA_STYLE* style);
		SSA_STYLE	virtual *FindSSAStyle(SSA_STYLE* style);

		int			virtual	ReadSSAHeader(void);
		int			virtual	ReadSSAStyles(void);
		int			virtual	ReadSSAEvents(void);
		int			virtual	RenderSSAScriptInfo(char** lpDest);
		int			virtual	RenderSSAStyles(char** lpDest);
		int			virtual RenderSSAHeaderAfterStyles(char** lpDest);

		void		virtual	SetSource(CTextFile* c);
		void		virtual	SetFormat(int iFormat);
		int			virtual	SSAStylesEqual(SSA_STYLE_STRUCT* lpSSA1,SSA_STYLE_STRUCT* lpSSA2);
		__int64		virtual	SSATime2NanoSec(char* lpcTime);
		int			virtual	StoreSSAHeaderInfo(char* lpcName,char* lpcInfo,char** lplpDest);
	public:
		SUBTITLESOURCE();
		int			virtual GetSSAStyleCount(void);
		SSA_HEADER  virtual* GetSSAHeader();
		SSA_STYLE	virtual* GetSSAStyle(int iIndex);
		int			virtual	GetFormat(void);		// SRT, SSA
		__int64		virtual GetNextTimecode() { return 0; }
		int			virtual GetType();
		int			virtual IsCompatible(SUBTITLESOURCE* s);
		int			virtual	Render2AVIChunk(void* lpDest);
		int			virtual	Render2Text(void* lpDest);
		int			virtual	SetRange(__int64 qwBegin, __int64 qwEnd);
		int			virtual	Read(void* lpDest, int* iSize = NULL, __int64* lpiTimecode = NULL,
									ADVANCEDREAD_INFO* lpAARI = NULL) { return 0; }
		int			virtual	RenderCodecPrivate(void* lpDest); // return size as result
};

// list of subtitles

typedef struct
{
	SUBTITLESOURCE**	subtitles;
	SUBTITLESOURCE*		active_source;
	int					iCount;
	int					iActiveSource;
} SUBTITLESOURCELIST_INFO;

class SUBTITLESOURCELIST: public SUBTITLESOURCE
{
	private:
		SUBTITLESOURCELIST_INFO		info;
	protected:
	public:
		SUBTITLESOURCELIST();
		int		virtual	Append(SUBTITLESOURCE* subtitles);
		int		virtual GetFormat();
		char	virtual* GetCodecID();
		__int64 virtual GetNextTimecode();
		int		virtual IsCompatible(SUBTITLESOURCE* s);
		int		virtual	Read(void* lpDest, int* iSize = NULL, __int64* lpiTimecode = NULL,
							ADVANCEDREAD_INFO* lpAARI = NULL);
		void	virtual ReInit();
		int		virtual Seek(__int64 iTime);
		int		virtual Enable(int bEnabled);
		int		virtual RenderCodecPrivate(void* lpDest);
		int		virtual GetCompressionAlgo();

};


typedef struct
{
	MATROSKA*	m;
	int			iStream;
	int			iFormat;
} SUBTITLESFROMMATROSKA_INFO;

class SUBTITLESFROMMATROSKA: public SUBTITLESOURCE
{
	private:
		SUBTITLESFROMMATROSKA_INFO	info;
		char*						cCodecPrivate;
		int							iCP_length;
	protected:
		__int64 virtual GetUnstretchedDuration();
		int		virtual	ReadLine(char* lpBuffer);
	public:
		SUBTITLESFROMMATROSKA();
		SUBTITLESFROMMATROSKA(MATROSKA* m, int iStream);

		int virtual		Read(void* lpDest, int* iSize = NULL, __int64* lpiTimecode = NULL,
							ADVANCEDREAD_INFO* lpAARI = NULL);
		int virtual		GetFormat() { return info.iFormat; };
		bool	virtual IsEndOfStream();
		int		virtual GetName(char* lpDest);
		int		virtual GetLanguageCode(char* lpDest);
		char	virtual* GetCodecID();
		__int64 virtual GetNextTimecode();
		int		virtual Enable(int bEnabled);
		int		virtual RenderCodecPrivate(void* lpDest);
		int		virtual GetCompressionAlgo();

};

// text sources, like plain ssa / srt files or 
class SUBTITLES: public SUBTITLESOURCE
{
	private:
		int	virtual		ReadLine(char* lpBuffer);

		int	virtual		ParseSRT();

	protected:
		int virtual		RenderSRTLine_time(SUBTITLE_DESCRIPTOR* subtitle, char** lpcDest, __int64 qwMSBegin, __int64 qwMSEnd);
		int virtual		RenderSRT2AVIChunk(void** lpDest);
		int	virtual		RenderSSAEvent(SUBTITLE_DESCRIPTOR* lpCurrSub, char** lpDest, __int64 qwMSBegin, __int64 qwMSEnd);
		int	virtual		RenderSSAEvent4MKV(SUBTITLE_DESCRIPTOR* lpCurrSub, char** lpDest, __int64 qwMSBegin, __int64 qwMSEnd);
		int virtual		RenderSSA2AVIChunk(void** lpDest);
		int virtual		Render2AVIChunk_Begin(void** lpDest,DWORD** lpdwLength);
		int	virtual		Render2AVIChunk_End(void* lpDest,DWORD* lpdwDest);
		int virtual		doClose();
	public:
		SUBTITLES(void);
		~SUBTITLES(void);
		SUBTITLE_DESCRIPTOR virtual* GetData(void);
		__int64 virtual GetNextTimecode();
		int virtual		Read(void* lpDest, int* iSize = NULL, __int64* lpiTimecode = NULL,
							ADVANCEDREAD_INFO* lpAARI = NULL);
		int	virtual		Open(CTextFile* source);
		int virtual		Merge(SUBTITLES* lpSubsToMerge,__int64 qwBias);
		int virtual		Render2AVIChunk(void* lpDest);
		int virtual		Render2Text(void* lpDest);
		int	virtual		Seek(__int64 iTime);
		__int64 virtual GetFeature(__int64 iFeature);
};

typedef struct
{
	DWORD*				lpdwFiles;
	SUBTITLESOURCE*		lpsubs;

	FILE_INFO*			lpfi;
	AVIStreamHeader*	lpash;
} SUBTITLE_STREAM_INFO;

#endif