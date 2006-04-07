#ifndef I_FILLASIS
#define I_FILLASIS

#include "stdafx.h"
#include "audiosourcetree.h"
#include "audiosource.h"


void FillAC3_ASI (AUDIO_STREAM_INFO** asi,AC3SOURCE* ac3source);
void FillDTS_ASI (AUDIO_STREAM_INFO** asi,DTSSOURCE* dtssource);
void FillMP3_ASI (AUDIO_STREAM_INFO** asi,MP3SOURCE* mp3source);
void FillAAC_ASI (AUDIO_STREAM_INFO** asi,AACSOURCE* aacsource);

// {8D2FD10B-5841-4a6b-8905-588FEC1ADED9}
DEFINE_GUID(MEDIASUBTYPE_Vorbis2, 
			0x8d2fd10b, 0x5841, 0x4a6b, 0x89, 0x5, 0x58, 0x8f, 0xec, 0x1a, 0xde, 0xd9);

void FillVorbis_ASI (AUDIO_STREAM_INFO** asi,VORBISFROMOGG* vorbis);


#endif