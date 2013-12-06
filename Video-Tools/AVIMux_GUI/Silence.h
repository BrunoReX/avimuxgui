#ifndef I_SILENCE
#define I_SILENCE

#include "AudioSource_generic.h"


typedef struct
{
	DWORD	dwFormat;
	DWORD	dwChannels;
	DWORD	dwSize;
	DWORD	dwFreq;
	void*	lpData;
	float	fBitrate;
} SILENCE_DESCRIPTOR;

const int SSF_SUCCEEDED	= 0x01;
const int SSF_FAILED		= 0x00;

const int SSD_SUCCEEDED	= 0x01;
const int SSD_FAILED		= 0x00;

class SILENCE: public AUDIOSOURCE
{
	private:
		SILENCE_DESCRIPTOR*		lpSD;
		SILENCE_DESCRIPTOR		sdDesiredFormat;
		int						iCompatibleSource;
		int						iNbrOfDescs;
		int						SetDescriptor(char* lpcName,DWORD dwSize,DWORD dwFormat,DWORD dwChannels,DWORD dwFreq,
			                                  float fBitrate,SILENCE_DESCRIPTOR* lpSD);
	public:
		SILENCE();
		~SILENCE();
		int virtual Init(char* lpName=NULL);
		int virtual				Read(void*,DWORD,DWORD*,__int64*);
		int virtual				Close(void);
		int virtual				SetFormat(DWORD dwFormat,DWORD dwChannels,DWORD dwFreq,float fBitrate);
};


#endif