#pragma once
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>


namespace ConditionVariableUsage
{
	// Adapted a little bit from https://docs.microsoft.com/en-us/windows/desktop/sync/using-condition-variables
#define BUFFER_SIZE 10
#define PRODUCER_SLEEP_TIME_MS 500
#define CONSUMER_SLEEP_TIME_MS 2000

	// We would usually use a Queue, to keep it simple let's leave a LONG array
	LONG buffer[BUFFER_SIZE];
	LONG LastItemProduced;
	ULONG queueSize;
	ULONG QueueStartOffset;

	ULONG totalItemsProduced;
	ULONG TotalItemsConsumed;

	CONDITION_VARIABLE CVNotEmptyBuffer;
	CONDITION_VARIABLE CVNotFullBuffer;
	CRITICAL_SECTION bufferMutex;

	BOOL shouldStop;

	DWORD WINAPI ProducerThreadProc(PVOID p)
	{
		ULONG producerId = (ULONG)(ULONG_PTR)p;

		while (!shouldStop)
		{
			// Produce a new item.

			Sleep(rand() % PRODUCER_SLEEP_TIME_MS);

			const ULONG item = InterlockedIncrement(&LastItemProduced);

			EnterCriticalSection(&bufferMutex);

			while (queueSize == BUFFER_SIZE && shouldStop == FALSE)
			{
				// Buffer is full.
				// SleepConditionVariable will unlock the critical section
				// and wait until someone wakes the condition variable
				// at that time, it will re-enter the critical section.
				// We are interested when the buffer length is less than max
				// this is why we use the condition variable bufferNotFull
				// which gets signaled by the Consumer Thread
				SleepConditionVariableCS(&CVNotFullBuffer, &bufferMutex, INFINITE);
			}

			if (shouldStop == TRUE)
			{
				LeaveCriticalSection(&bufferMutex);
				break;
			}

			// Insert the item at the end of the queue and increment size.

			buffer[(QueueStartOffset + queueSize) % BUFFER_SIZE] = item;
			queueSize++;
			totalItemsProduced++;

			printf("Producer %u: item %2d, queue size %2u\r\n", producerId, item, queueSize);

			LeaveCriticalSection(&bufferMutex);

			// If a consumer is waiting, wake it.

			WakeConditionVariable(&CVNotEmptyBuffer);
		}

		printf("Producer %u exiting\r\n", producerId);
		return 0;
	}

	DWORD WINAPI ConsumerThreadProc(PVOID p)
	{
		const ULONG ConsumerId = (ULONG)(ULONG_PTR)p;

		while (!shouldStop)
		{
			// This is our Lock, it should be the same as the one we used with Condition Variable
			EnterCriticalSection(&bufferMutex);

			while (queueSize == 0 && !shouldStop)
			{
				// Buffer is empty - sleep until a producer adds a new item
				// SleepCV will release the lock, and re-acquire it when wake up
				SleepConditionVariableCS(&CVNotEmptyBuffer, &bufferMutex, INFINITE);
			}

			// At this moment we have re-acquired the Lock
			// if shouldStop == TRUE then we have to release it.
			if (shouldStop == TRUE)
			{
				LeaveCriticalSection(&bufferMutex);
				continue;
			}

			// Consume the first available item.

			const LONG item = buffer[QueueStartOffset];

			queueSize--;
			QueueStartOffset++;
			TotalItemsConsumed++;

			if (QueueStartOffset == BUFFER_SIZE)
			{
				QueueStartOffset = 0;
			}

			printf("Consumer %u: item %2d, queue size %2u\r\n",
			       ConsumerId, item, queueSize);

			LeaveCriticalSection(&bufferMutex);

			// If a producer is waiting, wake it to let him know, the buffer is not full, so it can
			// add yet another value into the queue

			WakeConditionVariable(&CVNotFullBuffer);

			// Simulate processing of the item.

			Sleep(rand() % CONSUMER_SLEEP_TIME_MS);
		}

		printf("Consumer %u exiting\r\n", ConsumerId);
		return 0;
	}

	int main()
	{
		// Initialize the critical section, this will be our mutex if you understand that better
		InitializeCriticalSection(&bufferMutex);

		// This are the condition variables used for waiting for something, and notify waiters
		// When we are dealing with a max-limited shared data structure we need two CV, check
		// the snippet on CV and std::queue for an example with single CV (in that case the data structure does not have a max size)
		InitializeConditionVariable(&CVNotEmptyBuffer);
		InitializeConditionVariable(&CVNotFullBuffer);

		DWORD id;

		HANDLE hThreadProducer = CreateThread(NULL, 0, ProducerThreadProc, (PVOID)1, 0, &id);

		HANDLE hThreadConsumer1 = CreateThread(NULL, 0, ConsumerThreadProc, (PVOID)1, 0, &id);
		HANDLE hThreadConsumer2 = CreateThread(NULL, 0, ConsumerThreadProc, (PVOID)2, 0, &id);

		puts("Press enter to stop...");
		getchar();

		EnterCriticalSection(&bufferMutex);
		shouldStop = TRUE;
		LeaveCriticalSection(&bufferMutex);

		WakeAllConditionVariable(&CVNotFullBuffer);
		WakeAllConditionVariable(&CVNotEmptyBuffer);

		WaitForSingleObject(hThreadProducer, INFINITE);
		WaitForSingleObject(hThreadConsumer1, INFINITE);
		WaitForSingleObject(hThreadConsumer2, INFINITE);

		printf("TotalItemsProduced: %u, TotalItemsConsumed: %u\r\n",
		       totalItemsProduced, TotalItemsConsumed);
		return 0;
	}
}
