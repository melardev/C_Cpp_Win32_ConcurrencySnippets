#pragma once

#include <stdio.h>
#include <Windows.h>
#include <process.h>

namespace BeginThreadUsage
{
	struct WorkerThreadArg
	{
		bool fetchComplete;
		int computedResult;
	};

	void WorkerThreadFunc(void* arg)
	{
		const DWORD threadId = GetCurrentThreadId();
		printf("workerThreadFunc():  Thread Id is %lu, I am running on the background thread\n\n", threadId);
		WorkerThreadArg* threadArg = (WorkerThreadArg*)arg;
		printf("Performing work\n");
		Sleep(3000); // Simulate URL Fetch
		printf("Done!\n");
		threadArg->computedResult = 1000;
		threadArg->fetchComplete = true;
	}

	int main()
	{
		const DWORD threadId = GetCurrentThreadId();
		printf("main():Thread Id is %lu, I am running on the Main thread\n\n", threadId);

		WorkerThreadArg* threadArg = (WorkerThreadArg*)malloc(sizeof(WorkerThreadArg));
		threadArg->computedResult = 0;
		threadArg->fetchComplete = false;
		
		_beginthread(WorkerThreadFunc, 0, (void *)threadArg);

		while (!threadArg->fetchComplete)
		{
			Sleep(1000);
			printf("main(): Thread Id %lu, Work still in progress\n", threadId);
		}
		printf("main(): Thread Id %lu, Work complete\n", threadId);

		printf("Computed result is %d", threadArg->computedResult);

		free(threadArg);
		return 0;
	}
}
