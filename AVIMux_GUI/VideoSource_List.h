#ifndef I_VIDEOSOURCE_LIST
#define I_VIDEOSOURCE_LIST

#include "videosource_generic.h"

typedef struct
{
	VIDEOSOURCE**	videosources;
	VIDEOSOURCE*	curr_source;
	int				iCount;
	int				iActiveSource;
} VIDEOSOURCELIST_INFO;

class VIDEOSOURCELIST: public VIDEOSOURCE
{
	private:
		VIDEOSOURCELIST_INFO	info;
	protected:
		__int64		virtual GetUnstretchedDuration();
	public:
		VIDEOSOURCELIST(void);
		~VIDEOSOURCELIST(void);
		int					Append(VIDEOSOURCE* pNext);
		DWORD		virtual Close(bool bCloseSource);
		int			virtual Enable(int bEnabled);
		int			virtual GetFrame(void* lpDest,DWORD* lpdwSize,__int64* lpiTimecode = NULL, ADVANCEDREAD_INFO* lpAARI = NULL);
		int			virtual GetLatestReference(int* lpiCount = NULL, __int64* lpiRef1 = NULL, __int64* lpiRef2 = NULL);
		int			virtual GetNbrOfFrames(DWORD dwKind=FT_ALL);
		void		virtual *GetFormat(void);
		int			virtual GetResolution(int* lpdwWidth,int* lpdwHeight);
		AVIStreamHeader virtual *GetAVIStreamHeader(void);
		__int64		virtual	GetNanoSecPerFrame(void);
		char		virtual* GetIDString();
		bool		virtual IsAVIOutputPossible();
		void		virtual AllowAVIOutput(bool bAllow);
		bool		virtual IsEndOfStream();
		bool		virtual IsKeyFrame(DWORD dwNbr = CN_CURRENT_CHUNK);
		__int64		virtual GetSize(void);
		void		virtual ReInit();
		int			virtual Seek(__int64 iTime);
		int			virtual GetFormatSize(void);

};

#endif