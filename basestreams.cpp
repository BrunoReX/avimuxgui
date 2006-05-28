#include "stdafx.h"
#include "basestreams.h"
#include "memory.h"
#include "math.h"
#include "utf-8.h"
#include "unicodecalls.h"
#include "stdlib.h"
#include "stdio.h"
#include "crtdbg.h"
#include "Filenames.h"

#ifdef DEBUG_NEW
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif


int STREAM::GetOffset()
{
	 return iOffset;
}

int alignment_mask = 0x7FFF;

int STREAM::SetOffset(int iNewOffset)
{
	 iOffset = iNewOffset; 
	 return 1;
}

int STREAM::TruncateAt(__int64 iPosition)
{
	return 1;
}

bool STREAM::IsEnabled(int flag)
{
	return _Enable(flag, false, false);
}

bool STREAM::Enable(int flag)
{
	return _Enable(flag, true, true);
}

bool STREAM::Disable(int flag)
{
	return _Enable(flag, true, false);
}

bool STREAM::SetFlag(int flag, int value)
{
	return _Enable(flag, true, !!value);
}

STREAM_FILTER::STREAM_FILTER()
{
	source = NULL;
}

STREAM_FILTER::~STREAM_FILTER()
{
	if (GetSource())
		Close();
}

void STREAM_FILTER::SetSource(STREAM* s)
{
	source = s;
}

STREAM* STREAM_FILTER::GetSource()
{
	return source;
}

int STREAM_FILTER::Close()
{
	if (GetSource() && IsSourceAttached()) {
		GetSource()->Close();
		delete GetSource();
		SetSource(NULL);
	}

	return 1;
}

// für vernünftigen Zugriff auf Dateien > 4 GB)  
