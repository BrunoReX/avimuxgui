#ifndef I_MODE2FORM2READER
#define I_MODE2FORM2READER

/*

MODE2FORM2SOURCE  filter for reading of Mode 2 - Form 2 - RIFF-CDXA- files
                  on CDs made with Mode2CDMaker

*/

//#include "windows.h"
#include "RIFFFile.h"
#include "..\basestreams.h"

typedef struct {
  byte sync[12];
  byte header[4];
  byte subheader[8];
  byte data[2324];
  byte edc[4];
} RAWSECTOR;


class MODE2FORM2SOURCE : public RIFFFILE, public STREAM
{
	private:
		DWORD				dwDataStart;
		DWORD				dwPosition;
		DWORD				dwSectors;
		bool				bCheckCRC;
		int					ReadRAWSector(RAWSECTOR* lpDest,DWORD* lpdwCRC_OK = NULL);
		int					ReadPartialSector(DWORD dwNbr,DWORD dwOffset,DWORD dwLength,void* lpDest);
	public:
		int			virtual CheckCRC(bool bCCRC = true);
		int			virtual Close();
		int			virtual GetGranularity();
		__int64		virtual GetPos();
		__int64		virtual	GetSize();
		int					Open(STREAM* lpSource);
		int			virtual Read(void* lpDest,DWORD dwBytes);
		int			virtual Seek(__int64 qwPos);
};

#endif