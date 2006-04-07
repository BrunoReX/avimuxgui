#ifndef I_GLOBAL
#define I_GLOBAL

// shall somewhen be used for joblist stuff

char* GlobalMuxSemaphoreName(int i = 0);
char* GlobalMuxingStartedSemaphoreName(int i = 0);

#define newz(a,b,c) c=new a[b]; ZeroMemory(c,b*sizeof(a))

#endif
