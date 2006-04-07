#ifndef I_VIDEOSOOURCE_MATROSKA
#define I_VIDEOSOOURCE_MATROSKA

#include "..\matroska.h"
#include "videosource_generic.h"

typedef struct
{
	MATROSKA*	m;
	int			iStream;
} VIDEOSOURCEFROMMATROSKA_DATA;


class VIDEOSOURCEFROMMATROSKA: public VIDEOSOURCE
{
	private:
		VIDEOSOURCEFROMMATROSKA_DATA	info;
		AVIStreamHeader					avistreamheader;
		READ_INFO						curr_lace;
		bool							bDelace;
		int								iPos;
		int								iBytePosInLace;
		__int64							iNextTimecode;
	protected:
		__int64		virtual GetUnstretchedDuration();
		__int64		virtual GetExactSize();
	public:
		VIDEOSOURCEFROMMATROSKA();
		VIDEOSOURCEFROMMATROSKA(MATROSKA* matroska, int iStream = -1);
		void		virtual *GetFormat(void);
		AVIStreamHeader virtual *GetAVIStreamHeader(void);
		int			virtual GetFrame(void* lpDest,DWORD* lpdwSize,__int64* lpiTimecode = NULL,
										ADVANCEDREAD_INFO* lpAARI = NULL);
		__int64		virtual	GetNanoSecPerFrame(void);
		int			virtual GetNbrOfFrames(DWORD dwKind=FT_ALL);
		int			virtual GetResolution(int* lpdwWidth,int* lpdwHeight);
		void		virtual GetOutputResolution(RESOLUTION* r);
		int			virtual Enable(int bEnabled);
		bool		virtual IsKeyFrame(DWORD dwNbr = CN_CURRENT_CHUNK);
		bool		virtual IsEndOfStream();
		char		virtual* GetIDString();
		bool		virtual IsOpen();
		int			virtual Open(MATROSKA* matroska, int iStream = -1);
		void		virtual ReInit();
		int			virtual Seek(__int64 iTime);
		int			virtual GetFormatSize(void);

		~VIDEOSOURCEFROMMATROSKA();
};

#endif