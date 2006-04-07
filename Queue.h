#ifndef I_QUEUE
#define I_QUEUE

#include "buffers.h"

typedef struct QUEUE
{
	CBuffer*		buffer;
//	int				iSize;   // current total size in queue
	struct QUEUE*	pNext;   // next element in queue
	struct QUEUE*	pLast;   // last element in queue
} QUEUE;

const int QR_DONTREMOVE = 0x01;

void QUEUE_enter(QUEUE** q, CBuffer* pData);
CBuffer* QUEUE_read(QUEUE** q, int iFlags = 0);
void QUEUE_kill(QUEUE** q);
bool QUEUE_empty(QUEUE *q);
//int QUEUE_size(QUEUE *q);

#endif