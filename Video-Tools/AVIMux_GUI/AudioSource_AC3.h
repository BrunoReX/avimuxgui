#ifndef I_AUDIOSOURCE_AC3
#define I_AUDIOSOURCE_AC3

#include "audiosource_generic.h"
#include "audiosource_binary.h"
//#include "messagelists.h"

/* structure containing some information about an AC3 frame */
typedef struct
{
	DWORD	dwChannels; // includes dwLFE 
	DWORD	dwLFE;
	DWORD	dwBitrate;
	DWORD	dwFrameSize;
	DWORD	dwFrequency;
	__int64 iFrameDuration;
} AC3INFO, *LPAC3INFO;

class AC3SOURCE: public CBinaryAudioSource
{
	private:
		AC3INFO			ac3info;
		bool			bUseExternalSilence;

		void*			_silence;
		byte*			lpbFirstFrame;

		/* reads the first few bytes of an AC3 frame and retrieves basic
		   information about it */
		int				ParseFrameHeader(AC3INFO* ac3info);

		int				ReadFrame(void* lpDest,DWORD* lpdwMicroSecRead,__int64 *lpqwNanoSecRead,
									bool bStoreAC3Info=false,bool bResync=false);

		/* reads one frame from an AC3 source */
		int				ReadFrame(MULTIMEDIA_DATA_PACKET** dataPacket);
	protected:
		int		virtual doRead(void* lpDest,DWORD dwMicroSecDesired,DWORD* lpdwMicroSecRead,__int64* lpqwNanoSecRead);
		int     virtual doClose();
		int		virtual ConvertCrapDataToSilence(int size, int frameSize);
	public:
		AC3SOURCE();
		AC3SOURCE(STREAM* lpStream);
		virtual ~AC3SOURCE();
//		void	virtual SetResyncCallBack (RESYNCCALLBACK lpNewCallback,DWORD dwUserData) { lpRCB=lpNewCallback; dwRCBUserData=dwUserData; }
		int		virtual Open(STREAM* lpStream);

		/* returns the number of channels; this includes the low frequency
		   effects channel */
		int		virtual GetChannelCount();

		/* returns the number of channels like "2.0" or "5.1" */
		char	virtual *GetChannelString();

		/* returns the number of bits per second */
		int		virtual	GetBitrate();

		/* returns the smallest number of bytes that can be read at once;
		   this is 1 but could be changed to $framesize */
		int		virtual GetGranularity();

		/* returns the average number of bytes per second */
		int		virtual GetAvgBytesPerSec(void);

		/* returns 0x2000 */
		int		virtual GetFormatTag();

		/* returns the duration of one frame in nanoseconds */
		__int64 virtual GetFrameDuration();

		/* returns the file's sample rate */
		int		virtual GetFrequency(void);

		/* returns the size of one AC3 frame */
		int		virtual GetFrameSize() { return ac3info.dwFrameSize; };

		/* returns true because AC3 is CBR */
		bool	virtual IsCBR();

		/* returns 0B 77; useful for Matroska header stripping compression */
		int		virtual GetStrippableHeaderBytes(void* pBuffer, int max);
};

typedef struct 
{
	DWORD				dwStream;
	DWORD				dwBrokenBytes;
	bool				bBroken;
	AC3SOURCE*			ac3source;
//	MSG_LIST*			lpMessages;
} AC3_LOG;

const int AC3RCB_INSERTSILENCE	= 0x0002;
const int AC3RCB_OK				= 0x0001;


#endif