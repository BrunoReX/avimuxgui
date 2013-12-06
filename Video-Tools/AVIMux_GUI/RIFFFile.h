/*

  RIFFFILE		reading of RIFF structure files, such as WAV, AVI

*/

#ifndef F_IRIFFFILE
#define F_IRIFFFILE

#include "..\basestreams.h"

const int DT_CHUNK = 0x01;
const int DT_LIST = 0x02;
const int DT_RIFF = 0x03;

const int FA_READ = 0x01;
const int FA_WRITE = 0x02;
const int FA_DUMMY = 0x04;

typedef struct CHUNKHEADER
{
	DWORD	dwFourCC;
	DWORD   dwLength;
} *LPCHUNKHEADER;

typedef struct LISTHEADER
{
	DWORD	dwListID;
	DWORD   dwLength;
	DWORD   dwFourCC;
} *LPLISTHEADER;


class RIFFFILE
{
	private:
	    STREAM*				source;
	public:
		RIFFFILE(void);
		virtual ~RIFFFILE(void);
		bool		virtual	LocateData(DWORD dwFourCC,char** lpBuffer,DWORD* dwParentPos,void* lpDest,DWORD dwLength, DWORD dwType);
		void		virtual SetSource(STREAM* lpSource) { source=lpSource; }
		STREAM		virtual *GetSource(void) { return source; }

		HANDLE				hDebugFile;
		STREAM*				dest;
		int			virtual InvalidateCache();
};

// Zum Aufbauen von RIFF-Strukturen

#define LISTELEMENT_CHUNK  0x01
#define LISTELEMENT_LIST   0x02

const int LE_CHAIN			= 0x01;
const int LE_SINGLEELEMENT	= 0x02;
const int LE_USELASTSIZE	= 0x04;

class LISTELEMENT
{
	private:
		LISTELEMENT*	lpLast;
		
	public:
		LISTELEMENT*	lpNext;
		DWORD			dwFourCC;
		LISTELEMENT(void);
		~LISTELEMENT(void);
		DWORD			virtual GetSize(DWORD dwFlags);
		void			virtual FreeData(DWORD dwFlags);
		void			virtual *Store(void* lpDest,DWORD dwFlags);
		void			virtual StoreToStream(STREAM* lpstrDest,DWORD dwFlags);
		void			virtual *GetData(void) { return NULL; };
		int				virtual GetType(void) { return 0; };
		void			SetNext(LISTELEMENT* _lpNext);
		void			SetFourCC(DWORD _dwFourCC);
};

//const int	CHUNKSD_DONTCOPY    = 0x00000001
const int   CHUNKSD_OVERWRITE   = 0x00000002;

class CHUNK : public LISTELEMENT
{
	private:
		DWORD			dwSize, dwNextSize;
		CHUNK*			lpData;
		bool			bValid;
	public:
		CHUNK(void);
		~CHUNK(void);
		DWORD			virtual GetSize(DWORD dwFlags);
		void			virtual FreeData(DWORD dwFlags);
		void			virtual *Store(void* lpDest,DWORD dwFlags);
		void			virtual StoreToStream(STREAM* lpstrDest,DWORD dwFlags);
		void			virtual	*GetData(void);
		int				virtual GetType(void) { return LISTELEMENT_CHUNK; };
		void			virtual Set(DWORD dwFourCC, DWORD dwSize, void* pData, void* pNext);
		void			SetData(void* lpNewData,DWORD dwFlags = 0,DWORD dwOffset = 0);
		void			IncreaseSizeBy(DWORD dwAdditionalSize, DWORD* dwOldSize);
		void			SetSize(DWORD dwNewSize);
		bool			IsValid(void) { return bValid; }
};

class LIST : public LISTELEMENT
{
	private:
	    LISTELEMENT*	lpData;
		DWORD			dwLastSize;
	public:
		LIST(void);
		~LIST(void);
		DWORD			virtual GetSize(DWORD dwFlags);
		void			virtual FreeData(DWORD dwFlags);
		void			virtual *Store(void* lpDest,DWORD dwFlags);
		void			virtual StoreToStream(STREAM* lpstrDest,DWORD dwFlags);
		void			virtual *GetData(void) { return lpData; }
		int				virtual GetType(void) { return LISTELEMENT_LIST; };
		void			SetData(LISTELEMENT* lpNewData);
};

////////////////////////////////////////////////////////////////////////

DWORD MakeFourCC (char* lpFourCC);

#endif