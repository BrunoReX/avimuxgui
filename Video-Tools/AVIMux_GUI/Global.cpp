#include "stdafx.h"
#include "global.h"

TCHAR cGlobalMuxSemaphoreName[30];
TCHAR cGlobalMuxingStartedSemaphoreName[30];

TCHAR* GlobalMuxSemaphoreName(int i) {
	return cGlobalMuxSemaphoreName+i;
}

TCHAR* GlobalMuxingStartedSemaphoreName(int i)
{
	return cGlobalMuxingStartedSemaphoreName+i;
}
