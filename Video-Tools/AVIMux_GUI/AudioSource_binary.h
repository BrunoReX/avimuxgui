#ifndef I_AUDIOSOURCE_BINARY
#define I_AUDIOSOURCE_BINARY

#include "audiosource_generic.h"
#include "../basestreams.h"
#include "../dynarray.h"

class CBinaryAudioSource: public AUDIOSOURCE
{
	private:
		STREAM*			source;

		DWORD			dwMinFrames,dwMaxFrames;
		DWORD			dwResync_Range;
		__int64			unstretched_duration;
		bool			bEndReached;

		bool			m_Open;
	protected:
		int		virtual ReadFrame(MULTIMEDIA_DATA_PACKET** dataPacket) { return 0; }
		int		virtual doRead(void*,DWORD,DWORD*,__int64*) { return STREAM_ERR; }
		int		virtual doRead(MULTIMEDIA_DATA_PACKET** dataPacket);
		int		virtual doClose(void);
		__int64	virtual	GetExactSize();

		void    virtual SetIsOpen(bool value);
		bool    virtual GetIsOpen();

		void	virtual LogFrameHeaderReadingError();
		void	virtual LogFrameDataReadingError(__int64 errorPos, int sizeExpected);
public:
		CBinaryAudioSource();
		~CBinaryAudioSource();
		int		virtual Close();
		int		virtual GetAvgBytesPerSec(void);
		int		virtual GetChannelCount(void);
		int		virtual GetFrequency(void);
		int		virtual GetGranularity(void) { return 0; }
		int		virtual GetOffset();
		int		virtual GetResyncRange(void);
		STREAM	virtual* GetSource() { return source; }
		int		virtual	Open(STREAM* lpStream);
		bool	virtual IsCBR();
		bool	virtual	IsEndOfStream(void);
		int		virtual Read(void* pBuffer, DWORD dwMicroSecDesired, DWORD* lpdwMicrosecRead,
			__int64* lpqwNanosecRead, __int64* lpiTimeocde = NULL, ADVANCEDREAD_INFO* lpAARI = NULL);
		int		virtual Read(MULTIMEDIA_DATA_PACKET** dataPacket);
		int		virtual Seek(__int64 qwPos);
		void	virtual SetResyncRange(DWORD dwRange);
		__int64 virtual GetUnstretchedDuration();
		void	virtual ReInit();
};

/*

class CBRAUDIOSOURCE: public CBinaryAudioSource
{
	private:
	protected:
		int		virtual doRead(void* lpDest,DWORD dwMicroSecDesired,DWORD* lpdwMicroSecRead,__int64 *lpqwNanoSecRead);
		int		virtual doClose();
	public:
		CBRAUDIOSOURCE() {};
		~CBRAUDIOSOURCE() {};
		int		virtual GetGranularity(void) { return GetSource()->GetGranularity(); }
};

*/

#endif