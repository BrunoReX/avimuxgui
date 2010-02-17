#pragma once

#include <vector>

template<class T>
class TreeItemDeleter
{
private:
	typedef void (_stdcall *fnItemDeleter)(T* item);

	typedef struct
	{
		std::vector<T*>* itemsToDelete;
		HANDLE finishedSemaphore;
		fnItemDeleter m_itemDeleter;
	} WORKER_THREAD_DATA;


private:
	std::vector<std::vector<T*>> m_dataToDelete;
	fnItemDeleter m_itemDeleter;

	DWORD lastIndexUsed;
	DWORD vectorCount;


private:
	static DWORD WINAPI WorkerThread(WORKER_THREAD_DATA* data)
	{
		for each (T* item in *(data->itemsToDelete))
			(*data->m_itemDeleter)(item);

		ReleaseSemaphore(data->finishedSemaphore, 1, NULL);
		return 0;
	}

public:
	TreeItemDeleter(fnItemDeleter deleter)
	{
		SYSTEM_INFO si;
		GetSystemInfo(&si);
		vectorCount = si.dwNumberOfProcessors + 1;
		m_dataToDelete.resize(vectorCount);
		lastIndexUsed = 0;
		m_itemDeleter = deleter;
	}

	TreeItemDeleter<T>& operator += (T* item)
	{
		m_dataToDelete[lastIndexUsed++ % vectorCount].push_back(item);
		return *this;
	}

	void operator() ()
	{
		std::vector<DWORD> threadIds;
		std::vector<HANDLE> threadFinishedHandles;
		std::vector<WORKER_THREAD_DATA*> workerData;

		threadIds.resize(vectorCount);
		workerData.resize(vectorCount);
		threadFinishedHandles.resize(vectorCount);

		for (DWORD i = 0; i < vectorCount; i++)
		{
			WORKER_THREAD_DATA* data = new WORKER_THREAD_DATA;
			workerData[i] = data;
			data->itemsToDelete = &m_dataToDelete[i];
			data->finishedSemaphore = CreateSemaphore(NULL, 0, 1, NULL);
			data->m_itemDeleter = m_itemDeleter;
			threadFinishedHandles[i] = data->finishedSemaphore;

			HANDLE threadHandle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)&WorkerThread, 
				data, NULL, &threadIds[i]);
		}
		
		WaitForMultipleObjects(threadFinishedHandles.size(), &threadFinishedHandles[0], true, INFINITE);
		
		for (DWORD i = 0; i < vectorCount; i++)
		{
			delete workerData[i];
		}
	}
};
