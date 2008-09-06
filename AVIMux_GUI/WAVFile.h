#ifndef I_WAVEFILE
#define I_WAVEFILE
#include "RIFFFile.h"
#include "..\basestreams.h"
#include "AVIStructs.h"

//#include "mmreg.h"

#define WAVE_OPEN_OK	+0x01
#define WAVE_OPEN_ERROR	-0x00

#define WAVE_OPEN_READ	0x2317
#define WAVE_OPEN_WRITE 0x3A7D

#define WAVE_READ_ERROR -1

#define WAVE_GENERIC_ERROR -1
#define WAVE_GENERIC_OK +1

#define WAVE_HEADER_NOT_FOUND -2

class WAVEFILE: public RIFFFILE, public STREAM
{
	private:
		DWORD	dwAccess;
		DWORD	dwCurrentPos;
		bool	bOpen;
		DWORD	dwDataStart;
		DWORD	dwStreamSize;
		WAVEFORMATEX*	lpwfe;
		int					CheckRIFF_WAVE();
		void				InitValues();
	public:
		WAVEFILE();
		virtual ~WAVEFILE();
		int			virtual	Close();
		int			virtual GetAvgBytesPerSec();
		int			virtual GetGranularity();
		__int64		virtual GetPos();
		__int64		virtual GetSize();
		WAVEFORMATEX*		GetStreamFormat();
		bool		virtual IsEndOfStream();
		int					Open(STREAM* lpStream, DWORD _dwAccess);
		int			virtual Read(void* lpDest,DWORD dwBytes);
		int			virtual	Seek(__int64 qwPos);
};

#endif

