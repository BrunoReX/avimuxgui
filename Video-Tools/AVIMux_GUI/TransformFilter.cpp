#include "stdafx.h"
#include "transformfilter.h"

TRANSFORMFILTER::TRANSFORMFILTER()
{
	ZeroMemory(&info,sizeof(info));
}

void TRANSFORMFILTER::IncRefCount()
{
	info.iRefCount++;
}

int TRANSFORMFILTER::DecRefCount()
{
	return !--info.iRefCount;
}

void TRANSFORMFILTER::Connect(TRANSFORMFILTER* filter)
{
	info.source = filter;
	filter->IncRefCount();
}

void TRANSFORMFILTER::Disconnect()
{
	TRANSFORMFILTER* s = (TRANSFORMFILTER*)info.source;
	
	if (s->DecRefCount()) {
		s->Close();
	};

}

int TRANSFORMFILTER::Close()
{
	return 0;
}