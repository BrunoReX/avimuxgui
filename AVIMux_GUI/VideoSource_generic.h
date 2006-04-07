#ifndef I_VIDEOSOURCE_GENERIC
#define I_VIDEOSOURCE_GENERIC

#include "../multimedia_source.h"
#include "avifile.h"
#include <vector>

const int VS_OK			= 0x01;
const int VS_ERROR		= -0x01;
const int VS_INVALIDCALL= -0x02;

typedef struct 
{

	int		iCount;
	__int64	iThis;
	__int64	iReferences[256];
} REFERENCE_INFO;

typedef struct
{
	int		iWidth;
	int		iHeight;

	RECT	rcCrop;
} RESOLUTION;

class VIDEOSOURCE: public MULTIMEDIASOURCE
{
	private:
		void*				lpUserData;
		REFERENCE_INFO		last_ref;
		bool				bCFR;
		RESOLUTION			resOutput;
	protected:
//		void				SetLatestReference(int iCount, __int64 iRef1 = 0, __int64 iRef2 = 0);
		void				SetReferencedFramesAbsolute(int iCount, __int64 iThis, __int64 iRef1 = 0, __int64 iRef2 = 0);
		void				SetReferencedFramesAbsolute(__int64 iThis, std::vector<__int64> &iRef);
	public:
		VIDEOSOURCE(void);
		~VIDEOSOURCE(void);
		DWORD		virtual Close(bool bCloseSource);
		void		virtual GetCropping(RECT* r);
		void		virtual *GetFormat(void);
		int			virtual GetFormatSize(void);
		DWORD		virtual GetFourCC(void);
		int			virtual GetFrame(void* lpDest,DWORD* lpdwSize,__int64* lpiTimecode = NULL, ADVANCEDREAD_INFO* lpAARI = NULL);
		int			virtual GetLatestReference(int* lpiCount = NULL, __int64* lpiRef1 = NULL, __int64* lpiRef2 = NULL);
		int			virtual GetLatestReferenceAbsolute(int* lpiCount = NULL, __int64* iThis = 0, __int64* lpiRef1 = NULL, __int64* lpiRef2 = NULL);
		DWORD		virtual	GetMicroSecPerFrame(void);
		__int64		virtual	GetNanoSecPerFrame(void);
		int			virtual GetNbrOfFrames(DWORD dwKind=FT_ALL);
		DWORD		virtual GetPos(void);
		int			virtual GetResolution(int* lpdwWidth,int* lpdwHeight);
		int			virtual GetType();

		int			virtual Seek(__int64 iTime); // nanoseconds!!!
		void		virtual SetCFRFlag(bool bIsCFR);
		bool		virtual IsKeyFrame(DWORD dwNbr);
		bool		virtual IsEndOfStream(void);
		bool		virtual IsCFR(void);
		int			virtual InvalidateCache() { return 0; };
		AVIStreamHeader virtual *GetAVIStreamHeader(void);
//		void		virtual* GetUserData(void) { return lpUserData; }
//		void		virtual SetUserData(void* lpData) { lpUserData=lpData; }
		void		virtual SetOutputResolution(RESOLUTION* r);
		void		virtual GetOutputResolution(RESOLUTION* r);
};

#endif