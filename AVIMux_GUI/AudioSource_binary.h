#ifndef I_AUDIOSOURCE_BINARY
#define I_AUDIOSOURCE_BINARY

#include "audiosource_generic.h"
#include "../basestreams.h"
#include "../dynarray.h"

// defines cue points which are collected while reading
/*typedef struct
{
	__int64		iTimecode;
	__int64		iStreamPosition;
} AUDIO_CUEPOINT;

class CUEPOINTS
{
	private:
		CDynIntArray*	points;
	protected:

	public:
		CUEPOINTS();
		void	AddPoint(__int64 iTimecode, __int64 iPosition);
		int		FindClosestPoint(__int64 iTimecode, __int64* piTimecode = NULL, __int64* piPosition = NULL);
		void	Delete();
};
*/
class AUDIOSOURCEFROMBINARY: public AUDIOSOURCE
{
	private:
		STREAM*			source;
//		CUEPOINTS*		cues;

		DWORD			dwMinFrames,dwMaxFrames;
		DWORD			dwResync_Range;
		__int64			unstretched_duration;
		bool			bEndReached;
	protected:
		int		virtual doRead(void*,DWORD,DWORD*,__int64*) { return STREAM_ERR; }
		int		virtual doClose(void);
		__int64	virtual	GetExactSize();
	public:
		AUDIOSOURCEFROMBINARY();
		~AUDIOSOURCEFROMBINARY() {};

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
		int		virtual Read(void*,DWORD,DWORD*,__int64*,__int64* lpiTimeocde = NULL,
								ADVANCEDREAD_INFO* lpAARI = NULL);
		int		virtual Seek(__int64 qwPos);
		void	virtual SetResyncRange(DWORD dwRange);
		__int64 virtual GetUnstretchedDuration();
		void	virtual ReInit();
};

/*

class CBRAUDIOSOURCE: public AUDIOSOURCEFROMBINARY
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