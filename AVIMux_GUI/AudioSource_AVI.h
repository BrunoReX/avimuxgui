#ifndef I_AUDIOSOURCE_AVI
#define I_AUDIOSOURCE_AVI

#include "audiosource_generic.h"
#include "avifile.h"

typedef struct
{
	AVIFILEEX*	avifile;
	int			iStream;
} AUDIOSOURCEFROMAVI_INFO;

// audio source within an AVI file, which is to be treated as 'unknown CBR'
class AUDIOSOURCEFROMAVI: public AUDIOSOURCE
{
	private:
		AUDIOSOURCEFROMAVI_INFO	info;
	protected:
		int		virtual	doClose();
		__int64	virtual	GetExactSize();

	public:
		AUDIOSOURCEFROMAVI();
		AUDIOSOURCEFROMAVI(AVIFILEEX* avifile, int iStream);
		~AUDIOSOURCEFROMAVI();
		int		virtual GetAvgBytesPerSec();
		int		virtual GetBitDepth();
		int		virtual GetChannelCount();
		int		virtual GetFormatTag();
		int		virtual GetFrequency();
		int		virtual GetGranularity();
		int		virtual	GetName(char* lpDest);
		
		int		virtual GetOffset();
		bool	virtual IsCBR();
		bool	virtual IsEndOfStream();
		int		virtual	Open(AVIFILEEX* avifile, int iStream);
		int		virtual Read(void* lpDest,DWORD dwMicrosecDesired,DWORD* lpdwMicrosecRead,
							__int64* lpqwNanosecRead,__int64* lpiTimeocde = NULL, ADVANCEDREAD_INFO* lpAARI = NULL);
		int		virtual Seek(__int64 iPos);
};

#endif