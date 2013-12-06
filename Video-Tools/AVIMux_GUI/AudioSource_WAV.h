#ifndef I_AUDIOSOURCE_WAV
#define I_AUDIOSOURCR_WAV

#include "AudioSource_generic.h"
#include "WAVFile.h"

class WAVSOURCE: public AUDIOSOURCE
{
private:
	WAVEFILE*	wavefile;

	WAVEFILE*	GetSource();
protected:
	__int64	virtual	GetExactSize();
	WAVEFORMATEX* GetWAVHeader();
public:
	WAVSOURCE();
	virtual ~WAVSOURCE();
	int		virtual Open(WAVEFILE* source);

	int		virtual GetAvgBytesPerSec();
	int		virtual GetBitDepth();
	int		virtual GetChannelCount();
	int		virtual GetFormatTag(void);
//	__int64 virtual GetFeature(int iFeature);
	void	virtual *GetFormat();
	int		virtual GetFrequency();
	int		virtual GetGranularity();
	__int64	virtual GetUnstretchedDuration();
	bool	virtual IsCBR();
	int		virtual IsCompatible(AUDIOSOURCE* a);
	bool	virtual IsEndOfStream();
	int		virtual Read(void* lpDest,DWORD dwMicrosecDesired,DWORD* lpdwMicrosecRead,
						__int64* lpqwNanosecRead,__int64* lpiTimeocde = NULL,
						ADVANCEDREAD_INFO* lpAARI = NULL);
	int		virtual Seek(__int64 iPos);
};

#endif