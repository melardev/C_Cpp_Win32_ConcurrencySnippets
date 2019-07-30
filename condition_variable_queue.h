#pragma once
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <queue>
#include <tchar.h>
#include <strsafe.h>

namespace ConditionVariableUsageQueue
{
	void ErrorExit(const TCHAR* lpszFunction);

	std::queue<int> queue;
	LONG LastItemProduced;

	ULONG totalItemsProduced;
	ULONG TotalItemsConsumed;

	CRITICAL_SECTION bufferLock;
	CONDITION_VARIABLE bufferCV;

	bool shouldStop;

	DWORD WINAPI ProducerThreadProc(PVOID p)
	{
		ULONG producerId = (ULONG)(ULONG_PTR)p;

		while (!shouldStop)
		{
			ULONG item = InterlockedIncrement(&LastItemProduced);

			if (shouldStop == TRUE)
			{
				break;
			}

			EnterCriticalSection(&bufferLock);

			// You may even don't need to use MAX_QUEUE_SIZE, std::queues they can grow
			// "as much as you want"

			// Insert the item at the end of the queue and increment size.
			queue.push(item);

			totalItemsProduced++;

			printf("Producer %u: item %2d, queue size %2llu\r\n", producerId, item, queue.size());

			LeaveCriticalSection(&bufferLock);

			// If a consumer is waiting, wake it.
			WakeConditionVariable(&bufferCV);

			Sleep(rand() % 1000 + 1);
		}

		printf("Producer %u exiting\r\n", producerId);
		return 0;
	}

	DWORD WINAPI ConsumerThreadProc(PVOID p)
	{
		ULONG ConsumerId = (ULONG)(ULONG_PTR)p;

		while (!shouldStop)
		{
			EnterCriticalSection(&bufferLock);


			while (queue.empty() && !shouldStop)
			{
				// Waiting for queue to have at least one element, SleepCV will release the lock(CriticalSection)
				// so the producer may acquire it. But, once Sleep wakes up, it will re-acquire the lock
				SleepConditionVariableCS(&bufferCV, &bufferLock, INFINITE);
			}

			// At this point we either broke the while loop because we woke up, 
			// or because shouldStop is true
			if (shouldStop)
			{
				LeaveCriticalSection(&bufferLock);
				break;
			}

			// Consume the first available item.

			const LONG item = queue.front();
			queue.pop();

			LeaveCriticalSection(&bufferLock); // Release the Lock

			TotalItemsConsumed++;

			printf("Consumer %u: item %2d, queue size %2llu\r\n",
			       ConsumerId, item, queue.size());

			Sleep(rand() % 1000 + 1);
		}

		printf("Consumer %u exiting\r\n", ConsumerId);
		return 0;
	}

	int main()
	{
		// This snippet shows you that you may use a single Condition variable to achieve the almost the same
		// as with the snippet provided by MSDN and adapted in condition_variable.h
		// but this code uses C++ std::queue

		// Initialize the critical section, this will be our mutex if you understand that better
		InitializeCriticalSection(&bufferLock);

		// This are the condition variables used for waiting for something, and notify waiters
		InitializeConditionVariable(&bufferCV);


		DWORD id;

		const HANDLE hThreadProducer = CreateThread(NULL, 0, ProducerThreadProc, (PVOID)1, 0, &id);
		const HANDLE hThreadProducer2 = CreateThread(NULL, 0, ProducerThreadProc, (PVOID)2, 0, &id);

		const HANDLE hThreadConsumer1 = CreateThread(NULL, 0, ConsumerThreadProc, (PVOID)1, 0, &id);
		const HANDLE hThreadConsumer2 = CreateThread(NULL, 0, ConsumerThreadProc, (PVOID)2, 0, &id);


		if (hThreadProducer == NULL || hThreadProducer2 == NULL || hThreadConsumer1 == NULL || hThreadConsumer2 == NULL)
		{
			ErrorExit(_T("CreateThread"));
			return 1;
		}

		puts("Press enter to stop...");
		getchar();

		EnterCriticalSection(&bufferLock);
		shouldStop = TRUE;
		LeaveCriticalSection(&bufferLock);

		WakeAllConditionVariable(&bufferCV);

		WaitForSingleObject(hThreadProducer, INFINITE);
		WaitForSingleObject(hThreadConsumer1, INFINITE);
		WaitForSingleObject(hThreadConsumer2, INFINITE);

		printf("TotalItemsProduced: %lu, TotalItemsConsumed: %lu\r\n",
		       totalItemsProduced, TotalItemsConsumed);
		return 0;
	}


	void ErrorExit(const TCHAR* lpszFunction)

	// Format a readable error message, display a message box, 
	// and exit from the application.
	{
		LPVOID lpMsgBuf;
		LPVOID lpDisplayBuf;
		DWORD dw = GetLastError();

		FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			dw,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR)& lpMsgBuf,
			0, NULL);

		lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT,
		                                  (lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(
			                                  TCHAR));
		StringCchPrintf((LPTSTR)lpDisplayBuf,
		                LocalSize(lpDisplayBuf) / sizeof(TCHAR),
		                TEXT("%s failed with error %d: %s"),
		                lpszFunction, dw, lpMsgBuf);
		MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK);

		LocalFree(lpMsgBuf);
		LocalFree(lpDisplayBuf);
		ExitProcess(1);
	}
}
