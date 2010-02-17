#ifndef I_AUDIOSOURCE_LIST
#define I_AUDIOSOURCE_LIST

#include "audiosource_generic.h"

typedef struct
{
	AUDIOSOURCE**	audiosources;
	AUDIOSOURCE*	active_source;
	int				iCount;
	int				iActiveSource;
} AUDIOSOURCELIST_INFO;

// list of joined audio sources that appears as one single audio source
class AUDIOSOURCELIST: public AUDIOSOURCE
{
	private:
		AUDIOSOURCELIST_INFO	info;
	protected:
		int		virtual	doClose();
		__int64 virtual GetUnstretchedDuration();
	public:
		AUDIOSOURCELIST();
		~AUDIOSOURCELIST();
		int		virtual	Append(AUDIOSOURCE* audiosource);
		int		virtual Enable(int bEnable);
		int		virtual GetAvgBytesPerSec();
		int		virtual GetBitDepth();
		int		virtual GetChannelCount();
		void	virtual* GetFormat();
		__int64 virtual FormatSpecific(__int64 iCode, __int64 iValue = 0);
		int     virtual GetFormatTag(void);
		int		virtual GetFrameMode(void);
		int		virtual GetFrequency();
		int		virtual GetOutputFrequency();
		__int64	virtual GetFrameDuration();
		int		virtual GetGranularity();
		char	virtual *GetCodecID();
		std::string	virtual GetChannelString();
		int		virtual GetOffset();
		__int64	virtual	GetSize();
		int		virtual	SetFrameMode(DWORD dwMode);
		bool	virtual IsCBR(void);
		int		virtual IsCompatible(AUDIOSOURCE* a);
		bool	virtual IsEndOfStream();
		int		virtual Read(void* lpDest,DWORD dwMicrosecDesired,DWORD* lpdwMicrosecRead,
							__int64* lpqwNanosecRead,__int64* lpiTimecode = NULL, ADVANCEDREAD_INFO* lpAARI = NULL);
		void	virtual ReInit();
		int		virtual Seek(__int64 iPos);
		void	virtual AssumeCBR(void);
		void	virtual AssumeVBR(void);

		int		virtual GetStrippableHeaderBytes(void* pBuffer, int max);
};
#endif