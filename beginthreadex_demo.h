#pragma once
#include <windows.h>
#include <process.h>
#include <stdio.h>

namespace BeginThreadExUsage
{
	struct WorkerThreadArg
	{
		bool fetchComplete;
		int computedResult;
	};

	unsigned __stdcall WorkerThreadFunc(void* arg)
	{
		const DWORD threadId = GetCurrentThreadId();
		printf("WorkerThreadFunc():  Thread Id is %lu, I am running on the background thread\n\n", threadId);

		WorkerThreadArg* threadArg = (WorkerThreadArg*)arg;
		// Simulate work
		Sleep(3000);
		threadArg->computedResult = 1000;
		printf("WorkerThreadFunc(): Done!\n");

		_endthreadex(0); // Optional if at the end of the function
		
		return 0;
	}

	int main()
	{
		const DWORD threadId = GetCurrentThreadId();
		printf("main():Thread Id is %lu, I am running on the Main thread\n\n", threadId);

		unsigned workerThreadId;

		WorkerThreadArg* threadArg = (WorkerThreadArg*)malloc(sizeof(WorkerThreadArg));

		threadArg->computedResult = 0;
		threadArg->fetchComplete = false;

		// _beginthreadex returns a thread handle which we can use with other APIs, as done below.
		HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, &WorkerThreadFunc, (void*)threadArg, 0, &workerThreadId);

		// Wait until thread is done
		WaitForSingleObject(hThread, INFINITE);
		printf("main(): Done waiting\n");
		printf("main(): Computed result is %d\n", threadArg->computedResult);

		// Destroy the thread object.
		CloseHandle(hThread);
		free(threadArg);

		return 0;
	}
}
