#ifndef I_AUDIOSOURCE_DTS
#define I_AUDIOSOURCE_DTS

#include "audiosource_generic.h"
#include "audiosource_binary.h"

const int DTS_FRAMETYPE_NORMAL		= 1;
const int DTS_FRAMETYPE_TERMINATION	= 0;

#pragma pack(push,1)

typedef struct
{
	DWORD	dwChannels;
	float	fBitrate;
	DWORD	dwFrameSize;
	DWORD	dwFrequency;
} DTSINFO, *LPDTSINFO;

class DTSSOURCE: public CBRAUDIOSOURCE
{
	private:
		DTSINFO		dtsinfo;
		BITSTREAM*	bitsource;
		int			Resync();
		int			ProcessFrameHeader(DTSINFO* lpdtsinfo=NULL);
		int			ReadFrame(void* lpDest,DWORD* lpdwMicroSecRead,__int64 *lpqwNanoSecRead,bool bResync=false);
	protected:
		int		virtual doRead(void* lpDest,DWORD dwMicroSecDesired,DWORD* lpdwMicroSecRead,__int64* lpqwNanoSecRead);
		int		virtual doClose();

	public:
		DTSSOURCE();
		DTSSOURCE(STREAM* lpStream);
		~DTSSOURCE();
		int		virtual Open(STREAM* lpStream);
		int		virtual GetChannelCount();
		float	virtual	GetBitrate();
		int		virtual GetGranularity();
		int		virtual GetAvgBytesPerSec(void);
		int		virtual GetFormatTag();
		int		virtual GetFrequency(void);
		int		virtual GetFrameSize();
		bool	virtual IsCBR();

};

#pragma pack(pop)

#endif