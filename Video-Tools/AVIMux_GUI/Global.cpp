#include "stdafx.h"
#include "global.h"

char cGlobalMuxSemaphoreName[30];
char cGlobalMuxingStartedSemaphoreName[30];

char* GlobalMuxSemaphoreName(int i) {
	return cGlobalMuxSemaphoreName+i;
}

char* GlobalMuxingStartedSemaphoreName(int i)
{
	return cGlobalMuxingStartedSemaphoreName+i;
}
