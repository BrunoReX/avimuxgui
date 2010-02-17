/*

user report and bugfixing suggestion:

I am troubled with reading AAC ADTS files by AVI-Mux GUI v1.17.8.

The AAC files with the flag: protection_absent = 1 is imported correctly,
but ones with protection_absent = 0 cannot be imported.

This problem does not exist in v1.17.7.

I have checked source code of v1.17.8, and found the cause, I suppose.

At the line 118 in AudioSource_AAC.cpp,
the variable header_size is set as:
 118:    int header_size = 7; // size of ADTS header is known
and seek to the raw_data_block() and read it at the lines 133-134 as follows.
 133:    GetSource()->Seek(iPos + header_size);
 134:    int iRead = GetSource()->Read(lpDest,(int)(h.frame_length - header_size));

However, the raw_data_block() is not at (iPos + header_size)
if protection_absent = 0, as well as the size of raw_data_block()
is not (h.frame_length - header_size), because crc_check of 16 bits
exists between ADTS header and the raw_data_block().

It seems that similar problem is also included in the function:
int AACSOURCE::ReadFrame(MULTIMEDIA_DATA_PACKET** dataPacket)

This bug might be easily fixed by just adding
> > if ( !h.protection_absend  /why 'absend'?/ ) header_size += 2;


Moreover, this is true only when number_of_raw_data_blocks_in_frame = 0
( h.number_of_data_blocks  = 1).

When the number_of_raw_data_blocks_in_frame is not 0, data called
raw_data_block_position[] exist between ADTS header and crc_check.
In this case, additional crc_check is also added after each raw_data_block().

The number of the 16-bit data raw_data_block_position[] is equal to the value
of the number_of_raw_data_blocks_in_frame, (== h.number_of_data_blocks  - 1),
because the raw_data_block_position[] for the first raw_data_block() is
not necessary.

Thus, in the case of (protection_absent == 0 && number_of_raw_data_blocks_in_frame != 0),
(1) Read raw_data_block_position[] data, which are the byte-offset of
    raw_data_block() from the firstraw_data_block() in the ADTS frame.
(2) Skip one crc_check.
(3) Read one raw_data_block().
(4) Skip one crc_check.
(5) Seek to next raw_data_block_position[].
(6) Read one raw_data_block().
(7) Skip one crc_check.
(8) Repeat (5)-(7) until all raw_data_block()s in the ADTS frame are read.

The second problem regarding the number_of_raw_data_blocks_in_frame
is also in v1.17.7.

I hope the bug-fixed version of AVI-Mux GUI will be released.

Thank you in advance.

Best regards,
Takashi Yano



*/

#ifndef I_AAC_AUDIOSOURCE
#define I_AAC_AUDIOSOURCE

#include "audiosource_generic.h"
#include "audiosource_binary.h"
#include "..\bitstream.h"
#include "avifile.h"
#include <string>
#include <vector>

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

const int AAC_WFORMATTAG = 0x00FF;
const int OGG_WFORMATTAG = 0x6771;

const int aac_sampling_frequencies[] = {
	96000, 88200, 64000, 48000, 44100, 32000, 24000, 22050,
	16000, 12000, 11025,  8000,     0,     0,     0,     0 };


class AACSOURCE: public CBinaryAudioSource
{
public:
	class MPEGID {
	public:
		enum MPEGIDs {
			MPEG2 = 1,
			MPEG4 = 0
		};
	};

	class PackType {
	public:
		enum PackTypes {
			ADTS = 1,
			ADIF = 2
		};
	};

	class AdtsProfile {
	public:
		enum AdtsProfiles {
			Main = 0,
			LC = 1,
			SSR = 2,
			LTP = 3
		};
	};

	typedef struct
	{
		// fixed
		int		syncword;
		AACSOURCE::MPEGID::MPEGIDs ID;
		int		layer;
		int		protection_absent;
		AACSOURCE::AdtsProfile::AdtsProfiles profile;
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
		
		// not really part of the header
		unsigned __int16 crc16;
	} ADTSHEADER;

	class AdtsFrameHeader
	{
	public:
		AdtsFrameHeader()
		{
			memset(this, 0, sizeof(this));
		}

		bool Read(IBitStream& source);

		// helper members
		__int64 startPosition;

		// fixed
		__int16	syncword; // must be 0xFFF
		AACSOURCE::MPEGID::MPEGIDs ID;
		int layer;
		unsigned char protection_absent;
		AACSOURCE::AdtsProfile::AdtsProfiles profile;
		unsigned char sampling_fequency_index;
		unsigned char private_bit;
		unsigned char channel_configuration;
		unsigned char original_or_copy;
		unsigned char home;
		unsigned char emphasis;

		// variable
		unsigned char copyright_identification_bit;
		int		copyright_identification_start;
	
		int		frame_length; // including adts headers
		int		adts_buffer_fullness;
		int		number_of_data_blocks;
		
		// not really part of the header
		std::vector<unsigned __int16> rawDataBlockOffsets;
		unsigned __int16 crc16;
	};

	class AdtsFrame
	{
	public:
		AdtsFrameHeader Header;
	public:
		std::vector<unsigned char> RawData;
	};

	private:
		AACINFO		aacinfo;
		IBitStream*	bitsource;
		ADTSHEADER h; // adts headers of last frame that has been read

		void		ReadADTSHeader(ADTSHEADER* h);
		int			ReadFrame_ADTS(void* lpDest,DWORD* lpdwMicroSecRead,__int64 *lpqwNanoSecRead);
		int	virtual AACSOURCE::ReadFrame(MULTIMEDIA_DATA_PACKET**);

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
		int		virtual PerformCFRCheck(HANDLE* pSemaphore, DWORD*);
		int		virtual SetCFRFlag(int bFlag = false);
		int		virtual IsCFR();
		int		virtual GetFormatTag();
		int		virtual GetChannelCount();
		__int64 virtual GetFrameDuration();
		int		virtual	GetMPEGVersion();
		bool	virtual GetProfileString(std::string& result);
		int		virtual GetSampleRateIndex(int bDouble = false);
		__int64 virtual FormatSpecific(__int64 iCode, __int64 iValue = 0);
};



int	Pack2ADTS(void* lpSource, void* lpDest, AUDIOSOURCE* a, int iSize);

// WARNING!
//
// Unlike MP3, DTS and AC3 classes, this one cannot repair broken audio packing
// of source files! Broken input files result in undefined behaviour!

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