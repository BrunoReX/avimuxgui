#ifndef I_VIDEOINFORMATIONTYPES
#define I_VIDEOINFORMATIONTYPES

typedef struct
{
	__int64		qwFilePos;
	__int64		qwOldVal;
	__int64		qwNewVal;
	DWORD		dwSize;
	DWORD		dwValid;
	void*		lpNext;
} CHANGEAVIHEADER;

typedef struct
{
	DWORD		dwFlags;
	AVIFILEEX*	avifile;
} INFOLISTBOXENTRY;

#endif