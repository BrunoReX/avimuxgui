#ifndef f_VIDEOSOURCE_I
#define f_VIDEOSOURCE_I

#include "avifile.h"
#include "..\basestreams.h"
#include "multimedia_source.h"
#include "..\matroska.h"

#include "videosource_generic.h"
#include "videosource_list.h"
#include "videosource_avi.h"
#include "videosource_matroska.h"

typedef struct
{
	DWORD*		lpdwFiles;
//	AVILIST*	lpAVIList;
	VIDEOSOURCELIST*	lpVideosourceList;

} VIDEO_SOURCE_INFORMATION;




class VIDEOFILTER : public VIDEOSOURCE
{
	private:
		VIDEOSOURCE*		lpSource;
	public:
		VIDEOFILTER(void);
		~VIDEOFILTER(void);
		VIDEOSOURCE	virtual *GetSource(void);
		int			virtual GetResolution(int* lpiWidth,int* lpiHeight);
		DWORD		virtual Open(VIDEOSOURCE* lpSource);
		DWORD		virtual Close(bool bCloseSource);
};

class FRAMERATECHANGER : public VIDEOFILTER
{
	private:
		bool				IsSourceKeyFrame(DWORD dwNbr);
		DWORD*				lpdwFrameTable;
		__int64				qwForcedNSPF;
		DWORD				dwNewNbrOfFrames;
	public:
		FRAMERATECHANGER(void);
		~FRAMERATECHANGER(void);
		bool		virtual IsKeyFrame(DWORD dwNbr);
		DWORD		virtual Open(VIDEOSOURCE* lpSource);
		__int64		virtual GetNanoSecPerFrame(void);
		DWORD		virtual GetNbrOfFrames(void);
		DWORD				SetNanoSecPerFrame(__int64 qwNSPF);
};




#endif