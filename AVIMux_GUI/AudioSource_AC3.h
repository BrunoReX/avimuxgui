#ifndef I_AUDIOSOURCE_AC3
#define I_AUDIOSOURCE_AC3

#include "audiosource_generic.h"
#include "messagelists.h"

typedef struct
{
	DWORD	dwChannels;
	DWORD	dwBitrate;
	DWORD	dwFrameSize;
	DWORD	dwFrequency;
	__int64 iFrameDuration;
} AC3INFO, *LPAC3INFO;

class AC3SOURCE: public CBRAUDIOSOURCE
{
	private:
		AC3INFO			ac3info;
		bool			bUseExternalSilence;
		RESYNCCALLBACK  lpRCB;
		DWORD			dwRCBUserData;
		DWORD			dwReturnSilence;
		void*			_silence;
		byte*			lpbFirstFrame;
		int				ReadFrame(void* lpDest,DWORD* lpdwMicroSecRead,__int64 *lpqwNanoSecRead,
									bool bStoreAC3Info=false,bool bResync=false);
	protected:
		int		virtual doRead(void* lpDest,DWORD dwMicroSecDesired,DWORD* lpdwMicroSecRead,__int64* lpqwNanoSecRead);
		int     virtual doClose();

	public:
		AC3SOURCE();
		AC3SOURCE(STREAM* lpStream);
		void	virtual SetResyncCallBack (RESYNCCALLBACK lpNewCallback,DWORD dwUserData) { lpRCB=lpNewCallback; dwRCBUserData=dwUserData; }
		int		virtual Open(STREAM* lpStream);
		int		virtual GetChannelCount();
		int		virtual	GetBitrate();
		int		virtual GetGranularity();
		int		virtual GetAvgBytesPerSec(void);
		int		virtual GetFormatTag();
		__int64 virtual GetFrameDuration();
		int		virtual GetFrequency(void);
		int		virtual GetFrameSize() { return ac3info.dwFrameSize; };
		bool	virtual IsCBR();
};

typedef struct 
{
	DWORD				dwStream;
	DWORD				dwBrokenBytes;
	bool				bBroken;
	AC3SOURCE*			ac3source;
	MSG_LIST*			lpMessages;
} AC3_LOG;

const int AC3RCB_INSERTSILENCE	= 0x0002;
const int AC3RCB_OK				= 0x0001;


#endif