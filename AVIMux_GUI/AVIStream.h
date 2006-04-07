#ifndef I_AVISTREAM
#define I_AVISTREAM

#include "streams.h"
// erlaubt lesen eines Audiostreams aus einer AVI-Datei, 
// so als ob es eine Datei wäre
class AVISTREAM : public STREAM
{
	private:
		AVIFILEEX*	AVIFile;
		DWORD		dwStreamNbr;
	public:
		AVISTREAM(void) { dwStreamNbr=0; AVIFile=NULL; }
		AVISTREAM(AVIFILEEX* avifile, int iStreamNbr);
		~AVISTREAM(void) { Close(); }
		int					Open (AVIFILEEX* _AVIFile,DWORD _dwStreamNbr);
		int					Close (void);
		int			virtual	Read(void* lpDest,DWORD dwBytes);
		__int64		virtual GetSize(void);
		int			virtual	Seek(__int64 qwPos);
		int			virtual GetAvgBytesPerSec(void);
		int			virtual GetGranularity(void) { return AVIFile->GetStreamGranularity(dwStreamNbr); };
		int			virtual GetFrequency(void) { return AVIFile->GetStreamFrequency(dwStreamNbr); };
		int			virtual GetChannels(void) { return AVIFile->GetChannels(dwStreamNbr); };
		int			virtual SetName(char*);
		int			virtual GetName(char*);
		__int64		virtual GetPos(void);
		bool		virtual IsEndOfStream(void);
};



#endif