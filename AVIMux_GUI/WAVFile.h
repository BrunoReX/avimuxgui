#ifndef I_WAVEFILE
#define I_WAVEFILE
#include "RIFFFile.h"
#include "..\basestreams.h"
#include "AVIStructs.h"

//#include "mmreg.h"

#define WAV_OK		0x01
#define WAV_ERR		0x00

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
		~WAVEFILE();
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

