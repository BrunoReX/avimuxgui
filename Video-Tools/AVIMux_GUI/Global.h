#ifndef I_GLOBAL
#define I_GLOBAL

// shall somewhen be used for joblist stuff

TCHAR* GlobalMuxSemaphoreName(int i = 0);
TCHAR* GlobalMuxingStartedSemaphoreName(int i = 0);

#define newz(a,b,c) c=new a[b]; ZeroMemory(c,b*sizeof(a))

#endif
