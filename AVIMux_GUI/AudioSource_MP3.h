#ifndef I_AUDIOSOURCE_MP3
#define I_AUDIOSOURCE_MP3

#include "audiosource_binary.h"

DWORD BSWAP (DWORD x);

class MP3FRAMEHEADER
{
	private:
		DWORD	dwFrameHeader;
	public:
		MP3FRAMEHEADER(void) { dwFrameHeader=0; };
		~MP3FRAMEHEADER(void) {};
		void		SetFrameHeader(DWORD _dwFrameHeader) { dwFrameHeader=BSWAP(_dwFrameHeader); };
        int         GetChannelsNum(void);
		int			GetFrameSize(int*,float* fSize = NULL);
		int			GetMPEGVersion(void);
		int			GetMPEGVersionIndex(void);
		int			GetLayerIndex(void);
		int			GetLayerVersion(void);
		int			GetBitrate(void);
		int			GetFrequencyIndex(void);
		int			GetFrequency(void);
		int			GetPadding(void);
		bool		SyncOK(void);
};

class MP3SOURCE: public AUDIOSOURCEFROMBINARY
{
	private:
		MP3FRAMEHEADER*	fh;
		DWORD			ReadFrameHeader(void);
		DWORD			dwFrequency;
		DWORD			dwNanoSecPerFrame;
		DWORD			dwBitrate;			// nur bei MP3-CBR sinnvoll
		DWORD			dwFrameSize;		// auch nur CBR
		DWORD			dwMPEGVersion;		// MPEG1 oder MPEG2
		DWORD			dwRingBuffer[1024];
		DWORD			dwRingBufferPos;
		DWORD			dwChannels;
		__int64			iFrameDuration;
		DWORD			dwLayer;
		bool			bCBR;
	protected:
		int		virtual doRead(void* lpDest,DWORD dwMicroSecDesired,DWORD* lpdwMicroSecRead,
								__int64* lpqwNanoSecRead=NULL);
		int		virtual	doClose(void);
	public:
		MP3SOURCE(void);
		MP3SOURCE(STREAM* lpStream);
		~MP3SOURCE(void);
		bool	virtual IsCBR(void) { return bCBR; }
		int		virtual Open(STREAM* lpStream);
		int		virtual	ReadFrame(void* lpDest,DWORD* lpdwMicroSecRead,__int64* lpdwNanoSecRead);
        int     virtual GetChannelCount(void);
		int		virtual	GetFrequency(void);
		__int64 virtual GetFrameDuration(void);
		int		virtual GetLayerVersion(void) { return dwLayer; }
		int		virtual	GetMicroSecPerFrame(void);
		int		virtual GetMPEGVersion(void);
		__int64	virtual	GetNanoSecPerFrame(void);
		int		virtual GetAvgBytesPerSec(void);
	//	__int64 virtual GetUnstretchedDuration(void);
		__int64	virtual FormatSpecific(__int64 iCode, __int64 iValue = 0);
		int		virtual GetFormatTag();
		int		virtual GetFrameSize(void) { return (bCBR)?dwFrameSize:0; }
		bool	virtual ScanForCBR(DWORD dwNbrOfFrames);
		bool	virtual IsAVIOutputPossible();
		void	virtual AssumeCBR(void);
};


#endif