#ifndef I_AUDIOSOURCE_MATROSKA
#define I_AUDIOSOURCE_MATROSKA

#include "..\matroska.h"
#include "audiosource_generic.h"
#include "AudioSource_AAC.h"

// audio source from an matroska file

typedef struct {
	int	 iValid;
	AACSOURCE::AdtsProfile::AdtsProfiles iProfile;
	int	 iMPEGVersion;
	int	 iSBR;
	int  iSRI[2];
} AUDIOSOURCEFROMMATROSKA_AAC_INFO;

typedef struct
{
	int		iValid;
	DWORD	dwCfg[3];
	char*	pCfg[3];
} AUDIOSOURCEFROMMATROSKA_VORBIS_INFO;

typedef struct
{
	int		iValid;
	int		iRealLayer;
	int		iCodecIDLayer;
	int		iMPEGVersion;
} AUDIOSOURCEFROMMATROSKA_MPEG_INFO;

typedef struct
{
	MATROSKA*	m;
	int			iStream;
	int			iFramesize;
	AUDIOSOURCEFROMMATROSKA_AAC_INFO 		aac;
	AUDIOSOURCEFROMMATROSKA_VORBIS_INFO		vorbis;
	AUDIOSOURCEFROMMATROSKA_MPEG_INFO		mpeg;
	int			ac3;
	int			dts;
	
} AUDIOSOURCEFROMMATROSKA_INFO;


class AUDIOSOURCEFROMMATROSKA: public AUDIOSOURCE
{
	private:
		AUDIOSOURCEFROMMATROSKA_INFO	info;
		READ_INFO						curr_lace;
		bool							bDelace;
		int								iPos;
		int								iBytePosInLace;
	protected:
		int		virtual	doClose();
		__int64	virtual GuessTotalSize();
		MATROSKA* GetSource();
		int GetSourceStream();

	public:
		AUDIOSOURCEFROMMATROSKA();
		AUDIOSOURCEFROMMATROSKA(MATROSKA* matroska, int iStream);
		virtual ~AUDIOSOURCEFROMMATROSKA();
		__int64	virtual GetUnstretchedDuration();
		int		virtual	Open(MATROSKA* matroska,int iStream);
		int		virtual Enable(int bEnable);
		int		virtual GetAvgBytesPerSec();
		int		virtual GetBitDepth();
		int		virtual GetChannelCount();
		int		virtual GetFormatTag();
		void	virtual *GetFormat();
		int		virtual GetFrequency();
		int		virtual GetOutputFrequency();
		__int64 virtual GetFrameDuration();
		int		virtual GetGranularity();
		int		virtual	GetName(char* lpDest);
		int		virtual GetLanguageCode(std::string& result);
		char	virtual *GetCodecID();
		int		virtual GetOffset();
		__int64 virtual FormatSpecific(__int64 iCode, __int64 iValue);
		bool	virtual IsCBR();
		bool	virtual IsEndOfStream();
		int		virtual Read(void* lpDest,DWORD dwMicrosecDesired,DWORD* lpdwMicrosecRead,
							__int64* lpqwNanosecRead,__int64* lpiTimeocde = NULL, ADVANCEDREAD_INFO* lpAARI = NULL);
		void	virtual ReInit();
		int		virtual Seek(__int64 iPos);
		int		virtual GetStrippableHeaderBytes(void* pBuffer, int max);
};

#endif