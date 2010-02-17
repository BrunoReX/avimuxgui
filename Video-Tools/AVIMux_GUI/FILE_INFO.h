#ifndef I_FILE_INFO
#define I_FILE_INFO

#include "avifile.h"
#include "..\basestreams.h"
#include "mode2form2reader.h"
#include "wavfile.h"
#include "..\matroska.h"
#include "audiosource_mp3.h"
#include "audiosource_aac.h"
#include "audiosource_ac3.h"
#include "audiosource_dts.h"
#include "audiosource_vorbis.h"

const int FILEINFO_FLAG0_BOLD         = 0x00000001;
const int FILEINFO_FLAG0_EMPH         = 0x00000002;
const int FILEINFO_FLAG0_DEEMPH       = 0x00000004;


typedef struct
{
	DWORD				dwType; // 1 = AVI, 2 = MP3, 4 = AC3, 8 = WAV, 16 = M2F2, 32 = DTS
	//char*				lpcName;
	CUTF8				Name;
	char				cFileformatString[32];
	STREAM*				file;
	STREAM*				source;

	union {
		AVIFILEEX*			AVIFile;
		MATROSKA*			MKVFile;
		MP3SOURCE*			MP3File;
		AACSOURCE*			AACFile;
		AC3SOURCE*          AC3File;
		DTSSOURCE*			DTSFile;
		VORBISSOURCE*		VRBFile;
		
	};

	MODE2FORM2SOURCE*	lpM2F2;
	OGGFILE*			OGGFile;
	WAVEFILE*			lpwav;
	bool				bM2F2CRC;
	bool				bInUse;
	bool				bMP3VBF_forced;
	bool				bAddedImmediately;
	int					file_id;
	int					current_pos;

	DWORD				dwFlags[1];
} FILE_INFO;

const int FILETYPE_AVI     = 0x001;
const int FILETYPE_MP3     = 0x002;
const int FILETYPE_AC3     = 0x004;
const int FILETYPE_WAV     = 0x008;
const int FILETYPE_M2F2    = 0x010;
const int FILETYPE_MASK    = 0xFEF;
const int FILETYPE_DTS     = 0x020;
const int FILETYPE_SUBS    = 0x040;
const int FILETYPE_SCRIPT  = 0x080;
const int FILETYPE_MKV     = 0x100;
const int FILETYPE_AAC     = 0x200;
const int FILETYPE_OGGVORBIS = 0x400;
const int FILETYPE_XML     = 0x800;

const int FILETYPE_UNKNOWN = 0x000;

#endif