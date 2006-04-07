/*

AUDIOSOURCE: abstract class

CBRSOURCE: generic source for constant bitrate streams
MP3SOURCE: handles MP3-VBR
AC3SOURCE: handles AC3-CBR
DTSSOURCE: handles DTS-CBR

*/


#ifndef F_AUDIOSOURCE_I
#define F_AUDIOSOURCE_I

//#include <windows.h>
#include "MessageLists.h"
#include "audiosource_generic.h"
#include "audiosource_dts.h"
#include "audiosource_ac3.h"
#include "audiosource_mp3.h"
#include "audiosource_matroska.h"
#include "audiosource_avi.h"
#include "audiosource_aac.h"
#include "audiosource_list.h"
#include "audiosource_vorbis.h"

#include "..\basestreams.h"
#include "multimedia_source.h"


/*

  non self-evident calls:
    - GetOffset(): reports the amount of crap data (in bytes) at the beginning of a stream

*/

// enough paramters for Read(). Further ones are collected here


// audio sources from a binary stream without any special container around,
// such as MP3, AC3 and DTS





#endif