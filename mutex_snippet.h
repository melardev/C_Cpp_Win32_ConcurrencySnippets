#pragma once
#include <Windows.h>
#include <tchar.h>
#include <synchapi.h>
#include <stdio.h>


namespace MutexUsage
{
	typedef struct ThreadArg
	{
		HANDLE mutex;
		ULONG thId;
		const char* message;
	}* PThreadArg;

	DWORD WINAPI ArgOutReflector(LPVOID arg)
	{
		ThreadArg* threadArg = (PThreadArg)arg;

		// Comment this line to see what happens, also comment LeaveCriticalSection
		WaitForSingleObject(threadArg->mutex, INFINITE);

		for (unsigned i = 0; i < 10; i++)
		{
			_tprintf(_T("Thread Id %lu; Message: %hs\n"),
			         threadArg->thId, threadArg->message);

			Sleep(20);
		}

		_tprintf(_T("\n\n\n"));
		ReleaseMutex(threadArg->mutex);

		return 0;
	}

	static void main()
	{
		HANDLE hMutex = CreateMutex(NULL,
		                            FALSE, // Initially not owned
		                            _T("MutexSnippet"));

		ThreadArg* arg1 = (ThreadArg*)malloc(sizeof(ThreadArg));
		ThreadArg* arg2 = (ThreadArg*)malloc(sizeof(ThreadArg));
		ThreadArg* arg3 = (ThreadArg*)malloc(sizeof(ThreadArg));

		char msg1[] = "Message 1";
		char msg2[] = "Message 2";
		char msg3[] = "Message 3";

		arg1->thId = 1;
		arg1->mutex = hMutex;
		arg1->message = msg1;

		arg2->thId = 2;
		arg2->mutex = hMutex;
		arg2->message = msg2;

		arg3->thId = 3;
		arg3->mutex = hMutex;
		arg3->message = msg3;


		const HANDLE hThread =
			CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ArgOutReflector, arg1, 0, NULL);

		const HANDLE hThread2 =
			CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ArgOutReflector, arg2, 0, NULL);

		const HANDLE hThread3 =
			CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ArgOutReflector, arg3, 0, NULL);

		WaitForSingleObject(hThread, INFINITE);
		WaitForSingleObject(hThread2, INFINITE);
		WaitForSingleObject(hThread3, INFINITE);

		CloseHandle(hMutex);

		free(arg1);
		free(arg2);
		free(arg3);
	}
}
