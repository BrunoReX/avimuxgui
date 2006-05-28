#ifndef I_AAC_AUDIOSOURCE
#define I_AAC_AUDIOSOURCE

#include "audiosource_generic.h"
#include "audiosource_binary.h"
#include "..\bitstream.h"
#include "avifile.h"

typedef struct
{
	DWORD	dwChannels;
	float	fBitrate;
	DWORD	dwFrameSize;
	DWORD	dwFrequency;
	DWORD   dwSampleRateIndex;
	DWORD   dwProfile;
	DWORD	dwMPEGVersion;
	int		iIsSBR;
	int		iCFR;
	int		iPackType;
} AACINFO, *LPAACINFO;

const int AAC_ADTS_PROFILE_MAIN = 0;
const int AAC_ADTS_PROFILE_LC = 1;
const int AAC_ADTS_PROFILE_SSR = 2;
const int AAC_ADTS_PROFILE_LTP = 3;

const int AAC_WFORMATTAG = 0x00FF;
const int OGG_WFORMATTAG = 0x6771;

const int aac_sampling_frequencies[] = {
	96000, 88200, 64000, 48000, 44100, 32000, 24000, 22050,
	16000, 12000, 11025,  8000,     0,     0,     0,     0 };

typedef struct
{
	// fixed
	int		syncword;
	int		ID;
	int		layer;
	int		protection_absend;
	int		profile;
	int		sampling_fequency_index;
	int		private_bit;
	int		channel_configuration;
	int		original_or_copy;
	int		home;
	int		emphasis;
	// variable
	int		copyright_identification_bit;
	int		copyright_identification_start;
	int		frame_length;
	int		adts_buffer_fullness;
	int		number_of_data_blocks;
	unsigned __int16 crc16;
} ADTSHEADER;

class AACSOURCE: public AUDIOSOURCEFROMBINARY
{
	private:
		AACINFO		aacinfo;
		BITSTREAM*	bitsource;
		ADTSHEADER h; // adts headers of last frame that has been read

		void		ReadADTSHeader(ADTSHEADER* h);
		int			ReadFrame_ADTS(void* lpDest,DWORD* lpdwMicroSecRead,__int64 *lpqwNanoSecRead);
		int			GetProfile();
	protected:
		void		SetChannelCount(int i);
		void		SetFrequency(int i);
		void		SetSampleRateIndex(int i);
		void		SetProfile(int i);
		void		SetMPEGVersion(int i);

	public:
		AACSOURCE();
		AACSOURCE(STREAM* s);
		int				Open(STREAM* s);
		int		virtual GetFrequency(void);
		int		virtual GetOutputFrequency();
		int		virtual doRead(void* lpDest,DWORD dwMicroSecDesired,DWORD* lpdwMicroSecRead,__int64* lpqwNanoSecRead);
		int     virtual doClose();
		int		virtual PerformCFRCheck(char**, DWORD*);
		int		virtual SetCFRFlag(int bFlag = false);
		int		virtual IsCFR();
		int		virtual GetFormatTag();
		int		virtual GetChannelCount();
		__int64 virtual GetFrameDuration();
		int		virtual	GetMPEGVersion();
		void	virtual GetProfileString(char* buf, int buf_len);
		int		virtual GetSampleRateIndex(int bDouble = false);
		__int64 virtual FormatSpecific(__int64 iCode, __int64 iValue = 0);
};

int	Pack2ADTS(void* lpSource, void* lpDest, AUDIOSOURCE* a, int iSize);

// WARNING!
//
// Unlike MP3, DTS and AC3 classes, this one cannot repair broken audio packing
// of source files! Broken input files result in random behaviour!

class AACFROMAVI: public AACSOURCE
{
	private:
		AVIFILEEX*	avifile;
		int			stream;
	public:
		AACFROMAVI();
		AACFROMAVI(AVIFILEEX* s, int j);
		int			Open(AVIFILEEX* s, int j);
		int			virtual doRead(void* lpDest,DWORD dwMicroSecDesired,DWORD* lpdwMicroSecRead,__int64* lpqwNanoSecRead);
		int			virtual Seek(__int64 qwPos);
		__int64		virtual GetExactSize();
		int			virtual GetAvgBytesPerSec();
		bool		virtual IsEndOfStream();
		void		virtual ReInit();
};

#endif