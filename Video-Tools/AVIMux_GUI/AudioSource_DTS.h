#ifndef I_AUDIOSOURCE_DTS
#define I_AUDIOSOURCE_DTS

#define uint32 unsigned __int32

#include "audiosource_generic.h"
#include "audiosource_binary.h"
#include "..\bitstream.h"

const int DTS_FRAMETYPE_NORMAL		= 1;
const int DTS_FRAMETYPE_TERMINATION	= 0;

#pragma pack(push,1)

typedef struct
{
	uint32	dwChannels;
	float	fBitrate;
	uint32	dwFrameSize;
	__int64	nano_seconds_per_frame;
	uint32	dwFrequency;
	uint32  dwLFE;
} DTSINFO, *LPDTSINFO;

class DTSSOURCE: public CBinaryAudioSource//CBRAUDIOSOURCE
{
	private:
		DTSINFO		dtsinfo;
		BITSTREAM*	bitsource;
		int			Resync();
		int			ParseFrameHeader(DTSINFO* lpdtsinfo=NULL);
		int			ReadFrame(void* lpDest, DWORD* lpdwMicroSecRead,
								__int64 *lpqwNanoSecRead,bool bResync=false);
		int			ReadFrame(MULTIMEDIA_DATA_PACKET** packet);
	protected:
		int		virtual doRead(void* lpDest, DWORD dwMicroSecDesired, 
								DWORD* lpdwMicroSecRead,__int64* lpqwNanoSecRead);
//		int		virtual doRead(MULTIMEDIA_DATA_PACKET** dataPacket);
		int		virtual doClose();

	public:
		DTSSOURCE();
		DTSSOURCE(STREAM* lpStream);
		~DTSSOURCE();

		/* open a source stream and read it as DTS file */
		int		virtual Open(STREAM* lpStream);

		/* returns the number of audio channels */
		int		virtual GetChannelCount();

		/* returns the channels like "2" or "2.0" or "5.1" */
		std::string	virtual GetChannelString();

		/* returns the bitrate in bit/sec */
		float	virtual	GetBitrate();

		/* returns the smalltest number of bytes that can be returned;
		   this is 0 if no fixed number can be given */
		int		virtual GetGranularity();

		/* returns the average number of bytes per second */
		int		virtual GetAvgBytesPerSec(void);

		/* returns the format tag for WAVEFORMATEX */
		int		virtual GetFormatTag();

		/* returns the sample rate */
		int		virtual GetFrequency(void);

		/* returns the size of one frame in bytes */
		int		virtual GetFrameSize();

		/* returns true if the file is constant bitrate; this is
		   always true for DTS */
		bool	virtual IsCBR();

		/* returns the duration of one frame in nanoseconds */
		__int64	virtual	GetFrameDuration();

		/* returns the bytes that are identical at the beginning
		   of each frame */
		int		virtual GetStrippableHeaderBytes(void* pBuffer, int max);

};

#pragma pack(pop)

#endif