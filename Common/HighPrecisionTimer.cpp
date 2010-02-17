#include "stdafx.h"
#include "HighPrecisionTimer.h"
#include "stdio.h"

CHighPrecisionTimer::CHighPrecisionTimer()
{
	bStarted = false;
	bStopped = false;
	QueryPerformanceFrequency(&Frequency);
}

CHighPrecisionTimer::CHighPrecisionTimer(bool DoStart)
{
	QueryPerformanceFrequency(&Frequency);
	bStopped = false;
	if (DoStart)
	{
		QueryPerformanceCounter(&StartTime);
	}
	bStarted = DoStart;
}

void CHighPrecisionTimer::Start()
{
	if (!bStarted)
	{
		QueryPerformanceCounter(&StartTime);
		bStarted = true;
	}
}

void CHighPrecisionTimer::Stop()
{
	if (bStarted && !bStopped)
	{
		QueryPerformanceCounter(&StopTime);
		bStopped = false;
		bStarted = false;
	}
}

void CHighPrecisionTimer::Format(char* pDest, int max_len) const
{
	double seconds = static_cast<double>(StopTime.QuadPart - StartTime.QuadPart) /
		static_cast<double>(Frequency.QuadPart);

	if (seconds < 0.001)
	{
		sprintf_s(pDest, max_len, "%1.0f us", seconds * 1000000);
	} else if (seconds < 1)	{
		sprintf_s(pDest, max_len, "%1.3f ms", seconds * 1000);
	} else {
		sprintf_s(pDest, max_len, "%1.3f s", seconds);
	}

}

CHighPrecisionTimer::operator char*() const
{
	Format(string+0, sizeof(string));
	return string+0;
}