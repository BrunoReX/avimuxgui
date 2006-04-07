#ifndef I_FORMATTEXT
#define I_FORMATTEXT

#include "videosource.h"
#include "splitpointsdlg.h"
#include "..\FormatInt64.h"
#include "..\FormatTime.h"

const int STRF_OK	 = 0x01;
const int STRF_ERROR = -0x01;

void	FormatSize(char* d,__int64 qwSize);
int		String2SplitPointDescriptor(char* c, SPLIT_POINT_DESCRIPTOR* pDesc);
int		Str2Resolution(char* c, RESOLUTION* r);
__int64	SONChapStr2Millisec(char* c);

#endif